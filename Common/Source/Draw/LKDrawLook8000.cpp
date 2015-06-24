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

#define OAUX 2

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


    if (++flipflopcount > 2) {
        flipflop = !flipflop;
        flipflopcount = 0;
    }


    if (DoInit[MDI_DRAWLOOK8000]) {

        compass={rc.right-COMPASS_SPACE, rc.top+COMPASS_SPACE};

        TCHAR Tdummy[] = _T("E");
        Surface.SelectObject(LK8OverlayBigFont);
        Surface.GetTextSize(Tdummy, _tcslen(Tdummy), &SizeBigFont);

        Surface.SelectObject(LK8OverlayMediumFont);
        Surface.GetTextSize(Tdummy, _tcslen(Tdummy), &SizeMediumFont);

        Surface.SelectObject(LK8OverlaySmallFont);
        Surface.GetTextSize(Tdummy, _tcslen(Tdummy), &SizeSmallFont);

        Surface.SelectObject(LK8OverlayMcModeFont);
        Surface.GetTextSize(Tdummy, _tcslen(Tdummy), &SizeMcModeFont);

        Surface.SelectObject(LK8OverlayGatesFont);
        Surface.GetTextSize(Tdummy, _tcslen(Tdummy), &SizeGatesFont);

        Surface.SelectObject(MapScaleFont);
        Surface.GetTextSize(Tdummy, _tcslen(Tdummy), &SizeUnitFont);

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


        #ifndef linux
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

    if (ISPARAGLIDER && UseGates() && ActiveWayPoint == 0) {
        // if running a task, use the task index normally
        if (ValidTaskPoint(ActiveWayPoint) != false) {
            if (DoOptimizeRoute())
                index = RESWP_OPTIMIZED;
            else
                index = Task[ActiveWayPoint].Index;
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
        if (UseGates() && ActiveWayPoint == 0) {
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

            LKWriteText(Surface, Buffer, rcx , topmargin, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        } else {
            if (Overlay_TopLeft) {
                GetOvertargetName(Buffer);
                CharUpper(Buffer);
                RECT ClipRect = { rcx, topmargin, name_xmax, topmargin + SizeMediumFont.cy };
                LKWriteText(Surface, Buffer, rcx, topmargin, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true, &ClipRect);
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
                case OVT_FLARM:
                    LKFormatDist(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
                    break;
                default:
                    LKFormatValue(LK_NEXT_DIST, false, BufferValue, BufferUnit, BufferTitle);
                    break;
            }
        }

        if ( !OverlayClock && ScreenLandscape && (!(ISPARAGLIDER && UseGates()))) {
            _stprintf(BufferValue + _tcslen(BufferValue), _T(" %s"), BufferUnit);
            LKWriteText(Surface, BufferValue, compass.cx, topmargin, 
                0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
        } else {
            LKWriteText(Surface, BufferValue, rcx , topmargin + SizeMediumFont.cy, 
                0, WTMODE_OUTLINED, WTALIGN_LEFT, distcolor, true);
            
            if (!HideUnits) {
                Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
                Surface.SelectObject(MapScaleFont); 
                LKWriteText(Surface, BufferUnit, rcx + TextSize.cx, yDistUnit, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
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
                case OVT_FLARM:
                    LKFormatBrgDiff(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
                    break;
                default:
                    LKFormatValue(LK_BRGDIFF, false, BufferValue, BufferUnit, BufferTitle);
                    break;
            }

            Surface.SelectObject(LK8OverlayMediumFont); // restore previously selected font
            LKWriteText(Surface, BufferValue, topbearing.cx,  topbearing.cy, 0,
                WTMODE_OUTLINED, WTALIGN_CENTER, OverColorRef, true);
        }
        _skip_TopMid:


        // 
        // RIGHT MID  AUX-2
        // EFFICIENCY REQUIRED  and altitude arrival for destination waypoint
        // For paragliders, average efficiency and arrival destination
        //

        rcx = rightmargin;

        if (ISGLIDER) {

            if (!Overlay_RightMid) goto _skip_glider_RightMid;
            if (Overlay_RightMid==OAUX)
                LKFormatValue(GetInfoboxType(2), true, BufferValue, BufferUnit, BufferTitle);
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
                case OVT_FLARM:
                    LKFormatGR(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
                    break;
                default:
                    LKFormatValue(LK_NEXT_GR, false, BufferValue, BufferUnit, BufferTitle);
                    break;
            }

            Surface.SelectObject(LK8OverlayBigFont); // use this font for big values
            rcy = yrightoffset - SizeBigFont.cy;
            color=(redwarning&&Overlay_RightMid<OAUX)?AMBERCOLOR:OverColorRef;
            LKWriteText(Surface, BufferValue, rcx, rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, color, true);
            
            _skip_glider_RightMid:

            //
            // RIGHT BOTTOM  AUX-3
            // (GLIDERS) ALTITUDE DIFFERENCE  at current MC
            //
            if (!Overlay_RightBottom) goto _skip_glider_RightBottom;
            if (Overlay_RightBottom==OAUX)
                LKFormatValue(GetInfoboxType(3), true, BufferValue, BufferUnit, BufferTitle);
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
                case OVT_FLARM:
                    LKFormatAltDiff(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
                    break;
                default:
                    LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
                    break;
            }
            color=(redwarning&&Overlay_RightBottom<OAUX)?AMBERCOLOR:OverColorRef;
            LKWriteText(Surface, BufferValue, rcx, yrightoffset - fixBigInterline, 0,
                WTMODE_OUTLINED, WTALIGN_RIGHT, color, true); 

            //
            // (GLIDERS) SAFETY ALTITUDE INDICATOR 
            // For PGs there is a separate drawing, although it should be identical right now.
            //

            if (IsSafetyAltitudeInUse(index)&&Overlay_RightBottom<OAUX) {
                Surface.SelectObject(LK8OverlaySmallFont);
                _stprintf(BufferValue, _T(" + %.0f %s "), SAFETYALTITUDEARRIVAL / 10 * ALTITUDEMODIFY,
                        Units::GetUnitName(Units::GetUserAltitudeUnit()));
                LKWriteBoxedText(Surface, rc, BufferValue, rcx, yAltSafety,
                    0, WTALIGN_RIGHT, RGB_WHITE, RGB_WHITE);
            }
            _skip_glider_RightBottom: ;
        } // ISGLIDER
    // end of index>0
    } else { 
        // no valid index for current overmode, but we print something nevertheless
        // normally, only the T>
        // This should never happen, as we always have a valid overmode
        
        if (Overlay_TopLeft) {
            GetOvertargetName(Buffer);
            CharUpper(Buffer);
            Surface.SelectObject(LK8OverlayBigFont); // use this font for big values
            RECT ClipRect = { rcx, topmargin, name_xmax, topmargin + SizeBigFont.cy };
            LKWriteText(Surface, Buffer, rcx, topmargin, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true, &ClipRect);
        }
    }

    // moved out from task paragliders stuff - this is painted on the right
    if (ISPARAGLIDER) {

        if (UseGates() && ActiveWayPoint == 0) {
            Surface.SelectObject(LK8OverlayBigFont);

            if (HaveGates()) {
                // Time To Gate
                gatechrono = GateTime(ActiveGate) - LocalTime(); // not always already set, update it ... 

                Units::TimeToTextDown(BufferValue, gatechrono);
                rcx= rc.right-RIGHTMARGIN;
                rcy = yrightoffset - SizeBigFont.cy-NIBLSCALE(6); // 101112
                LKWriteText(Surface, BufferValue, rcx,rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

                // Gate ETE Diff 
                Value = WayPointCalc[DoOptimizeRoute() ? RESWP_OPTIMIZED : Task[0].Index].NextETE - gatechrono;
                Units::TimeToTextDown(BufferValue, (int) Value);
                rcy += SizeBigFont.cy-NIBLSCALE(2);
                color=(Value<=0)?AMBERCOLOR:OverColorRef;
                LKWriteText(Surface, BufferValue, rcx,rcy, 0, WTMODE_OUTLINED,WTALIGN_RIGHT,color, true);

                // Req. Speed For reach Gate
                if (LKFormatValue(LK_START_SPEED, false, BufferValue, BufferUnit, BufferTitle)) {
                    Surface.SelectObject(LK8OverlayBigFont);
                    Surface.GetTextSize(BufferUnit, _tcslen(BufferUnit), &TextSize);
                    rcx -= TextSize.cx;
                    Surface.SelectObject(LK8TargetFont);
                    Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
                    rcx -= (TextSize.cx + NIBLSCALE(2));
                    rcy += TextSize.cy-NIBLSCALE(2);

                    LKWriteText(Surface, BufferValue, rcx, rcy, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);

                    Surface.SelectObject(LK8OverlayBigFont);
                    LKWriteText(Surface, BufferUnit, rcx + TextSize.cx + NIBLSCALE(2), rcy + (TextSize.cy / 3), 0,
                            WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
                }
            }

        } else {
            Surface.SelectObject(LK8OverlayBigFont);
            if (!Overlay_RightMid) goto _skip_para_RightMid;
            if (Overlay_RightMid==OAUX)
                LKFormatValue(GetInfoboxType(2), true, BufferValue, BufferUnit, BufferTitle);
            else {
                if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
                    LKFormatValue(LK_TC_30S, false, BufferValue, BufferUnit, BufferTitle);
                else
                    LKFormatValue(LK_LD_AVR, false, BufferValue, BufferUnit, BufferTitle);
            }

            rcy = yrightoffset - SizeBigFont.cy;
            rcx = rightmargin;
            LKWriteText(Surface, BufferValue, rcx, rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
            _skip_para_RightMid:

            // Altitude difference with current MC
            if (!Overlay_RightBottom) goto _skip_para_RightBottom;
            if (Overlay_RightBottom==OAUX)
                LKFormatValue(GetInfoboxType(2), true, BufferValue, BufferUnit, BufferTitle);
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
                case OVT_FLARM:
                    LKFormatAltDiff(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
                    break;
                default:
                    LKFormatValue(LK_NEXT_ALTDIFF, false, BufferValue, BufferUnit, BufferTitle);
                    break;
            }
            color=(redwarning&&Overlay_RightBottom<OAUX)?AMBERCOLOR:OverColorRef;
            LKWriteText(Surface, BufferValue, rcx, yrightoffset - fixBigInterline, 0,
                WTMODE_OUTLINED, WTALIGN_RIGHT,color, true);  

            //
            // SAFETY ALTITUDE INDICATOR (FOR PARAGLIDERS)
            // Should be identical to that for other aircrafts, normally.
            //
            if (IsSafetyAltitudeInUse(GetOvertargetIndex())&&Overlay_RightBottom<OAUX) {
                Surface.SelectObject(LK8OverlaySmallFont);
                _stprintf(BufferValue, _T(" + %.0f %s "), SAFETYALTITUDEARRIVAL / 10 * ALTITUDEMODIFY,
                        Units::GetUnitName(Units::GetUserAltitudeUnit()));
                LKWriteBoxedText(Surface, rc, BufferValue, rcx, yAltSafety,
                    0, WTALIGN_RIGHT, RGB_WHITE, RGB_WHITE);

            }
            _skip_para_RightBottom: ;
        } // end no UseGates()
    } // is paraglider


    //
    // TIME GATES:  in place of MC, print gate time
    //
    if (UseGates() && ActiveWayPoint == 0) {
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
        LKWriteText(Surface, BufferValue, rcx, rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

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
        LKWriteText(Surface, BufferValue, rcx, rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, distcolor, true);

    } else 
    if ( (ISGLIDER || ISPARAGLIDER) && Overlay_RightTop) {
        //
        // MAC CREADY VALUE
        //

        Surface.SelectObject(LK8OverlayBigFont);
        if (Overlay_RightTop==OAUX)
            LKFormatValue(GetInfoboxType(1), true, BufferValue, BufferUnit, BufferTitle);
        else
            LKFormatValue(LK_MC, false, BufferValue, BufferUnit, BufferTitle);
        LKWriteText(Surface, BufferValue, rightmargin, yMcValue, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

        //
        // SAFETY MAC CREADY INDICATOR
        //
        extern bool IsSafetyMacCreadyInUse(int val);
        if (Overlay_RightTop<OAUX && IsSafetyMacCreadyInUse(GetOvertargetIndex()) && GlidePolar::SafetyMacCready > 0) {
            Surface.SelectObject(LK8OverlaySmallFont);
            _stprintf(BufferValue, _T(" %.1f %s "), GlidePolar::SafetyMacCready*LIFTMODIFY,
                Units::GetUnitName(Units::GetUserVerticalSpeedUnit()));
            LKWriteBoxedText(Surface, rc, BufferValue, rightmargin, yMcSafety, 0, WTALIGN_RIGHT, RGB_WHITE, RGB_WHITE);
        }

        //
        // AUTO MC INDICATOR
        //
        if (Overlay_RightTop<OAUX && DerivedDrawInfo.AutoMacCready == true) {
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
                    0, WTMODE_NORMAL, WTALIGN_LEFT, INVERTCOLORS?RGB_WHITE:RGB_BLACK, true);

        } // AutoMacCready true AUTO MC INDICATOR
    } // overlay RighTop

    short yoffset=0;
    if (OverlayClock && ScreenLandscape) yoffset = SizeMediumFont.cy-fixBigInterline;

    if (IsMultimapOverlaysGauges() && !mode.AnyPan())
        rcx = leftmargin + GlideBarOffset;
    else
        rcx = leftmargin;

    //
    // LEFT TOP
    //
    if (Overlay_LeftTop && (ISGLIDER||ISPARAGLIDER) ) {
        Surface.SelectObject(LK8OverlayBigFont);

        if (Overlay_LeftTop==OAUX)
            LKFormatValue(GetInfoboxType(4), true, BufferValue, BufferUnit, BufferTitle);
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
        LKWriteText(Surface, BufferValue, rcx, rcy+yoffset, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        if (!HideUnits) {
            Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
            Surface.SelectObject(MapScaleFont); 
            LKWriteText(Surface, BufferUnit, rcx + TextSize.cx, rcy+yoffset+unitbigoffset, 
                0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
   } // LeftTop

   //
   // LEFT MID
   //
   if (Overlay_LeftMid) {
        if (Overlay_LeftMid==OAUX) {
            LKFormatValue(GetInfoboxType(5), true, BufferValue, BufferUnit, BufferTitle);
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

        LKWriteText(Surface, BufferValue, rcx, rcy, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        if (!HideUnits) {
            Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
            Surface.SelectObject(MapScaleFont);
            LKWriteText(Surface, BufferUnit, rcx + TextSize.cx , rcy + unitbigoffset, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
    } // LeftMid

    //
    // LEFT BOTTOM
    // 
    if (Overlay_LeftBottom) {
        if (Overlay_LeftBottom==OAUX)
            LKFormatValue(GetInfoboxType(6), true, BufferValue, BufferUnit, BufferTitle);
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
        LKWriteText(Surface, BufferValue, rcx, rcy, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        if (!HideUnits) {
            Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
            Surface.SelectObject(MapScaleFont);
            LKWriteText(Surface, BufferUnit, rcx + TextSize.cx, rcy + unitbigoffset, 
                0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
    }

    //
    // CLOCK
    //
    if (OverlayClock || (ISPARAGLIDER && UseGates())) {
        LKFormatValue(LK_TIME_LOCALSEC, false, BufferValue, BufferUnit, BufferTitle);
        Surface.SelectObject(LK8OverlayMediumFont);
        if (!ScreenLandscape) {
            LKWriteText(Surface, BufferValue, rightmargin, compass.cy,
                0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
        } else { 
            LKWriteText(Surface, BufferValue, compass.cx, topmargin,
                0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
        }
    }

    //
    // LEFT DOWN (wind)
    //

    if (Overlay_LeftDown && !(MapSpaceMode != MSM_MAP && Current_Multimap_SizeY != SIZE4)) {
        Surface.SelectObject(LK8OverlayMediumFont);
        if (Overlay_LeftDown==OAUX) {
            LKFormatValue(GetInfoboxType(7), true, BufferValue, BufferUnit, BufferTitle);
        } else {
            if (ISCAR || ISGAAIRCRAFT) 
                LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
            else
                LKFormatValue(LK_WIND, false, BufferValue, BufferUnit, BufferTitle);
        }
        LKWriteText(Surface, BufferValue, leftmargin, yLeftWind,
            0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true); 

        if (!HideUnits) {
            Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
            Surface.SelectObject(MapScaleFont);
            LKWriteText(Surface, BufferUnit, leftmargin + TextSize.cx, yLeftWind + unitmediumoffset, 
                0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
    } // LeftDown


    // restore objects and return
    Surface.SelectObject(oldpen);
    Surface.SelectObject(oldbrush);
    Surface.SelectObject(oldfont);


}

