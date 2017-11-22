/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Terrain.cpp,v 8.6 2010/12/17 02:02:27 root Exp root $
 */

#include "externs.h"
#include "Terrain.h"
#include "RasterTerrain.h"
#include "Terrain/STScreenBuffer.h"
#include "Terrain/STHeightBuffer.h"
#include "RGB.h"
#include "Multimap.h"
#include "../Draw/ScreenProjection.h"
#include "NavFunctions.h"
#include "ColorRamps.h"
#include "Kobo/Model.hpp"
#include "Util/Clamp.hpp"
#include "Asset.hpp"
#include <utility>
#include <type_traits>
#include <memory>

#if (defined(__ARM_NEON) || defined(__ARM_NEON__))
 #if !GCC_OLDER_THAN(5,0)
  #include <arm_neon.h>
 #else
  /**
   * GCC 4.8 (kobo) & 4.9 (openvario) has the same problem :
   * 
   * ..../include/arm_neon.h:2769:57: internal compiler error: in copy_to_mode_reg, at explow.c:665
   *    return (int16x4_t)__builtin_neon_vmaxv4hi (__a, __b, 1);
   */
  #warning "too old compiler, optimized terrain drawing disabled"
 #endif
#endif

extern bool FastZoom;

extern void rgb_lightness( uint8_t &r, uint8_t &g, uint8_t &b, float light);

namespace {

COLORRAMP tshadow;
COLORRAMP thighlight;

const COLORRAMP (*lastColorRamp)[NUM_COLOR_RAMP_LEVELS] = nullptr;

#ifdef GREYSCALE

Luminosity8 terrain_lightness( const Luminosity8& color, double lightness) {
  if(lightness == 1.0) {
    return color;
  }
  return {static_cast<uint8_t >(std::min<uint32_t>(color.GetLuminosity() * lightness, 255)) };
}

#else

RGB8Color terrain_lightness( const RGB8Color& color, float lightness) {
  if(lightness == 1.0F) {
    return color;
  }

  uint8_t r = color.Red();
  uint8_t g = color.Green();
  uint8_t b = color.Blue();
  rgb_lightness(r, g, b, lightness);
  
  return { r, g, b };
}

#endif

template<typename T>
constexpr bool is_power_of_two(T v) {
  static_assert(std::is_integral<T>::value, "is_power_of_two : T must be integral");
  return v && ((v & (v - 1)) == 0);
}

template<uint32_t level>
static uint8_t linear_interpolation(uint8_t a, uint8_t b, uint32_t f) {
  static_assert(is_power_of_two(level), "linear_interpolation : level must be power of 2");
  const uint32_t of = level - f;

  return (f * b + of * a) / level;
}

template<uint32_t level>
static RGB8Color linear_interpolation(const RGB8Color& a, const RGB8Color& b, uint32_t f) {
  return {
    linear_interpolation<level>(a.Red(), b.Red(), f),
    linear_interpolation<level>(a.Green(), b.Green(), f),
    linear_interpolation<level>(a.Blue(), b.Blue(), f)
  };
}

template<uint32_t level>
static Luminosity8 linear_interpolation(const Luminosity8& a, const Luminosity8& b, unsigned f) {
  return  linear_interpolation<level>(a.GetLuminosity(), b.GetLuminosity(), f);
}

static TerrainColor ColorRampLookup(const int16_t h, const COLORRAMP(&ramp_colors)[NUM_COLOR_RAMP_LEVELS]) {

  constexpr uint32_t interp_level = 1<<16;

  const auto begin = std::begin(ramp_colors);
  const auto end = std::end(ramp_colors);

  TerrainColor color;

  // find first value with height > h
  auto it = std::upper_bound(begin, end, h, [](int16_t height, const COLORRAMP& v){
    return height < v.height;
  });

  if(it != begin) { 
    auto prev = std::prev(it);
    if(it != end && it->height != prev->height) {
      // interpolate color
      const uint32_t f = (h - prev->height) * interp_level / (it->height - prev->height);
      color = linear_interpolation<interp_level>(prev->color, it->color, f);
    } else {
      color = prev->color; // last defined color or no need to interpolate
    }
  } else {
    color = begin->color; // first defined color
  }

  return color;
}

template<class T>
inline T TerrainShading(const int16_t illum, const T& color) {

  constexpr uint32_t interp_level = 128;

  if (illum < 0) {
    // shadow to "tshadow.color"
    const uint32_t x = std::min<uint32_t>(tshadow.height, -illum);
    return linear_interpolation<interp_level>(color, tshadow.color, x);
  } else if (illum > 0) {
    // highlight "thighlight.color"
    if (thighlight.height != 255) {
      const uint32_t x = std::min<uint32_t>(thighlight.height, illum / 2);
      return linear_interpolation<interp_level>(color, thighlight.color, x);
    }
  }
  return color;
}

} // namespace

// map scale is approximately 2 points on the grid
// therefore, want one to one mapping if mapscale is 0.5
// there are approx 30 pixels in mapscale
// 240/DTQUANT resolution = 6 pixels per terrain
// (mapscale/30)  km/pixels
//        0.250   km/terrain
// (0.25*30/mapscale) pixels/terrain
//  mapscale/(0.25*30)
//  mapscale/7.5 terrain units/pixel
//
// this is for TerrainInfo.StepSize = 0.0025;

//
// Returning from constructor without setting terrain_ready will result in no draw terrain.
//
class TerrainRenderer {
    TerrainRenderer(const TerrainRenderer &) = delete; // disallowed
    TerrainRenderer &operator=(const TerrainRenderer &) = delete; // disallowed
    TerrainRenderer(TerrainRenderer &&) = delete; // disallowed
    TerrainRenderer &operator=(TerrainRenderer &&) = delete; // disallowed
public:

