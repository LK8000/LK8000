/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindow.cpp,v 8.28 2010/12/12 13:50:25 root Exp root $
*/

#include "StdAfx.h"
#include "compatibility.h"
#include "Defines.h"
#include "LKUtils.h"
#include "options.h"
#include "Cpustats.h"
#include "MapWindow.h"
#include "LKMapWindow.h"
#include "OnLineContest.h"
#include "Utils.h"
#include "Units.h"
#include "Logger.h"
#include "McReady.h"
#include "Airspace.h"
#include "Waypointparser.h"
#include "Dialogs.h"
#include "externs.h"
#include "VarioSound.h"
#include "InputEvents.h"
// #include <assert.h>
#include <windows.h>
#include <math.h>
#include <Message.h> // 091112

#include <tchar.h>

#include "Terrain.h"
#include "Task.h"
#include "AATDistance.h"
#include "LKObjects.h"

#ifndef NOVARIOGAUGE
#include "GaugeVarioAltA.h"
#endif
#ifndef NOCDIGAUGE
#include "GaugeCDI.h"
#endif
#ifndef NOFLARMGAUGE
#include "GaugeFLARM.h"
#endif
#include "InfoBoxLayout.h"
#include "RasterTerrain.h"
#include "Utils2.h"
#include "externs.h" // 091110

#if (WINDOWSPC>0)
#include <wingdi.h>
#endif

#include "LKGeneralAviation.h"


#ifdef GTCFIX
DWORD misc_tick_count=0;
#else
int misc_tick_count=0;
#endif

#ifdef DEBUG
#define DRAWLOAD
#define DEBUG_VIRTUALKEYS
#endif

#define INVERTCOLORS  (Appearance.InverseInfoBox)


int TrailActive = TRUE;
int VisualGlide = 0;

extern void DrawGlideCircle(HDC hdc, POINT Orig, RECT rc );
#ifdef CPUSTATS
extern void DrawCpuStats(HDC hdc, RECT rc );
#endif
#ifdef DRAWDEBUG
extern void DrawDebug(HDC hdc, RECT rc );
#endif

#define NUMSNAILRAMP 6

#ifdef LKPMODE
#define DONTDRAWTHEMAP NewMap&&Look8000&&!EnablePan&&MapSpaceMode!=MSM_MAP
#define MAPMODE8000    IsMapFullScreen()&&NewMap&&Look8000&&!EnablePan&&MapSpaceMode==MSM_MAP
#else
#define DONTDRAWTHEMAP IsMapFullScreen()&&NewMap&&Look8000&&!EnablePan&&MapSpaceMode!=MSM_MAP
#define MAPMODE8000    IsMapFullScreen()&&NewMap&&Look8000&&!EnablePan&&MapSpaceMode==MSM_MAP
#endif

//#define ISPARAGLIDER (AircraftCategory == (AircraftCategory_t)umParaglider) REMOVE

#ifdef OVERTARGET
extern int GetOvertargetIndex(void);
#endif

static const COLORREF taskcolor = RGB_TASKLINECOL; // 091216
static bool ignorenext=false;

const COLORRAMP snail_colors[] = {
  {0,         0xff, 0x3e, 0x00},
  {50,        0xcd, 0x4f, 0x27},
  {100,       0x8f, 0x8f, 0x8f},
  {150,       0x27, 0xcd, 0x4f},
  {201,       0x00, 0xff, 0x3e},
  {501,       0x00, 0xff, 0x3e}
};


DisplayMode_t UserForceDisplayMode = dmNone;
DisplayMode_t DisplayMode = dmCruise;

HBITMAP MapWindow::hBmpAirportReachable;
HBITMAP MapWindow::hBmpAirportUnReachable;
HBITMAP MapWindow::hBmpFieldReachable;
HBITMAP MapWindow::hBmpFieldUnReachable;
HBITMAP MapWindow::hBmpThermalSource;
HBITMAP MapWindow::hBmpTarget;
HBITMAP MapWindow::hBmpTeammatePosition;
HBITMAP MapWindow::hAboveTerrainBitmap;
HBRUSH  MapWindow::hAboveTerrainBrush;

HPEN    MapWindow::hpCompassBorder;
HBRUSH  MapWindow::hBrushFlyingModeAbort;
int MapWindow::SnailWidthScale = 16;

HBITMAP MapWindow::hBmpUnitKm;
HBITMAP MapWindow::hBmpUnitSm;
HBITMAP MapWindow::hBmpUnitNm;
HBITMAP MapWindow::hBmpUnitM;
HBITMAP MapWindow::hBmpUnitFt;
HBITMAP MapWindow::hBmpUnitMpS;

int MapWindow::ScaleListCount = 0;
double MapWindow::ScaleList[];
int MapWindow::ScaleCurrent;
HBITMAP MapWindow::hBmpCompassBg = NULL;
HBITMAP MapWindow::hBmpClimbeAbort = NULL;
HBITMAP MapWindow::hBmpMapScale=NULL;

POINT MapWindow::Orig_Screen;

RECT MapWindow::MapRect;
RECT MapWindow::MapRectBig;
RECT MapWindow::MapRectSmall;

HBITMAP MapWindow::hDrawBitMap = NULL;
HBITMAP MapWindow::hDrawBitMapTmp = NULL;
HBITMAP MapWindow::hMaskBitMap = NULL;
HDC MapWindow::hdcDrawWindow = NULL;
HDC MapWindow::hdcScreen = NULL;
HDC MapWindow::hDCTemp = NULL;
HDC MapWindow::hDCMask = NULL;

rectObj MapWindow::screenbounds_latlon;

double MapWindow::PanLatitude = 0.0;
double MapWindow::PanLongitude = 0.0;

int MapWindow::TargetDrag_State = 0;
double MapWindow::TargetDrag_Latitude = 0;
double MapWindow::TargetDrag_Longitude = 0;

bool MapWindow::EnablePan = false;
bool MapWindow::TargetPan = false;
int MapWindow::TargetPanIndex = 0;
double MapWindow::TargetZoomDistance = 500.0;
bool MapWindow::EnableTrailDrift=false;
int MapWindow::GliderScreenPosition = 40; // 20% from bottom
int MapWindow::GliderScreenPositionX = 50;  // 100216
int MapWindow::GliderScreenPositionY = 40;
int MapWindow::WindArrowStyle = 0;

BOOL MapWindow::CLOSETHREAD = FALSE;
BOOL MapWindow::THREADRUNNING = TRUE;
BOOL MapWindow::THREADEXIT = FALSE;
BOOL MapWindow::Initialised = FALSE;

bool MapWindow::BigZoom = true;
unsigned char MapWindow::DeclutterLabels = MAPLABELS_ALLON;

DWORD  MapWindow::dwDrawThreadID;
HANDLE MapWindow::hDrawThread;

double MapWindow::RequestMapScale; // VENTA9 = 5; 
double MapWindow::MapScale; // VENTA9 = 5; 
double MapWindow::MapScaleOverDistanceModify; // VENTA9  = 5/DISTANCEMODIFY;
double MapWindow::ResMapScaleOverDistanceModify = 0.0;
double MapWindow::DisplayAngle = 0.0;
double MapWindow::DisplayAircraftAngle = 0.0;
double MapWindow::DrawScale;
double MapWindow::InvDrawScale;

bool MapWindow::AutoZoom = false;
bool MapWindow::LandableReachable = false;

HBITMAP MapWindow::hTurnPoint;
HBITMAP MapWindow::hInvTurnPoint;
HBITMAP MapWindow::hSmall;
HBITMAP MapWindow::hInvSmall;
HBITMAP MapWindow::hCruise;
HBITMAP MapWindow::hClimb;
HBITMAP MapWindow::hFinalGlide;
HBITMAP MapWindow::hAutoMacCready;
HBITMAP MapWindow::hTerrainWarning;
HBITMAP MapWindow::hFLARMTraffic;
HBITMAP MapWindow::hGPSStatus1;
HBITMAP MapWindow::hGPSStatus2;
HBITMAP MapWindow::hAbort;
HBITMAP MapWindow::hLogger;
HBITMAP MapWindow::hLoggerOff;

HPEN MapWindow::hSnailPens[NUMSNAILCOLORS];
COLORREF MapWindow::hSnailColours[NUMSNAILCOLORS];

POINT MapWindow::Groundline[NUMTERRAINSWEEPS+1];

// 12 is number of airspace types
int      MapWindow::iAirspaceBrush[AIRSPACECLASSCOUNT] = 
  {2,0,0,0,3,3,3,3,0,3,2,3,3,3};
int      MapWindow::iAirspaceColour[AIRSPACECLASSCOUNT] = 
  {5,0,0,10,0,0,10,2,0,10,9,3,7,7};
int      MapWindow::iAirspaceMode[AIRSPACECLASSCOUNT] =
  {0,0,0,0,0,0,0,0,0,0,0,1,1,0};

HPEN MapWindow::hAirspacePens[AIRSPACECLASSCOUNT];
bool MapWindow::bAirspaceBlackOutline = false;

HBRUSH  MapWindow::hBackgroundBrush;
HBRUSH  MapWindow::hInvBackgroundBrush[LKMAXBACKGROUNDS];

HBRUSH  MapWindow::hAirspaceBrushes[NUMAIRSPACEBRUSHES];
HBITMAP MapWindow::hAirspaceBitmap[NUMAIRSPACEBRUSHES];

COLORREF MapWindow::Colours[NUMAIRSPACECOLORS] =
  {RGB(0xFF,0x00,0x00), RGB(0x00,0xFF,0x00),
   RGB(0x00,0x00,0xFF), RGB(0xFF,0xFF,0x00),
   RGB(0xFF,0x00,0xFF), RGB(0x00,0xFF,0xFF),
   RGB(0x7F,0x00,0x00), RGB(0x00,0x7F,0x00),
   RGB(0x00,0x00,0x7F), RGB(0x7F,0x7F,0x00),
   RGB(0x7F,0x00,0x7F), RGB(0x00,0x7F,0x7F),
   RGB(0xFF,0xFF,0xFF), RGB(0xC0,0xC0,0xC0),
   RGB(0x7F,0x7F,0x7F), RGB(0x00,0x00,0x00)};


HBRUSH MapWindow::hbCompass;
HBRUSH MapWindow::hbThermalBand;
HBRUSH MapWindow::hbBestCruiseTrack;
HBRUSH MapWindow::hbFinalGlideBelow;
HBRUSH MapWindow::hbFinalGlideBelowLandable;
HBRUSH MapWindow::hbFinalGlideAbove;
HBRUSH MapWindow::hbWind;


HPEN MapWindow::hpAircraft;
HPEN MapWindow::hpAircraftBorder;
HPEN MapWindow::hpWind;
HPEN MapWindow::hpWindThick;
HPEN MapWindow::hpBearing;
HPEN MapWindow::hpBestCruiseTrack;
HPEN MapWindow::hpCompass;

HPEN MapWindow::hpThermalCircle;
/*
#if OVERTARGET
HPEN MapWindow::hpOvertarget;
#endif
*/
HPEN MapWindow::hpThermalBand;
HPEN MapWindow::hpThermalBandGlider;
HPEN MapWindow::hpFinalGlideAbove;
HPEN MapWindow::hpFinalGlideBelow;
HPEN MapWindow::hpFinalGlideBelowLandable;
HPEN MapWindow::hpMapScale;
HPEN MapWindow::hpMapScale2;
HPEN MapWindow::hpTerrainLine;
HPEN MapWindow::hpTerrainLineBg;
HPEN MapWindow::hpSpeedSlow;
HPEN MapWindow::hpSpeedFast;
HPEN MapWindow::hpStartFinishThick;
HPEN MapWindow::hpStartFinishThin;
HPEN MapWindow::hpVisualGlideLightBlack; // VENTA3
HPEN MapWindow::hpVisualGlideHeavyBlack; // VENTA3
HPEN MapWindow::hpVisualGlideLightRed; // VENTA3
HPEN MapWindow::hpVisualGlideHeavyRed; // VENTA3

  
COLORREF MapWindow::BackgroundColor = RGB_WHITE;

bool MapWindow::MapDirty = true;
DWORD MapWindow::fpsTime0 = 0;
bool MapWindow::MapFullScreen = false;
bool MapWindow::RequestFullScreen = false;
bool MapWindow::ForceVisibilityScan = false;


extern int DisplayTimeOut;

NMEA_INFO MapWindow::DrawInfo;
DERIVED_INFO MapWindow::DerivedDrawInfo;

int SelectedWaypoint = -1;
bool EnableCDICruise = false;
bool EnableCDICircling = false;

extern HWND hWndCDIWindow;
extern int iround(double i);
extern void ShowMenu();

extern HFONT  TitleWindowFont;
extern HFONT  MapWindowFont;
extern HFONT  MapWindowBoldFont;
extern HFONT  InfoWindowFont;
extern HFONT  CDIWindowFont;
extern HFONT  StatisticsFont;
extern HFONT  MapLabelFont; // VENTA6
extern HFONT  TitleSmallWindowFont; // VENTA6



#ifdef DRAWLOAD
#ifdef GTCFIX
DWORD timestats_av = 0;
#else
int timestats_av = 0;
#endif
#endif

DWORD MapWindow::timestamp_newdata=0;
//#ifdef (DEBUG_MEM) 100211
#if defined DRAWLOAD || defined DEBUG_MEM
int cpuload=0;
#endif

bool timestats_dirty=false;

void MapWindow::UpdateTimeStats(bool start) {
#ifdef DRAWLOAD
  #ifdef GTCFIX
  static DWORD tottime=0;
  #else
  static long tottime=0;
  #endif
#endif
  if (start) {
    timestamp_newdata = ::GetTickCount();
    timestats_dirty = false;
  } else {
#ifdef DRAWLOAD
    if (!timestats_dirty) {
      DWORD time = ::GetTickCount();
      tottime = (2*tottime+(time-timestamp_newdata))/3;
      timestats_av = tottime;
      cpuload=0;
#ifdef DEBUG_MEM
      cpuload= MeasureCPULoad();
      DebugStore("%d # mem\n%d # latency\n", CheckFreeRam()/1024, timestats_av);
#endif
    }
#endif
    timestats_dirty = false;
  }
}



bool MapWindow::Event_NearestWaypointDetails(double lon, double lat, 
                                             double range,
                                             bool pan) {
  /*
    if (!pan) {
    dlgWayPointSelect(lon, lat, 0, 1);
    } else {
    dlgWayPointSelect(PanLongitude, PanLatitude, 0, 1);
    }
  */

  int i;
  if (!pan || !EnablePan) {
    i=FindNearestWayPoint(lon, lat, range);
  } else {
    // nearest to center of screen if in pan mode
    i=FindNearestWayPoint(PanLongitude, PanLatitude, range);
  }
  if(i != -1)
    {
      SelectedWaypoint = i;
      PopupWaypointDetails();
      return true;
    }

  return false;
}


bool MapWindow::Event_InteriorAirspaceDetails(double lon, double lat) {
  unsigned int i;
  bool found=false;
  bool inside;

  if (AirspaceCircle) {
    for (i=0; i<NumberOfAirspaceCircles; i++) {
      inside = false;
      if (AirspaceCircle[i].Visible) {
        inside = InsideAirspaceCircle(lon, lat, i);
      }
      if (inside) {
	dlgAirspaceDetails(i, -1);

	/*
	  DisplayAirspaceWarning(AirspaceCircle[i].Type , 
	  AirspaceCircle[i].Name , 
	  AirspaceCircle[i].Base, 
	  AirspaceCircle[i].Top );
	*/
        found = true;
      }
    }
  }
  if (AirspaceArea) {
    for (i=0; i<NumberOfAirspaceAreas; i++) {
      inside = false;
      if (AirspaceArea[i].Visible) {
        inside = InsideAirspaceArea(lon, lat, i);
      }
      if (inside) {
	dlgAirspaceDetails(-1, i);

	/*
	  DisplayAirspaceWarning(AirspaceArea[i].Type , 
	  AirspaceArea[i].Name , 
	  AirspaceArea[i].Base, 
	  AirspaceArea[i].Top );
	*/
        found = true;
      }
    }
  }

  return found; // nothing found..
}


void MapWindow::SwitchZoomClimb(void) {

  static bool doinit=true;
  static bool last_isclimb = false;
  static bool last_targetpan = false;

  bool isclimb = (DisplayMode == dmCircling);

  if (doinit) {
	SetMapScales();
	doinit=false;
  }

  if (TargetPan != last_targetpan) {
    if (TargetPan) {
      // save starting values
      if (isclimb) {
        ClimbMapScale = MapScale;
      } else {
        CruiseMapScale = MapScale;
      }
    } else {
      // restore scales
      if (isclimb) {
        RequestMapScale = LimitMapScale(ClimbMapScale);
      } else {
        RequestMapScale = LimitMapScale(CruiseMapScale);
      }
      BigZoom = true;
    }
    last_targetpan = TargetPan;
    return;
  }
  if (!TargetPan && CircleZoom) {
    if (isclimb != last_isclimb) {
      if (isclimb) {
        // save cruise scale
        CruiseMapScale = MapScale;
        // switch to climb scale
        RequestMapScale = LimitMapScale(ClimbMapScale);
      } else {
        // leaving climb
        // save cruise scale
        ClimbMapScale = MapScale;
        RequestMapScale = LimitMapScale(CruiseMapScale);
        // switch to climb scale
      }
      BigZoom = true;
      last_isclimb = isclimb;
    } else {
      // nothing to do.
    }
  }

}

bool MapWindow::isAutoZoom() {
  return AutoZoom;
}

bool TextInBoxMoveInView(POINT *offset, RECT *brect){

  bool res = false;

  int LabelMargin = 4;

  offset->x = 0;
  offset->y = 0;

  if (MapWindow::MapRect.top > brect->top){
    int d = MapWindow::MapRect.top - brect->top;
    brect->top += d;
    brect->bottom += d;
    offset->y += d;
    brect->bottom -= d;
    brect->left -= d;
    offset->x -= d;
    res = true;
  }

  if (MapWindow::MapRect.right < brect->right){
    int d = MapWindow::MapRect.right - brect->right;

    if (offset->y < LabelMargin){
      int dy;

      if (d > -LabelMargin){
        dy = LabelMargin-offset->y;
        if (d > -dy)
          dy = -d;
      } else {
        int x = d + (brect->right - brect->left) + 10;

        dy = x - offset->y;

        if (dy < 0)
          dy = 0;

        if (dy > LabelMargin)
          dy = LabelMargin;
      }

      brect->top += dy;
      brect->bottom += dy;
      offset->y += dy;

    }

    brect->right += d;
    brect->left += d;
    offset->x += d;

    res = true;
  }

  if (MapWindow::MapRect.bottom < brect->bottom){
    if (offset->x == 0){
      int d = MapWindow::MapRect.bottom - brect->bottom;
      brect->top += d;
      brect->bottom += d;
      offset->y += d;
    } else
      if (offset->x < -LabelMargin){
	int d = -(brect->bottom - brect->top) - 10;
	brect->top += d;
	brect->bottom += d;
	offset->y += d;
      } else {
	int d = -(2*offset->x + (brect->bottom - brect->top));
	brect->top += d;
	brect->bottom += d;
	offset->y += d;
      }

    res = true;
  }

  if (MapWindow::MapRect.left > brect->left){
    int d = MapWindow::MapRect.left - brect->left;
    brect->right+= d;
    brect->left += d;
    offset->x += d;
    res = true;
  }

  return(res);

}

