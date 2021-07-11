/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "ClimbAverageCalculator.h"

void Average30s(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static ClimbAverageCalculator<30,5> climbAverageCalculator;
  
	Calculated->Average30s = climbAverageCalculator.GetAverage(Basic->Time, Calculated->NavAltitude);	
}

void AverageThermal(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if (Calculated->ClimbStartTime>=0) {
    if(Basic->Time > Calculated->ClimbStartTime)
      {
        double Gain = 
          Calculated->NavAltitude+Calculated->EnergyHeight 
            - Calculated->ClimbStartAlt;

        BUGSTOP_LKASSERT((Basic->Time - Calculated->ClimbStartTime)!=0);

        if ((Basic->Time - Calculated->ClimbStartTime)!=0)
        Calculated->AverageThermal  = 
          Gain / (Basic->Time - Calculated->ClimbStartTime);
      }
  }
}

void MaxHeightGain(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if (!Calculated->Flying) return;
  if (!Calculated->FreeFlying && (ISGLIDER||ISPARAGLIDER)) return;

  if (Calculated->MinAltitude>0) {
    double height_gain = Calculated->NavAltitude - Calculated->MinAltitude;
    Calculated->MaxHeightGain = max(height_gain, Calculated->MaxHeightGain);
  } else {
    Calculated->MinAltitude = Calculated->NavAltitude;
  }
  Calculated->MinAltitude = min(Calculated->NavAltitude, Calculated->MinAltitude);
  Calculated->MaxAltitude = max(Calculated->NavAltitude, Calculated->MaxAltitude);
}


void ThermalGain(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if (Calculated->ClimbStartTime>=0) {
    if(Basic->Time >= Calculated->ClimbStartTime)
      {
        Calculated->ThermalGain = 
          Calculated->NavAltitude + Calculated->EnergyHeight 
          - Calculated->ClimbStartAlt;
      }
  }
}
