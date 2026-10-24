/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   VectorVario.h
 * Author: Bruno de Lacheisserie
 */
#pragma once

#include "GliderPolar/Discret.h"
#include <string_view>

namespace GliderPolar {

class VectorVario : public Discret {
 public:
  using Discret::Discret;
  using Points = Discret::Points;

  explicit VectorVario(Points) = delete;

  VectorVario(double sproj, double ar, double auw,
              const std::string_view& harness);
};

}  // namespace GliderPolar
