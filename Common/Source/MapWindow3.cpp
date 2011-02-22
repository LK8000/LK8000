/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindow3.cpp,v 8.46 2010/12/12 14:33:14 root Exp root $ BAD
*/

#include "StdAfx.h"
#include "options.h"
#include "Cpustats.h"
#include "XCSoar.h"
#include "compatibility.h"
#include "McReady.h"
#include "externs.h"
#include <windows.h>
#include <math.h>
#include <tchar.h>
#include "InfoBoxLayout.h"
#include "LKMapWindow.h"
#include "buildnumber.h"
#include "Utils2.h"
#if LKOBJ
#include "LKObjects.h"
#endif

#if (WINDOWSPC>0)
#include <wingdi.h>
#endif

extern void DrawGlideCircle(HDC hdc, POINT Orig, RECT rc );
extern void MapWaypointLabelAdd(TCHAR *Name, int X, int Y, TextInBoxMode_t Mode, int AltArivalAGL, bool inTask, 
	bool isLandable, bool isAirport, bool isExcluded, int index);
extern int _cdecl MapWaypointLabelListCompare(const void *elem1, const void *elem2 );

extern void DrawMapSpace(HDC hdc, RECT rc);
extern void DrawNearest(HDC hdc, RECT rc);
extern void DrawNearestTurnpoint(HDC hdc, RECT rc);
extern void DrawCommon(HDC hdc, RECT rc);
extern void DrawTraffic(HDC hdc, RECT rc);
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


/*
 * The VisualGlide by Paolo Ventafridda
 * Sort of a Stocker dynamic chart!
 *
 * VisualGlide=1 : Steady sector/circle
 *             2 : Moving sector/circle   optional configurable, not much useful.
 */
void MapWindow::DrawGlideCircle(HDC hdc, POINT Orig, RECT rc )
{
  double tmp=0;
  TCHAR gtext[LKSIZEBUFFERLARGE];
  char text[LKSIZETEXT]; 
  double cruise=1;
  int i;
  double gunit;
  COLORREF oldcolor=0;
  HFONT oldfont;

  static double maxcruise;
  static double mincruise;
  static int spread;

  maxcruise=(GlidePolar::bestld); 
  mincruise=(GlidePolar::bestld/4);

  cruise= CALCULATED_INFO.AverageLD; 

  if ( cruise <= 0 ) cruise = GlidePolar::bestld; // 091215 let cruise be always reasonable
  if ( cruise < mincruise ) return;
  if ( cruise >maxcruise ) cruise=maxcruise;


  // Spread from 
  static short turn=1;
  static short count=0;
  spread += (10 * turn); 
  if ( spread <-25 || spread >25 ) turn*=-1;
  if ( ++count >6) count=-1;

  SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
  // SetBkMode(hdc,TRANSPARENT);

  //oldfont = (HFONT)SelectObject(hdc, MapWindowBoldFont); // FIXFONT
  oldfont = (HFONT)SelectObject(hdc, LK8InfoNormalFont); // FIXFONT

  // 100m or 300ft scale
  if ( Units::GetUserAltitudeUnit() == unMeter ) gunit=100; else gunit = 91.44;

  for (i=1; i<9; i++) {

      SelectObject(hdc, hpVisualGlideHeavyBlack); // 091215
#if 0
    // toggling colors 
    if (turn>0 ) {
      if ( (i==2 || i==4 || i==6 || i == 8) ) SelectObject(hdc, hpVisualGlideHeavyRed);
      else SelectObject(hdc, hpVisualGlideLightRed);
    } else {
      if ( (i==2 || i==4 || i==6 || i == 8) ) SelectObject(hdc, hpVisualGlideHeavyBlack);
      else SelectObject(hdc, hpVisualGlideLightBlack);
    }
#endif
#if 0
    if (turn>0 )
		SelectObject(hdc, hpVisualGlideHeavyRed);
    else
		SelectObject(hdc, hpVisualGlideHeavyBlack);
#endif

    /*
     * TRACKUP, NORTHUP, NORTHCIRCLE, TRACKCIRCLE, NORTHTRACK
     */
	if ( ((DisplayOrientation == TRACKUP) || (DisplayOrientation == NORTHCIRCLE) || (DisplayOrientation == TRACKCIRCLE))
#ifndef MAP_ZOOM
		&& (DisplayMode != dmCircling) ) {

#else /* MAP_ZOOM */
           && (!mode.Is(MapWindow::Mode::MODE_CIRCLING)) ) {
#endif /* MAP_ZOOM */
		if ( VisualGlide == 1 ) {
#ifndef MAP_ZOOM
			tmp = i*gunit*cruise*ResMapScaleOverDistanceModify;
#else /* MAP_ZOOM */
			tmp = i*gunit*cruise*zoom.ResScaleOverDistanceModify();
#endif /* MAP_ZOOM */
			DrawArc(hdc, Orig.x, Orig.y,(int)tmp, rc, 315, 45);
		} else {
#ifndef MAP_ZOOM
			tmp = i*gunit*cruise*ResMapScaleOverDistanceModify;
#else /* MAP_ZOOM */
			tmp = i*gunit*cruise*zoom.ResScaleOverDistanceModify();
#endif /* MAP_ZOOM */
			DrawArc(hdc, Orig.x, Orig.y,(int)tmp, rc, 330+spread, 30+spread);
		}
	} else {
#ifndef MAP_ZOOM
		tmp = i*gunit*cruise*ResMapScaleOverDistanceModify;
#else /* MAP_ZOOM */
		tmp = i*gunit*cruise*zoom.ResScaleOverDistanceModify();
#endif /* MAP_ZOOM */
		Circle(hdc, Orig.x,Orig.y,(int)tmp, rc, true, false);
	}


	if (turn>0) oldcolor=SetTextColor(hdc, RGB_BLACK); 
	else oldcolor=SetTextColor(hdc, RGB_BLUE); // red

	if ( i==2 || i==4 || i==6 || i==8 ) { 
		if ( Units::GetUserAltitudeUnit() == unMeter ) 
			wsprintf(gtext,_T("-%dm"),i*100); 
		else
			wsprintf(gtext,_T("-%dft"),i*300);

			ExtTextOut( hdc, Orig.x+35, Orig.y-5 - (int) tmp, 0, NULL, gtext , _tcslen(gtext), NULL );
	}

	SetTextColor(hdc,oldcolor);
	if (turn>0)
		oldcolor=SetTextColor(hdc, RGB_BLACK); // dark grey
	else
		oldcolor=SetTextColor(hdc, RGB_BLUE); // red

	if ( i==2 || i==4 || i==6 || i==8 ) {
		if ( Units::GetUserDistanceUnit() == unKiloMeter ) {

			//sprintf(text,"%3.1f Km",i*100*cruise /1000);
			sprintf(text,"%3.0fkm",i*100*cruise /1000);
		} else
			if ( Units::GetUserDistanceUnit() == unNauticalMiles ) {

				//sprintf(text,"%3.1f nmi", i*100*cruise / 1852);
				sprintf(text,"%3.0fnm", i*100*cruise / 1852);
			} else
				if ( Units::GetUserDistanceUnit() == unStatuteMiles ) {
					//sprintf(text,"%3.1f mi", i*100*cruise / 1609);
					sprintf(text,"%3.0fm", i*100*cruise / 1609);
				}

				wsprintf(gtext,_T("%S"),text);; 
					ExtTextOut( hdc, Orig.x-100, Orig.y-5 - (int) tmp, 0, NULL, gtext , _tcslen(gtext), NULL );
	}	
	SetTextColor(hdc,oldcolor);

  }

  SelectObject(hdc, oldfont);
  SetTextColor(hdc,oldcolor);

}


