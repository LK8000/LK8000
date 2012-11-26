/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "RasterTerrain.h"



void WaypointAltitudeFromTerrain(WAYPOINT* Temp) {
  double myalt;
  RasterTerrain::Lock();
  RasterTerrain::SetTerrainRounding(0.0,0.0);
  
  myalt = RasterTerrain::GetTerrainHeight(Temp->Latitude, Temp->Longitude);
  if (myalt==TERRAIN_INVALID) myalt=0; //@ 101027 FIX

  if (myalt>0) {
    Temp->Altitude = myalt;
  } else {
    // error, can't find altitude for waypoint!
  }
  RasterTerrain::Unlock();
  
}
