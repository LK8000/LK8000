/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MagneticVariation.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "magfield.h"



//
// Accurate also for altitude, far too precise for humans on a plane!
// Will return 0 if error. Assuming there is always a magnetic variation.
//
double CalculateMagneticVariation() {

  double lat=GPS_INFO.Latitude;
  double lon=GPS_INFO.Longitude;

  if (  GPS_INFO.NAVWarning ||
	GPS_INFO.Year >2015 || GPS_INFO.Year<2010 ||
	lat == 0 || lon == 0
     )  return 0.0;

  double magvar=0;
  double h=CALCULATED_INFO.NavAltitude/1000.0;

  if (h<0.5) h=0.5; // 500m 

  short retry=3;
  while (retry-->0) {
	magvar =  RAD_TO_DEG*SGMagVar( 
			DEG_TO_RAD*lat,
			DEG_TO_RAD*lon,
			h,
	        	yymmdd_to_julian_days(GPS_INFO.Year-2000,GPS_INFO.Month, GPS_INFO.Day)
			);

	if (magvar != magvar) {
		#if TESTBENCH
		StartupStore(_T(".... CalculateMagVar OVERFLOW detected\n"));
		#endif
		continue;
	}
	break;
  }

  // Check for a consistent result
  if (magvar != magvar) {
	#if TESTBENCH
	StartupStore(_T(".... CalculateMagVar OVERFLOW PROBLEM = no result!\n"));
	#endif
	return 0.0;
  }
  if (magvar==0) {
	#if TESTBENCH
	StartupStore(_T(".... CalculateMagVar ERROR,no result!\n"));
	#endif
	return 0.0;
  }
  if (magvar>60||magvar<-60) {
	#if TESTBENCH
	StartupStore(_T(".... CalculateMagVar ERROR, ><60,no result!\n"));
	#endif
	return 0.0;
  }

  #if TESTBENCH
  StartupStore(_T(".... Lat=%f Lon=%f H=%f y=%d m=%d d=%d MAGVAR= %f\n"),
	GPS_INFO.Latitude, GPS_INFO.Longitude, h, 
	GPS_INFO.Year, GPS_INFO.Month, GPS_INFO.Day, magvar);
  #endif


  return magvar;

}

