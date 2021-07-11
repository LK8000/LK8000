/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights
 *
 * linux code adapted from XCSoar original source code Battery.cpp
 *

   $Id$
*/

#include "externs.h"
#include "Sound/Sound.h"

#if defined(PNA) && defined(UNDER_CE)
#include "LKHolux.h"
#endif

#include "DoInits.h"


#ifdef WIN32
#ifdef UNDER_CE
bool GetBatteryInfo(BATTERYINFO* pBatteryInfo)
{
    // set default return value
    DWORD result = 0;

    // check incoming pointer
    if(NULL == pBatteryInfo)
    {
        return false;
    }

    SYSTEM_POWER_STATUS_EX2 sps;

    // request the power status
    result = GetSystemPowerStatusEx2(&sps, sizeof(sps), TRUE);

    // only update the caller if the previous call succeeded
    if(result)
    {
        pBatteryInfo->acStatus = static_cast<Battery::battery_status>(sps.ACLineStatus);
        pBatteryInfo->chargeStatus = static_cast<Battery::battery_charge_status>(sps.BatteryFlag);
        pBatteryInfo->BatteryLifePercent = sps.BatteryLifePercent;
        pBatteryInfo->BatteryVoltage = sps.BatteryVoltage;
        pBatteryInfo->BatteryAverageCurrent = sps.BatteryAverageCurrent;
        pBatteryInfo->BatteryCurrent = sps.BatteryCurrent;
        pBatteryInfo->BatterymAHourConsumed = sps.BatterymAHourConsumed;
        pBatteryInfo->BatteryTemperature = sps.BatteryTemperature;
    }

    return (result != 0);
}
#else
bool GetBatteryInfo(BATTERYINFO* pBatteryInfo) {
    // not implemented on Windows PC
    if(!pBatteryInfo) {
        return false;
    }
    pBatteryInfo->BatteryLifePercent = BATTERY_UNKNOWN;
    pBatteryInfo->acStatus = Battery::UNKNOWN;
    pBatteryInfo->chargeStatus = Battery::CHARGE_UNKNOWN;

    return false;
}
#endif
#endif

#if defined(__linux__)
#ifdef KOBO
#include "OS/FileUtil.hpp"
#include "Hardware/CPU.hpp"
#include <memory>

std::unique_ptr<ScopeLockCPU> cpu;

bool GetBatteryInfo(BATTERYINFO* pBatteryInfo)
{
    // check incoming pointer
    if(NULL == pBatteryInfo)
    {
        return false;
    }

    // set default return value
    bool result = false;

    // assume failure at entry
    pBatteryInfo->BatteryLifePercent = BATTERY_UNKNOWN;
    pBatteryInfo->acStatus = Battery::UNKNOWN;
    pBatteryInfo->chargeStatus = Battery::CHARGE_UNKNOWN;

    // code shamelessly copied from OS/SystemLoad.cpp
    char line[256];
    if (!File::ReadString("/sys/devices/platform/pmic_battery.1/power_supply/mc13892_bat/uevent",
            line, sizeof (line))) {
        return false;
    }

    char field[80], value[80];
    int n;
    char* ptr = line;
    while (sscanf(ptr, "%[^=]=%[^\n]\n%n", field, value, &n) == 2) {
        ptr += n;
        if (!strcmp(field, "POWER_SUPPLY_STATUS")) {
            if (!strcmp(value, "Not charging") || !strcmp(value, "Charging")) {
                if(!cpu) {
                    cpu = std::make_unique<ScopeLockCPU>();
                }
                pBatteryInfo->acStatus = Battery::ONLINE;
            } else if (!strcmp(value, "Discharging")) {
                cpu = nullptr;
                pBatteryInfo->acStatus = Battery::OFFLINE;
            }
        } else if (!strcmp(field, "POWER_SUPPLY_CAPACITY")) {
            result = true;
            int rem = atoi(value);
            pBatteryInfo->BatteryLifePercent = rem;
            if (pBatteryInfo->acStatus == Battery::OFFLINE) {
                cpu = nullptr;
                if (rem > 30) {
                    pBatteryInfo->chargeStatus = Battery::HIGH;
                } else if (rem >= 10) {
                    pBatteryInfo->chargeStatus = Battery::LOW;
                } else if (rem < 10) {
                    pBatteryInfo->chargeStatus = Battery::CRITICAL;
                }
            } else {
                if(!cpu) {
                    cpu = std::make_unique<ScopeLockCPU>();
                }
                pBatteryInfo->chargeStatus = Battery::CHARGING;
            }
        }
    }

    return result;
}

#elif defined(ENABLE_SDL)

#include <SDL_power.h>

