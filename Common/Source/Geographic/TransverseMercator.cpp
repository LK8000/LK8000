/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TransverseMercator.cpp
 * Author: Bruno de Lacheisserie
 */

#include "Compiler.h"
#include "options.h"
#include "TransverseMercator.h"
#include "GeoPoint.h"
#include <cmath>

extern bool earth_model_wgs84;

namespace {

struct DATUM {
  double a; // a  Equatorial earth radius
  double b; // b  Polar earth radius
  double f; // f= (a-b)/a  Flatenning
  double esq; // esq = 1-(b*b)/(a*a)  Eccentricity Squared
  double e; // sqrt(esq)  Eccentricity
};


#ifdef _WGS84
// WGS84 data
const DATUM Datum_WGS84 = {
  6378137.0, // a
  6356752.3142, // b
  0.00335281066474748, // f = 1/298.257223563
  0.006694380004260807, // esq
  0.0818191909289062, // e
};
#endif

const DATUM Datum_FAI = {
  6371000.0, // a
  6371000.0, // b
  0, // f = 1/298.257223563
  0, // esq
  0, // e
};

inline double rad2deg(double rad) {
  return (rad * 180 / M_PI);
}

inline double deg2rad(double deg) {
  return (deg * M_PI / 180);
}

} // namespace

TransverseMercator::TransverseMercator(const GeoPoint& center) 
        : lat0(deg2rad(center.latitude))
        , lon0(deg2rad(center.longitude))
        , k0(1), N0(0.0), E0(0.0)
{

}

//====================================
// Local Grid to Lat/Lon conversion
//====================================

GeoPoint TransverseMercator::Reverse(Point2D<double> position) const {

#ifdef _WGS84
  const DATUM& Datum = earth_model_wgs84 ? Datum_WGS84 : Datum_FAI;
#else
  const DATUM& Datum = Datum_FAI;
#endif

  double a = Datum.a; // Semi-major axis of reference ellipsoid
  double f = Datum.f; // Ellipsoidal flattening
  double b = a * (1 - f);

  double e2 = 2 * f - (f * f);
  double e4 = e2*e2;
  double e6 = e4*e2;

  double A0 = 1 - e2 / 4 - 3 * e4 / 64 - 5 * e6 / 256;
  double A2 = 3 * (e2 + e4 / 4 + 15 * e6 / 256) / 8;
  double A4 = 15 * (e4 + 3 * e6 / 4) / 256;
  double A6 = 35 * e6 / 3072;

  double m0 = a * (A0 * lat0 - A2 * sin(2 * lat0) + A4 * sin(4 * lat0) - A6 * sin(6 * lat0));

  //================
  // GRID -> Lat/Lon
  //================
  // http://www.linz.govt.nz/geodetic/conversion-coordinates/projection-conversions/transverse-mercator-preliminary-computations#lbl1

  double N1 = position.y - N0;
  double m = m0 + N1 / k0;
  double n = (a - b) / (a + b);

  double G = a * (1 - n)*(1 - n * n)*(1 + 9 * n * n / 4 + 225 * n * n * n * n / 64);
  double s = m / G;
  double ph = s + (3 * n / 2 - 27 * n * n * n / 32) * sin(2 * s)+(21 * n * n / 16 - 55 * n * n * n * n / 32) * sin(4 * s)+(151 * n * n * n / 96) * sin(6 * s)+(1097 * n * n * n * n / 512) * sin(8 * s);

  double r = a * (1 - e2) / pow(1 - e2 * sin(s) * sin(s), 1.5);
  double nu = a / sqrt(1 - e2 * sin(s) * sin(s));
  double ps = nu / r;
  double t = tan(s);
  double E1 = position.x - E0;
  double x = E1 / (k0 * nu);


  double T1 = t / (k0 * r) * E1 * x / 2;
  double T2 = t / (k0 * r) * E1 * x * x * x / 24 * (-4 * ps * ps + 9 * ps * (1 - t * t) + 12 * t * t);
  double T3 = t / (k0 * r) * E1 * x * x * x / 720 * (8 * ps * ps * ps * ps * (11 - 24 * t * t) - 12 * ps * ps * ps * (21 - 71 * t * t) + 15 * ps * ps * (15 - 98 * t * t + 15 * t * t * t * t) + 180 * ps * (5 * t * t - 3 * t * t * t * t) + 360 * t * t * t * t);
  double T4 = t / (k0 * r) * E1 * x * x * x / 40320 * (1385 - 3633 * t * t + 4095 * t * t * t * t + 1575 * t * t * t * t * t * t);

  //	t = tan(lat);

  double secph = 1 / cos(ph);
  double T5 = x*secph;
  double T6 = x * x * x * secph / 6 * (ps + 2 * t * t);
  double T7 = x * x * x * x * x * secph / 120 * (-4 * ps * ps * ps + (1 - 6 * t * t) + ps * ps * (9 - 68 * t * t) + 72 * ps * t * t*+24 * t * t * t * t);
  double T8 = x * x * x * x * x * x * x * secph / 5040 * (61 + 662 * t * t + 1320 * t * t * t * t + 720 * t * t * t * t * t * t);

  return GeoPoint(
          rad2deg(ph - T1 + T2 - T3 + T4),
          rad2deg(lon0 + T5 - T6 + T7 - T8));
}

