/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


void ThermalBand(NMEA_INFO *Basic, DERIVED_INFO *Calculated) 
{
  static double LastTime = 0;
  if(Basic->Time <= LastTime)
    {
      LastTime = Basic->Time;
      return;
    }
  LastTime = Basic->Time;

  // JMW TODO accuracy: Should really work out dt here, 
  //           but i'm assuming constant time steps
  double dheight = Calculated->NavAltitude -Calculated->TerrainBase; 

  int index, i, j;

  if (dheight<0) {
    return; // nothing to do.
  }
  if (Calculated->MaxThermalHeight==0) {
    Calculated->MaxThermalHeight = dheight;
  }

  // only do this if in thermal and have been climbing
  if ((!Calculated->Circling)||(Calculated->Average30s<0)) return;

  //  LockFlightData(); 

  if (dheight > Calculated->MaxThermalHeight) {

    // moved beyond ceiling, so redistribute buckets
    double max_thermal_height_new;
    double tmpW[NUMTHERMALBUCKETS];
    int tmpN[NUMTHERMALBUCKETS];
    double h;

    // calculate new buckets so glider is below max
    double hbuk = Calculated->MaxThermalHeight/NUMTHERMALBUCKETS;
  
    max_thermal_height_new = max(1.0, Calculated->MaxThermalHeight);
    while (max_thermal_height_new<dheight) {
      max_thermal_height_new += hbuk;
    }

    // reset counters
    for (i=0; i<NUMTHERMALBUCKETS; i++) {
      tmpW[i]= 0.0;
      tmpN[i]= 0;
    }
    // shift data into new buckets
    for (i=0; i<NUMTHERMALBUCKETS; i++) {
      h = (i)*(Calculated->MaxThermalHeight)/(NUMTHERMALBUCKETS); 
      // height of center of bucket
      j = iround(NUMTHERMALBUCKETS*h/max_thermal_height_new);

      if (j<NUMTHERMALBUCKETS) {
        if (Calculated->ThermalProfileN[i]>0) {
          tmpW[j] += Calculated->ThermalProfileW[i];
          tmpN[j] += Calculated->ThermalProfileN[i];
        }
      }
    }
    for (i=0; i<NUMTHERMALBUCKETS; i++) {
      Calculated->ThermalProfileW[i]= tmpW[i];
      Calculated->ThermalProfileN[i]= tmpN[i];
    }
    Calculated->MaxThermalHeight= max_thermal_height_new;
  }

  index = min(NUMTHERMALBUCKETS-1,
	      iround(NUMTHERMALBUCKETS*(dheight/max(1.0,
		     Calculated->MaxThermalHeight))));

  Calculated->ThermalProfileW[index]+= Calculated->Vario;
  Calculated->ThermalProfileN[index]++;
  //  UnlockFlightData();

}
