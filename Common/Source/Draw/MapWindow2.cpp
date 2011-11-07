/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindow2.cpp,v 8.16 2010/12/26 22:05:15 root Exp root $
*/

#include "externs.h"
#include "LKInterface.h"
#include "MapWindow.h"
#include "Utils.h"
#include "Logger.h"
#include "Waypointparser.h"
#include "InputEvents.h"
#include "Bitmaps.h"
#include "RGB.h"

#include "Terrain.h"
#include "RasterTerrain.h"

#include "InfoBoxLayout.h"
#include "LKAirspace.h"
using std::min;
using std::max;

#if (WINDOWSPC>0)
#include <wingdi.h>
#endif


#define TASKINDEX    Task[ActiveWayPoint].Index

double MapWindow::findMapScaleBarSize(const RECT rc) {

  int range = rc.bottom-rc.top;
//  int nbars = 0;
//  int nscale = 1;
  double pixelsize = zoom.Scale()/GetMapResolutionFactor(); // km/pixel
  
  // find largest bar size that will fit in display

  double displaysize = range*pixelsize/2; // km

  if (displaysize>100.0) {
    return 100.0/pixelsize;
  }
  if (displaysize>10.0) {
    return 10.0/pixelsize;
  }
  if (displaysize>1.0) {
    return 1.0/pixelsize;
  }
  if (displaysize>0.1) {
    return 0.1/pixelsize;
  }
  // this is as far as is reasonable
  return 0.1/pixelsize;
}



rectObj MapWindow::CalculateScreenBounds(double scale) {
  // compute lat lon extents of visible screen
  rectObj sb;

  if (scale>= 1.0) {
    POINT screen_center;
    LatLon2Screen(PanLongitude, 
                  PanLatitude,
                  screen_center);
    
    sb.minx = sb.maxx = PanLongitude;
    sb.miny = sb.maxy = PanLatitude;
    
    int dx, dy;
    unsigned int maxsc=0;
    dx = screen_center.x-MapRect.right;
    dy = screen_center.y-MapRect.top;
    maxsc = max(maxsc, isqrt4(dx*dx+dy*dy));
    dx = screen_center.x-MapRect.left;
    dy = screen_center.y-MapRect.top;
    maxsc = max(maxsc, isqrt4(dx*dx+dy*dy));
    dx = screen_center.x-MapRect.left;
    dy = screen_center.y-MapRect.bottom;
    maxsc = max(maxsc, isqrt4(dx*dx+dy*dy));
    dx = screen_center.x-MapRect.right;
    dy = screen_center.y-MapRect.bottom;
    maxsc = max(maxsc, isqrt4(dx*dx+dy*dy));
    
    for (int i=0; i<10; i++) {
      double ang = i*360.0/10;
      POINT p;
      double X, Y;
      p.x = screen_center.x + iround(fastcosine(ang)*maxsc*scale);
      p.y = screen_center.y + iround(fastsine(ang)*maxsc*scale);
      Screen2LatLon(p.x, p.y, X, Y);
      sb.minx = min(X, sb.minx);
      sb.miny = min(Y, sb.miny);
      sb.maxx = max(X, sb.maxx);
      sb.maxy = max(Y, sb.maxy);
    }

  } else {

    double xmin, xmax, ymin, ymax;
    int x, y;
    double X, Y;
    
    x = MapRect.left; 
    y = MapRect.top; 
    Screen2LatLon(x, y, X, Y);
    xmin = X; xmax = X;
    ymin = Y; ymax = Y;

    x = MapRect.right; 
    y = MapRect.top; 
    Screen2LatLon(x, y, X, Y);
    xmin = min(xmin, X); xmax = max(xmax, X);
    ymin = min(ymin, Y); ymax = max(ymax, Y);
  
    x = MapRect.right; 
    y = MapRect.bottom; 
    Screen2LatLon(x, y, X, Y);
    xmin = min(xmin, X); xmax = max(xmax, X);
    ymin = min(ymin, Y); ymax = max(ymax, Y);
  
    x = MapRect.left; 
    y = MapRect.bottom; 
    Screen2LatLon(x, y, X, Y);
    xmin = min(xmin, X); xmax = max(xmax, X);
    ymin = min(ymin, Y); ymax = max(ymax, Y);
  

    sb.minx = xmin;
    sb.maxx = xmax;
    sb.miny = ymin;
    sb.maxy = ymax;

  }

  return sb;
}



void MapWindow::ScanVisibility(rectObj *bounds_active) {
  // received when the SetTopoBounds determines the visibility
  // boundary has changed.
  // This happens rarely, so it is good pre-filtering of what is visible.
  // (saves from having to do it every screen redraw)
  const rectObj bounds = *bounds_active;

  // far visibility for snail trail

  SNAIL_POINT *sv= SnailTrail;
  const SNAIL_POINT *se = sv+TRAILSIZE;
  while (sv<se) {
    sv->FarVisible = ((sv->Longitude> bounds.minx) &&
		      (sv->Longitude< bounds.maxx) &&
		      (sv->Latitude> bounds.miny) &&
		      (sv->Latitude< bounds.maxy));
    sv++;
  }

  // far visibility for waypoints

  if (WayPointList) {
    WAYPOINT *wv = WayPointList;
    const WAYPOINT *we = WayPointList+NumberOfWayPoints;
    while (wv<we) {
      // TODO code: optimise waypoint visibility
	// TODO 110203 make it happen in 3 steps, with MULTICALC approach
      wv->FarVisible = ((wv->Longitude> bounds.minx) &&
			(wv->Longitude< bounds.maxx) &&
			(wv->Latitude> bounds.miny) &&
			(wv->Latitude< bounds.maxy));
      wv++;
    }
  }

  // far visibility for airspace
  CAirspaceManager::Instance().SetFarVisible( *bounds_active );

}


void MapWindow::CalculateScreenPositionsThermalSources() {
  for (int i=0; i<MAX_THERMAL_SOURCES; i++) {
    if (DerivedDrawInfo.ThermalSources[i].LiftRate>0) {
      double dh = DerivedDrawInfo.NavAltitude
        -DerivedDrawInfo.ThermalSources[i].GroundHeight;
      if (dh<0) {
        DerivedDrawInfo.ThermalSources[i].Visible = false;
        continue;
      }

      double t = dh/DerivedDrawInfo.ThermalSources[i].LiftRate;
      double lat, lon;
      FindLatitudeLongitude(DerivedDrawInfo.ThermalSources[i].Latitude, 
                            DerivedDrawInfo.ThermalSources[i].Longitude,
                            DerivedDrawInfo.WindBearing, 
                            -DerivedDrawInfo.WindSpeed*t,
                            &lat, &lon);
      if (PointVisible(lon,lat)) {
        LatLon2Screen(lon, 
                      lat, 
                      DerivedDrawInfo.ThermalSources[i].Screen);
        DerivedDrawInfo.ThermalSources[i].Visible = 
          PointVisible(DerivedDrawInfo.ThermalSources[i].Screen);
      } else {
        DerivedDrawInfo.ThermalSources[i].Visible = false;
      }
    } else {
      DerivedDrawInfo.ThermalSources[i].Visible = false;
    }
  }
}