    explicit TerrainRenderer(const RECT& rc) {
        TestLog(_T(".... Init TerrainRenderer area LTRB (%d,%d,%d,%d)"), (int)rc.left, (int)rc.top, (int)rc.right, (int)rc.bottom);

        static bool error = false;
        // This will not disable terrain! So we shall get calling here again, but no problem.
        if (rc.right < 1 || rc.bottom < 1) {
            if(!error) {
                // log error only once
                StartupStore(_T(". CRITICAL PROBLEM, cannot render terrain. rcright=%d rcbottom=%d%s"),rc.right,rc.bottom,NEWLINE);
            }
            error = true;
            LKASSERT(0); // THIS WILL NOT POP UP A DIALOG ERROR!
            return;
        }
        error = false;

        dtquant = TerrainQuantization();
        LKASSERT(dtquant>=1);

#ifdef USE_TERRAIN_BLUR
        blursize = std::max(0U, (dtquant - 1U) / 2U); // always 0
#endif

        /*
        screen_size  dtq  blur  res_x  res_y terrain_pixels
          320x240     1    0    320    240      76800      
          640x480     2    0    320    240      76800      
          960x720     3    1    320    240      76800      
        */
        try {

            const int res_x = iround((rc.right - rc.left) / dtquant);
            const int res_y = iround((rc.bottom - rc.top) / dtquant);

            screen_buffer = std::make_unique<CSTScreenBuffer>(res_x, res_y);

            const size_t ixs = screen_buffer->GetCorrectedWidth();
            const size_t iys = screen_buffer->GetHeight();

            height_buffer = std::make_unique<CSTHeightBuffer>(ixs, iys);

            auto_brightness = 218;

            // Reset this, so ColorTable will reload colors
            lastColorRamp = nullptr;
            last_height_scale = 0;
            last_realscale = 0;

        } catch (std::bad_alloc& e) {

            screen_buffer = nullptr;
            height_buffer = nullptr;

            const tstring error = to_tstring(e.what());
            StartupStore(_T("TerrainRenderer : %s"), error.c_str());
            OutOfMemory(_T(__FILE__), __LINE__);
            ToggleMultimapTerrain();

            throw e; // forward exception to caller...
        }
    }

    ~TerrainRenderer() {
        TestLog(_T(".... Deinit TerrainRenderer"));
    }

    void SetDirty() {
        _dirty = true;
    }

    bool IsDirty() const {
        return _dirty;
    }

    short get_brightness() const {
        return auto_brightness;
    }

private:
    bool _dirty = true; // indicate screen_buffer is up-to-date

    unsigned int dtquant;
    unsigned int epx; // step size used for slope calculations

    double pixelsize_d;

#ifdef USE_TERRAIN_BLUR
// only used if blur...
    int blursize;
#endif
    std::unique_ptr<CSTScreenBuffer> screen_buffer;
    std::unique_ptr<CSTHeightBuffer> height_buffer;
    std::unique_ptr<int16_t[]> prev_iso_band;
    std::unique_ptr<int16_t[]> current_iso_band;

    BGRColor color_table[128][256] = {};

    const COLORRAMP (*color_ramp)[NUM_COLOR_RAMP_LEVELS] = {};

    unsigned last_height_scale;
    double last_terrain_whiteness;
    double last_realscale;

    unsigned height_scale; // scale factor  ((height - height_min) << height_scale) must be in [0 - 255] range
    int16_t height_min; // lower height visible terrain
    int16_t height_max; // highter height visible terrain

    short auto_brightness;

public:

    bool DoShading() const {
        assert(height_buffer && height_buffer->GetBuffer());

        const size_t ixs = height_buffer->GetWidth();
        const size_t iys = height_buffer->GetHeight();

        if (epx > min(ixs, iys) / 4U) {
            return false;
        }

        if (MapWindow::zoom.RealScale() > NoShadingScale()) {
            return false;
        }

        return (Shading && terrain_doshading[TerrainRamp]);
    }

    /**
     * Fill height Buffer with according to map projection
     * @offset : {top, left} coordinate of Terrain Rendering Rect relative to DrawRect
     */
    void Height(const RasterPoint& offset, const ScreenProjection& _Proj) {
        assert(height_buffer && height_buffer->GetBuffer());

        ScopeLock Lock(RasterTerrain::mutex);

        RasterMap* DisplayMap = RasterTerrain::TerrainMap.get();
        assert(DisplayMap && DisplayMap->isMapLoaded());
        if(!DisplayMap || !DisplayMap->isMapLoaded()) {
            return;
        }

        const int X0 = dtquant / 2;
        const int Y0 = dtquant / 2;
        const int X1 = X0 + dtquant * height_buffer->GetWidth();
        const int Y1 = Y0 + dtquant * height_buffer->GetHeight();

        const RasterPoint ScreenCenter = {
                (X0 + X1) / 2,
                (Y0 + Y1) / 2
        };
        const GeoPoint GeoCenter = _Proj.ToGeoPoint(ScreenCenter);

        const RasterPoint ScreenNearby = {
                ScreenCenter.x + static_cast<PixelScalar>(dtquant),
                ScreenCenter.y + static_cast<PixelScalar>(dtquant)
        };
        const GeoPoint GeoNearby = _Proj.ToGeoPoint(ScreenNearby);

        pixelsize_d = GeoCenter.Distance(GeoNearby) / 2.0;

        // set resolution
        DisplayMap->SetFieldRounding(std::abs(GeoCenter.longitude - GeoNearby.longitude)/3,
                                     std::abs(GeoCenter.latitude - GeoNearby.latitude)/3);

        epx = DisplayMap->GetEffectivePixelSize(&pixelsize_d, GeoCenter.latitude, GeoCenter.longitude);
        epx = std::max(4u, (epx / 4u ) * 4u); // "epx" must be divisible by 4 for compatibility with ARM NEON vectorized shadding algorithm

        RasterPoint orig = RasterPoint(MapWindow::GetOrigScreen()) - offset;

        if(DisplayMap->interpolate()) {

            FillHeightBuffer(X0 - orig.x, Y0 - orig.y, X1 - orig.x, Y1 - orig.y,
                    [DisplayMap](const double &lat, const double &lon) {
                        return DisplayMap->GetFieldInterpolate(lat,lon);
                    });
        } else {

            FillHeightBuffer(X0 - orig.x, Y0 - orig.y, X1 - orig.x, Y1 - orig.y,
                    [DisplayMap](const double &lat, const double &lon) {
                          return DisplayMap->GetFieldFine(lat,lon);
                    });
        }
    }

