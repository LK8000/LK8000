/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */
#ifndef TRACKING_LIVETRACK24V1HANDLER_H_
#define TRACKING_LIVETRACK24V1HANDLER_H_

#include "ITrackingHandler.h"
#include "Tracking.h"
#include "LiveTrack24Common.h"
#include "Thread/Thread.hpp"
#include "Thread/Mutex.hpp"
#include "Thread/Cond.hpp"
#include <deque>

class LiveTrack24V1Handler final : public ITrackingHandler, private Thread {
 public:
  LiveTrack24V1Handler() = delete;

  LiveTrack24V1Handler(const LiveTrack24V1Handler&) = delete;
  LiveTrack24V1Handler(LiveTrack24V1Handler&&) = delete;

  LiveTrack24V1Handler& operator=(const LiveTrack24V1Handler&) = delete;
  LiveTrack24V1Handler& operator=(LiveTrack24V1Handler&&) = delete;

  explicit LiveTrack24V1Handler(const tracking::Profile& profile);
  ~LiveTrack24V1Handler() override;

  void Update(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) override;

 private:
  void Run() override;
  bool InterruptibleSleep(int msecs);

  int GetUserIDFromServer(class http_session& http);
  bool SendStartOfTrackPacket(class http_session& http, unsigned int* packet_id,
                              unsigned int* session_id, int userid);
  bool SendEndOfTrackPacket(class http_session& http, unsigned int* packet_id,
                            unsigned int* session_id);
  bool SendGPSPointPacket(class http_session& http, unsigned int* packet_id,
                          unsigned int* session_id,
                          const livetracker_point_t& sendpoint);

  const tracking::Profile m_profile;
  int m_logtime = 0;

  Mutex m_mutex;
  Cond m_cond;
  bool m_run = false;
  std::deque<livetracker_point_t> m_points;
};

#endif  // TRACKING_LIVETRACK24V1HANDLER_H_
