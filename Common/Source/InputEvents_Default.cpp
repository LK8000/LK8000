/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: InputEvents_Default.cpp
 */

#include "InputEvents.h"

/*
 * Landscape :
 * 
 *                     0      1      2      3      4
 *                 +------+------+------+------+------+
 *               0 |  10  |  11  |  12  |  13  |   1  |
 *                 +------+------+------+------+------+
 *               1 |  14  |                    |   2  |
 *                 +------+                    +------+
 *               2 |  15  |                    |   3  |
 *                 +------+                    +------+
 *               3 |  16  |                    |   4  |
 *                 +------+------+------+------+------+
 *               4 |   5  |   6  |   7  |   8  |   9  |
 *                 +------+------+------+------+------+
 * 
 * Portrait :
 * 
 *                         0      1      2      3
 *                     +------+------+------+------+
 *                   0 |  10  |  11  |  12  |  13  |
 *                     +------+------+------+------+
 *                   1 |  14  |             |   5  |
 *                     +------+             +------+
 *                   2 |  15  |             |   6  |
 *                     +------+             +------+
 *                   3 |  16  |             |   7  |
 *                     +------+             +------+
 *                   4 |                    |   8  |
 *                     +                    +------+
 *                   5 |                    |   9  |
 *                     +------+------+------+------+
 *                   6 |   1  |   2  |   3  |   4  |
 *                     +------+------+------+------+
 * 
 */

void InputEvents::InitDefaultMode() {
  /* default Mode : No Menu */
  unsigned mode_id_default = mode2int(_T("default"), true);
  unsigned event_id_null = makeEvent(&eventNull, _T(""), 0);
  makeLabel(mode_id_default,_T(""),1,event_id_null);
  makeLabel(mode_id_default,_T(""),2,event_id_null);
  makeLabel(mode_id_default,_T(""),3,event_id_null);
  makeLabel(mode_id_default,_T(""),4,event_id_null);
  makeLabel(mode_id_default,_T(""),5,event_id_null);
  makeLabel(mode_id_default,_T(""),6,event_id_null);
  makeLabel(mode_id_default,_T(""),7,event_id_null);
  makeLabel(mode_id_default,_T(""),8,event_id_null);
  makeLabel(mode_id_default,_T(""),9,event_id_null);
  makeLabel(mode_id_default,_T(""),10,event_id_null);
  makeLabel(mode_id_default,_T(""),11,event_id_null);
  makeLabel(mode_id_default,_T(""),12,event_id_null);
  makeLabel(mode_id_default,_T(""),13,event_id_null); 
  makeLabel(mode_id_default,_T(""),14,event_id_null);
  makeLabel(mode_id_default,_T(""),15,event_id_null);
  makeLabel(mode_id_default,_T(""),16,event_id_null);

#ifndef TEST_MENU_LAYOUT
  /* Menu Mode : ( default Menu : used only if '_System/DEFAULT_MENU.TXT' is empty ) */

  int mode_id = mode2int(_T("Menu"), true);
  int default_event_id = makeEvent(&eventMode, _T("default"), 0);

  int event_id = makeEvent(&eventExit, _T(""), default_event_id);
  makeLabel(mode_id,_T("Exit"),10,event_id);

  event_id = makeEvent(&eventSetup, _T("System"), default_event_id);
  makeLabel(mode_id,_T("Setup\nSystem"),1,event_id);

  event_id = makeEvent(&eventSetup, _T("Aircraft"), default_event_id);
  makeLabel(mode_id,_T("Setup\nAircraft"),2,event_id);

  event_id = makeEvent(&eventSetup, _T("Pilot"), default_event_id);
  makeLabel(mode_id,_T("Setup\nPilot"),3,event_id);

  event_id = makeEvent(&eventSetup, _T("Device"), default_event_id);
  makeLabel(mode_id,_T("Setup\nDevice"),4,event_id);

  makeLabel(mode_id,_T("Cancel"),9,default_event_id);

#else
  // test code for show all menu with menu id

  int mode_id = mode2int(_T("Menu"), true);
  int event_id = makeEvent(&eventExit, _T("Exit"), 0);
  event_id = makeEvent(&eventMode, _T("default"), event_id);
  makeLabel(mode_id,_T("1"),1,event_id);
  makeLabel(mode_id,_T("2"),2,event_id);
  makeLabel(mode_id,_T("3"),3,event_id);
  makeLabel(mode_id,_T("4"),4,event_id);
  makeLabel(mode_id,_T("5"),5,event_id);
  makeLabel(mode_id,_T("6"),6,event_id);
  makeLabel(mode_id,_T("7"),7,event_id);
  makeLabel(mode_id,_T("8"),8,event_id);
  makeLabel(mode_id,_T("9"),9,event_id);
  makeLabel(mode_id,_T("10"),10,event_id);
  makeLabel(mode_id,_T("11"),11,event_id);
  makeLabel(mode_id,_T("12"),12,event_id);
  makeLabel(mode_id,_T("13"),13,event_id);
  makeLabel(mode_id,_T("14"),14,event_id);
  makeLabel(mode_id,_T("15"),15,event_id);
  makeLabel(mode_id,_T("16"),16,event_id);
#endif
}
