/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "AATDistance.h"
#include "CalcTask.h"



// This is called from main DoCalculations each time, only when running a real task
void InSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static int LastStartSector = -1;

  if (ActiveTaskPoint<0) return;
  LockTaskData();

// Paragliders task system
// Case A: start entering the sector/cylinder
//		you must be outside sector when gate is open.
//		you are warned that you are already inside sector before the gate is open, when gate is opening in <10 minutes
//		task restart is manual
// Case B: start exiting the sector

  // by default, we are not in the sector
  Calculated->IsInSector = false;

  if(ActiveTaskPoint == 0) {
	CheckStart(Basic, Calculated, &LastStartSector);
  } else {
	if(IsFinalWaypoint()) {
		LastStartSector = -1;
		AddAATPoint(Basic, Calculated, ActiveTaskPoint-1);
		CheckFinish(Basic, Calculated);
	} else {
		if (!UseGates()) CheckRestart(Basic, Calculated, &LastStartSector); // 100507
		if (ActiveTaskPoint>0) {
			CheckInSector(Basic, Calculated);
			LastStartSector = -1;
		}
	}
  }                   
  UnlockTaskData();
}

