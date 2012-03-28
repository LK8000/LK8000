/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"

using std::min;
using std::max;

extern void ResetTaskWaypoint(int j);


// Inserts a waypoint into the task, in the
// position of the ActiveWaypoint.  If append=true, insert at end of the
// task.
void InsertWaypoint(int index, bool append) {
  if (!CheckDeclaration())
    return;

  int i;
  
  LockTaskData();
  TaskModified = true;
  TargetModified = true;

  if ((ActiveWayPoint<0) || !ValidTaskPoint(0)) {
    ActiveWayPoint = 0;
    ResetTaskWaypoint(ActiveWayPoint);
    Task[ActiveWayPoint].Index = index;

    UnlockTaskData();
    return;
  }
  
  if (ValidTaskPoint(MAXTASKPOINTS-1)) {
    // No room for any more task points!
    MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M727_ = "Too many waypoints in task!" 
      gettext(TEXT("_@M727_")),
	// LKTOKEN  _@M357_ = "Insert Waypoint" 
      gettext(TEXT("_@M357_")),
      MB_OK|MB_ICONEXCLAMATION);
    
    UnlockTaskData();
    return;
  }

  int indexInsert = max(ActiveWayPoint,0);
  if (append) {
	for (i=indexInsert; i<MAXTASKPOINTS-2; i++) {
		#if 100526
		if (Task[i+2].Index<0 && Task[i+1].Index>=0) {
			// i+1 is the last one, so we insert before the last one: shift i+1 to i+2, insert in i+1
			Task[i+2] = Task[i+1];
			ResetTaskWaypoint(i+1);
			Task[i+1].Index = index;
			break;
		}
		if (Task[i+1].Index<0) {
			// i+1 is empty, so the activewaypoint is the last one: we append after finish, because
			// pilot can use insert waypoint to make it BEFORE finish
			ResetTaskWaypoint(i+1);
			Task[i+1].Index = index;
			break;
		}
		#else
		if (Task[i+1].Index<0) {
			ResetTaskWaypoint(i+1);
			Task[i+1].Index = index;
			break;
		}
		#endif
	}  
  } else {
    // Shuffle ActiveWaypoint and all later task points
    // to the right by one position
    for (i=MAXTASKPOINTS-1; i>indexInsert; i--) {
      Task[i] = Task[i-1];
    }  
    // Insert new point and update task details
    ResetTaskWaypoint(indexInsert);
    Task[indexInsert].Index = index;
  }
  
  RefreshTask();
  UnlockTaskData();
  
}
