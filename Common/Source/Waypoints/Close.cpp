/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"



int WaypointOutOfTerrainRangeDontAskAgain = -1;


void CloseWayPoints() {
  #if TESTBENCH
  StartupStore(TEXT(". CloseWayPoints%s"),NEWLINE);
  #endif
  unsigned int i;
  if (NumberOfWayPoints>0) { // we must free also RESWps comments!
	#if TESTBENCH
	StartupStore(TEXT(". Waypoint list was not empty, closing.%s"),NEWLINE);
	#endif
	for (i=0; i<NumberOfWayPoints; i++) {
		if (WayPointList[i].Details) {
			free(WayPointList[i].Details);
			WayPointList[i].Details = NULL;
		}
		if (WayPointList[i].Comment) {
			free(WayPointList[i].Comment);
			WayPointList[i].Comment = NULL;
		}
	}
  }
  NumberOfWayPoints = 0; // we must force realloc also for RESWPs
 
  // here we should not have any memory allocated for wps including Reswp
  if(WayPointList != NULL) {
	#if TESTBENCH
	StartupStore(TEXT(". WayPointList not null, LocalFree on.%s"),NEWLINE);
	#endif
	LocalFree((HLOCAL)WayPointList);
	WayPointList = NULL;
	LocalFree((HLOCAL)WayPointCalc); // VENTA3
	WayPointCalc = NULL;
  }
  WaypointOutOfTerrainRangeDontAskAgain = WaypointsOutOfRange;
}
