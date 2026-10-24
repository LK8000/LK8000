/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WinPilot.h
 * Author: Bruno de Lacheisserie
 */
#pragma once

#include "GliderPolar/Discret.h"
#include <array>

namespace GliderPolar {

/**
 * WinPilot class store data from ".plr" polar files.
 * TODO: Implement methods to read and parse ".plr" polar files.
 */
class WinPilot {
 public:
  using Point = Discret::Point;
  using Points = std::array<Point, 3>;

  WinPilot(double sproj, double ar, double auw, const std::string_view& harness);

  WinPilot(Points points, double dry_weight, double ballast_weight,
           double wing_area)
      : points(std::move(points)),
        dry_weight(dry_weight),
        ballast_weight(ballast_weight),
        wing_area(wing_area) {}

  double GetDryWeight() const {
    return dry_weight;
  }
  double GetBallastWeight() const {
    return ballast_weight;
  }
  double GetWingArea() const {
    return wing_area;
  }
  const Points& GetPoints() const {
    return points;
  }

 private:
  Points points;

  double dry_weight;
  double ballast_weight;
  double wing_area;
};

}  // namespace GliderPolar