void MapWindow::DrawHeading(HDC hdc, POINT Orig, RECT rc ) {

   if (GPS_INFO.NAVWarning) return; // 100214

#ifndef MAP_ZOOM
   if (MapScale>5 || (DisplayMode == dmCircling)) return;
#else /* MAP_ZOOM */
   if (zoom.Scale()>5 || mode.Is(MapWindow::Mode::MODE_CIRCLING)) return;
#endif /* MAP_ZOOM */
   POINT p2;

   #if 0
   if ( !( DisplayOrientation == TRACKUP || DisplayOrientation == NORTHCIRCLE || DisplayOrientation == TRACKCIRCLE )) return;
#ifndef MAP_ZOOM
   double tmp = 12000*ResMapScaleOverDistanceModify;
#else /* MAP_ZOOM */
   double tmp = 12000*zoom.ResScaleOverDistanceModify();
#endif /* MAP_ZOOM */
   p2.x=Orig.x;
   p2.y=Orig.y-(int)tmp;
   #else
#ifndef MAP_ZOOM
   double tmp = 12000*ResMapScaleOverDistanceModify;
#else /* MAP_ZOOM */
   double tmp = 12000*zoom.ResScaleOverDistanceModify();
#endif /* MAP_ZOOM */
   if ( !( DisplayOrientation == TRACKUP || DisplayOrientation == NORTHCIRCLE || DisplayOrientation == TRACKCIRCLE )) {
	double trackbearing = DrawInfo.TrackBearing;
	p2.y= Orig.y - (int)(tmp*fastcosine(trackbearing));
	p2.x= Orig.x + (int)(tmp*fastsine(trackbearing));
   } else {
	p2.x=Orig.x;
	p2.y=Orig.y-(int)tmp;
   }
   #endif

   if (BlackScreen)
	   _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), Orig, p2, RGB_INVDRAW, rc); // 091109
   else
	   _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), Orig, p2, RGB_BLACK, rc);

}

void MapWindow::DrawMapSpace(HDC hdc,  RECT rc ) {

  HFONT oldfont;
  HBRUSH hB;

  TextInBoxMode_t TextDisplayMode;
  TCHAR Buffer[LKSIZEBUFFERLARGE*2];
#ifdef DRAWLKSTATUS
  bool dodrawlkstatus=false;
#endif
  static bool doinit=true;
  static POINT p[10];

  #if LKOBJ
  if (MapSpaceMode==MSM_WELCOME) {
	if (INVERTCOLORS)
		hB=LKBrush_Petrol;
	  else
		hB=LKBrush_Mlight;
  } else {
	if (INVERTCOLORS)
		hB=LKBrush_Mdark;
	  else
		hB=LKBrush_Mlight;
  }
  #else
  if (INVERTCOLORS)
	hB=CreateSolidBrush(RGB_MDARK);
  else
	hB=CreateSolidBrush(RGB_MLIGHT);
  #endif
  oldfont = (HFONT)SelectObject(hdc, LKINFOFONT); // save font
  FillRect(hdc,&rc, hB); 
  #ifndef LKOBJ
  DeleteObject(hB);
  #endif
  //oldbkmode=SetBkMode(hdc,TRANSPARENT);

  if (doinit) {
	p[0].x=0; p[0].y=rc.bottom-BottomSize-NIBLSCALE(2); p[1].x=rc.right-1; p[1].y=p[0].y;
	p[2].x=0; p[2].y=0; p[3].x=rc.right-1; p[3].y=0; // 091230 right-1
	p[4].x=0; p[4].y=0; p[5].x=0; p[5].y=rc.bottom-BottomSize-NIBLSCALE(2);
	p[6].x=rc.right-1; p[6].y=0; p[7].x=rc.right-1; p[7].y=rc.bottom-BottomSize-NIBLSCALE(2); // 091230 right-1

//	p[8].x=0; p[8].y=rc.bottom-BottomSize-NIBLSCALE(2); p[9].x=rc.right; p[9].y=p[8].y;
	doinit=false; 
  }

  if (INVERTCOLORS) {
	_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[2], p[3], RGB_GREEN, rc);
	_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[4], p[5], RGB_GREEN, rc);
	_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[6], p[7], RGB_GREEN, rc);
	_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[0], p[1], RGB_GREEN, rc);
  } else {
	_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[2], p[3], RGB_DARKGREEN, rc);
	_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[4], p[5], RGB_DARKGREEN, rc);
	_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[6], p[7], RGB_DARKGREEN, rc);
	_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[0], p[1], RGB_DARKGREEN, rc);
  }


#ifdef DRAWLKSTATUS 
  if (LKevent==LKEVENT_NEWRUN) dodrawlkstatus=true;
#endif

  switch (MapSpaceMode) {
	case MSM_WELCOME:
#if (1)
		static double firsttime=GPS_INFO.Time;
		// delayed automatic exit from welcome mode
		if ( GPS_INFO.Time > (firsttime+1.0) ) {
			SetModeType(LKMODE_MAP,MP_MOVING);
			LKevent=LKEVENT_NONE;
			break;
		}
#endif
		DrawWelcome8000(hdc, rc);
		break;
	case MSM_LANDABLE:
	case MSM_AIRPORTS:
	case MSM_NEARTPS:
		DrawNearest(hdc, rc);
		break;
/* 101222 REMOVE
	case MSM_NEARTPS:
		DrawNearestTurnpoint(hdc, rc);
		break;
*/
	case MSM_COMMON:
	case MSM_RECENT:
		DrawCommon(hdc, rc);
		break;
	case MSM_MAP:
		break;
	case MSM_INFO_THERMAL:
	case MSM_INFO_CRUISE:
	case MSM_INFO_TASK:
	case MSM_INFO_AUX:
	case MSM_INFO_TRI:
	case MSM_INFO_TRF:
	case MSM_INFO_TARGET:
		DrawInfoPage(hdc,rc, false);
		break;
	case MSM_TRAFFIC:
		DrawTraffic(hdc,rc);
		break;
	default:
		TextDisplayMode.AsInt = 0;
		TextDisplayMode.AsFlag.Color = TEXTWHITE;
		TextDisplayMode.AsFlag.NoSetFont = 1; 
		TextDisplayMode.AsFlag.AlligneCenter = 1;
		SelectObject(hdc, LK8TargetFont);
		_stprintf(Buffer,TEXT("MapSpaceMode=%d"),MapSpaceMode);
		TextInBox(hdc, Buffer, (rc.right-rc.left)/2, NIBLSCALE(50) , 0, TextDisplayMode, false);
		break;
	}
#ifdef DRAWLKSTATUS
  // no need to clear dodrawlkstatus, it is already reset at each run
  if (dodrawlkstatus) DrawLKStatus(hdc, rc);
#endif
  //SetBkMode(hdc,oldbkmode);
  SelectObject(hdc, oldfont); 
}

