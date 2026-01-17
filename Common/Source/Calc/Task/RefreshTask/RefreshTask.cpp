/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Calculations2.h"
#include "NavFunctions.h"

BOOL CheckFAILeg(double leg, double total)
{
BOOL fai = true;
if(total > 0.0)
{
  double lrat = leg/total;
  if(total < FAI28_45Threshold)
  {
    if(lrat <= FAI_NORMAL_PERCENTAGE)  fai = false;
  }
  else
  {
    if(lrat <= FAI_BIG_PERCENTAGE)     fai = false;
    if(lrat >= FAI_BIG_MAX_PERCENTAGE) fai = false;
  }
} else fai = false;
return fai;
}


BOOL IsFAI_Task(void)
{
BOOL fai = true;
double Leg, Bear;
int i, from=1,to, TaskPoints =0;
double TaskTotalDistance =0;
CALCULATED_INFO.TaskFAI = false;
//if(AATEnabled)
	//return false;
while(ValidTaskPoint(TaskPoints))
{
  TaskTotalDistance += Task[TaskPoints].Leg;
  TaskPoints++;
}

if(TaskPoints < 2)
  fai = false;
if(TaskPoints > 5)
  fai = false;
CALCULATED_INFO.TaskTotalDistance = TaskTotalDistance;
CALCULATED_INFO.TaskFAIDistance   = TaskTotalDistance;
if(TaskPoints == 5)
{
  if(Task[0].Index != Task[4].Index)  /* closed task ? */
	fai = false;
  else
  {
    from = 2;
    to = 4;

    double lat1 = WayPointList[Task[1].Index].Latitude;  //calculate the resulting triangle leg
    double lon1 = WayPointList[Task[1].Index].Longitude;
    double lat2 = WayPointList[Task[3].Index].Latitude;
    double lon2 = WayPointList[Task[3].Index].Longitude;
    DistanceBearing(lat1, lon1,  lat2,  lon2, &Leg, &Bear);

    TaskTotalDistance -= Task[1].Leg;  // remove the two short legs to starting point
    TaskTotalDistance -= Task[4].Leg;
    TaskTotalDistance += Leg;          // add the direct leg instead
    CALCULATED_INFO.TaskFAIDistance   = TaskTotalDistance;
    fai = CheckFAILeg( Leg , TaskTotalDistance);
  }
}
else
{
  if(Task[0].Index != Task[3].Index)  /* closed task ? */
	fai = false;
  from = 1;
  to = TaskPoints;
}


if(fai)
{
  if ((TaskTotalDistance > 0))
  {
    for (i=from; i<to; i++) {
	  if (fai)
      {
	    fai = CheckFAILeg( Task[i].Leg, TaskTotalDistance);
      }
    }
   } else {
    fai = false;
  }
}
CALCULATED_INFO.TaskFAI = fai;
return fai;
}

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
    if (ValidWayPointFast(HomeWaypoint)) {
      WayPointList[HomeWaypoint].InTask = true;
    }
    for (i=0; i<MAXTASKPOINTS; i++) {
      if (ValidTaskPoint(i)) {
        WayPointList[Task[i].Index].InTask = true;
      }
    }
    if (EnableMultipleStartPoints()) {
      for (i=0; i<MAXSTARTPOINTS; i++) {
        if (ValidWayPoint(StartPoints[i].Index) && StartPoints[i].Active) {
          WayPointList[StartPoints[i].Index].InTask = true;
        }
      }
    }

  CalculateTaskSectors();
  CalculateAATTaskSectors();
  UnlockTaskData();
  IsFAI_Task();
  ClearOptimizedTargetPos();
}