    /**
     * Attention ! never call this without check if map is loaded.
     *
     * template avoid to test if interpolation is needed for each pixel.
     */
    template<typename GetHeight_t>
    void FillHeightBuffer(const int X0, const int Y0, const int X1, const int Y1, GetHeight_t GetHeight) {
        // fill the buffer
        assert(height_buffer && height_buffer->GetBuffer());

        const double PanLatitude = MapWindow::GetPanLatitude();
        const double PanLongitude = MapWindow::GetPanLongitude();
        const double InvDrawScale = MapWindow::GetInvDrawScale() / 1024.0;
        const double DisplayAngle = MapWindow::GetDisplayAngle();

        const int cost = ifastcosine(DisplayAngle);
        const int sint = ifastsine(DisplayAngle);

        const double ac2 = sint*InvDrawScale;
        const double ac3 = cost*InvDrawScale;

        height_scale = 0;

        // we need local variable for compatibility with all implementation of opemmp reduction
        int16_t _height_min = std::numeric_limits<int16_t>::max();
        int16_t _height_max = std::numeric_limits<int16_t>::min();

        const size_t col_count = height_buffer->GetWidth();
        const size_t row_count = height_buffer->GetHeight();


#if defined(_OPENMP)
        #pragma omp parallel for reduction(max : _height_max) reduction(min : _height_min)
#endif
        for (size_t iy = 0; iy < row_count; ++iy) {
            const int y = Y0 + (iy*dtquant);
            const double ac1 = PanLatitude - y*ac3;
            const double cc1 = y * ac2;

            int16_t *height_row = height_buffer->GetRow(iy);

            for (size_t ix = 0; ix < col_count; ++ix) {
                const int x = X0 + (ix*dtquant);
                const double Y = ac1 - x*ac2;
                const double X = PanLongitude + (invfastcosine(Y) * ((x * ac3) - cc1));

                int16_t& hDst = height_row[ix];

                /*
                 * Terrain height can be negative.
                 * do not clip height to 0 here, otherwise all height below 0
                 * will be painted like sea if topology does not contains coast_area
                 *
                 * all height will be sifted by #height_min in #TerrainRenderer::Slope method for ColorRamp lookup.
                 */
                hDst = GetHeight(Y, X);

                if(hDst != TERRAIN_INVALID) {
                  _height_min = std::min(_height_min, hDst);
                  _height_max = std::max(_height_max, hDst);
                }
            }
        }
        height_min = _height_min;
        height_max = _height_max;

        if (!terrain_minalt[TerrainRamp]) {
          // if ColorRamp is not relative to min height of visible terrain, we only use negative height_min.
          height_min = std::min<int16_t>(0, height_min);
        }

        if (TerrainRamp == 13) { // GA Relative
            if (!GPS_INFO.NAVWarning) {
                if (CALCULATED_INFO.Flying) {
                    height_min = static_cast<int16_t>(GPS_INFO.Altitude - 150); // 500ft
                } else {
                    height_min = static_cast<int16_t>(GPS_INFO.Altitude + 100); // 330ft
                }
            } else {
                height_min += 150;
            }
        }

        const int16_t height_span = height_max - height_min;
        while((height_span >> height_scale) >= 255) {
          ++height_scale;
        }
    }


#if (defined(__ARM_NEON) || defined(__ARM_NEON__)) && !GCC_OLDER_THAN(5,0)

    // JMW: if zoomed right in (e.g. one unit is larger than terrain
    // grid), then increase the step size to be equal to the terrain
    // grid for purposes of calculating slope, to avoid shading problems
    // (gridding of display) This is why epx is used instead of 1
    // previously.  for large zoom levels, epx=1

