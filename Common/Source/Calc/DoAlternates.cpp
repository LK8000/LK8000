/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: DoAlternates.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#ifdef GTL2
#include "Waypointparser.h"
#endif


/*
 * Used by Alternates and BestAlternate
 * Colors VGR are used by DrawNearest &c.
 */
void DoAlternates(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int AltWaypoint) {

  #ifdef GTL2
  // If flying an AAT and working on the RESWP_OPTIMIZED waypoint, then use
  // this "optimized" waypoint to store data for the AAT virtual waypoint.

  if ((AltWaypoint == RESWP_OPTIMIZED) && 
    (!ISPARAGLIDER || (AATEnabled && !DoOptimizeRoute()))) {
    WayPointList[AltWaypoint].Latitude  = Task[ActiveWayPoint].AATTargetLat;
    WayPointList[AltWaypoint].Longitude = Task[ActiveWayPoint].AATTargetLon;
    WaypointAltitudeFromTerrain(&WayPointList[AltWaypoint]);
  }
  #endif

  // handle virtual wps as alternates
  if (AltWaypoint<=RESWP_END) {
	if (!ValidResWayPoint(AltWaypoint)) return;
  } else {
	if (!ValidWayPoint(AltWaypoint)) return;
  }

  double *altwp_dist	= &WayPointCalc[AltWaypoint].Distance;
  double *altwp_gr	= &WayPointCalc[AltWaypoint].GR;
  double *altwp_arrival	= &WayPointCalc[AltWaypoint].AltArriv[AltArrivMode];

  DistanceBearing(WayPointList[AltWaypoint].Latitude, WayPointList[AltWaypoint].Longitude,
                  Basic->Latitude, Basic->Longitude,
                  altwp_dist, NULL);

  *altwp_gr = CalculateGlideRatio( *altwp_dist,
	Calculated->NavAltitude - WayPointList[AltWaypoint].Altitude - GetSafetyAltitude(AltWaypoint));

  // We need to calculate arrival also for BestAlternate, since the last "reachable" could be
  // even 60 seconds old and things may have changed drastically
  *altwp_arrival = CalculateWaypointArrivalAltitude(Basic, Calculated, AltWaypoint);
  
  WayPointCalc[AltWaypoint].VGR = GetVisualGlideRatio(*altwp_arrival, *altwp_gr);
} 


