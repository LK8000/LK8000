/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "McReady.h"
#include "InputEvents.h"
#include "Modeltype.h"
#include "Bitmaps.h"
#include "RGB.h"
#include "DoInits.h"
#include "Multimap.h"
#include "Logger.h"
#include "Dialogs.h"
#include "Event/Event.h"
#include "Asset.hpp"
#include "Sound/Sound.h"
#include "TunedParameter.h"
#include "ScreenProjection.h"
#include "NavFunctions.h"
#include "InfoBoxLayout.h"

#ifndef ENABLE_OPENGL
#include "Screen/LKBitmapSurface.h"
#endif

#ifdef TESTBENCH
// USE ONLY FOR DEBUGGING PURPOSES! Normally disabled.
// #define TRACEKEY
#endif

#define KEYDEBOUNCE 100
extern int ProcessSubScreenVirtualKey(int X, int Y, long keytime, short vkmode);

// #define DEBUG_VIRTUALKEYS
// #define DEBUG_MAPINPUT

static bool isListPage(void);

LKColor taskcolor = RGB_TASKLINECOL; // 091216
static bool ignorenext=false;
static bool MouseWasPanMoving=false;

#ifndef ENABLE_OPENGL
bool OnFastPanning=false;
#endif

MapWindow::Zoom MapWindow::zoom;
MapWindow::Mode MapWindow::mode;

#ifdef ENABLE_OPENGL
LKColor MapWindow::AboveTerrainColor;
#else
LKBrush  MapWindow::hAboveTerrainBrush;
#endif

int MapWindow::ScaleListCount = 0;
double MapWindow::ScaleList[];
int MapWindow::ScaleCurrent;

POINT MapWindow::Orig_Screen;

RECT MapWindow::MapRect; // the entire screen area in use
RECT MapWindow::DrawRect; // the portion of MapRect for drawing terrain, topology etc. (the map)

#ifndef ENABLE_OPENGL
LKWindowSurface MapWindow::WindowSurface;
LKBitmapSurface MapWindow::TempSurface;

LKMaskBitmapSurface MapWindow::hdcMask;
LKBitmapSurface MapWindow::hdcbuffer;
#endif

double MapWindow::PanLatitude = 0.0;
double MapWindow::PanLongitude = 0.0;

bool MapWindow::targetMoved = false;
double MapWindow::targetMovedLat = 0;
double MapWindow::targetMovedLon = 0;

int MapWindow::TargetPanIndex = 0;
double MapWindow::TargetZoomDistance = 500.0;
bool MapWindow::EnableTrailDrift=false; // initialized again in Globals. This is only the runtime, not the config.
int MapWindow::GliderScreenPosition = 40; // 20% from bottom
int MapWindow::GliderScreenPositionX = 50;  // 100216
int MapWindow::GliderScreenPositionY = 40;

double MapWindow::DisplayAngle = 0.0;
double MapWindow::DisplayAircraftAngle = 0.0;

unsigned MapWindow::targetPanSize = 0;

bool MapWindow::LandableReachable = false;

LKPen MapWindow::hSnailPens[NUMSNAILCOLORS+1];

#ifdef ENABLE_OPENGL
std::array<FloatPoint, NUMTERRAINSWEEPS+2> MapWindow::Groundline;
std::array<FloatPoint, NUMTERRAINSWEEPS+1> MapWindow::Groundline2;
#else
std::array<RasterPoint, NUMTERRAINSWEEPS+1> MapWindow::Groundline;
std::array<RasterPoint, NUMTERRAINSWEEPS+1> MapWindow::Groundline2;
#endif

// 17 is number of airspace types
int      MapWindow::iAirspaceColour[AIRSPACECLASSCOUNT] = {5,0,0,10,0,0,10,2,0,10,9,3,7,7,7,10,1,3};
int      MapWindow::iAirspaceMode[AIRSPACECLASSCOUNT] = {0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0};
#ifdef HAVE_HATCHED_BRUSH
int      MapWindow::iAirspaceBrush[AIRSPACECLASSCOUNT] = {2,0,0,0,3,3,3,3,0,3,2,3,3,3,3,3,3,3};
#endif

LKPen MapWindow::hAirspacePens[AIRSPACECLASSCOUNT];
LKPen MapWindow::hBigAirspacePens[AIRSPACECLASSCOUNT];
LKPen MapWindow::hAirspaceBorderPen;

BrushReference MapWindow::hInvBackgroundBrush[LKMAXBACKGROUNDS];

LKBrush  MapWindow::hAirspaceBrushes[NUMAIRSPACEBRUSHES];

LKColor MapWindow::Colours[NUMAIRSPACECOLORS] =
  {LKColor(0xFF,0x00,0x00), LKColor(0x00,0xFF,0x00),
   LKColor(0x00,0x00,0xFF), LKColor(0xFF,0xFF,0x00),
   LKColor(0xFF,0x00,0xFF), LKColor(0x00,0xFF,0xFF),
   LKColor(0x7F,0x00,0x00), LKColor(0x00,0x7F,0x00),
   LKColor(0x00,0x00,0x7F), LKColor(0x7F,0x7F,0x00),
   LKColor(0x7F,0x00,0x7F), LKColor(0x00,0x7F,0x7F),
   LKColor(0xFF,0xFF,0xFF), LKColor(0xC0,0xC0,0xC0),
   LKColor(0x7F,0x7F,0x7F), LKColor(0x00,0x00,0x00),
   LKColor(0x7F,0x7F,0x7F)
};


PenReference MapWindow::hpAircraft;
LKPen MapWindow::hpWindThick;

LKPen MapWindow::hpThermalBand;
LKPen MapWindow::hpThermalBandGlider;
LKPen MapWindow::hpFinalGlideAbove;
LKPen MapWindow::hpFinalGlideBelow;
LKPen MapWindow::hpTerrainLine;
LKPen MapWindow::hpTerrainLineBg;
LKPen MapWindow::hpStartFinishThick;
LKPen MapWindow::hpStartFinishThin;

bool MapWindow::MapDirty = true;
bool PanRefreshed=false;

bool MapWindow::ForceVisibilityScan = false;
bool MapWindow::ThermalBarDrawn = false;

NMEA_INFO MapWindow::DrawInfo;
DERIVED_INFO MapWindow::DerivedDrawInfo;

extern void ShowMenu();


// Values to be remembered
bool MapWindow::pressed = false;
double MapWindow::Xstart = 0.;
double MapWindow::Ystart = 0.;

PeriodClock MapWindow::tsDownTime;

double MapWindow::Xlat = 0.;
double MapWindow::Ylat = 0.;

// Touch Screen Events Area
short MapWindow::Y_BottomBar;

POINT MapWindow::P_Doubleclick_bottomright;
POINT MapWindow::P_MenuIcon_DrawBottom;
POINT MapWindow::P_MenuIcon_noDrawBottom;

POINT MapWindow::P_UngestureLeft;
POINT MapWindow::P_UngestureRight;

short MapWindow::Y_Up, MapWindow::Y_Down;
short MapWindow::X_Left, MapWindow::X_Right;

//
// Dragged position on screen: start and and end coordinates (globals!)
//
POINT startScreen;
POINT targetScreen;


void MapWindow::_OnSize(int cx, int cy) {
#ifndef ENABLE_OPENGL
    // this is Used for check Thread_Draw don't use surface object.
    ScopeLock Lock(Surface_Mutex);

    BackBufferSurface.Resize(cx, cy);

    DrawSurface.Resize(cx, cy);

    TempSurface.Resize(cx, cy);
    hdcbuffer.Resize(cx, cy);
    hdcMask.Resize(cx, cy);
#endif

    // don't resize BackBufferSuface, resize is done by next Begin.
    MapDirty = true;
}

