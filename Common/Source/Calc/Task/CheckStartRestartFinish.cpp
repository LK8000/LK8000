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

DERIVED_INFO Finish_Derived_Info;



// this is called only when ActiveTaskPoint is 0 (or 1 to check for restart), still waiting for start
void CheckStart(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int *LastStartSector) {
  BOOL StartCrossed= false;

  if (UseGates()) {
#if DEBUGATE
StartupStore(_T("... CheckStart Timenow=%d OpenTime=%d CloseTime=%d ActiveGate=%d\n"),LocalTime(),PGOpenTime,PGCloseTime,ActiveGate);
#endif
  	int gatetimediff=-1;
	if ( ActiveGate<0 ) {
		// init activegate: assign first valid gate, current or future
		ActiveGate=InitActiveGate();
		if (ActiveGate<0||ActiveGate>(PGNumberOfGates-1)) {
			StartupStore(_T("... INVALID ActiveGate=%d\n"),ActiveGate);
			DoStatusMessage(_T("ERR-430 INVALID ACTIVEGATE: DISABLED"));
			PGNumberOfGates=0;
			return;		
		}
		#if DEBUGATE
		StartupStore(_T("... CheckStart: INIT ActiveGate=%d\n"),ActiveGate);
		#endif
	} else {
		if (HaveGates()) {
			gatetimediff=GateTimeDiff(ActiveGate);
			#if DEBUGATE
			StartupStore(_T("... CheckStart: ActiveGate=%d RunningGate=%d\n"),ActiveGate,RunningGate());
			StartupStore(_T("... CheckStart: gatetimediff=%d\n"),gatetimediff);
			#endif
			// a gate can be in the future , or already open!
			// case: first start, activegate is the first gate
			if (gatetimediff==0) {
				#if DEBUGATE
				StartupStore(_T("... CheckStart: ActiveGate=%d now OPEN\n"),ActiveGate);
				#endif
				AlertGateOpen(ActiveGate);
				// nothing else to do: the current activegate has just open
			} else {
				// check that also non-armed start is detected
				if (ActiveGate<(PGNumberOfGates-1)) {
					if (GateTimeDiff(ActiveGate+1)==0) {
						#if DEBUGATE
						StartupStore(_T("... CheckStart: ActiveGate+1=%d now OPEN\n"),ActiveGate);
						#endif
						ActiveGate++;
						AlertGateOpen(ActiveGate);
					}
				}
			}
			// now check for special alerts on countdown, only on current armed start
			if (gatetimediff==3600 && ((PGGateIntervalTime>=70)||ActiveGate==0) ) { 
				//  850  FIRST GATE OPEN IN 1 HOUR
				DoStatusMessage(MsgToken(850));
				LKSound(_T("LK_DINGDONG.WAV"));
			}
			if (gatetimediff==1800 && ((PGGateIntervalTime>=45)||ActiveGate==0) ) { 
				//  851  FIRST GATE OPEN IN 30 MINUTES
				DoStatusMessage(MsgToken(851));
				LKSound(_T("LK_DINGDONG.WAV"));
			}
			if (gatetimediff==600 && ((PGGateIntervalTime>=15)||ActiveGate==0) ) { // 10 minutes to go
				//  852  10 MINUTES TO GO
				DoStatusMessage(MsgToken(852));
				LKSound(_T("LK_HITONE.WAV"));
			}
			if (gatetimediff==300 && ((PGGateIntervalTime>=10)||ActiveGate==0)) { // 5 minutes to go
				//  853  5 MINUTES TO GO
				DoStatusMessage(MsgToken(853));
				LKSound(_T("LK_HITONE.WAV"));
			}
			if (gatetimediff==60) { // 1 minute to go
				LKSound(_T("LK_3HITONES.WAV"));
			}

		} // HaveGates
	} // not init

  }

  bool start_from_inside = ExitStart(*Calculated);
  Calculated->IsInSector = InStartSector(Basic, Calculated, !start_from_inside, *LastStartSector, &StartCrossed);

  // from inside and we are inside
  // or 
  // from outside and we are outside
  if (start_from_inside == Calculated->IsInSector) {
    if (ReadyToStart(Calculated)) {
      aatdistance.AddPoint(GetCurrentPosition(*Basic), 0);
    }
    if (ValidStartSpeed(Basic, Calculated, StartMaxSpeedMargin)) {
      ReadyToAdvance(Calculated, false, true);
    }
  }

  if (StartCrossed && ValidGate() ) {  // 100509

	#if DEBUGTGATES
	StartupStore(_T("... CheckStart: start crossed and valid gate!\n"));
	#endif
	
	if(ISGAAIRCRAFT) {
		Calculated->ValidStart = true;
		ActiveTaskPoint=0; // enforce this since it may be 1
		StartTask(Basic,Calculated, true, false);
		return;
	}

    // ToLo: Check weather speed and height are within the rules or not (zero margin)
    if(!IsFinalWaypoint() && ValidStartSpeed(Basic, Calculated) && InsideStartHeight(Basic, Calculated)) {

      // This is set whether ready to advance or not, because it will
      // appear in the flight log, so if it's valid, it's valid.
      Calculated->ValidStart = true;

      if (ReadyToAdvance(Calculated, true, true)) {
        ActiveTaskPoint=0; // enforce this since it may be 1
        StartTask(Basic,Calculated, true, true);
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
    } else {
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


BOOL CheckRestart(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int *LastStartSector) {
  if((Basic->Time - Calculated->TaskStartTime < 3600)
     &&(ActiveTaskPoint<=1)) {

    CheckStart(Basic, Calculated, LastStartSector);
  }
  return FALSE;
}


void CheckFinish(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  if (InFinishSector(Basic,Calculated, ActiveTaskPoint)) {
    Calculated->IsInSector = true;
    aatdistance.AddPoint(GetCurrentPosition(*Basic), ActiveTaskPoint);
    if (!Calculated->ValidFinish) {
      Calculated->ValidFinish = true;
      if(!ISGAAIRCRAFT) AnnounceWayPointSwitch(Calculated, false);

      // JMWX save calculated data at finish
      memcpy(&Finish_Derived_Info, Calculated, sizeof(DERIVED_INFO));
    }
  }
}

