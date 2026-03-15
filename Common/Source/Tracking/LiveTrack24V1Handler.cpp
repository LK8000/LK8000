/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#include "LiveTrack24V1Handler.h"
#include "http_session.h"
#include "externs.h"
#include "utils/stringext.h"
#include "NavFunctions.h"
#include "OS/Sleep.h"
#include "Library/TimeFunctions.h"
#include "MessageLog.h"

namespace {
// Encode URLs in a standard form
char* UrlEncode(const char* szText, char* szDst, int bufsize) {
  static constexpr char hex_chars[16] = {'0', '1', '2', '3', '4', '5',
                                         '6', '7', '8', '9', 'A', 'B',
                                         'C', 'D', 'E', 'F'};

  int iMax = bufsize - 2;
  int j = 0;
  szDst[0] = '\0';
  for (int i = 0; szText[i] && j <= iMax; i++) {
    const char ch = szText[i];
    if (isalnum(ch)) {
      szDst[j++] = ch;
    }
    else if (ch == ' ') {
      szDst[j++] = '+';
    }
    else {
      if (j + 2 > iMax) {
        break;
      }
      szDst[j++] = '%';
      szDst[j++] = (hex_chars[((ch) & 0xF0) >> 4]);
      szDst[j++] = (hex_chars[((ch) & 0x0F) >> 0]);
    }
  }
  szDst[j] = '\0';
  return szDst;
}
}  // namespace

LiveTrack24V1Handler::LiveTrack24V1Handler(const tracking::Profile& profile)
    : Thread("LT24v1"), m_profile(profile) {
  m_run = true;
  Start();
  StartupStore(_T(". LiveTracker API V1 will use server %s."),
               m_profile.server.c_str());
}

LiveTrack24V1Handler::~LiveTrack24V1Handler() {
  if (IsDefined()) {
    m_run = false;
    m_newDataEvent.set();
    Join();
    StartupStore(_T(". LiveTracker V1 closed."));
  }
}

void LiveTrack24V1Handler::Update(const NMEA_INFO& Basic,
                                  const DERIVED_INFO& Calculated) {
  if (m_profile.interval <= 0 || Basic.NAVWarning) {
    return;
  }

  if (Basic.Time >= m_logtime) {
    m_logtime = (int)Basic.Time + m_profile.interval;
    if (m_logtime >= 86400) {
      m_logtime -= 86400;
    }
  }
  else {
    return;
  }

  livetracker_point_t newpoint;
  newpoint.unix_timestamp = to_time_t(Basic);
  newpoint.flying = m_profile.always_on || Calculated.Flying;
  newpoint.latitude = Basic.Latitude;
  newpoint.longitude = Basic.Longitude;
  newpoint.alt = Basic.Altitude;
  newpoint.ground_speed = Basic.Speed;
  newpoint.course_over_ground = Calculated.Heading;

  WithLock(m_mutex, [&]() {
    if (m_points.size() > static_cast<size_t>(1800 / m_profile.interval)) {
      m_points.pop_front();
    }
    m_points.emplace_back(newpoint);
  });
  m_newDataEvent.set();
}

void LiveTrack24V1Handler::InterruptibleSleep(int msecs) {
  if (m_newDataEvent.tryWait(msecs)) {
    m_newDataEvent.reset();
  }
}

int LiveTrack24V1Handler::GetUserIDFromServer(http_session& http) {
  int retval = -1;
  char txbuf[512];
  char username[128];
  char password[128];

  to_utf8(m_profile.user.c_str(), txbuf);
  UrlEncode(txbuf, username, std::size(username));
  to_utf8(m_profile.password.c_str(), txbuf);
  UrlEncode(txbuf, password, std::size(password));
  lk::snprintf(txbuf, "/client.php?op=login&user=%s&pass=%s", username, password);

  std::string response = http.request(m_profile.server.c_str(), m_profile.port, txbuf);
  if (!response.empty()) {
    sscanf(response.c_str(), "%d", &retval);
  }
  return retval;
}

bool LiveTrack24V1Handler::SendStartOfTrackPacket(http_session& http,
                                                  unsigned int* packet_id,
                                                  unsigned int* session_id,
                                                  int userid) {
  char username[100];
  char password[100];
  char txbuf[510];
  char phone[64];
  char gps[64];
  unsigned int vehicle_type = 8;
  char vehicle_name[64];

  if (!m_profile.user.empty()) {
    to_utf8(m_profile.user.c_str(), txbuf);
  }
  else {
    lk::strcpy(txbuf, "guest");
  }
  UrlEncode(txbuf, username, std::size(username));

  if (!m_profile.password.empty()) {
    to_utf8(m_profile.password.c_str(), txbuf);
  }
  else {
    lk::strcpy(txbuf, "guest");
  }
  UrlEncode(txbuf, password, std::size(password));

#ifdef PNA
  to_utf8(ModelType::GetName(), txbuf);
  UrlEncode(txbuf, phone, std::size(phone));
#elif (WINDOWSPC > 0)
  UrlEncode("PC", phone, std::size(phone));
#else
  UrlEncode("PDA", phone, std::size(phone));
#endif

  UrlEncode(SIMMODE ? "SIMULATED" : "GENERIC", gps, std::size(gps));

  to_utf8(AircraftType_Config, txbuf);
  UrlEncode(txbuf, vehicle_name, std::size(vehicle_name));
  switch (AircraftCategory) {
    case AircraftCategory_t::umParaglider:
      vehicle_type = 1;
      break;
    case AircraftCategory_t::umCar:
      vehicle_type = 17100;
      break;
    case AircraftCategory_t::umGAaircraft:
      vehicle_type = 64;
      break;
    default:
      vehicle_type = 8;
      break;
  }

  *packet_id = 1;
  *session_id =
      ((rand() << 24) & 0x7F000000) | (userid & 0x00ffffff) | 0x80000000;

  lk::snprintf(txbuf,
          "/track.php?leolive=2&sid=%u&pid=1&client=%s&v=%s%s&user=%s&pass=%s&"
          "phone=%s&gps=%s&trk1=%u&vtype=%u&vname=%s",
          *session_id, LKFORK, LKVERSION, LKRELEASE, username, password, phone,
          gps, m_profile.interval, vehicle_type, vehicle_name);

  std::string response = http.request(m_profile.server.c_str(), m_profile.port, txbuf);
  if (response == "OK") {
    (*packet_id)++;
    return true;
  }
  return false;
}

