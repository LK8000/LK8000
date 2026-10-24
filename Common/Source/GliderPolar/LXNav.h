/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LXNav.h
 * Author: Bruno de Lacheisserie
 */
#pragma once
#include "GliderPolar/Quadratic.h"

namespace GliderPolar {

/**
 * Create a Quadratic instance from LXNAV coefficients.
 *
 * LXNAV format:
 *   sink = a * (Vkmh / 100)^2
 *        + b * (Vkmh / 100)
 *        + c
 *
 * where:
 *   Vkmh is the speed in km/h
 *   sink is the sink rate in m/s
 *
 * Internal format:
 *   sink = a * Vms^2 + b * Vms + c
 *
 * where Vms is the speed in m/s.

 * @param a, b, c Coefficient from LXNAV polar.
 * @return Quadratic instance.
 */
inline Quadratic From_LXNAV(double a, double b, double c) noexcept {
  constexpr double K = 3.6 / 100.0;
  return {
    -a * K * K,
    -b * K,
    -c
  };
}

}  // namespace GliderPolar

