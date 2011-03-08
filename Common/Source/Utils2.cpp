/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils2.cpp,v 8.31 2010/12/13 00:55:29 root Exp root $
*/

#include "StdAfx.h"
#include <stdio.h>
#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#endif
#include "options.h"
#include "externs.h"
#include "XCSoar.h"
#include "InfoBoxLayout.h"
#include "Utils2.h"
#include "Cpustats.h"
#include "device.h"
#include "Logger.h"
#include "Parser.h"
#include "WaveThread.h"
#include "GaugeFLARM.h"
#include "LKUtils.h"
#include "Message.h"
#include "McReady.h"
#include "InputEvents.h"
#include "MapWindow.h"

extern void NextMapSpace();
extern void PreviousMapSpace();
extern void ShowMenu();

void BottomSounds();

#define CURTYPE	ModeType[ModeIndex]
#define ISPARAGLIDER (AircraftCategory == (AircraftCategory_t)umParaglider)

bool InitLDRotary(ldrotary_s *buf) {
short i, bsize;
#ifdef DEBUG_ROTARY
char ventabuffer[200];
FILE *fp;
#endif

	switch (AverEffTime) {
		case ae15seconds:
			bsize=15;	// useless, LDinst already there
			break;
		case ae30seconds:
			bsize=30;	// limited useful
			break;
		case ae60seconds:
			bsize=60;	// starting to be valuable
			break;
		case ae90seconds:
			bsize=90;	// good interval
			break;
		case ae2minutes:
			bsize=120;	// other software's interval
			break;
		case ae3minutes:
			bsize=180;	// probably too long interval
			break;
		default:
			bsize=3; // make it evident 
			break;
	}
	//if (bsize <3 || bsize>MAXLDROTARYSIZE) return false;
	for (i=0; i<MAXLDROTARYSIZE; i++) {
		buf->distance[i]=0;
		buf->altitude[i]=0;
		buf->ias[i]=0;
	}
	buf->totaldistance=0;
	buf->totalias=0;
	buf->start=-1;
	buf->size=bsize;
	buf->valid=false;
#ifdef DEBUG_ROTARY
	sprintf(ventabuffer,"InitLdRotary size=%d\r\n",buf->size);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
                    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
  return false; 
}

void InsertLDRotary(ldrotary_s *buf, int distance, int altitude) {
static short errs=0;
#ifdef DEBUG_ROTARY
char ventabuffer[200];
FILE *fp;
#endif
	if (CALCULATED_INFO.OnGround == TRUE) {
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"OnGround, ignore LDrotary\r\n");
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
		return;
	}
	
	if (CALCULATED_INFO.Circling == TRUE) {
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"Circling, ignore LDrotary\r\n");
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
		return;
	}


	//CALCULATED_INFO.Odometer += distance;
	if (distance<3 || distance>150) { // just ignore, no need to reset rotary
		if (errs>2) {
#ifdef DEBUG_ROTARY
			sprintf(ventabuffer,"Rotary reset after exceeding errors\r\n");
			if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
				    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
			InitLDRotary(&rotaryLD);
			errs=0;
			return;

		}
		errs++;
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"(errs=%d) IGNORE INVALID distance=%d altitude=%d\r\n",errs,distance,altitude);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
		return;
	}
	errs=0;

	if (++buf->start >=buf->size) { 
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"*** rotary reset and VALID=TRUE ++bufstart=%d >=bufsize=%d\r\n",buf->start, buf->size);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
		buf->start=0;
		buf->valid=true; // flag for a full usable buffer 
	}
	// need to fill up buffer before starting to empty it
	if ( buf->valid == true) {
		buf->totaldistance-=buf->distance[buf->start];
		buf->totalias-=buf->ias[buf->start];
	}
	buf->totaldistance+=distance;
	buf->distance[buf->start]=distance;
	// insert IAS in the rotary buffer, either real or estimated
	if (GPS_INFO.AirspeedAvailable) {
                buf->totalias += (int)GPS_INFO.IndicatedAirspeed;
                buf->ias[buf->start] = (int)GPS_INFO.IndicatedAirspeed;
	} else {
                buf->totalias += (int)CALCULATED_INFO.IndicatedAirspeedEstimated;
                buf->ias[buf->start] = (int)CALCULATED_INFO.IndicatedAirspeedEstimated;
	}
	buf->altitude[buf->start]=altitude;
#ifdef DEBUG_ROTARY
	sprintf(ventabuffer,"insert buf[%d/%d], distance=%d totdist=%d\r\n",buf->start, buf->size-1, distance,buf->totaldistance);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
		    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
}

/*
 * returns 0 if invalid, 999 if too high
 * EqMc is negative when no value is available, because recalculated and buffer still not usable
 */
double CalculateLDRotary(ldrotary_s *buf, DERIVED_INFO *Calculated ) {

	int altdiff;
	double eff;
	short bcold;
#ifdef DEBUG_ROTARY
	char ventabuffer[200];
	FILE *fp;
#endif
	double averias;
	double avertas;

	if ( CALCULATED_INFO.Circling == TRUE || CALCULATED_INFO.OnGround == TRUE) {
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"Not Calculating, on ground or circling\r\n");
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
		// StartupStore(_T("... Circling or grounded, EqMc -2 (---)\n"));
		Calculated->EqMc = -1;
		return(0);
	}

	if ( buf->start <0) {
#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"Calculate: invalid buf start<0\r\n");
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif
		// StartupStore(_T("... Invalid buf start, EqMc -2 (---)\n"));
		Calculated->EqMc = -1;
		return(0);
	}

	ldrotary_s bc;
  	memcpy(&bc, buf, sizeof(ldrotary_s));

	if (bc.valid == false ) {
		if (bc.start==0) {
			// StartupStore(_T("... bc.valid is false, EqMc -2 (---)\n"));
			Calculated->EqMc = -1;
			return(0); // unavailable
		}
		bcold=0;
	} else {

		if (bc.start < (bc.size-1))
			bcold=bc.start+1;
		else
			bcold=0;
	}

	altdiff= bc.altitude[bcold] - bc.altitude[bc.start];

	// StartupStore(_T("... bcold=%d bcstart=%d  old-start=%d\n"), bcold, bc.start, bcold-bc.start); // REMOVE
	// if ( bc.valid == true ) {
	// bcsize<=0  should NOT happen, but we check it for safety
	if ( (bc.valid == true) && bc.size>0 ) {
		averias = bc.totalias/bc.size;
		// According to Welch & Irving, suggested by Dave..
		// MC = Vso*[ (V/Vo)^3 - (Vo/V)]
		// Vso: sink at best L/D
		// Vo : speed at best L/D
		// V  : TAS

		avertas=averias*AirDensityRatio(Calculated->NavAltitude);

		double dtmp= avertas/GlidePolar::Vbestld;

		Calculated->EqMc = -1*GlidePolar::sinkratecache[GlidePolar::Vbestld] * ( (dtmp*dtmp*dtmp) - ( GlidePolar::Vbestld/avertas));
		//StartupStore(_T(".. eMC=%.2f (=%.1f)  Averias=%f Avertas=%f kmh, sinktas=%.1f ms  sinkmc0=%.1f ms Vbestld=%.1f\n"),
		//Calculated->EqMc, Calculated->EqMc, averias*TOKPH, avertas*TOKPH,-1*GlidePolar::sinkratecache[(int)avertas], 
		//GlidePolar::sinkratecache[GlidePolar::Vbestld], GlidePolar::Vbestld*TOKPH);

	}

	if (altdiff == 0 ) {
		return(INVALID_GR); // infinitum
	}
	eff= (double)bc.totaldistance / (double)altdiff;

#ifdef DEBUG_ROTARY
	sprintf(ventabuffer,"bcstart=%d bcold=%d altnew=%d altold=%d altdiff=%d totaldistance=%d eff=%d\r\n",
		bc.start, bcold,
		bc.altitude[bc.start], bc.altitude[bcold], altdiff, bc.totaldistance, eff);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
                    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
#endif

	if (eff>MAXEFFICIENCYSHOW) eff=INVALID_GR;

	return(eff);

}

#if 0
bool InitFilterBuffer(ifilter_s *buf, short bsize) {
short i;
	if (bsize <3 || bsize>RASIZE) return false;
	for (i=0; i<RASIZE; i++) buf->array[i]=0;
	buf->start=-1;
	buf->size=bsize;
	return true;
}

void InsertRotaryBuffer(ifilter_s *buf, int value) {
	if (++buf->start >=buf->size) { 
		buf->start=0;
	}
	buf->array[buf->start]=value;
}

int FilterFast(ifilter_s *buf, int minvalue, int maxvalue) {

  ifilter_s bc;
  memcpy(&bc, buf, sizeof(ifilter_s));

  short i,nc,iter;
  int s, *val;
  float aver=0.0, oldaver, low=minvalue, high=maxvalue, cutoff;

  for (iter=0; iter<MAXITERFILTER; iter++) {
 	 for (i=0, nc=0, s=0; i<bc.size; i++) {
		val=&bc.array[i];
		if (*val >=low && *val <=high) { s+=*val; nc++; }
	  }
	  if (nc==0) { aver=0.0; break; }
	  oldaver=aver; aver=((float)s/nc);
 	  //printf("Sum=%d count=%d Aver=%0.3f (old=%0.3f)\n",s,nc,aver,oldaver);
	  if (oldaver==aver) break;
	  cutoff=aver/50; // 2%
 	  low=aver-cutoff;
 	  high=aver+cutoff;
  }
  //printf("Found: aver=%d (%0.3f) after %d iterations\n",(int)aver, aver, iter);
  return ((int)aver);

}

int FilterRotary(ifilter_s *buf, int minvalue, int maxvalue) {

  ifilter_s bc;
  memcpy(&bc, buf, sizeof(ifilter_s));

  short i,curs,nc,iter;
  int s, val;
  float aver, low, high, cutoff;

  low  = minvalue;
  high = maxvalue;

  for (iter=0; iter<MAXITERFILTER; iter++) {
 	 for (i=0, nc=0, s=0,curs=bc.start; i<bc.size; i++) {

		val=bc.array[curs];
		if (val >=low && val <=high) {
			s+=val;
			nc++;
		}
		if (++curs >= bc.size ) curs=0;
	  }
	  if (nc==0) {
		aver=0.0;
		break;
	  }
	  aver=((float)s/nc);
 	  //printf("Sum=%d count=%d Aver=%0.3f\n",s,nc,aver);

	  cutoff=aver/50; // 2%
 	  low=aver-cutoff;
 	  high=aver+cutoff;
  }

  //printf("final: aver=%d\n",(int)aver);
  return ((int)aver);

}
#endif
/*
main(int argc, char *argv[])
{
  short i;
  ifilter_s buf;
  InitFilterBuffer(&buf,20);
  int values[20] = { 140,121,134,119,116,118,121,122,120,124,119,117,116,130,122,119,110,118,120,121 };
  for (i=0; i<20; i++) InsertRotaryBuffer(&buf, values[i]);
  FilterFast(&buf, 70,200 );
  buf.start=10; 
  for (i=0; i<20; i++) InsertRotaryBuffer(&buf, values[i]);
  FilterFast(&buf, 70,200 );
}
*/

/*
	Virtual Key Manager by Paolo Ventafridda

	Returns 0 if invalid virtual scan code, otherwise a valid transcoded keycode.

 */