bool GetBatteryInfo(BATTERYINFO* pBatteryInfo) {
    int remaining_percent = 0;

    // check incoming pointer
    if (NULL == pBatteryInfo) {
        return false;
    }

    // assume failure at entry
    pBatteryInfo->BatteryLifePercent = BATTERY_UNKNOWN;
    pBatteryInfo->acStatus = Battery::UNKNOWN;
    pBatteryInfo->chargeStatus = Battery::CHARGE_UNKNOWN;


    SDL_PowerState power_state = SDL_GetPowerInfo(NULL, &remaining_percent);
    if (remaining_percent >= 0) {
        pBatteryInfo->BatteryLifePercent = remaining_percent;
    }

    switch (power_state) {
        case SDL_POWERSTATE_CHARGING:
        case SDL_POWERSTATE_CHARGED:
            pBatteryInfo->acStatus = Battery::ONLINE;
            pBatteryInfo->chargeStatus = Battery::CHARGING;
            break;
        case SDL_POWERSTATE_ON_BATTERY:
            pBatteryInfo->acStatus = Battery::OFFLINE;
            if (remaining_percent >= 0) {
                if (remaining_percent > 30) {
                    pBatteryInfo->chargeStatus = Battery::HIGH;
                } else if (remaining_percent > 30) {
                    pBatteryInfo->chargeStatus = Battery::LOW;
                } else {
                    pBatteryInfo->chargeStatus = Battery::CRITICAL;
                }
            } else {
                pBatteryInfo->chargeStatus = Battery::CHARGE_UNKNOWN;
            }
            break;
        default:
            pBatteryInfo->chargeStatus = Battery::CHARGE_UNKNOWN;
            break;
    }

    return true;
}
#elif defined(ANDROID)
#include <jni.h>

static Battery::battery_status acStatus =  Battery::UNKNOWN;
static uint8_t BatteryLifePercent = BATTERY_UNKNOWN;
Battery::battery_charge_status chargeStatus = Battery::CHARGE_UNKNOWN;;

extern "C"
gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_LK8000_BatteryReceiver_setBatteryPercent(JNIEnv *env, jclass cls, jint value, jint plugged, jint status) {
	BatteryLifePercent = value;
	acStatus = (!plugged) ? Battery::OFFLINE : Battery::ONLINE;

	switch (status) {
		default :
		case 1 : // BATTERY_STATUS_UNKNOWN = 1;
			chargeStatus = Battery::CHARGE_UNKNOWN;
			break;
		case 2 : // BATTERY_STATUS_CHARGING = 2;
			chargeStatus = Battery::CHARGING;
			break;
		case 3 : // BATTERY_STATUS_DISCHARGING = 3;
		case 4 :  // BATTERY_STATUS_NOT_CHARGING = 4;
		case 5 :  // BATTERY_STATUS_FULL = 5;
			if (BatteryLifePercent > 30) {
				chargeStatus = Battery::HIGH;
			} else if (BatteryLifePercent > 30) {
				chargeStatus = Battery::LOW;
			} else {
				chargeStatus = Battery::CRITICAL;
			}
			break;
	}
}

bool GetBatteryInfo(BATTERYINFO* pBatteryInfo) {

	// check incoming pointer
	if (NULL == pBatteryInfo) {
		return false;
	}

	// assume failure at entry
	pBatteryInfo->BatteryLifePercent = BatteryLifePercent;
	pBatteryInfo->acStatus = acStatus;
	pBatteryInfo->chargeStatus = chargeStatus;

	return acStatus !=  Battery::UNKNOWN;
}

#else
// For now, for linux ==> TODO !
bool GetBatteryInfo(BATTERYINFO* pBatteryInfo) {

    // check incoming pointer
    if (NULL == pBatteryInfo) {
        return false;
    }

    // assume failure at entry
    pBatteryInfo->BatteryLifePercent = BATTERY_UNKNOWN;
    pBatteryInfo->acStatus = Battery::UNKNOWN;
    pBatteryInfo->chargeStatus = Battery::CHARGE_UNKNOWN;

    return false;
}

#endif
#endif



// 2015.02.07
// By default the global bool HaveBatteryInfo is always false.
// We set it true, and use battery infos, only if we find a real battery.
// Beware. If for any reason the battery no longer gives info we ought to
// clear the flag too.
static bool battInitialized=false;

