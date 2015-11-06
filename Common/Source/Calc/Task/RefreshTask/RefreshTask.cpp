/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Calculations2.h"


void RefreshTask(void) {
  double lengthtotal = 0.0;
  int i;

  LockTaskData();
  if ((ActiveTaskPoint<0)&&(Task[0].Index>=0)) {
    ActiveTaskPoint=0;
  }

  // Only need to refresh info where the removal happened
  // as the order of other taskpoints hasn't changed
  for (i=0; i<MAXTASKPOINTS; i++) {
    if (!ValidTaskPoint(i)) {
      Task[i].Index = -1;
    } else {
      RefreshTaskWaypoint(i);
      lengthtotal += Task[i].Leg;
    }
  }
  if (lengthtotal>0) {
    for (i=0; i<MAXTASKPOINTS; i++) {
      if (ValidTaskPoint(i)) {
	RefreshTaskWaypoint(i);
	TaskStats[i].LengthPercent = Task[i].Leg/lengthtotal;
	if (!ValidTaskPoint(i+1)) {
          // this is the finish waypoint
      Task[i].AATTargetOffsetRadius = 0.0;
	  Task[i].AATTargetOffsetRadial = 0.0;
	  Task[i].AATTargetLat = WayPointList[Task[i].Index].Latitude;
	  Task[i].AATTargetLon = WayPointList[Task[i].Index].Longitude;
	}
      }
    }
  }

  // Determine if a waypoint is in the task
    for (i=NUMRESWP; i< (int)WayPointList.size(); i++) { // maybe paragliders takeoff is set as home
      WayPointList[i].InTask = false;
      if ((WayPointList[i].Flags & HOME) == HOME) {
        WayPointList[i].InTask = true;
      }
    }
    if (HomeWaypoint>=0) {
      WayPointList[HomeWaypoint].InTask = true;
    }
    for (i=0; i<MAXTASKPOINTS; i++) {
      if (ValidTaskPoint(i)) {
        WayPointList[Task[i].Index].InTask = true;
      }
    }
    if (EnableMultipleStartPoints) {
      for (i=0; i<MAXSTARTPOINTS; i++) {
        if (ValidWayPoint(StartPoints[i].Index) && StartPoints[i].Active) {
          WayPointList[StartPoints[i].Index].InTask = true;
        }
      }
    }

  CalculateTaskSectors();
  CalculateAATTaskSectors();
  UnlockTaskData();

  ClearOptimizedTargetPos();
}

