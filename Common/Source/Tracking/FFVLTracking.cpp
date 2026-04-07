/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   FFVLTracking.h
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

  std::string url = R"(https://data.ffvl.fr/api/?device_type=LK8000)";
  url += R"(&key=a0ac0e7592a0c827cbc8a7b95737d044)";
  url += R"(&ffvl_tracker_key=)" + _user_key;
  url += R"(&latitude=)" + std::to_string(position.latitude);
  url += R"(&longitude=)" + std::to_string(position.longitude);
  url += R"(&altitude=)" + std::to_string(position.altitude);

  http.request(url);
}