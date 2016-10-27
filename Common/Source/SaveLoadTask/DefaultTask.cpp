/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


// Create a default task to home at startup if no task is present
void DefaultTask(void) {
  LockTaskData();
  TaskModified = true;
  TargetModified = true;
  if ((Task[0].Index == -1)||(ActiveTaskPoint==-1)) {

	if (HomeWaypoint >= 0) {
		if ( HomeWaypoint == 0 )  // 091213
			StartupStore(_T("... DefaultTask assigning TAKEOFF as default destination%s"),NEWLINE);
		else
		StartupStore(_T(". DefaultTask assigning Home (wp=%d) as default destination%s"),HomeWaypoint,NEWLINE); // 091213
		Task[0].Index = HomeWaypoint;
		ActiveTaskPoint = 0;
	} else
		StartupStore(_T(". DefaultTask: no task, no active waypoint and no Home%s"),NEWLINE); // 091112
  }
  RefreshTask();
  UnlockTaskData();
}