    void Slope_shading(const int sx, const int sy, const int sz) {
        assert(height_buffer && height_buffer->GetBuffer());
        assert(screen_buffer && screen_buffer->GetBuffer());

        const size_t ixs = height_buffer->GetWidth();
        const size_t iys = height_buffer->GetHeight();

        const int hscale = std::max<int>(1, pixelsize_d);

        const float32_t p20_left_offset[] = {
                static_cast<float32_t>(epx),
                static_cast<float32_t>(epx + 1),
                static_cast<float32_t>(epx + 2),
                static_cast<float32_t>(epx + 3)
        };
        const float32x4_t v_p20_left_offset = vld1q_f32(p20_left_offset);

        const float32_t p20_right_offset[] = {
                static_cast<float32_t>(epx + ixs - 1),
                static_cast<float32_t>(epx + ixs - 2),
                static_cast<float32_t>(epx + ixs - 3),
                static_cast<float32_t>(epx + ixs - 4)
        };
        const float32x4_t v_p20_right_offset = vld1q_f32(p20_right_offset);

        const int32x4_t mag_0 = vmovq_n_s32(0);
        const int32x4_t mag_127 = vmovq_n_s32(127);

        const int16x4_t height_0 = vmov_n_s16(0);
        const int16x4_t height_255 = vmov_n_s16(255);
        const int16x4_t v_height_min = vmov_n_s16(height_min);
        const int16x4_t v_height_scale = vmov_n_s16(height_scale);

        const float32x4_t v_sx = vmovq_n_f32(sx);
        const float32x4_t v_sy = vmovq_n_f32(sy);
        const float32x4_t v_sz = vmovq_n_f32(sz);

        const unsigned int ixsleft = std::max(4u, epx);
        const unsigned int ixsright = ixs - std::max(4u, epx);

        for(unsigned y = 0; y < iys; y++) {
            BGRColor* screen_row = screen_buffer->GetRow(y);

            const unsigned prev_row_index =  (y < epx) ? 0 : y - epx;
            const unsigned next_row_index =  (y + epx >= iys) ? iys - 1 : y + epx;

            const int16_t* prev_row = height_buffer->GetRow(prev_row_index);
            const int16_t* curr_row = height_buffer->GetRow(y);
            const int16_t* next_row = height_buffer->GetRow(next_row_index);

            const float32_t p31 = next_row_index - prev_row_index;
            const float32_t p31s = p31 * hscale;

            // left side
            {
                const int16x4_t left = vmov_n_s16(*(curr_row));
                for (unsigned int x = 0; x < ixsleft; x+=4) {
                    const int16x4_t up = vld1_s16(prev_row + x);
                    const int16x4_t bottom = vld1_s16(next_row + x);
                    const int16x4_t right = vld1_s16(curr_row + x + epx);

                    const float32x4_t p20 = v_p20_left_offset + (float32_t)x;
                    const float32x4_t p22 = vcvtq_f32_s32(vmovl_s16(right - left));
                    const float32x4_t p32 = vcvtq_f32_s32(vmovl_s16(bottom - up));

                    const float32x4_t dd0 = p22 * p31;
                    const float32x4_t dd1 = p20 * p32;
                    const float32x4_t dd2 = p20 * p31s;

                    const float32x4_t sqr_mag = (dd0 * dd0 + dd1 * dd1 + dd2 * dd2);
                    const float32x4_t inv_mag = vrsqrteq_f32(sqr_mag);
                    const float32x4_t fmag = (dd2 * v_sz + dd0 * v_sx + dd1 * v_sy) * inv_mag;

                    int32x4_t mag = vcvtq_s32_f32(fmag - v_sz);

                    mag = mag + 64;
                    mag = Clamp(mag, mag_0, mag_127);

                    int16x4_t h =  vld1_s16(curr_row + x);
                    h = (h - v_height_min) >> v_height_scale;
                    h = Clamp(h, height_0, height_255);

                    screen_row[x]   = color_table[vgetq_lane_s32(mag,0)][vget_lane_s16(h, 0)];
                    screen_row[x+1] = color_table[vgetq_lane_s32(mag,1)][vget_lane_s16(h, 1)];
                    screen_row[x+2] = color_table[vgetq_lane_s32(mag,2)][vget_lane_s16(h, 2)];
                    screen_row[x+3] = color_table[vgetq_lane_s32(mag,3)][vget_lane_s16(h, 3)];

                    assert(x+3 < ixs);
                }
            }
            // center
            for (unsigned int x = ixsleft; x < ixsright; x+=4) {

                const int16x4_t up = vld1_s16(prev_row + x);
                const int16x4_t bottom = vld1_s16(next_row + x);
                const int16x4_t left = vld1_s16(curr_row + x - epx);
                const int16x4_t right = vld1_s16(curr_row + x + epx);

                const float32_t p20 = epx+epx;
                const float32x4_t p22 = vcvtq_f32_s32(vmovl_s16(right - left));
                const float32x4_t p32 = vcvtq_f32_s32(vmovl_s16(bottom - up));

                const float32x4_t dd0 = p22 * p31;
                const float32x4_t dd1 = p20 * p32;
                const float32_t dd2 = p20 * p31s;

                const float32x4_t sqr_mag = (dd0 * dd0 + dd1 * dd1 + dd2 * dd2);
                const float32x4_t inv_mag = vrsqrteq_f32(sqr_mag);
                const float32x4_t fmag = (dd2 * v_sz + dd0 * v_sx + dd1 * v_sy) * inv_mag;

                int32x4_t mag = vcvtq_s32_f32(fmag - v_sz);

                mag = mag + 64;
                mag = Clamp(mag, mag_0, mag_127);

                int16x4_t h =  vld1_s16(curr_row + x);
                h = (h - height_min) >> height_scale;
                h = Clamp(h, height_0, height_255);

                screen_row[x]   = color_table[vgetq_lane_s32(mag,0)][vget_lane_s16(h, 0)];
                screen_row[x+1] = color_table[vgetq_lane_s32(mag,1)][vget_lane_s16(h, 1)];
                screen_row[x+2] = color_table[vgetq_lane_s32(mag,2)][vget_lane_s16(h, 2)];
                screen_row[x+3] = color_table[vgetq_lane_s32(mag,3)][vget_lane_s16(h, 3)];

                assert(x+3 < ixs);
            }
            // right side
            {
                const int16x4_t right = vmov_n_s16(*(curr_row + ixs - 1));
                for (unsigned int x = ixsright; x < (ixs-3); x+=4) {
                    const int16x4_t up = vld1_s16(prev_row + x);
                    const int16x4_t bottom = vld1_s16(next_row + x);
                    const int16x4_t left = vld1_s16(curr_row + x - epx);

                    const float32x4_t p20 = v_p20_right_offset - (float32_t)x;
                    const float32x4_t p22 = vcvtq_f32_s32(vmovl_s16(right - left));
                    const float32x4_t p32 = vcvtq_f32_s32(vmovl_s16(bottom - up));

                    const float32x4_t dd0 = p22 * p31;
                    const float32x4_t dd1 = p20 * p32;
                    const float32x4_t dd2 = p20 * p31s;

                    const float32x4_t sqr_mag = (dd0 * dd0 + dd1 * dd1 + dd2 * dd2);
                    const float32x4_t inv_mag = vrsqrteq_f32(sqr_mag);
                    const float32x4_t fmag = (dd2 * v_sz + dd0 * v_sx + dd1 * v_sy) * inv_mag;

                    int32x4_t mag = vcvtq_s32_f32(fmag - v_sz);

                    mag = mag + 64;
                    mag = Clamp(mag, mag_0, mag_127);

                    int16x4_t h =  vld1_s16(curr_row + x);
                    h = (h - height_min) >> height_scale;
                    h = Clamp(h, height_0, height_255);

                    screen_row[x]   = color_table[vgetq_lane_s32(mag,0)][vget_lane_s16(h, 0)];
                    screen_row[x+1] = color_table[vgetq_lane_s32(mag,1)][vget_lane_s16(h, 1)];
                    screen_row[x+2] = color_table[vgetq_lane_s32(mag,2)][vget_lane_s16(h, 2)];
                    screen_row[x+3] = color_table[vgetq_lane_s32(mag,3)][vget_lane_s16(h, 3)];

                	assert(x+3 < ixs);
                }
            }
        }
    }

