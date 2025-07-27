/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   vertical_bound.h
 * Author: Bruno de Lacheisserie
 */
#ifndef _AIRSPACE_VERTICAL_BOUND_H_
#define _AIRSPACE_VERTICAL_BOUND_H_

#include <optional>
#include "vertical_position.h"

class MD5;

class vertical_bound {
 public:
  vertical_bound() = default;

  vertical_bound(const vertical_position& first, const std::optional<vertical_position>& second) : _first(first), _second(second) {}

  void update(const vertical_position& first, const vertical_position& second);

  bool agl() const;
  bool sfc() const;

  void qnh_update();
  double altitude(double terrain_altitude) const;

  tstring text_alternate() const;

  tstring text() const;

  bool below_msl() const;

  bool below(double alt_msl, double height_agl) const;
  bool above(double alt_msl, double height_agl) const;

  void hash(MD5& md5) const;

 private:
  vertical_position _first;
  std::optional<vertical_position> _second;
};

#endif  //_AIRSPACE_VERTICAL_BOUND_H_
