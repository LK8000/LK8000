/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Orbiter.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "Sound/Sound.h"
#include "NavFunctions.h"

#define LKTH_LAT		Calculated->ThermalEstimate_Latitude
#define LKTH_LON		Calculated->ThermalEstimate_Longitude
#define LKTH_R			Calculated->ThermalEstimate_R
#define LKTH_W			Calculated->ThermalEstimate_W
#define LK_TURNRATE		Calculated->TurnRate
#define LK_CALCHEADING		Calculated->Heading
#define LK_BANKING		Calculated->BankAngle
#define LK_MYTRACK		Basic->TrackBearing
#define LK_MYLAT		Basic->Latitude
#define LK_MYLON		Basic->Longitude

//#define DEBUG_ORBITER	1

// Thermal orbiter by Paolo Ventafridda, November 2010
void CalculateOrbiter(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  static unsigned int timepassed=0;
  static bool alreadywarned=false;

  if (!Calculated->Circling || !EnableSoundModes) return;
  timepassed++;
  if (LKTH_R <1) {
	return;  // no thermal center available
  }

  double thtime = Basic->Time - Calculated->ClimbStartTime;
  // we need a valid thermal center to work on. Assuming 2 minutes is enough to sample the air around.
  if (thtime<120) return;
  // after 1500ft thermal gain, autodisable
  if (Calculated->ThermalGain>500) return;
  if (Calculated->ThermalGain<50) return;

  // StartupStore(_T("*** Tlat=%f Tlon=%f R=%f W=%f  TurnRate=%f \n"), LKTH_LAT, LKTH_LON, LKTH_R, LKTH_W, LK_TURNRATE);
  // StartupStore(_T("*** CalcHeading=%f Track=%f TurnRate=%f Bank=%f \n"), LK_CALCHEADING, LK_MYTRACK,  LK_TURNRATE, LK_BANKING);

  double th_center_distance, th_center_bearing;	// thermal center
  double orbital_bearing;                 	// orbital tangent 
  double orbital_brgdiff;			// current difference between track and orbital_bearing
  double orbital_warning_angle;			// warning angle difference

  double alpha;					// alpha angle to the tangent relative to th_center_bearing
  double circlesense=0;				// 1 for CW, -1 for CCW

  // these parameters should be dynamically calculated for each thermal in the future
  // th_angle should be correlated to IAS

  double th_radius;				// thermal radius
  double th_minoverradius, th_maxoverradius;	// radius+overradius is the minimal and max approach distance
  double th_angle;				// deg/s default turning in the thermal. Pilot should turn with this 
						// angolar speed.
  double reactiontime;			// how many seconds pass before the pilot start turning
  double turningcapacitypersecond;	// how many degrees can I change in my turnrate in a second?

  // TODO: TUNE THEM IN REAL TIME!
  if (ISPARAGLIDER) {
	th_radius=50;
	th_minoverradius=1;
	th_maxoverradius=100;
	th_angle=18;
	turningcapacitypersecond=10;
	reactiontime=1.0;
  } else { 
	th_radius=90;
	th_minoverradius=5;
	th_maxoverradius=160;
	th_angle=16.5;
	turningcapacitypersecond=10;
	reactiontime=1.0;
  }

  circlesense=LK_TURNRATE>0 ? 1 : -1;	//@ CCW: -1   CW: 1
  if (circlesense==0 || (fabs(LK_TURNRATE)<8)) { // 8 deg/sec are 45 seconds per turn
	alreadywarned=false;
	return;
  }

  DistanceBearing(LK_MYLAT, LK_MYLON,LKTH_LAT,LKTH_LON,&th_center_distance,&th_center_bearing);

  if (th_center_distance< (th_radius+th_minoverradius)) {
	#if DEBUG_ORBITER
	StartupStore(_T("*** Too near:  dist=%.0f < %.0f\n"), th_center_distance, th_radius+th_minoverradius);
	#endif
	return;
  }
  if (th_center_distance> (th_radius+th_maxoverradius)) {
	#if DEBUG_ORBITER
	StartupStore(_T("*** Too far:  dist=%.0f >  %.0f\n"), th_center_distance, th_radius+th_maxoverradius);
	#endif
	return;
  }
  // StartupStore(_T("*** distance to orbit %.0f (ratio:%0.f)\n"), 
  // th_center_distance- th_radius , (th_center_distance - th_radius)/th_radius);

  alpha= atan2(th_radius, th_center_distance)*RAD_TO_DEG;

  orbital_bearing  = th_center_bearing - (alpha*circlesense); //@ add for CCW
  if (orbital_bearing>359) orbital_bearing-=360;
  //  orbital_distance = th_radius / sin(alpha);

  // StartupStore(_T("*** tc_dist=%f th_center_bearing=%f  orbital_distance=%f orbital_bearing=%f  alpha=%f  mydir=%f\n"),
  // th_center_distance, th_center_bearing, orbital_distance, orbital_bearing, alpha, LK_CALCHEADING );

  if (circlesense==1) 
	orbital_brgdiff = orbital_bearing-LK_CALCHEADING; // CW
  else
	orbital_brgdiff = LK_CALCHEADING - orbital_bearing; // CCW

  if (orbital_brgdiff<0) {
	#if DEBUG_ORBITER
	StartupStore(_T("*** passed target orbit direction\n"));
	#endif
	return;
  }
  if (orbital_brgdiff>90) {
	#if DEBUG_ORBITER
	StartupStore(_T("*** wrong direction\n"));
	#endif
	return;
  }

  static int lasttimewarned=0;
  double actionadvancetime;		// how many seconds are needed in advance, given the current turnrate?

  // assuming circling with the same radius of the perfect thermal: 18deg/s, 20s turn rate approx.
  actionadvancetime=(circlesense*(LK_TURNRATE/turningcapacitypersecond))+reactiontime;

  // When circling with a radius greater than the default, we have a problem.
  // We must correct the predicted angle threshold
  orbital_warning_angle=actionadvancetime*(LK_TURNRATE*LK_TURNRATE)/th_angle;
  #if DEBUG_ORBITER
  StartupStore(_T("... LK_TURNRATE=%.0f  advancetime=%.2f  angle=%.1f \n"), LK_TURNRATE, actionadvancetime,orbital_warning_angle);
  #endif

  if (orbital_brgdiff<orbital_warning_angle) {
	if (!alreadywarned && ((timepassed-lasttimewarned)>12) ) {

		if (th_center_distance<(th_radius+(th_maxoverradius/2))) {
			#if DEBUG_ORBITER
			StartupStore(_T("****** GO STRAIGHT NOW FOR SHORT TIME ******\n"));
			#endif
			LKSound(_T("LK_ORBITER1.WAV"));
		} else {
			#if DEBUG_ORBITER
			StartupStore(_T("****** GO STRAIGHT NOW FOR LONGER TIME ******\n"));
			#endif
			LKSound(_T("LK_ORBITER2.WAV"));
		}
		alreadywarned=true;
		lasttimewarned=timepassed;
	} else {
		#if DEBUG_ORBITER
		StartupStore(_T("****** GO STRAIGHT, already warned\n"));
		#endif
	}
  } else {
	alreadywarned=false;
  }


}