    void Slope() {
        assert(height_buffer && height_buffer->GetBuffer());
        assert(screen_buffer && screen_buffer->GetBuffer());

        const int16x8_t qheight_0 = vmovq_n_s16(0);
        const int16x8_t qheight_255 = vmovq_n_s16(255);
        const int16x8_t qv_height_min = vmovq_n_s16(height_min);
        const int16x8_t qv_height_scale = vmovq_n_s16(height_scale);

        const int16x4_t height_0 = vmov_n_s16(0);
        const int16x4_t height_255 = vmov_n_s16(255);
        const int16x4_t v_height_min = vmov_n_s16(height_min);
        const int16x4_t v_height_scale = vmov_n_s16(height_scale);

        const size_t ixs = height_buffer->GetWidth();
        const size_t iys = height_buffer->GetHeight();

        for (unsigned int y = 0; y < iys; ++y) {
            BGRColor* screen_row = screen_buffer->GetRow(y);
            const int16_t *height_row = height_buffer->GetRow(y);

            // first loop to vectorize using neon quad
            unsigned int x;
            for (x = 0; x < (ixs-8); x+=8) {
                int16x8_t h =  vld1q_s16(height_row + x);
                h = (h - qv_height_min) >> qv_height_scale;
                h = Clamp(h, qheight_0, qheight_255);

                screen_row[x]   = GetColor(vgetq_lane_s16(h, 0));
                screen_row[x+1] = GetColor(vgetq_lane_s16(h, 1));
                screen_row[x+2] = GetColor(vgetq_lane_s16(h, 2));
                screen_row[x+3] = GetColor(vgetq_lane_s16(h, 3));
                screen_row[x+4] = GetColor(vgetq_lane_s16(h, 4));
                screen_row[x+5] = GetColor(vgetq_lane_s16(h, 5));
                screen_row[x+6] = GetColor(vgetq_lane_s16(h, 6));
                screen_row[x+7] = GetColor(vgetq_lane_s16(h, 7));
            }
            // next to vectorize using neon
            for (; x < (ixs-4); x+=4) {
                int16x4_t h =  vld1_s16(height_row + x);
                h = (h - v_height_min) >> v_height_scale;
                h = Clamp(h, height_0, height_255);

                screen_row[x]   = GetColor(vget_lane_s16(h, 0));
                screen_row[x+1] = GetColor(vget_lane_s16(h, 1));
                screen_row[x+2] = GetColor(vget_lane_s16(h, 2));
                screen_row[x+3] = GetColor(vget_lane_s16(h, 3));
            }
            // end without vector
            for (; x < ixs; ++x) {
                int16_t h =  *(height_row + x);
                h = ((unsigned)(h - height_min)) >> height_scale;
                h = Clamp<int16_t>(h, 0, 255);

                screen_row[x]   = GetColor(h);
            }
        }
    }

#else

    void Slope_shading(const int sx, const int sy, const int sz) {
        assert(height_buffer && height_buffer->GetBuffer());
        assert(screen_buffer && screen_buffer->GetBuffer());

        const int hscale = std::max<int>(1, pixelsize_d);

        const size_t ixs = height_buffer->GetWidth();
        const size_t iys = height_buffer->GetHeight();

#if defined(_OPENMP)
        #pragma omp parallel for
#endif
        for(size_t y = 0; y < iys; y++) {
            BGRColor* screen_row = screen_buffer->GetRow(y);

            const unsigned prev_row_index =  (y < epx) ? 0 : y - epx;
            const unsigned next_row_index =  (y + epx >= iys) ? iys - 1 : y + epx;

            const int16_t* prev_row = height_buffer->GetRow(prev_row_index);
            const int16_t* curr_row = height_buffer->GetRow(y);
            const int16_t* next_row = height_buffer->GetRow(next_row_index);

            const float p31 = next_row_index - prev_row_index;
            const float p31s = p31 * hscale;

            for (size_t x = 0; x < ixs; ++x) {
                const size_t prev_col_index =  (x < epx) ? 0 : x - epx;
                const size_t next_col_index =  (x + epx >= ixs) ? ixs - 1 : x + epx;

                const int16_t& up =     prev_row[x];
                const int16_t& bottom = next_row[x];
                const int16_t& left =   curr_row[prev_col_index];
                const int16_t& right =  curr_row[next_col_index];

                const int32_t p20 = next_col_index - prev_col_index;
                const int32_t p22 = right - left;
                const int32_t p32 = bottom - up;

                int32_t dd0 = p22 * p31;
                int32_t dd1 = p20 * p32;
                int32_t dd2 = p20 * p31s;

                // prevent overflow of magnitude calculation
                const int32_t scale = (dd2 / 512) + 1;
                dd0 /= scale;
                dd1 /= scale;
                dd2 /= scale;

                const uint32_t sqr_mag = (dd0 * dd0 + dd1 * dd1 + dd2 * dd2);
                int32_t mag = (dd2 * sz + dd0 * sx + dd1 * sy) / (isqrt4(sqr_mag)|1);
                mag = Clamp<int32_t>((mag - sz), -64, 63);

                // when h is invalid, result is clamped to 255 so we have invalid terrain color
                int16_t h =  *(curr_row + x);
                h = ((h - height_min) >> height_scale);
                h = Clamp<int16_t>(h, 0, 255);

                screen_row[x] = GetColor(h, mag);
            }
        }
    }

