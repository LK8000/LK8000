/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: WindKalman,v 1.1 2013/11/04 19:28:45 root Exp root $
*/


#include "externs.h"
#include "WindEKF.h"

#define BASIC_FILTER 1  // minimal filtering
#define SCAN_RANGE   1  // scan track range

#if TESTBENCH
// #define KALMAN_DEBUG 1
#endif

#define BLACKOUT_TIME  3.0
WindEKF EKWIND;

static unsigned int kalman_samples=0;
static double old_gs=0, old_tas=0, old_tb=361, old_time=0;

#if SCAN_RANGE
static void InitScanRange(void);
#if KALMAN_DEBUG
static void PrintScanRange(void);
#endif
static void InsertScanRange(double angle);
static unsigned short QualityScanRange(void);
#endif

static void WindKalmanReset(const bool reset) {
  static bool alreadydone=false;

  // we are using the filter, so in case of "true" reset, we should really do it.
  // This is called just to say the matrix is dirty being in use.
  if (reset==false) {
	alreadydone=false;
	return;
  }
  // Else we are asking for a reset with "true", but we do it only if data is dirty
  if (!alreadydone) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... *** WindKalman RESET DONE ***%s"),NEWLINE);
	#endif
	EKWIND.Init();
	alreadydone=true;
  	kalman_samples=0;
	old_gs=0; old_tas=0; old_tb=361;
	if (old_time==0) old_time=1; // no more first reset needed
        #if SCAN_RANGE
        InitScanRange();
        #endif
  }
  return;
}


