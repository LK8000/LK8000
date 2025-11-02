/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * linux code adapted from XCSoar original source code Battery.cpp
 *
 */

#include "externs.h"
#include "Sound/Sound.h"
#include "BatteryManager.h"
#include "utils/printf.h"

#if defined(PNA) && defined(UNDER_CE)
#include "Devices/LKHolux.h"
#endif

#include "DoInits.h"

#ifdef WIN32
#ifdef UNDER_CE
bool BatteryManager::GetBatteryInfo(BATTERYINFO& BatteryInfo) {
  // set default return value
  DWORD result = 0;

  SYSTEM_POWER_STATUS_EX2 sps;

  // request the power status
  result = GetSystemPowerStatusEx2(&sps, sizeof(sps), TRUE);

  // only update the caller if the previous call succeeded
  if (result) {
    BatteryInfo.acStatus =
        static_cast<Battery::battery_status>(sps.ACLineStatus);
    BatteryInfo.chargeStatus =
        static_cast<Battery::battery_charge_status>(sps.BatteryFlag);
    BatteryInfo.BatteryLifePercent = sps.BatteryLifePercent;
    BatteryInfo.BatteryVoltage = sps.BatteryVoltage;
    BatteryInfo.BatteryAverageCurrent = sps.BatteryAverageCurrent;
    BatteryInfo.BatteryCurrent = sps.BatteryCurrent;
    BatteryInfo.BatterymAHourConsumed = sps.BatterymAHourConsumed;
    BatteryInfo.BatteryTemperature = sps.BatteryTemperature;
  }

  return result != 0;
}
#else
bool BatteryManager::GetBatteryInfo(BATTERYINFO& BatteryInfo) {
  // not implemented on Windows PC
  BatteryInfo.BatteryLifePercent = BATTERY_UNKNOWN;
  BatteryInfo.acStatus = Battery::UNKNOWN;
  BatteryInfo.chargeStatus = Battery::CHARGE_UNKNOWN;

  return false;
}
#endif
#endif

#if defined(__linux__)
#ifdef KOBO
#include "OS/FileUtil.hpp"
#include "Hardware/CPU.hpp"
#include <memory>

static std::unique_ptr<ScopeLockCPU> cpu;

bool BatteryManager::GetBatteryInfo(BATTERYINFO& BatteryInfo) {
  // assume failure at entry
  BatteryInfo.BatteryLifePercent = BATTERY_UNKNOWN;
  BatteryInfo.acStatus = Battery::UNKNOWN;
  BatteryInfo.chargeStatus = Battery::CHARGE_UNKNOWN;

  // code shamelessly copied from OS/SystemLoad.cpp
  char line[256];
  if (!File::ReadString("/sys/class/power_supply/mc13892_bat/uevent", line,
                        sizeof(line))) {
    return false;
  }

  // set default return value
  bool result = false;

  char field[80], value[80];
  int n;
  char* ptr = line;
  while (sscanf(ptr, "%[^=]=%[^\n]\n%n", field, value, &n) == 2) {
    ptr += n;
    if (!strcmp(field, "POWER_SUPPLY_STATUS")) {
      if (!strcmp(value, "Not charging") || !strcmp(value, "Charging")) {
        if (!cpu) {
          cpu = std::make_unique<ScopeLockCPU>();
        }
        BatteryInfo.acStatus = Battery::ONLINE;
      }
      else if (!strcmp(value, "Discharging")) {
        cpu = nullptr;
        BatteryInfo.acStatus = Battery::OFFLINE;
      }
    }
    else if (!strcmp(field, "POWER_SUPPLY_CAPACITY")) {
      result = true;
      int rem = atoi(value);
      BatteryInfo.BatteryLifePercent = rem;
      if (BatteryInfo.acStatus == Battery::OFFLINE) {
        cpu = nullptr;
        if (rem > 30) {
          BatteryInfo.chargeStatus = Battery::HIGH;
        }
        else if (rem >= 10) {
          BatteryInfo.chargeStatus = Battery::LOW;
        }
        else if (rem < 10) {
          BatteryInfo.chargeStatus = Battery::CRITICAL;
        }
      }
      else {
        if (!cpu) {
          cpu = std::make_unique<ScopeLockCPU>();
        }
        BatteryInfo.chargeStatus = Battery::CHARGING;
      }
    }
  }

  return result;
}