// VENTA5 now returns true if really wrote something 
bool MapWindow::TextInBox(HDC hDC, TCHAR* Value, int x, int y, 
                          int size, TextInBoxMode_t Mode, bool noOverlap) {

#define WPCIRCLESIZE        2

  SIZE tsize;
  RECT brect;
  HFONT oldFont=0;
  POINT org;
  bool drawn=false;

  if ((x<MapRect.left-WPCIRCLESIZE) || 
      (x>MapRect.right+(WPCIRCLESIZE*3)) || 
      (y<MapRect.top-WPCIRCLESIZE) ||
      (y>MapRect.bottom+WPCIRCLESIZE)) {
    return drawn; // FIX Not drawn really
  }

  org.x = x;
  org.y = y;

  if (size==0) {
    size = _tcslen(Value);
  }
  
  HBRUSH hbOld;
  hbOld = (HBRUSH)SelectObject(hDC, GetStockObject(WHITE_BRUSH));

  if (Mode.AsFlag.Reachable){
    if (Appearance.IndLandable == wpLandableDefault){
      x += 5;  // make space for the green circle
    }else
      if (Appearance.IndLandable == wpLandableAltA){
	x += 0;
      }
  }

  // landable waypoint label inside white box 
  if (!Mode.AsFlag.NoSetFont) {  // VENTA5 predefined font from calling function
    if (Mode.AsFlag.Border){
      oldFont = (HFONT)SelectObject(hDC, MapWindowBoldFont);
    } else {
      oldFont = (HFONT)SelectObject(hDC, MapWindowFont);
    }
  }
  
  GetTextExtentPoint(hDC, Value, size, &tsize);

  if (Mode.AsFlag.AlligneRight){
    x -= tsize.cx;
  } else 
    if (Mode.AsFlag.AlligneCenter){
      x -= tsize.cx/2;
      y -= tsize.cy/2;
    }

  bool notoverlapping = true;

  if (Mode.AsFlag.Border || Mode.AsFlag.WhiteBorder){

    POINT offset;

    brect.left = x-2;
    brect.right = brect.left+tsize.cx+4;
    brect.top = y+((tsize.cy+4)>>3)-2;
    brect.bottom = brect.top+3+tsize.cy-((tsize.cy+4)>>3);

    if (Mode.AsFlag.AlligneRight)
      x -= 3;

    if (TextInBoxMoveInView(&offset, &brect)){
      x += offset.x;
      y += offset.y;
    }

	#if TOPOFASTLABEL
	notoverlapping = checkLabelBlock(&brect); 
	#else
    notoverlapping = checkLabelBlock(brect); 
	#endif

  
    if (!noOverlap || notoverlapping) {
      HPEN oldPen;
      if (Mode.AsFlag.Border) {
        oldPen = (HPEN)SelectObject(hDC, hpMapScale);
      } else {
        oldPen = (HPEN)SelectObject(hDC, GetStockObject(WHITE_PEN));
      }
      RoundRect(hDC, brect.left, brect.top, brect.right, brect.bottom, 
                NIBLSCALE(8), NIBLSCALE(8));
      SelectObject(hDC, oldPen);
#if (WINDOWSPC>0)
      SetBkMode(hDC,TRANSPARENT);
      ExtTextOut(hDC, x, y, 0, NULL, Value, size, NULL);
#else
      ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, Value, size, NULL);
#endif
      drawn=true;
    }


  } else if (Mode.AsFlag.FillBackground) {

    POINT offset;

    brect.left = x-1;
    brect.right = brect.left+tsize.cx+1;  
    brect.top = y+((tsize.cy+4)>>3);
    brect.bottom = brect.top+tsize.cy-((tsize.cy+4)>>3);

    if (Mode.AsFlag.AlligneRight)
      x -= 2;

    if (TextInBoxMoveInView(&offset, &brect)){
      x += offset.x;
      y += offset.y;
    }

	#if TOPOFASTLABEL
	notoverlapping = checkLabelBlock(&brect); 
	#else
    notoverlapping = checkLabelBlock(brect); 
	#endif
  
    if (!noOverlap || notoverlapping) {
      COLORREF oldColor = SetBkColor(hDC, RGB_WHITE);
      ExtTextOut(hDC, x, y, ETO_OPAQUE, &brect, Value, size, NULL);
      SetBkColor(hDC, oldColor);
      drawn=true;
    }

  } else if (Mode.AsFlag.WhiteBold) {

    switch (DeclutterMode) {
	// This is duplicated later on!
	case (DeclutterMode_t)dmVeryHigh:
	    brect.left = x-NIBLSCALE(10);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(10);
	    brect.top = y+((tsize.cy+NIBLSCALE(12))>>3)-NIBLSCALE(12);
	    brect.bottom = brect.top+NIBLSCALE(12)+tsize.cy-((tsize.cy+NIBLSCALE(12))>>3);
	    break;
	case (DeclutterMode_t)dmHigh:
	    brect.left = x-NIBLSCALE(5);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(5);
	    brect.top = y+((tsize.cy+NIBLSCALE(6))>>3)-NIBLSCALE(6);
	    brect.bottom = brect.top+NIBLSCALE(6)+tsize.cy-((tsize.cy+NIBLSCALE(6))>>3);
	    break;
	case (DeclutterMode_t)dmMedium:
	    brect.left = x-NIBLSCALE(2);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(3);
	    brect.top = y+((tsize.cy+NIBLSCALE(3))>>3)-NIBLSCALE(3);
	    brect.bottom = brect.top+NIBLSCALE(3)+tsize.cy-((tsize.cy+NIBLSCALE(3))>>3);
	    break;
	case (DeclutterMode_t)dmLow:
	case (DeclutterMode_t)dmDisabled: // BUGFIX 100909
	    brect.left = x;
	    brect.right = brect.left+tsize.cx;
	    brect.top = y+((tsize.cy)>>3);
	    brect.bottom = brect.top+tsize.cy-((tsize.cy)>>3);
	    break;
	default:
	    break;

    }

	#if TOPOFASTLABEL
	notoverlapping = checkLabelBlock(&brect); 
	#else
    notoverlapping = checkLabelBlock(brect); 
	#endif
  
    if (!noOverlap || notoverlapping) { 
      if (NewMap&&OutlinedTp)
	SetTextColor(hDC,RGB_BLACK);
      else
	SetTextColor(hDC,RGB_WHITE); 

//#if (WINDOWSPC>0) 091115 do not use custom things for PC in textinbox
#if (0)
      SetBkMode(hDC,TRANSPARENT);
      ExtTextOut(hDC, x+1, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x+2, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-1, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-2, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y+1, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y-1, 0, NULL, Value, size, NULL);
      if (NewMap&&OutlinedTp) {
	if (ScreenSize == (ScreenSize_t)ss800x480) {
		ExtTextOut(hDC, x, y+2, 0, NULL, Value, size, NULL); 
		ExtTextOut(hDC, x, y-2, 0, NULL, Value, size, NULL); 
		ExtTextOut(hDC, x-3, y, 0, NULL, Value, size, NULL); 
		ExtTextOut(hDC, x+3, y, 0, NULL, Value, size, NULL); 
		ExtTextOut(hDC, x, y+3, 0, NULL, Value, size, NULL); 
		ExtTextOut(hDC, x, y-3, 0, NULL, Value, size, NULL); 
	}
	TextColor(hDC,Mode.AsFlag.Color);
      } else
	SetTextColor(hDC,RGB_BLACK); 

      ExtTextOut(hDC, x, y, 0, NULL, Value, size, NULL);
      if (NewMap&&OutlinedTp)
	SetTextColor(hDC,RGB_BLACK); // TODO somewhere else text color is not set correctly

#else
      ExtTextOut(hDC, x+2, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x+1, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-1, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-2, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y+1, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y-1, ETO_OPAQUE, NULL, Value, size, NULL);
//#ifdef PNA 091115 no more big outlining for 314
#if (0)
	// On 800x480 resolution the following additional outlining is very nice.
	// But on 640x480 and lower resolutions it is bad.. Maybe we should work
	// something out for 640x480 devices in landscape mode, when there's nothing else to do. --paolo
      if (NewMap&&OutlinedTp) {
      	if (GlobalModelType == MODELTYPE_PNA_HP31X ) {
	      ExtTextOut(hDC, x+3, y, ETO_OPAQUE, NULL, Value, size, NULL);
	      ExtTextOut(hDC, x-3, y, ETO_OPAQUE, NULL, Value, size, NULL);
	      ExtTextOut(hDC, x, y+2, ETO_OPAQUE, NULL, Value, size, NULL);
	      ExtTextOut(hDC, x, y-2, ETO_OPAQUE, NULL, Value, size, NULL);
	      ExtTextOut(hDC, x, y+3, ETO_OPAQUE, NULL, Value, size, NULL);
	      ExtTextOut(hDC, x, y-3, ETO_OPAQUE, NULL, Value, size, NULL);
      	}
      }
#endif
      if (NewMap&&OutlinedTp) {
	TextColor(hDC,Mode.AsFlag.Color);
      } else
	SetTextColor(hDC,RGB_BLACK); 

      ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, Value, size, NULL);
      if (NewMap&&OutlinedTp)
	SetTextColor(hDC,RGB_BLACK); // TODO somewhere else text color is not set correctly
#endif
      drawn=true;
    }

  } else {

    switch (DeclutterMode) {
	// This is duplicated before!
	case (DeclutterMode_t)dmVeryHigh:
	    brect.left = x-NIBLSCALE(10);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(10);
	    brect.top = y+((tsize.cy+NIBLSCALE(12))>>3)-NIBLSCALE(12);
	    brect.bottom = brect.top+NIBLSCALE(12)+tsize.cy-((tsize.cy+NIBLSCALE(12))>>3);
	    break;
	case (DeclutterMode_t)dmHigh:
	    brect.left = x-NIBLSCALE(5);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(5);
	    brect.top = y+((tsize.cy+NIBLSCALE(6))>>3)-NIBLSCALE(6);
	    brect.bottom = brect.top+NIBLSCALE(6)+tsize.cy-((tsize.cy+NIBLSCALE(6))>>3);
	    break;
	case (DeclutterMode_t)dmMedium:
	    brect.left = x-NIBLSCALE(2);
	    brect.right = brect.left+tsize.cx+NIBLSCALE(3);
	    brect.top = y+((tsize.cy+NIBLSCALE(3))>>3)-NIBLSCALE(3);
	    brect.bottom = brect.top+NIBLSCALE(3)+tsize.cy-((tsize.cy+NIBLSCALE(3))>>3);
	    break;
	case (DeclutterMode_t)dmLow:
	case (DeclutterMode_t)dmDisabled: // BUGFIX 100909
	    brect.left = x;
	    brect.right = brect.left+tsize.cx;
	    brect.top = y+((tsize.cy)>>3);
	    brect.bottom = brect.top+tsize.cy-((tsize.cy)>>3);
	    break;
	default:
	    break;

    }

	#if TOPOFASTLABEL
	notoverlapping = checkLabelBlock(&brect); 
	#else
    notoverlapping = checkLabelBlock(brect); 
	#endif
  
    if (!noOverlap || notoverlapping) {
#if (WINDOWSPC>0)
      SetBkMode(hDC,TRANSPARENT);
	if (NewMap) {
		TextColor(hDC,Mode.AsFlag.Color);
      		ExtTextOut(hDC, x, y, 0, NULL, Value, size, NULL);
		SetTextColor(hDC,RGB_BLACK); 
	} else
      		ExtTextOut(hDC, x, y, 0, NULL, Value, size, NULL);
#else
	if (NewMap) {
		TextColor(hDC,Mode.AsFlag.Color);
      		ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, Value, size, NULL);
		SetTextColor(hDC,RGB_BLACK); 
	} else
      		ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, Value, size, NULL);
#endif
      drawn=true;
    }

  }
 
  if (!Mode.AsFlag.NoSetFont) SelectObject(hDC, oldFont); // VENTA5
  SelectObject(hDC, hbOld);

  return drawn;

}

bool userasked = false;

void MapWindow::RequestFastRefresh() {
  SetEvent(drawTriggerEvent);
}

void MapWindow::RefreshMap() {
  MapDirty = true;
  userasked = true;
  timestats_dirty = true;
  SetEvent(drawTriggerEvent);
}

bool MapWindow::IsMapFullScreen() {
  // SDP - Seems that RequestFullScreen
  // is always more accurate (MapFullSCreen is delayed)
  return RequestFullScreen;
  // return  MapFullScreen;
}


void MapWindow::ToggleFullScreenStart() {

  // ok, save the state.
  MapFullScreen = RequestFullScreen;

  // show infoboxes immediately

  if (MapFullScreen) {
    MapRect = MapRectBig;
    HideInfoBoxes();    
    DefocusInfoBox(); // BUGFIX 091115 BgColor
  } else {
    MapRect = MapRectSmall;
    ShowInfoBoxes(); // VENTA FIX QUI
  }
  #ifndef NOVARIOGAUGE
  GaugeVario::Show(!MapFullScreen);
  #endif
}


void MapWindow::RequestToggleFullScreen() {
  RequestFullScreen = !RequestFullScreen;
  RefreshMap();
}

void MapWindow::RequestOnFullScreen() {
  RequestFullScreen = true;
  RefreshMap();
}

void MapWindow::RequestOffFullScreen() {
  RequestFullScreen = false;
  RefreshMap();
}



extern BOOL extGPSCONNECT;
extern bool DialogActive;



void MapWindow::Event_AutoZoom(int vswitch) {
  if (vswitch== -1) {
    AutoZoom = !AutoZoom;
  } else {
    AutoZoom = (vswitch != 0); // 0 off, 1 on
  }
  
  if (AutoZoom) {
    if (EnablePan) {
      EnablePan = false;
      InputEvents::setMode(TEXT("default"));
      StoreRestoreFullscreen(false);
    }
  }
  RefreshMap();
}


void MapWindow::Event_PanCursor(int dx, int dy) {
  int X= (MapRect.right+MapRect.left)/2;
  int Y= (MapRect.bottom+MapRect.top)/2;
  double Xstart, Ystart, Xnew, Ynew;

  Screen2LatLon(X, Y, Xstart, Ystart);

  X+= (MapRect.right-MapRect.left)*dx/4;
  Y+= (MapRect.bottom-MapRect.top)*dy/4;
  Screen2LatLon(X, Y, Xnew, Ynew);

  if (EnablePan) {
    PanLongitude += Xstart-Xnew;
    PanLatitude += Ystart-Ynew;
  }
  RefreshMap();
}

bool MapWindow::isPan() {
  return EnablePan;
}

/* Event_TerrainToplogy Changes
   0       Show
   1       Toplogy = ON
   2       Toplogy = OFF
   3       Terrain = ON
   4       Terrain = OFF
   -1      Toggle through 4 stages (off/off, off/on, on/off, on/on)
   -2      Toggle terrain
   -3      Toggle toplogy
*/
void MapWindow::Event_TerrainTopology(int vswitch) {
  char val;

  if (vswitch== -1) { // toggle through 4 possible options
    val = 0;
    if (EnableTopology) val++;
    if (EnableTerrain) val += (char)2;
    val++;
    if (val>3) val=0;
    EnableTopology = ((val & 0x01) == 0x01);
    EnableTerrain  = ((val & 0x02) == 0x02);
    RefreshMap();

  } else if (vswitch == -2) { // toggle terrain
    EnableTerrain = !EnableTerrain;
    RefreshMap();

  } else if (vswitch == -3) { // toggle topology
    EnableTopology = !EnableTopology;
    RefreshMap();

  } else if (vswitch == 1) { // Turn on toplogy
    EnableTopology = true;
    RefreshMap();

  } else if (vswitch == 2) { // Turn off toplogy
    EnableTopology = false;
    RefreshMap();

  } else if (vswitch == 3) { // Turn on terrain 
    EnableTerrain = true;
    RefreshMap();

  } else if (vswitch == 4) { // Turn off terrain
    EnableTerrain = false;
    RefreshMap();

  } else if (vswitch == 0) { // Show terrain/Topology
    // ARH Let user know what's happening
    TCHAR buf[128];
    if (EnableTopology) {
	if (EnableTerrain)
		_stprintf(buf, _T("Topo:ON   Terra:ON"));
	else
		_stprintf(buf, _T("Topo:ON   Terra:OFF"));
    } else {
	if (EnableTerrain)
		_stprintf(buf, _T("Topo:OFF   Terra:ON"));
	else
		_stprintf(buf, _T("Topo:OFF   Terra:OFF"));
    }

    Message::Lock(); // 091211
    Message::AddMessage(500, 3, buf); // 091125
    Message::Unlock();
    // DoStatusMessage(TEXT("Topology / Terrain"), buf);
  }
}


void MapWindow::StoreRestoreFullscreen(bool store) {
  static bool oldfullscreen = 0;
  static bool SuperPan = false;
  if (store) {
    // pan not active on entry, save fullscreen status
    SuperPan = true;
    oldfullscreen = MapWindow::IsMapFullScreen();
  } else {
    if (SuperPan) {
      // pan is active, need to restore
      if (!oldfullscreen) {
        // change it if necessary
        RequestFullScreen = false;
      }
      SuperPan = false;
    }
  }
}


void MapWindow::Event_Pan(int vswitch) {
  //  static bool oldfullscreen = 0;  never assigned!
  bool oldPan = EnablePan;
  if (vswitch == -2) { // superpan, toggles fullscreen also

    if (!EnablePan) {
      StoreRestoreFullscreen(true);
    } else {
      StoreRestoreFullscreen(false);
    }
    // new mode
    EnablePan = !EnablePan;
    if (EnablePan) { // pan now on, so go fullscreen
      RequestFullScreen = true;
    }

  } else if (vswitch == -1) {
    EnablePan = !EnablePan;
  } else {
    EnablePan = (vswitch != 0); // 0 off, 1 on
  }

  if (EnablePan != oldPan) {
    if (EnablePan) {
      PanLongitude = DrawInfo.Longitude;
      PanLatitude = DrawInfo.Latitude;
      InputEvents::setMode(TEXT("pan"));
    } else 
      InputEvents::setMode(TEXT("default"));
  }
  RefreshMap();
}

double MapWindow::LimitMapScale(double value) {

  double minreasonable;

  if ( ISPARAGLIDER ) 
    minreasonable = 0.005;  // 091017 5m resolution for Para
  else
    minreasonable = 0.05; 

  if (AutoZoom && DisplayMode != dmCircling) {
    if (AATEnabled && (ActiveWayPoint>0)) {
      if ( ISPARAGLIDER ) minreasonable = 0.005; 
      else minreasonable = 0.88;
    } else {
      if ( ISPARAGLIDER ) minreasonable = 0.005; // 091016 0.01
      else minreasonable = 0.44; 
    }
  }

  if (ScaleListCount>0) {
    return FindMapScale(max(minreasonable,min(160.0,value)));
  } else {
    return max(minreasonable,min(160.0,value));
  }
}


void MapWindow::Event_SetZoom(double value) {


/* 091023 TEST REMOVE
//  static bool doinit_climb=true;
//  static bool doinit_cruise=true;
//
//  if (!CALCULATED_INFO.Circling && doinit_cruise) {
//	RequestMapScale=8.0;
//	doinit_cruise=false;
//  }
//  if (CALCULATED_INFO.Circling && doinit_climb) {
//	RequestMapScale=0.70;
//	doinit_climb=false;
//  }
*/


  static double lastRequestMapScale = RequestMapScale;

  RequestMapScale = LimitMapScale(value);
  if (lastRequestMapScale != RequestMapScale){
    lastRequestMapScale = RequestMapScale;
    BigZoom = true;
    RefreshMap();
  }
}


void MapWindow::Event_ScaleZoom(int vswitch) {

  static double lastRequestMapScale = RequestMapScale;
  double value = RequestMapScale;
  static int nslow=0;

  if (isAutoZoom()) {
	DoStatusMessage(_T("Autozoom OFF")); // FIXV2
	AutoZoom=0;
  }

  // For best results, zooms should be multiples or roots of 2

  if (ScaleListCount > 0){
    value = FindMapScale(RequestMapScale);
    value = StepMapScale(-vswitch);
  } else {

    if (abs(vswitch)>=4) {
      nslow++;
      if (nslow %2 != 0) {
        // JMW disabled        return;
      }
      if (vswitch==4) {
        vswitch = 1;
      }
      if (vswitch==-4) {
        vswitch = -1;
      }
    }
    if (vswitch==1) { // zoom in a little
      value /= 1.414;
    }
    if (vswitch== -1) { // zoom out a little
      value *= 1.414;
    }
    if (vswitch==2) { // zoom in a lot
      value /= 2.0;
    }
    if (vswitch== -2) { // zoom out a lot
      value *= 2.0;
    } 

  }
  RequestMapScale = LimitMapScale(value);

  if (lastRequestMapScale != RequestMapScale){
    lastRequestMapScale = RequestMapScale;
    BigZoom = true;

    RefreshMap();

    //    DrawMapScale(hdcScreen, MapRect, true);
    // JMW this is bad, happening from wrong thread.
  }
}


int MapWindow::GetMapResolutionFactor(void) { // TESTFIX 091017 CHECKFIX
  static int retglider=NIBLSCALE(30);
  //static int retpara=IBLSCALE(30);
/*
  if (ISPARAGLIDER)
	return IBLSCALE(3); // 091017 30 TESTFIX QUIQUI
  else
	return IBLSCALE(30);
*/
  //if (ISPARAGLIDER) return retpara; else return retglider;

  return retglider;
}

double MapWindow::StepMapScale(int Step){
  static int nslow=0;
  if (abs(Step)>=4) {
    nslow++;
    //    if (nslow %2 == 0) {
    ScaleCurrent += Step/4;
    //    }
  } else {
    ScaleCurrent += Step;
  }
  ScaleCurrent = max(0,min(ScaleListCount-1, ScaleCurrent));
  return((ScaleList[ScaleCurrent]*GetMapResolutionFactor())
         /(IBLSCALE(/*Appearance.DefaultMapWidth*/ MapRect.right)));
}

double MapWindow::FindMapScale(double Value){

  int    i;
  double BestFit = 99999;
  int    BestFitIdx=-1;
  double DesiredScale = 
    (Value*IBLSCALE(/*Appearance.DefaultMapWidth*/ MapRect.right))/GetMapResolutionFactor();

  for (i=0; i<ScaleListCount; i++){
    double err = fabs(DesiredScale - ScaleList[i])/DesiredScale;
    if (err < BestFit){
      BestFit = err;
      BestFitIdx = i;
    }
  }

  if (BestFitIdx != -1){
    ScaleCurrent = BestFitIdx;
    return((ScaleList[ScaleCurrent]*GetMapResolutionFactor())
           /IBLSCALE(/*Appearance.DefaultMapWidth*/ MapRect.right));
  }
  return(Value);
}


static void SetFontInfo(HDC hDC, FontHeightInfo_t *FontHeightInfo){
  TEXTMETRIC tm;
  int x,y=0;
  RECT  rec;
  int top, bottom;

  GetTextMetrics(hDC, &tm);
  FontHeightInfo->Height = tm.tmHeight;
  FontHeightInfo->AscentHeight = tm.tmAscent;
  FontHeightInfo->CapitalHeight = 0;

  SetBkMode(hDC, OPAQUE);
  SetBkColor(hDC,RGB_WHITE);
  SetTextColor(hDC,RGB_BLACK);
  rec.left = 0;
  rec.top = 0;
  rec.right = tm.tmAveCharWidth;
  rec.bottom = tm.tmHeight;
  ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rec, TEXT("M"), _tcslen(TEXT("M")), NULL);

  top = tm.tmHeight;
  bottom = 0;

  FontHeightInfo->CapitalHeight = 0;
  for (x=0; x<tm.tmAveCharWidth; x++){
    for (y=0; y<tm.tmHeight; y++){
      if ((GetPixel(hDC, x, y)) != RGB_WHITE){
        if (top > y)
          top = y;
        if (bottom < y)
          bottom = y;
      }
    }
  }

#ifdef GNAV
  // JMW: don't know why we need this in GNAV, but we do.
  if (FontHeightInfo->CapitalHeight<y)
    FontHeightInfo->CapitalHeight = bottom - top + 1;
#endif
  // This works for PPC
  if (FontHeightInfo->CapitalHeight <= 0)
    FontHeightInfo->CapitalHeight = tm.tmAscent - 1 -(tm.tmHeight/10);

  //  int lx = GetDeviceCaps(hDC,LOGPIXELSX);
  // dpi
}


