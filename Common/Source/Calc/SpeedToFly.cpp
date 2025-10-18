/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
  auto netto = [&]() {
    if (Basic->NettoVario.available()) {
      return Basic->NettoVario.value();
    }
    else {
      return Calculated->NettoVario;
    }
  };

    // calculate maximum efficiency speed to fly
    double HeadWind = 0;
    if ( Calculated->HeadWind != -999 ) {
        HeadWind = Calculated->HeadWind;
    }
    double Vme = GlidePolar::STF(0, netto(), HeadWind);
    Calculated->Vme = LowPassFilter(Calculated->Vme, Vme, 0.6);

    HeadWind = 0;
    if (Calculated->FinalGlide && ValidTaskPoint(ActiveTaskPoint)) {
        // according to MC theory STF take account of wind only if on final Glide
        // TODO : for the future add config parameter for always use wind.
        if (Calculated->HeadWind != -999) {
            HeadWind = Calculated->HeadWind;
        }
    }


    // this is IAS for best Ground Glide ratio acounting current air mass ( wind / Netto vario )
    double VOptnew = GlidePolar::STF(MACCREADY, netto(), HeadWind);

    // apply cruises efficiency factor.
    VOptnew *= CRUISE_EFFICIENCY;

    if (netto() > MACCREADY) {
        // this air mass is better than maccready, so don't fly at speed less than minimum sink speed adjusted for load factor
        const double GLoad = Calculated->Gload;

        VOptnew = max(VOptnew, GlidePolar::Vminsink() * sqrt(GLoad));
    } else {
        // never fly at speed less than min sink speed
        VOptnew = max(VOptnew, GlidePolar::Vminsink());
    }

    // use low pass filter for avoid big jump of value.
    Calculated->VOpt = LowPassFilter(Calculated->VOpt, VOptnew, 0.6);
}