// vkmode 0=normal 1=gesture up 2=gesture down
// however we consider a down as up, and viceversa
int ProcessVirtualKey(int X, int Y, long keytime, short vkmode) {

#define VKTIMELONG 1500
#ifndef MAP_ZOOM
#define DONTDRAWTHEMAP MapWindow::IsMapFullScreen()&&NewMap&&Look8000&&!MapWindow::EnablePan&&MapSpaceMode!=1
#else /* MAP_ZOOM */
#define DONTDRAWTHEMAP MapWindow::IsMapFullScreen()&&NewMap&&Look8000&&!MapWindow::mode.AnyPan()&&MapSpaceMode!=1
#endif /* MAP_ZOOM */

	#if 100228
	static int AIRCRAFTMENUSIZE=0;
	#else
	#define AIRCRAFTMENUSIZE	NIBLSCALE(28)
	#endif
	short yup, ydown;
	short sizeup;
	short i, j;
	short numpages=0;

	static short s_sizeright=0, s_xright=0, s_xleft=0;
	static short s_bottomY=0;
	static bool doinit=true;

	bool dontdrawthemap=(DONTDRAWTHEMAP);

	#ifdef DEBUG_PROCVK
	TCHAR buf[100];
	wsprintf(buf,_T("R=%d,%d,%d,%d, X=%d Y=%d kt=%ld"),MapWindow::MapRect.left, MapWindow::MapRect.top, 
	MapWindow::MapRect.right, MapWindow::MapRect.bottom,X,Y,keytime);
	DoStatusMessage(buf);
	#endif

	if (doinit) {

		// bottomline does not exist when infoboxes are painted, so we can make it static
		s_sizeright=MapWindow::MapRect.right-MapWindow::MapRect.left;

		// calculate left and right starting from center
		s_xleft=(s_sizeright/2)-(s_sizeright/6);
		s_xright=(s_sizeright/2)+(s_sizeright/6);

		// same for bottom navboxes: they do not exist in infobox mode
		s_bottomY=(MapWindow::MapRect.bottom-MapWindow::MapRect.top)-BottomSize-NIBLSCALE(2); // bugfix era 15, troppo 090731

		#if 100228
		#define _NOCOMPASSINCLUDE
		#include "./LKinclude_menusize.cpp"
		#endif

		doinit=false;
	}
	
	sizeup=MapWindow::MapRect.bottom-MapWindow::MapRect.top;
	// do not consider navboxes, they are processed separately
	// These are coordinates for up down center VKs
	// yup and ydown are used normally on nearest page item selection, but also for real VK
	// that currently are almost unused. 

	if (NewMap&&DrawBottom&&MapWindow::IsMapFullScreen()) {
		// Native LK mode: always fullscreen mode
		// If long click, we are processing an Enter, and we want a wider valid center area
		if ( keytime>=(VKSHORTCLICK*2)) { 
			yup=(short)((sizeup-BottomSize-TopSize)/3.7)+MapWindow::MapRect.top+TopSize;
			ydown=(short)(MapWindow::MapRect.bottom-BottomSize-((sizeup-BottomSize)/3.7));
		} else {
			yup=(short)((sizeup-BottomSize-TopSize)/2.7)+MapWindow::MapRect.top+TopSize;
			ydown=(short)(MapWindow::MapRect.bottom-BottomSize-((sizeup-BottomSize)/2.7));
		}
	} else {
		// Ibox mode, most likely
		yup=(short)(sizeup/2.7)+MapWindow::MapRect.top;
		ydown=(short)(MapWindow::MapRect.bottom-(sizeup/2.7));
	}
//	TCHAR buf[100];  VENTA REMOVE
//	wsprintf(buf,_T("sizeup=%d BottomSize=%d yup=%d ydown=%d"),
//		sizeup, BottomSize, yup, ydown);
//	DoStatusMessage(buf);

	#ifdef DEBUG_PROCVK
	TCHAR buf[100];
	#endif

	// Handle fullscreen 8000 mode 
	// sound clicks require some attention here
#ifndef MAP_ZOOM
	if (NewMap &&  DrawBottom && !MapWindow::EnablePan && vkmode==LKGESTURE_NONE) { 
#else /* MAP_ZOOM */
	if (NewMap &&  DrawBottom && !MapWindow::mode.AnyPan() && vkmode==LKGESTURE_NONE) { 
#endif /* MAP_ZOOM */
		//
		// CLICKS on NAVBOXES, any MapSpaceMode ok
		//
		if (Y>= s_bottomY ) { // TESTFIX 090930

			if ( UseMapLock ) {
				if (MapLock==false) {
#ifndef MAP_ZOOM
					if (!MapWindow::EnablePan ) LockMap();
#else /* MAP_ZOOM */
					if (!MapWindow::mode.AnyPan()) LockMap();
#endif /* MAP_ZOOM */
					MapWindow::RefreshMap();
					return 0;
				}
			}
			if ( X>s_xright ) {
				// standard configurable mode
				if (keytime >=CustomKeyTime) {
					// 2 is right key
					if (CustomKeyHandler(CKI_BOTTOMRIGHT)) return 0;
				}
				#ifdef DEBUG_PROCVK
				wsprintf(buf,_T("RIGHT in limit=%d"),sizeup-BottomSize-NIBLSCALE(20));
				DoStatusMessage(buf);
				#endif
				if (  (BottomMode+1) >BM_LAST ) {
#ifndef MAP_ZOOM
					if ( DisplayMode == dmCircling)
#else /* MAP_ZOOM */
					if ( MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
#endif /* MAP_ZOOM */
						BottomMode=BM_TRM;
					else
						BottomMode=BM_FIRST;
					BottomSounds();
					MapWindow::RefreshMap();
					return 0;
				} else ++BottomMode;
				BottomSounds(); // 100402
/*
				#ifndef DISABLEAUDIO
			        if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				#endif
*/
				MapWindow::RefreshMap();
				return 0;
			}
			if ( X<s_xleft ) { // following is ugly
				if (keytime >=CustomKeyTime) {
					// 1 is left key
					if (CustomKeyHandler(CKI_BOTTOMLEFT)) return 0;
				}

				#ifdef DEBUG_PROCVK
				wsprintf(buf,_T("LEFT in limit=%d"),sizeup-BottomSize-NIBLSCALE(20));
				DoStatusMessage(buf);
				#endif
				if ((BottomMode-1) == BM_TRM) {
#ifndef MAP_ZOOM
					if (DisplayMode != dmCircling) BottomMode=BM_LAST;
#else /* MAP_ZOOM */
					if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) BottomMode=BM_LAST;
#endif /* MAP_ZOOM */
					else {
						BottomMode=BM_TRM;
						/*
						#ifndef DISABLEAUDIO
                                        	if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
						#endif
						*/
						BottomSounds();
						MapWindow::RefreshMap();
						return 0;
					}
				}
				else if ((BottomMode-1)<0) {
					BottomMode=BM_LAST;
#ifndef MAP_ZOOM
				} else if ( ((BottomMode-1)==BM_FIRST)&& (DisplayMode!=dmCircling)) {
#else /* MAP_ZOOM */
				} else if ( ((BottomMode-1)==BM_FIRST)&& !MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
#endif /* MAP_ZOOM */
					/*
					#ifndef DISABLEAUDIO
                                       	if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
					#endif
					*/
					BottomMode--;
					BottomSounds();
					MapWindow::RefreshMap();
					return 0;
				} else BottomMode--;
				BottomSounds(); // 100402
/*
				#ifndef DISABLEAUDIO
			        if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				#endif
*/
				MapWindow::RefreshMap();
				return 0;
			}
			#ifdef DEBUG_PROCVK
			wsprintf(buf,_T("CENTER in limit=%d"),sizeup-BottomSize-NIBLSCALE(20));
			DoStatusMessage(buf);
			#endif

			//
			// VIRTUAL CENTER KEY HANDLING
			//
			// long press on center navbox 
			// Activate following choices for testing and experimenting. Always disable for real usage.
#if (0)
			// Output NMEA to device
			if (keytime>1000) {
				#ifndef DISABLEAUDIO
				if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
				#endif
				devWriteNMEAString(devA(),_T("$PGRMCE"));
				Message::AddMessage(1000, 3, _T("NMEA out $PGRMCE"));
				return 0;
			}
#endif
#if (0)
			// Simulate incoming NMEA string
			if (keytime>1000) {
				static TCHAR mbuf[200];
				wsprintf(mbuf,_T("$VARIO,1010.18,0.0,0.00,2.34,2,000.0,000.0*51\n"));
				NMEAParser::ParseNMEAString(0, (TCHAR *) mbuf, &GPS_INFO);
				return 0;
			}
#endif
#if (0)	// TESTKEY
			// Print a message on the screen for debugging purposes
			TCHAR mbuf[100];
			if (keytime>1000) {
				// _stprintf(mbuf,_T("Cache MCA %d/%d F=%d"), Cache_Hits_MCA, Cache_Calls_MCA, Cache_False_MCA );
				char *point;
				point=(char*)&mbuf;
				*point++='A';
				*point++='\0';
				*point++='B';
				*point++='\0';
				*point++=0x06;
				*point++=0x01;
				*point++='C';
				*point++='\0';
				*point++='\0';
				*point++='\0';
				//mbuf[1]=0xc4;
				//mbuf[1]=0x86;
				Message::Lock();
				Message::AddMessage(20000, 3, mbuf);
				Message::Unlock();
				return 0;
			}
#endif

#if (0)
			if (keytime>=CustomKeyTime) {
				if (OvertargetMode==OVT_MAXMODE) OvertargetMode=0;
				else OvertargetMode++;
				#ifndef DISABLEAUDIO
				if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
				#endif
				return 0;
			}
#endif
			// REAL USAGE, ALWAYS ACTIVATE 
			#if (1)
			// standard configurable mode
			if (keytime >=CustomKeyTime) {
				// 0 is center key
				if (CustomKeyHandler(CKI_BOTTOMCENTER)) return 0;
			}
			#endif

			// normally, we fall down here.
			// If CustomKeyHandler returned false, back as well here (nothing configured in custom).
			NextModeIndex();
			MapWindow::RefreshMap();
			SoundModeIndex();

			return 0;
		// End click on navboxes 
		} else 
		// CLICK ON SORTBOX line at the top, only with no map and only for nearest
		if ( (MapSpaceMode == MSM_LANDABLE || MapSpaceMode==MSM_AIRPORTS || 
			MapSpaceMode==MSM_NEARTPS || MapSpaceMode==MSM_TRAFFIC) && Y<=SortBoxY ) {

			// only search for 1-3, otherwise it's the fourth (fifth really)
			// we don't use 0 now
			for (i=0, j=4; i<4; i++) { // i=1 original 090925 FIX
				if (X <SortBoxX[i]) {
					j=i;
					break;
				}
			}
			switch(MapSpaceMode) {
				case MSM_LANDABLE:
				case MSM_AIRPORTS:
				case MSM_NEARTPS:
							SortedMode[MapSpaceMode]=j;
							LKForceDoNearest=true;
							#ifndef DISABLEAUDIO
							if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
							#endif
							break;
				case MSM_TRAFFIC:
							SortedMode[MapSpaceMode]=j;
							// force immediate resorting
							LastDoTraffic=0;
							#ifndef DISABLEAUDIO
							if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
							#endif
							break;
				default:
							DoStatusMessage(_T("ERR-022 UNKNOWN MSM in VK"));
							break;
			}
			SelectedPage[MapSpaceMode]=0;
			SelectedRaw[MapSpaceMode]=0;
			MapWindow::RefreshMap();

			return 0;
		// end sortbox
		}  
	// end newmap  with no gestures
	}

	// REAL virtual keys
	// Emulate real keypresses with wParam

	// UP gesture
	if (vkmode>LKGESTURE_NONE) {
		// do not handle gestures outside mapspacemode
		if (!dontdrawthemap) {
			// DoStatusMessage(_T("DBG-033 gesture not used here"));
			return 0;
		}
		switch(MapSpaceMode) {
			case MSM_LANDABLE:
			case MSM_AIRPORTS:
			case MSM_NEARTPS:
						LKForceDoNearest=true;
						numpages=Numpages; // TODO adopt Numpages[MapSpaceMode]
						break;
			case MSM_COMMON:
						LKForceDoCommon=true;
						// warning. Commons and Recents share the same variable!
						numpages=CommonNumpages;
						break;
			case MSM_RECENT:
						LKForceDoRecent=true;
						numpages=CommonNumpages;
						break;
			case MSM_TRAFFIC:
						numpages=TrafficNumpages;
						break;
			default:
						break;
		}
		SelectedRaw[MapSpaceMode]=0;

		switch(vkmode) {
			// SCROLL DOWN
			case LKGESTURE_DOWN:
				// careful, selectedpage starts from 0
				if (++SelectedPage[MapSpaceMode] >=numpages) {
					#ifndef DISABLEAUDIO
					if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
					#endif
					SelectedPage[MapSpaceMode]=0;
				} else {
					#ifndef DISABLEAUDIO
					if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
					#endif
				}
				LKevent=LKEVENT_NEWPAGE;
				MapWindow::RefreshMap();
				return 0;
			// SCROLL UP
			case LKGESTURE_UP:
				if (--SelectedPage[MapSpaceMode] <0) {
					#ifndef DISABLEAUDIO
					if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
					#endif
					SelectedPage[MapSpaceMode]=(numpages-1);
				} else {
					if (SelectedPage[MapSpaceMode]==0) {
						#ifndef DISABLEAUDIO
						if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
						#endif
					} else {
						#ifndef DISABLEAUDIO
						if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
						#endif
					}
				}
				LKevent=LKEVENT_NEWPAGE;
				MapWindow::RefreshMap();
				return 0;
			case LKGESTURE_RIGHT:
				NextModeType();
				MapWindow::RefreshMap();
				#ifndef DISABLEAUDIO
				if (EnableSoundModes) {
					if (CURTYPE == 0)
						PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
					else
						PlayResource(TEXT("IDR_WAV_CLICK"));
				}
				#endif
				return 0;

				break;

			case LKGESTURE_LEFT:
				PreviousModeType();
				MapWindow::RefreshMap();
				#ifndef DISABLEAUDIO
				if (EnableSoundModes) {
					if (CURTYPE == 0)
						PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
					else
						PlayResource(TEXT("IDR_WAV_CLICK"));
				}
				#endif
				return 0;

				break;
			default:
				return 0;
		}

		return 0;
	}

	if (Y<yup) {
		// we are processing up/down in mapspacemode i.e. browsing waypoints on the page
		if (dontdrawthemap) {
			if (MapSpaceMode<=MSM_MAP) {
				// DoStatusMessage(_T("DBG-032-A event up not used here"));
				return 0;
			}
			#ifndef DISABLEAUDIO
	        	if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			//if (--SelectedRaw[MapSpaceMode] <0) SelectedRaw[MapSpaceMode]=Numraws-1;
			LKevent=LKEVENT_UP;
			MapWindow::RefreshMap();
			// DoStatusMessage(_T("DBG-032-B event up used here"));
			return 0;
		}
		#ifndef DISABLEAUDIO
	        if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		if (keytime>=VKTIMELONG)
			return 0xc1;
		else
			return 38;
	}
	if (Y>ydown) {
		if (dontdrawthemap) {
			if (MapSpaceMode<=MSM_MAP) return 0;
			#ifndef DISABLEAUDIO
	        	if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			//if (++SelectedRaw[MapSpaceMode] >=Numraws) SelectedRaw[MapSpaceMode]=0;
			LKevent=LKEVENT_DOWN;
			MapWindow::RefreshMap();
			return 0;
		}
		#ifndef DISABLEAUDIO
	        if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		if (keytime>=VKTIMELONG)
			return 0xc2;
		else
			return 40;
	}
	// no click for already clicked events

	//  Swap white and black colours on LK8000 : working only with virtual keys on, map mode
	//  Currently as of 2.1 virtual keys are almost obsoleted, and it is very unlikely that 
	//  someone will ever use this feature, which is also undocumented!!
	if (keytime>=VKTIMELONG && !dontdrawthemap) {
		if (NewMap&&Look8000) {
			static short oldOutline=OutlinedTp;
			if (OutlinedTp>(OutlinedTp_t)otDisabled) OutlinedTp=(OutlinedTp_t)otDisabled;
			else
				OutlinedTp=oldOutline;
			// TODO CHECK EXPERIMENTAL
			Appearance.InverseInfoBox = !Appearance.InverseInfoBox;
			#ifndef DISABLEAUDIO
			if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			MapWindow::RefreshMap();
		}
		return 0;

		// return 27; virtual ESC 
	} else {
		// If in mapspacemode process ENTER 
		if ( (keytime>=(VKSHORTCLICK*2)) && dontdrawthemap) {
			#ifndef DISABLEAUDIO
			if (EnableSoundModes) LKSound(_T("LK_BELL.WAV"));
			#endif
			LKevent=LKEVENT_ENTER;
			MapWindow::RefreshMap();
			return 0;
		}
		// do not process enter in panmode, unused
/*
		if ( !MapWindow::EnablePan ) {
	             DoStatusMessage(_T("Virtual ENTER")); 
		     return 13;
		}
*/

		if (SIMMODE) {
#ifndef MAP_ZOOM
			if ( MapWindow::EnablePan  && ISPARAGLIDER) return 99; // 091221 return impossible value
#else /* MAP_ZOOM */
			if ( MapWindow::mode.AnyPan() && ISPARAGLIDER) return 99; // 091221 return impossible value
#endif /* MAP_ZOOM */
			else return 0;
		} else {
			return 0;
		}
	}
	DoStatusMessage(_T("VirtualKey Error")); 
	return 0;
}

void InitNewMap()
{

  StartupStore(_T(". InitNewMap%s"),NEWLINE); // 091213

  LOGFONT logfontTarget;	// StatisticsWindow
  LOGFONT logfontBig;		// InfoWindow
  LOGFONT logfontTitle;		// MapWindow
  LOGFONT logfontTitleNavbox;
  LOGFONT logfontMap;		// MapWindow compatible, safe to user changed with edit fonts
  LOGFONT logfontValue;		// StatisticsWindow
  LOGFONT logfontUnit;		// TitleSmallWindow
  LOGFONT logfontSmall;
  LOGFONT logfontMedium;
  LOGFONT logfontSymbol;
  LOGFONT logfontInfoBig;
  LOGFONT logfontInfoBigItalic;
  LOGFONT logfontInfoNormal;
  LOGFONT logfontInfoSmall;
  LOGFONT logfontPanelBig;
  LOGFONT logfontPanelMedium;
  LOGFONT logfontPanelSmall;
  LOGFONT logfontPanelUnit;


  memset ((char *)&logfontTarget, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontBig, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontTitle, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontTitleNavbox, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontMap, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontValue, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontUnit, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontSmall, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontMedium, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontSymbol, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoBig, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoBigItalic, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoNormal, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoSmall, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelBig, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelMedium, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelSmall, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelUnit, 0, sizeof (LOGFONT) );


  // ALL THESE VALUES HAVE BEEN TESTED ONE BY ONE AGAINST EACH DEVICE RESOLUTION
  // AND IT WAS A LONG WORK, SO DO NOT CHANGE UNLESS YOU KNOW WHAT YOU ARE DOING
  // > ALSO UNUSED SPLITTER VALUES ARE FINE TUNED!
 
  switch (ScreenSize) { 

	// Landscape

	// remember: changing values here may need cosmetic changes in InfoBoxLayout where
	// BottomSize is set manually... *UPDATE> apparently no more since december 09
	case (ScreenSize_t)ss800x480:		// PASSED DEV-1 090701 VENTA
		splitter=6;
		propGetFontSettingsFromString(TEXT("72,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);	// 64 600
		propGetFontSettingsFromString(TEXT("48,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
		propGetFontSettingsFromString(TEXT("41,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);	// 40 600
		propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);

	 	propGetFontSettingsFromString(TEXT("36,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	 	propGetFontSettingsFromString(TEXT("34,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
	 	propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap); // was 36
		// propGetFontSettingsFromString(TEXT("50,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);	// 46 600
		propGetFontSettingsFromString(TEXT("54,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 100914
		propGetFontSettingsFromString(TEXT("48,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);

		propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
		propGetFontSettingsFromString(TEXT("40,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic); 
		propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal); 
		propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);

		propGetFontSettingsFromString(TEXT("64,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);  
		propGetFontSettingsFromString(TEXT("48,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium); 
		propGetFontSettingsFromString(TEXT("32,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall); 
		propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
		BottomSize=80; // Title+Value-4
		break;
	case (ScreenSize_t)ss400x240:		// 091204
		splitter=6;
		propGetFontSettingsFromString(TEXT("36,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);	// 64 600
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
		propGetFontSettingsFromString(TEXT("21,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);	// 40 600
		propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);

	 	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	 	propGetFontSettingsFromString(TEXT("17,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
	 	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap); // was 36
		propGetFontSettingsFromString(TEXT("25,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);	// 46 600
		propGetFontSettingsFromString(TEXT("24,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);

		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic); 
		propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal); 
		propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);

		propGetFontSettingsFromString(TEXT("32,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);  
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium); 
		propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall); 
		propGetFontSettingsFromString(TEXT("11,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
		BottomSize=40; // Title+Value-4
		break;
	case (ScreenSize_t)ss480x272:		// PASSED SIM-1 090701 VENTA
		splitter=5;
		#if (WINDOWSPC>0)
		// propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		propGetFontSettingsFromString(TEXT("48,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		#else
		// propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig); changed 100910
		propGetFontSettingsFromString(TEXT("48,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		#endif
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
		propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		#if 0
		if (splitter==6) {
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("24,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=36; // Title+Value-4
		} else {
		#endif
			propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			// propGetFontSettingsFromString(TEXT("30,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); 100914
			propGetFontSettingsFromString(TEXT("32,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=48; // Title+Value-4 plus something more
		// }
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic); 
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig); 
		#else
		// propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontPanelBig); changed 100910
		propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig); 
		#endif
		propGetFontSettingsFromString(TEXT("26,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
		break;
	case (ScreenSize_t)ss720x408:
		splitter=5;
		propGetFontSettingsFromString(TEXT("72,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		propGetFontSettingsFromString(TEXT("33,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
		propGetFontSettingsFromString(TEXT("39,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("15,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("48,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("45,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("21,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
		BottomSize=72; // Title+Value-4 plus something more

		propGetFontSettingsFromString(TEXT("42,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
		propGetFontSettingsFromString(TEXT("42,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic); 
		propGetFontSettingsFromString(TEXT("33,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		propGetFontSettingsFromString(TEXT("57,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig); 
		propGetFontSettingsFromString(TEXT("39,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("27,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("21,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
		break;
	case (ScreenSize_t)ss480x234:		// PASSED SIM-1 090701 VENTA
		splitter=5;
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		#else
		propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
		#endif
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
		propGetFontSettingsFromString(TEXT("24,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		#if 0
		if (splitter==6) {
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=36; // Title+Value-4
		} else {
		#endif
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=41; // Title+Value-4
		// }
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("34,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
		#else
		propGetFontSettingsFromString(TEXT("34,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontPanelBig);
		#endif
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
		break;

	// Units are too big, tahoma is not enough. Units are disabled by default.
	// This is also used by 640x480 devices actually, if SE_VGA is not disabled

	case (ScreenSize_t)ss320x240:		// PASSED DEV-1 090701 VENTA
		splitter=5;
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("32,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		#else
		propGetFontSettingsFromString(TEXT("34,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		#endif
		propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
		propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		#if 0
		if (splitter==6) {
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("20,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("16,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=24; // Title+Value-4

		} else {
		#endif
			propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("24,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=38; // 100914
		//}
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
		break;

	case (ScreenSize_t)ss640x480:		// PASSED DEV-1 090701 VENTA
		splitter=5;
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("64,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		#else
		propGetFontSettingsFromString(TEXT("64,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
		#endif
		propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
		propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		#if 0
		if (splitter==6) {
			propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue); 
			propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=56; // Title+Value-4

		} else {		
		#endif
			propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("44,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=72; // Title+Value-4
		// }
		propGetFontSettingsFromString(TEXT("38,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
		propGetFontSettingsFromString(TEXT("38,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		propGetFontSettingsFromString(TEXT("56,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
		propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("28,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("20,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
		break;

	case (ScreenSize_t)ss896x672:		//  091204
		splitter=6;
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("89,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		#else
		propGetFontSettingsFromString(TEXT("89,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
		#endif
		propGetFontSettingsFromString(TEXT("45,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
		propGetFontSettingsFromString(TEXT("44,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("20,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		if (splitter==6) {
			propGetFontSettingsFromString(TEXT("38,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("38,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("38,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("62,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("44,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue); 
			propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=78; // Title+Value-4

		} 
		#if 0
		else {		
			propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("64,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("61,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=100; // Title+Value-4
		}
		#endif
		propGetFontSettingsFromString(TEXT("53,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
		propGetFontSettingsFromString(TEXT("53,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("47,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("30,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		propGetFontSettingsFromString(TEXT("78,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
		propGetFontSettingsFromString(TEXT("61,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("39,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("28,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
		break;

	///////////////       Portrait  mode        ////////////////

	case (ScreenSize_t)ss240x320:		// PASSED DEV-1 090701  DEV-2 101005
		splitter=3;
			#if (WINDOWSPC>0)
			propGetFontSettingsFromString(TEXT("30,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig); // 29 600 101005
			#else
			propGetFontSettingsFromString(TEXT("30,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
			#endif
			propGetFontSettingsFromString(TEXT("11,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
			propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
			propGetFontSettingsFromString(TEXT("6,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);

		if (splitter==3) {
			// Splitter = 3 on two rows
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("23,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=68; 
		}
		#if 0
		if (splitter==6) {
			propGetFontSettingsFromString(TEXT("9,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("9,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("9,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("11,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("6,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=20; // Title+Value-4  a bit bigger here
		}
		#endif
		if (splitter==5) {
			// very small, only a sample of what can be seen under landscape mode
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("6,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=27; // Title+Value-4  a bit bigger here
		} 
		#if 0
		else {
			// Splitter 4
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			#if (WINDOWSPC>0)
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			#else
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), &logfontValue);
			#endif
			propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=40; // Title+Value-4  a bit bigger here
		}
		#endif
		propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
		propGetFontSettingsFromString(TEXT("17,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("15,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig); // 24 101005
		#else
		propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontPanelBig);
		#endif
		propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);  // 10 101005
		break;

	case (ScreenSize_t)ss272x480:
		splitter=3;
		propGetFontSettingsFromString(TEXT("36,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
			propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
			propGetFontSettingsFromString(TEXT("20,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
			propGetFontSettingsFromString(TEXT("8,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		if (splitter==3) {
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), &logfontValue);
			propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=80; // Title+Value-4  a bit bigger here
		}
		if (splitter==5) {
			// very small, only a sample of what can be seen under landscape mode
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("26,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=40; // Title+Value-4  a bit bigger here
		} 
		#if 0
		else {
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("26,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), &logfontValue);
			propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=50; // Title+Value-4  a bit bigger here
		}
		#endif
		propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
		propGetFontSettingsFromString(TEXT("18,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		propGetFontSettingsFromString(TEXT("28,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
		propGetFontSettingsFromString(TEXT("18,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
		break;
// BigFont      LK8000 welcome
// FontMedium   Tactical Flight Computer welcome
// InfoNormal:  Dist Eff ReqE top line

	case (ScreenSize_t)ss480x640:		// PASSED DEV-1 090701 VENTA
		splitter=3;
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("60,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		#else
		propGetFontSettingsFromString(TEXT("60,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
		#endif
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
		propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);

		if (splitter==3) { 
			/* ----------------------------
			// Splitter = 6 , all-in-a-line  unused being too small,  otherwise ok
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("44,2,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=44; // Title+Value-4 a bit bigger here
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("40,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=75; // Title+Value-4  a bit bigger here
			   ----------------------------- */
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 44 101005
			propGetFontSettingsFromString(TEXT("46,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=135; 
		} else {	
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 600
			propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=62; // Title+Value-4 a bit bigger here
		}
		propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
		propGetFontSettingsFromString(TEXT("34,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("30,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);  // 101004: 49 800
		#else
		propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontPanelBig); 
		#endif
		propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);  // 101004 18
		break;

	case (ScreenSize_t)ss480x800:		// 100410
		splitter=3;
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("62,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		#else
		propGetFontSettingsFromString(TEXT("62,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
		#endif
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontSymbol);
		propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);

		if (splitter==3) {
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("46,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=135; 
		}
		#if 0
		if (splitter==6) { // unused
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("44,2,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
			propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=44; // Title+Value-4 a bit bigger here

		}
		else {		
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
			propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
			propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 600
			propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
			propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
			BottomSize=62; // Title+Value-4 a bit bigger here
		}
		#endif
		propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
		propGetFontSettingsFromString(TEXT("34,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("30,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("48,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);  // 101004 49 800
		#else
		propGetFontSettingsFromString(TEXT("48,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), &logfontPanelBig); 
		#endif
		propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("20,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
		break;
	// Default assumes a portrait 240x320 so all text will fit and users will report too small

	default:
		splitter=5;
	 	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	 	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	 	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("28,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		#else
		propGetFontSettingsFromString(TEXT("28,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
		#endif
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
		propGetFontSettingsFromString(TEXT("11,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("6,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("9,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("8,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
		propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
		BottomSize=38; // Title+Value-4
		break;
  }

  logfontTarget.lfQuality = GetFontRenderer(); 
  logfontBig.lfQuality = GetFontRenderer(); 
  logfontValue.lfQuality = GetFontRenderer(); 
  logfontTitle.lfQuality = GetFontRenderer(); 
  logfontMap.lfQuality = GetFontRenderer(); 
  logfontTitleNavbox.lfQuality = GetFontRenderer(); 
  logfontUnit.lfQuality = GetFontRenderer(); 
  logfontMedium.lfQuality = GetFontRenderer(); 
  logfontSmall.lfQuality = GetFontRenderer(); 
  logfontInfoBig.lfQuality = GetFontRenderer(); 
  logfontInfoBigItalic.lfQuality = GetFontRenderer(); 
  logfontInfoNormal.lfQuality = GetFontRenderer(); 
  logfontInfoSmall.lfQuality = GetFontRenderer(); 
  logfontSymbol.lfQuality = GetFontRenderer(); 
  logfontSymbol.lfCharSet = SYMBOL_CHARSET; // careful
  logfontPanelBig.lfQuality= GetFontRenderer();
  logfontPanelMedium.lfQuality= GetFontRenderer();
  logfontPanelSmall.lfQuality= GetFontRenderer();
  logfontPanelUnit.lfQuality= GetFontRenderer();

  LK8TargetFont	= CreateFontIndirect (&logfontTarget); 
  LK8BigFont	= CreateFontIndirect (&logfontBig);
  LK8ValueFont	= CreateFontIndirect (&logfontValue);
  LK8TitleFont	= CreateFontIndirect (&logfontTitle);
  LK8MapFont	= CreateFontIndirect (&logfontMap);
  LK8TitleNavboxFont	= CreateFontIndirect (&logfontTitleNavbox);
  LK8UnitFont	= CreateFontIndirect (&logfontUnit);
  LK8SymbolFont	= CreateFontIndirect (&logfontSymbol);
  LK8MediumFont	= CreateFontIndirect (&logfontMedium);
  LK8SmallFont	= CreateFontIndirect (&logfontSmall);
  LK8InfoBigFont	= CreateFontIndirect (&logfontInfoBig);
  LK8InfoBigItalicFont	= CreateFontIndirect (&logfontInfoBigItalic);
  LK8InfoNormalFont	= CreateFontIndirect (&logfontInfoNormal);
  LK8InfoSmallFont	= CreateFontIndirect (&logfontInfoSmall);
  LK8PanelBigFont	= CreateFontIndirect (&logfontPanelBig);
  LK8PanelMediumFont	= CreateFontIndirect (&logfontPanelMedium);
  LK8PanelSmallFont	= CreateFontIndirect (&logfontPanelSmall);
  LK8PanelUnitFont	= CreateFontIndirect (&logfontPanelUnit);

/* Old try
  LONG mastersize=800/22; // should be equivalent to PNA best MapWindowFont 36
   _tcscpy(logfontTarget.lfFaceName, _T("Tahoma"));
  logfontTarget.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE  ;
  logfontTarget.lfCharSet = ANSI_CHARSET;
  logfontTarget.lfQuality = ANTIALIASED_QUALITY; 
  logfontTarget.lfHeight = int (ceil(mastersize*1.33)); // eq Statistics 48
  logfontTarget.lfWidth =  0;
  logfontTarget.lfWeight = FW_NORMAL;
  logfontTarget.lfItalic = FALSE;
  LK8TargetFont = CreateFontIndirect (&logfontTarget); 
  BottomSize= int (ceil(mastersize*1.77 + mastersize));
*/


}


void InitScreenSize() {

#if (WINDOWSPC>0)
  int iWidth=SCREENWIDTH;
  int iHeight=SCREENHEIGHT;
#else
  int iWidth=GetSystemMetrics(SM_CXSCREEN);
  int iHeight=GetSystemMetrics(SM_CYSCREEN);
#endif

  ScreenSizeX=iWidth;
  ScreenSizeY=iHeight;
  ScreenSizeR.top=0;
  ScreenSizeR.bottom=iHeight-1;
  ScreenSizeR.left=0;
  ScreenSizeR.right=iWidth-1;

  ScreenSize=0;

  if (iWidth == 240 && iHeight == 320) ScreenSize=(ScreenSize_t)ss240x320; // QVGA      portrait
  if (iWidth == 272 && iHeight == 480) ScreenSize=(ScreenSize_t)ss272x480;
  if (iWidth == 480 && iHeight == 640) ScreenSize=(ScreenSize_t)ss480x640; //  VGA
  if (iWidth == 640 && iHeight == 480) ScreenSize=(ScreenSize_t)ss640x480; //   VGA
  if (iWidth == 320 && iHeight == 240) ScreenSize=(ScreenSize_t)ss320x240; //  QVGA
  if (iWidth == 720 && iHeight == 408) ScreenSize=(ScreenSize_t)ss720x408;
  if (iWidth == 480 && iHeight == 800) ScreenSize=(ScreenSize_t)ss480x800;
  if (iWidth == 400 && iHeight == 240) ScreenSize=(ScreenSize_t)ss400x240; // landscape
  if (iWidth == 480 && iHeight == 272) ScreenSize=(ScreenSize_t)ss480x272; // WQVGA     landscape
  if (iWidth == 480 && iHeight == 234) ScreenSize=(ScreenSize_t)ss480x234; //   iGo
  if (iWidth == 800 && iHeight == 480) ScreenSize=(ScreenSize_t)ss800x480; //  WVGA
  if (iWidth == 896 && iHeight == 672) ScreenSize=(ScreenSize_t)ss896x672; //  PC version only

  TCHAR tbuf[80];
  if (ScreenSize==0) {
        wsprintf(tbuf,_T(". InitScreenSize: ++++++ ERROR UNKNOWN RESOLUTION %dx%d !%s"),iWidth,iHeight,NEWLINE); // 091119
        StartupStore(tbuf);
  } else {
        wsprintf(tbuf,_T(". InitScreenSize: %dx%d%s"),iWidth,iHeight,NEWLINE); // 091213
        StartupStore(tbuf);
  }

  if (ScreenSize > (ScreenSize_t)sslandscape) 
	ScreenLandscape=true;
  else
	ScreenLandscape=false;

}

#ifndef MAP_ZOOM
void InitAircraftCategory()
{

 switch (AircraftCategory) {

	case (AircraftCategory_t)umGlider:
	case (AircraftCategory_t)umGAaircraft:
	case (AircraftCategory_t)umCar:

		MapWindow::RequestMapScale = 4; 
		MapWindow::MapScale = 4;
		MapWindow::MapScaleOverDistanceModify = 4/DISTANCEMODIFY;
		break;

	case (AircraftCategory_t)umParaglider:
		TCHAR buf[100];
		wsprintf(buf,_T(". PGCruiseZoom set to %d%s"),PGCruiseZoom,NEWLINE); // 091119
		StartupStore(buf);
		switch(PGCruiseZoom) { // 091108
			case 0:
				MapWindow::RequestMapScale = 0.10;  // 088
				MapWindow::MapScale = 0.10;
				break;
			case 1:
				MapWindow::RequestMapScale = 0.12;  // 117
				MapWindow::MapScale = 0.12;
				break;
			case 2:
				MapWindow::RequestMapScale = 0.14;  // 205
				MapWindow::MapScale = 0.14;
				break;
			case 3:
				MapWindow::RequestMapScale = 0.16;  // 293
				MapWindow::MapScale = 0.16;
				break;
			case 4:
				MapWindow::RequestMapScale = 0.18; 
				MapWindow::MapScale = 0.18;
				break;
			case 5:
				MapWindow::RequestMapScale = 0.20; 
				MapWindow::MapScale = 0.20;
				break;
			case 6:
				MapWindow::RequestMapScale = 0.23; 
				MapWindow::MapScale = 0.23;
				break;
			case 7:
				MapWindow::RequestMapScale = 0.25; 
				MapWindow::MapScale = 0.25;
				break;
			case 8:
			default:
				MapWindow::RequestMapScale = 0.3; 
				MapWindow::MapScale = 0.3;
				break;
		}
		MapWindow::MapScaleOverDistanceModify = MapWindow::RequestMapScale/DISTANCEMODIFY;
				
		break;

	default:
		// make it an evident problem
		MapWindow::RequestMapScale = 50;
		MapWindow::MapScale = 50;
		MapWindow::MapScaleOverDistanceModify = 50/DISTANCEMODIFY;
		break;
 }


}
#endif /* ! MAP_ZOOM */

// Requires restart if activated from config menu
void InitLK8000() 
{
	StartupStore(_T(". Init LK8000%s"),NEWLINE);
	LoadRecentList();

	InitModeTable();

	// By default, h=v=size/6 and here we set it better
	switch (ScreenSize) { 
		case (ScreenSize_t)ss800x480:
			GestureSize=50;
			LKVarioSize=50;
			// dscale=480/240=2  800/dscale=400 -(70+2+2)=  326 x dscale = 652
			LKwdlgConfig=652;
			break;
		case (ScreenSize_t)ss400x240:
			GestureSize=50;
			LKVarioSize=25;
			// dscale=240/240=1  400/dscale=400 -(70+2+2)=  326 x dscale = 326
			LKwdlgConfig=326;
			break;
		case (ScreenSize_t)ss640x480:
			GestureSize=50;
			LKVarioSize=40;
			// dscale=480/240=2  640/dscale=320 -(70+2+2)=  246 x dscale = 492
			LKwdlgConfig=492;
			break;
		case (ScreenSize_t)ss896x672:
			GestureSize=50;
			LKVarioSize=56;
			// dscale=672/240=2.8  896/dscale=320 -(70+2+2)=  246 x dscale = 689
			LKwdlgConfig=689;
			break;
		case (ScreenSize_t)ss480x272:
			GestureSize=50;
			LKVarioSize=30;
			// dscale=272/240=1.133  480/dscale=424 -(70+2+2)=  350 x dscale = 397
			LKwdlgConfig=395;
			break;
		case (ScreenSize_t)ss720x408:
			GestureSize=50;
			LKVarioSize=45;
			// dscale=408/240=1.133  720/dscale=423 -(70+2+2)=  350 x dscale = 594
			LKwdlgConfig=594;
			break;
		case (ScreenSize_t)ss480x234:
			GestureSize=50;
			LKVarioSize=30;
			// dscale=234/240=0.975  480/dscale=492 -(70+2+2)=  418 x dscale = 407
			LKwdlgConfig=405;
			break;
		case (ScreenSize_t)ss320x240:
			GestureSize=50;
			LKVarioSize=20;
			// dscale=240/240=1  320/dscale=320 -(70+2+2)=  246 x dscale = 246
                        // but 246 is too long..
			LKwdlgConfig=244;
			break;
		// PORTRAIT MODES
		case (ScreenSize_t)ss480x640:
			GestureSize=50;
			LKVarioSize=30;
			break;
		case (ScreenSize_t)ss480x800:
			GestureSize=50;
			LKVarioSize=30;
			// dscale=240/240=1  400/dscale=400 -(70+2+2)=  326 x dscale = 326
			LKwdlgConfig=324;
			break;
		case (ScreenSize_t)ss240x320:
			GestureSize=50;
			LKVarioSize=15;
			break;
		default:
			GestureSize=50;
			LKVarioSize=30;
			break;
	}

}

void LockMap(){
	if (MapLock==false) {
		// DoStatusMessage(_T("MAP LOCKED"));
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) 
		        PlayResource(TEXT("IDR_WAV_GREEN"));
		#endif
		DefocusInfoBox();
	}
	MapLock=true;
}

void UnlockMap(){
	if (MapLock) {
		// DoStatusMessage(_T("MAP UNLOCKED")); // annoying
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) 
		        LKSound(_T("UNLOCKMAP.WAV"));
		#endif
	}

	MapLock=false;
}

// colorcode is taken from a 5 bit AsInt union
void MapWindow::TextColor(HDC hDC, short colorcode) {

	switch (colorcode) {
	case TEXTBLACK: 
		if (BlackScreen) // 091109
		  SetTextColor(hDC,RGB_WHITE);  // black 
		else
		  SetTextColor(hDC,RGB_BLACK);  // black 
	  break;
	case TEXTWHITE: 
		if (BlackScreen) // 091109
		  SetTextColor(hDC,RGB_LIGHTYELLOW);  // white
		else
		  SetTextColor(hDC,RGB_WHITE);  // white
	  break;
	case TEXTGREEN: 
	  SetTextColor(hDC,RGB_GREEN);  // green
	  break;
	case TEXTRED:
	  SetTextColor(hDC,RGB_RED);  // red
	  break;
	case TEXTBLUE:
	  SetTextColor(hDC,RGB_BLUE);  // blue
	  break;
	case TEXTYELLOW:
	  SetTextColor(hDC,RGB_YELLOW);  // yellow
	  break;
	case TEXTCYAN:
	  SetTextColor(hDC,RGB_CYAN);  // cyan
	  break;
	case TEXTMAGENTA:
	  SetTextColor(hDC,RGB_MAGENTA);  // magenta
	  break;
	case TEXTLIGHTGREY: 
	  SetTextColor(hDC,RGB_LIGHTGREY);  // light grey
	  break;
	case TEXTGREY: 
	  SetTextColor(hDC,RGB_GREY);  // grey
	  break;
	case TEXTLIGHTGREEN:
	  SetTextColor(hDC,RGB_LIGHTGREEN);  //  light green
	  break;
	case TEXTLIGHTRED:
	  SetTextColor(hDC,RGB_LIGHTRED);  // light red
	  break;
	case TEXTLIGHTYELLOW:
	  SetTextColor(hDC,RGB_LIGHTYELLOW);  // light yellow
	  break;
	case TEXTLIGHTCYAN:
	  SetTextColor(hDC,RGB_LIGHTCYAN);  // light cyan
	  break;
	case TEXTORANGE:
	  SetTextColor(hDC,RGB_ORANGE);  // orange
	  break;
	case TEXTLIGHTORANGE:
	  SetTextColor(hDC,RGB_LIGHTORANGE);  // light orange
	  break;
	case TEXTLIGHTBLUE:
	  SetTextColor(hDC,RGB_LIGHTBLUE);  // light blue
	  break;
	default:
	  SetTextColor(hDC,RGB_MAGENTA);  // magenta so we know it's wrong: nobody use magenta..
	  break;
	}

}


#ifdef PNA
// VENTA-ADDON MODELTYPE

//
//	Check if the model type is encoded in the executable file name
//
//  GlobalModelName is a global variable, shown during startup and used for printouts only.
//  In order to know what model you are using, GlobalModelType is used.
// 
//  This "smartname" facility is used to override the registry/config Model setup to force
//  a model type to be used, just in case. The model types may not follow strictly those in
//  config menu, nor be updated. Does'nt hurt though.
//
void SmartGlobalModelType() {

	GlobalModelType=MODELTYPE_PNA;	// default for ifdef PNA by now!

	if ( GetGlobalModelName() ) 
	{
		ConvToUpper(GlobalModelName);
	
		if ( !_tcscmp(GlobalModelName,_T("PNA"))) {
					GlobalModelType=MODELTYPE_PNA_PNA;
					_tcscpy(GlobalModelName,_T("GENERIC") );
		}
		else 
			if ( !_tcscmp(GlobalModelName,_T("HP31X")))	{
					GlobalModelType=MODELTYPE_PNA_HP31X;
			}
		else	
			if ( !_tcscmp(GlobalModelName,_T("PN6000"))) {
					GlobalModelType=MODELTYPE_PNA_PN6000;
			}
		else	
			if ( !_tcscmp(GlobalModelName,_T("MIO"))) {
					GlobalModelType=MODELTYPE_PNA_MIO;
			}
		else
			_tcscpy(GlobalModelName,_T("UNKNOWN") );
	} else	
		_tcscpy(GlobalModelName, _T("UNKNOWN") );
}


bool SetModelType() {

  TCHAR sTmp[100];
  TCHAR szRegistryInfoBoxModel[]= TEXT("AppInfoBoxModel");
  DWORD Temp=0;

  GetFromRegistry(szRegistryInfoBoxModel, &Temp);
  
  if ( SetModelName(Temp) != true ) {
	_stprintf(sTmp,_T(". SetModelType failed: probably no registry entry%s"), NEWLINE);
	StartupStore(sTmp);
	GlobalModelType=MODELTYPE_PNA_PNA;
	_tcscpy(GlobalModelName,_T("GENERIC"));  // 100820
	return false;
  } else {
	GlobalModelType = Temp;
  }
  
  _stprintf(sTmp,_T(". SetModelType: Name=<%s> Type=%d%s"),GlobalModelName, GlobalModelType,NEWLINE);
  StartupStore(sTmp);
  return true;
}

// Parse a MODELTYPE value and set the equivalent model name. 
// If the modeltype is invalid or not yet handled, assume that
// the user changed it in the registry or in the profile, and 
// correct the error returning false: this will force a Generic Type.

bool SetModelName(DWORD Temp) {
  switch (Temp) {
  case MODELTYPE_PNA_PNA:
    _tcscpy(GlobalModelName,_T("GENERIC")); 
    return true;
    break;
  case MODELTYPE_PNA_HP31X:
    _tcscpy(GlobalModelName,_T("HP31X"));
    return true;
    break;
  case MODELTYPE_PNA_PN6000:
    _tcscpy(GlobalModelName,_T("PN6000"));
    return true;
  case MODELTYPE_PNA_MIO:
    _tcscpy(GlobalModelName,_T("MIO"));
    return true;
  case  MODELTYPE_PNA_MEDION_P5:
    _tcscpy(GlobalModelName,_T("MEDION P5"));
    return true;
  case MODELTYPE_PNA_NOKIA_500:
    _tcscpy(GlobalModelName,_T("NOKIA500"));
    return true;
  case MODELTYPE_PNA_NAVIGON:
    _tcscpy(GlobalModelName,_T("NAVIGON"));
    return true;
  default:
    _tcscpy(GlobalModelName,_T("UNKNOWN"));
    return false;
  }

}

#endif


#if defined(PNA) || defined(FIVV)  // VENTA-ADDON gmfpathname & C.

/* 
	Paolo Ventafridda 1 feb 08 
	Get pathname & c. from GetModuleFilename (gmf)
	In case of problems, always return \ERRORxx\  as path name
	It will be displayed at startup and users will know that
	something is wrong reporting the error code to us.
	Approach not followed: It works but we don't know why
	Approach followed: It doesn't work and we DO know why

	These are temporary solutions to be improved
 */

#define MAXPATHBASENAME MAX_PATH

/*
 * gmfpathname returns the pathname of the current executed program, with leading and trailing slash
 * example:  \sdmmc\   \SD CARD\
 * In case of double slash, it is assumed currently as a single "\" .
 */
TCHAR * gmfpathname ()
{
  static TCHAR gmfpathname_buffer[MAXPATHBASENAME];
  TCHAR  *p; 
  
  if (GetModuleFileName(NULL, gmfpathname_buffer, MAXPATHBASENAME) <= 0) {
//    StartupStore(TEXT("CRITIC- gmfpathname returned null GetModuleFileName\n")); // rob bughunt
    return(_T("\\ERROR_01\\") );
  }
  if (gmfpathname_buffer[0] != '\\' ) {
//   StartupStore(TEXT("CRITIC- gmfpathname starting without a leading backslash\n"));
    return(_T("\\ERROR_02\\"));
  }
  gmfpathname_buffer[MAXPATHBASENAME-1] = '\0';	// truncate for safety
  
  for (p=gmfpathname_buffer+1; *p != '\0'; p++)
    if ( *p == '\\' ) break;	// search for the very first "\"
  
  if ( *p == '\0') {
//    StartupStore(TEXT("CRITIC- gmfpathname no backslash found\n"));
    return(_T("\\ERROR_03\\"));
  }
  *++p = '\0';
  
  return (TCHAR *) gmfpathname_buffer;
}

/*
 * gmfbasename returns the filename of the current executed program, without leading path.
 * Example:  xcsoar.exe 
 */
TCHAR * gmfbasename ()
{
  static TCHAR gmfbasename_buffer[MAXPATHBASENAME];
  TCHAR *p, *lp;
  
  if (GetModuleFileName(NULL, gmfbasename_buffer, MAXPATHBASENAME) <= 0) {
    StartupStore(TEXT("++++++ CRITIC- gmfbasename returned null GetModuleFileName%s"),NEWLINE); // 091119
    return(_T("ERROR_04") );
  }
  if (gmfbasename_buffer[0] != '\\' ) {
    StartupStore(TEXT("++++++ CRITIC- gmfbasename starting without a leading backslash%s"),NEWLINE); // 091119
    return(_T("ERROR_05"));
  }
  for (p=gmfbasename_buffer+1, lp=NULL; *p != '\0'; p++)
    {
      if ( *p == '\\' ) {
	lp=++p;
	continue;
      }
    }
  return  lp;
}

/*
 *	A little hack in the executable filename: if it contains an
 *	underscore, then the following chars up to the .exe is
 *	considered a modelname
 *  Returns 0 if failed, 1 if name found
 */
int GetGlobalModelName ()
{
  TCHAR modelname_buffer[MAXPATHBASENAME];
  TCHAR *p, *lp, *np;
  
  _tcscpy(GlobalModelName, _T(""));
  
  if (GetModuleFileName(NULL, modelname_buffer, MAXPATHBASENAME) <= 0) {
    StartupStore(TEXT("++++++ CRITIC- GetGlobalFileName returned NULL%s"),NEWLINE); // 091119
    return 0;
  }
  if (modelname_buffer[0] != '\\' ) {
    StartupStore(TEXT("++++++ CRITIC- GetGlobalFileName starting without a leading backslash%s"),NEWLINE); // 091119
    return 0;
  }
  for (p=modelname_buffer+1, lp=NULL; *p != '\0'; p++) 
    {
      if ( *p == '\\' ) {
	lp=++p;
	continue;
      }
    } // assuming \sd\path\xcsoar_pna.exe  we are now at \xcsoar..
  
  for (p=lp, np=NULL; *p != '\0'; p++)
    {
      if (*p == '_' ) {
	np=++p;
	break;
      }
    } // assuming xcsoar_pna.exe we are now at _pna..
  
  if ( np == NULL ) {
    return 0;	// VENTA2-bugfix , null deleted
  }
  
  for ( p=np, lp=NULL; *p != '\0'; p++) 
    {
      if (*p == '.' ) {
	lp=p;
	break;
      }
    } // we found the . in pna.exe
  
  if (lp == NULL) return 0; // VENTA2-bugfix null return
  *lp='\0'; // we cut .exe
  
  _tcscpy(GlobalModelName, np);
  
  return 1;  // we say ok
  
}

#endif   // PNA

/*
 * Convert to uppercase a TCHAR array
 */
void ConvToUpper( TCHAR *str )
{
	if ( str )
	{
		for ( ; *str; ++str )
		*str = towupper(*str);

	}

	return ;
}

#ifdef FIVV
BOOL DelRegistryKey(const TCHAR *szDelKey)
{
   HKEY tKey;
   RegOpenKeyEx(HKEY_CURRENT_USER, _T(REGKEYNAME),0,0,&tKey);
   if ( RegDeleteValue(tKey, szDelKey) != ERROR_SUCCESS ) {
	return false;
   }
   RegCloseKey(tKey);
   return true;
}
#endif

#ifdef PNA
void CleanRegistry()
{
   HKEY tKey;
   RegOpenKeyEx(HKEY_CURRENT_USER, szRegistryKey ,0,0,&tKey);

	RegDeleteValue(tKey,_T("CDIWindowFont"));
	RegDeleteValue(tKey,_T("InfoWindowFont"));
	RegDeleteValue(tKey,_T("MapLabelFont"));
	RegDeleteValue(tKey,_T("MapWindowBoldFont"));
	RegDeleteValue(tKey,_T("MapWindowFont"));
	RegDeleteValue(tKey,_T("StatisticsFont"));
	RegDeleteValue(tKey,_T("TitleSmallWindowFont"));
	RegDeleteValue(tKey,_T("TitleWindowFont"));
	RegDeleteValue(tKey,_T("BugsBallastFont"));
	RegDeleteValue(tKey,_T("TeamCodeFont"));

   RegCloseKey(tKey);
}
#endif

#ifdef PNA 	
/* Paolo Ventafridda Apr 23th 2009 VENTA4
 * SetBacklight for PNA devices. There is no standard way of managing backlight on CE,
 * and every device may have different value names and settings. Microsoft did not set 
 * a standard and thus we need a custom solution for each device.
 * But the approach is always the same: change a value and call an event.
 * We do this in XCSoar.cpp at the beginning, no need to make these settings configurable:
 * max brightness and no timeout if on power is the rule. Otherwise, do it manually..
 */
bool SetBacklight() // VENTA4
{
  HKEY    hKey;
  DWORD   Disp=0;
  HRESULT hRes;
  bool doevent=false;

  if (EnableAutoBacklight == false ) return false;

  hRes = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ControlPanel\\Backlight"), 0,  0, &hKey);
  if (hRes != ERROR_SUCCESS) return false;

  switch (GlobalModelType)
  {
	case MODELTYPE_PNA_HP31X:

		Disp=20; // max backlight
		// currently we ignore hres, if registry entries are spoiled out user is already in deep troubles
		hRes = RegSetValueEx(hKey, _T("BackLightCurrentACLevel"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("BackLightCurrentBatteryLevel"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("TotalLevels"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=0;
		hRes = RegSetValueEx(hKey, _T("UseExt"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		RegDeleteValue(hKey,_T("ACTimeout"));
		doevent=true;
		break;

	default:
		doevent=false;
		break;
  }

  RegCloseKey(hKey); if (doevent==false) return false;

  HANDLE BLEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("BacklightChangeEvent")); 
  if ( SetEvent(BLEvent) == 0) doevent=false;
  	else CloseHandle(BLEvent);
  return doevent;
}

bool SetSoundVolume() // VENTA4
{

  if (EnableAutoSoundVolume == false ) return false;

/*
 * This does not work, dunno why
 *
  HKEY    hKey;
  DWORD   Disp=0;
  HRESULT hRes;

  hRes = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ControlPanel\\Volume"), 0,  0, &hKey);
  if (hRes != ERROR_SUCCESS) return false;
  switch (GlobalModelType)
  {
	case MODELTYPE_PNA_HP31X:
		Disp=0xFFFFFFFF; // max volume
		hRes = RegSetValueEx(hKey, _T("Volume"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=65538;
		hRes = RegSetValueEx(hKey, _T("Screen"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=0;
		hRes = RegSetValueEx(hKey, _T("Key"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=7;
		hRes = RegSetValueEx(hKey, _T("Mute"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0xF2, 0); 
	        RegCloseKey(hKey); 
		break;

	default:
		break;
  }
 */

  // should we enter critical section ?  probably... 
  waveOutSetVolume(0, 0xffff); // this is working for all platforms

  return true;
}


#endif

#if defined(FIVV) || defined(PNA)
// VENTA2-ADDON fonts install
/*
 * Get the localpath, enter XCSoarData/Config, see if there are fonts to copy,
 * check that they have not already been copied in \Windows\Fonts,
 * and eventually copy everything in place.
 *
 * 0 if nothing needed, and all is ok
 * 1 if made action, and all is ok
 * errors if >1
 * 
 * These are currently fonts used by PDA:
 *
	DejaVuSansCondensed2.ttf
	DejaVuSansCondensed-Bold2.ttf
	DejaVuSansCondensed-BoldOblique2.ttf
	DejaVuSansCondensed-Oblique2.ttf
 *
 *
 */

// This will NOT be called from PC versions
short InstallSystem() {

  TCHAR srcdir[MAX_PATH];
  TCHAR dstdir[MAX_PATH];
  TCHAR maindir[MAX_PATH];
  TCHAR fontdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  TCHAR dstfile[MAX_PATH];
  TCHAR tbuf[MAX_PATH*3];
#if (0)
  DWORD attrib;
#endif
  bool failure=false;

  #if ALPHADEBUG
  StartupStore(_T(". Welcome to InstallSystem v1.2%s"),NEWLINE);
  #endif
  LocalPath(srcdir,TEXT(LKD_SYSTEM));

  _stprintf(dstdir,_T(""));

  // search for the main system directory on the real device
  // Remember that SHGetSpecialFolder works differently on CE platforms, and you cannot check for result.
  // We need to verify if directory does really exist.

  SHGetSpecialFolderPath(hWndMainWindow, dstdir, CSIDL_WINDOWS, false);
  if ( wcslen(dstdir) <6) {
	_stprintf(tbuf,_T("------ InstallSystem PROBLEM: cannot locate the Windows folder, got string:<%s>%s"),dstdir,NEWLINE);
	StartupStore(tbuf);
	StartupStore(_T("------ InstallSystem attempting to use default \"\\Windows\" but no warranty!%s"),NEWLINE);
	_stprintf(dstdir,TEXT("\\Windows")); // 091118
  } else {
	StartupStore(_T(". InstallSystem: Windows path reported from device is: <%s>%s"),dstdir,NEWLINE);
  }
  _tcscpy(maindir,dstdir);

  _stprintf(tbuf,_T(". InstallSystem: copy DLL from <%s> to <%s>%s"), srcdir, dstdir,NEWLINE);
  StartupStore(tbuf);

  // We now test for a single file existing inside the directory, called _DIRECTORYNAME
  // because GetFileAttributes can be very slow or hang if checking a directory. In any case testing a file is 
  // much more faster.
#if (0)
  if (  GetFileAttributes(srcdir) != FILE_ATTRIBUTE_DIRECTORY) { // TODO FIX &= 
	StartupStore(_T("------ InstallSystem ERROR could not find source directory <%s> !%s"),srcdir,NEWLINE); // 091104
	failure=true;
#else
  _stprintf(srcfile,TEXT("%s\\_SYSTEM"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) {
	StartupStore(_T("------ InstallSystem ERROR could not find valid system directory <%s>%s"),srcdir,NEWLINE); // 091104
	StartupStore(_T("------ Missing checkfile <%s>%s"),srcfile,NEWLINE);
	failure=true;
#endif
  } else {
	StartupStore(_T(". InstallSystem source directory <%s> is available%s"),srcdir,NEWLINE);
  }

#if (0)
  attrib=GetFileAttributes(dstdir);
  if ( attrib == 0xffffffff ) {
	StartupStore(_T("------ InstallSystem ERROR Directory <%s> does not exist ???%s"),dstdir,NEWLINE);
	failure=true;
  }
  if ( attrib &= FILE_ATTRIBUTE_SYSTEM ) {
	StartupStore(_T(". Directory <%s> is identified as a system folder%s"),dstdir,NEWLINE);
  }
  if ( attrib &= FILE_ATTRIBUTE_COMPRESSED ) {
	StartupStore(_T(". Directory <%s> is a compressed folder%s"),dstdir,NEWLINE);
  }
  if ( attrib &= FILE_ATTRIBUTE_HIDDEN ) {
	StartupStore(_T(". Directory <%s> is a hidden folder%s"),dstdir,NEWLINE);
  }
  /* These attributes are not available
  if ( attrib &= FILE_ATTRIBUTE_INROM ) {
	StartupStore(_T("------ InstallSystem ERROR Directory <%s> is in ROM%s"),dstdir,NEWLINE);
	failure=true;
  }
  if ( attrib &= FILE_ATTRIBUTE_ROMMODULE ) {
	StartupStore(_T("------ InstallSystem ERROR Directory <%s> is a ROM MODULE%s"),dstdir,NEWLINE);
	failure=true;
  }
  */
  if ( attrib &= FILE_ATTRIBUTE_READONLY ) {
	StartupStore(_T("------ InstallSystem ERROR Directory <%s> is READ ONLY%s"),dstdir,NEWLINE);
	failure=true;
  }
#endif

  if (  failure ) {
	StartupStore(_T("------ WARNING: NO DLL install available (and thus NO G-RECORD FOR VALIDATING IGC FILES)%s"),NEWLINE);
	StartupStore(_T("------ WARNING: NO font will be installed on device (and thus wrong text size displayed)%s"),NEWLINE);
	return 5; // 091109
  } else {
#ifdef PPC2002
	_stprintf(srcfile,TEXT("%s\\GRECORD2002.XCS"),srcdir);
#endif
#ifdef PPC2003
	_stprintf(srcfile,TEXT("%s\\GRECORD2003.XCS"),srcdir);
#endif
#ifdef PNA
	_stprintf(srcfile,TEXT("%s\\GRECORDPNA.XCS"),srcdir);
#endif

	_stprintf(dstfile,TEXT("%s\\GRecordDll.dll"),dstdir);

	if (  GetFileAttributes(dstfile) != 0xffffffff ) {
		StartupStore(_T(". GRecordDll.dll already installed in device, very well.%s"),NEWLINE);
	} else {
		if (!CopyFile(srcfile,dstfile,TRUE)) {
			StartupStore(_T("++++++ COULD NOT INSTALL <%s> inside device. BAD!%s"),srcfile,NEWLINE);
			StartupStore(_T("++++++ Error code was: %ld%s"),GetLastError(),NEWLINE);
		} else {
			StartupStore(_T("... GRecordDll.dll installed using <%s>. Great.%s"),srcfile,NEWLINE);
		}
	}

#ifdef PNA
	if (GlobalModelType == MODELTYPE_PNA_HP31X) { // 091109

		StartupStore(_T(". InstallSystem checking desktop links for HP31X%s"),NEWLINE);

		_stprintf(dstdir,TEXT("\\Windows\\Desktop"));
		if (  GetFileAttributes(dstdir) != FILE_ATTRIBUTE_DIRECTORY) { // FIX
			StartupStore(_T("------ Desktop directory <%s> NOT found! Is this REALLY an HP31X?%s"),dstdir,NEWLINE);
		} else {
			_stprintf(srcfile,TEXT("%s\\LK8_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\LK8000.lnk"),dstdir);
			if (  GetFileAttributes(dstfile) != 0xffffffff ) {
				StartupStore(_T(". Link to LK8000 already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!CopyFile(srcfile,dstfile,FALSE))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			#if 0
			_stprintf(srcfile,TEXT("%s\\LK8SIM_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\SIM.lnk"),dstdir);
			if (  GetFileAttributes(dstfile) != 0xffffffff ) {
				StartupStore(_T(". Link to SIM LK8000 already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!CopyFile(srcfile,dstfile,FALSE))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			#endif
			_stprintf(srcfile,TEXT("%s\\BT_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\BlueTooth.lnk"),dstdir);
			if (  GetFileAttributes(dstfile) != 0xffffffff ) {
				StartupStore(_T(". Link to BlueTooth already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!CopyFile(srcfile,dstfile,FALSE))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			_stprintf(srcfile,TEXT("%s\\NAV_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\CarNav.lnk"),dstdir);
			if (  GetFileAttributes(dstfile) != 0xffffffff ) {
				StartupStore(_T(". Link to Car Navigator already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!CopyFile(srcfile,dstfile,FALSE))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			_stprintf(srcfile,TEXT("%s\\TLOCK_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\TouchLock.lnk"),dstdir);
			if (  GetFileAttributes(dstfile) != 0xffffffff ) {
				StartupStore(_T(". Link to TouchLock already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!CopyFile(srcfile,dstfile,FALSE))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
		}
	}
#endif

  }

  // we are shure that \Windows does exist already.

  _stprintf(fontdir,_T(""));
  _stprintf(dstdir,_T(""));
  #ifdef PNA
  if ( GetFontPath(fontdir) == FALSE ) {
	StartupStore(_T(". Special RegKey for fonts not found on this PNA, using standard folder.%s"), NEWLINE);
	SHGetSpecialFolderPath(hWndMainWindow, dstdir, CSIDL_FONTS, false);
	if ( wcslen(dstdir) <5 ) {
		_stprintf(tbuf,_T("------ PROBLEM: cannot locate the Fonts folder, got string:<%s>%s"),dstdir,NEWLINE);
		StartupStore(tbuf);
		_stprintf(tbuf,_T("------ Attempting to use directory <%s> as a fallback%s"),maindir,NEWLINE);
		StartupStore(tbuf);
		_tcscpy(dstdir,maindir);
	}
  } else {
	StartupStore(_T(". RegKey Font directory is <%s>%s"),fontdir,NEWLINE);
	CreateRecursiveDirectory(fontdir);
	_tcscpy(dstdir,fontdir); 
  }
  #else
  // this is not working correctly on PNA, it is reporting Windows Fonts even with another value in regkey
  SHGetSpecialFolderPath(hWndMainWindow, dstdir, CSIDL_FONTS, false);
  if ( wcslen(dstdir) <5 ) {
	_stprintf(tbuf,_T("------ PROBLEM: cannot locate the Fonts folder, got string:<%s>%s"),dstdir,NEWLINE);
	StartupStore(tbuf);
	_stprintf(tbuf,_T("------ Attempting to use directory <%s> as a fallback%s"),maindir,NEWLINE);
	StartupStore(tbuf);
	_tcscpy(dstdir,maindir);
  }
  #endif


  _stprintf(tbuf,_T(". InstallSystem: Copy Fonts from <%s> to <%s>%s"), srcdir, dstdir,NEWLINE);
  StartupStore(tbuf);
  // on PNAs sometimes FolderPath is reported correctly, but the directory is not existing!
  // this is not needed really on PNA, but doesnt hurt
  CreateDirectory(dstdir, NULL); // 100820


  // we cannot check directory existance without the risk of hanging for many seconds
  // we can only rely on singe real file existance, not on directories

  #if ALPHADEBUG
  StartupStore(_T(". Checking TAHOMA font%s"),NEWLINE);
  #endif
  _stprintf(srcfile,TEXT("%s\\TAHOMA.TTF"),srcdir);
  _stprintf(dstfile,TEXT("%s\\TAHOMA.TTF"),dstdir);
  if (  GetFileAttributes(dstfile) != 0xffffffff ) {
	StartupStore(_T(". Font TAHOMA.TTF is already installed%s"),NEWLINE);
  } else {
	if ( !CopyFile(srcfile,dstfile,TRUE))  {
		StartupStore(_T("------ Could not copy TAHOMA.TTF on device, not good.%s"),NEWLINE);
		StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
	} else
		StartupStore(_T("... Font TAHOMA.TTF installed on device%s"),NEWLINE);
  }

  // not needed, cannot overwrite tahoma while in use! Tahoma bold not used for some reason in this case.
  // Problem solved, look at FontPath !!

  #if ALPHADEBUG
  StartupStore(_T(". Checking TAHOMABD font%s"),NEWLINE);
  #endif
  _stprintf(srcfile,TEXT("%s\\TAHOMABD.TTF"),srcdir);
  _stprintf(dstfile,TEXT("%s\\TAHOMABD.TTF"),dstdir);
  if (  GetFileAttributes(dstfile) != 0xffffffff ) {
	StartupStore(_T(". Font TAHOMABD.TTF is already installed%s"),NEWLINE);
  } else {
	if ( !CopyFile(srcfile,dstfile,TRUE))  {
		StartupStore(_T("------ Could not copy TAHOMABD.TTF on device, not good.%s"),NEWLINE);
		StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
	} else
		StartupStore(_T("... Font TAHOMABD.TTF installed on device%s"),NEWLINE);
  }

  #if ALPHADEBUG
  StartupStore(_T(". InstallSystem completed OK%s"),NEWLINE);
  #endif

  return 0;

}



bool CheckRootDir() {
  TCHAR rootdir[MAX_PATH];
  LocalPath(rootdir,_T(""));
  DWORD fattr = GetFileAttributes(rootdir);
  if ((fattr != 0xFFFFFFFF) && (fattr & FILE_ATTRIBUTE_DIRECTORY)) return true;
  return false;
}


bool CheckDataDir() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir,_T(LKD_SYSTEM));
  _stprintf(srcfile,TEXT("%s\\_SYSTEM"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) return false;
  return true;
}

bool CheckLanguageDir() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_LANGUAGE));
  _stprintf(srcfile,TEXT("%s\\_LANGUAGE"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) {
	return false;
  }

  LocalPath(srcdir, _T(LKD_LANGUAGE));
  _stprintf(srcfile,TEXT("%s\\ENG_MSG.TXT"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) return false;

  return true;
}

bool CheckRegistryProfile() {
	TCHAR srcpath[MAX_PATH];
	if ( GlobalModelType == MODELTYPE_PNA_HP31X ) return false;
	LocalPath(srcpath,TEXT(LKD_CONF)); // 091101
	_stprintf(srcpath,_T("%s\\%s"),srcpath,XCSPROFILE); // 091101
	if (  GetFileAttributes(srcpath) == 0xffffffff) return false;
	return true;
}
#endif


int roundupdivision(int a, int b) {
   int c=a / b;
   if ( (a%b) >0) return ++c;
   else return c;
}

#ifdef CPUSTATS
// Warning this is called by several concurrent threads, no static variables here
void Cpustats(int *accounting, FILETIME *kernel_old, FILETIME *kernel_new, FILETIME *user_old, FILETIME *user_new) {
   __int64 knew=0, kold=0, unew=0, uold=0;
   int total=2; // show evident problem

   knew = kernel_new->dwHighDateTime;
   knew <<= 32;
   knew += kernel_new->dwLowDateTime;
    
   unew=user_new->dwHighDateTime;
   unew <<=32;
   unew+=user_new->dwLowDateTime;

   kold = kernel_old->dwHighDateTime;
   kold <<= 32;
   kold += kernel_old->dwLowDateTime;
   
   uold=user_old->dwHighDateTime;
   uold <<=32;
   uold+=user_old->dwLowDateTime;

#if (WINDOWSPC>0)   
   total = (int) ((knew+unew-kold-uold)/10.0);
   //if (total==0) return;
#else
   total = (int) ((knew+unew-kold-uold)/10000.0);
#endif
   *accounting=total;

}

#endif


// Conversion between submenus and global mapspace modes 
// Basic initialization of global variables and parameters.
//
void InitModeTable() {

	short i;
	StartupStore(_T(". Init ModeTable for LK8000: "));

	for (i=0; i<=LKMODE_TOP; i++)
		for (short j=0; j<=MSM_TOP; j++)
			ModeTable[i][j]=INVALID_VALUE;


	// this table is for submenus, order is not important
	ModeTable[LKMODE_MAP][MP_WELCOME]	=	MSM_WELCOME;
	ModeTable[LKMODE_MAP][MP_MOVING]	=	MSM_MAP;

	ModeTable[LKMODE_WP][WP_AIRPORTS]	=	MSM_AIRPORTS;
	ModeTable[LKMODE_WP][WP_LANDABLE]	=	MSM_LANDABLE;
	ModeTable[LKMODE_WP][WP_NEARTPS]	=	MSM_NEARTPS;

	ModeTable[LKMODE_INFOMODE][IM_CRUISE]	=	MSM_INFO_CRUISE;
	ModeTable[LKMODE_INFOMODE][IM_THERMAL]	=	MSM_INFO_THERMAL;
	ModeTable[LKMODE_INFOMODE][IM_TASK]	=	MSM_INFO_TASK;
	ModeTable[LKMODE_INFOMODE][IM_AUX]	=	MSM_INFO_AUX;
	ModeTable[LKMODE_INFOMODE][IM_TRI]	=	MSM_INFO_TRI;

	ModeTable[LKMODE_NAV][NV_COMMONS]	=	MSM_COMMON;
	ModeTable[LKMODE_NAV][NV_HISTORY]	=	MSM_RECENT;

	ModeTable[LKMODE_TRF][TF_LIST]		=	MSM_TRAFFIC;
	ModeTable[LKMODE_TRF][IM_TRF]		=	MSM_INFO_TRF;
	ModeTable[LKMODE_TRF][IM_TARGET]	=	MSM_INFO_TARGET;

	// startup mode
	ModeIndex=LKMODE_MAP;
	// startup values for each mode
	ModeType[LKMODE_MAP]	=	MP_WELCOME;
	ModeType[LKMODE_WP]	=	WP_AIRPORTS;
	ModeType[LKMODE_INFOMODE]=	IM_CRUISE;
	ModeType[LKMODE_NAV]	=	NV_COMMONS;
	ModeType[LKMODE_TRF]	=	TF_LIST;

	ModeTableTop[LKMODE_MAP]=MP_TOP;
	ModeTableTop[LKMODE_WP]=WP_TOP;
	ModeTableTop[LKMODE_INFOMODE]=IM_TOP;
	ModeTableTop[LKMODE_NAV]=NV_TOP;
	ModeTableTop[LKMODE_TRF]=TF_TOP;

	// set all sorting type to distance (default) even for unconventional modes just to be sure
	for (i=0; i<=MSM_TOP; i++) SortedMode[i]=1;

	for (i=0; i<MAXNEAREST;i++) 
		SortedTurnpointIndex[i]=-1;

	for (i=0; i<MAXNEAREST;i++) {
		SortedLandableIndex[i]=-1;
		SortedAirportIndex[i]=-1;
	}

	for (i=0; i<MAXCOMMON; i++) 
		CommonIndex[i]= -1;


	StartupStore(_T("Ok%s"),NEWLINE);
}


void SetModeType(short modeindex, short modetype) {

	UnselectMapSpace( ModeTable[ModeIndex][CURTYPE] );
	// TODO: make safe checks
	ModeIndex=modeindex;
	CURTYPE=modetype;
	SelectMapSpace( ModeTable[ModeIndex][CURTYPE] );

}

// Advance through types inside current mode
//
void NextModeType() {

	UnselectMapSpace( ModeTable[ModeIndex][CURTYPE] );
redo:
	if ( CURTYPE >= ModeTableTop[ModeIndex] ) {
		// point to first
		CURTYPE=0; 
	} else {
		CURTYPE++;
	}
	if (ISPARAGLIDER) {
		if (CURTYPE == IM_TRI) goto redo;
	}
	SelectMapSpace( ModeTable[ModeIndex][CURTYPE] );
}


void PreviousModeType() {
// usare ifcircling per decidere se 0 o 1
	UnselectMapSpace( ModeTable[ModeIndex][CURTYPE] );
redo:
	if ( CURTYPE <= 0 ) {
		// point to last
		CURTYPE=ModeTableTop[ModeIndex]; 
	} else {
		CURTYPE--;
	}
	if (ISPARAGLIDER) {
		if (CURTYPE == IM_TRI) goto redo;
	}
	SelectMapSpace( ModeTable[ModeIndex][CURTYPE] );
}


// Advance inside Mode Table between map, wp, infopages 
// and reselect previous state for that mode
// Notice: does NOT advance inside modes through types
//
void NextModeIndex() {
	UnselectMapSpace(ModeTable[ModeIndex][CURTYPE]);
	if ( GPS_INFO.FLARM_Available ) { // 100325
		if ( (ModeIndex+1)>LKMODE_TOP)
			ModeIndex=LKMODE_MAP;
		else
			ModeIndex++;
	} else {
		if ( (ModeIndex+1)>(LKMODE_TOP-1))
			ModeIndex=LKMODE_MAP;
		else
			ModeIndex++;
	}
	SelectMapSpace(ModeTable[ModeIndex][CURTYPE]);
}

void SoundModeIndex() {
#ifndef DISABLEAUDIO
	if (EnableSoundModes) {
		switch(ModeIndex) {
			case LKMODE_MAP:
				PlayResource(TEXT("IDR_WAV_TONE7"));
				break;
			case LKMODE_INFOMODE:
				PlayResource(TEXT("IDR_WAV_TONE1"));
				break;
			case LKMODE_WP:
				PlayResource(TEXT("IDR_WAV_TONE2"));
				break;
			case LKMODE_NAV:
				PlayResource(TEXT("IDR_WAV_TONE3"));
				break;
			case LKMODE_TRF:
				PlayResource(TEXT("IDR_WAV_TONE4"));
				break;
		}
	}
#endif
}

void BottomSounds() {
#ifndef DISABLEAUDIO
   if (EnableSoundModes) {
	switch(BottomMode) {
		case 1:
			PlayResource(TEXT("IDR_WAV_HIGHCLICK"));
			break;
		case 2:
			PlayResource(TEXT("IDR_WAV_BTONE2"));
			break;
		case 3:
			PlayResource(TEXT("IDR_WAV_BTONE2"));
			break;
		case 4:
			PlayResource(TEXT("IDR_WAV_BTONE4"));
			break;
		case 5:
			PlayResource(TEXT("IDR_WAV_BTONE4"));
			break;
		case 6:
			PlayResource(TEXT("IDR_WAV_BTONE5"));
			break;
		case 7:
			PlayResource(TEXT("IDR_WAV_BTONE5"));
			break;
		case 8:
			PlayResource(TEXT("IDR_WAV_BTONE6"));
			break;
		case 9:
			PlayResource(TEXT("IDR_WAV_BTONE6"));
			break;
		default:
			PlayResource(TEXT("IDR_WAV_CLICK"));
			break;
	}
  }
#endif
}

// This is currently unused.. we need a button!
void PreviousModeIndex() {
  UnselectMapSpace(ModeTable[ModeIndex][CURTYPE]);
  if ( (ModeIndex-1)<0) {
	if ( GPS_INFO.FLARM_Available ) { // 100325
		ModeIndex=LKMODE_TOP;
	} else {
		ModeIndex=LKMODE_TOP-1;
	}
  } else
	ModeIndex--;
  SelectMapSpace(ModeTable[ModeIndex][CURTYPE]);
}

// This will set mapspace directly, and set ModeIndex accordingly. So we can do a "goto" mapspace now.
void SetModeIndex(short i) {
  UnselectMapSpace(ModeTable[ModeIndex][CURTYPE]);
  if (i<0 || i>LKMODE_TOP) {
	DoStatusMessage(_T("ERR-137 INVALID MODEINDEX"));
	return;
  }
  ModeIndex=i;
  SelectMapSpace(ModeTable[ModeIndex][CURTYPE]);

}


// Selecting MapSpaceMode need also ModeIndex and ModeType to be updated!
// Do not use these functions directly..
// Toggling pages will force intermediate activations, so careful here
//
void UnselectMapSpace(short i) {

	return;
}


void SelectMapSpace(short i) {
	
	LKForceDoNearest=false;
	LKForceDoCommon=false;
	LKForceDoRecent=false;
	// Particular care not to leave pending events
	LKevent=LKEVENT_NONE;

	switch(i) {
		case MSM_LANDABLE:
		case MSM_AIRPORTS:
		case MSM_NEARTPS:
			// force DoNearest to run at once
			LKForceDoNearest=true;
			LKevent=LKEVENT_NEWRUN;
			SelectedPage[MapSpaceMode]=0;
			SelectedRaw[MapSpaceMode]=0;
			break;
		case MSM_COMMON:
			LKForceDoCommon=true;
			LKevent=LKEVENT_NEWRUN;
			SelectedPage[MapSpaceMode]=0;
			SelectedRaw[MapSpaceMode]=0;
			break;
		case MSM_RECENT:
			LKForceDoRecent=true;
			LKevent=LKEVENT_NEWRUN;
			SelectedPage[MapSpaceMode]=0;
			SelectedRaw[MapSpaceMode]=0;
			break;
		default:
			LKevent=LKEVENT_NEWRUN;
			break;
	}
	MapSpaceMode=i;
}

// Get the infobox type from configuration, selecting position i
// From 1-8 auxiliaries
//     0-16 dynamic page
//
int GetInfoboxType(int i) {

	int retval = 0;
	if (i<1||i>16) return LK_ERROR;

	// it really starts from 0
	if (i<=8)
		retval = (InfoType[i-1] >> 24) & 0xff; // auxiliary
	else {
#ifndef MAP_ZOOM
		switch ( DisplayMode ) {
			case dmCruise:
#else /* MAP_ZOOM */
		switch ( MapWindow::mode.Fly() ) {
			case MapWindow::Mode::MODE_FLY_CRUISE:
#endif /* MAP_ZOOM */
				retval = (InfoType[i-9] >> 8) & 0xff;
				break;
#ifndef MAP_ZOOM
			case dmFinalGlide:
#else /* MAP_ZOOM */
			case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
#endif /* MAP_ZOOM */
				retval = (InfoType[i-9] >> 16) & 0xff;
				break;
#ifndef MAP_ZOOM
			case dmCircling:
#else /* MAP_ZOOM */
			case MapWindow::Mode::MODE_FLY_CIRCLING:
#endif /* MAP_ZOOM */
				retval = (InfoType[i-9]) & 0xff; 
				break;
			default:
				// impossible case, show twice auxiliaries
				retval = (InfoType[i-9] >> 24) & 0xff;
				break;
		}
	}

	return min(NUMSELECTSTRINGS-1,retval);
}

// Returns the LKProcess index value for configured infobox (0-8) for dmCruise, dmFinalGlide, Auxiliary, dmCircling
// The function name is really stupid...
// dmMode is an enum, we simply use for commodity
#ifndef MAP_ZOOM
int GetInfoboxIndex(int i, short dmMode) {
#else /* MAP_ZOOM */
int GetInfoboxIndex(int i, MapWindow::Mode::TModeFly dmMode) {
#endif /* MAP_ZOOM */
	int retval = 0;
	if (i<0||i>8) return LK_ERROR;

	switch(dmMode) {
#ifndef MAP_ZOOM
		case dmCruise:
#else /* MAP_ZOOM */
		case MapWindow::Mode::MODE_FLY_CRUISE:
#endif /* MAP_ZOOM */
			retval = (InfoType[i-1] >> 8) & 0xff;
			break;
#ifndef MAP_ZOOM
		case dmFinalGlide:
#else /* MAP_ZOOM */
		case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
#endif /* MAP_ZOOM */
			retval = (InfoType[i-1] >> 16) & 0xff;
			break;
#ifndef MAP_ZOOM
		case dmCircling:
#else /* MAP_ZOOM */
		case MapWindow::Mode::MODE_FLY_CIRCLING:
#endif /* MAP_ZOOM */
			retval = (InfoType[i-1]) & 0xff; 
			break;
		default:
			// default is auxiliary
			retval = (InfoType[i-1] >> 24) & 0xff; 
			break;
	}
	return min(NUMSELECTSTRINGS-1,retval);
}


double GetMacCready(int wpindex, short wpmode)
{
	if (WayPointCalc[wpindex].IsLandable) {
		if (MACCREADY>GlidePolar::SafetyMacCready) 
			return MACCREADY;
		else
			return GlidePolar::SafetyMacCready;
	}
	return MACCREADY;

}

// UNUSED 091023
void unicodetoascii(TCHAR *utext, int utextsize, char *atext) {

	int i,j;
	if (utextsize==0) {
		atext[0]=0;
		return;
	}

	for (i=0,j=0; i<utextsize; i++) {
		if (utext[i]==0) continue;
		atext[j++]=utext[i];
	}
	atext[j]=0;

}

void InitWindRotary(windrotary_s *buf) {
short i;

	for (i=0; i<WCALC_ROTARYSIZE; i++) {
		buf->speed[i]=0;
		buf->ias[i]=0;
		buf->track[i]=0;
		buf->compass[i]=0;
		buf->altitude[i]=0;
	}
	buf->start=-1;
	// only last x seconds of data are used
	buf->size=WCALC_MAXSIZE;
	return;
}

//#define DEBUG_ROTARY

void InsertWindRotary(windrotary_s *buf, double speed, double track, double altitude ) {
static short errs=0;

#ifdef DEBUG_ROTARY
char ventabuffer[200];
FILE *fp;
#endif
	if (CALCULATED_INFO.OnGround == TRUE) {
		return;
	}
	
	if (CALCULATED_INFO.Circling == TRUE) {
		return;
	}


	// speed is in m/s
	if (speed<1 || speed>100 || altitude <1 || altitude >10000) { 
		if (errs>2) {
			#ifdef DEBUG_ROTARY
			sprintf(ventabuffer,"Wind Rotary reset after exceeding errors\n");
			if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
				    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
			#endif
			InitWindRotary(&rotaryWind);
			errs=0;
			return;

		}
		errs++;
		#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"(errs=%d) IGNORE INVALID speed=%f track=%f alt=%f\n",errs,speed, track, altitude);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
		#endif
		return;
	}
	errs=0;

	if (++buf->start >=buf->size) { 
		#ifdef DEBUG_ROTARY
		sprintf(ventabuffer,"*** rotary reset ++bufstart=%d >=bufsize=%d\n",buf->start, buf->size);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
			    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
		#endif
		buf->start=0;
	}


	// We store GS in kmh, simply.
	buf->speed[buf->start] = (int)(speed*TOKPH);
	// we use track+180 in order to be always over 0 in range calculations
	// we want track north to be 0, not 360
	if (track==360) track=0;
	buf->track[buf->start]=(int)track+180;
	buf->altitude[buf->start]=(int)altitude;
	// ias is in m/s
	if (GPS_INFO.AirspeedAvailable && GPS_INFO.IndicatedAirspeed>0) {
		buf->ias[buf->start] = (int)GPS_INFO.IndicatedAirspeed;
	}

	#ifdef DEBUG_ROTARY
	sprintf(ventabuffer,"insert buf[%d/%d], track=%d alt=%d GS=%d",
		buf->start, buf->size-1, 
		buf->track[buf->start]-180,
		buf->altitude[buf->start] , 
		buf->speed[buf->start]);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
		    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
}

// The Magic TrueWind returns: (see Utils2.h WCALC return codes) <=0 error values
//
// iaspeed is Indicated Air Speed in m/s. Target track is automatic. (TODO: use digital compass if available)
// IF IAS IS AVAILABLE, we use average IAS, in m/s and we ignore iaspeed
//

#if (WINDOWSPC>0)
//#define DEBUG_WCALC  // REMOVE BEFORE FLIGHT!
//#define STORE_WCALC  // REMOVE BEFORE FLIGHT!
#endif

// wmode  is 0 for NESW , 1 for 3,12,21,30    2 for 6 15 24 33
// For digital compass, wmode is heading degrees + 180 (that is, it starts from 180 degrees..)

#define WCALCTHRESHOLD 0.60
int CalculateWindRotary(windrotary_s *buf, double iaspeed , double *wfrom, double *wspeed, int windcalctime, int wmode) {

  #ifdef DEBUG_WCALC
  char ventabuffer[200];
  FILE *fp;
  #endif

  if (windcalctime<3) {
	StartupStore(_T("------ TrueWind: calctime=%d too low!%s"),windcalctime,NEWLINE);
	return WCALC_INVALID_DATA;
  }

  // make a copy of the working area, and work offline
  windrotary_s bc;
  memcpy(&bc, buf, sizeof(windrotary_s));

  short i,curs,nc,iter;
  int s, alt;
  int low, high;
  float averspeed, averias, avertrack, cutoff;
  double averaltitude=0;
  bool haveias=false;
  short gsquality=0, trackquality=0, iasquality=0;

  averspeed=0; averias=0; avertrack=0; cutoff=0;

  if (iaspeed<2||iaspeed>180) {
	StartupStore(_T("------ TrueWind: Invalid target speed=%f in Wind Calculation%s"),iaspeed,NEWLINE);
	return WCALC_INVALID_DATA;
  } else {
	// kmh range for GS 
	low = 10;
	high = 300;
  }

  #ifdef DEBUG_WCALC
  if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {
	fprintf(fp,"-------------------------------------------\n");
  	if (GPS_INFO.AirspeedAvailable) {
		fprintf(fp,"TAS/IAS are available, preconfig iaspeed=%.3f not used \n",iaspeed);
		fprintf(fp,"*** DUMP BUFFER, rotary size=%d (max buffer:%d) start=%d\n",bc.size, WCALC_ROTARYSIZE,bc.start);
		for (i=0; i< bc.size; i+=5) {
			sprintf(ventabuffer,"  [%02d] sp=%03d trk=%03d ias=%03d  ",i,bc.speed[i],bc.track[i]-180, (int)(bc.ias[i]*3.6));
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d ias=%03d  ",i+1,bc.speed[i+1],bc.track[i+1]-180, (int)(bc.ias[i]*3.6));
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d ias=%03d  ",i+2,bc.speed[i+2],bc.track[i+2]-180, (int)(bc.ias[i]*3.6));
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d ias=%03d  ",i+3,bc.speed[i+3],bc.track[i+3]-180, (int)(bc.ias[i]*3.6));
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d ias=%03d\n",i+4,bc.speed[i+4],bc.track[i+4]-180, (int)(bc.ias[i]*3.6));
			fprintf(fp,ventabuffer);
		}
		fclose(fp);

	} else {
		fprintf(fp,"Preconfigured iaspeed=%.3f \n",iaspeed);
		fprintf(fp,"*** DUMP BUFFER, rotary size=%d (max buffer:%d) start=%d\n",bc.size, WCALC_ROTARYSIZE,bc.start);
		for (i=0; i< bc.size; i+=5) {
			sprintf(ventabuffer,"  [%02d] sp=%03d trk=%03d  ",i,bc.speed[i],bc.track[i]-180);
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d  ",i+1,bc.speed[i+1],bc.track[i+1]-180);
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d  ",i+2,bc.speed[i+2],bc.track[i+2]-180);
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d  ",i+3,bc.speed[i+3],bc.track[i+3]-180);
			fprintf(fp,ventabuffer);
			sprintf(ventabuffer,"[%02d] sp=%03d trk=%03d\n",i+4,bc.speed[i+4],bc.track[i+4]-180);
			fprintf(fp,ventabuffer);
		}
		fclose(fp);

	}
  }
  #endif


  // search for a GS convergent value between these two margins. 

  for (iter=0; iter<MAXITERFILTER; iter++) {
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(speed iter=%d) low=%d high=%d",iter,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	// skip last 2 seconds and go backwards to last 40
	if (bc.start<WCALC_TIMESKIP) curs=bc.size-1; else curs=bc.start-WCALC_TIMESKIP;

	//for (i=0, nc=0, s=0; i<WCALC_TIMEBACK; i++) {
	for (i=0, nc=0, s=0; i<windcalctime; i++) {
		if (bc.speed[curs] >=low && bc.speed[curs] <=high) {
			s+=bc.speed[curs];
			nc++;
		}
		if (--curs <0 ) curs=bc.size-1;
	}
	// do not accept a valid result if we don't have at least (TIMEBACK/2)+1 valid samples
	// do not accept a valid result if we don't have at least 75% +1 valid samples
	//if (nc<((WCALC_TIMEBACK/2)+1)) {
	if (nc<((windcalctime*WCALCTHRESHOLD)+1)) {
		#ifdef DEBUG_WCALC
		sprintf(ventabuffer,"(speed iter=%d) nc=%d <%d : no valid GS averspeed",iter,nc,(int)(windcalctime*WCALCTHRESHOLD)+1);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
		{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
		#endif
		averspeed=0.0;
		break;
	}
	// ground speed quality is an absolute value: the valid fixes accounted
	gsquality=nc;
	averspeed=((float)s/nc);
	cutoff=averspeed/(float)(10.0*(iter+1)); // era 50
	if (cutoff<1) cutoff=1;
	low=(int)(averspeed-cutoff);
	high=(int)(averspeed+cutoff);
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(speed iter=%d) nc=%d  averspeed=%.3f >> new: cutoff=%.3f >> low=%d high=%d",
		iter,nc,averspeed,cutoff,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif

  }

  #ifdef DEBUG_WCALC
  if (averspeed>0) {
  	sprintf(ventabuffer,"GPS GS averspeed is %d kmh", (int)averspeed);
  	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  }
  #endif
  
  // track search, and also altitude average
  // Track is stored + 180, so 359 is now 539. There is NO 360, it is 0 + 180

  low=145;
  high=540;
  for (iter=0; iter<MAXITERFILTER; iter++) {
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(track iter=%d) low=%d high=%d",iter,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	// skip last seconds and go backwards to last 40 or whatever
	if (bc.start<WCALC_TIMESKIP) curs=bc.size-1; else curs=bc.start-WCALC_TIMESKIP;

	for (i=0,nc=0,s=0,alt=0; i<windcalctime; i++) {

		// Track over 325 (505) should appear as x-180 (145)  100316 ?? it was a bug
		int track=bc.track[curs];
		if (track >=low && track <=high) {
			//s+=bc.track[curs];
			s+=track;
			alt+=bc.altitude[curs];
			nc++;
		}
		if (--curs <0 ) curs=bc.size-1;
	}
	// do not accept a valid result if we don't have at least (TIMEBACK/2)+1 valid samples
	// do not accept a valid result if we don't have at least 75%+1 valid samples
	if (nc<((windcalctime*WCALCTHRESHOLD)+1)) {
		#ifdef DEBUG_WCALC
		sprintf(ventabuffer,"(track iter=%d) nc=%d <%d : no valid avertrack",iter,nc,(int)(windcalctime*WCALCTHRESHOLD)+1);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
		{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
		#endif
		avertrack=-1; // negative avertrack means invalid
		break;
	}
	// track quality is number of valid fixes found
	trackquality=nc;
	avertrack=((float)s/nc);
	averaltitude=alt/nc;

	cutoff=avertrack/(float)(10.0*(iter+1)); 
	if (cutoff<1) cutoff=1;
	low=(int)(avertrack-cutoff);
	high=(int)(avertrack+cutoff);

	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(track iter=%d) nc=%d  avertrack=%.3f >> new: cutoff=%.3f >> low=%d high=%d",
		iter,nc,avertrack,cutoff,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
  }

  #ifdef DEBUG_WCALC
  if (avertrack>-1) {
  	sprintf(ventabuffer,"GPS TRACK avertrack is %d", (int)avertrack-180);
  	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  } else {
  	sprintf(ventabuffer,"GPS TRACK avertrack is -1 : NO average track found)");
  	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  }
  #endif

  // If Airspeed is available, use it  (ias is in m/s) but not if using Condor
  #if TESTIASWITHCONDOR
  if (!GPS_INFO.AirspeedAvailable ) goto goto_NoAirspeed;
  #else
  if (!GPS_INFO.AirspeedAvailable || devIsCondor(devA()) ) goto goto_NoAirspeed;
  #endif
  low=2;
  high=60;
  for (iter=0; iter<MAXITERFILTER; iter++) {
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(IAS iter=%d) low=%d high=%d",iter,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	// skip last 2 seconds and go backwards to last 40
	if (bc.start<WCALC_TIMESKIP) curs=bc.size-1; else curs=bc.start-WCALC_TIMESKIP;

	for (i=0, nc=0, s=0; i<windcalctime; i++) {
		if (bc.ias[curs] >=low && bc.ias[curs] <=high) {
			s+=bc.ias[curs];
			nc++;
		}
		if (--curs <0 ) curs=bc.size-1;
	}
	// do not accept a valid result if we don't have at least 75% +1 valid samples
	if (nc<((windcalctime*WCALCTHRESHOLD)+1)) {
		#ifdef DEBUG_WCALC
		sprintf(ventabuffer,"(IAS iter=%d) nc=%d <%d : no valid average IAS",iter,nc,(int)(windcalctime*WCALCTHRESHOLD)+1);
		if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
		{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
		#endif
		averias=0.0;
		if (nc>0) return WCALC_INVALID_IAS;
		else return WCALC_INVALID_NOIAS;
		// otherwise, try to use eIAS
	}
	// iasquality is number of valid fixes found
	iasquality=nc;
	haveias=true; 
	averias=((float)s/nc);
	cutoff=averias/(float)(10.0*(iter+1));
	if (cutoff<1) cutoff=1;
	low=(int)(averias-cutoff);
	high=(int)(averias+cutoff);
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"(IAS iter=%d) nc=%d  averias=%.3f >> new: cutoff=%.3f >> low=%d high=%d",
		iter,nc,averias,cutoff,low,high);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif

  }

  // Assuming 10 seconds, GS is calculated on ground moving in this time. We want in 10 seconds at least 50m made
  // averias is >0 , but must be >18kmh, >5ms ?
  // NO, by now we keep 2ms minimum, for paragliders mainly
  #ifdef DEBUG_WCALC
  sprintf(ventabuffer,"REAL IAS averias is %d", (int)averias);
  if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  #endif
  if (averias<=2) {
	#ifdef STORE_WCALC
	StartupStore(_T("... TrueWind: REAL average IAS =%d TOO LOW%s"),(int)averias, NEWLINE);
	#endif
	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"REAL IAS averias is %d TOO LOW", (int)averias);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	return WCALC_INVALID_NOIAS;
  }

  goto_NoAirspeed:
  // track is inserted with +180 
  // a negative heading to 325 is 145, so 145-180 would be -35 . 360-35=325 correct
  if (avertrack>-1) { // if valid avertrack found
	avertrack-=180;
	if (avertrack<0) avertrack=360-avertrack;
  }
  #ifdef DEBUG_WCALC
  if (averspeed>0 && avertrack>=0) {
	#if TESTIASWITHCONDOR
	if (GPS_INFO.AirspeedAvailable ) {
	#else
	if (GPS_INFO.AirspeedAvailable && !devIsCondor(devA()) ) {
	#endif
		if (averias>0)
			sprintf(ventabuffer,"*** Averages found (AIRSPEED AVAILABLE): averIAS=%d speed=%d track=%d altitude=%d\n", 
				(int)(averias*TOKPH), (int)averspeed, (int)avertrack, (int)averaltitude);
		else
			sprintf(ventabuffer,"*** Averages found (AIRSPEED AVAILABLE): INVALID averIAS=%dm/s speed=%d track=%d altitude=%d\n", 
				(int)averias, (int)averspeed, (int)avertrack, (int)averaltitude);
	} else {
		sprintf(ventabuffer,"*** Averages found: speed=%d track=%d altitude=%d\n", 
			(int)averspeed, (int)avertrack, (int)averaltitude);
	}

	if ((fp=fopen("DEBUG.TXT","a"))!= NULL) {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  }
  #endif
  #ifdef STORE_WCALC
  #if TESTIASWITHCONDOR
  if (GPS_INFO.AirspeedAvailable && averias>0 )
  #else
  if (GPS_INFO.AirspeedAvailable && averias>0 && !devIsCondor(devA()) )
  #endif
	StartupStore(_T("... TrueWind: REAL average IAS =%d Average GS=%d Track=%d Altitude=%d%s"),
		(int)(averias*TOKPH),(int)averspeed,(int)avertrack,(int)averaltitude, NEWLINE);
  else
	StartupStore(_T("... TrueWind: IAS =%d Average GS=%d Track=%d Altitude=%d%s"),
		(int)(iaspeed*TOKPH),(int)averspeed,(int)avertrack,(int)averaltitude, NEWLINE);
  #endif

  if (averspeed<=0 && avertrack<0){
  	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"Fail: GPS averspeed<=0 && avertrack<0 INVALID ALL \n");
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	return WCALC_INVALID_ALL;
  }
  if (averspeed<=0) {
  	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"Fail: GPS averspeed<=0 INVALID SPEED \n");
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	return WCALC_INVALID_SPEED;
  }
  if (avertrack<0) {
  	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"Fail: GPS avertrack<0 INVALID TRACK \n");
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	return WCALC_INVALID_TRACK;
  }


  // p_heading is presumed heading , NSEW .. in degrees (understood from avertrack)
  // iaspeed is presumed IAS , as configured . in km/h (passed as parameter)

  // TODO If we have a digital compass, use wmode for it
#if 100215
  double p_heading=999;
  switch(wmode) {
	case 0:
		// wmode==0:  0 90  180  270   (N E S W)
		if (avertrack >=325.0 || avertrack<=35.0 ) p_heading=0;
		if (avertrack >=55.0 && avertrack<=125.0 ) p_heading=90;
		if (avertrack >=145.0 && avertrack<=215.0 ) p_heading=180;
		if (avertrack >=235.0 && avertrack<=305.0 ) p_heading=270;
		break;
	case 1:
		// wmode==1:  30 120 210 300
		if (avertrack >=355.0 || avertrack<=65.0 ) p_heading=30;
		if (avertrack >=85.0 && avertrack<=155.0 ) p_heading=120;
		if (avertrack >=175.0 && avertrack<=245.0 ) p_heading=210;
		if (avertrack >=265.0 && avertrack<=335.0 ) p_heading=300;
		break;
	case 2:
		// wmode==2:  60 150 240 330
		if (avertrack >=25.0 && avertrack<=95.0 ) p_heading=60;
		if (avertrack >=115.0 && avertrack<=185.0 ) p_heading=150;
		if (avertrack >=205.0 && avertrack<=275.0 ) p_heading=240;
		if (avertrack >=295.0 || avertrack<=5.0 ) p_heading=330;
		break;
	default:
		FailStore(_T("INVALID WMODE WINDCALC: %d%s"),wmode,NEWLINE);
		return WCALC_INVALID_DATA;
		break;
   }


#else
  double p_heading=999;
  if (avertrack >=325.0 || avertrack<=35.0 ) p_heading=0;
  if (avertrack >=55.0 && avertrack<=125.0 ) p_heading=90;
  if (avertrack >=145.0 && avertrack<=215.0 ) p_heading=180;
  if (avertrack >=235.0 && avertrack<=305.0 ) p_heading=270;
#endif

  if (p_heading==999) {
  	#ifdef DEBUG_WCALC
	sprintf(ventabuffer,"Fail: GPS avertrack=%d INVALID HEADING \n",(int)avertrack);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
	#endif
	return WCALC_INVALID_HEADING;
  } 
  #ifdef DEBUG_WCALC
  else {
	sprintf(ventabuffer,"*** avertrack=%d presumed HEADING is %d \n",(int)avertrack,(int)p_heading);
	if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
	{;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  }
  #endif
  #ifdef STORE_WCALC
  StartupStore(_T("... TrueWind: average track=%d , presumed heading to %d%s"),(int)avertrack,(int)p_heading, NEWLINE);
  #endif

  // Condor has its own wind precalculated: TrueWind will not cheat using read IAS
  #if TESTIASWITHCONDOR
  if (GPS_INFO.AirspeedAvailable && averias>0 ) iaspeed=averias;
  #else
  if (GPS_INFO.AirspeedAvailable && averias>0 && !devIsCondor(devA()) ) iaspeed=averias;
  #endif

  double tas=(iaspeed*TOKPH)*(1+0.02*(averaltitude/0.328/1000));

  double tasns=tas*cos( p_heading / RAD_TO_DEG );
  double tasew=tas*sin( p_heading / RAD_TO_DEG );

  double gsns=averspeed * cos(avertrack/ RAD_TO_DEG);
  double gsew=averspeed * sin(avertrack/ RAD_TO_DEG);

  double windns=gsns-tasns;
  double windew=gsew-tasew;

  double windsp=sqrt(pow(windns,2)+pow(windew,2));

  double windtmp=0;

  if (windns==0) windtmp=M_PI/2; else windtmp=atan(windew/windns);

  double q1=windtmp*RAD_TO_DEG;
  double q2=180+q1;
  double q3=q2;
  double q4=360+q1;
  double windto=0;
  if (windns>=0) {
	if ( windew>=0 )
		windto=q1;
	else
		windto=q4;
  }
  if (windns<0) {
	if ( windew>=0 )
		windto=q2;
	else
		windto=q3;
  }

  double windfrom=(windto-180)>0?windto-180:windto+180;
  if (windfrom==360) windfrom=0;

  #ifdef DEBUG_WCALC
  sprintf(ventabuffer,"ias=%.3f tas=%.3f tasns=%.3f tasew=%.3f gsns=%.3f gsew=%.3f\n windns=%.3f\
	windew=%.3f windsp=%.3f tmp=%.3f q1=%.3f q2=%.3f q3=%.3f q4=%.3f windto=%.3f windfrom=%.3f\n",
	iaspeed*TOKPH,tas, tasns, tasew, gsns, gsew, windns, windew, windsp, windtmp, q1, q2, q3,q4,windto, windfrom);
  if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  #endif

  *wfrom=windfrom;
  *wspeed=windsp;

  #ifdef DEBUG_WCALC
  sprintf(ventabuffer,"*** Wind from %.0f deg/ %.0f kmh\n", windfrom, windsp);
  if ((fp=fopen("DEBUG.TXT","a"))!= NULL)
    {;fprintf(fp,"%s\n",ventabuffer);fclose(fp);}
  #endif
  #ifdef STORE_WCALC
  StartupStore(_T("... TrueWind: calculated wind from %.0f deg, %.0f km/h%s"), windfrom, windsp,NEWLINE);
  #endif

  // StartupStore(_T(".......gsq=%d tq=%d wcalc=%d  qual=%d\n"),gsquality,trackquality, windcalctime,
  // 						((gsquality+trackquality)*100) / (windcalctime*2)  );
  // return quality percentage: part/total *100 
  if (haveias) 
	return ( ((gsquality+trackquality+iasquality)*100) / (windcalctime*3) );
  else
	return ( ((gsquality+trackquality)*100) / (windcalctime*2) );


}


void SetOverColorRef() {
  switch(OverColor) {
	case OcWhite:
		OverColorRef=RGB_WHITE;
		break;
	case OcBlack:
		OverColorRef=RGB_SBLACK;
		break;
	case OcBlue:
		OverColorRef=RGB_DARKBLUE;
		break;
	case OcGreen:
		OverColorRef=RGB_GREEN;
		break;
	case OcYellow:
		OverColorRef=RGB_YELLOW;
		break;
	case OcCyan:
		OverColorRef=RGB_CYAN;
		break;
	case OcOrange:
		OverColorRef=RGB_ORANGE;
		break;
	case OcGrey:
		OverColorRef=RGB_GREY;
		break;
	case OcDarkGrey:
		OverColorRef=RGB_DARKGREY;
		break;
	case OcDarkWhite:
		OverColorRef=RGB_DARKWHITE;
		break;
	case OcAmber:
		OverColorRef=RGB_AMBER;
		break;
	case OcLightGreen:
		OverColorRef=RGB_LIGHTGREEN;
		break;
	case OcPetrol:
		OverColorRef=RGB_PETROL;
		break;
	default:
		OverColorRef=RGB_MAGENTA;
		break;
  }
}

// handle custom keys. Input: key pressed (center, left etc.)
// Returns true if handled successfully, false if not
bool CustomKeyHandler(const int key) {

  int ckeymode;
  static bool doinit=true;
  static int oldModeIndex;

  if (doinit) {
	oldModeIndex=LKMODE_INFOMODE;;
	doinit=false;
  }

  switch(key) {
	case CKI_BOTTOMCENTER:
		ckeymode=CustomKeyModeCenter;
		break;
	case CKI_BOTTOMLEFT:	
		ckeymode=CustomKeyModeLeft;
		break;
	case CKI_BOTTOMRIGHT:	
		ckeymode=CustomKeyModeRight;
		break;
	case CKI_BOTTOMICON:	
		ckeymode=CustomKeyModeAircraftIcon;
		break;
	case CKI_TOPLEFT:
		ckeymode=CustomKeyModeLeftUpCorner;
		break;
	case CKI_TOPRIGHT:
		ckeymode=CustomKeyModeRightUpCorner;
		break;
	default:
		DoStatusMessage(_T("ERR-725 UNKNWOWN CUSTOMKEY"));
		return false;
		break;
  }

  switch(ckeymode) {
	case ckDisabled:
		break;
	case ckMenu:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		ShowMenu();
		return true;
	case ckBackMode:
		PreviousModeIndex();
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckToggleMap: //TODO
		if (ModeIndex==LKMODE_MAP)
			SetModeIndex(oldModeIndex);
		else {
			oldModeIndex=ModeIndex;
			SetModeIndex(LKMODE_MAP);
		}
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;

	case ckTrueWind:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::setMode(_T("TrueWind"));
//		MenuTimeOut = 0;
//		DisplayTimeOut = 0;
		return true;

	case ckTeamCode:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventSetup(_T("Teamcode"));
		return true;

	case ckToggleOverlays:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		ToggleOverlays();
		return true;

	case ckToggleMapLandable:
		if (ModeIndex==LKMODE_MAP)
			SetModeIndex(LKMODE_WP);
		else
			SetModeIndex(LKMODE_MAP);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckLandables:
		SetModeIndex(LKMODE_WP);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckToggleMapCommons:
		if (ModeIndex==LKMODE_MAP)
			SetModeIndex(LKMODE_NAV);
		else
			SetModeIndex(LKMODE_MAP);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckCommons:
		SetModeIndex(LKMODE_NAV);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckToggleMapTraffic:
		if (ModeIndex==LKMODE_MAP)
			SetModeIndex(LKMODE_TRF);
		else
			SetModeIndex(LKMODE_MAP);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckTraffic:
		SetModeIndex(LKMODE_TRF);
		MapWindow::RefreshMap();
		SoundModeIndex();
		return true;
	case ckInvertColors:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventInvertColor(NULL);
		return true;
	// Only used for aircraft icon
	case ckToggleInfobox:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) LKSound(_T("LK_BELL.WAV"));
		#endif
		MapWindow::RequestToggleFullScreen();
		return true;
	case ckTimeGates:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventTimeGates(NULL);
		return true;
	case ckMarkLocation:
		InputEvents::eventMarkLocation(_T(""));
		return true;
	case ckAutoZoom:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventZoom(_T("auto toggle"));
		InputEvents::eventZoom(_T("auto show"));
		return true;
	case ckActiveMap:
		InputEvents::eventActiveMap(_T("toggle"));
		InputEvents::eventActiveMap(_T("show"));
		return true;
	case ckBooster:
		DoStatusMessage(_T("FEEL THE THERMAL"));
		if (EnableSoundModes) LKSound(_T("LK_BOOSTER.WAV"));
		return true;
	case ckGoHome:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		if (ValidWayPoint(HomeWaypoint)) {
			if ( (ValidTaskPoint(ActiveWayPoint)) && (Task[ActiveWayPoint].Index == HomeWaypoint )) {
	// LKTOKEN  _@M82_ = "Already going home" 
				DoStatusMessage(gettext(TEXT("_@M82_")));
			} else {
				GotoWaypoint(HomeWaypoint);
			}
		} else
	// LKTOKEN  _@M465_ = "No Home to go!" 
			DoStatusMessage(gettext(TEXT("_@M465_")));
		return true;
	case ckPanorama:
		if (PGZoomTrigger==false)
			PGZoomTrigger=true;
		else
			LastZoomTrigger=0;
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		return true;

	case ckMultitargetRotate:
		RotateOvertarget();
		return true;

	case ckMultitargetMenu:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::setMode(_T("MTarget"));
		return true;
	case ckBaroToggle:
		ToggleBaroAltitude();
		return true;
	case ckBasicSetup:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::eventSetup(_T("Basic"));
		return true;
	case ckSimMenu:
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		InputEvents::setMode(_T("SIMMENU"));
		return true;
	default:
		DoStatusMessage(_T("ERR-726 INVALID CUSTOMKEY"));
		FailStore(_T("ERR-726 INVALID CUSTOMKEY=%d"),ckeymode);
		break;
  }

  return false;

}

#ifndef MAP_ZOOM
// set Climb and Cruis MapScale accordingly to PGClimbZoom, everytime it changes.
// Needed to avoid software restart to bypass doinit in MapWindow
void SetMapScales() {

  if (ISPARAGLIDER) {
	CruiseMapScale = MapWindow::RequestMapScale;

	switch(PGClimbZoom) {
		case 0:
			ClimbMapScale = 0.05;
			break;
		case 1:
			ClimbMapScale = 0.07;
			break;
		case 2:
			ClimbMapScale = 0.09;
			break;
		case 3:
			ClimbMapScale = 0.14;
			break;
		case 4:
		default:
			ClimbMapScale = 0.03;
			break;
	}
  } else {
	CruiseMapScale = MapWindow::RequestMapScale*2;
	ClimbMapScale = MapWindow::RequestMapScale/30; // 110104
  }

}
#endif /* ! MAP_ZOOM */

#ifdef PNA
bool LoadModelFromProfile()
{

  TCHAR tmpTbuf[MAX_PATH*2];
  char  tmpbuf[MAX_PATH*2];

  LocalPath(tmpTbuf,_T(LKD_CONF));
  _tcscat(tmpTbuf,_T("\\"));
  _tcscat(tmpTbuf,_T(XCSPROFILE));

  StartupStore(_T(". Searching modeltype inside default profile <%s>%s"),tmpTbuf,NEWLINE);

  FILE *fp=NULL;
  sprintf(tmpbuf,"%S",tmpTbuf);
  fp = fopen(tmpbuf, "rb");
  if(fp == NULL) {
	StartupStore(_T("... No default profile found%s"),NEWLINE);
	return false;
  }

  while (fgets(tmpbuf, (MAX_PATH*2)-1, fp) != NULL ) {

	if (strlen(tmpbuf)<21) continue;

	if (strncmp(tmpbuf,"AppInfoBoxModel",15) == 0) {
		int val=atoi(&tmpbuf[16]);
		GlobalModelType=val;
		SetModelName(val);
		StartupStore(_T("... ModelType found: <%s> val=%d%s"),GlobalModelName,GlobalModelType,NEWLINE);
		fclose(fp);
		return true;
	}
  }

  StartupStore(_T(". Modeltype not found in profile, probably Generic PNA is used.\n"));
  fclose(fp);
  return false;
}
#endif

#ifdef PNA
void CreateRecursiveDirectory(TCHAR *fullpath)
{
  TCHAR tmpbuf[MAX_PATH];
  TCHAR *p;
  TCHAR *lastslash;
  bool found;
  
  if ( _tcslen(fullpath) <10 || _tcslen(fullpath)>=MAX_PATH) {
	StartupStore(_T("... FontPath too short or too long, cannot create folders%s"),NEWLINE);
	return;
  }

  if (*fullpath != '\\' ) {
	StartupStore(TEXT("... FontPath <%s> has no leading backslash, cannot create folders on a relative path.%s"),fullpath,NEWLINE);
	return;
  }

  lastslash=tmpbuf;

  do {
	// we copy the full path in tmpbuf as a working copy 
	_tcscpy(tmpbuf,fullpath);
	found=false;
	// we are looking after a slash. like in /Disk/
	// special case: a simple / remaining which we ignore, because either it is the first and only (like in \)
	// or it is a trailing slash with a null following
	if (*(lastslash+1)=='\0') {
		break;
	}
	
	// no eol, so lets look for another slash, starting from the char after last
	for (p=lastslash+1; *p != '\0'; p++) {
		if ( *p == '\\' ) {
			*p='\0';
			found=true;
			lastslash=p;
			break;
		}
	}
	if (_tcscmp(tmpbuf,_T("\\Windows"))==0) {
		continue;
	}
	CreateDirectory(tmpbuf, NULL);
  } while (found);
			
  return;
}
#endif


// return current overtarget waypoint index, or -1 if not available
int GetOvertargetIndex(void) {
  int index;
  switch (OvertargetMode) {
	case OVT_TASK: // task 
		if ( ValidTaskPoint(ActiveWayPoint) != false ) {
			index = Task[ActiveWayPoint].Index;
			if ( index >=0 ) return index;
		}
		return -1;
		break;
	case OVT_ALT1: // alternate 1
		if ( ValidWayPoint(Alternate1) != false ) {
			index = Alternate1;
			if ( index >=0 ) return index;
		}
		return -1;
		break;
	case OVT_ALT2: // alternate 2
		if ( ValidWayPoint(Alternate2) != false ) {
			index = Alternate2;
			if ( index >=0 ) return index;
		}
		return -1;
		break;
	case OVT_BALT: // bestalternate
		if ( ValidWayPoint(BestAlternate) != false ) {
			index = BestAlternate;
			if ( index >=0 ) return index;
		}
		return -1;
		break;
	case OVT_HOME: // home waypoint
		if (ValidNotResWayPoint(HomeWaypoint)) {
			index = HomeWaypoint;
			if ( index >=0 ) return index;
		}
		return -1;
		break;

	case OVT_THER:
		index=RESWP_LASTTHERMAL;
		if (ValidResWayPoint(index)) return index;
		return -1;
		break;

	case OVT_MATE:
		index=RESWP_TEAMMATE;
		if (ValidResWayPoint(index)) return index;
		return -1;
		break;

	case OVT_FLARM:
		index=RESWP_FLARMTARGET;
		if (ValidResWayPoint(index)) return index;
		return -1;
		break;

	// 4: home, 5: traffic, 6: mountain pass, last thermal, etc.
	default:
		return -1;
		break;
  }
}

// return current overtarget waypoint name with leading identifier, even if empty
// exception for TEAM MATE: always report OWN CODE if available
void GetOvertargetName(TCHAR *overtargetname) {
  int index;
  if (OvertargetMode == OVT_MATE) {
	if (ValidWayPoint(TeamCodeRefWaypoint)) {
		if (TeammateCodeValid)
			_stprintf(overtargetname,_T("%s> %s"), GetOvertargetHeader(),CALCULATED_INFO.OwnTeamCode);
		else
			_stprintf(overtargetname,_T("%s: %s"), GetOvertargetHeader(),CALCULATED_INFO.OwnTeamCode);
	} else
		_stprintf(overtargetname,_T("%s ---"),GetOvertargetHeader());
	return;
  }
  index=GetOvertargetIndex();
  if (index<0)
	_stprintf(overtargetname,_T("%s ---"),GetOvertargetHeader());
  else 
	_stprintf(overtargetname,_T("%s%s"), GetOvertargetHeader(),WayPointList[index].Name);
}

#define OVERTARGETHEADER_MAX 3
// return current overtarget header name
TCHAR *GetOvertargetHeader(void) {
  static bool doinit=true;
  // Maxmode + 1 because maxmode does not account pos 0
  static TCHAR targetheader[OVT_MAXMODE+1][OVERTARGETHEADER_MAX+1];

  if (doinit) {
	// LKTOKEN _@M1323_ "T>"
	_tcsncpy(targetheader[OVT_TASK], gettext(TEXT("_@M1323_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1324_ "B>"
	_tcsncpy(targetheader[OVT_BALT], gettext(TEXT("_@M1324_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1325_ "1>"
	_tcsncpy(targetheader[OVT_ALT1], gettext(TEXT("_@M1325_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1326_ "2>"
	_tcsncpy(targetheader[OVT_ALT2], gettext(TEXT("_@M1326_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1327_ "H>"
	_tcsncpy(targetheader[OVT_HOME], gettext(TEXT("_@M1327_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1328_ "L>"
	_tcsncpy(targetheader[OVT_THER], gettext(TEXT("_@M1328_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1329_ "M"
	_tcsncpy(targetheader[OVT_MATE], gettext(TEXT("_@M1329_")), OVERTARGETHEADER_MAX);
	// LKTOKEN _@M1330_ "F>"
	_tcsncpy(targetheader[OVT_FLARM], gettext(TEXT("_@M1330_")), OVERTARGETHEADER_MAX);

	for (int i=0; i<OVT_MAXMODE+1; i++) targetheader[i][OVERTARGETHEADER_MAX]='\0';
	doinit=false;
  }

  return(targetheader[OvertargetMode]);
}

void RotateOvertarget(void) {

  OvertargetMode++;
  if (ISPARAGLIDER && BestWarning==false) {
	if (OvertargetMode==OVT_BALT) OvertargetMode++;
  }

  if (OvertargetMode==OVT_FLARM) {
	if (!GPS_INFO.FLARM_Available) OvertargetMode++;
  }

  if (OvertargetMode>OVT_ROTATE) {
	OvertargetMode=OVT_TASK;
  }
  #ifndef DISABLEAUDIO
  if (EnableSoundModes) {
	switch(OvertargetMode) {
		case 0:
			PlayResource(TEXT("IDR_WAV_OVERTONE7"));
			break;
		case 1:
			PlayResource(TEXT("IDR_WAV_OVERTONE0"));
			break;
		case 2:
			PlayResource(TEXT("IDR_WAV_OVERTONE1"));
			break;
		case 3:
			PlayResource(TEXT("IDR_WAV_OVERTONE2"));
			break;
		case 4:
			PlayResource(TEXT("IDR_WAV_OVERTONE3"));
			break;
		case 5:
			PlayResource(TEXT("IDR_WAV_OVERTONE4"));
			break;
		case 6:
			PlayResource(TEXT("IDR_WAV_OVERTONE5"));
			break;
		case 7:
			PlayResource(TEXT("IDR_WAV_OVERTONE6"));
			break;
		case 8:
			PlayResource(TEXT("IDR_WAV_OVERTONE7"));
			break;
		default:
			PlayResource(TEXT("IDR_WAV_OVERTONE5"));
			break;
	}
  }
  #endif
  return;

}


//
void ToggleOverlays() {
  static int oldLook8000;
  static bool doinit=true;

  if (doinit) {
	if (Look8000 == (Look8000_t)lxcNoOverlay)
		oldLook8000=(Look8000_t)lxcAdvanced;
	else
		oldLook8000=Look8000;
	doinit=false;
  }

  if (Look8000>lxcNoOverlay)
	Look8000=lxcNoOverlay;
  else
	Look8000=oldLook8000;

}

bool CheckClubVersion() {
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcfile, _T("CLUB"));
  if (  GetFileAttributes(srcfile) == 0xffffffff ) return false;
  return true;
}

void ClubForbiddenMsg() {
  MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M503_ = "Operation forbidden on CLUB devices" 
	gettext(TEXT("_@M503_")),
	_T("CLUB DEVICE"), 
	MB_OK|MB_ICONEXCLAMATION);
        return;
}

int GetFontRenderer() { // Karim

  switch(FontRenderer) {
	case 0:
		return NONANTIALIASED_QUALITY;
		break;
	case 1:
		return DEFAULT_QUALITY;
		break;
	case 2:
		return ANTIALIASED_QUALITY;
		break;
	case 3:
		return CLEARTYPE_COMPAT_QUALITY;
		break;
	default:
		return CLEARTYPE_COMPAT_QUALITY;
		break;
  }

}

// Are we using lockmode? What is the current status?
bool LockMode(const short lmode) {

  switch(lmode) {
	case 0:		// query availability of LockMode
		if (ISPARAGLIDER)
			return true;
		else
			return false;
		break;

	case 1:		// query lock/unlock status
		return LockModeStatus;
		break;

	case 2:		// invert lock status
		LockModeStatus = !LockModeStatus;
		return LockModeStatus;
		break;

	case 3:		// query button is usable or not, assuming ISPARAGLIDER
		// Positive if not flying
		return CALCULATED_INFO.Flying==TRUE?false:true;
		break;

	case 9:		// Check if we can unlock the screen
		if (CALCULATED_INFO.Flying == TRUE) {
			if ( (GPS_INFO.Time - CALCULATED_INFO.TakeOffTime)>10) {
				LockModeStatus=false;
			}
		}
		return LockModeStatus;
		break;

	default:
		return false;
		break;
  }

  return 0;

}


