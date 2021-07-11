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
  if (Basic->AirspeedAvailable && VarioAvailable(*Basic)  
      && (!Calculated->Circling)) {

    int vi = iround(Basic->IndicatedAirspeed);

    if ((vi<=0) || (vi>= SAFTEYSPEED)) {
      // out of range
      return;
    }
    if (Basic->AccelerationAvailable) {
      if (fabs(fabs(Basic->AccelZ)-1.0)>0.25) {
        // G factor too high
        return;
      }
    } 
    if (Basic->TrueAirspeed>0) {

      // TODO: Check this is correct for TAS/IAS

      double ias_to_tas = Basic->IndicatedAirspeed/Basic->TrueAirspeed;
      double w_tas = Basic->Vario*ias_to_tas;

      BUGSTOP_LKASSERT(vi<MAXAVERAGECLIMBRATESIZE);

      if (vi >= MAXAVERAGECLIMBRATESIZE) return; // because SAFTEYSPEED IS USER DEFINED!
      Calculated->AverageClimbRate[vi]+= w_tas;
      Calculated->AverageClimbRateN[vi]++;
    }
  }
}
