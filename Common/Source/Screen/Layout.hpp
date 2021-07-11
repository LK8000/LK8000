/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * File:   Layout.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on January 5, 2016, 1:30 AM
 */

#ifndef LAYOUT_HPP
#define	LAYOUT_HPP

#include "Screen/Point.hpp"
#include "Defines.h"
#include "ScreenGeometry.h"

namespace Layout {
   PixelScalar Scale(const PixelScalar x) {
       return IBLSCALE(x);
   }

};

#endif	/* LAYOUT_HPP */