LRESULT CALLBACK MapWindow::MapWndProc (HWND hWnd, UINT uMsg, WPARAM wParam,
                                        LPARAM lParam)
{
  int i;
  static double Xstart, Ystart;
  static int XstartScreen, YstartScreen;
  int X,Y;
  int gestX, gestY, gestDir=LKGESTURE_NONE, gestDist=-1;
  double Xlat, Ylat;
  double distance;
  int width = (int) LOWORD(lParam);
  int height = (int) HIWORD(lParam);
  
  static DWORD dwDownTime= 0L, dwUpTime= 0L, dwInterval= 0L;

  #if LKPMODE
  bool dontdrawthemap=(DONTDRAWTHEMAP);
  bool mapmode8000=(MAPMODE8000);
  #else
  bool dontdrawthemap=(DONTDRAWTHEMAP);
  bool mapmode8000=(MAPMODE8000);
  #endif

  static short navboxesY;
  // Attention... this is duplicated inside Utils2, I am lazy 
  // apparently only #include is duplicated, so no problems
  static bool doinit=true;
  static int AIRCRAFTMENUSIZE=0, COMPASSMENUSIZE=0;

  navboxesY=(MapWindow::MapRect.bottom-MapWindow::MapRect.top)-BottomSize-NIBLSCALE(2); // BUGFIX 091125


  switch (uMsg)
    {
      /* JMW THIS IS BAD!  Now done with GCE_AIRSPACE
	 case WM_USER+1:
	 dlgAirspaceWarningShowDlg(false);
	 return(0);
      */
    case WM_ERASEBKGND:
      // JMW trying to reduce flickering
      /*
	if (first || MapDirty) {
	first = false;
	MapDirty = true;
	return (DefWindowProc (hWnd, uMsg, wParam, lParam));
	} else
	return TRUE;
      */
      return TRUE;
    case WM_SIZE:

      hDrawBitMap = CreateCompatibleBitmap (hdcScreen, width, height);
      SelectObject(hdcDrawWindow, (HBITMAP)hDrawBitMap);

      hDrawBitMapTmp = CreateCompatibleBitmap (hdcScreen, width, height);
      SelectObject(hDCTemp, (HBITMAP)hDrawBitMapTmp);

      hMaskBitMap = CreateBitmap(width+1, height+1, 1, 1, NULL);
      SelectObject(hDCMask, (HBITMAP)hMaskBitMap);

      {
	HFONT      oldFont;

	oldFont = (HFONT)SelectObject(hDCTemp, TitleWindowFont);
	SetFontInfo(hDCTemp, &Appearance.TitleWindowFont);

	SelectObject(hDCTemp, MapWindowFont);
	SetFontInfo(hDCTemp, &Appearance.MapWindowFont);

	SelectObject(hDCTemp, MapWindowBoldFont);
	SetFontInfo(hDCTemp, &Appearance.MapWindowBoldFont);

	SelectObject(hDCTemp, InfoWindowFont);
	SetFontInfo(hDCTemp, &Appearance.InfoWindowFont);

	SelectObject(hDCTemp, CDIWindowFont);
	SetFontInfo(hDCTemp, &Appearance.CDIWindowFont);
//VENTA6
	SelectObject(hDCTemp, StatisticsFont);
	SetFontInfo(hDCTemp, &Appearance.StatisticsFont);

	SelectObject(hDCTemp, MapLabelFont);
	SetFontInfo(hDCTemp, &Appearance.MapLabelFont);

	SelectObject(hDCTemp, TitleSmallWindowFont);
	SetFontInfo(hDCTemp, &Appearance.TitleSmallWindowFont);

	SelectObject(hDCTemp, oldFont);
      }

      break;

    case WM_CREATE:

      hdcScreen = GetDC(hWnd);
      hdcDrawWindow = CreateCompatibleDC(hdcScreen);
      hDCTemp = CreateCompatibleDC(hdcDrawWindow);
      hDCMask = CreateCompatibleDC(hdcDrawWindow);

      #if LKOBJ
      hBackgroundBrush = LKBrush_White;
      hInvBackgroundBrush[0] = LKBrush_White;
      hInvBackgroundBrush[1] = LKBrush_LightGrey;
      hInvBackgroundBrush[2] = LKBrush_LcdGreen;
      hInvBackgroundBrush[3] = LKBrush_LcdDarkGreen;
      hInvBackgroundBrush[4] = LKBrush_Grey; 
      hInvBackgroundBrush[5] = LKBrush_Lake;
      hInvBackgroundBrush[6] = LKBrush_Emerald;
      hInvBackgroundBrush[7] = LKBrush_DarkSlate;
      hInvBackgroundBrush[8] = LKBrush_RifleGrey;
      hInvBackgroundBrush[9] = LKBrush_Black;
      #else
      hBackgroundBrush = CreateSolidBrush(BackgroundColor);
      hInvBackgroundBrush[0] = CreateSolidBrush(COLORREF RGB_WHITE);	 // black text required
      hInvBackgroundBrush[1] = CreateSolidBrush(COLORREF RGB_LIGHTGREY); // black  light grey
      hInvBackgroundBrush[2] = CreateSolidBrush(COLORREF RGB_LCDGREEN);	// black   lcd green 
      hInvBackgroundBrush[3] = CreateSolidBrush(COLORREF RGB_LCDDARKGREEN); // lcd green dark
      hInvBackgroundBrush[4] = CreateSolidBrush(COLORREF RGB_GREY);	// 
      hInvBackgroundBrush[5] = CreateSolidBrush(COLORREF RGB_LAKE);	// bianco , ok con topology
      hInvBackgroundBrush[6] = CreateSolidBrush(COLORREF RGB_EMERALD);   // slategreen
      // BlackScreen requested for white waypoints, no topology working
      hInvBackgroundBrush[7] = CreateSolidBrush(COLORREF RGB_DARKSLATE); // senza topo, richiede wp bianchi
      hInvBackgroundBrush[8] = CreateSolidBrush(COLORREF RGB_RIFLEGREY);  // dark grey
      hInvBackgroundBrush[9] = CreateSolidBrush(COLORREF RGB_BLACK); // richiede blackscreen
      #endif

      hFLARMTraffic=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FLARMTRAFFIC));
      hTerrainWarning=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TERRAINWARNING));
      hTurnPoint=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TURNPOINT));
      hInvTurnPoint=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_INVTURNPOINT));
      hSmall=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SMALL));
      hInvSmall=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_INVSMALL));
      hAutoMacCready=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AUTOMCREADY));
      hGPSStatus1=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_GPSSTATUS1));
      hGPSStatus2=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_GPSSTATUS2));
      hLogger=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LOGGER));
      hLoggerOff=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LOGGEROFF));
      hBmpTeammatePosition = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TEAMMATE_POS));

      if ( ISPARAGLIDER ) {
	hCruise=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CRUISEPARA));
	hClimb=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CLIMBPARA));
	hFinalGlide=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FINALGLIDEPARA));
	hAbort=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ABORT));
      } else {
	hCruise=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CRUISE));
	hClimb=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CLIMB));
	hFinalGlide=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FINALGLIDE));
	hAbort=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ABORT));
      }

      //hBmpCompassBg = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_COMPASSBG));


      // airspace brushes and colours

      hAirspaceBitmap[0]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE0));
      hAirspaceBitmap[1]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE1));
      hAirspaceBitmap[2]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE2));
      hAirspaceBitmap[3]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE3));
      hAirspaceBitmap[4]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE4));
      hAirspaceBitmap[5]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE5));
      hAirspaceBitmap[6]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE6));
      hAirspaceBitmap[7]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE7));

      hAboveTerrainBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ABOVETERRAIN));

      for (i=0; i<NUMAIRSPACEBRUSHES; i++) {
	hAirspaceBrushes[i] =
	  CreatePatternBrush((HBITMAP)hAirspaceBitmap[i]);
      }
      hAboveTerrainBrush = CreatePatternBrush((HBITMAP)hAboveTerrainBitmap);

	int iwidth;
#ifndef NEWTRAIL
      // normal colours
      BYTE Red,Green,Blue;
      int minwidth;
      minwidth = max(NIBLSCALE(2),IBLSCALE(SnailWidthScale)/16);
      for (i=0; i<NUMSNAILCOLORS; i++) {
	short ih = i*200/(NUMSNAILCOLORS-1);
	ColorRampLookup(ih, 
			Red, Green, Blue,
			snail_colors, NUMSNAILRAMP, 6);      
	if (i<NUMSNAILCOLORS/2) {
	  iwidth= minwidth;
	} else {
	  iwidth = max(minwidth,
		       (i-NUMSNAILCOLORS/2)
		       *IBLSCALE(SnailWidthScale)/NUMSNAILCOLORS);
	}

	hSnailColours[i] = RGB((BYTE)Red,(BYTE)Green,(BYTE)Blue);
	hSnailPens[i] = (HPEN)CreatePen(PS_SOLID, iwidth, hSnailColours[i]);

      }
#else
	iwidth=IBLSCALE(SnailWidthScale);
	hSnailColours[0] = RGB_BLACK;
	hSnailColours[1] = RGB_INDIGO;
	hSnailColours[2] = RGB_INDIGO;
	hSnailColours[3] = RGB_BLUE;
	hSnailColours[4] = RGB_BLUE;
	hSnailColours[5] = RGB_LAKE;
	hSnailColours[6] = RGB_LAKE;
	hSnailColours[7] = RGB_GREY;
	hSnailColours[8] = RGB_GREEN;
	hSnailColours[9] = RGB_GREEN;
	hSnailColours[10] = RGB_ORANGE;
	hSnailColours[11] = RGB_ORANGE;
	hSnailColours[12] = RGB_RED;
	hSnailColours[13] = RGB_RED;
	hSnailColours[14] = RGB_DARKRED;

	hSnailPens[0] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[0]);
	hSnailPens[1] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[1]);
	hSnailPens[2] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[2]);
	hSnailPens[3] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[3]);
	hSnailPens[4] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(2), hSnailColours[4]);
	hSnailPens[5] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), hSnailColours[5]);
	hSnailPens[6] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), hSnailColours[6]);
	hSnailPens[7] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(6), hSnailColours[7]);
	hSnailPens[8] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), hSnailColours[8]);
	hSnailPens[9] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), hSnailColours[9]);
	hSnailPens[10] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[10]);
	hSnailPens[11] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[11]);
	hSnailPens[12] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[12]);
	hSnailPens[13] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[13]);
	hSnailPens[14] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[14]);
#endif

      /* JMW created all re-used pens here */


      // testing only    Appearance.InverseAircraft = true;

	#if LKOBJ
      hpCompassBorder = LKPen_Black_N2;
      if (Appearance.InverseAircraft) {
	hpAircraft = LKPen_Black_N3;
	hpAircraftBorder = LKPen_White_N1;
      } else {
	hpAircraft = LKPen_White_N3;
	hpAircraftBorder = LKPen_Black_N1;
      }
	hpThermalCircle = LKPen_White_N3;

	#else
      hpCompassBorder = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB_BLACK); 
      if (Appearance.InverseAircraft) {
	hpAircraft = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(3), RGB_BLACK);
	hpAircraftBorder = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_WHITE);
      } else {
	hpAircraft = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(3), RGB_WHITE);
	hpAircraftBorder = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_BLACK);
      }
	hpThermalCircle = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(3), RGB_WHITE);
	#endif

	#if LKOBJ
#if (MONOCHROME_SCREEN > 0)
      hpWind = LKPen_Black_N2;
#else
      hpWind = LKPen_Red_N2;
#endif
	#else
#if (MONOCHROME_SCREEN > 0)
      hpWind = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB_BLACK);
#else
      hpWind = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB_RED);
#endif
	#endif

      hpWindThick = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(4), RGB(255,220,220));

	#if LKOBJ
      hpBearing = LKPen_Black_N2;
      hpBestCruiseTrack = LKPen_Blue_N1;
	#else
      hpBearing = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB_BLACK);
      hpBestCruiseTrack = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_BLUE);
	#endif
#if (MONOCHROME_SCREEN > 0)
      hpCompass = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_BLACK);
#else
      hpCompass = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(0xcf,0xcf,0xFF));
#endif
      hpThermalBand = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB(0x40,0x40,0xFF));
      hpThermalBandGlider = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB(0x00,0x00,0x30));

      hpFinalGlideBelow = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(0xFF,0xA0,0xA0));
      hpFinalGlideBelowLandable = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(255,196,0));

      // TODO enhancement: support red/green Color blind
      hpFinalGlideAbove = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(0xA0,0xFF,0xA0));

	#if LKOBJ
      hpSpeedSlow=LKPen_Red_N1;
      hpSpeedFast=LKPen_Green_N1;
      hpStartFinishThin=LKPen_Red_N1;
      hpMapScale = LKPen_Black_N1;

	#else
      hpSpeedSlow=(HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_RED);
      hpSpeedFast=(HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_GREEN);
      hpStartFinishThin=(HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_RED);
      hpMapScale = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1),  RGB_BLACK);
	#endif

      hpStartFinishThick=(HPEN)CreatePen(PS_SOLID, NIBLSCALE(5), taskcolor);
      hpMapScale2 = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1)+1, RGB_BLACK);
      // TerrainLine is for shade, Bg is for perimeter
      hpTerrainLine = (HPEN)CreatePen(PS_DASH, (1), RGB(0x30,0x30,0x30));
      hpTerrainLineBg = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB_LCDDARKGREEN);
      hpVisualGlideLightBlack = (HPEN)CreatePen(PS_DASH, (1), RGB_BLACK);
      hpVisualGlideHeavyBlack = (HPEN)CreatePen(PS_DASH, (2), RGB_BLACK);
      hpVisualGlideLightRed = (HPEN)CreatePen(PS_DASH, (1), RGB_RED);
      hpVisualGlideHeavyRed = (HPEN)CreatePen(PS_DASH, (2), RGB_RED);

      #if LKOBJ

      hbThermalBand=LKBrush_Emerald;
	#if (MONOCHROME_SCREEN > 0)
      hbCompass=LKBrush_White;
	#else
      hbCompass=LKBrush_Cyan;
	#endif
      hbBestCruiseTrack=LKBrush_Blue;
      hbFinalGlideBelow=LKBrush_Red;
      hbFinalGlideAbove=LKBrush_Green;
      hbFinalGlideBelowLandable=LKBrush_Orange;
	#if (MONOCHROME_SCREEN > 0)
      hbWind=LKBrush_Grey;
	#else
      hbWind=LKBrush_Grey;
	#endif
      #else

      hbThermalBand=(HBRUSH)CreateSolidBrush(RGB(0x80,0x80,0xFF));
	#if (MONOCHROME_SCREEN > 0)
      hbCompass=(HBRUSH)CreateSolidBrush(RGB_WHITE);
	#else
      hbCompass=(HBRUSH)CreateSolidBrush(RGB(0x40,0x40,0xFF));
	#endif
      hbBestCruiseTrack=(HBRUSH)CreateSolidBrush(RGB_BLUE);
      hbFinalGlideBelow=(HBRUSH)CreateSolidBrush(RGB_RED);
      hbFinalGlideAbove=(HBRUSH)CreateSolidBrush(RGB_GREEN);
      hbFinalGlideBelowLandable=(HBRUSH)CreateSolidBrush(RGB(0xFF,180,0x00));
	#if (MONOCHROME_SCREEN > 0)
      hbWind=(HBRUSH)CreateSolidBrush(RGB_MIDDLEGREY);
	#else
      hbWind=(HBRUSH)CreateSolidBrush(RGB_MIDDLEGREY);
	#endif

      #endif


      ScaleListCount = propGetScaleList(ScaleList, sizeof(ScaleList)/sizeof(ScaleList[0]));
      RequestMapScale = LimitMapScale(RequestMapScale);

      hBmpMapScale = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MAPSCALE_A));

      #if LKOBJ
      hBrushFlyingModeAbort = LKBrush_Red;
      #else
      hBrushFlyingModeAbort = (HBRUSH)CreateSolidBrush(RGB_RED);
      #endif

      if (Appearance.IndLandable == wpLandableDefault){
	hBmpAirportReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_REACHABLE));
	hBmpAirportUnReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LANDABLE));
	hBmpFieldReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_REACHABLE));
	hBmpFieldUnReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LANDABLE));
      }else
	if (Appearance.IndLandable == wpLandableAltA){
	  hBmpAirportReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRPORT_REACHABLE));
	  hBmpAirportUnReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRPORT_UNREACHABLE));
	  hBmpFieldReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_OUTFILED_REACHABLE));
	  hBmpFieldUnReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_OUTFILED_UNREACHABLE));
	}

      hBmpThermalSource = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_THERMALSOURCE));
      hBmpTarget = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TARGET));

      // Signal that draw thread can run now
      Initialised = TRUE;

      break;

    case WM_DESTROY:

      ReleaseDC(hWnd, hdcScreen);
      DeleteDC(hdcDrawWindow);
      DeleteDC(hDCTemp);
      DeleteDC(hDCMask);
      DeleteObject(hDrawBitMap);
      DeleteObject(hMaskBitMap);

      DeleteObject(hTurnPoint);
      DeleteObject(hSmall);
      DeleteObject(hInvTurnPoint);
      DeleteObject(hInvSmall);
      DeleteObject(hCruise);
      DeleteObject(hClimb);
      DeleteObject(hFinalGlide);
      DeleteObject(hAutoMacCready);
      DeleteObject(hFLARMTraffic);
      DeleteObject(hTerrainWarning);
      DeleteObject(hGPSStatus1);
      DeleteObject(hGPSStatus2);
      DeleteObject(hAbort);
      DeleteObject(hLogger);
      DeleteObject(hLoggerOff);
    
      DeleteObject((HPEN)hpWindThick);

	#ifndef LKOBJ
      DeleteObject((HPEN)hpBestCruiseTrack);
      DeleteObject((HPEN)hpBearing);
      DeleteObject((HPEN)hpWind);
      DeleteObject((HPEN)hpAircraft); 
      DeleteObject((HPEN)hpAircraftBorder); 
      DeleteObject((HPEN)hpCompass); 
      DeleteObject((HPEN)hpThermalCircle);
      DeleteObject((HPEN)hpSpeedSlow);
      DeleteObject((HPEN)hpSpeedFast);
      DeleteObject((HPEN)hpStartFinishThin);
      DeleteObject((HPEN)hpMapScale);
	#endif
/*
#if OVERTARGET
      DeleteObject((HPEN)hpOvertarget);
#endif
*/
      DeleteObject((HPEN)hpThermalBand);
      DeleteObject((HPEN)hpThermalBandGlider);
      DeleteObject((HPEN)hpFinalGlideAbove);
      DeleteObject((HPEN)hpFinalGlideBelow);
      DeleteObject((HPEN)hpTerrainLine);
      DeleteObject((HPEN)hpTerrainLineBg);
      DeleteObject((HPEN)hpStartFinishThick);

      DeleteObject((HPEN)hpVisualGlideLightBlack); // VENTA3
      DeleteObject((HPEN)hpVisualGlideLightRed); // VENTA3
      DeleteObject((HPEN)hpVisualGlideHeavyRed); // VENTA3
      DeleteObject((HPEN)hpVisualGlideHeavyBlack); // VENTA3
      DeleteObject((HPEN)hpFinalGlideBelowLandable);

      #ifndef LKOBJ
      DeleteObject((HBRUSH)hbCompass);
      DeleteObject((HBRUSH)hbThermalBand);
      DeleteObject((HBRUSH)hbBestCruiseTrack);
      DeleteObject((HBRUSH)hbFinalGlideBelow);
      DeleteObject((HBRUSH)hbFinalGlideAbove);
      DeleteObject((HBRUSH)hbWind);
      #endif

      DeleteObject(hBmpMapScale);
      DeleteObject(hBmpCompassBg);
      #ifndef LKOBJ
      DeleteObject((HBRUSH)hbFinalGlideBelowLandable);
      DeleteObject(hBackgroundBrush);
      DeleteObject(hInvBackgroundBrush[0]); // 091110
      DeleteObject(hInvBackgroundBrush[1]); // 091110
      DeleteObject(hInvBackgroundBrush[2]); // 091110
      DeleteObject(hInvBackgroundBrush[3]); // 091110
      DeleteObject(hInvBackgroundBrush[4]); // 091110
      DeleteObject(hInvBackgroundBrush[5]); // 091110
      DeleteObject(hInvBackgroundBrush[6]); // 091110
      DeleteObject(hInvBackgroundBrush[7]); // 091110
      DeleteObject(hInvBackgroundBrush[8]); // 091110
      DeleteObject(hInvBackgroundBrush[9]); // 091110
      DeleteObject((HBRUSH)hBrushFlyingModeAbort);
      #endif
      DeleteObject(hBmpClimbeAbort);

      DeleteObject((HPEN)hpCompassBorder);

      DeleteObject(hBmpAirportReachable);
      DeleteObject(hBmpAirportUnReachable);
      DeleteObject(hBmpFieldReachable);
      DeleteObject(hBmpFieldUnReachable);
      DeleteObject(hBmpThermalSource);
      DeleteObject(hBmpTarget);
      DeleteObject(hBmpTeammatePosition);

      for(i=0;i<NUMAIRSPACEBRUSHES;i++)
	{
	  DeleteObject(hAirspaceBrushes[i]);
	  DeleteObject(hAirspaceBitmap[i]);
	}

      DeleteObject(hAboveTerrainBitmap);
      DeleteObject(hAboveTerrainBrush);

      for (i=0; i<AIRSPACECLASSCOUNT; i++) {
	DeleteObject(hAirspacePens[i]);
      }

      for (i=0; i<NUMSNAILCOLORS; i++) {
	DeleteObject(hSnailPens[i]);
      }
    
      PostQuitMessage (0);

      break;

    case WM_LBUTTONDBLCLK: 
      // VNT TODO: do not handle this event and remove CS_DBLCLKS in register class.
      // Only handle timed clicks in BUTTONDOWN with no proximity. 
      //
      // Attention please: a DBLCLK is followed by a simple BUTTONUP with NO buttondown.

      dwDownTime = GetTickCount();  
      XstartScreen = LOWORD(lParam); YstartScreen = HIWORD(lParam);

      // Careful! If you ignorenext, any event timed as double click of course will be affected.
      // and this means also fast clicking on bottombar!!
      // so first lets see if we are in lk8000 text screens.. 
      if (dontdrawthemap || (mapmode8000 && (YstartScreen >=navboxesY))) {  
		// do not ignore next, let buttonup get the signal
		break;
      }

      if (UseMapLock&&NewMap) {
        // ignorenext=true; 100318
	if (MapLock)
	{
		// ignorenext only when expecting a double click when map is locked! 
        	ignorenext=true; 
		if (DrawBottom) { 
	      		Y = HIWORD(lParam); 
		        // Unlock map only if double clicking on real map, not on navboxes!
			if ( Y < (MapWindow::MapRect.bottom-MapWindow::MapRect.top-BottomSize-NIBLSCALE(15)) ) {
				  dwDownTime= 0L;
				  DefocusInfoBox();
				  SetFocus(hWnd);
				  iboxtoclick=true; 
	  			  UnlockMap();
	  			  break;
			} 
		// old screen mode, dblclk on real map, unlock..
		} else {
			  dwDownTime= 0L;
			  DefocusInfoBox();
			  iboxtoclick=true; 
			  SetFocus(hWnd);
			  UnlockMap();
			  break;
		}

        }
      }

#if 100318
	// only when activemap (which means newmap is on) is off
      if (ActiveMap) ignorenext=true;
      if (NewMap) break;
#else

      ignorenext=true;
      // VENTA 090721 doubleclick no more used in lk8000
      if (NewMap) break;
