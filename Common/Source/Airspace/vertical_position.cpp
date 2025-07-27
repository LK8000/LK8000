/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   vertical_position.h
 * Author: Bruno de Lacheisserie
 */
#include "options.h"
#include "externs.h"
#include "vertical_position.h"
#include "utils/printf.h"
#include "utils/tokenizer.h"
#include "Library/rapidxml/rapidxml.hpp"
#include "md5.h"

using xml_attribute = rapidxml::xml_attribute<char>;

vertical_position::vertical_position(vertical_ref r, double v, Units_t u) : _ref(r) {
  switch (_ref) {
    case vertical_ref::MSL:
      _msl = Units::From(u, v);
      break;
    case vertical_ref::AGL:
      _agl = Units::From(u, v);
      break;
    case vertical_ref::FL:
      _fl = Units::To(unFligthLevel, Units::From(u, v));
      _msl = QNEAltitudeToQNHAltitude(Units::From(u, v));
      break;
    case vertical_ref::Undef:
      assert(false);
      break;
  }
}

bool vertical_position::operator==(const vertical_position& pos) const {
  if (_ref == pos._ref) {
    switch (_ref) {
      case vertical_ref::MSL:
        return _msl == pos._msl;
      case vertical_ref::AGL:
        return _agl == pos._agl;
      case vertical_ref::FL:
        return _fl == pos._fl;
      case vertical_ref::Undef:
        return true;
    }
  }
  return false;
}

bool vertical_position::below(double alt_msl, double height_agl) const {
  if (agl()) {
    return height_agl < _agl;
  }
  return alt_msl < _msl;
}

bool vertical_position::above(double alt_msl, double height_agl) const {
  if (agl()) {
    return height_agl > _agl;
  }
  return alt_msl > _msl;
}

void vertical_position::qnh_update() {
  _msl = QNEAltitudeToQNHAltitude(Units::From(unFligthLevel, _fl));
}

double vertical_position::altitude(double terrain_altitude) const {
  if (agl()) {
    return terrain_altitude + std::max(0., _agl);
  }
  return _msl;
}

tstring vertical_position::text_alternate() const {
  TCHAR sUnitBuffer[24], sAltUnitBuffer[24], intbuf[128];
  auto altUnit = Units::GetAltitudeUnit();

  auto print_alt = [&](double val, const TCHAR* suffix) {
    Units::FormatAltitude(val, sUnitBuffer, std::size(sUnitBuffer));
    Units::FormatAlternateAltitude(val, sAltUnitBuffer, std::size(sAltUnitBuffer));
    if (altUnit == unMeter) {
      lk::snprintf(intbuf, TEXT("%s %s%s"), sUnitBuffer, sAltUnitBuffer, suffix);
    }
    else {
      lk::snprintf(intbuf, TEXT("%s%s"), sUnitBuffer, suffix);
    }
  };

  switch (_ref) {
    case vertical_ref::Undef:
      print_alt(_msl, TEXT(""));
      break;
    case vertical_ref::MSL:
      print_alt(_msl, TEXT(" MSL"));
      break;
    case vertical_ref::AGL:
      if (_agl <= 0) {
        lk::strcpy(intbuf, MsgToken<2387>());  // _@M2387_ "GND"
      }
      else {
        print_alt(_agl, TEXT(" AGL"));
      }
      break;
    case vertical_ref::FL:
      if (altUnit == unMeter) {
        lk::snprintf(intbuf, TEXT("FL%.0f %.0fm %.0fft"), _fl, Units::To(unMeter, _msl), Units::To(unFeet, _msl));
      }
      else {
        lk::snprintf(intbuf, TEXT("FL%.0f %.0fft"), _fl, Units::To(unFeet, _msl));
      }
      break;
  }
  return intbuf;
}

tstring vertical_position::text() const {
  TCHAR out[128];
  if (agl()) {
    if (_agl <= 0) {
      lk::strcpy(out, MsgToken<2387>());  // _@M2387_ "GND"
    }
    else {
      TCHAR sUnitBuffer[24];
      Units::FormatAltitude(_agl, sUnitBuffer, std::size(sUnitBuffer));
      lk::snprintf(out, _T("%s AGL"), sUnitBuffer);
    }
  }
  else {
    Units::FormatAltitude(_msl, out, std::size(out));
  }
  return out;
}

