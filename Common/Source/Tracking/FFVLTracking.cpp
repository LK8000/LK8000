/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   FFVLTracking.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on February 18, 2024
 */

#include <utility>
#include <string>
#include "FFVLTracking.h"
#include "NMEA/Info.h"
#include "http_session.h"

FFVLTracking::FFVLTracking(std::string user_key)
            : BaseTracking("ffvl_tracker"), _user_key(std::move(user_key)) {
  Start();
}

FFVLTracking::~FFVLTracking() {
  StopAndJoin();
}

void FFVLTracking::Update(const NMEA_INFO &basic, const DERIVED_INFO &calculated) {
  using namespace std::chrono_literals;

  if (basic.NAVWarning) {
    return; // ignore invalid fix
  }

  auto time_now = gps_time(basic.Time);

  if (time_now > next_time) {
    next_time = time_now + 60s;

    Push({{basic.Latitude, basic.Longitude}, basic.Altitude});
  }
}

void FFVLTracking::Send(http_session& http, const AGeoPoint& position) {
  http.get("https://data.ffvl.fr/api/", {
    {"device_type", "LK8000"},
    {"key", "a0ac0e7592a0c827cbc8a7b95737d044"},
    {"ffvl_tracker_key", _user_key},
    {"latitude", position.latitude},
    {"longitude", position.longitude},
    {"altitude", position.altitude}
  });
}
