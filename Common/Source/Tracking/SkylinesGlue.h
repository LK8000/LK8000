/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   SkylinesGlue.h
 * Author: Bruno de Lacheisserie
 */

#ifndef _TRACKING_SKYLINESGLUE_H_
#define _TRACKING_SKYLINESGLUE_H_

#include "ITrackingHandler.h"
#include "Tracking/TrackingGlue.hpp"

namespace tracking {
struct Profile;
}  // namespace tracking

class SkylinesGlue final : public TrackingGlue, public ITrackingHandler {

#ifdef HAVE_SKYLINES_TRACKING_HANDLER
protected:
  void OnTraffic(uint32_t pilot_id, unsigned time_of_day_ms,
                 const GeoPoint &location, int altitude) override;

  void OnUserName(uint32_t user_id, const TCHAR *name) override;

  void OnWave(unsigned time_of_day_ms,
              const GeoPoint &a, const GeoPoint &b) override {}
#endif
public:
  explicit SkylinesGlue(const tracking::Profile& profile);
  void Update(const NMEA_INFO &Basic, const DERIVED_INFO &Calculated) override;

private:
  const bool m_always_on;
};

#endif //  _TRACKING_SKYLINESGLUE_H_
