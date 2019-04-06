#ifndef _TRACKING_TRACKING_H
#define _TRACKING_TRACKING_H

struct NMEA_INFO;
struct DERIVED_INFO;

namespace tracking {

    enum platform {
        none,
        livetrack24,
        skylines_aero,
        ogn
    };

    void Initialize(platform id);
    void Update(const NMEA_INFO &Basic, const DERIVED_INFO &Calculated);
    void DeInitialize();
} // namespace tracking

#endif //_TRACKING_TRACKING_H
