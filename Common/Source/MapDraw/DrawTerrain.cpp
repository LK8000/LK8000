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

#include "./ColorRamps.h"
#include "Kobo/Model.hpp"

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

unsigned short minalt = TERRAIN_INVALID;

static short autobr=128;

extern bool FastZoom;

Topology* TopoStore[MAXTOPOLOGY] = {};

uint8_t tshadow_r, tshadow_g, tshadow_b, tshadow_h;
uint8_t thighlight_r, thighlight_g, thighlight_b, thighlight_h;

extern void rgb_lightness( uint8_t &r, uint8_t &g, uint8_t &b, float lightness);


static const COLORRAMP* lastColorRamp = NULL;

static void ColorRampLookup(const short h,
        uint8_t &r, uint8_t &g, uint8_t &b,
        const COLORRAMP* ramp_colors, const int numramp,
        const unsigned char interp_levels)
{

    const unsigned short is = 1 << interp_levels;

    for (unsigned int i = numramp - 1; i--;) {
        if (h >= ramp_colors[i].h) {
            const unsigned short f = (h - ramp_colors[i].h) * is / (ramp_colors[i + 1].h - ramp_colors[i].h);

            const unsigned short of = is - f;

            r = (f * ramp_colors[i + 1].r + of * ramp_colors[i].r) >> interp_levels;
            g = (f * ramp_colors[i + 1].g + of * ramp_colors[i].g) >> interp_levels;
            b = (f * ramp_colors[i + 1].b + of * ramp_colors[i].b) >> interp_levels;
            return;
        }
    }

    r = ramp_colors[0].r;
    g = ramp_colors[0].g;
    b = ramp_colors[0].b;
}



#define MIX(x,y,i) (uint8_t)((x*i+y*((1<<7)-i))>>7)

