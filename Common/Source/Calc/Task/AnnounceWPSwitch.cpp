/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InputEvents.h"
#include "AATDistance.h"
#include "CalcTask.h"


extern int FastLogNum; // number of points to log at high rate

void AnnounceWayPointSwitch(DERIVED_INFO *Calculated, bool do_advance) {
  if (ActiveWayPoint == 0) {
    InputEvents::processGlideComputer(GCE_TASK_START); // 101014

  } else if (Calculated->ValidFinish && IsFinalWaypoint()) {
    InputEvents::processGlideComputer(GCE_TASK_FINISH);
  } else {
    InputEvents::processGlideComputer(GCE_TASK_NEXTWAYPOINT);
  }

  if (do_advance) {
    LKASSERT(ValidTaskPoint(ActiveWayPoint+1));
    if (ValidTaskPoint(ActiveWayPoint+1)) ActiveWayPoint++;

  }

  SelectedWaypoint = TASKINDEX; 

  // set waypoint detail to active task WP

  // start logging data at faster rate
  FastLogNum = 5;
}


