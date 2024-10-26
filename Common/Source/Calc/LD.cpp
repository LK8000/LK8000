/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "NavFunctions.h"
#include "Vario.h"
#include "LDRotaryBuffer.h"
#include "Time/GPSClock.hpp"
#include "Vario.h"
#include "LDRotaryBuffer.h"
#include "Time/GPSClock.hpp"

extern void InsertWindRotary(windrotary_s* wbuf, double speed, double track, double altitude);

namespace {

double LimitLD(double LD) {
  if (fabs(LD) > INVALID_GR) {
    return INVALID_GR;
  } else {
    if ((LD >= 0.0) && (LD < 1.0)) {
      LD = 1.0;
    }
    if ((LD < 0.0) && (LD > -1.0)) {
      LD = -1.0;
    }
    return LD;
  }
}

double UpdateLD(double LD, double d, double h, double filter_factor) {
  if (LD == 0.) {
    return INVALID_GR;
  }
  if (d != 0) {
    double glideangle = LowPassFilter(1.0 / LD, h / d, filter_factor);
    if (glideangle != 0) {
      LD = LimitLD(1.0 / glideangle);
    }
    else {
      LD = INVALID_GR;
    }
  }
  return LD;
}

}  // namespace

void LD(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  static std::optional<AGeoPoint> LastPosition;
  static GPSClock clock;

  if (clock.CheckReverse(Basic->Time)) {
    Calculated->LDvario = INVALID_GR;
    Calculated->LD = INVALID_GR;
  }

  if (clock.CheckAdvance(Basic->Time, 0.1)) {  // 10 Hz maximum
    AGeoPoint Position = GetCurrentPosition(*Basic);
    Position.altitude = Calculated->NavAltitude;

    if (LastPosition) {
      double DistanceFlown = Position.Distance(LastPosition.value());
      double AltDiff = LastPosition->altitude - Calculated->NavAltitude;

      Calculated->LD = UpdateLD(Calculated->LD, DistanceFlown, AltDiff, 0.1);

      rotaryLD.Insert(DistanceFlown, *Basic, *Calculated);

      InsertWindRotary(&rotaryWind, Basic->Speed, Basic->TrackBearing, Calculated->NavAltitude);  // 100103

      if (DistanceFlown < 300) {
        if (ISCAR) {
          Calculated->Odometer += DistanceFlown;
        } else {
          if (!Calculated->Circling && DistanceFlown > 3) {
            Calculated->Odometer += DistanceFlown;
          }
        }
      }
    }

    if (std::exchange(LKSW_ResetOdometer, false)) {
      Calculated->Odometer = 0;
      TestLog(_T("... Odometer RESET by request"));
    }

    LastPosition = Position;
  }

  // LD instantaneous from vario, updated every reading..
  if (VarioAvailable(*Basic) && Basic->AirspeedAvailable && Calculated->Flying) {
    Calculated->LDvario = UpdateLD(Calculated->LDvario, Basic->IndicatedAirspeed, -Basic->Vario, 0.3);
  } else {
    Calculated->LDvario = INVALID_GR;
  }
}

void CruiseLD(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  if (!Calculated->Circling) {
    double DistanceFlown;

    if (Calculated->CruiseStartTime < 0) {
      Calculated->CruiseStartLat = Basic->Latitude;
      Calculated->CruiseStartLong = Basic->Longitude;
      Calculated->CruiseStartAlt = Calculated->NavAltitude;
      Calculated->CruiseStartTime = Basic->Time;
    } else {
      DistanceBearing(Basic->Latitude, Basic->Longitude, Calculated->CruiseStartLat, Calculated->CruiseStartLong,
                      &DistanceFlown, NULL);
      Calculated->CruiseLD =
          UpdateLD(Calculated->CruiseLD, DistanceFlown, Calculated->CruiseStartAlt - Calculated->NavAltitude, 0.5);
    }
  }
}
