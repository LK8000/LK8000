/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"

extern void ResetTaskWaypoint(int j);


void ReplaceWaypoint(int index) {
  if (!CheckDeclaration())
    return;

  LockTaskData();
  TaskModified = true;
  TargetModified = true;

  // ARH 26/06/05 Fixed array out-of-bounds bug
  if (ActiveWayPoint>=0 && ActiveWayPoint < MAXTASKPOINTS) {	
    ResetTaskWaypoint(ActiveWayPoint);
    Task[ActiveWayPoint].Index = index;
  } else {
	// 130206 this is no more called, normally, because we filter out this condition
	// in dlgWayPointDetails..
    
    // Insert a new waypoint since there's
    // nothing to replace
    ActiveWayPoint=0;
    ResetTaskWaypoint(ActiveWayPoint);
    Task[ActiveWayPoint].Index = index;
  }
  RefreshTask();
  UnlockTaskData();
}

