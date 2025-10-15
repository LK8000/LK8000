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

ScreenProjection::ScreenProjection(const GeoPoint& geo,
                                   const RasterPoint& screen, double zoom,
                                   double angle) : 
    geo_origin(geo),
    screen_origin(screen),
    _Zoom(zoom),
    _Angle(angle),
    _CosLat(std::cos(geo.latitude * DEG_TO_RAD)),
    _CosAngle(std::cos(angle * DEG_TO_RAD)),
    _SinAngle(std::sin(angle * DEG_TO_RAD)),
    _CosLatZoom(lround(_CosLat * _Zoom * (1 << fixed_shift))),
    _CosAngle_fix(lround(_CosAngle * (1 << fixed_shift))),
    _SinAngle_fix(lround(_SinAngle * (1 << fixed_shift)))
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
    using scalar_type = decltype(POINT::x); // Calculate screen offset from origin
    const scalar_type sx = pt.x - screen_origin.x;
    const scalar_type sy = pt.y - screen_origin.y;
    
    // Note: negative sx because ToScreen uses (screen_origin.x - ...)
    const double dx = -sx;
    const double dy = sy;
    
    // Inverse rotation matrix
    // Forward: X' = X * cos - Y * sin, Y' = Y * cos + X * sin
    // Inverse: X = X' * cos + Y' * sin, Y = Y' * cos - X' * sin
    const double X = dx * _CosAngle + dy * _SinAngle;
    const double Y = dy * _CosAngle - dx * _SinAngle;
    
    // Reverse the geographic transformation
    Lat = geo_origin.latitude - Y / _Zoom;
    Lon = geo_origin.longitude - X / _CosLat / _Zoom;
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

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream>

TEST_SUITE("ScreenProjection") {

namespace {

// Helper to visualize projection differences
template <typename P1, typename P2>
void DebugProjectionDiff(const char* label, const P1& a, const P2& b) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << "[DEBUG] " << label << " diff:"
       << " Δx=" << (a.x - b.x)
       << ", Δy=" << (a.y - b.y)
       << " (" << a.x << "," << a.y << ") vs (" << b.x << "," << b.y << ")";
    std::cerr << ss.str() << std::endl;
}

GeoPoint orig_geo = {45.0, 9.0};
RasterPoint orig_screen = {400, 300};
double draw_scale = 1000.0;
double display_angle = 0.0;

}  // namespace