void MapWindow::DrawWelcome8000(HDC hdc, RECT rc) {

  SIZE textSize, headerSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];

  short bottomlines;
  short middlex=(rc.right-rc.left)/2;
  //short left=rc.left+NIBLSCALE(5);
  short contenttop=rc.top+NIBLSCALE(50);

  switch (LKevent) {
	case LKEVENT_NONE:
		break;
	case LKEVENT_ENTER:
		// Event are cleared from called inner functions, but we do it nevertheless..
		SetModeType(LKMODE_MAP, MP_MOVING);
		LKevent=LKEVENT_NONE; // check if removable 
		break;
	default:
		LKevent=LKEVENT_NONE;
		break;
  }
  
  SelectObject(hdc, LK8BigFont);
  _stprintf(Buffer,TEXT("LK8000"));
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &headerSize);
  LKWriteText(hdc, Buffer, middlex, (headerSize.cy/2)+NIBLSCALE(2) , 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_SWHITE, false);

  _stprintf(Buffer,gettext(TEXT("_@M904_"))); // Tactical Flight Computer
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &textSize);
  SelectObject(hdc, LK8MediumFont);
  LKWriteText(hdc, Buffer, middlex, (headerSize.cy/2)+(textSize.cy/2)+NIBLSCALE(4)+1 , 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_SWHITE, false);


  //SelectObject(hdc, LK8InfoBigFont);
  SelectObject(hdc, LK8TitleFont);
  _stprintf(Buffer,TEXT("%s v%s.%s"),_T(LKFORK),_T(LKVERSION),_T(LKRELEASE));
  if (SIMMODE) _tcscat(Buffer,_T(" (Simulator)"));
  LKWriteText(hdc, Buffer, middlex, contenttop+(textSize.cy*1) , 0, WTMODE_OUTLINED, WTALIGN_CENTER,RGB_AMBER, false);


  //_stprintf(Buffer,TEXT("Click on center screen to begin")); // REMOVE FIXV2
  _stprintf(Buffer,gettext(TEXT("_@M874_"))); // Click on center screen to begin
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &textSize);
  LKWriteText(hdc, Buffer, middlex, ((rc.bottom-rc.top)-textSize.cy)/2 , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_SWHITE, false);


  SelectObject(hdc, LK8UnitFont);
  if (ScreenSize==0) {
	_stprintf(Buffer,TEXT("**SCREEN %dx%d NOT SUPPORTED**"),rc.right,rc.bottom );
	GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &textSize);
	bottomlines=rc.bottom-BottomSize-(textSize.cy*3);
	LKWriteText(hdc, Buffer, middlex, bottomlines , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITE, false);
	_stprintf(Buffer,TEXT("FONTS WILL NOT BE GOOD OR UNUSABLE"));
	LKWriteText(hdc, Buffer, middlex, bottomlines+textSize.cy , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_SWHITE, false);
  } else {
	_stprintf(Buffer,TEXT("%s build#%d"), XCSoar_Version,BUILDNUMBER);
	GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &textSize);
	bottomlines=rc.bottom-BottomSize-(textSize.cy*3);
	LKWriteText(hdc, Buffer, middlex, bottomlines , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITE, false);
	_stprintf(Buffer,TEXT("HTTP://WWW.LK8000.IT  email:info@lk8000.it"));
	LKWriteText(hdc, Buffer, middlex, bottomlines+textSize.cy , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_SWHITE, false);
 }

  SelectObject(hdc, LK8InfoSmallFont);

  _stprintf(Buffer, _T("%d WPs, %0.1fM free"),NumberOfWayPoints,CheckFreeRam()/1000000.0);
  if (PGNumberOfGates>0) _tcscat(Buffer,_T(" (+Tsk Gates)"));
#ifndef NDEBUG
  _tcscat(Buffer,_T(" (+debug)"));
#endif
#ifdef CPUSTATS
  _tcscat(Buffer,_T(" (+cpustats)"));
#endif
#ifdef DRAWLOAD
  _tcscat(Buffer,_T(" (+drawload)"));
#endif
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &textSize);
  LKWriteText(hdc, Buffer, middlex, bottomlines-(textSize.cy)-NIBLSCALE(2) , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_SWHITE, false);

  _stprintf(Buffer, _T(""));
  if (GPSAltitudeOffset != 0) _stprintf(Buffer, _T("(GpsOffset %+.0f)"), GPSAltitudeOffset/1000*ALTITUDEMODIFY); // 100429 /1000
#ifndef WINDOWSPC
  if (!LoggerGActive()) _tcscat(Buffer,_T(" (No GRecord)"));
#endif
  LKWriteText(hdc, Buffer, middlex, bottomlines-(textSize.cy*2)-NIBLSCALE(2) , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_SWHITE, false);

  if (WarningHomeDir) {
	TCHAR nopath[MAX_PATH];
	LocalPath(nopath,_T(""));
	// LKTOKEN _@M1209_ "CHECK INSTALLATION!"
	MessageBoxX(hWndMapWindow, nopath, gettext(TEXT("_@M1209_")), MB_OK|MB_ICONEXCLAMATION);
	WarningHomeDir=false;
  }
#if ( !defined(WINDOWSPC) || WINDOWSPC==0 )
  static bool checktickcountbug=true; // 100510
  if (checktickcountbug) {
	DWORD counts=GetTickCount();
	if (counts >(unsigned)2073600000l) {
	// LKTOKEN  _@M527_ = "Please exit LK8000 and reset your device.\n" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M527_")),
                TEXT("Device need reset!"),
                MB_OK|MB_ICONEXCLAMATION);
	}
	checktickcountbug=false;
  }
#endif
  static bool checksafetyaltitude=true; // 100709
  if (!ISPARAGLIDER) // 100925
  if (checksafetyaltitude) {
	if (SAFETYALTITUDEARRIVAL<50) {
	// LKTOKEN  _@M155_ = "CHECK safety arrival altitude\n" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M155_")),
                TEXT("Warning!"),
                MB_OK|MB_ICONEXCLAMATION);
	}
	checksafetyaltitude=false;
  }
  return;
}


#ifdef CPUSTATS
void MapWindow::DrawCpuStats(HDC hdc, RECT rc) {

  if (Appearance.InverseInfoBox == true) return;

  TCHAR Buffer[LKSIZEBUFFERLARGE];
  TextInBoxMode_t TextDisplayMode;
  TextDisplayMode.AsInt = 0;
  TextDisplayMode.AsFlag.Color = TEXTWHITE;
  TextDisplayMode.AsFlag.WhiteBorder = 1; // inside a white circle
  TextDisplayMode.AsFlag.Border = 1;      // add a black border to the circle

#if (WINDOWSPC>0)
  wsprintf(Buffer,_T("CPU Draw=%d Calc=%d us"), Cpu_Draw, Cpu_Calc);
#else
  wsprintf(Buffer,_T("CPU Draw=%d Calc=%d ms"), Cpu_Draw, Cpu_Calc);
#endif
  TextInBox(hdc, Buffer, 000, 200 , 0, TextDisplayMode, false);
#if (WINDOWSPC>0)
  wsprintf(Buffer,_T("CPU Inst=%d Port=%d us"), Cpu_Instrument, Cpu_Port);
#else
  wsprintf(Buffer,_T("CPU Inst=%d Port=%d ms"), Cpu_Instrument, Cpu_Port);
#endif
  TextInBox(hdc, Buffer, 000, 240 , 0, TextDisplayMode, false);

  //wsprintf(Buffer,_T("Landsc=%d Geom=%d"), InfoBoxLayout::landscape, InfoBoxLayout::InfoBoxGeometry);
  //TextInBox(hdc, Buffer, 000, 280 , 0, TextDisplayMode, false);
  //wsprintf(Buffer,_T("Recents=%d"), RecentNumber);
  //TextInBox(hdc, Buffer, 000, 280 , 0, TextDisplayMode, false);

}
#endif

