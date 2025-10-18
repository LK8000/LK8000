/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Vario.h"

void AverageClimbRate(NMEA_INFO *Basic, DERIVED_INFO *Calculated) 
{
  if (Basic->IndicatedAirSpeed.available() && VarioAvailable(*Basic) &&
      (!Calculated->Circling))
  {
    int vi = iround(Basic->IndicatedAirSpeed.value());

    if ((vi<=0) || (vi>= SAFTEYSPEED)) {
      // out of range
      return;
    }
    if (Basic->Acceleration.available() || Basic->Gload.available()) {
      if (fabs(fabs(Calculated->Gload) - 1.0) > 0.25) {
        // G factor too high
        return;
      }
    }

    if (Basic->TrueAirSpeed.available()) {

      // TODO: Check this is correct for TAS/IAS

      double ias_to_tas =
          Basic->IndicatedAirSpeed.value() / Basic->TrueAirSpeed.value();
      double w_tas = Basic->Vario.value() * ias_to_tas;

      BUGSTOP_LKASSERT(vi<MAXAVERAGECLIMBRATESIZE);

      if (vi >= MAXAVERAGECLIMBRATESIZE) return; // because SAFTEYSPEED IS USER DEFINED!
      Calculated->AverageClimbRate[vi]+= w_tas;
      Calculated->AverageClimbRateN[vi]++;
    }
  }
}
