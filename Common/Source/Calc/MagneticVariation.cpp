/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
  if (GPS_INFO.Year > 2029 || GPS_INFO.Year < 2025) {
    return 0.0;
  }
  if (GPS_INFO.NAVWarning) {
    return 0.0;
  }
  double lat = GPS_INFO.Latitude;
  double lon = GPS_INFO.Longitude;
  if (lat == 0.0 || lon == 0.0) {
    return 0.0;
  }

  double magvar = 0;
  double h = CALCULATED_INFO.NavAltitude / 1000.0;

  if (h < 0.5) {
    h = 0.5;  // 500m
  }

  short retry = 3;
  while (retry-- > 0) {
    magvar = RAD_TO_DEG * SGMagVar(DEG_TO_RAD * lat, DEG_TO_RAD * lon, h,
                                   yymmdd_to_julian_days(GPS_INFO.Year - 2000, GPS_INFO.Month, GPS_INFO.Day));

    if (magvar != magvar) {
      TestLog(_T(".... CalculateMagVar OVERFLOW detected"));
      continue;
    }
    break;
  }

  // Check for a consistent result
  if (magvar != magvar) {
    TestLog(_T(".... CalculateMagVar OVERFLOW PROBLEM = no result!"));
    return 0.0;
  }
  if (magvar == 0) {
    TestLog(_T(".... CalculateMagVar ERROR,no result!"));
    return 0.0;
  }
  if (magvar > 60 || magvar < -60) {
    TestLog(_T(".... CalculateMagVar ERROR, ><60,no result!"));
    return 0.0;
  }

  TestLog(_T(".... Lat=%f Lon=%f H=%f y=%d m=%d d=%d MAGVAR= %f"),
          GPS_INFO.Latitude, GPS_INFO.Longitude, h,
          GPS_INFO.Year, GPS_INFO.Month, GPS_INFO.Day, magvar);

  return magvar;
}
