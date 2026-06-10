/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   OsmAnd.h
 * Author: Bruno de Lacheisserie
 *
 * Created on June 03, 2026
 */

#pragma once

#include "BaseTracking.h"
#include "Geographic/GeoPoint.h"
#include "Tracking.h"

struct NMEA_INFO;
struct DERIVED_INFO;

struct OsmAndData {
  time_t timestamp = 0;
  AGeoPoint position = {};
  double speed = 0.0;
  double heading = 0.0;
};

class OsmAnd final : public BaseTracking<OsmAndData> {
 public:
  OsmAnd() = delete;

  OsmAnd(const OsmAnd&) = delete;
  OsmAnd& operator=(const OsmAnd&) = delete;

  OsmAnd(OsmAnd&&) = delete;
  OsmAnd& operator=(OsmAnd&&) = delete;

  explicit OsmAnd(const tracking::Profile& profile);

  ~OsmAnd() override;

  void Update(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) override;

 protected:
  void Send(http_session& http, const OsmAndData& data) override;

 private:
  const std::string _url;
  const std::string _device_id;
  const std::chrono::seconds _interval;

  using gps_time = std::chrono::duration<double>;
  gps_time next_time = {};
};
