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
#include "../Thread/Thread.hpp"

FFVLTracking::FFVLTracking(std::string user_key)
            : Thread("ffvl_tracker"), _user_key(std::move(user_key)) {
}

FFVLTracking::~FFVLTracking() {
  WithLock(queue_mtx, [&]() {
    thread_stop = true;
  });
  queue_cv.Broadcast();
  if (IsDefined()) {
    Join();
  }
}

void FFVLTracking::Update(const NMEA_INFO &basic, const DERIVED_INFO &calculated) {
  using namespace std::chrono_literals;

  if (basic.NAVWarning) {
    return; // ignore invalid fix
  }

  auto time_now = gps_time(basic.Time);

  if (time_now > next_time) {
    next_time = time_now + 60s;

    WithLock(queue_mtx, [&]() {
      queue = {{basic.Latitude, basic.Longitude}, basic.Altitude};
    });
    queue_cv.Broadcast();
  }
}

void FFVLTracking::Send(http_session& http, const AGeoPoint& position) const {

  std::string url = R"(https://data.ffvl.fr/api/?device_type=LK8000)";
  url += R"(&key=a0ac0e7592a0c827cbc8a7b95737d044)";
  url += R"(&ffvl_tracker_key=)" + _user_key;
  url += R"(&latitude=)" + std::to_string(position.latitude);
  url += R"(&longitude=)" + std::to_string(position.longitude);
  url += R"(&altitude=)" + std::to_string(position.altitude);

  http.request(url);
}

void FFVLTracking::Run() {

  http_session http;

  do {
    // get copy of queue
    auto position = WithLock(queue_mtx, [&]() {
      return std::exchange(queue, std::nullopt);
    });

    if (position) {
      Send(http, position.value());
    }
  }
  while(Wait());
}

bool FFVLTracking::Wait() {
  ScopeLock lock(queue_mtx);
  // if no new position wait for stop or new position
  while (!thread_stop && !queue.has_value()) {
    queue_cv.Wait(queue_mtx);
  }
  return !thread_stop;
}