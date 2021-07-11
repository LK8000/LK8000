/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InputEvents.h"
#include "AATDistance.h"
#include "CalcTask.h"


void AnnounceWayPointSwitch(DERIVED_INFO *Calculated, bool do_advance) {
  if (ActiveTaskPoint == 0) {
    InputEvents::processGlideComputer(GCE_TASK_START); // 101014

  } else if (Calculated->ValidFinish && IsFinalWaypoint()) {
    InputEvents::processGlideComputer(GCE_TASK_FINISH);
  } else {
    InputEvents::processGlideComputer(GCE_TASK_NEXTWAYPOINT);
  }

  if (do_advance) {
    LKASSERT(ValidTaskPoint(ActiveTaskPoint+1));
    if (ValidTaskPoint(ActiveTaskPoint+1)) ActiveTaskPoint++;

  }

  SelectedWaypoint = TASKINDEX; 

  // set waypoint detail to active task WP

}


