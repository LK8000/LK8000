/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindow.cpp,v 8.29 2011/01/06 02:07:52 root Exp root $
*/

#include "StdAfx.h"
#include "compatibility.h"
#include "Defines.h"
#include "options.h"
#include "MapWindow.h"
#include "LKMapWindow.h"
#include "Utils.h"
#include "Units.h"
#include "Logger.h"
#include "McReady.h"
#include "Airspace.h"
#include "Waypointparser.h"
#include "Dialogs.h"
#include "externs.h"
#include "InputEvents.h"
// #include <assert.h>
#include <windows.h>
#include <math.h>
#include <Message.h>

#include <tchar.h>
#include "Modeltype.h"

#include "Terrain.h"
#include "Task.h"
#include "AATDistance.h"
#include "LKObjects.h"

#include "InfoBoxLayout.h"
#include "RasterTerrain.h"
#include "Utils2.h"
#include "LKAirspace.h"
#include "Bitmaps.h"
#include "RGB.h"

using std::min;
using std::max;
#if (WINDOWSPC>0)
#include <wingdi.h>
#endif

#include "utils/heapcheck.h"

#include "LKGeneralAviation.h"


DWORD misc_tick_count=0;

#ifdef DEBUG
#define DRAWLOAD
#define DEBUG_VIRTUALKEYS
#endif

#define INVERTCOLORS  (Appearance.InverseInfoBox)


extern void DrawGlideCircle(HDC hdc, POINT Orig, RECT rc );
#ifdef CPUSTATS
extern void DrawCpuStats(HDC hdc, RECT rc );
#endif
#ifdef DRAWDEBUG
extern void DrawDebug(HDC hdc, RECT rc );
#endif

#define NUMSNAILRAMP 6

#define DONTDRAWTHEMAP !mode.AnyPan()&&MapSpaceMode!=MSM_MAP
#define MAPMODE8000    !mode.AnyPan()&&MapSpaceMode==MSM_MAP

extern int GetOvertargetIndex(void);

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


MapWindow::Zoom MapWindow::zoom;
MapWindow::Mode MapWindow::mode;

HBRUSH  MapWindow::hAboveTerrainBrush;

HPEN    MapWindow::hpCompassBorder;
HBRUSH  MapWindow::hBrushFlyingModeAbort;
int MapWindow::SnailWidthScale = 16;

int MapWindow::ScaleListCount = 0;
double MapWindow::ScaleList[];
int MapWindow::ScaleCurrent;

POINT MapWindow::Orig_Screen;

RECT MapWindow::MapRect;

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

bool MapWindow::targetMoved = false;
double MapWindow::targetMovedLat = 0;
double MapWindow::targetMovedLon = 0;

int MapWindow::TargetPanIndex = 0;
double MapWindow::TargetZoomDistance = 500.0;
bool MapWindow::EnableTrailDrift=false;
int MapWindow::GliderScreenPosition = 40; // 20% from bottom
int MapWindow::GliderScreenPositionX = 50;  // 100216
int MapWindow::GliderScreenPositionY = 40;


unsigned char MapWindow::DeclutterLabels = MAPLABELS_ALLON;


double MapWindow::DisplayAngle = 0.0;
double MapWindow::DisplayAircraftAngle = 0.0;

DWORD MapWindow::targetPanSize = 0;

bool MapWindow::LandableReachable = false;

HPEN MapWindow::hSnailPens[NUMSNAILCOLORS];
COLORREF MapWindow::hSnailColours[NUMSNAILCOLORS];

POINT MapWindow::Groundline[NUMTERRAINSWEEPS+1];

// 16 is number of airspace types
int      MapWindow::iAirspaceBrush[AIRSPACECLASSCOUNT] = 
  {2,0,0,0,3,3,3,3,0,3,2,3,3,3,3,3};
int      MapWindow::iAirspaceColour[AIRSPACECLASSCOUNT] = 
  {5,0,0,10,0,0,10,2,0,10,9,3,7,7,7,10};
int      MapWindow::iAirspaceMode[AIRSPACECLASSCOUNT] =
  {0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0};

HPEN MapWindow::hAirspacePens[AIRSPACECLASSCOUNT];
HPEN MapWindow::hAirspaceBorderPen;
bool MapWindow::bAirspaceBlackOutline = false;

HBRUSH  MapWindow::hBackgroundBrush;
HBRUSH  MapWindow::hInvBackgroundBrush[LKMAXBACKGROUNDS];

HBRUSH  MapWindow::hAirspaceBrushes[NUMAIRSPACEBRUSHES];

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


bool MapWindow::ForceVisibilityScan = false;


NMEA_INFO MapWindow::DrawInfo;
DERIVED_INFO MapWindow::DerivedDrawInfo;

extern int iround(double i);
extern void ShowMenu();


#ifdef DRAWLOAD
DWORD timestats_av = 0;
#endif

DWORD MapWindow::timestamp_newdata=0;
//#ifdef (DEBUG_MEM) 100211
#if defined DRAWLOAD || defined DEBUG_MEM
int cpuload=0;
#endif

bool timestats_dirty=false;

void MapWindow::UpdateTimeStats(bool start) {
#ifdef DRAWLOAD
  static DWORD tottime=0;
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
  int i;
  if(pan && (mode.Is(Mode::MODE_PAN) || mode.Is(Mode::MODE_TARGET_PAN)))
    // nearest to center of screen if in pan mode
    i=FindNearestWayPoint(PanLongitude, PanLatitude, range);
  else
    i=FindNearestWayPoint(lon, lat, range);
  if(i != -1)
    {
      SelectedWaypoint = i;
      PopupWaypointDetails();
      return true;
    }

  return false;
}


bool MapWindow::Event_InteriorAirspaceDetails(double lon, double lat) {
  bool found=false;
  CAirspaceList reslist = CAirspaceManager::Instance().GetVisibleAirspacesAtPoint(lon, lat);
  CAirspaceList::iterator it;
  for (it = reslist.begin(); it != reslist.end(); ++it) {
	dlgAirspaceDetails(*it);
	found = true;
  }
  return found; // nothing found..
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
	  if (Mode.AsFlag.SetTextColor) TextColor(hDC,Mode.AsFlag.Color); else TextColor(hDC, TEXTBLACK);
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
      if (OutlinedTp)
	SetTextColor(hDC,RGB_BLACK);
      else
	SetTextColor(hDC,RGB_WHITE); 

#ifdef WINE
      SetBkMode(hDC,TRANSPARENT);
      ExtTextOut(hDC, x+2, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x+1, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-1, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-2, y, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y+1, 0, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y-1, 0, NULL, Value, size, NULL);
#else /* WINE */
      ExtTextOut(hDC, x+2, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x+1, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-1, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x-2, y, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y+1, ETO_OPAQUE, NULL, Value, size, NULL);
      ExtTextOut(hDC, x, y-1, ETO_OPAQUE, NULL, Value, size, NULL);
#endif /* WINE */

      if (OutlinedTp) {
	TextColor(hDC,Mode.AsFlag.Color);
      } else
	SetTextColor(hDC,RGB_BLACK); 

#ifdef WINE
      ExtTextOut(hDC, x, y, 0, NULL, Value, size, NULL);
#else
      ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, Value, size, NULL);
