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

struct GeoPoint {

    GeoPoint() = default;
    
    constexpr GeoPoint(double lat, double lon) 
            : latitude(lat), longitude(lon) {}

    GeoPoint Direct(double bearing, double distance) const;

    void Reverse(const GeoPoint& point, double& bearing, double& distance) const;

    double Distance(const GeoPoint& point) const;
    double Bearing(const GeoPoint& point) const;

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


double ProjectedDistance(const GeoPoint p1, const GeoPoint p2, const GeoPoint p3,
                         double *xtd, double *crs);

double CrossTrackError(const GeoPoint& from, const GeoPoint& to, const GeoPoint& current);

#endif /* _GEOGRAPHIC_GEOPOINT_H_ */
