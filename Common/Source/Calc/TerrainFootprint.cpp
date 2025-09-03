/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "NavFunctions.h"

void FillGlideFootPrint(double latitude, double longitude, double altitude, DERIVED_INFO *Calculated, double max_range,  GeoPoint* out, size_t count) {

  const GeoPoint* first_out = out; // this is first polygon point (OpenGL or not), used for close polygon.
  for (size_t i = 0; i < count; ++i) {
    const double bearing = (i*360.0)/count;
    double lat, lon;
    bool out_of_range = false;
    const double distance = FinalGlideThroughTerrain(bearing, latitude, longitude, altitude,
                                        Calculated, &lat, &lon, max_range, &out_of_range, nullptr );
    if (out_of_range) {
      FindLatitudeLongitude(latitude, longitude, bearing, distance, &lat, &lon);
    }
    *(out++) = {lat, lon};
  }
  (*out) = (*first_out); // close polygon
}


void TerrainFootprint(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  const double ScreenRange = MapWindow::GetApproxScreenRange();

  // estimate max range (only interested in at most one screen distance away)
  // except we need to scan for terrain base, so 20km search minimum is required
  double mymaxrange = max(20000.0, ScreenRange);

  Calculated->TerrainBase = Calculated->TerrainAlt;

  GeoPoint* out = Calculated->GlideFootPrint;

  // this exist only for allow static_assert, compil time error instead of runtime error.
  const decltype(Calculated->GlideFootPrint) GlideFootPrint_Test = {};
  
#ifdef ENABLE_OPENGL
  static_assert(std::size(GlideFootPrint_Test) == NUMTERRAINSWEEPS +2, "#GlideFootPrint invalid array size");
  // first point is current poisition
  *(out++) = {Basic->Latitude, Basic->Longitude};
#else
  static_assert(std::size(GlideFootPrint_Test) == NUMTERRAINSWEEPS +1, "#GlideFootPrint invalid array size");
#endif

  if(FinalGlideTerrain) {
    FillGlideFootPrint(Basic->Latitude, Basic->Longitude, Calculated->NavAltitude, Calculated, mymaxrange, out, NUMTERRAINSWEEPS);
    Calculated->GlideFootPrint_valid = true;
  } else {
    Calculated->GlideFootPrint_valid = false;  
  }

  LockTaskData();
  
  bool GlideFootPrint2_valid = false;
  
  // Now calculate the 2nd glide footprint, which is the glide range
  // after reaching the next task waypoint.  First, let's make sure
  // we're flying a task.
  
  if ((FinalGlideTerrain > 2) && ValidTaskPoint(ActiveTaskPoint)) {
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
      lat_wp   = Task[ActiveTaskPoint].AATTargetLat;
      lon_wp   = Task[ActiveTaskPoint].AATTargetLon;
    } else {
      wp_index = TASKINDEX;
      lat_wp   = WayPointList[TASKINDEX].Latitude;
      lon_wp   = WayPointList[TASKINDEX].Longitude;
    }
    
    double alt_arriv = WayPointCalc[wp_index].AltArriv[AltArrivMode];
    
    // Calculate arrival altitude relative to the "terrain height"
    // safety setting.

    const double alt_arriv_agl = (CheckSafetyAltitudeApplies(wp_index)) ?
           (alt_arriv + (SAFETYALTITUDEARRIVAL/10)) : alt_arriv;

    // relative to "terrain height":
    alt_arriv = alt_arriv_agl - (SAFETYALTITUDETERRAIN/10);

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
   
      FillGlideFootPrint(lat_wp, lon_wp, alt_arriv_msl, Calculated, mymaxrange, Calculated->GlideFootPrint2, NUMTERRAINSWEEPS);
      GlideFootPrint2_valid = true;

    } // if reachable above "terrain height"
  } // if valid task point
  Calculated->GlideFootPrint2_valid = GlideFootPrint2_valid;
  
  UnlockTaskData();
}

