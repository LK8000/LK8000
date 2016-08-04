/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "DoInits.h"


#ifdef NEWCLIMBAV
ClimbAverageCalculator climbAverageCalculator;
void Average30s(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
	Calculated->Average30s = climbAverageCalculator.GetAverage(Basic->Time, Calculated->NavAltitude, 30);	
}

#endif

void Average30s(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static double LastTime = 0;
  static double Altitude[30];
  static double Vario[30];
  int Elapsed, i;
  long index = 0; 
  double Gain;
  static int num_samples = 0;
  static bool lastCircling = false;

  if (DoInit[MDI_AVERAGE30S]) {
	LastTime = 0;
	num_samples = 0;
	lastCircling = false;
	DoInit[MDI_AVERAGE30S]=false;
  }

  if(Basic->Time > LastTime)
    {

      if (Calculated->Circling != lastCircling) {
        num_samples = 0;
        // reset!
      }
      lastCircling = Calculated->Circling;

      Elapsed = (int)(Basic->Time - LastTime);
      for(i=0;i<Elapsed;i++)
        {
          index = (long)LastTime + i;
          index %= 30;

          Altitude[index] = Calculated->NavAltitude;
	  if (Basic->VarioAvailable) {
	    Vario[index] = Basic->Vario;
	  } else {
	    Vario[index] = Calculated->Vario;
	  }

          if (num_samples<30) {
            num_samples ++;
          }

        }

      double Vave = 0;
      int j;
      for (i=0; i< num_samples; i++) {
        j = (index - i) % 30;
        if (j<0) { 
          j += 30;
        }
        Vave += Vario[j];
      }
      if (num_samples) {
        Vave /= num_samples;
      }

      if (!Basic->VarioAvailable) {
        index = ((long)Basic->Time - 1)%30;
        Gain = Altitude[index];
        
        index = ((long)Basic->Time)%30;
        Gain = Gain - Altitude[index];

        Vave = Gain/30;
      }
      Calculated->Average30s = 
        LowPassFilter(Calculated->Average30s,Vave,0.8);
    }
  else
    {
      if (Basic->Time<LastTime) {
	// gone back in time
	for (i=0; i<30; i++) {
	  Altitude[i]= 0;
	  Vario[i]=0;
	}
      }
    }
  LastTime = Basic->Time;
}

void AverageThermal(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if (Calculated->ClimbStartTime>=0) {
    if(Basic->Time > Calculated->ClimbStartTime)
      {
        double Gain = 
          Calculated->NavAltitude+Calculated->EnergyHeight 
            - Calculated->ClimbStartAlt;
        #if BUGSTOP
        LKASSERT((Basic->Time - Calculated->ClimbStartTime)!=0);
        #endif
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