void MapWindow::CalculateScreenPositionsAirspace()
{
  CAirspaceManager::Instance().CalculateScreenPositionsAirspace(screenbounds_latlon, iAirspaceMode, iAirspaceBrush, zoom.ResScaleOverDistanceModify());
}

void MapWindow::CalculateScreenPositions(POINT Orig, RECT rc, 
                                         POINT *Orig_Aircraft)
{
  unsigned int i;

  Orig_Screen = Orig;

  if (!mode.AnyPan()) {
  
    if (GliderCenter 
        && DerivedDrawInfo.Circling 
        && (EnableThermalLocator==2)) {
      
      if (DerivedDrawInfo.ThermalEstimate_R>0) {
        PanLongitude = DerivedDrawInfo.ThermalEstimate_Longitude; 
        PanLatitude = DerivedDrawInfo.ThermalEstimate_Latitude;
        // TODO enhancement: only pan if distance of center to
        // aircraft is smaller than one third screen width

        POINT screen;
        LatLon2Screen(PanLongitude, 
                      PanLatitude, 
                      screen);

        LatLon2Screen(DrawInfo.Longitude, 
                      DrawInfo.Latitude, 
                      *Orig_Aircraft);
        if ((fabs((double)Orig_Aircraft->x-screen.x)<(rc.right-rc.left)/3)
            && (fabs((double)Orig_Aircraft->y-screen.y)<(rc.bottom-rc.top)/3)) {
          
        } else {
          // out of bounds, center on aircraft
          PanLongitude = DrawInfo.Longitude;
          PanLatitude = DrawInfo.Latitude;
        }
      } else {
        PanLongitude = DrawInfo.Longitude;
        PanLatitude = DrawInfo.Latitude;
      }
    } else {
      // Pan is off
      PanLongitude = DrawInfo.Longitude;
      PanLatitude = DrawInfo.Latitude;
    }
  }

  LatLon2Screen(DrawInfo.Longitude, 
                DrawInfo.Latitude, 
                *Orig_Aircraft);

  // very important
  screenbounds_latlon = CalculateScreenBounds(0.0);

  #if TOPOFASTLABEL
  // preserve this calculation for 0.0 until next round!
  // This is already done since screenbounds_latlon is global. Beware that DrawTrail will change it later on
  // to expand boundaries by 1 minute
  #endif

  // get screen coordinates for all task waypoints

  LockTaskData();

  if (WayPointList) {
    int index;
    for (i=0; i<MAXTASKPOINTS; i++) {
      index = Task[i].Index;
      if (index>=0) {
        
        LatLon2Screen(WayPointList[index].Longitude, 
                      WayPointList[index].Latitude, 
                      WayPointList[index].Screen);
        WayPointList[index].Visible = 
          PointVisible(WayPointList[index].Screen);
      }      
    }
    if (EnableMultipleStartPoints) {
      for(i=0;i<MAXSTARTPOINTS-1;i++) {
        index = StartPoints[i].Index;
        if (StartPoints[i].Active && (index>=0)) {

          LatLon2Screen(WayPointList[index].Longitude, 
                        WayPointList[index].Latitude, 
                        WayPointList[index].Screen);
          WayPointList[index].Visible = 
            PointVisible(WayPointList[index].Screen);
        }
      }
    }

    // only calculate screen coordinates for waypoints that are visible

    // TODO 110203 OPTIMIZE THIS !
    for(i=0;i<NumberOfWayPoints;i++)
      {
        WayPointList[i].Visible = false;
        if (!WayPointList[i].FarVisible) continue;
        if(PointVisible(WayPointList[i].Longitude, WayPointList[i].Latitude) )
          {
            LatLon2Screen(WayPointList[i].Longitude, WayPointList[i].Latitude,
                          WayPointList[i].Screen);
            WayPointList[i].Visible = PointVisible(WayPointList[i].Screen);
          }
      }
  }

  if(TrailActive)
  {
    iSnailNext = SnailNext; 
    // set this so that new data doesn't arrive between calculating
    // this and the screen updates
  }

  if (EnableMultipleStartPoints) {
    for(i=0;i<MAXSTARTPOINTS-1;i++) {
      if (StartPoints[i].Active && ValidWayPoint(StartPoints[i].Index)) {
        LatLon2Screen(StartPoints[i].SectorEndLon, 
                      StartPoints[i].SectorEndLat, StartPoints[i].End);
        LatLon2Screen(StartPoints[i].SectorStartLon, 
                      StartPoints[i].SectorStartLat, StartPoints[i].Start);
      }
    }
  }
  
  for(i=0;i<MAXTASKPOINTS-1;i++)
  {
    bool this_valid = ValidTaskPoint(i);
    bool next_valid = ValidTaskPoint(i+1);
    if (AATEnabled && this_valid) {
      LatLon2Screen(Task[i].AATTargetLon, Task[i].AATTargetLat, 
                    Task[i].Target);
    }

    if(this_valid && !next_valid)
    {
      // finish
      LatLon2Screen(Task[i].SectorEndLon, Task[i].SectorEndLat, Task[i].End);
      LatLon2Screen(Task[i].SectorStartLon, Task[i].SectorStartLat, Task[i].Start);      
    }
    if(this_valid && next_valid)
    {
      LatLon2Screen(Task[i].SectorEndLon, Task[i].SectorEndLat, Task[i].End);
      LatLon2Screen(Task[i].SectorStartLon, Task[i].SectorStartLat, Task[i].Start);

      if((AATEnabled) && (Task[i].AATType == SECTOR))
      {
        LatLon2Screen(Task[i].AATStartLon, Task[i].AATStartLat, Task[i].AATStart);
        LatLon2Screen(Task[i].AATFinishLon, Task[i].AATFinishLat, Task[i].AATFinish);
      }
      if (AATEnabled && (((int)i==ActiveWayPoint) || 
			 (mode.Is(Mode::MODE_TARGET_PAN) && ((int)i==TargetPanIndex)))) {

	for (int j=0; j<MAXISOLINES; j++) {
	  if (TaskStats[i].IsoLine_valid[j]) {
	    LatLon2Screen(TaskStats[i].IsoLine_Longitude[j], 
			  TaskStats[i].IsoLine_Latitude[j], 
			  TaskStats[i].IsoLine_Screen[j]);
	  }
	}
      }
    }
  }

  UnlockTaskData();

}


