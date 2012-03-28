/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


void ClearTask(void) {

  LockTaskData();
  TaskModified = true; 
  TargetModified = true;
  LastTaskFileName[0] = _T('\0');
  ActiveWayPoint = -1;
  int i;
  for(i=0;i<MAXTASKPOINTS;i++) {
    Task[i].Index = -1;
    Task[i].AATSectorRadius = SectorRadius; 
    Task[i].AATCircleRadius = SectorRadius;
    Task[i].AATTargetOffsetRadial = 0;
    if (DoOptimizeRoute())
      Task[i].AATTargetOffsetRadius = -100;
    else
      Task[i].AATTargetOffsetRadius = 0;
    Task[i].AATTargetLocked = false;
    for (int j=0; j<MAXISOLINES; j++) {
      TaskStats[i].IsoLine_valid[j] = false;
    }
  }
  for (i=0; i<MAXSTARTPOINTS; i++) {
    StartPoints[i].Index = -1;
  }
  UnlockTaskData();
}

