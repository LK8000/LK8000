/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "devGeneric.h"

namespace  {

BOOL OnHeartRate(DeviceDescriptor_t& d, NMEA_INFO& info, unsigned bpm) {
  UpdateHeartRate(info, d, bpm);
  return TRUE;
}

BOOL OnBarometricPressure(DeviceDescriptor_t& d, NMEA_INFO& info, double Pa) {
  UpdateBaroSource(&info, &d, StaticPressureToQNHAltitude(Pa));
  return TRUE;
}

BOOL OnOutsideTemperature(DeviceDescriptor_t& d, NMEA_INFO& info, double temp) {
  info.TemperatureAvailable = TRUE;
  info.OutsideAirTemperature = temp;
  return TRUE;
}

BOOL OnBatteryLevel(DeviceDescriptor_t& d, NMEA_INFO& info, double level) {
  switch (d.PortNumber) {
    case 0:
      info.ExtBatt1_Voltage = level + 1000;
      break;
    case 1:
      info.ExtBatt2_Voltage = level + 1000;
      break;
  }
  return TRUE;
}

} // namespace

void genInstall(DeviceDescriptor_t* d) {
  d->OnHeartRate = OnHeartRate;
  d->OnBarometricPressure = OnBarometricPressure;
  d->OnOutsideTemperature = OnOutsideTemperature;
  d->OnBatteryLevel = OnBatteryLevel;
}

void internalInstall(DeviceDescriptor_t* d) {

}
