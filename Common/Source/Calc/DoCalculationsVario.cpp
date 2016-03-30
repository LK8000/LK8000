/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

extern void Vario(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void NettoVario(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void SpeedToFly(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

void DoCalculationsVario(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
    
  Vario(Basic,Calculated);
  NettoVario(Basic, Calculated);
  SpeedToFly(Basic, Calculated);
}
