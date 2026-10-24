/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devVectorVario.cpp
 * Author: Bruno de Lacheisserie
 */
#include "devVectorVario.h"
#include <charconv>
#include <cmath>
#include "Comm/device.h"
#include "devGeneric.h"
#include "Comm/Bluetooth/gatt_utils.h"
#include "Comm/Bluetooth/characteristic_value.h"
#include "Comm/Bluetooth/gatt_service_table.h"
#include "Calc/Vario.h"
#include "Units.h"
#include "GliderPolar/Polar.h"
#include "utils/charset_helper.h"
#include "McReady.h"

extern Mutex CritSec_FlightData;
extern TCHAR szPolarName[80];

namespace {

using namespace std::string_view_literals;

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

std::vector<std::string_view> SplitNmea(std::string_view sv) {
  std::vector<std::string_view> fields;
  size_t start = 0;
  while (start <= sv.size()) {
    size_t pos = sv.find(',', start);
    if (pos == std::string_view::npos) {
      fields.emplace_back(sv.substr(start));
      break;
    }
    fields.emplace_back(sv.substr(start, pos - start));
    start = pos + 1;
  }
  return fields;
}

enum XCTODFields : unsigned {
  FW_VERSION = 7,
  TAKEOFF,
  SD_CARD,
  SOUND_LEVEL,
  PILOT_NAME,
  PILOT_ID,
  CONFIG,
  SOUND_GND,
  SOUND_BLE,
  OPTION_RR,
  NETTO_PLUS,
  NETTO_MINUS,
  NETTO_GLIDE,
  CALCULATION_MODE,
  INTEGRATION_TIME,
  WING_CHOICE,
  GLIDER_MODEL,
  GLIDER_ID,
  PROJECTED_SURFACE,
  FLAT_ASPECT_RATIO,
  HARNESS,
  AUW
};

double ToDouble(std::string_view sv) {
  double value;
  auto res = std::from_chars(sv.data(), sv.data() + sv.size(), value);
  if (res.ec != std::errc{} || !std::isfinite(value)) {
    throw std::runtime_error("Failed to convert string to double");
  }
  return value;
}

BOOL XTOD(DeviceDescriptor_t* d, std::string_view sv, NMEA_INFO* GPS_INFO) {
  try {
    std::vector<std::string_view> fields = SplitNmea(sv);
    if (fields.size() <= AUW) {
      return FALSE;  // Not enough fields
    }
    auto ps = ToDouble(fields[PROJECTED_SURFACE]);
    auto ar = ToDouble(fields[FLAT_ASPECT_RATIO]);
    auto harness = fields[HARNESS];
    auto auw = ToDouble(fields[AUW]);

    if (ps <= 0 || ar <= 0 || auw <= 0) {
      return FALSE;  // Invalid data
    }

    const std::lock_guard lock(CritSec_FlightData);
    if (GliderPolar::Update(ps, ar, auw, harness)) {
      std::string model(fields[GLIDER_MODEL]);
      from_unknown_charset(model.c_str(), szPolarName);
      GlidePolar::WingArea = ps;
      GlidePolar::SetBallast();
    }
  }
  catch (const std::exception& e) {
    DebugLog(_T("VectorVario XTOD parse error: %s"),
             to_tstring(e.what()).c_str());
    return FALSE;
  }
  return TRUE;
}

BOOL ParseNMEA(DeviceDescriptor_t* d, const char* String, NMEA_INFO* GPS_INFO) {
  std::string_view sv(String);
  if (sv.starts_with("$XCTOD,"sv)) {
    return XTOD(d, sv, GPS_INFO);
  }
  return FALSE;
}

}  // namespace

void VectorVario::Install(DeviceDescriptor_t* d) {
  using bluetooth::gatt_uuid;

  genInstall(d);  // install Generic driver callback first

  d->ParseNMEA = ParseNMEA;

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
