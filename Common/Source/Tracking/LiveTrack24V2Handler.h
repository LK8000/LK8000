/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */
#ifndef TRACKING_LIVETRACK24V2HANDLER_H
#define TRACKING_LIVETRACK24V2HANDLER_H
#include "ITrackingHandler.h"
#include "Tracking.h"
#include "LiveTrack24Common.h"
#include "Thread/Thread.hpp"
#include "Thread/Mutex.hpp"
#include "Thread/Cond.hpp"
#include <deque>
#include <string>
#include <memory>

#include "nlohmann/json.hpp"
using json = nlohmann::json;


class LiveTrack24V2Handler final : public ITrackingHandler {
 public:
  LiveTrack24V2Handler() = delete;
  LiveTrack24V2Handler(const LiveTrack24V2Handler&) = delete;
  LiveTrack24V2Handler& operator=(const LiveTrack24V2Handler&) = delete;

  explicit LiveTrack24V2Handler(const tracking::Profile& profile);
  ~LiveTrack24V2Handler() override;

  void Update(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) override;

 private:
  // Forward declarations for nested classes
  class TrackerThread;
  class RadarThread;

  // Helper functions
  json callLiveTrack24(class http_session& http,
                       const std::string& subURL,
                       bool calledSelf = false);
  bool LiveTrack24_Radar(class http_session& http);

  int GetUserIDFromServer2(class http_session& http);
  bool SendEndOfTrackPacket2(class http_session& http, unsigned int* packet_id);
  bool SendGPSPointPacket2(class http_session& http, unsigned int* packet_id);
  void PopSentPoints(time_t last_unix_timestamp);

  const tracking::Profile m_profile;
  int m_logtime = 0;

  // Shared state
  Mutex m_tracker_mutex;
  Cond m_tracker_cond;
  bool m_run_tracker = false;

  Mutex m_radar_mutex;
  Cond m_radar_cond;
  bool m_run_radar = false;
  std::deque<livetracker_point_t> m_points;

  // Threads
  std::unique_ptr<TrackerThread> m_trackerThread;
  std::unique_ptr<RadarThread> m_radarThread;

  // V2 state can be moved here
  std::string m_deviceID;
  std::string m_ut = "g_ut";
  std::string m_otpQuestion = "98d89cafa872ccdc";
  int m_sync = 0;
  bool m_flarmwasinit = false;

  std::string m_v2_pwt;
  int m_v2_sid = 0;
  int m_v2_userid = -1;
};

#endif  // TRACKING_LIVETRACK24V2HANDLER_H
