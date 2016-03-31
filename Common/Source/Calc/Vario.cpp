/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"


// It is called GPSVario but it is really a vario using best altitude available.. baro if possible
void Vario(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static double LastTime = 0;
  static double LastAlt = 0;

  const double myTime = Basic->Time; 
  const double myAlt = Calculated->NavAltitude;
  
  const double dT = (myTime - LastTime);
  if(dT > 0) {
    const double Gain = myAlt - LastAlt;
    Calculated->GPSVario = Gain / dT;
  }

  LastTime = myTime;
  LastAlt = myAlt;
  
  if (!Basic->VarioAvailable || ReplayLogger::IsEnabled()) {
    Calculated->Vario = Calculated->GPSVario;
  } else {
    // get value from instrument
    Calculated->Vario = Basic->Vario;
  }
}