vertical_position vertical_position::parse_open_air(const TCHAR* Text) {
  TCHAR sTmp[128];

  lk::strcpy(sTmp, Text);
  CharUpper(sTmp);

  double Value = 0.;
  Units_t Unit = unUndef;
  auto Ref = vertical_ref::Undef;

  lk::tokenizer<TCHAR> tok(sTmp);
  const TCHAR* pToken = tok.Next({_T(' ')}, true);

  while ((pToken) && (*pToken != '\0')) {
    // BugFix 110922
    // Malformed alt causes the parser to read wrong altitude, for example on line  AL FL65 (MNM ALT 5500ft)
    // Stop parsing if we have enough info!
    if ((Ref != vertical_ref::Undef) && (Unit != unUndef) && (Value != 0)) {
      break;
    }

    if (isdigit(*pToken)) {
      const TCHAR* Stop = nullptr;
      Value = StrToDouble(pToken, &Stop);
      if (Stop && *Stop != '\0') {
        pToken = Stop;
        continue;
      }
    }
    else if (_tcscmp(pToken, TEXT("GND")) == 0) {
      // JMW support XXXGND as valid, equivalent to XXXAGL
      Ref = vertical_ref::AGL;
    }
    else if ((_tcscmp(pToken, TEXT("SFC")) == 0) || (_tcscmp(pToken, TEXT("ASFC")) == 0)) {
      Ref = vertical_ref::AGL;
    }
    else if (_tcsstr(pToken, TEXT("FL")) == pToken) {
      // this parses "FL=150" and "FL150"
      Ref = vertical_ref::FL;
      Unit = unFligthLevel;
      if (pToken[2] != '\0') {  // no separator between FL and number
        pToken = &pToken[2];
        continue;
      }
    }
    else if ((_tcscmp(pToken, TEXT("FT")) == 0) || (_tcscmp(pToken, TEXT("F")) == 0)) {
      Unit = unFeet;
    }
    else if ((_tcscmp(pToken, TEXT("MSL")) == 0) || (_tcscmp(pToken, TEXT("AMSL")) == 0)) {
      Ref = vertical_ref::MSL;
    }
    else if (_tcscmp(pToken, TEXT("M")) == 0) {
      // JMW must scan for MSL before scanning for M
      Unit = unMeter;
    }
    else if (_tcscmp(pToken, TEXT("AGL")) == 0) {
      Ref = vertical_ref::AGL;
    }
    else if (_tcscmp(pToken, TEXT("STD")) == 0) {
      if (Ref != vertical_ref::Undef) {
        // warning! multiple base tags
      }
      Ref = vertical_ref::FL;
    }
    else if ((_tcsncmp(pToken, TEXT("UNL"), 3) == 0)) {
      // JMW added Unlimited (used by WGC2008)
      Ref = vertical_ref::MSL;
      Value = 50000;
    }

    pToken = tok.Next({_T(' '), _T('\t')}, true);
  }

  if (Ref == vertical_ref::Undef) {
    // ToDo warning! no base defined use MSL
    Ref = vertical_ref::MSL;
  }

  if (Unit == unUndef) {
    // ToDo warning! no unit defined use feet or user alt unit ?
    Unit = unFeet;
  }

  return { Ref, Value, Unit };
}

vertical_position vertical_position::parse_open_aip(const xml_node* node) {
  vertical_position Alt = {};
  if (!node) {
    throw std::runtime_error("no limit");
  }
  const xml_node* alt_node = node->first_node("ALT");
  if (!alt_node) {
    throw std::runtime_error("no alt node");
  }
  const xml_attribute* unit_attribute = alt_node->first_attribute("UNIT");
  if (!unit_attribute) {
    throw std::runtime_error("no unit attribute");
  }
  const char* dataStr = unit_attribute->value();
  if (!dataStr) {
    throw std::runtime_error("no unit value");
  }
  Units_t alt_unit = unUndef;
  switch (strlen(dataStr)) {
    case 1:  // F
      if (dataStr[0] == 'F') {
        alt_unit = unFeet;
      }
      // else if(dataStr[0]=='M') alt_unit=1; //TODO: meters not yet supported by OpenAIP
      break;
    case 2:  // FL
      if (dataStr[0] == 'F' && dataStr[1] == 'L') {
        alt_unit = unFligthLevel;
      }
      break;
    default:
      break;
  }

  if (alt_unit == unUndef) {
    throw std::runtime_error("invalid unit");
  }
  dataStr = alt_node->value();
  if (!dataStr) {
    throw std::runtime_error("no value");
  }
  double value = strtod(dataStr, nullptr);
  const xml_attribute* reference_attribute = node->first_attribute("REFERENCE");
  if (!reference_attribute) {
    throw std::runtime_error("no ref attribute");
  }
  dataStr = reference_attribute->value();
  if (dataStr && strlen(dataStr) == 3) {
    switch (dataStr[0]) {
      case 'M':  // MSL Main sea level
        if (dataStr[1] == 'S' && dataStr[2] == 'L') {
          Alt._ref = vertical_ref::MSL;
          Alt._msl = Units::From(alt_unit, value);
        }
        break;
      case 'S':  // STD Standard atmosphere
        if (dataStr[1] == 'T' && dataStr[2] == 'D') {
          Alt._ref = vertical_ref::FL;
          Alt._fl = value;
          Alt._msl = QNEAltitudeToQNHAltitude(Units::From(alt_unit, value));
        }
        break;
      case 'G':  // GND Ground
        if (dataStr[1] == 'N' && dataStr[2] == 'D') {
          Alt._ref = vertical_ref::AGL;
          Alt._agl = value > 0 ? Units::From(alt_unit, value) : -1;
        }
        break;
      default:
        break;
    }
  }
  if (Alt._ref != vertical_ref::Undef) {
    throw std::runtime_error("undef");
  }

  return Alt;
}

