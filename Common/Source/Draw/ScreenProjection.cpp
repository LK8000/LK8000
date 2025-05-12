/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#ifdef USE_GLSL
#include <glm/gtc/matrix_transform.hpp>
#endif

ScreenProjection::ScreenProjection() :
    geo_origin(MapWindow::GetPanLatitude(), MapWindow::GetPanLongitude()),
    screen_origin(MapWindow::GetOrigScreen()),
    _Zoom(MapWindow::GetDrawScale()),
    _Angle(MapWindow::GetDisplayAngle()),
    _CosLat(std::cos(MapWindow::GetPanLatitude() * DEG_TO_RAD)),
    _CosAngle(ifastcosine(MapWindow::GetDisplayAngle())),
    _SinAngle(ifastsine(MapWindow::GetDisplayAngle()))
{
}

double ScreenProjection::GetPixelSize() const {
    /* We want the min(width, height) but : 
     *   at equator, width & height of pixel in geographic coordinate are the same
     *   and in all other place height is smaller than width.
     * so no we only need to calculate width
     */
    const GeoPoint right = ToGeoPoint(screen_origin + RasterPoint(1,0));
    return geo_origin.Distance(right); // pixel width in meter
}

void ScreenProjection::Screen2LonLat(const POINT& pt, double &Lon, double &Lat) const {
    using scalar_type = decltype(POINT::x); // type depends of platform.
    const scalar_type sx = pt.x - screen_origin.x;
    const scalar_type sy = pt.y - screen_origin.y;

    Lat = geo_origin.latitude - ((sy * _CosAngle + sx * _SinAngle + 512) / 1024.) / _Zoom;
    Lon = geo_origin.longitude + ((sx * _CosAngle - sy * _SinAngle + 512) / 1024.) / _CosLat / _Zoom;
}

bool ScreenProjection::operator!=(const ScreenProjection& _Proj) const {
    if ( _Zoom != _Proj._Zoom 
            || screen_origin != _Proj.screen_origin 
            || fabs(_Angle - _Proj._Angle) >= 0.5 ) 
    {
        return true;
    }

    double offset = geo_origin.Distance(_Proj.geo_origin);
    return (offset >= GetPixelSize());
}

#ifdef USE_GLSL

glm::mat4 ScreenProjection::ToGLM() const {
  // Precompute cosine for geo_origin latitude
  auto cos_lat = glm::cos(glm::radians<GLfloat>(geo_origin.latitude));
  auto angle = glm::radians<GLfloat>(_Angle);

  // 1. Translate to screen_origin
  glm::mat4 matrix = glm::translate(glm::mat4(1.0), glm::vec3(screen_origin.x, screen_origin.y, 0.0f));
  // 2. Rotate by -_Angle around Z
  matrix = glm::rotate(matrix, angle, glm::vec3(0.0f, 0.0f, -1.0f));
  // 3. Scale longitude by cos(latitude) and apply zoom
  matrix = glm::scale(matrix, glm::vec3(_Zoom * cos_lat, -_Zoom, 1.0f));
  // 4. Move geo_origin to (0,0)
  matrix = glm::translate(matrix, glm::vec3(-geo_origin.longitude, -geo_origin.latitude, 0.0f));

  return matrix;
}

#endif
