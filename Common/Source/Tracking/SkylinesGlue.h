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
