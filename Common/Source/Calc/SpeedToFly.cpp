/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"


extern double CRUISE_EFFICIENCY;

//
// Sollfarh / Dolphin Speed calculator
//
void SpeedToFly(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  double n;
  // get load factor
  if (Basic->AccelerationAvailable) {
    n = fabs(Basic->AccelZ);
  } else {
    n = fabs(Calculated->Gload);
  }

  double delta_mc;
  double current_mc = MACCREADY;

  delta_mc = current_mc-Calculated->NettoVario;

  // TODO FIX should we use this approach?
  if (1 || (Calculated->Vario <= current_mc)) {
    // airmass value is worse than mc threshold, so find opt cruise speed

    double VOptnew;
    
    if (!ValidTaskPoint(ActiveWayPoint) || !Calculated->FinalGlide) {
      // calculate speed as if cruising, wind has no effect on opt speed
      GlidePolar::MacCreadyAltitude(delta_mc,
                                    100.0, // dummy value
                                    Basic->TrackBearing, 
                                    0.0, 
                                    0.0, 
                                    NULL, 
                                    &VOptnew, 
                                    false, 
                                    NULL, 0, CRUISE_EFFICIENCY);
    } else {
      GlidePolar::MacCreadyAltitude(delta_mc,
                                    100.0, // dummy value
                                    Basic->TrackBearing, 
                                    Calculated->WindSpeed, 
                                    Calculated->WindBearing, 
                                    0, 
                                    &VOptnew, 
                                    true,
                                    NULL, 1.0e6, CRUISE_EFFICIENCY);
    }
    
    // put low pass filter on VOpt so display doesn't jump around
    // too much
    if (Calculated->Vario <= current_mc) {
      Calculated->VOpt = max(Calculated->VOpt,
			     GlidePolar::Vminsink()*sqrt(n));
    } else {
      Calculated->VOpt = max(Calculated->VOpt,
			     (double)GlidePolar::Vminsink());
    }
    Calculated->VOpt = LowPassFilter(Calculated->VOpt,VOptnew, 0.6);
    
  } else {
    // this air mass is better than maccready, so fly at minimum sink speed
    // calculate speed of min sink adjusted for load factor 
    Calculated->VOpt = GlidePolar::Vminsink()*sqrt(n);
  }

}

