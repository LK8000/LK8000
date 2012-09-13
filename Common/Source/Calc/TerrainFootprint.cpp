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
  #ifdef GTL2
  double ScreenRange = MapWindow::GetApproxScreenRange();
  #endif

  // estimate max range (only interested in at most one screen distance away)
  // except we need to scan for terrain base, so 20km search minimum is required
  #ifdef GTL2
  double mymaxrange = max(20000.0, ScreenRange);
  #else
  double mymaxrange = max(20000.0, MapWindow::GetApproxScreenRange());
  #endif

  Calculated->TerrainBase = Calculated->TerrainAlt;

  for (int i=0; i<=NUMTERRAINSWEEPS; i++) {
    bearing = (i*360.0)/NUMTERRAINSWEEPS;
    distance = FinalGlideThroughTerrain(bearing, 
                                      #ifdef GTL2
                                        Basic->Latitude,
                                        Basic->Longitude,
                                        Calculated->NavAltitude,
                                      #else
                                        Basic, 
                                      #endif
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
  
  #ifdef GTL2
  // Now calculate the 2nd glide footprint, which is the glide range
  // after reaching the next task waypoint.  First, let's make sure
  // we're flying a task.
  
  if ((FinalGlideTerrain > 2) && ValidTaskPoint(ActiveWayPoint)) {
    // Only bother calculating the footprint, if
    // we can reach the next waypoint.
    
    double lat_wp, lon_wp; // location of footprint "center"
    bool AATandMTP  = ACTIVE_WP_IS_AAT_AREA; // MTP: mid-task point
    bool DoOptRoute = DoOptimizeRoute();
    int  wp_index = 0; // index for WayPointCalc and WayPointList
    
    if (DoOptRoute || AATandMTP) {
      // The next line is just to force an immediate refresh of
      // arrival altitude for an AAT or "optimized" virtual WP.
      // Without this, the glide line could briefly (1-2 seconds)
      // be drawn using the arrival altitude of the previously-
      // active WP.
      DoAlternates(Basic, Calculated, RESWP_OPTIMIZED);
      wp_index = RESWP_OPTIMIZED;
      lat_wp   = Task[ActiveWayPoint].AATTargetLat;
      lon_wp   = Task[ActiveWayPoint].AATTargetLon;
    } else {
      wp_index = TASKINDEX;
      lat_wp   = WayPointList[TASKINDEX].Latitude;
      lon_wp   = WayPointList[TASKINDEX].Longitude;
    }
    
    double alt_arriv = WayPointCalc[wp_index].AltArriv[AltArrivMode];
    
    // Calculate arrival altitude relative to the "terrain height"
    // safety setting.

    double alt_arriv_agl = (CheckSafetyAltitudeApplies(wp_index)) ?
           (alt_arriv + SAFETYALTITUDEARRIVAL) : alt_arriv;

    // relative to "terrain height":
    alt_arriv = alt_arriv_agl - SAFETYALTITUDETERRAIN;

    // Only bother calculating the 2nd glide footprint, if its
    // center (the next WP) is reachable above "terrain height".

    if (alt_arriv > 0) { // WP reachable above "terrain height"

      double alt_arriv_msl = alt_arriv_agl +
                             WayPointList[wp_index].Altitude;
      const double PanLatitude  = MapWindow::GetPanLatitude();
      const double PanLongitude = MapWindow::GetPanLongitude();

      // Rewrite "mymaxrange" to the distance from the next WP
      // to the center of the screen/map.  Then add "ScreenRange"
      // to make sure to cover the entire screen.

      DistanceBearing(lat_wp, lon_wp, PanLatitude,
                      PanLongitude, &mymaxrange, NULL);
      mymaxrange += ScreenRange;
      
      for (int i=0; i<=NUMTERRAINSWEEPS; i++) {
        bearing = (i * 360.0) / NUMTERRAINSWEEPS;

        distance = FinalGlideThroughTerrain(bearing, lat_wp, lon_wp,
                                            alt_arriv_msl,
                                            Calculated, &lat, &lon,
                                            mymaxrange, &out_of_range,
                                            NULL);

        if (out_of_range)
          FindLatitudeLongitude(lat_wp, lon_wp, 
                                bearing, distance,
                                &lat, &lon);

        GlideFootPrint2[i].x = lon;
        GlideFootPrint2[i].y = lat;
      }
    } // if reachable above "terrain height"
  } // if valid task point
  #endif
}