// JMW to be used for target preview
void MapWindow::SetTargetPan(bool do_pan, int target_point, DWORD dlgSize /* = 0 */)
{
  static double old_latitude;
  static double old_longitude;

  if(dlgSize)
    targetPanSize = dlgSize;

  if (!mode.Is(Mode::MODE_TARGET_PAN) || (TargetPanIndex != target_point)) {
    targetMoved = false;
  }

  TargetPanIndex = target_point;

  if (do_pan && !mode.Is(Mode::MODE_TARGET_PAN)) {
    old_latitude = PanLatitude;
    old_longitude = PanLongitude;
    mode.Special(do_pan ? Mode::MODE_SPECIAL_TARGET_PAN : Mode::MODE_SPECIAL_PAN, true);
    zoom.SwitchMode();
  }
  if (do_pan) {
    LockTaskData();
    if (ValidTaskPoint(target_point)) {
      PanLongitude = WayPointList[Task[target_point].Index].Longitude;
      PanLatitude = WayPointList[Task[target_point].Index].Latitude;
      if (target_point==0) {
        TargetZoomDistance = max(2e3, (double)StartRadius*2);
      } else if (!ValidTaskPoint(target_point+1)) {
        TargetZoomDistance = max(2e3, (double)FinishRadius*2);
      } else if (AATEnabled) {
        if (Task[target_point].AATType == SECTOR) {
          TargetZoomDistance = max(2e3, Task[target_point].AATSectorRadius*2);
        } else {
          TargetZoomDistance = max(2e3, Task[target_point].AATCircleRadius*2);
        }
      } else {
        TargetZoomDistance = max(2e3, (double)SectorRadius*2);
      }
    }
    UnlockTaskData();
  }
  else if (mode.Is(Mode::MODE_TARGET_PAN)) {
    PanLongitude = old_longitude;
    PanLatitude = old_latitude;
    mode.Special(Mode::MODE_SPECIAL_TARGET_PAN, do_pan);
    zoom.SwitchMode();
    }
  mode.Special(Mode::MODE_SPECIAL_TARGET_PAN, do_pan);
  }



int MapWindow::iSnailNext=0;


void MapWindow::CalculateScreenPositionsGroundline(void) {
  if (FinalGlideTerrain) {
    LatLon2Screen(DerivedDrawInfo.GlideFootPrint,
		  Groundline, NUMTERRAINSWEEPS+1, 1);
  }
}

//
// Draw the opaque glide amoeba
//
void MapWindow::DrawTerrainAbove(HDC hDC, const RECT rc) {

  if (!DerivedDrawInfo.Flying &&
	!(SIMMODE && DerivedDrawInfo.AltitudeAGL>100) ) return;

  if (!DerivedDrawInfo.FreeFlying) return;

  COLORREF whitecolor = RGB(0xff,0xff,0xff);
  COLORREF graycolor = RGB(0xf0,0xf0,0xf0);
  COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);

  SetBkMode(hDCTemp, TRANSPARENT);

  SelectObject(hDCTemp, (HBITMAP)hDrawBitMapTmp);
  SetBkColor(hDCTemp, whitecolor);

  SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
  SetTextColor(hDCTemp, graycolor);
  SelectObject(hDCTemp, hAboveTerrainBrush); // hAirspaceBrushes[3] or 6
  Rectangle(hDCTemp,rc.left,rc.top,rc.right,rc.bottom);

  SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
  SelectObject(hDCTemp, GetStockObject(WHITE_BRUSH));
  Polygon(hDCTemp,Groundline,NUMTERRAINSWEEPS+1);

  // need to do this to prevent drawing of colored outline
  SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
#if (WINDOWSPC<1)
    TransparentImage(hDC,
                     rc.left, rc.top,
                     rc.right-rc.left,rc.bottom-rc.top,
                     hDCTemp,
                     rc.left, rc.top,
                     rc.right-rc.left,rc.bottom-rc.top,
                     whitecolor
                     );
    
#else
    TransparentBlt(hDC,
                   rc.left,rc.top,
                   rc.right-rc.left,rc.bottom-rc.top,
                   hDCTemp,
                   rc.left,rc.top,
                   rc.right-rc.left,rc.bottom-rc.top,
                   whitecolor
                   );
  #endif

  // restore original color
  SetTextColor(hDCTemp, origcolor);
  SetBkMode(hDCTemp,OPAQUE);

}


void MapWindow::DrawProjectedTrack(HDC hdc, const RECT rc, const POINT Orig) {
  if ((ActiveWayPoint<=0) || !ValidTaskPoint(ActiveWayPoint) || !AATEnabled) { 
    return;
  }
  if (DerivedDrawInfo.Circling) {
    // don't display in various modes
    return;
  }

  // TODO feature: maybe have this work even if no task?
  // TODO feature: draw this also when in target pan mode

  LockTaskData();  // protect from external task changes

  double startLat = DrawInfo.Latitude;
  double startLon = DrawInfo.Longitude;
  double previousLat;
  double previousLon;
  if (AATEnabled) {
    previousLat = Task[max(0,ActiveWayPoint-1)].AATTargetLat;
    previousLon = Task[max(0,ActiveWayPoint-1)].AATTargetLon; 
  } else {
    previousLat = WayPointList[Task[max(0,ActiveWayPoint-1)].Index].Latitude;
    previousLon = WayPointList[Task[max(0,ActiveWayPoint-1)].Index].Longitude; 
  }
  UnlockTaskData();

  double distance_from_previous, bearing;
  DistanceBearing(previousLat, previousLon,
		  startLat, startLon,
		  &distance_from_previous,
		  &bearing);

  if (distance_from_previous < 100.0) {
    bearing = DrawInfo.TrackBearing;
    // too short to have valid data
  }
  POINT pt[2] = {{0,-75},{0,-400}};
  if (mode.Is(Mode::MODE_TARGET_PAN)) {
    double screen_range = GetApproxScreenRange();
    double flow = 0.4;
    double fhigh = 1.5;
    screen_range = max(screen_range, DerivedDrawInfo.WaypointDistance);

    double p1Lat;
    double p1Lon;
    double p2Lat;
    double p2Lon;
    FindLatitudeLongitude(startLat, startLon, 
			  bearing, flow*screen_range,
			  &p1Lat, &p1Lon);
    FindLatitudeLongitude(startLat, startLon, 
			  bearing, fhigh*screen_range,
			  &p2Lat, &p2Lon);
    LatLon2Screen(p1Lon, p1Lat, pt[0]);
    LatLon2Screen(p2Lon, p2Lat, pt[1]);
  } else if (fabs(bearing-DerivedDrawInfo.WaypointBearing)<10) {
    // too small an error to bother
    return;
  } else {
    pt[1].y = (long)(-max(MapRect.right-MapRect.left,
			  MapRect.bottom-MapRect.top)*1.2);
    PolygonRotateShift(pt, 2, Orig.x, Orig.y, 
		       bearing-DisplayAngle);
  }
  DrawDashLine(hdc, 2, pt[0], pt[1], RGB(0,0,0), rc);
}


bool MapWindow::TargetMoved(double &longitude, double &latitude) {
  bool retval = false;
  LockTaskData();
  if (targetMoved) {
    longitude = targetMovedLon;
    latitude = targetMovedLat;
    targetMoved = false;
    retval = true;
  }
  UnlockTaskData();
  return retval;
}





