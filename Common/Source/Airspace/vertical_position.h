/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   vertical_position.h
 * Author: Bruno de Lacheisserie
 */
#ifndef _AIRSPACE_VERTICAL_POSITION_H_
#define _AIRSPACE_VERTICAL_POSITION_H_

#include "tchar.h"
#include "Units.h"

namespace rapidxml {
  template <class Ch>
  class xml_node;
}

class MD5;

enum class vertical_ref : uint8_t {
  Undef = 0,
  MSL,
  AGL,
  FL
};

class vertical_position {
 public:
  vertical_position() = default;
  vertical_position(vertical_ref r, double v, Units_t u);

  bool operator==(const vertical_position& ref) const;

  bool agl() const {
    return _ref == vertical_ref::AGL;
  }

  bool sfc() const {
    return (_ref == vertical_ref::AGL) && (_agl <= 0);
  }

  bool below_msl() const {
    return (_ref == vertical_ref::MSL) && (_msl <= 0);
  }

  bool below(double alt_msl, double height_agl) const;
  bool above(double alt_msl, double height_agl) const;

  void qnh_update();
  double altitude(double terrain_altitude) const;

  tstring text_alternate() const;

  tstring text() const;

  static vertical_position parse_open_air(const TCHAR* Text);

  using xml_node = class rapidxml::xml_node<char>;
  static vertical_position parse_open_aip(const xml_node* node);

  void hash(MD5& md5) const;

 private:
  double _msl = 0.;
  double _fl = 0.;
  double _agl = 0.;
  vertical_ref _ref = vertical_ref::Undef;
};

#endif  // _AIRSPACE_VERTICAL_POSITION_H_
