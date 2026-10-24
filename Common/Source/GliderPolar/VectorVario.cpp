/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   VectorVario.cpp
 * Author: Bruno de Lacheisserie
 */
#include "GliderPolar/VectorVario.h"
#include <vector>
#include <cmath>
#include <cassert>
#include <string_view>
#include "utils/lookup_table.h"

namespace GliderPolar {
namespace {

/******************************************************************************************
 * Polar generation constants and functions based formulas used by VectorVario
 * original code released under the GPLv3 license here :
 * https://github.com/TheGoodWeather/Vector-Vario-Analyzer/blob/main/src/polar_generator.py
 */
constexpr double A0P = 0.0264191;
constexpr double A1P = 0.0253048;
constexpr double A2P = 0.2781714;
constexpr double KAR = 0.0030313;

constexpr double OPEN = 0.0389250;
constexpr double POD = 0.0296384;
constexpr double SUB = 0.0247878;

double GetHarnessOffset(const std::string_view& h) {
  constexpr auto harness_table = lookup_table<std::string_view, double>(
      {{"OPEN", OPEN}, {"POD", POD}, {"SUB", SUB}});
  return harness_table.get(h, POD);
}

using Point = Point2D<double>;

Point GeneratePoint(double sproj, double ar, double auw, double harness_offset,
                         double cl) {
  const double Cd = harness_offset + A0P - KAR * ar + A1P * cl + A2P * cl * cl / ar;

  const double vx = std::sqrt((2.0 * auw * 9.81) / (1.225 * cl * sproj));
  const double vz = -vx * Cd / cl;

  return {vx, vz};
}

std::vector<Point> Generate(double sproj, double ar, double auw,
                                 const std::string_view& harness) {
  std::vector<Point> samples = {};
  if (sproj > 0. && ar > 0. && auw > 0.) {
    const double harness_offset = GetHarnessOffset(harness);
    const double Cl0 = 0.0899 * ar + 0.3391;
    assert(Cl0 > 0.30); // division by zero or negative lift coefficient..

    samples.reserve(static_cast<size_t>(std::floor((Cl0 - 0.30) / 0.03)) + 1);
    for (double cl = Cl0; cl >= 0.30; cl -= 0.03) {
      samples.push_back(GeneratePoint(sproj, ar, auw, harness_offset, cl));
    }
  }
  return samples;
}

/***********************************************************************************/

}  // namespace

VectorVario::VectorVario(double sproj, double ar, double auw,
                         const std::string_view& harness)
    : Discret(Generate(sproj, ar, auw, harness)) {}

}  // namespace GliderPolar
