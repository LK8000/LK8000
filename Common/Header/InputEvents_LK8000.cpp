/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File: InputEvents_LK8000.cpp
 *
  */

/*
 * 
 * 
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

/* default Mode : No Menu */

unsigned event_id_null = InputEvents::makeEvent(&eventNull, TEXT(""), 0);
unsigned mode_id_default = InputEvents::mode2int(TEXT("default"), true);
makeLabel(mode_id_default,TEXT(""),1,event_id_null);
makeLabel(mode_id_default,TEXT(""),2,event_id_null);
makeLabel(mode_id_default,TEXT(""),3,event_id_null);
makeLabel(mode_id_default,TEXT(""),4,event_id_null);
makeLabel(mode_id_default,TEXT(""),5,event_id_null);
makeLabel(mode_id_default,TEXT(""),6,event_id_null);
makeLabel(mode_id_default,TEXT(""),7,event_id_null);
makeLabel(mode_id_default,TEXT(""),8,event_id_null);
makeLabel(mode_id_default,TEXT(""),9,event_id_null);
makeLabel(mode_id_default,TEXT(""),10,event_id_null);
makeLabel(mode_id_default,TEXT(""),11,event_id_null);
makeLabel(mode_id_default,TEXT(""),12,event_id_null);
makeLabel(mode_id_default,TEXT(""),13,event_id_null); 
makeLabel(mode_id_default,TEXT(""),14,event_id_null);
makeLabel(mode_id_default,TEXT(""),15,event_id_null);
makeLabel(mode_id_default,TEXT(""),16,event_id_null);

#ifndef TEST_MENU_LAYOUT
/* Menu Mode : ( default Menu : used only if '_System/DEFAULT_MENU.TXT' is empty ) */

int event_id = InputEvents::makeEvent(&eventExit, TEXT(""), 0);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
int mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Exit"),10,event_id);

event_id = InputEvents::makeEvent(&eventSetup, TEXT("System"), 0);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Setup\nSystem"),1,event_id);

event_id = InputEvents::makeEvent(&eventSetup, TEXT("Aircraft"), 0);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Setup\nAircraft"),2,event_id);

event_id = InputEvents::makeEvent(&eventSetup, TEXT("Pilot"), 0);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Setup\nPilot"),3,event_id);

event_id = InputEvents::makeEvent(&eventSetup, TEXT("Device"), 0);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Setup\nDevice"),4,event_id);

event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), 0);
mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("Cancel"),9,event_id);

#else
// test code for show all menu with menu id

int event_id = InputEvents::makeEvent(&eventExit, TEXT("system"), 0);
event_id = InputEvents::makeEvent(&eventMode, TEXT("default"), event_id);
int mode_id = InputEvents::mode2int(TEXT("Menu"), true);
makeLabel(mode_id,TEXT("1"),1,event_id);
makeLabel(mode_id,TEXT("2"),2,event_id);
makeLabel(mode_id,TEXT("3"),3,event_id);
makeLabel(mode_id,TEXT("4"),4,event_id);
makeLabel(mode_id,TEXT("5"),5,event_id);
makeLabel(mode_id,TEXT("6"),6,event_id);
makeLabel(mode_id,TEXT("7"),7,event_id);
makeLabel(mode_id,TEXT("8"),8,event_id);
makeLabel(mode_id,TEXT("9"),9,event_id);
makeLabel(mode_id,TEXT("10"),10,event_id);
makeLabel(mode_id,TEXT("11"),11,event_id);
makeLabel(mode_id,TEXT("12"),12,event_id);
makeLabel(mode_id,TEXT("13"),13,event_id);
makeLabel(mode_id,TEXT("14"),14,event_id);
makeLabel(mode_id,TEXT("15"),15,event_id);
makeLabel(mode_id,TEXT("16"),16,event_id);
#endif
