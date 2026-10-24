/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Base.h
 * Author: Bruno de Lacheisserie
 */
#pragma once

#include <memory>
#include <type_traits>

namespace GliderPolar {

class Base {
 public:
  Base() = default;
  virtual ~Base() = default;

  /**
   * Calculate the sink rate for a given airspeed.
   *
   * @param v Airspeed m/s.
   * @return Sink rate m/s.
   */
  virtual double Sink(double v) const = 0;

  /**
   * Calculate the speed to fly for a given MacCready value.
   *
   * @param mc MacCready value m/s.
   * @return Speed to fly m/s.
   */
  virtual double STF(double mc) const = 0;

  /**
   * Check if the polar is valid.
   *
   * @return True if valid, false otherwise.
   */
  virtual bool IsValid() const = 0;
};

template <typename TP, typename... Args>
  requires std::is_base_of_v<Base, TP>
std::unique_ptr<TP> Create(Args&&... args) {
  return std::make_unique<TP>(std::forward<Args>(args)...);
}

}  // namespace GliderPolar
