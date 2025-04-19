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
#include "task_zone.h"


namespace {

void AdvanceToNext(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  if (ReadyToAdvance(Calculated, true, false)) {
    AnnounceWayPointSwitch(Calculated, true);

    Calculated->LegStartTime = Basic->Time;
    flightstats.LegStartTime[ActiveTaskPoint] = Basic->Time;

    // Update 'IsInSector' is required because ActiveTaskPoint has changed !
    Calculated->IsInSector = InTurnSector(Basic, ActiveTaskPoint);
  }

  if (Calculated->Flying) {
    Calculated->ValidFinish = false;
  }
}

template<sector_type_t type>
struct is_circle {
  constexpr static bool value = false;
};

template<>
struct is_circle<sector_type_t::CIRCLE> {
  constexpr static bool value = true;
};

template<>
struct is_circle<sector_type_t::ESS_CIRCLE> {
  constexpr static bool value = true;
};

struct is_circle_t {
  using result_type = bool;

  template<sector_type_t type, int task_type>
  static bool invoke(int tp_index) {
    return is_circle<type>::value;
  }
};

bool IsCircle(int tp_index) {
  return task::invoke_for_task_point<is_circle_t>(tp_index);
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

  if (gTaskType != TSK_AAT) {
    if (Calculated->IsInSector) {
      AdvanceToNext(Basic, Calculated);
      return;
    }
  }

  if (aatdistance.HasEntered(ActiveTaskPoint)) {
    AdvanceToNext(Basic, Calculated);
  }
}
