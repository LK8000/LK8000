/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devFlyBeeper.cpp
 * Author: Bruno de Lacheisserie
 */
#include "externs.h"
#include "devFlyBeeper.h"
#include "Comm/device.h"
#include "devGeneric.h"
#include "Utils.h"
#include "Util/Clamp.hpp"
#include "Fanet/frame.h"
#include "utils/lookup_table.h"
#include <random>
#include "DeviceSettings.h"

#undef uuid_t

namespace {

enum aircraft_t : uint8_t {
  otherAircraft = 0,
  paraglider = 1,
  hangglider = 2,
  balloon = 3,
  glider = 4,
  poweredAircraft = 5,
  helicopter = 6,
  uav = 7,
};

aircraft_t type() {
  switch (AircraftCategory) {
    case umGlider: return aircraft_t::glider;
    case umParaglider: return aircraft_t::paraglider;
    case umCar : return aircraft_t::otherAircraft;
    case umGAaircraft: return aircraft_t::poweredAircraft;
  }
  return aircraft_t::otherAircraft;
}

Frame::payload_t serialize_tracking(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
  Frame::payload_t buffer;
  buffer.reserve(11);
  auto out_it = std::back_inserter(buffer);

  /* position */
  Frame::coord2payload_absolut(Basic.Latitude, Basic.Longitude, out_it);

  /* altitude set the lower 12bit */
  int alt = Clamp<int>(Basic.Altitude, 0, 8190);
  if (alt > 2047) {
    alt = ((alt + 2) / 4) | (1 << 11);  // set scale factor
  }
  uint16_t data = alt;
  /* online tracking */
  data |= true << 15;
  /* aircraft type */
  data |= (type() & 0x7) << 12;

  out_it = ((uint8_t*)&data)[0];
  out_it = ((uint8_t*)&data)[1];

  /* Speed */
  int speed2 = Clamp<int>(std::round(Basic.Speed * 2.0), 0, 635);
  if (speed2 > 127) {
    speed2 = ((speed2 + 2) / 5) | (1 << 7);  // set scale factor
  }
  out_it = speed2;

  /* Climb */
  int climb10 = Clamp<int>(std::round(Basic.Vario * 10.0), -315, 315);
  if (std::abs(climb10) > 63) {
    climb10 = ((climb10 + (climb10 >= 0 ? 2 : -2)) / 5) | (1 << 7);  // set scale factor
  }
  out_it = climb10 & 0x7F;

  /* Heading */
  out_it = Clamp<int>(std::round(Basic.TrackBearing * 256.0 / 360.0), 0, 255);

  return buffer;
}

Frame::payload_t serialize_name() {
  std::string name = to_utf8(PilotName_Config);
  return { name.begin(), name.end() };
}

MacAddr generate_id() {
  MacAddr addr;

  DeviceSettings settings(_T("FlyBeeper"));
  try {
    addr.manufacturer = settings.get<uint8_t>("fanet-manufacturer");
    addr.id = settings.get<uint16_t>("fanet-id");
  }
  catch(std::exception& e) { }

  std::random_device rd;   // a seed source for the random number engine
  std::mt19937 gen(rd());  // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint16_t> distrib(1, std::numeric_limits<uint16_t>::max());

  addr.manufacturer = 0xFC;
  addr.id = distrib(gen);

  settings.set<int>("fanet-manufacturer", addr.manufacturer);
  settings.set<int>("fanet-id", addr.id);

  return addr;
}

BOOL SendData(DeviceDescriptor_t* d, uint8_t type, std::vector<uint8_t>&& data) {
  
  static MacAddr src_addr = generate_id();

  auto frm = std::make_unique<Frame>(src_addr);
  frm->type = type;
  frm->payload = std::move(data);

  auto buffer = frm->serialize();

  d->Com->WriteGattCharacteristic(
        "00001819-0000-1000-8000-00805F9B34FB",
        "FEC81438-CB89-4C37-93D0-BADFCED4376E",
        buffer.data(), buffer.size());

  return TRUE;
}

BOOL SendData(DeviceDescriptor_t* d, const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
  try {
    static PeriodClock timeState;
    if (timeState.CheckUpdate(5000)) {
      // Every 5 second : Send current state
      return SendData(d, FRM_TYPE_TRACKING, serialize_tracking(Basic, Calculated));
    }

    static PeriodClock timeName;
    if (timeName.CheckUpdate(60000)) {
      // Every 1 minute : Send Pilot Name if not empty
      return SendData(d, FRM_TYPE_NAME, serialize_name());
    }
  }
  catch (std::exception& e) {
    DebugLog(_T("%s"), to_tstring(e.what()).c_str());
    return FALSE;
  }
  return TRUE;
}

bool EnableGattCharacteristic(DeviceDescriptor_t& d, uuid_t service, uuid_t characteristic) {
  if (service == "00001819-0000-1000-8000-00805F9B34FB") { // Location and Navigation service
    if (characteristic == "FEC81438-CB89-4C37-93D0-BADFCED4376E") {
      // FANET characteristic is available, enable SendData
      d.SendData = SendData;
      return true;
    }
    return (characteristic == "234337BF-F931-4D2D-A13C-07E2F06A0249"); // TAS 
  }
  return false;
}

constexpr auto function_table = lookup_table<uint8_t, fanet_parse_function>({
  { 0x01, &FanetParseType1Msg },
  { 0x02, &FanetParseType2Msg },
  { 0x03, &FanetParseType3Msg },
  { 0x04, &FanetParseType4Msg },
  { 0x07, &FanetParseType7Msg }
});

void OnGattCharacteristic(DeviceDescriptor_t& d, NMEA_INFO& info, uuid_t service,
                             uuid_t characteristic, const std::vector<uint8_t>& data) {
  if (service == "00001819-0000-1000-8000-00805F9B34FB") { // Location and Navigation service
    if (characteristic == "234337BF-F931-4D2D-A13C-07E2F06A0249") {
      // TAS
      if (data.size() == 2) {
        info.AirspeedAvailable = true;
        info.TrueAirspeed = Units::From(unKiloMeterPerHour, FromLE16(*reinterpret_cast<const int16_t*>(data.data())) / 10.);
        info.IndicatedAirspeed = IndicatedAirSpeed(info.TrueAirspeed, QNHAltitudeToQNEAltitude(info.Altitude));
      }
    }
    else if (characteristic == "FEC81438-CB89-4C37-93D0-BADFCED4376E") {
      // FANET
      Frame frame(data.data(), data.size());
      fanet_parse_function parse = function_table.get(frame.type, FanetParseUnknown);
      parse(&d, &info, frame.src.get(), frame.payload);
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
