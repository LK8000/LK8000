/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKSimulator.cpp,v 1.2 2010/12/11 19:34:01 root Exp root $
*/

// #ifdef _SIM_
#include "StdAfx.h"
#include "options.h"
#include "XCSoar.h"
#include "Cpustats.h"
#include "MapWindow.h"
#include "Calculations.h"
#include "Calculations2.h"
#include "Dialogs.h"

#include "Process.h"

#include "Utils.h"
#include "Utils2.h"
#include "Logger.h"
#include "McReady.h"

#include <commctrl.h>
#include <aygshell.h>
#if (WINDOWSPC<1)
#include <sipapi.h>
#endif

#include "Terrain.h"
#include "device.h"

#include "externs.h"
#include "Units.h"

#define ISPARAGLIDER	(AircraftCategory == (AircraftCategory_t)umParaglider)
#define IASMS		CALCULATED_INFO.IndicatedAirspeedEstimated
#define IAS		CALCULATED_INFO.IndicatedAirspeedEstimated*TOKPH
#define BEARING		GPS_INFO.TrackBearing
#define ALTITUDE	GPS_INFO.Altitude
#define GS		GPS_INFO.Speed*TOKPH
#define FLYING		CALCULATED_INFO.Flying
#define THERMALLING	CALCULATED_INFO.Circling
#define MINSPEED	GlidePolar::Vminsink*TOKPH
#define STALLSPEED	GlidePolar::Vminsink*TOKPH*0.85

// WE DONT USE LANDING, CRASHING AND FULL STALL SIMULATION NOW
// #define SIMLANDING	1