inline void TerrainShading(const short illum, uint8_t &r, uint8_t &g, uint8_t &b) {
    char x;
    if (illum < 0) { // shadow to blue
        x = min((int) tshadow_h, -illum);
        r = MIX(tshadow_r, r, x);
        g = MIX(tshadow_g, g, x);
        b = MIX(tshadow_b, b, x);
    } else if (illum > 0) { // highlight to yellow
        if (thighlight_h == 255) return; // 101016
        x = min((int) thighlight_h, illum / 2);
        r = MIX(thighlight_r, r, x);
        g = MIX(thighlight_g, g, x);
        b = MIX(thighlight_b, b, x);
    }
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

    TerrainRenderer(const RECT& rc) : _dirty(true), _ready(), sbuf(), hBuf(), colorBuf()  {

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

        hBuf = (unsigned short*) malloc(sizeof (unsigned short)*ixs * iys);
        if (!hBuf) {
            StartupStore(_T("------ TerrainRenderer: malloc(%u) failed!%s"), (unsigned)(sizeof(unsigned short)*ixs*iys), NEWLINE);
            OutOfMemory(_T(__FILE__), __LINE__);
            //
            // We *must* disable terrain at this point.
            //
            ToggleMultimapTerrain();
            return;
        }
#if TESTBENCH
        else {
            StartupStore(_T(". TerrainRenderer: malloc(%u) ok"), (unsigned)(sizeof(unsigned short)*ixs*iys));
        }
#endif

        colorBuf = (BGRColor*) malloc(256 * 128 * sizeof (BGRColor));
        if (colorBuf == NULL) {
            OutOfMemory(_T(__FILE__), __LINE__);
            ToggleMultimapTerrain();
            return;
        }
        // Reset this, so ColorTable will reload colors
        lastColorRamp = NULL;

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
        if (colorBuf) {
            free(colorBuf);
            colorBuf = NULL;
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

    unsigned short *hBuf;
    BGRColor *colorBuf;
    bool do_shading;
    static constexpr int interp_levels = 8;
    const COLORRAMP* color_ramp;
    static constexpr unsigned int height_scale = 4;

public:

    void SetShading() {
        do_shading = (Shading && terrain_doshading[TerrainRamp]);
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

        /*
        // We might accelerate drawing by disabling shading while quickdrawing,
        // but really this wouldnt change much the things now in terms of speed,
        // while instead creating a confusing effect.
        if (QUICKDRAW) {
            do_shading=false;
        } else ...
        */

        if (epx > min(ixs, iys) / 4) {
            do_shading = false;
        } else {
            if (AutoContrast) {
               if (MapWindow::zoom.RealScale() > 18) do_shading = false;
            } else {
               if (MapWindow::zoom.RealScale() > NOSHADING_REALSCALE) do_shading = false;
            }
        }

        POINT orig = MapWindow::GetOrigScreen();
        orig.x -= offset.x;
        orig.y -= offset.y;

        if(DisplayMap->interpolate()) {

            FillHeightBuffer(X0 - orig.x, Y0 - orig.y, X1 - orig.x, Y1 - orig.y,
                    [DisplayMap](const double &X, const double &Y){
                        return DisplayMap->GetFieldInterpolate(X,Y);
                    });
        } else {

            FillHeightBuffer(X0 - orig.x, Y0 - orig.y, X1 - orig.x, Y1 - orig.y,
                    [DisplayMap](const double &X, const double &Y){
                        return DisplayMap->GetFieldFine(X,Y);
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

#if defined(_OPENMP)
				#pragma omp parallel for
#endif
        for (unsigned int iy=0; iy < iys; iy++) {
            const int y = Y0 + (iy*dtquant);
            const double ac1 = PanLatitude - y*ac3;
            const double cc1 = y * ac2;

            for (unsigned int ix=0; ix < ixs; ix++) {
                const int x = X0 + (ix*dtquant);
                const double Y = ac1 - x*ac2;
                const double X = PanLongitude + (invfastcosine(Y) * ((x * ac3) - cc1));

                unsigned short& hDst = hBuf[iy*ixs+ix];
                // this is setting to 0 any negative terrain value and can be a problem for dutch people
                // myhbuf cannot load negative values!
                hDst = std::max<short>(GetHeight(Y, X),0);
            }
        }

        if (!terrain_minalt[TerrainRamp]) {
            minalt = 0; //@ 101110
        } else {
            minalt = *std::min_element(hBuf, hBuf+(ixs*iys));
        }


        if (TerrainRamp == 13) {
            if (!GPS_INFO.NAVWarning) {
                if (CALCULATED_INFO.Flying) {
                    minalt = (unsigned short) GPS_INFO.Altitude - 150; // 500ft
                } else {
                    minalt = (unsigned short) GPS_INFO.Altitude + 100; // 330ft
                }
            } else {
                minalt += 150;
            }
        }

        // StartupStore(_T("... MinAlt=%d MaxAlt=%d Multiplier=%.3f\n"),minalt,maxalt, (double)((double)maxalt/(double)(maxalt-minalt)));

    }

    // JMW: if zoomed right in (e.g. one unit is larger than terrain
    // grid), then increase the step size to be equal to the terrain
    // grid for purposes of calculating slope, to avoid shading problems
    // (gridding of display) This is why epx is used instead of 1
    // previously.  for large zoom levels, epx=1

    gcc_noinline
    void Slope(const int sx, const int sy, const int sz) {

        LKASSERT(hBuf != NULL);
        const int iepx = (int) epx;
        const unsigned int cixs = ixs;

        const unsigned int ciys = iys;

        const unsigned int ixsepx = cixs*epx;
        const unsigned int ixsright = cixs - 1 - iepx;
        const unsigned int iysbottom = ciys - iepx;
        const int hscale = max(1, (int) (pixelsize_d));

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

        const int tc_in_use = tc;
        const BGRColor* oColorBuf = colorBuf + 64 * 256;
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
            unsigned short *RowthBuf = &hBuf[y*cixs];

            for (unsigned int x = 0; x < cixs; ++x) {

                BGRColor* imageBuf = &RowBuf[x];
                unsigned short *thBuf = &RowthBuf[x];

                unsigned short h = *thBuf;
                // FIX here Netherland dutch terrain problem
                // if >=0 then the sea disappears...
                if (h != TERRAIN_INVALID) {
                    // if (h==0 && LKWaterThreshold==0) { // no LKM coasts, and water altitude
                    if (h == LKWaterThreshold) { // see above.. h cannot be -1000.. so only when LKW is 0 h can be equal
                        *imageBuf = BGRColor(85, 160, 255); // set water color #55 A0 FF
                        continue;
                    }
                    h = h - minalt + 1;

                    int p20, p22;

                    h = min(255, h >> height_scale);
                    // no need to calculate slope if undefined height or sea level

                    if (do_shading) {
                        if (x < ixsright) {
                            p20 = iepx;
                            p22 = *(thBuf + iepx);
                        } else {
                            int itss_x = cixs - x - 2;
                            p20 = itss_x;
                            p22 = *(thBuf + itss_x);
                        }

                        if (x >= (unsigned int) iepx) {
                            p20 += iepx;
                            p22 -= *(thBuf - iepx);
                        } else {
                            p20 += x;
                            p22 -= *(thBuf - x);
                        }

                        if (ybottom) {
                            p32 = *(thBuf + ixsepx);
                        } else {
                            p32 = *(thBuf + itss_y_ixs);
                        }

                        if (ytop) {
                            p32 -= *(thBuf - yixs);
                        } else {
                            p32 -= *(thBuf - ixsepx);
                        }

                        if ((p22 == 0) && (p32 == 0)) {

                            // slope is zero, so just look up the color
                            *imageBuf = oColorBuf[h];

                        } else {

                            // p20 and p31 are never 0... so only p22 or p32 can be zero
                            // if both are zero, the vector is 0,0,1 so there is no need
                            // to normalise the vector
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
                                mag = max(-64, min(63, (mag - sz) * tc_in_use / 128));
                                *imageBuf = oColorBuf[h + mag * 256];
                            } else {
                                *imageBuf = oColorBuf[h];
                            }
                        }
                    } else {
                        // not using shading, so just look up the color
                        *imageBuf = oColorBuf[h];
                    }
                } else {
                    // old: we're in the water, so look up the color for water
                    // new: h is TERRAIN_INVALID here
                    *imageBuf = oColorBuf[255];
                }
            } // for
        } // for
    };

    void ColorTable() {
        color_ramp = &terrain_colors[TerrainRamp][0];
        if (color_ramp == lastColorRamp) {
            // no need to update the color table
            return;
        }
        lastColorRamp = color_ramp;

        for (int i = 0; i < 256; i++) {
            for (int mag = -64; mag < 64; mag++) {
                uint8_t r, g, b;
                // i=255 means TERRAIN_INVALID. Water is colored in Slope
                if (i == 255) {
                    #ifdef DITHER
                    colorBuf[i + (mag + 64)*256] = BGRColor(255, 255, 255); // LCD green terrain invalid
                    #else
                    colorBuf[i + (mag + 64)*256] = BGRColor(194, 223, 197); // LCD green terrain invalid
                    #endif
                } else {
                    // height_scale, color_ramp interp_levels  used only for weather
                    // ColorRampLookup is preparing terrain color to pass to TerrainShading for mixing

                    ColorRampLookup(i << height_scale, r, g, b, color_ramp, NUM_COLOR_RAMP_LEVELS, interp_levels);
                    TerrainShading(mag, r, g, b);
                    rgb_lightness(r,g,b,TerrainWhiteness);

                    colorBuf[i + (mag + 64)*256] = BGRColor(r, g, b);
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
    static double old_TerrainWhiteness=0;
    static ScreenProjection old_ScreenProjection(_Proj);

    if( old_ScreenProjection != _Proj
            || old_TerrainWhiteness != TerrainWhiteness
            || old_TerrainContrast != TerrainContrast
            || old_TerrainBrightness != TerrainBrightness
            || old_TerrainRamp != TerrainRamp
            || old_Shading != Shading ) {

        if (old_TerrainWhiteness != TerrainWhiteness) lastColorRamp=NULL; // reload color table
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
        tshadow_r = terrain_shadow[TerrainRamp].r;
        tshadow_g = terrain_shadow[TerrainRamp].g;
        tshadow_b = terrain_shadow[TerrainRamp].b;
        tshadow_h = terrain_shadow[TerrainRamp].h;

        thighlight_r = terrain_highlight[TerrainRamp].r;
        thighlight_g = terrain_highlight[TerrainRamp].g;
        thighlight_b = terrain_highlight[TerrainRamp].b;
        thighlight_h = terrain_highlight[TerrainRamp].h;

        trenderer->SetShading();
        trenderer->ColorTable();
        // step 1: fill height buffer
        trenderer->Height({rc.left, rc.top}, _Proj);


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
