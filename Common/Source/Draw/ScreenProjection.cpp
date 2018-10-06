/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ScreenProjection.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on December 30, 2015, 1:32 AM
 */

#include "externs.h"
#include "MathFunctions.h"
#include "ScreenProjection.h"
#include "NavFunctions.h"

ScreenProjection::ScreenProjection() :
    _PanLat(MapWindow::GetPanLatitude()),
    _PanLon(MapWindow::GetPanLongitude()),
    _Zoom(MapWindow::GetDrawScale()),
    _Angle(MapWindow::GetDisplayAngle()),
    _Origin(MapWindow::GetOrigScreen()),
    _CosAngle(ifastcosine(MapWindow::GetDisplayAngle())),
    _SinAngle(ifastsine(MapWindow::GetDisplayAngle()))
{    
}

double ScreenProjection::GetPixelSize() const {
    double lon0, lat0, lon1, lat1, dlon, dlat;
    
    Screen2LonLat(_Origin, lon0, lat0);

    Screen2LonLat({_Origin.x+1,_Origin.y}, lon1, lat1);
    DistanceBearing(lat0, lon0, lat1, lon1, &dlon, NULL);

    Screen2LonLat({ _Origin.x, _Origin.y+1 }, lon1, lat1);
    DistanceBearing(lat0, lon0, lat1, lon1, &dlat, NULL);

    return std::min(dlon, dlat);
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
    if ( _Zoom != _Proj._Zoom 
            || _Origin != _Proj._Origin 
            || fabs(_Angle - _Proj._Angle) >= 0.5 ) 
    {
        return true;
    }

    double offset;
    DistanceBearing(_PanLat, _PanLon, _Proj._PanLat, _Proj._PanLon, &offset, NULL);

    return (offset >= GetPixelSize());
}
