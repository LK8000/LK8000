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

    // 2 loop, It's not optimized, but it's not a problem here.
    // we can avoid that 2 loop using std::string for Detail & Comment WAYPOINT members
    std::for_each(WayPointList.begin(), WayPointList.end(), std::bind(&free, std::bind(&WAYPOINT::Details, _1)));
    std::for_each(WayPointList.begin(), WayPointList.end(), std::bind(&free, std::bind(&WAYPOINT::Comment, _1)));
  }
  WayPointList.clear(); // we must force realloc also for RESWPs
  WayPointCalc.clear();

  WaypointOutOfTerrainRangeDontAskAgain = WaypointsOutOfRange;
}
