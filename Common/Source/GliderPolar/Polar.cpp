/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Polar.h
 * Author: Bruno de Lacheisserie
 */
#include "Polar.h"
#include "Quadratic.h"
#include "WinPilot.h"
#include "Defines.h"

extern double POLAR[3];
extern double WEIGHTS[3];

bool GliderPolar::Update(const Quadratic& polar, double DryWeight,
                         double BallastWeight) {
  if (!polar.IsValid()) {
    // Invalid polar coefficients
    return false;
  }

  if (!(DryWeight > WEIGHTS[WEIGHT_PILOT]) || !(BallastWeight >= 0.)) {
    // Invalid weight values
    return false;
  }

  std::ranges::copy(polar.GetCoefficients(), POLAR);

  WEIGHTS[WEIGHT_PLANEDRY] = DryWeight - WEIGHTS[WEIGHT_PILOT];
  WEIGHTS[WEIGHT_WATER] = BallastWeight;

  // now scale off weight
  if ((WEIGHTS[WEIGHT_PILOT] + WEIGHTS[WEIGHT_PLANEDRY]) >= 0) {
    double scale = std::sqrt(WEIGHTS[WEIGHT_PILOT] + WEIGHTS[WEIGHT_PLANEDRY]);
    POLAR[0] = POLAR[0] * scale;
    POLAR[2] = POLAR[2] / scale;
  }

  return true;
}

bool GliderPolar::Update(const Quadratic& polar) {
  return GliderPolar::Update(polar,
                             WEIGHTS[WEIGHT_PILOT] + WEIGHTS[WEIGHT_PLANEDRY],
                             WEIGHTS[WEIGHT_WATER]);
}

bool GliderPolar::Update(double sproj, double ar, double auw,
                         const std::string_view& harness) {
  try {
    WinPilot polar(sproj, ar, auw, harness);
    Quadratic quadratic(polar.GetPoints());
    return GliderPolar::Update(quadratic, polar.GetDryWeight(), polar.GetBallastWeight());
  }
  catch (...) {
  }
  return false;
}
