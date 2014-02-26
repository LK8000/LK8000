/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: WindKalman,v 1.1 2013/11/04 19:28:45 root Exp root $
*/


#include "externs.h"
#include "WindEKF.h"

#define MINIMAL_FILTER 1  // more filtering, 
//#define USE_RING 1	  // DO NOT USE: ring buffering for historic filtering, todo 

#if TESTBENCH
 //#define KALMAN_DEBUG
#endif

#define BLACKOUT_TIME  3.0
WindEKF EKWIND;

static unsigned int kalman_samples=0;

#if USE_RING
static double ring_trackbearing[4]={0,0,0,0};
static double ring_groundspeed[4]={0,0,0,0};
static double ring_trueairspeed[4]={0,0,0,0};
static double ring_brg=0, ring_gs=0, ring_tas=0;
#endif

unsigned CalcQualityLevel(unsigned i)
{
  if (i > 400) return 4;
  if (i > 100) return 3;
  if (i >  30) return 2;
return 1;
}

#if USE_RING
static void InsertRing(double brg, double gs, double tas) {

  #ifdef KALMAN_DEBUG
  StartupStore(_T(".... InsertRing [sample=%d] brg=%f gs=%f tas=%f%s"),kalman_samples,brg,gs,tas,NEWLINE);
  #endif

  ring_trackbearing[3]=ring_trackbearing[2];
  ring_trackbearing[2]=ring_trackbearing[1];
  ring_trackbearing[1]=ring_trackbearing[0];
  ring_trackbearing[0]=brg;

  ring_groundspeed[3]=ring_groundspeed[2];
  ring_groundspeed[2]=ring_groundspeed[1];
  ring_groundspeed[1]=ring_groundspeed[0];
  ring_groundspeed[0]=gs;

  ring_trueairspeed[3]=ring_trueairspeed[2];
  ring_trueairspeed[2]=ring_trueairspeed[1];
  ring_trueairspeed[1]=ring_trueairspeed[0];
  ring_trueairspeed[0]=tas;


}

static bool GetRing(void) {

// TODO: try to filter inconsistent values from external sensors.
// typically due to big latencies that will show same bearing, same groundspeed
// but different TAS, and the second later same TAS (exactly the same) with different
// bearing and groundspeed. This is not good, but there is little we can do about it
// if the instrument is working like that. We can expect in such cases a slightly wrong 
// wind speed and a quite wrong wind direction (error in the range of 30 to 90 degrees).

  // THESE DO NOT WORK because we are going to compare new values with old values
  // that really never got inside the matrix
  //
  if (ring_trueairspeed[0]==ring_trueairspeed[1]) {
    	#ifdef KALMAN_DEBUG
    	StartupStore(_T(".... GetRing:  tas has not changed%s"),NEWLINE);
    	#endif
	return false;
  }
  if (ring_groundspeed[0]==ring_groundspeed[1]) {
    	#ifdef KALMAN_DEBUG
    	StartupStore(_T(".... GetRing:  groundspeed has not changed%s"),NEWLINE);
    	#endif
	return false;
  }
  if (ring_trackbearing[0]==ring_trackbearing[1]) {
    	#ifdef KALMAN_DEBUG
    	StartupStore(_T(".... GetRing:  bearing has not changed%s"),NEWLINE);
    	#endif
	return false;
  }

  ring_tas=ring_trueairspeed[0];
  ring_gs=ring_groundspeed[0];
  ring_brg=ring_trackbearing[0];


  return true;

}


static void ResetRing(const bool reset) {
  static bool alreadydone=false;
  if (reset==false) {
	alreadydone=false;
	return;
  }

  if (!alreadydone) {
    #ifdef KALMAN_DEBUG
    StartupStore(_T(".... Reset RING%s"),NEWLINE);
    #endif
    ring_trackbearing[3]=0;
    ring_trackbearing[2]=0;
    ring_trackbearing[1]=0;
    ring_trackbearing[0]=0;
  
    ring_groundspeed[3]=0;
    ring_groundspeed[2]=0;
    ring_groundspeed[1]=0;
    ring_groundspeed[0]=0;

    ring_trueairspeed[3]=0;
    ring_trueairspeed[2]=0;
    ring_trueairspeed[1]=0;
    ring_trueairspeed[0]=0;

    ring_brg=0, ring_gs=0, ring_tas=0;
    alreadydone=true;
  }
}
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
  }
  return;
}


