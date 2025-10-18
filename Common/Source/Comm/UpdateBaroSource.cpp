/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "Baro.h"

namespace {

void NotifyInvalidAltitude(unsigned index, double fAlt) {
  static bool notifyErr = true;
  if (notifyErr) {
    StartupStore(_T("...<device : %c> RECEIVING INVALID BARO ALTITUDE : %f"), devLetter(index), fAlt);
    DoStatusMessage(MsgToken<1530>());
    notifyErr = false;
  }
}

}

bool BaroAltitudeAvailable(const NMEA_INFO& Info) {
  return Info.BaroAltitude.available();
}

void ResetBaroAvailable(NMEA_INFO& Info) {
  if (BaroAltitudeAvailable(Info)) {
    TestLog(_T("Baro source reset @%s"), WhatTimeIsIt());
  }
  Info.BaroAltitude.reset();
}

void CheckBaroAltitudeValidity(NMEA_INFO& Info) {
  if (Info.BaroAltitude.check_expired(6000)) {
    ResetBaroAvailable(Info);
  }
}

bool GotFirstBaroAltitude = false;

/*
 * We update only if incoming from device with the highest priority.
 * FLARM has the highest priority since it is the most common used Baro device (hopefully)
 *
 * We must consider the case of multiplexers sending mixed devices, and the special case when
 * one of these devices stop sending baro altitude,
 *
 * UpdateMonitor reset `BaroSourceIdx` if no data is coming from that nmea port.
 *
 * CAUTION do not use devicedescriptor's pointers to functions without LockComm!!
 */
void UpdateBaroSource(NMEA_INFO* pGPS, DeviceDescriptor_t* d, double fAlt) {
  if (!d) {
    return;
  }
  if (!pGPS) {
    return;
  }

  // if device call this function, it is a baro source until ComPort restart.
  if (!std::exchange(d->IsBaroSource, true)) {
    // TODO : notify thread waiting for `IsBaroSource` if change form false to true.
    // currently, only Android `Internal` wait for this. 
    // must be refactored if another one is added.
    if(d->Com) {
      d->Com->CancelWaitEvent();
    }
  }

  if (pGPS->BaroAltitude.index() >= (*d)) {
    // if 'd' device has a higher priority
    if (fAlt > 30000 || fAlt < -1000) {
      // ignore out of range altitude...
      NotifyInvalidAltitude(d->PortNumber, fAlt);
    }
    else if (pGPS->BaroAltitude.update(*d, fAlt)) {
      GotFirstBaroAltitude = true;
    }
  }
}
