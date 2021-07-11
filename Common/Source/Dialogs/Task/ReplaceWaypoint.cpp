/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
  int waypoint = ActiveTaskPoint;
  if(  ValidTaskPoint(PanTaskEdit))
	  waypoint = PanTaskEdit;

  TaskModified = true;
  TargetModified = true;

  // ARH 26/06/05 Fixed array out-of-bounds bug
  if (waypoint>=0 && waypoint < MAXTASKPOINTS) {
//   ResetTaskWaypoint(ActiveTaskPoint);
    Task[waypoint].Index = index;
  } else {
	// 130206 this is no more called, normally, because we filter out this condition
	// in dlgWayPointDetails..

    // Insert a new waypoint since there's
    // nothing to replace
    ActiveTaskPoint=0;
    ResetTaskWaypoint(waypoint);
    Task[waypoint].Index = index;
  }
  RefreshTask();
  UnlockTaskData();
}