void MapWindow::UpdateActiveScreenZone(RECT rc) {

    #if TESTBENCH
    StartupStore(_T("... ** UpdateActiveScreenZone %d,%d,%d,%d\n"),rc.left,rc.top,rc.right,rc.bottom);
    #endif

    Y_BottomBar = rc.bottom - BottomSize;
    P_Doubleclick_bottomright.x = rc.right - BottomSize - NIBLSCALE(15);
    P_Doubleclick_bottomright.y = rc.bottom - BottomSize - NIBLSCALE(15);
    P_MenuIcon_DrawBottom.y = Y_BottomBar - 14;
    P_MenuIcon_noDrawBottom.y = rc.bottom - AircraftMenuSize;
    P_MenuIcon_DrawBottom.x = rc.right - AircraftMenuSize;
    P_MenuIcon_noDrawBottom.x = P_MenuIcon_DrawBottom.x;
    P_UngestureLeft.x = CompassMenuSize;
    P_UngestureLeft.y = CompassMenuSize;
    P_UngestureRight.x = rc.right - CompassMenuSize;
    P_UngestureRight.y = CompassMenuSize;
    Y_Up = Y_BottomBar / 2;
    Y_Down = Y_BottomBar - Y_Up;
    X_Left = (rc.right+rc.left)/2 - (rc.right-rc.left)/3;
    X_Right = (rc.right+rc.left)/2 + (rc.right-rc.left)/3;

}

void MapWindow::_OnCreate(Window& Wnd, int cx, int cy) {
#ifndef ENABLE_OPENGL
    WindowSurface.Create(Wnd);
    DrawSurface.Create(WindowSurface, cx, cy);
    TempSurface.Create(WindowSurface, cx, cy);
    hdcbuffer.Create(WindowSurface, cx, cy);
    hdcMask.Create(WindowSurface, cx, cy);
#else
    LKWindowSurface WindowSurface(Wnd);
#endif

    BackBufferSurface.Create(WindowSurface, cx, cy);
}

void MapWindow::_OnDestroy() {

    BackBufferSurface.Release();

#ifndef ENABLE_OPENGL
    DrawSurface.Release();

    TempSurface.Release();
    hdcbuffer.Release();
    hdcMask.Release();
#endif
}

/*
 * Handle Mouse Event when we are in Pan Mode for Drag Map.
 */
void MapWindow::_OnDragMove(const POINT& Pos) {
    // Consider only pure PAN mode, ignore the rest of cases here
    if (mode.Is(Mode::MODE_PAN) && !mode.Is(Mode::MODE_TARGET_PAN)) {
        const ScreenProjection _Proj;

#ifndef ENABLE_OPENGL
        if (PanRefreshed) {
            // if map was redrawn, we update our position as well. just like
            // we just clicked on mouse from here, like in BUTTONDOWN
            startScreen = Pos;

            _Proj.Screen2LonLat(startScreen, Xstart, Ystart);
            PanRefreshed = false;
            // NO! This is causing false clicks passing underneath CANCEL button!
            // dwDownTime.update();
        } else {
#endif
            // set a min mouse move to trigger panning
            
            if ( ManhattanDistance<POINT, int>(startScreen, Pos) > IBLSCALE(1) ) {
                _Proj.Screen2LonLat(Pos, Xlat, Ylat);
                PanLongitude += (Xstart - Xlat);
                PanLatitude += (Ystart - Ylat);

                if (ValidTaskPoint(PanTaskEdit)) {
                    LockTaskData(); // protect from external task changes
                    WayPointList[RESWP_PANPOS].Latitude = PanLatitude;
                    WayPointList[RESWP_PANPOS].Longitude = PanLongitude;
                    CalculateTaskSectors(PanTaskEdit);
                    UnlockTaskData(); // protect from external task changes
                }

                ignorenext = true;
                MouseWasPanMoving = true;

#ifdef ENABLE_OPENGL
                RefreshMap();
            }
#else

                // With OnFastPanning we are saying: Since we are dragging the bitmap around,
                // forget the Redraw requests from other parts of LK, which would cause PanRefreshed.
                // We have no control on those requests issued for example by Calc thread.
                // However we force full map refresh after some time in ms

                // If time has passed  then force a MapDirty and redraw the whole screen.
                // This was previously not working in v3 because ThreadCalculations was forcing MapDirty
                // in the background each second, and we were loosing control!
                if (tsDownTime.CheckUpdate(TunedParameter_Fastpanning() )) {
                    OnFastPanning = false;
                    RefreshMap();
                } else {
                    targetScreen = Pos;
                    // Lets not forget that if we stop moving the mouse, or we exit the windows while
                    // dragging, or we endup on another window -f.e. a button - then we will NOT
                    // receive any more events in this loop! For this reason we must let the
                    // DrawThread clear this flag. Not thread safe, I know.
                    //
                    // There is a situation to consider, here: we are setting OnFastPanning but
                    // if we stop moving the mouse/finger, we shall never be back in MapWndProc .
                    // Remember.
                    OnFastPanning = true;

                    // This will force BitBlt operation in RenderMapWindow
                    RequestFastRefresh();
                }
            } // minimal movement detected
        } // mouse moved with Lbutton (dragging)
#endif
    }
}

/*
 * Handle Left Button Down or Touch Screen Down
 *    Drag Map start in Pan mode or long click begin.
 */

#define WM_USER_LONGTIME_CLICK_TIMER  50

void MapWindow::_OnLButtonDown(const POINT& Pos) {
#ifdef TRACEKEY
StartupStore(_T("BUTTON DOWN ")); // no CR
#endif
    if (!LockModeStatus) {
        pressed = true;
        tsDownTime.Update();
#ifdef LONGCLICK_FEEDBACK
        if (!mode.Is(Mode::MODE_PAN)) {
            SetTimer(hWnd, WM_USER_LONGTIME_CLICK_TIMER, AIRSPACECLICK, NULL);
        }
#endif
        // When we have buttondown these flags should be set off.
        MouseWasPanMoving = false;

#ifndef ENABLE_OPENGL
        OnFastPanning = false;
#endif

        // After calling a menu, on exit as we touch the screen we fall back here
        if (ignorenext) {
#ifdef DEBUG_MAPINPUT
            DoStatusMessage(TEXT("DBG-055 BUTTONDOWN with ignorenext"));
#endif
        } else {
            startScreen = Pos;

            // TODO VNT move Screen2LatLon in LBUTTONUP after making sure we really need Xstart and Ystart
            // so we save precious milliseconds waiting for BUTTONUP GetTickCount
            const ScreenProjection _Proj;
            _Proj.Screen2LonLat(startScreen, Xstart, Ystart);

            LKevent = LKEVENT_NONE; // CHECK FIX TODO VENTA10  probably useless 090915
        }
    }
    #ifdef TRACEKEY
    else StartupStore(_T(".... OnButtonDown in LockMode, keypress ignored  (pressed=%d)%s"),pressed,NEWLINE);
    #endif
}

/*
 * Touch screen or Left mouse button double click
 *   Unlock screen if inside bottom right screen corner if screen is locked
 *   otherwise, call OnLButtonDown ( this is probably useless )
 */
