/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


// Create a default task to home at startup if no task is present
void DefaultTask(void) {
  LockTaskData();
  if ((Task[0].Index == -1)||(ActiveTaskPoint==-1)) {
     if (HomeWaypoint >= RESWP_FIRST_MARKER) {
        StartupStore(_T(". DefaultTask assigning Home (wp=%d) as default destination"),HomeWaypoint);
        FlyDirectTo(HomeWaypoint);
        TaskModified = true;
        TargetModified = true;
        RefreshTask();
     } else {
       StartupStore(_T(". DefaultTask: no task, no active waypoint and no Home")); // 091112
     }
  }
  UnlockTaskData();
}
