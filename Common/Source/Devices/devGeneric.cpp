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

using namespace std::chrono_literals;

namespace  {

BOOL OnHeartRate(DeviceDescriptor_t& d, NMEA_INFO& info, unsigned bpm) {
  ScopeLock lock(CritSec_FlightData);
  info.HeartRate.update(d, bpm);
  return TRUE;
}

BOOL OnBarometricPressure(DeviceDescriptor_t& d, NMEA_INFO& info, double Pa) {
  ScopeLock lock(CritSec_FlightData);
  UpdateBaroSource(&info, &d, StaticPressureToQNHAltitude(Pa));
  return TRUE;
}

BOOL OnOutsideTemperature(DeviceDescriptor_t& d, NMEA_INFO& info, double temp) {
  ScopeLock lock(CritSec_FlightData);
  info.OutsideAirTemperature.update(d, temp);
  return TRUE;
}

BOOL OnRelativeHumidity(DeviceDescriptor_t& d, NMEA_INFO& info, double hr) {
  ScopeLock lock(CritSec_FlightData);
  info.RelativeHumidity.update(d, hr);
  return TRUE;
}

BOOL OnWindOriginDirection(DeviceDescriptor_t& d, NMEA_INFO& info, double direction) {
  ScopeLock lock(CritSec_FlightData);
  UpdateExternalWind(info, d, info.ExternalWind.value().Speed, direction);
  return TRUE;
}

BOOL OnWindSpeed(DeviceDescriptor_t& d, NMEA_INFO& info, double speed) {
  ScopeLock lock(CritSec_FlightData);
  UpdateExternalWind(info, d, speed, info.ExternalWind.value().Direction);
  return TRUE;
}

BOOL OnBatteryLevel(DeviceDescriptor_t& d, NMEA_INFO& info, double level) {
  ScopeLock lock(CritSec_FlightData);
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
  d->OnHeartRate = OnHeartRate;
  d->OnBarometricPressure = OnBarometricPressure;
  d->OnOutsideTemperature = OnOutsideTemperature;
  d->OnRelativeHumidity = OnRelativeHumidity;
  d->OnWindOriginDirection = OnWindOriginDirection;
  d->OnWindSpeed = OnWindSpeed;
  d->OnBatteryLevel = OnBatteryLevel;
  d->OnAcceleration = OnAcceleration;
}

void internalInstall(DeviceDescriptor_t* d){
  d->OnAcceleration = OnAcceleration;
}
