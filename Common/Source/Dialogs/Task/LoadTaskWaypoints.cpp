/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"


// this is called only from Task LoadTaskWaypoints
int FindOrAddWaypoint(WAYPOINT *read_waypoint, bool look_for_airfield) {
    // this is an invalid pointer!
    read_waypoint->Name[NAME_SIZE-1] = 0; // prevent overrun if data is bogus

    int waypoint_index=-1;
    if(look_for_airfield) waypoint_index = FindMatchingAirfield(read_waypoint);
    else waypoint_index = FindMatchingWaypoint(read_waypoint);
    if (waypoint_index == -1) { // waypoint not found, so add it!
        // TODO bug: Set WAYPOINTFILECHANGED so waypoints get saved?
        // NO, we dont save task waypoints inside WP files!
        WAYPOINT* new_waypoint = GrowWaypointList();
        if (!new_waypoint) { // error, can't allocate!
            return false;
        }
        memcpy(new_waypoint, read_waypoint, sizeof(WAYPOINT));
        // this is  needed for avoid freeing twice ...
        read_waypoint->Details = NULL;
        read_waypoint->Comment = NULL;

        // 100229 set no-save flag on
        new_waypoint->FileNum=-1;
        waypoint_index = NumberOfWayPoints-1;
    }
    return waypoint_index;
}