#elif defined(ENABLE_SDL)

#include <SDL_power.h>

bool BatteryManager::GetBatteryInfo(BATTERYINFO& BatteryInfo) {
  int remaining_percent = 0;

  // assume failure at entry
  BatteryInfo.BatteryLifePercent = BATTERY_UNKNOWN;
  BatteryInfo.acStatus = Battery::UNKNOWN;
  BatteryInfo.chargeStatus = Battery::CHARGE_UNKNOWN;

  SDL_PowerState power_state = SDL_GetPowerInfo(NULL, &remaining_percent);
  if (remaining_percent >= 0) {
    BatteryInfo.BatteryLifePercent = remaining_percent;
  }

  switch (power_state) {
    case SDL_POWERSTATE_CHARGING:
    case SDL_POWERSTATE_CHARGED:
      BatteryInfo.acStatus = Battery::ONLINE;
      BatteryInfo.chargeStatus = Battery::CHARGING;
      break;
    case SDL_POWERSTATE_ON_BATTERY:
      BatteryInfo.acStatus = Battery::OFFLINE;
      if (remaining_percent >= 0) {
        if (remaining_percent > 30) {
          BatteryInfo.chargeStatus = Battery::HIGH;
        }
        else if (remaining_percent > 30) {
          BatteryInfo.chargeStatus = Battery::LOW;
        }
        else {
          BatteryInfo.chargeStatus = Battery::CRITICAL;
        }
      }
      else {
        BatteryInfo.chargeStatus = Battery::CHARGE_UNKNOWN;
      }
      break;
    default:
      BatteryInfo.chargeStatus = Battery::CHARGE_UNKNOWN;
      break;
  }

  return true;
}
#elif defined(ANDROID)
#include <jni.h>

extern "C" gcc_visibility_default JNIEXPORT void JNICALL
Java_org_LK8000_BatteryReceiver_setBatteryPercent(JNIEnv* env, jclass cls,
                                                  jint value, jint plugged,
                                                  jint status) {
  g_BatteryManager.SetBatteryState(value, plugged, status);
}

void BatteryManager::SetBatteryState(int percent, int plugged, int status) {
  ScopeLock lock(android_battery_mutex);
  android_BatteryLifePercent = percent;
  android_acStatus = (!plugged) ? Battery::OFFLINE : Battery::ONLINE;

  switch (status) {
    default:
    case 1:  // BATTERY_STATUS_UNKNOWN = 1;
      android_chargeStatus = Battery::CHARGE_UNKNOWN;
      break;
    case 2:  // BATTERY_STATUS_CHARGING = 2;
      android_chargeStatus = Battery::CHARGING;
      break;
    case 3:  // BATTERY_STATUS_DISCHARGING = 3;
    case 4:  // BATTERY_STATUS_NOT_CHARGING = 4;
    case 5:  // BATTERY_STATUS_FULL = 5;
      if (android_BatteryLifePercent > 30) {
        android_chargeStatus = Battery::HIGH;
      }
      else if (android_BatteryLifePercent > 10) {  // Corrected logic
        android_chargeStatus = Battery::LOW;
      }
      else {
        android_chargeStatus = Battery::CRITICAL;
      }
      break;
  }
}

bool BatteryManager::GetBatteryInfo(BATTERYINFO& BatteryInfo) {
  ScopeLock lock(android_battery_mutex);

  // assume failure at entry
  BatteryInfo.BatteryLifePercent = android_BatteryLifePercent;
  BatteryInfo.acStatus = android_acStatus;
  BatteryInfo.chargeStatus = android_chargeStatus;

  return android_acStatus != Battery::UNKNOWN;
}

#else
// For now, for linux ==> TODO !
bool BatteryManager::GetBatteryInfo(BATTERYINFO& BatteryInfo) {
  // assume failure at entry
  BatteryInfo.BatteryLifePercent = BATTERY_UNKNOWN;
  BatteryInfo.acStatus = Battery::UNKNOWN;
  BatteryInfo.chargeStatus = Battery::CHARGE_UNKNOWN;

  return false;
}

#endif
#endif

