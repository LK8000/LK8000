#ifndef _TRACKING_SKYLINESGLUE_H_
#define _TRACKING_SKYLINESGLUE_H_

#include "Tracking/TrackingGlue.hpp"

class SkylinesGlue final : public TrackingGlue {

#ifdef HAVE_SKYLINES_TRACKING_HANDLER
protected:
  void OnTraffic(uint32_t pilot_id, unsigned time_of_day_ms,
                 const GeoPoint &location, int altitude) override;

  void OnUserName(uint32_t user_id, const TCHAR *name) override;

  void OnWave(unsigned time_of_day_ms,
              const GeoPoint &a, const GeoPoint &b) override {}
#endif

};

#endif //  _TRACKING_SKYLINESGLUE_H_
