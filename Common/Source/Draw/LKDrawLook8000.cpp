/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawLook8000.cpp,v 1.11 2011/01/06 01:20:11 root Exp root $
   Rewritten in june 2015

*/

#include "externs.h"
#include "LKInterface.h"
#include "Logger.h"
#include "LKProcess.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"
#include "McReady.h"
#include "Multimap.h"
#include "Sideview.h"
#include "Screen/FontReference.h"
#ifndef UNICODE
#include "Util/UTF8.hpp"
#endif

#define AMBERCOLOR  RGB_AMBER
// The size of compass drawn by DrawCompass on topright
#define COMPASS_SPACE NIBLSCALE(25)  // 11*2 + some margins
// The vertical margin added by LKWriteBoxedText in addition to textsize.y
#define BOXEDYMARGIN  NIBLSCALE(2)+1
#define LEFTMARGIN    NIBLSCALE(2) // spacing on left border
#define RIGHTMARGIN   NIBLSCALE(2) // spacing on right border
#define MAPSCALE_SIZE NIBLSCALE(46)

// For Overlay we use :  0       : Hidden
//                       1       : Default
//                       >= 1000 : Custom LKValue+1000
//
constexpr bool isOverlayHidden(int Overlay) {return (Overlay==0);}
constexpr bool isOverlayCustom(int Overlay) {return (Overlay>=1000);}
constexpr int  getCustomOverlay(int Overlay) {return (Overlay-1000);}

/*
 * Draw Text Overlay.
 * @Surface : surface to draw
 * @rc : Screen rect, ex. 0 800  0 480
 */
