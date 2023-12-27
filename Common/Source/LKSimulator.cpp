/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: LKSimulator.cpp,v 1.2 2010/12/11 19:34:01 root Exp root $
*/

#include "externs.h"
#include "Calculations2.h"
#include "LKMapWindow.h"

#include "McReady.h"
#include "Asset.hpp"
#include "NavFunctions.h"
#include "Calc/Vario.h"
#include "Tracking/Tracking.h"


#define IASMS		CALCULATED_INFO.IndicatedAirspeedEstimated
#define IAS		Units::To(Units_t::unKiloMeterPerHour, CALCULATED_INFO.IndicatedAirspeedEstimated)
#define BEARING		GPS_INFO.TrackBearing
#define ALTITUDE	GPS_INFO.Altitude
#define GS		Units::To(Units_t::unKiloMeterPerHour, GPS_INFO.Speed)
#define FLYING		CALCULATED_INFO.Flying
#define THERMALLING	CALCULATED_INFO.Circling

//#define MINSPEED	Units::To(Units_t::unKiloMeterPerHour, GlidePolar::Vminsink())
#define STALLSPEED	Units::To(Units_t::unKiloMeterPerHour, GlidePolar::Vminsink())*0.6

extern void SimFlarmTraffic(uint32_t RadioId, double offset);

// WE DONT USE LANDING, CRASHING AND FULL STALL SIMULATION NOW
// #define SIMLANDING	1

