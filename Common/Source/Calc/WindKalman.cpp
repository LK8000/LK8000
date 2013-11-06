/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: WindKalman,v 1.1 2013/11/04 19:28:45 root Exp root $
*/


#include "Externs.h"
#include "WindKalman.h"
#include "WindEKF.h"
#define KALMAN_DEBUG

#define BLACKOUT_TIME  3.0
WindEKF ekf;


unsigned CalcQualityLevel(unsigned i)
{
  if (i > 400) return 4;
  if (i > 100) return 3;
  if (i >  30) return 2;
return 1;
}

unsigned WindKalmanUpdate(NMEA_INFO *basic, DERIVED_INFO *derived,  double *windspeed, double *windbearing )
{
static unsigned int count=0;
double holdoff_time =0.0;

  if (! derived->Flying) {
#ifdef KALMAN_DEBUG
		StartupStore(_T(".... Reset Kalman Wind (not flying)%s"),NEWLINE);
#endif
	count =0;
    ekf.Init();
    holdoff_time=0;
    return 0;
  }

  if (basic->NAVWarning) {
#ifdef KALMAN_DEBUG
	StartupStore(_T(".... Reset Kalman by NavWarning %s"),NEWLINE);
#endif
	count =0;
	ekf.Init();
	holdoff_time=0;
    return 0;
  }


  if (!basic->AirspeedAvailable) {
#ifdef KALMAN_DEBUG
	StartupStore(_T(".... No Airspeed available%s"),NEWLINE);
#endif
    return (0);
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

  if ((fabs(derived->TurnRate) > 20.0))
  {
	StartupStore(_T(".... Turning%s"),NEWLINE);
	holdoff_time = basic->Time + BLACKOUT_TIME;
    return (0);
  }


  if (basic->Time <  holdoff_time)
  {
	StartupStore(_T(".... Holdoff%s"),NEWLINE);
    return (0);
  }

  holdoff_time =0;



  double V = basic->TrueAirspeed;;
  double dynamic_pressure = (V*V);
  float gps_vel[2];

  double VG = basic->Speed;;
  gps_vel[0] = (float)(  fastsine( basic->TrackBearing ) * VG);
  gps_vel[1] = (float)(fastcosine( basic->TrackBearing ) * VG);

  float dT = 1.0;

  ekf.StatePrediction(gps_vel, dT);
  ekf.Correction(dynamic_pressure, gps_vel);



  /* prevent value update while circling */
  if (derived->Circling)
	  count = 0;


  ++count;

  /* update values every 10 vales only */
  if (count%10!=0)
    return 0;

  /*************************************************************/
  const float* x = ekf.get_state();
  double speed   = sqrt((x[0]*x[0])+(x[1]*x[1]));
  double bearing = atan2(-x[0], -x[1])*RAD_TO_DEG;
  /************************************************************/
  bearing =AngleLimit360(bearing);
  /************************************************************/
#ifdef KALMAN_DEBUG
  StartupStore(_T(".... Kalman Filter Wind %4.1fkm/h %4.1fgrad%s"),speed*3.6,bearing,NEWLINE);
#endif
  (*windbearing) = bearing;
  (*windspeed)   = speed;
  derived->WindBearing = bearing;
  derived->WindSpeed   = speed;

  return 0; //CalcQualityLevel(count);
}
