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
#include <chrono>
#include "BaseTracking.h"
#include "Geographic/GeoPoint.h"

struct NMEA_INFO;
struct DERIVED_INFO;

class FFVLTracking final : public BaseTracking<AGeoPoint> {
public:
  FFVLTracking() = delete;
  explicit FFVLTracking(std::string user_key);

  ~FFVLTracking() override;

  void Update(const NMEA_INFO &basic, const DERIVED_INFO &calculated) override;

private:

  void Send(http_session& http, const AGeoPoint& position) override;

  const std::string _user_key;

  using gps_time = std::chrono::duration<double>;

  gps_time next_time = {};
};

#endif // _TRACKING_FFVLTRACKING_H_