unsigned WindKalmanUpdate(NMEA_INFO *basic, DERIVED_INFO *derived,  double *windspeed, double *windbearing )
{
static double kalman_holdoff_time =0.0;

  //
  // CASES where we reset the matrix and we are not willing to fill it up 
  //

  // Normally we shall not select ZIGZAG if we dont have airspeed!!
  if (! basic->AirspeedAvailable) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... No Airspeed available%s"),NEWLINE);
	#endif
	WindKalmanReset(true);
	#if USE_RING
	ResetRing(true);
	#endif
  	kalman_holdoff_time=0;
	return 0;
  }

  //if (!derived->Flying || (!ISGAAIRCRAFT && !derived->FreeFlying) ) {
  if (!derived->Flying ) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Reset Kalman Wind (not flying)%s"),NEWLINE);
	#endif
	WindKalmanReset(true);
	#if USE_RING
	ResetRing(true);
	#endif
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
	#if USE_RING
	ResetRing(true);
	#endif
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
	ResetRing(true);
  	kalman_holdoff_time=0;
	return 0;
  }
  #endif


  //
  // CASES where we do not reset the matrix, but we dont fill it up either
  //

  // In tight turning do not sample wind, set it on hold
  if ((fabs(derived->TurnRate) > 20.0))
  {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Turning%s"),NEWLINE);
	#endif
	kalman_holdoff_time = basic->Time + BLACKOUT_TIME;
	#if USE_RING
	ResetRing(true);
	#endif
	return (0);
  }

  // After a tight turn skip sampling and wait some 3 more seconds to settle
  if (basic->Time <  kalman_holdoff_time)
  {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Holdoff%s"),NEWLINE);
	#endif
	#if USE_RING
	ResetRing(true);
	#endif
	return (0);
  }

  #if USE_RING
  // If nothing has changed in the last second, we assume we are dealing with a frozen instrument..
  // We dont reset ring buffer.
  if (ring_trackbearing[0]==basic->TrackBearing 
	&& ring_trueairspeed[0]==basic->TrueAirspeed 
	&& ring_groundspeed[0]==basic->Speed ) {

	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... NOTHING HAS CHANGED!%s"),NEWLINE);
	#endif
	return (0);
  }
  #endif

  #if MINIMAL_FILTER
  // below 10kmh we will not assume the gps bearing is still correct!
  // do not reset ring, simply discard data
  if (basic->Speed < 3 || basic->TrueAirspeed < 1) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... speed too low!%s"),NEWLINE);
	#endif
	return (0);
  }
  #endif

  #if USE_RING
  ResetRing(false);
  InsertRing(basic->TrackBearing, basic->Speed, basic->TrueAirspeed);

  if (!GetRing()) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Ring buffer not ready%s"),NEWLINE);
	#endif
	return (0);
  }
  #endif


  //
  // OK PROCEED > Matrix becomes dirty
  // 

  WindKalmanReset(false);
  kalman_holdoff_time =0;

  #if USE_RING
  double V = ring_tas;
  #else
  double V = basic->TrueAirspeed;
  #endif
  double dynamic_pressure = (V*V);
  float gps_vel[2];

  #if USE_RING
  double VG = ring_gs;
  gps_vel[0] = (float)(  fastsine( ring_brg ) * VG);
  gps_vel[1] = (float)(fastcosine( ring_brg ) * VG);
  #else
  double VG = basic->Speed;;
  gps_vel[0] = (float)(  fastsine( basic->TrackBearing ) * VG);
  gps_vel[1] = (float)(fastcosine( basic->TrackBearing ) * VG);
  #endif

  float dT = 1.0;

  EKWIND.StatePrediction(gps_vel, dT);
  EKWIND.Correction(dynamic_pressure, gps_vel);

  #if MINIMAL_FILTER
  static double validHBtime=0;
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

  #if MINIMAL_FILTER
  #define REQUIRED_SAMPLES 30
  // Wait to have at least REQUIRED_SAMPLES valid samples in order to use the new wind
  // (must be a %10 value so that next will be used at once)
  // (if not using circling wind, halved time)
  if (kalman_samples< (REQUIRED_SAMPLES/(AutoWindMode==D_AUTOWIND_BOTHCIRCZAG?1:2)) ) {
	#ifdef KALMAN_DEBUG
  	const float* x = EKWIND.get_state();
  	double speed   = sqrt((x[0]*x[0])+(x[1]*x[1]));
  	double bearing = atan2(-x[0], -x[1])*RAD_TO_DEG;
  	bearing =AngleLimit360(bearing);
  	StartupStore(_T(".... (WAITING, NOT USING) Kalman Filter Wind %4.1fkm/h %4.1fgrad  kalman_samples=%d%s"),speed*3.6,bearing,kalman_samples,NEWLINE);
  	StartupStore(_T("%s"),NEWLINE);
	#endif
	return 0; 
  }

  if (LKHearthBeats<validHBtime) {
	#ifdef KALMAN_DEBUG
	if (derived->Circling)
		StartupStore(_T(".... (WAITING while circling, not updating the wind\n"));
	else
		StartupStore(_T(".... (WAITING thermal timeout time, %.0f seconds to go\n"),(validHBtime-LKHearthBeats)/2);
  	StartupStore(_T("%s"),NEWLINE);
	#endif
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
  StartupStore(_T(".... (SETWIND) Kalman Filter Wind %4.1fkm/h %4.1f deg  kalman_samples=%d%s"),speed*3.6,bearing,kalman_samples,NEWLINE);
#endif

#if MINIMAL_FILTER
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