void MapWindow::_OnLButtonDblClick(const POINT& Pos) {
#ifdef TRACEKEY
    StartupStore(_T("BUTTON DOUBLE CLICK "));
#endif
    // WINDOWS: Attention please: a DBLCLK is followed by a simple BUTTONUP with NO buttondown.
    // LINUX: a DBLCLK is preceded by buttondown-up (two times), then DblClick.
    //        Sometimes a single down-up, dlblclk, down-up
    //
    // 111110 there is no need to process buttondblclick if the map is unlocked.
    // So we go directly to buttondown, simulating a non-doubleclick.
    if (!LockModeStatus) {
        _OnLButtonDown(Pos);
    } else {
        tsDownTime.Update();
        startScreen = Pos;

        ignorenext = true; // do not interpret the second click of the doubleclick as a real click.
        if ((startScreen.x >= P_Doubleclick_bottomright.x)
                && (startScreen.y >= P_Doubleclick_bottomright.y)) {
            LockMode(2);
            DoStatusMessage(MsgToken(964)); // SCREEN IS UNLOCKED

            // Careful! If you ignorenext, any event timed as double click of course will be affected.
            // and this means also fast clicking on bottombar!!
            // so first lets see if we are in lk8000 text screens..
            // The idea was/is: break if we are in the nearest pages, or in main map but on the bottom bar.
            if ((DONTDRAWTHEMAP)
                    || (IsMultiMap() && (startScreen.y >= Y_BottomBar))) {
                // do not ignore next, let buttonup get the signal

            }
        }
        #ifdef TRACEKEY
        else StartupStore(_T(".... DoubleClick out of range, ignored. Screen is locked, pressed=%d%s"),pressed,NEWLINE);
        #endif
    }
}