bool LiveTrack24V1Handler::SendEndOfTrackPacket(http_session& http,
                                                unsigned int* packet_id,
                                                unsigned int* session_id) {
  char txbuf[500];
  lk::snprintf(txbuf, "/track.php?leolive=3&sid=%u&pid=%u&prid=0", *session_id,
          *packet_id);
  std::string response = http.request(m_profile.server.c_str(), m_profile.port, txbuf);
  if (response == "OK") {
    (*packet_id)++;
    return true;
  }
  return false;
}

bool LiveTrack24V1Handler::SendGPSPointPacket(
    http_session& http, unsigned int* packet_id, unsigned int* session_id,
    const livetracker_point_t& sendpoint) {
  char txbuf[500];
  lk::snprintf(txbuf,
          "/track.php?leolive=4&sid=%u&pid=%u&lat=%.5f&lon=%.5f&alt=%.0f&sog=%."
          "0f&cog=%.0f&tm=%zu",
          *session_id, *packet_id, sendpoint.latitude, sendpoint.longitude,
          Units::To(Units_t::unMeter, sendpoint.alt),
          Units::To(Units_t::unKiloMeterPerHour, sendpoint.ground_speed),
          sendpoint.course_over_ground, sendpoint.unix_timestamp);

  std::string response = http.request(m_profile.server.c_str(), m_profile.port, txbuf);
  if (response == "OK") {
    (*packet_id)++;
    return true;
  }
  return false;
}

void LiveTrack24V1Handler::Run() {
  int tracker_fsm = 0;
  livetracker_point_t sendpoint = {};
  bool sendpoint_valid = false;
  bool sendpoint_processed = false;
  bool sendpoint_processed_old = false;
  unsigned int packet_id = 0;
  unsigned int session_id = 0;
  int userid = -1;

  srand(MonotonicClockMS());
  http_session http;

  do {
    InterruptibleSleep(5000);
    if (!m_run) {
      break;
    }

    do {
      sendpoint_valid = WithLock(m_mutex, [&]() {
        if (!m_points.empty()) {
          sendpoint = m_points.front();
          return true;
        }
        return false;
      });

      if (sendpoint_valid) {
        sendpoint_processed = false;
        do {
          switch (tracker_fsm) {
            default:
            case 0:  // Wait for flying
              if (!sendpoint.flying) {
                sendpoint_processed = true;
                break;
              }
              tracker_fsm++;
              break;

            case 1:  // Get User ID
              userid = GetUserIDFromServer(http);
              if (userid >= 0) {
                tracker_fsm++;
              }
              break;

            case 2:  // Start of track packet
              sendpoint_processed =
                  SendStartOfTrackPacket(http, &packet_id, &session_id, userid);
              if (sendpoint_processed) {
                StartupStore(_T(". Livetracker new track started."));
                sendpoint_processed_old = true;
                tracker_fsm++;
              }
              break;

            case 3:  // Gps point packet
              sendpoint_processed =
                  SendGPSPointPacket(http, &packet_id, &session_id, sendpoint);
              if (sendpoint_processed_old && !sendpoint_processed) {
                StartupStore(_T(". Livetracker connection to server lost."));
              }
              if (!sendpoint_processed_old && sendpoint_processed) {
                int queue_size = WithLock(m_mutex, [&]() {
                  return m_points.size();
                });
                StartupStore(
                    _T(". Livetracker connection to server established, start ")
                    _T("sending %d queued packets."),
                    queue_size);
              }
              sendpoint_processed_old = sendpoint_processed;
              if (!sendpoint.flying) {
                tracker_fsm++;
              }
              break;

            case 4:  // End of track packet
              sendpoint_processed =
                  SendEndOfTrackPacket(http, &packet_id, &session_id);
              if (sendpoint_processed) {
                StartupStore(
                    _T(". Livetracker track finished, sent %d points."),
                    packet_id);
                tracker_fsm = 0;
              }
              break;
          }

          if (sendpoint_processed) {
            ScopeLock guard(m_mutex);
            m_points.pop_front();
          }
          else {
            InterruptibleSleep(2500);
            if (!m_run) {
              break;
            }
          }
        } while (!sendpoint_processed && m_run);
      }
    } while (sendpoint_valid && m_run);
  } while (m_run);
}
