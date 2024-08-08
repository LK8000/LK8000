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
#include "Comm/Bluetooth/gatt_utils.h"

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

payload_t serialize_tracking(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
  payload_t buffer;
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
  constexpr uint16_t online_tracking = 0x01;
  data |= online_tracking << 15;
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

payload_t serialize_gound_tracking(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
  payload_t buffer;
  buffer.reserve(6);
  auto out_it = std::back_inserter(buffer);

  /* position */
  Frame::coord2payload_absolut(Basic.Latitude, Basic.Longitude, out_it);


  /* state & Online Tracking */
  constexpr uint8_t state = 0;
  constexpr uint8_t online_tracking = 0x01;

  out_it = (state&0x0F)<<4 | online_tracking;

  return buffer;
}

payload_t serialize_name() {
  std::string name = to_utf8(PilotName_Config);
  return { name.begin(), name.end() };
}

MacAddr generate_id(tstring_view sn) {
  if (sn.size() > 4) {
    // use last four digit as fanet id
    return { 
      0x0B, // FlyBeeper manufacturer id
      hex::to_uint16_t(sn.substr(sn.size() - 4).data())
    };
  }
  throw std::domain_error("SN is too short");
}

BOOL SendData(DeviceDescriptor_t* d, uint8_t type, payload_t&& data) {
  using bluetooth::gatt_uuid;
  
  MacAddr src_addr = generate_id(d->SerialNumber);

  Frame frm(src_addr, type, std::move(data));
  auto buffer = frm.serialize();

  d->Com->WriteGattCharacteristic(
        gatt_uuid(0x1819),
        "FEC81438-CB89-4C37-93D0-BADFCED4376E",
        buffer.data(), buffer.size());

  return TRUE;
}

BOOL SendData(DeviceDescriptor_t* d, const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
  try {
    static PeriodClock timeState;
    if (timeState.CheckUpdate(5000)) {
      // Every 5 second : Send current state
      if (Calculated.Flying) {
        return SendData(d, FRM_TYPE_TRACKING, serialize_tracking(Basic, Calculated));
      }
      else {
        return SendData(d, FRM_TYPE_GROUNDTRACKING, serialize_gound_tracking(Basic, Calculated));
      }
    }

    static PeriodClock timeName;
    if (timeName.CheckUpdate(60000)) {
      // Every 1 minute : Send Pilot Name if not empty
      return SendData(d, FRM_TYPE_NAME, serialize_name());
    }
  }
  catch (std::exception& e) {
    DebugLog(_T("FlyBeeper::SendData : <%s>"), to_tstring(e.what()).c_str());
    return FALSE;
  }
  return TRUE;
}

void TAS(DeviceDescriptor_t& d, NMEA_INFO& info, const std::vector<uint8_t>& data) {
  // TAS
  if (data.size() == 2) {
    info.AirspeedAvailable = true;
    info.TrueAirspeed = Units::From(unKiloMeterPerHour, FromLE16(*reinterpret_cast<const int16_t*>(data.data())) / 10.);
    info.IndicatedAirspeed = IndicatedAirSpeed(info.TrueAirspeed, QNHAltitudeToQNEAltitude(info.Altitude));
  }
}

void Fanet(DeviceDescriptor_t& d, NMEA_INFO& info, const std::vector<uint8_t>& data) {

  constexpr static auto function_table = lookup_table<uint8_t, fanet_parse_function>({
    { FRM_TYPE_TRACKING, &FanetParseType1Msg },
    { FRM_TYPE_NAME, &FanetParseType2Msg },
    { FRM_TYPE_MESSAGE, &FanetParseType3Msg },
    { FRM_TYPE_SERVICE, &FanetParseType4Msg },
    { FRM_TYPE_GROUNDTRACKING, &FanetParseType7Msg },
    { FRM_TYPE_THERMAL, &FanetParseType9Msg }
  });

  // FANET
  Frame frame(data.data(), data.size());
  const fanet_parse_function& parse = function_table.get(frame.type, FanetParseUnknown);
  parse(&d, &info, frame.src.get(), frame.payload);
}

bool EnableFanet(DeviceDescriptor_t& d) {
  d.SendData = SendData;
  return true;
}

template<bool b>
bool Enable(DeviceDescriptor_t&) {
  return b;
}

using OnGattCharacteristicT = std::function<void(DeviceDescriptor_t&, NMEA_INFO&, const std::vector<uint8_t>&)>;
using DoEnableNotificationT = std::function<bool(DeviceDescriptor_t&)>;

struct DataHandlerT {
  OnGattCharacteristicT OnGattCharacteristic;
  DoEnableNotificationT DoEnableNotification;
};

using service_table_t = bluetooth::service_table_t<DataHandlerT>;

const service_table_t& service_table() {
  using bluetooth::gatt_uuid;
  static const service_table_t table = {{
    { gatt_uuid(0x1819), {{ // Location and Navigation service
        { "FEC81438-CB89-4C37-93D0-BADFCED4376E", {
            &Fanet,
            &EnableFanet,
        }},
        { "234337BF-F931-4D2D-A13C-07E2F06A0249", {
            &TAS,
            &Enable<true>
        }}
    }}}
  }};
  return table;
}

bool DoEnableGattCharacteristic(DeviceDescriptor_t& d, uuid_t service, uuid_t characteristic) {
  auto handler = service_table().get(service, characteristic);
  return handler && std::invoke(handler->DoEnableNotification, d);
}

void OnGattCharacteristic(DeviceDescriptor_t& d, NMEA_INFO& info, uuid_t service,
                             uuid_t characteristic, const std::vector<uint8_t>& data) {
  auto handler = service_table().get(service, characteristic);
  if (handler) {
    std::invoke(handler->OnGattCharacteristic, d, info, data);
  }
}

} // namespace

void FlyBeeper::Install(DeviceDescriptor_t* d) {
  genInstall(d); // install Generic driver callback first

  d->DoEnableGattCharacteristic = DoEnableGattCharacteristic;
  d->OnGattCharacteristic = OnGattCharacteristic;
}
