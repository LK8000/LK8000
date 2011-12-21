/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: AddSnailPoint.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"

using std::min;
using std::max;



//
// This function is called by DoLogging only
//
void AddSnailPoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
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
  // paragliders use Vario and not NettoVario
  if ( AircraftCategory != (AircraftCategory_t)umParaglider ) 
	SnailTrail[SnailNext].Vario = (float)(Calculated->NettoVario) ;
  else
	SnailTrail[SnailNext].Vario = (float)(Calculated->Vario) ;
#endif

  SnailTrail[SnailNext].Colour = -1; // need to have colour calculated
  SnailTrail[SnailNext].Circling = Calculated->Circling;

  SnailNext ++;
  SnailNext %= TRAILSIZE;

}

