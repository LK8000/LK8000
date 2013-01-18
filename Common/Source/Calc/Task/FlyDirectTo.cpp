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

  AATEnabled = FALSE;

  InsertRecentList(index);

  ClearTask();
  Task[0].Index = index;
  ActiveWayPoint = 0;
  RefreshTask();
  UnlockTaskData();
}

