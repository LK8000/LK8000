/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

*/

#include "externs.h"
#include "McReady.h"



//
// Simply returns gps or baro altitude, and total energy in use within LK
// Using total energy means adding a speed energy calculated here as 
// EnergyHeight to the arrival altitudes. It won't change glide ratios.
// As of 110913 this is totally experimental.
//
void EnergyHeightNavAltitude(NMEA_INFO *Basic, DERIVED_INFO *Calculated) 
{

  // Determine which altitude to use for nav functions
  if (EnableNavBaroAltitude && Basic->BaroAltitudeAvailable) {
    Calculated->NavAltitude = Basic->BaroAltitude;
  } else {
    Calculated->NavAltitude = Basic->Altitude;
  }

  if (UseTotalEnergy) {
	double ias_to_tas;
	double V_tas, wastefactor;

	if (Basic->AirspeedAvailable && (Basic->IndicatedAirspeed>0)) {
		ias_to_tas = Basic->TrueAirspeed/Basic->IndicatedAirspeed;
		V_tas = Basic->TrueAirspeed;
		wastefactor=0.80;
	} else {
		ias_to_tas = 1.0;
		V_tas = Calculated->TrueAirspeedEstimated;
		wastefactor=0.70;
	}
	double V_min_tas = ( GlidePolar::Vminsink() + (GlidePolar::Vbestld() - GlidePolar::Vminsink())/2)*ias_to_tas;
	V_tas = max(V_tas, V_min_tas);

	Calculated->EnergyHeight = ( (V_tas*V_tas-V_min_tas*V_min_tas)/(9.81*2.0)*wastefactor);
  } else 
	Calculated->EnergyHeight = 0;

}
