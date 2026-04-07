/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#include "OsmAndTracking.h"
#include "NMEA/Info.h"
#include "NMEA/Derived.h"
#include <format>
#include <iostream>
#include "BatteryManager.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

OsmAndTracking::OsmAndTracking(const tracking::Profile& profile)
    : BaseTracking("osmand_tracker"),
      _url(profile.server),
      _device_id(profile.user),
      _interval(std::chrono::seconds(profile.interval)) {}

OsmAndTracking::~OsmAndTracking() {
  StopAndJoin();
}

void OsmAndTracking::Update(const NMEA_INFO &basic, const DERIVED_INFO &calculated) {
  using namespace std::chrono_literals;

  if (basic.NAVWarning) {
    return; // ignore invalid fix
  }

  auto time_now = gps_time(basic.Time);

  if (time_now > next_time) {
    next_time = time_now + _interval;

    auto timestamp = std::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.000Z",
                                 basic.Year, basic.Month, basic.Day, basic.Hour,
                                 basic.Minute, basic.Second);

    Push({{{basic.Latitude, basic.Longitude}, basic.Altitude},
          basic.Speed,
          basic.TrackBearing,
          std::move(timestamp),
          calculated.Flying});
  }
}

extern int PDABatteryPercent;
extern int PDABatteryStatus;
extern int PDABatteryFlag;

void OsmAndTracking::Send(http_session& http, const OsmAndData& data) {

  json location = {
    {"timestamp", data.timestamp},
    {"coords", {
      {"latitude", data.position.latitude},
      {"longitude", data.position.longitude},
      {"altitude", data.position.altitude},
      {"speed", data.speed},
      {"heading", data.heading},
    }},
    {"activity", {
      {"type", data.flying ? "flying" : "still"},
    }},
  };

  double level = (PDABatteryPercent >= 0 && PDABatteryPercent <= 100)
                     ? PDABatteryPercent / 100.0
                     : -1.0;

  if (level >= 0.0) {
    bool charging = PDABatteryFlag == Battery::CHARGING ||
                    PDABatteryStatus == Battery::ONLINE;

    location["battery"] = {
      {"level", level},
      {"is_charging", charging},
    };
  }

  json root = {
    {"location", std::move(location)},
    {"device_id", _device_id},
  };

  http.post(_url, root.dump(), "application/json");
}
