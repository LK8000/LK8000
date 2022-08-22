/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
#include "Bitmaps.h"
#include "Asset.hpp"
#include "Library/TimeFunctions.h"
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

namespace {

// For Overlay we use :  0       : Hidden
//                       1       : Default
//                       >= 1000 : Custom LKValue+1000
//
//constexpr bool isOverlayDefault(int Overlay) {return (Overlay==1);}
constexpr bool isOverlayHidden(int Overlay) {return (Overlay==0);}
constexpr bool isOverlayCustom(int Overlay) {return (Overlay>=1000);}
constexpr int  getCustomOverlay(int Overlay) {return (Overlay-1000);}


int GetTargetIndex() {
    if (UseGates() && ActiveTaskPoint == 0) {
        // if running a task, use the task index normally
        if (ValidTaskPoint(ActiveTaskPoint) != false) {
            if (DoOptimizeRoute())
                return RESWP_OPTIMIZED;
            else
                return Task[ActiveTaskPoint].Index;
        }
    }
    return GetOvertargetIndex();
}

}

void MapWindow::DrawTextOrBitmap(LKSurface& Surface, const TCHAR* wText, int x, int y, const bool lwmode, const short align, const LKColor& rgb_text, bool invertable, SIZE txtSize, DrawBmp_t bmp){
    const LKIcon* pBmpTemp = GetDrawBmpIcon(bmp);
    if (pBmpTemp) {
        PixelSize iconSize = pBmpTemp->GetSize();            
        if (IsDithered()&&!IsEinkColored()) {
            //Draw whit rect to see better the Icon
            PixelRect PrintAreaR = {
                { x - iconSize.cx, y }, // origin
                iconSize, // size
            };
            PrintAreaR.Grow(NIBLSCALE(1));

            Surface.FillRect(&PrintAreaR,LK_WHITE_BRUSH); //Draw rect to see bitmap better
        }
        pBmpTemp->Draw(Surface, x-iconSize.cx, y,iconSize.cx , iconSize.cy);
    }
    else {
        LKWriteText(Surface, wText, x, y, lwmode, align, rgb_text, true);
    }
}

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
    DrawBmp_t bmpValue = BmpNone;
    DrawBmp_t bmpTitle = BmpNone;
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

    static int yrightoffset, yMcSafety, yMcValue, yMcMode, yAltSafety, yLeftWind;
    static int yMcTitle ;
    static int yMcUnit;

    static int yRightMid;
    static int yRightMidTitle;
    static int yRightMidUnit;

    static int yRightBtm; 
    static int yRightBtmTitle;
    static int yRightBtmUnit;

    static SIZE compass, topbearing;
    static unsigned short fixBigInterline, unitbigoffset, unitmediumoffset;

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


        yMcValue = yrightoffset - 2* SizeBigFont.cy + fixBigInterline;
        yMcTitle = yMcValue + SizeSmallFont.cy;
        yMcUnit  = yMcValue + SizeMcModeFont.cy;       
        yMcSafety= yMcValue + fixBigInterline -1 - (SizeSmallFont.cy+BOXEDYMARGIN);
        yMcMode  = yMcValue + (SizeBigFont.cy -SizeMcModeFont.cy)/2;

        yRightMid     =  yrightoffset - SizeBigFont.cy + fixBigInterline;
        yRightMidTitle=  yRightMid + SizeSmallFont.cy;
        yRightMidUnit =  yRightMid + SizeMcModeFont.cy;

        yRightBtm      = yrightoffset + fixBigInterline;
        yRightBtmTitle = yRightBtm   + SizeSmallFont.cy;
        yRightBtmUnit  = yRightBtm   + SizeMcModeFont.cy;
        yAltSafety= yrightoffset  - fixBigInterline + SizeSmallFont.cy +SizeBigFont.cy - fixBigInterline -1;
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



    if (IsMultimapOverlaysGauges() && MapWindow::ThermalBarDrawn) {
        rcx = leftmargin + NIBLSCALE(40);
    } else {
        rcx = leftmargin;
    }

    int OverTargetIndex = GetTargetIndex();

    // Waypoint name and distance
    Surface.SelectObject(LK8OverlayMediumFont);

    if (OverTargetIndex >= 0) {
        // OVERTARGET reswp not using redwarning because Reachable is not calculated
        if (OverTargetIndex <= RESWP_END)
            redwarning = false;
        else {
            if (WayPointCalc[OverTargetIndex].AltArriv[AltArrivMode] > 0 && !WayPointList[OverTargetIndex].Reachable)
                redwarning = true;
            else
                redwarning = false;
        }
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
            gatechrono = GateTime(ActiveGate) - LocalTime(DrawInfo.Time);
        }
        if (gateinuse < 0) {
            // LKTOKEN  _@M157_ = "CLOSED"
            _tcscpy(Buffer, MsgToken(157));
        } else {
            const TCHAR* StartGateName = ScreenLandscape ? _T("Start ") : _T("ST ");
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
    LKFormatValue(LK_START_DIST, false, BufferValue, BufferUnit, BufferTitle,&bmpValue,&bmpTitle);
    if (gateinuse >= -1) {
        // if we are still painting , it means we did not start yet..so we use colors
        if (!CorrectSide()) distcolor = AMBERCOLOR;

    } else {
        if (!Overlay_TopRight) goto _skip_TopRight;
        if (OverTargetIndex < 0) goto _skip_TopRight;
        // Using FormatDist will give PGs 3 decimal units on overlay only
        // because changing FormatValue to 3 digits would bring them also
        // on bottom bar, and there is no space for 1.234km on the bottom bar.
        LKFormatDist(OverTargetIndex, BufferValue, BufferUnit);
    }

    if ( !OverlayClock && ScreenLandscape && (!((gTaskType==TSK_GP) && UseGates()))) {
        int dx = compass.cx ;
        int yDistUnit= topmargin + unitmediumoffset;
        if((!HideUnits) || Overlay_Title) {
            Surface.SelectObject(MapScaleFont);
            Surface.GetTextSize(BufferUnit, &TextSize);
            dx = dx - TextSize.cx+ NIBLSCALE(2);
        }
        if (!HideUnits) {
            LKWriteText(Surface, BufferUnit, dx, yDistUnit, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
        if(Overlay_Title) {
            Surface.SelectObject(LK8OverlaySmallFont);
            DrawTextOrBitmap(Surface, BufferTitle, dx, yDistUnit-SizeSmallFont.cy/2, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true, TextSize, bmpTitle);
        }
        Surface.SelectObject(LK8OverlayMediumFont);
        LKWriteText(Surface, BufferValue, dx, topmargin, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

    } else {
        int dx = rcx;
        int dy = topmargin + SizeMediumFont.cy;
        int yDistUnit= topmargin + SizeMediumFont.cy + unitmediumoffset;
        LKWriteText(Surface, BufferValue, dx ,dy, WTMODE_OUTLINED, WTALIGN_LEFT, distcolor, true);
        Surface.GetTextSize(BufferValue, &TextSize);
        dx = rcx + TextSize.cx + NIBLSCALE(2);
        if (!HideUnits) {
            Surface.SelectObject(MapScaleFont);
            LKWriteText(Surface, BufferUnit, dx, yDistUnit, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
        if(Overlay_Title) {
            Surface.SelectObject(LK8OverlaySmallFont);
            DrawTextOrBitmap(Surface, BufferTitle, dx, yDistUnit-SizeSmallFont.cy/2, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true, TextSize, bmpTitle);
        }
    }
    _skip_TopRight:

    //
    // BEARING DIFFERENCE   displayed only when not circling
    //
    if (!Overlay_TopMid) goto _skip_TopMid;
    if (OverTargetIndex < 0) goto _skip_TopMid;

    if (!ISGAAIRCRAFT) {
        LKFormatBrgDiff(OverTargetIndex, BufferValue, BufferUnit);
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

    // moved out from task paragliders stuff - this is painted on the right
    if (UseGates() && ActiveTaskPoint == 0) {
        Surface.SelectObject(LK8OverlayBigFont);

        if (HaveGates()) {
            // Time To Gate
            gatechrono = GateTime(ActiveGate) - LocalTime(DrawInfo.Time); // not always already set, update it ...

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

        //
        // TIME GATES:  in place of MC, print gate time
        //
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

    } else {
        if (ISGLIDER) {
            int unit_len = 0;
            bmpValue = BmpNone;
            bmpTitle = BmpNone;
            if (isOverlayHidden(Overlay_RightMid)) goto _skip_glider_RightMid;
            if (isOverlayCustom(Overlay_RightMid)) {
                LKFormatValue(getCustomOverlay(Overlay_RightMid), true, BufferValue, BufferUnit, BufferTitle,&bmpValue,&bmpTitle);
            } else {
                if (OverTargetIndex < 0) goto _skip_glider_RightMid;
               _tcscpy(BufferTitle, _T(""));
                LKFormatGR(OverTargetIndex, BufferValue, BufferUnit);
            }
            rcy = yrightoffset - SizeBigFont.cy;
            if (!HideUnits) {
                Surface.SelectObject(MapScaleFont);
                Surface.GetTextSize(BufferUnit, &TextSize);
                unit_len = TextSize.cx;
                LKWriteText(Surface, BufferUnit, rcx-unit_len, yRightMidUnit, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
            }

            Surface.SelectObject(LK8OverlayBigFont); // use this font for big values

            color=(redwarning&&!isOverlayCustom(Overlay_RightMid))?AMBERCOLOR:OverColorRef;
            Surface.GetTextSize(BufferValue, &TextSize);
            DrawTextOrBitmap(Surface, BufferValue, rcx-unit_len, yRightMid, WTMODE_OUTLINED, WTALIGN_RIGHT, color, true, TextSize, bmpValue);
            if(Overlay_Title){
              Surface.GetTextSize(BufferValue, &TextSize);
              int dx = TextSize.cx+ NIBLSCALE(2);
              Surface.SelectObject(LK8OverlaySmallFont);
              DrawTextOrBitmap(Surface, BufferTitle, rcx-unit_len-dx, yRightMid, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true, TextSize, bmpTitle);
            }

            _skip_glider_RightMid:
            Surface.SelectObject(LK8OverlayBigFont);
            //
            // RIGHT BOTTOM  AUX-3
            // (GLIDERS) ALTITUDE DIFFERENCE  at current MC
            //
            unit_len =0;
            if (isOverlayHidden(Overlay_RightBottom)) goto _skip_glider_RightBottom;
            bmpValue = BmpNone;
            bmpTitle = BmpNone;
            if (isOverlayCustom(Overlay_RightBottom))
            {                
                LKFormatValue(getCustomOverlay(Overlay_RightBottom), true, BufferValue, BufferUnit, BufferTitle,&bmpValue,&bmpTitle);
            } else {
              if (OverTargetIndex < 0)  goto _skip_glider_RightBottom;
              _tcscpy(BufferTitle, _T(""));
              LKFormatAltDiff(OverTargetIndex, BufferValue, BufferUnit);
            }

            if (!HideUnits) {
                Surface.SelectObject(MapScaleFont);
                Surface.GetTextSize(BufferUnit, &TextSize);
                unit_len = TextSize.cx;
                LKWriteText(Surface, BufferUnit, rcx-unit_len, yRightBtmUnit, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
                Surface.SelectObject(LK8OverlayBigFont);
            }

            color=(redwarning&&!isOverlayCustom(Overlay_RightBottom))?AMBERCOLOR:OverColorRef;
            DrawTextOrBitmap(Surface, BufferValue, rcx-unit_len, yRightBtm, WTMODE_OUTLINED, WTALIGN_RIGHT, color, true, TextSize, bmpValue);
            if(Overlay_Title){
              Surface.GetTextSize(BufferValue, &TextSize);
              int dx = TextSize.cx+ NIBLSCALE(2);
              Surface.SelectObject(LK8OverlaySmallFont);
              DrawTextOrBitmap(Surface, BufferTitle, rcx-unit_len-dx, yRightBtmTitle, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true, TextSize, bmpTitle);
            }
            //
            // (GLIDERS) SAFETY ALTITUDE INDICATOR
            // For PGs there is a separate drawing, although it should be identical right now.
            //

            if (IsSafetyAltitudeInUse(OverTargetIndex)&&!isOverlayCustom(Overlay_RightBottom)) {
                Surface.SelectObject(LK8OverlaySmallFont);
                _stprintf(BufferValue, _T(" + %.0f %s "), SAFETYALTITUDEARRIVAL / 10 * ALTITUDEMODIFY,
                        Units::GetUnitName(Units::GetUserAltitudeUnit()));
                LKWriteBoxedText(Surface, rc, BufferValue, rcx, yAltSafety, WTALIGN_RIGHT, RGB_WHITE, RGB_WHITE);
            }
            _skip_glider_RightBottom: ;
        } else { // ISGLIDER
            int unit_len=0;
            _tcscpy(BufferTitle, _T(""));
            _tcscpy(BufferUnit, _T(""));
            Surface.SelectObject(LK8OverlayBigFont);
            if (isOverlayHidden(Overlay_RightMid)) goto _skip_para_RightMid;
            bmpValue = BmpNone;
            bmpTitle = BmpNone;
            if (isOverlayCustom(Overlay_RightMid))
                LKFormatValue(getCustomOverlay(Overlay_RightMid), true, BufferValue, BufferUnit, BufferTitle,&bmpValue,&bmpTitle);
            else {
                if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING))
                    LKFormatValue(LK_TC_30S, false, BufferValue, BufferUnit, BufferTitle);
                else
                    LKFormatValue(LK_LD_AVR, false, BufferValue, BufferUnit, BufferTitle);
            }

            rcy = yrightoffset - SizeBigFont.cy;
            if (!HideUnits) {
                Surface.SelectObject(MapScaleFont);
                Surface.GetTextSize(BufferUnit, &TextSize);
                unit_len = TextSize.cx;
                LKWriteText(Surface, BufferUnit, rcx-unit_len, yRightMidUnit, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
            }
             Surface.SelectObject(LK8OverlayBigFont);

            rcx = rightmargin;
            Surface.GetTextSize(BufferValue, &TextSize);
            DrawTextOrBitmap(Surface, BufferValue, rcx-unit_len, yRightMid, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true, TextSize, bmpValue);
            if(Overlay_Title){
              Surface.GetTextSize(BufferValue, &TextSize);
              int dx = TextSize.cx+unit_len+ NIBLSCALE(2);
              Surface.SelectObject(LK8OverlaySmallFont);
              DrawTextOrBitmap(Surface, BufferTitle, rcx-dx, yRightMidTitle, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true, TextSize, bmpTitle);
              Surface.SelectObject(LK8OverlayBigFont);
            }

            _skip_para_RightMid:


            _tcscpy(BufferTitle, _T(""));
            _tcscpy(BufferUnit, _T(""));
            // Altitude difference with current MC
            if (isOverlayHidden(Overlay_RightBottom)) goto _skip_para_RightBottom;
            bmpValue = BmpNone;
            bmpTitle = BmpNone;
            if (isOverlayCustom(Overlay_RightBottom)) {
                LKFormatValue(getCustomOverlay(Overlay_RightBottom), true, BufferValue, BufferUnit, BufferTitle,&bmpValue,&bmpTitle);
            } else {
                if (OverTargetIndex < 0) goto _skip_para_RightBottom;
                LKFormatAltDiff(OverTargetIndex, BufferValue, BufferUnit);
            }

            rcy = yrightoffset - SizeBigFont.cy;
            if (!HideUnits) {
                Surface.SelectObject(MapScaleFont);
                Surface.GetTextSize(BufferUnit, &TextSize);
                unit_len = TextSize.cx;
                LKWriteText(Surface, BufferUnit, rcx-unit_len, yRightBtmUnit, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
            }
            Surface.SelectObject(LK8OverlayBigFont);
            color=(redwarning&&!isOverlayCustom(Overlay_RightBottom))?AMBERCOLOR:OverColorRef;
            Surface.GetTextSize(BufferValue, &TextSize);
            DrawTextOrBitmap(Surface, BufferValue, rcx-unit_len, yRightBtm, WTMODE_OUTLINED, WTALIGN_RIGHT, color, true, TextSize, bmpValue);
            if(Overlay_Title){
                Surface.GetTextSize(BufferValue, &TextSize);
                int dx = TextSize.cx+ unit_len+ NIBLSCALE(2);
                Surface.SelectObject(LK8OverlaySmallFont);
                DrawTextOrBitmap(Surface, BufferTitle, rcx-dx, yRightBtmTitle, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true, TextSize, bmpTitle);
                Surface.SelectObject(LK8OverlayBigFont);
            }
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
        }
    } // end no UseGates()

    int right_m = rightmargin;
    if (!UseGates() || ActiveTaskPoint != 0) {
        if ( (ISGLIDER || ISPARAGLIDER) && !isOverlayHidden(Overlay_RightTop)) {
            //
            // MAC CREADY VALUE
            //
            bmpValue = BmpNone;
            bmpTitle = BmpNone;
            Surface.SelectObject(LK8OverlayBigFont);
            if (isOverlayCustom(Overlay_RightTop)) {
              LKFormatValue(getCustomOverlay(Overlay_RightTop), true, BufferValue, BufferUnit, BufferTitle,&bmpValue,&bmpTitle);
              right_m = rightmargin;
            }
            else
                LKFormatValue(LK_MC, false, BufferValue, BufferUnit, BufferTitle);

            if (!HideUnits) {    
                Surface.SelectObject(MapScaleFont);
                Surface.GetTextSize(BufferUnit, &TextSize);                
                right_m =   rightmargin - TextSize.cx;   
                LKWriteText(Surface, BufferUnit, right_m , yMcUnit , WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);                   
            }
            Surface.SelectObject(LK8OverlayBigFont);
            LKWriteText(Surface, BufferValue, right_m, yMcValue, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

            if(Overlay_Title){
              Surface.SelectObject(LK8OverlayBigFont);
              Surface.GetTextSize(BufferValue, &TextSize);
              int dx = TextSize.cx+ NIBLSCALE(2);
              Surface.SelectObject(LK8OverlaySmallFont);
              DrawTextOrBitmap(Surface, BufferTitle, right_m-dx, yMcTitle, WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true, TextSize, bmpTitle);
            } 
            //
            // SAFETY MAC CREADY INDICATOR
            //

            if (!isOverlayCustom(Overlay_RightTop) && IsSafetyMacCreadyInUse(OverTargetIndex) && GlidePolar::SafetyMacCready > 0) {
                Surface.SelectObject(LK8OverlaySmallFont);
                _stprintf(BufferValue, _T(" %.1f %s "), GlidePolar::SafetyMacCready*LIFTMODIFY,
                    Units::GetUnitName(Units::GetUserVerticalSpeedUnit()));
                LKWriteBoxedText(Surface, rc, BufferValue, right_m, yMcSafety, WTALIGN_RIGHT, RGB_WHITE, RGB_WHITE);
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
    }
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
        bmpValue = BmpNone;
        bmpTitle = BmpNone;
        if (isOverlayCustom(Overlay_LeftTop))
            LKFormatValue(getCustomOverlay(Overlay_LeftTop), true, BufferValue, BufferUnit, BufferTitle,&bmpValue,&bmpTitle);
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
        Surface.GetTextSize(BufferValue, &TextSize);
        if (!HideUnits) {
            Surface.SelectObject(MapScaleFont);
            LKWriteText(Surface, BufferUnit, rcx + TextSize.cx, rcy+yoffset+unitbigoffset, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
        if(Overlay_Title){
          Surface.SelectObject(LK8OverlaySmallFont);
          LKWriteText(Surface, BufferTitle, rcx + TextSize.cx, rcy+yoffset+unitbigoffset-SizeSmallFont.cy, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
    } // LeftTop

    //
    // LEFT MID
    //
    if (!isOverlayHidden(Overlay_LeftMid)) {
        bmpValue = BmpNone;
        bmpTitle = BmpNone;
        if (isOverlayCustom(Overlay_LeftMid)) {
            LKFormatValue(getCustomOverlay(Overlay_LeftMid), true, BufferValue, BufferUnit, BufferTitle,&bmpValue,&bmpTitle);
        } else {
            if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) || LKVarioVal == vValVarioVario) {
                LKFormatValue(LK_VARIO, false, BufferValue, BufferUnit, BufferTitle,&bmpValue,&bmpTitle);
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

        if(Overlay_Title){
          if(HideUnits) Surface.GetTextSize(BufferValue, &TextSize);
          Surface.SelectObject(LK8OverlaySmallFont);
          LKWriteText(Surface, BufferTitle, rcx + TextSize.cx, rcy + unitbigoffset-SizeSmallFont.cy , WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }

    } // LeftMid

    //
    // LEFT BOTTOM
    //
    if (!isOverlayHidden(Overlay_LeftBottom)) {
        bmpValue = BmpNone;
        bmpTitle = BmpNone;
        if (isOverlayCustom(Overlay_LeftBottom))
            LKFormatValue(getCustomOverlay(Overlay_LeftBottom), true, BufferValue, BufferUnit, BufferTitle,&bmpValue,&bmpTitle);
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
        if(Overlay_Title){
          if((HideUnits)) Surface.GetTextSize(BufferValue, &TextSize);
          Surface.SelectObject(LK8OverlaySmallFont);     
          LKWriteText(Surface, BufferTitle, rcx + TextSize.cx, rcy + unitbigoffset-SizeSmallFont.cy, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }
    }

    //
    // CLOCK
    //
    if ((OverlayClock && Overlay_TopRight) || ((gTaskType==TSK_GP) && UseGates())) {
        LKFormatValue(LK_TIME_LOCALSEC, false, BufferValue, BufferUnit, BufferTitle);
        Surface.SelectObject(LK8OverlayMediumFont);
        int cx,cy;
        if (!ScreenLandscape) {
            LKWriteText(Surface, BufferValue, rightmargin, compass.cy,
                WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);
            cx = rightmargin;
            cy = compass.cy;
        } else {
          cx = compass.cx;
          cy = topmargin;
        }
        LKWriteText(Surface, BufferValue, cx, cy,
                WTMODE_OUTLINED, WTALIGN_RIGHT, OverColorRef, true);

    }

    //
    // LEFT DOWN (wind)
    //

    if (!isOverlayHidden(Overlay_LeftDown) && !(MapSpaceMode != MSM_MAP && Current_Multimap_SizeY != SIZE4)) {
        bmpValue = BmpNone;
        bmpTitle = BmpNone;
        Surface.SelectObject(LK8OverlayMediumFont);
        if (isOverlayCustom(Overlay_LeftDown)) {
            LKFormatValue(getCustomOverlay(Overlay_LeftDown), true, BufferValue, BufferUnit, BufferTitle,&bmpValue,&bmpTitle);
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
        if(Overlay_Title){
          if(HideUnits) Surface.GetTextSize(BufferValue, &TextSize);
          Surface.SelectObject(LK8OverlaySmallFont);      
          LKWriteText(Surface, BufferTitle, leftmargin + TextSize.cx, yLeftWind+SizeSmallFont.cy/2 , WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true);
        }   
    } // LeftDown


    // restore objects and return
    Surface.SelectObject(oldpen);
    Surface.SelectObject(oldbrush);
    Surface.SelectObject(oldfont);
}
