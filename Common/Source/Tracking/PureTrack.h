/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <deque>
#include <string>
#include <vector>

#include "Geographic/GeoPoint.h"
#include "ITrackingHandler.h"
#include "Thread/Cond.hpp"
#include "Thread/Mutex.hpp"
#include "Thread/Thread.hpp"
#include "Tracking.h"
#include "Asset.hpp"

struct NMEA_INFO;
struct DERIVED_INFO;
class http_session;

class PureTrack final : public Thread, public ITrackingHandler {
 public:

  struct PureTrackPoint {
    time_t timestamp = 0;
    AGeoPoint position = {};
    double speed = 0.0;
    double heading = 0.0;
  };

  PureTrack() = delete;

  PureTrack(const PureTrack&) = delete;
  PureTrack& operator=(const PureTrack&) = delete;

  PureTrack(PureTrack&&) = delete;
  PureTrack& operator=(PureTrack&&) = delete;

  explicit PureTrack(const std::string& id, const tracking::Profile& profile);

  ~PureTrack() override;

  void Update(const NMEA_INFO& basic, const DERIVED_INFO& calculated) override;

 private:
  using PureTrackBatch = std::vector<PureTrackPoint>;
  using interval_t = std::chrono::seconds;
  using gps_time = std::chrono::duration<double, std::ratio<1>>;

  static size_t MaxQueuePoints(interval_t interval);

  bool Push(const NMEA_INFO& basic);
  bool Push(PureTrackPoint&& point);

  void Run() override;

  void StopAndJoin();

  bool WaitForBatch(bool retry_pending, PureTrackBatch& batch);
  bool Send(http_session& http, const PureTrackBatch& data) const;
  bool HandleSendResult(time_t timestamp);

  static constexpr const char* api_endpoint = "https://puretrack.io/api/insert";
  static constexpr const char* api_key = "343lk8000gjuhu";

  const std::string device_id;
  const interval_t interval;
  const bool always_on;
  const size_t max_queue_points;

  bool thread_stop = false;
  std::deque<PureTrackPoint> queue;

  Mutex queue_mtx;
  Cond queue_cv;

  gps_time next_time = {};
};