void UpdateBatteryInfos(void) {

  BATTERYINFO BatteryInfo;
  BatteryInfo.acStatus = Battery::UNKNOWN;

  #ifdef PNA
  if (DeviceIsGM130) {
	PDABatteryPercent = GM130PowerLevel();
	PDABatteryStatus =  GM130PowerStatus();
	PDABatteryFlag =    GM130PowerFlag();

	PDABatteryTemperature = 0;
	HaveBatteryInfo=true;
  } else
  #endif
  if (GetBatteryInfo(&BatteryInfo)) {
    PDABatteryPercent = BatteryInfo.BatteryLifePercent;
    PDABatteryTemperature = BatteryInfo.BatteryTemperature;
    PDABatteryStatus=BatteryInfo.acStatus;
    PDABatteryFlag=BatteryInfo.chargeStatus;

    #if TESTBENCH
    if (HaveBatteryInfo==false)
        StartupStore(_T("... LKBatteryManager: HaveBatteryInfo  ENABLED%s"),NEWLINE);
    #endif
    HaveBatteryInfo=true;

    // All you need to display extra Battery informations...
    //	TCHAR vtemp[1000];
    //	_stprintf(vtemp,_T("Battpercent=%d Volt=%d Curr=%d AvCurr=%d mAhC=%d Temp=%d Lifetime=%d Fulllife=%d\n"),
    //		BatteryInfo.BatteryLifePercent, BatteryInfo.BatteryVoltage,
    //		BatteryInfo.BatteryCurrent, BatteryInfo.BatteryAverageCurrent,
    //		BatteryInfo.BatterymAHourConsumed,
    //		BatteryInfo.BatteryTemperature, BatteryInfo.BatteryLifeTime, BatteryInfo.BatteryFullLifeTime);
    //	StartupStore( vtemp );
  } else {
      #if TESTBENCH
      if (HaveBatteryInfo==true)
          StartupStore(_T("... LKBatteryManager: HaveBatteryInfo DISABLED%s"),NEWLINE);
      #endif
      HaveBatteryInfo=false;

  }
  battInitialized=true;
}


static int  numwarn=0;
extern bool GiveBatteryWarnings(void);

