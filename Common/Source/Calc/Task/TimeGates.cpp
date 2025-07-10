/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TimeGates.cpp
 */

#include "externs.h"
#include "TimeGates.h"
#include "LKProcess.h"
#include "utils/stl_utils.h"
#include "utils/printf.h"
#include "Sound/Sound.h"
#include "Library/TimeFunctions.h"

namespace TimeGates {

open_type GateType = open_type::anytime;


int PGOpenTimeH = 0;
int PGOpenTimeM = 0;

int PGCloseTimeH = 23;
int PGCloseTimeM = 59;

int PGGateIntervalTime = 30;

int PGNumberOfGates = 59;

int WaitingTime = 5;
int StartWindow = 10;

void ResetSettings() {
  PGOpenTimeH = 12;  // in Hours
  PGOpenTimeM = 0;   // in Minute

  PGCloseTimeH = 23;  // in Hours
  PGCloseTimeM = 59;  // in Minute

  PGGateIntervalTime = 30; // Interval, in minutes
  PGNumberOfGates = 0; // How many gates, 1-x

  WaitingTime = 5;
  StartWindow = 10;
}

}  // namespace TimeGates

using namespace TimeGates;

namespace {

  // ------------------------------
// Rutime Global

// PGOpenTime and PGCloseTime are in seconds from 00:00:00
//  Using Local Time to avoid to manage UTC midnight wrapping

int PGOpenTime = 0;
int PGCloseTime = 86399;  // 23:59:59

// 0 before first gate open, 1..x for next gates 
int NextGate = -1;

// ------------------------------

// Returns the specified gate time (hours), negative -1 if invalid
int GateTime(int gate) {
  if (gate < 0) {
    throw std::out_of_range("Invalid gate");
  }
  if (gate >= PGNumberOfGates) {
    throw std::out_of_range("Invalid gate");
  }
  return PGOpenTime + (gate * PGGateIntervalTime * 60);
}

// Returns the next gate number, 0-x, -1 (negative) if no gates left
int CalcNextGate(int timenow) {
  for (int gate = 0; gate < PGNumberOfGates; gate++) {
    if (timenow < GateTime(gate)) {
      return gate;
    }
  }
  return -1;  // no gates left
}

bool NotifyCalled = false;
inline void Notify(const TCHAR* text) {
  NotifyCalled = true;
  DoStatusMessage(text);
}

template<typename ...Args>
inline void Notify(const TCHAR* fmt, const Args&... args) {
  TCHAR text[128];
  lk::snprintf(text, fmt, std::forward<const Args&>(args)...);
  ::Notify(text);
}

class GateOpeningNotification {
 private:
  const int TimeDiff;
  MsgToken_t MessageFirst;  // Removed const to allow move semantics
  MsgToken_t MessageNext;
  const TCHAR* const SoundFile;
  bool Flag = false;

 public:
  GateOpeningNotification(int timeDiff, MsgToken_t messageFirst, MsgToken_t messageNext, const TCHAR* soundFile)
      : TimeDiff(timeDiff),
        MessageFirst(std::move(messageFirst)),  // Use std::move for MsgToken_t
        MessageNext(std::move(messageNext)),
        SoundFile(soundFile) {}

  bool Check(int time) const {
    return time <= TimeDiff;
  }

  // Method to notify gate opening
  bool Notify(int time) {
    if (Flag) {
      return true;  // Already notified
    }

    if (!Check(time)) {
      return false;  // Not the right time
    }

    Flag = true;

    if (NextGate > 0 && TimeDiff > ((PGGateIntervalTime * 60) * 3 / 4)) {
      // no notification if less than 25% of the interval has elapsed since the previous gate trigger.
      return false;
    }

    if (NextGate == 0) {
      DebugLog(_T("Time to First Gate : %02d:%02d:%02d"), time / 3600, (time % 3600) / 60, time % 60);
      if (MessageFirst != nullptr) {
        ::Notify(MessageFirst());
      }
    }
    else {      
      DebugLog(_T("Time to Next Gate : %02d:%02d:%02d"), time / 3600, (time % 3600) / 60, time % 60);
      if (MessageNext != nullptr) {
        ::Notify(MessageNext());
      }
    }
    LKSound(SoundFile);

    return true;  // Notification sent
  }

