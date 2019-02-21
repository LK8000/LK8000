/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Terrain.cpp,v 8.6 2010/12/17 02:02:27 root Exp root $
 */

#include "externs.h"
#include "Terrain.h"
#include "RasterTerrain.h"
#include "STScreenBuffer.h"
#include "RGB.h"
#include "Multimap.h"
#include "../Draw/ScreenProjection.h"
#include "NavFunctions.h"

//#define DRAW_TIMER

#include "ColorRamps.h"
#include "Kobo/Model.hpp"
#include "Util/Clamp.hpp"

#if defined(__ARM_NEON) ||defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

//
// Choose the scale threshold for disabling shading. This happens at low zoom levels.
// Values are in RealScale. Imperial and nautical distance units are using it too.
// For CE it is also a matter of CPU, reducing calculations for low zoom.
// For the rest of platforms, if using AUTOCONTRAST this value may be rised up to 18 or more.
// 
#ifdef UNDER_CE
#define NOSHADING_REALSCALE  5.4  // After 7.5Km zoom
#else
#define NOSHADING_REALSCALE  14.3 // After 20Km  zoom 14.3
#endif

extern bool FastZoom;

static COLORRAMP tshadow;
static COLORRAMP thighlight;

static const COLORRAMP (*lastColorRamp)[NUM_COLOR_RAMP_LEVELS] = nullptr;

extern void rgb_lightness( uint8_t &r, uint8_t &g, uint8_t &b, float light);

#ifdef GREYSCALE

static Luminosity8 terrain_lightness( const Luminosity8& color, double lightness) {
  if(lightness == 1.0) {
    return color;
  }
  return {static_cast<uint8_t >(std::min<uint32_t>(color.GetLuminosity() * lightness, 255)) };
}

#else

static RGB8Color terrain_lightness( const RGB8Color& color, float lightness) {
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

#if !defined(NDEBUG) || TESTBENCH
static bool IsValidColorRamp(const COLORRAMP(&ramp_colors)[NUM_COLOR_RAMP_LEVELS]) {

  const auto begin = std::begin(ramp_colors);
  const auto end = std::end(ramp_colors);
  
  bool is_sorted = std::is_sorted(begin, end, [](const COLORRAMP& a, const COLORRAMP& b) {
    return a.height < b.height; 
  } );
  
  return is_sorted;
}
#endif

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

  if (illum < 0) { // shadow to blue
    const uint32_t x = std::min<uint32_t>(tshadow.height, -illum);
    return linear_interpolation<interp_level>(color, tshadow.color, x);
  } else if (illum > 0) { // highlight to yellow
    if (thighlight.height != 255) {
      const uint32_t x = std::min<uint32_t>(thighlight.height, illum / 2);
      return linear_interpolation<interp_level>(color, tshadow.color, x);
    }
  }
  return color;
}


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
public:

    explicit TerrainRenderer(const RECT& rc) : _dirty(true), _ready(), screen_buffer(), height_buffer(), color_table()  {

#if !defined(NDEBUG) || TESTBENCH
      // check if all colors_ramps are valid.
      for( const auto& ramp : terrain_colors) {
        if(!IsValidColorRamp(ramp)) {
          StartupStore(_T(".... Invalid Color Ramp : %u"), (unsigned)std::distance(terrain_colors, &ramp) );
          assert(false);
        }
      }
#endif

#if TESTBENCH
        StartupStore(_T(".... Init TerrainRenderer area (%d,%d) (%d,%d)\n"), (int)rc.left, (int)rc.top, (int)rc.right, (int)rc.bottom);
#endif
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

        extern unsigned int TerrainQuantization();
        dtquant = TerrainQuantization();
        LKASSERT(dtquant>=1);

#ifdef USE_TERRAIN_BLUR
        blursize = max((unsigned int) 0, (dtquant - 1) / 2); // always 0
        oversampling = max(1, (blursize + 1) / 2 + 1); // always 1
        if (blursize == 0) {
            oversampling = 1; // no point in oversampling, just let stretchblt do the scaling
        }
#endif

        /*
          dtq  ovs  blur  res_x  res_y   sx  sy  terrain_loads  pixels
           1    1    0    320    240    320 240    76800        76800
           2    1    0    160    120    160 120    19200        19200
           3    2    1    213    160    107  80     8560        34080
           4    2    1    160    120     80  60     4800        19200
           5    3    2    192    144     64  48     3072        27648
         */




        const int res_x = iround((rc.right - rc.left) * oversampling / dtquant);
        const int res_y = iround((rc.bottom - rc.top) * oversampling / dtquant);

        screen_buffer = new (std::nothrow) CSTScreenBuffer(res_x, res_y);
        if(!screen_buffer) {
            OutOfMemory(_T(__FILE__), __LINE__);
            ToggleMultimapTerrain();
            return;
        }

        ixs = screen_buffer->GetCorrectedWidth() / oversampling;
        iys = screen_buffer->GetHeight() / oversampling;

        TESTBENCH_DO_ONLY(5,StartupStore(_T("... Terrain quant=%d ixs=%d iys=%d  TOTAL=%d\n"),dtquant,ixs,iys,ixs*iys));

        height_buffer = (int16_t*) malloc(sizeof(int16_t)*ixs * iys);
        if (!height_buffer) {
            StartupStore(_T("------ TerrainRenderer: malloc(%u) failed!%s"), (unsigned)(sizeof(int16_t)*ixs*iys), NEWLINE);
            OutOfMemory(_T(__FILE__), __LINE__);
            //
            // We *must* disable terrain at this point.
            //
            ToggleMultimapTerrain();
            return;
        }
#if TESTBENCH
        else {
            StartupStore(_T(". TerrainRenderer: malloc(%u) ok"), (unsigned)(sizeof(int16_t)*ixs*iys));
        }
#endif

        auto_brightness = 218;

        // Reset this, so ColorTable will reload colors
        lastColorRamp = NULL;
        last_height_scale = 0;
        last_realscale = 0;

        // this is validating terrain construction
        _ready = true;
    }

    ~TerrainRenderer() {
#if TESTBENCH
        StartupStore(_T(".... Deinit TerrainRenderer\n"));
#endif
        if (height_buffer) {
            free(height_buffer);
            height_buffer = NULL;
        }

        delete screen_buffer;
        screen_buffer = nullptr;
    }

    void SetDirty() {
        _dirty = true;
    }

    bool IsDirty() const {
        return _dirty;
    }
    
    bool IsReady() const {
        return _ready;
    }

    short get_brightness() const {
        return auto_brightness;
    }