#ifdef DRAWDEBUG
void MapWindow::DrawDebug(HDC hdc, RECT rc) {

  TCHAR Buffer[LKSIZEBUFFERLARGE];
  TextInBoxMode_t TextDisplayMode;
  TextDisplayMode.AsInt = 0;
  TextDisplayMode.AsFlag.Color = TEXTWHITE;
  TextDisplayMode.AsFlag.WhiteBorder = 1; // inside a white circle
  TextDisplayMode.AsFlag.Border = 1;      // add a black border to the circle

  wsprintf(Buffer,_T("ModeIndex=%d CURTYPE=%d MSM=%d"), ModeIndex, ModeType[ModeIndex],MapSpaceMode );
  TextInBox(hdc, Buffer, 000, 200 , 0, TextDisplayMode, false);
  wsprintf(Buffer,_T("MTableTop=%d ModeTable=%d=MSM"), ModeTableTop[ModeIndex], ModeTable[ModeIndex][ModeType[ModeIndex]] );
  TextInBox(hdc, Buffer, 000, 240 , 0, TextDisplayMode, false);

}
#endif


void MapWindow::WriteInfo(HDC hdc, bool *showunit, TCHAR *BufferValue, TCHAR *BufferUnit, TCHAR *BufferTitle, 
				short *columnvalue, short *columntitle, short *row1, short *row2, short *row3) {

  static bool doinit=true;
  static short unitrowoffset=0;
  if (doinit) {
	switch(ScreenSize) {
		case ss896x672:
			unitrowoffset=6;
			break;
		case ss800x480:
			unitrowoffset=10;
			break;
		case ss640x480:
			unitrowoffset=5;
			break;
		case ss400x240:
			unitrowoffset=7;
			break;
		case ss480x272:
			unitrowoffset=5;
			break;
		case ss480x234:
			unitrowoffset=3;
			break;
		case ss320x240:
			unitrowoffset=3;
			break;
		// portrait mode
		case ss240x320:
			unitrowoffset=-5;
			break;
		case ss272x480:
			unitrowoffset=-14;
			break;
		case ss480x640:
			unitrowoffset=-8;
			break;
		case ss480x800:
			unitrowoffset=-19;
			break;
		case ss720x408:
			unitrowoffset=8;
			break;
		default:
			break;
	}
	doinit=false;
  }

  SelectObject(hdc, LK8PanelBigFont);
  if (*showunit)
	LKWriteText(hdc, BufferValue, *columnvalue,*row1, 0, WTMODE_NORMAL,WTALIGN_RIGHT, RGB_WHITE, false);
  else
	LKWriteText(hdc, BufferValue, *columnvalue,*row1, 0, WTMODE_NORMAL,WTALIGN_RIGHT, RGB_AMBER, false);

  if (*showunit==true && !HideUnits) {
       	SelectObject(hdc, LK8PanelUnitFont); // 091230
        LKWriteText(hdc, BufferUnit, *columnvalue,*row2+unitrowoffset, 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_WHITE, false);
  }
  SelectObject(hdc, LK8PanelSmallFont);
  LKWriteText(hdc, BufferTitle, *columntitle,*row3, 0, WTMODE_NORMAL, WTALIGN_RIGHT, RGB_LIGHTGREEN, false);

}

