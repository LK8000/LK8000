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
  // memory allocation
  if (!AllocateWaypointList()) {
    return 0;
  }

  if (((NumberOfWayPoints+1) % 50) == 0) {
    WAYPOINT *p;
    
    if ((p = 
         (WAYPOINT *)LocalReAlloc(WayPointList, 
                                  (((NumberOfWayPoints+1)/50)+1) 
                                  * 50 * sizeof(WAYPOINT), 
                                  LMEM_MOVEABLE | LMEM_ZEROINIT)) == NULL){
      
	StartupStore(_T("+++ GrowWaypointList FAILED!%s"),NEWLINE);
      MessageBoxX(
	// LKTOKEN  _@M486_ = "Not Enough Memory For Waypoints" 
                  gettext(TEXT("_@M486_")),
	// LKTOKEN  _@M266_ = "Error" 
                  gettext(TEXT("_@M266_")),MB_OK|MB_ICONSTOP);
      
      return 0; // failed to allocate
    }
    
    if (p != WayPointList){
      WayPointList = p;      
    }

    WPCALC *q;
    
    if ((q = 
         (WPCALC *)LocalReAlloc(WayPointCalc, 
                                  (((NumberOfWayPoints+1)/50)+1) 
                                  * 50 * sizeof(WPCALC), 
                                  LMEM_MOVEABLE | LMEM_ZEROINIT)) == NULL){
      
	StartupStore(_T("+++ GrowWaypointCalc FAILED!%s"),NEWLINE);
      MessageBoxX(
	// LKTOKEN  _@M486_ = "Not Enough Memory For Waypoints" 
                  gettext(TEXT("_@M486_")),
	// LKTOKEN  _@M266_ = "Error" 
                  gettext(TEXT("_@M266_")),MB_OK|MB_ICONSTOP);
      
      return 0; // failed to allocate
    }
    
    if (q != WayPointCalc){
      WayPointCalc = q;      
    }
  }

  NumberOfWayPoints++;
  return WayPointList + NumberOfWayPoints-1;
  // returns the newly created waypoint
}
