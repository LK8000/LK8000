/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "McReady.h"
#include "Math/SelfTimingKalmanFilter1d.hpp"

extern double CRUISE_EFFICIENCY;

namespace {

double GetNettoVario(const NMEA_INFO* Basic, const DERIVED_INFO* Calculated) {
  if (Basic->NettoVario.available()) {
    return Basic->NettoVario.value();
  }
  return Calculated->NettoVario;
}

double GetHeadWind(const NMEA_INFO* Basic, const DERIVED_INFO* Calculated) {
  if (Calculated->HeadWind != -999) {
    return Calculated->HeadWind;
  }
  return 0.;
}

class stf_filter final {
 public:
  stf_filter() = default;

  double update(double v_raw, double var_measure = 0.64) {
    if (active_mode != AircraftCategory) {
      set_mode(AircraftCategory);
    }

    auto t_us = MonotonicClockUS();
    // ---------- anti-overshoot ----------
    double delta = v_raw - filter.GetXAbs();
    double dt = t_us - filter.GetLastUpdateUS();
    double limit = max_slope * dt;
    if (std::abs(delta) > limit) {
      v_raw = filter.GetXAbs() + std::copysign(limit, delta);
    }

    // ---------- Kalman update ----------
    filter.Update(t_us, v_raw, var_measure);

    return filter.GetXAbs();
  }

 private:
  void set_mode(AircraftCategory_t mode) {
    if (mode == AircraftCategory_t::umParaglider) {
      var_meas = 3.0;   // process noise
      max_slope = 4.0;  // m/s²
    }
    else {
      var_meas = 0.4;   // process noise
      max_slope = 1.0;  // m/s²
    }
    active_mode = mode;
    filter.SetAccelerationVariance(var_meas);
    filter.Reset();
  }

  AircraftCategory_t active_mode = AircraftCategory_t::umUnknown;
  double var_meas;
  double max_slope;
  SelfTimingKalmanFilter1d filter = {1};
};

}  // namespace

//
// Sollfahrt / Dolphin Speed calculator
//
void SpeedToFly(const NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  double Netto = GetNettoVario(Basic, Calculated);
  double HeadWind = GetHeadWind(Basic, Calculated);

  // calculate maximum efficiency speed to fly
  double Vme_raw = GlidePolar::STF(0, Netto, HeadWind);

  static stf_filter vme_filter;
  Calculated->Vme = vme_filter.update(Vme_raw);

  double VOptHeadWind = 0;
  if (Calculated->FinalGlide && ValidTaskPoint(ActiveTaskPoint)) {
    // according to MC theory STF take account of wind only if on final Glide
    // TODO : for the future add config parameter for always use wind.
    VOptHeadWind = HeadWind;
  }

  // this is IAS for best Ground Glide ratio accounting current air mass
  // ( wind / Netto vario )
  double VOptnew = GlidePolar::STF(MACCREADY, Netto, VOptHeadWind);
  // apply cruises efficiency factor.
  VOptnew *= CRUISE_EFFICIENCY;

  if (Netto > MACCREADY) {
    // this air mass is better than maccready, so don't fly at speed less than
    // minimum sink speed adjusted for load factor
    VOptnew =
        std::max(VOptnew, GlidePolar::Vminsink() * sqrt(Calculated->Gload));
  }
  // never fly at speed less than min sink speed
  VOptnew = std::max(VOptnew, GlidePolar::Vminsink());

  // use low pass filter to avoid big jump of value.
  static stf_filter vopt_filter;
  Calculated->VOpt = vopt_filter.update(VOptnew);
}
