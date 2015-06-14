/*

   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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

void MapWindow::DrawVisualGlide(LKSurface& Surface, DiagrammStruct* pDia) {

    unsigned short numboxrows = 1;

#if BUGSTOP
    LKASSERT(Current_Multimap_SizeY < SIZE4);
#endif
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

    TCHAR tmpT[30];

    switch (ScreenSize) {
        case ss800x480:
#ifdef __linux__
            _tcscpy(tmpT, _T("MMMMMMMM"));
#else
            _tcscpy(tmpT, _T("MMMMMMMMM"));
#endif
            line1Font = LK8GenericVar03Font;
            line2Font = CDIWindowFont;
            break;
        case ss480x272:
        case ss320x240:
            _tcscpy(tmpT, _T("MMMMMMMM"));
            line1Font = LK8GenericVar03Font;
            line2Font = LK8InfoSmallFont;
            break;
        case ss640x480:
            _tcscpy(tmpT, _T("MMMMMM"));
            line1Font = CDIWindowFont;
            line2Font = CDIWindowFont;
            break;

        case ss272x480:
            _tcscpy(tmpT, _T("MMMMMMMM"));
            line1Font = LK8GenericVar03Font;
            line2Font = LK8PanelSmallFont;
            break;

        case ss480x800:
        case ss480x640:
        case ss240x320:
        case ss240x400:
            _tcscpy(tmpT, _T("MMMMMMM"));
            line1Font = LK8PanelSmallFont;
            line2Font = LK8PanelSmallFont;
            break;
        case ss600x800:
            _tcscpy(tmpT, _T("MMMMMMMM"));
            line2Font = LK8GenericVar01Font;
            line1Font = LK8GenericVar02Font;
            break;
        case ss800x600:
            _tcscpy(tmpT, _T("MMMMMMMM"));
            line1Font = CDIWindowFont;
            line2Font = CDIWindowFont;
            break;
        default:
#ifdef __linux__
            if (!ScreenLandscape) {
                _tcscpy(tmpT, _T("MMMMMMM"));
                line1Font = LK8PanelSmallFont;
                line2Font = LK8PanelSmallFont;
            } else {
                _tcscpy(tmpT, _T("MMMMMMM"));
                line1Font = CDIWindowFont;
                line2Font = CDIWindowFont;
            }
#else
            _tcscpy(tmpT, _T("MMMMMMM"));
            line1Font = LK8PanelSmallFont;
            line2Font = LK8PanelSmallFont;
#endif
            break;
    }


    SIZE textSize;
    Surface.SelectObject(line1Font);
    Surface.GetTextSize(tmpT, _tcslen(tmpT), &textSize);
    maxtSizeX = textSize.cx;

    int variooffset=0;
    if (LKVarioBar > 0 && LKVarioBar <= vBarVarioGR)
        if (IsMultimapOverlaysGauges())
            variooffset = LKVarioSize;

    int a = (MapRect.right-MapRect.left-variooffset) / textSize.cx;
    int b = (MapRect.right-MapRect.left-variooffset) - a * (textSize.cx)-(BOXINTERVAL * (a + 1));

    boxSizeX = textSize.cx + (b / (a + 1));
    boxSizeY = textSize.cy + 1; // distance from bottombar

    if (numboxrows > 1) {
        Surface.SelectObject(line2Font);
        Surface.GetTextSize(tmpT, _tcslen(tmpT), &textSize);
        boxSizeY += (textSize.cy * (numboxrows - 1)) - NIBLSCALE(2);
        if (numboxrows > 2) boxSizeY -= NIBLSCALE(1);
    }

#if DEBUG_SCR
    StartupStore(_T("boxX=%d boxY=%d  \n"), boxSizeX, boxSizeY);
#endif

    RECT vrc;
    vrc.left = MapRect.left+variooffset;
    vrc.right = MapRect.right;
    vrc.bottom = MapRect.bottom - BottomSize;
    if (Current_Multimap_SizeY == SIZE0) // Full screen?
        vrc.top = MapRect.top;
    else
        vrc.top = pDia->rc.bottom;

#if DEBUG_SCR
    StartupStore(_T("VG AREA LTRB: %d,%d %d,%d\n"), vrc.left, vrc.top, vrc.right, vrc.bottom);
#endif

    const auto oldBrush = Surface.SelectObject(LKBrush_White);
    const auto oldPen = Surface.SelectObject(LK_BLACK_PEN);

    BrushReference brush_back;
    if (!INVERTCOLORS) {
        brush_back = LKBrush_Black;
    } else {
        brush_back = LKBrush_Nlight;
    }

    Surface.FillRect(&vrc, brush_back);

    POINT center, p1, p2;
    center.y = vrc.top + (vrc.bottom - vrc.top) / 2;
    center.x = vrc.left + (vrc.right - vrc.left) / 2;

    // numSlotX is the number items we can print horizontally.
    unsigned short numSlotX = (vrc.right - vrc.left) / (boxSizeX + BOXINTERVAL);
    if (numSlotX > MAXBSLOT) numSlotX = MAXBSLOT;
#if BUGSTOP
    LKASSERT(numSlotX > 0);
#endif
    if (numSlotX == 0) return;

    unsigned short boxInterval = ((vrc.right - vrc.left)-(boxSizeX * numSlotX)) / (numSlotX + 1);
    unsigned short oddoffset = ( (MapRect.right-MapRect.left-variooffset) - (boxSizeX * numSlotX) - boxInterval * (numSlotX + 1)) / 2;

    /*
    #if BUGSTOP
    // not really harmful
    LKASSERT(oddoffset<=boxInterval);
    #endif
     */

