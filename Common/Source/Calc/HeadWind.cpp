/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

// These values are all in m/s
// We can have a serious problem when the headwind is so strong that the
// aircraft is actually flying backwards.
// In such case, the heading will show correctly and the pilot should see
// that there is a problem.
//
// A positive value indicates a headwind, and a negative value indicates a
// tailwind.
//
void CalculateHeadWind(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  if (Basic->NAVWarning) {
    Calculated->HeadWind = -999; // invalid value for LKProcess
    return;
  }

  if (Basic->TrueAirSpeed.available()) {
    Calculated->HeadWind = Basic->TrueAirSpeed - Basic->Speed;
  } else {
    double CrossBearing = AngleLimit360(Calculated->Heading - Calculated->WindBearing);
    Calculated->HeadWind = Calculated->WindSpeed * fastcosine(CrossBearing);
  }
}