void MapWindow::_OnLButtonUp(const POINT& Pos) {
#ifdef TRACEKEY
    StartupStore(_T("BUTTON UP "));
#endif
    if (!LockModeStatus && pressed) {

        const ScreenProjection _Proj;

        pressed = false;
        // Mouse released DURING panning, full redraw requested.
        // Otherwise process virtual keys etc. as usual
        if (MouseWasPanMoving) {
            MouseWasPanMoving = false;

#ifndef ENABLE_OPENGL
            OnFastPanning = false;
#endif

            /**
             * if TaskEdit in progrees, we need to recalculate all task
             *   only edited taskpoint are recalculated when panning
             */
            if (ValidTaskPoint(PanTaskEdit)) {
                LockTaskData(); // protect from external task changes
                CalculateTaskSectors();
                UnlockTaskData(); // protect from external task changes
            }


            ignorenext = false;
            RefreshMap();
            tsDownTime.Reset(); // otherwise we shall get a fake click passthrough
            return;
        }
        // This means that there was no buttondown, in practice, or that we have to ignore it
        if (ignorenext || (!tsDownTime.IsDefined())) {
            ignorenext = false;
            #ifdef TRACEKEY
            StartupStore(_T("... ButtonUp with tsDownTime==0, ignored%s"),NEWLINE);
            #endif
            return;
        }

        int dwInterval = tsDownTime.Elapsed();
        tsDownTime.Reset(); // do it once forever

        // This happens expecially with fake doubleclicks or loosy touch screen: buttondow/up immediate
        if (dwInterval == 0) {
           #ifdef TRACEKEY
           StartupStore(_T("... ButtonUp Debounced dwInterval==0%s"),NEWLINE);
           #endif
           return;
        }

        // we save these flags for the entire processing, just in case they change
        // while processing a virtual key for example, and also for acceleration.
        bool dontdrawthemap = (DONTDRAWTHEMAP);


        // LK v6: check we are not out of MapRect bounds.
        if (Pos.x<MapWindow::MapRect.left||Pos.x>MapWindow::MapRect.right||Pos.y<MapWindow::MapRect.top||Pos.y>MapWindow::MapRect.bottom) {
            ProcessSubScreenVirtualKey(Pos.x, Pos.y, dwInterval, LKGESTURE_NONE);
            return;
        }

        int gestDir = LKGESTURE_NONE;

        int gestX = startScreen.x - Pos.x;
        int gestY = startScreen.y - Pos.y;
        int gestDist = isqrt4((gestX * gestX) + (gestY * gestY));

        if ((dontdrawthemap && (Pos.y < Y_BottomBar)) || ((MapSpaceMode == MSM_MAP))) {

            // GESTURE DETECTION
            // if gestX >0 gesture from right to left , gestX <0 gesture from left to right
            // if gestY >0 gesture from down to up ,    gestY <0 gesture from up to down
            if (gestDist < GestureSize) { // TODO FIX tune this GestureSize, maybe 100?
                gestDir = LKGESTURE_NONE;
            } else {
                // horizontal includes also perfectly diagonal gestures
                if (abs(gestX) >= abs(gestY)) {
                    // we use LKGESTURE definition, but they have nothing to do with those used in other part of source code
                    if (gestX > 0)
                        gestDir = IphoneGestures ? LKGESTURE_RIGHT : LKGESTURE_LEFT;
                    else
                        gestDir = IphoneGestures ? LKGESTURE_LEFT : LKGESTURE_RIGHT;
                } else {
                    if (IsMultiMapNoMain()) {
                        if (gestY > 0)
                            gestDir = LKGESTURE_UP;
                        else
                            gestDir = LKGESTURE_DOWN;
                    } else {
                        if (gestY > 0)
                            gestDir = IphoneGestures ? LKGESTURE_DOWN : LKGESTURE_UP;
                        else
                            gestDir = IphoneGestures ? LKGESTURE_UP : LKGESTURE_DOWN;
                    }
                }
            }
            // end dontdrawthemap and inside mapscreen looking for a gesture
        }

        if (mode.Is(Mode::MODE_TARGET_PAN)) {
            if (UseAATTarget()) {
                _Proj.Screen2LonLat(Pos, Xlat, Ylat);
                LockTaskData();
                targetMoved = true;
                targetMovedLat = Ylat;
                targetMovedLon = Xlat;
                UnlockTaskData();
            }
            // else we are in simple TARGET dialog, and we must NOT process anything
            return;
        }

        short topicon;
        if (DrawBottom)
            topicon = P_MenuIcon_DrawBottom.y;
        else
            topicon = P_MenuIcon_noDrawBottom.y;

        if ((Pos.x > P_MenuIcon_DrawBottom.x) && (Pos.y > topicon)) {

            // short click on aircraft icon
            //
            if (dwInterval <= DOUBLECLICKINTERVAL) {

                ShowMenu();
                return;
            } else {
                // Long click on aircraft icon, toggle thermal mode
                //
                if (dwInterval >= VKLONGCLICK) { // in Defines.h
#if 0
                    if (mode.Is(Mode::MODE_CIRCLING)) {
                        mode.UserForcedMode(Mode::MODE_FLY_CRUISE);

                        PlayResource(TEXT("IDR_WAV_CLICK"));

                        break;
                    } else if (mode.Is(Mode::MODE_CRUISE)) {
                        mode.UserForcedMode(Mode::MODE_FLY_NONE);

                        PlayResource(TEXT("IDR_WAV_CLICK"));

                        break;
                    }
#endif
                } else {
                    // We are here in any case only when dwInterval is <VKLONGCLICK
                    if (dwInterval >= CustomKeyTime) {
                        if (!CustomKeyHandler(CKI_BOTTOMICON)) {
                            ShowMenu();
                        }
                        #ifdef TRACEKEY
                        else StartupStore(_T(".... CustomKeyHandler is CKI_BOTTOMICON%s"),NEWLINE);
                        #endif
                    }
                    #ifdef TRACEKEY
                    else StartupStore(_T(".... dwInterval<VKLONGCLICK%s"),NEWLINE);
                    #endif
                    return;
                }
            }

            // end aircraft icon check
        }

        // MultiMap custom specials, we use same geometry of MSM_MAP

        if ((dwInterval < AIRSPACECLICK) || ISPARAGLIDER) {
            if (NOTANYPAN && IsMultiMapCustom()) {
                if ((Pos.x <= P_UngestureLeft.x) && (Pos.y <= P_UngestureLeft.y)) {

                    PlayResource(TEXT("IDR_WAV_CLICK"));

                    LKevent = LKEVENT_TOPLEFT;
                    MapWindow::RefreshMap();
                    return;
                }
                if ((Pos.x > P_UngestureRight.x) && (Pos.y <= P_UngestureRight.y)) {

                    PlayResource(TEXT("IDR_WAV_CLICK"));

                    LKevent = LKEVENT_TOPRIGHT;
                    MapWindow::RefreshMap();
                    return;
                }
            }

            // Otherwise not in multimap, proceed with normal checks
            if (NOTANYPAN && IsMultiMapShared()) {
                if ((Pos.x <= P_UngestureLeft.x) && (Pos.y <= P_UngestureLeft.y)) {

                    if (!CustomKeyHandler(CKI_TOPLEFT)) {
                        // we click in any case to let the user have a response feeling
                        PlayResource(TEXT("IDR_WAV_CLICK"));
                        MapWindow::zoom.EventScaleZoom(1);
                        return;
                    }
                    #ifdef TRACEKEY
                    else StartupStore(_T(".... CustomKeyHandler is CKI TOPLEFT%s"),NEWLINE);
                    #endif
                    MapWindow::RefreshMap();
                    return;
                }
            }
        }
        if (NOTANYPAN && IsMultiMapShared()) {
            if (ISPARAGLIDER) {
                // Use the compass to pullup UTM informations to paragliders
                if ((Pos.x > P_UngestureRight.x) && (Pos.y <= P_UngestureRight.y)) {

                    if (dwInterval >= DOUBLECLICKINTERVAL) {

                        // if we are running a real task, with gates, and we could still start
                        // if only 1 time gate, and we passed valid start, no reason to resettask
                        int acceptreset = 2;
                        if (PGNumberOfGates == 1) acceptreset = 1;

                        if (UseGates() && ValidTaskPoint(1) && ActiveTaskPoint < acceptreset) { // 100507 101110
                            //
                            // Reset task
                            //
                            InputEvents::eventResetTask(_T(""));
                        } else {
                            //
                            // Report UTM position
                            //
                            InputEvents::eventService(_T("UTMPOS"));
                        }
                        return;
                    }

                } // End compass icon check
            } // PARAGLIDERs special buttons
            // else not a paraglider key, process it for gliders
            // else {
            // change in 2.3q: we let paragliders use the CK as well
            {
                if ((Pos.x > P_UngestureRight.x) && (Pos.y <= P_UngestureRight.y)) {
                    if (!CustomKeyHandler(CKI_TOPRIGHT)) {
                        // we click in any case to let the user have a response feeling
                        PlayResource(TEXT("IDR_WAV_CLICK"));
                        MapWindow::zoom.EventScaleZoom(1);
                        return;
                    }
                    #ifdef TRACEKEY
                    else StartupStore(_T(".... CustomKeyHandler is CKI TOPRIGHT%s"),NEWLINE);
                    #endif
                    MapWindow::RefreshMap();
                    return;
                }
                // if not topright, continue
            }
            // do all of this only if !dontdrawthemap = we are in fullscreen and not PAN etc.
            // indentation is wrong here
        }


        // "fast virtual keys" are handled locally and not passed to event handler.
        // they are processed even when virtual keys are disabled, because they concern special lk8000 menus.

        // First case: for mapspacemodes we manage gestures as well
        if (dontdrawthemap || (NOTANYPAN && IsMultiMapShared())) {

            if (gestDir == LKGESTURE_LEFT) {
                ProcessVirtualKey(Pos.x, Pos.y, dwInterval, LKGESTURE_LEFT);
                return;
            }
            if (gestDir == LKGESTURE_RIGHT) {
                ProcessVirtualKey(Pos.x, Pos.y, dwInterval, LKGESTURE_RIGHT);
                return;
            }

            // basically we do this only to process left/right gestures in main map as well
            // In SIM mode, gestDir are not detected because we manage simulation movements
            if (MapSpaceMode != MSM_MAP) {
                if (gestDir == LKGESTURE_UP) {
                    ProcessVirtualKey(Pos.x, Pos.y, dwInterval, LKGESTURE_UP);
                    return;
                }
                if (gestDir == LKGESTURE_DOWN) {
                    ProcessVirtualKey(Pos.x, Pos.y, dwInterval, LKGESTURE_DOWN);
                    return;
                }

#if 0 // NO MORE NEEDED BUT PLEASE DO NOT REMOVE
                // We are here when lk8000, and NO moving map displayed: virtual enter, virtual up/down, or
                // navbox operations including center key.
                if (dwInterval == 0) {
                   #ifdef TRACEKEY
                   StartupStore(_T("... Debounced in dontdrawthemap or NOTANYPAN dwInterval==0%s"),NEWLINE);
                   #endif
                   return; // should be impossible. It happens because of strange event buttonup/down after dblck
                }
#endif
                int wParam = ProcessVirtualKey(Pos.x, Pos.y, dwInterval, LKGESTURE_NONE);
#ifdef DEBUG_MAPINPUT
                DoStatusMessage(_T("DBG-035 navboxes"));
#endif
                // we could use a single ProcessVirtual for all above, and check that wParam on return
                // is really correct for gestures as well... since we do not want to go to wirth with gestures!
                if (wParam != 0) {
                    DoStatusMessage(_T("ERR-033 Invalid Virtual Key"));
                }
                return;
            }
        }

        // if clicking on navboxes, process fast virtual keys
        // maybe check LK8000 active?
        // This point is selected when in MapSpaceMode==MSM_MAP, i.e. lk8000 with moving map on.
        if (DrawBottom && (Pos.y >= Y_BottomBar) && !mode.AnyPan()) {
            int wParam = ProcessVirtualKey(Pos.x, Pos.y, dwInterval, LKGESTURE_NONE);

            if (wParam != 0) {
                DoStatusMessage(_T("ERR-034 Invalid Virtual Key"));
            }
            return;
        }


#if 0 // NO MORE NEEDED BUT PLEASE DO NOT REMOVE
        if (dwInterval == 0) {
            #ifdef TRACEKEY
            StartupStore(_T("... Debounced, dwInterval == 0 !%s"),NEWLINE);
            #endif
            return; // should be impossible. It happens because of strange event buttonup/down after dblck
        }
#endif

        // Handling double click passthrough
        // Caution, timed clicks from PC with a mouse are different from real touchscreen devices

        // Process faster clicks here and no precision, but let DBLCLK pass through
        // VK are used in the bottom line in this case, forced on for this situation.
        if (DrawBottom && (Pos.y >= Y_BottomBar)) {
            // DoStatusMessage(_T("Click on hidden map ignored"));

            // do not process virtual key if it is timed as a DBLCLK
            // we want users to get used to double clicking only on infoboxes
            // and avoid triggering unwanted waypoints details
            if (dwInterval >= FASTDOUBLECLICK) { // fast dblclk required here.

                ProcessVirtualKey(Pos.x, Pos.y, dwInterval, LKGESTURE_NONE);

                return;
            }
            // do not process click on the underneath window
            #ifdef TRACEKEY
            StartupStore(_T("... ButtonUp ignored, underneath window%s"),NEWLINE);
            #endif
            return;
        }
        if (dontdrawthemap) {
            #ifdef TRACEKEY
            StartupStore(_T("... ButtonUp ignored, dontdrawthemap%s"),NEWLINE);
            #endif
            return;
        }

        _Proj.Screen2LonLat(Pos, Xlat, Ylat);
        if (SIMMODE && NOTANYPAN && (gestDist > NIBLSCALE(36))) {
            // This drag moves the aircraft (changes speed and direction)
            //
            // Notice: Parser is not active because there is no real gps. We can use GPS_INFO.
            //
            double newbearing;
            double oldbearing = DrawInfo.TrackBearing;
            double minspeed = 1.1 * GlidePolar::Vminsink();
            DistanceBearing(Ystart, Xstart, Ylat, Xlat, NULL, &newbearing);
            if ((fabs(AngleLimit180(newbearing - oldbearing)) < 30) || (DrawInfo.Speed < minspeed)) {
                // sink we shall be sinking, lets raise the altitude when using old simulator interface
                if ((DerivedDrawInfo.TerrainValid) && (DerivedDrawInfo.AltitudeAGL < 0))
                    GPS_INFO.Altitude = DerivedDrawInfo.TerrainAlt;
                GPS_INFO.Altitude += 200;
                if (ISPARAGLIDER)
                    GPS_INFO.Speed = min(16.0, max<double>(minspeed, gestDist / NIBLSCALE(9)));
                else
                    GPS_INFO.Speed = min(100.0, max<double>(minspeed, gestDist / NIBLSCALE(3)));
            }
            GPS_INFO.TrackBearing = (int) newbearing;
            if (GPS_INFO.TrackBearing == 360) GPS_INFO.TrackBearing = 0;
            TriggerGPSUpdate();

            return;
        }

        if (!mode.Is(Mode::MODE_TARGET_PAN)) {

            //
            // Finally process normally a click on the moving map.
            //
            if (dwInterval < AIRSPACECLICK) { // original and untouched interval
                {
                    if (!mode.AnyPan() && (UseUngestures || !ISPARAGLIDER)) {
                        if (Pos.x <= X_Left) {
                            PreviousModeType();
                            MapWindow::RefreshMap();
                            return;
                        }
                        if (Pos.x >= X_Right) {
                            NextModeType();
                            MapWindow::RefreshMap();
                            return;
                        }
                    }

                    if (Pos.y < Y_Up) {
                        MapWindow::zoom.EventScaleZoom(1);
                        PlayResource(TEXT("IDR_WAV_CLICK"));
                    } else {
                        if (Pos.y > Y_Down) {
                            MapWindow::zoom.EventScaleZoom(-1);
                            PlayResource(TEXT("IDR_WAV_CLICK"));
                        } else {
                            // process center key, do nothing
                            // v5 obsoleted, should not happen
                            #ifdef TRACEKEY
                            StartupStore(_T(".... ignored process center key%s"),NEWLINE);
                            #endif
                            return;
                        }
                    }
                    tsDownTime.Reset();
                    return;
                }
            } else {

                // Event_NearestWaypointDetails , really. Before multiselect we were using the long click
                // to select airspaces, hence the name.
                // 120526 moved out of anypan, buggy because we want airspace selection with priority
                if (/*IsMultimapAirspace() &&*/ Event_InteriorAirspaceDetails(Xstart, Ystart))
                    return;

                if (!mode.AnyPan()) {
                    // match only center screen
                    #ifdef TRACEKEY
                    StartupStore(_T(".... not mode anyPan! ignored%s"),NEWLINE);
                    #endif
                }
                #ifdef TRACEKEY
                else StartupStore(_T("... ButtonUp ignored, long click in mode AnyPan%s"),NEWLINE);
                #endif
                return;
            }
        } // !TargetPan
    }
    #ifdef TRACEKEY
    else StartupStore(_T(".... OnButtonUp ignored! LockModeStatus=%d pressed=%d%s"),LockModeStatus,pressed,NEWLINE);
    #endif
}

