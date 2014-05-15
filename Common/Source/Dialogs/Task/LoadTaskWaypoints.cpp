/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"


// This function is called only from Task functions:
//
// CTaskFileHelper.cpp
// LoadCupTask.cpp
// LoadNewTask.cpp
// LoadGpxTask.cpp
// 
// It is supposed that calling function has its own structure.
// The problem is that Comment and Details are always pointers, and in this case
// the calling function should allocate it and loose control.
// So we DONT WANT COMMENTS AND DETAILS BEING PASSED TO TASKPOINTS ANYMORE.
// We dont use them for any purpose in flight. If a pilot want details and comments, he
// can create a real waypoint, and do not expect to load details from a task.
// Details is an external file, always!
//
// NOTE: up to v4 we have always saved taskpoints WITH NO COMMENTS AND NO DETAILS
// only the old pointers to them, in the waypointlist, were save to tsk, but not used anymore.

// Returns the waypoint index, or -1 if error (an unsigned int, not a bool)

int FindOrAddWaypoint(WAYPOINT *read_waypoint, bool look_for_airfield) {
    read_waypoint->Name[NAME_SIZE-1] = 0; // prevent overrun if data is bogus

    int waypoint_index=-1;

    // Search for waypoint having same name AND same lat/lon/flags.
    // If we are searching for airfield, search for waypoint having same name and
    // lat/lon within 1km , and having flags set as airfieldsolid or grass.
    // We dont look at anything else for matching.

    if (look_for_airfield) 
        waypoint_index = FindMatchingAirfield(read_waypoint);
    else
        waypoint_index = FindMatchingWaypoint(read_waypoint);

    if (waypoint_index == -1) { // waypoint not found, so add it!
        WAYPOINT* new_waypoint = GrowWaypointList();
        if (!new_waypoint) { // error, can't allocate!
            return -1;
        }

        //
        // Note: we dont save task waypoints inside WP files!
        // SO WE DONT NEED TO USE COMMENTS and DETAILS. They are useless.
        //
        memcpy(new_waypoint, read_waypoint, sizeof(WAYPOINT));
        // this is  needed for avoid freeing twice ...
        // ownership of allocated memory is transfered from "read_waypoint" to "new_waypoint"
        read_waypoint->Details = NULL;
        read_waypoint->Comment = NULL;
        
        new_waypoint->FileNum=-1; // HERE WE SET THE FLAG FOR "DO NOT SAVE TO WAYPOINT FILE"
        waypoint_index = NumberOfWayPoints-1;
    }
    return waypoint_index;
}

