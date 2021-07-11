/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"



bool InsideStartHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated, unsigned Margin) {
  bool valid = true;
  if ((StartMaxHeight!=0)&&(Calculated->TerrainValid)) {
    if (StartHeightRef == 0) {
      if ((Calculated->AltitudeAGL*1000)>(StartMaxHeight+Margin)) // 101015
	valid = false;
    } else {
      if ((Calculated->NavAltitude*1000)>(StartMaxHeight+Margin)) // 101015
	valid = false;
    }
  }
  return valid;
}

bool InsideStartHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  return InsideStartHeight(Basic, Calculated, 0);
}