extern void SimFastForward(void);

// return true if Shift key is down
static bool GetShiftKeyState() {

#ifdef ENABLE_SDL
    const Uint8 *keystate = ::SDL_GetKeyboardState(NULL);
    return keystate[SDL_SCANCODE_LSHIFT] || keystate[SDL_SCANCODE_RSHIFT];
#elif defined(WIN32)
    short nVirtKey = GetKeyState(VK_SHIFT);
    return (nVirtKey & 0x8000);
#else
    return false;
#endif
}



void MapWindow::_OnKeyDown(unsigned KeyCode) {
    //
    // Special SIM mode keys for PC
    //
    if(HasKeyboard() && SIMMODE && IsMultiMapShared()) {
        if (!ReplayLogger::IsEnabled()) {
            switch (KeyCode) {
                case KEY_PRIOR: // VK_PRIOR PAGE UP
                    if (GetShiftKeyState()) {
                        if (Units::GetUserAltitudeUnit() == unFeet)
                            GPS_INFO.Altitude += 45.71999999 * 10;
                        else
                            GPS_INFO.Altitude += 10 * 10;
                    } else {
                        if (Units::GetUserAltitudeUnit() == unFeet)
                            GPS_INFO.Altitude += 45.71999999;
                        else
                            GPS_INFO.Altitude += 10;
                    }
                    TriggerGPSUpdate();
                return;
                case KEY_NEXT: // VK_NEXT PAGE DOWN
                    if (GetShiftKeyState()) {
                        if (Units::GetUserAltitudeUnit() == unFeet)
                            GPS_INFO.Altitude -= 45.71999999 * 10;
                        else
                            GPS_INFO.Altitude -= 10 * 10;
                    } else {
                        if (Units::GetUserAltitudeUnit() == unFeet)
                            GPS_INFO.Altitude -= 45.71999999;
                        else
                            GPS_INFO.Altitude -= 10;
                    }
                    if (GPS_INFO.Altitude <= 0) {
                        GPS_INFO.Altitude = 0;
                    }
                    TriggerGPSUpdate();
                    return;
                case KEY_UP: // VK_UP
                    if (GetShiftKeyState()) {
                        SimFastForward();
                    } else {
                        InputEvents::eventChangeGS(_T("kup"));
                    }
                    TriggerGPSUpdate();
                    return;
                case KEY_DOWN: // VK_DOWN
                    InputEvents::eventChangeGS(_T("kdown"));
                    TriggerGPSUpdate();
                    return;
                case KEY_LEFT: // VK_LEFT
                    if (GetShiftKeyState()) {
                        GPS_INFO.TrackBearing -= 0.1;

                    } else {
                        GPS_INFO.TrackBearing -= 5;
                    }
                    if (GPS_INFO.TrackBearing < 0) GPS_INFO.TrackBearing += 360;
                    else if (GPS_INFO.TrackBearing > 359) GPS_INFO.TrackBearing -= 360;

                    TriggerGPSUpdate();
                    return;
                case KEY_RIGHT: // VK_RIGHT
                    if (GetShiftKeyState()) {
                        GPS_INFO.TrackBearing += 0.1;
                    } else {
                        GPS_INFO.TrackBearing += 5;
                    }
                    GPS_INFO.TrackBearing = AngleLimit360(GPS_INFO.TrackBearing);

                    TriggerGPSUpdate();
                    return;
            }
        } else {
            // ReplayLogger::IsEnabled()
            extern double ReplayTime;
            switch (KeyCode) {
                case KEY_PRIOR: // VK_PRIOR PAGE UP
                    ReplayTime += 300;
                    return;
                case KEY_UP: // VK_UP
                    ReplayLogger::TimeScale++;
                    return;
                case KEY_DOWN: // VK_DOWN
                    if (ReplayLogger::TimeScale > 0) ReplayLogger::TimeScale--;
                    if (ReplayLogger::TimeScale < 0) ReplayLogger::TimeScale = 0; // to be safe
                    return;
                case KEY_RIGHT: // VK_RIGHT
                    ReplayTime += 60;
                    return;
            }
        }
    }

#ifndef LXMINIMAP
    //
    // LX MINIMAP II HARDWARE KEYS
    // From left to right, top mode, we find:
    //   NAME     LABEL
    //    A		rotary knob "set"
    //    B		button Menu
    //    C		button Zoom ent
    //    D		button Zoom sel
    //    E		rotary knob Esc
    //
    if (GlobalModelType == MODELTYPE_PNA_MINIMAP) {

      switch (KeyCode) {

      case KEY_APP1: // L : NAV
      case KEY_APP2: // N : TSK/TRG
      case KEY_APP3: // C : SET/SYS
      case KEY_APP4: // P : INFO
      case KEY_F1: // F : AN/CLC (long press)
      case KEY_F2: // H : START/R (long press)
      case KEY_F3: // R : INFO (long press)
      case KEY_F4: // M : NAV (long press)
      case KEY_F5: // E : AN/CLC
      case KEY_F6: // G : START/R
      case KEY_F7: // O : TSK/TRG (long press)
      case KEY_F8: // I : SET/SYS (long press)
        break;

      case 'D':
      case KEY_ESCAPE:
        // Button B is generating alternate codes 68 and 27
        // we use both as a single one
        PlayResource(TEXT("IDR_WAV_CLICK"));
        // Toggle Menu
        if (ButtonLabel::IsVisible()) {
          InputEvents::setMode(_T("default"));
          return;
        }
        if (!CustomKeyHandler(CKI_BOTTOMLEFT)) {
          InputEvents::setMode(_T("Menu"));
          return;
        }
        break;

      case KEY_RETURN:
        // Button C is generating a RETURN
        if (ButtonLabel::IsVisible()) {
          InputEvents::triggerSelectedButton();
          return;
        }
        if (CustomKeyHandler(CKI_BOTTOMCENTER)) {
          PlayResource(TEXT("IDR_WAV_CLICK"));
          return;
        }
        break;

      case KEY_MENU: // press rotary knop (left buttom)
        if (CustomKeyHandler(CKI_BOTTOMRIGHT)) {
          PlayResource(TEXT("IDR_WAV_CLICK"));
          return;
        }
        break;

      case KEY_UP:
        // Rotary knob A is generating a 38 (turn left) and 40 (turn right)
        // v5> careful not the same as _key gotos.
        if (ButtonLabel::IsVisible()) {
          PlayResource(TEXT("IDR_WAV_CLICK"));
          InputEvents::selectPrevButton();
        } else if (!MapWindow::mode.AnyPan() && MapSpaceMode != 1) {
          // dontdrawthemap
          if (MapSpaceMode > MSM_MAP) {

            PlayResource(TEXT("IDR_WAV_CLICK"));

            LKevent = LKEVENT_UP;
            MapWindow::RefreshMap();
          }
        } else {
          MapWindow::zoom.EventScaleZoom(-1);
        }
        return;

      case KEY_DOWN:
        if (ButtonLabel::IsVisible()) {
          PlayResource(TEXT("IDR_WAV_CLICK"));
          InputEvents::selectNextButton();
        } else if (!MapWindow::mode.AnyPan() && MapSpaceMode != 1) {
          // dontdrawthemap
          if (MapSpaceMode > MSM_MAP) {
            PlayResource(TEXT("IDR_WAV_CLICK"));
            LKevent = LKEVENT_DOWN;
            MapWindow::RefreshMap();
          }
        } else {
          MapWindow::zoom.EventScaleZoom(1);
        }
        return;

      case KEY_LEFT:
        // Rotary knob E is generating a 37 (turn left) and 39 (turn right)
        key_previous_mode();
        return;

      case KEY_RIGHT:
        key_next_mode();
        return;

      default:
        break;
      }
    }
#endif // not LXMINIMAP
    //
    // This is the handler for bluetooth keyboards
    //
    // BTK1 is representing the screen in portrait mode. Space is on the right.
    // Keyboard type 1.
    //
    if (GlobalModelType == MODELTYPE_PNA_GENERIC_BTK1) {
        #ifndef __linux__
        #if (WINDOWSPC<1)
        if (!Debounce(KEYDEBOUNCE)) return;
        #endif
        #endif
        switch (KeyCode) {
                //
                // THE BOTTOM BAR
                //
            case '1':
            case '2':
                key_bottombar_previous();
                return;

            case 'a':
            case 'A':
            case 's':
            case 'S':
            case 'q':
            case 'Q':
            case 'W':
            case 'w':
                key_next_mode();
                return;

            case 'z':
            case 'Z':
            case 'x':
            case 'X':
                key_bottombar_next();
                return;

                //
                // THE LEFT SCREEN
                //
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                key_previous_page();
                return;

                //
                // THE RIGHT SCREEN
                //
            case 0x20: // space
            case 172:
            case 191:
                key_next_page();
                return;


                //
                // THE CENTER-UP
                //
            case 'I':
            case 'i':
            case 'K':
            case 'k':
            case 188:

            case 'M':
            case 'm':
            case 'J':
            case 'j':
            case 'U':
            case 'u':
            case 'N':
            case 'n':
            case 'H':
            case 'h':
            case 'Y':
            case 'y':
                key_up();
                return;

                //
                // THE CENTER-DOWN
                //
            case 'T':
            case 't':
            case 'G':
            case 'g':
            case 'B':
            case 'b':
            case 'R':
            case 'r':
            case 'F':
            case 'f':
            case 'V':
            case 'v':
            case 'E':
            case 'e':
            case 'D':
            case 'd':
            case 'C':
            case 'c':
                key_down();
                return;

                //
                // THE TOP-LEFT
                //
            case '0':
            case '9':
            case 'P':
            case 'p':
            case 'O':
            case 'o':
                key_topleft();
                return;

                //
                // THE TOP-RIGHT
                //
            case 13: // enter
            case 8: // BS! not the number
            case 'L':
            case 'l':
            case 190:
            case 186:
                key_topright();
                return;

                //
                // the special Shift corner keys
                //
            case 16:
                key_enter();
                return;

            default:
                break;
        }
    }

    //
    // BTK2 is representing the screen in portrait mode. Space is on the right.
    // Keyboard type 2.
    //
    if (GlobalModelType == MODELTYPE_PNA_GENERIC_BTK2) {
        #ifndef __linux__
        #if (WINDOWSPC<1)
        if (!Debounce(KEYDEBOUNCE)) return;
        #endif
        #endif
        switch (KeyCode) {
                //
                // THE BOTTOM BAR
                //
            case '1':
            case '2':
                key_bottombar_previous();
                return;

            case 'a':
            case 'A':
            case 's':
            case 'S':
            case 'q':
            case 'Q':
            case 'W':
            case 'w':
                key_next_mode();
                return;

            case 'z':
            case 'Z':
            case 'x':
            case 'X':
                key_bottombar_next();
                return;

                //
                // THE LEFT SCREEN
                //
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                key_previous_page();
                return;

                //
                // THE RIGHT SCREEN
                //
            case 0x20: // space
                key_next_page();
                return;


                //
                // THE CENTER-UP
                //
            case 'I':
            case 'i':
            case 'K':
            case 'k':
            case 188:

            case 'M':
            case 'm':
            case 'J':
            case 'j':
            case 'U':
            case 'u':
            case 'N':
            case 'n':
            case 'H':
            case 'h':
            case 'Y':
            case 'y':
                key_up();
                return;

                //
                // THE CENTER-DOWN
                //
            case 'T':
            case 't':
            case 'G':
            case 'g':
            case 'B':
            case 'b':
            case 'R':
            case 'r':
            case 'F':
            case 'f':
            case 'V':
            case 'v':
            case 'E':
            case 'e':
            case 'D':
            case 'd':
            case 'C':
            case 'c':
                key_down();
                return;

                //
                // THE TOP-LEFT
                //
            case '0':
            case '9':
            case 'P':
            case 'p':
            case 'O':
            case 'o':
                key_topleft();
                return;

                //
                // THE TOP-RIGHT
                //
            case 13: // enter
            case 8: // Del
            case 'L':
            case 'l':
            case 190:
            case 186:
                key_topright();
                return;

                //
                // the special Shift and Ctrol
                //
            case 0x10:
            case 0x11:
                key_enter();
                return;

            default:
                break;
        }
    }
    //
    // BTK3 is representing the screen in portrait mode. Space is on the right.
    // Keyboard type 3.
    //
    if (GlobalModelType == MODELTYPE_PNA_GENERIC_BTK3) {
        #ifndef __linux__
        #if (WINDOWSPC<1)
        if (!Debounce(KEYDEBOUNCE)) return;
        #endif
        #endif
        switch (KeyCode) {
                //
                // THE BOTTOM BAR
                //
            case '1':
            case '2':
                key_bottombar_previous();
                return;

            case 'a':
            case 'A':
            case 's':
            case 'S':
            case 'q':
            case 'Q':
            case 'W':
            case 'w':
                key_next_mode();
                return;

            case 'z':
            case 'Z':
            case 'x':
            case 'X':
                key_bottombar_next();
                return;

                //
                // THE LEFT SCREEN
                //
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                key_previous_page();
                return;

                //
                // THE RIGHT SCREEN
                //
            case 0x20: // space
            case 0xbc: // 188
            case 0xbe: // 190
                key_next_page();
                return;


                //
                // THE CENTER-UP
                //
            case 'I':
            case 'i':
            case 'K':
            case 'k':
            case 0xba: // 186

            case 'M':
            case 'm':
            case 'J':
            case 'j':
            case 'U':
            case 'u':
            case 'N':
            case 'n':
            case 'H':
            case 'h':
            case 'Y':
            case 'y':
                key_up();
                return;

                //
                // THE CENTER-DOWN
                //
            case 'T':
            case 't':
            case 'G':
            case 'g':
            case 'B':
            case 'b':
            case 'R':
            case 'r':
            case 'F':
            case 'f':
            case 'V':
            case 'v':
            case 'E':
            case 'e':
            case 'D':
            case 'd':
            case 'C':
            case 'c':
                key_down();
                return;

                //
                // THE TOP-LEFT
                //
            case '0':
            case '9':
            case 'P':
            case 'p':
            case 'O':
            case 'o':
                key_topleft();
                return;

                //
                // THE TOP-RIGHT
                //
            case 13: // enter
            case 8: // Del
            case 'L':
            case 'l':
            case 0xde: // 222
                key_topright();
                return;

                //
                // the special Shift and Ctrol
                //
            case 0x10:
            case 0x11:
                key_enter();
                return;

            default:
                break;
        }
    }

    //
    // BTKA is for using 1 2 3 4 5 6 7 8 9 0  assigned to customkeys of customenu.
    // universal Keyboard type
    //
    if (GlobalModelType == MODELTYPE_PNA_GENERIC_BTKA) {
        #ifndef __linux__
        #if (WINDOWSPC<1)
        if (!Debounce(KEYDEBOUNCE)) return;
        #endif
        #endif
        switch (KeyCode) {
            case '1':
                CustomKeyHandler(CustomMenu1 + 1000);
                return;
            case '2':
                CustomKeyHandler(CustomMenu2 + 1000);
                return;
            case '3':
                CustomKeyHandler(CustomMenu3 + 1000);
                return;
            case '4':
                CustomKeyHandler(CustomMenu4 + 1000);
                return;
            case '5':
                CustomKeyHandler(CustomMenu5 + 1000);
                return;
            case '6':
                CustomKeyHandler(CustomMenu6 + 1000);
                break;
            case '7':
                CustomKeyHandler(CustomMenu7 + 1000);
                return;
            case '8':
                CustomKeyHandler(CustomMenu8 + 1000);
                return;
            case '9':
                CustomKeyHandler(CustomMenu9 + 1000);
                return;
            case '0':
                CustomKeyHandler(CustomMenu10 + 1000);
                return;
            default:
                break;
        }
    }

    //
    // BTKB is for using 1 2 3 4 5 6 7 8 9 0  assigned to customkeys of customenu.
    // universal Keyboard type
    //
    if (GlobalModelType == MODELTYPE_PNA_GENERIC_BTKB) {
        #ifndef __linux__
        #if (WINDOWSPC<1)
        if (!Debounce(KEYDEBOUNCE)) return;
        #endif
        #endif
        switch (KeyCode) {
            case '1':
            case '2':
            case 'Q':
            case 'W':
            case 'q':
            case 'w':
                CustomKeyHandler(CustomMenu1 + 1000);
                return;
            case '3':
            case '4':
            case 'e':
            case 'E':
            case 'r':
            case 'R':
                CustomKeyHandler(CustomMenu2 + 1000);
                return;
            case '5':
            case '6':
            case 't':
            case 'T':
            case 'Y':
            case 'y':
                CustomKeyHandler(CustomMenu3 + 1000);
                return;
            case '7':
            case '8':
            case 'u':
            case 'i':
            case 'U':
            case 'I':
                CustomKeyHandler(CustomMenu4 + 1000);
                return;
            case '9':
            case '0':
            case 'O':
            case 'o':
            case 'P':
            case 'p':
                CustomKeyHandler(CustomMenu5 + 1000);
                return;
            case 'A':
            case 'Z':
            case 'S':
            case 'X':
            case 'a':
            case 'z':
            case 's':
            case 'x':
                CustomKeyHandler(CustomMenu6 + 1000);
                return;
            case 'D':
            case 'F':
            case 'C':
            case 'V':
            case 'd':
            case 'f':
            case 'c':
            case 'v':
                CustomKeyHandler(CustomMenu7 + 1000);
                return;
            case 'G':
            case 'H':
            case 'B':
            case 'N':
            case 'g':
            case 'h':
            case 'b':
            case 'n':
                CustomKeyHandler(CustomMenu8 + 1000);
                return;
            case 'J':
            case 'j':
            case 'K':
            case 'k':
            case 'M':
            case 'm':
            case 188:
                CustomKeyHandler(CustomMenu9 + 1000);
                return;
            case 13: // enter
            case 8: // Del
            case 'L':
            case 'l':
            case 190:
                CustomKeyHandler(CustomMenu10 + 1000);
                return;
            default:
                break;
        }
    }

    if(HasKeyboard()) {
      PlayResource(TEXT("IDR_WAV_CLICK"));
      // default keyboard usage
      switch (KeyCode) {
      case KEY_F1:
        // Toggle Menu
        InputEvents::setMode(ButtonLabel::IsVisible() ? _T("default") : _T("Menu")); 
        MenuTimeOut = 0;
        return;
      case KEY_ESCAPE:
        if (ButtonLabel::IsVisible()) {
          // Close Menu
          InputEvents::setMode(_T("default"));
          return;
        }
        // TODO : return to default moving map ?
        break;
      case KEY_RETURN:
        if (ButtonLabel::IsVisible()) {
          InputEvents::triggerSelectedButton();
          MenuTimeOut = 0;
          return;
        }
        break;
      case KEY_UP:
      case KEY_RIGHT:
          if (ButtonLabel::IsVisible()) {
            InputEvents::selectPrevButton();
            MenuTimeOut = 0;
            return;
          }
          // TODO :
          break;
      case KEY_DOWN:
      case KEY_LEFT:
          if (ButtonLabel::IsVisible()) {
            InputEvents::selectNextButton();
            MenuTimeOut = 0;
            return;
          }
          // TODO :
          break;
       // always mapped to zoom
      default:
          break;
      } 
    }

    InputEvents::processKey(KeyCode);
}