// Turn Rate Indicator
void MapWindow::DrawTRI(HDC hDC, const RECT rc)
{
  POINT Start;
  
  static short top=(((rc.bottom-BottomSize-(rc.top+TOPLIMITER)-BOTTOMLIMITER)/PANELROWS)+rc.top+TOPLIMITER)- (rc.top+TOPLIMITER);

  Start.y = ((rc.bottom-BottomSize-top)/2)+top-NIBLSCALE(10);
  Start.x = (rc.right - rc.left)/2;

  HPEN		hpBlack;
  HBRUSH	hbBlack;
  HPEN		hpWhite;
  HBRUSH	hbWhite;
  HPEN		hpBorder;
  HBRUSH	hbBorder;
  HPEN   hpOld;
  HBRUSH hbOld;

  // gauge size radius
  static int radius = NIBLSCALE(65);
  static int planesize = radius-NIBLSCALE(10);
  // planebody
  static int planeradius = NIBLSCALE(6);
  static int tailsize = planesize/4+NIBLSCALE(2);
  static int innerradius = radius - NIBLSCALE(8);
  static POINT d00[2][2],d15[2][4],d30[2][4], d45[2][4], d60[2][4];
  static bool doinit=true;
  TCHAR Buffer[LKSIZEBUFFERVALUE];
  double beta=0.0;
  bool disabled=false;

  if (doinit) {
  // [a][b]  a=0 external circle a=1 inner circle  b=1-4

  d00[0][0].x= Start.x - radius;
  d00[0][0].y= Start.y;
  d00[1][0].x= Start.x - innerradius;
  d00[1][0].y= Start.y;
  d00[0][1].x= Start.x + radius;
  d00[0][1].y= Start.y;
  d00[1][1].x= Start.x + innerradius;
  d00[1][1].y= Start.y;

  d15[0][0].x= Start.x - (long) (radius*fastcosine(15.0));
  d15[0][0].y= Start.y + (long) (radius*fastsine(15.0));
  d15[1][0].x= Start.x - (long) (innerradius*fastcosine(15.0));
  d15[1][0].y= Start.y + (long) (innerradius*fastsine(15.0));
  d15[0][1].x= Start.x - (long) (radius*fastcosine(15.0));
  d15[0][1].y= Start.y - (long) (radius*fastsine(15.0));
  d15[1][1].x= Start.x - (long) (innerradius*fastcosine(15.0));
  d15[1][1].y= Start.y - (long) (innerradius*fastsine(15.0));
  d15[0][2].x= Start.x + (long) (radius*fastcosine(15.0));
  d15[0][2].y= Start.y + (long) (radius*fastsine(15.0));
  d15[1][2].x= Start.x + (long) (innerradius*fastcosine(15.0));
  d15[1][2].y= Start.y + (long) (innerradius*fastsine(15.0));
  d15[0][3].x= Start.x + (long) (radius*fastcosine(15.0));
  d15[0][3].y= Start.y - (long) (radius*fastsine(15.0));
  d15[1][3].x= Start.x + (long) (innerradius*fastcosine(15.0));
  d15[1][3].y= Start.y - (long) (innerradius*fastsine(15.0));

  d30[0][0].x= Start.x - (long) (radius*fastcosine(30.0));
  d30[0][0].y= Start.y + (long) (radius*fastsine(30.0));
  d30[1][0].x= Start.x - (long) (innerradius*fastcosine(30.0));
  d30[1][0].y= Start.y + (long) (innerradius*fastsine(30.0));
  d30[0][1].x= Start.x - (long) (radius*fastcosine(30.0));
  d30[0][1].y= Start.y - (long) (radius*fastsine(30.0));
  d30[1][1].x= Start.x - (long) (innerradius*fastcosine(30.0));
  d30[1][1].y= Start.y - (long) (innerradius*fastsine(30.0));
  d30[0][2].x= Start.x + (long) (radius*fastcosine(30.0));
  d30[0][2].y= Start.y + (long) (radius*fastsine(30.0));
  d30[1][2].x= Start.x + (long) (innerradius*fastcosine(30.0));
  d30[1][2].y= Start.y + (long) (innerradius*fastsine(30.0));
  d30[0][3].x= Start.x + (long) (radius*fastcosine(30.0));
  d30[0][3].y= Start.y - (long) (radius*fastsine(30.0));
  d30[1][3].x= Start.x + (long) (innerradius*fastcosine(30.0));
  d30[1][3].y= Start.y - (long) (innerradius*fastsine(30.0));

  d45[0][0].x= Start.x - (long) (radius*fastcosine(45.0));
  d45[0][0].y= Start.y + (long) (radius*fastsine(45.0));
  d45[1][0].x= Start.x - (long) (innerradius*fastcosine(45.0));
  d45[1][0].y= Start.y + (long) (innerradius*fastsine(45.0));
  d45[0][1].x= Start.x - (long) (radius*fastcosine(45.0));
  d45[0][1].y= Start.y - (long) (radius*fastsine(45.0));
  d45[1][1].x= Start.x - (long) (innerradius*fastcosine(45.0));
  d45[1][1].y= Start.y - (long) (innerradius*fastsine(45.0));
  d45[0][2].x= Start.x + (long) (radius*fastcosine(45.0));
  d45[0][2].y= Start.y + (long) (radius*fastsine(45.0));
  d45[1][2].x= Start.x + (long) (innerradius*fastcosine(45.0));
  d45[1][2].y= Start.y + (long) (innerradius*fastsine(45.0));
  d45[0][3].x= Start.x + (long) (radius*fastcosine(45.0));
  d45[0][3].y= Start.y - (long) (radius*fastsine(45.0));
  d45[1][3].x= Start.x + (long) (innerradius*fastcosine(45.0));
  d45[1][3].y= Start.y - (long) (innerradius*fastsine(45.0));

  d60[0][0].x= Start.x - (long) (radius*fastcosine(60.0));
  d60[0][0].y= Start.y + (long) (radius*fastsine(60.0));
  d60[1][0].x= Start.x - (long) (innerradius*fastcosine(60.0));
  d60[1][0].y= Start.y + (long) (innerradius*fastsine(60.0));
  d60[0][1].x= Start.x - (long) (radius*fastcosine(60.0));
  d60[0][1].y= Start.y - (long) (radius*fastsine(60.0));
  d60[1][1].x= Start.x - (long) (innerradius*fastcosine(60.0));
  d60[1][1].y= Start.y - (long) (innerradius*fastsine(60.0));
  d60[0][2].x= Start.x + (long) (radius*fastcosine(60.0));
  d60[0][2].y= Start.y + (long) (radius*fastsine(60.0));
  d60[1][2].x= Start.x + (long) (innerradius*fastcosine(60.0));
  d60[1][2].y= Start.y + (long) (innerradius*fastsine(60.0));
  d60[0][3].x= Start.x + (long) (radius*fastcosine(60.0));
  d60[0][3].y= Start.y - (long) (radius*fastsine(60.0));
  d60[1][3].x= Start.x + (long) (innerradius*fastcosine(60.0));
  d60[1][3].y= Start.y - (long) (innerradius*fastsine(60.0));

  doinit=false;
  } // end dirty hack doinit

  //if (!CALCULATED_INFO.Flying) {
  // speed is in m/s
  if (GPS_INFO.Speed <5.5) disabled=true; 

  if (disabled) {
	#if LKOBJ
	hpBlack = LKPen_Grey_N1;
	hbBlack = LKBrush_Grey;
	#else
	hpBlack = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_GREY);
	hbBlack = (HBRUSH)CreateSolidBrush(RGB_GREY);
	#endif
  } else {
	#if LKOBJ
	hpBlack = LKPen_Black_N1;
	hbBlack = LKBrush_Black;
	#else
	hpBlack = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_BLACK);
	hbBlack = (HBRUSH)CreateSolidBrush(RGB_BLACK);
	#endif
  	beta = DerivedDrawInfo.BankAngle;
  }
  #if LKOBJ
  hpWhite = LKPen_White_N1;
  hbWhite = LKBrush_White;
  hpBorder = LKPen_Grey_N2;
  hbBorder = LKBrush_Grey;
  #else
  hpWhite = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_WHITE);
  hbWhite = (HBRUSH)CreateSolidBrush( RGB_WHITE);
  hpBorder = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB_GREY);
  hbBorder = (HBRUSH)CreateSolidBrush( RGB_GREY);
  #endif

  hpOld = (HPEN)SelectObject(hDC, hpWhite);
  hbOld = (HBRUSH)SelectObject(hDC, hbWhite);
  Circle(hDC, Start.x, Start.y, radius, rc, false, true );
  SelectObject(hDC, hpBorder);
  SelectObject(hDC, hbBorder);
  Circle(hDC, Start.x, Start.y, radius+NIBLSCALE(2), rc, false, false );

  SelectObject(hDC, hpBlack);
  SelectObject(hDC, hbBlack); 
  Circle(hDC, Start.x, Start.y, planeradius, rc, false, true );

  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d00[0][0], d00[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d00[0][1], d00[1][1], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][0], d15[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][1], d15[1][1], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][2], d15[1][2], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][3], d15[1][3], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][0], d30[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][1], d30[1][1], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][2], d30[1][2], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][3], d30[1][3], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][0], d45[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][1], d45[1][1], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][2], d45[1][2], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][3], d45[1][3], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][0], d60[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][1], d60[1][1], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][2], d60[1][2], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][3], d60[1][3], RGB_BLUE,rc);

  POINT a1, a2;

  a1.x = Start.x - (long) ( planesize * fastcosine(beta));
  a1.y = Start.y - (long) ( planesize * fastsine(beta));
  a2.x = Start.x + (long) ( planesize * fastcosine(beta));
  a2.y = Start.y + (long) ( planesize * fastsine(beta));
    if (disabled) 
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_GREY,rc);
    else
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_BLACK,rc); 

  a1.x = Start.x;
  a1.y = Start.y;
  a2.x = Start.x + (long) ( tailsize * fastsine(beta));
  a2.y = Start.y - (long) ( tailsize * fastcosine(beta));
  if (disabled) 
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_GREY,rc);
  else
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_BLACK,rc);

  TextInBoxMode_t Mode;
  Mode.AsInt=0;
  Mode.AsFlag.Color = TEXTBLUE;
  Mode.AsFlag.NoSetFont = 1;
  Mode.AsFlag.WhiteBold = 0;
  Mode.AsFlag.AlligneRight = 0;
  Mode.AsFlag.AlligneCenter = 1;

  SelectObject(hDC, LK8TitleFont);
  int bankindy=Start.y+radius/2;
#ifndef __MINGW32__
  if (beta > 1)
	_stprintf(Buffer, TEXT("%2.0f\xB0"), beta);
  else if (beta < -1)
	_stprintf(Buffer, TEXT("%2.0f\xB0"), -beta);
  else
	_tcscpy(Buffer, TEXT("--"));
#else
  if (beta > 1)
	_stprintf(Buffer, TEXT("%2.0f°"), beta);
  else if (beta < -1)
	_stprintf(Buffer, TEXT("%2.0f°"), -beta);
  else
	_tcscpy(Buffer, TEXT("--"));
