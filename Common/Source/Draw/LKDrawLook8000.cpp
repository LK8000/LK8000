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

/*
 * Draw Text Overlay. 
 * @Surface : surface to draw
 * @rc : Screen rect, ex. 0 800  0 480
 * @bThermaBar : true if place older for Thermal bar is needed ( use return value of #DrawThermalBand in most case).
 */
void MapWindow::DrawLook8000(LKSurface& Surface, const RECT& rc, bool bThermalBar) {

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

    short leftmargin = 0;

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
    static unsigned short fixBigInterline, unitbigoffset;

    // This is going to be the START 1/3  name replacing waypoint name when gates are running
    static TCHAR StartGateName[12];

    redwarning = false;

    if (INVERTCOLORS) {
        oldfont = Surface.SelectObject(LK8OverlaySmallFont);
        oldbrush = Surface.SelectObject(LKBrush_Black);
        oldpen = Surface.SelectObject(LKPen_Grey_N1);
    } else {
        oldfont = Surface.SelectObject(LK8OverlaySmallFont);
        oldbrush = Surface.SelectObject(LKBrush_White);
        oldpen = Surface.SelectObject(LKPen_Grey_N1);
    }


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
        unsigned short unitmediumoffset= SizeMediumFont.cy-SizeUnitFont.cy - (SizeMediumFont.cy-SizeUnitFont.cy)/5;
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
        // TahomaBD need some adjustments
        yMcSafety+=NIBLSCALE(2);
        yAltSafety= yrightoffset  - fixBigInterline + SizeBigFont.cy - fixBigInterline -1 +NIBLSCALE(2);
        #endif

        DoInit[MDI_DRAWLOOK8000] = false;

    } // end doinit

    distcolor=OverColorRef;

    // First we draw flight related values such as instant efficiency, altitude, new infoboxes etc.

    if (IsMultimapOverlaysGauges() && (LKVarioBar && !mode.AnyPan())) {
        leftmargin = (LKVarioSize + NIBLSCALE(3)); // VARIOWIDTH + middle separator right extension
    } else {
        leftmargin = LEFTMARGIN;
    }

    // no overlay - but we are still drawing MC and the wind on bottom left!
    if (Look8000 == (Look8000_t) lxcNoOverlay) goto drawOverlay;

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

    if (bThermalBar) {
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
            TCHAR buffername[LKSIZEBUFFERLARGE];
            GetOvertargetName(buffername);
            CharUpper(buffername);
            const int space_avail=name_xmax-rcx;
            int len=_tcslen(buffername);
            do {
                LK_tcsncpy(Buffer, buffername, len);
                Surface.GetTextSize(Buffer,len,&TextSize);
                if (TextSize.cx < space_avail) break;
            } while ( --len>2 );

            LKWriteText(Surface, Buffer, rcx, topmargin, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }

        //
        // TARGET DISTANCE
        //

        if (gateinuse >= -1) {
            // if we are still painting , it means we did not start yet..so we use colors
            if (!CorrectSide()) distcolor = AMBERCOLOR;
            LKFormatValue(LK_START_DIST, false, BufferValue, BufferUnit, BufferTitle);
        } else {
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

        if ((!OverlayClock || Look8000 == lxcStandard) && ScreenLandscape && (!(ISPARAGLIDER && UseGates()))) {
            _stprintf(BufferValue + _tcslen(BufferValue), _T(" %s"), BufferUnit);
            LKWriteText(Surface, BufferValue, compass.cx, topmargin, 
                0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
        } else
            LKWriteText(Surface, BufferValue, rcx , topmargin + SizeMediumFont.cy, 
                0, WTMODE_OUTLINED, WTALIGN_LEFT, distcolor, true);

        Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
        if (!HideUnits) {
            Surface.SelectObject(MapScaleFont); 
            if ((!OverlayClock || Look8000 == lxcStandard) && ScreenLandscape && !(ISPARAGLIDER && UseGates())) {

            } else {
                LKWriteText(Surface, BufferUnit, rcx + TextSize.cx, yDistUnit, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
            }
        }

        //
        // BEARING DIFFERENCE   displayed only when not circling
        //

        // if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
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

        Surface.SelectObject(LK8OverlayMediumFont);
        Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
        if (!ISGAAIRCRAFT) { 
            LKWriteText(Surface, BufferValue, topbearing.cx,  topbearing.cy, 0,
                WTMODE_OUTLINED, WTALIGN_CENTER, OverColorRef, true);
        }


        // 
        // EFFICIENCY REQUIRED  and altitude arrival for destination waypoint
        // For paragliders, average efficiency and arrival destination
        //

        Surface.SelectObject(LK8OverlayBigFont); // use this font for big values

        if (ISGLIDER) {
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

            rcy = yrightoffset - SizeBigFont.cy;
            rcx = rightmargin;
            color=redwarning?AMBERCOLOR:OverColorRef;
            LKWriteText(Surface, BufferValue, rcx, rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, color, true);


            //
            // ALTITUDE DIFFERENCE  at current MC
            //

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
            color=redwarning?AMBERCOLOR:OverColorRef;
            LKWriteText(Surface, BufferValue, rcx, yrightoffset - fixBigInterline, 0,
                WTMODE_OUTLINED, WTALIGN_RIGHT, color, true); 


            //
            // SAFETY ALTITUDE INDICATOR (NOT FOR PARAGLIDERS)
            // For PGs there is a separate drawing, although it should be identical right now.
            //

            if (IsSafetyAltitudeInUse(index)) {
                Surface.SelectObject(LK8OverlaySmallFont);
                _stprintf(BufferValue, _T(" + %.0f %s "), SAFETYALTITUDEARRIVAL / 10 * ALTITUDEMODIFY,
                        Units::GetUnitName(Units::GetUserAltitudeUnit()));
                LKWriteBoxedText(Surface, rc, BufferValue, rcx, yAltSafety,
                    0, WTALIGN_RIGHT, RGB_WHITE, RGB_BLACK);
            }
        }
    // end of index>0
    } else { 
        // no valid index for current overmode, but we print something nevertheless
        // normally, only the T>
        TCHAR buffername[LKSIZEBUFFERLARGE];
        GetOvertargetName(buffername);
        CharUpper(buffername);
        const int space_avail=name_xmax-rcx;
        int len=_tcslen(buffername);
        do {
            LK_tcsncpy(Buffer, buffername, len);
            Surface.GetTextSize(Buffer,len,&TextSize);
            if (TextSize.cx < space_avail) break;
        } while ( --len>2 );

        LKWriteText(Surface, Buffer, rcx, topmargin, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
    }

    // moved out from task paragliders stuff - this is painted on the right
    if (ISPARAGLIDER) {

        if (UseGates() && ActiveWayPoint == 0) {
            Surface.SelectObject(LK8OverlayBigFont);

            if (HaveGates()) {
                gatechrono = GateTime(ActiveGate) - LocalTime(); // not always already set, update it ... 

                Units::TimeToTextDown(BufferValue, gatechrono);
                rcx= rc.right-RIGHTMARGIN;
                rcy = yrightoffset - SizeBigFont.cy;
                LKWriteText(Surface, BufferValue, rcx, rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

                Value = WayPointCalc[DoOptimizeRoute() ? RESWP_OPTIMIZED : Task[0].Index].NextETE - gatechrono;
                Units::TimeToTextDown(BufferValue, (int) Value);
                color=(Value<=0)?AMBERCOLOR:OverColorRef;
                LKWriteText(Surface, BufferValue, rcx, yrightoffset - NIBLSCALE(2), 0, WTMODE_OUTLINED, WTALIGN_RIGHT, color, true); 

            }

        } else {
            Surface.SelectObject(LK8OverlayBigFont);
            if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
                LKFormatValue(LK_TC_30S, false, BufferValue, BufferUnit, BufferTitle);
            else
                LKFormatValue(LK_LD_AVR, false, BufferValue, BufferUnit, BufferTitle);

            rcy = yrightoffset - SizeBigFont.cy;
            rcx = rightmargin;
            LKWriteText(Surface, BufferValue, rcx, rcy, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

            // Altitude difference with current MC
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
            color=redwarning?AMBERCOLOR:OverColorRef;
            LKWriteText(Surface, BufferValue, rcx, yrightoffset - fixBigInterline, 0,
                WTMODE_OUTLINED, WTALIGN_RIGHT,color, true);  

            //
            // SAFETY ALTITUDE INDICATOR (FOR PARAGLIDERS)
            // Should be identical to that for other aircrafts, normally.
            //
            if (IsSafetyAltitudeInUse(GetOvertargetIndex())) {
                Surface.SelectObject(LK8OverlaySmallFont);
                _stprintf(BufferValue, _T(" + %.0f %s "), SAFETYALTITUDEARRIVAL / 10 * ALTITUDEMODIFY,
                        Units::GetUnitName(Units::GetUserAltitudeUnit()));
                LKWriteBoxedText(Surface, rc, BufferValue, rcx, yAltSafety,
                    0, WTALIGN_RIGHT, RGB_WHITE, RGB_BLACK);

            }
        } // end no UseGates()
    } // is paraglider

drawOverlay:
    // In place of MC, print gate time
    // Even if lxcNoOverlay, we print startgates..
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

        //
        // MAC CREADY VALUE
        //
        if (McOverlay && Look8000 > lxcNoOverlay && (ISGLIDER || ISPARAGLIDER)) {

        Surface.SelectObject(LK8OverlayBigFont);
        LKFormatValue(LK_MC, false, BufferValue, BufferUnit, BufferTitle);
        LKWriteText(Surface, BufferValue, rightmargin, yMcValue, 0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

        //
        // SAFETY MAC CREADY INDICATOR
        //
        extern bool IsSafetyMacCreadyInUse(int val);
        if (IsSafetyMacCreadyInUse(GetOvertargetIndex()) && GlidePolar::SafetyMacCready > 0) {
            Surface.SelectObject(LK8OverlaySmallFont);
            _stprintf(BufferValue, _T(" %.1f %s "), GlidePolar::SafetyMacCready*LIFTMODIFY,
                    Units::GetUnitName(Units::GetUserVerticalSpeedUnit()));
            LKWriteBoxedText(Surface, rc, BufferValue, rightmargin, yMcSafety, 0, WTALIGN_RIGHT, RGB_WHITE, RGB_BLACK);
        }

        //
        // AUTO MC INDICATOR
        //
        if (DerivedDrawInfo.AutoMacCready == true) {
            Surface.SelectObject(LK8OverlayMcModeFont);

            LKSurface::OldBrush ob{};
            if (LKTextBlack) {
                ob = Surface.SelectObject(LKBrush_White);
            }

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
            if (LKTextBlack) Surface.SelectObject(ob);
            LKWriteText(Surface, amcmode, rightmargin + NIBLSCALE(1), yMcMode,
                    0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_WHITE, true);

        } // AutoMacCready true AUTO MC INDICATOR

    }
    if (Look8000 == (Look8000_t) lxcNoOverlay) goto Drawbottom;


    if ((Look8000 == (Look8000_t) lxcAdvanced)) {

        Surface.SelectObject(LK8OverlayBigFont);
        if (ISPARAGLIDER) {
            LKFormatValue(LK_HNAV, false, BufferValue, BufferUnit, BufferTitle); // 091115
        } else {
            if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
                LKFormatValue(LK_TC_30S, false, BufferValue, BufferUnit, BufferTitle);
            else
                LKFormatValue(LK_LD_AVR, false, BufferValue, BufferUnit, BufferTitle);
        }
        Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
        if (IsMultimapOverlaysGauges() && !mode.AnyPan())
            rcx = leftmargin + GlideBarOffset;
        else
            rcx = leftmargin;
        //
        // CENTER THE LEFT OVERLAYS
        //
        short yoffset=0;
        if (ISPARAGLIDER || (IsMultimapOverlaysGauges() && LKVarioBar)) {

            rcy = yMcValue;
            if (OverlayClock && ScreenLandscape) yoffset = SizeMediumFont.cy-fixBigInterline;
        } else {
            rcy = yMcValue+SizeBigFont.cy-fixBigInterline;
        }
        LKWriteText(Surface, BufferValue, rcx, rcy+yoffset, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        if (!HideUnits) {
            Surface.SelectObject(MapScaleFont); 
            LKWriteText(Surface, BufferUnit, rcx + TextSize.cx, rcy+yoffset+unitbigoffset, 
                0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }

        if (ISPARAGLIDER || ((IsMultimapOverlaysGauges() && LKVarioBar)&&!ISCAR)) {
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

            Surface.SelectObject(LK8OverlayBigFont);
            rcy = yrightoffset - SizeBigFont.cy +yoffset;

            LKWriteText(Surface, BufferValue, rcx, rcy, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
            if (!HideUnits) {
                Surface.GetTextSize(BufferValue, _tcslen(BufferValue), &TextSize);
                Surface.SelectObject(MapScaleFont);
                LKWriteText(Surface, BufferUnit, rcx + TextSize.cx , rcy + unitbigoffset, 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
            }
        }
        if (!ISGAAIRCRAFT && !ISCAR) {
            if (ISPARAGLIDER) {
                LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
            } else {
                if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
                    LKFormatValue(LK_HNAV, false, BufferValue, BufferUnit, BufferTitle);
                } else {
                    LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
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

        LKFormatValue(LK_TIME_LOCALSEC, false, BufferValue, BufferUnit, BufferTitle);

        if (OverlayClock || (ISPARAGLIDER && UseGates())) {
            Surface.SelectObject(LK8OverlayMediumFont);
            if (!ScreenLandscape) {
                LKWriteText(Surface, BufferValue, rightmargin, compass.cy,
                    0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
            } else { 
                LKWriteText(Surface, BufferValue, compass.cx, topmargin,
                    0, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
            }
        }

    }

Drawbottom:

    if (MapSpaceMode != MSM_MAP && Current_Multimap_SizeY != SIZE4) goto TheEnd;


    //
    // Draw wind 
    //
    Surface.SelectObject(LK8OverlayMediumFont);

    if (Look8000 == lxcNoOverlay) goto afterWind; // 100930

    if (ISCAR || ISGAAIRCRAFT) {
        if (!HideUnits) {
            LKFormatValue(LK_GNDSPEED, false, BufferValue, BufferUnit, BufferTitle);
            _stprintf(Buffer, _T("%s %s"), BufferValue, BufferUnit);
        } else {
            LKFormatValue(LK_GNDSPEED, false, Buffer, BufferUnit, BufferTitle);
        }
    } else {
        LKFormatValue(LK_WIND, false, Buffer, BufferUnit, BufferTitle);
    }

    if (DrawBottom) {
        LKWriteText(Surface, Buffer, leftmargin, yLeftWind,
            0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true); 
    } else {
        // This is unused, we dont paint overlays in pan mode
        LKWriteText(Surface, Buffer, leftmargin, 
            rc.bottom - SizeMediumFont.cy - NIBLSCALE(5), 
            0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
    }

afterWind:


    TheEnd :

    // restore objects and return
    Surface.SelectObject(oldpen);
    Surface.SelectObject(oldbrush);
    Surface.SelectObject(oldfont);


}

