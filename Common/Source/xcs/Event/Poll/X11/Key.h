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

#ifndef XCSOAR_SCREEN_X11_KEY_H
#define XCSOAR_SCREEN_X11_KEY_H

#include <X11/keysym.h>

enum {
  KEY_RETURN = XK_Return,
  KEY_ESCAPE = XK_Escape,
  KEY_PRIOR = XK_Page_Up,
  KEY_NEXT = XK_Page_Down,
  KEY_UP = XK_Up,
  KEY_DOWN = XK_Down,
  KEY_LEFT = XK_Left,
  KEY_RIGHT = XK_Right,
  KEY_HOME = XK_Home,
  KEY_END = XK_End,
  KEY_TAB = XK_Tab,
  KEY_BACK = XK_BackSpace,
  KEY_SPACE = XK_space,
  KEY_F1 = XK_F1,
  KEY_F2 = XK_F2,
  KEY_F3 = XK_F3,
  KEY_F4 = XK_F4,
  KEY_F5 = XK_F5,
  KEY_F6 = XK_F6,
  KEY_F7 = XK_F7,
  KEY_F8 = XK_F8,
  KEY_F9 = XK_F9,
  KEY_F10 = XK_F10,
  KEY_F11 = XK_F11,
  KEY_F12 = XK_F12,
  KEY_MENU = XK_Menu,
  KEY_APP1,
  KEY_APP2,
  KEY_APP3,
  KEY_APP4,
  KEY_APP5,
  KEY_APP6,

  KEY_0 = XK_0,
  KEY_1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9 = XK_9, // '9'

  KEY_A = XK_A,  // 'A'
  KEY_B,
  KEY_C,
  KEY_D,
  KEY_E,
  KEY_F,
  KEY_G,
  KEY_H,
  KEY_I,
  KEY_J,
  KEY_K,
  KEY_L,
  KEY_M,
  KEY_N,
  KEY_O,
  KEY_P,
  KEY_Q,
  KEY_R,
  KEY_S,
  KEY_T,
  KEY_U,
  KEY_V,
  KEY_W,
  KEY_X,
  KEY_Y,
  KEY_Z = XK_Z,  // 'Z'


  KEY_SHIFT = XK_Shift_L,
  KEY_CONTROL = XK_Control_L,
};

#endif