void MapWindow::DrawTeammate(HDC hdc, RECT rc)
{
  POINT point;

  if (TeammateCodeValid)
    {
      if(PointVisible(TeammateLongitude, TeammateLatitude) )
	{
	  LatLon2Screen(TeammateLongitude, TeammateLatitude, point);

	  SelectObject(hDCTemp,hBmpTeammatePosition);
	  DrawBitmapX(hdc,
		      point.x-NIBLSCALE(10), 
		      point.y-NIBLSCALE(10),
		      20,20,
		      hDCTemp,0,0,SRCPAINT);
	
	  DrawBitmapX(hdc,
		      point.x-NIBLSCALE(10), 
		      point.y-NIBLSCALE(10),
		      20,20,
		      hDCTemp,20,0,SRCAND);
	}
    }
}



void MapWindow::DrawThermalBand(HDC hDC, const RECT rc)
{
  POINT GliderBand[5] = { {0,0},{23,0},{22,0},{24,0},{0,0} };
  
  if ((DerivedDrawInfo.TaskAltitudeDifference>50)
      &&(mode.Is(Mode::MODE_FINAL_GLIDE))) {
    return;
  }

  // JMW TODO accuracy: gather proper statistics
  // note these should/may also be relative to ground
  int i;
  double mth = DerivedDrawInfo.MaxThermalHeight;
  double maxh, minh;
  double h;
  double Wt[NUMTHERMALBUCKETS];
  double ht[NUMTHERMALBUCKETS];
  double Wmax=0.0;
  int TBSCALEY = ( (rc.bottom - rc.top )/2)-NIBLSCALE(30);
#define TBSCALEX 20
  
  // calculate height above safety altitude
  double hoffset = DerivedDrawInfo.TerrainBase;
  h = DerivedDrawInfo.NavAltitude-hoffset;

  bool draw_start_height = ((ActiveWayPoint==0) && (ValidTaskPoint(0)) 
			    && (StartMaxHeight!=0)
			    && (DerivedDrawInfo.TerrainValid));
  double hstart=0;
  if (draw_start_height) {
    if (StartHeightRef == 0) {
      hstart = (StartMaxHeight/1000)+DerivedDrawInfo.TerrainAlt; //@ 100315
    } else {
      hstart = StartMaxHeight/1000; // 100315
    }
    hstart -= hoffset;
  }

  short lkvariooffset;
  lkvariooffset=rc.left + LKVarioBar?(LKVarioSize+1):0; 

  // calculate top/bottom height
  maxh = max(h, mth);
  minh = min(h, 0.0);

  if (draw_start_height) {
    maxh = max(maxh, hstart);
    minh = min(minh, hstart);
  }
  
  // no thermalling has been done above safety altitude
  if (mth<=1) {
    return;
  }
  if (maxh-minh<=0) {
    return;
  }

  // normalised heights
  double hglider = (h-minh)/(maxh-minh);
  hstart = (hstart-minh)/(maxh-minh);

  // calculate averages
  int numtherm = 0;

  double mc = MACCREADY;
  Wmax = max(0.5,mc);

  for (i=0; i<NUMTHERMALBUCKETS; i++) {
    double wthis = 0;
    // height of this thermal point [0,mth]
    double hi = i*mth/NUMTHERMALBUCKETS;
    double hp = ((hi-minh)/(maxh-minh));

    if (DerivedDrawInfo.ThermalProfileN[i]>5) {
      // now requires 10 items in bucket before displaying,
      // to eliminate kinks
      wthis = DerivedDrawInfo.ThermalProfileW[i]
                 /DerivedDrawInfo.ThermalProfileN[i];
    }
    if (wthis>0.0) {
      ht[numtherm]= hp;
      Wt[numtherm]= wthis;
      Wmax = max(Wmax,wthis/1.5);
      numtherm++;
    }
  }

  if ((!draw_start_height) && (numtherm<=1)) {
    return; // don't display if insufficient statistics
    // but do draw if start height needs to be drawn
  }
  
  // drawing info
  HPEN hpOld;
  

  // position of thermal band
  if (numtherm>1) {
    hpOld = (HPEN)SelectObject(hDC, hpThermalBand);
    HBRUSH hbOld = (HBRUSH)SelectObject(hDC, hbThermalBand);
 

    POINT ThermalProfile[NUMTHERMALBUCKETS+2];
    for (i=0; i<numtherm; i++) {    
      ThermalProfile[1+i].x = 
	(iround((Wt[i]/Wmax)*IBLSCALE(TBSCALEX)))+lkvariooffset; //@ 091118
      
      ThermalProfile[1+i].y = 
	NIBLSCALE(4)+iround(TBSCALEY*(1.0-ht[i]))+rc.top;
    }
    ThermalProfile[0].x = lkvariooffset;
    ThermalProfile[0].y = ThermalProfile[1].y;
    ThermalProfile[numtherm+1].x = lkvariooffset; //@ 091118
    ThermalProfile[numtherm+1].y = ThermalProfile[numtherm].y;

    Polygon(hDC,ThermalProfile,numtherm+2);
    SelectObject(hDC, hbOld);
  }
    
  // position of thermal band

  GliderBand[0].x += lkvariooffset; // 091123 added
  GliderBand[0].y = NIBLSCALE(4)+iround(TBSCALEY*(1.0-hglider))+rc.top;
  GliderBand[1].y = GliderBand[0].y;
  GliderBand[1].x = max(iround((mc/Wmax)*IBLSCALE(TBSCALEX)),NIBLSCALE(4)) +lkvariooffset; //@ 091118 rc.left

  GliderBand[2].x = GliderBand[1].x-NIBLSCALE(4);
  GliderBand[2].y = GliderBand[0].y-NIBLSCALE(4);
  GliderBand[3].x = GliderBand[1].x;
  GliderBand[3].y = GliderBand[1].y;
  GliderBand[4].x = GliderBand[1].x-NIBLSCALE(4);
  GliderBand[4].y = GliderBand[0].y+NIBLSCALE(4);

  hpOld = (HPEN)SelectObject(hDC, hpThermalBandGlider);
  
  Polyline(hDC,GliderBand, 2);
  Polyline(hDC,GliderBand+2, 3); // arrow head

  if (draw_start_height) {
    SelectObject(hDC, hpFinalGlideBelow);
    GliderBand[0].y = NIBLSCALE(4)+iround(TBSCALEY*(1.0-hstart))+rc.top;
    GliderBand[1].y = GliderBand[0].y;
    Polyline(hDC, GliderBand, 2);
  }

  SelectObject(hDC, hpOld);
  
}

