/* 
 * File:   ScreenProjection.cpp
 * Author: bruno
 * 
 * Created on December 30, 2015, 1:32 AM
 */

#include "externs.h"
#include "MathFunctions.h"
#include "ScreenProjection.h"

ScreenProjection::ScreenProjection() :
    _PanLat(MapWindow::GetPanLatitude()),
    _PanLon(MapWindow::GetPanLongitude()),
    _Zoom(MapWindow::GetDrawScale()),
    _Origin(MapWindow::GetOrigScreen()),
    _CosAngle(ifastcosine(MapWindow::GetDisplayAngle())),
    _SinAngle(ifastsine(MapWindow::GetDisplayAngle()))
{
}

ScreenProjection::~ScreenProjection() {
}

void ScreenProjection::Screen2LonLat(const POINT& pt, double &Lon, double &Lat) const {
    const int sx = pt.x - _Origin.x;
    const int sy = pt.y - _Origin.y;

    Lat = _PanLat - ((sy * _CosAngle + sx * _SinAngle + 512) / 1024) / _Zoom;
    Lon = _PanLon + ((sx * _CosAngle - sy * _SinAngle + 512) / 1024) * invfastcosine(Lat) / _Zoom;
}

bool ScreenProjection::operator!=(const ScreenProjection& _Proj) const {
    return ( _PanLat != _Proj._PanLat
            || _PanLon != _Proj._PanLon
            || _Zoom != _Proj._Zoom
            || _Origin != _Proj._Origin
            || _CosAngle != _Proj._CosAngle
            || _SinAngle != _Proj._SinAngle);
}
