/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: DoTarget.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "McReady.h"
#include "NavFunctions.h"



// Running every n seconds ONLY when the Target Infopage is active ( we are not drawing map).
// We DONT use FLARM_Traffic after we copy inside LKTraffic!!
// Returns true if did calculations, false if ok to use old values
//
// 121214 no need to lock, this is using local draw copy
#define LKTARG LKTraffic[LKTargetIndex]
bool DoTarget(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
   double x0, y0, etas, ttgo=0;
   double bearing, distance;

   if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) return false;

   // DoTarget is called from MapWindow, in real time. We have enough CPU power there now

   #if 0
   if (  LastDoTarget > Basic->Time ) LastDoTarget=Basic->Time;
   // We calculate in real time, because PFLAA sentences are calculated too in real time
   if ( Basic->Time < (LastDoTarget+3.0) ) { 
	return false;
   }
   LastDoTarget=Basic->Time;
   #endif

   #ifdef DEBUG_LKT
   StartupStore(_T("... DoTarget Copy LKTraffic\n"));
   #endif
   //LockFlightData();
   memcpy(LKTraffic, Basic->FLARM_Traffic, sizeof(LKTraffic));
   //UnlockFlightData();

   if (LKTARG.RadioId <= 0) return false;

   DistanceBearing(Basic->Latitude, Basic->Longitude, 
		LKTARG.Latitude, LKTARG.Longitude,
		&distance, &bearing);
   LKTARG.Distance=distance;
   LKTARG.Bearing=bearing;

   if (LKTARG.Speed>1) {
	x0 = fastsine(LKTARG.TrackBearing)*LKTARG.Speed;
	y0 = fastcosine(LKTARG.TrackBearing)*LKTARG.Speed;
	x0 += fastsine(Calculated->WindBearing)*Calculated->WindSpeed;
	y0 += fastcosine(Calculated->WindBearing)*Calculated->WindSpeed;
	// LKTARG.Heading = AngleLimit360(atan2(x0,y0)*RAD_TLK_DEG); // 101210 check
	etas = isqrt4((unsigned long)(x0*x0*100+y0*y0*100))/10.0;
	LKTARG.EIAS = IndicatedAirSpeed(etas, LKTARG.Altitude);
   } else {
	LKTARG.EIAS=0;
   }


   //double height_above_target = Calculated->NavAltitude - LKTARG.Altitude;

   // We DONT use EnergyHeight here because we are not considering the Target's TE either
   LKTARG.AltArriv = Calculated->NavAltitude - GlidePolar::MacCreadyAltitude(MACCREADY,
	distance,
	bearing,
	Calculated->WindSpeed, Calculated->WindBearing,
	0, 0,
	// final glide, use wind
	true, 
	&ttgo) - LKTARG.Altitude;
	
   // We CANNOT use RelativeAltitude because when ghost or zombie, it wont be updated in real time in respect
   // to OUR real position!! Lets use the last target altitude known.
   double relalt=Calculated->NavAltitude - LKTARG.Altitude;
   if (relalt==0)
	LKTARG.GR=999;
   else {
	// we need thus to invert the relative altitude
	LKTARG.GR=distance/(relalt);
   }


   return true;
}


