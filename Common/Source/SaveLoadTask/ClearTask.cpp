/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "utils/stl_utils.h"
#include "Waypoints/SetHome.h"

extern void ResetTaskWpt(TASK_POINT& TaskWpt);
extern void ResetTaskStat(TASKSTATS_POINT& StatPt);

namespace {

inline void ResetStartPoint(START_POINT& StartPt) {
  StartPt.Index = -1;
}

} // namespace

void ClearTask() {
  ScopeLock lock(CritSec_TaskData);

  EnableFAIFinishHeight = false;
  FinishMinHeight = 0;

  StartHeightRef = 0;
  StartMaxHeight = 0;
  StartMaxHeightMargin = 0;
  StartMaxSpeed = 0;
  StartMaxSpeedMargin = 0;

  TaskModified = true;
  TargetModified = true;
  TskOptimizeRoute = TskOptimizeRoute_Config;
  LastTaskFileName[0] = _T('\0');
  ActiveTaskPoint = -1;

  EnableMultipleStartPoints = false;

  std::for_each(std::begin(Task), std::end(Task), ResetTaskWpt);
  std::for_each(std::begin(TaskStats), std::end(TaskStats), ResetTaskStat);
  std::for_each(std::begin(StartPoints), std::end(StartPoints), ResetStartPoint);
}
