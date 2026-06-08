/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "Terrain.h"
#include "Bitmaps.h"
#include "ScreenProjection.h"
#include "NavFunctions.h"

void MapWindow::DrawBearing(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj)
{
  double startLat = DrawInfo.Latitude;
  double startLon = DrawInfo.Longitude;
  double targetLat;
  double targetLon;

  // Approach: track target depends on mode. Direct = outer point of direct leg (user distance),
  // Circuit = waypoint centre.
  if (mode.Is(Mode::MODE_APPROACH_PAN) && MapApproachEnabled && MapApproachWaypoint >= 0) {
    double wlat = 0.0, wlon = 0.0;
    int rw_dir = 0;
    bool approach_valid = false;
    {
      const std::lock_guard lock(CritSec_TaskData);
      const int wp_index = MapApproachWaypoint;
      if (ValidWayPointFast(wp_index) && WayPointCalc[wp_index].IsLandable) {
        const WAYPOINT& wp = WayPointList[wp_index];
        wlat = wp.Latitude;
        wlon = wp.Longitude;
        rw_dir = MapApproachRunwayDir >= 0 ? MapApproachRunwayDir : wp.RunwayDir;
        approach_valid = true;
      }
    }

    if (approach_valid) {
      if (rw_dir < 0) rw_dir = 0;
      const double rw_recip = AngleLimit360(static_cast<double>(rw_dir) + 180.0);

      if (MapApproachMode == 0) {
        // Direct: bearing line goes to the outer direct point (start of direct leg), not waypoint centre
        const double leg_m = max(100.0, MapApproachDirectDistance_m);
        FindLatitudeLongitude(wlat, wlon, rw_recip, leg_m, &targetLat, &targetLon);
        DrawGreatCircle(Surface, rc, _Proj, startLon, startLat, targetLon, targetLat);
        return;
      }
      // Circuit: track to waypoint centre
      targetLat = wlat;
      targetLon = wlon;
      DrawGreatCircle(Surface, rc, _Proj, startLon, startLat, targetLon, targetLat);
      return;
    }
  }

  int overindex=GetOvertargetIndex();
  if (overindex<0) return;

  if (OvertargetMode>OVT_TASK) {
    LockTaskData();
    targetLat = WayPointList[overindex].Latitude;
    targetLon = WayPointList[overindex].Longitude;
    UnlockTaskData();
    DrawGreatCircle(Surface, rc, _Proj, startLon, startLat, targetLon, targetLat);
  }
  else {
    if (!ValidTaskPoint(ActiveTaskPoint)) {
      return;
    }
    LockTaskData();

    if (UseAATTarget() && ( DoOptimizeRoute() || ((ActiveTaskPoint>0) && ValidTaskPoint(ActiveTaskPoint+1))) ) {
      targetLat = Task[ActiveTaskPoint].AATTargetLat;
      targetLon = Task[ActiveTaskPoint].AATTargetLon;
    } else {
      targetLat = WayPointList[overindex].Latitude;
      targetLon = WayPointList[overindex].Longitude;
    }
    UnlockTaskData();

    DrawGreatCircle(Surface, rc, _Proj, startLon, startLat, targetLon, targetLat);

    if (mode.Is(Mode::MODE_TARGET_PAN)) {
      // Draw all of task if in target pan mode
      startLat = targetLat;
      startLon = targetLon;

      LockTaskData();
      const int loopStart = (ISGAAIRCRAFT && DirectToActive && ValidWayPointFast(DirectToWaypointIndex))
                            ? ActiveTaskPoint : ActiveTaskPoint + 1;
      for (int i = loopStart; i < MAXTASKPOINTS; i++) {
        if (ValidTaskPoint(i)) {

          if (UseAATTarget() && ValidTaskPoint(i+1)) {
            targetLat = Task[i].AATTargetLat;
            targetLon = Task[i].AATTargetLon;
          } else {
            targetLat = WayPointList[Task[i].Index].Latitude;
            targetLon = WayPointList[Task[i].Index].Longitude;
          }

          DrawGreatCircle(Surface, rc, _Proj, startLon, startLat,
                          targetLon, targetLat);

          startLat = targetLat;
          startLon = targetLon;
        }
      }
      UnlockTaskData();
    }
  }

  if (UseAATTarget()) {
    // draw symbol at target, makes it easier to see
    LockTaskData();
    if(mode.Is(Mode::MODE_TARGET_PAN)) {
      for(int i=ActiveTaskPoint+1; i<MAXTASKPOINTS; i++) {
        if(ValidTaskPoint(i) && ValidTaskPoint(i+1)) {
          if(i>= ActiveTaskPoint) {
            if(PointVisible(Task[i].AATTargetLon, Task[i].AATTargetLat)) {
              const RasterPoint sct = _Proj.ToRasterPoint(Task[i].AATTargetLat, Task[i].AATTargetLon);
              DrawBitmapIn(Surface, sct, hBmpTarget);
            }
          }
        }
      }
    }
    if(ValidTaskPoint(ActiveTaskPoint+1) && (DoOptimizeRoute() || (ActiveTaskPoint>0)) ) {
      if(PointVisible(Task[ActiveTaskPoint].AATTargetLon, Task[ActiveTaskPoint].AATTargetLat)) {
        const POINT sct = _Proj.ToRasterPoint(Task[ActiveTaskPoint].AATTargetLat, Task[ActiveTaskPoint].AATTargetLon);
        DrawBitmapIn(Surface, sct, hBmpTarget);
      }
    }
    UnlockTaskData();
  }
}
