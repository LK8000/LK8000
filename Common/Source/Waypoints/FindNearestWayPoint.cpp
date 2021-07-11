/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "LKStyle.h"
#include "NavFunctions.h"



// This is slow, careful!
int FindNearestWayPoint(double X, double Y, double MaxRange)
{
  int NearestIndex = -1;
  double NearestDistance, Dist;

  NearestDistance = MaxRange;

    for(unsigned i=RESWP_FIRST_MARKER;i<WayPointList.size(); ++i) {

      // Consider only valid markers
      if ( (i<NUMRESWP)  &&  (WayPointCalc[i].WpType!=WPT_TURNPOINT) ) continue;

      // Ignore Thermal Hotspot
      if (WayPointList[i].Style == STYLE_THERMAL) {
          continue;
      }

      DistanceBearing(Y,X,
                      WayPointList[i].Latitude,
                      WayPointList[i].Longitude, &Dist, NULL);
      if(Dist < NearestDistance) {
        NearestIndex = i;
        NearestDistance = Dist;
      }
    }
   if(NearestIndex == -1) {
       return -1;
   }

	// now look at TAKEOFF... TODO check all virtuals too
	// Takeoff can be normally very closed to actual airport, but not the same point!
	DistanceBearing(Y,X, WayPointList[RESWP_TAKEOFF].Latitude, WayPointList[RESWP_TAKEOFF].Longitude, &Dist, NULL);
	if ( Dist<=NearestDistance ) {
		// takeoff is closer, and next wp is not even visible...maybe because of zoom
		if  (NearestIndex >RESWP_TAKEOFF) { //  100227 BUGFIX
			if ( WayPointList[NearestIndex].Visible == FALSE ) {
				NearestIndex = RESWP_TAKEOFF;
				NearestDistance = Dist;
			}
		} else { // else ok 100227
			NearestIndex = RESWP_TAKEOFF;
			NearestDistance = Dist;
		}
	}

  if(NearestDistance < MaxRange) {
    return NearestIndex;
  } else {
    return -1;
  }
}
