/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "utils/heapcheck.h"


bool ValidStartSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated, DWORD Margin) {
  bool valid = true;
  if (StartMaxSpeed!=0) {
    if (Basic->AirspeedAvailable) {
      if ((Basic->IndicatedAirspeed*1000)>(StartMaxSpeed+Margin))
        valid = false;
    } else {
	// StartMaxSpeed is in millimeters per second, and so is Margin
	if ((Basic->Speed*1000)>(StartMaxSpeed+Margin))  { //@ 101014 FIX
		valid = false;
	}
    }
  }
  return valid;
}


bool ValidStartSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  return ValidStartSpeed(Basic, Calculated, 0);
}


bool ValidFinish(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
 (void)Basic;
  if ( ((FinishMinHeight/1000)>0) && (Calculated->TerrainValid) && (Calculated->AltitudeAGL < (FinishMinHeight/1000))) {
	return false;
  } else {
	return true;
  }
  
}