// revised for variometer gauge
void MapWindow::DrawFinalGlide(HDC hDC, const RECT rc)
{

  SIZE           TextSize;

  if ((GlideBarMode == (GlideBarMode_t)gbDisabled)) {
	GlideBarOffset=0;
	return;
  }

  POINT GlideBar[6] = { {0,0},{9,-9},{18,0},{18,0},{9,0},{0,0} };
  POINT GlideBar0[6] = { {0,0},{9,-9},{18,0},{18,0},{9,0},{0,0} };
  
  HPEN hpOld;
  HBRUSH hbOld;
  
  TCHAR Value[10];
  
  int Offset;
  int Offset0;
  int i;
  int lkVarioOffset=0, minBar, maxBar;

  if (LKVarioBar)
	lkVarioOffset=LKVarioSize+NIBLSCALE(2); //@ 091114

  // 091114
  switch(ScreenSize) {
	case (ScreenSize_t)ss480x234:
	case (ScreenSize_t)ss480x272:
	case (ScreenSize_t)ss720x408:
		minBar=-40;
		maxBar=40;
		break;
	case (ScreenSize_t)ss800x480:
	case (ScreenSize_t)ss400x240:
		minBar=-45;
		maxBar=45;
	default:
		minBar=-50; // was 60
		maxBar=50;
		break;
  }
  
  LockTaskData();  // protect from external task changes
  #ifdef HAVEEXCEPTIONS
  __try{
  #endif

  #if 101004
  int barindex;
  barindex=GetOvertargetIndex();
  if (barindex>=0) {
  #else
  if (ValidTaskPoint(ActiveWayPoint)){
  #endif

	const int y0 = ( (rc.bottom - rc.top )/2)+rc.top;

	#if 110609
	if ( ValidTaskPoint(1) && OvertargetMode == OVT_TASK && GlideBarMode == (GlideBarMode_t)gbFinish ) {
		Offset = ((int)DerivedDrawInfo.TaskAltitudeDifference)/8; 
		Offset0 = ((int)DerivedDrawInfo.TaskAltitudeDifference0)/8; 
	} else {
		Offset=(int)WayPointCalc[barindex].AltArriv[AltArrivMode];
		Offset0=Offset;
	}
	#else
	// This is wrong, we are painting values relative to MC and ignoring safetyMC
	if (OvertargetMode != OVT_TASK) { //@ 101004
		Offset=(int)WayPointCalc[barindex].AltArriv[AltArrivMode];
		Offset0=Offset;
	} else {
		// 60 units is size, div by 8 means 60*8 = 480 meters.
		if ( (GlideBarMode == (GlideBarMode_t)gbFinish)) {
			Offset = ((int)DerivedDrawInfo.TaskAltitudeDifference)/8; 
			Offset0 = ((int)DerivedDrawInfo.TaskAltitudeDifference0)/8; 
		} else {
			Offset = ((int)DerivedDrawInfo.NextAltitudeDifference)/8; 
			Offset0 = ((int)DerivedDrawInfo.NextAltitudeDifference0)/8; 
		}
	}
	#endif

	// TODO feature: should be an angle if in final glide mode

	if(Offset > maxBar) Offset = maxBar;
	if(Offset < minBar) Offset = minBar;
	Offset = IBLSCALE(Offset);
	if(Offset<0) {
		GlideBar[1].y = NIBLSCALE(9);
	}
      
	if(Offset0 > maxBar) Offset0 = maxBar;
	if(Offset0 < minBar) Offset0 = minBar;
	Offset0 = IBLSCALE(Offset0);
	if(Offset0<0) {
		GlideBar0[1].y = NIBLSCALE(9);
	}
 
	for(i=0;i<6;i++) {
		GlideBar[i].y += y0;
		// if vario activated
		GlideBar[i].x = IBLSCALE(GlideBar[i].x)+rc.left+lkVarioOffset; //@ 091114
	}
	GlideBar[0].y -= Offset;
	GlideBar[1].y -= Offset;
	GlideBar[2].y -= Offset;

	for(i=0;i<6;i++) {
		GlideBar0[i].y += y0;
		GlideBar0[i].x = IBLSCALE(GlideBar0[i].x)+rc.left+lkVarioOffset; //@ 091114
	}
	GlideBar0[0].y -= Offset0;
	GlideBar0[1].y -= Offset0;
	GlideBar0[2].y -= Offset0;

	if ((Offset<0)&&(Offset0<0)) {
		// both below
		if (Offset0!= Offset) {
			int dy = (GlideBar0[0].y-GlideBar[0].y) +(GlideBar0[0].y-GlideBar0[3].y); dy = max(NIBLSCALE(3), dy);
			GlideBar[3].y = GlideBar0[0].y-dy;
			GlideBar[4].y = GlideBar0[1].y-dy;
			GlideBar[5].y = GlideBar0[2].y-dy;
          
			GlideBar0[0].y = GlideBar[3].y;
			GlideBar0[1].y = GlideBar[4].y;
			GlideBar0[2].y = GlideBar[5].y;
		} else {
			Offset0 = 0;
		}

	} else if ((Offset>0)&&(Offset0>0)) {
		// both above
		GlideBar0[3].y = GlideBar[0].y;
		GlideBar0[4].y = GlideBar[1].y;
		GlideBar0[5].y = GlideBar[2].y;

		if (abs(Offset0-Offset)<NIBLSCALE(4)) {
			Offset= Offset0;
		} 
	}

	// draw actual glide bar

	if (Offset<=0) {
		if (LandableReachable && (OvertargetMode==OVT_TASK)) { //@ 101004
			hpOld = (HPEN)SelectObject(hDC, hpFinalGlideBelowLandable);
			hbOld = (HBRUSH)SelectObject(hDC, hbFinalGlideBelowLandable);
		} else {
			hpOld = (HPEN)SelectObject(hDC, hpFinalGlideBelow);
			hbOld = (HBRUSH)SelectObject(hDC, hbFinalGlideBelow);
		}
	} else {
		hpOld = (HPEN)SelectObject(hDC, hpFinalGlideAbove);
		hbOld = (HBRUSH)SelectObject(hDC, hbFinalGlideAbove);
	}
	Polygon(hDC,GlideBar,6);

	// draw glide bar at mc 0 and X  only for OVT_TASK 101004
	// we dont have mc0 calc ready for other overtargets, not granted at least
	if (OvertargetMode == OVT_TASK) {
		if (Offset0<=0) {
			if (LandableReachable) {
				SelectObject(hDC, hpFinalGlideBelowLandable);
				SelectObject(hDC, GetStockObject(HOLLOW_BRUSH));
			} else {
				SelectObject(hDC, hpFinalGlideBelow);
				SelectObject(hDC, GetStockObject(HOLLOW_BRUSH));
			}
		} else {
			SelectObject(hDC, hpFinalGlideAbove);
			SelectObject(hDC, GetStockObject(HOLLOW_BRUSH));
		}
		if (Offset!=Offset0) {
			Polygon(hDC,GlideBar0,6);
		}

		// JMW draw x on final glide bar if unreachable at current Mc
		// hpAircraftBorder

		if ( (GlideBarMode == (GlideBarMode_t)gbFinish) ) {
			if ((DerivedDrawInfo.TaskTimeToGo>0.9*ERROR_TIME) || 
			((MACCREADY<0.01) && (DerivedDrawInfo.TaskAltitudeDifference<0))) {
				SelectObject(hDC, hpAircraftBorder);
				POINT Cross[4] = { {-5, -5}, { 5,  5}, {-5,  5}, { 5, -5} };
				for (i=0; i<4; i++) {
					Cross[i].x = IBLSCALE(Cross[i].x+9)+lkVarioOffset; //@ 091114
					Cross[i].y = IBLSCALE(Cross[i].y+9)+y0;
				}
				Polygon(hDC,Cross,2);
				Polygon(hDC,&Cross[2],2);
			}
		} else {
			if ((MACCREADY<0.01) && (DerivedDrawInfo.NextAltitudeDifference<0)) {
				SelectObject(hDC, hpAircraftBorder);
				POINT Cross[4] = { {-5, -5}, { 5,  5}, {-5,  5}, { 5, -5} };
				for (i=0; i<4; i++) {
					Cross[i].x = IBLSCALE(Cross[i].x+9)+lkVarioOffset;
					Cross[i].y = IBLSCALE(Cross[i].y+9)+y0;
				}
				Polygon(hDC,Cross,2);
				Polygon(hDC,&Cross[2],2);
			}
		}
	}


	// draw boxed value in the center
		if (OvertargetMode == OVT_TASK ) { //@ 101004
			// A task is made of at least 2 tps, otherwise its a goto
			if (( (GlideBarMode == (GlideBarMode_t)gbFinish) && ValidTaskPoint(1)) ) {
				if ( (ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeDifference) <ALTDIFFLIMIT) //@ 091114
					_stprintf(Value,TEXT(" --- "));
				else
					_stprintf(Value,TEXT("%1.0f "), ALTITUDEMODIFY*DerivedDrawInfo.TaskAltitudeDifference);
			} else {
				if ( (ALTITUDEMODIFY*WayPointCalc[barindex].AltArriv[AltArrivMode]) < ALTDIFFLIMIT)
					_stprintf(Value,TEXT(" --- "));
				else
					_stprintf(Value,TEXT("%1.0f "), ALTITUDEMODIFY*WayPointCalc[barindex].AltArriv[AltArrivMode]);
				/*
				 * Well this was the reason why the glidebar value was out of sync with overlays
				if ( (ALTITUDEMODIFY*DerivedDrawInfo.NextAltitudeDifference) < ALTDIFFLIMIT) //@ 091114
					_stprintf(Value,TEXT(" --- "));
				else
					_stprintf(Value,TEXT("%1.0f "), ALTITUDEMODIFY*DerivedDrawInfo.NextAltitudeDifference);
				*/
			}
		} else {
			if ( (ALTITUDEMODIFY*WayPointCalc[barindex].AltArriv[AltArrivMode]) < ALTDIFFLIMIT)
				_stprintf(Value,TEXT(" --- "));
			else
				_stprintf(Value,TEXT("%1.0f "), ALTITUDEMODIFY*WayPointCalc[barindex].AltArriv[AltArrivMode]);
		}

		if (Offset>=0) {
			Offset = GlideBar[2].y+Offset+NIBLSCALE(5);
		} else {
			if (Offset0>0) {
				Offset = GlideBar0[1].y-NIBLSCALE(15);
			} else {
				Offset = GlideBar[2].y+Offset-NIBLSCALE(15);
			}
		}
		// VENTA10
		GetTextExtentPoint(hDC, Value, _tcslen(Value), &TextSize); 
		GlideBarOffset=max(NIBLSCALE(11),(int)TextSize.cx) - NIBLSCALE(2);
 
		TextInBoxMode_t TextInBoxMode = {1|8};
		// boxed numbers are a bit too much on the left, so increase the offset
		TextInBox(hDC, Value, lkVarioOffset+NIBLSCALE(1), (int)Offset, 0, TextInBoxMode); //@ 091114

	SelectObject(hDC, hbOld);
	SelectObject(hDC, hpOld);
    } else GlideBarOffset=0; 	// 091125 BUGFIX glidebaroffset is zero when no task point
#ifdef HAVEEXCEPTIONS
  }__finally
#endif
     {
       UnlockTaskData();
     }
  
}


void MapWindow::DrawCompass(HDC hDC, const RECT rc)
{
  POINT Start;
  HPEN hpOld;
  HBRUSH hbOld; 

    static double lastDisplayAngle = 9999.9;
    static int lastRcRight = 0, lastRcTop = 0;
    static POINT Arrow[5] = { {0,-11}, {-5,9}, {0,3}, {5,9}, {0,-11}};


    if (lastDisplayAngle != DisplayAngle || lastRcRight != rc.right || lastRcTop != rc.top){

      Arrow[0].x  = 0;
      Arrow[0].y  = -11;
      Arrow[1].x  = -5;
      Arrow[1].y  = 9;
      Arrow[2].x  = 0;
      Arrow[2].y  = 3;
      Arrow[3].x  = 5;
      Arrow[3].y  = 9;
      Arrow[4].x  = 0;
      Arrow[4].y  = -11;

	// no more clock, no need to have different compass position
	Start.y = rc.top + NIBLSCALE(11); 
	Start.x = rc.right - NIBLSCALE(11);

      // North arrow
      PolygonRotateShift(Arrow, 5, Start.x, Start.y, 
                         -DisplayAngle);

      lastDisplayAngle = DisplayAngle;
      lastRcRight = rc.right;
      lastRcTop = rc.top;
    }

    hpOld = (HPEN)SelectObject(hDC, hpCompassBorder);
    hbOld = (HBRUSH)SelectObject(hDC, hbCompass);
    Polygon(hDC,Arrow,5);

    SelectObject(hDC, hpCompass);
    Polygon(hDC,Arrow,5);

    SelectObject(hDC, hbOld);
    SelectObject(hDC, hpOld);

}



void MapWindow::DrawMapScale(HDC hDC, const RECT rc /* the Map Rect*/, 
                             const bool ScaleChangeFeedback)
{
  static short terrainwarning=0;

    TCHAR Scale[80];
    TCHAR Scale2[80];
    TCHAR TEMP[20];
    POINT Start, End;
    COLORREF origcolor = SetTextColor(hDC, OverColorRef);

    HPEN hpOld;
    TextInBoxMode_t TextDisplayMode; 
    hpOld = (HPEN)SelectObject(hDC, hpMapScale2);


    // TODO use Appearance font Hight to calculate correct offset

    Start.x = rc.right-NIBLSCALE(6); End.x = rc.right-NIBLSCALE(6);

    Start.y = rc.bottom-BottomSize-NIBLSCALE(4); // 100922
    End.y = Start.y - NIBLSCALE(42);
    DrawSolidLine(hDC,Start,End, rc);

    Start.x = rc.right-NIBLSCALE(11); End.x = rc.right-NIBLSCALE(6);
    End.y = Start.y;
    DrawSolidLine(hDC,Start,End, rc);

     Start.y = Start.y - NIBLSCALE(42); End.y = Start.y;
    DrawSolidLine(hDC,Start,End, rc);

    SelectObject(hDC, hpOld);

    _tcscpy(Scale2,TEXT(""));

    // warn about missing terrain
    if (!CALCULATED_INFO.TerrainValid) {
	if (terrainwarning < 120) {
		// LKTOKEN _@M1335_ " TERRAIN?"
		_tcscat(Scale2, gettext(TEXT("_@M1335_")));
		terrainwarning++;
	} else  {
		// LKTOKEN _@M1336_ " T?"
		_tcscat(Scale2, gettext(TEXT("_@M1336_")));
		terrainwarning=120;
	}
    } else terrainwarning=0;

    if (ActiveMap) {
      _tcscat(Scale2, gettext(TEXT("_@M1661_"))); // ACT
    }
    if (UseTotalEnergy) {
      _tcscat(Scale2, TEXT("[TE]")); // Total Energy indicator
    }
    if (zoom.AutoZoom()) {
		// LKTOKEN _@M1337_ " AZM"
      _tcscat(Scale2, gettext(TEXT("_@M1337_")));
    }
    if (mode.AnyPan()) {
		// LKTOKEN _@M1338_ " PAN"
      _tcscat(Scale2, gettext(TEXT("_@M1338_")));
    }

    if (DrawBottom) {
	switch(BottomMode) {
		case BM_TRM:
				// LKTOKEN _@M1340_ " TRM0"
      			_tcscat(Scale2, gettext(TEXT("_@M1340_")));
			break;
		case BM_CRU:
				// LKTOKEN _@M1341_ " NAV1"
      			_tcscat(Scale2, gettext(TEXT("_@M1341_")));
			break;
		case BM_HGH:
				// LKTOKEN _@M1342_ " ALT2"
      			_tcscat(Scale2, gettext(TEXT("_@M1342_")));
			break;
		case BM_AUX:
				// LKTOKEN _@M1343_ " STA3"
      			_tcscat(Scale2, gettext(TEXT("_@M1343_")));
			break;
		case BM_TSK:
				// LKTOKEN _@M1344_ " TSK4"
      			_tcscat(Scale2, gettext(TEXT("_@M1344_")));
			break;
		case BM_ALT:
				// LKTOKEN _@M1345_ " ATN5"
      			_tcscat(Scale2, gettext(TEXT("_@M1345_")));
			break;
		case BM_SYS:
				// LKTOKEN _@M1346_ " SYS6"
      			_tcscat(Scale2, gettext(TEXT("_@M1346_")));
			break;
		case BM_CUS2:
				// LKTOKEN _@M1347_ " CRU7"
      			_tcscat(Scale2, gettext(TEXT("_@M1347_")));
			break;
		case BM_CUS3:
				// LKTOKEN _@M1348_ " FIN8"
      			_tcscat(Scale2, gettext(TEXT("_@M1348_")));
			break;
		case BM_CUS:
				// LKTOKEN _@M1349_ " AUX9"
      			_tcscat(Scale2, gettext(TEXT("_@M1349_")));
			break;
		default:
			break;
	}
    }

    if (ReplayLogger::IsEnabled()) {
		// LKTOKEN _@M1350_ " REPLAY"
      _tcscat(Scale2, gettext(TEXT("_@M1350_")));
    }
    if (BallastTimerActive) {
		// LKTOKEN _@M1351_ " BALLAST"
      _stprintf(TEMP,TEXT("%s %3.0fL"), gettext(TEXT("_@M1351_")), WEIGHTS[2]*BALLAST);
      _tcscat(Scale2, TEMP);
    }

    _tcscpy(Scale,TEXT(""));
    double mapScale=zoom.Scale()*1.4; // FIX 091117
    if (ISPARAGLIDER) {
	if ((mapScale) <1.0) {
		_stprintf(Scale,TEXT("%1.2f"),mapScale);
	}
	else if((mapScale*3) <3) {
		_stprintf(Scale,TEXT("%1.1f"),mapScale);
	}
	else {
		_stprintf(Scale,TEXT("%1.0f"),mapScale);
	}
    } else {
	if (mapScale <0.1)
	{
		_stprintf(Scale,TEXT("%1.2f"),mapScale);
	}
	else if(mapScale <3)
	{
		_stprintf(Scale,TEXT("%1.1f"),mapScale);
	}
	else
	{
		_stprintf(Scale,TEXT("%1.0f"),mapScale);
	}
    }
   _tcscat(Scale, Units::GetDistanceName()); 


    SIZE tsize;

    TextDisplayMode.AsInt = 0;
    TextDisplayMode.AsFlag.Color = TEXTWHITE;
    TextDisplayMode.AsFlag.WhiteBold = 1;
    TextDisplayMode.AsFlag.NoSetFont = 0;
    TextDisplayMode.AsFlag.AlligneRight = 0;
    TextDisplayMode.AsFlag.AlligneCenter = 0;

    GetTextExtentPoint(hDC, Scale, _tcslen(Scale), &tsize);
    LKWriteText(hDC, Scale, rc.right-NIBLSCALE(11)-tsize.cx, End.y+NIBLSCALE(3), 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true); 

    GetTextExtentPoint(hDC, Scale2, _tcslen(Scale2), &tsize);
    COLORREF mapscalecolor=OverColorRef;
    if (!CALCULATED_INFO.TerrainValid) 
	if (terrainwarning>0 && terrainwarning<120) mapscalecolor=RGB_RED;
		
    LKWriteText(hDC, Scale2, rc.right-NIBLSCALE(11)-tsize.cx, End.y+NIBLSCALE(3)+tsize.cy, 
	0, WTMODE_OUTLINED, WTALIGN_LEFT, mapscalecolor, true); 


    // restore original color
    SetTextColor(hDC, origcolor);

    SelectObject(hDC, hpOld);


}

// Glide Amoeba area
// NOTE: Shaded line is displayed only when in flight, otherwise only a dashed line is used
void MapWindow::DrawGlideThroughTerrain(HDC hDC, const RECT rc) {
  HPEN hpOld;

  //double h,dh;
  TCHAR hbuf[10];
  static bool doinit=true;
  static TextInBoxMode_t tmode;
  bool wrotevalue=false;

  if (doinit) {
	tmode.AsInt=0;
	tmode.AsFlag.Border=1;
	doinit=false;
  }

  hpOld = (HPEN)SelectObject(hDC, hpTerrainLineBg); 

  // draw a dashed perimetral line first
  _Polyline(hDC,Groundline,NUMTERRAINSWEEPS+1, rc);

  // draw shade if selected and during a flight
  if ((FinalGlideTerrain==1) || ((!EnableTerrain || !DerivedDrawInfo.Flying) && (FinalGlideTerrain==2))) { 
	SelectObject(hDC,hpTerrainLine);
	_Polyline(hDC,Groundline,NUMTERRAINSWEEPS+1, rc);
  }

  // draw red cross obstacles only if destination looks reachable!
  // only if using OVT_TASK of course!

  if ( (OvertargetMode==OVT_TASK) && DerivedDrawInfo.Flying && ValidTaskPoint(ActiveWayPoint))
  if (WayPointCalc[TASKINDEX].AltArriv[AltArrivMode] >0) { 

	POINT sc;
	// If calculations detected an obstacle...
	if ((DerivedDrawInfo.TerrainWarningLatitude != 0.0) &&(DerivedDrawInfo.TerrainWarningLongitude != 0.0)) {

		// only if valid position, and visible
		if (DerivedDrawInfo.FarObstacle_Lon >0) 
		if (PointVisible(DerivedDrawInfo.FarObstacle_Lon, DerivedDrawInfo.FarObstacle_Lat)) {
			LatLon2Screen(DerivedDrawInfo.FarObstacle_Lon, DerivedDrawInfo.FarObstacle_Lat, sc);
			DrawBitmapIn(hDC, sc, hTerrainWarning);

			if (DerivedDrawInfo.FarObstacle_AltArriv <=-50 ||  DerivedDrawInfo.FarObstacle_Dist<5000 ) {
				_stprintf(hbuf,_T(" %.0f"),ALTITUDEMODIFY*DerivedDrawInfo.FarObstacle_AltArriv);
				TextInBox(hDC,hbuf,sc.x+NIBLSCALE(15), sc.y, 0, tmode,false); 
				wrotevalue=true;
			}
		} // visible far obstacle

		if (PointVisible(DerivedDrawInfo.TerrainWarningLongitude, DerivedDrawInfo.TerrainWarningLatitude)) {
			LatLon2Screen(DerivedDrawInfo.TerrainWarningLongitude, DerivedDrawInfo.TerrainWarningLatitude, sc);
			DrawBitmapIn(hDC, sc, hTerrainWarning);
#if 0
			// 091203 add obstacle altitude on moving map
			h =  max(0,RasterTerrain::GetTerrainHeight(DerivedDrawInfo.TerrainWarningLatitude, 
				DerivedDrawInfo.TerrainWarningLongitude)); 
			if (h==TERRAIN_INVALID) h=0; //@ 101027 FIX but unused
			dh = CALCULATED_INFO.NavAltitude - h - SAFETYALTITUDETERRAIN;
			_stprintf(hbuf,_T(" %.0f"),ALTITUDEMODIFY*dh);
			TextInBox(hDC,hbuf,sc.x+NIBLSCALE(10), sc.y, 0, tmode,false); 
#else
			// if far obstacle was painted with value...
			if (wrotevalue) {
				// if it is not too near the nearest..
				if ( (fabs(DerivedDrawInfo.FarObstacle_Lon - DerivedDrawInfo.TerrainWarningLongitude) >0.02) &&
					(fabs(DerivedDrawInfo.FarObstacle_Lat - DerivedDrawInfo.TerrainWarningLatitude) >0.02)) {
					// and it the arrival altitude is actually negative (rounding terrain errors?)
					if ( DerivedDrawInfo.ObstacleAltArriv <=-50)
					// and there is a significant difference in the numbers, then paint value also for nearest
					if (  fabs(DerivedDrawInfo.ObstacleAltArriv - DerivedDrawInfo.FarObstacle_AltArriv) >100 ) {
						_stprintf(hbuf,_T(" %.0f"),ALTITUDEMODIFY*DerivedDrawInfo.ObstacleAltArriv);
						TextInBox(hDC,hbuf,sc.x+NIBLSCALE(15), sc.y, 0, tmode,false); 
					}
				}
			} else {
				// else paint value only if meaningful or very close to us
				// -1 to 10m become -1 for rounding errors
				if ( (DerivedDrawInfo.ObstacleAltArriv >-1) && (DerivedDrawInfo.ObstacleAltArriv <10))
					DerivedDrawInfo.ObstacleAltArriv=-1;
				if (DerivedDrawInfo.ObstacleAltArriv <=-50 ||  
				 ((DerivedDrawInfo.ObstacleAltArriv<0) && (DerivedDrawInfo.ObstacleDistance<5000)) ) {

					_stprintf(hbuf,_T(" %.0f"),ALTITUDEMODIFY*DerivedDrawInfo.ObstacleAltArriv);
					TextInBox(hDC,hbuf,sc.x+NIBLSCALE(15), sc.y, 0, tmode,false); 
				}
			}
#endif
		} // visible nearest obstacle
	} // obstacles detected
  } // within glide range

  SelectObject(hDC, hpOld);
}


void MapWindow::DrawBestCruiseTrack(HDC hdc, const POINT Orig)
{
  HPEN hpOld;
  HBRUSH hbOld;

  if (ActiveWayPoint<0) {
    return; // nothing to draw..
  }
  if (!ValidTaskPoint(ActiveWayPoint)) {
    return;
  }

  if (DerivedDrawInfo.WaypointDistance < 0.010)
    return;

  // dont draw bestcruise indicator if not needed
  if (fabs(DerivedDrawInfo.BestCruiseTrack-DerivedDrawInfo.WaypointBearing)<2) { // 091202 10 to 2
	return;
  } 


  hpOld = (HPEN)SelectObject(hdc, hpBestCruiseTrack);
  hbOld = (HBRUSH)SelectObject(hdc, hbBestCruiseTrack);

  if (Appearance.BestCruiseTrack == ctBestCruiseTrackDefault){

    int dy = (long)(70); 
    POINT Arrow[7] = { {-1,-40}, {1,-40}, {1,0}, {6,8}, {-6,8}, {-1,0}, {-1,-40}};

    Arrow[2].y -= dy;
    Arrow[3].y -= dy;
    Arrow[4].y -= dy;
    Arrow[5].y -= dy;

    PolygonRotateShift(Arrow, 7, Orig.x, Orig.y, 
                       DerivedDrawInfo.BestCruiseTrack-DisplayAngle);

    Polygon(hdc,Arrow,7);

  } else
  if (Appearance.BestCruiseTrack == ctBestCruiseTrackAltA){

    POINT Arrow[] = { {-1,-40}, {-1,-62}, {-6,-62}, {0,-70}, {6,-62}, {1,-62}, {1,-40}, {-1,-40}};

    PolygonRotateShift(Arrow, sizeof(Arrow)/sizeof(Arrow[0]),
                       Orig.x, Orig.y, 
                       DerivedDrawInfo.BestCruiseTrack-DisplayAngle);
    Polygon(hdc, Arrow, (sizeof(Arrow)/sizeof(Arrow[0])));
  }

  SelectObject(hdc, hpOld);
  SelectObject(hdc, hbOld);
}

