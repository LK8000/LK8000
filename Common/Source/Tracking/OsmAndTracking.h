/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#ifndef _TRACKING_OSMANDTRACKING_H_
#define _TRACKING_OSMANDTRACKING_H_

#include <string>
#include <chrono>
#include "Geographic/GeoPoint.h"
#include "BaseTracking.h"
#include "Tracking.h"

struct NMEA_INFO;
struct DERIVED_INFO;

struct OsmAndData {
  AGeoPoint position = {};
  double speed = 0.0;
  double heading = 0.0;
  std::string timestamp;
  bool flying = false;
};

class OsmAndTracking final : public BaseTracking<OsmAndData> {
public:
  OsmAndTracking() = delete;
  explicit OsmAndTracking(const tracking::Profile& profile);

  ~OsmAndTracking() override;

  void Update(const NMEA_INFO &basic, const DERIVED_INFO &calculated) override;

protected:
  void Send(http_session& http, const OsmAndData& data) override;

private:
  const std::string _url;
  const std::string _device_id;
  const std::chrono::seconds _interval;

  using gps_time = std::chrono::duration<double>;
  gps_time next_time = {};
};

#endif // _TRACKING_OSMANDTRACKING_H_
