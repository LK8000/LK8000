/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devVectorVario.cpp
 * Author: Bruno de Lacheisserie
 */
#include "devVectorVario.h"
#include "Comm/device.h"
#include "devGeneric.h"
#include "Comm/Bluetooth/gatt_utils.h"
#include "Comm/Bluetooth/characteristic_value.h"
#include "Comm/Bluetooth/gatt_service_table.h"
#include "Calc/Vario.h"
#include "Units.h"

extern Mutex CritSec_FlightData;

namespace {

void GattVario(DeviceDescriptor_t& d, NMEA_INFO& info,
           const std::vector<uint8_t>& data) {
  auto value = Units::From(unDecimeterPersecond,
                           characteristic_value<int32_t>(data).get());

  const std::lock_guard lock(CritSec_FlightData);
  UpdateVarioSource(info, d, value);
}

void VarioNetto(DeviceDescriptor_t& d, NMEA_INFO& info,
                const std::vector<uint8_t>& data) {
  auto value = Units::From(unDecimeterPersecond,
                           characteristic_value<int32_t>(data).get());

  const std::lock_guard lock(CritSec_FlightData);
  info.NettoVario.update(d, value);
}

void TAS(DeviceDescriptor_t& d, NMEA_INFO& info,
         const std::vector<uint8_t>& data) {
  auto value = Units::From(unDecimeterPersecond,
                           characteristic_value<int32_t>(data).get());

  const std::lock_guard lock(CritSec_FlightData);
  info.TrueAirSpeed.update(d, value);
}

void IAS(DeviceDescriptor_t& d, NMEA_INFO& info,
         const std::vector<uint8_t>& data) {
  auto value = Units::From(unDecimeterPersecond,
                           characteristic_value<int32_t>(data).get());

  const std::lock_guard lock(CritSec_FlightData);
  info.IndicatedAirSpeed.update(d, value);
}

void Azimuth(DeviceDescriptor_t& d, NMEA_INFO& info,
         const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint16_t>(data).get();

  const std::lock_guard lock(CritSec_FlightData);
  info.MagneticHeading.update(d, value);
}

void GLoad(DeviceDescriptor_t& d, NMEA_INFO& info,
            const std::vector<uint8_t>& data) {
  auto value = characteristic_value<int32_t>(data).get();
  const std::lock_guard lock(CritSec_FlightData);
  info.Gload.update(d, value / 10.0);
}

void Roll(DeviceDescriptor_t& d, NMEA_INFO& info,
            const std::vector<uint8_t>& data) {
  double value = characteristic_value<int16_t>(data).get()  / 10.;
  const std::lock_guard lock(CritSec_FlightData);
  GyroscopeData GyroData = info.Gyroscope.value(); 
  GyroData.Roll = value;
  info.Gyroscope.update(d, std::move(GyroData));
}

void Pitch(DeviceDescriptor_t& d, NMEA_INFO& info,
            const std::vector<uint8_t>& data) {
  double value = characteristic_value<int16_t>(data).get() / 10.;
  const std::lock_guard lock(CritSec_FlightData);
  GyroscopeData GyroData = info.Gyroscope.value(); 
  GyroData.Pitch = value;
  info.Gyroscope.update(d, std::move(GyroData));
}

#ifndef NDEBUG
void VarioMode(DeviceDescriptor_t& d, NMEA_INFO& info,
           const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint8_t>(data).get();
  DebugLog(_T("VarioMode : %d"), value);
  // TODO : 0 = Classic, 1 = Total energy
}
#endif

}  // namespace

void VectorVario::Install(DeviceDescriptor_t* d) {
  genInstall(d);  // install Generic driver callback first

  d->GattAppend({
      { "2fce4890-0197-47e0-a825-d4777b9a5d67", {{ // BLE SERVICE Vector Vario
            { "2fce4891-0197-47e0-a825-d4777b9a5d67", { &GattVario }},
            { "2fce4892-0197-47e0-a825-d4777b9a5d67", { &TAS }},
            { "2fce4893-0197-47e0-a825-d4777b9a5d67", { &IAS }},
            { "2fce4895-0197-47e0-a825-d4777b9a5d67", { &GLoad }},
            { "2fce4896-0197-47e0-a825-d4777b9a5d67", { &Azimuth }},
            { "2fce4902-0197-47e0-a825-d4777b9a5d67", { &VarioNetto }},
            { "2fce4897-0197-47e0-a825-d4777b9a5d67", { &Roll }},
            { "2fce4898-0197-47e0-a825-d4777b9a5d67", { &Pitch }},
#ifndef NDEBUG
            { "2fce4903-0197-47e0-a825-d4777b9a5d67", { &VarioMode }},
#endif
        }}
      }});
}
