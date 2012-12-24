/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"




double QNHAltitudeToStaticPressure(double alt) {
  // http://wahiduddin.net/calc/density_altitude.htm
  const double k1=0.190263;
  const double k2=8.417286e-5;
  return 100.0*pow((pow(QNH,k1)-k2*alt),1.0/k1);
  // example, alt= 100, QNH=1014
  // ps = 100203 Pa
}


double StaticPressureToAltitude(double ps) {
  // http://wahiduddin.net/calc/density_altitude.htm
  const double k1=0.190263;
  const double k2=8.417286e-5;
  return (pow(QNH,k1) - pow(ps/100.0, k1))/k2;
  // example, QNH=1014, ps=100203
  // alt= 100
}


// Converts altitude with QNH=1013.25 reference to QNH adjusted altitude
double AltitudeToQNHAltitude(double alt) {
  const double k1=0.190263;
  double ps = pow((44330.8-alt)/4946.54,1.0/k1);
  return StaticPressureToAltitude(ps);
}


// LK convert QNH altitude to QNE altitude
double StaticPressureToQNEAltitude(double ps) {
  // http://wahiduddin.net/calc/density_altitude.htm
  const double k1=0.190263;
  const double k2=8.417286e-5;
  return (pow(1013.25,k1) - pow(ps/100.0, k1))/k2;
}


double AltitudeToQNEAltitude(double alt) {
  return StaticPressureToQNEAltitude(QNHAltitudeToStaticPressure(alt));
}


double FindQNH(double alt_raw, double alt_known) {
  // find QNH so that the static pressure gives the specified altitude
  // (altitude can come from GPS or known airfield altitude or terrain
  // height on ground)

  // This function assumes the barometric altitude (alt_raw) is 
  // already adjusted for QNH ---> the function returns the
  // QNH value to make the barometric altitude equal to the
  // alt_known value.

  const double k1=0.190263;
  const double k2=8.417286e-5;

  // step 1, find static pressure from device assuming it's QNH adjusted
  double psraw = QNHAltitudeToStaticPressure(alt_raw);
  // step 2, calculate QNH so that reported alt will be known alt 
  return pow(pow(psraw/100.0,k1) + k2*alt_known,1/k1);

  // example, QNH=1014, ps=100203
  // alt= 100
  // alt_known = 120
  // qnh= 1016
}



double AirDensity(double altitude) {
  double rho = pow((44330.8-altitude)/42266.5,1.0/0.234969);
  #if BUGSTOP
  LKASSERT(rho>0);
  #endif
  if (rho<=0) rho=0.001; // we always give some pressure for the boys
  return rho;
}


// divide TAS by this number to get IAS
double AirDensityRatio(double altitude) {
  double rho = AirDensity(altitude);
  double rho_rat = sqrt(1.225/rho);
  return rho_rat;
}

// Air Density(kg/m3) from relative humidity(%), temperature(°C) and absolute pressure(Pa)
double AirDensity( double hr, double temp, double abs_press ) {
	return (1/(287.06*(temp+273.15)))*(abs_press - 230.617 * hr * exp((17.5043*temp)/(241.2+temp)));
}

// Air Speed from air density, humidity, temperature and absolute pressure
double TrueAirSpeed( double delta_press, double hr, double temp, double abs_press ) {
	double rho = AirDensity(hr,temp,abs_press);
	return sqrt(2 * delta_press / rho);
}
