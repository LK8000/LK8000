#ifndef _TRACKING_TRACKING_H
#define _TRACKING_TRACKING_H

#include <cstdint>
#include <cstdio>
#include "tchar.h"

namespace settings {
    class writer;
}

struct NMEA_INFO;
struct DERIVED_INFO;

namespace tracking {

    enum class platform : uint8_t {
        none,
        livetrack24,
        skylines_aero
    };

    extern int  interval; // sending position interval (sec)
    extern bool radar_config;  // feed FLARM with Livetrack24 livedata only in PG/HG mode
    extern int  start_config;  // Livetracking only in flight or always

    extern TCHAR    server_config[100]; // server name or ip address
    extern uint16_t port_config; // tcp port
    extern TCHAR    usr_config[100]; // user name ( token for skylines )
    extern TCHAR    pwd_config[100]; // user pwd

    void ResetSettings();
    bool LoadSettings(const char *key, const char *value);
    void SaveSettings(settings::writer& writer_settings);

    platform GetPlatform();
    
    void Initialize(platform id);
    void Update(const NMEA_INFO &Basic, const DERIVED_INFO &Calculated);
    void DeInitialize();
} // namespace tracking

#endif //_TRACKING_TRACKING_H
