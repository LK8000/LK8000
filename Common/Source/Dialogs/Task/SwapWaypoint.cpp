/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"


// Swaps waypoint at current index with next one.
void SwapWaypoint(int index) {
  if (!CheckDeclaration())
    return;

  LockTaskData();
  TaskModified = true;
  TargetModified = true;
  if (index<0) {
    return;
  }
  if (index+1>= MAXTASKPOINTS-1) {
    return;
  }
  if ((Task[index].Index != -1)&&(Task[index+1].Index != -1)) {
    TASK_POINT tmpPoint;
    tmpPoint = Task[index];
    Task[index] = Task[index+1];
    Task[index+1] = tmpPoint;
  }
  RefreshTask();
  UnlockTaskData();
}
