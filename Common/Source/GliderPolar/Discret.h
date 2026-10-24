/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Discret.h
 * Author: Bruno de Lacheisserie
 */
#pragma once

#include "GliderPolar/Base.h"
#include "Math/Point2D.hpp"
#include <algorithm>
#include <cstddef>
#include <vector>
#include <ranges>

namespace GliderPolar {

/**
 * Discretized glider polar representation.
 */
class Discret : public GliderPolar::Base {
 public:
  using Point = Point2D<double>;
  using Points = std::vector<Point>;

 public:
  Discret() = delete;

  Discret(Discret&&) = default;
  Discret& operator=(Discret&&) = default;

  Discret(const Discret&) = default;
  Discret& operator=(const Discret&) = default;

  explicit Discret(Points pts) : points(std::move(pts)) {
    std::ranges::sort(points, ComparePointByX());
  }

  /**
   * Calculate the sink rate for a given airspeed.
   *
   * @param v Airspeed m/s.
   * @return Sink rate m/s.
   * @pre The discret polar must be valid (IsValid() returns true).
   */
  double Sink(double v) const override;

  /**
   * Calculate the speed to fly for a given MacCready value.
   *
   * @param mc MacCready value m/s.
   * @return Speed to fly m/s.
   */
  double STF(double mc) const override;

  /**
   * Check if the discret polar is valid.
   *
   * @return True if valid, false otherwise.
   */
  bool IsValid() const override {
    // The discret polar is considered valid if it contains at least three
    // points sorted by x-coordinate.
    if (points.size() >= 3) {
      return std::ranges::is_sorted(points, ComparePointByX());
    }
    return false;
  }

  /**
   * stl-style accessors for the points vector.
   */
  auto begin() const {
    return points.begin();
  }
  auto end() const {
    return points.end();
  }
  auto front() const {
    return points.front();
  }
  auto back() const {
    return points.back();
  }
  size_t size() const {
    return points.size();
  }

 private:
  Points points;

  struct ComparePointByX {
    bool operator()(const Point& a, const Point& b) const {
      return a.x < b.x;
    }
  };
};

}  // namespace GliderPolar