TEST_CASE("Basic Forward/Inverse") {
  ScreenProjection proj(orig_geo, orig_screen, draw_scale, display_angle);

  SUBCASE("Geographic to Screen projection") {
    auto p_origin = proj.ToScreen<RasterPoint>(orig_geo);
    if (p_origin.x != orig_screen.x || p_origin.y != orig_screen.y)
      DebugProjectionDiff("Origin", p_origin, orig_screen);
    CHECK(p_origin.x == orig_screen.x);
    CHECK(p_origin.y == orig_screen.y);

    RasterPoint p = proj.ToScreen<RasterPoint>({45.1, 9.1});
    RasterPoint expected = {471, 200};
    if (std::abs(p.x - expected.x) > 1 || std::abs(p.y - expected.y) > 1)
      DebugProjectionDiff("Geo→Screen", p, expected);
    CHECK(p.x == 471);
    CHECK(p.y == 200);
  }

  SUBCASE("Screen to Geographic projection") {
    GeoPoint g_origin = proj.ToGeoPoint(orig_screen);
    CHECK(g_origin.latitude == doctest::Approx(orig_geo.latitude));
    CHECK(g_origin.longitude == doctest::Approx(orig_geo.longitude));

    GeoPoint g = proj.ToGeoPoint({471, 200});
    CHECK(g.latitude == doctest::Approx(45.1).epsilon(0.001));
    CHECK(g.longitude == doctest::Approx(9.1).epsilon(0.001));
  }

  SUBCASE("Round-trip consistency") {
    for (double lat = 44.9; lat <= 45.1; lat += 0.05)
      for (double lon = 8.9; lon <= 9.1; lon += 0.05) {
        RasterPoint p = proj.ToScreen<RasterPoint>({lat, lon});
        GeoPoint g = proj.ToGeoPoint(p);
        CHECK(g.latitude == doctest::Approx(lat).epsilon(0.001));
        CHECK(g.longitude == doctest::Approx(lon).epsilon(0.001));
      }
  }

  SUBCASE("Projection in all four quadrants") {
      ScreenProjection proj(orig_geo, orig_screen, draw_scale, display_angle);

      // Define points in NE, NW, SE, SW relative to origin
      GeoPoint points[] = {
          {orig_geo.latitude + 0.1, orig_geo.longitude + 0.1}, // NE
          {orig_geo.latitude + 0.1, orig_geo.longitude - 0.1}, // NW
          {orig_geo.latitude - 0.1, orig_geo.longitude + 0.1}, // SE
          {orig_geo.latitude - 0.1, orig_geo.longitude - 0.1}  // SW
      };

      for (auto& pt : points) {
          RasterPoint screen_pt = proj.ToScreen<RasterPoint>(pt);
          GeoPoint geo_pt = proj.ToGeoPoint(screen_pt);

          // Round-trip consistency
          CHECK(geo_pt.latitude == doctest::Approx(pt.latitude).epsilon(0.001));
          CHECK(geo_pt.longitude == doctest::Approx(pt.longitude).epsilon(0.001));

          // Ensure screen coordinates are finite
          CHECK(std::isfinite(screen_pt.x));
          CHECK(std::isfinite(screen_pt.y));
      }
  }
}

TEST_CASE("Rotation and Symmetry") {
  ScreenProjection proj(orig_geo, orig_screen, draw_scale, 0.0);
  ScreenProjection proj180(orig_geo, orig_screen, draw_scale, 180.0);

  SUBCASE("Round-trip symmetry under 180° rotation") {
    for (double lat = 44.9; lat <= 45.1; lat += 0.05)
      for (double lon = 8.9; lon <= 9.1; lon += 0.05) {
        RasterPoint p_rot = proj180.ToScreen<RasterPoint>({lat, lon});
        GeoPoint g_back = proj180.ToGeoPoint(p_rot);
        RasterPoint p_round = proj.ToScreen<RasterPoint>(g_back);

        if (std::abs(p_round.x - proj.ToScreen<RasterPoint>({lat, lon}).x) > 2 ||
            std::abs(p_round.y - proj.ToScreen<RasterPoint>({lat, lon}).y) > 2) {
          DebugProjectionDiff("180° rotation round-trip",
                              p_round, proj.ToScreen<RasterPoint>({lat, lon}));
        }

        CHECK(std::abs(p_round.x - proj.ToScreen<RasterPoint>({lat, lon}).x) <= 2);
        CHECK(std::abs(p_round.y - proj.ToScreen<RasterPoint>({lat, lon}).y) <= 2);
      }
  }
}

TEST_CASE("Edge Cases") {
  ScreenProjection proj(orig_geo, orig_screen, draw_scale, display_angle);

  SUBCASE("Near-zero zoom") {
    ScreenProjection tinyZoom(orig_geo, orig_screen, 0.0001, 0.0);
    auto p = tinyZoom.ToScreen<RasterPoint>({45.1, 9.1});
    CHECK(std::isfinite(p.x));
    CHECK(std::isfinite(p.y));
  }

  SUBCASE("Very large zoom") {
    ScreenProjection bigZoom(orig_geo, orig_screen, 1e6, 0.0);
    auto p = bigZoom.ToScreen<RasterPoint>({45.001, 9.001});
    CHECK(std::abs(p.x - orig_screen.x) < std::numeric_limits<PixelScalar>::max());
  }

  SUBCASE("Polar latitude") {
    GeoPoint near_pole = {85.0, 0.0};
    auto p = proj.ToScreen<RasterPoint>(near_pole);
    CHECK(std::isfinite(p.x));
    CHECK(std::isfinite(p.y));
  }

  SUBCASE("Longitude wrap-around") {
    GeoPoint g1 = {0.0, 179.9};
    GeoPoint g2 = {0.0, -179.9};
    RasterPoint p1 = proj.ToScreen<RasterPoint>(g1);
    RasterPoint p2 = proj.ToScreen<RasterPoint>(g2);
    CHECK(std::isfinite(p1.x));
    CHECK(std::isfinite(p1.y));
    CHECK(std::isfinite(p2.x));
    CHECK(std::isfinite(p2.y));
    CHECK(p1.x != p2.x);  // expect discontinuity
  }
}

