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

FFVLTracking::FFVLTracking(const std::string& user_key)
            : Thread("ffvl_tracker"), _user_key(user_key) {
  Start();
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
  ScopeLock lock(queue_mtx);
  if (basic.NAVWarning) {
    return; // ignore invalide fix
  }

  using namespace std::chrono_literals;
  auto time_now = gps_time(basic.Time);

  if (time_now > next_time) {
    next_time = time_now + 60s;

    queue = {{basic.Latitude, basic.Longitude}, basic.Altitude};
    
    queue_cv.Signal();
  }
}

void FFVLTracking::Send(AGeoPoint position) const {

  std::string url = R"(https://data.ffvl.fr/api/?device_type=LK8000)";
  url += R"(&key=a0ac0e7592a0c827cbc8a7b95737d044)";
  url += R"(&ffvl_tracker_key=)" + _user_key;
  url += R"(&latitude=)" + std::to_string(position.latitude);
  url += R"(&longitude=)" + std::to_string(position.longitude);
  url += R"(&altitude=)" + std::to_string(position.altitude);

  http.request(url);
}

void FFVLTracking::Run() {
  while(true) {
    // get copy of queue
    auto position = WithLock(queue_mtx, [&]() {
      return std::exchange(queue, std::nullopt);
    });

    if (position) {
      Send(position.value());
    } else {
      ScopeLock lock(queue_mtx);
      // no new position check for stop request
      if (thread_stop) {
        return;  // stop requested...
      }
      // wait for stop or new position
      queue_cv.Wait(queue_mtx);
    }
  }
}