static bool isListPage(void) {
    return ( MapSpaceMode == MSM_COMMON
            || MapSpaceMode == MSM_RECENT
            || MapSpaceMode == MSM_LANDABLE
            || MapSpaceMode == MSM_AIRPORTS
            || MapSpaceMode == MSM_NEARTPS
            || MapSpaceMode == MSM_AIRSPACES
            || MapSpaceMode == MSM_THERMALS
            || MapSpaceMode == MSM_TRAFFIC);
}

void MapWindow::key_bottombar_previous() {
    BottomBarChange(false); // prev bb
    BottomSounds();
    MapWindow::RefreshMap();
}

void MapWindow::key_bottombar_next() {
    BottomBarChange(true); // next bb
    BottomSounds();
    MapWindow::RefreshMap();
}

void MapWindow::key_overtarget_rotate() {
    RotateOvertarget();
    MapWindow::RefreshMap();
}

void MapWindow::key_topcenter() {
    if (isListPage())
        key_gesture_down();
    else
        key_topright();
}

void MapWindow::key_topleft() {
    if (NOTANYPAN && IsMultiMapCustom()) {
        LKevent = LKEVENT_TOPLEFT;
        MapWindow::RefreshMap();
    } else {
        if (NOTANYPAN && IsMultiMapShared()) {
            CustomKeyHandler(CKI_TOPLEFT);
        }
        if (isListPage()) {
            key_gesture_down();
        }
    }
}