void MapWindow::DrawLook8000(LKSurface& Surface, const RECT& rc) {

    // Locals
    LKSurface::OldFont oldfont;
    LKSurface::OldBrush oldbrush;
    LKSurface::OldPen oldpen;

    SIZE TextSize;
    LKColor color,distcolor;
    TCHAR Buffer[LKSIZEBUFFERLARGE];
    TCHAR BufferValue[LKSIZEBUFFERVALUE];
    TCHAR BufferUnit[LKSIZEBUFFERUNIT];
    TCHAR BufferTitle[LKSIZEBUFFERTITLE];
    int index = -1;
    double Value;
    short rcx, rcy;
    bool redwarning;
    int gatechrono = 0;

    short leftmargin = rc.left;

    // Statics
    static short rightmargin = 0, topmargin=0;
    static bool flipflop = true;
    static short flipflopcount = 0;

    static SIZE SizeBigFont;
    static SIZE SizeGatesFont;
    static SIZE SizeSmallFont;
    static SIZE SizeMcModeFont;
    static SIZE SizeMediumFont;
    static SIZE SizeUnitFont;
    static int name_xmax;

    static int yrightoffset, yMcSafety, yMcValue, yMcMode, yDistUnit, yAltSafety, yLeftWind;
    static SIZE compass, topbearing;
    static unsigned short fixBigInterline, unitbigoffset, unitmediumoffset;

    // This is going to be the START 1/3  name replacing waypoint name when gates are running
    static TCHAR StartGateName[12];

    redwarning = false;

    oldfont = Surface.SelectObject(LK8OverlaySmallFont);
    oldbrush = Surface.SelectObject(LKBrush_Black);
    oldpen = Surface.SelectObject(LKPen_Grey_N1);

    Surface.SetBackgroundTransparent();


    if (++flipflopcount > 2) {
        flipflop = !flipflop;
        flipflopcount = 0;
    }


    if (DoInit[MDI_DRAWLOOK8000]) {

        compass={rc.right-COMPASS_SPACE, rc.top+COMPASS_SPACE};

        TCHAR Tdummy[] = _T("E");
        Surface.SelectObject(LK8OverlayBigFont);
        Surface.GetTextSize(Tdummy, &SizeBigFont);

        Surface.SelectObject(LK8OverlayMediumFont);
        Surface.GetTextSize(Tdummy, &SizeMediumFont);

        Surface.SelectObject(LK8OverlaySmallFont);
        Surface.GetTextSize(Tdummy, &SizeSmallFont);

        Surface.SelectObject(LK8OverlayMcModeFont);
        Surface.GetTextSize(Tdummy, &SizeMcModeFont);

        Surface.SelectObject(LK8OverlayGatesFont);
        Surface.GetTextSize(Tdummy, &SizeGatesFont);

        Surface.SelectObject(MapScaleFont);
        Surface.GetTextSize(Tdummy, &SizeUnitFont);

        //
        // TIME GATES OFFSETS FOR PARAGLIDERS
        //
        if (!ScreenLandscape) {
            _tcscpy(StartGateName, _T("ST "));
        } else {
            _tcscpy(StartGateName, _T("Start "));
        }

        // the mid point on the right
        yrightoffset= ((rc.bottom - BottomSize - MAPSCALE_SIZE+NIBLSCALE(2)) + COMPASS_SPACE)/2 + SizeBigFont.cy/2;
        topmargin= rc.top+NIBLSCALE(1);
        rightmargin = rc.right - SizeMcModeFont.cx - RIGHTMARGIN;
        // This is a workaround for missing glyph real size. Currently TextSize vertically
        // includes also the extra spacing.
        fixBigInterline=SizeBigFont.cy/8;
        unitmediumoffset= SizeMediumFont.cy-SizeUnitFont.cy - (SizeMediumFont.cy-SizeUnitFont.cy)/5;
        unitbigoffset= SizeBigFont.cy-SizeUnitFont.cy - (SizeBigFont.cy-SizeUnitFont.cy)/5;

        topbearing.cy= topmargin + SizeMediumFont.cy/2;
        if (ScreenLandscape) {
            topbearing.cx= (rc.right + rc.left) / 2;
        } else {
            topbearing.cx= rc.right-COMPASS_SPACE-NIBLSCALE(2) -(SizeMediumFont.cx*2)-(SizeMediumFont.cx/2);
        }
        name_xmax=topbearing.cx-SizeMediumFont.cx*2-SizeMediumFont.cx/2-NIBLSCALE(3);


        yMcValue = yrightoffset - SizeBigFont.cy - SizeBigFont.cy + fixBigInterline;
        yMcSafety=  yMcValue + fixBigInterline -1 - (SizeSmallFont.cy+BOXEDYMARGIN);
        yMcMode  =  yMcValue + (SizeBigFont.cy -SizeMcModeFont.cy)/2;
        yDistUnit= topmargin + SizeMediumFont.cy + unitmediumoffset;
        yAltSafety= yrightoffset  - fixBigInterline + SizeBigFont.cy - fixBigInterline -1;
        yLeftWind =  rc.bottom - BottomSize - SizeMediumFont.cy - NIBLSCALE(2);


        #ifndef __linux__
        // TahomaBD need a bit adjustments for perfection
        yMcSafety+=NIBLSCALE(1);
        yAltSafety+= NIBLSCALE(1);
        #endif


        DoInit[MDI_DRAWLOOK8000] = false;

    } // end doinit

    distcolor=OverColorRef;

    // First we draw flight related values such as instant efficiency, altitude, new infoboxes etc.

    if (IsMultimapOverlaysGauges() && (LKVarioBar && !mode.AnyPan())) {
        leftmargin = rc.left+(LKVarioSize + NIBLSCALE(3)); // VARIOWIDTH + middle separator right extension
    } else {
        leftmargin = rc.left+LEFTMARGIN;
    }

    //
    // TARGET NAME
    //

    if (UseGates() && ActiveTaskPoint == 0) {
        // if running a task, use the task index normally
        if (ValidTaskPoint(ActiveTaskPoint) != false) {
            if (DoOptimizeRoute())
                index = RESWP_OPTIMIZED;
            else
                index = Task[ActiveTaskPoint].Index;
        } else
            index = -1;
    } else {
        index = GetOvertargetIndex();
    }

    if (IsMultimapOverlaysGauges() && MapWindow::ThermalBarDrawn) {
        rcx = leftmargin + NIBLSCALE(40);
    } else {
        rcx = leftmargin;
    }

    // Waypoint name and distance
    Surface.SelectObject(LK8OverlayMediumFont);

    if (index >= 0) {
        // OVERTARGET reswp not using redwarning because Reachable is not calculated
        if (index <= RESWP_END)
            redwarning = false;
        else {
            if (WayPointCalc[index].AltArriv[AltArrivMode] > 0 && !WayPointList[index].Reachable)
                redwarning = true;
            else
                redwarning = false;
        }

        int gateinuse = -2;
        if (UseGates() && ActiveTaskPoint == 0) {
            gateinuse = ActiveGate;
        }
        if (flipflop && (gateinuse != -2)) {
            if (!HaveGates()) {
                gateinuse = -1;
            } else {
                // this is set here for the first time, when havegates
                gatechrono = GateTime(ActiveGate) - LocalTime();
            }
            if (gateinuse < 0) {
                // LKTOKEN  _@M157_ = "CLOSED"
                _tcscpy(Buffer, MsgToken(157));
            } else {
                _stprintf(Buffer, _T("%s%d/%d"), StartGateName, gateinuse + 1, PGNumberOfGates);
            }

            LKWriteText(Surface, Buffer, rcx , topmargin, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        } else {
            if (Overlay_TopLeft) {
                GetOvertargetName(Buffer);
                RECT ClipRect = { rcx, topmargin, name_xmax, topmargin + SizeMediumFont.cy };
                LKWriteText(Surface, Buffer, rcx, topmargin, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true, &ClipRect);
            }
        }

        //
        // TARGET DISTANCE
        //

        if (gateinuse >= -1) {
            // if we are still painting , it means we did not start yet..so we use colors
            if (!CorrectSide()) distcolor = AMBERCOLOR;
            LKFormatValue(LK_START_DIST, false, BufferValue, BufferUnit, BufferTitle);
        } else {
            if (!Overlay_TopRight) goto _skip_TopRight;
            switch (OvertargetMode) {
                case OVT_TASK:
                    // Using FormatDist will give PGs 3 decimal units on overlay only
                    // because changing FormatValue to 3 digits would bring them also
                    // on bottom bar, and there is no space for 1.234km on the bottom bar.
                    if (ACTIVE_WP_IS_AAT_AREA || DoOptimizeRoute())
                        LKFormatDist(RESWP_OPTIMIZED, true, BufferValue, BufferUnit);
                    else
                        LKFormatDist(index, false, BufferValue, BufferUnit);
                    break;
                case OVT_TASKCENTER:
                    LKFormatDist(index, false, BufferValue, BufferUnit);
                    break;
                case OVT_BALT:
                    LKFormatDist(BestAlternate, false, BufferValue, BufferUnit);
                    break;
                case OVT_ALT1:
                    LKFormatDist(Alternate1, false, BufferValue, BufferUnit);
                    break;
                case OVT_ALT2:
                    LKFormatDist(Alternate2, false, BufferValue, BufferUnit);
                    break;
                case OVT_HOME:
                    LKFormatDist(HomeWaypoint, false, BufferValue, BufferUnit);
                    break;
                case OVT_THER:
                    LKFormatDist(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
                    break;
                case OVT_MATE:
                    LKFormatDist(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
                    break;
                case OVT_XC:
                    LKFormatDist(RESWP_FAIOPTIMIZED, false, BufferValue, BufferUnit);
                break;
                case OVT_FLARM:
                    LKFormatDist(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
                    break;
                default:
                    LKFormatValue(LK_NEXT_DIST, false, BufferValue, BufferUnit, BufferTitle);
                    break;
            }
        }

        if ( !OverlayClock && ScreenLandscape && (!((gTaskType==TSK_GP) && UseGates()))) {
            _stprintf(BufferValue + _tcslen(BufferValue), _T(" %s"), BufferUnit);
            LKWriteText(Surface, BufferValue, compass.cx, topmargin, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
        } else {
            LKWriteText(Surface, BufferValue, rcx , topmargin + SizeMediumFont.cy, WTMODE_OUTLINED, WTALIGN_LEFT, distcolor, true);

            if (!HideUnits) {
                Surface.GetTextSize(BufferValue, &TextSize);
                Surface.SelectObject(MapScaleFont);
                LKWriteText(Surface, BufferUnit, rcx + TextSize.cx, yDistUnit, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
            }

        }

        _skip_TopRight:

        //
        // BEARING DIFFERENCE   displayed only when not circling
        //
        if (!Overlay_TopMid) goto _skip_TopMid;

        if (!ISGAAIRCRAFT) {
            switch (OvertargetMode) {
                case OVT_TASK:
                    // Do not use FormatBrgDiff for TASK, could be AAT! (??)
                    LKFormatBrgDiff(index, false, BufferValue, BufferUnit);
                    break;
                case OVT_TASKCENTER:
                    LKFormatBrgDiff(index, false, BufferValue, BufferUnit);
                    break;
                case OVT_BALT:
                    LKFormatBrgDiff(BestAlternate, false, BufferValue, BufferUnit);
                    break;
                case OVT_ALT1:
                    LKFormatBrgDiff(Alternate1, false, BufferValue, BufferUnit);
                    break;
                case OVT_ALT2:
                    LKFormatBrgDiff(Alternate2, false, BufferValue, BufferUnit);
                    break;
                case OVT_HOME:
                    LKFormatBrgDiff(HomeWaypoint, false, BufferValue, BufferUnit);
                    break;
                case OVT_THER:
                    LKFormatBrgDiff(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
                    break;
                case OVT_MATE:
                    LKFormatBrgDiff(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
                    break;
                case OVT_XC:
                    LKFormatBrgDiff(RESWP_FAIOPTIMIZED, false, BufferValue, BufferUnit);
                    break;
                case OVT_FLARM:
                    LKFormatBrgDiff(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
                    break;
                default:
                    LKFormatValue(LK_BRGDIFF, false, BufferValue, BufferUnit, BufferTitle);
                    break;
            }

            Surface.SelectObject(LK8OverlayMediumFont); // restore previously selected font
            LKWriteText(Surface, BufferValue, topbearing.cx,  topbearing.cy, WTMODE_OUTLINED, WTALIGN_CENTER, OverColorRef, true);
        }
        _skip_TopMid:


        //
        // RIGHT MID  AUX-2
        // EFFICIENCY REQUIRED  and altitude arrival for destination waypoint
        // For paragliders, average efficiency and arrival destination
        //

        rcx = rightmargin;

        if (ISGLIDER) {

            if (isOverlayHidden(Overlay_RightMid)) goto _skip_glider_RightMid;
            if (isOverlayCustom(Overlay_RightMid))
                LKFormatValue(getCustomOverlay(Overlay_RightMid), true, BufferValue, BufferUnit, BufferTitle);
            else
            switch (OvertargetMode) {
                case OVT_TASK:
                    LKFormatValue(LK_NEXT_GR, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_TASKCENTER:
                    LKFormatValue(LK_NEXT_CENTER_GR, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_BALT:
                    LKFormatValue(LK_BESTALTERN_GR, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_ALT1:
                    LKFormatValue(LK_ALTERN1_GR, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_ALT2:
                    LKFormatValue(LK_ALTERN2_GR, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_HOME:
                    LKFormatGR(HomeWaypoint, false, BufferValue, BufferUnit);
                    break;
                case OVT_THER:
                    LKFormatGR(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
                    break;
                case OVT_MATE:
                    LKFormatGR(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
                    break;
                case OVT_XC:
                    LKFormatGR(RESWP_FAIOPTIMIZED, false, BufferValue, BufferUnit);
                    break;
                case OVT_FLARM:
                    LKFormatGR(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
                    break;
                default:
                    LKFormatValue(LK_NEXT_GR, false, BufferValue, BufferUnit, BufferTitle);
                    break;
            }

            Surface.SelectObject(LK8OverlayBigFont); // use this font for big values
            rcy = yrightoffset - SizeBigFont.cy;
            color=(redwarning&&!isOverlayCustom(Overlay_RightMid))?AMBERCOLOR:OverColorRef;
            LKWriteText(Surface, BufferValue, rcx, rcy, WTMODE_OUTLINED, WTALIGN_RIGHT, color, true);

            _skip_glider_RightMid:

            //
            // RIGHT BOTTOM  AUX-3
            // (GLIDERS) ALTITUDE DIFFERENCE  at current MC
            //
            if (isOverlayHidden(Overlay_RightBottom)) goto _skip_glider_RightBottom;
            if (isOverlayCustom(Overlay_RightBottom))
                LKFormatValue(getCustomOverlay(Overlay_RightBottom), true, BufferValue, BufferUnit, BufferTitle);
            else
            switch (OvertargetMode) {
                case OVT_TASK:
                    LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_TASKCENTER:
                    LKFormatValue(LK_NEXT_CENTER_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_BALT:
                    LKFormatValue(LK_BESTALTERN_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_ALT1:
                    LKFormatValue(LK_ALTERN1_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_ALT2:
                    LKFormatValue(LK_ALTERN2_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_HOME:
                    LKFormatAltDiff(HomeWaypoint, false, BufferValue, BufferUnit);
                    break;
                case OVT_THER:
                    LKFormatAltDiff(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
                    break;
                case OVT_MATE:
                    LKFormatAltDiff(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
                    break;
                case OVT_XC:
                    LKFormatAltDiff(RESWP_FAIOPTIMIZED, false, BufferValue, BufferUnit);
                    break;
                case OVT_FLARM:
                    LKFormatAltDiff(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
                    break;
                default:
                    LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
                    break;
            }
            color=(redwarning&&!isOverlayCustom(Overlay_RightBottom))?AMBERCOLOR:OverColorRef;
            LKWriteText(Surface, BufferValue, rcx, yrightoffset - fixBigInterline, WTMODE_OUTLINED, WTALIGN_RIGHT, color, true);

            //
            // (GLIDERS) SAFETY ALTITUDE INDICATOR
            // For PGs there is a separate drawing, although it should be identical right now.
            //

            if (IsSafetyAltitudeInUse(index)&&!isOverlayCustom(Overlay_RightBottom)) {
                Surface.SelectObject(LK8OverlaySmallFont);
                _stprintf(BufferValue, _T(" + %.0f %s "), SAFETYALTITUDEARRIVAL / 10 * ALTITUDEMODIFY,
                        Units::GetUnitName(Units::GetUserAltitudeUnit()));
                LKWriteBoxedText(Surface, rc, BufferValue, rcx, yAltSafety, WTALIGN_RIGHT, RGB_WHITE, RGB_WHITE);
            }
            _skip_glider_RightBottom: ;
        } // ISGLIDER
    // end of index>0
    } else {
        // no valid index for current overmode, but we print something nevertheless
        // normally, only the T>
        // This should never happen, as we always have a valid overmode

        if (Overlay_TopLeft) {
            Surface.SelectObject(LK8OverlayMediumFont);
            GetOvertargetName(Buffer);
            CharUpper(Buffer);
            RECT ClipRect = { rcx, topmargin, name_xmax, topmargin + SizeMediumFont.cy };
            LKWriteText(Surface, Buffer, rcx, topmargin, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true, &ClipRect);
        }
    }

    // moved out from task paragliders stuff - this is painted on the right
        if (UseGates() && ActiveTaskPoint == 0) {
            Surface.SelectObject(LK8OverlayBigFont);

            if (HaveGates()) {
                // Time To Gate
                gatechrono = GateTime(ActiveGate) - LocalTime(); // not always already set, update it ...

                Units::TimeToTextDown(BufferValue, gatechrono);
                rcx= rc.right-RIGHTMARGIN;
                rcy = yrightoffset - SizeBigFont.cy-NIBLSCALE(6); // 101112
                LKWriteText(Surface, BufferValue, rcx,rcy, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

                // Gate ETE Diff
                Value = WayPointCalc[DoOptimizeRoute() ? RESWP_OPTIMIZED : Task[0].Index].NextETE - gatechrono;
                Units::TimeToTextDown(BufferValue, (int) Value);
                rcy += SizeBigFont.cy-NIBLSCALE(2);
                color=(Value<=0)?AMBERCOLOR:OverColorRef;
                LKWriteText(Surface, BufferValue, rcx,rcy, WTMODE_OUTLINED,WTALIGN_RIGHT,color, true);

                // Req. Speed For reach Gate
                if (LKFormatValue(LK_START_SPEED, false, BufferValue, BufferUnit, BufferTitle)) {
                    Surface.SelectObject(LK8TargetFont);
                    Surface.GetTextSize(BufferUnit, &TextSize);
                    rcx -= TextSize.cx;
                    Surface.GetTextSize(BufferValue, &TextSize);
                    rcx -= (TextSize.cx + NIBLSCALE(2));
                    rcy += TextSize.cy-NIBLSCALE(2);

                    LKWriteText(Surface, BufferValue, rcx, rcy + (TextSize.cy / 3), WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);

                    LKWriteText(Surface, BufferUnit, rcx + TextSize.cx + NIBLSCALE(2), rcy + (TextSize.cy / 3), WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
                    Surface.SelectObject(LK8OverlayBigFont);
                }
            }

        } else {
            Surface.SelectObject(LK8OverlayBigFont);
            if (isOverlayHidden(Overlay_RightMid)) goto _skip_para_RightMid;
            if (isOverlayCustom(Overlay_RightMid))
                LKFormatValue(getCustomOverlay(Overlay_RightMid), true, BufferValue, BufferUnit, BufferTitle);
            else {
                if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
                    LKFormatValue(LK_TC_30S, false, BufferValue, BufferUnit, BufferTitle);
                else
                    LKFormatValue(LK_LD_AVR, false, BufferValue, BufferUnit, BufferTitle);
            }

            rcy = yrightoffset - SizeBigFont.cy;
            rcx = rightmargin;
            LKWriteText(Surface, BufferValue, rcx, rcy, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
            _skip_para_RightMid:

            // Altitude difference with current MC
            if (isOverlayHidden(Overlay_RightBottom)) goto _skip_para_RightBottom;
            if (isOverlayCustom(Overlay_RightBottom))
                LKFormatValue(getCustomOverlay(Overlay_RightBottom), true, BufferValue, BufferUnit, BufferTitle);
            else
            switch (OvertargetMode) {
                case OVT_TASK:
                    LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_TASKCENTER:
                    LKFormatValue(LK_NEXT_CENTER_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_BALT:
                    LKFormatValue(LK_BESTALTERN_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_ALT1:
                    LKFormatValue(LK_ALTERN1_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_ALT2:
                    LKFormatValue(LK_ALTERN2_ARRIV, false, BufferValue, BufferUnit, BufferTitle);
                    break;
                case OVT_HOME:
                    LKFormatAltDiff(HomeWaypoint, false, BufferValue, BufferUnit);
                    break;
                case OVT_THER:
                    LKFormatAltDiff(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
                    break;
                case OVT_MATE:
                    LKFormatAltDiff(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
                    break;
                case OVT_XC:
                    LKFormatAltDiff(RESWP_FAIOPTIMIZED, false, BufferValue, BufferUnit);
                    break;
                case OVT_FLARM:
                    LKFormatAltDiff(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
                    break;
                default:
                    LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
                    break;
            }
            color=(redwarning&&!isOverlayCustom(Overlay_RightBottom))?AMBERCOLOR:OverColorRef;
            LKWriteText(Surface, BufferValue, rcx, yrightoffset - fixBigInterline, WTMODE_OUTLINED, WTALIGN_RIGHT,color, true);

            //
            // SAFETY ALTITUDE INDICATOR (FOR PARAGLIDERS)
            // Should be identical to that for other aircrafts, normally.
            //
            if (IsSafetyAltitudeInUse(GetOvertargetIndex())&&!isOverlayCustom(Overlay_RightBottom)) {
                Surface.SelectObject(LK8OverlaySmallFont);
                _stprintf(BufferValue, _T(" + %.0f %s "), SAFETYALTITUDEARRIVAL / 10 * ALTITUDEMODIFY,
                        Units::GetUnitName(Units::GetUserAltitudeUnit()));
                LKWriteBoxedText(Surface, rc, BufferValue, rcx, yAltSafety, WTALIGN_RIGHT, RGB_WHITE, RGB_WHITE);

            }
            _skip_para_RightBottom: ;
        } // end no UseGates()


    //
    // TIME GATES:  in place of MC, print gate time
    //
    if (UseGates() && ActiveTaskPoint == 0) {
        Surface.SelectObject(LK8OverlayGatesFont);

        if (HaveGates()) {
            Units::TimeToText(BufferTitle, GateTime(ActiveGate));
            _stprintf(BufferValue, _T("START %s"), BufferTitle);
        } else {
            // LKTOKEN  _@M316_ = "GATES CLOSED"
            _tcscpy(BufferValue, MsgToken(316));
        }
        rcy = yrightoffset - SizeBigFont.cy - (SizeGatesFont.cy * 2);
        rcx = rc.right - RIGHTMARGIN;
        LKWriteText(Surface, BufferValue, rcx, rcy, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

        // USE THIS SPACE FOR MESSAGES TO THE PILOT
        rcy += SizeGatesFont.cy;
        if (HaveGates()) {
            if (!DerivedDrawInfo.Flying) {
                // LKTOKEN  _@M922_ = "NOT FLYING"
                _tcscpy(BufferValue, MsgToken(922));
            } else {
                if (gatechrono > 0) {
                    // IsInSector works reversed!
                    if (PGStartOut && DerivedDrawInfo.IsInSector) {
                        // LKTOKEN  _@M923_ = "WRONG inSIDE"
                        _tcscpy(BufferValue, MsgToken(923));
                    } else if (!PGStartOut && !DerivedDrawInfo.IsInSector) {
                        // LKTOKEN  _@M924_ = "WRONG outSIDE"
                        _tcscpy(BufferValue, MsgToken(924));
                    } else {
                        // LKTOKEN  _@M921_ = "countdown"
                        _tcscpy(BufferValue, MsgToken(921));
                    }
                } else {
                    BufferValue[0] = _T('\0');
                    // gate is open
                    int CloseTime = GateCloseTime();
                    if (flipflopcount > 0) {
                        if ((ActiveGate < (PGNumberOfGates - 1) || CloseTime < 86340) && flipflopcount == 1) {
                            if (CloseTime < 86340) {
                                Units::TimeToText(BufferTitle, CloseTime);
                                _stprintf(BufferValue, _T("CLOSE %s"), BufferTitle);
                            } else {
                                Units::TimeToText(BufferTitle, GateTime(ActiveGate + 1));
                                _stprintf(BufferValue, _T("NEXT %s"), BufferTitle);
                            }
                        } else {
                            // IsInSector works reversed!
                            if (PGStartOut && DerivedDrawInfo.IsInSector) {
                                // LKTOKEN  _@M923_ = "WRONG inSIDE"
                                _tcscpy(BufferValue, MsgToken(923));
                            } else if (!PGStartOut && !DerivedDrawInfo.IsInSector) {
                                // LKTOKEN  _@M924_ = "WRONG outSIDE"
                                _tcscpy(BufferValue, MsgToken(924));
                            }
                        }
                    }
                    if (BufferValue[0] == _T('\0')) {
                        // LKTOKEN  _@M314_ = "GATE OPEN"
                        _tcscpy(BufferValue, MsgToken(314));
                    }
                }
            }
        } else {
            // LKTOKEN  _@M925_ = "NO TSK START"
            _tcscpy(BufferValue, MsgToken(925));
        }
        LKWriteText(Surface, BufferValue, rcx, rcy, WTMODE_OUTLINED, WTALIGN_RIGHT, distcolor, true);

    } else
    if ( (ISGLIDER || ISPARAGLIDER) && !isOverlayHidden(Overlay_RightTop)) {
        //
        // MAC CREADY VALUE
        //

        Surface.SelectObject(LK8OverlayBigFont);
        if (isOverlayCustom(Overlay_RightTop))
            LKFormatValue(getCustomOverlay(Overlay_RightTop), true, BufferValue, BufferUnit, BufferTitle);
        else
            LKFormatValue(LK_MC, false, BufferValue, BufferUnit, BufferTitle);
        LKWriteText(Surface, BufferValue, rightmargin, yMcValue, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

        //
        // SAFETY MAC CREADY INDICATOR
        //
        if (!isOverlayCustom(Overlay_RightTop) && IsSafetyMacCreadyInUse(GetOvertargetIndex()) && GlidePolar::SafetyMacCready > 0) {
            Surface.SelectObject(LK8OverlaySmallFont);
            _stprintf(BufferValue, _T(" %.1f %s "), GlidePolar::SafetyMacCready*LIFTMODIFY,
                Units::GetUnitName(Units::GetUserVerticalSpeedUnit()));
            LKWriteBoxedText(Surface, rc, BufferValue, rightmargin, yMcSafety, WTALIGN_RIGHT, RGB_WHITE, RGB_WHITE);
        }

        //
        // AUTO MC INDICATOR
        //
        if (!isOverlayHidden(Overlay_RightTop) && DerivedDrawInfo.AutoMacCready == true) {
            Surface.SelectObject(LK8OverlayMcModeFont);

            TCHAR amcmode[10];
            switch (AutoMcMode) {
                case amcFinalGlide:
                    _tcscpy(amcmode, MsgToken(1676));
                    break;
                case amcAverageClimb:
                    _tcscpy(amcmode, MsgToken(1678));
                    break;
                case amcFinalAndClimb:
                    if (DerivedDrawInfo.FinalGlide)
                        _tcscpy(amcmode, MsgToken(1676));
                    else
                        _tcscpy(amcmode, MsgToken(1678));
                    break;
                case amcEquivalent:
                    _tcscpy(amcmode, MsgToken(1680));
                    break;
                default:
                    _tcscpy(amcmode, _T("?"));
                    break;
            }
            Surface.Rectangle( rightmargin, yMcMode, rc.right, yMcMode + SizeMcModeFont.cy);
            LKWriteText(Surface, amcmode, rightmargin + NIBLSCALE(1), yMcMode,
                    // We paint white on black always here, but WriteText can invert them. So we reverse.
                    WTMODE_NORMAL, WTALIGN_LEFT, INVERTCOLORS?RGB_WHITE:RGB_BLACK, true);

        } // AutoMacCready true AUTO MC INDICATOR
    } // overlay RighTop

    short yoffset=0;
    if (OverlayClock && Overlay_TopRight && ScreenLandscape) yoffset = SizeMediumFont.cy-fixBigInterline;

    if (IsMultimapOverlaysGauges() && !mode.AnyPan())
        rcx = leftmargin + GlideBarOffset;
    else
        rcx = leftmargin;

    //
    // LEFT TOP
    //
    if (!isOverlayHidden(Overlay_LeftTop) && (ISGLIDER||ISPARAGLIDER) ) {
        Surface.SelectObject(LK8OverlayBigFont);

        if (isOverlayCustom(Overlay_LeftTop))
            LKFormatValue(getCustomOverlay(Overlay_LeftTop), true, BufferValue, BufferUnit, BufferTitle);
        else {
            if (ISPARAGLIDER) {
                LKFormatValue(LK_HNAV, false, BufferValue, BufferUnit, BufferTitle); // 091115
            } else {
                if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
                    LKFormatValue(LK_TC_30S, false, BufferValue, BufferUnit, BufferTitle);
                else
                    LKFormatValue(LK_LD_AVR, false, BufferValue, BufferUnit, BufferTitle);
            }
        }

        //
        // CENTER THE LEFT OVERLAYS
        //
        rcy = yMcValue;
        LKWriteText(Surface, BufferValue, rcx, rcy+yoffset, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        if (!HideUnits) {
            Surface.GetTextSize(BufferValue, &TextSize);
            Surface.SelectObject(MapScaleFont);
            LKWriteText(Surface, BufferUnit, rcx + TextSize.cx, rcy+yoffset+unitbigoffset, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
   } // LeftTop

   //
   // LEFT MID
   //
   if (!isOverlayHidden(Overlay_LeftMid)) {
        if (isOverlayCustom(Overlay_LeftMid)) {
            LKFormatValue(getCustomOverlay(Overlay_LeftMid), true, BufferValue, BufferUnit, BufferTitle);
        } else {
            if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) || LKVarioVal == vValVarioVario) {
                LKFormatValue(LK_VARIO, false, BufferValue, BufferUnit, BufferTitle);
            } else {
                switch (LKVarioVal) {
                    case vValVarioNetto:
                        LKFormatValue(LK_NETTO, false, BufferValue, BufferUnit, BufferTitle);
                        _tcscpy(BufferUnit, _T("NT"));
                        break;
                    case vValVarioSoll:
                    default:
                        LKFormatValue(LK_SPEED_DOLPHIN, false, BufferValue, BufferUnit, BufferTitle);
                        _tcscpy(BufferUnit, _T("SF"));
                        break;
                }
            }
        }

        Surface.SelectObject(LK8OverlayBigFont);
        rcy = yrightoffset - SizeBigFont.cy +yoffset;

        LKWriteText(Surface, BufferValue, rcx, rcy, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        if (!HideUnits) {
            Surface.GetTextSize(BufferValue, &TextSize);
            Surface.SelectObject(MapScaleFont);
            LKWriteText(Surface, BufferUnit, rcx + TextSize.cx , rcy + unitbigoffset, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
    } // LeftMid

    //
    // LEFT BOTTOM
    //
    if (!isOverlayHidden(Overlay_LeftBottom)) {
        if (isOverlayCustom(Overlay_LeftBottom))
            LKFormatValue(getCustomOverlay(Overlay_LeftBottom), true, BufferValue, BufferUnit, BufferTitle);
        else {
            if (ISPARAGLIDER) {
                LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
            } else {
                if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
                    LKFormatValue(LK_HNAV, false, BufferValue, BufferUnit, BufferTitle);
                } else {
                    LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
                }
            }
        }
        Surface.SelectObject(LK8OverlayBigFont);
        rcy=yrightoffset - fixBigInterline+yoffset;
        LKWriteText(Surface, BufferValue, rcx, rcy, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        if (!HideUnits) {
            Surface.GetTextSize(BufferValue, &TextSize);
            Surface.SelectObject(MapScaleFont);
            LKWriteText(Surface, BufferUnit, rcx + TextSize.cx, rcy + unitbigoffset,
                WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
    }

    //
    // CLOCK
    //
    if ((OverlayClock && Overlay_TopRight) || ((gTaskType==TSK_GP) && UseGates())) {
        LKFormatValue(LK_TIME_LOCALSEC, false, BufferValue, BufferUnit, BufferTitle);
        Surface.SelectObject(LK8OverlayMediumFont);
        if (!ScreenLandscape) {
            LKWriteText(Surface, BufferValue, rightmargin, compass.cy,
                WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
        } else {
            LKWriteText(Surface, BufferValue, compass.cx, topmargin,
                WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
        }
    }

    //
    // LEFT DOWN (wind)
    //

    if (!isOverlayHidden(Overlay_LeftDown) && !(MapSpaceMode != MSM_MAP && Current_Multimap_SizeY != SIZE4)) {
        Surface.SelectObject(LK8OverlayMediumFont);
        if (isOverlayCustom(Overlay_LeftDown)) {
            LKFormatValue(getCustomOverlay(Overlay_LeftDown), true, BufferValue, BufferUnit, BufferTitle);
        } else {
            if (ISCAR || ISGAAIRCRAFT)
                LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
            else
                LKFormatValue(LK_WIND, false, BufferValue, BufferUnit, BufferTitle);
        }
        LKWriteText(Surface, BufferValue, leftmargin, yLeftWind,
            WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);

        if (!HideUnits) {
            Surface.GetTextSize(BufferValue, &TextSize);
            Surface.SelectObject(MapScaleFont);
            LKWriteText(Surface, BufferUnit, leftmargin + TextSize.cx, yLeftWind + unitmediumoffset,
                WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
    } // LeftDown


    // restore objects and return
    Surface.SelectObject(oldpen);
    Surface.SelectObject(oldbrush);
    Surface.SelectObject(oldfont);


}