    void Slope() {
        assert(height_buffer && height_buffer->GetBuffer() );
        assert(screen_buffer && screen_buffer->GetBuffer() );

        const size_t ixs = height_buffer->GetWidth();
        const size_t iys = height_buffer->GetHeight();

#if defined(_OPENMP)
        #pragma omp parallel for
#endif
        for (size_t y = 0; y < iys; ++y) {
            BGRColor* screen_row = screen_buffer->GetRow(y);
            const int16_t *height_row = height_buffer->GetRow(y);
            
            for (size_t x = 0; x < ixs; ++x) {
                int16_t h = height_row[x];
                h = ((h - height_min) >> height_scale);
                h = Clamp<int16_t>(h, 0, 255);
                screen_row[x] = GetColor(h);
            }
        }
    }
#endif

    void FixOldMapWater() {
        // this exist only for compatibility with old topology file without water shape
        // in this case all altitude equal to zero are water.
        // if topology file contain water shape this fonction have zero overhead in this case.
        if(!LKWaterTopology) {

            const size_t ixs = height_buffer->GetWidth();
            const size_t iys = height_buffer->GetHeight();

            for (unsigned int y = 0; y < iys; ++y) {
                BGRColor* screen_row = screen_buffer->GetRow(y);
                const int16_t *height_row = height_buffer->GetRow(y);

                for (unsigned int x = 0; x < ixs; ++x) {
                    int16_t h = height_row[x];
                    screen_row[x] = (h == 0) ? BGRColor(85, 160, 255) : screen_row[x];
                }
            }
        }
    }

    static
    int16_t IsoBand(int16_t height, int zoom) {
        return (std::max<int16_t>(0, height)) >> (6 + zoom); // 64m, can't be smaller to avoid uint8_t overflow.
        // return (std::max<int16_t>(0, height)) >> (7 + zoom); // 128m
        // return (std::max<int16_t>(0, height)) >> 8; // 256m
    }

#if (defined(__ARM_NEON) || defined(__ARM_NEON__)) && !GCC_OLDER_THAN(5,0)
private:

    static
    int16x8_t IsoBand(int16x8_t height, int zoom) {
        int16x8_t h = vmaxq_s16(height, vdupq_n_s16(0));
        return vshlq_s16(h, vdupq_n_s16(-(6 + zoom)));
    }

    template<typename T>
    T GetIsoLineColor(int16x8_t height) const {
        return T{
            GetIsoLineColor(vgetq_lane_s16(height, 0)).value.GetNativeValue(),
            GetIsoLineColor(vgetq_lane_s16(height, 1)).value.GetNativeValue(),
            GetIsoLineColor(vgetq_lane_s16(height, 2)).value.GetNativeValue(),
            GetIsoLineColor(vgetq_lane_s16(height, 3)).value.GetNativeValue(),
            GetIsoLineColor(vgetq_lane_s16(height, 4)).value.GetNativeValue(),
            GetIsoLineColor(vgetq_lane_s16(height, 5)).value.GetNativeValue(),
            GetIsoLineColor(vgetq_lane_s16(height, 6)).value.GetNativeValue(),
            GetIsoLineColor(vgetq_lane_s16(height, 7)).value.GetNativeValue()
        };
    }

    /**
     * drawIsoLinePixel : we need 2 specialisation
     *  - first is for 16bit Color ( android, openvario ... )
     *  - second is for 8bit Color ( Kobo )
     */
    template<typename Color_t>
    std::enable_if_t<(sizeof(Color_t) == sizeof(int16_t))>
    drawIsoLinePixel(Color_t *pixel_src, int16x8_t height, uint16x8_t mask) const {
      uint16x8_t line_color = GetIsoLineColor<uint16x8_t>(height);
      uint16_t* screen_ptr = reinterpret_cast<uint16_t*>(pixel_src);
      uint16x8_t mask_line = vmvnq_u16(mask);
      uint16x8_t pixel = vld1q_u16(screen_ptr);
      pixel = (pixel&mask) | (line_color&mask_line);
      vst1q_u16(screen_ptr, pixel);
    }

    template<typename Color_t>
    std::enable_if_t<(sizeof(Color_t) == sizeof(int8_t))>
    drawIsoLinePixel(Color_t *pixel_src, int16x8_t height, uint16x8_t mask) const {
        uint8x8_t line_color = GetIsoLineColor<uint8x8_t>(height);
        uint8_t* screen_ptr = reinterpret_cast<uint8_t*>(pixel_src);
        uint8x8_t mask_color = vmovn_u16(mask);
        uint8x8_t mask_line = vmvn_u8(mask_color);
        uint8x8_t pixel = vld1_u8(screen_ptr);
        pixel = (pixel&mask_color) | (line_color&mask_line);
        vst1_u8(screen_ptr, pixel);
    }
#endif

public:

