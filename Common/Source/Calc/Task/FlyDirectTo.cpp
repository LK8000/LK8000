/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "Waypoints/SetHome.h"

void FlyDirectTo(int index) {
  if (!CheckDeclaration())
    return;

  LockTaskData();

  gTaskType = task_type_t::DEFAULT;

  InsertRecentList(index);

  ClearTask();
  SetHome(true);  // force home reload
  Task[0].Index = index;
  ActiveTaskPoint = 0;
  RefreshTask();
  UnlockTaskData();
}
