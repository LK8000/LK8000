#include <cstdint>
#include <cassert>
#include <memory>
#include "Tracking.h"
#include "SkylinesGlue.h"
#include "NMEA/Info.h"
#include "NMEA/Derived.h"
#include "LiveTrack24.h"
#include "Defines.h"
#include "Settings/read.h"
#include "Settings/write.h"

extern int LKTime_Real;
extern int LKTime_Ghost;
extern int LKTime_Zombie;

namespace tracking {

    int  interval; // sending position interval (sec)
    bool radar_config;  // feed FLARM with Livetrack24 livedata only in PG/HG mode
    int  start_config;  // Livetracking only in flight or always

    TCHAR    server_config[100]; // server name or ip address
    uint16_t port_config; // tcp port
    TCHAR    usr_config[100]; // user name ( token for skylines )
    TCHAR    pwd_config[100]; // user pwd

    namespace {
        // private global;
        std::unique_ptr<SkylinesGlue> skylines_glue;
        platform tracking_platform = platform::none;

        // key used to save/load setting from pref files
        constexpr char registry_interval[] = "LiveTrackerInterval";
        constexpr char registry_radar_config[] = "LiveTrackerRadar_config";
        constexpr char registry_start_config[] = "LiveTrackerStart_config";
        constexpr char registry_srv[] =  "LiveTrackersrv";
        constexpr char registry_port[] =  "LiveTrackerport";
        constexpr char registry_usr[] =  "LiveTrackerusr";
        constexpr char registry_pwd[] =  "LiveTrackerpwd";

        uint64_t hex_to_uint64(const tstring& string) {
            typedef std::is_same<uint64_t, unsigned long> is_long;
            typedef std::is_same<uint64_t, unsigned long long> is_long_long;
            static_assert(is_long::value || is_long_long::value, "invalid type");

            try {
                if (is_long::value) {
                    return std::stoul(string, 0, 16);
                }
                if (is_long_long::value) {
                    return std::stoull(string, 0, 16);
                }
            } catch(std::exception& e) {
                return 0U;
            }
        }

    }

    void Initialize(platform id) {
        tracking_platform = id;
        switch (tracking_platform) {
            case platform::none:
                break; // tracking disabled
            case platform::livetrack24:
                LiveTrackerInit();
                break;
            case platform::skylines_aero:
                skylines_glue = std::make_unique<SkylinesGlue>();

                TrackingSettings tracking_settings;
                tracking_settings.SetDefaults();

                tracking_settings.skylines.interval = interval;
                tracking_settings.skylines.enabled = (interval > 0);
                tracking_settings.skylines.key = hex_to_uint64(usr_config);

                tracking_settings.skylines.traffic_enabled = radar_config;
                tracking_settings.skylines.near_traffic_enabled = radar_config;

                skylines_glue->SetSettings(tracking_settings);
                if (radar_config) {
                    LKTime_Real = 90;
                    LKTime_Ghost = 180;
                    LKTime_Zombie = 360;
                }


                break;
            default:
                assert(false);
                break;
        }
    }

    void Update(const NMEA_INFO &Basic, const DERIVED_INFO &Calculated) {
        switch (tracking_platform) {
            case platform::none:
                break; // tracking disabled
            case platform::livetrack24:
                LiveTrackerUpdate(Basic, Calculated);
                break;
            case platform::skylines_aero:
                if (skylines_glue) {
                    skylines_glue->OnTimer(Basic, Calculated);
                }
                break;
            default:
                assert(false);
                break;
        }
    }

    void DeInitialize() {
        if (tracking_platform == platform::livetrack24) {
            LiveTrackerShutdown();
        }
        skylines_glue = nullptr;
    }

    void ResetSettings() {
        interval = 0;
        radar_config = false;
        start_config = 0;

        _tcscpy(server_config,_T("www.livetrack24.com"));
        port_config = 80;

        _tcscpy(usr_config,_T("LK8000"));
        _tcscpy(pwd_config,_T(""));
    }

    bool LoadSettings(const char *key, const char *value) {
        return settings::read(key, value, registry_interval, interval)
            || settings::read(key, value, registry_radar_config, radar_config)
            || settings::read(key, value, registry_start_config, start_config)
            || settings::read(key, value, registry_srv, server_config)
            || settings::read(key, value, registry_port, port_config)
            || settings::read(key, value, registry_usr, usr_config)
            || settings::read(key, value, registry_pwd, pwd_config);
    }

    void SaveSettings(settings::writer& writer_settings) {
        writer_settings(registry_interval, interval);
        writer_settings(registry_radar_config, radar_config);
        writer_settings(registry_start_config, start_config);
        writer_settings(registry_srv, server_config);
        writer_settings(registry_port, port_config);
        writer_settings(registry_usr, usr_config);
        writer_settings(registry_pwd, pwd_config);
    }

    platform GetPlatform() {
        if(interval == 0) {
            return platform::none;
        }
        tstring snu = server_config;
        std::transform(snu.begin(), snu.end(), snu.begin(), _totlower);
        if(snu.compare(_T("skylines.aero")) == 0) {
            return platform::skylines_aero;
        }
        return platform::livetrack24;
    }

} // namespace tracking