  // Reset the notification flag
  void ResetFlag() {
    Flag = false;
  }
};

GateOpeningNotification notifications[] = {
    {60, nullptr, nullptr, _T("LK_3HITONES.WAV")},
    {300, MsgToken<852>, MsgToken<2512>, _T("LK_HITONE.WAV")},
    {600, MsgToken<852>, MsgToken<2511>, _T("LK_HITONE.WAV")},
    {1800, MsgToken<851>, MsgToken<2510>, _T("LK_DINGDONG.WAV")},
    {3600, MsgToken<850>, MsgToken<2509>, _T("LK_DINGDONG.WAV")},
};

int InitActiveGate(int utc_time) {
  NextGate = -1;

  if (TimeGates::GateType == TimeGates::fixed_gates) {
    PGOpenTime = ((PGOpenTimeH * 60) + PGOpenTimeM) * 60;
    PGCloseTime = ((PGCloseTimeH * 60) + PGCloseTimeM) * 60;
  }
  else {
    PGNumberOfGates = 0;
    PGOpenTime = 0;
    PGCloseTime = 86399;
  }

  if (PGCloseTime > 86399) {
    PGCloseTime = 86399;  // 23:59:59
  }

  int timenow = LocalTime(utc_time);
  if (timenow > PGCloseTime) {
    return -1;
  }
  if (timenow < PGOpenTime) {
    return 0;
  }
  return CalcNextGate(timenow) - 1;
}

// Returns the gatetime difference to current local time. Positive if gate is in the future.
int GateTimeDiff(int utc_time, int gate) {
  return (GateTime(gate) - LocalTime(utc_time));
}

// AlertGateOpen() is called when a gate is open
// Show the message notification and play sound alert
void AlertGateOpen(int utc_time) {
  int local_time = LocalTime(utc_time);
  DebugLog(_T("... CheckStart: ActiveGate=%d now OPEN (%02d:%02d:%02d)\n"), NextGate - 1, local_time / 3600, (local_time % 3600) / 60, local_time % 60);

  for (auto& notification : notifications) {
    notification.ResetFlag();
  }

  if (NextGate == (PGNumberOfGates - 1)) {
    ::Notify(MsgToken<372>());  // LKTOKEN  _@M372_ = "LAST GATE IS OPEN"
  }
  else if (PGNumberOfGates == 1) {
    ::Notify(MsgToken<314>());  // "GATE OPEN"
  }
  else {
    ::Notify(_T("%s %d / %d %s"),
                 MsgToken<315>(),  // LKTOKEN  _@M315_ = "GATE"
                 NextGate, PGNumberOfGates,
                 MsgToken<347>());  // LKTOKEN  _@M347_ = "IS OPEN"
  }
  LKSound(_T("LK_GATEOPEN.WAV"));
}

void AlertGateClose(int utc_time) {
  NextGate = -1;
  int local_time = LocalTime(utc_time);
  DebugLog(_T("... CheckStart: Gate Closed (%02d:%02d:%02d)"), local_time / 3600, (local_time % 3600) / 60, local_time % 60);
  ::Notify(MsgToken<316>());  // LKTOKEN  _@M316_ = "GATES CLOSED"
}

bool ValidTask() {
  // check for valid taskpoint 0 can be useless, should be valid if we have a valid taskpoint 1
  ScopeLock lock(CritSec_TaskData);
  return ValidTaskPointFast(1) && ValidTaskPointFast(0);
}

}  // namespace

bool UseGates() {
  if (!ValidTask()) {
    return false;  // no gates for simple "goto" task
  }
  if (GateType == TimeGates::pev_start) {
    return true;
  }
  if (gTaskType != task_type_t::GP) {
    return false;
  }
  return (PGNumberOfGates > 0);
}

int ActiveGate() {
  if (NextGate < 0 || NextGate >= PGNumberOfGates) {
    return -1;
  }
  return NextGate - 1;
}

int OpenGateTime() {
  if (NextGate > 0) {
    return GateTime(NextGate - 1);
  }
  return PGOpenTime;
}

int NextGateTime() {
  return GateTime(NextGate);
}

int GateCloseTime() {
  return PGCloseTime;
}

int NextGateTimeDiff(int utc_time) {
  return GateTimeDiff(utc_time, NextGate);
}

// Do we have some gates available, either running right now or in the future?
// Basically mytime <CloseTime...
bool HaveGates(int utc_time) {
  int timenow = LocalTime(utc_time);
  return (timenow < PGCloseTime);
}

// returns the current gate we are in, either in the past or in the future.
// It does not matter if it is still valid (it is expired).
// There is ALWAYS an activegate, it cannot be negative!
int InitActiveGate() {
  return InitActiveGate(LocalTime());
}

// autonomous check for usegates, and current chosen activegate is open, so a valid start
// is available crossing the start sector..
bool ValidGate(int utc_time) {
  // always ok to start, if no usegates
  if (!UseGates()) {
    return true;
  }

  if (TimeGates::GateType == TimeGates::pev_start && PGOpenTime == 0) {
    return false;
  }

  int timenow = LocalTime(utc_time);
  if (timenow >= PGCloseTime) {
    return false;  // HaveGates
  }
  if (timenow < PGOpenTime) {
    return false;
  }
  return true;
}

