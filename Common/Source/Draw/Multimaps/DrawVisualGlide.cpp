/*

   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "Multimap.h"
#include "Sideview.h"
#include "LKObjects.h"
#include "RGB.h"
#include "LKStyle.h"
#include "Screen/FontReference.h"
#include "Asset.hpp"

extern short GetVisualGlidePoints(unsigned short numslots);
extern bool CheckLandableReachableTerrainNew(NMEA_INFO *Basic, DERIVED_INFO *Calculated, double LegToGo, double LegBearing);
extern void ResetVisualGlideGlobals(void);


// border margins in boxed text
#define XYMARGIN NIBLSCALE(1)

// minimum space between two adjacent boxes on the same row
#define BOXINTERVAL NIBLSCALE(1)

// space between row0 and center line
#define CENTERYSPACE NIBLSCALE(1)

// Size of the box, fixed for each waypoint at this resolution
static unsigned int boxSizeX = 0, boxSizeY = 0;
static int maxtSizeX = 0;
static FontReference line1Font, line2Font;

extern int slotWpIndex[MAXBSLOT + 1];

// This is used to check for key presses inside boxes, to trigger wp details
RECT Sideview_VGBox[MAXBSLOT + 1];
int Sideview_VGWpt[MAXBSLOT + 1];
// This is used to know in advance if we do have painted boxes around
unsigned short Sideview_VGBox_Number = 0;


//#define DEBUG_DVG	1
//#define DEBUG_SCR	1

// Use center middle line for scaling
// #define MIDCENTER	1

void MapWindow::DrawVisualGlide(LKSurface& Surface, const DiagrammStruct& sDia) {

    const RECT& rci = sDia.rc;

    unsigned short numboxrows = 1;

    BUGSTOP_LKASSERT(Current_Multimap_SizeY < SIZE4);
    switch (Current_Multimap_SizeY) {
        case SIZE0:
        case SIZE1:
            numboxrows = 3;
            break;
        case SIZE2:
            numboxrows = 2;
            break;
        case SIZE3:
            numboxrows = 1;
            break;
        case SIZE4:
            return;
        default:
            LKASSERT(0);
            break;
    }

    if (!ScreenLandscape) {
        numboxrows++;
        if (numboxrows > 3) numboxrows = 3;
    }

    TCHAR tmpT[70];

    line1Font = LK8VisualTopFont;
    line2Font = LK8VisualBotFont;
    SIZE textSizeTop, textSizeBot;
    Surface.SelectObject(line1Font);
    _stprintf(tmpT, _T("MMMM"));
    Surface.GetTextSize(tmpT, &textSizeTop);
    Surface.SelectObject(line2Font);
    _stprintf(tmpT, _T("55.5%s 79%s%s "), Units::GetDistanceName(), MsgToken<2179>(), MsgToken<2183>());
    Surface.GetTextSize(tmpT, &textSizeBot);

    // we can cut the waypoint name, but not the value data, so we use the second row of data
    // to size the box for everything.
    maxtSizeX = textSizeBot.cx;

    int a = (rci.right-rci.left) / (textSizeBot.cx+BOXINTERVAL);
    int b = (rci.right-rci.left) - a * (textSizeBot.cx)-(BOXINTERVAL * (a + 1));

    boxSizeX = textSizeBot.cx + (b / (a + 1));
    boxSizeY = textSizeTop.cy + 1; // single line (wp name) + distance from bottombar

    if (numboxrows > 1) {
        boxSizeY += (textSizeBot.cy * (numboxrows - 1)) - NIBLSCALE(2);
        if (numboxrows > 2) boxSizeY -= NIBLSCALE(1);
    }

#if DEBUG_SCR
    StartupStore(_T("boxX=%d boxY=%d  \n"), boxSizeX, boxSizeY);
#endif

#if DEBUG_SCR
    StartupStore(_T("VG AREA LTRB: %d,%d %d,%d\n"), rci.left, rci.top, rci.right, rci.bottom);
#endif

    const auto oldBrush = Surface.SelectObject(LKBrush_White);
    const auto oldPen = Surface.SelectObject(LK_BLACK_PEN);

    BrushReference brush_back;
    if (!INVERTCOLORS) {
        brush_back = LKBrush_Black;
    } else {
        brush_back = LKBrush_Nlight;
    }

    Surface.FillRect(&rci, brush_back);

    POINT center, p1, p2;
    center.y = rci.top + (rci.bottom - rci.top) / 2;
    center.x = rci.left + (rci.right - rci.left) / 2;

    // numSlotX is the number items we can print horizontally.
    unsigned short numSlotX = (rci.right - rci.left) / (boxSizeX + BOXINTERVAL);
    if (numSlotX > MAXBSLOT) numSlotX = MAXBSLOT;
    BUGSTOP_LKASSERT(numSlotX > 0);
    if (numSlotX == 0) return;

    unsigned short boxInterval = ((rci.right - rci.left)-(boxSizeX * numSlotX)) / (numSlotX + 1);
    unsigned short oddoffset = ( (rci.right-rci.left) - (boxSizeX * numSlotX) - boxInterval * (numSlotX + 1)) / 2;

    /*
    // not really harmful
    BUGSTOP_LKASSERT(oddoffset<=boxInterval);
     */

