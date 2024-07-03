/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devFlyBeeper.cpp
 * Author: Bruno de Lacheisserie
 */
#include "devFlyBeeper.h"
#include "Comm/device.h"
#include "devGeneric.h"
#include "Utils.h"

namespace {

bool EnableGattCharacteristic(uuid_t service, uuid_t characteristic) {
  if (service == "00001819-0000-1000-8000-00805F9B34FB") { // Location and Navigation service
    return (characteristic == "234337BF-F931-4D2D-A13C-07E2F06A0249"); // TAS
  }
  return false;
}

void OnGattCharacteristic(DeviceDescriptor_t& d, NMEA_INFO& info, uuid_t service,
                             uuid_t characteristic, const std::vector<uint8_t>& data) {
  if (service == "00001819-0000-1000-8000-00805F9B34FB") { // Location and Navigation service
    if (characteristic == "234337BF-F931-4D2D-A13C-07E2F06A0249") {
      // TAS
      if (data.size() == 2) {
        info.AirspeedAvailable = TRUE;
        info.TrueAirspeed = FromLE16(*reinterpret_cast<const int16_t*>(data.data())) / 10.;
        info.IndicatedAirspeed = IndicatedAirSpeed(info.TrueAirspeed, QNHAltitudeToQNEAltitude(info.Altitude));
      }
    }
  }
}

} // namespace

void FlyBeeper::Install(DeviceDescriptor_t* d) {
  genInstall(d); // install Generic driver callback first
  _tcscpy(d->Name, Name);

  d->EnableGattCharacteristic = EnableGattCharacteristic;
  d->OnGattCharacteristic = OnGattCharacteristic;
}
