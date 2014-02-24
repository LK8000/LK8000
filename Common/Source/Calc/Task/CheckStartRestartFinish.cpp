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

extern AATDistance aatdistance;


DERIVED_INFO Finish_Derived_Info;



// this is called only when ActiveWayPoint is 0, still waiting for start
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
				DoStatusMessage(gettext(TEXT("_@M850_")));
				if (EnableSoundModes) {
					LKSound(_T("LK_DINGDONG.WAV"));
				}
			}
			if (gatetimediff==1800 && ((PGGateIntervalTime>=45)||ActiveGate==0) ) { 
				//  851  FIRST GATE OPEN IN 30 MINUTES
				DoStatusMessage(gettext(TEXT("_@M851_")));
				if (EnableSoundModes) {
					LKSound(_T("LK_DINGDONG.WAV"));
				}
			}
			if (gatetimediff==600 && ((PGGateIntervalTime>=15)||ActiveGate==0) ) { // 10 minutes to go
				//  852  10 MINUTES TO GO
				DoStatusMessage(gettext(TEXT("_@M852_")));
				if (EnableSoundModes) {
					LKSound(_T("LK_HITONE.WAV"));
				}
			}
			if (gatetimediff==300 && ((PGGateIntervalTime>=10)||ActiveGate==0)) { // 5 minutes to go
				//  853  5 MINUTES TO GO
				DoStatusMessage(gettext(TEXT("_@M853_")));
				if (EnableSoundModes) {
					LKSound(_T("LK_HITONE.WAV"));
				}
			}
			if (gatetimediff==60) { // 1 minute to go
				if (EnableSoundModes) {
					LKSound(_T("LK_3HITONES.WAV"));
				}
			}

		} // HaveGates
	} // not init

  }

  if (ISPARAGLIDER && PGStartOut) {
	// start OUT and go in
	if (!InStartSector(Basic,Calculated,*LastStartSector, &StartCrossed)) {
		Calculated->IsInSector = false;

		if (ReadyToStart(Calculated)) {
			aatdistance.AddPoint(Basic->Longitude, Basic->Latitude, 0);
		}
		if (ValidStartSpeed(Basic, Calculated, StartMaxSpeedMargin)) {
			ReadyToAdvance(Calculated, false, true);
		}
	} else
		Calculated->IsInSector = true;
  } else {
	// start IN and go out, OLD CLASSIC MODE
	if (InStartSector(Basic,Calculated,*LastStartSector, &StartCrossed)) {
		// InSector check calling this function is resetting IsInSector at each run, so it was false.
		Calculated->IsInSector = true;

		if (ReadyToStart(Calculated)) {
			aatdistance.AddPoint(Basic->Longitude, Basic->Latitude, 0);
		}
    		// ToLo: we are ready to start even when outside start rules but within margin
		if (ValidStartSpeed(Basic, Calculated, StartMaxSpeedMargin)) {
			ReadyToAdvance(Calculated, false, true);
		}
    		// TODO accuracy: monitor start speed throughout time in start sector
  	}
  } // end start mode

  if (StartCrossed && ValidGate() ) {  // 100509

	#if DEBUGTGATES
	StartupStore(_T("... CheckStart: start crossed and valid gate!\n"));
	#endif
	
	if(ISGAAIRCRAFT) {
		Calculated->ValidStart = true;
		ActiveWayPoint=0; // enforce this since it may be 1
		StartTask(Basic,Calculated, true, false);
		return;
	}

    // ToLo: Check weather speed and height are within the rules or not (zero margin)
    if(!IsFinalWaypoint() && ValidStartSpeed(Basic, Calculated) && InsideStartHeight(Basic, Calculated)) {

      // This is set whether ready to advance or not, because it will
      // appear in the flight log, so if it's valid, it's valid.
      Calculated->ValidStart = true;

      if (ReadyToAdvance(Calculated, true, true)) {
        ActiveWayPoint=0; // enforce this since it may be 1
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
    
      if ((ActiveWayPoint<=1) 
          && !IsFinalWaypoint()
          && (Calculated->ValidStart==false)
          && (Calculated->Flying)) {
        
	#if 0
	// 101014 This is called from wrong thread, and cause bad crashes
	// moved to new GCE event inside InputEvents - paolo
        // need to detect bad starts, just to get the statistics
        // in case the bad start is the best available, or the user
        // manually started
        StartTask(Basic, Calculated, false, false);
//        Calculated->ValidStart = false;
        bool startTaskAnyway = false;
        if (ReadyToAdvance(Calculated, true, true)) {
          dlgStartTaskShowModal(&startTaskAnyway,
                                Calculated->TaskStartTime,
                                Calculated->TaskStartSpeed,
                                Calculated->TaskStartAltitude);
          if (startTaskAnyway) {
            ActiveWayPoint=0; // enforce this since it may be 1
            StartTask(Basic,Calculated, true, true);
          }
        }
        Calculated->ValidStart = startTaskAnyway;
	#else // 101014
        StartTask(Basic, Calculated, false, false);
        if (ReadyToAdvance(Calculated, true, true)) {
		InputEvents::processGlideComputer(GCE_TASK_CONFIRMSTART);
	}
	#endif
        
        if (Calculated->Flying) {
		Calculated->ValidFinish = false;
        }
      }

    }
  }
}


BOOL CheckRestart(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int *LastStartSector) {
  if((Basic->Time - Calculated->TaskStartTime < 3600)
     &&(ActiveWayPoint<=1)) {

    CheckStart(Basic, Calculated, LastStartSector);
  }
  return FALSE;
}


void CheckFinish(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  if (InFinishSector(Basic,Calculated, ActiveWayPoint)) {
    Calculated->IsInSector = true;
    aatdistance.AddPoint(Basic->Longitude,
                         Basic->Latitude,
                         ActiveWayPoint);
    if (!Calculated->ValidFinish) {
      Calculated->ValidFinish = true;
      if(!ISGAAIRCRAFT) AnnounceWayPointSwitch(Calculated, false);

      // JMWX save calculated data at finish
      memcpy(&Finish_Derived_Info, Calculated, sizeof(DERIVED_INFO));
    }
  }
}

