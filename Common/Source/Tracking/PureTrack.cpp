/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#include "PureTrack.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include "http_session.h"
#include "Library/TimeFunctions.h"
#include "NMEA/Info.h"
#include "NMEA/Derived.h"
#include "Units.h"
#include "Enums.h"
#include "MessageLog.h"

using json = nlohmann::json;
using namespace std::chrono_literals;

extern AircraftCategory_t AircraftCategory;

namespace {

int type_id() {
  switch (AircraftCategory) {
    case AircraftCategory_t::umGlider:
      return 1;
    case AircraftCategory_t::umParaglider:
      return 4;
    case AircraftCategory_t::umCar:
      return 22;
    case AircraftCategory_t::umGAaircraft:
      return 8;
    default:
      return 0;
  }
}

}  // namespace

void to_json(json& j, const PureTrack::PureTrackPoint& point) {
  j = json{
      {"ts", point.timestamp},
      {"lat", point.position.latitude},
      {"lng", point.position.longitude},
      {"alt", Units::To(unMeter, point.position.altitude)},
      {"speed", Units::To(unMeterPerSecond, point.speed)},
      {"course", point.heading},
  };
}

PureTrack::PureTrack(const std::string& id, const tracking::Profile& profile)
    : Thread("puretrack"),
      device_id(id),
      interval(std::chrono::seconds(std::clamp(profile.interval, 5, 300))),
      always_on(profile.always_on),
      max_queue_points(MaxQueuePoints(interval)) {
  Start();
}

PureTrack::~PureTrack() {
  StopAndJoin();
}

size_t PureTrack::MaxQueuePoints(interval_t interval) {
  // Estimate the maximum number of points that can be queued for 30 minutes
  // of tracking data, based on the configured sending interval.
  auto max_points =
      std::chrono::duration_cast<interval_t>(30min).count() /
      std::max<int>(1, interval.count());
  return static_cast<std::size_t>(max_points);
}

void PureTrack::Update(const NMEA_INFO& basic, const DERIVED_INFO& calculated) {
  if (basic.NAVWarning) {
    return;  // ignore invalid fix
  }
  if (!always_on && !calculated.Flying) {
    return;
  }

  auto time_now = gps_time(basic.Time);

  if (next_time <= gps_time::zero()) {
    next_time = time_now + interval;
  }

  if (time_now >= next_time) {
    next_time = time_now + interval;

    if (Push(basic)) {
      queue_cv.notify_all();
    }
  }
  else if (calculated.Circling && max_queue_points >= 60U) {
    // add to queue in thermal if max_queue_points is large enough to hold 1 minute of points,
    // but do not notify queue_cv to only send at the scheduled interval.
    // this allows for more accurate tracking of thermalling without increasing
    // the number of requests sent to the server.
    Push(basic);
  }
}

bool PureTrack::Push(const NMEA_INFO& basic) {
  return Push({
      to_time_t(basic),
      {{basic.Latitude, basic.Longitude}, basic.Altitude},
      basic.Speed,
      basic.TrackBearing,
  });
}

bool PureTrack::Push(PureTrackPoint&& point) {
  std::lock_guard lock(queue_mtx);
  // never send more than 1 point per seconds !
  if (queue.empty() || queue.back().timestamp + 1 <= point.timestamp) {
    if (queue.size() >= max_queue_points) {
      queue.pop_front();
    }
    queue.emplace_back(std::move(point));
    return true;
  }
  return false;
}

void PureTrack::Run() {
  StartupStore(_T(R"(Start "puretrack.io" tracking)"));

  http_session http;
  bool retry_pending = false;

  PureTrackBatch batch;

  while (true) {
    if (!WaitForBatch(retry_pending, batch)) {
      return;
    }

    retry_pending = !Send(http, batch);

    if (!retry_pending) {
      if (!HandleSendResult(batch.back().timestamp)) {
        return;
      }
    }
  }

  StartupStore(_T(R"(Stop "puretrack.io" tracking)"));
}

// Waits for a batch of points to send, or until the thread is stopped.
// @return: true if a batch is ready to send,
//          false if the thread is stopped.
bool PureTrack::WaitForBatch(bool retry_pending, PureTrackBatch& batch) {
  std::unique_lock<Mutex> lock(queue_mtx);

  if (retry_pending) {
    // always wait at least 20 seconds before retrying
    queue_cv.wait_for(lock, 20s, [&]() {
      return thread_stop;
    });

    if (thread_stop) {
      return false;
    }
  }

  // wait until we have at least one point to send,
  // or until the thread is stopped
  queue_cv.wait(lock, [&]() {
    return thread_stop || !queue.empty();
  });

  if (thread_stop) {
    return false;
  }

  // copy the current queue to the batch to send
  batch.assign(queue.begin(), queue.end());
  return true;
}

bool PureTrack::Send(http_session& http, const PureTrackBatch& data) const {
  if (data.empty()) {
    return true;
  }

  json send_payload = {
      {"key", api_key},
      {"deviceID", device_id},
      {"type", type_id()},
      {"points", data},
  };

  DebugLog("PureTrack: > %s", send_payload.dump().c_str());
  
  const auto response =
      http.post(api_endpoint, send_payload.dump(), "application/json");
  if (response.empty()) {
    return false;
  }

  DebugLog("PureTrack: < %s", response.c_str());

  try {
    auto response_payload = json::parse(response);
    if (response_payload.value("http_code", 0) != 200) {
      return false;
    }
    if (response_payload.value("success", false)) {
      return true;
    }
  }
  catch (std::exception& e) {
    DebugLog("PureTrack: JSON parse error: %s", e.what());
  }
  return false;
}

// Handles the result of a successful send operation by removing points
// from the queue that have been sent successfully, based on the timestamp
// of the last point in the batch.
// @return: true if the thread should continue,
//          false if the thread should stop.
bool PureTrack::HandleSendResult(time_t timestamp) {
  std::lock_guard<Mutex> lock(queue_mtx);
  if (thread_stop) {
    return false;
  }

  while (!queue.empty() && queue.front().timestamp <= timestamp) {
    queue.pop_front();
  }

  return true;
}

// Stops the thread and waits for it to finish.
void PureTrack::StopAndJoin() {
  WithLock(queue_mtx, [&]() {
    thread_stop = true;
  });
  queue_cv.notify_all();

  if (IsDefined()) {
    Join();
  }
}