private:
    bool _dirty; // indicate screen_buffer is up-to-date
    bool _ready; // indicate renderer is fully initialised

    unsigned int ixs, iys; // screen dimensions in coarse pixels
    unsigned int dtquant;
    unsigned int epx; // step size used for slope calculations

    CSTScreenBuffer *screen_buffer;

    double pixelsize_d;

#ifdef USE_TERRAIN_BLUR
// only used if blur...
    int blursize;
    int oversampling;
#else
    static constexpr int oversampling = 1; //no oversampling if no "Blur"
#endif

    int16_t *height_buffer;

    BGRColor color_table[128][256];

    const COLORRAMP (*color_ramp)[NUM_COLOR_RAMP_LEVELS];

    unsigned last_height_scale;
    double last_terrain_whiteness;
    double last_realscale;

    unsigned height_scale; // scale factor  ((height - height_min) << height_scale) must be in [0 - 255] range
    int16_t height_min; // lower height visible terrain
    int16_t height_max; // highter height visible terrain

    short auto_brightness;

public:

    bool DoShading() const {

        if (epx > min(ixs, iys) / 4) {
            return false;
        } else {
            if (AutoContrast) {
                if (MapWindow::zoom.RealScale() > 18) {
                    return false;
                }
            } else {
                if (MapWindow::zoom.RealScale() > NOSHADING_REALSCALE) {
                    return false;
                }
            }
        }

        return (Shading && terrain_doshading[TerrainRamp]);
    }

    /**
     * Fill height Buffer with according to map projection
     * @offset : {top, left} coordinate of Terrain Rendering Rect relative to DrawRect
     */
    void Height(const POINT& offset, const ScreenProjection& _Proj) {
        RasterTerrain::Lock();
        RasterMap* DisplayMap = RasterTerrain::TerrainMap;
        assert(DisplayMap);
        if(!DisplayMap) {
            return;
        }
        LKASSERT(DisplayMap->isMapLoaded());

        double X, Y;

        const int X0 = (unsigned int) (dtquant / 2);
        const int Y0 = (unsigned int) (dtquant / 2);
        const int X1 = (unsigned int) (X0 + dtquant * ixs);
        const int Y1 = (unsigned int) (Y0 + dtquant * iys);

        double pixelDX, pixelDY;

        RasterPoint Pt = {
            (X0 + X1) / 2,
            (Y0 + Y1) / 2
        };
        _Proj.Screen2LonLat(Pt, X, Y);
        double xmiddle = X;
        double ymiddle = Y;

        Pt.x = (X0 + X1) / 2 + dtquant;
        Pt.y = (Y0 + Y1) / 2;
        _Proj.Screen2LonLat(Pt, X, Y);

        double dX = std::abs(xmiddle - X);
        DistanceBearing(ymiddle, xmiddle, Y, X, &pixelDX, NULL);

        Pt.x = (X0 + X1) / 2;
        Pt.y = (Y0 + Y1) / 2 + dtquant;
        _Proj.Screen2LonLat(Pt, X, Y);
        double dY = std::abs(ymiddle - Y);
        DistanceBearing(ymiddle, xmiddle, Y, X, &pixelDY, NULL);

        pixelsize_d = sqrt((pixelDX * pixelDX + pixelDY * pixelDY) / 2.0);

        // OK, ready to start loading height

        // set resolution
        DisplayMap->SetFieldRounding(dX/3, dY/3);
        epx = DisplayMap->GetEffectivePixelSize(&pixelsize_d, ymiddle, xmiddle);


        POINT orig = MapWindow::GetOrigScreen();
        orig.x -= offset.x;
        orig.y -= offset.y;

        if(DisplayMap->interpolate()) {

            FillHeightBuffer(X0 - orig.x, Y0 - orig.y, X1 - orig.x, Y1 - orig.y,
                    [DisplayMap](const double &lat, const double &lon){
                        return DisplayMap->GetFieldInterpolate(lat,lon);
                    });
        } else {

            FillHeightBuffer(X0 - orig.x, Y0 - orig.y, X1 - orig.x, Y1 - orig.y,
                    [DisplayMap](const double &lat, const double &lon){
                        return DisplayMap->GetFieldFine(lat,lon);
                    });
        }

        RasterTerrain::Unlock();
    }

    /**
     * Attention ! never call this without check if map is loaded.
     *
     * template is needed for avoid to test if interpolation is needed for each pixel.
     */
    template<typename GetHeight_t>
    void FillHeightBuffer(const int X0, const int Y0, const int X1, const int Y1, GetHeight_t GetHeight) {
        // fill the buffer
        LKASSERT(height_buffer != NULL);

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

#if defined(_OPENMP)
        #pragma omp parallel for reduction(max : _height_max) reduction(min : _height_min)
#endif
        for (unsigned int iy=0; iy < iys; iy++) {
            const int y = Y0 + (iy*dtquant);
            const double ac1 = PanLatitude - y*ac3;
            const double cc1 = y * ac2;

            for (unsigned int ix=0; ix < ixs; ix++) {
                const int x = X0 + (ix*dtquant);
                const double Y = ac1 - x*ac2;
                const double X = PanLongitude + (invfastcosine(Y) * ((x * ac3) - cc1));

                int16_t& hDst = height_buffer[iy*ixs+ix];

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
                    height_min = (int16_t) GPS_INFO.Altitude - 150; // 500ft
                } else {
                    height_min = (int16_t) GPS_INFO.Altitude + 100; // 330ft
                }
            } else {
                height_min += 150;
            }
        }

        const int16_t height_span = height_max - height_min;
        while((height_span >> height_scale) >= 255) {
          ++height_scale;
        }

