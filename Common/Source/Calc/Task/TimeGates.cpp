/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKProcess.h"
#include "utils/stl_utils.h"
#include "Sound/Sound.h"
#include "Library/TimeFunctions.h"

// ALL TIME VALUES ARE IN SECONDS! 
bool UseGates() {
  if (!(gTaskType==TSK_GP) ) return(false);
  if (PGNumberOfGates>0) {
	if (ValidTaskPoint(0) && ValidTaskPoint(1)) {
		return(true);
	} else
		return(false);
  } else
	return(false);
}

// Is the gate time open?
bool IsGateOpen() {
   int timenow;
   timenow=LocalTime();

   if ( (timenow>=PGOpenTime) && (timenow<=PGCloseTime))
	return true;
   else
	return false;

}


// Returns the next gate number, 0-x, -1 (negative) if no gates left or time is over
int NextGate() {
  int timenow, gate, gatetime;
  timenow=LocalTime();
  if (timenow>PGCloseTime) {
	#if DEBUGATE
	StartupStore(_T("... Timenow: %d over, gate closed at %d\n"),timenow, PGCloseTime);
	#endif
	return(-1);
  }
  for (gate=0; gate<PGNumberOfGates; gate++) {
	gatetime=PGOpenTime + (gate * PGGateIntervalTime *60);
	if (timenow < gatetime) {
		#if DEBUGATE
		StartupStore(_T("... Timenow: %d Nextgate is n.%d(0-%d) at %d\n"),timenow, gate, PGNumberOfGates-1, gatetime);
		#endif
		return(gate);
	}
  }
  #if DEBUGATE
  StartupStore(_T("... Timenow: %d no NextGate\n"),timenow);
  #endif
  return(-1);
}

// Returns the specified gate time (hours), negative -1 if invalid
int GateTime(int gate) {
  if (gate<0) return(-1);
  int gatetime;
  gatetime=PGOpenTime + (gate * PGGateIntervalTime *60);
  return(gatetime);
}


int GateCloseTime() {
  return PGCloseTime;
}


// Returns the gatetime difference to current local time. Positive if gate is in the future.
int GateTimeDiff(int gate) {
  int timenow, gatetime;
  timenow=LocalTime();
  gatetime=PGOpenTime + (gate * PGGateIntervalTime *60);
  return(gatetime-timenow);
}

// Returns the current open gate number, 0-x, or -1 (negative) if out of time.
// This is NOT the next start! It tells you if a gate is open right now, within time limits.
int RunningGate() {
  if(!UseGates()) {
    return (-1);
  }
    
  int timenow, gate, gatetime;
  timenow=LocalTime();
  if (timenow<PGOpenTime || timenow>PGCloseTime) return(-1);

  // search up to gates+1 ex. 12.40 > 13:00 is end time
  // we are checking the END of the gate, so it is like having a gate+1
  for (gate=1; gate<PGNumberOfGates; gate++) {
	gatetime=PGOpenTime + (gate * PGGateIntervalTime *60);
	// timenow cannot be lower than gate 0, because gate0 is PGOpenTime
	if (timenow < gatetime) {
  		#if DEBUGATE
 		StartupStore(_T("... Timenow: %d RunningGate n.%d (0-%d)\n"),timenow,gate-1,PGNumberOfGates-1);
  		#endif
		return(gate-1);
	}
  }
  if(gate == PGNumberOfGates && timenow < PGCloseTime) {
      return PGNumberOfGates-1;
  }
  
  StartupStore(_T("--- RunningGate invalid: timenow=%d Open=%d Close=%d NumGates=%d Interval=%d%s"),
	timenow,PGOpenTime,PGCloseTime,PGNumberOfGates,PGGateIntervalTime,NEWLINE);
  return(-1);
}

// Do we have some gates available, either running right now or in the future?
// Basically mytime <CloseTime...
bool HaveGates() {
  int timenow;
  timenow=LocalTime();
  if (timenow>PGCloseTime)
	return(false);
  else
	return(true);
}

// returns the current gate we are in, either in the past or in the future. 
// It does not matter if it is still valid (it is expired).
// There is ALWAYS an activegate, it cannot be negative!
int InitActiveGate() {
  ActiveGate = -1;

  PGOpenTime=((PGOpenTimeH*60)+PGOpenTimeM)*60;
  PGCloseTime=((PGCloseTimeH*60)+PGCloseTimeM)*60;;
  if (PGCloseTime>86399) PGCloseTime=86399; // 23:59:59    
    
  int timenow;
  timenow=LocalTime();
  if (timenow<PGOpenTime) return(0);
  if (timenow>PGCloseTime) return(PGNumberOfGates-1);
  return(RunningGate());
}

void AlertGateOpen(int gate) {
  TCHAR tag[100] ={0};
  if (gate == (PGNumberOfGates-1)) {
	// LKTOKEN  _@M372_ = "LAST GATE IS OPEN"
	_tcsncpy(tag,MsgToken(372), std::size(tag)-1);
  } else {
	_sntprintf(tag, std::size(tag)-1, _T("%s %d of %d %s"),
	// LKTOKEN  _@M315_ = "GATE" 
		MsgToken(315),
		gate+1, PGNumberOfGates,
	// LKTOKEN  _@M347_ = "IS OPEN" 
		MsgToken(347));
  }
  DoStatusMessage(tag);
  LKSound(_T("LK_GATEOPEN.WAV"));
}

// Are we on the correct side of start cylinder?
bool CorrectSide() {
  // Remember that IsInSector works reversed...
#if DEBUGTGATES
StartupStore(_T("CorrectSide: PGstartout=%d InSector=%d\n"),PGStartOut,CALCULATED_INFO.IsInSector);
#endif

  LockTaskData();
  bool ExitWpt = ((ActiveTaskPoint > 0) ? (Task[ActiveTaskPoint].OutCircle) : !PGStartOut);
  UnlockTaskData();

  if (!ExitWpt && CALCULATED_INFO.IsInSector)
	  return false;
  if (ExitWpt && !CALCULATED_INFO.IsInSector) 
	  return false;

  return true;
}

// autonomous check for usegates, and current chosen activegate is open, so a valid start
// is available crossing the start sector..
bool ValidGate() {
  // always ok to start, if no usegates
  if (!UseGates()) return true;
  if (ActiveGate <0 || ActiveGate>=PGNumberOfGates) {
	#if DEBUGTGATES
	StartupStore(_T("... ValidGate false, bad ActiveGate\n"));
	#endif
	return false;
  }
  int timenow;
  timenow=LocalTime();
  if (timenow>PGCloseTime) {
	#if DEBUGTGATES
	StartupStore(_T("... ValidGate false, timenow>PGCloseTime\n"));
	#endif
	return false; // HaveGates
  }
  int timegate;
  timegate=GateTime(ActiveGate);
  if (timegate<1) {
	#if DEBUGTGATES
	StartupStore(_T("... ValidGate false, GateTime returned<0 for ActiveGate=%d\n"),ActiveGate);
	#endif
	return false;
  }
  if ( timenow<timegate ) {
	#if DEBUGTGATES
	StartupStore(_T("... ValidGate false, timenow<timegate for ActiveGate=%d\n"),ActiveGate);
	#endif
	return false;
  }

  #if DEBUGTGATES
  StartupStore(_T("... ValidGate TRUE for ActiveGate=%d\n"),ActiveGate);
  #endif
  return true;
}
