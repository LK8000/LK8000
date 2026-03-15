/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights.
 */

#include "LiveTrack24V2Handler.h"
#include "http_session.h"
#include "externs.h"
#include "utils/stringext.h"
#include "utils/hmac_sha2.h"
#include "utils/base64.h"
#include "NavFunctions.h"
#include "OS/Sleep.h"
#include "Library/TimeFunctions.h"
#include "MessageLog.h"
#include "LiveTrack24APIKey.h"
#include "FlarmCalculations.h"
#include "md5.h"
#include <zlib.h>
#include <random>
#include <sstream>

extern NMEA_INFO GPS_INFO;
extern Mutex CritSec_FlightData;
extern double LastFlarmCommandTime;
extern bool flarmwasinit;

// --- Helper classes for threads ---
class LiveTrack24V2Handler::TrackerThread final : public Thread {
 public:
  explicit TrackerThread(LiveTrack24V2Handler& handler)
      : Thread("LT24v2Tracker"), m_handler(handler) {}
  void Run() override;

 private:
  LiveTrack24V2Handler& m_handler;
};

class LiveTrack24V2Handler::RadarThread final : public Thread {
 public:
  explicit RadarThread(LiveTrack24V2Handler& handler)
      : Thread("LT24v2Radar"), m_handler(handler) {}
  void Run() override;

 private:
  LiveTrack24V2Handler& m_handler;
};

