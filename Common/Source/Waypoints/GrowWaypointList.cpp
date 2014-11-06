/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "Waypointparser.h"
#include "Dialogs.h"

WAYPOINT* GrowWaypointList() {

#if TESTBENCH
    StartupStore(_T(". AllocateWaypointList: "));
#endif
    try {
        if(WayPointList.capacity() <= WayPointList.size()) {
            // always reserve more than need, it's required By Waypoint loading algorithm
            WayPointList.reserve(WayPointList.size()+10);
        }
        WayPointList.resize(WayPointList.size());
    } catch (std::exception& e) {
        StartupStore(_T("FAILED! <%s>%s"), e.what(), NEWLINE);
        MessageBoxX(gettext(TEXT("_@M486_")), // "Not Enough Memory For Waypoints"
                gettext(TEXT("_@M266_")) /* "Error" */, mbOk);
        return NULL;
    }

#if TESTBENCH
    StartupStore(_T("OK%s"), NEWLINE);
    StartupStore(_T(". AllocateWayPointCalc..."));
#endif
    try {
        WayPointList.reserve(WayPointList.size()+1);
        WayPointCalc.resize(WayPointList.size());
    } catch (std::exception& e) {
        StartupStore(_T("FAILED! <%s>%s"), e.what(), NEWLINE);
        MessageBoxX(gettext(TEXT("_@M486_")), // "Not Enough Memory For Waypoints"
                gettext(TEXT("_@M266_")) /* "Error" */, mbOk);
        return NULL;
    }
#if TESTBENCH
    StartupStore(_T("OK%s"), NEWLINE);
#endif
    
    // returns the newly created waypoint
    return &WayPointList.back();
}
