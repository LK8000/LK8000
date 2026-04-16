/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GDL90/decoders/traffic.h
 * Author: Bruno de Lacheisserie
 */

#pragma once

#include "../helpers.h"

#include <optional>
#include <utility>

namespace gdl90_parser {

// ─────────────────────────────────────────────────────────────────────────────
// Traffic / Ownship report (MSG_TRAFFIC 0x14, MSG_OWNSHIP 0x0A)
// Both messages share the same wire format; ownship_report is a distinct type
// for compile-time dispatch and allowlist filtering.
// ─────────────────────────────────────────────────────────────────────────────

inline constexpr uint8_t MSG_TRAFFIC = 0x14;
inline constexpr uint8_t MSG_OWNSHIP = 0x0A;

enum class EmergencyCode : uint8_t {
  None = 0,
  General = 1,
  Medical = 2,
  MinFuel = 3,
  NoCommunication = 4,
  UnlawfulInterf = 5,
  DownedAircraft = 6,
};

enum class AddressType : uint8_t {
  ADSB_ICAO = 0,
  ADSB_SelfAssign = 1,
  TISB_ICAO = 2,
  TISB_TrackFile = 3,
  Surface = 4,
  GroundStation = 5,
  Reserved = 6,
};

enum class NavIntegrity : uint8_t {
  Unknown = 0,
  Less20NM = 1,
  Less8NM = 2,
  Less4NM = 3,
  Less2NM = 4,
  Less1NM = 5,
  Less0_6NM = 6,
  Less0_2NM = 7,
  Less0_1NM = 8,
  HPL_75m = 9,
  HPL_25m = 10,
  HPL_7_5m = 11,
};

enum class NavAccuracy : uint8_t {
  Unknown = 0,
  Less10NM = 1,
  Less4NM = 2,
  Less2NM = 3,
  Less1NM = 4,
  Less0_5NM = 5,
  Less0_3NM = 6,
  Less0_1NM = 7,
  Less0_05NM = 8,
  HFOM_30m = 9,
  HFOM_10m = 10,
  HFOM_3m = 11,
};

enum class EmitterCategory : uint8_t {
  NoInfo = 0,
  Light = 1,
  Small = 2,
  Large = 3,
  HighVortex = 4,
  Heavy = 5,
  HighPerformance = 6,
  Rotorcraft = 7,
  Glider = 9,
  LighterThanAir = 10,
  Parachutist = 11,
  UltraLight = 12,
  UAV = 14,
  Space = 15,
  SurfaceVehicle = 17,
  ServiceVehicle = 18,
  PointObstacle = 19,
};

struct traffic_report {
  uint32_t icao_address;    // 24-bit ICAO / participant address
  AddressType address_type;

  double latitude;          // degrees  [-90..+90]
  double longitude;         // degrees  [-180..+180]
  int32_t pressure_alt;     // feet, -1000 = invalid
  bool air_ground;          // true = airborne

  uint16_t heading;         // degrees [0..359], valid only when horiz_velocity is valid
  bool heading_is_magnetic;     // false = true track angle, true = magnetic heading
  uint16_t horiz_velocity;  // kt, 0xFFFF = invalid
  int32_t vert_velocity;    // ft/min, INT32_MAX = invalid

  NavIntegrity nic;
  NavAccuracy nacp;
  EmitterCategory emitter;
  EmergencyCode emergency;

  char callsign[9];  // 8 chars + NUL
  bool has_callsign;

  static constexpr int32_t invalid_pressure_alt = -1000;
  static constexpr uint16_t invalid_horiz_velocity = 0xFFFF;
  static constexpr int32_t invalid_vert_velocity = INT32_MAX;

