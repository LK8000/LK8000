/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ScreenProjection.h
 * Author: Bruno de Lacheisserie
 *
 * Created on December 30, 2015, 1:32 AM
 */

#ifndef SCREENPROJECTION_H
#define	SCREENPROJECTION_H

#include "Screen/Point.hpp"
#include "Math/Point2D.hpp"
#include "Geographic/GeoPoint.h"
#ifdef USE_GLSL
#include <glm/fwd.hpp>
#endif

/**
 * @class ScreenProjection
 * @brief Local equirectangular map projection with rotation and zoom.
 *
 * This class provides bidirectional conversion between geographic
 * coordinates (latitude/longitude in degrees) and screen coordinates
 * (pixels). It implements a *local tangent-plane* equirectangular
 * projection centered on a geographic origin, with configurable zoom
 * and rotation.
 *
 * The projection is designed to supports both floating-point and fixed-point
 * arithmetic.
 *
 * ----------------------------------------------------------------------------
 * Projection model (forward transform)
 * ----------------------------------------------------------------------------
 *
 * Given:
 *   geo_origin = (lat₀, lon₀)      // projection center
 *   screen_origin = (x₀, y₀)       // screen center in pixels
 *   Zoom = pixels per degree
 *   Angle = display rotation (degrees, clockwise)
 *
 * Forward transform (ToScreen):
 *   dLon = (lon₀ - lon)
 *   dLat = (lat₀ - lat)
 *   X = dLon * cos(lat₀) * Zoom
 *   Y = dLat * Zoom
 *
 *   // Apply rotation and screen translation
 *   x = x₀ - (X * cos(Angle) - Y * sin(Angle))
 *   y = y₀ + (Y * cos(Angle) + X * sin(Angle))
 *
 * Inverse transform (Screen2LonLat):
 *   dx = -(x - x₀)
 *   dy =  (y - y₀)
 *   X = dx * cos(Angle) + dy * sin(Angle)
 *   Y = dy * cos(Angle) - dx * sin(Angle)
 *
 *   lat = lat₀ - Y / Zoom
 *   lon = lon₀ - X / (Zoom * cos(lat₀))
 *
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * - This is a *local* equirectangular projection (not true Web Mercator).
 * - Accurate for small regions around geo_origin (typically <200 km).
 * - Cosine of latitude is precomputed for speed (_CosLat).
 * - Rotation and zoom are precomputed for both float and fixed-point use.
 * - Perfect round-trip consistency between ToScreen() and ToGeoPoint().
 */

class ScreenProjection final {
public:
    ScreenProjection() = delete;
    ScreenProjection(const GeoPoint& geo, const RasterPoint& screen, double zoom, double angle);

    inline
    RasterPoint ToRasterPoint(double lat, double lon) const;

    template <typename ScreenPoint>
    inline
    ScreenPoint ToScreen(const GeoPoint& pt) const {
        using scalar_type = decltype(ScreenPoint::x);

        assert(!Overflow<scalar_type>(pt)); // undefined result

        const GeoPoint diff = (geo_origin - pt) * _Zoom;

        const auto Y = to_scalar<scalar_type>(diff.latitude);
        const auto X = to_scalar<scalar_type>(diff.longitude * _CosLat);

        return {
            static_cast<scalar_type>(screen_origin.x - (X * _CosAngle - Y * _SinAngle)),
            static_cast<scalar_type>(screen_origin.y + (Y * _CosAngle + X * _SinAngle))
        };
    }

    inline
    GeoPoint ToGeoPoint(const RasterPoint& pt) const {
        GeoPoint geoPoint;
        Screen2LonLat(pt, geoPoint.longitude, geoPoint.latitude);
        return geoPoint;
    }

    void Screen2LonLat(const POINT& pt, double &Lon, double &Lat) const;

    bool operator!=(const ScreenProjection& _Proj) const;

#ifdef USE_GLSL
    glm::mat4 ToGLM() const;
#endif

    double GetPixelSize() const;

protected:
    /* geographic center of projection
     * usually aircraft position in wgs84 geographic coordinate
     */
    GeoPoint geo_origin;  

    /*
     * position of geographic projection center on screen. 
     */
    RasterPoint screen_origin;

    double _Zoom;
    double _Angle;
    double _CosLat;

    double _CosAngle;
    double _SinAngle;

    // Fixed point optimization
    int64_t _CosLatZoom;
    int64_t _CosAngle_fix;
    int64_t _SinAngle_fix;
    static constexpr int32_t fixed_shift = 18;

private:

    template<typename scalar_type>
    using integral_t = std::enable_if_t<std::is_integral_v<scalar_type>, scalar_type>;

    template<typename scalar_type>
    using floating_point_t = std::enable_if_t<std::is_floating_point_v<scalar_type>, scalar_type>;

    // helper to convert double to scalar type
    template<typename scalar_type>
    static floating_point_t<scalar_type> to_scalar(double value) {
        return value;
    }

    template<typename scalar_type>
    static integral_t<scalar_type> to_scalar(double value) {
        return lround(value);
    }