#if DEBUG_SCR
    StartupStore(_T("numSlotX=%d ScreenSizeX=%d boxSizeX=%d interval=%d offset=%d\n"), numSlotX, ScreenSizeX, boxSizeX, boxInterval, oddoffset);
#endif

    unsigned int t;

    // The horizontal grid
    unsigned int slotCenterX[MAXBSLOT + 1];
    for (t = 0; t < numSlotX; t++) {
        slotCenterX[t] = (t * boxSizeX) + boxInterval * (t + 1)+(boxSizeX / 2) + oddoffset+rci.left;
#if DEBUG_SCR
        StartupStore(_T("slotCenterX[%d]=%d\n"), t, slotCenterX[t]);
#endif
    }

    // Vertical coordinates of each up/down subwindow, excluding center line
    int upYtop = rci.top;
#if MIDCENTER
    int upYbottom = center.y + (boxSizeY / 2);
    int downYtop = center.y - (boxSizeY / 2);
#else
    int upYbottom = center.y - CENTERYSPACE;
    int downYtop = center.y + CENTERYSPACE;
#endif
    int upSizeY = upYbottom - upYtop - (boxSizeY);
    ;
    int downYbottom = rci.bottom;
    int downSizeY = downYbottom - downYtop - (boxSizeY);
    ;

#if 0
    // Reassign dynamically the vertical scale for each subwindow size
    double vscale = 1000 * (100 - Current_Multimap_SizeY) / 100;
#else
    // Set the vertical range
    double vscale;
    if (Units::GetUserAltitudeUnit() == unFeet)
        vscale = Units::ToSys(unFeet, 1000);
    else
        vscale = 300.0;
#endif



    Surface.SetBackgroundTransparent();

    RECT trc = rci;

    // Top part of visual rect, target is over us=unreachable=red
    trc.top = rci.top;
    trc.bottom = center.y - 1;
    if (!IsDithered()) {
        RenderSky(Surface, trc, RGB_WHITE, LKColor(150, 255, 150), GC_NO_COLOR_STEPS / 2);
    } else {
        RenderSky(Surface, trc, RGB_WHITE, RGB_WHITE, GC_NO_COLOR_STEPS / 2);
    }
    // Bottom part, target is below us=reachable=green
    trc.top = center.y + 1;
    trc.bottom = rci.bottom;
    if (!IsDithered()) {
        RenderSky(Surface, trc, LKColor(255, 150, 150), RGB_WHITE, GC_NO_COLOR_STEPS / 2);
    } else {
        RenderSky(Surface, trc, RGB_WHITE, RGB_WHITE, GC_NO_COLOR_STEPS / 2);
    }

    // Draw center line
    p1.x = rci.left + 1;
    p1.y = center.y;
    p2.x = rci.right - 1;
    p2.y = center.y;
    Surface.SelectObject(LKPen_Black_N1);
    Surface.DrawSolidLine(p1, p2, rci);

