/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devGDL90.cpp
 * Author: Bruno de Lacheisserie
 */

#include "devGDL90.h"
#include "Comm/DeviceDescriptor.h"
#include "GDL90/frame_scanner.h"
#include "utils/strcpy.h"
#include "utils/charset_helper.h"
#include "MessageLog.h"
#include "NMEA/Info.h"
#include "Baro.h"
#include "Units.h"
#include "Calc/Vario.h"
#include "Defines.h"
#include "lk8000.h"
#include "Utils.h"

namespace {
/**
 * dispatch a decoded GDL90 message to appropriate handler.
 * each handler updates GPS_INFO and/or device state.
 */
class MessageDispatcher {
  DeviceDescriptor_t& d;
  NMEA_INFO& NmeaInfo;

 public:
  MessageDispatcher() = delete;
  MessageDispatcher(DeviceDescriptor_t& d, NMEA_INFO& Info)
      : d(d), NmeaInfo(Info) {}

  void operator()(const gdl90_parser::heartbeat& h) const {
    DebugLog(_T("[GDL90][HB] t=%u gps_valid=%d"), h.utc_seconds(),
             h.gps_pos_valid());

    const uint32_t utc_seconds = h.utc_seconds();

    d.nmeaParser.connected = true;
    if (d.nmeaParser.activeGPS) {
      NmeaInfo.Time = utc_seconds;
      NmeaInfo.Hour = utc_seconds / 3600;
      NmeaInfo.Minute = (utc_seconds / 60) % 60;
      NmeaInfo.Second = utc_seconds % 60;
    }

    if (h.gps_pos_valid()) {
      d.nmeaParser.lastGpsValid.Update();
      d.nmeaParser.gpsValid = true;
    }
    else {
      d.nmeaParser.lastGpsValid.Reset();
      d.nmeaParser.gpsValid = false;
    }
  }

  void operator()(const gdl90_parser::ownship_report& r) const {
    DebugLog(_T("[GDL90][OWN] lat=%.5f lon=%.5f alt=%d ft callsign=%s"),
             r.latitude, r.longitude, r.pressure_alt,
             r.has_callsign ? to_tstring(r.callsign).c_str() : _T("<none>"));

    const bool gps_valid = r.is_valid_position();

    d.nmeaParser.connected = true;
    d.nmeaParser.gpsValid = gps_valid;
    if (gps_valid) {
      d.nmeaParser.lastGpsValid.Update();
    }

    if (d.nmeaParser.activeGPS) {
      NmeaInfo.NAVWarning = !gps_valid;
      if (gps_valid) {
        NmeaInfo.Latitude = r.latitude;
        NmeaInfo.Longitude = r.longitude;
        if (auto h = r.get_heading()) {
          if (r.heading_is_magnetic) {
            NmeaInfo.MagneticHeading.update(d, *h);
          }
          else {
            NmeaInfo.TrackBearing = *h;
          }
        }
        if (auto v = r.get_horiz_velocity()) {
          NmeaInfo.Speed = Units::From(unKnots, *v);
        }
      }
      TriggerGPSUpdate();
    }
    if (auto a = r.get_pressure_alt()) {
      double altitude = QNEAltitudeToQNHAltitude(Units::From(unFeet, *a));
      UpdateBaroSource(&NmeaInfo, &d, altitude);
    }
    if (auto vv = r.get_vert_velocity()) {
      UpdateVarioSource(NmeaInfo, d, Units::From(unFeetPerMinutes, *vv));
    }
  }

  void operator()(const gdl90_parser::traffic_report& r) const {
    DebugLog(_T("[GDL90][TRF] lat=%.5f lon=%.5f alt=%d ft callsign=%s"),
             r.latitude, r.longitude, r.pressure_alt,
             r.has_callsign ? to_tstring(r.callsign).c_str() : _T("<none>"));

    int flarm_slot = FLARM_FindSlot(&NmeaInfo, r.icao_address);
    if (flarm_slot < 0) {
      return;  // not found in FLARM traffic list, ignore
    }
    d.nmeaParser.setFlarmAvailable(&NmeaInfo);
    FLARM_TRAFFIC& traffic = NmeaInfo.FLARM_Traffic[flarm_slot];
    if (traffic.RadioId != r.icao_address) {
      traffic = {};
    }
    else {
      // before changing timefix, see if it was an old target back locked in!
      CheckBackTarget(NmeaInfo, flarm_slot);
    }
    traffic.RadioId = r.icao_address;
    traffic.Time_Fix = NmeaInfo.Time;
    traffic.Status = LKT_REAL;
    traffic.Latitude = r.latitude;
    traffic.Longitude = r.longitude;
    if (auto h = r.get_heading()) {
      traffic.TrackBearing = *h;
    }

    if (auto a = r.get_pressure_alt()) {
      traffic.Altitude = QNEAltitudeToQNHAltitude(Units::From(unFeet, *a));
    }
    if (auto v = r.get_horiz_velocity()) {
      traffic.Speed = Units::From(unKnots, *v);
    }
    if (auto vv = r.get_vert_velocity()) {
      traffic.ClimbRate = Units::From(unFeetPerMinutes, *vv);
    }

    traffic.Cn[0] = 0;  // FLARM CN is not provided by GDL90, leave empty
    traffic.IDType = r.address_type == gdl90_parser::AddressType::ADSB_ICAO
                         ? 1
                         : 0;  // FLARM IDType: 1=ICAO, 0=unknown

    if (!traffic.Name[0] || traffic.UpdateNameFlag) {
      if (r.has_callsign) {
        traffic.UpdateNameFlag = false;  // clear flag first
        from_utf8(r.callsign, traffic.Name, MAXFLARMNAME);
      }
      else {
        lk::strcpy(traffic.Name, _T("?"));
        traffic.UpdateNameFlag = true;
      }
    }
  }

