/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Azimuth.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "LKProcess.h"



double GetAzimuth() {
  double sunazimuth=0;
  if (!Shading) return 0;
  if (CALCULATED_INFO.WindSpeed<1.7) { // below 6kmh the sun will prevail, for a guess.

	#if 0
	if (GPS_INFO.Latitude>=0)
		// NORTHERN EMISPHERE
		sunazimuth = MapWindow::GetDisplayAngle() + 135; //@  -45+180=135
	else
		// SOUTHERN EMISPHERE
		sunazimuth = MapWindow::GetDisplayAngle() + 45.0;
	#else
	int sunoffset=0; // 0 for southern emisphere, 180 for northern. 
	if (GPS_INFO.Latitude>=0) sunoffset=180; 

	int dd = LocalTime();
	int hours = (dd/3600);
	int mins = (dd/60-hours*60);
	hours = hours % 24;
	if (SIMMODE) {
		// for those who test the sim mode in the evening..
		if (hours>21) hours-=12;
		if (hours<7) hours+=12;
	}
	double h=(12-(double)hours)-((double)mins/60.0);

	if (h>=0) {
		if (h>5) h=5;
		sunazimuth = MapWindow::GetDisplayAngle() +sunoffset + (h*12); //@ 60 deg from east, max
	} else {
		h*=-1;
		if (h>6) h=6;
		sunazimuth = MapWindow::GetDisplayAngle() +sunoffset - (h*10);
	}
	#endif

  } else {
	// else we use wind direction for shading. 
	sunazimuth = MapWindow::GetDisplayAngle()-CALCULATED_INFO.WindBearing;
  }
  return sunazimuth;
}



