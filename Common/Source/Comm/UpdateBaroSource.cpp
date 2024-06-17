/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "Baro.h"

namespace {

BaroIndex GetBaroIndex(const DeviceDescriptor_t* d) {
  /**
   * id `d` is nullptr, source is internal sensor, use `NUMDEV` for it's index
   */
  return {
    d ? d->nmeaParser.isFlarm : false,
    d ? d->PortNumber : NUMDEV
  };
}

void NotifyInvalidAltitude(unsigned index, double fAlt) {
  static bool notifyErr = true;
  if (notifyErr) {
    StartupStore(_T("...<device : %u> RECEIVING INVALID BARO ALTITUDE : %f"), index, fAlt);
    DoStatusMessage(MsgToken<1530>());
    notifyErr = false;
  }
}

PeriodClock lastBaroUpdate; // to check time elapsed since last baro altitude update

}

bool BaroAltitudeAvailable(const NMEA_INFO& Info) {
  return Info.BaroSourceIdx.device_index < NUMDEV;
}

void ResetBaroAvailable(NMEA_INFO& Info) {
  Info.BaroSourceIdx.is_flarm = false;
  Info.BaroSourceIdx.device_index = NUMDEV;
  lastBaroUpdate.Reset();
}

void CheckBaroAltitudeValidity(NMEA_INFO& Info) {
  if (lastBaroUpdate.Check(6000)) {
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
  assert(pGPS);
  assert(d);

  // if device call this function, it is a baro source until ComPort restart.
  if (!std::exchange(d->IsBaroSource, true)) {
    // TODO : notify thread waiting for `IsBaroSource` if change form false to true.
    // currently, only Android `Internal` wait for this. 
    // must be refactored if another one is added.
    if(d->Com) {
      d->Com->CancelWaitEvent();
    }
  }

  BaroIndex Index = GetBaroIndex(d);
  if (Index <= pGPS->BaroSourceIdx) {
    // if 'd' device has a higher priority
    if (fAlt > 30000 || fAlt < -1000) {
      // ignore out of range altitude...
      NotifyInvalidAltitude(Index.device_index, fAlt);
    }
    else {
      pGPS->BaroAltitude = fAlt;
      pGPS->BaroSourceIdx = Index;

      GotFirstBaroAltitude = true;
      lastBaroUpdate.Update();
    }
  }
}
