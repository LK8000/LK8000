/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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


// #define DEBUG_VIRTUALKEYS
// #define DEBUG_MAPINPUT

extern bool IsMultiMap();

COLORREF taskcolor = RGB_TASKLINECOL; // 091216
static bool ignorenext=false;
static bool MouseWasPanMoving=false;
bool OnFastPanning=false;

MapWindow::Zoom MapWindow::zoom;
MapWindow::Mode MapWindow::mode;

HBRUSH  MapWindow::hAboveTerrainBrush;
HPEN    MapWindow::hpCompassBorder;

int MapWindow::SnailWidthScale = 16;
int MapWindow::ScaleListCount = 0;
double MapWindow::ScaleList[];
int MapWindow::ScaleCurrent;

POINT MapWindow::Orig_Screen;

RECT MapWindow::MapRect;	// the entire screen area in use
RECT MapWindow::DrawRect;	// the portion of MapRect for drawing terrain, topology etc. (the map)

HBITMAP MapWindow::hDrawBitMap = NULL;
HBITMAP MapWindow::hDrawBitMapTmp = NULL;
HBITMAP MapWindow::hMaskBitMap = NULL;
HDC MapWindow::hdcDrawWindow = NULL;
HDC MapWindow::hdcScreen = NULL;
HDC MapWindow::hDCTemp = NULL;
HDC MapWindow::hDCMask = NULL;

#if NEWSMARTZOOM
HBITMAP MapWindow::hQuickDrawBitMap = NULL;
HDC MapWindow::hdcQuickDrawWindow = NULL;
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

unsigned char MapWindow::DeclutterLabels = MAPLABELS_ALLON;

double MapWindow::DisplayAngle = 0.0;
double MapWindow::DisplayAircraftAngle = 0.0;

DWORD MapWindow::targetPanSize = 0;

bool MapWindow::LandableReachable = false;

HPEN MapWindow::hSnailPens[NUMSNAILCOLORS];
COLORREF MapWindow::hSnailColours[NUMSNAILCOLORS];

POINT MapWindow::Groundline[NUMTERRAINSWEEPS+1];
#ifdef GTL2
POINT MapWindow::Groundline2[NUMTERRAINSWEEPS+1];
#endif

// 16 is number of airspace types
int      MapWindow::iAirspaceBrush[AIRSPACECLASSCOUNT] = {2,0,0,0,3,3,3,3,0,3,2,3,3,3,3,3};
int      MapWindow::iAirspaceColour[AIRSPACECLASSCOUNT] = {5,0,0,10,0,0,10,2,0,10,9,3,7,7,7,10};
int      MapWindow::iAirspaceMode[AIRSPACECLASSCOUNT] = {0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0};

HPEN MapWindow::hAirspacePens[AIRSPACECLASSCOUNT];
HPEN MapWindow::hAirspaceBorderPen;
bool MapWindow::bAirspaceBlackOutline = false;

HBRUSH  MapWindow::hInvBackgroundBrush[LKMAXBACKGROUNDS];

HBRUSH  MapWindow::hAirspaceBrushes[NUMAIRSPACEBRUSHES];

COLORREF MapWindow::Colours[NUMAIRSPACECOLORS] =
  {RGB(0xFF,0x00,0x00), RGB(0x00,0xFF,0x00),
   RGB(0x00,0x00,0xFF), RGB(0xFF,0xFF,0x00),
   RGB(0xFF,0x00,0xFF), RGB(0x00,0xFF,0xFF),
   RGB(0x7F,0x00,0x00), RGB(0x00,0x7F,0x00),
   RGB(0x00,0x00,0x7F), RGB(0x7F,0x7F,0x00),
   RGB(0x7F,0x00,0x7F), RGB(0x00,0x7F,0x7F),
   RGB(0xFF,0xFF,0xFF), RGB(0xC0,0xC0,0xC0),
   RGB(0x7F,0x7F,0x7F), RGB(0x00,0x00,0x00)};

HBRUSH MapWindow::hbCompass;
HBRUSH MapWindow::hbThermalBand;
HBRUSH MapWindow::hbBestCruiseTrack;
HBRUSH MapWindow::hbFinalGlideBelow;
HBRUSH MapWindow::hbFinalGlideAbove;
HBRUSH MapWindow::hbWind;


HPEN MapWindow::hpAircraft;
HPEN MapWindow::hpAircraftBorder;
HPEN MapWindow::hpWind;
HPEN MapWindow::hpWindThick;
HPEN MapWindow::hpBearing;
HPEN MapWindow::hpBestCruiseTrack;
HPEN MapWindow::hpCompass;

HPEN MapWindow::hpThermalBand;
HPEN MapWindow::hpThermalBandGlider;
HPEN MapWindow::hpFinalGlideAbove;
HPEN MapWindow::hpFinalGlideBelow;
HPEN MapWindow::hpMapScale;
HPEN MapWindow::hpMapScale2;
HPEN MapWindow::hpTerrainLine;
HPEN MapWindow::hpTerrainLineBg;
#ifdef GTL2
HPEN MapWindow::hpTerrainLine2Bg;
#endif
HPEN MapWindow::hpStartFinishThick;
HPEN MapWindow::hpStartFinishThin;
HPEN MapWindow::hpVisualGlideLightBlack; // VENTA3
HPEN MapWindow::hpVisualGlideHeavyBlack; // VENTA3
HPEN MapWindow::hpVisualGlideLightRed; // VENTA3
HPEN MapWindow::hpVisualGlideHeavyRed; // VENTA3

