
/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Info.cpp
 */

#include "Info.h"
#include <limits>
#include "Logger.h"
#include "../Comm/device.h"

void ResetHeartRateAvailable(NMEA_INFO& info) {
    info.HeartRateIdx = NUMDEV;
}

bool HeartRateAvailable(const NMEA_INFO& info) {
  return (!ReplayLogger::IsEnabled()) && (info.HeartRateIdx < NUMDEV);
}

void UpdateHeartRate(NMEA_INFO& info, const DeviceDescriptor_t& d, unsigned bpm) {
  if (d.PortNumber <= info.HeartRateIdx) {
    info.HeartRateIdx = d.PortNumber;
    info.HeartRate = bpm;
  }
}
