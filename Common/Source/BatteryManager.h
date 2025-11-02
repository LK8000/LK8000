/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights
 */

#ifndef BATTERYMANAGER_H
#define BATTERYMANAGER_H

#include "lk8000.h"
#include "Time/PeriodClock.hpp"

namespace Battery {

#ifndef WIN32
#define AC_LINE_OFFLINE 0
#define AC_LINE_ONLINE 1
#define AC_LINE_BACKUP_POWER 2
#define AC_LINE_UNKNOWN 255

#define BATTERY_FLAG_HIGH 1
#define BATTERY_FLAG_LOW 2
#define BATTERY_FLAG_CRITICAL 4
#define BATTERY_FLAG_CHARGING 8
#define BATTERY_FLAG_NO_BATTERY 128
#define BATTERY_FLAG_UNKNOWN 255
#endif

#define BATTERY_UNKNOWN 255

enum battery_status : uint8_t {
  OFFLINE = AC_LINE_OFFLINE,
  ONLINE = AC_LINE_ONLINE,
  BACKUP_POWER = AC_LINE_BACKUP_POWER,
  UNKNOWN = AC_LINE_UNKNOWN
};

enum battery_charge_status : uint8_t {
  HIGH = BATTERY_FLAG_HIGH,
  LOW = BATTERY_FLAG_LOW,
  CRITICAL = BATTERY_FLAG_CRITICAL,
  CHARGING = BATTERY_FLAG_CHARGING,
  NO_BATTERY = BATTERY_FLAG_NO_BATTERY,
  CHARGE_UNKNOWN = BATTERY_FLAG_UNKNOWN
};

}  // namespace Battery

struct BATTERYINFO {
  Battery::battery_status acStatus;
  // 0 offline
  // 1 online
  // 255 unknown
  Battery::battery_charge_status chargeStatus;
  // 1 high
  // 2 low
  // 4 critical
  // 8 charging
  // 128 no system battery
  // 255 unknown
  uint8_t BatteryLifePercent;
  // 0-100 or 255 if unknown

  // VENTA-TEST BATTERY
  uint32_t BatteryVoltage;
  uint32_t BatteryCurrent;
  uint32_t BatteryAverageCurrent;
  uint32_t BatterymAHourConsumed;
  uint32_t BatteryTemperature;
  uint32_t BatteryLifeTime;
  uint32_t BatteryFullLifeTime;
  // END VENTA-TEST
};


class BatteryManager final {
 public:
  BatteryManager() = default;

  /**
   * @brief Updates battery information and manages battery-related warnings.
   * This should be called periodically.
   */
  void Update();

#ifdef ANDROID
  /**
   * @brief Updates the battery state from Android's broadcast receiver.
   * @param percent The battery percentage.
   * @param plugged The AC connection status.
   * @param status The battery charge status.
   */
  void SetBatteryState(int percent, int plugged, int status);
#endif

 private:
  /**
   * @brief Platform-specific method to get battery information.
   * @param pBatteryInfo Pointer to a BATTERYINFO struct to be filled.
   * @return True if battery information was successfully retrieved, false
   * otherwise.
   */
  bool GetBatteryInfo(BATTERYINFO& pBatteryInfo);

  /**
   * @brief Manages battery state and triggers warnings.
   */
  void Manage();

  /**
   * @brief Checks if it's appropriate to issue a battery warning.
   * @return True if a warning can be issued, false if warnings are suppressed.
   */
  bool GiveBatteryWarnings();

  // State from LKBatteryManager
  bool invalid = false;
  bool recharging = false;
  bool warn33 = true;
  bool warn100 = true;
  double last_time = 0;
  double init_time = 0;
  int last_percent = 0;

  int last_status = Battery::UNKNOWN;

  // State from GiveBatteryWarnings
  int numwarn = 0;
  bool toomany_warnings = false;
  double last_warning_time = 0;

  // State from UpdateBatteryInfos
  bool initialized = false;

#ifdef ANDROID
  // Android-specific battery state
  Mutex android_battery_mutex;
  Battery::battery_status android_acStatus = Battery::UNKNOWN;
  uint8_t android_BatteryLifePercent = BATTERY_UNKNOWN;
  Battery::battery_charge_status android_chargeStatus = Battery::CHARGE_UNKNOWN;
#endif
};

extern BatteryManager g_BatteryManager;

#endif  // BATTERYMANAGER_H