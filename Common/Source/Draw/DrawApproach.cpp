/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   Draw approach path overlay: direct (extended centerline) and/or circuit
   pattern for the selected airfield and assigned runway.
*/

#include "externs.h"
#include "LKInterface.h"
#include "LKObjects.h"
#include "NavFunctions.h"
#include "ScreenProjection.h"

namespace {

// Direct: start point at 5 km from runway center on extended centerline
constexpr double DIRECT_5KM_M = 5000.0;

// VFR circuit: downwind at 30 s flight from runway center (~800 m at 50 kt)
constexpr double DOWNWIND_OFFSET_M = 800.0;   // ~30 s at 50 kt
// Turn from downwind to base at 45° radial from threshold: along downwind = offset
constexpr double DOWNWIND_TO_45_M = 800.0;    // same as offset in standard pattern
constexpr double BASE_LEN_M = 1500.0;         // base leg to final

} // namespace

void MapWindow::DrawApproach(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj)
{
  if (!MapApproachEnabled) {
    return;
  }

  int wp_index = MapApproachWaypoint >= 0 ? MapApproachWaypoint : GetOvertargetIndex();
  if (wp_index < 0) {
    return;
  }

  WAYPOINT wp = {};
  bool landable = false;
  {
    ScopeLock lock(CritSec_TaskData);
    if (!ValidWayPointFast(wp_index)) {
      return;
    }
    wp = WayPointList[wp_index];
    landable = WayPointCalc[wp_index].IsLandable;
  }

  if (!landable) {
    return;
  }

  // Runway final heading (direction of approach, magnetic/true as in waypoint)
  int rw_dir = MapApproachRunwayDir >= 0 ? MapApproachRunwayDir : wp.RunwayDir;
  if (rw_dir < 0) {
    rw_dir = 0;  // fallback
  }
  const double rw_brg = AngleLimit360(static_cast<double>(rw_dir));
  const double rw_recip = AngleLimit360(rw_brg + 180.0);

  const double clat = wp.Latitude;
  const double clon = wp.Longitude;

  auto drawSegment = [&](double lat1, double lon1, double lat2, double lon2) {
    DrawGreatCircle(Surface, rc, _Proj, lon1, lat1, lon2, lat2);
  };

  const auto oldPen = Surface.SelectObject(LKPen_Blue_N1);

  // Draw only when all required choices are made: Direct + runway, or Circuit + runway + left/right
  const bool runway_selected = (MapApproachRunwayDir >= 0 || wp.RunwayDir >= 0);

  // Direct: only when Direct is selected (MapApproachMode == 0) and runway is selected.
  if (runway_selected && MapApproachMode == 0) {
    double start_lat, start_lon;
    FindLatitudeLongitude(clat, clon, rw_recip, DIRECT_5KM_M, &start_lat, &start_lon);
    drawSegment(start_lat, start_lon, clat, clon);
  }

  // Circuit: runway and circuit side (left/right) selected.
  if (runway_selected && MapApproachMode == 1 && MapApproachCircuitSide >= 0) {
    // Left = 0, Right = 1. Circuit side: offset 90° from approach direction.
    const double side = (MapApproachCircuitSide == 0) ? -90.0 : 90.0;
    const double downwind_brg = rw_recip;
    const double base_brg = AngleLimit360(rw_brg + side);

    // Downwind leg at 30 s from center (~800 m); perpendicular to runway on circuit side
    double dw_lat, dw_lon;
    const double perp_brg = AngleLimit360(rw_brg + side);
    FindLatitudeLongitude(clat, clon, perp_brg, DOWNWIND_OFFSET_M, &dw_lat, &dw_lon);

    // Turn to base at 45° radial from threshold (in standard pattern, distance along downwind = offset)
    double turn45_lat, turn45_lon;
    FindLatitudeLongitude(dw_lat, dw_lon, downwind_brg, DOWNWIND_TO_45_M, &turn45_lat, &turn45_lon);

    double base_end_lat, base_end_lon;
    FindLatitudeLongitude(turn45_lat, turn45_lon, base_brg, BASE_LEN_M, &base_end_lat, &base_end_lon);

    // Downwind: from entry to 45° turn point
    drawSegment(dw_lat, dw_lon, turn45_lat, turn45_lon);
    // Base leg
    drawSegment(turn45_lat, turn45_lon, base_end_lat, base_end_lon);
    // Final: from end of base to runway centre
    drawSegment(base_end_lat, base_end_lon, clat, clon);
  }

  Surface.SelectObject(oldPen);
}
