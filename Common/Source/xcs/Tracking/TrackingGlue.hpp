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

#ifndef XCSOAR_TRACK_THREAD_HPP
#define XCSOAR_TRACK_THREAD_HPP

#include "Tracking/Features.hpp"

#ifdef HAVE_TRACKING

#include "Tracking/SkyLines/Glue.hpp"
#include "Tracking/TrackingSettings.hpp"
#include "Tracking/SkyLines/Handler.hpp"
#include "Tracking/SkyLines/Data.hpp"
#include "Time/PeriodClock.hpp"
#include "Geographic/GeoPoint.h"

struct NMEA_INFO;
struct DERIVED_INFO;

class TrackingGlue
#if defined(HAVE_SKYLINES_TRACKING_HANDLER)
  : private SkyLinesTracking::Handler
#endif
{
  PeriodClock clock;

  TrackingSettings settings;

#ifdef HAVE_SKYLINES_TRACKING
  SkyLinesTracking::Glue skylines;

#ifdef HAVE_SKYLINES_TRACKING_HANDLER
  SkyLinesTracking::Data skylines_data;
#endif
#endif

public:
  TrackingGlue();

  void SetSettings(const TrackingSettings &_settings);
  void OnTimer(const NMEA_INFO &basic, const DERIVED_INFO &calculated);


#ifdef HAVE_SKYLINES_TRACKING_HANDLER
protected:
  /* virtual methods from SkyLinesTracking::Handler */
  void OnTraffic(uint32_t pilot_id, unsigned time_of_day_ms,
                 const GeoPoint &location, int altitude) override;

  void OnUserName(uint32_t user_id, const TCHAR *name) override;

  void OnWave(unsigned time_of_day_ms,
              const GeoPoint &a, const GeoPoint &b) override;

public:
  const SkyLinesTracking::Data &GetSkyLinesData() const {
    return skylines_data;
  }
#endif
};

#endif /* HAVE_TRACKING */
#endif
