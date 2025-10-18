
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

void ResetAccelerationAvailable(NMEA_INFO& info) {
  info.AccelerationIdx = NUMDEV;
}

bool AccelerationAvailable(const NMEA_INFO& info) {
  return info.AccelerationIdx < NUMDEV;
}
