/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: AddSnailPoint.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"



//
// This function is called by DoLogging only
//
// Note that Visibility is updated by DrawThread in ScanVisibility, inside MapWindow_Utils
//
void AddSnailPoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  // In CAR mode, we call this function when at least 5m were made in 5 seconds
  // We add points every second AFTER start of flight/trip
  // The interpolator need (apparently) a fix a second right now, we should make it dynamical.
  if (!Calculated->Flying) return;

  SnailTrail[SnailNext].Latitude = (float)(Basic->Latitude);
  SnailTrail[SnailNext].Longitude = (float)(Basic->Longitude);
  SnailTrail[SnailNext].Time = Basic->Time;
  SnailTrail[SnailNext].FarVisible = true; // hasn't been filtered out yet.
  if (Calculated->TerrainValid) {
	double hr = max(0.0, Calculated->AltitudeAGL)/100.0;
	SnailTrail[SnailNext].DriftFactor = 2.0/(1.0+exp(-hr))-1.0;
  } else {
	SnailTrail[SnailNext].DriftFactor = 1.0;
  }

#if 0
  if (Calculated->Circling) {
	SnailTrail[SnailNext].Vario = (float)(Calculated->NettoVario) ;
  } else {
	SnailTrail[SnailNext].Vario = (float)(Calculated->NettoVario) ;
  }
#else
  // paragliders and car/trekking use Vario and not NettoVario
  if (ISPARAGLIDER || ISCAR)
	SnailTrail[SnailNext].Vario = (float)(Calculated->Vario) ;
  else
	SnailTrail[SnailNext].Vario = (float)(Calculated->NettoVario) ;
#endif

  SnailTrail[SnailNext].Colour = -1; // need to have colour calculated
  SnailTrail[SnailNext].Circling = Calculated->Circling;

  SnailNext ++;
  SnailNext %= TRAILSIZE;

}

