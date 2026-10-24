/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Test.cpp
 * Author: Bruno de Lacheisserie
 */
#include "options.h"

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include "Quadratic.h"
#include "LXNav.h"

TEST_CASE("Quadratic Glider Polar Tests") {
  GliderPolar::Quadratic polar(-0.01, 0.1, -1.0);
  SUBCASE("Sink evaluates quadratic function") {
    CHECK(polar.Sink(5.0) == doctest::Approx(-0.75));
    CHECK(polar.Sink(10.0) == doctest::Approx(-1.0));
    CHECK(polar.Sink(20.0) == doctest::Approx(-3.0));
    CHECK(polar.Sink(25.0) == doctest::Approx(-4.75));
  }
  SUBCASE("MinSpeed returns parabola vertex") {
    CHECK(polar.MinSpeed() == doctest::Approx(5.0));
  }
  SUBCASE("STF(mc) returns minimum speed when mc equals c") {
    CHECK(polar.STF(-1.0) == doctest::Approx(polar.MinSpeed()));
  }
  SUBCASE("STF(mc) returns minimum speed when no real solution exists") {
    CHECK(polar.STF(-2.0) == doctest::Approx(polar.MinSpeed()));
  }
  SUBCASE("STF(mc) matches analytical solution") {
    // mc = 0
    // sqrt((c - mc) / a)
    // sqrt(1 / -0.01) = 10
    CHECK(polar.STF(0.0) == doctest::Approx(10.0));
  }
  SUBCASE("STF(mc) increases with MacCready") {
    const auto stf0 = polar.STF(0.0);
    const auto stf1 = polar.STF(0.5);
    CHECK(stf1 > stf0);
  }
  SUBCASE("STF(mc, netto, wind) known value") {
    CHECK(polar.STF(0.0, 0.0, 5.0) == doctest::Approx(13.6602540378444));
  }
  SUBCASE("STF satisfies MacCready condition") {
    const double mc = 0.2;
    const double v = polar.STF(mc);
    const double dSink = -0.02 * v + 0.1;
    CHECK(polar.Sink(v) - v * dSink == doctest::Approx(mc));
  }
  SUBCASE("LXNAV polar to Quadratic") {
    double fa = 1.58;
    double fb = -2.46;
    double fc = 1.54;
    auto polar = GliderPolar::From_LXNAV(fa, fb, fc);
    CHECK(polar.IsValid());
    CHECK(polar.Sink(100 / 3.6) == doctest::Approx(-(fa + fb + fc)));
  }
}

#endif