#endif /* WINE */
      if (OutlinedTp)
	SetTextColor(hDC,RGB_BLACK); // TODO somewhere else text color is not set correctly
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
		TextColor(hDC,Mode.AsFlag.Color);
      		ExtTextOut(hDC, x, y, 0, NULL, Value, size, NULL);
		SetTextColor(hDC,RGB_BLACK); 
#else
		TextColor(hDC,Mode.AsFlag.Color);
      		ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, Value, size, NULL);
		SetTextColor(hDC,RGB_BLACK); 
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



void MapWindow::Event_PanCursor(int dx, int dy) {
  int X= (MapRect.right+MapRect.left)/2;
  int Y= (MapRect.bottom+MapRect.top)/2;
  double Xstart, Ystart, Xnew, Ynew;

  Screen2LatLon(X, Y, Xstart, Ystart);

  X+= (MapRect.right-MapRect.left)*dx/4;
  Y+= (MapRect.bottom-MapRect.top)*dy/4;
  Screen2LatLon(X, Y, Xnew, Ynew);

  if(mode.AnyPan()) {
    PanLongitude += Xstart-Xnew;
    PanLatitude += Ystart-Ynew;
  }
  RefreshMap();
}

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


void MapWindow::Event_Pan(int vswitch) {
  //  static bool oldfullscreen = 0;  never assigned!
  bool oldPan = mode.AnyPan();
  if (vswitch == -2) { // superpan, toggles fullscreen also

    // new mode
    mode.Special(Mode::MODE_SPECIAL_PAN, !oldPan);

  } else if (vswitch == -1) {
    mode.Special(Mode::MODE_SPECIAL_PAN, !oldPan);
  } else {
    mode.Special(Mode::MODE_SPECIAL_PAN, vswitch != 0); // 0 off, 1 on
  }

  if (mode.AnyPan() != oldPan) {
    if (mode.AnyPan()) {
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

  if (zoom.AutoZoom() && !mode.Is(Mode::MODE_CIRCLING)) {
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


int MapWindow::GetMapResolutionFactor(void) { // TESTFIX 091017 CHECKFIX
  static int retglider=NIBLSCALE(30);
  //static int retpara=IBLSCALE(30);
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

  bool dontdrawthemap=(DONTDRAWTHEMAP);
  bool mapmode8000=(MAPMODE8000);

  static short navboxesY;
  // Attention... this is duplicated inside Utils2, I am lazy 
  // apparently only #include is duplicated, so no problems
  static bool doinit=true;
  static int AIRCRAFTMENUSIZE=0, COMPASSMENUSIZE=0;

  navboxesY=(MapWindow::MapRect.bottom-MapWindow::MapRect.top)-BottomSize-NIBLSCALE(2); // BUGFIX 091125


  switch (uMsg)
    {
    case WM_ERASEBKGND:
      return TRUE;
    case WM_SIZE:

      hDrawBitMap = CreateCompatibleBitmap (hdcScreen, width, height);
      SelectObject(hdcDrawWindow, (HBITMAP)hDrawBitMap);

      hDrawBitMapTmp = CreateCompatibleBitmap (hdcScreen, width, height);
      SelectObject(hDCTemp, (HBITMAP)hDrawBitMapTmp);

      hMaskBitMap = CreateBitmap(width+1, height+1, 1, 1, NULL);
      SelectObject(hDCMask, (HBITMAP)hMaskBitMap);

      break;

    case WM_CREATE:

      hdcScreen = GetDC(hWnd);
      hdcDrawWindow = CreateCompatibleDC(hdcScreen);
      hDCTemp = CreateCompatibleDC(hdcDrawWindow);
      hDCMask = CreateCompatibleDC(hdcDrawWindow);
  
      AlphaBlendInit();

      LKLoadFixedBitmaps();
    
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

      for (i=0; i<NUMAIRSPACEBRUSHES; i++) {
	hAirspaceBrushes[i] =
	  CreatePatternBrush((HBITMAP)hAirspaceBitmap[i]);
      }
      hAboveTerrainBrush = CreatePatternBrush((HBITMAP)hAboveTerrainBitmap);

	int iwidth;
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

      hpCompassBorder = LKPen_Black_N2;
      hpAircraft = LKPen_White_N3;
      hpAircraftBorder = LKPen_Black_N1;

      hpWind = LKPen_Black_N2;

      hpWindThick = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(4), RGB(255,220,220));

      if (ISGAAIRCRAFT)
	hpBearing = LKPen_GABRG;
      else
	hpBearing = LKPen_Black_N2;

      hpBestCruiseTrack = LKPen_Blue_N1;
      hpCompass = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_BLACK);
      hpThermalBand = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB(0x40,0x40,0xFF));
      hpThermalBandGlider = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB(0x00,0x00,0x30));

      hpFinalGlideBelow = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(0xFF,0xA0,0xA0));
      hpFinalGlideBelowLandable = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(255,196,0));

      // TODO enhancement: support red/green Color blind
      hpFinalGlideAbove = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(0xA0,0xFF,0xA0));

      hpSpeedSlow=LKPen_Red_N1;
      hpSpeedFast=LKPen_Green_N1;
      hpStartFinishThin=LKPen_Red_N1;
      hpMapScale = LKPen_Black_N1;


      hpStartFinishThick=(HPEN)CreatePen(PS_SOLID, NIBLSCALE(5), taskcolor);
      hpMapScale2 = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1)+1, RGB_BLACK);
      // TerrainLine is for shade, Bg is for perimeter
      hpTerrainLine = (HPEN)CreatePen(PS_DASH, (1), RGB(0x30,0x30,0x30));
      hpTerrainLineBg = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB_LCDDARKGREEN);
      hpVisualGlideLightBlack = (HPEN)CreatePen(PS_DASH, (1), RGB_BLACK);
      hpVisualGlideHeavyBlack = (HPEN)CreatePen(PS_DASH, (2), RGB_BLACK);
      hpVisualGlideLightRed = (HPEN)CreatePen(PS_DASH, (1), RGB_RED);
      hpVisualGlideHeavyRed = (HPEN)CreatePen(PS_DASH, (2), RGB_RED);

      hbThermalBand=LKBrush_Emerald;
      hbCompass=LKBrush_White;
      hbBestCruiseTrack=LKBrush_Blue;
      hbFinalGlideBelow=LKBrush_Red;
      hbFinalGlideAbove=LKBrush_Green;
      hbFinalGlideBelowLandable=LKBrush_Orange;
      hbWind=LKBrush_Grey;

      ScaleListCount = propGetScaleList(ScaleList, sizeof(ScaleList)/sizeof(ScaleList[0]));
      zoom.RequestedScale(LimitMapScale(zoom.RequestedScale()));

      hBrushFlyingModeAbort = LKBrush_Red;

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

      AlphaBlendDestroy();

      DeleteObject((HPEN)hpWindThick);

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

      DeleteObject((HPEN)hpCompassBorder);

      for(i=0;i<NUMAIRSPACEBRUSHES;i++)
	{
	  DeleteObject(hAirspaceBrushes[i]);
	}

      for (i=0; i<AIRSPACECLASSCOUNT; i++) {
	DeleteObject(hAirspacePens[i]);
      }
      DeleteObject(hAirspaceBorderPen);

      for (i=0; i<NUMSNAILCOLORS; i++) {
	DeleteObject(hSnailPens[i]);
      }
    
      LKUnloadFixedBitmaps(); // After removing brushes using Bitmaps
      LKUnloadProfileBitmaps(); 
      PostQuitMessage (0);

      break;

    case WM_LBUTTONDBLCLK: 
      // VNT TODO: do not handle this event and remove CS_DBLCLKS in register class.
      // Only handle timed clicks in BUTTONDOWN with no proximity. 
      //
      // Attention please: a DBLCLK is followed by a simple BUTTONUP with NO buttondown.

      dwDownTime = GetTickCount();  
      XstartScreen = LOWORD(lParam); YstartScreen = HIWORD(lParam);

	if (LockModeStatus) {
        	ignorenext=true; // do not interpret the second click of the doubleclick as a real click.
		if (XstartScreen <(MapRect.right-BottomSize-NIBLSCALE(15))) break;
		if (YstartScreen<(MapRect.bottom-MapRect.top-BottomSize-NIBLSCALE(15))) break;
		LockMode(2);
		DoStatusMessage(gettext(_T("_@M964_"))); // SCREEN IS UNLOCKED
	}

      // Careful! If you ignorenext, any event timed as double click of course will be affected.
      // and this means also fast clicking on bottombar!!
      // so first lets see if we are in lk8000 text screens.. 
      if (dontdrawthemap || (mapmode8000 && (YstartScreen >=navboxesY))) {  
		// do not ignore next, let buttonup get the signal
		break;
      }

	// only when activemap (which means newmap is on) is off
      if (ActiveMap) ignorenext=true;
      break;

    case WM_LBUTTONDOWN:
      #ifdef DEBUG_DBLCLK
      DoStatusMessage(_T("BUTTONDOWN MapWindow")); 
      #endif
      if (LockModeStatus) break;
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

      FullScreen();
      break;

    case WM_LBUTTONUP:
	if (LockModeStatus) break;
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

        if (AATEnabled && mode.Is(Mode::MODE_TARGET_PAN)) {
          Screen2LatLon(X, Y, Xlat, Ylat);
          LockTaskData();
          targetMoved = true;
          targetMovedLat = Ylat;
          targetMovedLon = Xlat;
          UnlockTaskData();
          break;
        }

      // Process Active Icons
	if (doinit) {
		#include "./LKinclude_menusize.cpp"
		doinit=false;
	}

	short topicon;
	if (DrawBottom) topicon=MapRect.bottom-MapRect.top-BottomSize-14; // 100305
		else
			topicon=MapRect.bottom-MapRect.top-AIRCRAFTMENUSIZE;