#endif
      #ifndef DISABLEAUDIO
      if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
      #endif
      ShowMenu();
      break;

    case WM_LBUTTONDOWN:
      #ifdef DEBUG_DBLCLK
      DoStatusMessage(_T("BUTTONDOWN MapWindow")); 
      #endif
      DisplayTimeOut = 0;
      dwDownTime = GetTickCount();
      // After calling a menu, on exit as we touch the screen we fall back here
      if (ignorenext) {
#ifdef DEBUG_MAPINPUT
		DoStatusMessage(TEXT("DBG-055 BUTTONDOWN with ignorenext"));
#endif
		break;
      }
      XstartScreen = LOWORD(lParam); YstartScreen = HIWORD(lParam);
      // TODO VNT move Screen2LatLon in LBUTTONUP after making sure we really need Xstart and Ystart
      // so we save precious milliseconds waiting for BUTTONUP GetTickCount
      Screen2LatLon(XstartScreen, YstartScreen, Xstart, Ystart);

      LKevent=LKEVENT_NONE; // CHECK FIX TODO VENTA10  probably useless 090915

      LockTaskData();
      if (AATEnabled && TargetPan) {
	if (ValidTaskPoint(TargetPanIndex)) {
	  POINT tscreen;
	  LatLon2Screen(Task[TargetPanIndex].AATTargetLon, 
			Task[TargetPanIndex].AATTargetLat, 
			tscreen);
	  distance = isqrt4((long)((XstartScreen-tscreen.x)
				   *(XstartScreen-tscreen.x)+
				   (YstartScreen-tscreen.y)
				   *(YstartScreen-tscreen.y)))
	    /InfoBoxLayout::scale;

	  if (distance<10) {
	    TargetDrag_State = 1;
	  }
	}
      }
      UnlockTaskData();

      FullScreen();
      break;

    case WM_LBUTTONUP:
	if (ignorenext||dwDownTime==0) { 
#ifdef DEBUG_MAPINPUT
		if (ignorenext && (dwDownTime==0) )
			DoStatusMessage(_T("DBG-098 ignorenext&&dwDownTime0"));
		else if (dwDownTime==0)
			DoStatusMessage(_T("DBG-099 dwDownTime0"));
		else
			DoStatusMessage(_T("DBG-097 ignorenext"));
#endif
		ignorenext=false;
		break;
	}
      RECT rc;
      dwUpTime = GetTickCount(); 
      dwInterval=dwUpTime-dwDownTime;
      dwDownTime=0; // do it once forever

      GetClientRect(hWnd,&rc);

      X = LOWORD(lParam); Y = HIWORD(lParam); 

	gestY=YstartScreen-Y;
	gestX=XstartScreen-X;

	if (  dontdrawthemap && (Y <(rc.bottom-BottomSize)) ) { 

		gestDist=isqrt4((long)((gestX*gestX) + (gestY*gestY)));

		// GESTURE DETECTION
		// if gestX >0 gesture from right to left , gestX <0 gesture from left to right
		// if gestY >0 gesture from down to up ,    gestY <0 gesture from up to down
		if (gestDist<GestureSize) { 	// TODO FIX tune this GestureSize, maybe 100?
			gestDir=LKGESTURE_NONE;
		} else {
			// horizontal includes also perfectly diagonal gestures
			if (abs(gestX) >= abs(gestY) ) {
				// we use LKGESTURE definition, but they have nothing to do with those used in other part of source code
				if (gestX>0)
					gestDir=IphoneGestures?LKGESTURE_RIGHT:LKGESTURE_LEFT;
				else
					gestDir=IphoneGestures?LKGESTURE_LEFT:LKGESTURE_RIGHT;
			} else { 
				if (gestY>0)
					gestDir=IphoneGestures?LKGESTURE_DOWN:LKGESTURE_UP;
				else
					gestDir=IphoneGestures?LKGESTURE_UP:LKGESTURE_DOWN;
			}
		}
		// end dontdrawthemap and inside mapscreen looking for a gesture
	} 

      // Process Active Icons
	if (doinit) {
		#include "./LKinclude_menusize.cpp"
		doinit=false;
	}
      if (NewMap)  {

	short topicon;
	if (DrawBottom) topicon=MapRect.bottom-MapRect.top-BottomSize-14; // 100305
		else
			topicon=MapRect.bottom-MapRect.top-AIRCRAFTMENUSIZE;

if ( (X > ((MapRect.right-MapRect.left)- AIRCRAFTMENUSIZE)) &&
   (Y > topicon) ) {

	/*
	 * Available for future usage: short click on aircraft icon.
	 * Remember there's already a normal click and a long click on the same icon!
	   090720 short click replacing doubleclick forever!
	 */
	
		// short click on aircraft icon
		//
		if ( dwInterval <= (DOUBLECLICKINTERVAL)) {
goto_menu:
			#ifndef DISABLEAUDIO
                	if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			ShowMenu();
			break;
		} else
		// Long click on aircraft icon, toggle thermal mode
		//
		if ( dwInterval >=VKLONGCLICK) { // in Defines.h
			if (DisplayMode == dmCircling) {
				UserForceDisplayMode=dmCruise;
				#ifndef DISABLEAUDIO
				if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				#endif
				break;
			} else 
			if (DisplayMode == dmCruise) {
				UserForceDisplayMode=dmNone;
				#ifndef DISABLEAUDIO
				if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				#endif
				break;
			}
		} else {
			// We are here in any case only when dwInterval is <VKLONGCLICK
			if (dwInterval >=(unsigned)CustomKeyTime) {
				if (!CustomKeyHandler(CKI_BOTTOMICON)) goto goto_menu;
			}
			break;
		}

      // end aircraft icon check				
      } 
	if (mapmode8000) { 
	if ( (X <= (MapRect.left + COMPASSMENUSIZE)) && (Y <= (MapRect.top+COMPASSMENUSIZE)) ) {
		if (!CustomKeyHandler(CKI_TOPLEFT)) {
			#ifndef DISABLEAUDIO
         		 if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			wParam = 0x26; 
			// we can have problems if fast double clicks?
			// zoom in
			InputEvents::processKey(wParam);
			return TRUE;
		}
		MapWindow::RefreshMap();
		break;
	}

      if (ISPARAGLIDER) {
	// Use the compass to pullup UTM informations to paragliders
	if ( (X > ((MapRect.right-MapRect.left)- COMPASSMENUSIZE)) && (Y <= MapRect.top+COMPASSMENUSIZE) ) {

		if ((dwInterval >= DOUBLECLICKINTERVAL) ) {

extern void LatLonToUtmWGS84 (int& utmXZone, char& utmYZone, double& easting, double& northing, double lat, double lon);

			// if we are running a real task, with gates, and we could still start
			// if only 1 time gate, and we passed valid start, no reason to resettask
			int acceptreset=2;
			if (PGNumberOfGates==1) acceptreset=1;
			if (UseGates() && ValidTaskPoint(1) && ActiveWayPoint<acceptreset) { // 100507 101110
				InputEvents::eventResetTask(_T(""));
			} else {
			int utmzone; char utmchar;
			double easting, northing;
			TCHAR mbuf[80];
			#ifndef DISABLEAUDIO
			if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			LatLonToUtmWGS84 ( utmzone, utmchar, easting, northing, GPS_INFO.Latitude, GPS_INFO.Longitude );
			_stprintf(mbuf,_T("UTM %d%c  %.0f  %.0f"), utmzone, utmchar, easting, northing);
			Message::Lock(); // 091211
			Message::AddMessage(60000, 1, mbuf);
			TCHAR sLongitude[16];
			TCHAR sLatitude[16];
			Units::LongitudeToString(GPS_INFO.Longitude, sLongitude, sizeof(sLongitude)-1);
			Units::LatitudeToString(GPS_INFO.Latitude, sLatitude, sizeof(sLatitude)-1);
			_stprintf(mbuf,_T("%s %s"), sLatitude, sLongitude);
			Message::AddMessage(60000, 1, mbuf);
			Message::Unlock();

			break;

			} // real UTM, no reset task
		}

	} // End compass icon check
      } // PARAGLIDERs special buttons
	// else not a paraglider key, process it for gliders
	else { 
		if ( (X > ((MapRect.right-MapRect.left)- COMPASSMENUSIZE)) && (Y <= MapRect.top+COMPASSMENUSIZE) ) {
			if (!CustomKeyHandler(CKI_TOPRIGHT)) {
				#ifndef DISABLEAUDIO
         			if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				#endif
				wParam = 0x26; 
				// we can have problems if fast double clicks?
				InputEvents::processKey(wParam);
				return TRUE;
			}
			MapWindow::RefreshMap();
			break;
		}
		// if not topright, continue
	}
	// do all of this only if !dontdrawthemap = we are in fullscreen and not PAN etc.
	// indentation is wrong here
	} 


	// "fast virtual keys" are handled locally and not passed to event handler.
	// they are processed even when virtual keys are disabled, because they concern special lk8000 menus.

	// First case: for mapspacemodes we manage gestures as well
	if (dontdrawthemap) {

		if ( gestDir == LKGESTURE_UP) {
			ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_UP);
			break;
		}
		if ( gestDir == LKGESTURE_DOWN) {
			ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_DOWN);
			break;
		}
		if ( gestDir == LKGESTURE_LEFT) {
			ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_LEFT);
			break;
		}
		if ( gestDir == LKGESTURE_RIGHT) {
			ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_RIGHT);
			break;
		}


		// We are here when lk8000, and NO moving map displayed: virtual enter, virtual up/down, or 
		// navbox operations including center key.
		wParam=ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_NONE);
#ifdef DEBUG_MAPINPUT
		DoStatusMessage(_T("DBG-035 navboxes")); 
#endif
		// we could use a single ProcessVirtual for all above, and check that wParam on return
		// is really correct for gestures as well... since we do not want to go to wirth with gestures!
		if (wParam!=0) {
			DoStatusMessage(_T("ERR-033 Invalid Virtual Key")); 
			break;
		}
		break;

	}
      
	// if clicking on navboxes, process fast virtual keys
	// maybe check LK8000 active?
	// This point is selected when in MapSpaceMode==MSM_MAP, i.e. lk8000 with moving map on.
	if (  DrawBottom && IsMapFullScreen() && (Y >= (rc.bottom-BottomSize)) && !MapWindow::EnablePan ) {
		wParam=ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_NONE);
#ifdef DEBUG_MAPINPUT
		DoStatusMessage(_T("DBG-034 navboxes")); 
#endif
		if (wParam!=0) {
			DoStatusMessage(_T("ERR-034 Invalid Virtual Key")); 
			break;
		}
		break;
	}



      } // end newmap preliminar checks

      if(NewMap&&UseMapLock&&MapLock&&!EnablePan&&!(dontdrawthemap)) 
      {
		// With LOCKED map...
		// When you single click on the map, here you come.
		// When you press double click to release lock, you fall here, also.

	      if ((VirtualKeys==(VirtualKeys_t)vkEnabled) && (dwInterval>= DOUBLECLICKINTERVAL)) {   
			wParam=ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_NONE);
			if (wParam==0) {
				// Here we fall when you press for example long center map or do a gesture
				// DoStatusMessage(_T("ERR-035 Invalid Virtual Key"));
				break;
			}
			// Otherwise its a vk up/down/enter and must be handled, passing vk code down
			goto Wirth; 
	      }

	  // while locked, any keypress on the map which is not a virtual key will unfocus infoboxes
	  // but first check if it was already unfocused so you will not play a click 
	  if (InfoFocus>=0) {
	  	DefocusInfoBox();
	  	SetFocus(hWnd);
		#ifndef DISABLEAUDIO
         	 if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		#endif
		 iboxtoclick=true; 
		 // DoStatusMessage(_T("Map is locked, defocus ibox")); 
	  } else {
		// newmap on, maplock on, NO VK, special case 
		if (  DrawBottom && IsMapFullScreen() && (Y >= (rc.bottom-BottomSize)) ) {
			goto Escamotage; 
		}
		// DoStatusMessage(_T("Map is locked, skipping")); 
	  }
	  break;
      }

      if (dwInterval == 0) {
		break; // should be impossible
      }

	// we need to calculate it here only if needed
	if (gestDist>=0)
		distance = gestDist /InfoBoxLayout::scale;
	else
		distance = isqrt4((long)((XstartScreen-X)*(XstartScreen-X)+ (YstartScreen-Y)*(YstartScreen-Y))) /InfoBoxLayout::scale;

	#ifdef DEBUG_VIRTUALKEYS
	TCHAR buf[80]; char sbuf[80];
	sprintf(sbuf,"%.0f",distance);
	wsprintf(buf,_T("XY=%d,%d dist=%S Up=%ld Down=%ld Int=%ld"),X,Y,sbuf,dwUpTime,dwDownTime,dwInterval);
        DoStatusMessage(buf);
	#endif

	// Handling double click passthrough
	// Caution, timed clicks from PC with a mouse are different from real touchscreen devices

      // On PC a single click is around 80ms, and a doubleclick is around 150ms. 
      // CHECK TODO FIX if only for NewMap when enabled.. VK work only with new map...so no need?

	// This is called when long press in center map i.e. for inverting colors, 
	// and for Virtual keys zoom in/out
      if ((VirtualKeys==(VirtualKeys_t)vkEnabled) && distance<50 && (dwInterval>= DOUBLECLICKINTERVAL)) { 
		wParam=ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_NONE);
		if (wParam==0) {
			#ifdef DEBUG_VIRTUALKEYS
			DoStatusMessage(_T("DBG-095 invalid Virtual Key!")); 
			#endif
			break;
		}
		//break; // TESTFIX 090930
		goto Wirth; 
      }


      // Process faster clicks here and no precision, but let DBLCLK pass through
      // VK are used in the bottom line in this case, forced on for this situation.
      if (  DrawBottom && IsMapFullScreen() && (Y >= (rc.bottom-BottomSize)) ) {
Escamotage:
		// DoStatusMessage(_T("Click on hidden map ignored")); 

		// do not process virtual key if it is timed as a DBLCLK
		// we want users to get used to double clicking only on infoboxes
		// and avoid triggering unwanted waypoints details
		if (dwInterval >= ( (DOUBLECLICKINTERVAL/2-30) )) { // fast dblclk required here.
			#ifdef DEBUG_VIRTUALKEYS // 100320
			wParam=ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_NONE);
			if (wParam==0) {
				DoStatusMessage(_T("P00 Virtual Key 0")); 
				break;
			}
			#else
			ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_NONE);
			#endif
			break; 
		}
		// do not process click on the underneath window
		break;
      }
      if (dontdrawthemap) break;

      Screen2LatLon(X, Y, Xlat, Ylat);
    
      if (AATEnabled && TargetPan && (TargetDrag_State>0)) {
	LockTaskData();
	TargetDrag_State = 2;
	TargetDrag_Latitude = Ylat;
	TargetDrag_Longitude = Xlat;
	UnlockTaskData();
	break;
      } else if (!TargetPan && EnablePan && (distance>36)) { // TODO FIX should be IBLSCALE 36 instead?
	PanLongitude += Xstart-Xlat;
	PanLatitude  += Ystart-Ylat;
	RefreshMap();
	// disable picking when in pan mode
	break; 
      } 
#if NOSIM
      else if (SIMMODE && (!TargetPan && (distance>NIBLSCALE(36)))) {
	// This drag moves the aircraft (changes speed and direction)
	double newbearing;
	double oldbearing = GPS_INFO.TrackBearing;
	double minspeed = 1.1*GlidePolar::Vminsink;
	DistanceBearing(Ystart, Xstart, Ylat, Xlat, NULL, &newbearing);
	if ((fabs(AngleLimit180(newbearing-oldbearing))<30) || (GPS_INFO.Speed<minspeed)) {
		// sink we shall be sinking, lets raise the altitude when using old simulator interface
		if ( (CALCULATED_INFO.TerrainValid) && ( CALCULATED_INFO.AltitudeAGL <0 ))
			GPS_INFO.Altitude=CALCULATED_INFO.TerrainAlt;
		GPS_INFO.Altitude+=200;
		GPS_INFO.Speed = min(100.0,max(minspeed,distance/3));
	} 
	GPS_INFO.TrackBearing = newbearing;
	TriggerGPSUpdate();
      
	break;
      }

#else
#ifdef _SIM_
      else if (!TargetPan && (distance>NIBLSCALE(36))) {
	// This drag moves the aircraft (changes speed and direction)
	double newbearing;
	double oldbearing = GPS_INFO.TrackBearing;
	double minspeed = 1.1*GlidePolar::Vminsink;
	DistanceBearing(Ystart, Xstart, Ylat, Xlat, NULL, &newbearing);
	if ((fabs(AngleLimit180(newbearing-oldbearing))<30) || (GPS_INFO.Speed<minspeed)) {
		GPS_INFO.Speed = min(100.0,max(minspeed,distance/3));
	} 
	GPS_INFO.TrackBearing = newbearing;
	TriggerGPSUpdate();
      
	break;
      }
#endif
#endif
      if (!TargetPan) {
		// if map is locked and we are here, then if infobox are under focus accept the click
		// as an order to defocus. Otherwise since we are under lock condition we simply
		// ignore the click and break out.
		//
		if (NewMap&&UseMapLock&&MapLock) { 
			if (InfoFocus>=0) {
				// DoStatusMessage(_T("Map is locked, defocus ibox")); 
				DefocusInfoBox();
				SetFocus(hWnd);
				#ifndef DISABLEAUDIO
				 if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				#endif
				iboxtoclick=true; 
			} else {
				// ignore click
				// DoStatusMessage(_T("PAN map is locked, skipping")); 
			}
			break;
		}
		//
		// We need to defocus infoboxes on demand here.  
		// Probably should be a good idea to use it also for standard old map with no VK
		// We do it also for old standard map with no VK.
		//
		if ( InfoFocus>=0) { // 
			// DoStatusMessage(_T("Defocus ibox")); 
			DefocusInfoBox();
			SetFocus(hWnd);
			#ifndef DISABLEAUDIO
			 if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			iboxtoclick=true; 
			break;
		}
		
		//
		// Finally process normally a click on the moving map.
		// Virtual keys have been processed earlier, so we are now looking for a map selection.
		// However, timings are different if virtual keys are enabled, for this operation.
		//
		if (VirtualKeys==(VirtualKeys_t)vkEnabled) {
			// Shorter the time needed to trigger a WP select, solving also the annoying problem
			// of unwanted wp selection while double clicking too slow!
			// And at the same time let this action pass transparently to virtual keys.
			//
			if(dwInterval < VKSHORTCLICK) { //100ms is NOT  enough for a short click since GetTickCount is OEM custom!
#if 100318 
			if (ActiveMap) {
				if (Event_NearestWaypointDetails(Xstart, Ystart, 500*MapScale, false)) {
					break;
				}
			} else {
savecodesize1:
			int yup, ydown, ytmp;
			ytmp=(int)((MapWindow::MapRect.bottom-MapWindow::MapRect.top-BottomSize)/2);
			yup=ytmp+MapWindow::MapRect.top;
                	ydown=MapWindow::MapRect.bottom-BottomSize-ytmp;

			if (Y<yup) {
				// pg UP = zoom in
				wParam = 0x26;
			} else {
				if (Y>ydown) {
					// pg DOWN = zoom out
					wParam = 0x28;
				} 
				else {
					// process center key, do nothing 
					break;
				}
			}
			#ifndef DISABLEAUDIO
			if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif

			InputEvents::processKey(wParam);
			dwDownTime= 0L;
			return TRUE; 
			}
#else
				if (Event_NearestWaypointDetails(Xstart, Ystart, 500*MapScale, false)) {
					break;
				}
#endif
			} else {
				// in pan mode and SIM mode, click to center current position
				#if NOSIM
				if (SIMMODE) {
					if (EnablePan) {
						// match only center screen
						if (  (abs(X-((rc.left+rc.right)/2)) <NIBLSCALE(12)) && 
						      (abs(Y-((rc.bottom+rc.top)/2)) <NIBLSCALE(12)) ) {
							// LKTOKEN  _@M204_ = "Current position updated" 
							DoStatusMessage(gettext(TEXT("_@M204_")));
							GPS_INFO.Latitude=PanLatitude;
							GPS_INFO.Longitude=PanLongitude;
							break;
						}
					}
				}
				#else
				#if _SIM_
				if (EnablePan) {
					// match only center screen
					if (  (abs(X-((rc.left+rc.right)/2)) <NIBLSCALE(5)) && 
					      (abs(Y-((rc.bottom+rc.top)/2)) <NIBLSCALE(5)) ) {
						DoStatusMessage(_T("Current position updated")); 
						GPS_INFO.Latitude=PanLatitude;
						GPS_INFO.Longitude=PanLongitude;
						break;
					}
				}
				#endif
				#endif
				// If we are here,  (DCI/2)+30 < dwDownTime < DOUBLECLICKINTERVAL
				// SO this is a tight interval. DCI should not be set too low. See Defines.h
				// NO: VKSHORTCLICK-DCI  150-350 ?
				if (!OnAirSpace) break; // 100119
				if (Event_InteriorAirspaceDetails(Xstart, Ystart)) {
					break;
				}
			}
		} else {
			if(dwInterval < AIRSPACECLICK) { // original and untouched interval
#if 100318
				if (ActiveMap) {
					if (Event_NearestWaypointDetails(Xstart, Ystart, 500*MapScale, false)) {
						break;
					}
				} else
					goto savecodesize1;
#else
				if (Event_NearestWaypointDetails(Xstart, Ystart, 500*MapScale, false)) {
					break;
				}
#endif
			} else {
				#if NOSIM
				if (SIMMODE) {
					if (EnablePan) {
						// match only center screen
						if (  (abs(X-((rc.left+rc.right)/2)) <NIBLSCALE(5)) && 
						      (abs(Y-((rc.bottom+rc.top)/2)) <NIBLSCALE(5)) ) {
	// LKTOKEN  _@M204_ = "Current position updated" 
							DoStatusMessage(gettext(TEXT("_@M204_")));
							GPS_INFO.Latitude=PanLatitude;
							GPS_INFO.Longitude=PanLongitude;
							break;
						}
					}
				}
				#else
				#if _SIM_
				if (EnablePan) {
					// match only center screen
					if (  (abs(X-((rc.left+rc.right)/2)) <NIBLSCALE(5)) && 
					      (abs(Y-((rc.bottom+rc.top)/2)) <NIBLSCALE(5)) ) {
						DoStatusMessage(_T("Current position updated"));
						GPS_INFO.Latitude=PanLatitude;
						GPS_INFO.Longitude=PanLongitude;
						break;
					}
				}
				#endif
				#endif
				if (!OnAirSpace) break; // 100119
				if (Event_InteriorAirspaceDetails(Xstart, Ystart)) {
					break;
				}
			}
		} // VK enabled
      } // !TargetPan

      break;
      /*
	case WM_PAINT:
	if ((hWnd == hWndMapWindow) && (ProgramStarted==3)) {
	//    RequestFastRefresh();
	return TRUE;
	} else {
	break;
	}
      */




#if defined(GNAV) || defined(PNA) // VENTA FIXED PNA SCROLL WHEEL 
    case WM_KEYDOWN: // JMW was keyup
#else
    case WM_KEYUP: // JMW was keyup
#endif
      // VENTA-TODO careful here, keyup no more trapped for PNA. 
      // Forbidden usage of keypress timing.

#ifdef VENTA_DEBUG_KEY
      TCHAR ventabuffer[80];
      wsprintf(ventabuffer,TEXT("WMKEY uMsg=%d wParam=%ld lParam=%ld"), uMsg, wParam,lParam);
      DoStatusMessage(ventabuffer);
#endif
      DisplayTimeOut = 0;
      InterfaceTimeoutReset();

#if defined(PNA) // VENTA-ADDON HARDWARE KEYS TRANSCODING

      if ( GlobalModelType == MODELTYPE_PNA_HP31X )
	{
	  //		if (wParam == 0x7b) wParam=0xc1;  // VK_APP1 	
	  if (wParam == 0x7b) wParam=0x1b;  // VK_ESCAPE
	  //		if (wParam == 0x7b) wParam=0x27;  // VK_RIGHT
	  //		if (wParam == 0x7b) wParam=0x25;  // VK_LEFT
	} else
	if ( GlobalModelType == MODELTYPE_PNA_PN6000 )
	  {
	    switch(wParam) {
	    case 0x79:					// Upper Silver key short press
	      wParam = 0xc1;	// F10 -> APP1
	      break;
	    case 0x7b:					// Lower Silver key short press
	      wParam = 0xc2;	// F12 -> APP2
	      break;
	    case 0x72:					// Back key plus
	      wParam = 0xc3;	// F3  -> APP3
	      break;
	    case 0x71:					// Back key minus
	      wParam = 0xc4;	// F2  -> APP4
	      break;
	    case 0x7a:					// Upper silver key LONG press
	      wParam = 0x70;	// F11 -> F1
	      break;
	    case 0x7c:					// Lower silver key LONG press
	      wParam = 0x71;	// F13 -> F2
	      break;
	    }
	  }
      if ( GlobalModelType == MODELTYPE_PNA_NOKIA_500 )
	{
	  switch(wParam) {
	  case 0xc1:				
	    wParam = 0x0d;	// middle key = enter
	    break;
	  case 0xc5:				
	    wParam = 0x26;	// + key = pg Up
	    break;
	  case 0xc6:				
	    wParam = 0x28;	// - key = pg Down
	    break;
	  }
	}
      if ( GlobalModelType == MODELTYPE_PNA_MEDION_P5 )
	{
	  switch(wParam) {
	  case 0x79:				
	    wParam = 0x0d;	// middle key = enter
	    break;
	  case 0x75:				
	    wParam = 0x26;	// + key = pg Up
	    break;
	  case 0x76:				
	    wParam = 0x28;	// - key = pg Down
	    break;
	  }
	}

#endif


#if defined(GNAV)
      if (wParam == 0xF5){

	if (MessageBoxX(hWnd,
			TEXT("Shutdown?"),
			TEXT("Altair system message"),
			MB_YESNO|MB_ICONQUESTION) == IDYES) {

	  SendMessage(hWnd, 
		      WM_ACTIVATE, 
		      MAKEWPARAM(WA_INACTIVE, 0), 
		      (LPARAM)hWndMainWindow);
	  SendMessage (hWndMainWindow, WM_CLOSE, 0, 0);
	}

	break;

      }
#endif
Wirth:
#ifdef DEBUG_MAPINPUT
	DoStatusMessage(_T("Wirth"));
#endif
      dwDownTime= 0L;

      if (!DialogActive) { // JMW prevent keys being trapped if dialog is active
	if (InputEvents::processKey(wParam)) {
	  // TODO code: change to debugging DoStatusMessage(TEXT("Event in default"));
	}
	// XXX Should we only do this if it IS processed above ?
	dwDownTime= 0L;
	return TRUE; // don't go to default handler
      } else {
	// TODO code: debugging DoStatusMessage(TEXT("Event in dialog"));
	if (InputEvents::processKey(wParam)) {
	}
	dwDownTime= 0L;
	return TRUE; // don't go to default handler
      }
      // break; unreachable!
    }

  return (DefWindowProc (hWnd, uMsg, wParam, lParam));
}


void MapWindow::ModifyMapScale(void) {
  // limit zoomed in so doesn't reach silly levels
  RequestMapScale = LimitMapScale(RequestMapScale); // FIX VENTA remove limit
  MapScaleOverDistanceModify = RequestMapScale/DISTANCEMODIFY;
  ResMapScaleOverDistanceModify = 
    GetMapResolutionFactor()/MapScaleOverDistanceModify;
  DrawScale = MapScaleOverDistanceModify;
  DrawScale = DrawScale/111194;
  DrawScale = GetMapResolutionFactor()/DrawScale;
  InvDrawScale = 1.0/DrawScale;
  MapScale = RequestMapScale;
}


