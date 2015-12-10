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

// #define TDEBUG 1

#include "./ColorRamps.h"


unsigned short minalt = TERRAIN_INVALID;

static bool terrain_ready = false;

extern bool FastZoom;

Topology* TopoStore[MAXTOPOLOGY] = {};

uint8_t tshadow_r, tshadow_g, tshadow_b, tshadow_h;
uint8_t thighlight_r, thighlight_g, thighlight_b, thighlight_h;


static const COLORRAMP* lastColorRamp = NULL;

void ColorRampLookup(const short h,
        uint8_t &r, uint8_t &g, uint8_t &b,
        const COLORRAMP* ramp_colors, const int numramp,
        const unsigned char interp_levels) 
{

    unsigned short f, of;
    unsigned short is = 1 << interp_levels;

    // gone past end, so use last color
    if (h >= ramp_colors[numramp - 1].h) {
        r = ramp_colors[numramp - 1].r;
        g = ramp_colors[numramp - 1].g;
        b = ramp_colors[numramp - 1].b;
        return;
    }
    for (unsigned int i = numramp - 2; i--;) {
        if (h >= ramp_colors[i].h) {
            f = (unsigned short) (h - ramp_colors[i].h) * is /
                    (unsigned short) (ramp_colors[i + 1].h - ramp_colors[i].h);
            
            of = is - f;

            r = (f * ramp_colors[i + 1].r + of * ramp_colors[i].r) >> interp_levels;
            g = (f * ramp_colors[i + 1].g + of * ramp_colors[i].g) >> interp_levels;
            b = (f * ramp_colors[i + 1].b + of * ramp_colors[i].b) >> interp_levels;
            return;
        }
    }

    // check if h lower than lowest
    if (h <= ramp_colors[0].h) {
        r = ramp_colors[0].r;
        g = ramp_colors[0].g;
        b = ramp_colors[0].b;
        return;
    }
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

    TerrainRenderer(const RECT& rc) {

#if (WINDOWSPC>0) && TESTBENCH
        StartupStore(_T(".... Init TerrainRenderer area (%ld,%ld) (%ld,%ld)\n"), rc.left, rc.top, rc.right, rc.bottom);
#endif

        // This will not disable terrain! So we shall get calling here again, but no problem.
        if (rc.right < 1 || rc.bottom < 1) {
            LKASSERT(0);
            return;
        }

        // need at least 2Ghz singlecore CPU here for dtquant 1
        dtquant = 2;

#if defined(_WIN32_WCE) || defined(__arm__) || !defined(NDEBUG)
        // scale dtquant so resolution is not too high on large displays
        dtquant *= ScreenScale; // lower resolution a bit.. (no need for CPU >800mHz)

        if (ScreenSize != ss640x480) {
            if (dtquant > 3) dtquant = 3; // .. but not too much
        }
#endif
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


        LKASSERT(ScreenScale != 0);
        const int res_x = iround((rc.right - rc.left) * oversampling / dtquant);
        const int res_y = iround((rc.bottom - rc.top) * oversampling / dtquant);

        sbuf = new CSTScreenBuffer(res_x, res_y);
        if(!sbuf) {
            OutOfMemory(_T(__FILE__), __LINE__);
            ToggleMultimapTerrain();
            return;
        }
        
        ixs = sbuf->GetCorrectedWidth() / oversampling;
        iys = sbuf->GetHeight() / oversampling;

        hBuf = (unsigned short*) malloc(sizeof (unsigned short)*ixs * iys);
        if (!hBuf) {
            StartupStore(_T("------ TerrainRenderer: malloc(%u) failed!%s"), (unsigned)(sizeof(unsigned short)*ixs*iys), NEWLINE);
            OutOfMemory(_T(__FILE__), __LINE__);
            //
            // We *must* disable terrain at this point.
            //
            ToggleMultimapTerrain();
            delete sbuf;
            sbuf = NULL;
            return;
        }
#if (WINDOWSPC>0) && TESTBENCH
        else {
            StartupStore(_T(". TerrainRenderer: malloc(%d) ok%s"), sizeof (unsigned short)*ixs*iys, NEWLINE);
        }
#endif

        colorBuf = (BGRColor*) malloc(256 * 128 * sizeof (BGRColor));
        if (colorBuf == NULL) {
            OutOfMemory(_T(__FILE__), __LINE__);
            ToggleMultimapTerrain();
            delete sbuf;
            sbuf = NULL;
            free(hBuf);
            hBuf = NULL;
            return;
        }
        // Reset this, so ColorTable will reload colors
        lastColorRamp = NULL;

        // this is validating terrain construction
        terrain_ready = true;
    }

    ~TerrainRenderer() {
#if (WINDOWSPC>0) && TESTBENCH
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
        if (sbuf) {
            delete sbuf;
            sbuf = NULL;
        }
        terrain_ready = false;
    }

private:

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
    RasterMap *DisplayMap;
    bool is_terrain;
    static constexpr int interp_levels = 2;
    const COLORRAMP* color_ramp;
    static constexpr unsigned int height_scale = 4;
   
public:

    bool SetMap() {
        if (hBuf == NULL || colorBuf == NULL) return false;
        is_terrain = true;
        DisplayMap = RasterTerrain::TerrainMap;
        color_ramp = &terrain_colors[TerrainRamp][0];

        if (is_terrain) {
            do_shading = true;
        } else {
            do_shading = false;
        }

        return (DisplayMap);
    }

    void SetShading() {
        do_shading = (is_terrain && Shading && terrain_doshading[TerrainRamp]);
    }

    /**
     * Fill height Buffer with according to map projection
     * @offset : {top, left} coordinate of Terrain Rendering Rect relative to DrawRect
     */
    void Height(const POINT& offset) {

        double X, Y;

        const int X0 = (unsigned int) (dtquant / 2);
        const int Y0 = (unsigned int) (dtquant / 2);
        const int X1 = (unsigned int) (X0 + dtquant * ixs);
        const int Y1 = (unsigned int) (Y0 + dtquant * iys);

        unsigned int rfact = 1;

        double pixelDX, pixelDY;

        int x = (X0 + X1) / 2;
        int y = (Y0 + Y1) / 2;
        MapWindow::Screen2LatLon(x, y, X, Y);
        double xmiddle = X;
        double ymiddle = Y;
        int dd = (int) lround(dtquant * rfact);

        x = (X0 + X1) / 2 + dd;
        y = (Y0 + Y1) / 2;
        MapWindow::Screen2LatLon(x, y, X, Y);
        DistanceBearing(ymiddle, xmiddle, Y, X, &pixelDX, NULL);

        x = (X0 + X1) / 2;
        y = (Y0 + Y1) / 2 + dd;
        MapWindow::Screen2LatLon(x, y, X, Y);
        DistanceBearing(ymiddle, xmiddle, Y, X, &pixelDY, NULL);

        pixelsize_d = sqrt((pixelDX * pixelDX + pixelDY * pixelDY) / 2.0);

        // OK, ready to start loading height

        DisplayMap->Lock();

        // set resolution
        DisplayMap->SetFieldRounding(0, 0);
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

#ifdef UNDER_CE
            if (MapWindow::zoom.RealScale() > 5.4) do_shading = false;
#else
            if (MapWindow::zoom.RealScale() > 7.2) do_shading = false;
#endif
        }

        POINT orig = MapWindow::GetOrigScreen();
        orig.x -= offset.x;
        orig.y -= offset.y;
        
        FillHeightBuffer(X0 - orig.x, Y0 - orig.y, X1 - orig.x, Y1 - orig.y);

        DisplayMap->Unlock();
    }

    /**
     * Attention ! never call this without check if map is loaded.
     */
    gcc_noinline
    void FillHeightBuffer(const int X0, const int Y0, const int X1, const int Y1) {
        // fill the buffer
        LKASSERT(hBuf != NULL);
        LKASSERT(DisplayMap->isMapLoaded());

        unsigned short* myhbuf = hBuf;
#ifndef NDEBUG
        const unsigned short* hBufTop = hBuf + ixs*iys;
#endif


        const double PanLatitude = MapWindow::GetPanLatitude();
        const double PanLongitude = MapWindow::GetPanLongitude();
        const double InvDrawScale = MapWindow::GetInvDrawScale() / 1024.0;
        const double DisplayAngle = MapWindow::GetDisplayAngle();

        const int cost = ifastcosine(DisplayAngle);
        const int sint = ifastsine(DisplayAngle);

        const double ac2 = sint*InvDrawScale;
        const double ac3 = cost*InvDrawScale;

        if (!terrain_minalt[TerrainRamp]) {
            minalt = 0; //@ 101110  
        } else {
            minalt = TERRAIN_INVALID;
        }

        for (int y = Y0; y < Y1; y += dtquant) {

            const double ac1 = PanLatitude - y*ac3;
            const double cc1 = y * ac2;

            for (int x = X0; x < X1; x += dtquant, myhbuf++) {
                assert(myhbuf < hBufTop);

                const double Y = ac1 - x*ac2;
                const double X = PanLongitude + (invfastcosine(Y) * ((x * ac3) - cc1));

                // this is setting to 0 any negative terrain value and can be a problem for dutch people
                // myhbuf cannot load negative values!
                *myhbuf = std::max(DisplayMap->GetField(Y, X),(short)0);

                if (*myhbuf < minalt) {
                    minalt = *myhbuf;
                }
            }
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
        const int tc = TerrainContrast;
        unsigned short *thBuf = hBuf;

        const BGRColor* oColorBuf = colorBuf + 64 * 256;
        if (!sbuf->GetBuffer()) return;

        unsigned short h;

        BGRColor* RowBuf = sbuf->GetTopRow();

        for (unsigned int y = 0; y < iys; ++y) {
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

            BGRColor* imageBuf = RowBuf;
            RowBuf = sbuf->GetNextRow(RowBuf);

            for (unsigned int x = 0; x < cixs; ++x, ++thBuf, ++imageBuf) {

                // FIX here Netherland dutch terrain problem
                // if >=0 then the sea disappears...
                if ((h = *thBuf) != TERRAIN_INVALID) {
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

                            int mag = (dd0 * dd0 + dd1 * dd1 + dd2 * dd2);
                            if (mag > 0) {
                                mag = (dd2 * sz + dd0 * sx + dd1 * sy) / isqrt4(mag);
                                mag = max(-64, min(63, (mag - sz) * tc / 128));
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
                    #ifdef UNDITHER
                    colorBuf[i + (mag + 64)*256] = BGRColor(255, 255, 255); // LCD green terrain invalid
                    #else
                    colorBuf[i + (mag + 64)*256] = BGRColor(194, 223, 197); // LCD green terrain invalid
                    #endif
                } else {
                    // height_scale, color_ramp interp_levels  used only for weather
                    // ColorRampLookup is preparing terrain color to pass to TerrainShading for mixing

                    ColorRampLookup(i << height_scale, r, g, b, color_ramp, NUM_COLOR_RAMP_LEVELS, interp_levels);
                    TerrainShading(mag, r, g, b);
                    colorBuf[i + (mag + 64)*256] = BGRColor(r, g, b);
                }
            }
        }
    }

    void Draw(LKSurface& Surface, const RECT& rc) {
#ifdef USE_TERRAIN_BLUR
        if (blursize > 0) {
            sbuf->HorizontalBlur(blursize);
            sbuf->VerticalBlur(blursize);
        }
#endif
        sbuf->DrawStretch(Surface, rc, oversampling);
    }
};


TerrainRenderer *trenderer = NULL;

void CloseTerrainRenderer() {
#if TESTBENCH
    if (!trenderer) {
        StartupStore(_T(".... CANNOT CloseTerrainRenderer, trenderer null!!\n"));
    }
#endif

    if (trenderer) {
        delete trenderer;
        trenderer = NULL;
    }
}

void DrawTerrain(LKSurface& Surface, const RECT& rc,
        const double sunazimuth, const double sunelevation) {
    (void) sunelevation; // TODO feature: sun-based rendering option
    (void) rc;

    if (!RasterTerrain::isTerrainLoaded()) {
        return;
    }

#if TESTBENCH
    if (!trenderer && terrain_ready) {
        LKASSERT(0);
        StartupStore(_T("... DRAWTERRAIN trenderer null, but terrain is ready! Recovering.\n"));
        terrain_ready = false;
    }
#endif

    if (trenderer && !terrain_ready) {
#if BUGSTOP
        LKASSERT(0);
#endif
        StartupStore(_T("... DRAWTERRAIN trenderer not null, but terrain not ready! Recovering.\n"));
        LKSW_ResetTerrainRenderer = true;
    }


    static RECT oldrc;
_redo:

    if (!trenderer) {
        oldrc = rc;
        trenderer = new TerrainRenderer(rc);
        LKASSERT(trenderer);
        if (!terrain_ready) {
#if TESTBENCH
            StartupStore(_T("... DrawTerrain: ERROR terrain not ready\n"));
#endif
            CloseTerrainRenderer();
        }
        if (!trenderer || !terrain_ready) return;
    }

    // Resolution has changed, probably PAN mode on with bottombar full opaque
    // We paint full screen, so we resize it.
    if (LKSW_ResetTerrainRenderer || (rc.bottom != oldrc.bottom) || (rc.right != oldrc.right)
            || (rc.left != oldrc.left) || (rc.top != oldrc.top)) {
#if (WINDOWSPC>0) && TESTBENCH
        if (LKSW_ResetTerrainRenderer) {
            StartupStore(_T("... SWITCH forced for TerrainRenderer Reset\n"));
        } else {
            StartupStore(_T("... Change vertical resolution from %ld,%ld,%ld,%ld  to %ld,%ld,%ld,%ld\n"),
                    oldrc.left, oldrc.top, oldrc.right, oldrc.bottom, rc.left, rc.top, rc.right, rc.bottom);
        }
#endif
        LKASSERT(rc.right > 0 && rc.bottom > 0);
        LKSW_ResetTerrainRenderer = false; // in any case
        CloseTerrainRenderer();
        goto _redo;
    }

    if (!trenderer->SetMap()) {
        return;
    }

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

    // step 1: calculate sunlight vector
    int sx, sy, sz;
    double fudgeelevation = (10.0 + 80.0 * TerrainBrightness / 255.0);

    sx = (int) (255 * (fastcosine(fudgeelevation) * fastsine(sunazimuth)));
    sy = (int) (255 * (fastcosine(fudgeelevation) * fastcosine(sunazimuth)));
    sz = (int) (255 * fastsine(fudgeelevation));

    trenderer->SetShading();
    trenderer->ColorTable();
    // step 2: fill height buffer

    trenderer->Height({rc.left, rc.top});

    // step 3: calculate derivatives of height buffer
    // step 4: calculate illumination and colors
    trenderer->Slope(sx, sy, sz);

    // step 5: draw
    trenderer->Draw(Surface, rc);
}
