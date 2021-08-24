/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Azimuth.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "LKProcess.h"



double GetAzimuth(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
  double sunazimuth=0;
  if (!Shading) return 0;
  if (Calculated.WindSpeed<1.7) { // below 6kmh the sun will prevail, for a guess.

	int sunoffset = (Basic.Latitude>=0) ? 180 : 0; // 0 for southern emisphere, 180 for northern. 

	int dd = LocalTime(Basic.Time);
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
  } else {
	// else we use wind direction for shading. 
	sunazimuth = MapWindow::GetDisplayAngle()-Calculated.WindBearing;
  }
  return sunazimuth;
}
