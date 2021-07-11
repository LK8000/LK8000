/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: AddSnailPoint.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"



//
// This function is called by DoLogging only
//
// Note that Visibility is updated by DrawThread in ScanVisibility, inside MapWindow_Utils
//
// Update v5: we calculate here the Colour of the trail, and not in LKDrawTrail.
// We also do not need any more to save the float Vario for each point.
//
void AddSnailPoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  // In CAR mode, we call this function when at least 5m were made in 5 seconds
  // We add points only AFTER start of flight/trip
  // Interval is variable, for gliders is 1s in thermal, 5s in cruise.
  if (!Calculated->Flying) return;

  static long lastLongSnailTime=0;
  static bool wascircling=false;

  // Check if we are overwriting an old value with a new one, and add the old one
  // to the longsnailtrail, only if we still have space inside it. No rollover!
  if (SnailTrail[SnailNext].Time>0 && LongSnailNext<LONGTRAILSIZE) {

      if (SnailTrail[SnailNext].Time > (lastLongSnailTime+60)) {

          // log only once for each thermal, roughly
          // The idea is to make the code readable, not for a beauty contest
          // After 10 minutes in thermal, log anyway.
          if (SnailTrail[SnailNext].Circling && wascircling) {
              if (SnailTrail[SnailNext].Time < (lastLongSnailTime+600)) goto _skipout;
          }
          if (SnailTrail[SnailNext].Circling && !wascircling) wascircling=true;
          if (!SnailTrail[SnailNext].Circling) wascircling=false;

          LongSnailTrail[LongSnailNext].Latitude = SnailTrail[SnailNext].Latitude;
          LongSnailTrail[LongSnailNext].Longitude = SnailTrail[SnailNext].Longitude;
          LongSnailTrail[LongSnailNext].FarVisible = true; // hasn't been filtered out yet.

          // find the next
          int snext = SnailNext+2;
          snext %= TRAILSIZE;
          // no mistake: longsnailnext+1 is a valid position
          LongSnailTrail[LongSnailNext+1].Latitude = SnailTrail[snext].Latitude;
          LongSnailTrail[LongSnailNext+1].Longitude = SnailTrail[snext].Longitude;
          LongSnailTrail[LongSnailNext+1].FarVisible = true;

	  lastLongSnailTime=SnailTrail[SnailNext].Time;

          // LongSnailTrail is sized LONGTRAILSIZE+1, we use last position for the closing point
          // so we have LongSnailNext=LONGTRAILSIZE mean invalid
          if (++LongSnailNext > LONGTRAILSIZE) LongSnailNext=LONGTRAILSIZE;
      }
  }
_skipout:

  SnailTrail[SnailNext].Latitude = (float)(Basic->Latitude);
  SnailTrail[SnailNext].Longitude = (float)(Basic->Longitude);
  SnailTrail[SnailNext].Time = Basic->Time;
  if (Calculated->TerrainValid) {
	double hr = max(0.0, Calculated->AltitudeAGL)/100.0;
	SnailTrail[SnailNext].DriftFactor = 2.0/(1.0+exp(-hr))-1.0;
  } else {
	SnailTrail[SnailNext].DriftFactor = 1.0;
  }

  // paragliders and car/trekking use Vario and not NettoVario
  float tvario;
  if (ISPARAGLIDER || ISCAR)
	tvario = (float)(Calculated->Vario) ;
  else
	tvario = (float)(Calculated->NettoVario) ;

  float offval=1.0;
  int usecol;

  if (tvario<0) offval=-1;
  const float useval=fabs(tvario);

  if (ISCAR) {
	// vario values for CAR mode are 5 times more sensibles
	if (useval <=0.1 ) {; usecol=1; goto go_setcolor; }
	if (useval <=0.2 ) {; usecol=2; goto go_setcolor; }
	if (useval <=0.3 ) {; usecol=3; goto go_setcolor; }
	if (useval <=0.4 ) {; usecol=4; goto go_setcolor; }
	if (useval <=0.6 ) {; usecol=5; goto go_setcolor; }
	if (useval <=0.8 ) {; usecol=6; goto go_setcolor; }
	usecol=7; // 7th : 1ms and up
	goto go_setcolor;
  }

  // Normal NON-CAR mode
  if ( useval <0.1 ) {
	usecol=0;
	goto go_setcolor;
  }
  if (useval <=0.5 ) {; usecol=1; goto go_setcolor; }
  if (useval <=1.0 ) {; usecol=2; goto go_setcolor; }
  if (useval <=1.5 ) {; usecol=3; goto go_setcolor; }
  if (useval <=2.0 ) {; usecol=4; goto go_setcolor; }
  if (useval <=3.0 ) {; usecol=5; goto go_setcolor; }
  if (useval <=4.0 ) {; usecol=6; goto go_setcolor; }
  usecol=7; // 7th : 4ms and up

go_setcolor:
    SnailTrail[SnailNext].Colour = 7+(short int)(usecol*offval);
    SnailTrail[SnailNext].Circling = Calculated->Circling;

    SnailNext ++;
    SnailNext %= TRAILSIZE;

  }
