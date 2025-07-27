/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   vertical_bound.h
 * Author: Bruno de Lacheisserie
 */
#include "options.h"
#include "externs.h"
#include "vertical_bound.h"
#include <algorithm>
#include "utils/strcpy.h"

void vertical_bound::update(const vertical_position& first, const vertical_position& second) {
  if (!_second) {
    if (_first == first) {
      _second = second;
    }
    else if (_first == second) {
      _second = first;
    }
    else {
      assert(false); // invalid file ???
    }
  }
}

bool vertical_bound::agl() const {
  return _first.agl() || (_second && _second->agl());
}

bool vertical_bound::sfc() const {
  return _first.sfc() || (_second && _second->sfc());
}

void vertical_bound::qnh_update() {
  _first.qnh_update();
  if (_second) {
    _second->qnh_update();
  }
}

double vertical_bound::altitude(double terrain_altitude) const {
  double alt = _first.altitude(terrain_altitude);
  if (_second) {
    return std::max(alt, _second->altitude(terrain_altitude));
  }
  return alt;
}

tstring vertical_bound::text_alternate() const {
  tstring out = _first.text_alternate();
  if (_second) {
    out += _T(" / ");
    out += _second->text_alternate();
  }
  return out;
}

tstring vertical_bound::text() const {
  tstring out = _first.text();
  if (_second) {
    out += _T(" / ");
    out += _second->text();
  }
  return out;
}

bool vertical_bound::below_msl() const {
  return _first.below_msl();
}

// Returns true if either _first or _second (if present) is below the given msl or agl
bool vertical_bound::below(double alt_msl, double height_agl) const {
  return _first.below(alt_msl, height_agl) || (_second && _second->below(alt_msl, height_agl));
}

// Returns true if _first is above and (if _second exists) _second is also above
bool vertical_bound::above(double alt_msl, double height_agl) const {
  return _first.above(alt_msl, height_agl) && (!_second || _second->above(alt_msl, height_agl));
}

void vertical_bound::hash(MD5& md5) const {
  _first.hash(md5);
  if (_second) {
    _second->hash(md5);
  }
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("vertical_bound") {
  QNH = 1013.25;
  tstring sGND = _T("GND");
  LKMessages[2387] = sGND.data();

  SUBCASE("method : vertical_bound::altitude(...)") {
    CHECK_EQ(vertical_bound({vertical_ref::AGL, 100, unMeter}, {}).altitude(100),
             doctest::Approx(Units::From(unMeter, 200)));
    CHECK_EQ(vertical_bound({vertical_ref::MSL, 100, unMeter}, {}).altitude(100),
             doctest::Approx(Units::From(unMeter, 100)));
    CHECK_EQ(vertical_bound({vertical_ref::FL, 25, unFligthLevel}, {}).altitude(100),
             doctest::Approx(Units::From(unFligthLevel, 25)));


    CHECK_EQ(vertical_bound({vertical_ref::MSL, 100, unMeter}, 
                            {{vertical_ref::AGL, 75, unMeter}}).altitude(0),
             doctest::Approx(Units::From(unMeter, 100)));
    CHECK_EQ(vertical_bound({vertical_ref::MSL, 100, unMeter}, 
                            {{vertical_ref::AGL, 70, unMeter}}).altitude(50),
             doctest::Approx(Units::From(unMeter, 120)));
  }

  SUBCASE("method : below(...) / above(...)") {
    CHECK(vertical_bound({vertical_ref::AGL, 100, unMeter}, {}).below(110, 90)); // 90 < 100
    CHECK(vertical_bound({vertical_ref::AGL, 100, unMeter}, {}).above(90, 110)); // 110 > 100

    CHECK(vertical_bound({vertical_ref::MSL, 100, unMeter},
                         {{vertical_ref::AGL, 70, unMeter}}).below(90, 60));

    CHECK(vertical_bound({vertical_ref::MSL, 100, unMeter},
                         {{vertical_ref::AGL, 70, unMeter}}).above(110, 90));


    CHECK_FALSE(vertical_bound({vertical_ref::MSL, 100, unMeter},
                         {{vertical_ref::AGL, 70, unMeter}}).below(110, 90));

    CHECK_FALSE(vertical_bound({vertical_ref::MSL, 100, unMeter},
                         {{vertical_ref::AGL, 70, unMeter}}).above(90, 60));


    CHECK(vertical_bound({vertical_ref::AGL, 0, unMeter}, {}).sfc());
    CHECK(vertical_bound({vertical_ref::MSL, 0, unMeter}, {}).below_msl());

    CHECK_FALSE(vertical_bound({vertical_ref::AGL, 100, unMeter}, {}).sfc());
    CHECK_FALSE(vertical_bound({vertical_ref::MSL, 100, unMeter}, {}).below_msl());
    CHECK_FALSE(vertical_bound({vertical_ref::FL, 100, unMeter}, {}).below_msl());
    CHECK_FALSE(vertical_bound({vertical_ref::FL, 0, unMeter}, {}).below_msl());
  }

  SUBCASE("method : text(...)") {
    Units::AltitudeUnit_Config = 0; // Feet
    Units::NotifyUnitChanged();

    CHECK_EQ(vertical_bound({vertical_ref::AGL, 100, unFeet}, {}).text(), _T("100ft AGL"));
    CHECK_EQ(vertical_bound({vertical_ref::AGL, 100, unFeet},
                            {{vertical_ref::MSL, 150, unFeet}}).text(), _T("100ft AGL / 150ft"));
  }

  SUBCASE("method : text_alternate(...)") {

    Units::AltitudeUnit_Config = 0; // Feet
    Units::NotifyUnitChanged();

    CHECK_EQ(vertical_bound({vertical_ref::AGL, 100, unFeet}, {}).text_alternate(), _T("100ft AGL"));
    CHECK_EQ(vertical_bound({vertical_ref::AGL, 100, unFeet},
                            {{vertical_ref::MSL, 150, unFeet}}).text_alternate(), _T("100ft AGL / 150ft MSL"));
  }  

  std::fill(LKMessages.begin(), LKMessages.end(), nullptr);
}
#endif  // DOCTEST_CONFIG_DISABLE
