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

// Approach path lengths in meters
constexpr double FINAL_EXTEND_M = 5000.0;
constexpr double DOWNWIND_LEN_M = 2000.0;
constexpr double BASE_LEN_M = 1500.0;
constexpr double PATTERN_OFFSET_M = 500.0;

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

  if (MapApproachMode == 0 || MapApproachMode == 2) {
    // Direct: extended centerline (from far point toward runway)
    double far_lat, far_lon;
    FindLatitudeLongitude(clat, clon, rw_recip, FINAL_EXTEND_M, &far_lat, &far_lon);
    drawSegment(far_lat, far_lon, clat, clon);
  }

  if (MapApproachMode == 1 || MapApproachMode == 2) {
    // Circuit: downwind (opposite to final) -> base -> final
    // Offset side: left circuit = pattern to the left of approach direction (offset 90° left from rw_recip)
    const double side = (MapApproachCircuitSide == 0) ? -90.0 : 90.0;
    const double downwind_brg = rw_recip;
    const double base_brg = AngleLimit360(rw_brg + side);

    double dw_lat, dw_lon;
    FindLatitudeLongitude(clat, clon, downwind_brg, PATTERN_OFFSET_M, &dw_lat, &dw_lon);

    double dw_end_lat, dw_end_lon;
    FindLatitudeLongitude(dw_lat, dw_lon, downwind_brg, DOWNWIND_LEN_M, &dw_end_lat, &dw_end_lon);

    double base_end_lat, base_end_lon;
    FindLatitudeLongitude(dw_end_lat, dw_end_lon, base_brg, BASE_LEN_M, &base_end_lat, &base_end_lon);

    // Downwind leg
    drawSegment(dw_lat, dw_lon, dw_end_lat, dw_end_lon);
    // Base leg
    drawSegment(dw_end_lat, dw_end_lon, base_end_lat, base_end_lon);
    // Final: from end of base to runway centre
    drawSegment(base_end_lat, base_end_lon, clat, clon);
  }

  Surface.SelectObject(oldPen);
}