//
// LK8000 SS1 = Soaring Simulator V1 by Paolo Ventafridda
// Still basic, but usable
//
void LKSimulator(void) {

  LockFlightData();

  // 
  GPS_INFO.NAVWarning = FALSE;
  GPS_INFO.SatellitesUsed = 6;
  // Even on ground, we can turn the glider in the hangar
  BEARING += SimTurn; 
  if (BEARING<0) BEARING+=360;
  else if (BEARING>359) BEARING-=360;

  #if SIMLANDING
  static bool crashed=false, landedwarn=true;
  #endif
  static bool doinit=true, landing=false, stallwarn=true, circling=false;
  static short counter=0;

  double tdistance, tbearing;
  double thermalstrength=0, sinkstrength=0;

  if (doinit||!CALCULATED_INFO.TerrainValid) {
	if (counter++<3) {
		UnlockFlightData();
		return;
	}
	if (ALTITUDE==0)
		if (CALCULATED_INFO.TerrainValid) ALTITUDE= CALCULATED_INFO.TerrainAlt;
	doinit=false;
  }
 
  // SetBallast is calculating sinkratecache for values starting from 4 to MAXSPEED, in m/s .
  // ONLY during flight, we will sink in the air
  if (FLYING && (IASMS>3) && (IASMS<MAXSPEED) ) {

	double sinkias=-1*(GlidePolar::sinkratecache[(int)IASMS]);
	if (sinkias>10) sinkias=10; // set a limiter for sink rate
	// StartupStore(_T(".... ias=%.0f sinkias=%.3f oldAlt=%.3f newAlt=%.3f\n"), 
	// CALCULATED_INFO.IndicatedAirspeedEstimated*TOKPH, sinkias, GPS_INFO.Altitude, GPS_INFO.Altitude+sinkias);
	double simlift=0;
	if (THERMALLING == TRUE) {
		// entering the thermal mode right now
		if (!circling) {
			circling=true;
			
			DistanceBearing(GPS_INFO.Latitude,GPS_INFO.Longitude,ThLatitude,ThLongitude,&tdistance,&tbearing);
			if (tdistance>1000) {
				// a new thermal
				ThLatitude=GPS_INFO.Latitude; // we mark the new thermal
				ThLongitude=GPS_INFO.Longitude;
				ALTITUDE+=simlift; // sink rate adjusted later
			} else {
				// start circling near the old thermal
			}
		} else {
			// already thermalling
		}
		// ALTITUDE+=simlift+GlidePolar::minsink;
	} else {
		if (circling) {
			// we were circling, now leaving the thermal
			circling=false;
		} else {
			// not circling, already cruising
		}
	}

	// Are we near the thermal?
	DistanceBearing(GPS_INFO.Latitude,GPS_INFO.Longitude,ThLatitude,ThLongitude,&tdistance,&tbearing);
	thermalstrength=4; // m/s
	ThermalRadius=200; // we assume a perfect thermal, a circle of this diameter. Stronger in the center.
	// thermalbase
	sinkstrength=2; //  how intense is the fallout of the thermal
	SinkRadius=150; //  circular ring of the fallout

	if (tdistance>=ThermalRadius && tdistance<(ThermalRadius+SinkRadius) ) {
		// we are in the sinking zone of the thermal..
		simlift= sinkstrength- ((tdistance-ThermalRadius)/SinkRadius)*sinkstrength;
		simlift+=0.1; // adjust rounding errors
		simlift*=-1;
		//StartupStore(_T(".. sinking zone:  dist=%.1f  sink=%.1f\n"), tdistance,simlift);
	}
	if (tdistance<ThermalRadius) {
		// we are in the lift zone
		simlift= thermalstrength- (tdistance/ThermalRadius)*thermalstrength;
		simlift+=0.1; // adjust rounding errors
		//StartupStore(_T(".. climbing zone:  dist=%.1f  climb=%.1f\n"), tdistance,simlift);
	}
	// Update altitude with the lift or sink, 
	ALTITUDE+=simlift;
	// Update the new altitude with the natural sink, but not going lower than 0
	ALTITUDE-=(sinkias+0.1); // rounding errors require a correction
	if (ALTITUDE<=0) ALTITUDE=0;

	#if SIMLANDING
	if (CALCULATED_INFO.TerrainValid && (CALCULATED_INFO.AltitudeAGL <=20) ) {
		if (IAS <= (MINSPEED+3)) landing=true;
		else {
			// we dont simulate crashing. LK8000 pilots never crash. 
			crashed=true;
		}
	} 
	if (CALCULATED_INFO.TerrainValid && (CALCULATED_INFO.AltitudeAGL >100) ) {
		landing=false;
	}

	if (!landing && CALCULATED_INFO.TerrainValid && (CALCULATED_INFO.AltitudeAGL <=0) ) {
		GPS_INFO.Speed=0;
		landing=true;
		if (landedwarn) {
			DoStatusMessage(_T("YOU HAVE LANDED"));
			landedwarn=false;
		}
	} else landedwarn=true;
	#endif
		
  } 

  if (FLYING) {
	// simple stall at 1 G
	if (!landing && (IAS<=STALLSPEED && IASMS>3)) {
		if (stallwarn) {
			DoStatusMessage(_T("STALLING"));
			stallwarn=false;
		}
		#if 0 // DO NOT SIMULATE STALLING NOW
		// GPS_INFO.Speed= (GlidePolar::Vminsink*0.85)+1;
		ALTITUDE-=20;
		if (ALTITUDE<=0) ALTITUDE=0;
		#endif
	} else stallwarn=true;
	#if SIMLANDING
	if (landing || IASMS<4) {
		GPS_INFO.Speed-=GPS_INFO.Speed*0.2;
	}
	if (crashed) {
		GPS_INFO.Speed=0;
	}
	#endif

	if (GS<0) {
		GPS_INFO.Speed=0;
	}
  }


  FindLatitudeLongitude(GPS_INFO.Latitude, GPS_INFO.Longitude, 
                          GPS_INFO.TrackBearing, GPS_INFO.Speed*1.0,
                          &GPS_INFO.Latitude,
                          &GPS_INFO.Longitude);
  GPS_INFO.Time+= 1.0;
  long tsec = (long)GPS_INFO.Time;
  GPS_INFO.Hour = tsec/3600;
  GPS_INFO.Minute = (tsec-GPS_INFO.Hour*3600)/60;
  GPS_INFO.Second = (tsec-GPS_INFO.Hour*3600-GPS_INFO.Minute*60);

  UnlockFlightData();
  }

// #endif
