/* 
 * File:   ScreenProjection.h
 * Author: bruno
 *
 * Created on December 30, 2015, 1:32 AM
 */

#ifndef SCREENPROJECTION_H
#define	SCREENPROJECTION_H

#include "Screen/Point.hpp"
#include "Math/Point2D.hpp"
#include "mapprimitive.h"
#include "Geographic/GeoPoint.h"

class ScreenProjection final {
public:
    ScreenProjection();
    ~ScreenProjection();
    
    inline
    RasterPoint ToRasterPoint(double lon, double lat) const {
        const int64_t Y = _lround((_PanLat - lat) * _Zoom);
        const int64_t X = _lround((_PanLon - lon) * fastcosine(lat) * _Zoom);

        return RasterPoint{
            static_cast<RasterPoint::scalar_type>(_Origin.x - (X * _CosAngle - Y * _SinAngle + 512) / 1024),
            static_cast<RasterPoint::scalar_type>(_Origin.y + (Y * _CosAngle + X * _SinAngle + 512) / 1024)
        };        
    }
    
    inline 
    FloatPoint ToFloatPoint(double lon, double lat) const {
        typedef FloatPoint::scalar_type scalar_type;

        const scalar_type Y = (_PanLat - lat) * _Zoom;
        const scalar_type X = (_PanLon - lon) * fastcosine(lat) * _Zoom;

        return FloatPoint{
            static_cast<scalar_type>(_Origin.x - (X * _CosAngle - Y * _SinAngle + 512) / 1024),
            static_cast<scalar_type>(_Origin.y + (Y * _CosAngle + X * _SinAngle + 512) / 1024)
        };        
    }
    
    void Screen2LonLat(const POINT& pt, double &Lon, double &Lat) const;
    
    bool operator!=(const ScreenProjection& _Proj) const; 

protected:
    double GetPixelSize() const;
    
protected:
    double _PanLat;
    double _PanLon;
    double _Zoom;
    double _Angle;
    RasterPoint _Origin;
    short _CosAngle;
    short _SinAngle;
    
    double _PixelSize;
};


template<typename T>
struct GeoToScreen {
    GeoToScreen(const ScreenProjection& Proj) : _Proj(Proj) {}

    template<typename U = T>
    typename std::enable_if<std::is_same<U, RasterPoint>::value, T>::type
    operator()(double lon, double lat) const {
        return _Proj.ToRasterPoint(lon,lat);
    }

    template<typename U = T>
    typename std::enable_if<std::is_same<U, FloatPoint>::value, T>::type
    operator()(double lon, double lat) const {
        return _Proj.ToFloatPoint(lon,lat);
    }
		
    template<typename U = T>
    typename std::enable_if<std::is_same<U, RasterPoint>::value, T>::type
    operator()(const pointObj& pt) const {
        return _Proj.ToRasterPoint(pt.x,pt.y);
    }

    template<typename U = T>
    typename std::enable_if<std::is_same<U, FloatPoint>::value, T>::type
    operator()(const pointObj& pt) const {
        return _Proj.ToFloatPoint(pt.x,pt.y);
    }

    template<typename U = T>
    typename std::enable_if<std::is_same<U, RasterPoint>::value, T>::type
    operator()(const GeoPoint& pt) const {
        return _Proj.ToRasterPoint(pt.longitude, pt.latitude);
    }

    template<typename U = T>
    typename std::enable_if<std::is_same<U, FloatPoint>::value, T>::type
    operator()(const GeoPoint& pt) const {
        return _Proj.ToFloatPoint(pt.longitude, pt.latitude);
    }

    const ScreenProjection& _Proj;
};

#endif	/* SCREENPROJECTION_H */