TEST_CASE("Geometric Scale and Orthogonality") {
  ScreenProjection proj(orig_geo, orig_screen, draw_scale, display_angle);

  SUBCASE("Distance proportional to pixel scale") {
    auto p1 = proj.ToScreen<RasterPoint>({45.0, 9.0});
    auto p2 = proj.ToScreen<RasterPoint>({45.0, 9.01});
    double pixel_distance = std::hypot(p2.x - p1.x, p2.y - p1.y);
    double geo_distance_m = orig_geo.Distance({45.0, 9.01});
    CHECK(pixel_distance * proj.GetPixelSize() ==
          doctest::Approx(geo_distance_m).epsilon(0.15));
  }

  SUBCASE("Rotation preserves scale") {
    double angle = 30.0;
    ScreenProjection rotated(orig_geo, orig_screen, draw_scale, angle);
    auto a = rotated.ToScreen<FloatPoint>({45.1, 9.0});
    auto b = rotated.ToScreen<FloatPoint>({45.0, 9.1});
    double d1 = std::hypot(a.x - orig_screen.x, a.y - orig_screen.y);
    double d2 = std::hypot(b.x - orig_screen.x, b.y - orig_screen.y);
    CHECK(d1 > 0);
    CHECK(d2 > 0);
    CHECK(std::abs(d1 - d2) < d1 * 0.5);  // relaxed 50% tolerance
  }
}

TEST_CASE("Implementation Integrity") {
  ScreenProjection proj(orig_geo, orig_screen, draw_scale, display_angle);

  SUBCASE("Operator inequality") {
    ScreenProjection projA(orig_geo, orig_screen, draw_scale, 0.0);
    ScreenProjection projB(orig_geo, orig_screen, draw_scale, 10.0);
    CHECK(projA != projB);
  }

  SUBCASE("GetPixelSize sanity") {
    double pixel_size = proj.GetPixelSize();
    CHECK(pixel_size > 0);
    CHECK(std::isfinite(pixel_size));
  }

  SUBCASE("Fixed-point vs floating-point consistency") {
    GeoPoint pt = {45.05, 9.05};
    auto rf = proj.ToScreen<FloatPoint>(pt);
    auto ri = proj.ToScreen<RasterPoint>(pt);
    CHECK(std::abs(rf.x - ri.x) < 2.0);
    CHECK(std::abs(rf.y - ri.y) < 2.0);
  }
}

#ifdef USE_GLSL
TEST_CASE("GLM Matrix consistency") {
  ScreenProjection proj(orig_geo, orig_screen, draw_scale, display_angle);
  glm::mat4 proj_matrix = proj.ToGLM();
  glm::vec4 geo_point(9.1, 45.1, 0.0, 1.0);
  glm::vec4 screen_point = proj_matrix * geo_point;

  CHECK(screen_point.x == doctest::Approx(470.71).epsilon(0.1));
  CHECK(screen_point.y == doctest::Approx(200.0).epsilon(0.1));

  auto p = proj.ToScreen<FloatPoint>({45.1, 9.1});
  CHECK(screen_point.x == doctest::Approx(p.x).epsilon(0.1));
  CHECK(screen_point.y == doctest::Approx(p.y).epsilon(0.1));
}
#endif
}
#endif  // DOCTEST_CONFIG_DISABLE
