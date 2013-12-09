/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "RasterTerrain.h"



double AltitudeFromTerrain(double Lat, double Lon) {
  double myalt = 0.;
  RasterTerrain::Lock();
  RasterTerrain::SetTerrainRounding(0.0,0.0);
  myalt = RasterTerrain::GetTerrainHeight(Lat, Lon);
  RasterTerrain::Unlock();
  
  return (myalt==TERRAIN_INVALID)?0:myalt;
}

void WaypointAltitudeFromTerrain(WAYPOINT* Temp) {
  double myalt = AltitudeFromTerrain(Temp->Latitude, Temp->Longitude);
  if (myalt>0) {
    Temp->Altitude = myalt;
  } else {
    // error, can't find altitude for waypoint!
  }
}

void UpdateTargetAltitude(int i) {
    Task[i].AATTargetAltitude = AltitudeFromTerrain(Task[i].AATTargetLat, Task[i].AATTargetLon);
    if(!DoOptimizeRoute() && AATEnabled) {
        // for AAT task, use center Alt if target point alt is less than center...
        Task[i].AATTargetAltitude = std::max(WayPointList[Task[i].Index].Altitude, Task[i].AATTargetAltitude);
    }
    if(Task[i].AATTargetAltitude <= 0) {
        Task[i].AATTargetAltitude = WayPointList[Task[i].Index].Altitude;
    } 
}