if ( (X > ((MapRect.right-MapRect.left)- AIRCRAFTMENUSIZE)) &&
   (Y > topicon) ) {

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
			if (mode.Is(Mode::MODE_CIRCLING)) {
				mode.UserForcedMode(Mode::MODE_FLY_CRUISE);
				#ifndef DISABLEAUDIO
				if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				#endif
				break;
			} else 
			if (mode.Is(Mode::MODE_CRUISE)) {
				mode.UserForcedMode(Mode::MODE_FLY_NONE);
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

			// if we are running a real task, with gates, and we could still start
			// if only 1 time gate, and we passed valid start, no reason to resettask
			int acceptreset=2;
			if (PGNumberOfGates==1) acceptreset=1;

			if (UseGates() && ValidTaskPoint(1) && ActiveWayPoint<acceptreset) { // 100507 101110
				//
				// Reset task
				//
				InputEvents::eventResetTask(_T(""));
			} else {
				//
				// Report UTM position
				//
				InputEvents::eventService(_T("UTMPOS"));
			}
			break;
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
	if (  DrawBottom && (Y >= (rc.bottom-BottomSize)) && !mode.AnyPan() ) {
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


      if (dwInterval == 0) {
		break; // should be impossible
      }

	// we need to calculate it here only if needed
	if (gestDist>=0)
		distance = gestDist /ScreenScale;
	else
		distance = isqrt4((long)((XstartScreen-X)*(XstartScreen-X)+ (YstartScreen-Y)*(YstartScreen-Y))) /ScreenScale;

	#ifdef DEBUG_VIRTUALKEYS
	TCHAR buf[80]; char sbuf[80];
	sprintf(sbuf,"%.0f",distance);
	wsprintf(buf,_T("XY=%d,%d dist=%S Up=%ld Down=%ld Int=%ld"),X,Y,sbuf,dwUpTime,dwDownTime,dwInterval);
        DoStatusMessage(buf);
	#endif

	// Handling double click passthrough
	// Caution, timed clicks from PC with a mouse are different from real touchscreen devices

      // Process faster clicks here and no precision, but let DBLCLK pass through
      // VK are used in the bottom line in this case, forced on for this situation.
      if (  DrawBottom && (Y >= (rc.bottom-BottomSize)) ) {
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
    
      if (!mode.Is(Mode::MODE_TARGET_PAN) && mode.Is(Mode::MODE_PAN) && (distance>36)) { // TODO FIX should be IBLSCALE 36 instead?
	PanLongitude += Xstart-Xlat;
	PanLatitude  += Ystart-Ylat;
	RefreshMap();
	// disable picking when in pan mode
	break; 
      } 
      else if (SIMMODE && (!mode.Is(Mode::MODE_TARGET_PAN) && (distance>NIBLSCALE(36)))) {
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
		if (ISPARAGLIDER)
		  GPS_INFO.Speed = min(16.0,max(minspeed,distance/9));
		else
		  GPS_INFO.Speed = min(100.0,max(minspeed,distance/3));
	} 
	GPS_INFO.TrackBearing = newbearing;
	TriggerGPSUpdate();
      
	break;
      }

      if (!mode.Is(Mode::MODE_TARGET_PAN)) {
		
		//
		// Finally process normally a click on the moving map.
		//
			if(dwInterval < AIRSPACECLICK) { // original and untouched interval
				if (ActiveMap) {
                                  if (Event_NearestWaypointDetails(Xstart, Ystart, 500*zoom.Scale(), false)) {
						break;
					}
				} else {
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
			} else {
				if (SIMMODE) {
					if (mode.AnyPan()) {
						// match only center screen
						if (  (abs(X-((rc.left+rc.right)/2)) <NIBLSCALE(5)) && 
						      (abs(Y-((rc.bottom+rc.top)/2)) <NIBLSCALE(5)) ) {
							// LKTOKEN  _@M204_ = "Current position updated" 
							DoStatusMessage(gettext(TEXT("_@M204_")));
							GPS_INFO.Latitude=PanLatitude;
							GPS_INFO.Longitude=PanLongitude;
							LastDoRangeWaypointListTime=0; // force DoRange
							break;
						}
						// if we are not long clicking in center screen, before setting new position
						// we check if we are on an airspace for informations, if activemap is on
						if (OnAirSpace && Event_InteriorAirspaceDetails(Xstart, Ystart)) {
							break;
						}
						// Ok, so we reposition the aircraft
						Screen2LatLon(X,Y,PanLongitude,PanLatitude);
						// LKTOKEN  _@M204_ = "Current position updated" 
						DoStatusMessage(gettext(TEXT("_@M204_")));
						GPS_INFO.Latitude=PanLatitude;
						GPS_INFO.Longitude=PanLongitude;
						LastDoRangeWaypointListTime=0;
						break;
					}
				}
				if (!mode.AnyPan()) {
					// Select airspace on moving map only if they are visible, (and activemap optionally!)
					if (OnAirSpace && Event_InteriorAirspaceDetails(Xstart, Ystart))
						break;

					// match only center screen
					if (  (abs(X-((rc.left+rc.right)/2)) <NIBLSCALE(100)) && 
					      (abs(Y-((rc.bottom+rc.top)/2)) <NIBLSCALE(100)) ) {

						if (CustomKeyHandler(CKI_CENTERSCREEN)) break;
					}
				}
				// else ignore
				/*
				// Pan mode airspace details
				if (!OnAirSpace) break; 
				if (Event_InteriorAirspaceDetails(Xstart, Ystart)) {
					break;
				}
				*/
				break;
			}
      } // !TargetPan

      break;


    // VENTA-TODO careful here, keyup no more trapped for PNA. 
    // Forbidden usage of keypress timing.
    #if defined(PNA)
    case WM_KEYDOWN:
    #else
    case WM_KEYUP: 
    #endif

      #ifdef VENTA_DEBUG_KEY
      TCHAR ventabuffer[80];
      wsprintf(ventabuffer,TEXT("WMKEY uMsg=%d wParam=%ld lParam=%ld"), uMsg, wParam,lParam);
      DoStatusMessage(ventabuffer);
      #endif

      #if defined(PNA)

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

      #endif // PNA TRANSCODING

      dwDownTime= 0L;

      InputEvents::processKey(wParam);
      dwDownTime= 0L;
      return TRUE; 
    }

  return (DefWindowProc (hWnd, uMsg, wParam, lParam));
}


bool MapWindow::GliderCenter=false;


void MapWindow::CalculateOrientationNormal(void) {
  double trackbearing = DrawInfo.TrackBearing;

  if( (DisplayOrientation == NORTHUP) ||
      ((DisplayOrientation == NORTHTRACK) &&(!mode.Is(Mode::MODE_CIRCLING)))
	|| (DisplayOrientation == NORTHSMART) || 
	( ((DisplayOrientation == NORTHCIRCLE) ||(DisplayOrientation==TRACKCIRCLE)) && (mode.Is(Mode::MODE_CIRCLING)) ) )
  {
		if (mode.Is(Mode::MODE_CIRCLING))
			GliderCenter=true;
		else
			GliderCenter=false;
    
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
  // Target pan mode, show target up when looking at current task point,
  // otherwise north up.
  GliderCenter = true;
  if ((ActiveWayPoint==TargetPanIndex)
      &&(DisplayOrientation != NORTHUP)
      &&(DisplayOrientation != NORTHSMART) // 100419
      &&(DisplayOrientation != NORTHTRACK)) {
    // target-up
    DisplayAngle = DerivedDrawInfo.WaypointBearing;
    DisplayAircraftAngle = DrawInfo.TrackBearing-DisplayAngle;
  }
  else {
    // North up
    DisplayAngle = 0.0;
    DisplayAircraftAngle = DrawInfo.TrackBearing;
  }
}


void MapWindow::CalculateOrigin(const RECT rc, POINT *Orig)
{
  if (mode.Is(Mode::MODE_TARGET_PAN)) {
	CalculateOrientationTargetPan();
  } else {
	CalculateOrientationNormal();
  }
  
  if(mode.Is(Mode::MODE_TARGET_PAN)) {
    if (ScreenLandscape) {
      Orig->x = (rc.left + rc.right - targetPanSize)/2;
      Orig->y = (rc.bottom + rc.top)/2;
    }
    else {
      Orig->x = (rc.left + rc.right)/2;
      Orig->y = (rc.bottom + rc.top + targetPanSize)/2;
    }
  }
  else if(mode.Is(Mode::MODE_PAN) || mode.Is(Mode::MODE_CIRCLING)) {
	Orig->x = (rc.left + rc.right)/2;
	Orig->y = (rc.bottom + rc.top)/2;
  } else {
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
	} else {
		// 100924 if we are in north up autorient, position the glider in middle screen
		if ((zoom.Scale()*1.4) >= AutoOrientScale) {
			Orig->x = (rc.left + rc.right)/2;
			Orig->y=((rc.bottom-BottomSize)+rc.top)/2;
		} else {
			// else do it normally using configuration
			Orig->x = ((rc.right - rc.left )*GliderScreenPositionX/100)+rc.left;
			Orig->y = ((rc.top - rc.bottom )*GliderScreenPositionY/100)+rc.bottom;
		}
	}
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
  if (!EnableThermalLocator) return;

  if (mode.Is(Mode::MODE_CIRCLING)) {
	if (DerivedDrawInfo.ThermalEstimate_R>0) {
		LatLon2Screen(DerivedDrawInfo.ThermalEstimate_Longitude, DerivedDrawInfo.ThermalEstimate_Latitude, screen);
		DrawBitmapIn(hdc, screen, hBmpThermalSource);

		SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		double tradius;
		if (ISPARAGLIDER)
			tradius=50;
		else
			tradius=100;
			
		oldPen=(HPEN)SelectObject(hdc, LKPen_White_N3); 
		Circle(hdc, screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify()), rc);
		SelectObject(hdc, hpAircraftBorder); 
		Circle(hdc, screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify())+NIBLSCALE(2), rc);
		Circle(hdc, screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify()), rc);

		/* 101219 This would display circles around the simulated thermal, but people is confused.
		if (SIMMODE && (ThLatitude>1 && ThLongitude>1)) { // there's a thermal to show
			if ((counter==5 || counter==6|| counter==7)) {
				LatLon2Screen(ThLongitude, ThLatitude, screen);
				SelectObject(hdc, hSnailPens[7]);  
				Circle(hdc, screen.x, screen.y, (int)(ThermalRadius*zoom.ResScaleOverDistanceModify()), rc); 
				SelectObject(hdc, hSnailPens[7]); 
				Circle(hdc, screen.x, screen.y, (int)((ThermalRadius+SinkRadius)*zoom.ResScaleOverDistanceModify()), rc); 
			}
			if (++counter>=60) counter=0;
		}
 		*/

		SelectObject(hdc,oldPen);
	}
  } else {
	if (zoom.Scale() <= 4) {
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

  // Calculations are taking time and slow down painting of map, beware
  #define MULTICALC_MINROBIN	5	// minimum split
  #define MULTICALC_MAXROBIN	20	// max split
  static short multicalc_slot=0;// -1 (which becomes immediately 0) will force full loading on startup, but this is not good
				// because currently we are not waiting for ProgramStarted=3
				// and the first scan is made while still initializing other things

  // TODO assign numslots with a function, based also on available CPU time
  short numslots=1;
  if (NumberOfWayPoints>200) {
	numslots=NumberOfWayPoints/400;
	// keep numslots optimal
	if (numslots<MULTICALC_MINROBIN) numslots=MULTICALC_MINROBIN; // seconds for full scan, as this is executed at 1Hz
	if (numslots>MULTICALC_MAXROBIN) numslots=MULTICALC_MAXROBIN;

	// When waypointnumber has changed, we wont be using an exceeded multicalc_slot, which would crash the sw
	// In this case, we shall probably continue for the first round to calculate without going from the beginning
	// but this is not a problem, we are round-robin all the time here.
	if (++multicalc_slot>numslots) multicalc_slot=1;
  } else {
	multicalc_slot=0; // forcing full scan
  }

  // Here we calculate arrival altitude, GD etc for map waypoints. Splitting with multicalc will result in delayed
  // updating of visible landables, for example. The nearest pages do this separately, with their own sorting.
  // Basically we assume -like for nearest- that values will not change that much in the multicalc split time.
  // Target and tasks are recalculated in real time in any case. Nearest too. 
  LKCalculateWaypointReachable(multicalc_slot, numslots);
  CalculateScreenPositionsAirspace();
  CalculateScreenPositionsThermalSources();
  CalculateScreenPositionsGroundline();

  if (PGZoomTrigger) {
    if(!mode.Is(Mode::MODE_PANORAMA)) {
      mode.Special(Mode::MODE_SPECIAL_PANORAMA, true);
		LastZoomTrigger=GPS_INFO.Time;
      
		Message::Lock();
	        Message::AddMessage(1000, 3, gettext(TEXT("_@M872_"))); // LANDSCAPE ZOOM FOR 20s
		Message::Unlock();
      		#ifndef DISABLEAUDIO
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_TONEUP"));
		#endif
    }
    else {
		// previously called, see if time has passed
		if ( GPS_INFO.Time > (LastZoomTrigger + 20.0)) {
			// time has passed, lets go back
			LastZoomTrigger=0; // just for safety
        mode.Special(Mode::MODE_SPECIAL_PANORAMA, false);
			PGZoomTrigger=false;
			Message::Lock(); 
	        	Message::AddMessage(1500, 3, gettext(TEXT("_@M873_"))); // BACK TO NORMAL ZOOM
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
	SelectObject(hdc, hInvBackgroundBrush[BgMapColor]);
	SelectObject(hdc, GetStockObject(WHITE_PEN));

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
  
  if (zoom.BigZoom()) {
    zoom.BigZoom(false);
  }
  
  if (DONTDRAWTHEMAP) { // 100319
	SelectObject(hdcDrawWindow, hfOld);
	goto QuickRedraw;
  }

  if ((EnableTerrain && (DerivedDrawInfo.TerrainValid) 
       && RasterTerrain::isTerrainLoaded())
	) {
	// sunelevation is never used, it is still a todo in Terrain
	double sunelevation = 40.0;
	double sunazimuth=GetAzimuth();

    if (MapDirty) {
      // map has been dirtied since we started drawing, so hurry up
      zoom.BigZoom(true);
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
  
  if (ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) { // 100503
	DrawTaskAAT(hdc, rc);
  }

  
 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
 
  if (OnAirSpace > 0)  // Default is true, always true at startup no regsave 
  {
    if ( (GetAirSpaceFillType() == asp_fill_ablend_full) || (GetAirSpaceFillType() == asp_fill_ablend_borders) )
      DrawTptAirSpace(hdc, rc);
    else
      DrawAirSpace(hdc, rc);
  }

 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
  
  if(TrailActive) {
    // TODO enhancement: For some reason, the shadow drawing of the 
    // trail doesn't work in portrait mode.  No idea why.

      double TrailFirstTime = 
	LKDrawTrail(hdc, Orig_Aircraft, rc);
  }

 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}

  DrawThermalEstimate(hdc, rc);
  if (OvertargetMode==OVT_THER) DrawThermalEstimateMultitarget(hdc, rc);
 
  if (ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) { // 100503
	DrawTask(hdc, rc, Orig_Aircraft);
  }
  
  // draw red cross on glide through terrain marker
  if (FinalGlideTerrain && DerivedDrawInfo.TerrainValid) {
    DrawGlideThroughTerrain(hdc, rc);
  }
  
 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
  if ((OnAirSpace > 0) && AirspaceWarningMapLabels)
  {
	DrawAirspaceLabels(hdc, rc, Orig_Aircraft);
	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
  }
  DrawWaypointsNew(hdc,rc);
 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}

  DrawTeammate(hdc, rc);

  if (extGPSCONNECT) {
    // TODO enhancement: don't draw offtrack indicator if showing spot heights
    DrawProjectedTrack(hdc, rc, Orig_Aircraft);
    // DrawOffTrackIndicator(hdc, rc); 
    DrawBestCruiseTrack(hdc, Orig_Aircraft);
    DrawBearing(hdc, rc);
  }

  // draw wind vector at aircraft
  if (!mode.AnyPan()) {
    DrawWindAtAircraft2(hdc, Orig_Aircraft, rc);
  } else if (mode.Is(Mode::MODE_TARGET_PAN)) {
    DrawWindAtAircraft2(hdc, Orig, rc);
  }

  // VisualGlide drawn BEFORE lk8000 overlays
  if (!mode.AnyPan() && VisualGlide > 0) {
    DrawGlideCircle(hdc, Orig, rc); 
  }

 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}

  // Draw traffic and other specifix LK gauges
  	LKDrawFLARMTraffic(hdc, rc, Orig_Aircraft);
	if ( !mode.AnyPan()) DrawLook8000(hdc,rc); 
	if (LKVarioBar && !mode.AnyPan()) // 091214 do not draw Vario when in Pan mode
		LKDrawVario(hdc,rc); // 091111
  
  // finally, draw you!
  // Draw cross air for panmode, instead of aircraft icon
  if (mode.AnyPan() && !mode.Is(Mode::MODE_TARGET_PAN)) {
    DrawCrossHairs(hdc, Orig, rc);
  }

  // Draw glider or paraglider
  if (extGPSCONNECT) {
    DrawAircraft(hdc, Orig_Aircraft);
  }

  if (!mode.AnyPan()) {
	if (TrackBar) DrawHeading(hdc, Orig, rc); 
  }

  #if USETOPOMARKS
  // marks on top...
  DrawMarks(hdc, rc);
  #endif

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

  if (LockModeStatus) LockMode(9); // check if unlock is now possible 
  
  POINT Orig, Orig_Aircraft;

  SetAutoOrientation(false); // false for no reset Old values
  CalculateOrigin(rc, &Orig);

  // this is calculating waypoint visible, and must be executed before rendermapwindowbg which calls   
  // CalculateWayPointReachable new, setting values for visible wps!
  // This is also calculating CalculateScreenBounds 0.0  and placing it inside MapWindow::screenbounds_latlon
  CalculateScreenPositions(Orig, rc, &Orig_Aircraft);

  LKUpdateOlc();

  RenderMapWindowBg(hdcDrawWindow, rc, Orig, Orig_Aircraft);

  if (DONTDRAWTHEMAP) {
  	DrawFlightMode(hdcDrawWindow, rc);
  	DrawGPSStatus(hdcDrawWindow, rc);
	DrawLKAlarms(hdcDrawWindow, rc);
	#if (WINDOWSPC<1)
	LKBatteryManager();
	#endif

	return;
  }
  // overlays

  hfOld = (HFONT)SelectObject(hdcDrawWindow, MapWindowFont);
  
  DrawMapScale(hdcDrawWindow,rc, zoom.BigZoom());

  DrawCompass(hdcDrawWindow, rc);
  
  DrawFlightMode(hdcDrawWindow, rc);
  
  if (!mode.AnyPan()) {
    // REMINDER TODO let it be configurable for not circling also, as before
    if ((mode.Is(Mode::MODE_CIRCLING)) )
      if (ThermalBar) DrawThermalBand(hdcDrawWindow, rc); // 091122
    
    DrawFinalGlide(hdcDrawWindow,rc);
  }
  
  // DrawSpeedToFly(hdcDrawWindow, rc);  // Usable

  DrawGPSStatus(hdcDrawWindow, rc);

  #if (WINDOWSPC<1)
  LKBatteryManager();
  #endif

  DrawLKAlarms(hdcDrawWindow, rc);

  SelectObject(hdcDrawWindow, hfOld);

}


void MapWindow::UpdateInfo(NMEA_INFO *nmea_info,
                           DERIVED_INFO *derived_info) {
  LockFlightData();
  memcpy(&DrawInfo,nmea_info,sizeof(NMEA_INFO));
  memcpy(&DerivedDrawInfo,derived_info,sizeof(DERIVED_INFO));
  zoom.UpdateMapScale(); // done here to avoid double latency due to locks 
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

  if (EnableTerrain) {
	if (RenderTimeAvailable() || ((fpsTimeThis-fpsTimeLast_terrain)>5000) || force) {
		fpsTimeLast_terrain = fpsTimeThis;
		RasterTerrain::ServiceCache();
	}
  }
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
    cost = ICOSTABLE[deg]*ScreenScale;
    sint = ISINETABLE[deg]*ScreenScale;
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
      hbPAircraftSolid = LKBrush_LightCyan;
      hbPAircraftSolidBg = LKBrush_Blue;
    } else {
      hbPAircraftSolid = LKBrush_Blue;
      hbPAircraftSolidBg = LKBrush_Grey;
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

    return;
  }

  if ( ISGAAIRCRAFT ) {

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

    hbAircraftSolid = LKBrush_Black;
    hbAircraftSolidBg = LKBrush_White;

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
    
    return;

  }

      // GLIDER AICRAFT NORMAL ICON

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

      int n = sizeof(Aircraft)/sizeof(Aircraft[0]);

      double angle = DisplayAircraftAngle+
	(DerivedDrawInfo.Heading-DrawInfo.TrackBearing);

      PolygonRotateShift(Aircraft, n,
			 Orig.x-1, Orig.y, angle);

      oldPen = (HPEN)SelectObject(hdc, hpAircraft);
      Polygon(hdc, Aircraft, n);

      HBRUSH hbOld;
      hbOld = (HBRUSH)SelectObject(hdc, GetStockObject(BLACK_BRUSH));
      SelectObject(hdc, hpAircraftBorder); // hpBearing
      Polygon(hdc, Aircraft, n);

      SelectObject(hdc, oldPen);
      SelectObject(hdc, hbOld);


}

void MapWindow::DrawBitmapX(HDC hdc, int x, int y,
                            int sizex, int sizey,
                            HDC source,
                            int offsetx, int offsety,
                            DWORD mode) {
  if (ScreenScale>1) {
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

// This will draw both GPS and LOCK SCREEN status
void MapWindow::DrawGPSStatus(HDC hDC, const RECT rc)
{

  HFONT oldfont=NULL;
  if ((MapSpaceMode==MSM_WELCOME)||(mode.AnyPan()) ) return; // 100210

  if (!LKLanguageReady) return;

  if (extGPSCONNECT && !(DrawInfo.NAVWarning) && (DrawInfo.SatellitesUsed != 0)) {
	if (LockModeStatus) goto goto_DrawLockModeStatus;
	return;
  }

  static bool firstrun=true;

  if (!extGPSCONNECT) {

    oldfont=(HFONT)SelectObject(hDC,LK8TargetFont); 
    TextInBoxMode_t TextInBoxMode = {2};
    TextInBoxMode.AsInt=0;
    TextInBoxMode.AsFlag.Color = TEXTWHITE;
    TextInBoxMode.AsFlag.NoSetFont=1;
    TextInBoxMode.AsFlag.AlligneCenter = 1;
    TextInBoxMode.AsFlag.WhiteBorder = 1;
    TextInBoxMode.AsFlag.Border = 1;
    if (ComPortStatus[0]==CPS_OPENKO) {
    	TextInBox(hDC, gettext(_T("_@M971_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, TextInBoxMode); // No ComPort
    } else {
    	if (ComPortStatus[0]==CPS_OPENOK) {
		if ((ComPortRx[0]>0) && !firstrun) {
			// Gps is missing
    			TextInBox(hDC, gettext(_T("_@M973_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, TextInBoxMode);
			firstrun=false; // 100214
		} else {
			// No Data Rx
    			TextInBox(hDC, gettext(_T("_@M972_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, TextInBoxMode);
		}
	} else  {
		if (ComPortStatus[0]==CPS_EFRAME)  {
			// Data error
    			TextInBox(hDC, gettext(_T("_@M975_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, TextInBoxMode);
		} else {
			// Not Connected
    			TextInBox(hDC, gettext(_T("_@M974_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, TextInBoxMode);
		}
	}

    }


  } else
    if (DrawInfo.NAVWarning || (DrawInfo.SatellitesUsed == 0)) {
    oldfont=(HFONT)SelectObject(hDC,LK8TargetFont); // 100210
    TextInBoxMode_t TextInBoxMode = {2};
    TextInBoxMode.AsInt=0;
    TextInBoxMode.AsFlag.Color = TEXTWHITE;
    TextInBoxMode.AsFlag.NoSetFont=1;
    TextInBoxMode.AsFlag.AlligneCenter = 1;
    TextInBoxMode.AsFlag.WhiteBorder = 1;
    TextInBoxMode.AsFlag.Border = 1;
    // No Valid Fix
    TextInBox(hDC, gettext(_T("_@M970_")), (rc.right-rc.left)/2, (rc.bottom-rc.top)/3, 0, TextInBoxMode);

    }

goto_DrawLockModeStatus:
  if (LockModeStatus) {
	if (oldfont!=NULL)
		oldfont=(HFONT)SelectObject(hDC,LK8MediumFont);
	else
		SelectObject(hDC,LK8MediumFont);

	TextInBoxMode_t TextInBoxModeL = {2};
	TextInBoxModeL.AsInt=0;
	TextInBoxModeL.AsFlag.Color = TEXTWHITE;
	TextInBoxModeL.AsFlag.NoSetFont=1;
	TextInBoxModeL.AsFlag.AlligneCenter = 1;
	TextInBoxModeL.AsFlag.WhiteBorder = 1;
	TextInBoxModeL.AsFlag.Border = 1;
	if (ISPARAGLIDER)
	  TextInBox(hDC, gettext(_T("_@M962_")), (rc.right-rc.left)/2, rc.bottom-((rc.bottom-rc.top)/3), 0, TextInBoxModeL);
	SelectObject(hDC,LK8MapFont);
	TextInBox(hDC, gettext(_T("_@M1601_")), (rc.right-rc.left)/2, rc.bottom-((rc.bottom-rc.top)/5), 0, TextInBoxModeL);
  }

  SelectObject(hDC,oldfont);
  return;
}

void MapWindow::DrawFlightMode(HDC hdc, const RECT rc)
{
  static bool flip= true; 
  int offset = -1;

  //
  // Logger indicator
  //

  if (!DisableAutoLogger || LoggerActive) {
	flip = !flip;
	if (LoggerActive || flip) {
		if (LoggerActive)
			SelectObject(hDCTemp,hLogger);
		else
			SelectObject(hDCTemp,hLoggerOff);

		offset -= 7;

		DrawBitmapX(hdc, rc.right+IBLSCALE(offset),
			rc.bottom - BottomSize+NIBLSCALE(4),
			7,7, hDCTemp, 0,0,SRCPAINT);

		DrawBitmapX(hdc, rc.right+IBLSCALE(offset),
			rc.bottom-BottomSize+NIBLSCALE(4),
			7,7, hDCTemp, 7,0,SRCAND);

		offset +=7;
	}
  }
  

  //
  // Flight mode Icon
  //

  if (mode.Is(Mode::MODE_CIRCLING)) {
	SelectObject(hDCTemp,hClimb);
  } else
	if (mode.Is(Mode::MODE_FINAL_GLIDE)) {
		SelectObject(hDCTemp,hFinalGlide);
	} else {
		SelectObject(hDCTemp,hCruise);
	}

  offset -= 24;

  DrawBitmapX(hdc,
	rc.right+IBLSCALE(offset-1),
	rc.bottom+IBLSCALE(-20-1),
	24,20,
	hDCTemp,
	0,0,SRCPAINT);
    
  DrawBitmapX(hdc,
	rc.right+IBLSCALE(offset-1),
	rc.bottom+IBLSCALE(-20-1),
	24,20,
	hDCTemp,
	24,0,SRCAND);


  //
  // Battery indicator
  // 

  #if TESTBENCH
  // Battery test in Simmode
  if (SIMMODE) {; PDABatteryPercent-=5; if (PDABatteryPercent<0) PDABatteryPercent=100; }
  #endif

  if (PDABatteryPercent==0 && PDABatteryStatus==AC_LINE_ONLINE && PDABatteryFlag!=BATTERY_FLAG_CHARGING) {
	SelectObject(hDCTemp,hBatteryFull);
	goto _drawbattery;
  }

  if (PDABatteryPercent<20) {
	SelectObject(hDCTemp,hBattery15);
	goto _drawbattery;
  }
  if (PDABatteryPercent<45) {
	SelectObject(hDCTemp,hBattery25);
	goto _drawbattery;
  }
  if (PDABatteryPercent<65) {
	SelectObject(hDCTemp,hBattery50);
	goto _drawbattery;
  }
  if (PDABatteryPercent<90) {
	SelectObject(hDCTemp,hBattery70);
	goto _drawbattery;
  }
  SelectObject(hDCTemp,hBatteryFull);

_drawbattery:
  if (!DisableAutoLogger || LoggerActive) offset-=5;
  DrawBitmapX(hdc,
	rc.right+IBLSCALE(offset-1),
	rc.bottom - BottomSize + NIBLSCALE(2),
	22,11,
	hDCTemp,
	0,0,SRCPAINT);
    
  DrawBitmapX(hdc,
	rc.right+IBLSCALE(offset-1),
	rc.bottom - BottomSize + NIBLSCALE(2),
	22,11,
	hDCTemp,
	22,0,SRCAND);

}



bool MapWindow::WaypointInTask(int ind) {
  if (!WayPointList) return false;
  return WayPointList[ind].InTask;
}

MapWaypointLabel_t MapWaypointLabelList[200]; 
int MapWaypointLabelListCount=0;

bool MapWindow::WaypointInRange(int i) {
  return ((WayPointList[i].Zoom >= zoom.Scale()*10) 
          || (WayPointList[i].Zoom == 0)) 
    && (zoom.Scale() <= 10);
}

int _cdecl MapWaypointLabelListCompare(const void *elem1, const void *elem2 ){

  // Now sorts elements in task preferentially.
  if (((MapWaypointLabel_t *)elem1)->AltArivalAGL > ((MapWaypointLabel_t *)elem2)->AltArivalAGL)
    return (-1);
  if (((MapWaypointLabel_t *)elem1)->AltArivalAGL < ((MapWaypointLabel_t *)elem2)->AltArivalAGL)
    return (+1);
  return (0);
}


void MapWaypointLabelAdd(TCHAR *Name, int X, int Y, 
			 TextInBoxMode_t Mode, 
			 int AltArivalAGL, bool inTask, bool isLandable, bool isAirport, bool isExcluded, int index, short style){
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
  E->index = index;
  E->style = style;

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
    tmp = StartRadius*zoom.ResScaleOverDistanceModify();
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
	if (ActiveWayPoint>1 || !ValidTaskPoint(2)) { 
	  // only draw finish line when past the first
	  // waypoint. FIXED 110307: or if task is with only 2 tps
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
	    tmp = FinishRadius*zoom.ResScaleOverDistanceModify(); 
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
	    tmp = SectorRadius*zoom.ResScaleOverDistanceModify();

	    Circle(hdc,
		   WayPointList[Task[i].Index].Screen.x,
		   WayPointList[Task[i].Index].Screen.y,
		   (int)tmp, rc, false, false); 

	  }
	  // FAI SECTOR
	  if(SectorType==1) {
	    tmp = SectorRadius*zoom.ResScaleOverDistanceModify();

	    Segment(hdc,
		    WayPointList[Task[i].Index].Screen.x,
		    WayPointList[Task[i].Index].Screen.y,(int)tmp, rc, 
		    Task[i].AATStartRadial-DisplayAngle, 
		    Task[i].AATFinishRadial-DisplayAngle); 

	  }
	  if(SectorType== 2) {
	    // JMW added german rules
	    tmp = 500*zoom.ResScaleOverDistanceModify();
	    Circle(hdc,
		   WayPointList[Task[i].Index].Screen.x,
		   WayPointList[Task[i].Index].Screen.y,
		   (int)tmp, rc, false, false); 

	    tmp = 10e3*zoom.ResScaleOverDistanceModify();
          
	    Segment(hdc,
		    WayPointList[Task[i].Index].Screen.x,
		    WayPointList[Task[i].Index].Screen.y,(int)tmp, rc, 
		    Task[i].AATStartRadial-DisplayAngle, 
		    Task[i].AATFinishRadial-DisplayAngle); 

	  }
	} else {
		// ELSE HERE IS   *** AAT ***
	  // JMW added iso lines
	  if ((i==ActiveWayPoint) || (mode.Is(Mode::MODE_TARGET_PAN) && (i==TargetPanIndex))) {
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
	if (AATEnabled) {
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
	      tmp = Task[i].AATCircleRadius*zoom.ResScaleOverDistanceModify();
          
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
          
	      tmp = Task[i].AATSectorRadius*zoom.ResScaleOverDistanceModify();
          
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

  int kx = tsize.cx/ScreenScale/2;

  POINT Arrow[7] = { {0,-20}, {-6,-26}, {0,-20}, 
                     {6,-26}, {0,-20}, 
                     {8+kx, -24}, 
                     {-8-kx, -24}};

  for (i=1;i<4;i++)
    Arrow[i].y -= wmag;

  PolygonRotateShift(Arrow, 7, Start.x, Start.y, 
		     DerivedDrawInfo.WindBearing-DisplayAngle);

  //
  // Draw Wind Arrow
  //
  POINT Tail[2] = {{0,-20}, {0,-26-min(20,wmag)*3}};
  double angle = AngleLimit360(DerivedDrawInfo.WindBearing-DisplayAngle);
  for(i=0; i<2; i++) {
    if (ScreenScale>1) {
      Tail[i].x *= ScreenScale;
      Tail[i].y *= ScreenScale;
    }
    protateshift(Tail[i], angle, Start.x, Start.y);
  }
  // optionally draw dashed line for wind arrow
  _DrawLine(hdc, PS_DASH, 1, Tail[0], Tail[1], RGB(0,0,0), rc);

  // Paint wind value only while circling
  if ( (mode.Is(Mode::MODE_CIRCLING)) ) {

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
  int overindex=GetOvertargetIndex();
  if (overindex<0) return;

  double startLat = DrawInfo.Latitude;
  double startLon = DrawInfo.Longitude;
  double targetLat;
  double targetLon;

  if (overindex>OVT_TASK) {
    LockTaskData();
    if(OvertargetMode == OVT_TASK && AATEnabled && (ActiveWayPoint>0) && ValidTaskPoint(ActiveWayPoint+1)) {
      targetLat = Task[ActiveWayPoint].AATTargetLat;
      targetLon = Task[ActiveWayPoint].AATTargetLon;
    }
    else {
      targetLat = WayPointList[overindex].Latitude;
      targetLon = WayPointList[overindex].Longitude;
    }
    UnlockTaskData();
    //  DrawGreatCircle(hdc, startLon, startLat, targetLon, targetLat, rc);
    //HPEN hpOld = (HPEN)SelectObject(hdc, hpOvertarget);
    HPEN hpOld = (HPEN)SelectObject(hdc, hpBearing);
    POINT pt[2];
    LatLon2Screen(startLon, startLat, pt[0]);
    LatLon2Screen(targetLon, targetLat, pt[1]);
    ClipPolygon(hdc, pt, 2, rc, false);
    SelectObject(hdc, hpOld);
  }
  else {
    if (!ValidTaskPoint(ActiveWayPoint)) {
      return; 
    }
    LockTaskData();

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

    if (mode.Is(Mode::MODE_TARGET_PAN)) {
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
      UnlockTaskData();
    }
  }

  if (AATEnabled) {
    // draw symbol at target, makes it easier to see
    LockTaskData();
    if(mode.Is(Mode::MODE_TARGET_PAN)) {
      for(int i=ActiveWayPoint+1; i<MAXTASKPOINTS; i++) {
        if(ValidTaskPoint(i) && ValidTaskPoint(i+1)) {
          if(i>= ActiveWayPoint) {
            POINT sct;
            LatLon2Screen(Task[i].AATTargetLon, 
                          Task[i].AATTargetLat, 
                          sct);
            DrawBitmapIn(hdc, sct, hBmpTarget);
          }
        }
      }
    }
    if(ValidTaskPoint(ActiveWayPoint+1) && (ActiveWayPoint>0)) {
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
  return (zoom.Scale() * max(MapRect.right-MapRect.left,
                         MapRect.bottom-MapRect.top))
    *1000.0/GetMapResolutionFactor();
}

bool MapWindow::IsDisplayRunning() {
  return (THREADRUNNING && GlobalRunning && ProgramStarted);
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


// RETURNS Longitude, Latitude!

void MapWindow::OrigScreen2LatLon(const int &x, const int &y, 
                                  double &X, double &Y) 
{
  int sx = x;
  int sy = y;
  irotate(sx, sy, DisplayAngle);
  Y= PanLatitude  - sy*zoom.InvDrawScale();
  X= PanLongitude + sx*invfastcosine(Y)*zoom.InvDrawScale();
}


void MapWindow::Screen2LatLon(const int &x, const int &y, 
                              double &X, double &Y) 
{
  int sx = x-(int)Orig_Screen.x;
  int sy = y-(int)Orig_Screen.y;
  irotate(sx, sy, DisplayAngle);
  Y= PanLatitude  - sy*zoom.InvDrawScale();
  X= PanLongitude + sx*invfastcosine(Y)*zoom.InvDrawScale();
}

void MapWindow::LatLon2Screen(const double &lon, const double &lat, 
                              POINT &sc) {
  int Y = Real2Int((PanLatitude-lat)*zoom.DrawScale());
  int X = Real2Int((PanLongitude-lon)*fastcosine(lat)*zoom.DrawScale());
    
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
  const double mDrawScale = zoom.DrawScale();
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