bool MapWindow::isTargetPan(void) {
  return TargetPan;
}


void MapWindow::UpdateMapScale()
{
  static int AutoMapScaleWaypointIndex = -1;
  static double StartingAutoMapScale=0.0;
  double AutoZoomFactor;

  bool useraskedforchange = false;

  // if there is user intervention in the scale
  if(MapScale != RequestMapScale) {
	ModifyMapScale();
	useraskedforchange = true;
  }

  double wpd;
  if (TargetPan) {
	wpd = TargetZoomDistance;
  } else {
	wpd = DerivedDrawInfo.ZoomDistance; 
  }
  if (TargetPan) {
	// set scale exactly so that waypoint distance is the zoom factor across the screen
	RequestMapScale = LimitMapScale(wpd *DISTANCEMODIFY/ 4.0);
	ModifyMapScale();
	return;
  } 
  
  if (AutoZoom) {
	if(wpd > 0) {
      
		if(
		   (((DisplayOrientation == NORTHTRACK)
		     &&(DisplayMode != dmCircling))
		    ||(DisplayOrientation == NORTHUP) 
		    ||(DisplayOrientation == NORTHSMART)  // 100419
		    || 
		    (((DisplayOrientation == NORTHCIRCLE) 
		      || (DisplayOrientation == TRACKCIRCLE)) 
		     && (DisplayMode == dmCircling) ))
		   && !TargetPan
		   )
		{
	 		AutoZoomFactor = 2.5;
		} else {
			AutoZoomFactor = 4;
		}
      
		if(
		  (wpd < ( AutoZoomFactor * MapScaleOverDistanceModify))
		  || 
	  	  (StartingAutoMapScale==0.0)) 
		{
			// waypoint is too close, so zoom in
			// OR just turned waypoint

			// this is the first time this waypoint has gotten close,
			// so save original map scale

			if (StartingAutoMapScale==0.0) {
				StartingAutoMapScale = MapScale;
			}
			else { // 101007 BUGFIX XCSOAR

				// set scale exactly so that waypoint distance is the zoom factor across the screen
				RequestMapScale = LimitMapScale(wpd *DISTANCEMODIFY/ AutoZoomFactor);
				if (MapScale != RequestMapScale) { // do not loose time if same scale
					ModifyMapScale();
				}
			}
		} else {
			if (useraskedforchange) {
				// user asked for a zoom change and it was achieved, so reset starting map scale
			}

		}
      } // wpd>0
  } else { // !AutoZoom
    
	// reset starting map scale for auto zoom if momentarily switch
	// off autozoom
	// StartingAutoMapScale = RequestMapScale;
	StartingAutoMapScale=0; //@ 101007 BUGFIX we need to reset it to let current mapscale be used on next azoom on
  }

  if (TargetPan) {
	return;
  }

  LockTaskData();  // protect from external task changes
#ifdef HAVEEXCEPTIONS
  __try{
#endif
    // if we aren't looking at a waypoint, see if we are now
    if (AutoMapScaleWaypointIndex == -1) {
	if (ValidTaskPoint(ActiveWayPoint)) {
		AutoMapScaleWaypointIndex = Task[ActiveWayPoint].Index;
	}
    }

    if (ValidTaskPoint(ActiveWayPoint)) {

	// if the current zoom focused waypoint has changed...
	if (AutoMapScaleWaypointIndex != Task[ActiveWayPoint].Index) {

		AutoMapScaleWaypointIndex = Task[ActiveWayPoint].Index;

		// zoom back out to where we were before
		if (StartingAutoMapScale> 0.0) {
			RequestMapScale = StartingAutoMapScale;
		}

		// reset search for new starting zoom level
		StartingAutoMapScale = 0.0;
	}

    }
#ifdef HAVEEXCEPTIONS
  }__finally
#endif
     {
       UnlockTaskData();
     }

}


bool MapWindow::GliderCenter=false;


void MapWindow::CalculateOrientationNormal(void) {
  double trackbearing = DrawInfo.TrackBearing;
  //  trackbearing = DerivedDrawInfo.NextTrackBearing;

  if( (DisplayOrientation == NORTHUP) 
      ||
      ((DisplayOrientation == NORTHTRACK)
       &&(DisplayMode != dmCircling))
	|| (DisplayOrientation == NORTHSMART)  // 100419
      || 
      (
       ((DisplayOrientation == NORTHCIRCLE)
        ||(DisplayOrientation==TRACKCIRCLE))
       && (DisplayMode == dmCircling) )
      ) {
#ifndef NEWMOVEICON
    GliderCenter = true;
#else
	if (DisplayMode == dmCircling)
		GliderCenter=true;
	else
		GliderCenter=false;
#endif
    
    if (DisplayOrientation == TRACKCIRCLE) {
      DisplayAngle = DerivedDrawInfo.WaypointBearing;
      DisplayAircraftAngle = trackbearing-DisplayAngle;
    } else {
      DisplayAngle = 0.0;
      DisplayAircraftAngle = trackbearing;
    }
  } else {
    // normal, glider forward
    GliderCenter = false;
    DisplayAngle = trackbearing;
    DisplayAircraftAngle = 0.0;    
  }
  DisplayAngle = AngleLimit360(DisplayAngle);
  DisplayAircraftAngle = AngleLimit360(DisplayAircraftAngle);
}


void MapWindow::CalculateOrientationTargetPan(void) {
  // Target pan mode, show track up when looking at current task point,
  // otherwise north up.  If circling, orient towards target.
  GliderCenter = true;
  if ((ActiveWayPoint==TargetPanIndex)
      &&(DisplayOrientation != NORTHUP)
      &&(DisplayOrientation != NORTHSMART) // 100419
      &&(DisplayOrientation != NORTHTRACK)
      )    {
    if (DisplayMode == dmCircling) {
      // target-up
      DisplayAngle = DerivedDrawInfo.WaypointBearing;
      DisplayAircraftAngle = 
        DrawInfo.TrackBearing-DisplayAngle;
    } else {
      // track up
      DisplayAngle = DrawInfo.TrackBearing;
      DisplayAircraftAngle = 0.0;
    }
  } else {
    // North up
    DisplayAngle = 0.0;
    DisplayAircraftAngle = DrawInfo.TrackBearing;
  }
 
}


void MapWindow::CalculateOrigin(const RECT rc, POINT *Orig)
{
  if (TargetPan) {
	CalculateOrientationTargetPan();
  } else {
	CalculateOrientationNormal();
  }
  
  if ( EnablePan || DisplayMode==dmCircling) {
	Orig->x = (rc.left + rc.right)/2;
	Orig->y = (rc.bottom + rc.top)/2;
  } else {
	#if NEWMOVEICON
	#if 100415
	// automagic northup smart
	if (DisplayOrientation == NORTHSMART) { 
		double trackbearing = DrawInfo.TrackBearing;
		int middleXY,spanxy;
		if (ScreenLandscape) {
			middleXY=((rc.bottom-BottomSize)+rc.top)/2;
			spanxy=NIBLSCALE(50);
			Orig->y= middleXY + (int)(spanxy*fastcosine(trackbearing));
			// This was moving too much the map!
			// spanx=NIBLSCALE(40);
			// Orig->x= middleX - (int)(spanx*fastsine(trackbearing));
			Orig->x = (rc.left + rc.right)/2;
		} else {
			middleXY=(rc.left+rc.right)/2;
			spanxy=NIBLSCALE(50);
			Orig->x= middleXY - (int)(spanxy*fastsine(trackbearing));
			Orig->y = ((rc.bottom-BottomSize) + rc.top)/2;
		}
/* REMOVE

		//double spany=(rc.bottom-BottomSize)-NIBLSCALE(40)- middleY;
		if (InfoBoxLayout::landscape) {
			spany=NIBLSCALE(50);
			Orig->y= middleY + (int)(spany*fastcosine(trackbearing));
			Orig->x = (rc.left + rc.right)/2;
		} else { 
			spanx=NIBLSCALE(40);
			Orig->x= middleX + (int)(spany*fastcosine(trackbearing));
			Orig->y = ((rc.bottom-BottomSize) + rc.top)/2;
		}
*/
	} else {
/*
		if (DisplayOrientation == NORTHUP) { 
			Orig->x = (rc.left + rc.right)/2;
			Orig->y = (rc.bottom + rc.top)/2;
		} else {
			Orig->x = ((rc.right - rc.left )*GliderScreenPositionX/100)+rc.left;
			Orig->y = ((rc.top - rc.bottom )*GliderScreenPositionY/100)+rc.bottom;
		}
*/
		// 100924 if we are in north up autorient, position the glider in middle screen
		if ((MapScale*1.4) >= AutoOrientScale) {
			Orig->x = (rc.left + rc.right)/2;
			Orig->y=((rc.bottom-BottomSize)+rc.top)/2;
		} else {
			// else do it normally using configuration
			Orig->x = ((rc.right - rc.left )*GliderScreenPositionX/100)+rc.left;
			Orig->y = ((rc.top - rc.bottom )*GliderScreenPositionY/100)+rc.bottom;
		}
	}
	#else // no 100415
		Orig->x = ((rc.right - rc.left )*GliderScreenPositionX/100)+rc.left;
		Orig->y = ((rc.top - rc.bottom )*GliderScreenPositionY/100)+rc.bottom;
	#endif
	#else
	Orig->x = (rc.left + rc.right)/2;
	Orig->y = ((rc.top - rc.bottom )*GliderScreenPosition/100)+rc.bottom;
	#endif
  }
}


bool MapWindow::RenderTimeAvailable() {
  DWORD fpsTime = ::GetTickCount();
  if (MapDirty) return false;

  if (fpsTime-timestamp_newdata<700) { 
    // it's been less than 700 ms since last data
    // was posted
    return true;
  } else {
    return false;
  }
}


void MapWindow::DrawThermalEstimate(HDC hdc, const RECT rc) {
  POINT screen;
  HPEN oldPen;
  /*
  #if NOSIM
  static short counter=0;
  #else
  #ifdef _SIM_
  static short counter=0;
  #endif
  #endif
  */
  if (!EnableThermalLocator) return;

  if (DisplayMode == dmCircling) {
	if (DerivedDrawInfo.ThermalEstimate_R>0) {
		LatLon2Screen(DerivedDrawInfo.ThermalEstimate_Longitude, DerivedDrawInfo.ThermalEstimate_Latitude, screen);
		DrawBitmapIn(hdc, screen, hBmpThermalSource);

		SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		oldPen=(HPEN)SelectObject(hdc, hpThermalCircle); // white
		if (ISPARAGLIDER) {
			Circle(hdc, screen.x, screen.y, (int)(50*ResMapScaleOverDistanceModify), rc); //@ 101101
		} else {
			Circle(hdc, screen.x, screen.y, (int)(100*ResMapScaleOverDistanceModify), rc); //@ 101101
			SelectObject(hdc, hpAircraftBorder); 
			Circle(hdc, screen.x, screen.y, (int)(100*ResMapScaleOverDistanceModify)+NIBLSCALE(2), rc); //@ 101101
			Circle(hdc, screen.x, screen.y, (int)(100*ResMapScaleOverDistanceModify), rc); //@ 101101
		}
/* 101219 This would display circles around the simulated thermal, but people is confused.
		#if NOSIM
		if (SIMMODE && (ThLatitude>1 && ThLongitude>1)) { // there's a thermal to show
			if ((counter==5 || counter==6|| counter==7)) {
				LatLon2Screen(ThLongitude, ThLatitude, screen);
				SelectObject(hdc, hSnailPens[7]);  
				Circle(hdc, screen.x, screen.y, (int)(ThermalRadius*ResMapScaleOverDistanceModify), rc); 
				SelectObject(hdc, hSnailPens[7]); 
				Circle(hdc, screen.x, screen.y, (int)((ThermalRadius+SinkRadius)*ResMapScaleOverDistanceModify), rc); 
			}
			if (++counter>=60) counter=0;
		}
		#else
		#ifdef _SIM_	//@ 101104
		if (ThLatitude>1 && ThLongitude>1) { // there's a thermal to show
			if (counter==5 || counter==6|| counter==7) {
				LatLon2Screen(ThLongitude, ThLatitude, screen);
				SelectObject(hdc, hSnailPens[7]);  
				Circle(hdc, screen.x, screen.y, (int)(ThermalRadius*ResMapScaleOverDistanceModify), rc); 
				SelectObject(hdc, hSnailPens[7]); 
				Circle(hdc, screen.x, screen.y, (int)((ThermalRadius+SinkRadius)*ResMapScaleOverDistanceModify), rc); 
			}
			if (++counter>=30) counter=0;
		}
		#endif
		#endif
 */
		SelectObject(hdc,oldPen);
	}
  } else {
	if (MapScale <= 4) {
		for (int i=0; i<MAX_THERMAL_SOURCES; i++) {
			if (DerivedDrawInfo.ThermalSources[i].Visible) {
				DrawBitmapIn(hdc, DerivedDrawInfo.ThermalSources[i].Screen, hBmpThermalSource);
			}
		}
	}
  }
}


void MapWindow::RenderMapWindowBg(HDC hdc, const RECT rc,
				  const POINT &Orig,
				  const POINT &Orig_Aircraft)
{
  HFONT hfOld;


  static bool alreadyTriggered=false;
  //static double lastTrigger=0;
  static double savedMapScale=0;
  static double savedRequestMapScale=0;
  static double savedMapScaleOverDistanceModify=0;

  // do slow calculations before clearing the screen
  // to reduce flicker
#ifdef LK8000_OPTIMIZE
  #ifdef FLIPFLOP
  static bool flipflop=true;
  if (flipflop) {
	CalculateWaypointReachableNew();
	flipflop=false;
  } else flipflop=true;
  #else
  CalculateWaypointReachableNew();
  #endif
#else
  CalculateWaypointReachable();
#endif
  CalculateScreenPositionsAirspace();
  CalculateScreenPositionsThermalSources();
  CalculateScreenPositionsGroundline();

  if (PGZoomTrigger) {
	if (!alreadyTriggered) {
		alreadyTriggered=true;
		LastZoomTrigger=GPS_INFO.Time;
		savedMapScale=MapWindow::MapScale;
		savedRequestMapScale=RequestMapScale;
		savedMapScaleOverDistanceModify=MapScaleOverDistanceModify;
		// maybe todo check mode and remember where were these parameters taken from.. 
		if (ISPARAGLIDER) // 100316
			Event_SetZoom(5.0);
		else
			Event_SetZoom(7.0);
		Message::Lock(); // 091211
	        Message::AddMessage(1000, 3, _T("LANDSCAPE ZOOM for 20\"")); // FIXV2
		Message::Unlock();
      		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_TONEUP"));
		#endif
	} else {
		// previously called, see if time has passed
		if ( GPS_INFO.Time > (LastZoomTrigger + 20.0)) {
			// time has passed, lets go back
			Event_SetZoom(savedRequestMapScale);
			LastZoomTrigger=0; // just for safety
			alreadyTriggered=false;
			PGZoomTrigger=false;
			Message::Lock(); // 091211
	        	Message::AddMessage(1500, 3, _T("BACK TO NORMAL ZOOM")); // FIXV2
			Message::Unlock();
      			#ifndef DISABLEAUDIO
			if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_TONEDOWN"));
			#endif
		}
	}
  }
	

  // let the calculations run, but dont draw anything but the look8000 when in MapSpaceMode != MSM_MAP
  if (DONTDRAWTHEMAP) 
  {
QuickRedraw: // 100318 speedup redraw
	DrawLook8000(hdc,rc);
#ifdef CPUSTATS
	DrawCpuStats(hdc,rc);
#endif
#ifdef DRAWDEBUG
	DrawDebug(hdc,rc);
#endif
	// no need to do SelectObject as at the bottom of function
	return;
  }

  // When no terrain is painted, set a background0
  // Remember that in this case we have plenty of cpu time to spend for best result
  if (!EnableTerrain || !DerivedDrawInfo.TerrainValid || !RasterTerrain::isTerrainLoaded() ) {

    // display border and fill background..
	if(InfoWindowActive) {
		SelectObject(hdc, hInvBackgroundBrush[BgMapColor]); 
		SelectObject(hdc, GetStockObject(BLACK_PEN));
	} else {
		// Here we are if no terrain is used or available
		if (INVERTCOLORS) { 
			SelectObject(hdc, hInvBackgroundBrush[BgMapColor]);
			SelectObject(hdc, GetStockObject(WHITE_PEN));
		} else {
			SelectObject(hdc, hInvBackgroundBrush[BgMapColor]);
			SelectObject(hdc, GetStockObject(WHITE_PEN));
		}
	}
	Rectangle(hdc,rc.left,rc.top,rc.right,rc.bottom);
	// We force LK painting black values on screen depending on the background color in use
	// TODO make it an array once settled
	// blackscreen would force everything to be painted white, instead
	LKTextBlack=BgMapColorTextBlack[BgMapColor];
	if (BgMapColor>6 ) BlackScreen=true; else BlackScreen=false; 
  } else {
	LKTextBlack=false;
	BlackScreen=false;
  }
  
  SelectObject(hdc, GetStockObject(BLACK_BRUSH));
  SelectObject(hdc, GetStockObject(BLACK_PEN));
  hfOld = (HFONT)SelectObject(hdc, MapWindowFont);
  
  // ground first...
  
  if (BigZoom) {
    BigZoom = false;
  }
  
  if (DONTDRAWTHEMAP) { // 100319
	SelectObject(hdcDrawWindow, hfOld);
	goto QuickRedraw;
  }

  if ((EnableTerrain && (DerivedDrawInfo.TerrainValid) 
       && RasterTerrain::isTerrainLoaded())
      || RasterTerrain::render_weather) {
	// sunelevation is never used, it is still a todo in Terrain
	double sunelevation = 40.0;
	#if 0
	double sunazimuth = DisplayAngle-DerivedDrawInfo.WindBearing;
	// draw sun from constant angle if very low wind speed
	if (DerivedDrawInfo.WindSpeed<0.5) {
		sunazimuth = DisplayAngle + 45.0;
	} 
	#else
	// 101013 XCSOAR BUGFIX SUNAZIMUTH
	double sunazimuth=GetAzimuth();
	#endif

    if (MapDirty) {
      // map has been dirtied since we started drawing, so hurry up
      BigZoom = true;
    }

    LockTerrainDataGraphics();
 	if (DONTDRAWTHEMAP) { // 100318
		UnlockTerrainDataGraphics();
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
    DrawTerrain(hdc, rc, sunazimuth, sunelevation); // LOCKED 091105
 	if (DONTDRAWTHEMAP) { // 100318
		UnlockTerrainDataGraphics();
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
    if ((FinalGlideTerrain==2) && DerivedDrawInfo.TerrainValid) {
      DrawTerrainAbove(hdc, rc);
    }
    UnlockTerrainDataGraphics();
  }

 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
  if (EnableTopology) {
    DrawTopology(hdc, rc); // LOCKED 091105
  }
  #if 0
  StartupStore(_T("... Experimental1=%.0f\n"),Experimental1);
  StartupStore(_T("... Experimental2=%.0f\n"),Experimental2);
  Experimental1=0.0;
  Experimental2=0.0;
  #endif

  // Topology labels are printed first, using OLD wps positions from previous run!
  // Reset for topology labels decluttering engine occurs also in another place here!

  nLabelBlocks = 0;
  #if TOPOFASTLABEL
  for (short nvi=0; nvi<SCREENVSLOTS; nvi++) nVLabelBlocks[nvi]=0;
  #endif
  
  #ifndef NOTASKABORT 
  if (!TaskIsTemporary()) {
    DrawTaskAAT(hdc, rc);
  }
  #else
  if (ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) { // 100503
	DrawTaskAAT(hdc, rc);
  }
  #endif

  
 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
  if ( OnAirSpace >0 ) DrawAirSpace(hdc, rc); // VENTA3 default is true, always true at startup no regsave

 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
  
  if(TrailActive) {
    // TODO enhancement: For some reason, the shadow drawing of the 
    // trail doesn't work in portrait mode.  No idea why.

      double TrailFirstTime = 
#ifdef NEWTRAIL
	LKDrawTrail(hdc, Orig_Aircraft, rc);
#else
	DrawTrail(hdc, Orig_Aircraft, rc);
#endif
      DrawTrailFromTask(hdc, rc, TrailFirstTime);
  }

 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}

  DrawThermalEstimate(hdc, rc);
 
  #ifdef NOTASKABORT 
  if (ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) { // 100503
	DrawTask(hdc, rc, Orig_Aircraft);
  }
  #else
  if (TaskAborted) {
    DrawAbortedTask(hdc, rc, Orig_Aircraft);
  } else {
    DrawTask(hdc, rc, Orig_Aircraft);
  }
  #endif
  
  // draw red cross on glide through terrain marker
  if (FinalGlideTerrain && DerivedDrawInfo.TerrainValid) {
    DrawGlideThroughTerrain(hdc, rc);
  }
  
 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}

  #ifdef LK8000_OPTIMIZE
  DrawWaypointsNew(hdc,rc);
  #else
  if (NewMap) DrawWaypointsNew(hdc,rc);
  else DrawWaypoints(hdc,rc);
  #endif

 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}

  DrawTeammate(hdc, rc);
 
//  VENTA 090711 TEST disabled DrawSpotHeights QUI
// watchout for upcoming SSA errors in the gcc compiler if not using #if 0
#if (0)
  if ((EnableTerrain && (DerivedDrawInfo.TerrainValid))
      || RasterTerrain::render_weather) {
    DrawSpotHeights(hdc);
  } 
#endif
  
  if (extGPSCONNECT) {
    // TODO enhancement: don't draw offtrack indicator if showing spot heights
    DrawProjectedTrack(hdc, rc, Orig_Aircraft);
    #ifndef LK8000_OPTIMIZE
    DrawOffTrackIndicator(hdc, rc);
    #endif
    DrawBestCruiseTrack(hdc, Orig_Aircraft);
    DrawBearing(hdc, rc);
  }

  // draw wind vector at aircraft
  if (!EnablePan) {
    DrawWindAtAircraft2(hdc, Orig_Aircraft, rc);
  } else if (TargetPan) {
    DrawWindAtAircraft2(hdc, Orig, rc);
  }

  // VisualGlide drawn BEFORE lk8000 overlays
  if ( (!TargetPan) && (!EnablePan) && (VisualGlide>0) ) {
    DrawGlideCircle(hdc, Orig, rc); 
  }

 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}

  // Draw traffic and other specifix LK gauges
  if (Look8000) { // 091111
  	LKDrawFLARMTraffic(hdc, rc, Orig_Aircraft);
	if ( !EnablePan) DrawLook8000(hdc,rc); 
	if (LKVarioBar && IsMapFullScreen() && !EnablePan) // 091214 do not draw Vario when in Pan mode
		LKDrawVario(hdc,rc); // 091111
  #ifdef LK8000_OPTIMIZE
  }
  #else
  } else {
	DrawFLARMTraffic(hdc, rc, Orig_Aircraft);
  }
  #endif
  
  // finally, draw you!
  // Draw cross air for panmode, instead of aircraft icon
  if (EnablePan && !TargetPan) {
    DrawCrossHairs(hdc, Orig, rc);
  }

  // Draw glider or paraglider
  if (extGPSCONNECT) {
    DrawAircraft(hdc, Orig_Aircraft);
  }

  if ( (!TargetPan) && (!EnablePan) && (Look8000)  ) {
	if (TrackBar) DrawHeading(hdc, Orig, rc); 
  }

  // marks on top...
  DrawMarks(hdc, rc);

  if (ISGAAIRCRAFT) DrawHSI(hdc,Orig,rc); 

