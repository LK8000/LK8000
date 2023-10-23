/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GeoPoint.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 19, 2019, 12:59 AM
 */

#include "GeoPoint.h"
#include "NavFunctions.h"

GeoPoint GeoPoint::Direct(double bearing, double distance) const {
  GeoPoint Out;
  FindLatitudeLongitude(latitude, longitude, bearing, distance, &Out.latitude, &Out.longitude);
  return Out;
}

void GeoPoint::Reverse(const GeoPoint& point, double& bearing, double& distance) const {
  DistanceBearing(latitude, longitude, point.latitude, point.longitude, &distance, &bearing);
}

double GeoPoint::Distance(const GeoPoint& point) const {
  double d;
  DistanceBearing(latitude, longitude, point.latitude, point.longitude, &d, nullptr );
  return d;
}

double GeoPoint::Bearing(const GeoPoint& point) const {
  double d;
  DistanceBearing(latitude, longitude, point.latitude, point.longitude, nullptr, &d);
  return d;
}

double ProjectedDistance(const GeoPoint p1, const GeoPoint p2, const GeoPoint p3, double* xtd, double* crs) {
  return ProjectedDistance(p1.longitude, p1.latitude, p2.longitude, p1.latitude, p3.longitude, p1.latitude, xtd, crs);
}
