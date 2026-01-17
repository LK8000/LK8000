/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"


void RotateStartPoints(void) {
  if (ActiveTaskPoint>0) return;
  if (!EnableMultipleStartPoints()) return;

  LockTaskData();

  int found = -1;
  int imax = 0;
  for (int i=0; i<MAXSTARTPOINTS; i++) {
    if (StartPoints[i].Active && ValidWayPoint(StartPoints[i].Index)) {
      if (Task[0].Index == StartPoints[i].Index) {
        found = i;
      }
      imax = i;
    }
  }
  found++;
  if (found>imax) {
    found = 0;
  }
  if (ValidWayPoint(StartPoints[found].Index)) {
    Task[0].Index = StartPoints[found].Index;
  }

  RefreshTask();
  UnlockTaskData();
}
