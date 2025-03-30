/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "RasterTerrain.h"

double AltitudeFromTerrain(double Lat, double Lon) {
  double myalt = WithLock(RasterTerrain::mutex, [&]() {
    RasterTerrain::SetTerrainRounding(0., 0.);
    return RasterTerrain::GetTerrainHeight(Lat, Lon);
  });
  return (myalt == TERRAIN_INVALID) ? 0 : myalt;
}

void WaypointAltitudeFromTerrain(WAYPOINT* Temp) {
  double myalt = AltitudeFromTerrain(Temp->Latitude, Temp->Longitude);
  if (myalt>0) {
    Temp->Altitude = myalt;
  } else {
    // error, can't find altitude for waypoint!
  }
}

void UpdateTargetAltitude(TASK_POINT& TskPt) {
    TskPt.AATTargetAltitude = AltitudeFromTerrain(TskPt.AATTargetLat, TskPt.AATTargetLon);
    if(!DoOptimizeRoute() && UseAATTarget()) {
        // for AAT task, use center Alt if target point alt is less than center...
        TskPt.AATTargetAltitude = std::max(WayPointList[TskPt.Index].Altitude, TskPt.AATTargetAltitude);
    }
    if(TskPt.AATTargetAltitude <= 0) {
        TskPt.AATTargetAltitude = WayPointList[TskPt.Index].Altitude;
    }
}