    void DrawIsoLine() {
        assert(height_buffer && height_buffer->GetBuffer());
        assert(screen_buffer && screen_buffer->GetBuffer());

        const double current_scale = Units::ToUserDistance(MapWindow::zoom.Scale());
        if (current_scale >= 15000 || current_scale <= 100) {
            // No Iso line if zoom are too small or too huge
            return;
        }


        int zoom = ((current_scale >= 3500) ? 2 : ((current_scale >= 1000) ? 1 : 0 ));


        const size_t ixs = height_buffer->GetWidth();
        const size_t iys = height_buffer->GetHeight();

        if(!prev_iso_band) {
            // array used to store iso band value of previous row
            prev_iso_band = std::make_unique<int16_t[]>(ixs);
        }
        if(!current_iso_band) {
            // array used to store iso band value of current row
            //   this become previous row in next loop.
            current_iso_band = std::make_unique<int16_t[]>(ixs);
        }

        // initialize previous row with first height row
        std::transform(height_buffer->GetRow(0), height_buffer->GetRow(1), prev_iso_band.get(), [&](int16_t h){
            return IsoBand(h, zoom);
        });

        for (size_t y = 1; y < iys; ++y) {
            BGRColor* screen_row = screen_buffer->GetRow(y);
            const int16_t *height_row = height_buffer->GetRow(y);


            size_t x = 1;

#if (defined(__ARM_NEON) || defined(__ARM_NEON__)) && !GCC_OLDER_THAN(5,0)

            // iso-band value of first column
            int16x8_t height =  vld1q_s16(height_row);
            vst1q_s16(&prev_iso_band[0], IsoBand(height, zoom));

            const int16x8_t qheight_0 = vmovq_n_s16(0);
            const int16x8_t qheight_255 = vmovq_n_s16(255);
            const int16x8_t qv_height_min = vmovq_n_s16(height_min);
            const int16x8_t qv_height_scale = vmovq_n_s16(height_scale);


            for (; x < (ixs-8); x+=8) {
                // iso band value of current pixel
                height =  vld1q_s16(height_row + x);

                const int16x8_t h = IsoBand(height, zoom);
                vst1q_s16(&current_iso_band[x], h);

                const int16x8_t h1 = vld1q_s16(&prev_iso_band[x-1]); // top left value
                const int16x8_t h2 = vld1q_s16(&prev_iso_band[x]); // top value
                const int16x8_t h3 = vld1q_s16(&current_iso_band[x-1]); // left value

                uint16x8_t mask = (vceqq_s16(h, h2) | vceqq_s16(h3, h1)) & vceqq_s16(h, h3);
                height = (height - qv_height_min) >> qv_height_scale;
                height = Clamp(height, qheight_0, qheight_255);
                drawIsoLinePixel<BGRColor>(&screen_row[x], height, mask);
            }
#else
            // iso band value of first column
            prev_iso_band[0] = IsoBand(height_row[0], zoom);
#endif

            for (; x < ixs; ++x) {
                // iso band value of current pixel
                const int16_t& h = current_iso_band[x] = IsoBand(height_row[x], zoom);

                const int16_t& h1 = prev_iso_band[x-1]; // top left value
                const int16_t& h2 = prev_iso_band[x]; // top value
                const int16_t& h3 = current_iso_band[x-1]; // left value

                int16_t height = (height_row[x] - height_min) >> height_scale;
                height = Clamp<int16_t>(height, 0, 255);

                // apply marching squares algorithm : https://en.wikipedia.org/wiki/Marching_squares#Disambiguation_of_saddle_points
                // 2 equal point are in same iso band, so one is above iso-line and the other is bellow iso-line
                //  tips : to get thinner line we eliminate case [1011, 0111, 0100 1000]
                screen_row[x] = ((h == h2 || h3 == h1) && h == h3) ? screen_row[x] : GetIsoLineColor(height);
            }

            // swap prev & current iso band value
            // current become prev and old prev will be used for store value of next row.
            std::swap(prev_iso_band, current_iso_band);
        }
    }

private:

    inline
    double NoShadingScale() const {
        //
        // Choose the scale threshold for disabling shading. This happens at low zoom levels.
        // Values are in RealScale. Imperial and nautical distance units are using it too.
        // For CE it is also a matter of CPU, reducing calculations for low zoom.
        // For the rest of platforms, if using AUTOCONTRAST this value may be rised up to 18 or more.
        // 
#ifdef UNDER_CE
        // After 7.5Km zoom
        constexpr double NOSHADING_REALSCALE = 5.4;
#else
        // After 20Km  zoom 14.3
        constexpr double NOSHADING_REALSCALE = 14.3;
#endif
        return AutoContrast ? 18.0 : NOSHADING_REALSCALE;
    }

    inline 
    const BGRColor& GetColor(int16_t height, int mag = 0) const {
        return color_table[mag + 64][height];
    }

    inline 
    void SetColor(int16_t height, int mag, BGRColor&& color) {
        color_table[mag + 64][height] = std::forward<BGRColor>(color);
    }

    static BGRColor GetInvalidColor() {
      return IsDithered()
                 ? BGRColor(255, 255, 255) // White terrain invalid
                 : BGRColor(194, 223, 197); // LCD green terrain invalid
    }


    const BGRColor GetIsoLineColor(int16_t height) const {
        return GetColor(height, -64);
    }

public:
    void ColorTable() {
        color_ramp = &terrain_colors[TerrainRamp];
        if (color_ramp == lastColorRamp &&
                height_scale == last_height_scale &&
                last_terrain_whiteness == TerrainWhiteness &&
                last_realscale == MapWindow::zoom.RealScale())
        {
            // no need to update the color table
            return;
        }

        lastColorRamp = color_ramp;
        last_height_scale = height_scale;
        last_terrain_whiteness = TerrainWhiteness;
        last_realscale = MapWindow::zoom.RealScale();


        short auto_contrast = TerrainContrast;

        if (AutoContrast) {
            //
            // The more you zoom in (lower RealScale), the more you rise contrast and brightness.
            // More details are visible at high zoom levels, automatically, and shading artifacts are not
            // appearing at low zoom.
            // Linear scaling not giving good enough results, going manual here.
            //

            // 100->255 %  (1:2.55) :
            // 10=25.5 15=38 20=51 25=64 30=76.5 40=102 45=114 50=127.5 60=153 65=166 70=178.5 75=191 80=204 85=218 90=229
            if      (last_realscale > 10.8) { auto_contrast=51;  auto_brightness=114; } // 20 km and up 20 45
            else if (last_realscale > 7.2)  { auto_contrast=64;  auto_brightness=128; } // 15 km   25 50
            else if (last_realscale > 5.4)  { auto_contrast=64;  auto_brightness=128; } //  10 km  25 50
            else if (last_realscale > 3.58) { auto_contrast=64;  auto_brightness=166; } // 7.5 km  25 65
            else if (last_realscale > 2.55) { auto_contrast=102; auto_brightness=179; } // 5.0 km  40 70
            else if (last_realscale > 1.43) { auto_contrast=128; auto_brightness=179; } // 3.5 km  50 70
            else if (last_realscale > 1.1)  { auto_contrast=179; auto_brightness=191; } // 2.0 km  70 75
            else if (last_realscale > 0.72) { auto_contrast=204; auto_brightness=191; } // 1.5 km  80 75
            else if (last_realscale > 0.54) { auto_contrast=204; auto_brightness=204; } // 1.0 km  80 80
            else                            { auto_contrast=218; auto_brightness=218; } //  < 1km  85 85

        }

        for (int h = 0; h < 256; h++) {

            const TerrainColor height_color = ColorRampLookup(h << height_scale, *color_ramp);

            for (int mag = -64; mag < 64; mag++) {
                // i=255 means TERRAIN_INVALID. Water is colored in Slope
                if (h == 255) {
                    SetColor(h, mag, GetInvalidColor());
                } else {
                    SetColor(h, mag, BGRColor(terrain_lightness(TerrainShading(mag * auto_contrast / 128, height_color),TerrainWhiteness)) );
                }
            }
        }
    }

