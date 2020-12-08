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
  TskOptimizeRoute = TskOptimizeRoute_Config;
  LastTaskFileName[0] = _T('\0');
  ActiveTaskPoint = -1;

  EnableMultipleStartPoints = false;

  std::for_each(std::begin(Task), std::end(Task), ResetTaskWpt);
  std::for_each(std::begin(TaskStats), std::end(TaskStats), ResetTaskStat);
  std::for_each(std::begin(StartPoints), std::end(StartPoints), ResetStartPoint);

  UnlockTaskData();
}