#if DEBUG_SCR
    StartupStore(_T("... Center line: Y=%d\n"), center.y);
#endif

    Surface.SelectObject(line1Font);
    Surface.SelectObject(LKPen_Black_N0);

    ResetVisualGlideGlobals();

    short res = GetVisualGlidePoints(numSlotX);

    if (res == INVALID_VALUE) {
#if DEBUG_DVG
        StartupStore(_T("...... GVGP says not ready, wait..\n"));
#endif
        return;
    }
    if (res == 0) {
#if DEBUG_DVG
        StartupStore(_T("...... GVGP says no data available!\n"));
#endif
        return;
    }

    // Print them all!
    int offset = (boxSizeY / 2) + CENTERYSPACE;

    LKBrush bcolor;
    LKColor rgbcolor, textcolor;
    int wp;

    unsigned short zeroslot = 0;
    double minbrgdiff = 999.0;
    double minabrgdiff = 999.0; // absolute never negative
    for (unsigned short n = 0; n < numSlotX; n++) {
        wp = slotWpIndex[n];
        if (!ValidWayPoint(wp)) {
            // empty slot nothing to print
            continue;
        }
        double brgdiff = WayPointCalc[wp].Bearing - DrawInfo.TrackBearing;
        // this check is worthless
        if (brgdiff < -180.0) {
            brgdiff += 360.0;
        } else {
            if (brgdiff > 180.0) brgdiff -= 360.0;
        }
        double abrgdiff = brgdiff;
        if (abrgdiff < 0) abrgdiff *= -1;

        if (abrgdiff < minabrgdiff) {
            zeroslot = n;
            minabrgdiff = abrgdiff;
            minbrgdiff = brgdiff;
        }
    }

    // Draw vertical line
