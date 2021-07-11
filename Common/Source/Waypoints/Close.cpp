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

  for(WAYPOINT &wp :WayPointList) {
    if(wp.Details) {
      free(wp.Details);
      wp.Details = nullptr;
    }
    if(wp.Comment) {
      free(wp.Comment);
      wp.Comment = nullptr;
    }
  }

  // tips : this is same as clear() but force to free allocated memory...
  WayPointList = std::vector<WAYPOINT>();
  WayPointCalc = std::vector<WPCALC>();

  WaypointOutOfTerrainRangeDontAskAgain = WaypointsOutOfRange;
}