#if DEBUG_SCR
    StartupStore(_T("numSlotX=%d ScreenSizeX=%d boxSizeX=%d interval=%d offset=%d\n"), numSlotX, ScreenSizeX, boxSizeX, boxInterval, oddoffset);
#endif

    unsigned int t;

    // The horizontal grid
    unsigned int slotCenterX[MAXBSLOT + 1];
    for (t = 0; t < numSlotX; t++) {
        slotCenterX[t] = (t * boxSizeX) + boxInterval * (t + 1)+(boxSizeX / 2) + oddoffset+MapRect.left+variooffset;
#if DEBUG_SCR
        StartupStore(_T("slotCenterX[%d]=%d\n"), t, slotCenterX[t]);
#endif
    }

    // Vertical coordinates of each up/down subwindow, excluding center line
    int upYtop = vrc.top;
#if MIDCENTER
    int upYbottom = center.y + (boxSizeY / 2);
    int downYtop = center.y - (boxSizeY / 2);
#else
    int upYbottom = center.y - CENTERYSPACE;
    int downYtop = center.y + CENTERYSPACE;
#endif
    int upSizeY = upYbottom - upYtop - (boxSizeY);
    ;
    int downYbottom = vrc.bottom;
    int downSizeY = downYbottom - downYtop - (boxSizeY);
    ;

#if 0
    // Reassign dynamically the vertical scale for each subwindow size
    double vscale = 1000 * (100 - Current_Multimap_SizeY) / 100;
#else
    // Set the vertical range 
    double vscale;
    if (Units::GetUserAltitudeUnit() == unFeet)
        vscale = (1000 / TOFEET);
    else
        vscale = 300.0;
#endif



    Surface.SetBackgroundTransparent();

    RECT trc;
    trc = vrc;

    // Top part of visual rect, target is over us=unreachable=red
    trc.top = vrc.top;
    trc.bottom = center.y - 1;
    #ifndef UNDITHER
    RenderSky(Surface, trc, RGB_WHITE, LKColor(150, 255, 150), GC_NO_COLOR_STEPS / 2);
    #else
    RenderSky(Surface, trc, RGB_WHITE, RGB_WHITE, GC_NO_COLOR_STEPS / 2);
    #endif
    // Bottom part, target is below us=reachable=green
    trc.top = center.y + 1;
    trc.bottom = vrc.bottom;
    #ifndef UNDITHER
    RenderSky(Surface, trc, LKColor(255, 150, 150), RGB_WHITE, GC_NO_COLOR_STEPS / 2);
    #else
    RenderSky(Surface, trc, RGB_WHITE, RGB_WHITE,GC_NO_COLOR_STEPS / 2);
    #endif

    // Draw center line
    p1.x = vrc.left + 1;
    p1.y = center.y;
    p2.x = vrc.right - 1;
    p2.y = center.y;
    Surface.SelectObject(LKPen_Black_N1);
    Surface.DrawSolidLine(p1, p2, vrc);

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

    p1.y = vrc.top + 1;
    p2.y = vrc.bottom - 1;
    Surface.SelectObject(LKPen_Black_N1);
    Surface.DrawSolidLine(p1, p2, vrc);



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
#ifdef UNDITHER
                    rgbcolor = RGB_WHITE;
#else
                    rgbcolor = RGB_LIGHTGREEN;