void MapWindow::key_topright() {
    if (NOTANYPAN && IsMultiMapCustom()) {
        LKevent = LKEVENT_TOPRIGHT;
        MapWindow::RefreshMap();
    } else {
        if (NOTANYPAN && IsMultiMapShared()) {
            CustomKeyHandler(CKI_TOPRIGHT);
        }
        if (isListPage()) {
            key_gesture_down();
        }
    }
}

void MapWindow::key_enter() {
    if (!MapWindow::mode.AnyPan() && MapSpaceMode != 1) { // dontdrawthemap
        if (MapSpaceMode > MSM_MAP) {
            PlayResource(TEXT("IDR_WAV_CLICK"));

            LKevent = LKEVENT_ENTER;
            MapWindow::RefreshMap();
        }
    }
#if 0 // no center screen ck anymore
    else {
        if (CustomKeyHandler(CKI_CENTERSCREEN)) {
            PlayResource(TEXT("IDR_WAV_CLICK"));
        }
        // MapWindow::RefreshMap();
    }
#endif
}

void MapWindow::key_gesture_down() {
    ProcessVirtualKey(0, 0, 0, LKGESTURE_DOWN);
    MapWindow::RefreshMap();
}

void MapWindow::key_gesture_up() {
    ProcessVirtualKey(0, 0, 0, LKGESTURE_UP);
    MapWindow::RefreshMap();
}

