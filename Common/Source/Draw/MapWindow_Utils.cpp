/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"
#include "NavFunctions.h"




bool MapWindow::WaypointInTask(int ind) {
  if (!WayPointList) return false;
  return WayPointList[ind].InTask;
}



double MapWindow::GetApproxScreenRange() {
  return (zoom.Scale() * max(DrawRect.right-DrawRect.left,
                         DrawRect.bottom-DrawRect.top))
    *1000.0/GetMapResolutionFactor();
}


// Used only by Thread_Calculation main loop
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
  if(( P.x >= DrawRect.left ) 
     &&
     ( P.x <= DrawRect.right ) 
     &&
     ( P.y >= DrawRect.top  ) 
     &&
     ( P.y <= DrawRect.bottom  ) 
     )
    return TRUE;
  else
    return FALSE;
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



// Used by dlgTarget only
// A note about this function. We are changing map drawing parameters here, but we paint from the winmain thread.
// We are NOT using this from drawthread! the dialog target pan is sort of an hack, and not a good example.
// Do not consider emulating the target dialog, because it should be moved to draw thread somehow.
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
          const double start = Task[target_point].AATStartRadial;
          const double finish = Task[target_point].AATFinishRadial;
          const double xs = fastsine(start);
          const double ys = fastcosine(start);
          const double xf = fastsine(finish);
          const double yf = fastcosine(finish);
          
          // calculate rectangle area taken by the sector
          const double top    = AngleInRange(start, finish, 0,   true) ?  1 : max(max(ys, yf), 0.0);
          const double right  = AngleInRange(start, finish, 90,  true) ?  1 : max(max(xs, xf), 0.0);
          const double bottom = AngleInRange(start, finish, 180, true) ? -1 : min(min(ys, yf), 0.0);
          const double left   = AngleInRange(start, finish, 270, true) ? -1 : min(min(xs, xf), 0.0);
          
          // get area center
          const double radius = Task[target_point].AATSectorRadius;
          const double x = (left + right) / 2;
          const double y = (top + bottom) / 2;
          double bearing, range;
          xXY_Brg_Rng(0, 0, x, y, &bearing, &range);
          
          // find area center geographic data
          FindLatitudeLongitude(WayPointList[Task[target_point].Index].Latitude,
                                WayPointList[Task[target_point].Index].Longitude,
                                bearing, range * radius, &PanLatitude, &PanLongitude);
          TargetZoomDistance = max(2e3, max(right - left, top - bottom) * radius);
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


//
// A simple function telling us if we are currently USING any gauge
// We need to know it for a coherent rotation of Display 1/3 Overlays button.
// And also for customkey action.
//
bool HaveGauges(void) {


  // GA has always the big compass overlay active
  if (ISGAAIRCRAFT) return true;

  return (GlideBarMode||LKVarioBar||ThermalBar);

}
