/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Discret.h
 * Author: Bruno de Lacheisserie
 */
#include "Discret.h"
#include <cassert>
#include <algorithm>

namespace GliderPolar {

double Discret::Sink(double v) const {
  assert(IsValid());

  auto it = std::upper_bound(points.begin(), points.end(), v,
                             [](double value, const Point& p) {
                               return value < p.x;
                             });

  if (it != points.begin() && it != points.end()) {
    const auto& p1 = *std::prev(it);
    const auto& p2 = *it;
    // Linear interpolation
    const double t = (v - p1.x) / (p2.x - p1.x);
    return p1.y + t * (p2.y - p1.y);
  }
  else if (it == points.begin()) {
    return points.front().y;
  }
  else if (it == points.end()) {
    return points.back().y;
  }

  return 0.0;
}

double Discret::STF(double mc) const {
  double best_v = points.front().x;
  double best_f = std::numeric_limits<double>::max();

  for (const auto& p : points) {
    const double f = std::abs((-p.y + mc) / p.x);
    if (f < best_f) {
      best_f = f;
      best_v = p.x;
    }
  }

  return best_v;
}

}  // namespace GliderPolar
