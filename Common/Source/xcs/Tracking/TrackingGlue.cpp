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

#include "TrackingGlue.hpp"
#include "NMEA/Info.h"
#include "NMEA/Derived.h"

TrackingGlue::TrackingGlue()
{
  settings.SetDefaults();

#ifdef HAVE_SKYLINES_TRACKING_HANDLER
  skylines.SetHandler(this);
#endif
}

void
TrackingGlue::SetSettings(const TrackingSettings &_settings)
{
#ifdef HAVE_SKYLINES_TRACKING
  skylines.SetSettings(_settings.skylines);
#endif
}

void
TrackingGlue::OnTimer(const NMEA_INFO &basic, const DERIVED_INFO &calculated)
{
#ifdef HAVE_SKYLINES_TRACKING
  skylines.Tick(basic, calculated);
#endif
}

#ifdef HAVE_SKYLINES_TRACKING_HANDLER

void
TrackingGlue::OnTraffic(uint32_t pilot_id, unsigned time_of_day_ms,
                        const GeoPoint &location, int altitude)
{
  bool user_known;

  {
    const ScopeLock protect(skylines_data.mutex);
    const SkyLinesTracking::Data::Traffic traffic(time_of_day_ms,
                                                  location, altitude);
    skylines_data.traffic[pilot_id] = traffic;

    user_known = skylines_data.IsUserKnown(pilot_id);
  }

  if (!user_known)
    /* we don't know this user's name yet - try to find it out by
       asking the server */
    skylines.RequestUserName(pilot_id);
}

void
TrackingGlue::OnUserName(uint32_t user_id, const TCHAR *name)
{
  const ScopeLock protect(skylines_data.mutex);
  skylines_data.user_names[user_id] = name;
}

void
TrackingGlue::OnWave(unsigned time_of_day_ms,
                     const GeoPoint &a, const GeoPoint &b)
{
  const ScopeLock protect(skylines_data.mutex);

  /* garbage collection - hard-coded upper limit */
  auto n = skylines_data.waves.size();
  while (n-- >= 64)
    skylines_data.waves.pop_front();

  // TODO: replace existing item?
  skylines_data.waves.emplace_back(time_of_day_ms, a, b);
}

#endif
