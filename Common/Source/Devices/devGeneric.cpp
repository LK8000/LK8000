/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "devGeneric.h"
#include "Comm/ExternalWind.h"
#include <numeric>
#include <deque>
#include <chrono>
#include "Comm/Bluetooth/gatt_utils.h"
#include "Comm/Bluetooth/characteristic_value.h"

using namespace std::chrono_literals;

namespace  {

void OnHeartRate(DeviceDescriptor_t& d, NMEA_INFO& info, const std::vector<uint8_t>& data) {
  if (!data.empty()) {
    auto bpm = [&]() -> uint32_t {
      if (data[0] & 0x01) {
        return characteristic_value<uint16_t>(data).get(1);
      }
      else {
        return characteristic_value<uint8_t>(data).get(1);
      }
    }();

    const std::lock_guard lock(CritSec_FlightData);
    info.HeartRate.update(d, bpm);
  }
}

void OnBarometricPressure(DeviceDescriptor_t& d, NMEA_INFO& info, const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint32_t>(data).get();
  const std::lock_guard lock(CritSec_FlightData);
  UpdateBaroSource(&info, &d, StaticPressureToQNHAltitude(value / 10.));
}

void OnOutsideTemperature(DeviceDescriptor_t& d, NMEA_INFO& info, const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint16_t>(data).get();
  if (value != 0x8000) {
    const std::lock_guard lock(CritSec_FlightData);
    info.OutsideAirTemperature.update(d, static_cast<int16_t>(value) / 100.);
  }
}

void OnRelativeHumidity(DeviceDescriptor_t& d, NMEA_INFO& info, const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint16_t>(data).get();
  if (value < 10000) {
    const std::lock_guard lock(CritSec_FlightData);
    info.RelativeHumidity.update(d, value / 100.);
  }
}

void OnWindOriginDirection(DeviceDescriptor_t& d, NMEA_INFO& info, const std::vector<uint8_t>& data) {
  auto value = characteristic_value<uint16_t>(data).get();
  if (value < 35999) {
    const std::lock_guard lock(CritSec_FlightData);
    UpdateExternalWind(info, d, info.ExternalWind.value().Speed, value / 100.);
  }
}

void OnWindSpeed(DeviceDescriptor_t& d, NMEA_INFO& info, const std::vector<uint8_t>& data) {
  auto value = characteristic_value<int16_t>(data).get();
  double speed = Units::From(Units_t::unCentimeterPersecond, value);
  const std::lock_guard lock(CritSec_FlightData);
  UpdateExternalWind(info, d, speed, info.ExternalWind.value().Direction);
}

void OnGattBatteryLevel(DeviceDescriptor_t& d, NMEA_INFO& info, const std::vector<uint8_t>& data) {
  auto level = characteristic_value<uint8_t>(data).get(0);
  d.OnBatteryLevel(d, info, level);
}

void Hm10Data(DeviceDescriptor_t& d, NMEA_INFO&, const std::vector<uint8_t>& data) {
  d.Com->ProcessData(reinterpret_cast<const char*>(data.data()), data.size());
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

class AccelerationData final : public DriverData {
 public:
  using clock = std::chrono::steady_clock;
  using time_point = clock::time_point;

  void add(time_point time, Point3D value) {
    datas.emplace_back(time, value);

    // keep only last 1 second of data
    auto cutoff = time - 1s;
    while (!datas.empty() && std::get<0>(datas.front()) < cutoff) {
      datas.erase(datas.begin());
    }
  }

  Point3D average() const {
    if (datas.empty()) {
      return {};
    }

    Point3D sum = {};
    for (const auto& item : datas) {
      sum = sum + std::get<1>(item);
    }
    return sum / static_cast<double>(datas.size());
  }

 private:

  std::deque<std::tuple<time_point, Point3D>> datas;
};

constexpr unsigned TagAcceleration = 0;

BOOL OnAcceleration(DeviceDescriptor_t& d, NMEA_INFO& info, uint64_t timestamp,
                    double gx, double gy, double gz) {
  using nanoseconds = std::chrono::nanoseconds;

  auto data = d.get_data<AccelerationData>(TagAcceleration);
  if (data) {
    AccelerationData::time_point time{nanoseconds(timestamp)};
    data->add(time, {gx, gy, gz});

    info.Acceleration.update(d, data->average());
  }
  return TRUE;
}

} // namespace

void genInstall(DeviceDescriptor_t* d) {
  using bluetooth::gatt_uuid;

  d->OnAcceleration = OnAcceleration;
  d->OnBatteryLevel = OnBatteryLevel;

  d->GattAppend({
    { gatt_uuid(0x180D), {{ // Heart Rate
        { gatt_uuid(0x2A37), { &OnHeartRate }}
    }}},
    { gatt_uuid(0x180F), {{ // Battery Service
        { gatt_uuid(0x2A19), { &OnGattBatteryLevel }}
    }}},
    { gatt_uuid(0x181A), {{ // Environmental Sensing Service
        { gatt_uuid(0x2A6D), { &OnBarometricPressure }},
        { gatt_uuid(0x2A6E), { &OnOutsideTemperature }},
        { gatt_uuid(0x2A6F), { &OnRelativeHumidity }},
        { gatt_uuid(0x2A70), { &OnWindSpeed }},
        { gatt_uuid(0x2A71), { &OnWindOriginDirection }},
    }}},
    { gatt_uuid(0xFFE0), {{ // HM-10 and compatible bluetooth modules
        { gatt_uuid(0xFFE1), { &Hm10Data }}, // HM-10 RX
        { gatt_uuid(0xFFE4), { &Hm10Data }}, // SkyDrop2 RX
    }}},
    { bluetooth::NUS_SERVICE, {{ // Nordic UART Service
        { bluetooth::NUS_TX_CHARACTERISTIC, { &Hm10Data }} // RX
    }}}
  });
}

void internalInstall(DeviceDescriptor_t* d){
  d->OnAcceleration = OnAcceleration;
}
