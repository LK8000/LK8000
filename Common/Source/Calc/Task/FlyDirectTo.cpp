/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"

void FlyDirectTo(int index) {
  if (!CheckDeclaration())
    return;

  LockTaskData();

  TaskModified = true;
  TargetModified = true;
  ActiveWayPoint = -1; 

  AATEnabled = FALSE;

  InsertRecentList(index);

  Task[0].Index = index;
  for (int i=1; i<=MAXTASKPOINTS; i++) {
    Task[i].Index = -1;
  }
  ActiveWayPoint = 0;
  RefreshTask();
  UnlockTaskData();
}