namespace {

template <typename T>
std::string toString(const T& value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

std::string random_string(size_t length) {
  constexpr std::string_view charset =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned> distrib(0, charset.size() - 1);

  auto randchar = [&]() -> char {
    return charset[distrib(gen)];
  };

  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

bool gzipInflate(const char* compressedBytes, unsigned nBytes,
                 std::string& uncompressedBytes) {
  if (nBytes == 0) {
    return false;
  }

  uncompressedBytes.clear();

  unsigned full_length = nBytes;
  unsigned half_length = nBytes / 2;

  unsigned uncompLength = full_length;

  z_stream strm;
  strm.next_in = (Bytef*)compressedBytes;
  strm.avail_in = nBytes;
  strm.total_out = 0;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;

  bool done = false;

  if (inflateInit2(&strm, (16 + MAX_WBITS)) != Z_OK) {
    return false;
  }

  auto uncomp = std::make_unique<char[]>(uncompLength);
  while (!done) {
    if (strm.total_out >= uncompLength) {
      auto uncomp2 = std::make_unique<char[]>(uncompLength + half_length);
      memcpy(uncomp2.get(), uncomp.get(), uncompLength);
      uncompLength += half_length;
      std::swap(uncomp, uncomp2);
    }

    strm.next_out = (Bytef*)(uncomp.get() + strm.total_out);
    strm.avail_out = uncompLength - strm.total_out;

    int err = inflate(&strm, Z_SYNC_FLUSH);
    if (err == Z_STREAM_END) {
      done = true;
    }
    else if (err != Z_OK) {
      break;
    }
  }

  if (inflateEnd(&strm) != Z_OK) {
    return false;
  }

  if (done) {
    std::copy_n(uncomp.get(), strm.total_out,
                std::back_inserter(uncompressedBytes));
  }
  return done;
}

std::string otpReply(const std::string& question) {
  char mac[SHA256_DIGEST_SIZE];

  unsigned int key_size = strlen(appSecret);
  unsigned int message_len = question.length();

  hmac_sha256((unsigned char*)appSecret, key_size,
              (unsigned char*)question.c_str(), message_len,
              (unsigned char*)mac, (unsigned)SHA256_DIGEST_SIZE);

  static constexpr char Digit[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                   '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  std::string out;
  out.reserve(16);
  auto out_iterator = std::back_inserter(out);
  for (int i = 0; i < 8; i++) {
    out_iterator = Digit[(mac[i] & 0xF0) >> 4];
    out_iterator = Digit[(mac[i] & 0x0F) >> 0];
  }
  return out;
}

std::string downloadJSON(http_session& http, const std::string& url) {
  std::string response = http.request("api.livetrack24.com", 80, url.c_str());
  if (!response.empty()) {
    if (url.find("gzip/1") != std::string::npos) {
      std::string unzip;
      if (gzipInflate(response.data(), response.size(), unzip)) {
        response = std::move(unzip);
      }
    }
  }
  return response;
}

std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
  std::vector<unsigned char> bytes;
  bytes.reserve(hex.size() / 2);
  for (std::string::size_type i = 0, i_end = hex.size(); i < i_end; i += 2) {
    unsigned byte;
    std::istringstream hex_byte(hex.substr(i, 2));
    hex_byte >> std::hex >> byte;
    bytes.push_back(static_cast<unsigned char>(byte));
  }
  return bytes;
}

std::string md5_str(const std::string& text, bool tolower) {
  std::string lowText = text;
  if (tolower) {
    transform(lowText.begin(), lowText.end(), lowText.begin(), ::tolower);
  }
  MD5 md5;
  md5.Update(lowText);
  return md5.Final();
}

std::string passwordToken(const std::string& plainTextPassword,
                          const std::string& sessionID) {
  std::string lowerPassMD5 = md5_str(plainTextPassword, true);
  std::string lowerPassMD5_and_sessionID = lowerPassMD5 + sessionID;
  std::string tokenString = md5_str(lowerPassMD5_and_sessionID, false);
  tokenString += lowerPassMD5 + appSecret;
  std::string tokenStringMd5 = md5_str(tokenString, false);

  auto bytes = hex_to_bytes(tokenStringMd5);

  return base64url_encode(bytes.data(), bytes.size(), false);
}

static const std::string mapGBase64Index =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ()";

std::string intToGBase64(int num) {
  std::string sign;
  std::string str;

  if (num < 0) {
    num = -num;
    sign = "-";
  }
  else {
    sign = "";
  }

  do {
    str = mapGBase64Index[num & 0x3f] + str;
    num = floor(num / 64);
  } while (num);

  return sign + str;
}

int createSID() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> distrib(0, 2147483646);
  return distrib(gen);
}

std::string DeltaRLE(const std::vector<int>& data) {
  int rle = 0;
  std::string times;
  int dif, lastDif = -99999, last = 0;
  std::string res;
  std::string lastRes;

  for (unsigned int i = 0; i < data.size(); i++) {
    int datai = (data[i]);

    if (i == 0) {
      res += intToGBase64(datai);
    }
    else {
      dif = datai - last;
      if ((lastDif == dif) && (rle < 63)) {
        rle++;
        times = intToGBase64(rle);
        res = lastRes + ((dif == 0)
                             ? "*" + times
                             : (dif > 0 ? "$" + times + intToGBase64(dif)
                                        : "_" + times + intToGBase64(-dif)));
      }
      else {
        lastDif = dif;
        rle = 1;
        lastRes = res;
        res += (dif == 0) ? "."
                          : (dif > 0 ? ":" + intToGBase64(dif)
                                     : "!" + intToGBase64(-dif));
      }
    }
    last = datai;
  }

  return res;
}
}  // namespace

// --- Main Handler Implementation ---

LiveTrack24V2Handler::LiveTrack24V2Handler(const tracking::Profile& profile)
    : m_profile(profile) {
  m_deviceID = random_string(10);
  m_v2_sid = createSID();
  m_v2_pwt = passwordToken(m_profile.password, toString(m_v2_sid));

  StartupStore(_T(". LiveTracker API V2 will use server %s."),
               m_profile.server.c_str());

  if (m_profile.interval > 0) {
    m_run_tracker = true;
    m_trackerThread = std::make_unique<TrackerThread>(*this);
    m_trackerThread->Start();
  }
  if (m_profile.radar) {
    LKTime_Real = 60, LKTime_Ghost = 120, LKTime_Zombie = 360;
  }

  if (m_profile.radar && EnableFLARMMap) {
    m_run_radar = true;
    m_radarThread = std::make_unique<RadarThread>(*this);
    m_radarThread->Start();
  }
}

LiveTrack24V2Handler::~LiveTrack24V2Handler() {
  if (m_run_tracker) {
    m_run_tracker = false;
    m_newDataEvent.set();
    if (m_trackerThread->IsDefined()) {
      m_trackerThread->Join();
    }
  }
  if (m_run_radar) {
    m_run_radar = false;
    if (m_radarThread && m_radarThread->IsDefined()) {
      m_radarThread->Join();
    }
  }
  StartupStore(_T(". LiveTracker V2 closed."));
}

void LiveTrack24V2Handler::Update(const NMEA_INFO& Basic,
                                  const DERIVED_INFO& Calculated) {
  if (m_profile.interval <= 0 || Basic.NAVWarning) {
    return;
  }

  // This logic is identical to V1 and can be shared
  if (Basic.Time < m_logtime) {
    return;
  }
  m_logtime = (int)Basic.Time + m_profile.interval;
  if (m_logtime >= 86400) {
    m_logtime -= 86400;
  }

  livetracker_point_t newpoint = {
      .unix_timestamp = to_time_t(Basic),
      .flying = m_profile.always_on || Calculated.Flying,
      .latitude = Basic.Latitude,
      .longitude = Basic.Longitude,
      .alt = Basic.Altitude,
      .ground_speed = Basic.Speed,
      .course_over_ground = Calculated.Heading,
  };

  WithLock(m_mutex, [&]() {
    if (m_points.size() > static_cast<size_t>(1800 / m_profile.interval)) {
      m_points.pop_front();
    }
    m_points.push_back(newpoint);
  });
  m_newDataEvent.set();
}

json LiveTrack24V2Handler::callLiveTrack24(http_session& http,
                                           const std::string& subURL,
                                           bool calledSelf) {

  std::string url = "/api/v2/op/" + subURL;
  url += "/ak/" + std::string(appKey) + "/vc/" + otpReply(m_otpQuestion);
  if (calledSelf) {
    url += "/di/" + m_deviceID + "/ut/" + m_ut;
  }

  std::string reply = downloadJSON(http, url);

  if (reply.empty()) {
    DebugLog(_T(".LiveRadar callLiveTrack24 : Empty response from server"));
    return nullptr;
  }

  json res = json::parse(reply);

  if (!res.is_object()) {
    return nullptr;
  }

  json::iterator it = res.find("qwe");
  if (it != res.end() && it->is_string()) {
    m_otpQuestion = *it;
  }

  it = res.find("ut");
  if (it != res.end() && it->is_string()) {
    m_ut = *it;
  }

  it = res.find("sync");
  if (it != res.end() && it->is_number()) {
    m_sync = static_cast<int>(*it);
  }

  if (!calledSelf) {
    it = res.find("newqwe");
    if (it != res.end() && it->is_number() && it->get<double>() == 1.) {
      res = callLiveTrack24(http, subURL, true);
    }

    it = res.find("reLogin");
    if (it != res.end() && it->is_number() && it->get<double>() == 1.) {
      res = callLiveTrack24(http,
                            "login/username/" + m_profile.user +
                                "/pass/" + m_profile.password,
                            true);

      json::iterator ut = res.find("ut");
      if (ut != res.end() && ut->is_string()) {
        res = callLiveTrack24(http, subURL, true);
      }
    }
  }
  return res;
}

bool LiveTrack24V2Handler::LiveTrack24_Radar(http_session& http) {
  DebugLog(_T(".LiveRadar RADAR"));

  time_t t_of_day = to_time_t(GPS_INFO);

  std::ostringstream strsCommand;
  strsCommand << "liveList";
  strsCommand << "/friends/1";
  strsCommand << "/sync/" << m_sync;
  strsCommand << "/gzip/1";

  try {
    json payload = callLiveTrack24(http, strsCommand.str());

    if (payload.is_null() || !payload.is_object()) {
      DebugLog(_T(".LiveRadar json null or not an object"));
      return false;
    }

    json::iterator userlist_it = payload.find("userlist");
    if (userlist_it == payload.end() || !userlist_it->is_array()) {
      DebugLog(_T(".LiveRadar list not an array"));
      return false;
    }

    auto& userlist = *userlist_it;
    DebugLog(_T(". LiveRadar list.size =%u"), static_cast<unsigned>(userlist.size()));

    for (const auto& elmt : userlist) {
      try {
        double lat = elmt.at("lat");
        double lon = elmt.at("lon");
        double alt = Units::From(Units_t::unMeter, elmt.at("alt"));
        double sog = Units::From(Units_t::unKiloMeterPerHour, elmt.at("sog"));
        auto lastTM = static_cast<int>(elmt.at("lastTM"));
        auto userID = static_cast<uint32_t>(elmt.at("userID"));
        auto category = static_cast<int>(elmt.at("category"));
        auto isLiveDB = static_cast<int>(elmt.at("isLiveDB"));
        std::string username = elmt.at("username");
        transform(username.begin(), username.end(), username.begin(), ::toupper);

        double Distance, Bearing;
        DistanceBearing(lat, lon, GPS_INFO.Latitude, GPS_INFO.Longitude,
                        &Distance, &Bearing);

        if (Distance > 30000) {
          continue;
        }

        double delay = t_of_day - lastTM;
        std::string profile_user = m_profile.user;
        std::string profile_pilot = to_utf8(PilotName_Config);
        transform(profile_user.begin(), profile_user.end(), profile_user.begin(),
                  ::toupper);
        transform(profile_pilot.begin(), profile_pilot.end(),
                  profile_pilot.begin(), ::toupper);

        if (delay > 300 || isLiveDB == 0 ||
            (category != 1 && category != 2 && category != 4 && category != 8) ||
            username.compare(profile_user) == 0 ||
            username.compare(profile_pilot) == 0) {
          continue;
        }

        if (!m_flarmwasinit) {
          DoStatusMessage(
              MsgToken<279>(),
              TEXT("LiveTrack24"));  // FLARM DETECTED from LiveTrack24
          m_flarmwasinit = true;
        }

        FLARM_Inject(GPS_INFO, userID, std::move(username), lat, lon, alt, sog, 0,
                    lastTM, isLiveDB);
      }
      catch (std::exception& e) {
        TestLog(_T(".LiveRadar exception processing user: %s"), e.what());
      }
    }
  }
  catch (std::exception& e) {
    TestLog(_T(".LiveRadar exception: %s"), e.what());
    return false;
  }
  return true;
}
  

int LiveTrack24V2Handler::GetUserIDFromServer2(http_session& http) {
  int retval = -1;
  char txbuf[512];

  std::string pwt0 = passwordToken(m_profile.password, "0");
  lk::snprintf(txbuf, "/api/t/lt/getUserID/%s/1.0/LK8000/0/0/%s/%s", appKey,
          pwt0.c_str(), m_profile.user.c_str());

  std::string response = http.request("t2.livetrack24.com", 80, txbuf);
  if (!response.empty()) {
    std::vector<std::string> strings;
    std::istringstream f(response);
    std::string s;
    while (getline(f, s, '\n')) {
      strings.push_back(s);
    }

    if (strings.size() < 2 || strings[0] != "0;OK") {
      return -1;
    }

    retval = -1;
    sscanf(strings[1].c_str(), "%d", &retval);
  }

  return retval;
}

bool LiveTrack24V2Handler::SendEndOfTrackPacket2(http_session& http,
                                                 unsigned int* packet_id) {
  std::ostringstream stringStream;
  stringStream << "/api/t/lt/trackEnd/" << appKey;
  stringStream << "/1.0/LK8000" << "/" << m_v2_sid;
  stringStream << "/" << m_v2_userid << "/" << m_v2_pwt;
  stringStream << "/0/99/" << (*packet_id) + 1 << "/";
  stringStream << "0/0";

  std::string command = stringStream.str();
  std::string response =
      http.request("t2.livetrack24.com", 80, command.c_str());
  return (response == "0;OK");
}

bool LiveTrack24V2Handler::SendGPSPointPacket2(http_session& http,
                                               unsigned int* packet_id) {
  time_t _last_unix_timestamp = 0;
  std::vector<int> TimeList, LatList, LonList, AltList, SOGlist, COGlist;

  {
    ScopeLock guard(m_mutex);

    if (m_points.empty()) {
      return false;
    }
    _last_unix_timestamp = m_points.back().unix_timestamp;

    for (const auto& point : m_points) {
      TimeList.emplace_back(point.unix_timestamp);
      LatList.emplace_back(std::floor(point.latitude * 60000.));
      LonList.emplace_back(std::floor(point.longitude * 60000.));
      AltList.emplace_back(Units::To(Units_t::unMeter, point.alt));
      SOGlist.emplace_back(
          Units::To(Units_t::unKiloMeterPerHour, point.ground_speed));
      COGlist.emplace_back(point.course_over_ground);
    }
  }

  std::ostringstream stringStream;
  stringStream << "/api/d/lt/track/";
  stringStream << appKey << "/";
  stringStream << LKVERSION << "." << LKRELEASE << "/" << m_deviceID << "/";
  stringStream << m_v2_sid << "/";
  stringStream << m_v2_userid << "/";
  stringStream << m_v2_pwt << "/";
  stringStream << "9";
  stringStream << "/0/";
  stringStream << (*packet_id) + 1 << "/";
  stringStream << DeltaRLE(TimeList) << "/";
  stringStream << DeltaRLE(LatList) << "/";
  stringStream << DeltaRLE(LonList) << "/";
  stringStream << DeltaRLE(AltList) << "/";
  stringStream << DeltaRLE(SOGlist) << "/";
  stringStream << DeltaRLE(COGlist) << "/";
  stringStream << "LK8000";

  const std::string command = stringStream.str();
  std::string response =
      http.request("t2.livetrack24.com", 80, command.c_str());
  if (response != "0;OK") {
    return false;
  }
  WithLock(m_mutex, [&]() {
    while (!m_points.empty() &&
           m_points.front().unix_timestamp <= _last_unix_timestamp) {
      m_points.pop_front();
    }
  });
  (*packet_id)++;
  DebugLog(_T(".Livetrack24 TRACKER sent %u points"),
           static_cast<unsigned>(TimeList.size()));
  return true;
}

void LiveTrack24V2Handler::TrackerThread::Run() {
  int tracker_fsm = 0;
  livetracker_point_t sendpoint = {};
  bool sendpoint_processed = false;
  bool packet_processed = false;
  bool sendpoint_valid = false;
  unsigned int packet_id = 0;

  http_session http;

  do {
    if (m_handler.m_newDataEvent.tryWait(5000)) {
      m_handler.m_newDataEvent.reset();
    }
    if (!m_handler.m_run_tracker) {
      break;
    }

    sendpoint_processed = false;

    sendpoint_valid = WithLock(m_handler.m_mutex, [&]() {
      if (!m_handler.m_points.empty()) {
        sendpoint = m_handler.m_points.front();
        return true;
      }
      return false;
    });

    if (sendpoint_valid) {
      DebugLog(
          _T(". Livetracker TRACKER sendpoint.flying: %d - tracker_fsm: %d"),
          sendpoint.flying, tracker_fsm);

      switch (tracker_fsm) {
        default:
        case 0:  // Wait for flying
          if (!sendpoint.flying) {
            ScopeLock guard(m_handler.m_mutex);
            m_handler.m_points.pop_front();
            sendpoint_processed = true;
            break;
          }
          tracker_fsm++;
          break;
        case 1:  // Get User ID
          m_handler.m_v2_userid = m_handler.GetUserIDFromServer2(http);
          sendpoint_processed = false;
          if (m_handler.m_v2_userid >= 0) {
            tracker_fsm++;
          }
          break;
        case 2:  // Start of track packet (implicit in V2)
          sendpoint_processed = false;
          tracker_fsm++;
          break;
        case 3:  // Gps point packet
          packet_processed = m_handler.SendGPSPointPacket2(http, &packet_id);
          if (!sendpoint.flying) {
            tracker_fsm++;
          }
          break;
        case 4:  // End of track packet
          sendpoint_processed = m_handler.SendEndOfTrackPacket2(http, &packet_id);
          StartupStore(
              _T(". Livetracker TRACKER SendEndOfTrackPacket2 .%d ..."),
              sendpoint_processed);

          if (sendpoint_processed) {
            tracker_fsm = 0;
          }
          break;
      }
    }
    if (packet_processed) {
      Sleep(m_handler.m_profile.interval * 1000);
    }
  } while (m_handler.m_run_tracker);
}

void LiveTrack24V2Handler::RadarThread::Run() {
  http_session http;
  Sleep(5000);
  do {
    if (!m_handler.m_run_radar) {
      break;
    }

    if (!m_handler.LiveTrack24_Radar(http)) {
      Sleep(5000);
    }
    else {
      Sleep(5000);
    }
  } while (m_handler.m_run_radar);
}
