/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawLook8000.cpp,v 1.11 2011/01/06 01:20:11 root Exp root $
*/

#include "StdAfx.h"
#include "options.h"
#include "Cpustats.h"
#include "XCSoar.h"
#include "Utils2.h"
#include "compatibility.h"
#include "MapWindow.h"
#include "Units.h"
#include "McReady.h"
#include "externs.h"
#include "InputEvents.h"
#include <windows.h>
#include <math.h>
#include <tchar.h>
#include "InfoBoxLayout.h"
#include "Logger.h"
#include "Process.h"
#include "RasterTerrain.h" // 091109
#include "LKUtils.h"
#include "LKMapWindow.h"
#include "LKObjects.h"

#include "utils/heapcheck.h"


extern void DrawGlideCircle(HDC hdc, POINT Orig, RECT rc );
extern void MapWaypointLabelAdd(TCHAR *Name, int X, int Y, TextInBoxMode_t Mode, int AltArivalAGL, bool inTask, 
	bool isLandable, bool isAirport, bool isExcluded, int index);
extern int _cdecl MapWaypointLabelListCompare(const void *elem1, const void *elem2 );

extern void DrawMapSpace(HDC hdc, RECT rc);
extern void DrawNearest(HDC hdc, RECT rc);
extern void DrawNearestTurnpoint(HDC hdc, RECT rc);
extern void DrawCommon(HDC hdc, RECT rc);
extern void DrawWelcome8000(HDC hdc, RECT rc);
#ifdef CPUSTATS
extern void DrawCpuStats(HDC hdc, RECT rc);
#endif
#ifdef DRAWDEBUG
extern void DrawDebug(HDC hdc, RECT rc);
#endif

extern void WriteInfo(HDC hdc, bool *showunit, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle, 
				short *columnvalue, short *columntitle, short *row1, short *row2, short *row3);

extern int PDABatteryPercent;
extern int PDABatteryTemperature;
extern int MapWaypointLabelListCount;
extern void ConvToUpper(TCHAR *str);

extern NMEAParser nmeaParser1;
extern NMEAParser nmeaParser2;

typedef struct{
  TCHAR Name[NAME_SIZE+1];
  POINT Pos;
  TextInBoxMode_t Mode;
  int AltArivalAGL;
  bool inTask;
  bool isLandable; // VENTA5
  bool isAirport; // VENTA5
  bool isExcluded;
  int  index;
}MapWaypointLabel_t;


extern MapWaypointLabel_t MapWaypointLabelList[];