void LKBatteryManager() {

  static bool invalid=false, recharging=false;
  static bool warn33=true, warn50=true, warn100=true;
  static double last_time=0, init_time=0;
  static int last_percent=0, last_status=0;

  if (!battInitialized) {
	#if TESTBENCH
	StartupStore(_T("... LKBatteryManager waiting for first update%s"),NEWLINE);
	#endif
        return;
  }
  if (invalid) return;
  if (DoInit[MDI_BATTERYMANAGER]) {

	invalid=false, recharging=false;
	warn33=true, warn50=true, warn100=true;
	last_time=0, init_time=0;
	last_percent=0, last_status=0;
	numwarn=0;

	if (!HaveBatteryInfo|| PDABatteryPercent<1 || PDABatteryPercent>100) {
		StartupStore(_T("... LK BatteryManager V1: internal battery information not available, function disabled%s"), NEWLINE);
		invalid=true;
		DoInit[MDI_BATTERYMANAGER]=false; // just to be sure
		return;
	}

	StartupStore(_T(". LK Battery Manager V1 started, current charge=%d%%%s"),PDABatteryPercent,NEWLINE);
	init_time=GPS_INFO.Time;
	DoInit[MDI_BATTERYMANAGER]=false;
  }


  // if first run,  and not passed 30 seconds, do nothing
  if (last_percent==0 && (GPS_INFO.Time<(init_time+30))) {
	// StartupStore(_T("... first run, waiting for 30s\n"));
	return;
  }

  TCHAR mbuf[100];

  // first run after 30 seconds: give a message
  if (last_percent==0) {
	// StartupStore(_T("... first run, last percent=0\n"));
	if (PDABatteryPercent <=50) {
		last_time=GPS_INFO.Time;
		// LKTOKEN _@M1352_ "BATTERY LEVEL"
		_stprintf(mbuf,_T("%s %d%%"), MsgToken(1352), PDABatteryPercent);
		DoStatusMessage(mbuf);
		warn50=false;
	}
	// special case, pdabattery is 0...
	if (PDABatteryPercent <1) {
		StartupStore(_T("... LK Battery Manager disabled, low battery %s"),NEWLINE);
		// LKTOKEN _@M1353_ "BATTERY MANAGER DISABLED"
		DoStatusMessage(MsgToken(1353));
		invalid=true;
		return;
	} else
		last_percent=PDABatteryPercent;

	if (PDABatteryStatus!=Battery::UNKNOWN) {
		last_status=PDABatteryStatus;
	}
	// StartupStore(_T("... last_percent first assigned=%d\n"),last_percent);
	return;
  }

  if (PDABatteryStatus!=Battery::UNKNOWN) {
	if (last_status != PDABatteryStatus) {
		if (PDABatteryStatus==Battery::OFFLINE) {
			if (GiveBatteryWarnings())
	// LKTOKEN  _@M514_ = "POWER SUPPLY OFF"
			DoStatusMessage(MsgToken(514));
		} else {
			if (PDABatteryStatus==Battery::ONLINE) {
				if (GiveBatteryWarnings())
	// LKTOKEN  _@M515_ = "POWER SUPPLY ON"
				DoStatusMessage(MsgToken(515));
			} else {
				if (PDABatteryStatus==Battery::BACKUP_POWER) {
					if (GiveBatteryWarnings())
	// LKTOKEN  _@M119_ = "BACKUP POWER SUPPLY ON"
					DoStatusMessage(MsgToken(119));
				}
			}
		}
	}
	last_status=PDABatteryStatus;
  }

  // Only check every 5 minutes normally
  if (GPS_INFO.Time<(last_time+(60*5))) return;

  // if battery is recharging, reset warnings and do nothing
  if (last_percent<PDABatteryPercent) {
	warn33=true;
	warn50=true;
	warn100=true;
	last_percent=PDABatteryPercent;
	if (!recharging) {
		recharging=true;
		if (PDABatteryFlag==Battery::CHARGING || PDABatteryStatus==Battery::ONLINE) {
			if (GiveBatteryWarnings())
	// LKTOKEN  _@M124_ = "BATTERY IS RECHARGING"
			DoStatusMessage(MsgToken(124));
			last_time=GPS_INFO.Time;
		}
	}
	return;
  }

  // if battery is same level, do nothing except when 100% during recharge
  if (last_percent == PDABatteryPercent) {
	if (recharging && (PDABatteryPercent==100) && warn100) {
		if (GiveBatteryWarnings())
	// LKTOKEN  _@M123_ = "BATTERY 100% CHARGED"
		DoStatusMessage(MsgToken(123));
		warn100=false;
		last_time=GPS_INFO.Time;
	}
	return;
  }

  // else battery is discharging
  recharging=false;

  // Time to give a message to the user, if necessary
  if (PDABatteryPercent <=5) {
	// LKTOKEN _@M1354_ "BATTERY LEVEL CRITIC!"
	_stprintf(mbuf,_T("%d%% %s"), PDABatteryPercent, MsgToken(1354));
	DoStatusMessage(mbuf);
    LKSound(TEXT("LK_RED.WAV"));

	// repeat after 1 minute, forced
	last_time=GPS_INFO.Time-(60*4);
	last_percent=PDABatteryPercent;
	return;
  }
  if (PDABatteryPercent <=10) {
	// LKTOKEN _@M1355_ "BATTERY LEVEL VERY LOW!"
	_stprintf(mbuf,_T("%d%% %s"), PDABatteryPercent, MsgToken(1355));
	DoStatusMessage(mbuf);
	// repeat after 2 minutes, forced
	last_time=GPS_INFO.Time-(60*3);
	last_percent=PDABatteryPercent;
	return;
  }
  if (PDABatteryPercent <=20) {
	// LKTOKEN _@M1356_ "BATTERY LEVEL LOW!"
	_stprintf(mbuf,_T("%d%% %s"), PDABatteryPercent, MsgToken(1356));
	DoStatusMessage(mbuf);
	last_time=GPS_INFO.Time;
	last_percent=PDABatteryPercent;
	return;
  }

  if (PDABatteryPercent <=30) {
	if (warn33) {
		// LKTOKEN _@M1352_ "BATTERY LEVEL"
		_stprintf(mbuf, _T("%s %d%%"), MsgToken(1352), PDABatteryPercent);
		DoStatusMessage(mbuf);
		warn33=false;
	}
	last_time=GPS_INFO.Time;
	last_percent=PDABatteryPercent;
	return;
  }
  // DISABLED
  if (PDABatteryPercent <=50) {
	if (warn50) {
		// LKTOKEN _@M1352_ "BATTERY LEVEL"
	//	_stprintf(mbuf, _T("%s %d%%"), MsgToken(1352), PDABatteryPercent);
	//	DoStatusMessage(mbuf);
		warn50=false;
	}
	last_time=GPS_INFO.Time;
	last_percent=PDABatteryPercent;
	return;
  }
}

// returns true if no problems with too many warnings
#define MAXBATTWARN   15
bool GiveBatteryWarnings(void)
{
  static bool toomany=false;
  static double last_time=0;

  // If last warning was issued more than 60 minutes ago, reset toomany.
  if (GPS_INFO.Time>(last_time+3600)) {
	#if TESTBENCH
	if (last_time>0 && numwarn>0)
		StartupStore(_T("... GiveBatteryWarnings resetting at %s%s"),WhatTimeIsIt(),NEWLINE);
	#endif
	toomany=false;
	numwarn=0;
  }

  if (toomany) return false;

  numwarn++;

  if (numwarn>MAXBATTWARN) {
	// LKTOKEN _@M1357_ "BATTERY WARNINGS DISABLED"
	DoStatusMessage(MsgToken(1357));
	StartupStore(_T("... Too many battery warnings, disabling Battery Manager at %s%s"),WhatTimeIsIt(),NEWLINE);
	toomany=true;
	return false;
  }
  last_time=GPS_INFO.Time;
  return true;
}
