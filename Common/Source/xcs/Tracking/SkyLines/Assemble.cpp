/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Assemble.hpp"
#include "Export.hpp"
#include "Protocol.hpp"
#include "NMEA/Info.h"
#include "Calc/Vario.h"
#include "OS/ByteOrder.hpp"
#include "Util/CRC.hpp"

#include <assert.h>
#include <cmath>

SkyLinesTracking::PingPacket
SkyLinesTracking::MakePing(uint64_t key, uint16_t id)
{
  assert(key != 0);

  PingPacket packet;
  packet.header.magic = ToBE32(MAGIC);
  packet.header.crc = 0;
  packet.header.type = ToBE16(Type::PING);
  packet.header.key = ToBE64(key);
  packet.id = ToBE16(id);
  packet.reserved = 0;
  packet.reserved2 = 0;

  packet.header.crc = ToBE16(UpdateCRC16CCITT(&packet, sizeof(packet), 0));
  return packet;
}

SkyLinesTracking::FixPacket
SkyLinesTracking::ToFix(uint64_t key, const NMEA_INFO &basic)
{
  assert(key != 0);
  assert(!basic.NAVWarning);

  FixPacket packet;
  packet.header.magic = ToBE32(MAGIC);
  packet.header.crc = 0;
  packet.header.type = ToBE16(Type::FIX);
  packet.header.key = ToBE64(key);
  packet.flags = 0;

  packet.time = ToBE32(uint32_t(basic.Time * 1000));
  packet.reserved = 0;

  if (!basic.NAVWarning) {
    packet.flags |= ToBE32(FixPacket::FLAG_LOCATION);
    packet.location = ExportGeoPoint(basic.Latitude, basic.Longitude);
  } else
    packet.location.latitude = packet.location.longitude = 0;

  if (!basic.NAVWarning) {
    packet.flags |= ToBE32(FixPacket::FLAG_TRACK);
    packet.track = ToBE16(uint16_t(basic.TrackBearing));
  } else
    packet.track = 0;

  if (!basic.NAVWarning) {
    packet.flags |= ToBE32(FixPacket::FLAG_GROUND_SPEED);
    packet.ground_speed = ToBE16(uint16_t(basic.Speed * 16));
  } else
    packet.ground_speed = 0;

  if (basic.AirspeedAvailable) {
    packet.flags |= ToBE32(FixPacket::FLAG_AIRSPEED);
    packet.airspeed = ToBE16(uint16_t(basic.IndicatedAirspeed * 16));
  } else
    packet.airspeed = 0;

  if (basic.BaroAltitudeAvailable) {
    packet.flags |= ToBE32(FixPacket::FLAG_ALTITUDE);
    packet.altitude = ToBE16(int(basic.BaroAltitude));
  } else if (!basic.NAVWarning) {
    packet.flags |= ToBE32(FixPacket::FLAG_ALTITUDE);
    packet.altitude = ToBE16(int(basic.Altitude));
  } else
    packet.altitude = 0;

  /*if (basic.total_energy_vario_available) {
    packet.flags |= ToBE32(FixPacket::FLAG_VARIO);
    packet.vario = ToBE16(int(basic.total_energy_vario * 256));
  } else */if (basic.NettoVarioAvailable) {
    packet.flags |= ToBE32(FixPacket::FLAG_VARIO);
    packet.vario = ToBE16(int(basic.NettoVario * 256));
  } else if (VarioAvailable(basic)) {
    packet.flags |= ToBE32(FixPacket::FLAG_VARIO);
    packet.vario = ToBE16(int(basic.Vario * 256));
  } else
    packet.vario = 0;
  /*
  if (basic.engine_noise_level_available) {
    packet.flags |= ToBE32(FixPacket::FLAG_ENL);
    packet.engine_noise_level = ToBE16(basic.engine_noise_level);
  } else */
    packet.engine_noise_level = 0;

  packet.header.crc = ToBE16(UpdateCRC16CCITT(&packet, sizeof(packet), 0));
  return packet;
}

SkyLinesTracking::ThermalSubmitPacket
SkyLinesTracking::MakeThermalSubmit(uint64_t key, uint32_t time,
                                    const ::GeoPoint& bottom_location,
                                    int bottom_altitude,
                                    const ::GeoPoint& top_location,
                                    int top_altitude,
                                    double lift)
{
  ThermalSubmitPacket packet;
  packet.header.magic = ToBE32(MAGIC);
  packet.header.crc = 0;
  packet.header.type = ToBE16(Type::THERMAL_SUBMIT);
  packet.header.key = ToBE64(key);
  packet.thermal.time = time;
  packet.thermal.reserved1 = 0;
  packet.thermal.bottom_location = ExportGeoPoint(bottom_location);
  packet.thermal.top_location = ExportGeoPoint(top_location);
  packet.thermal.bottom_altitude = ToBE16(bottom_altitude);
  packet.thermal.top_altitude = ToBE16(top_altitude);
  packet.thermal.lift = ToBE16(lround(lift * 256));
  packet.thermal.reserved2 = 0;
  packet.header.crc = ToBE16(UpdateCRC16CCITT(&packet, sizeof(packet), 0));
  return packet;
}

#ifdef HAVE_SKYLINES_TRACKING_HANDLER

SkyLinesTracking::TrafficRequestPacket
SkyLinesTracking::MakeTrafficRequest(uint64_t key, bool followees, bool club,
                                     bool near)
{
  assert(key != 0);

  TrafficRequestPacket packet;
  packet.header.magic = ToBE32(MAGIC);
  packet.header.crc = 0;
  packet.header.type = ToBE16(Type::TRAFFIC_REQUEST);
  packet.header.key = ToBE64(key);
  packet.flags = ToBE32((followees ? packet.FLAG_FOLLOWEES : 0)
                        | (club ? packet.FLAG_CLUB : 0)
                        | (near ? packet.FLAG_NEAR : 0));
  packet.reserved = 0;

  packet.header.crc = ToBE16(UpdateCRC16CCITT(&packet, sizeof(packet), 0));
  return packet;
}

SkyLinesTracking::UserNameRequestPacket
SkyLinesTracking::MakeUserNameRequest(uint64_t key, uint32_t user_id)
{
  assert(key != 0);

  UserNameRequestPacket packet;
  packet.header.magic = ToBE32(MAGIC);
  packet.header.crc = 0;
  packet.header.type = ToBE16(Type::USER_NAME_REQUEST);
  packet.header.key = ToBE64(key);
  packet.user_id = ToBE32(user_id);
  packet.reserved = 0;

  packet.header.crc = ToBE16(UpdateCRC16CCITT(&packet, sizeof(packet), 0));
  return packet;
}

#endif