//        StartupStore(_T("... MinAlt=%d MaxAlt=%d height_scale=%d\n"),height_min, height_max, height_scale);

    }


#if defined(__ARM_NEON) || defined(__ARM_NEON__)

    // JMW: if zoomed right in (e.g. one unit is larger than terrain
    // grid), then increase the step size to be equal to the terrain
    // grid for purposes of calculating slope, to avoid shading problems
    // (gridding of display) This is why epx is used instead of 1
    // previously.  for large zoom levels, epx=1

    void Slope_shading(const int sx, const int sy, const int sz) {

        LKASSERT(height_buffer != NULL);
        if(!height_buffer) {
            return;
        }
        if (!screen_buffer->GetBuffer()) {
            return;
        }

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
                static_cast<float32_t>(epx + ixs - 3)
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

        const unsigned int ixsright = std::max(4u, ixs - 1 - epx);
        const unsigned int ixsleft = std::max(4u, epx);

        for(unsigned y = 0; y < iys; y++) {
            BGRColor* screen_row = screen_buffer->GetRow(y);

            const unsigned prev_row_index =  (y < epx) ? 0 : y - epx;
            const unsigned next_row_index =  (y + epx >= iys) ? iys - 1 : y + epx;

            const int16_t* prev_row = &height_buffer[prev_row_index * ixs];
            const int16_t* curr_row = &height_buffer[y * ixs];
            const int16_t* next_row = &height_buffer[next_row_index * ixs];

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
            }


            // right side
            {
                const int16x4_t right = vmov_n_s16(*(curr_row + ixs - 1));
                for (unsigned int x = ixsright; x < ixs; x+=4) {
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
                }
            }
        }
    }

    void Slope() {
        LKASSERT(height_buffer != NULL);
        if(!height_buffer) {
            return;
        }
        if (!screen_buffer->GetBuffer()) {
            return;
        }

        const int16x4_t height_0 = vmov_n_s16(0);
        const int16x4_t height_255 = vmov_n_s16(255);
        const int16x4_t v_height_min = vmov_n_s16(height_min);
        const int16x4_t v_height_scale = vmov_n_s16(height_scale);

        for (unsigned int y = 0; y < iys; ++y) {
            BGRColor* screen_row = screen_buffer->GetRow(y);
            const int16_t *height_row = &height_buffer[y*ixs];
            
            for (unsigned int x = 0; x < ixs; x+=4) {
                int16x4_t h =  vld1_s16(height_row + x);
                h = (h - v_height_min) >> v_height_scale;
                h = Clamp(h, height_0, height_255);

                screen_row[x]   = GetColor(vget_lane_s16(h, 0));
                screen_row[x+1] = GetColor(vget_lane_s16(h, 1));
                screen_row[x+2] = GetColor(vget_lane_s16(h, 2));
                screen_row[x+3] = GetColor(vget_lane_s16(h, 3));
            }
        }
    }