unsigned WindKalmanUpdate(NMEA_INFO *basic, DERIVED_INFO *derived,  double *windspeed, double *windbearing )
{
static double kalman_holdoff_time =0.0;
static bool airspeedreset=true;

  //
  // CASES where we reset the matrix and we are not willing to fill it up 
  //

  // Normally we shall not select ZIGZAG if we dont have airspeed!!
  // We reset only one time, and it should not be necessary also.
  if (! basic->TrueAirSpeed.available()) {
	if (!airspeedreset) return 0;
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... No Airspeed available%s"),NEWLINE);
	#endif
	WindKalmanReset(true);
  	kalman_holdoff_time=0;
	airspeedreset=false;
	return 0;
  }

  if (!derived->Flying ) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Reset Kalman Wind (not flying)%s"),NEWLINE);
	#endif
	WindKalmanReset(true);
  	kalman_holdoff_time=0;
	return 0;
  }


  #if 0 // TODO
  // TODO  use timewarp check, and do navwarning reset only if we have a very long blackout.
  if (basic->NAVWarning) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Reset Kalman by NavWarning %s"),NEWLINE);
	#endif
	WindKalmanReset(true);
  	kalman_holdoff_time=0;
	return 0;
  }
  #endif

  #if 0 // TODO
  // TODO> apply this only if we gained significant altitude 
  // while circling we reset the matrix, because the wind is different at a different altitude
  if (derived->Circling) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Reset Kalman by circling %s"),NEWLINE);
	#endif
	WindKalmanReset(true);
  	kalman_holdoff_time=0;
	return 0;
  }
  #endif

  //
  // CASES where we reset the matrix and we may still fill it up
  //
  if (old_time>1) {
	if ((basic->Time-old_time)>1800) {
		#ifdef KALMAN_DEBUG
		StartupStore(_T(".... Reset Kalman Wind (time passed 30 minutes since last insert)%s"),NEWLINE);
		#endif
		WindKalmanReset(true);
  		kalman_holdoff_time=0;
		old_time=basic->Time; // we do it here after a reset
	}
  } else {
	if (old_time==0) {
		// This is the very first run and we must init the matrix
		// If we don't do this, the windEKF SerialUpdate will miserably fail.
		#ifdef KALMAN_DEBUG
		StartupStore(_T(".... Reset Kalman Wind (FIRST RUN)%s"),NEWLINE);
		#endif
		WindKalmanReset(true);
  		kalman_holdoff_time=0;
		old_time=1; // a flag to remember
	}
  }




  //
  // CASES where we do not reset the matrix, but we dont fill it up either
  //

  // In tight turning do not sample wind, set it on hold
  // Lets not confuse circling mode and turning state. We can still go straight while 
  // in Circling mode state.
  if ((fabs(derived->TurnRate) > 18.0))
  {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Turning%s"),NEWLINE);
	#endif
	kalman_holdoff_time = basic->Time + BLACKOUT_TIME;
	return (0);
  }

  // After a tight turn skip sampling and wait some 3 more seconds to settle
  if (basic->Time <  kalman_holdoff_time)
  {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Holdoff%s"),NEWLINE);
	#endif
	return (0);
  }

  #if BASIC_FILTER
  // below 10kmh we will not assume the gps bearing is still correct!
  // do not reset ring, simply discard data
  if (basic->Speed < 3 || basic->TrueAirSpeed.value() < 3) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... speed too low!%s"),NEWLINE);
	#endif
	return (0);
  }

  // If nothing has changed in the last second, we assume we are dealing with a repetition
  // Note: we only check gps data because it is consistent in track and gs. 
  // It should never be exactly the same, being floating values. If they are, then it is 
  // the same gps fix! And we don't consider the TAS, in any case it is either a repetition or
  // an error. Most likely a repetition.
  if (old_tb==basic->TrackBearing && old_gs==basic->Speed ) {

	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... GPS FIX UNCHANGED, old TAS=%f new TAS=%f discarded%s"),
	   old_tas, basic->TrueAirspeed, NEWLINE);
	#endif
	return (0);
  }

  // If the tas is exactly the same, it is either a repetition or an error.
  // Notice that we must check the TAS is provided in m/s, because for example Zander is 
  // providing kmh, and accuracy is not granted. But in this case we would insert the same tas
  // for a different bearing, which is quite inaccurate too.
  if (old_tas==basic->TrueAirSpeed.value()) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... TAS UNCHANGED, discard\n"));
	#endif
	return (0);
  }
  #endif


  //
  // OK PROCEED > Matrix becomes dirty
  // 

  WindKalmanReset(false);
  kalman_holdoff_time =0;

  double V = basic->TrueAirSpeed.value();
  double dynamic_pressure = (V*V);
  float gps_vel[2];

  double VG = basic->Speed;;
  gps_vel[0] = (float)(  fastsine( basic->TrackBearing ) * VG);
  gps_vel[1] = (float)(fastcosine( basic->TrackBearing ) * VG);

  float dT = 1.0;

  old_tas=V;
  old_gs=VG;
  old_tb=basic->TrackBearing;
  old_time=basic->Time;

  #if SCAN_RANGE
  InsertScanRange(old_tb);
  #endif

  #ifdef KALMAN_DEBUG
  StartupStore(_T("... KALMAN insert gps speed=%f m/s (%f kmh) track=%f TAS=%f  (gps_vel[0]=%f gps_vel[1]=%f dyn=%f)\n"),
      basic->Speed, basic->Speed*3.6, basic->TrackBearing, basic->TrueAirSpeed.value(),
      gps_vel[0], gps_vel[1], dynamic_pressure);
  #if SCAN_RANGE
  PrintScanRange();
  StartupStore(_T(".... QUALITY=%d\n"),QualityScanRange());
  #endif
  #endif

  EKWIND.StatePrediction(gps_vel, dT);
  EKWIND.Correction(dynamic_pressure, gps_vel);

  #if BASIC_FILTER
  static unsigned validHBtime=0;
  // we are using the internal beats counter, not the gps time
  if (derived->Circling) {
	validHBtime=LKHearthBeats+20; // 2hz,  means 10 seconds
  }
  #else
  /* prevent value update while circling */
  // this is simply delaying 10 seconds the usage of ekf wind after a thermal 
  // but we are loosing the real counter! this is not good
  if (derived->Circling)
         kalman_samples = 0;
  #endif

  ++kalman_samples;

  #if BASIC_FILTER
  #if SCAN_RANGE
  #define REQUIRED_SAMPLES 400
  #define REQUIRED_QUALITY 46
  // The ZigZag wind is called like that because a pilot is supposed to collect direction samples.
  // Quality is about checking if he did so. With no quality, no wind.

  bool ok=false;
  // at 800 samples after approx. 15 minutes we give up and accept a weak guess
  // quality 12 is really borderline for a real guess.
  // The error is greater if wind is stronger of course. With weak winds who cares!
  if ( kalman_samples>1000 ) ok=true; 
  if ( kalman_samples> 800 && QualityScanRange() >= 12) ok=true; 
  if ( kalman_samples> 600 && QualityScanRange() >= 24) ok=true;
  if ( kalman_samples> 400 && QualityScanRange() >= 34) ok=true;
  if ( kalman_samples> 300 && QualityScanRange() >= 40) ok=true;
  if ( kalman_samples> 200 && QualityScanRange() >= 46) ok=true;
  if (!ok) {
  #else
  #define REQUIRED_SAMPLES 60
  // Wait to have at least REQUIRED_SAMPLES valid samples in order to use the new wind
  // (must be a %10 value so that next will be used at once)
  if (kalman_samples< REQUIRED_SAMPLES) {
  #endif

	// --- debug ---
	#ifdef KALMAN_DEBUG
  	const float* x = EKWIND.get_state();
  	double speed   = sqrt((x[0]*x[0])+(x[1]*x[1]));
  	double bearing = atan2(-x[0], -x[1])*RAD_TO_DEG;
  	bearing =AngleLimit360(bearing);
  	StartupStore(_T(".... (WAITING, NOT USING) Kalman Filter Wind %4.1fkm/h %4.1fdeg  kalman_samples=%d%s"),speed*3.6,bearing,kalman_samples,NEWLINE);
        #if SCAN_RANGE
        StartupStore(_T(".... (QUALITY=%d  SAMPLES=%d\n"),QualityScanRange(),kalman_samples);
        #endif
	if (ExternalWindAvailable(*Basic)) {
		StartupStore(_T(".... (EXTERNAL WIND: %4.1fkmh  %4.1fdeg\n"),
		  basic->ExternalWindSpeed, basic->ExternalWindDirection);
	}
  	StartupStore(_T("%s"),NEWLINE);
	#endif
	// --- debug ---

	return 0; 
  }

  if (LKHearthBeats<validHBtime) {

	// --- debug ---
	#ifdef KALMAN_DEBUG
	if (derived->Circling)
		StartupStore(_T(".... (WAITING while circling, not updating the wind\n"));
	else
		StartupStore(_T(".... (WAITING thermal timeout time, %u seconds to go\n"),(validHBtime-LKHearthBeats)/2);
  	const float* x = EKWIND.get_state();
  	double speed   = sqrt((x[0]*x[0])+(x[1]*x[1]));
  	double bearing = atan2(-x[0], -x[1])*RAD_TO_DEG;
  	bearing =AngleLimit360(bearing);
  	StartupStore(_T(".... (NOT USING) Kalman Filter Wind %4.1fkm/h %4.1fgrad  kalman_samples=%d%s"),speed*3.6,bearing,kalman_samples,NEWLINE);
        #if SCAN_RANGE
        StartupStore(_T(".... (QUALITY=%d\n"),QualityScanRange());
        #endif
	if (ExternalWindAvailable(*Basic)) {
		StartupStore(_T(".... (EXTERNAL WIND: %4.1fkmh  %4.1fdeg\n"),
		  basic->ExternalWindSpeed, basic->ExternalWindDirection);
	}
  	StartupStore(_T("%s"),NEWLINE);
	#endif
	// --- debug ---

	return 0;
  }
  #endif

  /* update values every 10 values only */
  if (kalman_samples%10!=0)
    return 0;


  const float* x = EKWIND.get_state();
  double speed   = sqrt((x[0]*x[0])+(x[1]*x[1]));
  double bearing = atan2(-x[0], -x[1])*RAD_TO_DEG;

  bearing =AngleLimit360(bearing);

#ifdef KALMAN_DEBUG
  StartupStore(_T(".... (SETWIND!) Kalman Filter Wind %4.1fkm/h %4.1f deg  kalman_samples=%d%s"),speed*3.6,bearing,kalman_samples,NEWLINE);
  #if SCAN_RANGE
  StartupStore(_T(".... (QUALITY=%d SAMPLES=%d\n"),QualityScanRange(),kalman_samples);
  #endif
  if (ExternalWindAvailable(*Basic)) {
	StartupStore(_T(".... (EXTERNAL WIND: %4.1fkmh  %4.1fdeg\n"),
	  basic->ExternalWindSpeed, basic->ExternalWindDirection);
  }
#endif

#if BASIC_FILTER
  // Anything filtered here will require 10 more samples for next try
  if (speed>41) { // 150kmh
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... (SETWIND) SPEED TOO HIGH, DISCARDED\n"));
	#endif
	return 0;
  }
#endif

  #ifdef KALMAN_DEBUG
  StartupStore(_T("%s"),NEWLINE);
  #endif

  (*windbearing) = bearing;
  (*windspeed)   = speed;

  return 1; 
}

