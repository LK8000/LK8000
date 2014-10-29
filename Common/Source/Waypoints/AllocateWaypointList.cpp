/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "Dialogs.h"



// VENTA3 added additional WP calculated list
bool AllocateWaypointList(void) {
  if (!WayPointList) {
    NumberOfWayPoints = 0;
    #if TESTBENCH
    StartupStore(_T(". AllocateWaypointList: "));
    #endif
    WayPointList = (WAYPOINT *)LocalAlloc(LPTR, 50 * sizeof(WAYPOINT));
    if(WayPointList == NULL) 
      {
	StartupStore(_T("FAILED!%s"),NEWLINE);
        MessageBoxX(
	// LKTOKEN  _@M486_ = "Not Enough Memory For Waypoints" 
                    gettext(TEXT("_@M486_")),
	// LKTOKEN  _@M266_ = "Error" 
                    gettext(TEXT("_@M266_")),MB_OK|MB_ICONSTOP);
        return 0;
      }
    #if TESTBENCH
    StartupStore(_T("OK%s"),NEWLINE);
    StartupStore(_T(". AllocateWayPointCalc..."));
    #endif

    WayPointCalc = (WPCALC *)LocalAlloc(LPTR, 50 * sizeof(WPCALC));
    if(WayPointCalc == NULL) 
      {
	StartupStore(_T("FAILED!%s"),NEWLINE);
        MessageBoxX(
	// LKTOKEN  _@M486_ = "Not Enough Memory For Waypoints" 
                    gettext(TEXT("_@M486_")),
	// LKTOKEN  _@M266_ = "Error" 
                    gettext(TEXT("_@M266_")),MB_OK|MB_ICONSTOP);
        return 0;
      }
    #if TESTBENCH
    StartupStore(_T("OK%s"),NEWLINE);
    #endif
    return true;
  }
  return true;
}