#define DEGRANGE 10	// degrees left and right to perfect target
    if (minabrgdiff < 1) {
        p1.x = slotCenterX[zeroslot];
    } else {
        // set fullscale range
        if (minabrgdiff > DEGRANGE) {
            minabrgdiff = DEGRANGE;
            if (minbrgdiff < 0)
                minbrgdiff = -1 * DEGRANGE;
            else
                minbrgdiff = DEGRANGE;
        }
        // we shift of course in the opposite direction
        p1.x = slotCenterX[zeroslot]-(int) ((boxSizeX / (DEGRANGE * 2)) * minbrgdiff);
    }
    p2.x = p1.x;

    p1.y = rci.top + 1;
    p2.y = rci.bottom - 1;
    Surface.SelectObject(LKPen_Black_N1);
    Surface.DrawSolidLine(p1, p2, rci);



    for (unsigned short n = 0; n < numSlotX; n++) {

        wp = slotWpIndex[n];
        if (!ValidWayPoint(wp)) {
            // empty slot nothing to print
            continue;
        }
#if DEBUG_DVG
        StartupStore(_T("... DVG PRINT [%d]=%d <%s>\n"), n, wp, WayPointList[wp].Name);
#endif

        Sideview_VGWpt[n] = wp;

        double altdiff = WayPointCalc[wp].AltArriv[AltArrivMode];
        int ty;
#if DEBUG_SCR
        StartupStore(_T("... wp=<%s>\n"), WayPointList[wp].Name);
#endif

        // Since terrain can be approximated due to low precision maps, or waypoint position or altitude error,
        // we have a common problem: we get an obstacle to get to the waypoint because it is
        // positioned "BELOW" the terrain itself. We try to reduce this problem here.
#define SAFETERRAIN	50

        // Positive arrival altitude for the waypoint, upper window
        if (altdiff >= 0) {
            if (altdiff == 0)altdiff = 1;
            double d = vscale / altdiff;
            if (d == 0) d = 1;
            ty = upYbottom - (int) ((double) upSizeY / d)-(boxSizeY / 2);
#if DEBUG_SCR
            StartupStore(_T("... upYbottom=%d upSizeY=%d / (vscale=%f/altdiff=%f = %f) =- %d  ty=%d  offset=%d\n"),
                    upYbottom, upSizeY, vscale, altdiff, d, (int) ((double) upSizeY / d), ty, offset);
#endif
            if ((ty - offset) < upYtop) ty = upYtop + offset;
            if ((ty + offset) > upYbottom) ty = upYbottom - offset;
#if DEBUG_SCR
            StartupStore(_T("... upYtop=%d upYbottom=%d final ty=%d\n"), upYtop, upYbottom, ty);
#endif


            //
            // This is too confusing. We want simple colors, not shaded
            // rgbcolor = MixColors( LKColor(50,255,50), LKColor(230,255,230),  altdiff/(vscale-50));
            //

            if (altdiff <= SAFETERRAIN) {
                rgbcolor = RGB_LIGHTYELLOW;
            } else {
                if (!CheckLandableReachableTerrainNew(&DrawInfo, &DerivedDrawInfo,
                        WayPointCalc[wp].Distance, WayPointCalc[wp].Bearing)) {
                    rgbcolor = RGB_LIGHTRED;
                } else {
                    rgbcolor = IsDithered() ? RGB_WHITE : RGB_LIGHTGREEN;
                }
            }
            bcolor.Create(rgbcolor);

        } else {
            double d = vscale / altdiff;
            if (d == 0) d = -1;
            ty = downYtop - (int) ((double) downSizeY / d)+(boxSizeY / 2); // - because the left part is negative, we are really adding.
            if ((ty - offset) < downYtop) ty = downYtop + offset;
            if ((ty + offset) > downYbottom) ty = downYbottom - offset;

            rgbcolor = IsDithered() ? RGB_WHITE : RGB_LIGHTRED; // negative part, no need to render dark
            bcolor.Create(rgbcolor);
        }

        TCHAR value[40], unit[30];
        TCHAR line2[140], line3[50];
        TCHAR name[NAME_SIZE + 1];
        double ar = Units::ToUserAltitude(WayPointCalc[wp].AltArriv[AltArrivMode]);
        _tcscpy(name, WayPointList[wp].Name);
        CharUpper(name);

        if (IsSafetyAltitudeInUse(wp))
            textcolor = RGB_DARKBLUE;
        else
            textcolor = RGB_BLACK;

        switch (numboxrows) {
            case 0:
                BUGSTOP_LKASSERT(0);
                return;

            case 1:
                // 1 line: waypoint name
                VGTextInBox(Surface, n, 1, name, NULL, NULL, slotCenterX[n], ty, textcolor, bcolor);
                break;

            case 2:
                // 2 lines: waypoint name + altdiff
                LKFormatAltDiff(wp, value, unit);
                // Should we print also the GR?
                if ((ar >= -9999 && ar <= 9999) && (WayPointCalc[wp].GR < MAXEFFICIENCYSHOW)) {
                    if (ar >= -999 && ar <= 999)
                        _stprintf(line2, _T("%s   "), value);
                    else
                        _stprintf(line2, _T("%s  "), value);
                    LKFormatGR(wp, value, unit);
                    _tcscat(line2, value);
                } else {
                    _stprintf(line2, _T("%s   ---"), value);
                }

                VGTextInBox(Surface, n, 2, name, line2, NULL, slotCenterX[n], ty, textcolor, bcolor);
                break;

            case 3:
                // 3 lines: waypoint name + dist + altdiff
                LKFormatDist(wp, value, unit);
                _stprintf(line2, _T("%s%s"), value, unit);

                LKFormatBrgDiff(wp, value, unit);
                _stprintf(tmpT, _T(" %s%s"), value, unit);
                _tcscat(line2, tmpT);

                LKFormatAltDiff(wp, value, unit);
                // Should we print also the GR?
                if ((ar >= -9999 && ar <= 9999) && (WayPointCalc[wp].GR < MAXEFFICIENCYSHOW)) {
                    if (ar >= -999 && ar <= 999)
                        _stprintf(line3, _T("%s   "), value);
                    else
                        _stprintf(line3, _T("%s  "), value);
                    LKFormatGR(wp, value, unit);
                    _tcscat(line3, value);
                } else {
                    _stprintf(line3, _T("%s   ---"), value);
                }

                VGTextInBox(Surface, n, 3, name, line2, line3, slotCenterX[n], ty, textcolor, bcolor);
                break;
            default:
                BUGSTOP_LKASSERT(0);
                return;
        }

    } // for numSlotX



    // Cleanup and return
    Surface.SelectObject(oldBrush);
    Surface.SelectObject(oldPen);
    return;
}