#ifdef CPUSTATS
  DrawCpuStats(hdc,rc);
#endif
#ifdef DRAWDEBUG
  DrawDebug(hdc,rc);
#endif
  SelectObject(hdcDrawWindow, hfOld);

}


void MapWindow::RenderMapWindow(  RECT rc)
{
  bool drawmap = false;
  HFONT hfOld;
  DWORD fpsTime = ::GetTickCount();

  // only redraw map part every 800 s unless triggered
  if (((fpsTime-fpsTime0)>800)||(fpsTime0== 0)||(userasked)) {
    fpsTime0 = fpsTime;
    drawmap = true;
    userasked = false;
  }
  MapWindow::UpdateTimeStats(true);
  
  POINT Orig, Orig_Aircraft;

  #if AUTORIENT
  SetAutoOrientation(false); // false for no reset Old values
  #endif  
  CalculateOrigin(rc, &Orig);

  // this is calculating waypoint visible, and must be executed before rendermapwindowbg which calls   
  // CalculateWayPointReachable new, setting values for visible wps!
  // This is also calculating CalculateScreenBounds 0.0  and placing it inside MapWindow::screenbounds_latlon
  CalculateScreenPositions(Orig, rc, &Orig_Aircraft);

  RenderMapWindowBg(hdcDrawWindow, rc, Orig, Orig_Aircraft);

  if (DONTDRAWTHEMAP) {
  	DrawFlightMode(hdcDrawWindow, rc);
  	DrawGPSStatus(hdcDrawWindow, rc);

	return;
  }
  // overlays
  #ifndef NOCDIGAUGE
  DrawCDI();
  #endif

  hfOld = (HFONT)SelectObject(hdcDrawWindow, MapWindowFont);
  
  DrawMapScale(hdcDrawWindow,rc, BigZoom);

  DrawCompass(hdcDrawWindow, rc);

  // JMW Experimental only! EXPERIMENTAL
#if 0
  //  #ifdef GNAV
  if (EnableAuxiliaryInfo) {
//    DrawHorizon(hdcDrawWindow, rc);
  }
  //  #endif
#endif

  DrawFlightMode(hdcDrawWindow, rc);

  // REMINDER TODO let it be configurable for not circling also, as before
  if (!(NewMap && Look8000) || (DisplayMode == dmCircling) )
	if (ThermalBar) DrawThermalBand(hdcDrawWindow, rc); // 091122


  if (!EnablePan) // 091214
  DrawFinalGlide(hdcDrawWindow,rc);

  // DrawSpeedToFly(hdcDrawWindow, rc);  // Usable

  DrawGPSStatus(hdcDrawWindow, rc);

  /*
   * This may not be the correct place for locking map. 
   * In fact we just need once in a second somewhere to check for the following:
   * 	if infobox are under focus and maplocking active then keep it locked
   */
  if ( InfoWindowActive && UseMapLock && NewMap ) {
              LockMap();
  }


  SelectObject(hdcDrawWindow, hfOld);

}


void MapWindow::UpdateInfo(NMEA_INFO *nmea_info,
                           DERIVED_INFO *derived_info) {
  LockFlightData();
  memcpy(&DrawInfo,nmea_info,sizeof(NMEA_INFO));
  memcpy(&DerivedDrawInfo,derived_info,sizeof(DERIVED_INFO));
  UpdateMapScale(); // done here to avoid double latency due to locks 
  UnlockFlightData();
}


void MapWindow::UpdateCaches(bool force) {
  // map was dirtied while we were drawing, so skip slow process
  // (unless we haven't done it for 2000 ms)
  DWORD fpsTimeThis;
  static DWORD fpsTimeMapCenter = 0;


  if (MapWindow::ForceVisibilityScan) {
    force = true;
    MapWindow::ForceVisibilityScan = false;
  }

  // have some time, do shape file cache update if necessary
  LockTerrainDataGraphics();
  SetTopologyBounds(MapRect, force);
  UnlockTerrainDataGraphics();

  // JMW experimental jpeg2000 rendering/tile management
  // Must do this even if terrain is not displayed, because
  // raster terrain is used by terrain footprint etc.

  fpsTimeThis = ::GetTickCount(); // 100115
  if (force || ( (fpsTimeThis - fpsTimeMapCenter) > 5000)) {

    fpsTimeMapCenter=fpsTimeThis; 
    RasterTerrain::ServiceTerrainCenter(DrawInfo.Latitude, 
                                        DrawInfo.Longitude);
  }
  
  fpsTimeThis = ::GetTickCount();
  static DWORD fpsTimeLast_terrain=0;


#ifdef LK8000_OPTIMIZE  	// 100115
  if (EnableTerrain) {
	if (RenderTimeAvailable() || ((fpsTimeThis-fpsTimeLast_terrain)>5000) || force) {
		fpsTimeLast_terrain = fpsTimeThis;
		RasterTerrain::ServiceCache();
	}
  }
#else
  if (RenderTimeAvailable() ||
      (fpsTimeThis-fpsTimeLast_terrain>5000) || force) {
    // have some time, do graphics terrain cache update if necessary
    if (EnableTerrain) {
      fpsTimeLast_terrain = fpsTimeThis;
      RasterTerrain::ServiceCache();
    }
  }
#endif
}


DWORD MapWindow::DrawThread (LPVOID lpvoid)
{

#ifdef CPUSTATS
  FILETIME CreationTime, ExitTime, StartKernelTime, EndKernelTime, StartUserTime, EndUserTime ;
#endif


  while ((!ProgramStarted) || (!Initialised)) {
    Sleep(100);
  }

  //  THREADRUNNING = FALSE;
  THREADEXIT = FALSE;

  // Reset for topology labels decluttering engine occurs also in another place here!
  nLabelBlocks = 0;
  #if TOPOFASTLABEL
  for (short nvi=0; nvi<SCREENVSLOTS; nvi++) nVLabelBlocks[nvi]=0;
  #endif

  GetClientRect(hWndMapWindow, &MapRectBig);

  UpdateTimeStats(true);
  
  MapRectSmall = MapRect;
  MapRect = MapRectSmall;
  
  SetBkMode(hdcDrawWindow,TRANSPARENT);
  SetBkMode(hDCTemp,OPAQUE);
  SetBkMode(hDCMask,OPAQUE);

  // paint draw window black to start
  SelectObject(hdcDrawWindow, GetStockObject(BLACK_PEN));
  Rectangle(hdcDrawWindow,MapRectBig.left,MapRectBig.top,
            MapRectBig.right,MapRectBig.bottom);

  BitBlt(hdcScreen, 0, 0, MapRectBig.right-MapRectBig.left,
         MapRectBig.bottom-MapRectBig.top, 
         hdcDrawWindow, 0, 0, SRCCOPY);

  // This is just here to give fully rendered start screen
  UpdateInfo(&GPS_INFO, &CALCULATED_INFO);
  MapDirty = true;
  UpdateTimeStats(true);
  //

  RequestMapScale = MapScale;
  ModifyMapScale();
  
  bool first = true;

  for (int i=0; i<AIRSPACECLASSCOUNT; i++) {
    hAirspacePens[i] =
      CreatePen(PS_SOLID, NIBLSCALE(2), Colours[iAirspaceColour[i]]);
  }

  while (!CLOSETHREAD) 
    {
      WaitForSingleObject(drawTriggerEvent, 5000);
      ResetEvent(drawTriggerEvent);
      if (CLOSETHREAD) break; // drop out without drawing

      if ((!THREADRUNNING) || (!GlobalRunning)) {
	Sleep(100);
	continue;
      }

#ifdef CPUSTATS
	GetThreadTimes( hDrawThread, &CreationTime, &ExitTime,&StartKernelTime,&StartUserTime);
#endif

      if (!MapDirty && !first) {
	// redraw old screen, must have been a request for fast refresh
	BitBlt(hdcScreen, 0, 0, MapRectBig.right-MapRectBig.left,
	       MapRectBig.bottom-MapRectBig.top, 
	       hdcDrawWindow, 0, 0, SRCCOPY);
	continue;
      } else {
	MapDirty = false;
      }

#ifndef LK8000_OPTIMIZE
      if (BigZoom && !NewMap) {
	// quickly draw zoom level on top
	// Messy behaviour with NewMap
	DrawMapScale(hdcScreen, MapRect, true); 
      }
#endif

      MapWindow::UpdateInfo(&GPS_INFO, &CALCULATED_INFO);

      if (RequestFullScreen != MapFullScreen) {
	ToggleFullScreenStart();
      }

      //if ( !( IsMapFullScreen() && !EnablePan && Look8000 && NewMap && MapSpaceMode==1) ) { // VENTA TODO QUI FIX CRITIC

	      #ifndef NOFLARMGAUGE
	      GaugeFLARM::Render(&DrawInfo);
	      #endif
	      RenderMapWindow(MapRect);
      //}
    
      if (!first) {
	BitBlt(hdcScreen, 0, 0, 
	       MapRectBig.right-MapRectBig.left,
	       MapRectBig.bottom-MapRectBig.top, 
	       hdcDrawWindow, 0, 0, SRCCOPY);
	InvalidateRect(hWndMapWindow, &MapRect, false);
      }
      UpdateTimeStats(false);


	#if (WINDOWSPC<1)
	LKBatteryManager();
	#endif


      // we do caching after screen update, to minimise perceived delay
      UpdateCaches(first);
      first = false;
      if (ProgramStarted==psInitDone) {
	ProgramStarted = psFirstDrawDone;

#ifndef NOVARIOGAUGE
	if ( (InfoBoxLayout::InfoBoxGeometry==6) && (InfoBoxLayout::landscape == true) )
		GaugeVario::Show(!MapFullScreen);
#endif
      }
#ifdef CPUSTATS
	if ( (GetThreadTimes( hDrawThread, &CreationTime, &ExitTime,&EndKernelTime,&EndUserTime)) == 0) {
		Cpu_Draw=9999;
	} else {
		Cpustats(&Cpu_Draw,&StartKernelTime, &EndKernelTime, &StartUserTime, &EndUserTime);
	}
#endif
    
    }
  THREADEXIT = TRUE;
  return 0;
}


void MapWindow::DrawCrossHairs(HDC hdc, const POINT Orig,
			       const RECT rc)
{
  POINT o1, o2;
  
  o1.x = Orig.x+20;
  o2.x = Orig.x-20;
  o1.y = Orig.y;
  o2.y = Orig.y;

  if (BlackScreen)
	  DrawDashLine(hdc, NIBLSCALE(1), o1, o2, RGB_INVDRAW, rc);
  else
	  DrawDashLine(hdc, NIBLSCALE(1), o1, o2, RGB_DARKGREY, rc);

  o1.x = Orig.x;
  o2.x = Orig.x;
  o1.y = Orig.y+20;
  o2.y = Orig.y-20;

  if (BlackScreen)
	  DrawDashLine(hdc, NIBLSCALE(1), o1, o2, RGB_INVDRAW, rc); // 091219
  else
	  DrawDashLine(hdc, NIBLSCALE(1), o1, o2, RGB_DARKGREY, rc); // 091219

}


void PolygonRotateShift(POINT* poly, const int n, const int xs, const int ys, const double angle) {
  static double lastangle = -1;
  static int cost=1024, sint=0;

  if(angle != lastangle) {
    lastangle = angle;
    int deg = DEG_TO_INT(AngleLimit360(angle));
    cost = ICOSTABLE[deg]*InfoBoxLayout::scale;
    sint = ISINETABLE[deg]*InfoBoxLayout::scale;
  }
  const int xxs = xs*1024+512;
  const int yys = ys*1024+512;
  POINT *p = poly;
  const POINT *pe = poly+n;

  while (p<pe) {
    int x= p->x;
    int y= p->y;
    p->x = (x*cost - y*sint + xxs)/1024;
    p->y = (y*cost + x*sint + yys)/1024;
    p++;
  }
}


void MapWindow::DrawAircraft(HDC hdc, const POINT Orig)
{

  if ( ISPARAGLIDER || ISCAR ) {

    #define NUMPARAPOINTS 3

    POINT Para[3] = {
      { 0,-5},
      {5,9},
      {-5,9}
    };

    int pi;
    HPEN hpPOld;
    HBRUSH hbPAircraftSolid; 
    HBRUSH hbPAircraftSolidBg;

    if (BlackScreen) {
	#if LKOBJ
      hbPAircraftSolid = LKBrush_LightCyan;
      hbPAircraftSolidBg = LKBrush_Blue;
    } else {
      hbPAircraftSolid = LKBrush_Blue;
      hbPAircraftSolidBg = LKBrush_Grey;
	#else
      hbPAircraftSolid = (HBRUSH) CreateSolidBrush(RGB_LIGHTCYAN);
      hbPAircraftSolidBg = (HBRUSH) CreateSolidBrush(RGB_BLUE);
    } else {
      hbPAircraftSolid = (HBRUSH) CreateSolidBrush(RGB_BLUE);
      hbPAircraftSolidBg = (HBRUSH) CreateSolidBrush(RGB_GREY);
	#endif
    }

    HBRUSH hbPOld = (HBRUSH)SelectObject(hdc, hbPAircraftSolidBg);
    hpPOld = (HPEN)SelectObject(hdc, hpAircraft);
  
    PolygonRotateShift(Para, NUMPARAPOINTS, Orig.x+1, Orig.y+1,
                       DisplayAircraftAngle+
                       (DerivedDrawInfo.Heading-DrawInfo.TrackBearing));

    Polygon(hdc, Para, NUMPARAPOINTS);

    // draw it again so can get white border
    SelectObject(hdc, hpAircraftBorder);
    SelectObject(hdc, hbPAircraftSolid);

    for(pi=0; pi<NUMPARAPOINTS; pi++)
      {
	Para[pi].x -= 1;  Para[pi].y -= 1;
      }

    Polygon(hdc, Para, NUMPARAPOINTS);

    SelectObject(hdc, hpPOld);
    SelectObject(hdc, hbPOld);

    #ifndef LKOBJ
    DeleteObject(hbPAircraftSolid);
    DeleteObject(hbPAircraftSolidBg);
    #endif
    
    return;
  }

  if (Appearance.Aircraft == afAircraftDefault){

#define NUMAIRCRAFTPOINTS 16

    POINT Aircraft[NUMAIRCRAFTPOINTS] = {
      { 1,-6},
      {2,-1},
      {15,0},
      {15,2},
      {1,2},
      {0,10},
      {4,11},
      {4,12},
      {-4,12},
      {-4,11},
      {0,10},
      {-1,2},
      {-15,2},
      {-15,0},
      {-2,-1},
      {-1,-6}
    };

    int i;
    HPEN hpOld;
    HBRUSH hbAircraftSolid; 
    HBRUSH hbAircraftSolidBg;

	#if LKOBJ
    if (Appearance.InverseAircraft) {
      hbAircraftSolid = LKBrush_White;
      hbAircraftSolidBg = LKBrush_Black;
    } else {
      hbAircraftSolid = LKBrush_Black;
      hbAircraftSolidBg = LKBrush_White;
    }
	#else
    if (Appearance.InverseAircraft) {
      hbAircraftSolid = (HBRUSH) CreateSolidBrush(RGB_WHITE);
      hbAircraftSolidBg = (HBRUSH) CreateSolidBrush(RGB_BLACK);
    } else {
      hbAircraftSolid = (HBRUSH) CreateSolidBrush(RGB_BLACK);
      hbAircraftSolidBg = (HBRUSH) CreateSolidBrush(RGB_WHITE);
    }
	#endif

    HBRUSH hbOld = (HBRUSH)SelectObject(hdc, hbAircraftSolidBg);
    hpOld = (HPEN)SelectObject(hdc, hpAircraft);
  
    PolygonRotateShift(Aircraft, NUMAIRCRAFTPOINTS, Orig.x+1, Orig.y+1,
                       DisplayAircraftAngle+
                       (DerivedDrawInfo.Heading-DrawInfo.TrackBearing));

    Polygon(hdc, Aircraft, NUMAIRCRAFTPOINTS);

    // draw it again so can get white border
    SelectObject(hdc, hpAircraftBorder);
    SelectObject(hdc, hbAircraftSolid);

    for(i=0; i<NUMAIRCRAFTPOINTS; i++)
      {
	Aircraft[i].x -= 1;  Aircraft[i].y -= 1;
      }

    Polygon(hdc, Aircraft, NUMAIRCRAFTPOINTS);

    SelectObject(hdc, hpOld);
    SelectObject(hdc, hbOld);

    #ifndef LKOBJ
    DeleteObject(hbAircraftSolid);
    DeleteObject(hbAircraftSolidBg);
    #endif
    
  } else

    if (Appearance.Aircraft == afAircraftAltA){

      HPEN oldPen;
      POINT Aircraft[] = {
	{1, -5},
	{1, 0},
	{14, 0}, 
	{14, 1}, 
	{1, 1},
	{1, 8},
	{4, 8},
	{4, 9},
	{-3, 9},
	{-3, 8},
	{0, 8},
	{0, 1},
	{-13, 1}, 
	{-13, 0}, 
	{0, 0},
	{0, -5},
	{1, -5},
      };

      /* Experiment, when turning show the high wing larger, 
	 low wing smaller
	 if (DerivedDrawInfo.TurnRate>10) {
	 Aircraft[3].y = 0;
	 Aircraft[12].y = 2;
	 } else if (DerivedDrawInfo.TurnRate<-10) {
	 Aircraft[3].y = 2;
	 Aircraft[12].y = 0;
	 }
      */

      int n = sizeof(Aircraft)/sizeof(Aircraft[0]);

      double angle = DisplayAircraftAngle+
	(DerivedDrawInfo.Heading-DrawInfo.TrackBearing);

      PolygonRotateShift(Aircraft, n,
			 Orig.x-1, Orig.y, angle);

      oldPen = (HPEN)SelectObject(hdc, hpAircraft);
      Polygon(hdc, Aircraft, n);

      HBRUSH hbOld;
      if (Appearance.InverseAircraft) {
	hbOld = (HBRUSH)SelectObject(hdc, GetStockObject(WHITE_BRUSH));
      } else {
	hbOld = (HBRUSH)SelectObject(hdc, GetStockObject(BLACK_BRUSH));
      }
      SelectObject(hdc, hpAircraftBorder); // hpBearing
      Polygon(hdc, Aircraft, n);

      SelectObject(hdc, oldPen);
      SelectObject(hdc, hbOld);

    }

}

void MapWindow::DrawBitmapX(HDC hdc, int x, int y,
                            int sizex, int sizey,
                            HDC source,
                            int offsetx, int offsety,
                            DWORD mode) {
  if (InfoBoxLayout::scale>1) {
    StretchBlt(hdc, x, y, 
               IBLSCALE(sizex), 
               IBLSCALE(sizey), 
               source,
               offsetx, offsety, sizex, sizey,
               mode);
  } else {
    BitBlt(hdc, x, y, sizex, sizey, 
           source, offsetx, offsety, mode); 
  }
}

void MapWindow::DrawBitmapIn(const HDC hdc, const POINT &sc, const HBITMAP h) {
  if (!PointVisible(sc)) return;

  SelectObject(hDCTemp, h);

  DrawBitmapX(hdc,
              sc.x-NIBLSCALE(5),
              sc.y-NIBLSCALE(5),
              10,10,
	      hDCTemp,0,0,SRCPAINT);
  DrawBitmapX(hdc,
              sc.x-NIBLSCALE(5),
              sc.y-NIBLSCALE(5),
              10,10,
              hDCTemp,10,0,SRCAND);
}


