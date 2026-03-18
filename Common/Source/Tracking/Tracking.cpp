/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Tracking.cpp
 * Author: Bruno de Lacheisserie
 */
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <memory>
#include "Tracking.h"
#include "ITrackingHandler.h"
#include "SkylinesGlue.h"
#include "NMEA/Info.h"
#include "LiveTrack24V1Handler.h"
#include "LiveTrack24V2Handler.h"
#include "Defines.h"
#include "Settings/read.h"
#include "Settings/write.h"
#include "TrackingSettings.h"
#include "FFVLTracking.h"
#include "utils/stringext.h"
#include "utils/strcpy.h"
#include "MessageLog.h"
#include "Bitmaps.h"
#include "Logger.h"

extern int LKTime_Real;
extern int LKTime_Ghost;
extern int LKTime_Zombie;
extern BYTE RUN_MODE;

namespace tracking {  

std::vector<Profile> profiles;

namespace {
// private global;
std::vector<std::unique_ptr<ITrackingHandler>> active_handlers;

namespace migration {
// --- Variables for migrating old settings ---
bool new_settings_found = false;
bool old_settings_found = false;
int interval = 0;
std::string server_config;
uint16_t port_config = 0;
std::string usr_config;
std::string pwd_config;
std::string ffvl_user_key;
bool radar_config = false;
bool always_config = false;
}  // namespace migration

void DoMigration() {
  if (migration::new_settings_found || !migration::old_settings_found) {
    // New settings are present, or no settings were found. No migration needed.
    return;
  }

  // --- Starting migration ---
  StartupStore(_T(". Migrating old tracking settings to new format."));

  // Clear existing profiles to start from a clean slate
  profiles.clear();

  // --- Migration of LiveTrack24/Skylines to profile 0 ---
  if (migration::interval > 0) {
    // Determine the protocol (LiveTrack24 vs Skylines)
    std::string snu = migration::server_config;
    std::transform(snu.begin(), snu.end(), snu.begin(), ::tolower);
    auto protocol = (snu.compare("skylines.aero") == 0)
                      ? platform::skylines_aero
                      : platform::livetrack24;

    profiles.push_back({
      .protocol = protocol,
      .interval = migration::interval,
      .radar = migration::radar_config,
      .always_on = migration::always_config,
      .server = migration::server_config,
      .port = migration::port_config,
      .user = migration::usr_config,
      .password = migration::pwd_config
    });

  }

  if (http_session::ssl_available()) {
    // --- Migration of FFVL to profile 1 ---
    if (!migration::ffvl_user_key.empty()) {
      profiles.push_back({
        .protocol = platform::ffvl,
        .interval = 60,
        .radar = false,
        .always_on = true,
        .server = {},
        .port = 0,
        .user = migration::ffvl_user_key,
        .password = {}
      });
    }
  }
}

}  // anonymous namespace

void Initialize() {
  active_handlers.clear();

  DoMigration();

  for (const auto& profile : profiles) {
    switch (profile.protocol) {
      case platform::livetrack24:
        if (profile.interval > 0) {
          std::string server_upper = profile.server;
          std::transform(server_upper.begin(), server_upper.end(),
                         server_upper.begin(), ::toupper);
          if (server_upper.compare("WWW.LIVETRACK24.COM") == 0) {
            active_handlers.emplace_back(
                std::make_unique<LiveTrack24V2Handler>(profile));
          }
          else {
            active_handlers.emplace_back(
                std::make_unique<LiveTrack24V1Handler>(profile));
          }
        }
        break;
      case platform::skylines_aero:
        if (profile.interval > 0) {
          auto skylines_glue = std::make_unique<SkylinesGlue>(profile);

          if (profile.radar) {
            LKTime_Real = 90;
            LKTime_Ghost = 180;
            LKTime_Zombie = 360;
          }
          active_handlers.emplace_back(std::move(skylines_glue));
        }
        break;
      case platform::ffvl:
        if (http_session::ssl_available() && !profile.user.empty()) {
          auto ffvl_handler = std::make_unique<FFVLTracking>(profile.user);
          ffvl_handler->Start();
          active_handlers.emplace_back(std::move(ffvl_handler));
        }
        break;
      case platform::none:
      default:
        break;
    }
  }
}

void Update(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
#ifdef NDEBUG
  if (RUN_MODE != RUN_FLY && !ReplayLogger::IsEnabled()) {
    return;  // skip tracking in simulation mode or replay mode
  }
#endif

  for (auto& handler : active_handlers) {
    handler->Update(Basic, Calculated);
  }
}

void DeInitialize() {
  active_handlers.clear();
}

void ResetSettings() {
  active_handlers.clear();
  profiles.clear();
  migration::new_settings_found = false;
  migration::old_settings_found = false;
  migration::interval = 0;
  migration::server_config.clear();
  migration::port_config = 0;
  migration::usr_config.clear();
  migration::pwd_config.clear();
  migration::ffvl_user_key.clear();
  migration::radar_config = false;
  migration::always_config = false;
}

bool LoadSettings(const char* key, const char* value) {
  if (settings_io::LoadProfileSettings(key, value, profiles)) {
    migration::new_settings_found = true;
    return true;
  }

  // Attempt to load old settings for migration
  if (!migration::new_settings_found) {
    if (settings::read(key, value, "LiveTrackerInterval",
                       migration::interval)) {
      migration::old_settings_found = true;
      return true;
    }
    if (settings::read(key, value, "LiveTrackerRadar_config",
                       migration::radar_config)) {
      migration::old_settings_found = true;
      return true;
    }
    if (settings::read(key, value, "LiveTrackerStart_config",
                       migration::always_config)) {
      migration::old_settings_found = true;
      return true;
    }
    if (settings::read(key, value, "LiveTrackersrv",
                       migration::server_config)) {
      migration::old_settings_found = true;
      return true;
    }
    if (settings::read(key, value, "LiveTrackerport", migration::port_config)) {
      migration::old_settings_found = true;
      return true;
    }
    if (settings::read(key, value, "LiveTrackerusr", migration::usr_config)) {
      migration::old_settings_found = true;
      return true;
    }
    if (settings::read(key, value, "LiveTrackerpwd", migration::pwd_config)) {
      migration::old_settings_found = true;
      return true;
    }
    if (settings::read(key, value, "ffvl_user_key", migration::ffvl_user_key)) {
      migration::old_settings_found = true;
      return true;
    }
  }

  return false;
}

void SaveSettings(settings::writer& writer_settings) {
  settings_io::SaveProfileSettings(writer_settings, profiles);
}

const TCHAR* PlatformLabel(platform platform) {
  switch (platform) {
    case tracking::platform::none:
      return _T("none");
    case tracking::platform::livetrack24:
      return _T("livetrack24");
    case tracking::platform::skylines_aero:
      return _T("skylines.aero");
#ifdef USE_CURL
    case tracking::platform::ffvl:
      return _T("VLSafe");
#endif
    default:
      break;
  }
  return _T("");
}

LKBitmap load_bitmap(platform platform) {
  switch (platform) {
    case tracking::platform::livetrack24:    
      return LKLoadBitmap(_T("LT24"), false);
    case tracking::platform::skylines_aero:
      return LKLoadBitmap(_T("SKYLINES"), false);
    case tracking::platform::ffvl:
#ifdef USE_CURL
      return LKLoadBitmap(_T("FFVL"), false);
#endif
    case tracking::platform::none:
      break;
  }
  return {};
}

}  // namespace tracking
