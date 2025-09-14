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
#include "Calc/Vario.h"
#include "Units.h"

extern Mutex CritSec_FlightData;

namespace {

void Vario(DeviceDescriptor_t& d, NMEA_INFO& info,
           const std::vector<uint8_t>& data) {
  auto value = Units::From(unDecimeterPersecond,
                           characteristic_value<int32_t>(data).get());

  ScopeLock lock(CritSec_FlightData);
  UpdateVarioSource(info, d, value);
}

void VarioNetto(DeviceDescriptor_t& d, NMEA_INFO& info,
                const std::vector<uint8_t>& data) {
  auto value = Units::From(unDecimeterPersecond,
                           characteristic_value<int32_t>(data).get());

  ScopeLock lock(CritSec_FlightData);
  // TODO : implement device Priority
  info.NettoVarioAvailable = true;
  info.NettoVario = value;
}

void TAS(DeviceDescriptor_t& d, NMEA_INFO& info,
         const std::vector<uint8_t>& data) {
  auto value = Units::From(unDecimeterPersecond,
                           characteristic_value<int32_t>(data).get());

  ScopeLock lock(CritSec_FlightData);
  // TODO : implement device Priority
  info.AirspeedAvailable = true;
  info.TrueAirspeed = value;
}

void IAS(DeviceDescriptor_t& d, NMEA_INFO& info,
         const std::vector<uint8_t>& data) {
  auto value = Units::From(unDecimeterPersecond,
                           characteristic_value<int32_t>(data).get());

  ScopeLock lock(CritSec_FlightData);
  // TODO : implement device Priority
  info.AirspeedAvailable = true;
  info.IndicatedAirspeed = value;
}

void Azimut(DeviceDescriptor_t& d, NMEA_INFO& info,
         const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint16_t>(data).get();

  ScopeLock lock(CritSec_FlightData);
  // TODO : implement device Priority
  info.MagneticHeadingAvailable = true;
  info.MagneticHeading = value;
}

void GLoad(DeviceDescriptor_t& d, NMEA_INFO& info,
            const std::vector<uint8_t>& data) {
  auto value = characteristic_value<int32_t>(data).get();
  ScopeLock lock(CritSec_FlightData);
  UpdateGLoad(info, d, value / 10.0);
}

#ifndef NDEBUG
void VarioMode(DeviceDescriptor_t& d, NMEA_INFO& info,
           const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint8_t>(data).get();
  DebugLog(_T("VarioMode : %d"), value);
  // TODO : 0 = Classic, 1 = Total energy
}
#endif

using OnGattCharacteristicT = std::function<void(
    DeviceDescriptor_t&, NMEA_INFO&, const std::vector<uint8_t>&)>;

struct DataHandlerT {
  OnGattCharacteristicT OnGattCharacteristic;
};

using service_table_t = bluetooth::service_table_t<DataHandlerT>;

const service_table_t& service_table() {
  using bluetooth::gatt_uuid;
  static const service_table_t table = {
      // BLE SERVICE Vector Vario
      {{"2fce4890-0197-47e0-a825-d4777b9a5d67",
        {{
            {"2fce4891-0197-47e0-a825-d4777b9a5d67", {&Vario}},
            {"2fce4892-0197-47e0-a825-d4777b9a5d67", {&TAS}},
            {"2fce4893-0197-47e0-a825-d4777b9a5d67", {&IAS}},
            {"2fce4895-0197-47e0-a825-d4777b9a5d67", {&GLoad}},
            {"2fce4896-0197-47e0-a825-d4777b9a5d67", {&Azimut}},
            {"2fce4902-0197-47e0-a825-d4777b9a5d67", {&VarioNetto}},
#ifndef NDEBUG
            {"2fce4903-0197-47e0-a825-d4777b9a5d67", {&VarioMode}},
#endif
        }}}}};
  return table;
}

bool DoEnableGattCharacteristic(DeviceDescriptor_t& d, uuid_t service,
                                uuid_t characteristic) {
  return service_table().get(service, characteristic);
}

void OnGattCharacteristic(DeviceDescriptor_t& d, NMEA_INFO& info,
                          uuid_t service, uuid_t characteristic,
                          const std::vector<uint8_t>& data) {
  auto handler = service_table().get(service, characteristic);
  if (handler) {
    std::invoke(handler->OnGattCharacteristic, d, info, data);
  }
}

}  // namespace

void VectorVario::Install(DeviceDescriptor_t* d) {
  genInstall(d);  // install Generic driver callback first

  d->DoEnableGattCharacteristic = DoEnableGattCharacteristic;
  d->OnGattCharacteristic = OnGattCharacteristic;
}
