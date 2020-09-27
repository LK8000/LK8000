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

#include "Glue.hpp"
#include "Settings.hpp"
#include "Queue.hpp"
#include "Assemble.hpp"
#include "NMEA/Info.h"
#include "NMEA/Derived.h"
#include "Net/State.hpp"
#include "Net/StaticSocketAddress.hxx"
#include "Util/StaticString.hpp"
#include "OS/ByteOrder.hpp"
#include "Geographic/GeoPoint.h"
#ifdef HAVE_POSIX
#include "IO/Async/GlobalIOThread.hpp"
#endif


static constexpr fixed CLOUD_INTERVAL = fixed(60);

SkyLinesTracking::Glue::Glue()
  :interval(0),
#ifdef HAVE_SKYLINES_TRACKING_HANDLER
   traffic_enabled(false),
   near_traffic_enabled(false),
#endif
   roaming(true),
   queue(nullptr),
   last_climb_time(-1)
{
#ifdef HAVE_SKYLINES_TRACKING_HANDLER
  assert(io_thread != nullptr);
  client.SetIOThread(io_thread);
#endif
}

SkyLinesTracking::Glue::~Glue()
{
  delete queue;
}

inline bool
SkyLinesTracking::Glue::IsConnected() const
{
  switch (GetNetState()) {
  case NetState::UNKNOWN:
    /* we don't know if we have an internet connection - be
       optimistic, and assume everything's ok */
    return true;

  case NetState::DISCONNECTED:
    return false;

  case NetState::CONNECTED:
    return true;

  case NetState::ROAMING:
    return roaming;
  }

  assert(false);
  gcc_unreachable();
}

inline void
SkyLinesTracking::Glue::SendFixes(const NMEA_INFO &basic)
{
  assert(client.IsDefined());

  if (basic.NAVWarning) {
    clock.Reset();
    return;
  }

  if (!IsConnected()) {
    if (clock.CheckAdvance(basic.Time, fixed(interval))) {
      /* queue the packet, send it later */
      if (queue == nullptr)
        queue = new Queue();
      queue->Push(ToFix(client.GetKey(), basic));
    }

    return;
  }

  if (queue != nullptr) {
    /* send queued fix packets, 8 at a time */
    unsigned n = 8;
    while (n-- > 0) {
      const auto &packet = queue->Peek();
      if (!client.SendPacket(packet))
        break;

      queue->Pop();
      if (queue->IsEmpty()) {
        delete queue;
        queue = nullptr;
        break;
      }
    }

    return;
  } else if (clock.CheckAdvance(basic.Time, fixed(interval)))
    client.SendFix(basic);
}

void
SkyLinesTracking::Glue::SendCloudFix(const NMEA_INFO &basic,
                                     const DERIVED_INFO &calculated)
{
  assert(cloud_client.IsDefined());

  if (basic.NAVWarning) {
    cloud_clock.Reset();
    return;
  }

  if (calculated.Flying)
    return;

  if (!IsConnected())
    return;

  if (cloud_clock.CheckAdvance(basic.Time, CLOUD_INTERVAL))
    cloud_client.SendFix(basic);

  if (last_climb_time > basic.Time)
    /* recover from time warp */
    last_climb_time = fixed(-1);

  constexpr fixed min_climb_duration(30);
  constexpr fixed min_height_gain(100);
  if (!calculated.Circling &&
      calculated.ClimbStartTime >= fixed(0) &&
      calculated.ClimbStartTime > last_climb_time &&
      calculated.CruiseStartTime > calculated.ClimbStartTime + min_climb_duration &&
      calculated.CruiseStartAlt > calculated.ClimbStartAlt + min_height_gain) {
    /* we just stopped circling - send our thermal location to the
       XCSoar Cloud server */
    // TODO: use TE altitude?
    last_climb_time = calculated.CruiseStartTime;

    fixed duration = calculated.CruiseStartTime - calculated.ClimbStartTime;
    fixed height_gain = calculated.CruiseStartAlt - calculated.ClimbStartAlt;
    fixed lift = height_gain / duration;

    cloud_client.SendThermal(ToBE32(uint32_t(basic.Time * 1000)),
                             ::GeoPoint(calculated.ClimbStartLat, calculated.ClimbStartLong),
                             iround(calculated.ClimbStartAlt),
                             ::GeoPoint(calculated.CruiseStartLat, calculated.CruiseStartLong),
                             iround(calculated.CruiseStartAlt),
                             (double)lift);
  }
}

void
SkyLinesTracking::Glue::Tick(const NMEA_INFO &basic,
                             const DERIVED_INFO &calculated)
{
    // TODO: disable in simulator/replay

  if (client.IsDefined()) {
    SendFixes(basic);

#ifdef HAVE_SKYLINES_TRACKING_HANDLER
    if (traffic_enabled && traffic_clock.CheckAdvance(basic.Time, fixed(60)))
      client.SendTrafficRequest(true, true, near_traffic_enabled);
#endif
  }

  if (cloud_client.IsDefined())
    SendCloudFix(basic, calculated);
}

void
SkyLinesTracking::Glue::SetSettings(const Settings &settings)
{
  if (settings.cloud_enabled == TriState::TRUE && settings.cloud_key != 0) {
    cloud_client.SetKey(settings.cloud_key);
    if (!cloud_client.IsDefined()) {
      NarrowString<32> service;
      service.UnsafeFormat("%u", Client::GetDefaultPort());

      StaticSocketAddress address;
      if (address.Lookup("cloud.xcsoar.net", service, SOCK_DGRAM)) {
        cloud_client.Open(address);
      }
    }
  } else {
    cloud_client.Close();
  }

  if (!settings.enabled || settings.key == 0) {
    delete queue;
    queue = nullptr;
    client.Close();
    return;
  }

  client.SetKey(settings.key);

  interval = settings.interval;

  if (!client.IsDefined()) {
    NarrowString<32> service;
    service.UnsafeFormat("%u", Client::GetDefaultPort());

    StaticSocketAddress address;
    if (address.Lookup("tracking.skylines.aero", service, SOCK_DGRAM)) {
      client.Open(address);
    }
  }

#ifdef HAVE_SKYLINES_TRACKING_HANDLER
  traffic_enabled = settings.traffic_enabled;
  near_traffic_enabled = settings.near_traffic_enabled;
#endif

  roaming = settings.roaming;
}