// trgb color is used only for line1, because using any other color than black  on thin characters on line 2 and 3 would
// make it quite less visible.

void MapWindow::VGTextInBox(LKSurface& Surface, unsigned short nslot, short numlines, const TCHAR* wText1, const TCHAR* wText2, const TCHAR *wText3, int x, int y, const LKColor& trgb, const LKBrush& bbrush) {

    BUGSTOP_LKASSERT(wText1 != NULL);
    if (!wText1) return;

    LKColor oldTextColor = Surface.SetTextColor(trgb);

    SIZE tsize;
    int tx, ty;


    Sideview_VGBox_Number++;

    Surface.SelectObject(line1Font);
    Surface.GetTextSize(wText1, &tsize);
    int line1fontYsize = tsize.cy;

    short vy = y + (boxSizeY / 2);
    Surface.SelectObject(bbrush);
    Surface.Rectangle(
            x - (boxSizeX / 2),
            y - (boxSizeY / 2)-1,
            x + (boxSizeX / 2),
            vy-1);

    Sideview_VGBox[nslot].top = y - (boxSizeY / 2);
    Sideview_VGBox[nslot].left = x - (boxSizeX / 2);
    Sideview_VGBox[nslot].bottom = vy;
    Sideview_VGBox[nslot].right = x + (boxSizeX / 2);

    PixelRect ClipRect(Sideview_VGBox[nslot]);
    ClipRect.Grow(-1*NIBLSCALE(2), 0); // text padding

    //
    // LINE 1
    //
    tx = max<PixelScalar>(ClipRect.left, x - (tsize.cx / 2));
    ty = y - (vy - y);

    Surface.DrawText(tx, ty, wText1, &ClipRect);

    if (numlines == 1) goto _end;
    BUGSTOP_LKASSERT(wText2 != NULL);
    if (!wText2) goto _end;

    //
    // LINE 2
    //
    Surface.SetTextColor(RGB_BLACK);
    Surface.SelectObject(line2Font);
    Surface.GetTextSize(wText2, &tsize);
    tx = x - (tsize.cx / 2);
    ty += line1fontYsize - NIBLSCALE(2);
    Surface.DrawText(tx, ty, wText2);

    if (numlines == 2) goto _end;
    BUGSTOP_LKASSERT(wText3 != NULL);
    if (!wText3) goto _end;

    //
    // LINE 3
    //
    Surface.SetTextColor(RGB_BLACK);
    Surface.GetTextSize(wText3, &tsize);
    tx = x - (tsize.cx / 2);
    ty += tsize.cy - NIBLSCALE(2);
    Surface.DrawText(tx, ty, wText3);

_end:
    Surface.SetTextColor(oldTextColor);
    return;
}

void ResetVisualGlideGlobals() {

    for (unsigned short i = 0; i < MAXBSLOT; i++) {
        Sideview_VGBox[i].top = 0;
        Sideview_VGBox[i].bottom = 0;
        Sideview_VGBox[i].left = 0;
        Sideview_VGBox[i].right = 0;

        Sideview_VGWpt[i] = INVALID_VALUE;
    }
    // This is not really needed because it is called in another part.
    // However it is better not to forget it!
    Sideview_VGBox_Number = 0;

}
