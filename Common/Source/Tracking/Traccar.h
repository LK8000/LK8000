/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#ifndef _TRACKING_TRACCAR_H_
#define _TRACKING_TRACCAR_H_

#include <string>
#include <chrono>
#include "Geographic/GeoPoint.h"
#include "BaseTracking.h"
#include "Tracking.h"

struct NMEA_INFO;
struct DERIVED_INFO;

struct TraccarData {
  AGeoPoint position = {};
  double speed = 0.0;
  double heading = 0.0;
  std::string timestamp;
  bool flying = false;
};

class Traccar final : public BaseTracking<TraccarData> {
public:
  Traccar() = delete;

  Traccar(const Traccar&) = delete;
  Traccar& operator=(const Traccar&) = delete;

  Traccar(Traccar&&) = delete;
  Traccar& operator=(Traccar&&) = delete;

  explicit Traccar(const tracking::Profile& profile);

  ~Traccar() override;

  void Update(const NMEA_INFO &basic, const DERIVED_INFO &calculated) override;

protected:
  void Send(http_session& http, const TraccarData& data) override;

private:
  const std::string _url;
  const std::string _device_id;
  const std::chrono::seconds _interval;

  using gps_time = std::chrono::duration<double>;
  gps_time next_time = {};
};

#endif // _TRACKING_OSMANDTRACKING_H_