void MapWindow::DrawGPSStatus(HDC hDC, const RECT rc)
{

//StartupStore(_T("NAVWarn=%d Sats=%d\n"),DrawInfo.NAVWarning,DrawInfo.SatellitesUsed); REMOVE
#ifdef NEWWARNINGS
  HFONT oldfont=NULL;
  if ((MapSpaceMode==MSM_WELCOME)||(MapWindow::isPan()) ) return; // 100210
#endif

  if (extGPSCONNECT && !(DrawInfo.NAVWarning) && (DrawInfo.SatellitesUsed != 0)) 
    // nothing to do
    return;
#ifndef COMDIAG
  TCHAR gpswarningtext1[] = TEXT(" GPS not connected ");
#endif
  TCHAR gpswarningtext2[] = TEXT(" GPS: NO VALID FIX ");
#ifdef COMDIAG
  static bool firstrun=true;
  TCHAR gpswarningtext3[] = TEXT(" GPS: No ComPort ");
  TCHAR gpswarningtext4[] = TEXT(" GPS: No Data Rx ");
  TCHAR gpswarningtext5[] = TEXT(" GPS is missing ");
  TCHAR gpswarningtext6[] = TEXT(" GPS not connected ");
#endif
  TextInBoxMode_t TextInBoxMode = {2};

  if (!extGPSCONNECT) {

#ifndef NEWWARNINGS
    SelectObject(hDCTemp,hGPSStatus2);
    DrawBitmapX(hDC, 
                rc.left+NIBLSCALE(2),
                rc.bottom+IBLSCALE(Appearance.GPSStatusOffset.y-22),
                20, 20,
                hDCTemp, 
                0, 0, SRCAND);
    TextInBox(hDC, gettext(gpswarningtext1), 
              rc.left+NIBLSCALE(24), 
              rc.bottom+IBLSCALE(Appearance.GPSStatusOffset.y-19),
              0, TextInBoxMode);
#else
    #ifdef COMDIAG
    oldfont=(HFONT)SelectObject(hDC,LK8TargetFont);   // 100222 
    TextInBoxMode.AsInt=0;
    TextInBoxMode.AsFlag.Color = TEXTWHITE;
    TextInBoxMode.AsFlag.NoSetFont=1;
    TextInBoxMode.AsFlag.AlligneCenter = 1;
    TextInBoxMode.AsFlag.WhiteBorder = 1;
    TextInBoxMode.AsFlag.Border = 1;
    if (ComPortStatus[0]==CPS_OPENKO) 
    	TextInBox(hDC, gpswarningtext3, (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, TextInBoxMode);
    else {
    	if (ComPortStatus[0]==CPS_OPENOK) {
		if ((ComPortRx[0]>0) && !firstrun) {
    			TextInBox(hDC, gpswarningtext5, (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, TextInBoxMode);
			firstrun=false; // 100214
		} else
    			TextInBox(hDC, gpswarningtext4, (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, TextInBoxMode);

	} else 
    		TextInBox(hDC, gpswarningtext6, (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, TextInBoxMode); // 100214

    }

    #else
    oldfont=(HFONT)SelectObject(hDC,LK8TargetFont);  // 100210
    TextInBoxMode.AsInt=0;
    TextInBoxMode.AsFlag.Color = TEXTWHITE;
    TextInBoxMode.AsFlag.NoSetFont=1;
    TextInBoxMode.AsFlag.AlligneCenter = 1;
    TextInBoxMode.AsFlag.WhiteBorder = 1;
    TextInBoxMode.AsFlag.Border = 1;
    TextInBox(hDC, gpswarningtext1, (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, TextInBoxMode);
    #endif
#endif

  } else
    if (DrawInfo.NAVWarning || (DrawInfo.SatellitesUsed == 0)) {
#ifndef NEWWARNINGS
      SelectObject(hDCTemp,hGPSStatus1);

      DrawBitmapX(hDC, 
                  rc.left+NIBLSCALE(2),
                  rc.bottom+IBLSCALE(Appearance.GPSStatusOffset.y-22),
                  20, 20,
                  hDCTemp, 
                  0, 0, SRCAND);

      TextInBox(hDC, gettext(gpswarningtext2), 
                rc.left+NIBLSCALE(24), 
                rc.bottom+
                IBLSCALE(Appearance.GPSStatusOffset.y-19),
                0, TextInBoxMode);
#else
    oldfont=(HFONT)SelectObject(hDC,LK8TargetFont); // 100210
    TextInBoxMode.AsInt=0;
    TextInBoxMode.AsFlag.Color = TEXTWHITE;
    TextInBoxMode.AsFlag.NoSetFont=1;
    TextInBoxMode.AsFlag.AlligneCenter = 1;
    TextInBoxMode.AsFlag.WhiteBorder = 1;
    TextInBoxMode.AsFlag.Border = 1;
    TextInBox(hDC, gpswarningtext2, 
              (rc.right-rc.left)/2, 
              (rc.bottom-rc.top)/3,
              0, TextInBoxMode);
#endif

    }
#ifdef NEWWARNINGS
  SelectObject(hDC,oldfont);
#endif

}

void MapWindow::DrawFlightMode(HDC hdc, const RECT rc)
{
  static bool flip= true;
  static double LastTime = 0;
  bool drawlogger = true;
  static bool lastLoggerActive=false;
  int offset = -1;

  if (!Appearance.DontShowLoggerIndicator){

	// has GPS time advanced?
	if(DrawInfo.Time <= LastTime) {
		LastTime = DrawInfo.Time;
	} else {

		flip = !flip;
		// don't bother drawing logger if not active for more than one second
		if ((!LoggerActive)&&(!lastLoggerActive)) {
			drawlogger = false;
		}
		lastLoggerActive = LoggerActive;
	}

	if (drawlogger) {
		offset -= 7;

		if (LoggerActive && flip) {
			SelectObject(hDCTemp,hLogger);
		} else {
			SelectObject(hDCTemp,hLoggerOff);
		}

		DrawBitmapX(hdc, rc.right+IBLSCALE(offset+Appearance.FlightModeOffset.x),
                  	rc.bottom - BottomSize+NIBLSCALE(1),
			7,7, hDCTemp, 0,0,SRCPAINT);

		DrawBitmapX(hdc, rc.right+IBLSCALE(offset+Appearance.FlightModeOffset.x),
                  	rc.bottom-BottomSize+NIBLSCALE(1),
			7,7, hDCTemp, 7,0,SRCAND);

		// not really needed if we remove offset next on
		offset +=7;
	}
  }


  if (Appearance.FlightModeIcon == apFlightModeIconDefault){

    #ifndef NOTASKABORT
    if (TaskAborted) {
      SelectObject(hDCTemp,hAbort);
    } else {
    #else
      if (DisplayMode == dmCircling) {
        SelectObject(hDCTemp,hClimb);
      } else if (DisplayMode == dmFinalGlide) {
        SelectObject(hDCTemp,hFinalGlide);
      } else {
        SelectObject(hDCTemp,hCruise);
      }
    #endif
    #ifndef NOTASKABORT
    }
    #endif
    // Code already commented as of 12aug05 - redundant? -st
    //          BitBlt(hdc,rc.right-35,5,24,20,
    //                           hDCTemp,20,0,SRCAND);

    // code for pre 12aug icons - st
    //BitBlt(hdc,rc.right-24-3,rc.bottom-20-3,24,20,
    //  hDCTemp,0,0,SRCAND);

    offset -= 24;

    DrawBitmapX(hdc,
                rc.right+IBLSCALE(offset-1+Appearance.FlightModeOffset.x),
                rc.bottom+IBLSCALE(-20-1+Appearance.FlightModeOffset.y),
                24,20,
                hDCTemp,
                0,0,SRCPAINT);
    
    DrawBitmapX(hdc,
                rc.right+IBLSCALE(offset-1+Appearance.FlightModeOffset.x),
                rc.bottom+IBLSCALE(-20-1+Appearance.FlightModeOffset.y),
                24,20,
                hDCTemp,
                24,0,SRCAND);

  // FlightModeIcon is always 0, unused!
  #if 100920
  }
  #else
  } else if (Appearance.FlightModeIcon == apFlightModeIconAltA){

#define SetPoint(Idx,X,Y) Arrow[Idx].x = X; Arrow[Idx].y = Y

    POINT Arrow[3];
    POINT Center;
    HBRUSH oldBrush;
    HPEN   oldPen;

    Center.x = rc.right-10;
    Center.y = rc.bottom-10;

    if (DisplayMode == dmCircling) {

      SetPoint(0, 
               Center.x,
               Center.y-NIBLSCALE(4));
      SetPoint(1, 
               Center.x-NIBLSCALE(8), 
               Center.y+NIBLSCALE(4));
      SetPoint(2, 
               Center.x+NIBLSCALE(8), 
               Center.y+NIBLSCALE(4));

    } else if (DisplayMode == dmFinalGlide) {

      SetPoint(0, 
               Center.x, 
               Center.y+NIBLSCALE(4));
      SetPoint(1, 
               Center.x-NIBLSCALE(8), 
               Center.y-NIBLSCALE(4));
      SetPoint(2, 
               Center.x+NIBLSCALE(8), 
               Center.y-NIBLSCALE(4));
    } else {

      SetPoint(0, 
               Center.x+NIBLSCALE(4), 
               Center.y);
      SetPoint(1, 
               Center.x-NIBLSCALE(4), 
               Center.y+NIBLSCALE(8));
      SetPoint(2, 
               Center.x-NIBLSCALE(4), 
               Center.y-NIBLSCALE(8));

    }

    #ifndef NOTASKABORT
    if (TaskAborted)
      oldBrush = (HBRUSH)SelectObject(hdc, hBrushFlyingModeAbort);
    else
      oldBrush = (HBRUSH)SelectObject(hdc, hbCompass);
    #else
     oldBrush = (HBRUSH)SelectObject(hdc, hbCompass);
    #endif

    oldPen = (HPEN)SelectObject(hdc, hpCompassBorder);
    Polygon(hdc, Arrow, 3);

    SelectObject(hdc, hpCompass);
    Polygon(hdc, Arrow, 3);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);

  }
  #endif // no flighticon 100920


  if (!Appearance.DontShowAutoMacCready && DerivedDrawInfo.AutoMacCready) {
    SelectObject(hDCTemp,hAutoMacCready);

    offset -= 24;

    //changed draw mode & icon for higher opacity 12aug -st

    DrawBitmapX(hdc,
		rc.right+IBLSCALE(offset-3+Appearance.FlightModeOffset.x),
		rc.bottom+IBLSCALE(-20-3+Appearance.FlightModeOffset.y),
		24,20,
		hDCTemp,
		0,0,SRCPAINT);

    DrawBitmapX(hdc,
		rc.right+IBLSCALE(offset-3+Appearance.FlightModeOffset.x),
		rc.bottom+IBLSCALE(-20-3+Appearance.FlightModeOffset.y),
		24,20,
		hDCTemp,
		24,0,SRCAND);

    //  commented @ 12aug st
    //  BitBlt(hdc,rc.right-48-3,rc.bottom-20-3,24,20,
    //    hDCTemp,0,0,SRCAND);
  };
  
}


typedef struct{
  TCHAR Name[NAME_SIZE+1];
  POINT Pos;
  TextInBoxMode_t Mode;
  int AltArivalAGL;
  bool inTask;
  bool isLandable; // VENTA5
  bool isAirport; // VENTA5
  bool isExcluded;
  int index;
}MapWaypointLabel_t;

bool MapWindow::WaypointInTask(int ind) {
  if (!WayPointList) return false;
  return WayPointList[ind].InTask;
}

//FIX
//static void MapWaypointLabelAdd(TCHAR *Name, int X, int Y, TextInBoxMode_t Mode, int AltArivalAGL, bool inTask=false, bool isLandable=false, bool isAirport=false, bool isExcluded=false);
void MapWaypointLabelAdd(TCHAR *Name, int X, int Y, TextInBoxMode_t Mode, int AltArivalAGL, bool inTask, bool isLandable, bool isAirport, bool isExcluded, int index);
//static int _cdecl MapWaypointLabelListCompare(const void *elem1, const void *elem2 );
int _cdecl MapWaypointLabelListCompare(const void *elem1, const void *elem2 );
//static MapWaypointLabel_t MapWaypointLabelList[50];

MapWaypointLabel_t MapWaypointLabelList[200]; 
//static int MapWaypointLabelListCount=0;
int MapWaypointLabelListCount=0;

bool MapWindow::WaypointInRange(int i) {
  return ((WayPointList[i].Zoom >= MapScale*10) 
          || (WayPointList[i].Zoom == 0)) 
    && (MapScale <= 10);
}

#ifndef LK8000_OPTIMIZE
void MapWindow::DrawWaypoints(HDC hdc, const RECT rc)
{
  unsigned int i;
  TCHAR Buffer[32];
  TCHAR Buffer2[32];
  TCHAR sAltUnit[4];
  TextInBoxMode_t TextDisplayMode;

  // if pan mode, show full names
  int pDisplayTextType = DisplayTextType;
  if (EnablePan) {
    pDisplayTextType = DISPLAYNAME;
  }

  if (!WayPointList) return;

  _tcscpy(sAltUnit, Units::GetAltitudeName());

  MapWaypointLabelListCount = 0;

  for(i=0;i<NumberOfWayPoints;i++)
    {
      if(WayPointList[i].Visible )
	{

#ifdef HAVEEXCEPTIONS
	  __try{
#endif

	    bool irange = false;
	    bool intask = false;
	    bool islandable = false;
	    bool dowrite;

	    intask = WaypointInTask(i);
	    dowrite = intask;

	    TextDisplayMode.AsInt = 0;

	    irange = WaypointInRange(i);

	    if(MapScale > 20) {
	      SelectObject(hDCTemp,hSmall);
	    #ifdef USEISLANDABLE
	    } else if( WayPointCalc[i].IsLandable ) {
	    #else
	    } else if( ((WayPointList[i].Flags & AIRPORT) == AIRPORT) 
		       || ((WayPointList[i].Flags & LANDPOINT) == LANDPOINT) ) {
	    #endif
	      islandable = true; // so we can always draw them
	      if(WayPointList[i].Reachable){

		TextDisplayMode.AsFlag.Reachable = 1;

		if ((DeclutterLabels<MAPLABELS_ALLOFF)||intask) {

		  if (intask || (DeclutterLabels<MAPLABELS_ONLYTOPO)) {
		    TextDisplayMode.AsFlag.Border = 1;
		  }
		  // show all reachable landing fields unless we want a decluttered
		  // screen.
		  dowrite = true;
		}

		#ifdef USEISLANDABLE
		if (WayPointCalc[i].IsAirport)
		#else
		if ((WayPointList[i].Flags & AIRPORT) == AIRPORT)
		#endif
		  SelectObject(hDCTemp,hBmpAirportReachable);
		else
		  SelectObject(hDCTemp,hBmpFieldReachable);
	      } else {
		#ifdef USEISLANDABLE
		if (WayPointCalc[i].IsAirport)
		#else
		if ((WayPointList[i].Flags & AIRPORT) == AIRPORT)
		#endif
		  SelectObject(hDCTemp,hBmpAirportUnReachable);
		else
		  SelectObject(hDCTemp,hBmpFieldUnReachable);
	      }
	    } else {
	      if(MapScale > 4) {
		SelectObject(hDCTemp,hSmall);
	      } else {
		SelectObject(hDCTemp,hTurnPoint);
	      }
	    }

	    if (intask) { // VNT 
	      TextDisplayMode.AsFlag.WhiteBold = 1;
	    }

	    if(irange || intask || islandable || dowrite) {
        
	      DrawBitmapX(hdc,
			  WayPointList[i].Screen.x-NIBLSCALE(10), 
			  WayPointList[i].Screen.y-NIBLSCALE(10),
			  20,20,
			  hDCTemp,0,0,SRCPAINT);
        
	      DrawBitmapX(hdc,
			  WayPointList[i].Screen.x-NIBLSCALE(10), 
			  WayPointList[i].Screen.y-NIBLSCALE(10),
			  20,20,
			  hDCTemp,20,0,SRCAND);
	    }

	    if(intask || irange || dowrite) {
	      bool draw_alt = TextDisplayMode.AsFlag.Reachable 
		&& ((DeclutterLabels<MAPLABELS_ONLYTOPO) || intask);

	      switch(pDisplayTextType) {
	      case DISPLAYNAMEIFINTASK:
		dowrite = intask;
		if (intask) {
		  if (draw_alt)
		    wsprintf(Buffer, TEXT("%s:%d%s"),
			     WayPointList[i].Name, 
			     (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), 
			     sAltUnit);
		  else
		    wsprintf(Buffer, TEXT("%s"),WayPointList[i].Name);
		}
		break;
	      case DISPLAYNAME:
		dowrite = (DeclutterLabels<MAPLABELS_ALLOFF) || intask;
		if (draw_alt)
		  wsprintf(Buffer, TEXT("%s:%d%s"),
			   WayPointList[i].Name, 
			   (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), 
			   sAltUnit);
		else
		  wsprintf(Buffer, TEXT("%s"),WayPointList[i].Name);
          
		break;
	      case DISPLAYNUMBER:
		dowrite = (DeclutterLabels<MAPLABELS_ALLOFF) || intask;
		if (draw_alt)
		  wsprintf(Buffer, TEXT("%d:%d%s"),
			   WayPointList[i].Number, 
			   (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), 
			   sAltUnit);
		else
		  wsprintf(Buffer, TEXT("%d"),WayPointList[i].Number);
          
		break;
	      case DISPLAYFIRSTFIVE:
		dowrite = (DeclutterLabels<MAPLABELS_ALLOFF) || intask;
		_tcsncpy(Buffer2, WayPointList[i].Name, 5);
		Buffer2[5] = '\0';
		if (draw_alt)
		  wsprintf(Buffer, TEXT("%s:%d%s"),
			   Buffer2, 
			   (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), 
			   sAltUnit);
		else
		  wsprintf(Buffer, TEXT("%s"),Buffer2);
          
		break;
	      case DISPLAYFIRSTTHREE:
		dowrite = (DeclutterLabels<MAPLABELS_ALLOFF) || intask;
		_tcsncpy(Buffer2, WayPointList[i].Name, 3);
		Buffer2[3] = '\0';
		if (draw_alt)
		  wsprintf(Buffer, TEXT("%s:%d%s"),
			   Buffer2, 
			   (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), 
			   sAltUnit);
		else
		  wsprintf(Buffer, TEXT("%s"),Buffer2);
          
		break;
	      case DISPLAYNONE:
		dowrite = (DeclutterLabels<MAPLABELS_ALLOFF) || intask;
		if (draw_alt)
		  wsprintf(Buffer, TEXT("%d%s"), 
			   (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY), 
			   sAltUnit);
		else
		  Buffer[0]= '\0';
	      default:
#if (WINDOWSPC<1)
		ASSERT(0);
#endif
		break;
	      }
        
	      if (dowrite) {
		MapWaypointLabelAdd(
				    Buffer,
				    WayPointList[i].Screen.x+5,
				    WayPointList[i].Screen.y,
				    TextDisplayMode,
				    (int)(WayPointList[i].AltArivalAGL*ALTITUDEMODIFY),
				    intask,false,false,false,i);
	      }
        
	    }
      
#ifdef HAVEEXCEPTIONS
	  }__finally
#endif
	     { ; }
	}
    }
  
  qsort(&MapWaypointLabelList, 
        MapWaypointLabelListCount,
        sizeof(MapWaypointLabel_t), 
        MapWaypointLabelListCompare);

  int j;

  // now draw task/landable waypoints in order of range (closest last)
  // writing unconditionally
  for (j=MapWaypointLabelListCount-1; j>=0; j--){
    MapWaypointLabel_t *E = &MapWaypointLabelList[j];
    // draws if they are in task unconditionally,
    // otherwise, does comparison
    if (E->inTask) {
      TextInBox(hdc, E->Name, E->Pos.x,
                E->Pos.y, 0, E->Mode, 
                false);
    }
  }

  // now draw normal waypoints in order of range (furthest away last)
  // without writing over each other (or the task ones)
  for (j=0; j<MapWaypointLabelListCount; j++) {
    MapWaypointLabel_t *E = &MapWaypointLabelList[j];
    if (!E->inTask) {
      TextInBox(hdc, E->Name, E->Pos.x,
                E->Pos.y, 0, E->Mode, 
                true);
    }
  }

}
#endif

//static int _cdecl MapWaypointLabelListCompare(const void *elem1, const void *elem2 ){
int _cdecl MapWaypointLabelListCompare(const void *elem1, const void *elem2 ){

  // Now sorts elements in task preferentially.
  /*
    if (((MapWaypointLabel_t *)elem1)->inTask && ! ((MapWaypointLabel_t *)elem2)->inTask)
    return (-1);
  */
  if (((MapWaypointLabel_t *)elem1)->AltArivalAGL > ((MapWaypointLabel_t *)elem2)->AltArivalAGL)
    return (-1);
  if (((MapWaypointLabel_t *)elem1)->AltArivalAGL < ((MapWaypointLabel_t *)elem2)->AltArivalAGL)
    return (+1);
  return (0);
}


//static void MapWaypointLabelAdd(TCHAR *Name, int X, int Y,  FIX
void MapWaypointLabelAdd(TCHAR *Name, int X, int Y, 
			 TextInBoxMode_t Mode, 
			 int AltArivalAGL, bool inTask, bool isLandable, bool isAirport, bool isExcluded, int index){
  MapWaypointLabel_t *E;

  if ((X<MapWindow::MapRect.left-WPCIRCLESIZE)
      || (X>MapWindow::MapRect.right+(WPCIRCLESIZE*3))
      || (Y<MapWindow::MapRect.top-WPCIRCLESIZE)
      || (Y>MapWindow::MapRect.bottom+WPCIRCLESIZE)){
    return;
  }

  if (MapWaypointLabelListCount >= (( (signed int)(sizeof(MapWaypointLabelList)/sizeof(MapWaypointLabel_t)))-1)){  // BUGFIX 100207
    return;
  }

  E = &MapWaypointLabelList[MapWaypointLabelListCount];

  _tcscpy(E->Name, Name);
  E->Pos.x = X;
  E->Pos.y = Y;
  E->Mode = Mode;
  E->AltArivalAGL = AltArivalAGL;
  E->inTask = inTask;
  E->isLandable = isLandable;
  E->isAirport  = isAirport;
  E->isExcluded = isExcluded;

  MapWaypointLabelListCount++;

}


void MapWindow::DrawAbortedTask(HDC hdc, const RECT rc, const POINT me)
{
  int i;
  if (!WayPointList) return;
  
  LockTaskData();  // protect from external task changes
#ifdef HAVEEXCEPTIONS
  __try{
#endif
    for(i=0;i<MAXTASKPOINTS-1;i++)
      {
	int index = Task[i].Index;
	if(ValidWayPoint(index))
	  {
	    DrawDashLine(hdc, 1, 
			 WayPointList[index].Screen,
			 me,
			 taskcolor, rc);
	  }
      }
#ifdef HAVEEXCEPTIONS
  }__finally
#endif
     {
       UnlockTaskData();
     }
}


void MapWindow::DrawStartSector(HDC hdc, const RECT rc, 
                                POINT &Start,
                                POINT &End, int Index) {
  double tmp;

  if(StartLine) {
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(5), WayPointList[Index].Screen,
              Start, taskcolor, rc);
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(5), WayPointList[Index].Screen,
              End, taskcolor, rc);
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), WayPointList[Index].Screen,
              Start, RGB(255,0,0), rc);
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), WayPointList[Index].Screen,
              End, RGB(255,0,0), rc);
  } else {
    tmp = StartRadius*ResMapScaleOverDistanceModify;
    SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    SelectObject(hdc, hpStartFinishThick);
    Circle(hdc,
           WayPointList[Index].Screen.x,
           WayPointList[Index].Screen.y,(int)tmp, rc, false, false);
    SelectObject(hdc, hpStartFinishThin);
    Circle(hdc,
           WayPointList[Index].Screen.x,
           WayPointList[Index].Screen.y,(int)tmp, rc, false, false);
  }

}


void MapWindow::DrawTask(HDC hdc, RECT rc, const POINT &Orig_Aircraft)
{
  int i;
  double tmp;

  COLORREF whitecolor = RGB_WHITE;
  COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);

  if (!WayPointList) return;

  LockTaskData();  // protect from external task changes
#ifdef HAVEEXCEPTIONS
  __try{
#endif

    if(ValidTaskPoint(0) && ValidTaskPoint(1) && (ActiveWayPoint<2))
      {
	DrawStartSector(hdc,rc, Task[0].Start, Task[0].End, Task[0].Index);
	if (EnableMultipleStartPoints) {
	  for (i=0; i<MAXSTARTPOINTS; i++) {
	    if (StartPoints[i].Active && ValidWayPoint(StartPoints[i].Index)) {
	      DrawStartSector(hdc,rc, 
			      StartPoints[i].Start, 
			      StartPoints[i].End, StartPoints[i].Index);
	    }
	  }
	}
      }
  
    for(i=1;i<MAXTASKPOINTS-1;i++) {

      if(ValidTaskPoint(i) && !ValidTaskPoint(i+1)) { // final waypoint
	if (ActiveWayPoint>1) { 
	  // only draw finish line when past the first
	  // waypoint.
	  if(FinishLine) {
	    _DrawLine(hdc, PS_SOLID, NIBLSCALE(5), 
		      WayPointList[Task[i].Index].Screen,
		      Task[i].Start, taskcolor, rc);
	    _DrawLine(hdc, PS_SOLID, NIBLSCALE(5), 
		      WayPointList[Task[i].Index].Screen,
		      Task[i].End, taskcolor, rc);
	    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), 
		      WayPointList[Task[i].Index].Screen,
		      Task[i].Start, RGB(255,0,0), rc);
	    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), 
		      WayPointList[Task[i].Index].Screen,
		      Task[i].End, RGB(255,0,0), rc);
	  } else {
	    tmp = FinishRadius*ResMapScaleOverDistanceModify; 
	    SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	    SelectObject(hdc, hpStartFinishThick);
	    Circle(hdc,
		   WayPointList[Task[i].Index].Screen.x,
		   WayPointList[Task[i].Index].Screen.y,
		   (int)tmp, rc, false, false); 
	    SelectObject(hdc, hpStartFinishThin);
	    Circle(hdc,
		   WayPointList[Task[i].Index].Screen.x,
		   WayPointList[Task[i].Index].Screen.y,
		   (int)tmp, rc, false, false); 
	  }        
	}
      } // final waypoint
      // DRAW TASK SECTORS
      if(ValidTaskPoint(i) && ValidTaskPoint(i+1)) { // normal sector
	if(AATEnabled != TRUE) {
	  _DrawLine(hdc, PS_DASH,NIBLSCALE(4), WayPointList[Task[i].Index].Screen, Task[i].Start, RGB_MAGENTA, rc); // 091216 127,127,127
	  _DrawLine(hdc, PS_DASH,NIBLSCALE(4), WayPointList[Task[i].Index].Screen, Task[i].End, RGB_MAGENTA, rc); // 091216

	  SelectObject(hdc, GetStockObject(HOLLOW_BRUSH)); 
	  SelectObject(hdc, hpBearing); // 091216
	  if(SectorType== 0) {
	    tmp = SectorRadius*ResMapScaleOverDistanceModify;

	    Circle(hdc,
		   WayPointList[Task[i].Index].Screen.x,
		   WayPointList[Task[i].Index].Screen.y,
		   (int)tmp, rc, false, false); 

	  }
	  // FAI SECTOR
	  if(SectorType==1) {
	    tmp = SectorRadius*ResMapScaleOverDistanceModify;

	    Segment(hdc,
		    WayPointList[Task[i].Index].Screen.x,
		    WayPointList[Task[i].Index].Screen.y,(int)tmp, rc, 
		    Task[i].AATStartRadial-DisplayAngle, 
		    Task[i].AATFinishRadial-DisplayAngle); 

	  }
	  if(SectorType== 2) {
	    // JMW added german rules
	    tmp = 500*ResMapScaleOverDistanceModify;
	    Circle(hdc,
		   WayPointList[Task[i].Index].Screen.x,
		   WayPointList[Task[i].Index].Screen.y,
		   (int)tmp, rc, false, false); 

	    tmp = 10e3*ResMapScaleOverDistanceModify;
          
	    Segment(hdc,
		    WayPointList[Task[i].Index].Screen.x,
		    WayPointList[Task[i].Index].Screen.y,(int)tmp, rc, 
		    Task[i].AATStartRadial-DisplayAngle, 
		    Task[i].AATFinishRadial-DisplayAngle); 

	  }
	} else {
		// ELSE HERE IS   *** AAT ***
	  // JMW added iso lines
	  if ((i==ActiveWayPoint) || (TargetPan && (i==TargetPanIndex))) {
	    // JMW 20080616 flash arc line if very close to target
	    static bool flip = false;
	  
	    if (DerivedDrawInfo.WaypointDistance<AATCloseDistance()*2.0) {
	      flip = !flip;
	    } else {
	      flip = true;
	    }
	    if (flip) {
	      for (int j=0; j<MAXISOLINES-1; j++) {
		if (TaskStats[i].IsoLine_valid[j] 
		    && TaskStats[i].IsoLine_valid[j+1]) {
		  _DrawLine(hdc, PS_SOLID, NIBLSCALE(2), 
			    TaskStats[i].IsoLine_Screen[j], 
			    TaskStats[i].IsoLine_Screen[j+1],
			    RGB(0,0,255), rc);
		}
	      }
	    }
	  }
	}
      }
    }

    for(i=0;i<MAXTASKPOINTS-1;i++) {
      if(ValidTaskPoint(i) && ValidTaskPoint(i+1)) {
	bool is_first = (Task[i].Index < Task[i+1].Index);
	int imin = min(Task[i].Index,Task[i+1].Index);
	int imax = max(Task[i].Index,Task[i+1].Index);
	// JMW AAT!
	double bearing = Task[i].OutBound;
	POINT sct1, sct2;
	if (AATEnabled && !TargetPan) {
	  LatLon2Screen(Task[i].AATTargetLon, 
			Task[i].AATTargetLat, 
			sct1);
	  LatLon2Screen(Task[i+1].AATTargetLon, 
			Task[i+1].AATTargetLat, 
			sct2);
	  DistanceBearing(Task[i].AATTargetLat,
			  Task[i].AATTargetLon,
			  Task[i+1].AATTargetLat,
			  Task[i+1].AATTargetLon,
			  NULL, &bearing);

	  // draw nominal track line
	  DrawDashLine(hdc, NIBLSCALE(1),   // 091217
		       WayPointList[imin].Screen, 
		       WayPointList[imax].Screen, 
		       taskcolor, rc);
	} else {
	  sct1 = WayPointList[Task[i].Index].Screen;
	  sct2 = WayPointList[Task[i+1].Index].Screen;
	}

	if (is_first) {
	  DrawDashLine(hdc, NIBLSCALE(3), 
		       sct1, 
		       sct2, 
		       taskcolor, rc);
	} else {
	  DrawDashLine(hdc, NIBLSCALE(3), 
		       sct2, 
		       sct1, 
		       taskcolor, rc); 
	}

	// draw small arrow along task direction
	POINT p_p;
	POINT Arrow[2] = { {6,6}, {-6,6} };
	ScreenClosestPoint(sct1, sct2, 
			   Orig_Aircraft, &p_p, NIBLSCALE(25));
	PolygonRotateShift(Arrow, 2, p_p.x, p_p.y, 
			   bearing-DisplayAngle);

	_DrawLine(hdc, PS_SOLID, NIBLSCALE(2), Arrow[0], p_p, taskcolor, rc);
	_DrawLine(hdc, PS_SOLID, NIBLSCALE(2), Arrow[1], p_p, taskcolor, rc);
      }
    }