#else

    void Slope_shading(const int sx, const int sy, const int sz) {

        LKASSERT(height_buffer != NULL);
        if(!height_buffer) {
            return;
        }
        if (!screen_buffer->GetBuffer()) {
            return;
        }

        const int hscale = std::max<int>(1, pixelsize_d);

#if defined(_OPENMP)
        #pragma omp parallel for
#endif
        for(unsigned y = 0; y < iys; y++) {
            BGRColor* screen_row = screen_buffer->GetRow(y);

            const unsigned prev_row_index =  (y < epx) ? 0 : y - epx;
            const unsigned next_row_index =  (y + epx >= iys) ? iys - 1 : y + epx;

            const int16_t* prev_row = &height_buffer[prev_row_index * ixs];
            const int16_t* curr_row = &height_buffer[y * ixs];
            const int16_t* next_row = &height_buffer[next_row_index * ixs];

            const float p31 = next_row_index - prev_row_index;
            const float p31s = p31 * hscale;

            for (unsigned int x = 0; x < ixs; ++x) {
                const unsigned prev_col_index =  (x < epx) ? 0 : x - epx;
                const unsigned next_col_index =  (x + epx >= ixs) ? ixs - 1 : x + epx;

                const int16_t up =     *(prev_row + x);
                const int16_t bottom = *(next_row + x);
                const int16_t left =   *(curr_row + prev_col_index);
                const int16_t right =  *(curr_row + next_col_index);

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

                const int32_t sqr_mag = (dd0 * dd0 + dd1 * dd1 + dd2 * dd2);
                int32_t mag = (dd2 * sz + dd0 * sx + dd1 * sy) / isqrt4(sqr_mag);
                mag = Clamp<int32_t >((mag - sz), -64, 63);

                int16_t h =  *(curr_row + x);
                h = ((h - height_min) >> height_scale);
                h = Clamp<int16_t>(h, 0, 255);

                screen_row[x] = GetColor(h, mag);
            }
        }
    }

    void Slope() {
        LKASSERT(height_buffer != NULL);
        if(!height_buffer) {
            return;
        }
        if (!screen_buffer->GetBuffer()) {
            return;
        }

#if defined(_OPENMP)
        #pragma omp parallel for
#endif
        for (unsigned int y = 0; y < iys; ++y) {
            BGRColor* screen_row = screen_buffer->GetRow(y);
            const int16_t *height_row = &height_buffer[y*ixs];
            
            for (unsigned int x = 0; x < ixs; ++x) {
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

            for (unsigned int y = 0; y < iys; ++y) {
                BGRColor* screen_row = screen_buffer->GetRow(y);
                const int16_t *height_row = &height_buffer[y*ixs];

                for (unsigned int x = 0; x < ixs; ++x) {
                    int16_t h = height_row[x];
                    screen_row[x] = (h == 0) ? BGRColor(85, 160, 255) : screen_row[x];
                }
            }
        }
    }


private:

    inline 
    const BGRColor& GetColor(int16_t height, int mag = 0) const {
        return color_table[mag + 64][height];
    }

    inline 
    void SetColor(int16_t height, int mag, BGRColor&& color) {
        color_table[mag + 64][height] = std::forward<BGRColor>(color);
    }

    static constexpr BGRColor GetInvalidColor() {
#ifdef DITHER
        return BGRColor(255, 255, 255); // White terrain invalid
#else
        return BGRColor(194, 223, 197); // LCD green terrain invalid
#endif
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

#ifdef DRAW_TIMER
        StartupStore("Draw Terrain : updated ColorTable");
#endif
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
        screen_buffer->DrawStretch(Surface, rc, oversampling);
    }
};


