/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_DISPLAY_ORIENTATION_HPP
#define XCSOAR_DISPLAY_ORIENTATION_HPP

#include <stdint.h>

enum class DisplayOrientation_t : uint8_t {
  DEFAULT,
  PORTRAIT,
  LANDSCAPE,
  REVERSE_PORTRAIT,
  REVERSE_LANDSCAPE,
};

#ifdef KOBO
/* Kobo defaults to portrait */
static constexpr DisplayOrientation_t DEFAULT_DISPLAY_ORIENTATION =
  DisplayOrientation_t::PORTRAIT;
#else
/* everything else defaults to landscape */
static constexpr DisplayOrientation_t DEFAULT_DISPLAY_ORIENTATION =
  DisplayOrientation_t::LANDSCAPE;
#endif

static constexpr inline DisplayOrientation_t
TranslateDefaultDisplayOrientation(DisplayOrientation_t orientation)
{
  return orientation == DisplayOrientation_t::DEFAULT
    ? DEFAULT_DISPLAY_ORIENTATION
    : orientation;
}

static inline bool
AreAxesSwapped(DisplayOrientation_t orientation)
{
  switch (TranslateDefaultDisplayOrientation(orientation)) {
  case DisplayOrientation_t::DEFAULT:
  case DisplayOrientation_t::LANDSCAPE:
  case DisplayOrientation_t::REVERSE_LANDSCAPE:
    break;

  case DisplayOrientation_t::PORTRAIT:
  case DisplayOrientation_t::REVERSE_PORTRAIT:
    return true;
  }

  return false;
}

#endif
