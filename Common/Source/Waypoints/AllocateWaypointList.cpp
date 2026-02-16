/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "Dialogs.h"
#include <exception>


static void InitOneWayPointCalc(size_t i) {
    WayPointCalc[i].Preferred = false;
    WayPointCalc[i].Distance = -1;
    WayPointCalc[i].Bearing = -1;
    WayPointCalc[i].GR = -1;
    WayPointCalc[i].VGR = -1;
    WayPointCalc[i].NextETE = 0;
    WayPointCalc[i].NextAvrETE = 0;
    if ((WayPointList[i].Flags & AIRPORT) == AIRPORT) {
        WayPointCalc[i].IsAirport = true;
        WayPointCalc[i].IsLandable = true;
        WayPointCalc[i].IsOutlanding = false;
        WayPointCalc[i].WpType = WPT_AIRPORT;
        DisableBestAlternate = false;
    } else if ((WayPointList[i].Flags & LANDPOINT) == LANDPOINT) {
        WayPointCalc[i].IsAirport = false;
        WayPointCalc[i].IsLandable = true;
        WayPointCalc[i].IsOutlanding = true;
        WayPointCalc[i].WpType = WPT_OUTLANDING;
        DisableBestAlternate = false;
    } else {
        WayPointCalc[i].IsAirport = false;
        WayPointCalc[i].IsLandable = false;
        WayPointCalc[i].IsOutlanding = false;
        WayPointCalc[i].WpType = WPT_TURNPOINT;
    }
    for (short j = 0; j < ALTA_SIZE; j++) {
        WayPointCalc[i].AltArriv[j] = -1;
        WayPointCalc[i].AltReqd[j] = -1;
    }
}

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
        MessageBoxX(MsgToken<486>(), // "Not Enough Memory For Waypoints"
                MsgToken<266>() /* "Error" */, mbOk);
        return false;
    }

    try {
        WayPointCalc.push_back(WPCALC{});
        InitOneWayPointCalc(WayPointCalc.size() - 1);
    } catch (std::exception& e) {
        const tstring what = to_tstring(e.what());
        StartupStore(_T("FAILED! <%s>" NEWLINE), what.c_str());
        MessageBoxX(MsgToken<486>(), // "Not Enough Memory For Waypoints"
                MsgToken<266>() /* "Error" */, mbOk);
        WayPointList.pop_back();
        return false;
    }
    return true;
}

bool AddWaypointBulk(WAYPOINT& Waypoint) {
    try {
        WayPointList.push_back(Waypoint);
        Waypoint = {};
    } catch (std::exception& e) {
        const tstring what = to_tstring(e.what());
        StartupStore(_T("FAILED! <%s>" NEWLINE), what.c_str());
        MessageBoxX(MsgToken<486>(), MsgToken<266>(), mbOk);
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
