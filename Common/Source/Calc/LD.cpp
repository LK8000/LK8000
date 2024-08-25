/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "NavFunctions.h"
#include "Calc/Vario.h"

extern void InsertLDRotary(ldrotary_s *buf, double distance, NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void InsertWindRotary(windrotary_s *wbuf, double speed, double track, double altitude);


double LimitLD(double LD) {
  if (fabs(LD)>INVALID_GR) {
    return INVALID_GR;
  } else {
    if ((LD>=0.0)&&(LD<1.0)) {
      LD= 1.0;
    } 
    if ((LD<0.0)&&(LD>-1.0)) {
      LD= -1.0;
    }
    return LD;
  }
}


double UpdateLD(double LD, double d, double h, double filter_factor) {
  double glideangle;
  if (LD != 0) {
    glideangle = 1.0/LD;
  } else {
    glideangle = 1.0;
  }
  if (d!=0) {
    LKASSERT(LD!=0);
    if (LD==0) return INVALID_GR;

    glideangle = LowPassFilter(1.0/LD, h/d, filter_factor);
    if (fabs(glideangle) > 1.0/INVALID_GR) {
      LKASSERT(glideangle!=0);
      if (glideangle!=0) 
      	LD = LimitLD(1.0/glideangle);
      else
        LD = INVALID_GR;
    } else {
      LD = INVALID_GR;
    }
  }
  return LD;
}


void LD(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static double LastLat = 0;
  static double LastLon = 0;
  static double LastTime = 0;
  static double LastAlt = 0;

  if (Basic->Time<LastTime) {
    LastTime = Basic->Time;
    Calculated->LDvario = INVALID_GR;
    Calculated->LD = INVALID_GR;
  } 
  if(Basic->Time >= LastTime+1.0)
    {
      double DistanceFlown=0;

      // 121212 do not insert LD with a distance from the north pole..
      if (LastLat!=0)
          DistanceBearing(Basic->Latitude, Basic->Longitude, 
                      LastLat, LastLon,
                      &DistanceFlown, NULL);

      Calculated->LD = UpdateLD(Calculated->LD,
                                DistanceFlown,
                                LastAlt - Calculated->NavAltitude, 0.1);

      if (LastLat!=0) InsertLDRotary(&rotaryLD, DistanceFlown, Basic, Calculated);
      InsertWindRotary(&rotaryWind, Basic->Speed, Basic->TrackBearing, Calculated->NavAltitude); // 100103
      if (ISCAR) {
         if (DistanceFlown<300) Calculated->Odometer += DistanceFlown;
      } else {
         if (!Calculated->Circling && DistanceFlown >3 && DistanceFlown<300) Calculated->Odometer += DistanceFlown;
      }

      if (LKSW_ResetOdometer) {
		Calculated->Odometer = 0;
		TestLog(_T("... Odometer RESET by request"));
		LKSW_ResetOdometer=false;
      }

      LastLat = Basic->Latitude;
      LastLon = Basic->Longitude;
      LastAlt = Calculated->NavAltitude;
      LastTime = Basic->Time;
    }

  // LD instantaneous from vario, updated every reading..
  if (VarioAvailable(*Basic) && Basic->AirspeedAvailable 
      && Calculated->Flying) {
    Calculated->LDvario = UpdateLD(Calculated->LDvario,
                                   Basic->IndicatedAirspeed,
                                   -Basic->Vario,
                                   0.3);
  } else {
    Calculated->LDvario = INVALID_GR;
  }
}



void CruiseLD(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{

  if(!Calculated->Circling)
    {
      double DistanceFlown;

      if (Calculated->CruiseStartTime<0) {
        Calculated->CruiseStartLat = Basic->Latitude;
        Calculated->CruiseStartLong = Basic->Longitude;
        Calculated->CruiseStartAlt = Calculated->NavAltitude;
        Calculated->CruiseStartTime = Basic->Time;
      } else {

        DistanceBearing(Basic->Latitude, Basic->Longitude, 
                        Calculated->CruiseStartLat, 
                        Calculated->CruiseStartLong, &DistanceFlown, NULL);
        Calculated->CruiseLD = 
          UpdateLD(Calculated->CruiseLD,
                   DistanceFlown,
                   Calculated->CruiseStartAlt - Calculated->NavAltitude,
                   0.5);
      }
    }
}


