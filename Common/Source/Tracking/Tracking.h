/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Tracking.h
 * Author: Bruno de Lacheisserie
 */

#ifndef TRACKING_TRACKING_H
#define TRACKING_TRACKING_H

#include <vector>
#include <cstdint>
#include <cstdio>
#include <string>
#include "tchar.h"
#include "Util/tstring.hpp"
#include "Screen/LKBitmap.h"

namespace settings {
class writer;
}  // namespace settings

struct NMEA_INFO;
struct DERIVED_INFO;

namespace tracking {

// clang-format off
enum class platform : uint8_t {
  none,
  livetrack24,
  skylines_aero,
  ffvl
};
// clang-format on

const TCHAR* PlatformLabel(platform p);
LKBitmap load_bitmap(platform p);

struct Profile {
  platform protocol = platform::none;
  int interval = 0;        // sending position interval (sec)
  bool radar = false;      // feed FLARM with livedata
  bool always_on = false;  // Livetracking only in flight or always
  std::string server;      // server name or ip address
  uint16_t port = 0;       // tcp port
  std::string user;        // user name ( token for skylines/VLSafe )
  std::string password;    // user pwd
};

extern std::vector<Profile> profiles;

void ResetSettings();
bool LoadSettings(const char* key, const char* value);
void SaveSettings(settings::writer& writer_settings);

void Initialize();
void Update(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated);
void DeInitialize();

}  // namespace tracking

#endif  // TRACKING_TRACKING_H