  bool is_valid_position() const noexcept {
    return latitude != 0.0 || longitude != 0.0;
  }
  std::optional<int32_t> get_pressure_alt() const noexcept {
    return pressure_alt != invalid_pressure_alt ? std::optional{pressure_alt}
                                                : std::nullopt;
  }
  std::optional<uint16_t> get_heading() const noexcept {
    // track/heading byte is only meaningful when horizontal velocity is valid
    return horiz_velocity != invalid_horiz_velocity ? std::optional{heading}
                                                    : std::nullopt;
  }
  std::optional<uint16_t> get_horiz_velocity() const noexcept {
    return horiz_velocity != invalid_horiz_velocity
               ? std::optional{horiz_velocity}
               : std::nullopt;
  }
  std::optional<int32_t> get_vert_velocity() const noexcept {
    return vert_velocity != invalid_vert_velocity ? std::optional{vert_velocity}
                                                  : std::nullopt;
  }
};

struct ownship_report : traffic_report {
  ownship_report() = default;
  explicit ownship_report(traffic_report report) noexcept
      : traffic_report(std::move(report)) {}
};

template<> struct msg_id_for<traffic_report> {
  static constexpr uint8_t value = MSG_TRAFFIC;
};

template<> struct msg_id_for<ownship_report> {
  static constexpr uint8_t value = MSG_OWNSHIP;
};

namespace detail {

[[nodiscard]]
inline traffic_report decode_traffic_report(frame_data payload) {
  if (payload.size() < 27) {
    throw too_short();
  }

  traffic_report report{};
  report.address_type = static_cast<AddressType>(payload[0] & 0x0F);
  report.icao_address = read_u24_be(payload, 1);

  const uint32_t lat_raw = read_u24_be(payload, 4);
  report.latitude = decode_lat_lon(lat_raw) * (180.0 / (1 << 23));

  const uint32_t lon_raw = read_u24_be(payload, 7);
  report.longitude = decode_lat_lon(lon_raw) * (180.0 / (1 << 23));

  const uint16_t alt_raw =
      (static_cast<uint16_t>(payload[10]) << 4) | (payload[11] >> 4);
  report.pressure_alt = decode_altitude(alt_raw);
  report.air_ground = (payload[11] & 0x08) != 0;

  report.nic = static_cast<NavIntegrity>((payload[12] & 0xF0) >> 4);
  report.nacp = static_cast<NavAccuracy>(payload[12] & 0x0F);

  const uint16_t hvel_raw =
      (static_cast<uint16_t>(payload[13]) << 4) | (payload[14] >> 4);
  report.horiz_velocity =
      (hvel_raw == 0xFFF) ? report.invalid_horiz_velocity : hvel_raw;

  const uint16_t vvel_raw =
      ((static_cast<uint16_t>(payload[14] & 0x0F)) << 8) | payload[15];
  if (vvel_raw == 0x800) {
    report.vert_velocity = report.invalid_vert_velocity;
  }
  else if (vvel_raw & 0x800) {
    // negative: sign-extend 12-bit two's complement to int32_t
    report.vert_velocity = static_cast<int32_t>(vvel_raw | 0xFFFFF000u) * 64;
  }
  else {
    report.vert_velocity = static_cast<int32_t>(vvel_raw) * 64;
  }

  report.heading_is_magnetic = (payload[11] & 0x04) != 0;
  report.heading = static_cast<uint16_t>(payload[16]) * 360 / 256;

  report.emitter = static_cast<EmitterCategory>(payload[17]);
  report.emergency = static_cast<EmergencyCode>((payload[26] >> 4) & 0x0F);

  bool any_nonspace = false;
  for (int i = 0; i < 8; ++i) {
    report.callsign[i] = static_cast<char>(payload[18 + i]);
    if (report.callsign[i] != ' ' && report.callsign[i] != '\0') {
      any_nonspace = true;
    }
  }
  report.callsign[8] = '\0';
  report.has_callsign = any_nonspace;
  return report;
}

[[nodiscard]]
inline ownship_report decode_ownship_report(frame_data payload) {
  return ownship_report{decode_traffic_report(payload)};
}

}  // namespace detail
}  // namespace gdl90_parser
