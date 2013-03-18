/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "utils/stl_utils.h"

extern void ResetTaskWpt(TASK_POINT& TaskWpt);
extern void ResetTaskStat(TASKSTATS_POINT& StatPt);
extern void ResetStartPoint(START_POINT& StartPt);

void ClearTask(void) {

  LockTaskData();
  TaskModified = true; 
  TargetModified = true;
  LastTaskFileName[0] = _T('\0');
  ActiveWayPoint = -1;

  PGNumberOfGates = 0;
  EnableMultipleStartPoints = false;

  std::for_each(begin(Task), end(Task), ResetTaskWpt);
  std::for_each(begin(TaskStats), end(TaskStats), ResetTaskStat);
  std::for_each(begin(StartPoints), end(StartPoints), ResetStartPoint);

  UnlockTaskData();
}

