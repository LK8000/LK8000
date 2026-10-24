/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Quadratic.h
 * Author: Bruno de Lacheisserie
 */
#pragma once

#include <array>
#include <cmath>
#include "GliderPolar/Base.h"
#include "Math/Point2D.hpp"

namespace GliderPolar {

class Quadratic : public GliderPolar::Base {
 public:
  using Point = Point2D<double>;

 public:
  Quadratic() = delete;

  Quadratic(Quadratic&&) = default;
  Quadratic& operator=(Quadratic&&) = default;

  Quadratic(const Quadratic&) = default;
  Quadratic& operator=(const Quadratic&) = default;

  Quadratic(double a, double b, double c) noexcept : a(a), b(b), c(c) {}

  Quadratic(const Point& p1, const Point& p2, const Point& p3) noexcept;

  explicit Quadratic(const std::array<Point, 3>& points) noexcept
      : Quadratic(points[0], points[1], points[2]) {}

  /**
   * Calculate the sink rate for a given airspeed.
   *
   * @param v Airspeed m/s.
   * @return Sink rate m/s.
   */
  double Sink(double v) const override {
    return a * v * v + b * v + c;
  }

  /**
   * Calculate the speed to fly for a given MacCready value.
   *
   * @param mc MacCready value m/s.
   * @return Speed to fly m/s.
   */
  double STF(double mc) const override {
    double num = c - mc;
    if (num < 0.) {
      return std::sqrt(num / a);
    }
    return MinSpeed();
  }

  /**
   * Calculate the speed to fly for a given MacCready value, netto, and headwind.
   *
   * @param mc MacCready value m/s.
   * @param netto Netto lift m/s.
   * @param headwind Headwind m/s.
   * @return Speed to fly m/s.
   */
  double STF(double mc, double netto, double headwind) const {
    double num = a * (netto - mc + Sink(headwind));
    if (num > 0.) {
      return headwind - std::sqrt(num) / a;
    }
    return MinSpeed();
  }

  /**
   * Calculate the minimum speed for the quadratic polar.
   * The minimum speed is calculated as the speed at which the sink rate is minimum.
   * It corresponds to the vertex of the quadratic function.
   * @note The vertex of a quadratic function ax^2 + bx + c is at x = -b / (2a).
   *
   * @return Minimum speed m/s.
   */
  double MinSpeed() const noexcept {
    return -b / (2. * a);
  }

  /**
   * Check if the quadratic polar is valid.
   *
   * @return True if valid, false otherwise.
   */
  bool IsValid() const override;

  /**
   * Get the coefficients of the quadratic polar.
   *
   * @return Array containing the coefficients [a, b, c].
   */
  std::array<double, 3> GetCoefficients() const noexcept {
    return {a, b, c};
  }
  
 private:
  double a;
  double b;
  double c;
};

}  // namespace GliderPolar
