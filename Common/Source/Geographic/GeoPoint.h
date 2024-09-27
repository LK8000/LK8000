/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GeoPoint.h
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 12:59 AM
 */

#ifndef _GEOGRAPHIC_GEOPOINT_H_
#define _GEOGRAPHIC_GEOPOINT_H_

#include <type_traits>
#include "NavFunctions.h"

struct GeoPoint {

    GeoPoint() = default;
    
    constexpr GeoPoint(double lat, double lon) 
            : latitude(lat), longitude(lon) {}

    [[nodiscard]]
    GeoPoint Direct(double bearing, double distance) const {
      GeoPoint Out;
      FindLatitudeLongitude(latitude, longitude, bearing, distance,
                            &Out.latitude, &Out.longitude);
      return Out;
    }

    void Reverse(const GeoPoint& point, double& bearing, double& distance) const {
      DistanceBearing(latitude, longitude, point.latitude, point.longitude,
                      &distance, &bearing);
    }

    [[nodiscard]]
    double Distance(const GeoPoint& point) const {
      double d;
      DistanceBearing(latitude, longitude, point.latitude, point.longitude,
                      &d, nullptr );
      return d;
    }

    [[nodiscard]]
    double Bearing(const GeoPoint& point) const {
      double d;
      DistanceBearing(latitude, longitude, point.latitude, point.longitude, nullptr, &d);
      return d;
    }

    bool operator== (const GeoPoint &point) const {
        return (longitude == point.longitude && latitude == point.latitude);
    }

    bool operator != (const GeoPoint& point) const {
      return !(*this == point);
    }

    GeoPoint operator- (const GeoPoint &point) const {
        return {
            latitude - point.latitude, 
            longitude - point.longitude
        };
    }

    GeoPoint operator* (double value) const {
        return {
            latitude * value, 
            longitude * value
        };
    }

    double latitude;
    double longitude;
};

static_assert(std::is_trivial<GeoPoint>::value, "type is not trivial");

struct AGeoPoint: public GeoPoint {

    AGeoPoint() = default;

    constexpr AGeoPoint(const GeoPoint& p, double alt)
            : GeoPoint(p), altitude(alt) {}

    double altitude;
};

static_assert(std::is_trivial<AGeoPoint>::value, "type is not trivial");

inline
double ProjectedDistance(const GeoPoint& p1, const GeoPoint& p2, const GeoPoint& p3,
                         double *xtd, double *crs) {

  return ProjectedDistance(p1.longitude, p1.latitude,
                           p2.longitude, p1.latitude,
                           p3.longitude, p1.latitude,
                           xtd, crs);
}

double CrossTrackError(const GeoPoint& from, const GeoPoint& to, const GeoPoint& current);

#endif /* _GEOGRAPHIC_GEOPOINT_H_ */
