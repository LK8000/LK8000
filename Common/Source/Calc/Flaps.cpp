/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"

void Flaps(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{	
	if (GlidePolar::FlapsMass<=0) return; // avoid division by zero crashes
	double speed = 0.0;
	if (Basic->AirspeedAvailable) {
		speed = (int)(Basic->IndicatedAirspeed);
	} else {
		speed = (int)(Calculated->IndicatedAirspeedEstimated);
	}

	// correcting speed for calculated bank angle
	if (cos(Calculated->BankAngle*DEG_TO_RAD)>0) 
	speed = speed/sqrt(1/cos(Calculated->BankAngle*DEG_TO_RAD));

	double massCorrectionFactor = sqrt(GlidePolar::GetAUW()/GlidePolar::FlapsMass);

	for (int i=0;i<GlidePolar::FlapsPosCount-1;i++) {
		if (speed >= GlidePolar::FlapsPos[i]*massCorrectionFactor 
			&& speed < GlidePolar::FlapsPos[i+1]*massCorrectionFactor) {
			LK_tcsncpy(Calculated->Flaps,GlidePolar::FlapsName[i],MAXFLAPSNAME);
		}
	}	
}
