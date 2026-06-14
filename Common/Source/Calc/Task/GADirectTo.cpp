/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights
*/

#include "externs.h"
#include "NavFunctions.h"
#include "AATDistance.h"
#include "GADirectTo.h"

void GA_UpdateDirectToOriginForCourseCapture(NMEA_INFO* Basic) {
  if (!ISGAAIRCRAFT) return;
  if (!DirectToActive || DirectToWaypointIndex < 0) return;
  if (!ValidWayPointFast(DirectToWaypointIndex)) return;

  double dist = 0., bearing_to_fix = 0.;
  DistanceBearing(Basic->Latitude, Basic->Longitude,
                  WayPointList[DirectToWaypointIndex].Latitude,
                  WayPointList[DirectToWaypointIndex].Longitude,
                  &dist, &bearing_to_fix);

  constexpr double COURSE_CAPTURE_DEG = 15.0;
  if (fabs(AngleLimit180(bearing_to_fix - Basic->TrackBearing)) > COURSE_CAPTURE_DEG) {
    DirectToOriginLat = Basic->Latitude;
    DirectToOriginLon = Basic->Longitude;
  }
}

void GA_CheckDirectToOffTaskArrival(NMEA_INFO* Basic) {
  if (!ISGAAIRCRAFT) return;
  if (!DirectToActive || DirectToWaypointIndex < 0) return;
  if (!ValidWayPointFast(DirectToWaypointIndex)) return;

  double dist = 0., bearing = 0.;
  DistanceBearing(Basic->Latitude, Basic->Longitude,
                  WayPointList[DirectToWaypointIndex].Latitude,
                  WayPointList[DirectToWaypointIndex].Longitude,
                  &dist, &bearing);

  constexpr double radius = 1000.;

  if (dist < radius) {
    const double fix_lat = WayPointList[DirectToWaypointIndex].Latitude;
    const double fix_lon = WayPointList[DirectToWaypointIndex].Longitude;

    DirectToOriginLat = fix_lat;
    DirectToOriginLon = fix_lon;

    ActiveTaskPoint = GA_FindNextForwardTaskWP(fix_lat, fix_lon);
    DirectToWaypointIndex = -1;
    DirectToActive = false;
  }
}

int GA_FindNextForwardTaskWP(double from_lat, double from_lon) {
  int result = ActiveTaskPoint;

  for (int i = ActiveTaskPoint; i < MAXTASKPOINTS; i++) {
    if (!ValidTaskPointFast(i)) break;
    result = i;

    // No previous WP: cannot determine leg direction, treat as always ahead.
    if (i == 0 || !ValidTaskPointFast(i - 1)) break;

    // Bearing of the task leg arriving at WP[i] (direction WP[i-1] -> WP[i]).
    double leg_bearing = 0., d = 0.;
    DistanceBearing(WayPointList[Task[i-1].Index].Latitude,
                    WayPointList[Task[i-1].Index].Longitude,
                    WayPointList[Task[i].Index].Latitude,
                    WayPointList[Task[i].Index].Longitude,
                    &d, &leg_bearing);

    // Bearing from WP[i] toward the reference position.
    double wp_to_pos = 0.;
    DistanceBearing(WayPointList[Task[i].Index].Latitude,
                    WayPointList[Task[i].Index].Longitude,
                    from_lat, from_lon,
                    &d, &wp_to_pos);

    // If the reference position is in the "forward" half-plane of WP[i]
    // (within 90° of the leg direction), the position has passed WP[i] — skip it.
    if (fabs(AngleLimit180(wp_to_pos - leg_bearing)) < 90.) continue;

    // WP[i] is still ahead of the position.
    break;
  }

  return result;
}

bool GA_ComputeDirectToDistanceBearing(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  if (!ISGAAIRCRAFT) return false;
  if (!DirectToActive || !ValidWayPointFast(DirectToWaypointIndex)) return false;

  DistanceBearing(Basic->Latitude, Basic->Longitude,
                  WayPointList[DirectToWaypointIndex].Latitude,
                  WayPointList[DirectToWaypointIndex].Longitude,
                  &Calculated->WaypointDistance,
                  &Calculated->WaypointBearing);
  Calculated->ZoomDistance = Calculated->WaypointDistance;
  return true;
}

// Waypoint index set by the Target dialog while the pilot browses with Next/Prev.
// Overrides the default nav index so the bearing line points aircraft → browsed fix.
// -1 means no browse override is active.
static int ga_browse_wp_index  = -1;
// Corresponding Task[] array index for ga_browse_wp_index.
// Stored separately so DrawBearing can query the loop start without acquiring a lock.
static int ga_browse_task_idx  = -1;

void GA_SetTargetBrowseWP(int wp_index, int task_idx) {
  ga_browse_wp_index = wp_index;
  ga_browse_task_idx = task_idx;
}

int GA_GetTargetPanLoopStart(int fallback) {
  if (!ISGAAIRCRAFT) return fallback;

  if (ga_browse_task_idx >= 0) {
    // Off-task Direct To browsing: skip the chain entirely.
    // The approach line (aircraft → browsed WP) is drawn by the first DrawGreatCircle;
    // DrawTask already renders the dashed route, so drawing solid lines here would cover it.
    if (DirectToActive && ValidWayPointFast(DirectToWaypointIndex)) {
      return MAXTASKPOINTS - 1;  // loop starts at MAXTASKPOINTS, never runs
    }
    // Normal task browsing: draw chain from the browsed WP.
    return ga_browse_task_idx;
  }

  // Off-task Direct To (not browsing): include the re-join leg (DirectToWP → Task[fallback]).
  if (DirectToActive && ValidWayPointFast(DirectToWaypointIndex)) {
    return fallback - 1;
  }

  return fallback;
}

int GA_GetDirectToNavIndex() {
  if (!ISGAAIRCRAFT) return -1;

  // Target dialog browse: the pilot is previewing a task WP via Next/Prev.
  // The browsed WP takes priority so the bearing line shows aircraft → browsed
  // fix (which is at the map centre) rather than the off-screen off-task target.
  if (ValidWayPointFast(ga_browse_wp_index)) {
    return ga_browse_wp_index;
  }

  // Off-task Direct To (not currently browsing a task WP).
  if (DirectToActive && ValidWayPointFast(DirectToWaypointIndex)) {
    return DirectToWaypointIndex;
  }

  return -1;
}

void GA_ApplyDirectToAutopilotOverride(int& prev_index, int& next_index) {
  if (ISGAAIRCRAFT && DirectToActive && DirectToWaypointIndex >= 0
      && ValidWayPointFast(DirectToWaypointIndex)) {
    next_index = DirectToWaypointIndex;
    prev_index = -1;
  }
}