void vertical_position::hash(MD5& md5) const {
  md5.Update(static_cast<std::underlying_type_t<vertical_ref>>(_ref));
  switch (_ref) {
    case vertical_ref::MSL:
      md5.Update(_msl);
      break;
    case vertical_ref::AGL:
      md5.Update(_agl);
      break;
    case vertical_ref::FL:
      md5.Update(_fl);
    default:
      break;
  }
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("vertical_position") {
  tstring sGND = _T("GND");
  LKMessages[2387] = sGND.data();

  QNH = 1013.25;

  SUBCASE("method : vertical_position::parse_open_air(...)") {
    CHECK_EQ(vertical_position::parse_open_air(_T("GND")), vertical_position(vertical_ref::AGL, 0, unFeet));

    CHECK_EQ(vertical_position::parse_open_air(_T("100 SFC")), vertical_position(vertical_ref::AGL, 100, unFeet));
    CHECK_EQ(vertical_position::parse_open_air(_T("100 ASFC")), vertical_position(vertical_ref::AGL, 100, unFeet));
    CHECK_EQ(vertical_position::parse_open_air(_T("100 AGL")), vertical_position(vertical_ref::AGL, 100, unFeet));

    CHECK_EQ(vertical_position::parse_open_air(_T("100m SFC")), vertical_position(vertical_ref::AGL, 100, unMeter));
    CHECK_EQ(vertical_position::parse_open_air(_T("100m ASFC")), vertical_position(vertical_ref::AGL, 100, unMeter));
    CHECK_EQ(vertical_position::parse_open_air(_T("100m AGL")), vertical_position(vertical_ref::AGL, 100, unMeter));

    CHECK_EQ(vertical_position::parse_open_air(_T("100ft SFC")), vertical_position(vertical_ref::AGL, 100, unFeet));
    CHECK_EQ(vertical_position::parse_open_air(_T("100ft ASFC")), vertical_position(vertical_ref::AGL, 100, unFeet));
    CHECK_EQ(vertical_position::parse_open_air(_T("100ft AGL")), vertical_position(vertical_ref::AGL, 100, unFeet));

    CHECK_EQ(vertical_position::parse_open_air(_T("100")), vertical_position(vertical_ref::MSL, 100, unFeet));
    CHECK_EQ(vertical_position::parse_open_air(_T("100 MSL")), vertical_position(vertical_ref::MSL, 100, unFeet));
    CHECK_EQ(vertical_position::parse_open_air(_T("100 AMSL")), vertical_position(vertical_ref::MSL, 100, unFeet));

    CHECK_EQ(vertical_position::parse_open_air(_T("100m MSL")), vertical_position(vertical_ref::MSL, 100, unMeter));
    CHECK_EQ(vertical_position::parse_open_air(_T("100m AMSL")), vertical_position(vertical_ref::MSL, 100, unMeter));
    CHECK_EQ(vertical_position::parse_open_air(_T("100m AMSL")), vertical_position(vertical_ref::MSL, 100, unMeter));

    CHECK_EQ(vertical_position::parse_open_air(_T("100ft MSL")), vertical_position(vertical_ref::MSL, 100, unFeet));
    CHECK_EQ(vertical_position::parse_open_air(_T("100ft AMSL")), vertical_position(vertical_ref::MSL, 100, unFeet));
    CHECK_EQ(vertical_position::parse_open_air(_T("100ft AMSL")), vertical_position(vertical_ref::MSL, 100, unFeet));

    CHECK_EQ(vertical_position::parse_open_air(_T("FL65")), vertical_position(vertical_ref::FL, 65, unFligthLevel));
  }

  SUBCASE("method : vertical_position::altitude(...)") {
    CHECK_EQ(vertical_position(vertical_ref::AGL, 100, unMeter).altitude(100),
             doctest::Approx(Units::From(unMeter, 200)));
    CHECK_EQ(vertical_position(vertical_ref::MSL, 100, unMeter).altitude(100),
             doctest::Approx(Units::From(unMeter, 100)));
    CHECK_EQ(vertical_position(vertical_ref::FL, 25, unFligthLevel).altitude(100),
             doctest::Approx(Units::From(unFligthLevel, 25)));

    // TODO : check for QNH change
  }

  SUBCASE("method : below(...) / above(...)") {
    CHECK(vertical_position(vertical_ref::AGL, 100, unMeter).below(110, 90)); // 90 < 100
    CHECK(vertical_position(vertical_ref::AGL, 100, unMeter).above(90, 110)); // 110 > 100

    CHECK(vertical_position(vertical_ref::MSL, 100, unMeter).below(90, 110)); // 90 < 100
    CHECK(vertical_position(vertical_ref::MSL, 100, unMeter).above(110, 90)); // 110 > 100

    CHECK(vertical_position(vertical_ref::FL, 100, unMeter).below(90, 110)); // 100 < 110
    CHECK(vertical_position(vertical_ref::FL, 100, unMeter).above(110, 90)); // 100 > 110

    CHECK_FALSE(vertical_position(vertical_ref::AGL, 100, unMeter).below(90, 110)); // 110 > 100
    CHECK_FALSE(vertical_position(vertical_ref::AGL, 100, unMeter).above(110, 90)); // 90 < 100

    CHECK_FALSE(vertical_position(vertical_ref::MSL, 100, unMeter).below(110, 90)); // 110 > 100
    CHECK_FALSE(vertical_position(vertical_ref::MSL, 100, unMeter).above(90, 110)); // 90 < 100

    CHECK_FALSE(vertical_position(vertical_ref::FL, 100, unMeter).below(110, 90)); // 100 > 110
    CHECK_FALSE(vertical_position(vertical_ref::FL, 100, unMeter).above(90, 110)); // 100 < 110


    CHECK(vertical_position(vertical_ref::AGL, 0, unMeter).sfc());
    CHECK(vertical_position(vertical_ref::MSL, 0, unMeter).below_msl());

    CHECK_FALSE(vertical_position(vertical_ref::AGL, 100, unMeter).sfc());
    CHECK_FALSE(vertical_position(vertical_ref::MSL, 100, unMeter).below_msl());
    CHECK_FALSE(vertical_position(vertical_ref::FL, 100, unMeter).below_msl());
    CHECK_FALSE(vertical_position(vertical_ref::FL, 0, unMeter).below_msl());
  }

  SUBCASE("method : text(...)") {

    Units::AltitudeUnit_Config = 0; // Feet
    Units::NotifyUnitChanged();

    CHECK_EQ(vertical_position(vertical_ref::AGL, 00, unFeet).text(), _T("GND"));
    CHECK_EQ(vertical_position(vertical_ref::AGL, 100, unFeet).text(), _T("100ft AGL"));
    CHECK_EQ(vertical_position(vertical_ref::MSL, 100, unFeet).text(), _T("100ft"));
    CHECK_EQ(vertical_position(vertical_ref::FL, 100, unFeet).text(), _T("100ft"));

    Units::AltitudeUnit_Config = 1; // meter
    Units::NotifyUnitChanged();

    CHECK_EQ(vertical_position(vertical_ref::AGL, 00, unMeter).text(), _T("GND"));
    CHECK_EQ(vertical_position(vertical_ref::AGL, 100, unMeter).text(), _T("100m AGL"));
    CHECK_EQ(vertical_position(vertical_ref::MSL, 100, unMeter).text(), _T("100m"));
    CHECK_EQ(vertical_position(vertical_ref::FL, 100, unMeter).text(), _T("100m"));
  }

  SUBCASE("method : text_alternate(...)") {

    Units::AltitudeUnit_Config = 0; // Feet
    Units::NotifyUnitChanged();

    CHECK_EQ(vertical_position(vertical_ref::AGL, 00, unFeet).text_alternate(), _T("GND"));
    CHECK_EQ(vertical_position(vertical_ref::AGL, 100, unFeet).text_alternate(), _T("100ft AGL"));
    CHECK_EQ(vertical_position(vertical_ref::MSL, 100, unFeet).text_alternate(), _T("100ft MSL"));
    CHECK_EQ(vertical_position(vertical_ref::FL, 45, unFligthLevel).text_alternate(), _T("FL45 4500ft"));

    Units::AltitudeUnit_Config = 1; // meter
    Units::NotifyUnitChanged();

    CHECK_EQ(vertical_position(vertical_ref::AGL, 00, unMeter).text_alternate(), _T("GND"));
    CHECK_EQ(vertical_position(vertical_ref::AGL, 100, unMeter).text_alternate(), _T("100m 328ft AGL"));
    CHECK_EQ(vertical_position(vertical_ref::MSL, 100, unMeter).text_alternate(), _T("100m 328ft MSL"));
    CHECK_EQ(vertical_position(vertical_ref::FL, 45, unFligthLevel).text_alternate(), _T("FL45 1372m 4500ft"));
  }  

  std::fill(LKMessages.begin(), LKMessages.end(), nullptr);
}

#endif  // DOCTEST_CONFIG_DISABLE