    void Draw(LKSurface& Surface, const RECT& rc) {
        if(_dirty) {
            screen_buffer->SetDirty();

#ifdef USE_TERRAIN_BLUR
            if (blursize > 0) {
                screen_buffer->Blur(blursize);
            }
#endif
        }

        _dirty = false;
        screen_buffer->DrawStretch(Surface, rc);
    }
};

namespace {

std::unique_ptr<TerrainRenderer> trenderer;

/**
 * @return true if all terrain parameters are same, false if one or more change
 */
bool UpToDate(short TerrainContrast, short TerrainBrightness, short TerrainRamp, short Shading, const ScreenProjection& _Proj) {

    static short old_TerrainContrast(TerrainContrast);
    static short old_TerrainBrightness(TerrainBrightness);
    static short old_TerrainRamp(TerrainRamp);
    static short old_Shading(Shading);
    static double old_TerrainWhiteness(TerrainWhiteness);
    static ScreenProjection old_ScreenProjection(_Proj);

    if( old_ScreenProjection != _Proj
            || old_TerrainWhiteness != TerrainWhiteness
            || old_TerrainContrast != TerrainContrast
            || old_TerrainBrightness != TerrainBrightness
            || old_TerrainRamp != TerrainRamp
            || old_Shading != Shading ) {

        old_TerrainContrast = TerrainContrast;
        old_TerrainBrightness = TerrainBrightness;
        old_TerrainWhiteness = TerrainWhiteness;
        old_TerrainRamp = TerrainRamp;
        old_Shading = Shading;
        old_ScreenProjection = _Proj;

        return false;
    }

    return true;
}

} // namespace

/**
 * Require LockTerrainDataGraphics() everytime !
 */
bool DrawTerrain(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj,
        const double sunazimuth, const double sunelevation) {
    (void) sunelevation; // TODO feature: sun-based rendering option

    if (!RasterTerrain::isTerrainLoaded()) {
        return false;
    }

    static RECT oldrc = {};

    if (PixelRect(rc) != PixelRect(oldrc)) {
        // Resolution has changed, probably PAN mode on with bottombar full opaque
        // We paint full screen, so we resize it.
        trenderer = nullptr;
    }

    try {
        if (!trenderer) {
            oldrc = rc;
            trenderer = std::make_unique<TerrainRenderer>(rc);
        }
    } catch(std::exception& e) {
        return false;
    }

    if(trenderer->IsDirty() || (!UpToDate(TerrainContrast, TerrainBrightness, TerrainRamp, Shading, _Proj))) {
        trenderer->SetDirty();
    }

    if(trenderer->IsDirty()) {

        // load terrain shading parameters
        // Make them instead dynamically calculated based on previous average terrain illumination
        tshadow = terrain_shadow[TerrainRamp];
        thighlight = terrain_highlight[TerrainRamp];

        // step 0: fill height buffer
        trenderer->Height({rc.left, rc.top}, _Proj);

        // step 1: update color table
        //   need to be done after fill height buffer because depends of min 
        //   and max height of terrain
        trenderer->ColorTable();

        // step 3: calculate derivatives of height buffer
        // step 4: calculate illumination and colors
        if(trenderer->DoShading()) {
            // calculate sunlight vector
            const double fudgeelevation  = (10.0 + 80.0 * trenderer->get_brightness() / 255.0);
            const int sx = (255 * (fastcosine(fudgeelevation) * fastsine(sunazimuth)));
            const int sy = (255 * (fastcosine(fudgeelevation) * fastcosine(sunazimuth)));
            const int sz = (255 * fastsine(fudgeelevation));

            trenderer->Slope_shading(sx, sy, sz);
        } else {
            trenderer->Slope();
        }

        trenderer->FixOldMapWater();
        if(IsoLine_Config) {
            trenderer->DrawIsoLine();
        }
    }
    // step 5: draw
    trenderer->Draw(Surface, rc);

    return true;
}

/**
 * Require LockTerrainDataGraphics() everytime !
 */
void CloseTerrainRenderer() {
    trenderer = nullptr;
}


#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

// check if ramp_colors is valid (height sorted).
static
bool IsValidColorRamp(const COLORRAMP(&ramp_colors)[NUM_COLOR_RAMP_LEVELS]) {

    const auto begin = std::begin(ramp_colors);
    const auto end = std::end(ramp_colors);

    return std::is_sorted(begin, end, [](const COLORRAMP& a, const COLORRAMP& b) {
        return a.height < b.height;
    });
}

TEST_SUITE("Terrain renderer") {
	TEST_CASE("color ramp") {
        SUBCASE("ramp height sorted") {

            for (const auto& ramp : terrain_colors) {
                CHECK(IsValidColorRamp(ramp));
            }
        }
    }
}

#endif
