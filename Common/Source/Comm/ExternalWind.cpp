/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ExternalWind.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 18 Avril 2024
 */

#include "externs.h"
#include <numeric>
#include "ExternalWind.h"
#include "../NMEA/Info.h"
#include "Logger.h"

void UpdateExternalWind(NMEA_INFO& Info, const DeviceDescriptor_t& d, double Speed, double Direction) {
  Info.ExternalWind.update(d, {Speed, AngleLimit360(Direction)});
}

void ResetExternalWindAvailable(NMEA_INFO& Info) {
  Info.ExternalWind.reset();
}

bool ExternalWindAvailable(const NMEA_INFO& Info) {
  return (!ReplayLogger::IsEnabled()) && Info.ExternalWind.available();
}