// ResetGates() is called when the task is reset
void ResetGates() {
  NextGate = InitActiveGate();
}

void NotifyGateState(int utc_time) {
  try {
    if (!UseGates()) {
      return;  // No TimeGates are configured
    }

    if (NextGate < 0) {
      NextGate = InitActiveGate(utc_time);
      return;  // Nothing to notify
    }

    if (!HaveGates(utc_time)) {
      AlertGateClose(utc_time);
      return;  // Last Gate is closed
    }

    if (NextGate >= PGNumberOfGates) {
      return;  // No more gates
    }

    int gatetimediff = GateTimeDiff(utc_time, NextGate);
    if (gatetimediff <= 0) {
      NextGate++;
      AlertGateOpen(utc_time);
      // nothing else to do: the current activegate has just opened
      return;
    }

    for (auto& notification : notifications) {
      if (notification.Notify(gatetimediff)) {
        return;  // Notification sent
      }
    }
  }
  catch (std::exception&) {
    DebugLog(_T("Invalid Gates"));
    // ignore invalid gate
  }
}

void TriggerPevStart(int utc_time) {
  PGOpenTime =  LocalTime(utc_time) + WaitingTime * 60;
  PGCloseTime = PGOpenTime + StartWindow * 60;

  NextGate = 0;
  PGNumberOfGates = 1;
}

bool PilotEventEnabled() {
  return GateType == TimeGates::pev_start;
}

bool WaitForPilotEvent() {
  if (GateType == TimeGates::pev_start) {
    if (NextGate < 0) {
      return true;
    }
    if (PGNumberOfGates < 1) {
      return true;
    }
  }
  return false;
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("TimeGates") {
  LKLoadLanguageFile();

  WayPointList.resize(10);
  Task[0].Index = 0;
  Task[1].Index = 1;

  gTaskType = task_type_t::GP;

  SUBCASE("Fixed gates") {
    // Mock dependencies
    TimeGates::GateType = TimeGates::fixed_gates;
    PGNumberOfGates = 3;
    PGGateIntervalTime = 30;  // 30 minutes

    PGCloseTimeH = 4;  // Close time is 04:00
    PGCloseTimeM = 0;
    PGOpenTimeH = 2;  // Open time is 02:00
    PGOpenTimeM = 0;

    ResetGates();

    for (int time = 1; time < 86399; time += 2) {
      NotifyCalled = false;

      NotifyGateState(time);

      if (time >= PGOpenTime && time < PGCloseTime) {
        CHECK(ValidGate(time));
      }
      else {
        CHECK_FALSE(ValidGate(time));
      }

      switch (time) {
        case 1:
          CHECK_EQ(NextGate,  0);
          CHECK_FALSE(NotifyCalled);
          break;
        case 3601:
        case 5401:
        case 6601:
        case 6901:
          CHECK_EQ(NextGate,  0);
          CHECK(NotifyCalled);
          break;
        case 7201:
        case 8401:
        case 8701:
          CHECK_EQ(NextGate,  1);
          CHECK(NotifyCalled);
          break;
        case 9001:
        case 10201:
        case 10501:
          CHECK_EQ(NextGate,  2);
          CHECK(NotifyCalled);
          break;
        case 10801:
          CHECK_EQ(NextGate,  3);
          CHECK(NotifyCalled);
          break;
        case 14401:
          CHECK_EQ(NextGate,  -1);
          CHECK(NotifyCalled);
          break;
        default:
          CHECK_FALSE(NotifyCalled);
          break;
      }
    }
  }

  SUBCASE("PEV start") {
    TimeGates::GateType = TimeGates::pev_start;
    WaitingTime = 5;
    StartWindow = 10;

    ResetGates();
    bool trigger = true;

    for (int time = 1; time < 86399; time++) {
      NotifyCalled = false;
      NotifyGateState(time);

      if (trigger && time > 100) {
        trigger = false;
        TriggerPevStart(time);
      }

      if (trigger) {
        CHECK(WaitForPilotEvent());
        CHECK_FALSE(ValidGate(time));  // gate invalid until trigger
      }
      else if (time <= 100 + (WaitingTime * 60)) {
        CHECK_FALSE(WaitForPilotEvent());
        CHECK_FALSE(ValidGate(time));  // gate still invalid after trigger until WaitingTine not elapsed
      }
      else if (time > 100 + (WaitingTime * 60) + StartWindow * 60) {
        CHECK(WaitForPilotEvent());
        CHECK_FALSE(ValidGate(time));  // gate become invalid after trigger + waitingtime + startwindow
      }
      else {
        CHECK_FALSE(WaitForPilotEvent());
        CHECK(ValidGate(time));  // gate open
      }
    }
  }

  LKUnloadLanguageFile();
}

#endif
