/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "AATDistance.h"
#include "CalcTask.h"
#include "NavFunctions.h"

extern AATDistance aatdistance;

namespace {

void AdvanceToNext(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  if (ReadyToAdvance(Calculated, true, false)) {
    AnnounceWayPointSwitch(Calculated, true);

    Calculated->LegStartTime = Basic->Time;
    flightstats.LegStartTime[ActiveTaskPoint] = Basic->Time;

    // Update 'IsInSector' is required because ActiveTaskPoint has changed !
    Calculated->IsInSector = InTurnSector(Basic, ActiveTaskPoint);
  }
}

bool IsCircle(const size_t& idx) {
  assert(gTaskType == TSK_GP);
  ScopeLock lock(CritSec_FlightData);
  if (idx > 0 && ValidTaskPointFast(idx + 1)) { 
    // Not Start or Finish
    switch (Task[idx].AATType) {
      case sector_type_t::CIRCLE:
      case sector_type_t::CONE:
      case sector_type_t::ESS_CIRCLE:
        return true;
      default:
        return false;
    }
  }
  return false;
}

}  // namespace


void CheckInSector(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  if (ActiveTaskPoint > 0) {
    AddAATPoint(Basic, Calculated, ActiveTaskPoint - 1);
  }
  AddAATPoint(Basic, Calculated, ActiveTaskPoint);

  if (gTaskType == TSK_GP) {
    if (IsCircle(ActiveTaskPoint)) {
      static bool last_isInSector = Calculated->IsInSector;
      if (last_isInSector != Calculated->IsInSector) {
        AdvanceToNext(Basic, Calculated);
        last_isInSector = Calculated->IsInSector;
      }
      return;
    }
  }

  if (aatdistance.HasEntered(ActiveTaskPoint)) {
    AdvanceToNext(Basic, Calculated);
    if (Calculated->Flying) {
      Calculated->ValidFinish = false;
    }
  }
}
