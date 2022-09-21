/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "NavFunctions.h"
#include <optional>
#include "Geographic/GeoPoint.h"

namespace {

  std::optional<GeoPoint> GetHomePosition() {
    ScopeLock lock(CritSec_TaskData);
    if (ValidWayPointFast(HomeWaypoint)) {
      return GetWayPointPosition(WayPointList[HomeWaypoint]);
    }
    return {};
  }

} // namespace


void DistanceToHome(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  auto home = GetHomePosition();
  if(home) {
    DistanceBearing(home->latitude, home->longitude,
                    Basic->Latitude, Basic->Longitude,
                    &Calculated->HomeDistance, &Calculated->HomeRadial);
  } else {
    Calculated->HomeDistance = 0.0;
    Calculated->HomeRadial = 0.0; // VENTA3
  }
}