//====================================
// Lat/Lon to Local Grid conversion
//====================================

Point2D<double> TransverseMercator::Forward(const GeoPoint& position) const {
  // Datum data for Lat/Lon to TM conversion

#ifdef _WGS84
  const DATUM& Datum = earth_model_wgs84 ? Datum_WGS84 : Datum_FAI;
#else
  const DATUM& Datum = Datum_FAI;
#endif

  double a = Datum.a; // Semi-major axis of reference ellipsoid
  double f = Datum.f; // Ellipsoidal flattening

  double lat = deg2rad(position.latitude);
  double lon = deg2rad(position.longitude);

  //===============
  // Lat/Lon -> TM
  //===============
  double slat1 = sin(lat);
  double clat1 = cos(lat);
  double clat1sq = clat1*clat1;

  double e2 = 2 * f - (f * f);
  double e4 = e2*e2;
  double e6 = e4*e2;

  double A0 = 1 - e2 / 4 - 3 * e4 / 64 - 5 * e6 / 256;
  double A2 = 3 * (e2 + e4 / 4 + 15 * e6 / 256) / 8;
  double A4 = 15 * (e4 + 3 * e6 / 4) / 256;
  double A6 = 35 * e6 / 3072;

  double m = a * (A0 * lat - A2 * sin(2 * lat) + A4 * sin(4 * lat) - A6 * sin(6 * lat));
  double m0 = a * (A0 * lat0 - A2 * sin(2 * lat0) + A4 * sin(4 * lat0) - A6 * sin(6 * lat0));

  double r = a * (1 - e2) / pow((1 - (e2 * (slat1 * slat1))), 1.5);
  double n = a / sqrt(1 - (e2 * (slat1 * slat1)));
  double ps = n / r;
  double t = tan(lat);
  double o = lon - lon0;

  double K1 = k0 * (m - m0);
  double K2 = k0 * o * o * n * slat1 * clat1 / 2;
  double K3 = k0 * o * o * o * o * n * slat1 * clat1 * clat1sq / 24 * (4 * ps * ps + ps - t * t) / 24;
  double K4 = k0 * o * o * o * o * o * o * n * slat1 * clat1sq * clat1sq * clat1 * (8 * ps * ps * ps * ps * (11 - (24 * t * t)) - 28 * ps * ps * ps * (1 - 6 * t * t) + ps * ps * (1 - 32 * t * t) - ps * 2 * t * t + t * t * t * t) / 720;
  double K5 = k0 * o * o * o * o * o * o * o * o * n * slat1 * clat1sq * clat1sq * clat1sq * clat1 * (1385 - 311 * t * t * 543 * t * t * t * t - t * t * t * t * t) / 40320;

  double K6 = o * o * clat1sq * (ps - t * t) / 6;
  double K7 = o * o * o * o * clat1sq * clat1sq * (4 * ps * ps * ps * (1 - 6 * t * t) + ps * ps * (1 + 8 * t * t) - ps * 2 * t * t + t * t * t * t) / 120;
  double K8 = o * o * o * o * o * o * clat1sq * clat1sq * clat1sq * (61 - 479 * t * t + 179 * t * t * t * t - t * t * t * t * t * t);

  return {
    // ING east (x)
    E0 + k0 * n * o * clat1 * (1 + K6 + K7 + K8),
    // ING north (y)
    N0 + K1 + K2 + K3 + K4 + K5
  };
}