#endif
                }
            }
            bcolor.Create(rgbcolor);

        } else {
            double d = vscale / altdiff;
            if (d == 0) d = -1;
            ty = downYtop - (int) ((double) downSizeY / d)+(boxSizeY / 2); // - because the left part is negative, we are really adding.
            if ((ty - offset) < downYtop) ty = downYtop + offset;
            if ((ty + offset) > downYbottom) ty = downYbottom - offset;

#ifdef UNDITHER
            rgbcolor = RGB_WHITE; // negative part, no need to render dark
#else
            rgbcolor = RGB_LIGHTRED;
#endif
            bcolor.Create(rgbcolor);
        }

        TCHAR line2[40], line3[40];
        TCHAR value[40], unit[30];
        TCHAR name[NAME_SIZE + 1];
        double ar = (WayPointCalc[wp].AltArriv[AltArrivMode] * ALTITUDEMODIFY);
        _tcscpy(name, WayPointList[wp].Name);
        CharUpper(name);

        if (IsSafetyAltitudeInUse(wp))
            textcolor = RGB_DARKBLUE;
        else
            textcolor = RGB_BLACK;

        switch (numboxrows) {
            case 0:
#if BUGSTOP
                LKASSERT(0);
#endif
                return;

            case 1:
                // 1 line: waypoint name
                VGTextInBox(Surface, n, 1, name, NULL, NULL, slotCenterX[n], ty, textcolor, bcolor);
                break;

            case 2:
                // 2 lines: waypoint name + altdiff
                LKFormatAltDiff(wp, false, value, unit);
                // Should we print also the GR?
                if ((ar >= -9999 && ar <= 9999) && (WayPointCalc[wp].GR < MAXEFFICIENCYSHOW)) {
                    if (ar >= -999 && ar <= 999)
                        _stprintf(line2, _T("%s   "), value);
                    else
                        _stprintf(line2, _T("%s  "), value);
                    LKFormatGR(wp, false, value, unit);
                    _tcscat(line2, value);
                } else {
                    _stprintf(line2, _T("%s   ---"), value);
                }

                VGTextInBox(Surface, n, 2, name, line2, NULL, slotCenterX[n], ty, textcolor, bcolor);
                break;

            case 3:
                // 3 lines: waypoint name + dist + altdiff
                LKFormatDist(wp, false, value, unit);
                _stprintf(line2, _T("%s%s"), value, unit);

                LKFormatBrgDiff(wp, false, value, unit);
                _stprintf(tmpT, _T(" %s%s"), value, unit);
                _tcscat(line2, tmpT);

                LKFormatAltDiff(wp, false, value, unit);
                // Should we print also the GR?
                if ((ar >= -9999 && ar <= 9999) && (WayPointCalc[wp].GR < MAXEFFICIENCYSHOW)) {
                    if (ar >= -999 && ar <= 999)
                        _stprintf(line3, _T("%s   "), value);
                    else
                        _stprintf(line3, _T("%s  "), value);
                    LKFormatGR(wp, false, value, unit);
                    _tcscat(line3, value);
                } else {
                    _stprintf(line3, _T("%s   ---"), value);
                }

                VGTextInBox(Surface, n, 3, name, line2, line3, slotCenterX[n], ty, textcolor, bcolor);
                break;
            default:
#if BUGSTOP
                LKASSERT(0);
#endif
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

#if BUGSTOP
    LKASSERT(wText1 != NULL);
#endif
    if (!wText1) return;

    LKColor oldTextColor = Surface.SetTextColor(trgb);

    SIZE tsize;
    int tx, ty;


    Sideview_VGBox_Number++;

    Surface.SelectObject(line1Font);
    unsigned int tlen = _tcslen(wText1);
    Surface.GetTextSize(wText1, tlen, &tsize);
    int line1fontYsize = tsize.cy;

    // Fit as many characters in the available boxed space
    if (tsize.cx > maxtSizeX) {
        LKASSERT(tlen > 0);
        for (short t = tlen - 1; t > 0; t--) {
            Surface.GetTextSize(wText1, t, &tsize);
            if (tsize.cx <= maxtSizeX) {
                tlen = t;
                break;
            }
        }
    }

    short vy = y + (boxSizeY / 2);
    Surface.SelectObject(bbrush);
    Surface.Rectangle(
            x - (boxSizeX / 2),
            y - (boxSizeY / 2),
            x + (boxSizeX / 2),
            vy);

    Sideview_VGBox[nslot].top = y - (boxSizeY / 2);
    Sideview_VGBox[nslot].left = x - (boxSizeX / 2);
    Sideview_VGBox[nslot].bottom = vy;
    Sideview_VGBox[nslot].right = x + (boxSizeX / 2);

    // 
    // LINE 1
    // 
    tx = x - (tsize.cx / 2);
    ty = y - (vy - y);

    Surface.DrawText(tx, ty, wText1, tlen);

    if (numlines == 1) goto _end;
#if BUGSTOP
    LKASSERT(wText2 != NULL);
#endif
    if (!wText2) goto _end;

    //
    // LINE 2
    // 
    Surface.SetTextColor(RGB_BLACK);
    Surface.SelectObject(line2Font);
    tlen = _tcslen(wText2);
    Surface.GetTextSize(wText2, tlen, &tsize);
    tx = x - (tsize.cx / 2);
    ty += tsize.cy - NIBLSCALE(2);
    if ((line1fontYsize - tsize.cy) > 0) ty -= (line1fontYsize - tsize.cy);
    Surface.DrawText(tx, ty, wText2, tlen);

    if (numlines == 2) goto _end;
#if BUGSTOP
    LKASSERT(wText3 != NULL);
#endif
    if (!wText3) goto _end;

    //
    // LINE 3
    //
    Surface.SetTextColor(RGB_BLACK);
    tlen = _tcslen(wText3);
    Surface.GetTextSize(wText3, tlen, &tsize);
    tx = x - (tsize.cx / 2);
    ty += tsize.cy - NIBLSCALE(1);
    Surface.DrawText(tx, ty, wText3, tlen);

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


