/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "NavFunctions.h"

int CalculateWaypointApproxDistance(int scx_aircraft, int scy_aircraft,
                                    int i) {

  // Do preliminary fast search, by converting to screen coordinates
  int sc_x, sc_y;
  LatLon2Flat(WayPointList[i].Longitude, 
              WayPointList[i].Latitude, &sc_x, &sc_y);
  int dx, dy;
  dx = scx_aircraft-sc_x;
  dy = scy_aircraft-sc_y;

  return isqrt4(dx*dx+dy*dy);
}

