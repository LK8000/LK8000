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

// #define TDEBUG 1

#include "ColorRamps.h"
#include "Kobo/Model.hpp"
#include "Util/Clamp.hpp"

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

static short autobr=128;

extern bool FastZoom;

static COLORRAMP tshadow;
static COLORRAMP thighlight;

static const COLORRAMP (*lastColorRamp)[NUM_COLOR_RAMP_LEVELS] = nullptr;
static unsigned last_height_scale = 0;
static double last_terrain_whiteness = 0;

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

#if !defined(NDEBUG) || defined(TESTBENCH)
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

    explicit TerrainRenderer(const RECT& rc) : _dirty(true), _ready(), sbuf(), hBuf(), colorBuf()  {

#if !defined(NDEBUG) || defined(TESTBENCH)
      // check if all colors_ramps are valid.
      for( const auto& ramp : terrain_colors) {
        if(!IsValidColorRamp(ramp)) {
          StartupStore(_T(".... Invalid Color Ramp : %u"), (unsigned)std::distance(terrain_colors, &ramp) );
          assert(false);
        }
      }
#endif

#if (WINDOWSPC>0) && TESTBENCH
        StartupStore(_T(".... Init TerrainRenderer area (%ld,%ld) (%ld,%ld)\n"), rc.left, rc.top, rc.right, rc.bottom);
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

        sbuf = new (std::nothrow) CSTScreenBuffer(res_x, res_y);
        if(!sbuf) {
            OutOfMemory(_T(__FILE__), __LINE__);
            ToggleMultimapTerrain();
            return;
        }

        ixs = sbuf->GetCorrectedWidth() / oversampling;
        iys = sbuf->GetHeight() / oversampling;

        TESTBENCH_DO_ONLY(5,StartupStore(_T("... Terrain quant=%d ixs=%d iys=%d  TOTAL=%d\n"),dtquant,ixs,iys,ixs*iys));

        hBuf = (int16_t*) malloc(sizeof(int16_t)*ixs * iys);
        if (!hBuf) {
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

        // Reset this, so ColorTable will reload colors
        lastColorRamp = NULL;
        last_height_scale = 0;

        // this is validating terrain construction
        _ready = true;
    }

    ~TerrainRenderer() {
#if TESTBENCH
        StartupStore(_T(".... Deinit TerrainRenderer\n"));
#endif
        if (hBuf) {
            free(hBuf);
            hBuf = NULL;
        }

        delete sbuf;
        sbuf = nullptr;
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

private:
    bool _dirty; // indicate sbuf is up-to-date
    bool _ready; // indicate renderer is fully initialised

    unsigned int ixs, iys; // screen dimensions in coarse pixels
    unsigned int dtquant;
    unsigned int epx; // step size used for slope calculations

    CSTScreenBuffer *sbuf;

    double pixelsize_d;

#ifdef USE_TERRAIN_BLUR
// only used if blur...
    int blursize;
    int oversampling;
#else
    static constexpr int oversampling = 1; //no oversampling if no "Blur"
#endif

    int16_t *hBuf;
    BGRColor colorBuf[128][256];
    static constexpr int interp_levels = 8;
    const COLORRAMP (*color_ramp)[NUM_COLOR_RAMP_LEVELS];

    unsigned height_scale; // scale factor  ((height - height_min + 1) << height_scale) must be in [0 - 255] range
    int16_t height_min; // lower height visible terrain
    int16_t height_max; // highter height visible terrain

    bool DoShading() const {

        /*
        // We might accelerate drawing by disabling shading while quickdrawing,
        // but really this wouldnt change much the things now in terms of speed,
        // while instead creating a confusing effect.
        if (QUICKDRAW) {
            return false;
        } else ...
        */

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

public:

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
        LKASSERT(hBuf != NULL);

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

                int16_t& hDst = hBuf[iy*ixs+ix];

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

        const int16_t height_span = height_max - height_min + 1;
        while((height_span >> height_scale) >= 255) {
          ++height_scale;
        }

//        StartupStore(_T("... MinAlt=%d MaxAlt=%d height_scale=%d\n"),height_min, height_max, height_scale);

    }

    // JMW: if zoomed right in (e.g. one unit is larger than terrain
    // grid), then increase the step size to be equal to the terrain
    // grid for purposes of calculating slope, to avoid shading problems
    // (gridding of display) This is why epx is used instead of 1
    // previously.  for large zoom levels, epx=1

    void Slope(const int sx, const int sy, const int sz) {

        LKASSERT(hBuf != NULL);
        const int iepx = (int) epx;
        const unsigned int cixs = ixs;

        const unsigned int ciys = iys;

        const unsigned int ixsepx = cixs*epx;
        const unsigned int ixsright = cixs - 1 - iepx;
        const unsigned int iysbottom = ciys - iepx;
        const int hscale = std::max<int>(1, pixelsize_d);

        const bool do_shading = DoShading();

        int tc=1;
     if (AutoContrast) {
        //
        // The more you zoom in (lower RealScale), the more you rise contrast and brightness.
        // More details are visible at high zoom levels, automatically, and shading artifacts are not
        // appearing at low zoom.
        // Linear scaling not giving good enough results, going manual here.
        //

        // 100->255 %  (1:2.55) :
        // 10=25.5 15=38 20=51 25=64 30=76.5 40=102 45=114 50=127.5 60=153 65=166 70=178.5 75=191 80=204 85=218 90=229
        const double ftc=MapWindow::zoom.RealScale();
        if (ftc>10.8) { tc=51; autobr=114; }                               // 20 km and up 20 45
           else if (ftc>7.2) { tc=64; autobr=128; }                        // 15 km   25 50
              else if (ftc>5.4) { tc=64; autobr=128; }                     //  10 km  25 50
                 else if (ftc>3.58) { tc=64; autobr=166; }                 // 7.5 km  25 65
                    else if (ftc>2.55) { tc=102; autobr=179; }             // 5.0 km  40 70
                       else if (ftc>1.43) { tc=128; autobr=179; }          // 3.5 km  50 70
                          else if (ftc>1.1) { tc=179; autobr=191; }        // 2.0 km  70 75
                             else if (ftc>0.72) { tc=204; autobr=191; }    // 1.5 km  80 75
                                else if (ftc>0.54) { tc=204; autobr=204; } // 1.0 km  80 80
                                   else { tc=218; autobr=218; }            //  < 1km  85 85

        // StartupStore(_T("AUTOC scale=%f  contrast=%d  brightness=%d\n"),MapWindow::zoom.RealScale(), tc,autobr);

     } else { // NOT AUTOCONTRAST
        tc = TerrainContrast;
        // StartupStore(_T("CONTRAST=%d  BRIGHT=%d  scale=%f \n"),TerrainContrast,TerrainBrightness, MapWindow::zoom.RealScale());
     }


#ifndef NDEBUG
        const int16_t* hBuf_end = std::next(hBuf, ixs * iys);
        const int16_t* hBuf_begin = hBuf;
#endif
        const int tc_in_use = tc;
        if (!sbuf->GetBuffer()) return;

#if defined(_OPENMP)
        #pragma omp parallel for
#endif
        for (unsigned int y = 0; y < ciys; ++y) {
            const int itss_y = ciys - 1 - y;
            const int itss_y_ixs = itss_y*cixs;
            const int yixs = y*cixs;
            bool ybottom = false;
            bool ytop = false;
            int p31, p32, p31s;

            if (y < iysbottom) {
                p31 = iepx;
                ybottom = true;
            } else {
                p31 = itss_y;
            }

            if (y >= (unsigned int) iepx) {
                p31 += iepx;
            } else {
                p31 += y;
                ytop = true;
            }
            p31s = p31*hscale;

            BGRColor* RowBuf = sbuf->GetRow(y);
            const int16_t *RowthBuf = &hBuf[y*cixs];
            assert(RowthBuf < hBuf_end);
            assert(RowthBuf >= hBuf_begin);

            for (unsigned int x = 0; x < cixs; ++x) {

                BGRColor* imageBuf = &RowBuf[x];
                const int16_t *thBuf = &RowthBuf[x];
                assert(thBuf < hBuf_end);
                assert(thBuf >= hBuf_begin);

                int16_t h = *thBuf;

                // if >=0 then the sea disappears...
                if (h != TERRAIN_INVALID) {
                    // if (h==0 && LKWaterThreshold==0) { // no LKM coasts, and water altitude
                    if (h == LKWaterThreshold) { // see above.. h cannot be -1000.. so only when LKW is 0 h can be equal
                        *imageBuf = BGRColor(85, 160, 255); // set water color #55 A0 FF
                        continue;
                    }

                    // this fix rendering of negative elevation terrain ( 45 country are concerned ... )
                    h = h - height_min + 1;
                    assert(TerrainRamp == 13 || h >= 0); // height_min is wrong... 

                    h = h >> height_scale;
                    assert(TerrainRamp == 13 || h < 256); // height_scale is wrong ...

                    h = Clamp<int16_t>(h, 0, 255); // avoid buffer overflow....

                    int p20, p22;
                    // no need to calculate slope if undefined height or sea level
                    if (do_shading) {
                        if (x < ixsright) {
                            p20 = iepx;
                            assert((thBuf + iepx) < hBuf_end);
                            assert((thBuf + iepx) >= hBuf_begin);
                            p22 = *(thBuf + iepx);
                        } else {
                            int itss_x = cixs - x - 2;
                            p20 = itss_x;
                            assert((thBuf + itss_x) < hBuf_end);
                            assert((thBuf + itss_x) >= hBuf_begin);
                            p22 = *(thBuf + itss_x);
                        }

                        if (x >= (unsigned int) iepx) {
                            p20 += iepx;
                            assert((thBuf - iepx) < hBuf_end);
                            assert((thBuf - iepx) >= hBuf_begin);
                            p22 -= *(thBuf - iepx);
                        } else {
                            p20 += x;
                            p22 -= *(thBuf - x);
                        }

                        if (ybottom) {
                            assert((thBuf + ixsepx) < hBuf_end);
                            assert((thBuf + ixsepx) >= hBuf_begin);
                            p32 = *(thBuf + ixsepx);
                        } else {
                            assert((thBuf + itss_y_ixs) < hBuf_end);
                            assert((thBuf + itss_y_ixs) >= hBuf_begin);
                            p32 = *(thBuf + itss_y_ixs);
                        }

                        if (ytop) {
                            assert((thBuf - yixs) < hBuf_end);
                            assert((thBuf - yixs) >= hBuf_begin);
                            p32 -= *(thBuf - yixs);
                        } else {
                            assert((thBuf - ixsepx) < hBuf_end);
                            assert((thBuf - ixsepx) >= hBuf_begin);
                            p32 -= *(thBuf - ixsepx);
                        }

                        if ((p22 == 0) && (p32 == 0)) {

                            // slope is zero, so just look up the color
                            *imageBuf = GetColor(h);

                        } else {

                            // p20 and p31 are never 0... so only p22 or p32 can be zero
                            // if both are zero, the vector is 0,0,1 so there is no need
                            // to normalize the vector
                            int dd0 = p22*p31;
                            int dd1 = p20*p32;
                            int dd2 = p20*p31s;

                            // prevent overflow of magnitude calculation
                            const int scale = (dd2 / 512) + 1;
                            dd0 /= scale;
                            dd1 /= scale;
                            dd2 /= scale;

                            int mag = isqrt4(dd0 * dd0 + dd1 * dd1 + dd2 * dd2);
                            if (mag > 0) {
                                mag = (dd2 * sz + dd0 * sx + dd1 * sy) / mag;
                                mag = Clamp((mag - sz) * tc_in_use / 128, -64, 63);
                                *imageBuf = GetColor(h, mag);
                            } else {
                                *imageBuf = GetColor(h);
                            }
                        }
                    } else {
                        // not using shading, so just look up the color
                        *imageBuf = GetColor(h);
                    }
                } else {
                    // old: we're in the water, so look up the color for water
                    // new: h is TERRAIN_INVALID here
                    *imageBuf = GetColor(255);
                }
            } // for
        } // for
    };


private:

    inline 
    const BGRColor& GetColor(int16_t height, int mag = 0) const {
        return colorBuf[mag + 64][height];
    }

    inline 
    void SetColor(int16_t height, int mag, BGRColor&& color) {
        colorBuf[mag + 64][height] = std::forward<BGRColor>(color);
    }


public:
    void ColorTable() {
        color_ramp = &terrain_colors[TerrainRamp];
        if (color_ramp == lastColorRamp &&
                height_scale == last_height_scale &&
                last_terrain_whiteness == TerrainWhiteness)
        {
            // no need to update the color table
            return;
        }
        lastColorRamp = color_ramp;
        last_height_scale = height_scale;
        last_terrain_whiteness = TerrainWhiteness;

        for (int h = 0; h < 256; h++) {

            const TerrainColor height_color = ColorRampLookup(h << height_scale, *color_ramp);

            for (int mag = -64; mag < 64; mag++) {
                // i=255 means TERRAIN_INVALID. Water is colored in Slope
                if (h == 255) {
                    #ifdef DITHER
                    SetColor(h, mag, BGRColor(255, 255, 255)); // White terrain invalid
                    #else
                    SetColor(h, mag, BGRColor(194, 223, 197)); // LCD green terrain invalid
                    #endif
                } else {
                    SetColor(h, mag, BGRColor(terrain_lightness(TerrainShading(mag, height_color),TerrainWhiteness)) );
                }
            }
        }
    }

    void Draw(LKSurface& Surface, const RECT& rc) {
        if(_dirty) {
            sbuf->SetDirty();

#ifdef USE_TERRAIN_BLUR
            if (blursize > 0) {
                sbuf->Blur(blursize);
            }
#endif
        }

        _dirty = false;
        sbuf->DrawStretch(Surface, rc, oversampling);
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
        trenderer->Height({rc.left, rc.top}, _Proj);

        // step 1: update color table
        //   need to be done after fill height buffer because depends of min 
        //   and max height of terrain
        trenderer->ColorTable();

        // step 2: calculate sunlight vector

        double fudgeelevation = 1;
        if (AutoContrast)
           fudgeelevation = (10.0 + 80.0 * autobr / 255.0);
        else
           fudgeelevation = (10.0 + 80.0 * TerrainBrightness / 255.0);

        const int sx = (255 * (fastcosine(fudgeelevation) * fastsine(sunazimuth)));
        const int sy = (255 * (fastcosine(fudgeelevation) * fastcosine(sunazimuth)));
        const int sz = (255 * fastsine(fudgeelevation));

        // step 3: calculate derivatives of height buffer
        // step 4: calculate illumination and colors
        trenderer->Slope(sx, sy, sz);
    }
    // step 5: draw
    trenderer->Draw(Surface, rc);

    return true;
}
