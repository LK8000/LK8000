/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "utils/heapcheck.h"


// It is called GPSVario but it is really a vario using best altitude available.. baro if possible
void Vario(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static double LastTime = 0;
  static double LastAlt = 0;
  double myTime;

  myTime=Basic->Time; 

  if(myTime <= LastTime) {
    LastTime = myTime;
    LastAlt = Calculated->NavAltitude;
  } else {
    double Gain = Calculated->NavAltitude - LastAlt;
    double dT = (Basic->Time - LastTime);
    Calculated->GPSVario = Gain / dT;
    LastAlt = Calculated->NavAltitude;
    LastTime = myTime;
  }

  if (!Basic->VarioAvailable || ReplayLogger::IsEnabled()) {
    Calculated->Vario = Calculated->GPSVario;
  } else {
    // get value from instrument
    Calculated->Vario = Basic->Vario;
  }
}
