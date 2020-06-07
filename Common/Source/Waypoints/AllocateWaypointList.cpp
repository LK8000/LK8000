/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "Dialogs.h"
#include <exception>


bool AddWaypoint(WAYPOINT& Waypoint) {

    try {
        WayPointList.push_back(Waypoint);
        // WAYPOINT struct contains pointer to malloc string,
        // ownership of this string is transfered to WayPointList
        // Reset all content by security
        Waypoint = {};

    } catch (std::exception& e) {
        const tstring what = to_tstring(e.what());
        StartupStore(_T("FAILED! <%s>" NEWLINE), what.c_str());
        MessageBoxX(MsgToken(486), // "Not Enough Memory For Waypoints"
                MsgToken(266) /* "Error" */, mbOk);
        return false;
    }

    try {
        WayPointCalc.resize(WayPointList.size());
    } catch (std::exception& e) {
        const tstring what = to_tstring(e.what());
        StartupStore(_T("FAILED! <%s>" NEWLINE), what.c_str());
        MessageBoxX(MsgToken(486), // "Not Enough Memory For Waypoints"
                MsgToken(266) /* "Error" */, mbOk);
        return false;
    }
    return true;
}

void SetWaypointComment(WAYPOINT& waypoint, const TCHAR* string) {
    if(waypoint.Comment) {
        free(waypoint.Comment);
        waypoint.Comment = nullptr;
    }
    if(string && string[0] != _T('\0')) {
        waypoint.Comment = _tcsdup(string);
    }
}

void SetWaypointDetails(WAYPOINT& waypoint, const TCHAR* string) {
    if(waypoint.Details) {
        free(waypoint.Details);
        waypoint.Details = nullptr;
    }
    if(string && string[0] != _T('\0')) {
        waypoint.Details = _tcsdup(string);
    }
}
