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


// VENTA3 added additional WP calculated list
bool AllocateWaypointList(void) {
    if (WayPointList.empty()) {
#if TESTBENCH
        StartupStore(_T(". AllocateWaypointList: "));
#endif
        try {
            WayPointList.reserve(50);
            WayPointList.resize(NUMRESWP);

        } catch (std::exception& e) {
            StartupStore(_T("FAILED! <%s>%s"), e.what(), NEWLINE);
            MessageBoxX(gettext(TEXT("_@M486_")), // "Not Enough Memory For Waypoints"
                        gettext(TEXT("_@M266_")) /* "Error" */,mbOk);
            return false;
        }

#if TESTBENCH
        StartupStore(_T("OK%s"),NEWLINE);
        StartupStore(_T(". AllocateWayPointCalc..."));
#endif
        try {
            WayPointCalc.reserve(50);
            WayPointCalc.resize(NUMRESWP);
        } catch (std::exception& e) {
            StartupStore(_T("FAILED! <%s>%s"), e.what(), NEWLINE);
            MessageBoxX(gettext(TEXT("_@M486_")), // "Not Enough Memory For Waypoints"
                        gettext(TEXT("_@M266_")) /* "Error" */,mbOk);
            return false;
        }
#if TESTBENCH
        StartupStore(_T("OK%s"),NEWLINE);
#endif
    }
    return true;
}
