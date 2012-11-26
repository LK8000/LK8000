/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"






int FindMatchingWaypoint(WAYPOINT *waypoint) {
  if (!WayPointList) {
    return -1;
  }
  unsigned int i;

  for (i=NUMRESWP; i<NumberOfWayPoints; i++) {

	// if different name, no match
	if (_tcscmp(waypoint->Name, WayPointList[i].Name)!=0) continue;

	// if same name, lat lon and flags must be the same in order to match
	// a previously existing waypoint
	if ((fabs(waypoint->Latitude-WayPointList[i].Latitude)<1.0e-6) 
	&& (fabs(waypoint->Longitude-WayPointList[i].Longitude)<1.0e-6) &&
	(waypoint->Flags == WayPointList[i].Flags) ) {
		// name, lat,lon, flags are the same: same wp!
		return i;
	}
  }
  
  return -1;
}