    /**
     * @return true if geo to screen projection overflow the screen point coordinate type
     *   that can happen with OpenGL ES renderer because screen coordinate is stored in int16_t
     *   to avoid that, object outside screen bounding box must be clipped/filtered in
     *   geographic coordinate before, ( or must be draw using FloatPoint instead of RasterPoint)
     *
     * this is specialization for integral type only.
     */
    template<typename scalar_type, integral_t<scalar_type>* = nullptr>
    bool Overflow(const GeoPoint& pt) const {
        using numeric_limits = std::numeric_limits<scalar_type>;
        FloatPoint screen_point = ToScreen<FloatPoint>(pt);
        return (screen_point.x < static_cast<FloatPoint::scalar_type>(numeric_limits::min())
             || screen_point.x > static_cast<FloatPoint::scalar_type>(numeric_limits::max())
             || screen_point.y < static_cast<FloatPoint::scalar_type>(numeric_limits::min())
             || screen_point.y > static_cast<FloatPoint::scalar_type>(numeric_limits::max()));
    }

    /*
     * this is specialization for floating point type to avoid recursive call.
     */
    template<typename scalar_type, floating_point_t<scalar_type>* = nullptr>
    bool Overflow(const GeoPoint& pt) const {
        return false;
    }
};

// Specialization for RasterPoint for fixed-point optimization
template <>
inline
RasterPoint ScreenProjection::ToScreen<RasterPoint>(const GeoPoint& pt) const {
    // Fixed-point optimized version for non-FPU targets
    // Floating-point reference:
    //   X = (geo_origin.longitude - pt.longitude) * _Zoom * _CosLat
    //   Y = (geo_origin.latitude - pt.latitude) * _Zoom
    
    // Calculate Y directly (no cosine correction)
    const int64_t Y = lround((geo_origin.latitude - pt.latitude) * _Zoom);
    
    // Calculate X using precomputed _CosLatZoom = _CosLat * _Zoom * (1 << fixed_shift)
    const double diff_lon = geo_origin.longitude - pt.longitude;
    const auto diff_lon_fixed = static_cast<int64_t>(diff_lon * (1LL << fixed_shift));
    const int64_t X = (diff_lon_fixed * _CosLatZoom) >> (fixed_shift * 2);
    
    // Apply rotation with fixed-point trig
    const auto rotated_x = static_cast<int32_t>((X * _CosAngle_fix - Y * _SinAngle_fix) >> fixed_shift);
    const auto rotated_y = static_cast<int32_t>((Y * _CosAngle_fix + X * _SinAngle_fix) >> fixed_shift);
    
    return {
        static_cast<PixelScalar>(screen_origin.x - rotated_x),
        static_cast<PixelScalar>(screen_origin.y + rotated_y)
    };
}

inline
RasterPoint ScreenProjection::ToRasterPoint(double lat, double lon) const {
    return ToScreen<RasterPoint>({lat, lon});
}


/**
 * @brief Generic geographic-to-screen conversion functor.
 *
 * A callable adapter that provides a
 * templated `ToScreen<ScreenPoint>(GeoPoint)` method.
 *
 * Example usage:
 *   GeoToScreen<RasterPoint> toScreen(projection);
 *   RasterPoint rp = toScreen(lat, lon);
 *
 * @tparam ScreenPoint  The target screen coordinate type (e.g. RasterPoint, FloatPoint).
 * @tparam Projection    Projection class type (defaults to ScreenProjection).
 */
template <typename ScreenPoint>
struct GeoToScreen final {

    explicit GeoToScreen(const ScreenProjection& proj) noexcept : _proj(proj) {}

    GeoToScreen(const GeoToScreen&) = default;
    GeoToScreen(GeoToScreen&&) noexcept = default;
    GeoToScreen& operator=(const GeoToScreen&) = default;
    GeoToScreen& operator=(GeoToScreen&&) noexcept = default;

    /**
     * @brief Converts latitude/longitude (degrees) to screen coordinates.
     */
    inline ScreenPoint operator()(double lat, double lon) const noexcept {
        return _proj.template ToScreen<ScreenPoint>({lat, lon});
    }

    /**
     * @brief Converts a generic point type with x/y fields (lon/lat order).
     *        Example: pointObj from MapServer.
     */
    template <typename T>
    inline ScreenPoint operator()(const T& pt,
                           decltype(std::declval<T>().x)* = nullptr,
                           decltype(std::declval<T>().y)* = nullptr) const noexcept {
        return _proj.ToScreen<ScreenPoint>({pt.y, pt.x});
    }

    /**
     * @brief Converts from a GeoPoint (lat/lon fields).
     */
    inline ScreenPoint operator()(const GeoPoint& pt) const noexcept {
        return _proj.ToScreen<ScreenPoint>(pt);
    }

    /**
     * @brief Converts from a class exposing Latitude() and Longitude() accessors.
     *        Example: CPoint2D.
     */
    template <typename T>
    inline ScreenPoint operator()(const T& pt,
                           decltype(std::declval<T>().Latitude())* = nullptr,
                           decltype(std::declval<T>().Longitude())* = nullptr) const noexcept {
        return _proj.ToScreen<ScreenPoint>(
            {pt.Latitude(), pt.Longitude()});
    }

private:
    const ScreenProjection& _proj;
};

#endif	/* SCREENPROJECTION_H */
