/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "RasterTerrain.h"
#include "Airspace/LKAirspace.h"
#include "RGB.h"
#include "Sideview.h"
#include "Multimap.h"
#include "LKObjects.h"
#include "Asset.hpp"
#include "utils/array_adaptor.h"

using std::min;
using std::max;

constexpr LKColor RGB_ROYAL_BLUE(18,32,139);
constexpr LKColor RGB_STEEL_BLUE(70,130,180);

extern AspSideViewList_t Sideview_pHandeled;
extern LKColor Sideview_TextColor;

//#define OUTLINE_2ND    // double outline for airspaces

void RenderAirspaceTerrain(LKSurface& Surface, double PosLat, double PosLon, double brg, DiagrammStruct* psDiag) {
    RECT rc = psDiag->rc;
    //rc.bottom +=BORDER_Y;
    double range = psDiag->fXMax - psDiag->fXMin; // km
    double hmax = psDiag->fYMax;
    double lat, lon;

    if (!IsDithered() && IsMultimapTerrain()) {
        RenderSky(Surface, rc, SKY_HORIZON_COL, SKY_SPACE_COL, GC_NO_COLOR_STEPS);
    } else {
        Surface.FillRect(&rc, MapWindow::hInvBackgroundBrush[BgMapColor]);
    }
    FindLatitudeLongitude(PosLat, PosLon, brg, psDiag->fXMin, &lat, &lon);
    POINT apTerrainPolygon[AIRSPACE_SCANSIZE_X + 4] = {};
    double d_lat[AIRSPACE_SCANSIZE_X] = {};
    double d_lon[AIRSPACE_SCANSIZE_X] = {};
    double d_h[AIRSPACE_SCANSIZE_X] = {};

#define   FRAMEWIDTH 2
    WithLock(RasterTerrain::mutex, [&]() {
        RasterTerrain::SetTerrainRounding(0, 0);
        for (size_t j = 0; j < AIRSPACE_SCANSIZE_X; j++) { // scan range
            double fj = (double) j * 1.0 / (double) (AIRSPACE_SCANSIZE_X - 1);
            FindLatitudeLongitude(lat, lon, brg, range*fj, &d_lat[j], &d_lon[j]);
            d_h[j] = RasterTerrain::GetTerrainHeight(d_lat[j], d_lon[j]);
            if (d_h[j] == TERRAIN_INVALID) {
                d_h[j] = 0; //@ 101027 BUGFIX
            }
            hmax = max(hmax, d_h[j]);
        }
    });

    /********************************************************************************
     * scan line
     ********************************************************************************/
    if (IsMultimapAirspace()) {
        Sideview_pHandeled = CAirspaceManager::Instance().ScanAirspaceLineList(d_lat, d_lon, d_h);
    }
    else {
        Sideview_pHandeled.clear();
    }

    /********************************************************************************
     * sort to start with biggest airspaces
     ********************************************************************************/
    auto comp_area = [](const auto& a, const auto& b) {
      return a.iAreaSize > b.iAreaSize;
    };
    std::sort(Sideview_pHandeled.begin(), Sideview_pHandeled.end(), comp_area);

    /**********************************************************************************
     * transform into diagram coordinates
     **********************************************************************************/
    double dx1 = (double) (rc.right) / (double) (AIRSPACE_SCANSIZE_X - 1);
    int x0 = rc.left;
    for (auto& p : Sideview_pHandeled) {
        p.rc.left = ((p.rc.left) * dx1) + x0 - FRAMEWIDTH / 2;
        p.rc.right = ((p.rc.right) * dx1) + x0 + FRAMEWIDTH / 2;

        p.rc.bottom = CalcHeightCoordinat(p.rc.bottom, psDiag) + FRAMEWIDTH / 2;
        p.rc.top = CalcHeightCoordinat(p.rc.top, psDiag) - FRAMEWIDTH / 2;

        if (p.bRectAllowed) {
            p.iMaxBase = p.rc.bottom;
            p.iMinTop = p.rc.top;
        }
        else {
            auto& polygon = p.apPolygon;

            p.iMaxBase = 0;
            p.iMinTop = std::numeric_limits<PixelScalar>::max();
            
            for(auto& pt : polygon) {
                pt.x = static_cast<PixelScalar>((pt.x * dx1) + x0);
                pt.y = CalcHeightCoordinat(pt.y, psDiag);
                p.iMaxBase = std::max(p.iMaxBase, pt.y);
                p.iMinTop = std::min(p.iMinTop, pt.y);
            }
        }
    }
    /**********************************************************************************
     * draw airspaces
     **********************************************************************************/
    const auto oldpen = Surface.SelectObject(LK_NULL_PEN);

    for (const auto& item : Sideview_pHandeled) {

        int type = item.iType;
        RECT rcd = item.rc;
        LKColor FrameColor;
        double fFrameColFact;
        int Framewidth = FRAMEWIDTH;
        if (item.bEnabled) {
            if ((MapWindow::GetAirSpaceFillType() == MapWindow::asp_fill_border_only) || (item.psAS->DrawStyle()== adsOutline)) {
                Surface.SelectObject(LKBrush_Hollow);
                Framewidth *=3;
            }
            else {
                Surface.SelectObject(MapWindow::GetAirspaceBrushByClass(type));
            }
            Surface.SetTextColor(MapWindow::GetAirspaceColourByClass(type));
            fFrameColFact = 0.8;
            FrameColor = MapWindow::GetAirspaceColourByClass(type);
        } else {
            Surface.SelectObject(LKBrush_Hollow);
            Surface.SetTextColor(RGB_GGREY);
            FrameColor = RGB_GGREY;
            fFrameColFact = 1.2;
        }
        if (INVERTCOLORS)
            fFrameColFact *= 0.8;
        else
            fFrameColFact *= 1.2;
        LKColor Color = FrameColor.ChangeBrightness(fFrameColFact);
        LKPen mpen2(PEN_SOLID, Framewidth, Color);
        const auto oldpen2 = Surface.SelectObject(mpen2);

        if (item.bRectAllowed == true)
            Surface.Rectangle(rcd.left + 1, rcd.top, rcd.right, rcd.bottom);
        else
            Surface.Polygon(item.apPolygon);

        Surface.SelectObject(oldpen2);

        if (item.bEnabled)
            Surface.SetTextColor(Sideview_TextColor); // RGB_MENUTITLEFG
        else
            Surface.SetTextColor(RGB_GGREY);

        /***********************************************
         * build view overlap for centering text
         ***********************************************/
        rcd.bottom = std::min({rcd.bottom, item.iMaxBase, rc.bottom});
        rcd.top = std::max({rcd.top, item.iMinTop, rc.top});
        rcd.left = max(rcd.left, rc.left);
        rcd.right = min(rcd.right, rc.right);

        SIZE textsize;
        SIZE aispacesize = {
            rcd.right - rcd.left,
            rcd.bottom - rcd.top
        };

        const TCHAR* text = item.szAS_Name;
        Surface.GetTextSize(text, &textsize);

        int x = rcd.left + aispacesize.cx / 2;
        int y = rcd.top + aispacesize.cy / 2;
        int iOffset = 0;
        BOOL blongtext = false;
        if (aispacesize.cy > (2 * textsize.cy) && (textsize.cx < aispacesize.cx)) {
            iOffset = textsize.cy / 2;
        }

        if ((textsize.cx < aispacesize.cx) && (textsize.cy < aispacesize.cy)) {
            Surface.DrawText(x - textsize.cx / 2, y - iOffset - textsize.cy / 2, text);
            blongtext = true;
        }

        text = CAirspaceManager::GetAirspaceTypeShortText(item.iType);
        Surface.GetTextSize(text, &textsize);
        if (textsize.cx < aispacesize.cx) {
            if (2 * textsize.cy < aispacesize.cy) {
                Surface.DrawText(x - textsize.cx / 2, y + iOffset - textsize.cy / 2, text);
            } else {
                if ((textsize.cy < aispacesize.cy) && (!blongtext))
                    Surface.DrawText(x - textsize.cx / 2, y - iOffset - textsize.cy / 2, text);
            }
        }
    }
    Surface.SelectObject(oldpen);

    /**********************************************************************************
     * draw airspace frames in reversed order
     **********************************************************************************/
#ifdef OUTLINE_2ND
    for (const auto& item : Sideview_pHandeled) {
        if (item.bEnabled) {
            int type = item.iType;
            RECT rcd = item.rc;
            LKColor FrameColor = MapWindow::GetAirspaceColourByClass(type);
            double fFrameColFact;
            Surface.SelectObject(LKBrush_Hollow);
            if (item.bEnabled) {
                //		Surface.SelectObject(MapWindow::GetAirspaceBrushByClass(type));
                Surface.SetTextColor(MapWindow::GetAirspaceColourByClass(type));
                fFrameColFact = 0.8;
            } else {
                Surface.SetTextColor(RGB_GGREY);
                FrameColor = RGB_GGREY;
                fFrameColFact = 1.2;
            }

            if (INVERTCOLORS)
                fFrameColFact *= 0.8;
            else
                fFrameColFact *= 1.2;
            LKColor lColor = FrameColor.ChangeBrightness(fFrameColFact);
            LKPen mpen2(PEN_SOLID, FRAMEWIDTH, lColor);
            const auto oldpen2 = Surface.SelectObject(mpen2);

            if (item.bRectAllowed == true)
                Surface.Rectangle(rcd.left + 1, rcd.top, rcd.right, rcd.bottom);
            else
                Surface.Polygon(item.apPolygon);


            Surface.SelectObject(oldpen2);
        }
    }
#endif

    /*************************************************************
     * draw ground
     *************************************************************/

    // draw ground

    /*********************************************************************
     * draw terrain
     *********************************************************************/
    LKPen hpHorizonGround(PEN_SOLID, IBLSCALE(1) + 1, LKColor(126, 62, 50));
    LKBrush hbHorizonGround(GROUND_COLOUR);
    const auto oldPen = Surface.SelectObject(hpHorizonGround);
    const auto oldBrush = Surface.SelectObject(hbHorizonGround);

    for (int j = 0; j < AIRSPACE_SCANSIZE_X; j++) { // scan range
        apTerrainPolygon[j].x = iround(j * dx1) + x0;
        apTerrainPolygon[j].y = CalcHeightCoordinat(d_h[j], psDiag);
    }

    apTerrainPolygon[AIRSPACE_SCANSIZE_X].x = iround(AIRSPACE_SCANSIZE_X * dx1) + x0;
    apTerrainPolygon[AIRSPACE_SCANSIZE_X].y = CalcHeightCoordinat(0, psDiag); //iBottom;

    apTerrainPolygon[AIRSPACE_SCANSIZE_X + 1].x = iround(0 * dx1) + x0; //iround(j*dx1)+x0;
    apTerrainPolygon[AIRSPACE_SCANSIZE_X + 1].y = CalcHeightCoordinat(0, psDiag); //iBottom;

    apTerrainPolygon[AIRSPACE_SCANSIZE_X + 2] = apTerrainPolygon[0];

    static_assert(std::size(apTerrainPolygon) >= AIRSPACE_SCANSIZE_X + 3, "wrong array size");
    Surface.Polygon(apTerrainPolygon, AIRSPACE_SCANSIZE_X + 3);

    Surface.SelectObject(oldPen);
    Surface.SelectObject(oldBrush);

    /*********************************************************************
     * draw sea
     *********************************************************************/
#ifdef MSL_SEA_DRAW
    // draw sea
    if (psDiag->fYMin < GC_SEA_LEVEL_TOLERANCE) {
        RECT sea = {rc.left, rc.bottom, rc.right, rc.bottom + SV_BORDER_Y};
      if (!IsDithered()) {
        RenderSky(Surface, sea, RGB_STEEL_BLUE, RGB_ROYAL_BLUE, 7);
      } else {
        RenderSky(Surface, sea, RGB_BLACK, RGB_BLACK, 2);
      }
    }
#else
    if (psDiag->fYMin < GC_SEA_LEVEL_TOLERANCE)
        Rectangle(hdc, rc.left, rc.bottom, rc.right, rc.bottom + BORDER_Y);
#endif

    Surface.SetTextColor(Sideview_TextColor); // RGB_MENUTITLEFG
}
