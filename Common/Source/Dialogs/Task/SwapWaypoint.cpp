/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"

// Swaps waypoint at current index with next one.
//  `CritSec_TaskData` must be locked by caller
void SwapWaypoint(int index) {
  if (!CheckDeclaration()) {
    return;
  }

  if (!ValidTaskPointFast(index)) {
    return;
  }
  if (!ValidTaskPointFast(index + 1)) {
    return;
  }

  TaskModified = true;
  TargetModified = true;
  std::swap(Task[index], Task[index + 1]);

  RefreshTask();
}
