/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "Dialogs.h"

extern void ResetTaskWaypoint(int j);


// Inserts a waypoint into the task, in the
// position of the ActiveWaypoint.  If append=true, insert at end of the
// task.
void InsertWaypoint(int index, unsigned short append) {
  if (!CheckDeclaration())
    return;

  int i;

  LockTaskData();
  TaskModified = true;
  TargetModified = true;

  if ((ActiveTaskPoint<0) || !ValidTaskPoint(0)) {
    ActiveTaskPoint = 0;
    ResetTaskWaypoint(ActiveTaskPoint);
    Task[ActiveTaskPoint].Index = index;

    UnlockTaskData();
    return;
  }

  if (ValidTaskPoint(MAXTASKPOINTS-1)) {
    // No room for any more task points!
    MessageBoxX(
	// LKTOKEN  _@M727_ = "Too many waypoints in task!"
      MsgToken(727),
	// LKTOKEN  _@M357_ = "Insert Waypoint"
      MsgToken(357),
      mbOk);

    UnlockTaskData();
    return;
  }

  int indexInsert = max(ActiveTaskPoint,0);

  switch(append) {
	// append 0 = insert in current position
	case 0:
		// Shift ActiveWaypoint and all later task points
		// to the right by one position
		for (i=MAXTASKPOINTS-1; i>indexInsert; i--) {
			Task[i] = Task[i-1];
		}
		// Insert new point and update task details
		ResetTaskWaypoint(indexInsert);
		Task[indexInsert].Index = index;

		break;

	// append 1 = add before finish
	case 1:
		for (i=indexInsert; i<MAXTASKPOINTS-2; i++) {
			if (Task[i+2].Index<0 && Task[i+1].Index>=0) {
				// i+1 is the last one, so we insert before the last one: shift i+1 to i+2, insert in i+1
				Task[i+2] = Task[i+1];
				ResetTaskWaypoint(i+1);
				Task[i+1].Index = index;
				break;
			}
			// special case, we started already from the last point. We insert.
			if (Task[i+1].Index<0) {
				ResetTaskWaypoint(i+1);
				Task[i+1]=Task[i];
				Task[i].Index = index;
				break;
			}
		}
		break;

	// append 2 = add after finish
	case 2:
		for (i=indexInsert; i<MAXTASKPOINTS-2; i++) {
			if (Task[i+1].Index<0) {
				ResetTaskWaypoint(i+1);
				Task[i+1].Index = index;
				break;
			}
		}
		break;

	default:
		BUGSTOP_LKASSERT(0);
		break;
  }

  RefreshTask();
  UnlockTaskData();

}