bool MapWindow::MapDirty = true;
bool PanRefreshed=false;

bool MapWindow::ForceVisibilityScan = false;

NMEA_INFO MapWindow::DrawInfo;
DERIVED_INFO MapWindow::DerivedDrawInfo;

extern void ShowMenu();

//
// Dragged position on screen: start and and end coordinates (globals!)
//
int XstartScreen, YstartScreen, XtargetScreen, YtargetScreen;

//
// Reminder: this is a callback function called each time an event is signalled.
// This means it can happen several times per second, while dragging for example.
// Some events are unmanaged and thus ignored, btw.
//
LRESULT CALLBACK MapWindow::MapWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

  //
  // Values to be remembered
  //
  static double Xstart, Ystart;
  static DWORD dwDownTime= 0L, dwUpTime= 0L, dwInterval= 0L;
  static double Xlat, Ylat;
  static double distance;

  //
  // Not needed to be static for remembering old values. Only for speedup.
  //
  static int lparam_X;
  static int lparam_Y;
  static int gestX, gestY, gestDir, gestDist;
  static bool dontdrawthemap;
  static bool mapmode8000;

  //
  // Candidates to be simplified, or to be made globals
  //
  static short Y_BottomBar;		  // this is different from BottomBarY
  static POINT P_HalfScreen;
  static POINT P_Doubleclick_bottomright; // squared area for screen lock doubleclick, normally on right bottombar
  static POINT P_MenuIcon_DrawBottom; 	  // Menu icon area (topleft coord)
  static POINT P_MenuIcon_noDrawBottom;   // same, without bottombar drawn, forgot why it is different

  static POINT P_UngestureLeft;
  static POINT P_UngestureRight;

  static short Y_Up, Y_Down;		// Up and Down keys vertical limits, ex. for zoom in out on map
  static short X_Left, X_Right;		// Ungestured fast clicks on infopages (THE SAME AS IN: PROCESS_VIRTUALKEY)
  
  //
  // Did the screen area changed somehow?
  //
  // Caution> always check that variables in use have been initialized by other threads on startup.
  // This is done by resetting, for example, MDI_MAPWNDPROC in LKInit.
  // 
  // MapRect is NOT ready yet on startup when we enter here. Do not use it for DoInit.
  //
  if (DoInit[MDI_MAPWNDPROC]) {

	Y_BottomBar=ScreenSizeY-BottomSize;
	P_HalfScreen.y=ScreenSizeY/2;
	P_HalfScreen.x=ScreenSizeX/2;
	P_Doubleclick_bottomright.x=ScreenSizeX-BottomSize-NIBLSCALE(15);
	P_Doubleclick_bottomright.y=ScreenSizeY-BottomSize-NIBLSCALE(15);

	// These were all using MapRect
	P_MenuIcon_DrawBottom.y=Y_BottomBar-14;
	P_MenuIcon_noDrawBottom.y=ScreenSizeY-AircraftMenuSize;
	P_MenuIcon_DrawBottom.x=ScreenSizeX-AircraftMenuSize;
	P_MenuIcon_noDrawBottom.x=P_MenuIcon_DrawBottom.x;
	P_UngestureLeft.x=CompassMenuSize;
	P_UngestureLeft.y=CompassMenuSize;
	P_UngestureRight.x=ScreenSizeX-CompassMenuSize;
	P_UngestureRight.y=CompassMenuSize;
	Y_Up=Y_BottomBar/2;
	Y_Down=Y_BottomBar - Y_Up;
	X_Left=(ScreenSizeX/2) - (ScreenSizeX/3);
	X_Right=(ScreenSizeX/2) + (ScreenSizeX/3);

	DoInit[MDI_MAPWNDPROC]=false;
  }

  //
  // service values for this run, the minimum necessary
  //
  lparam_X = (int) LOWORD(lParam);
  lparam_Y = (int) HIWORD(lParam);


  //
  // CURRENTLY DialogActive is never set. 
  // 120322 To be carefully checks for all situations.
  //
  #if 0
  if (DialogActive) return TRUE;
  #endif

  switch (uMsg)
    {
    case WM_ERASEBKGND:
	return TRUE;
    case WM_SIZE:
	if (!THREADRUNNING) {
		#if TESTBENCH
		StartupStore(_T("... MapWndProc WM_SIZE detected, DrawThread not running\n"));
		LKASSERT(hdcScreen);
		#endif
		ReleaseDC(hWnd, hdcScreen);

		LKASSERT(hdcDrawWindow);
		DeleteDC(hdcDrawWindow);

		LKASSERT(hDCTemp);
		DeleteDC(hDCTemp);

		LKASSERT(hDCMask);
		DeleteDC(hDCMask);

		#if NEWSMARTZOOM
		LKASSERT(hdcQuickDrawWindow);
		DeleteDC(hdcQuickDrawWindow);
		#endif
	
		hdcScreen = GetDC(hWnd);
		LKASSERT(hdcScreen);

		hdcDrawWindow = CreateCompatibleDC(hdcScreen);
		LKASSERT(hdcDrawWindow);
		#if NEWSMARTZOOM
		hdcQuickDrawWindow = CreateCompatibleDC(hdcScreen);
		LKASSERT(hdcQuickDrawWindow);
		#endif
		hDCTemp = CreateCompatibleDC(hdcDrawWindow);
		LKASSERT(hDCTemp);
		hDCMask = CreateCompatibleDC(hdcDrawWindow);
		LKASSERT(hdcDrawWindow);

	}

	if (hDrawBitMap) DeleteObject(hDrawBitMap);
	hDrawBitMap = CreateCompatibleBitmap (hdcScreen, lparam_X, lparam_Y);
	LKASSERT(hDrawBitMap);
	SelectObject(hdcDrawWindow, (HBITMAP)hDrawBitMap);

	if (hDrawBitMapTmp) DeleteObject(hDrawBitMapTmp);
	hDrawBitMapTmp = CreateCompatibleBitmap (hdcScreen, lparam_X, lparam_Y);
	LKASSERT(hDrawBitMapTmp);
	SelectObject(hDCTemp, (HBITMAP)hDrawBitMapTmp);

	if (hMaskBitMap) DeleteObject(hMaskBitMap);
	hMaskBitMap = CreateBitmap(lparam_X+1, lparam_Y+1, 1, 1, NULL);
	LKASSERT(hMaskBitMap);
	SelectObject(hDCMask, (HBITMAP)hMaskBitMap);

	#if NEWSMARTZOOM
	if (hQuickDrawBitMap) DeleteObject(hQuickDrawBitMap);
	hQuickDrawBitMap = CreateCompatibleBitmap (hdcScreen, lparam_X, lparam_Y);
	LKASSERT(hQuickDrawBitMap);
	SelectObject(hdcQuickDrawWindow, (HBITMAP)hQuickDrawBitMap);
	#endif

	break;

    case WM_CREATE:

	LKASSERT(!hdcScreen); 
	hdcScreen = GetDC(hWnd);
	LKASSERT(hdcScreen); 

	LKASSERT(!hdcDrawWindow);
	hdcDrawWindow = CreateCompatibleDC(hdcScreen);
	LKASSERT(hdcDrawWindow);

	#if NEWSMARTZOOM
	LKASSERT(!hdcQuickDrawWindow);
	hdcQuickDrawWindow = CreateCompatibleDC(hdcScreen);
	LKASSERT(hdcQuickDrawWindow);
	#endif
	LKASSERT(!hDCTemp);
	hDCTemp = CreateCompatibleDC(hdcDrawWindow);
	LKASSERT(hDCTemp);
	LKASSERT(!hDCMask);
	hDCMask = CreateCompatibleDC(hdcDrawWindow);
	LKASSERT(hDCMask);
  
	AlphaBlendInit();

	// Signal that draw thread can run now
	Initialised = TRUE;

	break;


    case WM_DESTROY:

	if (hdcScreen) {
		ReleaseDC(hWnd, hdcScreen);
		hdcScreen=NULL;
	}

	if (hdcDrawWindow) {     
		DeleteDC(hdcDrawWindow);
		hdcDrawWindow=NULL;
	}
	if (hDCTemp) {
		DeleteDC(hDCTemp);
		hDCTemp=NULL;
	}
	if (hDCMask) {
		DeleteDC(hDCMask);
		hDCMask=NULL;
	}
	if (hDrawBitMap) {
		DeleteObject(hDrawBitMap);
		hDrawBitMap=NULL;
	}
	if (hMaskBitMap) {
		DeleteObject(hMaskBitMap);
		hMaskBitMap=NULL;
	}
	#if NEWSMARTZOOM
	if (hdcQuickDrawWindow) {
		DeleteDC(hdcQuickDrawWindow);
		hdcQuickDrawWindow=NULL;
	}
	if (hQuickDrawBitMap) {
		DeleteObject(hQuickDrawBitMap);
		hQuickDrawBitMap=NULL;
	}
	#endif

	AlphaBlendDestroy();
	PostQuitMessage (0);
	break;


    case WM_LBUTTONDBLCLK: 
      //
      // Attention please: a DBLCLK is followed by a simple BUTTONUP with NO buttondown.
      //
      // 111110 there is no need to process buttondblclick if the map is unlocked.
      // So we go directly to buttondown, simulating a non-doubleclick.
      if (!LockModeStatus) goto _buttondown;

      dwDownTime = GetTickCount();  
      XstartScreen = lparam_X; YstartScreen = lparam_Y;

	if (LockModeStatus) {
        	ignorenext=true; // do not interpret the second click of the doubleclick as a real click.
		if (XstartScreen < P_Doubleclick_bottomright.x) break;
		if (YstartScreen < P_Doubleclick_bottomright.y) break;
		LockMode(2);
		DoStatusMessage(gettext(_T("_@M964_"))); // SCREEN IS UNLOCKED
	}

      // Careful! If you ignorenext, any event timed as double click of course will be affected.
      // and this means also fast clicking on bottombar!!
      // so first lets see if we are in lk8000 text screens.. 
      if (DONTDRAWTHEMAP || (MAPMODE8000 && (YstartScreen >=BottomBarY))) {  
		// do not ignore next, let buttonup get the signal
		break;
      }

	// only when activemap (which means newmap is on) is off
      if (ActiveMap) ignorenext=true;
      break;

    case WM_MOUSEMOVE:
	// Mouse moving!
	if (wParam==MK_LBUTTON) {
		// Consider only pure PAN mode, ignore the rest of cases here
		if (!mode.Is(Mode::MODE_PAN)) break;
		if (mode.Is(Mode::MODE_TARGET_PAN)) break;

		if (PanRefreshed) {
			// if map was redrawn, we update our position as well. just like
			// we just clicked on mouse from here, like in BUTTONDOWN
			XstartScreen = lparam_X; YstartScreen = lparam_Y;
			Screen2LatLon(XstartScreen, YstartScreen, Xstart, Ystart);
			PanRefreshed=false;
			// NO! This is causing false clicks passing underneath CANCEL button!
			// dwDownTime=GetTickCount();
			break;
		}
		// set a min mouse move to trigger panning
		if ( (abs(XstartScreen-lparam_X)+abs(YstartScreen-lparam_Y)) > (ScreenScale+1)) {
			Screen2LatLon(lparam_X, lparam_Y, Xlat, Ylat);
			PanLongitude += (Xstart-Xlat);
			PanLatitude  += (Ystart-Ylat);
			ignorenext=true;
			MouseWasPanMoving=true;

			// With OnFastPanning we are saying: Since we are dragging the bitmap around,
			// forget the Redraw requests from other parts of LK, which would cause PanRefreshed.
			// We have no control on those requests issued for example by Calc thread.
			// However we force full map refresh after some time in ms
			#if (WINDOWSPC>0)
			// on pc force full rendering in real time, but not in TESTBENCH
			  #if TESTBENCH
			if ( (GetTickCount()-dwDownTime)>700) {
			  #else
			if (1) {
			  #endif
			#else
			if ( (GetTickCount()-dwDownTime)>700) {  // half a second + max 1s
			#endif
				dwDownTime=GetTickCount();
				OnFastPanning=false;
				RefreshMap();
			} else {
				XtargetScreen=lparam_X; YtargetScreen=lparam_Y;
				// Lets not forget that if we stop moving the mouse, or we exit the windows while
				// dragging, or we endup on another window -f.e. a button - then we will NOT
				// receive any more events in this loop! For this reason we must let the
				// DrawThread clear this flag. Not thread safe, I know.
				OnFastPanning=true;

				// This will force BitBlt operation in RenderMapWindow
				RequestFastRefresh();
			}
		} // minimal movement detected
	} // mouse moved with Lbutton (dragging)
	break;

    case WM_LBUTTONDOWN:
_buttondown:
      if (LockModeStatus) break;
      dwDownTime = GetTickCount();
      // When we have buttondown these flags should be set off.
      MouseWasPanMoving=false;
      OnFastPanning=false;
      // After calling a menu, on exit as we touch the screen we fall back here
      if (ignorenext) {
                #ifdef DEBUG_MAPINPUT
		DoStatusMessage(TEXT("DBG-055 BUTTONDOWN with ignorenext"));
                #endif
		break;
      }
      XstartScreen = lparam_X; YstartScreen = lparam_Y;
      // TODO VNT move Screen2LatLon in LBUTTONUP after making sure we really need Xstart and Ystart
      // so we save precious milliseconds waiting for BUTTONUP GetTickCount
      Screen2LatLon(XstartScreen, YstartScreen, Xstart, Ystart);

      LKevent=LKEVENT_NONE; // CHECK FIX TODO VENTA10  probably useless 090915

      break;

    case WM_LBUTTONUP:
	if (LockModeStatus) break;
	// Mouse released DURING panning, full redraw requested.
	// Otherwise process virtual keys etc. as usual
	if (MouseWasPanMoving) {
		MouseWasPanMoving=false;
		OnFastPanning=false;
		ignorenext=false;
		RefreshMap();
		dwDownTime=0; // otherwise we shall get a fake click passthrough
		break;
	}
	if (ignorenext||dwDownTime==0) { 
                #ifdef DEBUG_MAPINPUT
		if (ignorenext && (dwDownTime==0) )
			DoStatusMessage(_T("DBG-098 ignorenext&&dwDownTime0"));
		else if (dwDownTime==0)
			DoStatusMessage(_T("DBG-099 dwDownTime0"));
		else
			DoStatusMessage(_T("DBG-097 ignorenext"));
                #endif
		ignorenext=false;
		break;
	}

	// we save these flags for the entire processing, just in case they change
	// while processing a virtual key for example, and also for acceleration.
	dontdrawthemap=(DONTDRAWTHEMAP);
	mapmode8000=(MAPMODE8000);

	dwUpTime = GetTickCount(); 
	dwInterval=dwUpTime-dwDownTime;
	dwDownTime=0; // do it once forever

	gestDir=LKGESTURE_NONE; gestDist=-1;

	gestY=YstartScreen-lparam_Y;
	gestX=XstartScreen-lparam_X;

	if (  dontdrawthemap && (lparam_Y <Y_BottomBar) ) { 

		gestDist=isqrt4((long)((gestX*gestX) + (gestY*gestY)));

		// GESTURE DETECTION
		// if gestX >0 gesture from right to left , gestX <0 gesture from left to right
		// if gestY >0 gesture from down to up ,    gestY <0 gesture from up to down
		if (gestDist<GestureSize) { 	// TODO FIX tune this GestureSize, maybe 100?
			gestDir=LKGESTURE_NONE;
		} else {
			// horizontal includes also perfectly diagonal gestures
			if (abs(gestX) >= abs(gestY) ) {
				// we use LKGESTURE definition, but they have nothing to do with those used in other part of source code
				if (gestX>0)
					gestDir=IphoneGestures?LKGESTURE_RIGHT:LKGESTURE_LEFT;
				else
					gestDir=IphoneGestures?LKGESTURE_LEFT:LKGESTURE_RIGHT;
			} else { 
				if (gestY>0)
					gestDir=IphoneGestures?LKGESTURE_DOWN:LKGESTURE_UP;
				else
					gestDir=IphoneGestures?LKGESTURE_UP:LKGESTURE_DOWN;
			}
		}
		// end dontdrawthemap and inside mapscreen looking for a gesture
	} 

        if (mode.Is(Mode::MODE_TARGET_PAN)) {
		if (AATEnabled) {
			Screen2LatLon(lparam_X, lparam_Y, Xlat, Ylat);
			LockTaskData();
			targetMoved = true;
			targetMovedLat = Ylat;
			targetMovedLon = Xlat;
			UnlockTaskData();
		}
		// else we are in simple TARGET dialog, and we must NOT process anything
		break;
	}

	short topicon;
	if (DrawBottom)
		topicon=P_MenuIcon_DrawBottom.y;
	else
		topicon=P_MenuIcon_noDrawBottom.y;

	if ( (lparam_X > P_MenuIcon_DrawBottom.x ) && (lparam_Y > topicon) ) {

		// short click on aircraft icon
		//
		if ( dwInterval <= (DOUBLECLICKINTERVAL)) {
goto_menu:
			#ifndef DISABLEAUDIO
                	if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			ShowMenu();
			break;
		} else
		// Long click on aircraft icon, toggle thermal mode
		//
		if ( dwInterval >=VKLONGCLICK) { // in Defines.h
			if (mode.Is(Mode::MODE_CIRCLING)) {
				mode.UserForcedMode(Mode::MODE_FLY_CRUISE);
				#ifndef DISABLEAUDIO
				if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				#endif
				break;
			} else 
			if (mode.Is(Mode::MODE_CRUISE)) {
				mode.UserForcedMode(Mode::MODE_FLY_NONE);
				#ifndef DISABLEAUDIO
				if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				#endif
				break;
			}
		} else {
			// We are here in any case only when dwInterval is <VKLONGCLICK
			if (dwInterval >=(unsigned)CustomKeyTime) {
				if (!CustomKeyHandler(CKI_BOTTOMICON)) goto goto_menu;
			}
			break;
		}

      // end aircraft icon check				
      } 

	// MultiMap specials, we use same geometry of MSM_MAP
	if (!MapWindow::mode.AnyPan() && IsMultiMap() ) {
		if ( (lparam_X <= P_UngestureLeft.x) && (lparam_Y <= P_UngestureLeft.y) ) {
			#ifndef DISABLEAUDIO
         		 if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			LKevent=LKEVENT_TOPLEFT;
			MapWindow::RefreshMap();
			return TRUE;
		}
		if ( (lparam_X > P_UngestureRight.x) && (lparam_Y <= P_UngestureRight.y) ) {
			#ifndef DISABLEAUDIO
         		 if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			LKevent=LKEVENT_TOPRIGHT;
			MapWindow::RefreshMap();
			return TRUE;
		}
	}

	// Otherwise not in multimap, proceed with normal checks
	if (mapmode8000) { 
	if ( (lparam_X <= P_UngestureLeft.x) && (lparam_Y <= P_UngestureLeft.y) ) {
		
		if (!CustomKeyHandler(CKI_TOPLEFT)) {
			// Well we better NOT play a click while zoomin in and out, because on slow
			// devices it will slow down the entire process.
			// Instead, we make a click from InputEvents, debounced.
			#if 0
			#ifndef DISABLEAUDIO
         		 if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			#endif
			wParam = 0x26; 
			// zoom in
			InputEvents::processKey(wParam);
			return TRUE;
		}
		MapWindow::RefreshMap();
		break;
	}

      if (ISPARAGLIDER) {
	// Use the compass to pullup UTM informations to paragliders
	if ( (lparam_X > P_UngestureRight.x) && (lparam_Y <= P_UngestureRight.y) ) {

		if ((dwInterval >= DOUBLECLICKINTERVAL) ) {

			// if we are running a real task, with gates, and we could still start
			// if only 1 time gate, and we passed valid start, no reason to resettask
			int acceptreset=2;
			if (PGNumberOfGates==1) acceptreset=1;

			if (UseGates() && ValidTaskPoint(1) && ActiveWayPoint<acceptreset) { // 100507 101110
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
			break;
		}

	} // End compass icon check
      } // PARAGLIDERs special buttons
	// else not a paraglider key, process it for gliders
	// else { 
	// change in 2.3q: we let paragliders use the CK as well
	{ 
		if ( (lparam_X > P_UngestureRight.x) && (lparam_Y <= P_UngestureRight.y) ) {
			if (!CustomKeyHandler(CKI_TOPRIGHT)) {
				#if 0
				#ifndef DISABLEAUDIO
         			if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				#endif
				#endif
				wParam = 0x26; 
				InputEvents::processKey(wParam);
				return TRUE;
			}
			MapWindow::RefreshMap();
			break;
		}
		// if not topright, continue
	}
	// do all of this only if !dontdrawthemap = we are in fullscreen and not PAN etc.
	// indentation is wrong here
	} 


	// "fast virtual keys" are handled locally and not passed to event handler.
	// they are processed even when virtual keys are disabled, because they concern special lk8000 menus.

	// First case: for mapspacemodes we manage gestures as well
	if (dontdrawthemap) {

		if ( gestDir == LKGESTURE_UP) {
			ProcessVirtualKey(lparam_X,lparam_Y,dwInterval,LKGESTURE_UP);
			break;
		}
		if ( gestDir == LKGESTURE_DOWN) {
			ProcessVirtualKey(lparam_X,lparam_Y,dwInterval,LKGESTURE_DOWN);
			break;
		}
		if ( gestDir == LKGESTURE_LEFT) {
			ProcessVirtualKey(lparam_X,lparam_Y,dwInterval,LKGESTURE_LEFT);
			break;
		}
		if ( gestDir == LKGESTURE_RIGHT) {
			ProcessVirtualKey(lparam_X,lparam_Y,dwInterval,LKGESTURE_RIGHT);
			break;
		}


		// We are here when lk8000, and NO moving map displayed: virtual enter, virtual up/down, or 
		// navbox operations including center key.
		wParam=ProcessVirtualKey(lparam_X,lparam_Y,dwInterval,LKGESTURE_NONE);
                #ifdef DEBUG_MAPINPUT
		DoStatusMessage(_T("DBG-035 navboxes")); 
                #endif
		// we could use a single ProcessVirtual for all above, and check that wParam on return
		// is really correct for gestures as well... since we do not want to go to wirth with gestures!
		if (wParam!=0) {
			DoStatusMessage(_T("ERR-033 Invalid Virtual Key")); 
			break;
		}
		break;

	}
      
	// if clicking on navboxes, process fast virtual keys
	// maybe check LK8000 active?
	// This point is selected when in MapSpaceMode==MSM_MAP, i.e. lk8000 with moving map on.
	if (  DrawBottom && (lparam_Y >= Y_BottomBar) && !mode.AnyPan() ) {
		wParam=ProcessVirtualKey(lparam_X,lparam_Y,dwInterval,LKGESTURE_NONE);
                #ifdef DEBUG_MAPINPUT
		DoStatusMessage(_T("DBG-034 navboxes")); 
                #endif
		if (wParam!=0) {
			DoStatusMessage(_T("ERR-034 Invalid Virtual Key")); 
			break;
		}
		break;
	}


      if (dwInterval == 0) {
		break; // should be impossible
      }

	// we need to calculate it here only if needed
	if (gestDist>=0)
		distance = gestDist /ScreenScale;
	else
		distance = isqrt4((long)((XstartScreen-lparam_X)*(XstartScreen-lparam_X)+ (YstartScreen-lparam_Y)*(YstartScreen-lparam_Y))) /ScreenScale;

	#ifdef DEBUG_VIRTUALKEYS
	TCHAR buf[80]; char sbuf[80];
	sprintf(sbuf,"%.0f",distance);
	wsprintf(buf,_T("XY=%d,%d dist=%S Up=%ld Down=%ld Int=%ld"),lparam_X,lparam_Y,sbuf,dwUpTime,dwDownTime,dwInterval);
        DoStatusMessage(buf);
	#endif

	// Handling double click passthrough
	// Caution, timed clicks from PC with a mouse are different from real touchscreen devices

      // Process faster clicks here and no precision, but let DBLCLK pass through
      // VK are used in the bottom line in this case, forced on for this situation.
      if (  DrawBottom && (lparam_Y >= Y_BottomBar) ) {
		// DoStatusMessage(_T("Click on hidden map ignored")); 

		// do not process virtual key if it is timed as a DBLCLK
		// we want users to get used to double clicking only on infoboxes
		// and avoid triggering unwanted waypoints details
		if (dwInterval >= ( (DOUBLECLICKINTERVAL/2-30) )) { // fast dblclk required here.
			#ifdef DEBUG_VIRTUALKEYS // 100320
			wParam=ProcessVirtualKey(lparam_X,lparam_Y,dwInterval,LKGESTURE_NONE);
			if (wParam==0) {
				DoStatusMessage(_T("P00 Virtual Key 0")); 
				break;
			}
			#else
			ProcessVirtualKey(lparam_X,lparam_Y,dwInterval,LKGESTURE_NONE);
			#endif
			break; 
		}
		// do not process click on the underneath window
		break;
      }
      if (dontdrawthemap) break;

      Screen2LatLon(lparam_X, lparam_Y, Xlat, Ylat);
      if (SIMMODE && (!mode.Is(Mode::MODE_TARGET_PAN) && (distance>NIBLSCALE(36)))) {
	// This drag moves the aircraft (changes speed and direction)
	double newbearing;
	double oldbearing = GPS_INFO.TrackBearing;
	double minspeed = 1.1*GlidePolar::Vminsink;
	DistanceBearing(Ystart, Xstart, Ylat, Xlat, NULL, &newbearing);
	if ((fabs(AngleLimit180(newbearing-oldbearing))<30) || (GPS_INFO.Speed<minspeed)) {
		// sink we shall be sinking, lets raise the altitude when using old simulator interface
		if ( (CALCULATED_INFO.TerrainValid) && ( CALCULATED_INFO.AltitudeAGL <0 ))
			GPS_INFO.Altitude=CALCULATED_INFO.TerrainAlt;
		GPS_INFO.Altitude+=200;
		if (ISPARAGLIDER)
		  GPS_INFO.Speed = min(16.0,max(minspeed,distance/9));
		else
		  GPS_INFO.Speed = min(100.0,max(minspeed,distance/3));
	} 
	GPS_INFO.TrackBearing = (int)newbearing;
	if (GPS_INFO.TrackBearing==360) GPS_INFO.TrackBearing=0;
	TriggerGPSUpdate();
      
	break;
      }

      if (!mode.Is(Mode::MODE_TARGET_PAN)) {
		
		//
		// Finally process normally a click on the moving map.
		//
			if(dwInterval < AIRSPACECLICK) { // original and untouched interval
				if (ActiveMap) {
                                  if (Event_NearestWaypointDetails(Xstart, Ystart, 500*zoom.RealScale(), false)) {
						break;
					}
				} else {
/* REMOVE
					int yup, ydown, ytmp;
					ytmp=(int)((MapWindow::MapRect.bottom-MapWindow::MapRect.top-BottomSize)/2);
					yup=ytmp+MapWindow::MapRect.top;
					ydown=MapWindow::MapRect.bottom-BottomSize-ytmp;
					// 
					// 120912 Process ungesture left and right on the moving map
					// THE SAME AS IN: PROCESS_VIRTUALKEY
					//
					int s_sizeright=MapWindow::MapRect.right-MapWindow::MapRect.left;
					// used by ungesture fast click on infopages
					int s_unxleft=(s_sizeright/2)-(s_sizeright/3);
					int s_unxright=(s_sizeright/2)+(s_sizeright/3);
*/

					if (!mode.AnyPan() && (UseUngestures || !ISPARAGLIDER)) {
						if (lparam_X<=X_Left) {
							PreviousModeType();
							MapWindow::RefreshMap();
							return TRUE;
						}
						if (lparam_X>=X_Right) {
							NextModeType();
							MapWindow::RefreshMap();
							return TRUE;
						}
					}

					if (lparam_Y<Y_Up) {
						// pg UP = zoom in
						wParam = 0x26;
					} else {
						if (lparam_Y>Y_Down) {
							// pg DOWN = zoom out
							wParam = 0x28;
						} 
						else {
							// process center key, do nothing 
							break;
						}
					}
					// no sound for zoom clicks
					#if 0 // REMOVE
					#ifndef DISABLEAUDIO
					if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
					#endif
					#endif
					InputEvents::processKey(wParam);
					dwDownTime= 0L;
					return TRUE; 
				}
			} else {

				// Select airspace on moving map only if they are visible
				// 120526 moved out of anypan, buggy because we want airspace selection with priority
				if (OnAirSpace && Event_InteriorAirspaceDetails(Xstart, Ystart))
					break;

				if (!mode.AnyPan()) {
					// match only center screen
					if (  (abs(lparam_X-P_HalfScreen.x) <NIBLSCALE(100)) && 
					      (abs(lparam_Y-P_HalfScreen.y) <NIBLSCALE(100)) ) {

						if (CustomKeyHandler(CKI_CENTERSCREEN)) break;
					}
				}
				break;
			}
      } // !TargetPan

      break;


    // VENTA-TODO careful here, keyup no more trapped for PNA. 
    // Forbidden usage of keypress timing.
    #if defined(PNA)
    case WM_KEYDOWN:
    #else
    case WM_KEYUP: 
    #endif


      #if defined(PNA)

      if ( GlobalModelType == MODELTYPE_PNA_HP31X )
	{
	  //		if (wParam == 0x7b) wParam=0xc1;  // VK_APP1 	
	  if (wParam == 0x7b) wParam=0x1b;  // VK_ESCAPE
	  //		if (wParam == 0x7b) wParam=0x27;  // VK_RIGHT
	  //		if (wParam == 0x7b) wParam=0x25;  // VK_LEFT
	} else
	if ( GlobalModelType == MODELTYPE_PNA_PN6000 )
	  {
	    switch(wParam) {
	    case 0x79:					// Upper Silver key short press
	      wParam = 0xc1;	// F10 -> APP1
	      break;
	    case 0x7b:					// Lower Silver key short press
	      wParam = 0xc2;	// F12 -> APP2
	      break;
	    case 0x72:					// Back key plus
	      wParam = 0xc3;	// F3  -> APP3
	      break;
	    case 0x71:					// Back key minus
	      wParam = 0xc4;	// F2  -> APP4
	      break;
	    case 0x7a:					// Upper silver key LONG press
	      wParam = 0x70;	// F11 -> F1
	      break;
	    case 0x7c:					// Lower silver key LONG press
	      wParam = 0x71;	// F13 -> F2
	      break;
	    }
	  }
      if ( GlobalModelType == MODELTYPE_PNA_NOKIA_500 )
	{
	  switch(wParam) {
	  case 0xc1:				
	    wParam = 0x0d;	// middle key = enter
	    break;
	  case 0xc5:				
	    wParam = 0x26;	// + key = pg Up
	    break;
	  case 0xc6:				
	    wParam = 0x28;	// - key = pg Down
	    break;
	  }
	}
      if ( GlobalModelType == MODELTYPE_PNA_MEDION_P5 )
	{
	  switch(wParam) {
	  case 0x79:				
	    wParam = 0x0d;	// middle key = enter
	    break;
	  case 0x75:				
	    wParam = 0x26;	// + key = pg Up
	    break;
	  case 0x76:				
	    wParam = 0x28;	// - key = pg Down
	    break;
	  }
	}

      #endif // PNA TRANSCODING

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
	// THIS IS AN ALTERNATE LXMINIMAP USAGE, will not work for official release
	#ifndef LXMINIMAP
	// if (1) {
	if ( GlobalModelType == MODELTYPE_PNA_MINIMAP ) {
		switch(wParam) {

			// Button A is generating a C
			case 67:
				if ( !MapWindow::mode.AnyPan()&&MapSpaceMode!=1) { // dontdrawthemap
					if (MapSpaceMode<=MSM_MAP) {
						return TRUE;
					}
					#ifndef DISABLEAUDIO
					if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
					#endif
					LKevent=LKEVENT_ENTER;
					MapWindow::RefreshMap();
					return TRUE;
				} else {
					if (CustomKeyHandler(CKI_CENTERSCREEN)) {
						#ifndef DISABLEAUDIO
						if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
						#endif
					}
					// MapWindow::RefreshMap();
					return TRUE;
				}
				break;
			// Button B is generating alternate codes 68 and 27
			// we use both as a single one
			case 68:
			case 27:
				if (CustomKeyHandler(CKI_BOTTOMLEFT)) {
					#ifndef DISABLEAUDIO
					if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
					#endif
					// MapWindow::RefreshMap();
					return TRUE;
				}
				// else transcode here
				break;
			// Button C is generating a RETURN
			case 13:
				if (CustomKeyHandler(CKI_BOTTOMCENTER)) {
					#ifndef DISABLEAUDIO
					if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
					#endif
					// MapWindow::RefreshMap();
					return TRUE;
				}
				// else transcode here
				break;
			// Button D is generating a SPACE
			case 32:
				if (CustomKeyHandler(CKI_BOTTOMRIGHT)) {
					#ifndef DISABLEAUDIO
					if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
					#endif
					// MapWindow::RefreshMap();
					return TRUE;
				}
				// else transcode here
				break;

			// Rotary knob A is generating a 38 (turn left) and 40 (turn right)
			case 38:
				if ( !MapWindow::mode.AnyPan()&&MapSpaceMode!=1) { // dontdrawthemap
					if (MapSpaceMode<=MSM_MAP) {
						return TRUE;
					}
					#ifndef DISABLEAUDIO
					if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
					#endif
					LKevent=LKEVENT_UP;
					MapWindow::RefreshMap();
				} else {
					MapWindow::zoom.EventScaleZoom(-1);
				}
				return TRUE;
				break;

			case 40:
				if ( !MapWindow::mode.AnyPan()&&MapSpaceMode!=1) { // dontdrawthemap
					if (MapSpaceMode<=MSM_MAP) {
						return TRUE;
					}
					#ifndef DISABLEAUDIO
					if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
					#endif
					LKevent=LKEVENT_DOWN;
					MapWindow::RefreshMap();
				} else {
					MapWindow::zoom.EventScaleZoom(1);
				}
				return TRUE;
				break;

			// Rotary knob E is generating a 37 (turn left) and 39 (turn right)
			case 37:
				PreviousModeIndex();
				MapWindow::RefreshMap();
				SoundModeIndex();
				return TRUE;
			case 39:
				NextModeIndex();
				MapWindow::RefreshMap();
				SoundModeIndex();
				return TRUE;

			default:
				break;
                }
	}
	#endif // not LXMINIMAP

	dwDownTime= 0L; // removable? check
	InputEvents::processKey(wParam);
	dwDownTime= 0L;
	return TRUE; 
    }

  return (DefWindowProc (hWnd, uMsg, wParam, lParam));
}


