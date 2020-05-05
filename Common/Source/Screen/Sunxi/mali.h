/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   mali.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 29 décembre 2017
 */

#ifndef SCREEN_SUNXI_MALI_H
#define SCREEN_SUNXI_MALI_H

#ifdef HAVE_MALI

#include "Compiler.h"
#include "Screen/Point.hpp"
#include "DisplayOrientation.hpp"

namespace mali {
  
  /**
   * request display Size in pixel to "/dev/disp"
   */
  gcc_pure
  PixelSize GetScreenSize();
  
  /**
   * read display Orientation in "/boot/config.uEnv"
   */
  gcc_pure
  DisplayOrientation_t GetScreenOrientation();
}

#endif /* HAVE_MALI */
#endif /* SCREEN_SUNXI_MALI_H */
