/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "Vario.h"

extern void NettoVario(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void SpeedToFly(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

void DoCalculationsVario(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  if(Basic && Calculated) {
    Vario(*Basic,*Calculated);
    NettoVario(Basic, Calculated);
    SpeedToFly(Basic, Calculated);
  }
}
