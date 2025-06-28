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
#include "Sound/Sound.h"
#include "Calc/Task/TimeGates.h"

DERIVED_INFO Finish_Derived_Info;

// this is called only when ActiveTaskPoint is 0 (or 1 to check for restart), still waiting for start
void CheckStart(NMEA_INFO* Basic, DERIVED_INFO* Calculated, int* LastStartSector) {
  BOOL StartCrossed = false;

  NotifyGateState(Basic->Time);

  bool start_from_inside = ExitStart(*Calculated);
  Calculated->IsInSector = InStartSector(Basic, Calculated, !start_from_inside, *LastStartSector, &StartCrossed);

  // from inside and we are inside
  // or
  // from outside and we are outside
  if (start_from_inside == Calculated->IsInSector) {
    if (ReadyToStart(Basic, Calculated)) {
      aatdistance.AddPoint(GetCurrentPosition(*Basic), 0);
    }
    if (ValidStartSpeed(Basic, Calculated, StartMaxSpeedMargin)) {
      ReadyToAdvance(Calculated, false, true);
    }
  }

  if (StartCrossed && ValidGate(Basic->Time)) {  // 100509

    DebugLog(_T("... CheckStart: start crossed and valid gate!\n"));

    if (ISGAAIRCRAFT) {
      Calculated->ValidStart = true;
      ActiveTaskPoint = 0;  // enforce this since it may be 1
      StartTask(Basic, Calculated, true, false);
      return;
    }

    // ToLo: Check weather speed and height are within the rules or not (zero margin)
    if (!IsFinalWaypoint() && ValidStartSpeed(Basic, Calculated) && InsideStartHeight(Basic, Calculated)) {
      // This is set whether ready to advance or not, because it will
      // appear in the flight log, so if it's valid, it's valid.
      Calculated->ValidStart = true;

      if (ReadyToAdvance(Calculated, true, true)) {
        ActiveTaskPoint = 0;  // enforce this since it may be 1
        StartTask(Basic, Calculated, true, true);
      }
      if (Calculated->Flying) {
        Calculated->ValidFinish = false;
      }
      // JMW TODO accuracy: This causes Vaverage to go bonkers
      // if the user has already passed the start
      // but selects the start

      // Note: pilot must have armed advance
      // for the start to be registered

      // ToLo: If speed and height are outside the rules they must be within the margin...
    }
    else {
      if ((ActiveTaskPoint <= 1) && !IsFinalWaypoint() && (!Calculated->ValidStart) && (Calculated->Flying)) {
        StartTask(Basic, Calculated, false, false);
        if (ReadyToAdvance(Calculated, true, true)) {
          InputEvents::processGlideComputer(GCE_TASK_CONFIRMSTART);
        }

        if (Calculated->Flying) {
          Calculated->ValidFinish = false;
        }
      }
    }
  }
}

BOOL CheckRestart(NMEA_INFO* Basic, DERIVED_INFO* Calculated, int* LastStartSector) {
  if ((Basic->Time - Calculated->TaskStartTime < 3600) && (ActiveTaskPoint <= 1)) {
    CheckStart(Basic, Calculated, LastStartSector);
  }
  return FALSE;
}

void CheckFinish(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  if (InFinishSector(Basic, Calculated, ActiveTaskPoint)) {
    Calculated->IsInSector = true;
    aatdistance.AddPoint(GetCurrentPosition(*Basic), ActiveTaskPoint);
    if (!Calculated->ValidFinish) {
      Calculated->ValidFinish = true;
      if (!ISGAAIRCRAFT) {
        AnnounceWayPointSwitch(Calculated, false);
      }

      // JMWX save calculated data at finish
      memcpy(&Finish_Derived_Info, Calculated, sizeof(DERIVED_INFO));
    }
  }
}
