/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: DoAlternates.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "Waypointparser.h"
#include "NavFunctions.h"
#include "utils/printf.h"

/*
 * Used by Alternates and BestAlternate
 * Colors VGR are used by DrawNearest &c.
 */
void DoAlternates(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int AltWaypoint) {
  ScopeLock lock(CritSec_TaskData);

  if (AltWaypoint == RESWP_EXT_TARGET) {
    if (WayPointList[RESWP_EXT_TARGET].Altitude == RESWP_INVALIDNUMBER) {
      // set EXT altitude from sterrain if not set by external device
      WaypointAltitudeFromTerrain(&WayPointList[RESWP_EXT_TARGET]);
    }
  }

  // If flying an AAT and working on the RESWP_OPTIMIZED waypoint, then use
  // this "optimized" waypoint to store data for the AAT virtual waypoint.

  if ((AltWaypoint == RESWP_OPTIMIZED) && (gTaskType == task_type_t::AAT)) {
    WayPointList[RESWP_OPTIMIZED].Latitude = Task[ActiveTaskPoint].AATTargetLat;
    WayPointList[RESWP_OPTIMIZED].Longitude = Task[ActiveTaskPoint].AATTargetLon;
    WaypointAltitudeFromTerrain(&WayPointList[RESWP_OPTIMIZED]);
    int wp_idx = Task[ActiveTaskPoint].Index;
    WayPointList[RESWP_OPTIMIZED].Altitude = WayPointList[wp_idx].Altitude;
    lk::snprintf(WayPointList[RESWP_OPTIMIZED].Name, NAME_SIZE, _T("!%s"), WayPointList[wp_idx].Name);
  }

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


