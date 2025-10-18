/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

*/

#include "externs.h"
#include "McReady.h"
#include "Baro.h"



//
// Simply returns gps or baro altitude, and total energy in use within LK
// Using total energy means adding a speed energy calculated here as 
// EnergyHeight to the arrival altitudes. It won't change glide ratios.
// As of 110913 this is totally experimental.
//
void EnergyHeightNavAltitude(NMEA_INFO *Basic, DERIVED_INFO *Calculated) 
{
  // Determine which altitude to use for nav functions
  if (EnableNavBaroAltitude && BaroAltitudeAvailable(*Basic)) {
    Calculated->NavAltitude = Basic->BaroAltitude.value();
  } else {
    Calculated->NavAltitude = Basic->Altitude;
  }

  if (UseTotalEnergy) {
	double ias_to_tas;
	double V_tas, wastefactor;

	if (Basic->IndicatedAirSpeed.available() && Basic->TrueAirSpeed.available()) {
		ias_to_tas = Basic->TrueAirSpeed.value()/Basic->IndicatedAirSpeed.value();
		V_tas = Basic->TrueAirSpeed.value();
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
