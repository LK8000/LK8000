/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindow.cpp,v 8.29 2011/01/06 02:07:52 root Exp root $
*/

#include "externs.h"
#include "LKInterface.h"
#include "MapWindow.h"
#include "Waypointparser.h"
#include "InputEvents.h"
#include <Message.h>
#include "Modeltype.h"
#include "Terrain.h"
#include "LKObjects.h"
#include "RasterTerrain.h"
#include "Bitmaps.h"
#include "RGB.h"

using std::min;
using std::max;

#include "LKGeneralAviation.h"


#ifdef DEBUG
#define DEBUG_VIRTUALKEYS
#endif


extern void DrawGlideCircle(HDC hdc, POINT Orig, RECT rc );
#ifdef CPUSTATS
extern void DrawCpuStats(HDC hdc, RECT rc );
#endif
#ifdef DRAWDEBUG
extern void DrawDebug(HDC hdc, RECT rc );
#endif

extern int iround(double i);

#define DONTDRAWTHEMAP !mode.AnyPan()&&MapSpaceMode!=MSM_MAP
#define MAPMODE8000    !mode.AnyPan()&&MapSpaceMode==MSM_MAP

rectObj MapWindow::screenbounds_latlon;
DWORD MapWindow::timestamp_newdata=0;


void MapWindow::UpdateTimeStats(bool start) {
  if (start) {
    timestamp_newdata = ::GetTickCount();
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
	// NEED REWRITING
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
  #if RASTERCACHE
  DWORD fpsTimeThis;
  static DWORD fpsTimeMapCenter = 0;
  #endif

  if (MapWindow::ForceVisibilityScan) {
    force = true;
    MapWindow::ForceVisibilityScan = false;
  }

  // have some time, do shape file cache update if necessary
  LockTerrainDataGraphics();
  SetTopologyBounds(MapRect, force);
  UnlockTerrainDataGraphics();

  #if RASTERCACHE
  // JP2 no more supported, however if rastercache will ever be enabled..
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
  #endif // RASTERCACHE
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



extern COLORREF taskcolor;

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

