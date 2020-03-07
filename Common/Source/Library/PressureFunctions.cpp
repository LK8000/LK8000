/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "options.h"
#include "externs.h"
#include "McReady.h"

static constexpr double k1 = 0.190263;
static constexpr double inv_k1 = 1.0 / k1;
static constexpr double k2 = 8.417286e-5;
static constexpr double inv_k2 = 1.0 / k2;

//
// 6 march 2014 by Bruno and Paolo
// This is to correct sinkrate adjusted for air density
//
double AirDensitySinkRate(double ias, double qnhaltitude) {

    double sinkias = GlidePolar::SinkRate(ias)*AirDensityRatio(QNHAltitudeToQNEAltitude(qnhaltitude));

    // this can actually happen with a bad polar file loaded!
    BUGSTOP_LKASSERT(sinkias<=0);
    if (sinkias>0) sinkias=0;
    return sinkias;
}


double AirDensitySinkRate(double ias, double qnhaltitude, double gload) {

    double w0 = AirDensitySinkRate(ias, qnhaltitude);
    gload = max(0.1,fabs(gload));
    double v2 = GlidePolar::Vbestld()/max(GlidePolar::Vbestld()/2,ias);

    LKASSERT(GlidePolar::bestld!=0);
    if (GlidePolar::bestld==0) return -1; // UNMANAGED
    return w0-(ias/(2*GlidePolar::bestld))* (gload*gload-1)*(v2*v2);
}


double AltitudeToStaticPressure(double ref, double alt) {
  // example, alt= 100, ref=1014
  // ps = 100203 Pa

  double ps = 100.0*pow((pow(ref,k1)-k2*alt), inv_k1);
  assert(isfinite(alt)); // bug with Android ndk r21 ( release only )
  return ps;
}

double QNHAltitudeToStaticPressure(double alt) {
  return AltitudeToStaticPressure(QNH, alt);
}

double QNEAltitudeToStaticPressure(double alt) {
  return AltitudeToStaticPressure(PRESSURE_STANDARD, alt);
}

double StaticPressureToAltitude(double ref, double ps) {
  // example, ref=1014, ps=100203
  // alt= 100

  double alt = (pow(ref,k1) - pow(ps/100.0, k1))* inv_k2;
  assert(isfinite(alt)); // bug with Android ndk r21 ( release only )
  return alt;
}

double StaticPressureToQNHAltitude(double ps) {
  return StaticPressureToAltitude(QNH, ps);
}

double StaticPressureToQNEAltitude(double ps) {
  return StaticPressureToAltitude(PRESSURE_STANDARD, ps);
}

// Converts altitude with QNH=1013.25 reference to QNH adjusted altitude
double QNEAltitudeToQNHAltitude(double alt) {
  return StaticPressureToQNHAltitude(QNEAltitudeToStaticPressure(alt));
}

double QNHAltitudeToQNEAltitude(double alt) {
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

  // step 1, find static pressure from device assuming it's QNH adjusted
  double psraw = QNHAltitudeToStaticPressure(alt_raw);
  // step 2, calculate QNH so that reported alt will be known alt  
  return AltitudeToStaticPressure(psraw, alt_known);

  // example, QNH=1014, ps=100203
  // alt= 100
  // alt_known = 120
  // qnh= 1016
}



double AirDensity(double qne_altitude) {
  if (qne_altitude>44330){
      qne_altitude=44330;
  }
  if (qne_altitude<-200) { // min altitude
      TESTBENCH_DO_ONLY(10,StartupStore(_T(".... INVALID ALTITUDE in AirDensity: %f%s"),qne_altitude,NEWLINE));
      qne_altitude=-200;
  }

  double rho = pow((44330.8-qne_altitude)/42266.5,1.0/0.234969);
  BUGSTOP_LKASSERT(rho>0);
  if (rho<=0) rho=1; // we always give some pressure for the boys
  return rho;
}


// divide TAS by this number to get IAS
double AirDensityRatio(double qne_altitude) {
  double rho = AirDensity(qne_altitude);
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

// True Air Speed from Indicated Air Speed and QNE Altitude
double TrueAirSpeed( double ias, double qne_altitude) {
  double rho = AirDensity(qne_altitude);
  return ias * sqrt(1.225 / rho);
}

// Indicated Air Speed from True Air Speed and QNE Altitude
double IndicatedAirSpeed( double tas, double qne_altitude) {
  double rho = AirDensity(qne_altitude);
  return tas / sqrt(1.225 / rho);
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("pressure") {

  auto Approx = [](double value) {
    return doctest::Approx(value).epsilon(0.01);
  };

  SUBCASE("pressure to QNE") {
    CHECK(Approx(   0.0) == StaticPressureToQNEAltitude(101325.));
    CHECK(Approx( 540.1) == StaticPressureToQNEAltitude( 95000.));
    CHECK(Approx( 988.1) == StaticPressureToQNEAltitude( 90000.));
    CHECK(Approx(1948.2) == StaticPressureToQNEAltitude( 80000.));
    CHECK(Approx(3010.9) == StaticPressureToQNEAltitude( 70000.));
  }

  SUBCASE("QNE to pressure") {
    CHECK(Approx(101325.00) == QNEAltitudeToStaticPressure(   0.));
    CHECK(Approx( 90970.09) == QNEAltitudeToStaticPressure( 900.));
    CHECK(Approx( 81489.22) == QNEAltitudeToStaticPressure(1800.));
    CHECK(Approx( 70108.54) == QNEAltitudeToStaticPressure(3000.));
  }

  double old_QNH = std::exchange(QNH, 996.);

  SUBCASE("pressure to QNH") {
    CHECK(Approx(-144.829) == StaticPressureToQNHAltitude(101325.));
    CHECK(Approx(   0.000) == StaticPressureToQNHAltitude( 99600.));
    CHECK(Approx( 844.860) == StaticPressureToQNHAltitude( 90000.));
    CHECK(Approx(1804.302) == StaticPressureToQNHAltitude( 80000.));
    CHECK(Approx(2867.575) == StaticPressureToQNHAltitude( 70000.));
  }

  SUBCASE("QNH to pressure") {
    CHECK(Approx(99600.00) == QNHAltitudeToStaticPressure(   0.));
    CHECK(Approx(88464.50) == QNHAltitudeToStaticPressure(1000.));
    CHECK(Approx(78574.00) == QNHAltitudeToStaticPressure(2000.));
    CHECK(Approx(68828.00) == QNHAltitudeToStaticPressure(3000.));
  }

  QNH = 1013.25;

  SUBCASE("QNE to QNH") {
    CHECK(doctest::Approx(1000.) == QNEAltitudeToQNHAltitude(1000.));
    CHECK(doctest::Approx(2000.) == QNEAltitudeToQNHAltitude(2000.));
    CHECK(doctest::Approx(3000.) == QNEAltitudeToQNHAltitude(3000.));
  }

  SUBCASE("QNH to QNE") {
    CHECK(doctest::Approx(1000.) == QNHAltitudeToQNEAltitude(1000.));
    CHECK(doctest::Approx(2000.) == QNHAltitudeToQNEAltitude(2000.));
    CHECK(doctest::Approx(3000.) == QNHAltitudeToQNEAltitude(3000.));
  }

  QNH = old_QNH;
}

#endif
