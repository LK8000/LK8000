/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "Dialogs.h"




// Index specifies a waypoint in the WP list
// It won't necessarily be a waypoint that's
// in the task
void RemoveWaypoint(int index) {
  int i;

  if (!CheckDeclaration())
    return;

  if (ActiveTaskPoint<0) {
    return; // No waypoint to remove
  }

  // Check to see whether selected WP is actually
  // in the task list.
  // If not, we'll ask the user if they want to remove
  // the currently active task point.
  // If the WP is in the task multiple times then we'll
  // remove the first instance after (or including) the
  // active WP.
  // If they're all before the active WP then just remove
  // the nearest to the active WP

  LockTaskData();
  TaskModified = true;
  TargetModified = true;

  // Search forward first
  i = ActiveTaskPoint;
  while ((i < MAXTASKPOINTS) && (Task[i].Index != index)) {
    ++i;
  }

  if (i < MAXTASKPOINTS) {
    // Found WP, so remove it
    RemoveTaskPoint(i);

    if (Task[ActiveTaskPoint].Index == -1) {
      // We've just removed the last task point and it was
      // active at the time
      ActiveTaskPoint--;
    }

  } else {
    // Didn't find WP, so search backwards

    i = ActiveTaskPoint;
    do {
      --i;
    } while (i >= 0 && Task[i].Index != index);

    if (i >= 0) {
      // Found WP, so remove it
      RemoveTaskPoint(i);
      ActiveTaskPoint--;

    } else {
      // WP not found, so ask user if they want to
      // remove the active WP
      UnlockTaskData();

	TCHAR tlkbuf[100];
	_stprintf(tlkbuf,_T("%s\n%s"),
	// LKTOKEN  _@M169_ = "Chosen Waypoint not in current task."
        MsgToken<169>(),
	// LKTOKEN  _@M551_ = "Remove active Waypoint?"
	MsgToken<551>());

      int ret = MessageBoxX(
        tlkbuf,
	// LKTOKEN  _@M550_ = "Remove Waypoint"
        MsgToken<550>(),
        mbYesNo);
      LockTaskData();

      if (ret == IdYes) {
        RemoveTaskPoint(ActiveTaskPoint);
        if (Task[ActiveTaskPoint].Index == -1) {
          // Active WayPoint was last in the list so is currently
          // invalid.
          ActiveTaskPoint--;
        }
      }
    }
  }
  RefreshTask();
  UnlockTaskData();

}