#if SCAN_RANGE

static bool scanrange[32];

static void InitScanRange(void) {
    #ifdef KALMAN_DEBUG
    StartupStore(_T(".... INIT SCAN RANGE\n"));
    #endif
    for (unsigned short i=0; i<32; i++) scanrange[i]=false;
}

static void InsertScanRange(double angle) {

    unsigned short direction;

    if (angle<0) angle+=360;
    if (angle>=360) angle-=360;
    direction= (unsigned short)(angle/11.25);
    LKASSERT(direction<32);

    scanrange[direction]=true;
}

#if KALMAN_DEBUG
static void PrintScanRange(void) {
    TCHAR dstring[200];

    wcscpy(dstring,_T(""));
    for (unsigned short i=0; i<16; i++)  {
#error "dstring can't be src and dst "
        _stprintf(dstring,_T("%s %d"),dstring,scanrange[i]);
    }
    StartupStore(_T("%s\n"),dstring);

    wcscpy(dstring,_T(""));
    for (unsigned short i=16; i<32; i++) {
#error "dstring can't be src and dst "
	  _stprintf(dstring,_T("%s %d"),dstring,scanrange[i]);
    }
    StartupStore(_T("%s\n"),dstring);
}
#endif

// Return a quality parameter 0 to 48
static unsigned short QualityScanRange(void) {

    unsigned short qual=0;
    for (unsigned short i=0; i<32; i++) {
        qual+= scanrange[i];
    }

    for (unsigned short i=0; i<16; i++) {
        if ( scanrange[i] && scanrange[i+16] ) qual++;
    }

    return qual;

}

#endif
