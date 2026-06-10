/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   OsmAnd.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on June 03, 2026
 */
#include "OsmAnd.h"
#include "NMEA/Info.h"
#include "NMEA/Derived.h"
#include "Library/TimeFunctions.h"

OsmAnd::OsmAnd(const tracking::Profile& profile)
    : BaseTracking("osmand_legacy_tracker"),
      _url(profile.server),
      _device_id(profile.user),
      _interval(std::chrono::seconds(profile.interval)) {
  Start();
}

OsmAnd::~OsmAnd() {
  StopAndJoin();
}

void OsmAnd::Update(const NMEA_INFO& Basic,
                          const DERIVED_INFO& Calculated) {
  using namespace std::chrono_literals;

  if (Basic.NAVWarning) {
    return;  // ignore invalid fix
  }

  auto time_now = gps_time(Basic.Time);

  if (time_now > next_time) {
    next_time = time_now + _interval;

    Push({
      to_time_t(Basic),
      AGeoPoint({Basic.Latitude, Basic.Longitude}, Basic.Altitude),
      Basic.Speed, Basic.TrackBearing
    });
  }
}

void OsmAnd::Send(http_session& http, const OsmAndData& data) {
  http.get(_url, {
    {"id", _device_id},
    {"valid", "true"},
    {"timestamp", data.timestamp},
    {"lat", data.position.latitude},
    {"lon", data.position.longitude},
    {"altitude", data.position.altitude},
    {"speed", data.speed},
    {"heading", data.heading},
  });
}