static TerrainRenderer *trenderer = NULL;

/**
 * Require LockTerrainDataGraphics() everytime !
 */
void CloseTerrainRenderer() {
    delete trenderer;
    trenderer = nullptr;
}

/**
 * @return true if all terrain parameters are same, false if one or more change
 */
static bool UpToDate(short TerrainContrast, short TerrainBrightness, short TerrainRamp, short Shading, const ScreenProjection& _Proj) {

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


/**
 * Require LockTerrainDataGraphics() everytime !
 */
bool DrawTerrain(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj,
        const double sunazimuth, const double sunelevation) {
    (void) sunelevation; // TODO feature: sun-based rendering option
    (void) rc;

    if (!RasterTerrain::isTerrainLoaded()) {
        return false;
    }

    if (trenderer && !trenderer->IsReady()) {
#if BUGSTOP
        LKASSERT(0);
#endif
        StartupStore(_T("... DRAWTERRAIN trenderer not null, but terrain not ready! Recovering.\n"));
        CloseTerrainRenderer();
    }


    static RECT oldrc;
_redo:

    if (!trenderer) {
        oldrc = rc;
        trenderer = new(std::nothrow) TerrainRenderer(rc);
        LKASSERT(trenderer);
        if (trenderer && !trenderer->IsReady()) {
#if TESTBENCH
            StartupStore(_T("... DrawTerrain: ERROR terrain not ready\n"));
#endif
            CloseTerrainRenderer();
        }
        
        if (!trenderer) {
            return false;
        }
    }

    // Resolution has changed, probably PAN mode on with bottombar full opaque
    // We paint full screen, so we resize it.
    if ((rc.bottom != oldrc.bottom) || (rc.right != oldrc.right)
            || (rc.left != oldrc.left) || (rc.top != oldrc.top)) {
#if TESTBENCH
        StartupStore(_T("... Change vertical resolution from %d,%d,%d,%d  to %d,%d,%d,%d"),
                    oldrc.left, oldrc.top, oldrc.right, oldrc.bottom, rc.left, rc.top, rc.right, rc.bottom);
#endif
        LKASSERT(rc.right > 0 && rc.bottom > 0);
        CloseTerrainRenderer();
        goto _redo;
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
#ifdef DRAW_TIMER
        uint64_t height_start=MonotonicClockUS();
#endif
        trenderer->Height({rc.left, rc.top}, _Proj);

#ifdef DRAW_TIMER
        uint64_t height_time=MonotonicClockUS()-height_start;
        StartupStore(_T("Draw Terrain : height time        < %u.%u ms >\n"),(int)height_time/1000, (int)height_time%1000);
#endif
        // step 1: update color table
        //   need to be done after fill height buffer because depends of min 
        //   and max height of terrain
        trenderer->ColorTable();

        // step 3: calculate derivatives of height buffer
        // step 4: calculate illumination and colors
#ifdef DRAW_TIMER
        uint64_t slope_start=MonotonicClockUS();
#endif

        // step 3: calculate derivatives of height buffer
        // step 4: calculate illumination and colors
        if(trenderer->DoShading()) {
            // calculate sunlight vector
            const double fudgeelevation  = (10.0 + 80.0 * trenderer->get_brightness() / 255.0);
            const int sx = (255 * (fastcosine(fudgeelevation) * fastsine(sunazimuth)));
            const int sy = (255 * (fastcosine(fudgeelevation) * fastcosine(sunazimuth)));
            const int sz = (255 * fastsine(fudgeelevation));

            trenderer->Slope_shading(sx, sy, sz);

#ifdef DRAW_TIMER
            uint64_t slope_time=MonotonicClockUS()-slope_start;
            StartupStore(_T("Draw Terrain : slope shading time < %u.%u ms >\n"),(int)slope_time/1000, (int)slope_time%1000);
#endif

        } else {
            trenderer->Slope();

#ifdef DRAW_TIMER
            uint64_t slope_time=MonotonicClockUS()-slope_start;
            StartupStore(_T("Draw Terrain : slope              < %u.%u ms >\n"),(int)slope_time/1000, (int)slope_time%1000);
#endif
        }

        trenderer->FixOldMapWater();
    }
    // step 5: draw
    trenderer->Draw(Surface, rc);

    return true;
}