#endif

  LKWriteText(hDC, Buffer, Start.x , bankindy, 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_BLUE, false);

  if (!disabled) MapWindow::RefreshMap();

  SelectObject(hDC, hbOld);
  SelectObject(hDC, hpOld);
  #ifndef LKOBJ
  DeleteObject((HPEN)hpBlack);
  DeleteObject((HBRUSH)hbBlack);
  DeleteObject((HPEN)hpWhite);
  DeleteObject((HBRUSH)hbWhite);
  DeleteObject((HPEN)hpBorder);
  DeleteObject((HBRUSH)hbBorder);
  #endif
}


#ifdef DRAWLKSTATUS
// LK Status message
void MapWindow::DrawLKStatus(HDC hdc, RECT rc) {

  TextInBoxMode_t TextDisplayMode;
  TCHAR Buffer[LKSIZEBUFFERLARGE];

  short bottomlines;
  short middlex=(rc.right-rc.left)/2;
  short left=rc.left+NIBLSCALE(5);
  short contenttop=rc.top+NIBLSCALE(50);

  TextDisplayMode.AsInt = 0;
  TextDisplayMode.AsFlag.Color = TEXTBLACK;
  TextDisplayMode.AsFlag.NoSetFont = 1; 
  //TextDisplayMode.AsFlag.AlligneRight = 0;
  TextDisplayMode.AsFlag.AlligneCenter = 1;
  TextDisplayMode.AsFlag.WhiteBold = 1;
  TextDisplayMode.AsFlag.Border = 1;
  // HFONT oldfont=(HFONT)SelectObject(hdc, LK8PanelBigFont);

  switch(ModeIndex) {
	case LKMODE_MAP:
		wsprintf(Buffer,TEXT("MAP mode, 1 of 1"));
		break;
	case LKMODE_INFOMODE:
		_stprintf(Buffer,TEXT("%d-%d"), ModeIndex,CURTYPE+1);
		break;
	case LKMODE_WP:
		_stprintf(Buffer,TEXT("%d-%d"), ModeIndex,CURTYPE+1);
		break;
	case LKMODE_NAV:
		_stprintf(Buffer,TEXT("%d-%d"), ModeIndex,CURTYPE+1);
		break;
	default:
		wsprintf(Buffer,TEXT("UNKOWN mode"));
		break;
  }
  TextInBox(hdc, Buffer, middlex, 200 , 0, TextDisplayMode, false);

  //SelectObject(hdc, oldfont);
  return;
}
#endif


