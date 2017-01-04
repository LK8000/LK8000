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

class ScreenProjection final {
public:
    ScreenProjection();
    ~ScreenProjection();
    
    inline
    RasterPoint LonLat2Screen(double lon, double lat) const {
        const int64_t Y = _lround((_PanLat - lat) * _Zoom);
        const int64_t X = _lround((_PanLon - lon) * fastcosine(lat) * _Zoom);

        return RasterPoint{
            static_cast<RasterPoint::scalar_type>(_Origin.x - (X * _CosAngle - Y * _SinAngle + 512) / 1024),
            static_cast<RasterPoint::scalar_type>(_Origin.y + (Y * _CosAngle + X * _SinAngle + 512) / 1024)
        };        
    }
    
    inline 
    RasterPoint LonLat2Screen(const pointObj& pt) const {
      return LonLat2Screen(pt.x,pt.y);
    }

    inline 
    FloatPoint ToFloatPoint(const pointObj& pt) const {
        typedef FloatPoint::scalar_type scalar_type;

        const scalar_type Y = (_PanLat - pt.y) * _Zoom;
        const scalar_type X = (_PanLon - pt.x) * fastcosine(pt.y) * _Zoom;

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

#endif	/* SCREENPROJECTION_H */

