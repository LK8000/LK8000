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

#ifndef _TRACKING_FFVLTRACKING_H_
#define _TRACKING_FFVLTRACKING_H_

#include <string>
#include <optional>
#include <chrono>
#include "Thread/Thread.hpp"
#include "Thread/Cond.hpp"
#include "Geographic/GeoPoint.h"
#include "http_session.h"

struct NMEA_INFO;
struct DERIVED_INFO;

class FFVLTracking : public Thread {
public:
  FFVLTracking() = delete;
  explicit FFVLTracking(const std::string& user_key);

  ~FFVLTracking();

  void Update(const NMEA_INFO &basic, const DERIVED_INFO &calculated);

private:

  void Run() override;

  void Send(AGeoPoint position) const;

  const std::string _user_key;

  bool thread_stop = false;
  std::optional<AGeoPoint> queue;
  Mutex queue_mtx;
  Cond queue_cv;

  using gps_time = std::chrono::duration<double>;

  gps_time next_time = {};

  http_session http;
};

#endif // _TRACKING_FFVLTRACKING_H_