//
// LK8000 SS1 = Soaring Simulator V1 by Paolo Ventafridda
// Still basic, but usable
//
void LKSimulator(void) {

  LockFlightData();

  //
  GPS_INFO.NAVWarning = false;
  GPS_INFO.SatellitesUsed = 6;
  ResetVarioAvailable(GPS_INFO);

  // Even on ground, we can turn the glider in the hangar
  BEARING += SimTurn;
  if (BEARING<0) BEARING+=360;
  else if (BEARING>359) BEARING-=360;

  #if SIMLANDING
  static bool crashed=false, landedwarn=true;
  #endif
  static bool doinit=true, landing=false, stallwarn=true, circling=false, flarmwasinit=false;
  static short counter=0;

  double tdistance, tbearing;
  double thermalstrength=0, sinkstrength=0;

    if (doinit) {
	if (counter++<4) {
		UnlockFlightData();
		return;
	}
	#if TESTBENCH
	StartupStore(_T(". SIMULATOR: real init%s"),NEWLINE);
	#endif

	// Add a couple of thermals for the boys
	InsertThermalHistory(GPS_INFO.Time-1887, GPS_INFO.Latitude-0.21, GPS_INFO.Longitude+0.13, 873, 1478,1.5);
	InsertThermalHistory(GPS_INFO.Time-1250, GPS_INFO.Latitude+0.15, GPS_INFO.Longitude-0.19, 991, 1622,0.9);
	InsertThermalHistory(GPS_INFO.Time-987, GPS_INFO.Latitude-0.11, GPS_INFO.Longitude+0.13, 762, 1367,1.8);
	InsertThermalHistory(GPS_INFO.Time-100, GPS_INFO.Latitude-0.02, GPS_INFO.Longitude-0.03, 650, 1542,2.2);
	WayPointList[RESWP_LASTTHERMAL].Latitude  = GPS_INFO.Latitude-0.022;
	WayPointList[RESWP_LASTTHERMAL].Longitude = GPS_INFO.Longitude-0.033;
	WayPointList[RESWP_LASTTHERMAL].Altitude  = 650;
	ThLatitude=GPS_INFO.Latitude-0.022;
	ThLongitude=GPS_INFO.Longitude-0.033;

	if (EnableFLARMMap && !tracking::radar_config ) {
		srand(MonotonicClockMS());
		SimFlarmTraffic(0xdd8951,22.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8944,31.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a43,16.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a42,41.0+(double)(rand() % 32));
/*
		SimFlarmTraffic(0xdd8a11,11.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a12,21.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a13,31.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a14,41.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a15,51.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a16,61.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a17,71.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a18,81.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a19,91.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a10,01.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a21,21.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a22,22.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a23,23.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a24,24.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a25,25.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a26,26.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a27,27.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a28,28.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a29,29.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a20,31.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a31,32.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a32,33.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a33,34.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a34,35.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a35,36.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a36,37.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a37,41.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a38,42.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a39,43.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a30,44.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a41,45.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a42,46.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a43,47.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a44,48.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a45,49.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a46,01.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a47,02.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a48,03.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a49,04.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a50,81.1+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a51,82.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a52,83.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a53,84.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a54,85.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a55,86.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a56,87.0+(double)(rand() % 32));
*/
	}
	doinit=false;
  }

  // First Aircraft min altitude is at ground level
  if (ALTITUDE==0)
	if (CALCULATED_INFO.TerrainValid) ALTITUDE= CALCULATED_INFO.TerrainAlt;


  if (ISGAAIRCRAFT) {
	// todo: fuel consumption, engine efficiency etc.
  }

  // We cannot use doinit for flarm, because it could be enabled from configuration AFTER startup,
  // and it must work all the way the same in order not to confuse users.
  if (EnableFLARMMap && !tracking::radar_config ) {
	if (!flarmwasinit) {
		srand(MonotonicClockMS());
		// Add a poker of traffic for the boys
		SimFlarmTraffic(0xdd8951,22.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8944,31.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a43,16.0+(double)(rand() % 32));
		SimFlarmTraffic(0xdd8a42,41.0+(double)(rand() % 32));
		DoStatusMessage(MsgToken<279>()); // FLARM DETECTED (in sim)
		flarmwasinit=true;
	} else {
		// Let one of the objects be a ghost and a zombie, and keep the rest real
		SimFlarmTraffic(0xdd8951,0);
		SimFlarmTraffic(0xdd8944,0);
		SimFlarmTraffic(0xdd8a43,0);
	}
  }

  if (ISPARAGLIDER || ISGLIDER) {

    // SetBallast is calculating sinkratecache for values starting from 4 to MAXSPEED, in m/s .
    // ONLY during flight, we will sink in the air
    if (FLYING && (IASMS>3) && (IASMS<MAXSPEED) ) {

	double sinkias=-1*AirDensitySinkRate(IASMS, GPS_INFO.Altitude);
	if (sinkias>10) sinkias=10; // set a limiter for sink rate
	// StartupStore(_T(".... ias=%.1f sinkias=%.3f oldAlt=%.3f newAlt=%.3f\n"),
	// CALCULATED_INFO.IndicatedAirspeedEstimated*TOKPH, sinkias, GPS_INFO.Altitude, GPS_INFO.Altitude-sinkias);
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
        sinkias -= SimNettoVario;
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
	ALTITUDE-=sinkias;
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
  } // Glider/Paragliders

  if (FLYING) {
	// simple stall at 1 G
	if (!landing && (IAS<=STALLSPEED && IASMS>3)) {
		if (stallwarn) {
			// DoStatusMessage(_T("STALLING")); // OK, people do not like stalling.
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


  if (GS>0) {
      FindLatitudeLongitude(GPS_INFO.Latitude, GPS_INFO.Longitude,
                          GPS_INFO.TrackBearing, GPS_INFO.Speed,
                          &GPS_INFO.Latitude,
                          &GPS_INFO.Longitude);
  }
  GPS_INFO.Time+= 1.0;
  long tsec = (long)GPS_INFO.Time;
  GPS_INFO.Hour = tsec/3600;
  GPS_INFO.Minute = (tsec-GPS_INFO.Hour*3600)/60;
  GPS_INFO.Second = (tsec-GPS_INFO.Hour*3600-GPS_INFO.Minute*60);

  UnlockFlightData();
}


void SimFastForward() {
    if(HasKeyboard()) {
        double gs=GPS_INFO.Speed*10.0;
        if (gs<100) gs=100;
        FindLatitudeLongitude(GPS_INFO.Latitude, GPS_INFO.Longitude,
                          GPS_INFO.TrackBearing, gs,
                          &GPS_INFO.Latitude,
                          &GPS_INFO.Longitude);
    }
}
