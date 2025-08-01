#include <GeographicLib/Geodesic.hpp>
using GeographicLib::Geodesic;

double AngleLimit360(double theta) {
  while (theta>=360.0) {
    theta-= 360.0;
  }
  while (theta<0.0) {
    theta+= 360.0;
  }
  return theta;
}

double AngleLimit180(double theta) {
  while (theta>180.0) {
    theta-= 360.0;
  }
  while (theta<-180.0) {
    theta+= 360.0;
  }
  return theta;
}

void DistanceBearing(double lat1, double lon1, double lat2, double lon2, double* Distance, double* Bearing) {
  double t;
  unsigned outmask = 0;
  const Geodesic& geod = Geodesic::WGS84();
  if (Distance) {
    outmask |= Geodesic::DISTANCE;
  }
  else {
    Distance = &t;
  }
  if (Bearing) {
    outmask |= Geodesic::AZIMUTH;
  }
  else {
    Bearing = &t;
  }
  geod.GenInverse(lat1, lon1, lat2, lon2, outmask, *Distance, *Bearing, t, t, t, t, t);
  if (outmask & Geodesic::AZIMUTH) {
    *Bearing = AngleLimit360(*Bearing);
  }
}

void StartupStoreV(const char* fmt, va_list args) {
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}