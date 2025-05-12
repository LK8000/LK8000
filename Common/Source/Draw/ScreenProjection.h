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

class ScreenProjection final {
public:
    ScreenProjection();

    inline
    RasterPoint ToRasterPoint(double lat, double lon) const {
        return ToScreen<RasterPoint>({lat, lon});
    }

    template <typename ScreenPoint>
    ScreenPoint ToScreen(const GeoPoint& pt) const {
        using scalar_type = decltype(ScreenPoint::x);

        assert(!Overflow<scalar_type>(pt)); // undefined result

        const GeoPoint diff = (geo_origin - pt) * _Zoom;

        const auto Y = to_scalar<scalar_type>(diff.latitude);
        const auto X = to_scalar<scalar_type>(diff.longitude * _CosLat);

        return {
            static_cast<scalar_type>(screen_origin.x - (X * _CosAngle - Y * _SinAngle + 512) / 1024),
            static_cast<scalar_type>(screen_origin.y + (Y * _CosAngle + X * _SinAngle + 512) / 1024)
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

protected:
    double GetPixelSize() const;

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

    short _CosAngle;
    short _SinAngle;

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

template<typename ScreenPoint>
struct GeoToScreen final {
    explicit GeoToScreen(const ScreenProjection& Proj) : _Proj(Proj) {}

    GeoToScreen(const GeoToScreen&) = delete;
    GeoToScreen(GeoToScreen&&) = delete;

    ScreenPoint operator()(double lat, double lon) const {
        return _Proj.ToScreen<ScreenPoint>({lat, lon});
    }

    ScreenPoint operator()(const pointObj& pt) const {
        return _Proj.ToScreen<ScreenPoint>({pt.y, pt.x});
    }

    ScreenPoint operator()(const GeoPoint& pt) const {
        return _Proj.ToScreen<ScreenPoint>(pt);
    }

    ScreenPoint operator()(const CPoint2D& pt) const {
        return _Proj.ToScreen<ScreenPoint>({pt.Latitude(), pt.Longitude()});
    }

    const ScreenProjection& _Proj;
};

#endif	/* SCREENPROJECTION_H */
