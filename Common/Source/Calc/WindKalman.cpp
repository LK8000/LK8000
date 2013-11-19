/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: WindKalman,v 1.1 2013/11/04 19:28:45 root Exp root $
*/


#include "externs.h"
#include "WindEKF.h"

#if TESTBENCH
// #define KALMAN_DEBUG
#endif

#define BLACKOUT_TIME  3.0
WindEKF EKWIND;

unsigned CalcQualityLevel(unsigned i)
{
  if (i > 400) return 4;
  if (i > 100) return 3;
  if (i >  30) return 2;
return 1;
}


void WindKalmanReset(const bool reset) {
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
	StartupStore(_T(".... WindKalman RESET DONE%s"),NEWLINE);
	#endif
	EKWIND.Init();
	alreadydone=true;
	return;
  }
}


unsigned WindKalmanUpdate(NMEA_INFO *basic, DERIVED_INFO *derived,  double *windspeed, double *windbearing )
{
static unsigned int count=0;
double holdoff_time =0.0;

  if (! basic->AirspeedAvailable) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... No Airspeed available%s"),NEWLINE);
	#endif
	WindKalmanReset(true);
	holdoff_time=0;
	return 0;
  }

  if (!derived->Flying || (!ISGAAIRCRAFT && !derived->FreeFlying) ) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Reset Kalman Wind (not flying)%s"),NEWLINE);
	#endif
	count =0;
	WindKalmanReset(true);
	holdoff_time=0;
	return 0;
  }

  if (basic->NAVWarning) {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Reset Kalman by NavWarning %s"),NEWLINE);
	#endif
	count =0;
	WindKalmanReset(true);
	holdoff_time=0;
	return 0;
  }


/*
  static int oldspeed=0;
  int newspeed = (int)basic->Speed*100;
  if(oldspeed == newspeed)
  {
#ifdef KALMAN_DEBUG
	StartupStore(_T("....skipped Samespeed%s"),NEWLINE);
#endif
	return (0);
  }
  oldspeed =  newspeed;
*/


  // In tight turning do not sample wind
  if ((fabs(derived->TurnRate) > 20.0))
  {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Turning%s"),NEWLINE);
	#endif
	holdoff_time = basic->Time + BLACKOUT_TIME;
	return (0);
  }


  // After a tight turn skip sampling and wait some 3 more seconds to settle
  if (basic->Time <  holdoff_time)
  {
	#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Holdoff%s"),NEWLINE);
	#endif
	return (0);
  }


  // Matrix becomes dirty
  WindKalmanReset(false);
  holdoff_time =0;


  double V = basic->TrueAirspeed;;
  double dynamic_pressure = (V*V);
  float gps_vel[2];

  double VG = basic->Speed;;
  gps_vel[0] = (float)(  fastsine( basic->TrackBearing ) * VG);
  gps_vel[1] = (float)(fastcosine( basic->TrackBearing ) * VG);

  float dT = 1.0;

  EKWIND.StatePrediction(gps_vel, dT);
  EKWIND.Correction(dynamic_pressure, gps_vel);


  /* prevent value update while circling */
  if (derived->Circling)
	  count = 0;

  ++count;

  /* update values every 10 vales only */
  if (count%10!=0)
    return 0;

  /*************************************************************/
  const float* x = EKWIND.get_state();
  double speed   = sqrt((x[0]*x[0])+(x[1]*x[1]));
  double bearing = atan2(-x[0], -x[1])*RAD_TO_DEG;
  /************************************************************/
  bearing =AngleLimit360(bearing);
  /************************************************************/
#ifdef KALMAN_DEBUG
  StartupStore(_T(".... Kalman Filter Wind %4.1fkm/h %4.1fgrad  count-%d%s"),speed*3.6,bearing,count,NEWLINE);
#endif
  (*windbearing) = bearing;
  (*windspeed)   = speed;

  return 1; 
}
