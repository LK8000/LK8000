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

#ifndef XCSOAR_SCREEN_ANDROID_KEY_H
#define XCSOAR_SCREEN_ANDROID_KEY_H

/**
 * @see http://developer.android.com/reference/android/view/KeyEvent.html
 */
enum {
  KEY_UP = 0x13, // KEYCODE_DPAD_UP
  KEY_DOWN = 0x14, // KEYCODE_DPAD_DOWN
  KEY_LEFT = 0x15, // KEYCODE_DPAD_LEFT
  KEY_RIGHT = 0x16, // KEYCODE_DPAD_RIGHT
  KEY_SHIFT = 0x3b, // KEYCODE_SHIFT_LEFT
  KEY_TAB = 0x3d, // KEYCODE_TAB
  KEY_SPACE = 0x3e, // KEYCODE_SPACE
  KEY_RETURN = 0x42, // KEYCODE_ENTER
  KEY_MENU = 0x52, // KEYCODE_MENU
  KEY_ESCAPE = 0x6f, // KEYCODE_ESCAPE
  KEY_CONTROL = 0x71, // KEYCODE_CTRL_LEFT

  KEY_BACK = 0x43, // KEYCODE_DEL (Backspace key. Deletes characters before the insertion point, unlike KEYCODE_FORWARD_DEL.)
  KEY_HOME = 0x7a, // KEYCODE_MOVE_HOME
  KEY_END = 0x7b, // KEYCODE_MOVE_END
  KEY_PRIOR = 0x5c, // KEYCODE_PAGE_UP
  KEY_NEXT = 0x5d, // KEYCODE_PAGE_DOWN
  KEY_F1 = 0x83, // KEYCODE_F1
  KEY_F2 = 0x84, // KEYCODE_F2
  KEY_F3 = 0x85, // KEYCODE_F3
  KEY_F4 = 0x86, // KEYCODE_F4
  KEY_F5 = 0x87, // KEYCODE_F5
  KEY_F6 = 0x88, // KEYCODE_F6
  KEY_F7 = 0x89, // KEYCODE_F7
  KEY_F8 = 0x8a, // KEYCODE_F8
  KEY_F9 = 0x8b, // KEYCODE_F9
  KEY_F10 = 0x8c, // KEYCODE_F10
  KEY_F11 = 0x8d, // KEYCODE_F11
  KEY_F12 = 0x8e, // KEYCODE_F12
  KEY_APP1 = 0x91, // KEYCODE_NUMPAD_1
  KEY_APP2 = 0x92, // KEYCODE_NUMPAD_2
  KEY_APP3 = 0x93, // KEYCODE_NUMPAD_3
  KEY_APP4 = 0x94, // KEYCODE_NUMPAD_4
  KEY_APP5 = 0x95, // KEYCODE_NUMPAD_5
  KEY_APP6 = 0x96, // KEYCODE_NUMPAD_6

  KEY_0 = 0x07, // KEYCODE_0
  KEY_1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9 = 0x10, // KEYCODE_9

  KEY_A = 0x1d,  // KEYCODE_A
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
  KEY_Z = 0x36,  // KEYCODE_Z
};

#endif
