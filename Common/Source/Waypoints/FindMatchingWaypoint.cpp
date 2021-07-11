/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "LKStyle.h"


int FindMatchingWaypoint(WAYPOINT *waypoint) {

    for (unsigned i=NUMRESWP; i<WayPointList.size(); ++i) {
        if (_tcscmp(waypoint->Name, WayPointList[i].Name)!=0) continue; // if different name, no match
        // if same name, lat lon and flags must be the same in order to match a previously existing waypoint
        if ((fabs(waypoint->Latitude-WayPointList[i].Latitude)<1.0e-6) &&
                (fabs(waypoint->Longitude-WayPointList[i].Longitude)<1.0e-6) &&
                (waypoint->Flags == WayPointList[i].Flags) ) { // name, lat,lon, flags are the same: same wp!
            return i;
        }
    }
  return -1;
}

int FindMatchingAirfield(WAYPOINT *waypoint) {

    const double limit=0.00899928005; //1 Km expressed in deg
    for (unsigned i=NUMRESWP; i<WayPointList.size(); ++i) {
        if(WayPointList[i].Style>=STYLE_AIRFIELDGRASS && //if it is any kind of airport/airfield
                WayPointList[i].Style<=STYLE_AIRFIELDSOLID &&
                (fabs(waypoint->Latitude-WayPointList[i].Latitude)<limit) && //and if coordinates within 1 Km range
                (fabs(waypoint->Longitude-WayPointList[i].Longitude)<limit)) return i; //assume this as the desired WP
    }
    return -1;
}
