/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "AATDistance.h"
#include "CalcTask.h"
#include "Calc/Task/TimeGates.h"
#include "NavFunctions.h"

// GA only: check arrival at an off-task DirectTo fix and transition to task leg.
// Called each cycle when DirectToWaypointIndex >= 0.
static void CheckDirectToOffTaskArrival(NMEA_INFO* Basic) {
  if (!ISGAAIRCRAFT) return;
  if (!DirectToActive || DirectToWaypointIndex < 0) return;
  if (!ValidWayPointFast(DirectToWaypointIndex)) return;

  double dist = 0., bearing = 0.;
  DistanceBearing(Basic->Latitude, Basic->Longitude,
                  WayPointList[DirectToWaypointIndex].Latitude,
                  WayPointList[DirectToWaypointIndex].Longitude,
                  &dist, &bearing);

  // Arrival radius: use current task sector radius if available, else 500 m
  double radius = 500.;
  if (ValidTaskPointFast(ActiveTaskPoint)) {
    radius = (Task[ActiveTaskPoint].AATType == sector_type_t::SECTOR)
             ? Task[ActiveTaskPoint].AATSectorRadius
             : Task[ActiveTaskPoint].AATCircleRadius;
  }

  if (dist < radius) {
    const double fix_lat = WayPointList[DirectToWaypointIndex].Latitude;
    const double fix_lon = WayPointList[DirectToWaypointIndex].Longitude;

    // Origin for autopilot XTE: the fix just reached
    DirectToOriginLat = fix_lat;
    DirectToOriginLon = fix_lon;

    // Resume to nearest remaining task point (measured from the fix position)
    double min_dist = 1e20;
    int nearest_tp = ActiveTaskPoint;
    for (int i = ActiveTaskPoint; i < MAXTASKPOINTS; i++) {
      if (!ValidTaskPointFast(i)) break;
      double d = 0., b = 0.;
      DistanceBearing(fix_lat, fix_lon,
                      WayPointList[Task[i].Index].Latitude,
                      WayPointList[Task[i].Index].Longitude,
                      &d, &b);
      if (d < min_dist) {
        min_dist = d;
        nearest_tp = i;
      }
    }
    ActiveTaskPoint = nearest_tp;
    DirectToWaypointIndex = -1;
  }
}

// This is called from main DoCalculations each time, only when running a real task
void InSector(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  static int LastStartSector = -1;

  const std::lock_guard lock(CritSec_TaskData);

  CheckDirectToOffTaskArrival(Basic);

  if (ActiveTaskPoint < 0) {
    return;
  }

  if (!ValidTaskPointFast(ActiveTaskPoint)) {
    return;
  }

  // only if running a real task (a least 2 task point)
  if (!ValidTaskPointFast(1) || !ValidTaskPointFast(1)) {
    return;
  }

  // Paragliders task system
  // Case A: start entering the sector/cylinder
  //    you must be outside sector when gate is open.
  //    you are warned that you are already inside sector
  //    before the gate is open, when gate is opening
  //    in <10 minutes task restart is manual
  // Case B: start exiting the sector

  // by default, we are not in the sector
  Calculated->IsInSector = false;

  if (ActiveTaskPoint == 0) { // before Start
    CheckStart(Basic, Calculated, &LastStartSector);
  } else {
    LastStartSector = -1;
    if (IsFinalWaypoint()) { // final glide
      AddAATPoint(Basic, Calculated, ActiveTaskPoint - 1);
      CheckFinish(Basic, Calculated);
    } else { // turpoint in between
      if (!UseGates()) {
        CheckRestart(Basic, Calculated, &LastStartSector);  // 100507
      }
      CheckInSector(Basic, Calculated);
    }
  }
}
