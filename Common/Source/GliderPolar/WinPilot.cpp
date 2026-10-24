/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WinPilot.cpp
 * Author: Bruno de Lacheisserie
 */
#include "WinPilot.h"
#include <limits>
#include "VectorVario.h"
#include "Quadratic.h"

namespace GliderPolar {
namespace {

// Calculate the squared error between the discret polar STF and the STF
// predicted by the quadratic polar.
double STFError(const Quadratic& quadratic, const Discret& discret) {
  struct MCWeight {
    double mc;
    double weight;
  };

  // Test cases for different MC values and their corresponding weights,
  // weight is used to prioritize most used MC values in the error calculation.
  constexpr MCWeight tests[] = {
    {0.0, 5.0},
    {0.5, 5.0},
    {1.0, 4.0},
    {2.0, 3.0},
    {3.0, 2.0},
    {4.0, 1.0}
  };

  double err = 0.0;
  for (auto t : tests) {
    const double v_a = discret.STF(t.mc);
    const double v_b = quadratic.STF(t.mc);

    const double dv = v_a - v_b;

    err += t.weight * dv * dv;
  }
  return err;
}

// find the best STF polar for the given discretized polar points to minimize
// the STF error.
WinPilot::Points Generate(const Discret& discret) {
  double best_error = std::numeric_limits<double>::max();
  if(discret.size() < 3) {
    return {};
  }
  const auto& first = discret.front();
  Discret::Point best_middle = {};
  const auto& last = discret.back();

  const auto middle_range = std::ranges::subrange(std::next(discret.begin()),
                                                  std::prev(discret.end()));

  for (const auto& mid : middle_range) {
    const GliderPolar::Quadratic polar(first, mid, last);
    if (!polar.IsValid()) {
      continue;
    }

    const double error = STFError(polar, discret);
    if (error < best_error) {
      best_middle = mid;
      best_error = error;
    }
  }

  return {first, best_middle, last};
}

}  // namespace

WinPilot::WinPilot(double sproj, double ar, double auw,
                   const std::string_view& harness)
    : WinPilot(Generate(GliderPolar::VectorVario(sproj, ar, auw, harness)), auw,
               0.0, sproj) {}

}  // namespace GliderPolar
