/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "NavFunctions.h"

namespace {

void AATStats_Time(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // Task time to go calculations

  double aat_tasktime_elapsed = Basic->Time - Calculated->TaskStartTime;
  double aat_tasklength_seconds = AATTaskLength * 60;

  if (ActiveTaskPoint == 0) {
    // BUG fixed in dlgTaskWaypoint: changing AATTaskLength had no effect until
    // restart because AATTimeToGo was reset only once.
    if (Calculated->AATTimeToGo == 0) {
      Calculated->AATTimeToGo = aat_tasklength_seconds;
    }
  }
  else if (aat_tasktime_elapsed >= 0) {
    Calculated->AATTimeToGo =
        std::max(0.0, aat_tasklength_seconds - aat_tasktime_elapsed);
  }

  if(ValidTaskPointFast(ActiveTaskPoint) && (Calculated->AATTimeToGo>0)) {
    Calculated->AATMaxSpeed =
        Calculated->AATMaxDistance / Calculated->AATTimeToGo;
    Calculated->AATMinSpeed =
        Calculated->AATMinDistance / Calculated->AATTimeToGo;
    Calculated->AATTargetSpeed =
        Calculated->TaskDistanceToGo / Calculated->AATTimeToGo;
  }
}

void AATStats_Distance(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  // Calculate Task Distances

  if (!ValidTaskPointFast(ActiveTaskPoint)) {
    return;
  }

  double MaxDistance = 0;
  double MinDistance = 0;

  int i = ActiveTaskPoint;

  double LegToGo = 0;

  if (i > 0) {
    // RLD only include distance from glider to next leg if we've started the
    // task
    DistanceBearing(Basic->Latitude, Basic->Longitude,
                    WayPointList[Task[i].Index].Latitude,
                    WayPointList[Task[i].Index].Longitude,
                    &LegToGo, nullptr);

    if (Task[i].AATType == sector_type_t::CIRCLE) {
      MaxDistance =
          LegToGo +
          Task[i].AATCircleRadius;  // ToDo: should be adjusted for angle of
                                    // max target and for national rules
      MinDistance = LegToGo - Task[i].AATCircleRadius;
    }
    else {
      MaxDistance =
          LegToGo + (Task[i].AATSectorRadius);  // ToDo: should be adjusted for
                                                // angle of max target.
      MinDistance = LegToGo;
    }
  }

  i++;
  while (ValidTaskPointFast(i)) {
    double LegDistance;

    DistanceBearing(WayPointList[Task[i].Index].Latitude,
                    WayPointList[Task[i].Index].Longitude,
                    WayPointList[Task[i - 1].Index].Latitude,
                    WayPointList[Task[i - 1].Index].Longitude,
                    &LegDistance,
                    nullptr);

    MaxDistance += LegDistance;
    MinDistance += LegDistance;

    if (Task[i].AATType == sector_type_t::CIRCLE) {
      // breaking out single Areas increases accuracy for start
      // and finish

      // sector at start of (i)th leg
      if (i == 1) {  // first leg of task
        // add nothing
        MaxDistance -=
            StartRadius;  // e.g. Sports 2009 US Rules A116.3.2.  To Do: This
                          // should be configured multiple countries
        MinDistance -= StartRadius;
      }
      else {  // not first leg of task
        MaxDistance +=
            (Task[i - 1].AATCircleRadius);  // ToDo: should be adjusted for
                                            // angle of max target
        MinDistance -=
            (Task[i - 1].AATCircleRadius);  // ToDo: should be adjusted for
                                            // angle of max target
      }

      // sector at end of ith leg
      if (!ValidTaskPointFast(i + 1)) {  // last leg of task
        // add nothing
        MaxDistance -=
            FinishRadius;  // To Do: This can be configured for finish rules
        MinDistance -= FinishRadius;
      }
      else {                                       // not last leg of task
        MaxDistance += (Task[i].AATCircleRadius);  // ToDo: should be adjusted
                                                   // for angle of max target
        MinDistance -= (Task[i].AATCircleRadius);  // ToDo: should be adjusted
                                                   // for angle of max target
      }
    }
    else {  // not circle (pie slice)
      // sector at start of (i)th leg
      if (i == 1) {  // first leg of task
        // add nothing
        MaxDistance += 0;  // To Do: This can be configured for start rules
      }
      else {  // not first leg of task
        MaxDistance +=
            (Task[i - 1].AATSectorRadius);  // ToDo: should be adjusted for
                                            // angle of max target
      }

      // sector at end of ith leg
      if (!ValidTaskPointFast(i + 1)) {  // last leg of task
        // add nothing
        MaxDistance += 0;  // To Do: This can be configured for finish rules
      }
      else {                                       // not last leg of task
        MaxDistance += (Task[i].AATSectorRadius);  // ToDo: should be adjusted
                                                   // for angle of max target
      }
    }
    i++;
  }

  // JMW TODO accuracy: make these calculations more accurate, because
  // currently they are very approximate.

  Calculated->AATMaxDistance = MaxDistance;
  Calculated->AATMinDistance = MinDistance;
}

} // namespace

void AATStats(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  if (Calculated->ValidFinish) {
    return;
  }

  const std::lock_guard lock(CritSec_TaskData);

  if (WayPointList.empty()) {
    return;
  }
  if (gTaskType != task_type_t::AAT) {
    return;
  }

  AATStats_Distance(Basic, Calculated);
  AATStats_Time(Basic, Calculated);
}
