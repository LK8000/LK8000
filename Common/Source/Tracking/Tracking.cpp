#ifdef WIN32
#include <winsock2.h> // avoid include orders warnning
#include <windows.h>
#endif
#include "Tracking.h"
#include "NMEA/Info.h"
#include "NMEA/Derived.h"
#include "LiveTrack24.h"
#include "Tracking/TrackingGlue.hpp"
#include <memory>
#include <assert.h>

namespace tracking {
    namespace {
        // private global;
        std::unique_ptr<TrackingGlue> skylines_glue;
        platform tracking_platform = platform::skylines_aero;
        TrackingSettings tracking_settings;
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
                skylines_glue = std::make_unique<TrackingGlue>();
                tracking_settings.SetDefaults();
/*
                tracking_settings.skylines.interval = LiveTrackerInterval;
                tracking_settings.skylines.enabled =(LiveTrackerInterval > 0);
                tracking_settings.skylines.traffic_enabled = LiveTrackerRadar_config;
                tracking_settings.skylines.key = private_key;
*/
                skylines_glue->SetSettings(tracking_settings);

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
        switch (tracking_platform) {
            case platform::none:
                break; // tracking disabled
            case platform::livetrack24:
                LiveTrackerShutdown();
                break;
            case platform::skylines_aero:
                skylines_glue = nullptr;
                break;
            default:
                assert(false);
                break;
        }
    }

} // namespace tracking