// invertable is used coped with LKTextBlack: if both are active, then text is forced reversed
void MapWindow::LKWriteText(HDC hDC, const TCHAR* wText, int x, int y, 
                          int maxsize, const bool mode, const short align, COLORREF rgb_text, bool invertable ) {

	SIZE tsize;
	if (maxsize==0) maxsize=_tcslen(wText);
  
	GetTextExtentPoint(hDC, wText, maxsize, &tsize);

	// by default, LK8000 is white on black, i.e. inverted
	if ((!INVERTCOLORS) || (LKTextBlack&&invertable)) switch(rgb_text) { // 091110
		case RGB_WHITE:
			rgb_text=RGB_BLACK;
			break;
		case RGB_BLACK:
			rgb_text=RGB_WHITE;
			break;
		case RGB_SBLACK:		// FIXED MISSING 100511
			rgb_text=RGB_SWHITE;
			break;
		case RGB_LIGHTGREEN:
			// rgb_text=RGB_DARKBLUE; 100915
			rgb_text=RGB_DARKGREEN;
			break;
		case RGB_LIGHTRED:
			rgb_text=RGB_DARKRED;
			break;
		case RGB_LIGHTYELLOW:
			rgb_text=RGB_DARKYELLOW;
			break;
		case RGB_SWHITE:
			rgb_text=RGB_SBLACK;
			break;
		case RGB_AMBER:
			rgb_text=RGB_ORANGE;
			break;
		case RGB_PETROL:
			rgb_text=RGB_ICEWHITE;
			break;
	}

	switch(align) {
		case WTALIGN_RIGHT:
			x -= tsize.cx;
			break;
		case WTALIGN_CENTER:
			x -= tsize.cx/2;
			y -= tsize.cy/2;
			break;
	}
//rgb_text=RGB_MAGENTA;

	switch(mode) {
		case  WTMODE_OUTLINED:
			switch (rgb_text ) {
				// Here we invert colors, looking at the foreground. The trick is that the foreground
				// colour is slightly different white to white, in order to understand how to invert it
				// correctly!
				case RGB_BLACK:
				//case RGB_SWHITE:
					// text black, light background
					SetTextColor(hDC,RGB_WHITE);
					break;
				case RGB_SWHITE:  
					SetTextColor(hDC,RGB_SBLACK);
					break;
				case RGB_SBLACK:
					SetTextColor(hDC,RGB_SWHITE);
					break;
				case RGB_DARKBLUE:
					SetTextColor(hDC,RGB_WHITE);
					break;
				case RGB_GREEN:
					SetTextColor(hDC,RGB_BLACK);
					break;
				case RGB_PETROL:
				case RGB_DARKGREY:
				case RGB_VDARKGREY:
				case RGB_DARKGREEN:
					SetTextColor(hDC,RGB_WHITE);
					break;
				default:
					// this is the default also for white text. Normally we are writing on a 
					// not-too-light background
					SetTextColor(hDC,RGB_BLACK);
					break;
			}
				

#if (WINDOWSPC>0)
			ExtTextOut(hDC, x+1, y, 0, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x+2, y, 0, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x-1, y, 0, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x-2, y, 0, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x, y+1, 0, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x, y-1, 0, NULL, wText, maxsize, NULL);

			if (ScreenSize == (ScreenSize_t)ss800x480) {
				ExtTextOut(hDC, x, y+2, 0, NULL, wText, maxsize, NULL); 
				ExtTextOut(hDC, x, y-2, 0, NULL, wText, maxsize, NULL); 
				ExtTextOut(hDC, x-3, y, 0, NULL, wText, maxsize, NULL); 
				ExtTextOut(hDC, x+3, y, 0, NULL, wText, maxsize, NULL); 
				ExtTextOut(hDC, x, y+3, 0, NULL, wText, maxsize, NULL); 
				ExtTextOut(hDC, x, y-3, 0, NULL, wText, maxsize, NULL); 
			}

			SetTextColor(hDC,rgb_text); 
			ExtTextOut(hDC, x, y, 0, NULL, wText, maxsize, NULL);
			SetTextColor(hDC,RGB_BLACK); 
#else

			ExtTextOut(hDC, x+2, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x+1, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x-1, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x-2, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x, y+1, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			ExtTextOut(hDC, x, y-1, ETO_OPAQUE, NULL, wText, maxsize, NULL);

			if (ScreenSize == (ScreenSize_t)ss800x480) {
				ExtTextOut(hDC, x+3, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
				ExtTextOut(hDC, x-3, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
				ExtTextOut(hDC, x, y+2, ETO_OPAQUE, NULL, wText, maxsize, NULL);
				ExtTextOut(hDC, x, y-2, ETO_OPAQUE, NULL, wText, maxsize, NULL);
				ExtTextOut(hDC, x, y+3, ETO_OPAQUE, NULL, wText, maxsize, NULL);
				ExtTextOut(hDC, x, y-3, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			}

			SetTextColor(hDC,rgb_text);

			ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			SetTextColor(hDC,RGB_BLACK);
#endif
			break;

		case WTMODE_NORMAL:

#if (WINDOWSPC>0)
			SetTextColor(hDC,rgb_text); 
			ExtTextOut(hDC, x, y, 0, NULL, wText, maxsize, NULL);
			SetTextColor(hDC,RGB_BLACK); 
#else
			SetTextColor(hDC,rgb_text); 
      			ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, wText, maxsize, NULL);
			SetTextColor(hDC,RGB_BLACK); 
#endif
			break;

	}

//	SelectObject(hDC, hbOld);

	return;

}


// Sight Traffic graphics
void MapWindow::DrawTarget(HDC hDC, const RECT rc, int ttop, int tbottom, int tleft, int tright)
{

  HPEN   hp, hpOld;
  HBRUSH hb, hbOld;


  static bool doinit=true;
  bool disabled=false,notraffic=false;

  static POINT cross_top, cross_bottom, cross_left, cross_right;
  static POINT degline_top[10];
  static POINT degline_bottom[10];
  static POINT altline_left[6];
  static POINT altline_right[6];


  static int nleft,nright,ntop,nbottom;
  static int ncenterx, ncentery;
  if (doinit) {

	nleft=tleft+IBLSCALE(3);
	nright=tright;
	ntop=ttop;
	nbottom=tbottom;
	ncenterx=((nright-nleft)/2)+nleft;
	ncentery=((nbottom-ntop)/2)+ntop;

	cross_top.x=ncenterx;
	cross_top.y=ntop;
	cross_bottom.x=ncenterx;
	cross_bottom.y=nbottom;
	cross_left.x=nleft;
	cross_left.y=ncentery;
	cross_right.x=nright;
	cross_right.y=ncentery;

	// hoffset is the position of vertical green lines on the horizon, each one representing
	// 20 degrees shift. The [0] and [5] are for 10 to 20 degrees target's offset
	// The 9th line on extreme position is not drawn
	int deg10=(nleft-ncenterx)/9; 
	// height of the vertical line
	int hdeg=(ncentery-ntop)/7;

	degline_top[0].x		=	ncenterx-deg10;
	degline_top[0].y		=	ncentery-hdeg;
	degline_bottom[0].x		=	ncenterx-deg10;
	degline_bottom[0].y		=	ncentery+hdeg;
	// right side
	degline_top[5].x		=	ncenterx+deg10;
	degline_top[5].y		=	ncentery-hdeg;
	degline_bottom[5].x		=	ncenterx+deg10;
	degline_bottom[5].y		=	ncentery+hdeg;

	degline_top[1].x		=	ncenterx-(deg10*2);
	degline_top[1].y		=	ncentery-hdeg;
	degline_bottom[1].x		=	ncenterx-(deg10*2);
	degline_bottom[1].y		=	ncentery+hdeg;
	degline_top[6].x		=	ncenterx+(deg10*2);
	degline_top[6].y		=	ncentery-hdeg;
	degline_bottom[6].x		=	ncenterx+(deg10*2);
	degline_bottom[6].y		=	ncentery+hdeg;
	
	degline_top[2].x		=	ncenterx-(deg10*4);
	degline_top[2].y		=	ncentery-hdeg;
	degline_bottom[2].x		=	ncenterx-(deg10*4);
	degline_bottom[2].y		=	ncentery+hdeg;
	degline_top[7].x		=	ncenterx+(deg10*4);
	degline_top[7].y		=	ncentery-hdeg;
	degline_bottom[7].x		=	ncenterx+(deg10*4);
	degline_bottom[7].y		=	ncentery+hdeg;

	degline_top[3].x		=	ncenterx-(deg10*6);
	degline_top[3].y		=	ncentery-hdeg;
	degline_bottom[3].x		=	ncenterx-(deg10*6);
	degline_bottom[3].y		=	ncentery+hdeg;
	degline_top[8].x		=	ncenterx+(deg10*6);
	degline_top[8].y		=	ncentery-hdeg;
	degline_bottom[8].x		=	ncenterx+(deg10*6);
	degline_bottom[8].y		=	ncentery+hdeg;

	degline_top[4].x		=	ncenterx-(deg10*8);
	degline_top[4].y		=	ncentery-hdeg;
	degline_bottom[4].x		=	ncenterx-(deg10*8);
	degline_bottom[4].y		=	ncentery+hdeg;
	degline_top[9].x		=	ncenterx+(deg10*8);
	degline_top[9].y		=	ncentery-hdeg;
	degline_bottom[9].x		=	ncenterx+(deg10*8);
	degline_bottom[9].y		=	ncentery+hdeg;


	// sizes of horizontal altitudes lines
	int alth=(int)((ncentery-ntop)/3.5);
	int altw=(int)((ncenterx-nleft)/3.5);

	altline_left[0].x		=	ncenterx-altw;
	altline_left[0].y		=	ncentery-alth;
	altline_right[0].x		=	ncenterx+altw;
	altline_right[0].y		=	ncentery-alth;

	altline_left[1].x		=	ncenterx-altw;
	altline_left[1].y		=	ncentery-(alth*2);
	altline_right[1].x		=	ncenterx+altw;
	altline_right[1].y		=	ncentery-(alth*2);

	altline_left[2].x		=	ncenterx-altw;
	altline_left[2].y		=	ncentery-(alth*3);
	altline_right[2].x		=	ncenterx+altw;
	altline_right[2].y		=	ncentery-(alth*3);


	altline_left[3].x		=	ncenterx-altw;
	altline_left[3].y		=	ncentery+alth;
	altline_right[3].x		=	ncenterx+altw;
	altline_right[3].y		=	ncentery+alth;

	altline_left[4].x		=	ncenterx-altw;
	altline_left[4].y		=	ncentery+(alth*2);
	altline_right[4].x		=	ncenterx+altw;
	altline_right[4].y		=	ncentery+(alth*2);

	altline_left[5].x		=	ncenterx-altw;
	altline_left[5].y		=	ncentery+(alth*3);
	altline_right[5].x		=	ncenterx+altw;
	altline_right[5].y		=	ncentery+(alth*3);

	doinit=false;
  } 

  // The flag "disabled" will force no plane to be painted

  // Check target exists, just for safe
  if (LKTargetIndex>=0 && LKTargetIndex<MAXTRAFFIC) {
	if (!GPS_INFO.FLARM_Traffic[LKTargetIndex].Locked) {
		disabled=true;
		notraffic=true;
	}
  } else {
	disabled=true;
	notraffic=true;
  }

  // check visibility +-80 degrees
  double tangle = LKTraffic[LKTargetIndex].Bearing -  GPS_INFO.TrackBearing;
  if (tangle < -180.0) {
	tangle += 360.0;
  } else {
	if (tangle > 180.0)
		tangle -= 360.0;
  }

  if (tangle<-80 || tangle >80) {
	disabled=true;
  }

  COLORREF hscalecol, vscalecol;
  // First we draw the cross sight
  if (disabled) {
	if (notraffic) {
		if (Appearance.InverseInfoBox) {
			_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_left, cross_right, RGB_GREY,rc);
			_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_top, cross_bottom, RGB_GREY,rc);
		} else {
			_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_left, cross_right, RGB_DARKGREY,rc);
			_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_top, cross_bottom, RGB_DARKGREY,rc);
		}
		hscalecol=RGB_DARKGREEN;
		vscalecol=RGB_DARKGREEN;
	} else {
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_left, cross_right, RGB_DARKGREY,rc);
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_top, cross_bottom, RGB_DARKGREY,rc);
		hscalecol=RGB_DARKGREEN;
		vscalecol=RGB_DARKGREEN;
	}
  } else {
	if (Appearance.InverseInfoBox) {
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_left, cross_right, RGB_ICEWHITE,rc);
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_top, cross_bottom, RGB_ICEWHITE,rc);
		hscalecol=RGB_GREEN;
		vscalecol=RGB_GREEN;
	} else {
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_left, cross_right, RGB_DARKGREY,rc);
		_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), cross_top, cross_bottom, RGB_DARKGREY,rc);
		hscalecol=RGB_DARKGREEN;
		vscalecol=RGB_DARKGREEN;
	}
  }


  // Then we draw the scales, degrees on horizontal line
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[0], degline_bottom[0], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[5], degline_bottom[5], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[1], degline_bottom[1], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[6], degline_bottom[6], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[2], degline_bottom[2], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[7], degline_bottom[7], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[3], degline_bottom[3], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[8], degline_bottom[8], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[4], degline_bottom[4], hscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), degline_top[9], degline_bottom[9], hscalecol,rc);
  // altitudes on vertical line
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[0], altline_right[0], vscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[1], altline_right[1], vscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[2], altline_right[2], vscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[3], altline_right[3], vscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[4], altline_right[4], vscalecol,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), altline_left[5], altline_right[5], vscalecol,rc);



  // If out of range, paint diff bearing
  TCHAR tbear[10];
  if (disabled && !notraffic) {
	if (tangle > 1) {
		_stprintf(tbear, TEXT("%2.0f°»"), tangle);
	} else {
		if (tangle < -1) {
			_stprintf(tbear, TEXT("«%2.0f°"), -tangle);
		} else {
			_tcscpy(tbear, TEXT("«»"));
		}
	}
	SelectObject(hDC, LK8PanelBigFont);
	switch ( LKTraffic[LKTargetIndex].Status ) {
		case LKT_GHOST:
			LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTYELLOW, false);
			break;
		case LKT_ZOMBIE:
			LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTRED, false);
			break;
		default:
			LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITE, false);
			break;
	}

	// do not paint bearing, it is confusing
	#if 0
	double tbearing = LKTraffic[LKTargetIndex].Bearing;
	if (tbearing != 360) {
		_stprintf(tbear, TEXT("%2.0f°"), tbearing);
	} else {
		_stprintf(tbear, TEXT("0°"));
	}
	LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITE, false);
	#endif
  }


  // Target wing size, half of it
  #define TWINGSIZE	NIBLSCALE(53)
  POINT tcenter;
  // Paint the airplane only if within 160 deg sight angle
  if (!disabled) {

	// Position of the glider on the sight screen
	int leftwingsize=0, rightwingsize=0;
	COLORREF planecolor;
	int tailsize= (TWINGSIZE/4) +NIBLSCALE(2);

	tcenter.x= (int)(ncenterx+(((ncenterx-nleft)/80)*tangle));

	if ( LKTraffic[LKTargetIndex].AltArriv >300 ) {
		tcenter.y=nbottom;
	} else {
		if ( LKTraffic[LKTargetIndex].AltArriv <-300 ) {
			tcenter.y=ntop+IBLSCALE(5);
			tailsize=IBLSCALE(5);
		} else {
			tcenter.y=ncentery+ (int) (((ncentery-ntop)/300.0)*LKTraffic[LKTargetIndex].AltArriv);
		}
	}

	if (Appearance.InverseInfoBox) {
		switch(LKTraffic[LKTargetIndex].Status) {
			case LKT_GHOST:
				planecolor=RGB_LIGHTYELLOW;
				break;
			case LKT_ZOMBIE:
				planecolor=RGB_LIGHTRED;
				break;
			default:
				planecolor=RGB_WHITE;
				break;
		}
	} else {
		switch(LKTraffic[LKTargetIndex].Status) {
			case LKT_GHOST:
				planecolor=RGB_ORANGE;
				break;
			case LKT_ZOMBIE:
				planecolor=RGB_RED;
				break;
			default:
				planecolor=RGB_BLACK;
				break;
		}
	}

	hp = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), planecolor);
	hb = (HBRUSH)CreateSolidBrush( planecolor);
	hpOld = (HPEN) SelectObject(hDC, hp);
	hbOld = (HBRUSH) SelectObject(hDC, hb);

	// does the glider exceed screen space on the right?
	if (tangle>1) {
		leftwingsize=TWINGSIZE;
		// if right wing is exceeding space, reduce it
		if ( (tcenter.x+TWINGSIZE) >nright ) {
			rightwingsize=  nright-tcenter.x;
		} else
			rightwingsize=TWINGSIZE;
	}

	// Now check the left wing
	if (tangle<1) {
		rightwingsize=TWINGSIZE;
		if ( (tcenter.x-TWINGSIZE) < nleft ) {
			leftwingsize=  tcenter.x-nleft;
		} else
			leftwingsize=TWINGSIZE;
	}

	if (tangle==0) {
		rightwingsize=TWINGSIZE;
		leftwingsize=TWINGSIZE;
	}

	Circle(hDC, tcenter.x, tcenter.y, NIBLSCALE(6), rc, false, true );

	POINT a1, a2;

	// Draw the wing
	a1.x = tcenter.x - leftwingsize;
	a1.y = tcenter.y;
	a2.x = tcenter.x + rightwingsize;
	a2.y = tcenter.y;
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, planecolor,rc);
	// Draw the tail
	a1.x = tcenter.x;
	a1.y = tcenter.y;
	a2.x = tcenter.x;
	a2.y = tcenter.y - tailsize;
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, planecolor,rc);

	SelectObject(hDC, hbOld);
	SelectObject(hDC, hpOld);
	DeleteObject((HPEN)hp);
	DeleteObject((HBRUSH)hb);
  }

  // always paint the bearing difference, cleverly
  if (!disabled && !notraffic) {
	if (tangle > 1) {
		_stprintf(tbear, TEXT("%2.0f°»"), tangle);
	} else {
		if (tangle < -1) {
			_stprintf(tbear, TEXT("«%2.0f°"), -tangle);
		} else {
			_tcscpy(tbear, TEXT("«»"));
		}
	}
	SelectObject(hDC, LK8PanelBigFont);
	// if target is below middle line, paint on top
	int yposbear;
	if (tcenter.y >= ncentery ) {
		yposbear=altline_left[2].y;
	} else
		yposbear=altline_left[5].y;

//	LKWriteText(hDC, tbear, ncenterx,altline_left[5].y, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITE, false);
	switch ( LKTraffic[LKTargetIndex].Status ) {
		case LKT_GHOST:
			//LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTYELLOW, false);
			LKWriteText(hDC, tbear, ncenterx,yposbear, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTYELLOW, false);
			break;
		case LKT_ZOMBIE:
			//LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTRED, false);
			LKWriteText(hDC, tbear, ncenterx,yposbear, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTRED, false);
			break;
		default:
			//LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITE, false);
			LKWriteText(hDC, tbear, ncenterx,yposbear, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITE, false);
			break;
	}
  }

}
