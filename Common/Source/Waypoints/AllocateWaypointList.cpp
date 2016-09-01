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
        Waypoint = {0};

    } catch (std::exception& e) {
        StartupStore(_T("FAILED! <%s>%s"), e.what(), NEWLINE);
        MessageBoxX(MsgToken(486), // "Not Enough Memory For Waypoints"
                MsgToken(266) /* "Error" */, mbOk);
        return false;
    }

    try {
        WayPointCalc.resize(WayPointList.size());
    } catch (std::exception& e) {
        StartupStore(_T("FAILED! <%s>%s"), e.what(), NEWLINE);
        MessageBoxX(MsgToken(486), // "Not Enough Memory For Waypoints"
                MsgToken(266) /* "Error" */, mbOk);
        return false;
    }
    return true;
}
