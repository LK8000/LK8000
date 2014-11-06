/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include <functional>
using std::placeholders::_1;

int WaypointOutOfTerrainRangeDontAskAgain = -1;


void CloseWayPoints() {
  #if TESTBENCH
  StartupStore(TEXT(". CloseWayPoints%s"),NEWLINE);
  #endif
  if(!WayPointList.empty()) { // we must free also RESWps comments!
	#if TESTBENCH
	StartupStore(TEXT(". Waypoint list was not empty, closing.%s"),NEWLINE);
	#endif
    std::for_each(WayPointList.begin(), WayPointList.end(), std::bind(&free, std::bind(&WAYPOINT::Details, _1)));
    
	for (unsigned i=0; i<WayPointList.size(); ++i) {
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
  WayPointList.clear(); // we must force realloc also for RESWPs
  WayPointCalc.clear();

  WaypointOutOfTerrainRangeDontAskAgain = WaypointsOutOfRange;
}