void MapWindow::DrawLook8000(HDC hdc,  RECT rc )
{
  HFONT		oldfont=0;
  SIZE TextSize, TextSize2;
  TCHAR Buffer[LKSIZEBUFFERLARGE];
  TCHAR BufferValue[LKSIZEBUFFERVALUE];
  TCHAR BufferUnit[LKSIZEBUFFERUNIT];
  TCHAR BufferTitle[LKSIZEBUFFERTITLE];
  char text[LKSIZETEXT];
  int index=-1;
  double Value;
  short rcx, rcy;
  short wlen;
  bool redwarning; // 091203
  int gatechrono=0;

  HFONT *bigFont;

  short leftmargin=0;

  static bool doinit=true;
  static bool flipflop=true;
  static short flipflopcount=0;

  static COLORREF barTextColor=RGB_WHITE; // default bottom bar text color, reversable

  short tlen;
  static int ySizeLK8BigFont;
  static int ySizeLK8MediumFont;
  static int ySizeLK8TargetFont;
  static short tlenFullScreen;
  static short tlenHalfScreen;
  // position Y of text in navboxes
  static short yRow2Title=0;	// higher row in portrait, unused in landscape
  static short yRow2Value=0;
  static short yRow2Unit=0;
  static short yRow1Title=0;	// lower row in portrait, the only one in landscape
  static short yRow1Value=0;
  static short yRow1Unit=0;

  static int splitoffset;

  static int splitoffset2; // second raw, which really is the first from top!



  // This is going to be the START 1/3  name replacing waypoint name when gates are running
  TCHAR StartGateName[12]; // 100506
  static TCHAR StartGateNameHS[12];
  static TCHAR StartGateNameFS[12];

  if (NewMap==false) return;

  if (!IsMapFullScreen()) return; // 101203


  redwarning=false;
  oldfont = (HFONT)SelectObject(hdc, LKINFOFONT); // FIXFONT

#ifndef MAP_ZOOM
  if ( IsMapFullScreen() && !EnablePan )
#else /* MAP_ZOOM */
  if ( IsMapFullScreen() && !mode.AnyPan() )
#endif /* MAP_ZOOM */
	DrawBottom=true; // TODO maybe also !TargetPan
  else
	DrawBottom=false;

  if ( ++flipflopcount >2 ) {
	flipflop = !flipflop;
	flipflopcount=0;
  }

  if (OverlaySize==0)
	bigFont=(HFONT *)LK8BigFont;
  else
	bigFont=(HFONT *)LK8TargetFont;

  if (doinit) {
	TCHAR Tdummy[]=_T("T");
	int iconsize;
	SelectObject(hdc, bigFont); 
	GetTextExtentPoint(hdc, Tdummy, _tcslen(Tdummy), &TextSize);
	ySizeLK8BigFont = TextSize.cy;

	SelectObject(hdc, LK8TargetFont); 
	GetTextExtentPoint(hdc, Tdummy, _tcslen(Tdummy), &TextSize);
	ySizeLK8TargetFont = TextSize.cy;

	SelectObject(hdc, LK8MediumFont); 
	GetTextExtentPoint(hdc, Tdummy, _tcslen(Tdummy), &TextSize);
	ySizeLK8MediumFont = TextSize.cy;

	// All these values are fine tuned for font/resolution/screenmode.
	// there is no speed issue inside doinit. take your time.
	SelectObject(hdc, LK8TitleNavboxFont); 
	GetTextExtentPoint(hdc, Tdummy, _tcslen(Tdummy), &TextSize);
	int syTitle = TextSize.cy;
	SelectObject(hdc, LK8ValueFont); 
	GetTextExtentPoint(hdc, Tdummy, _tcslen(Tdummy), &TextSize);
	int syValue = TextSize.cy;
	switch (ScreenSize) {
		// Row1 is the lower, Row2 is the top, for portrait
		// WARNING, algos are wrong, need to check and recalculate for each resolution!!
		// Changing font size in Utils2 does require checking and fixing here.
		case ss480x640:
		case ss480x800:
		case ss240x320:
		case ss272x480:
			yRow2Value =  rc.bottom-(syValue*2);
			yRow2Unit  =  yRow2Value;
			yRow2Title =  yRow2Value - (syValue/2) - (syTitle/2) + NIBLSCALE(2);
			yRow1Value =  rc.bottom-(syValue/2);
			yRow1Unit  =  yRow1Value;
			yRow1Title =  yRow1Value - (syValue/2) - (syTitle/2) + NIBLSCALE(2);
			break;
		
		case ss800x480:
		case ss640x480:
		case ss400x240:
			yRow2Value =  rc.bottom-(syValue*2);
			yRow2Unit  =  rc.bottom-(syValue*2) - NIBLSCALE(2);
			yRow2Title =  rc.bottom-(syValue*2) - syTitle;
			yRow1Value =  rc.bottom-(syValue/2);
			yRow1Unit  =  yRow1Value;
			yRow1Title =  rc.bottom-(syValue/2) - syTitle;
			break;

		default:
			yRow2Value =  rc.bottom-(syValue*2);
			yRow2Unit  =  rc.bottom-(syValue*2) - NIBLSCALE(2);
			yRow2Title =  rc.bottom-(syValue*2) - syTitle;
			yRow1Value =  rc.bottom-(syValue/2);
			yRow1Unit  =  rc.bottom-(syValue/2) - NIBLSCALE(2);
			yRow1Title =  rc.bottom-(syValue/2) - syTitle;
			break;
	}
	if ( ScreenSize < (ScreenSize_t)sslandscape ) {
		switch (ScreenSize) {			// portrait fullscreen
			case (ScreenSize_t)ss240x320:
				tlenFullScreen=8;
				// ST 1/3
				_tcscpy(StartGateNameFS,_T("ST "));
				break;
			default:
				_tcscpy(StartGateNameFS,_T("ST "));
				tlenFullScreen=8;
				break;
		}
		switch (ScreenSize) {			// portrait not fullscreen
			case (ScreenSize_t)ss240x320:
				_tcscpy(StartGateNameHS,_T("ST "));
				tlenHalfScreen=8;
				break;
			default:
				_tcscpy(StartGateNameHS,_T("ST "));
				tlenHalfScreen=8;
				break;
		}
	} else  {
		switch (ScreenSize) {			// landscape fullscreen
			case (ScreenSize_t)ss800x480:
			case (ScreenSize_t)ss400x240:
			case (ScreenSize_t)ss480x272:
			case (ScreenSize_t)ss720x408:
				// START 1/3
				_tcscpy(StartGateNameFS,_T("Start "));
				tlenFullScreen=9;
				break;
			case (ScreenSize_t)ss320x240:
				// STRT 1/3
				_tcscpy(StartGateNameFS,_T("Start "));
				tlenFullScreen=8; // 091114 reduced from 9
				break;
			case (ScreenSize_t)ss640x480:
				// STRT 1/3
				_tcscpy(StartGateNameFS,_T("Start "));
				tlenFullScreen=8;
				break;
			case (ScreenSize_t)ss896x672:
				_tcscpy(StartGateNameFS,_T("Start "));
				tlenFullScreen=9;
				break;
			default:
				_tcscpy(StartGateNameFS,_T("Start "));
				tlenFullScreen=9;
				break;
		}
		switch (ScreenSize) {			// landscape not fullscreen
			case (ScreenSize_t)ss480x272:
			case (ScreenSize_t)ss720x408:
				_tcscpy(StartGateNameHS,_T("ST "));
				tlenHalfScreen=7; // 091114 reduced from 9
				break;
			case (ScreenSize_t)ss320x240:
			case (ScreenSize_t)ss640x480:
				_tcscpy(StartGateNameHS,_T("ST "));
				tlenHalfScreen=6; // 091114 reduced from 7
				break;
			default:
				_tcscpy(StartGateNameHS,_T("ST "));
				tlenHalfScreen=7;
				break;
		}
	}
	
	if (ScreenLandscape) {
		iconsize=NIBLSCALE(26);
		splitoffset= ((rc.right-iconsize)-rc.left)/splitter;
	} else {
		iconsize=NIBLSCALE(26);
		splitoffset= ((rc.right-iconsize)-rc.left)/splitter;
		// splitoffset2= (rc.right-rc.left)/splitter;
		splitoffset2= splitoffset;
	}
	doinit=false; 
  } // end doinit

  COLORREF overcolor,distcolor;
  overcolor=OverColorRef;
  distcolor=OverColorRef;
  int yrightoffset;
  if (ScreenLandscape)
	yrightoffset=((rc.bottom + rc.top)/2)-NIBLSCALE(10);	// 101112
  else
	yrightoffset=((rc.bottom + rc.top)/3)-NIBLSCALE(10);	// 101112

  if (DrawBottom && MapSpaceMode!= MSM_MAP) {
	DrawMapSpace(hdc, rc);
	goto Drawbottom;
  }


  if ( MapWindow::IsMapFullScreen() ) {
	tlen=tlenFullScreen;
	_tcscpy(StartGateName,StartGateNameFS);
  } else {
	tlen=tlenHalfScreen;
	_tcscpy(StartGateName,StartGateNameHS);
  }


  // First we draw flight related values such as instant efficiency, altitude, new infoboxes etc.

#ifndef MAP_ZOOM
  if (MapWindow::IsMapFullScreen() && LKVarioBar && !EnablePan) { // 091214 Vario non available in pan mode
#else /* MAP_ZOOM */
  if (MapWindow::IsMapFullScreen() && LKVarioBar && !mode.AnyPan()) { // 091214 Vario non available in pan mode
#endif /* MAP_ZOOM */
	leftmargin=(LKVarioSize+NIBLSCALE(3)); // VARIOWIDTH + middle separator right extension
	tlen-=2; // 091115
	
  } else {
	leftmargin=0;
  }

  // no overlay - but we are still drawing MC and the wind on bottom left!
  if ( Look8000 == (Look8000_t)lxcNoOverlay ) goto drawOverlay;

  // PRINT WP TARGET NAME
  if ( ISPARAGLIDER && UseGates() && ActiveWayPoint==0) {
	// if running a task, use the task index normally
	if ( ValidTaskPoint(ActiveWayPoint) != false )
		index = Task[ActiveWayPoint].Index;
	else
		index=-1;
  } else {
	index = GetOvertargetIndex();
  }
#ifndef MAP_ZOOM
		if (DisplayMode != dmCircling) {
#else /* MAP_ZOOM */
		if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
#endif /* MAP_ZOOM */
			rcx=rc.left+leftmargin+NIBLSCALE(1);
			rcy=rc.top+NIBLSCALE(1);
		} else {
			if (ThermalBar) 
				rcx=rc.left+leftmargin+NIBLSCALE(40);
			else
				rcx=rc.left+leftmargin+NIBLSCALE(1);
			rcy=rc.top+NIBLSCALE(1);
		}
		// Waypoint name and distance
		SelectObject(hdc, LK8TargetFont);

	if ( index >=0 ) {
		#if 0
		// Active colours
		if (WayPointList[index].Reachable) {
			TextDisplayMode.AsFlag.Color = TEXTGREEN; 
		} else {
			TextDisplayMode.AsFlag.Color = TEXTRED;
		}
		#endif
		// OVERTARGET reswp not using redwarning because Reachable is not calculated
		if (index<=RESWP_END)
			redwarning=false;
		else {
			if (WayPointCalc[index].AltArriv[AltArrivMode]>0 && !WayPointList[index].Reachable ) 
				redwarning=true;
			else	
				redwarning=false;
		}

		int gateinuse=-2;
		if (UseGates() && ActiveWayPoint==0) {
			gateinuse=ActiveGate;
			if (!HaveGates()) {
				gateinuse=-1;
			} else {
				// this is set here for the first time, when havegates
				gatechrono=GateTime(ActiveGate)-LocalTime();
			}
			if (gateinuse<0) {
	// LKTOKEN  _@M157_ = "CLOSED" 
				_tcscpy(Buffer,gettext(TEXT("_@M157_")));
			} else {
				_stprintf(Buffer,_T("%s%d/%d"),StartGateName,gateinuse+1,PGNumberOfGates);
			}

	 		LKWriteText(hdc,Buffer, rcx+NIBLSCALE(2), rcy,0, WTMODE_OUTLINED, WTALIGN_LEFT, overcolor, true);
		} else {
			TCHAR buffername[LKSIZEBUFFERLARGE];
			GetOvertargetName(buffername);
			wlen=wcslen(buffername);
 			if (wlen>tlen) {
 			 	_tcsncpy(Buffer, buffername, tlen); Buffer[tlen]='\0';
			} else {
 			 	_tcsncpy(Buffer, buffername, wlen); Buffer[wlen]='\0';
			}

 			 ConvToUpper(Buffer);
			 LKWriteText(hdc,Buffer, rcx+NIBLSCALE(2), rcy,0, WTMODE_OUTLINED, WTALIGN_LEFT, overcolor, true);
		}

		 if (gateinuse>=-1) {
			// if we are still painting , it means we did not start yet..so we use colors
			if (CorrectSide() ) {
				distcolor=overcolor;
			} else {
				distcolor=RGB_AMBER;
			}
		 	LKFormatValue(LK_START_DIST, false, BufferValue, BufferUnit, BufferTitle);
		 } else {
			switch (OvertargetMode) {
				case OVT_TASK:
		 			LKFormatValue(LK_NEXT_DIST, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_ALT1:
		 			LKFormatDist(Alternate1, false, BufferValue, BufferUnit);
					break;
				case OVT_ALT2:
		 			LKFormatDist(Alternate2, false, BufferValue, BufferUnit);
					break;
				case OVT_BALT:
		 			LKFormatDist(BestAlternate, false, BufferValue, BufferUnit);
					break;
				case OVT_THER:
		 			LKFormatDist(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
					break;
				case OVT_HOME:
		 			LKFormatDist(HomeWaypoint, false, BufferValue, BufferUnit);
					break;
				case OVT_MATE:
		 			LKFormatDist(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
					break;
				case OVT_FLARM:
		 			LKFormatDist(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
					break;
				default:
		 			LKFormatValue(LK_NEXT_DIST, false, BufferValue, BufferUnit, BufferTitle);
					break;
			}
		}

		if ( (!OverlayClock || Look8000==lxcStandard) && ScreenLandscape && (!(ISPARAGLIDER && UseGates())) ) {
			_stprintf(BufferValue,_T("%s %s"),BufferValue,BufferUnit);
			if (MapWindow::IsMapFullScreen() ) {
				SelectObject(hdc, LK8TargetFont); 
				LKWriteText(hdc, BufferValue, rc.right-NIBLSCALE(30),rc.top+NIBLSCALE(1), 0, WTMODE_OUTLINED,WTALIGN_RIGHT,overcolor, true);
			} else {
				SelectObject(hdc, LK8MediumFont); 
				LKWriteText(hdc, BufferValue, rc.right-NIBLSCALE(28),rc.top+NIBLSCALE(1), 0, WTMODE_OUTLINED,WTALIGN_RIGHT,overcolor, true);
			}
		} else
			LKWriteText(hdc,BufferValue, rcx+NIBLSCALE(2), rcy+ ySizeLK8TargetFont, 0, WTMODE_OUTLINED, WTALIGN_LEFT, distcolor, true);

 		GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &TextSize2);
		if (!HideUnits) {
			SelectObject(hdc, LKMAPFONT); // TODO FIX BUG here.. using different font from size
			if (!OverlayClock && ScreenLandscape && !(ISPARAGLIDER && UseGates())) {

			} else {
			 LKWriteText(hdc, BufferUnit, rcx+NIBLSCALE(4)+TextSize2.cx,rcy+ySizeLK8TargetFont+(ySizeLK8TargetFont/3)-NIBLSCALE(1), 0, WTMODE_OUTLINED, WTALIGN_LEFT, overcolor, true); 
			}
		}

		// DIFF Bearing value displayed only when not circling
#ifndef MAP_ZOOM
	  	if (DisplayMode != dmCircling) {
#else /* MAP_ZOOM */
	  	if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
#endif /* MAP_ZOOM */
			switch (OvertargetMode) {
				case OVT_TASK:
		 			LKFormatValue(LK_BRGDIFF, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_ALT1:
		 			LKFormatBrgDiff(Alternate1, false, BufferValue, BufferUnit);
					break;
				case OVT_ALT2:
		 			LKFormatBrgDiff(Alternate2, false, BufferValue, BufferUnit);
					break;
				case OVT_BALT:
		 			LKFormatBrgDiff(BestAlternate, false, BufferValue, BufferUnit);
					break;
				case OVT_THER:
		 			LKFormatBrgDiff(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
					break;
				case OVT_HOME:
		 			LKFormatBrgDiff(HomeWaypoint, false, BufferValue, BufferUnit);
					break;
				case OVT_MATE:
		 			LKFormatBrgDiff(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
					break;
				case OVT_FLARM:
		 			LKFormatBrgDiff(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
					break;
				default:
		 			LKFormatValue(LK_BRGDIFF, false, BufferValue, BufferUnit, BufferTitle);
					break;
			}

			SelectObject(hdc, bigFont);
			if (!ISGAAIRCRAFT) {
				if (ScreenLandscape)
					LKWriteText(hdc, BufferValue, (rc.right+rc.left)/2, rc.top+ NIBLSCALE(15), 0, 
						WTMODE_OUTLINED, WTALIGN_CENTER, overcolor, true);
				else
					LKWriteText(hdc, BufferValue, ((rc.right+rc.left)/3)*2, rc.top+ NIBLSCALE(15), 0, 
						WTMODE_OUTLINED, WTALIGN_CENTER, overcolor, true);
			}
		}

		// Draw efficiency required and altitude arrival for destination waypoint
		// For paragliders, average efficiency and arrival destination

		SelectObject(hdc, bigFont); // use this font for big values

		if ( ISGLIDER) {
			switch (OvertargetMode) {
				case OVT_TASK:
		 			LKFormatValue(LK_NEXT_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_ALT1:
		 			LKFormatValue(LK_ALTERN1_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_ALT2:
		 			LKFormatValue(LK_ALTERN2_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_BALT:
		 			LKFormatValue(LK_BESTALTERN_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_HOME:
		 			LKFormatGR(HomeWaypoint, false, BufferValue, BufferUnit);
					break;
				case OVT_THER:
		 			LKFormatGR(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
					break;
				case OVT_MATE:
		 			LKFormatGR(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
					break;
				case OVT_FLARM:
		 			LKFormatGR(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
					break;
				default:
		 			LKFormatValue(LK_NEXT_GR, false, BufferValue, BufferUnit, BufferTitle);
					break;
			}

			GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &TextSize);
			//	rcy=(rc.bottom + rc.top)/2 -TextSize.cy-NIBLSCALE(10); OLD
			rcy=yrightoffset -TextSize.cy; // 101112
			rcx=rc.right-NIBLSCALE(10);
			if (redwarning)  // 091203
				LKWriteText(hdc, BufferValue, rcx,rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, RGB_AMBER, true);
			else
				LKWriteText(hdc, BufferValue, rcx,rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, overcolor, true);

			// Altitude difference with current MC
			switch (OvertargetMode) {
				case OVT_TASK:
		 			LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_ALT1:
		 			LKFormatValue(LK_ALTERN1_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_ALT2:
		 			LKFormatValue(LK_ALTERN2_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_BALT:
		 			LKFormatValue(LK_BESTALTERN_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
					break;
				case OVT_HOME:
		 			LKFormatAltDiff(HomeWaypoint, false, BufferValue, BufferUnit);
					break;
				case OVT_THER:
		 			LKFormatAltDiff(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
					break;
				case OVT_MATE:
		 			LKFormatAltDiff(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
					break;
				case OVT_FLARM:
		 			LKFormatAltDiff(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
					break;
				default:
		 			LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit,BufferTitle);
					break;
			}
			if (redwarning) 
				LKWriteText(hdc, BufferValue, rcx,rcy+TextSize.cy-NIBLSCALE(2), 0, 
					WTMODE_OUTLINED,WTALIGN_RIGHT,RGB_AMBER, true);
			else
				LKWriteText(hdc, BufferValue, rcx,rcy+TextSize.cy-NIBLSCALE(2), 0, 
					WTMODE_OUTLINED,WTALIGN_RIGHT,overcolor, true);
		}

	} // index>0
	// no valid index for current overmode, but we print something nevertheless
	else {
		TCHAR buffername[LKSIZEBUFFERLARGE];
		GetOvertargetName(buffername);
		wlen=wcslen(buffername);
	 	if (wlen>tlen) {
	 	 	_tcsncpy(Buffer, buffername, tlen); Buffer[tlen]='\0';
		} else {
	 	 	_tcsncpy(Buffer, buffername, wlen); Buffer[wlen]='\0';
		}
 		ConvToUpper(Buffer);
		LKWriteText(hdc,Buffer, rcx+NIBLSCALE(2), rcy,0, WTMODE_OUTLINED, WTALIGN_LEFT, overcolor, true);
	}

  // moved out from task paragliders stuff - this is painted on the right
  if ( ISPARAGLIDER ) {

	if (UseGates()&&ActiveWayPoint==0) {
		SelectObject(hdc, bigFont); // use this font for big values

		if (HaveGates()) {
			Units::TimeToTextDown(BufferValue,gatechrono ); 
			rcx=rc.right-NIBLSCALE(10);
			GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &TextSize);
			rcy=yrightoffset -TextSize.cy; // 101112
			LKWriteText(hdc, BufferValue, rcx,rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, overcolor, true);

			Value=WayPointCalc[Task[0].Index].NextETE-gatechrono;
			Units::TimeToTextDown(BufferValue, (int)Value);
			if (Value<=0) 
				LKWriteText(hdc, BufferValue, rcx,rcy+TextSize.cy-NIBLSCALE(2), 0, WTMODE_OUTLINED,WTALIGN_RIGHT,RGB_AMBER, true);
			else
				LKWriteText(hdc, BufferValue, rcx,rcy+TextSize.cy-NIBLSCALE(2), 0, WTMODE_OUTLINED,WTALIGN_RIGHT,overcolor, true);
		}

	} else {
		SelectObject(hdc, bigFont); // use this font for big values
#ifndef MAP_ZOOM
	  	if (DisplayMode == dmCircling)
#else /* MAP_ZOOM */
	  	if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
#endif /* MAP_ZOOM */
			LKFormatValue(LK_TC_30S, false, BufferValue, BufferUnit, BufferTitle);
		else
			LKFormatValue(LK_LD_AVR, false, BufferValue, BufferUnit, BufferTitle);

		GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &TextSize);
		rcy=yrightoffset -TextSize.cy; // 101112

		rcx=rc.right-NIBLSCALE(10);
		LKWriteText(hdc, BufferValue, rcx,rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, overcolor, true);

		// Altitude difference with current MC
		switch (OvertargetMode) {
			case OVT_TASK:
	 			LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
				break;
			case OVT_ALT1:
	 			LKFormatValue(LK_ALTERN1_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
				break;
			case OVT_ALT2:
	 			LKFormatValue(LK_ALTERN2_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
				break;
			case OVT_BALT:
	 			LKFormatValue(LK_BESTALTERN_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
				break;
			case OVT_HOME:
				LKFormatAltDiff(HomeWaypoint, false, BufferValue, BufferUnit);
				break;
			case OVT_THER:
				LKFormatAltDiff(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
				break;
			case OVT_MATE:
				LKFormatAltDiff(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
				break;
			case OVT_FLARM:
		 		LKFormatAltDiff(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
				break;
			default:
	 			LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
				break;
		}
		if (redwarning)
			LKWriteText(hdc, BufferValue, rcx,rcy+TextSize.cy-NIBLSCALE(2), 0, WTMODE_OUTLINED,WTALIGN_RIGHT,RGB_AMBER, true);
		else
			LKWriteText(hdc, BufferValue, rcx,rcy+TextSize.cy-NIBLSCALE(2), 0, WTMODE_OUTLINED,WTALIGN_RIGHT,overcolor, true);

	} // end no UseGates()
  } // is paraglider

drawOverlay:
  // In place of MC, print gate time
  // Even if lxcNoOverlay, we print startgates..
  if (UseGates()&&ActiveWayPoint==0) {
	SelectObject(hdc, LK8MediumFont); 

	if (HaveGates()) {
		Units::TimeToText(BufferTitle,GateTime(ActiveGate)); 
		_stprintf(BufferValue,_T("START %s"),BufferTitle);
	} else {
	// LKTOKEN  _@M316_ = "GATES CLOSED" 
		_tcscpy(BufferValue,gettext(TEXT("_@M316_")));
	}
	rcy=yrightoffset -ySizeLK8BigFont-(ySizeLK8MediumFont*2); // 101112
	rcx=rc.right-NIBLSCALE(10);
	LKWriteText(hdc, BufferValue, rcx,rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, overcolor, true);

	// USE THIS SPACE FOR MESSAGES TO THE PILOT
	rcy+=ySizeLK8MediumFont;
	if (HaveGates()) {
		if (gatechrono>0) {
			// IsInSector works reversed!
			if (PGStartOut && CALCULATED_INFO.IsInSector) {
				// LKTOKEN  _@M923_ = "WRONG inSIDE"
				_tcscpy(BufferValue,gettext(TEXT("_@M923_")));
			} else {
				if (!PGStartOut && !CALCULATED_INFO.IsInSector) {
					// LKTOKEN  _@M924_ = "WRONG outSIDE"
					_tcscpy(BufferValue,gettext(TEXT("_@M924_")));
				} else {
					// LKTOKEN  _@M921_ = "countdown"
					_tcscpy(BufferValue,gettext(TEXT("_@M921_")));
				}
			}
			if (!CALCULATED_INFO.Flying) {
				// LKTOKEN  _@M922_ = "NOT FLYING"
				_tcscpy(BufferValue,gettext(TEXT("_@M922_")));
			}
		} else {
			// gate is open
			if ( (ActiveGate<(PGNumberOfGates-1)) && (gatechrono<-300)) {
				// LKTOKEN  _@M314_ = "GATE OPEN" 
				_tcscpy(BufferValue,gettext(TEXT("_@M314_")));
			} else {
				if ( ActiveGate>=(PGNumberOfGates-1) )  {
					Units::TimeToText(BufferTitle,GateTime(ActiveGate+1)); 
					_stprintf(BufferValue,_T("CLOSE %s"),BufferTitle);
				} else {
					if (flipflop) {
						Units::TimeToText(BufferTitle,GateTime(ActiveGate+1)); 
						_stprintf(BufferValue,_T("NEXT %s"),BufferTitle);
					} else {
						// LKTOKEN  _@M314_ = "GATE OPEN" 
						_tcscpy(BufferValue,gettext(TEXT("_@M314_")));
					}
				}
				// LKTOKEN  _@M314_ = "GATE OPEN" 
				_tcscpy(BufferValue,gettext(TEXT("_@M314_")));
			}
		}
	} else {
		// LKTOKEN  _@M925_ = "NO TSK START"
		_tcscpy(BufferValue,gettext(TEXT("_@M925_")));
	}
	LKWriteText(hdc, BufferValue, rcx,rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, distcolor, true);

  } else
  if (McOverlay && Look8000>lxcNoOverlay && (ISGLIDER || ISPARAGLIDER)) {
	SelectObject(hdc, bigFont); 
	LKFormatValue(LK_MC, false, BufferValue, BufferUnit, BufferTitle);
	// rcy=(rc.bottom + rc.top)/2 -ySizeLK8BigFont-NIBLSCALE(10)-ySizeLK8BigFont; VERTICAL CENTERED
	rcy=yrightoffset -ySizeLK8BigFont-ySizeLK8BigFont;
	rcx=rc.right-NIBLSCALE(10);
	LKWriteText(hdc, BufferValue, rcx,rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, overcolor, true);
  }
  if ( Look8000 == (Look8000_t)lxcNoOverlay ) goto Drawbottom;


  if ( (Look8000==(Look8000_t)lxcAdvanced) ) {

	SelectObject(hdc, bigFont); 
	if (ISPARAGLIDER) {
		LKFormatValue(LK_HNAV, false, BufferValue, BufferUnit, BufferTitle); // 091115
	} else {
#ifndef MAP_ZOOM
		if (DisplayMode == dmCircling)
#else /* MAP_ZOOM */
		if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
#endif /* MAP_ZOOM */
			LKFormatValue(LK_TC_30S, false, BufferValue, BufferUnit, BufferTitle);
		else
			LKFormatValue(LK_LD_AVR, false, BufferValue, BufferUnit, BufferTitle);
	}
	GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &TextSize);
#ifndef MAP_ZOOM
	if (!EnablePan) // 091214
#else /* MAP_ZOOM */
	if (!mode.AnyPan()) // 091214
#endif /* MAP_ZOOM */
		rcx=rc.left+NIBLSCALE(10)+leftmargin+GlideBarOffset;   // 091115
	else
		rcx=rc.left+NIBLSCALE(10)+leftmargin;   // 091115
	if (ISPARAGLIDER||LKVarioBar)
		rcy=(rc.bottom + rc.top-BottomSize)/2 -TextSize.cy-NIBLSCALE(5);
	else
		rcy=(rc.bottom + rc.top)/2 -TextSize.cy-NIBLSCALE(10);

	if (ISPARAGLIDER) {
		LKWriteText(hdc, BufferValue, rcx, rcy-NIBLSCALE(2), 0, WTMODE_OUTLINED,WTALIGN_LEFT,overcolor, true);
		if (!HideUnits) {
			SelectObject(hdc, LKMAPFONT);  // FIXFONT
			LKWriteText(hdc, BufferUnit, rcx+TextSize.cx+NIBLSCALE(2),rcy+(TextSize.cy/3), 0, WTMODE_OUTLINED,WTALIGN_LEFT,overcolor, true);
		}

	} else {
		if (ISGLIDER) // 101204
		LKWriteText(hdc, BufferValue, rcx+NIBLSCALE(9), rcy-NIBLSCALE(2), 0, WTMODE_OUTLINED,WTALIGN_LEFT,overcolor, true);
	}

	if (ISPARAGLIDER || LKVarioBar) { // 100213
		//LKFormatValue(LK_HGPS, false, BufferValue, BufferUnit, BufferTitle);
		LKFormatValue(LK_VARIO, false, BufferValue, BufferUnit, BufferTitle); // 091115
		SelectObject(hdc, bigFont); 
		GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &TextSize);
		rcy+=TextSize.cy;
		LKWriteText(hdc, BufferValue, rcx,rcy-NIBLSCALE(2), 0, WTMODE_OUTLINED,WTALIGN_LEFT,overcolor, true);
		if (!HideUnits) {
			SelectObject(hdc, LKMAPFONT); // FIXFONT
			LKWriteText(hdc, BufferUnit, rcx+TextSize.cx+NIBLSCALE(2),rcy+(TextSize.cy/3), 0, WTMODE_OUTLINED,WTALIGN_LEFT,overcolor, true);
		}
	}

	if (!ISGAAIRCRAFT && !ISCAR) {
		LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
		SelectObject(hdc, bigFont); 
		GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &TextSize);
		rcy+=TextSize.cy;
		LKWriteText(hdc, BufferValue, rcx,rcy-NIBLSCALE(2), 0, WTMODE_OUTLINED,WTALIGN_LEFT,overcolor, true);
		if (!HideUnits) {
			SelectObject(hdc, LKMAPFONT);  
			LKWriteText(hdc, BufferUnit, rcx+TextSize.cx+NIBLSCALE(2),rcy+(TextSize.cy/3), 0, 
				WTMODE_OUTLINED,WTALIGN_LEFT,overcolor, true);
		}
	}

	LKFormatValue(LK_TIME_LOCALSEC, false, BufferValue, BufferUnit, BufferTitle);

	if ( ScreenSize < (ScreenSize_t)sslandscape ) {
		// 101005 Do not display CLOCK in portrait mode anymore
	} else {
		if (OverlayClock || (ISPARAGLIDER && UseGates()) ) {
			if (MapWindow::IsMapFullScreen() ) {
				SelectObject(hdc, LK8TargetFont); 
				LKWriteText(hdc, BufferValue, rc.right-NIBLSCALE(30),rc.top+NIBLSCALE(1), 0, 
					WTMODE_OUTLINED,WTALIGN_RIGHT,overcolor, true);
			} else {
				SelectObject(hdc, LK8MediumFont); 
				LKWriteText(hdc, BufferValue, rc.right-NIBLSCALE(28),rc.top+NIBLSCALE(1), 0, 
					WTMODE_OUTLINED,WTALIGN_RIGHT,overcolor, true);
			}
		}
	}

  }

  /*
   * We want a standard interface for everybody with a minimal configuration. 
   * This navbox mode is NOT optional, and it will not be configurable.
   */

Drawbottom:
  if (DrawBottom) {

    RECT nrc;

    nrc.left=0;
    nrc.top=rc.bottom-BottomSize;
    nrc.right=rc.right;
    nrc.bottom=rc.bottom;

  // HPEN hP; REMOVE 101204
  HBRUSH hB;
  if ( INVERTCOLORS ) {
	#if LKOBJ
  	hB = LKBrush_Black;
  	// hB = LKBrush_Ndark; REMOVE 101204
	// hP = LKPen_White_N0; // FIX  with Yellow  REMOVE 101204
	#else
  	hB = (HBRUSH)CreateSolidBrush(RGB_NDARK);
	// hP = (HPEN)CreatePen(PS_SOLID,0,RGB_YELLOW);  REMOVE 101204
	#endif
  } else {
	#if LKOBJ
  	hB = LKBrush_Nlight;
	// hP = LKPen_Black_N0; REMOVE 101204
	#else
  	hB = (HBRUSH)CreateSolidBrush(RGB_NLIGHT);
	// hP = (HPEN)CreatePen(PS_SOLID,0,RGB_BLACK); REMOVE 101204
	#endif
  }


  if (MapWindow::AlphaBlendSupported() && MapSpaceMode==MSM_MAP && BarOpacity<100) {
	static bool initablend=true;
	static HDC hdc2;
	static HBITMAP bitmapnew;
	if (initablend) {
		hdc2=CreateCompatibleDC(hdc);
		bitmapnew=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
		initablend=false;
	}

	if (BarOpacity==0) {
		barTextColor=RGB_BLACK;
	} else {
		BLENDFUNCTION bs;
		bs.BlendOp=AC_SRC_OVER;
		bs.BlendFlags=0;
		// A good value is 195
		bs.SourceConstantAlpha=BarOpacity*255/100;
		bs.AlphaFormat=0;
		SelectObject(hdc2,bitmapnew); 
		FillRect(hdc2,&nrc, hB); 
		MapWindow::AlphaBlendF(hdc,0,rc.bottom-BottomSize,rc.right,BottomSize,hdc2,0,rc.bottom-BottomSize,rc.right,BottomSize,bs);
		if (BarOpacity>25)
			barTextColor=RGB_WHITE;
		else
			barTextColor=RGB_BLACK;
	}

	// We have no un-init function. Todo!
	// DeleteObject(hdc2); DeleteObject(bitmapnew);
	
  } else {
	barTextColor=RGB_WHITE;
	FillRect(hdc,&nrc, hB); 
  }

  // NAVBOXES

  static bool wascircling=false; // init not circling of course
  static short OldBottomMode=BM_FIRST;
  bool showunit=false;

#ifndef MAP_ZOOM
  if ( (DisplayMode == dmCircling) && !wascircling) {
#else /* MAP_ZOOM */
  if ( MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) && !wascircling) {
#endif /* MAP_ZOOM */
	// switch to thermal mode
	OldBottomMode=BottomMode;
	BottomMode=BM_TRM;
	wascircling=true;
  }
#ifndef MAP_ZOOM
  if ( (DisplayMode != dmCircling) && wascircling) {
#else /* MAP_ZOOM */
  if ( !MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) && wascircling) {
#endif /* MAP_ZOOM */
	// back to cruise mode
	BottomMode=OldBottomMode;
	wascircling=false;
  }

  /*
   *   FIRST VALUE
   */

  showunit=true; // normally we do have a unit to show


  switch(BottomMode) {
	case BM_TRM:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(1,dmCircling);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(1,MapWindow::Mode::MODE_FLY_CIRCLING);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU:
		showunit=LKFormatValue(LK_TL_AVG, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_HGPS, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		showunit=LKFormatValue(LK_TC_ALL, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_FIN_DIST, true, BufferValue, BufferUnit, BufferTitle);
		break;

	case BM_SYS:
		showunit=LKFormatValue(LK_BATTERY, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;

	case BM_ALT:
		showunit=LKFormatValue(LK_BESTALTERN_GR, true, BufferValue, BufferUnit, BufferTitle); // 100221
		BufferTitle[7]='\0';
		break;

	case BM_CUS:
		index=GetInfoboxType(1);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	case BM_CUS2:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(1,dmCruise);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(1,MapWindow::Mode::MODE_FLY_CRUISE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
		
	case BM_CUS3:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(1,dmFinalGlide);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(1,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }

  rcx=rc.left+(splitoffset/2);
  if (ScreenLandscape) {
	#include "LKMW3include_navbox1.cpp"
  } else {
	#include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(7), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   SECOND VALUE
   */
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(2,dmCircling);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(2,MapWindow::Mode::MODE_FLY_CIRCLING);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU:
		showunit=LKFormatValue(LK_GNDSPEED, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_HBARO, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		showunit=LKFormatValue(LK_ODOMETER, true, BufferValue, BufferUnit, BufferTitle); // 100221
		// showunit=false; 100221
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_FIN_ALTDIFF, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_SYS:
		showunit=LKFormatValue(LK_EXTBATT1VOLT, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;

	case BM_ALT:
		if (ScreenLandscape) {
			showunit=LKFormatValue(LK_BESTALTERN_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
			wcscpy(BufferTitle,_T("<<<"));
		} else {
			showunit=LKFormatValue(LK_ALTERN1_GR, true, BufferValue, BufferUnit, BufferTitle);
			BufferTitle[7]='\0';
		}
		break;
	case BM_CUS:
		index=GetInfoboxType(2);
		showunit=LKFormatValue(index, false, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS2:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(2,dmCruise);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(2,MapWindow::Mode::MODE_FLY_CRUISE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
		
	case BM_CUS3:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(2,dmFinalGlide);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(2,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, false, BufferValue, BufferUnit, BufferTitle);
		break;
  }

  rcx+=splitoffset;
  if (ScreenLandscape) {
	#include "LKMW3include_navbox1.cpp"
  } else {
	#include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(7), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   THIRD VALUE
   */

  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(3,dmCircling);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(3,MapWindow::Mode::MODE_FLY_CIRCLING);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU:
		showunit=LKFormatValue(LK_HNAV, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_QFE, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		showunit=LKFormatValue(LK_TIMEFLIGHT, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_FIN_ETE, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_SYS:
		// TODO MAKE IT in LKPROCESS
		#if 100221
  		showunit=true;
  			wsprintf(BufferUnit, TEXT(""));
			if (SIMMODE) {
				// LKTOKEN _@M1199_ "Sat"
				wsprintf(BufferTitle, gettext(TEXT("_@M1199_")));
				wsprintf(BufferValue,TEXT("SIM"));
			} else {
				Value=GPS_INFO.SatellitesUsed;
				if (Value<1 || Value>30) {
					wsprintf(BufferValue,TEXT("---"));
				} else {
					sprintf(text,"%d",(int)Value);
					wsprintf(BufferValue, TEXT("%S"),text);

				}
				if (nmeaParser1.activeGPS == true)
					// LKTOKEN _@M1199_ "Sat"
					wsprintf(BufferTitle, TEXT("%s:A"), gettext(TEXT("_@M1199_")));
				else {
					if (nmeaParser2.activeGPS == true)
						// LKTOKEN _@M1199_ "Sat"
						wsprintf(BufferTitle, TEXT("%s:B"), gettext(TEXT("_@M1199_")));
					else
						// LKTOKEN _@M1199_ "Sat"
						wsprintf(BufferTitle, TEXT("%s:?"), gettext(TEXT("_@M1199_")));
				}
			}

		#else
		Value=GPS_INFO.SatellitesUsed;
		if (Value<1 || Value>30) {
			wsprintf(BufferValue,TEXT("---"));
 			//TextDisplayMode.AsFlag.Color = TEXTRED;
		}
		else {
		//	if (Value<3)
 		//		TextDisplayMode.AsFlag.Color = TEXTYELLOW;
  			sprintf(text,"%d",(int)Value);
			wsprintf(BufferValue, TEXT("%S"),text);
		}
  		showunit=false;
		// LKTOKEN _@M1199_ "Sat"
  		wsprintf(BufferTitle, gettext(TEXT("_@M1199_")));
		#endif
		break;
	case BM_ALT:
		if (ScreenLandscape)
			showunit=LKFormatValue(LK_ALTERN1_GR, true, BufferValue, BufferUnit, BufferTitle); // 100221
		else
			showunit=LKFormatValue(LK_ALTERN2_GR, true, BufferValue, BufferUnit, BufferTitle); // 100221
		BufferTitle[7]='\0';
		break;
	case BM_CUS:
		index=GetInfoboxType(3);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS2:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(3,dmCruise);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(3,MapWindow::Mode::MODE_FLY_CRUISE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
		
	case BM_CUS3:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(3,dmFinalGlide);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(3,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }

  rcx+=splitoffset;
  if (ScreenLandscape) {
	#include "LKMW3include_navbox1.cpp"
  } else {
	#include "LKMW3include_navbox2.cpp"
  }
  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(7), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   FOURTH VALUE
   */

  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(4,dmCircling);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(4,MapWindow::Mode::MODE_FLY_CIRCLING);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU:
		showunit=LKFormatValue(LK_NEXT_DIST, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_HAGL, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:  

		showunit=LKFormatValue(LK_HOME_DIST, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_TASK_DISTCOV, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_SYS:
		// LKTOKEN _@M1068_ "HBAR"
  		wsprintf(BufferTitle, gettext(TEXT("_@M1068_")));
		if (GPS_INFO.BaroAltitudeAvailable) {
			if (EnableNavBaroAltitude)
				// LKTOKEN _@M894_ "ON"
				wsprintf(BufferValue,gettext(TEXT("_@M894_")));
			else
				// LKTOKEN _@M491_ "OFF"
				wsprintf(BufferValue,gettext(TEXT("_@M491_")));
		} else
			wsprintf(BufferValue,TEXT("---"));
  		showunit=false;
		break;
	case BM_ALT:
		if (ScreenLandscape) {
			showunit=LKFormatValue(LK_ALTERN1_ARRIV, true, BufferValue, BufferUnit, BufferTitle); // 100221
			wcscpy(BufferTitle,_T("<<<"));
		} else {
			showunit=LKFormatValue(LK_BESTALTERN_ARRIV, true, BufferValue, BufferUnit, BufferTitle); // 100221
			wcscpy(BufferTitle,_T(""));
		}
		break;
	case BM_CUS:
		index=GetInfoboxType(4);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS2:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(4,dmCruise);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(4,MapWindow::Mode::MODE_FLY_CRUISE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
		
	case BM_CUS3:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(4,dmFinalGlide);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(4,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }


  if (ScreenLandscape) {
	rcx+=splitoffset;
  }else {
	rcx=rc.left+(splitoffset2/2);
  }
  #include "LKMW3include_navbox1.cpp"
  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(7), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   FIFTH VALUE
   */
  if (ScreenLandscape && (splitter<5)) goto EndOfNavboxes;
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(5,dmCircling);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(5,MapWindow::Mode::MODE_FLY_CIRCLING);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU: 
		showunit=LKFormatValue(LK_NEXT_GR, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_FL, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_AUX:
		showunit=LKFormatValue(LK_HOMERADIAL, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_TSK:
// TODO MAKE IT LKPROCESS
  		Value=ALTITUDEMODIFY*CALCULATED_INFO.TaskStartAltitude;
		if (Value>0) {
			sprintf(text,"%d",(int)Value);
			wsprintf(BufferValue, TEXT("%S"),text);
  			wsprintf(BufferUnit, TEXT("%s"),(Units::GetAltitudeName()));
		} else {
			wsprintf(BufferValue, TEXT("---"));
			wsprintf(BufferUnit, TEXT(""));
			showunit=false;
		}
 		// LKTOKEN _@M1200_ "Start"
		wsprintf(BufferTitle, gettext(TEXT("_@M1200_")));
		break;
	case BM_SYS:
		showunit=LKFormatValue(LK_EMPTY, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_ALT:
		if (ScreenLandscape) {
			showunit=LKFormatValue(LK_ALTERN2_GR, true, BufferValue, BufferUnit, BufferTitle); // 100221
			BufferTitle[7]='\0';
		} else {
			showunit=LKFormatValue(LK_ALTERN1_ARRIV, false, BufferValue, BufferUnit, BufferTitle); // 100221
			wcscpy(BufferTitle,_T(""));
		}
		break;
	case BM_CUS:
		index=GetInfoboxType(5);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS2:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(5,dmCruise);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(5,MapWindow::Mode::MODE_FLY_CRUISE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
		
	case BM_CUS3:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(5,dmFinalGlide);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(5,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }


  if (ScreenLandscape) {
	rcx+=splitoffset;
  }else {
	rcx+=splitoffset2;
  }
  #include "LKMW3include_navbox1.cpp"

  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(3), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *   SIXTH VALUE
   */
  if (ScreenLandscape && (splitter<6)) goto EndOfNavboxes;
  showunit=true;
  switch(BottomMode) {
	case BM_TRM:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(6,dmCircling);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(6,MapWindow::Mode::MODE_FLY_CIRCLING);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CRU:
		showunit=LKFormatValue(LK_LD_AVR, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_HGH:
		showunit=LKFormatValue(LK_AQNH, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_AUX:
		showunit=LKFormatValue(LK_LD_CRUISE, true, BufferValue, BufferUnit, BufferTitle); // 100221
		break;
	case BM_TSK:
		showunit=LKFormatValue(LK_SPEEDTASK_ACH, true, BufferValue, BufferUnit, BufferTitle);
		break;
	case BM_SYS:
		if (LoggerGActive()) {
  			wsprintf(BufferValue, TEXT("OK"));
		} else {
  			wsprintf(BufferValue, TEXT("NO!"));
		}
  		wsprintf(BufferTitle, TEXT("GRec"));
  		wsprintf(BufferUnit, TEXT("")); // 100221
		showunit=true;
		break;
	case BM_ALT:
		showunit=LKFormatValue(LK_ALTERN2_ARRIV, true, BufferValue, BufferUnit, BufferTitle); // 100221
		if (ScreenLandscape)
			wcscpy(BufferTitle,_T("<<<"));
		else
			wcscpy(BufferTitle,_T(""));
		break;
	case BM_CUS:
		index=GetInfoboxType(6);
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
	case BM_CUS2:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(6,dmCruise);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(6,MapWindow::Mode::MODE_FLY_CRUISE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;
		
	case BM_CUS3:
#ifndef MAP_ZOOM
		index=GetInfoboxIndex(6,dmFinalGlide);
#else /* MAP_ZOOM */
		index=GetInfoboxIndex(6,MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
#endif /* MAP_ZOOM */
		showunit=LKFormatValue(index, true, BufferValue, BufferUnit, BufferTitle);
		BufferTitle[7]='\0';
		break;

	default:
		showunit=LKFormatValue(LK_ERROR, true, BufferValue, BufferUnit, BufferTitle);
		break;
  }


  if (ScreenLandscape) {
	rcx+=splitoffset;
  }else {
	rcx+=splitoffset2;
  }
  #include "LKMW3include_navbox1.cpp"
  LKWriteText(hdc, BufferTitle, rcx+NIBLSCALE(3), rcy, 0, WTMODE_NORMAL,WTALIGN_CENTER,barTextColor, false);

  /*
   *    CLEAN UP 
   */

EndOfNavboxes:
  #ifndef LKOBJ
  DeleteObject(hB);
  // DeleteObject(hP);  REMOVE 101204
  #else
  ;
  #endif

} // drawbottom

  if (DrawBottom && MapSpaceMode != MSM_MAP) goto TheEnd;

  //
  // Draw wind 
  //
  SelectObject(hdc, LK8TargetFont);

  if (Look8000 == lxcNoOverlay) goto afterWind; // 100930

  if (ISCAR || ISGAAIRCRAFT) {
	if (!HideUnits) {
		LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
		_stprintf(Buffer,_T("%s %s"),BufferValue,BufferUnit);
	} else {
		LKFormatValue(LK_GNDSPEED, false, Buffer, BufferUnit, BufferTitle);
	}
  } else {
	LKFormatValue(LK_WIND, false, Buffer, BufferUnit, BufferTitle);
  }

  rcy=ySizeLK8TargetFont;
 
  if (DrawBottom)
  	LKWriteText(hdc, Buffer, rc.left+NIBLSCALE(5)+leftmargin, rc.bottom - BottomSize- rcy-NIBLSCALE(2), 0, WTMODE_OUTLINED,WTALIGN_LEFT,overcolor, true);
  else
  	LKWriteText(hdc, Buffer, rc.left+NIBLSCALE(5)+leftmargin, rc.bottom - rcy-NIBLSCALE(5), 0, WTMODE_OUTLINED,WTALIGN_LEFT,overcolor, true);


afterWind:

   if ( UseMapLock && MapLock ) {
	_stprintf(Buffer,TEXT("MAPLOCK"));
  	SelectObject(hdc, LKMAPFONT); // FIXFONT
  	GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &TextSize);
	if (DrawBottom)
  		LKWriteText(hdc, Buffer, ((rc.right-rc.left-leftmargin)/2)+leftmargin, rc.bottom - BottomSize- (TextSize.cy/2)-NIBLSCALE(2) , 0, WTMODE_OUTLINED,WTALIGN_CENTER,RGB_WHITE, true);
  	else
  		LKWriteText(hdc, Buffer, ((rc.right-rc.left-leftmargin)/2)+leftmargin,  rc.bottom - (TextSize.cy/2)-NIBLSCALE(2) , 0, WTMODE_OUTLINED,WTALIGN_CENTER,RGB_WHITE, true);

   }
   if ( !MapWindow::IsMapFullScreen() && InfoFocus>=0 ) {

	_stprintf(Buffer,TEXT("IBOX"));
  	SelectObject(hdc, LKMAPFONT); // FIXFONT
  	GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &TextSize);
  	LKWriteText(hdc, Buffer, rc.right - (TextSize.cx)-NIBLSCALE(20) ,  rc.bottom - (TextSize.cy/2)-NIBLSCALE(2) , 0, WTMODE_OUTLINED,WTALIGN_CENTER,RGB_LIGHTGREEN, true);
	if (iboxtoclick) {
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK")); 
		#endif
		iboxtoclick=false;
	}
   } 

TheEnd:

  // restore font and return
  SelectObject(hdc, oldfont); 


}

