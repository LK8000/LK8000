/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

int WaypointOutOfTerrainRangeDontAskAgain = -1;

void CloseWayPoints() {

  ClearTask();

  // tips : this is same as clear() but force to free allocated memory...
  WayPointList = std::vector<WAYPOINT>();
  WayPointCalc = std::vector<WPCALC>();

  WaypointOutOfTerrainRangeDontAskAgain = WaypointsOutOfRange;
}