void MapWindow::key_previous_page() {
    PreviousModeType();
    MapWindow::RefreshMap();
    if (ModeIndex != LKMODE_MAP) {
        PlayResource(TEXT("IDR_WAV_CLICK"));
    }
}

void MapWindow::key_next_page() {
    NextModeType();
    MapWindow::RefreshMap();
    if (ModeIndex != LKMODE_MAP) {
        PlayResource(TEXT("IDR_WAV_CLICK"));
    }
}

void MapWindow::key_down() {
#ifdef TRACEKEY
    StartupStore(_T("KEY DOWN "));
#endif

    if (!MapWindow::mode.AnyPan() && MapSpaceMode != 1) { // dontdrawthemap
        if (MapSpaceMode <= MSM_MAP) {

        } else {
            PlayResource(TEXT("IDR_WAV_CLICK"));

            LKevent = LKEVENT_DOWN;
            MapWindow::RefreshMap();
        }
    } else {
        // careful! zoom works inverted!!
        PlayResource(TEXT("IDR_WAV_CLICK"));

        MapWindow::zoom.EventScaleZoom(-1);
    }
}

void MapWindow::key_up() {
#ifdef TRACEKEY
    StartupStore(_T("KEY UP"));
#endif
    //
    // UP
    //
    if (!MapWindow::mode.AnyPan() && MapSpaceMode != 1) { // dontdrawthemap
        if (MapSpaceMode <= MSM_MAP) {
            ;
        } else {
            PlayResource(TEXT("IDR_WAV_CLICK"));
            LKevent = LKEVENT_UP;
            MapWindow::RefreshMap();
        }
    } else {
        // careful! zoom works inverted!!
        PlayResource(TEXT("IDR_WAV_CLICK"));
        MapWindow::zoom.EventScaleZoom(1);
    }
}

void MapWindow::key_previous_mode() {
    PreviousModeIndex();
    MapWindow::RefreshMap();
    SoundModeIndex();
}

void MapWindow::key_next_mode() {
    NextModeIndex();
    MapWindow::RefreshMap();
    SoundModeIndex();
}

#ifdef ENABLE_OPENGL
void MapWindow::Render(LKSurface& Surface, const PixelRect& Rect ) {
    UpdateTimeStats(true);

    if (ProgramStarted==psInitDone) {
        ProgramStarted = psFirstDrawDone;
    }

    if(ProgramStarted >= psNormalOp && THREADRUNNING) {
        UpdateInfo(&GPS_INFO, &CALCULATED_INFO);
        RenderMapWindow(Surface, Rect);

        const ScreenProjection _Proj;

        // Draw cross sight for pan mode, in the screen center,
        if (mode.AnyPan() && !mode.Is(Mode::MODE_TARGET_PAN)) {
            const RasterPoint centerscreen = { ScreenSizeX/2, ScreenSizeY/2 };
            DrawMapScale(Surface, Rect, _Proj);
            DrawCompass(Surface, Rect, GetDisplayAngle());
            DrawCrossHairs(Surface, centerscreen, Rect);
        }

        MapDirty = false;

        // we do caching after screen update, to minimise perceived delay
        // UpdateCaches is updating topology bounds when either forced (only here)
        // or because MapWindow::ForceVisibilityScan  is set true.
        static bool first_run = true;
        UpdateCaches(_Proj, first_run);
        first_run=false;
    }
    UpdateTimeStats(false);
}
#endif
