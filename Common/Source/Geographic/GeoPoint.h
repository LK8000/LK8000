/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GeoPoint.h
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 12:59 AM
 */

#ifndef GEOGRAPHIC_GEOPOINT_H
#define GEOGRAPHIC_GEOPOINT_H

#include "NavFunctions.h"

class GeoPoint {
public:
    GeoPoint() {}
    explicit GeoPoint(double lat, double lon) : latitude(lat), longitude(lon) {}
    ~GeoPoint() {}

    GeoPoint Direct(double bearing, double distance) const {
      GeoPoint Out;
      FindLatitudeLongitude(latitude, longitude, bearing, distance, &Out.latitude, &Out.longitude);
      return Out;
    }

    double latitude;
    double longitude;
};

#endif /* GEOGRAPHIC_GEOPOINT_H */
