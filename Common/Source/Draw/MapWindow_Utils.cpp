/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"
#include "NavFunctions.h"
#include "ScreenProjection.h"


ScreenProjection MapWindow::GetProjection() {
  return {
    {GetPanLatitude(), GetPanLongitude()},
    RasterPoint(GetOrigScreen()),
    GetDrawScale(),
    GetDisplayAngle()
  };
}

bool MapWindow::WaypointInTask(int ind) {
  bool retval = false;
  LockTaskData();
  if ((ind>=0)&&(ind<(int)WayPointList.size())) {
    retval = WayPointList[ind].InTask;
  }
  UnlockTaskData();
  return retval;
}



double MapWindow::GetApproxScreenRange() {
  return (zoom.Scale() * max(DrawRect.right-DrawRect.left,
                         DrawRect.bottom-DrawRect.top))
    *1000.0/GetMapResolutionFactor();
}


// Used only by Thread_Calculation main loop
bool MapWindow::IsDisplayRunning() {
  return (!ThreadSuspended && GlobalRunning && ProgramStarted);
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

//#define DEBUG_SCANVIS 1
#define DEBOUNCE_SCANVISIBILITY 1

void MapWindow::ScanVisibility(rectObj *bounds_active) {
  // received when the SetTopoBounds determines the visibility
  // boundary has changed.
  // This happens rarely, so it is good pre-filtering of what is visible.
  // (saves from having to do it every screen redraw)
  const rectObj bounds = *bounds_active;
#if DEBOUNCE_SCANVISIBILITY
  static rectObj oldbounds={0,0,0,0};
  static float oldzoomscale=0;
  static unsigned lasthere=0;

  if (oldbounds.minx==0||MapWindow::ForceVisibilityScan) goto _normal_run; // careful, bounds can be negative

  #if DEBUG_SCANVIS
  StartupStore(_T("OLD BOUNDS: %f %f %f %f  scale=%f\n"),oldbounds.minx, oldbounds.maxx, oldbounds.miny, oldbounds.maxy,oldzoomscale);
  StartupStore(_T("NEW BOUNDS: %f %f %f %f  scale=%f\n"),bounds.minx, bounds.maxx, bounds.miny, bounds.maxy,MapWindow::zoom.Scale());
  #endif

  // If boundaries are inside old boundaries we dont immediately recalculate everything.
  // The drawback is that after many zoom in, we still have "Visible" all waypoints calculated for a low zoom.
  // This is expecially true while swapping multimaps that can have different zoom level saved individually.
  //

  // first we check that some time has passed and we are not debouncing calculations
  if ( (LKHearthBeats-30) >lasthere ) {
	#if DEBUG_SCANVIS
	StartupStore(_T("... too much time has passed, normal run\n\n"));
	#endif
	goto _normal_run;
  }
  // then we look at scale change
  float diffscale;
  diffscale=oldzoomscale-MapWindow::zoom.Scale();
  if (diffscale>3) {
	// scale has lowered by more than 3, so it is a significant zoom
	// notice that while zoomin out we dont have the problem, because
	// boundaries are going to expand.
	#if DEBUG_SCANVIS
	StartupStore(_T("... scale diff=%f  >3, normal run\n\n"),diffscale);
	#endif
	goto _normal_run;
  }
  #if 0
  if (diffscale >= (MapWindow::zoom.Scale()/2)) {
	// .. and also when the zoom has halved,
	#if DEBUG_SCANVIS
	StartupStore(_T("... scale diff=%f  halved, normal run\n\n"),diffscale);
	#endif
	goto _normal_run;
  }
  #endif


  // else check if it is really needed

  if ( (oldbounds.minx <= bounds.minx) &&
     (oldbounds.maxx >= bounds.maxx) &&
     (oldbounds.miny <= bounds.miny) &&
     (oldbounds.maxy >= bounds.maxy) ) {
	#if DEBUG_SCANVIS
	StartupStore(_T("... BOUNDS OVERLOADED, skipping scan\n\n"));
	#endif
	return;
  }

_normal_run:
  lasthere=LKHearthBeats;
  oldbounds=bounds;
  oldzoomscale=MapWindow::zoom.Scale();
#endif // DEBOUNCE_SCANVISIBILITY

  // far visibility for long snail trail

  LONG_SNAIL_POINT *lsv= LongSnailTrail;
  LKASSERT(iLongSnailNext>=0 && iLongSnailNext<=LONGTRAILSIZE);
  const LONG_SNAIL_POINT *lse = lsv+iLongSnailNext;
  while (lsv<lse) {
      lsv->FarVisible = ((lsv->Longitude> bounds.minx) &&
	    (lsv->Longitude< bounds.maxx) &&
	    (lsv->Latitude> bounds.miny) &&
	    (lsv->Latitude< bounds.maxy));
      lsv++;
  }

  // far visibility for waypoints
/*
    WAYPOINT& wv = WayPointList.front();
    const WAYPOINT& we = WayPointList.back();
 */
  for( std::vector<WAYPOINT>::iterator wv = WayPointList.begin(); wv != WayPointList.end(); ++wv) {
    // TODO code: optimise waypoint visibility
	// TODO 110203 make it happen in 3 steps, with MULTICALC approach
      wv->FarVisible = ((wv->Longitude> bounds.minx) &&
			(wv->Longitude< bounds.maxx) &&
			(wv->Latitude> bounds.miny) &&
			(wv->Latitude< bounds.maxy));
  }

  // far visibility for airspace
  CAirspaceManager::Instance().SetFarVisible( *bounds_active );

}



// Used by dlgTarget only
// A note about this function. We are changing map drawing parameters here, but we paint from the winmain thread.
// We are NOT using this from drawthread! the dialog target pan is sort of an hack, and not a good example.
// Do not consider emulating the target dialog, because it should be moved to draw thread somehow.
void MapWindow::SetTargetPan(bool do_pan, int target_point, unsigned dlgSize /* = 0 */)
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
      } else if (UseAATTarget()) {
        if (Task[target_point].AATType == sector_type_t::SECTOR) {
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


void MapWindow::SetPanTaskEdit(unsigned TskPoint) {
    LockTaskData();
    if (ValidTaskPointFast(TskPoint)) {

        MapWindow::Event_Pan(1);

        PanTaskEdit = TskPoint;
        PanLongitude = WayPointList[Task[PanTaskEdit].Index].Longitude;
        PanLatitude = WayPointList[Task[PanTaskEdit].Index].Latitude;

        if ((mode.Is(Mode::MODE_PAN)) || (mode.Is(Mode::MODE_TARGET_PAN))) {
            {
                if (Task[PanTaskEdit].Index != RESWP_PANPOS) {
                    RealActiveWaypoint = Task[PanTaskEdit].Index;
                    LKASSERT(ValidWayPoint(Task[PanTaskEdit].Index));
                    WayPointList[RESWP_PANPOS].Latitude =
                            WayPointList[RealActiveWaypoint].Latitude;
                    WayPointList[RESWP_PANPOS].Longitude =
                            WayPointList[RealActiveWaypoint].Longitude;
                    WayPointList[RESWP_PANPOS].Altitude =
                            WayPointList[RealActiveWaypoint].Altitude;

                    Task[PanTaskEdit].Index = RESWP_PANPOS;
                    RefreshMap();
                }
            }
        }
    }
    UnlockTaskData();
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

//
// Same as above, but we also look if the gauges were drawn in the current run.
// Notice that we are assuming all gauges were drawn before calling this function.
//
bool HaveGaugesDrawn(void) {
  // GA has always the big compass overlay active
  if (ISGAAIRCRAFT) return true;
  return (GlideBarMode||LKVarioBar||(ThermalBar&&MapWindow::ThermalBarDrawn));
}
