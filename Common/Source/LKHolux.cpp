/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifdef PNA

#include "StdAfx.h"
#include "compatibility.h"
#include "options.h"
#include "Defines.h"
#include "externs.h"
#include "utils/heapcheck.h"


bool DeviceIsGM130=false;

bool Init_GM130(void) {
  DeviceIsGM130=false;
  return false;
}

void DeInit_GM130(void) {
  if (!DeviceIsGM130) return;
  DeviceIsGM130=false;
  return;
}

int GM130BarAltitude(void) {
  return 0;
}

int GM130PowerLevel(void) {
  return 0;
}

int GM130PowerStatus(void) {
  return 0;
}

int GM130PowerFlag(void) {
  return 0;
}

void GM130MaxBacklight(void) {
  return;
}

// There is no real sound, only a buzzzer. A real pity on this jewel.
void GM130MaxSoundVolume(void) {
  return;
}

#endif // PNA only