  void operator()(const gdl90_parser::ownship_geo_alt& g) const {
    DebugLog(_T("[GDL90][GEO] geo_alt=%d ft vfom=%u m"), g.geo_altitude,
             g.vfom);
    if (auto alt = g.get_geo_altitude()) {
      NmeaInfo.Altitude = Units::From(unFeet, *alt);
    }
  }

  void operator()(const gdl90_parser::stratux_status& s) const {
    DebugLog(
        _T("[GDL90][SX] v=%u.%u.%u.%u gps=%u/%u uat=%u es=%u temp=%.1fC towers=%u"),
        s.version_major, s.version_minor, s.version_build_type, s.version_build,
        s.gps_satellites_locked, s.gps_satellites_tracked,
        s.uat_messages_per_minute, s.es_messages_per_minute, s.cpu_temp_c,
        s.num_towers);

    d.nmeaParser.nSatellites = s.gps_satellites_locked;
    if (d.nmeaParser.activeGPS) {
      NmeaInfo.SatellitesUsed = s.gps_satellites_locked;
    }
  }

#ifndef NDEBUG
  // for debugging currently just log messages that are not used...
  void operator()(const gdl90_parser::foreflight_id& f) const {
    if (f.sub_id != 0) {
      return;
    }

    DebugLog(_T("[GDL90][FF]  device=%s serial=%llu"),
             to_tstring(f.device_name).c_str(),
             static_cast<unsigned long long>(f.device_serial));
  }

  void operator()(const gdl90_parser::stratux_heartbeat& a) const {
    DebugLog(_T("[GDL90][SX-HB] proto=%u gps=%d ahrs=%d"),
             static_cast<unsigned>(a.protocol_version()),
             static_cast<int>(a.gps_valid()),
             static_cast<int>(a.ahrs_valid()));
  }

  void operator()(const gdl90_parser::levil_ahrs& a) const {
    DebugLog(_T("[GDL90][LEVIL] pitch=%.1f roll=%.1f hdg=%.1f ias=%.1f alt=%d vspd=%d"),
         a.get_pitch(),
         a.get_roll(),
         a.get_heading()      .value_or(-1.0),
         a.get_airspeed()     .value_or(-1.0),
         a.get_pressure_alt() .value_or(-1),
         static_cast<int>(a.get_vert_velocity().value_or(-1)));
  }

  void operator()(const gdl90_parser::unknown_msg& u) const {
    DebugLog(_T("[GDL90][???]  id=0x%02X len=%zu"), u.id, u.payload.size());
  }
#endif
};

enum data_tag : unsigned {
  // 0 is reserved for "Generic driver data"
  frame_scanner = 1
};

/**
 * used to store gdl90_parser::frame_scanner in DeviceDescriptor_t::driver_data.
 * allow to maintain state across multiple calls to ParseStream, which may
 * contain partial frames.
 */
struct GDL90frame_scanner : public DriverData, public gdl90_parser::frame_scanner {
  using frame_scanner::frame_scanner;  // inherit constructors

  void parse(const uint8_t* data, size_t len, DeviceDescriptor_t& d, NMEA_INFO& GPS_INFO) {
    try {
      push<MessageDispatcher>(data, len, d, GPS_INFO);
    }
    catch (const std::exception& e) {
      DebugLog(_T("[GDL90][ERR] %s"), to_tstring(e.what()).c_str());
    }
  }
};

BOOL ParseStream(DeviceDescriptor_t* d, char* data, int len,
                 NMEA_INFO* GPS_INFO) {
  if (len <= 0) {
    return TRUE;
  }
  auto scanner = d->get_data<GDL90frame_scanner>(data_tag::frame_scanner);
  if (scanner) {
    auto data_ptr = reinterpret_cast<const uint8_t*>(data);
    auto data_len = static_cast<size_t>(len);
    scanner->parse(data_ptr, data_len, *d, *GPS_INFO);
  }
  return TRUE;
}

BOOL ParseNMEA(DeviceDescriptor_t* d, const char* String, NMEA_INFO* GPS_INFO) {
  return TRUE;  // ignore NMEA sentences, we only care about GDL90 frames
  // returning TRUE indicates successful parsing to avoid to send data to other
  // parsers (e.g. NMEAParser)
}

}  // namespace

void GDL90::Install(DeviceDescriptor_t* d) {
  d->ParseStream = ParseStream;
  d->ParseNMEA = ParseNMEA;
}