void BatteryManager::Update() {
  BATTERYINFO BatteryInfo;
  BatteryInfo.acStatus = Battery::UNKNOWN;

#ifdef PNA
  if (DeviceIsGM130) {
    PDABatteryPercent = GM130PowerLevel();
    PDABatteryStatus = GM130PowerStatus();
    PDABatteryFlag = GM130PowerFlag();

    HaveBatteryInfo = true;
  }
  else
#endif
      if (GetBatteryInfo(BatteryInfo)) {
    PDABatteryPercent = BatteryInfo.BatteryLifePercent;
    PDABatteryStatus = BatteryInfo.acStatus;
    PDABatteryFlag = BatteryInfo.chargeStatus;

    if (!HaveBatteryInfo) {
      TestLog(_T("... LKBatteryManager: HaveBatteryInfo  ENABLED"));
    }
    HaveBatteryInfo = true;
  }
  else {
    if (HaveBatteryInfo) {
      TestLog(_T("... LKBatteryManager: HaveBatteryInfo DISABLED"));
    }
    HaveBatteryInfo = false;
  }
  initialized = true;

  Manage();
}

void BatteryManager::Manage() {
  if (!initialized) {
    TestLog(_T("... LKBatteryManager waiting for first update"));
    return;
  }
  if (invalid) {
    return;
  }
  if (DoInit[MDI_BATTERYMANAGER]) {
    invalid = false, recharging = false;
    warn33 = true, warn100 = true;
    last_time = 0, init_time = 0;
    last_percent = 0, last_status = 0;
    numwarn = 0;

    if (!HaveBatteryInfo || PDABatteryPercent < 1 || PDABatteryPercent > 100) {
      StartupStore(
          _T("... LK BatteryManager V1: internal battery information not ")
          _T("available, function disabled"));
      invalid = true;
      DoInit[MDI_BATTERYMANAGER] = false;  // just to be sure
      return;
    }

    StartupStore(_T(". LK Battery Manager V1 started, current charge=%d%%"),
                 PDABatteryPercent);
    init_time = GPS_INFO.Time;
    DoInit[MDI_BATTERYMANAGER] = false;
  }

  // if first run,  and not passed 30 seconds, do nothing
  if (last_percent == 0 && (GPS_INFO.Time < (init_time + 30))) {
    return;
  }

  TCHAR mbuf[100];

  // first run after 30 seconds: give a message
  if (last_percent == 0) {
    if (PDABatteryPercent <= 50) {
      last_time = GPS_INFO.Time;
      // LKTOKEN _@M1352_ "BATTERY LEVEL"
      lk::snprintf(mbuf, _T("%s %d%%"), MsgToken<1352>(), PDABatteryPercent);
      DoStatusMessage(mbuf);
    }
    // special case, pdabattery is 0...
    if (PDABatteryPercent < 1) {
      StartupStore(_T("... LK Battery Manager disabled, low battery"));
      // LKTOKEN _@M1353_ "BATTERY MANAGER DISABLED"
      DoStatusMessage(MsgToken<1353>());
      invalid = true;
      return;
    }
    else {
      last_percent = PDABatteryPercent;
    }

    if (PDABatteryStatus != Battery::UNKNOWN) {
      last_status = PDABatteryStatus;
    }
    return;
  }

  if (last_status != PDABatteryStatus) {
    if (PDABatteryStatus == Battery::OFFLINE) {
      if (GiveBatteryWarnings()) {
        // LKTOKEN  _@M514_ = "POWER SUPPLY OFF"
        DoStatusMessage(MsgToken<514>());
      }
    }
    else {
      if (PDABatteryStatus == Battery::ONLINE) {
        if (GiveBatteryWarnings()) {
          // LKTOKEN  _@M515_ = "POWER SUPPLY ON"
          DoStatusMessage(MsgToken<515>());
        }
      }
      else {
        if (PDABatteryStatus == Battery::BACKUP_POWER) {
          if (GiveBatteryWarnings()) {
            // LKTOKEN  _@M119_ = "BACKUP POWER SUPPLY ON"
            DoStatusMessage(MsgToken<119>());
          }
        }
      }
    }
    last_status = PDABatteryStatus;
  }

  // Only check every 5 minutes normally
  if (GPS_INFO.Time < (last_time + (60 * 5))) {
    return;
  }

  // if battery is recharging, reset warnings and do nothing
  if (last_percent < PDABatteryPercent) {
    warn33 = true;
    warn100 = true;
    last_percent = PDABatteryPercent;
    if (!recharging) {
      recharging = true;
      if (PDABatteryFlag == Battery::CHARGING ||
          PDABatteryStatus == Battery::ONLINE) {
        if (GiveBatteryWarnings()) {
          // LKTOKEN  _@M124_ = "BATTERY IS RECHARGING"
          DoStatusMessage(MsgToken<124>());
        }
        last_time = GPS_INFO.Time;
      }
    }
    return;
  }

  // if battery is same level, do nothing except when 100% during recharge
  if (last_percent == PDABatteryPercent) {
    if (recharging && (PDABatteryPercent == 100) && warn100) {
      if (GiveBatteryWarnings()) {
        // LKTOKEN  _@M123_ = "BATTERY 100% CHARGED"
        DoStatusMessage(MsgToken<123>());
      }
      warn100 = false;
      last_time = GPS_INFO.Time;
    }
    return;
  }

  // else battery is discharging
  recharging = false;

  // Time to give a message to the user, if necessary
  if (PDABatteryPercent <= 5) {
    // LKTOKEN _@M1354_ "BATTERY LEVEL CRITIC!"
    lk::snprintf(mbuf, _T("%d%% %s"), PDABatteryPercent, MsgToken<1354>());
    DoStatusMessage(mbuf);
    LKSound(TEXT("LK_RED.WAV"));

    // repeat after 1 minute, forced
    last_time = GPS_INFO.Time - (60 * 4);
    last_percent = PDABatteryPercent;
    return;
  }
  if (PDABatteryPercent <= 10) {
    // LKTOKEN _@M1355_ "BATTERY LEVEL VERY LOW!"
    lk::snprintf(mbuf, _T("%d%% %s"), PDABatteryPercent, MsgToken<1355>());
    DoStatusMessage(mbuf);
    // repeat after 2 minutes, forced
    last_time = GPS_INFO.Time - (60 * 3);
    last_percent = PDABatteryPercent;
    return;
  }
  if (PDABatteryPercent <= 20) {
    // LKTOKEN _@M1356_ "BATTERY LEVEL LOW!"
    lk::snprintf(mbuf, _T("%d%% %s"), PDABatteryPercent, MsgToken<1356>());
    DoStatusMessage(mbuf);
    last_time = GPS_INFO.Time;
    last_percent = PDABatteryPercent;
    return;
  }

  if (PDABatteryPercent <= 30) {
    if (warn33) {
      // LKTOKEN _@M1352_ "BATTERY LEVEL"
      lk::snprintf(mbuf, _T("%s %d%%"), MsgToken<1352>(), PDABatteryPercent);
      DoStatusMessage(mbuf);
      warn33 = false;
    }
    last_time = GPS_INFO.Time;
    last_percent = PDABatteryPercent;
    return;
  }
  // DISABLED
  if (PDABatteryPercent <= 50) {
    last_time = GPS_INFO.Time;
    last_percent = PDABatteryPercent;
    return;
  }
}

// returns true if no problems with too many warnings
#define MAXBATTWARN 15
bool BatteryManager::GiveBatteryWarnings(void) {
  // If last warning was issued more than 60 minutes ago, reset toomany.
  if (GPS_INFO.Time > (last_warning_time + 3600)) {
    if (last_warning_time > 0 && numwarn > 0) {
      StartupStore(_T("... GiveBatteryWarnings resetting at %s"),
                   WhatTimeIsIt());
    }
    toomany_warnings = false;
    numwarn = 0;
  }

  if (toomany_warnings) {
    return false;
  }

  numwarn++;

  if (numwarn > MAXBATTWARN) {
    // LKTOKEN _@M1357_ "BATTERY WARNINGS DISABLED"
    DoStatusMessage(MsgToken<1357>());
    StartupStore(
        _T("... Too many battery warnings, disabling Battery Manager at %s"),
        WhatTimeIsIt());
    toomany_warnings = true;
    return false;
  }
  last_warning_time = GPS_INFO.Time;
  return true;
}

BatteryManager g_BatteryManager;
