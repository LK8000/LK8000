/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


// HeadWind error will be shown as -999
// These values are all in m/s
// We can have a serious problem when the headwind is so strong that the
// aircraft is actually flying backwards. 
// In such case, the heading will show correctly and the pilot should see
// that there is a problem.
//
// A positive value indicates a headwind, and a negative value indicates a tailwind.
//
void CalculateHeadWind(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  static double CrossBearingLast= -1.0;
  static double WindSpeedLast= -1.0;

  if (Basic->NAVWarning) {
	Calculated->HeadWind  = -999;
	return;
  }

  double CrossBearing;
  CrossBearing = AngleLimit360(Calculated->Heading - Calculated->WindBearing);

  #if 1 // vector wind
  if ((CrossBearing != CrossBearingLast)||(Calculated->WindSpeed != WindSpeedLast)) {
	Calculated->HeadWind = Calculated->WindSpeed * fastcosine(CrossBearing);
	// CrossWind = WindSpeed * fastsine(CrossBearing);  UNUSED
	CrossBearingLast = CrossBearing;
	WindSpeedLast = Calculated->WindSpeed;
  }
  #else
  if (Basic->AirspeedAvailable) {
	Calculated->HeadWind = Basic->TrueAirspeed - Basic->Speed;
  } else {
	// for estimated IAS, this is also vector wind
	Calculated->HeadWind = Calculated->TrueAirspeedEstimated - Basic->Speed;
  }
  #endif
  //StartupStore(_T("..... CrossBearing=%f  hdwind=%f windspeed=%f\n"),CrossBearing,Calculated->HeadWind, Calculated->WindSpeed);

}

