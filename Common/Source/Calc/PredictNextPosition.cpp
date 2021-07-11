/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

extern int WarningTime;

void PredictNextPosition(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if(Calculated->Circling)
    {
      Calculated->NextLatitude = Basic->Latitude;
      Calculated->NextLongitude = Basic->Longitude;
      Calculated->NextAltitude = 
        Calculated->NavAltitude + Calculated->Average30s * WarningTime;
    }
  else
    {
      FindLatitudeLongitude(Basic->Latitude, 
                            Basic->Longitude, 
                            Basic->TrackBearing, 
                            Basic->Speed*WarningTime,
                            &Calculated->NextLatitude,
                            &Calculated->NextLongitude);

       Calculated->NextAltitude = Calculated->NavAltitude + Calculated->Average30s * WarningTime;
    }
    // We are assuming that terrain altitude will not change much, not accurate though.
    // This is used by airspace engine.
    Calculated->NextAltitudeAGL = Calculated->NextAltitude - Calculated->TerrainAlt;

}



