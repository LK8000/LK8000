/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"

extern void ResetTaskWaypoint(int j);

// RemoveTaskpoint removes a single waypoint
// from the current task.  index specifies an entry
// in the Task[] array - NOT a waypoint index.
//
// If you call this function, you MUST deal with
// correctly setting ActiveTaskPoint yourself!
void RemoveTaskPoint(int index) {
  if (!CheckDeclaration())
    return;

  int i;

  if (index < 0 || index >= MAXTASKPOINTS) {
    return; // index out of bounds
  }

  LockTaskData();
  TaskModified = true;
  TargetModified = true;

  if (Task[index].Index == -1) {
    UnlockTaskData();
    return; // There's no WP at this location
  }

  // Shuffle all later taskpoints to the left to
  // fill the gap
  for (i=index; i<MAXTASKPOINTS-1; ++i) {
    Task[i] = Task[i+1];
  }
  ResetTaskWaypoint(MAXTASKPOINTS-1);

  RefreshTask();
  UnlockTaskData();

}
