/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "Calc/Vario.h"

void ResetVarioAvailable(NMEA_INFO& Info) {
  Info.VarioSourceIdx = NUMDEV;
}

bool VarioAvailable(const NMEA_INFO& Info) {
  return (!ReplayLogger::IsEnabled()) && (Info.VarioSourceIdx < NUMDEV);
}

const DeviceDescriptor_t* getVarioDevice(const NMEA_INFO& Info) {
  if (VarioAvailable(Info)) {
    return devGetDeviceOnPort(Info.VarioSourceIdx);
  }
  return nullptr;
}

void UpdateVarioSource( NMEA_INFO& Info, const DeviceDescriptor_t& d, double Vario) {
  if (d.PortNumber <= Info.VarioSourceIdx) {

    Info.VarioSourceIdx = d.PortNumber;
    Info.Vario = Vario;

    TriggerVarioUpdate();
  }
}

// It is called GPSVario but it is really a vario using best altitude available.. baro if possible
void Vario(const NMEA_INFO& Info, DERIVED_INFO& Calculated)
{
  static double LastTime = 0;
  static double LastAlt = 0;

  const double myTime = Info.Time; 
  const double myAlt = Calculated.NavAltitude;
  
  const double dT = (myTime - LastTime);
  if(dT > 0) {
    const double Gain = myAlt - LastAlt;
    Calculated.GPSVario = Gain / dT;
  }

  LastTime = myTime;
  LastAlt = myAlt;
  
  if (!VarioAvailable(Info) || ReplayLogger::IsEnabled()) {
    Calculated.Vario = Calculated.GPSVario;
  } else {
    // get value from instrument
    Calculated.Vario = Info.Vario;
  }
}
