/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include <stdio.h>
#include "externs.h"

#ifdef PNA
#include "LKHolux.h"
#include "LKRoyaltek3200.h"
#endif

void BottomSounds();


// vkmode 0=normal 1=gesture up 2=gesture down
// however we consider a down as up, and viceversa
int ProcessVirtualKey(int X, int Y, long keytime, short vkmode) {

#define UNGESTURES 1
#define VKTIMELONG 1500
#define DONTDRAWTHEMAP !MapWindow::mode.AnyPan()&&MapSpaceMode!=1

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

		#define _NOCOMPASSINCLUDE
		#include "./LKinclude_menusize.cpp"

		doinit=false;
	}
	
	sizeup=MapWindow::MapRect.bottom-MapWindow::MapRect.top;
	// do not consider navboxes, they are processed separately
	// These are coordinates for up down center VKs
	// yup and ydown are used normally on nearest page item selection, but also for real VK
	// that currently are almost unused. 

	if (DrawBottom) {
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

	#ifdef DEBUG_PROCVK
	TCHAR buf[100];
	#endif

	// Handle fullscreen 8000 mode 
	// sound clicks require some attention here
	if (DrawBottom && !MapWindow::mode.AnyPan() && vkmode==LKGESTURE_NONE) { 
		//
		// CLICKS on NAVBOXES, any MapSpaceMode ok
		//
		if (Y>= s_bottomY ) { // TESTFIX 090930

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
				BottomBarChange(true); // advance
				BottomSounds();
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
				BottomBarChange(false); // backwards
				BottomSounds();
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
		// CLICK ON SORTBOX line at the top, only with no map and only for enabled pages
		if ( (MapSpaceMode == MSM_LANDABLE || MapSpaceMode==MSM_AIRPORTS || 
			MapSpaceMode==MSM_NEARTPS || MapSpaceMode==MSM_TRAFFIC ||
			MapSpaceMode==MSM_AIRSPACES || MapSpaceMode==MSM_THERMALS)
			&& Y<=SortBoxY[MapSpaceMode] ) {

			// only search for 1-3, otherwise it's the fourth (fifth really)
			// we don't use 0 now
			for (i=0, j=4; i<4; i++) { // i=1 original 090925 FIX
				if (X <SortBoxX[MapSpaceMode][i]) {
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
				case MSM_AIRSPACES:
							SortedMode[MapSpaceMode]=j;
							LastDoAirspaces=0;
							#ifndef DISABLEAUDIO
							if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
							#endif
							break;

				case MSM_THERMALS:
							SortedMode[MapSpaceMode]=j;
							// force immediate resorting
							LastDoThermalH=0;
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
			case MSM_AIRSPACES:
						numpages=AspNumpages;
						break;
			case MSM_TRAFFIC:
						numpages=TrafficNumpages;
						break;
			case MSM_THERMALS:
						numpages=THistoryNumpages;
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
#if UNGESTURES
gesture_right:
#endif
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
#if UNGESTURES
gesture_left:
#endif
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
#if UNGESTURES
	// No need to use gestures if clicking on right or left center border screen
	// This will dramatically speed up the user interface in turbulence
	if (dontdrawthemap) {
		if (X<=s_xleft) goto gesture_left;
		if (X>=s_xright) goto gesture_right;
	}
#endif


	// no click for already clicked events

	//  Swap white and black colours on LK8000 : working only with virtual keys on, map mode
	//  Currently as of 2.1 virtual keys are almost obsoleted, and it is very unlikely that 
	//  someone will ever use this feature, which is also undocumented!!
	if (keytime>=VKTIMELONG && !dontdrawthemap) {
			static short oldOutline=OutlinedTp;
			if (OutlinedTp>(OutlinedTp_t)otDisabled) OutlinedTp=(OutlinedTp_t)otDisabled;
			else
				OutlinedTp=oldOutline;
			Appearance.InverseInfoBox = !Appearance.InverseInfoBox;
			#ifndef DISABLEAUDIO
			if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			MapWindow::RefreshMap();
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
/*
		// do not process enter in panmode, unused
		if ( !MapWindow::mode.AnyPan() ) {
	             DoStatusMessage(_T("Virtual ENTER")); 
		     return 13;
		}
*/

		if (SIMMODE) {
			if ( MapWindow::mode.AnyPan() && ISPARAGLIDER) return 99; // 091221 return impossible value
			else return 0;
		} else {
			return 0;
		}
	}
	DoStatusMessage(_T("VirtualKey Error")); 
	return 0;
}




