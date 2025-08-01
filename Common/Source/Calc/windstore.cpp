/***********************************************************************
**
**   windstore.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**
***********************************************************************/
#include "Compiler.h"
#include "options.h"
#include "windstore.h"
#include "NMEA/Info.h"
#include "NMEA/Derived.h"
#include "Defines.h"
#include "MessageLog.h"
#include "Units.h"
/**
  * Called with new measurements. The quality is a measure for how
  * good the measurement is. Higher quality measurements are more
  * important in the end result and stay in the store longer.
  */
void WindStore::slot_measurement(NMEA_INFO *nmeaInfo,
                                 DERIVED_INFO *derivedInfo,
                                 Vector windvector, int quality){
  updated = true;
  windlist.addMeasurement(nmeaInfo->Time, windvector, nmeaInfo->Altitude, quality);
  //we may have a new wind value, so make sure it's emitted if needed!
  recalculateWind(nmeaInfo, derivedInfo);
}


/**
  * Called if the altitude changes.
  * Determines where measurements are stored and may result in a
  * newWind signal.
  */

void WindStore::slot_Altitude(NMEA_INFO *nmeaInfo,
                              DERIVED_INFO *derivedInfo){

  if ((fabs(nmeaInfo->Altitude-_lastAltitude)>100.0)||(updated)) {
    //only recalculate if there is a significant change
    recalculateWind(nmeaInfo, derivedInfo);

    updated = false;
    _lastAltitude=nmeaInfo->Altitude;
  }
}


Vector WindStore::getWind(double Time, double h, bool *found) {
  return windlist.getWind(Time, h, found);
}

/** Recalculates the wind from the stored measurements.
  * May result in a newWind signal. */

void WindStore::recalculateWind(NMEA_INFO *nmeaInfo,
                                DERIVED_INFO *derivedInfo) {
  bool found;
  Vector CurWind = windlist.getWind(nmeaInfo->Time,
                                    nmeaInfo->Altitude, &found);

  if (found) {
    if (updated || ManhattanDistance(CurWind, _lastWind) > 1.0) {
      _lastWind = CurWind;

      updated = false;
      _lastAltitude = nmeaInfo->Altitude;

      newWind(nmeaInfo, derivedInfo, CurWind);
    }
  }  // otherwise, don't change anything
}


void WindStore::newWind(NMEA_INFO *nmeaInfo, DERIVED_INFO *derivedInfo,
                        Vector &wind) {
  //
  double mag = Length(wind);
  double bearing;

  if (wind.y == 0 && wind.x == 0)
    bearing = 0;
  else
    bearing = atan2(wind.y, wind.x)*RAD_TO_DEG;

  if (mag<30) { // limit to reasonable values
    derivedInfo->WindSpeed = mag;
    if (bearing<0) {
      bearing += 360;
    }
    derivedInfo->WindBearing = bearing;
  } else {
    // TODO code: give warning, wind estimate bogus or very strong!
  }

#ifndef NDEBUG
  TCHAR Time[48];
  Units::TimeToTextS(Time, nmeaInfo->Time);

  DebugLog(_T("wind <%s> : update %3.0f° / %3.0f km/h"),
                Time,
                derivedInfo->WindBearing, 
                derivedInfo->WindSpeed * 3.6);
#endif

}
