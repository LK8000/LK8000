/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"



void TerrainFootprint(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  double bearing, distance;
  double lat, lon;
  bool out_of_range;

  // estimate max range (only interested in at most one screen distance away)
  // except we need to scan for terrain base, so 20km search minimum is required
  double mymaxrange = max(20000.0, MapWindow::GetApproxScreenRange());

  Calculated->TerrainBase = Calculated->TerrainAlt;

  for (int i=0; i<=NUMTERRAINSWEEPS; i++) {
    bearing = (i*360.0)/NUMTERRAINSWEEPS;
    distance = FinalGlideThroughTerrain(bearing, 
                                        Basic, 
                                        Calculated, &lat, &lon,
                                        mymaxrange, &out_of_range,
					&Calculated->TerrainBase);
    if (out_of_range) {
      FindLatitudeLongitude(Basic->Latitude, Basic->Longitude, 
                            bearing, 
                            distance,	 // limited, originally maxrange and more..
                            &lat, &lon);
    }
    Calculated->GlideFootPrint[i].x = lon;
    Calculated->GlideFootPrint[i].y = lat;
  }
  Calculated->Experimental = Calculated->TerrainBase;
}

