/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKProcess.h"
#include "utils/stl_utils.h"
#include "utils/printf.h"
#include "Sound/Sound.h"
#include "Library/TimeFunctions.h"

// ALL TIME VALUES ARE IN SECONDS!

namespace {

int PGOpenTime = 0;
int PGCloseTime = 86399;  // 23:59:59

}  // namespace


bool UseGates() {
  if (gTaskType == task_type_t::GP) {
    if (PGNumberOfGates > 0) {
      if (ValidTaskPoint(0) && ValidTaskPoint(1)) {
        return true;
      }
    }
  }
  return false;
}

// Is the gate time open?
bool IsGateOpen() {
  int timenow = LocalTime();
  return ((timenow >= PGOpenTime) && (timenow <= PGCloseTime));
}

// Returns the next gate number, 0-x, -1 (negative) if no gates left or time is over
int NextGate() {
  int timenow = LocalTime();
  if (timenow > PGCloseTime) {
    DebugLog(_T("... Timenow: %d over, gate closed at %d\n"), timenow, PGCloseTime);
    return -1;
  }
  for (int gate = 0; gate < PGNumberOfGates; gate++) {
    int gatetime = PGOpenTime + (gate * PGGateIntervalTime * 60);
    if (timenow < gatetime) {
      DebugLog(_T("... Timenow: %d NextGate is n.%d(0-%d) at %d\n"), timenow, gate, PGNumberOfGates - 1, gatetime);
      return gate;
    }
  }
  DebugLog(_T("... Timenow: %d no NextGate\n"), timenow);
  return -1;
}

// Returns the specified gate time (hours), negative -1 if invalid
int GateTime(int gate) {
  if (gate < 0) {
    return -1;
  }
  return PGOpenTime + (gate * PGGateIntervalTime * 60);
}

int GateCloseTime() {
  return PGCloseTime;
}

// Returns the gatetime difference to current local time. Positive if gate is in the future.
int GateTimeDiff(int gate) {
  int timenow = LocalTime();
  int gatetime = PGOpenTime + (gate * PGGateIntervalTime * 60);
  return (gatetime - timenow);
}

// Returns the current open gate number, 0-x, or -1 (negative) if out of time.
// This is NOT the next start! It tells you if a gate is open right now, within time limits.
int RunningGate() {
  if (!UseGates()) {
    return -1;
  }

  int timenow = LocalTime();
  if (timenow < PGOpenTime || timenow > PGCloseTime) {
    return -1;
  }

  int gate = 1;
  // search up to gates+1 ex. 12.40 > 13:00 is end time
  // we are checking the END of the gate, so it is like having a gate+1
  for (; gate < PGNumberOfGates; gate++) {
    int gatetime = PGOpenTime + (gate * PGGateIntervalTime * 60);
    // timenow cannot be lower than gate 0, because gate0 is PGOpenTime
    if (timenow < gatetime) {
      DebugLog(_T("... Timenow: %d RunningGate n.%d (0-%d)\n"), timenow, gate - 1, PGNumberOfGates - 1);
      return (gate - 1);
    }
  }
  if (gate == PGNumberOfGates && timenow < PGCloseTime) {
    return PGNumberOfGates - 1;
  }

  StartupStore(_T("--- RunningGate invalid: timenow=%d Open=%d Close=%d NumGates=%d Interval=%d"), timenow, PGOpenTime,
               PGCloseTime, PGNumberOfGates, PGGateIntervalTime);
  return -1;
}

// Do we have some gates available, either running right now or in the future?
// Basically mytime <CloseTime...
bool HaveGates() {
  int timenow = LocalTime();
  return (timenow <= PGCloseTime);
}

// returns the current gate we are in, either in the past or in the future.
// It does not matter if it is still valid (it is expired).
// There is ALWAYS an activegate, it cannot be negative!
int InitActiveGate() {
  ActiveGate = -1;

  PGOpenTime = ((PGOpenTimeH * 60) + PGOpenTimeM) * 60;
  PGCloseTime = ((PGCloseTimeH * 60) + PGCloseTimeM) * 60;

  if (PGCloseTime > 86399) {
    PGCloseTime = 86399;  // 23:59:59
  }

  int timenow = LocalTime();
  if (timenow < PGOpenTime) {
    return 0;
  }
  if (timenow > PGCloseTime) {
    return (PGNumberOfGates - 1);
  }
  return RunningGate();
}

void AlertGateOpen(int gate) {
  TCHAR tag[100] = {0};
  if (gate == (PGNumberOfGates - 1)) {
    // LKTOKEN  _@M372_ = "LAST GATE IS OPEN"
    lk::strcpy(tag, MsgToken<372>());
  }
  else {
    lk::snprintf(tag, _T("%s %d of %d %s"),
                 // LKTOKEN  _@M315_ = "GATE"
                 MsgToken<315>(), gate + 1, PGNumberOfGates,
                 // LKTOKEN  _@M347_ = "IS OPEN"
                 MsgToken<347>());
  }
  DoStatusMessage(tag);
  LKSound(_T("LK_GATEOPEN.WAV"));
}

// autonomous check for usegates, and current chosen activegate is open, so a valid start
// is available crossing the start sector..
bool ValidGate() {
  // always ok to start, if no usegates
  if (!UseGates()) {
    return true;
  }
  if (ActiveGate < 0 || ActiveGate >= PGNumberOfGates) {
    DebugLog(_T("... ValidGate false, bad ActiveGate"));
    return false;
  }
  int timenow = LocalTime();
  if (timenow > PGCloseTime) {
    DebugLog(_T("... ValidGate false, timenow>PGCloseTime"));
    return false;  // HaveGates
  }
  int timegate = GateTime(ActiveGate);
  if (timegate < 1) {
    DebugLog(_T("... ValidGate false, GateTime returned<0 for ActiveGate=%d"), ActiveGate);
    return false;
  }
  if (timenow < timegate) {
    DebugLog(_T("... ValidGate false, timenow<timegate for ActiveGate=%d"), ActiveGate);
    return false;
  }
  DebugLog(_T("... ValidGate TRUE for ActiveGate=%d"), ActiveGate);
  return true;
}