#ifdef HAVEEXCEPTIONS
  }__finally
#endif
     {
       UnlockTaskData();
     }

  // restore original color
  SetTextColor(hDCTemp, origcolor);

}


void MapWindow::DrawTaskAAT(HDC hdc, const RECT rc)
{
  int i;
  double tmp;

  if (!WayPointList) return;
  if (!AATEnabled) return;
  
  LockTaskData();  // protect from external task changes
#ifdef HAVEEXCEPTIONS
  __try{
#endif

    COLORREF whitecolor = RGB_WHITE;
    COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);

    SelectObject(hDCTemp, (HBITMAP)hDrawBitMapTmp);

    SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
    SelectObject(hDCTemp, GetStockObject(WHITE_BRUSH));
    Rectangle(hDCTemp,rc.left,rc.top,rc.right,rc.bottom);
    
    for(i=MAXTASKPOINTS-2;i>0;i--)
      {
	if(ValidTaskPoint(i) && ValidTaskPoint(i+1)) {
	  if(Task[i].AATType == CIRCLE)
	    {
	      tmp = Task[i].AATCircleRadius*ResMapScaleOverDistanceModify;
          
	      // this color is used as the black bit
	      SetTextColor(hDCTemp, 
			   Colours[iAirspaceColour[AATASK]]);
          
	      // this color is the transparent bit
	      SetBkColor(hDCTemp, 
			 whitecolor);

	      if (i<ActiveWayPoint) {
		SelectObject(hDCTemp, GetStockObject(HOLLOW_BRUSH));
	      } else {
		SelectObject(hDCTemp, hAirspaceBrushes[iAirspaceBrush[AATASK]]);
	      }
	      SelectObject(hDCTemp, GetStockObject(BLACK_PEN));
          
	      Circle(hDCTemp,
		     WayPointList[Task[i].Index].Screen.x,
		     WayPointList[Task[i].Index].Screen.y,
		     (int)tmp, rc, true, true); 
	    }
	  else
	    {
          
	      // this color is used as the black bit
	      SetTextColor(hDCTemp, 
			   Colours[iAirspaceColour[AATASK]]);
          
	      // this color is the transparent bit
	      SetBkColor(hDCTemp, 
			 whitecolor);
          
	      if (i<ActiveWayPoint) {
		SelectObject(hDCTemp, GetStockObject(HOLLOW_BRUSH));
	      } else {
		SelectObject(hDCTemp, hAirspaceBrushes[iAirspaceBrush[AATASK]]);
	      }
	      SelectObject(hDCTemp, GetStockObject(BLACK_PEN));
          
	      tmp = Task[i].AATSectorRadius*ResMapScaleOverDistanceModify;
          
	      Segment(hDCTemp,
		      WayPointList[Task[i].Index].Screen.x,
		      WayPointList[Task[i].Index].Screen.y,(int)tmp, rc, 
		      Task[i].AATStartRadial-DisplayAngle, 
		      Task[i].AATFinishRadial-DisplayAngle); 
          
	      DrawSolidLine(hDCTemp,
			    WayPointList[Task[i].Index].Screen, Task[i].AATStart,
			    rc);
	      DrawSolidLine(hDCTemp,
			    WayPointList[Task[i].Index].Screen, Task[i].AATFinish,
			    rc);

	    }

	}
      }

    // restore original color
    SetTextColor(hDCTemp, origcolor);

#if (WINDOWSPC<1)
    TransparentImage(hdc,
		     rc.left,rc.top,
		     rc.right-rc.left,rc.bottom-rc.top,
		     hDCTemp,
		     rc.left,rc.top,
		     rc.right-rc.left,rc.bottom-rc.top,
		     whitecolor
		     );

#else
    TransparentBlt(hdc,
                   rc.left,rc.top,
                   rc.right-rc.left,rc.bottom-rc.top,
                   hDCTemp,
                   rc.left,rc.top,
                   rc.right-rc.left,rc.bottom-rc.top,
                   whitecolor
                   );
#endif
  
#ifdef HAVEEXCEPTIONS
  }__finally
#endif
     {
       UnlockTaskData();
     }
}


void MapWindow::DrawWindAtAircraft2(HDC hdc, const POINT Orig, const RECT rc) {
  int i;
  POINT Start;
  HPEN hpOld;
  HBRUSH hbOld; 
  TCHAR sTmp[12];
  static SIZE tsize = {0,0};
  
  if (DerivedDrawInfo.WindSpeed<1) {
    return; // JMW don't bother drawing it if not significant
  }
  
  if (tsize.cx == 0){

    HFONT oldFont = (HFONT)SelectObject(hdc, MapWindowBoldFont);
    GetTextExtentPoint(hdc, TEXT("99"), 2, &tsize);
    SelectObject(hdc, oldFont);
    tsize.cx = tsize.cx/2;
  }

  hpOld = (HPEN)SelectObject(hdc, hpWind);
  hbOld = (HBRUSH)SelectObject(hdc, hbWind);
  
  int wmag = iround(4.0*DerivedDrawInfo.WindSpeed);
  
  Start.y = Orig.y;
  Start.x = Orig.x;

  int kx = tsize.cx/InfoBoxLayout::scale/2;

  POINT Arrow[7] = { {0,-20}, {-6,-26}, {0,-20}, 
                     {6,-26}, {0,-20}, 
                     {8+kx, -24}, 
                     {-8-kx, -24}};

  for (i=1;i<4;i++)
    Arrow[i].y -= wmag;

  PolygonRotateShift(Arrow, 7, Start.x, Start.y, 
		     DerivedDrawInfo.WindBearing-DisplayAngle);

  if (WindArrowStyle==1) {
    POINT Tail[2] = {{0,-20}, {0,-26-min(20,wmag)*3}};
    double angle = AngleLimit360(DerivedDrawInfo.WindBearing-DisplayAngle);
    for(i=0; i<2; i++) {
      if (InfoBoxLayout::scale>1) {
        Tail[i].x *= InfoBoxLayout::scale;
        Tail[i].y *= InfoBoxLayout::scale;
      }
      protateshift(Tail[i], angle, Start.x, Start.y);
    }

    // optionally draw dashed line
    _DrawLine(hdc, PS_DASH, 1, Tail[0], Tail[1], RGB(0,0,0), rc);
  }

  if ( !(NewMap&&Look8000) || (DisplayMode == dmCircling) ) {

  	_itot(iround(DerivedDrawInfo.WindSpeed * SPEEDMODIFY), sTmp, 10);

  	TextInBoxMode_t TextInBoxMode = { 16 | 32 }; // JMW test {2 | 16};
  	if (Arrow[5].y>=Arrow[6].y) {
  	  TextInBox(hdc, sTmp, Arrow[5].x-kx, Arrow[5].y, 0, TextInBoxMode);
  	} else {
  	  TextInBox(hdc, sTmp, Arrow[6].x-kx, Arrow[6].y, 0, TextInBoxMode);
  	}

  }

  Polygon(hdc,Arrow,5);

  SelectObject(hdc, hbOld);
  SelectObject(hdc, hpOld);
}


void MapWindow::DrawBearing(HDC hdc, const RECT rc)
{

  #if OVERTARGET
  int overindex=GetOvertargetIndex();
  if (overindex<0) return;
/* REMOVE
  int overindex=-1;
  if (OvertargetMode >OVT_TASK) overindex=GetOvertargetIndex();
  if ( (!ValidTaskPoint(ActiveWayPoint)) && (overindex<0)) {
	return; 
  }
*/
  #else
  if (!ValidTaskPoint(ActiveWayPoint)) {
	return; 
  }
  LockTaskData();  // protect from external task changes
  #endif

  double startLat = DrawInfo.Latitude;
  double startLon = DrawInfo.Longitude;
  double targetLat;
  double targetLon;

  #if OVERTARGET
  if (overindex>OVT_TASK) {
  	LockTaskData();
	targetLat = WayPointList[overindex].Latitude;
	targetLon = WayPointList[overindex].Longitude; 
	UnlockTaskData();
	//  DrawGreatCircle(hdc, startLon, startLat, targetLon, targetLat, rc);
	//HPEN hpOld = (HPEN)SelectObject(hdc, hpOvertarget);
	HPEN hpOld = (HPEN)SelectObject(hdc, hpBearing);
	POINT pt[2];
	LatLon2Screen(startLon, startLat, pt[0]);
	LatLon2Screen(targetLon, targetLat, pt[1]);
	ClipPolygon(hdc, pt, 2, rc, false);
	SelectObject(hdc, hpOld);
	return; // remove return to let multiple destination lines drawn
  }
  if (!ValidTaskPoint(ActiveWayPoint)) {
	return; 
  }
  LockTaskData();
  #endif

  if (AATEnabled && (ActiveWayPoint>0) && ValidTaskPoint(ActiveWayPoint+1)) {
    targetLat = Task[ActiveWayPoint].AATTargetLat;
    targetLon = Task[ActiveWayPoint].AATTargetLon; 
  } else {
    targetLat = WayPointList[Task[ActiveWayPoint].Index].Latitude;
    targetLon = WayPointList[Task[ActiveWayPoint].Index].Longitude; 
  }
  UnlockTaskData();

  DrawGreatCircle(hdc, startLon, startLat,
                  targetLon, targetLat, rc);

  if (TargetPan) {
    // Draw all of task if in target pan mode
    startLat = targetLat;
    startLon = targetLon;

    LockTaskData();
    for (int i=ActiveWayPoint+1; i<MAXTASKPOINTS; i++) {
      if (ValidTaskPoint(i)) {

        if (AATEnabled && ValidTaskPoint(i+1)) {
          targetLat = Task[i].AATTargetLat;
          targetLon = Task[i].AATTargetLon; 
        } else {
          targetLat = WayPointList[Task[i].Index].Latitude;
          targetLon = WayPointList[Task[i].Index].Longitude; 
        }
       
        DrawGreatCircle(hdc, startLon, startLat,
                        targetLon, targetLat, rc);

        startLat = targetLat;
        startLon = targetLon;
      }
    }

    // JMW draw symbol at target, makes it easier to see

    if (AATEnabled) {
      for (int i=ActiveWayPoint+1; i<MAXTASKPOINTS; i++) {
        if(ValidTaskPoint(i) && ValidTaskPoint(i+1)) {
          if (i>= ActiveWayPoint) {
            POINT sct;
            LatLon2Screen(Task[i].AATTargetLon, 
                          Task[i].AATTargetLat, 
                          sct);
            DrawBitmapIn(hdc, sct, hBmpTarget);
          }
        }
      }
    }

    UnlockTaskData();

  }

  if (AATEnabled) {
    LockTaskData();
    if (ValidTaskPoint(ActiveWayPoint+1) && (ActiveWayPoint>0)) {
      POINT sct;
      LatLon2Screen(Task[ActiveWayPoint].AATTargetLon, 
                    Task[ActiveWayPoint].AATTargetLat, 
                    sct);
      DrawBitmapIn(hdc, sct, hBmpTarget);
    }
    UnlockTaskData();
  }
}


double MapWindow::GetApproxScreenRange() {
  return (MapScale * max(MapRectBig.right-MapRectBig.left,
                         MapRectBig.bottom-MapRectBig.top))
    *1000.0/GetMapResolutionFactor();
}




extern bool ScreenBlanked;

bool MapWindow::IsDisplayRunning() {
  return (THREADRUNNING && GlobalRunning && !ScreenBlanked && ProgramStarted);
}


void MapWindow::CreateDrawingThread(void)
{
  CLOSETHREAD = FALSE;
  THREADEXIT = FALSE;
  hDrawThread = CreateThread (NULL, 0,  
                              (LPTHREAD_START_ROUTINE )MapWindow::DrawThread, 
                              0, 0, &dwDrawThreadID);
  SetThreadPriority(hDrawThread,THREAD_PRIORITY_NORMAL);
}

void MapWindow::SuspendDrawingThread(void)
{
  LockTerrainDataGraphics();
  THREADRUNNING = FALSE;
  UnlockTerrainDataGraphics();
  //  SuspendThread(hDrawThread);
}

void MapWindow::ResumeDrawingThread(void)
{
  LockTerrainDataGraphics();
  THREADRUNNING = TRUE;
  UnlockTerrainDataGraphics();
  //  ResumeThread(hDrawThread);
}

void MapWindow::CloseDrawingThread(void)
{
  CLOSETHREAD = TRUE;
  SetEvent(drawTriggerEvent); // wake self up
  LockTerrainDataGraphics();
  SuspendDrawingThread();
  UnlockTerrainDataGraphics();
  while(!THREADEXIT) { Sleep(100); };
}





bool MapWindow::PointInRect(const double &lon, const double &lat,
                            const rectObj &bounds) {
  if ((lon> bounds.minx) &&
      (lon< bounds.maxx) &&
      (lat> bounds.miny) &&
      (lat< bounds.maxy)) 
    return true;
  else
    return false;
}


bool MapWindow::PointVisible(const double &lon, const double &lat) {
  if ((lon> screenbounds_latlon.minx) &&
      (lon< screenbounds_latlon.maxx) &&
      (lat> screenbounds_latlon.miny) &&
      (lat< screenbounds_latlon.maxy)) 
    return true;
  else
    return false;
}


bool MapWindow::PointVisible(const POINT &P)
{
  if(( P.x >= MapRect.left ) 
     &&
     ( P.x <= MapRect.right ) 
     &&
     ( P.y >= MapRect.top  ) 
     &&
     ( P.y <= MapRect.bottom  ) 
     )
    return TRUE;
  else
    return FALSE;
}


void MapWindow::DisplayAirspaceWarning(int Type, TCHAR *Name , 
                                       AIRSPACE_ALT Base, AIRSPACE_ALT Top )
{
  TCHAR szMessageBuffer[1024];
  TCHAR szTitleBuffer[1024];
  
  FormatWarningString(Type, Name , Base, Top, szMessageBuffer, szTitleBuffer );

  DoStatusMessage(TEXT("Airspace Query"), szMessageBuffer);
}


// RETURNS Longitude, Latitude!

void MapWindow::OrigScreen2LatLon(const int &x, const int &y, 
                                  double &X, double &Y) 
{
  int sx = x;
  int sy = y;
  irotate(sx, sy, DisplayAngle);
  Y= PanLatitude  - sy*InvDrawScale;
  X= PanLongitude + sx*invfastcosine(Y)*InvDrawScale;
}


void MapWindow::Screen2LatLon(const int &x, const int &y, 
                              double &X, double &Y) 
{
  int sx = x-(int)Orig_Screen.x;
  int sy = y-(int)Orig_Screen.y;
  irotate(sx, sy, DisplayAngle);
  Y= PanLatitude  - sy*InvDrawScale;
  X= PanLongitude + sx*invfastcosine(Y)*InvDrawScale;
}

void MapWindow::LatLon2Screen(const double &lon, const double &lat, 
                              POINT &sc) {
  int Y = Real2Int((PanLatitude-lat)*DrawScale);
  int X = Real2Int((PanLongitude-lon)*fastcosine(lat)*DrawScale);
    
  irotate(X, Y, DisplayAngle);
    
  sc.x = Orig_Screen.x - X;
  sc.y = Orig_Screen.y + Y;
}

// This one is optimised for long polygons
void MapWindow::LatLon2Screen(pointObj *ptin, POINT *ptout, const int n,
			      const int skip) {
  static double lastangle = -1;
  static int cost=1024, sint=0;
  const double mDisplayAngle = DisplayAngle;

  if(mDisplayAngle != lastangle) {
    lastangle = mDisplayAngle;
    int deg = DEG_TO_INT(AngleLimit360(mDisplayAngle));
    cost = ICOSTABLE[deg];
    sint = ISINETABLE[deg];
  }
  const int xxs = Orig_Screen.x*1024-512;
  const int yys = Orig_Screen.y*1024+512;
  const double mDrawScale = DrawScale;
  const double mPanLongitude = PanLongitude;
  const double mPanLatitude = PanLatitude;
  pointObj* p = ptin;
  const pointObj* ptend = ptin+n;

  while (p<ptend) {
    int Y = Real2Int((mPanLatitude-p->y)*mDrawScale);
    int X = Real2Int((mPanLongitude-p->x)*fastcosine(p->y)*mDrawScale);
    ptout->x = (xxs-X*cost + Y*sint)/1024;
    ptout->y = (Y*cost + X*sint + yys)/1024;
    ptout++;
    p+= skip;
  }
}


void MapWindow::_Polyline(HDC hdc, POINT* pt, const int npoints, 
			  const RECT rc) {
#ifdef BUG_IN_CLIPPING
  ClipPolygon(hdc, pt, npoints, rc, false);
  //VENTA2
#elif defined(PNA)
  // if (GlobalModelType == MODELTYPE_PNA_HP31X)
  if (needclipping==true)
    ClipPolygon(hdc, pt, npoints, rc, false);
  else
    Polyline(hdc, pt, npoints);
#else
  Polyline(hdc, pt, npoints);
#endif
}

void MapWindow::DrawSolidLine(const HDC& hdc, const POINT &ptStart, 
                              const POINT &ptEnd, const RECT rc)
{
  POINT pt[2];
  
  pt[0].x= ptStart.x;
  pt[0].y= ptStart.y;
  pt[1].x= ptEnd.x;
  pt[1].y= ptEnd.y;

  _Polyline(hdc, pt, 2, rc);
} 


void MapWindow::_DrawLine(HDC hdc, const int PenStyle, const int width, 
			  const POINT ptStart, const POINT ptEnd, 
			  const COLORREF cr, 
			  const RECT rc) {

  HPEN hpDash,hpOld;
  POINT pt[2];
  //Create a dot pen
  hpDash = (HPEN)CreatePen(PenStyle, width, cr);
  hpOld = (HPEN)SelectObject(hdc, hpDash);

  pt[0].x = ptStart.x;
  pt[0].y = ptStart.y;
  pt[1].x = ptEnd.x;
  pt[1].y = ptEnd.y;

  _Polyline(hdc, pt, 2, rc);

  SelectObject(hdc, hpOld);
  DeleteObject((HPEN)hpDash);
}


void MapWindow::DrawDashLine(HDC hdc, const int width, 
			     const POINT ptStart, const POINT ptEnd, const COLORREF cr,
			     const RECT rc)
{
  int i;
  HPEN hpDash,hpOld;
  POINT pt[2];
  //Create a dot pen
  hpDash = (HPEN)CreatePen(PS_DASH, 1, cr);
  hpOld = (HPEN)SelectObject(hdc, hpDash);

  pt[0].x = ptStart.x;
  pt[0].y = ptStart.y;
  pt[1].x = ptEnd.x;
  pt[1].y = ptEnd.y;
  
  //increment on smallest variance
  if(abs(ptStart.x - ptEnd.x) < abs(ptStart.y - ptEnd.y)){
    for (i = 0; i < width; i++){
      pt[0].x += 1;
      pt[1].x += 1;     
      _Polyline(hdc, pt, 2, rc);
    }   
  } else {
    for (i = 0; i < width; i++){
      pt[0].y += 1;
      pt[1].y += 1;     
      _Polyline(hdc, pt, 2, rc);
    }   
  }
  
  SelectObject(hdc, hpOld);
  DeleteObject((HPEN)hpDash);
  
} 


/* Not used
   void DrawDotLine(HDC hdc, POINT ptStart, POINT ptEnd, COLORREF cr,
   const RECT rc)
   {
   HPEN hpDot, hpOld;
   LOGPEN dashLogPen;
   POINT pt[2];
   //Create a dot pen
   dashLogPen.lopnColor = cr;
   dashLogPen.lopnStyle = PS_DOT;
   dashLogPen.lopnWidth.x = 0;
   dashLogPen.lopnWidth.y = 0;

   hpDot = (HPEN)CreatePenIndirect(&dashLogPen);
   hpOld = (HPEN)SelectObject(hdc, hpDot);

   pt[0].x = ptStart.x;
   pt[0].y = ptStart.y;
   pt[1].x = ptEnd.x;
   pt[1].y = ptEnd.y;
  
   Polyline(hdc, pt, 2);
  
   SelectObject(hdc, hpOld);
   DeleteObject((HPEN)hpDot);
   } 

*/

