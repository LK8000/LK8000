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
#include "Multimap.h"
#include "Logger.h"
#include "Dialogs.h"
#include "Screen/LKBitmapSurface.h"

#define KEYDEBOUNCE 100

// #define DEBUG_VIRTUALKEYS
// #define DEBUG_MAPINPUT

static bool isListPage(void);

LKColor taskcolor = RGB_TASKLINECOL; // 091216
static bool ignorenext=false;
static bool MouseWasPanMoving=false;
bool OnFastPanning=false;

MapWindow::Zoom MapWindow::zoom;
MapWindow::Mode MapWindow::mode;

LKBrush  MapWindow::hAboveTerrainBrush;

int MapWindow::SnailWidthScale = 16;
int MapWindow::ScaleListCount = 0;
double MapWindow::ScaleList[];
int MapWindow::ScaleCurrent;

POINT MapWindow::Orig_Screen;

RECT MapWindow::MapRect;	// the entire screen area in use
RECT MapWindow::DrawRect;	// the portion of MapRect for drawing terrain, topology etc. (the map)
/*
LKBitmap MapWindow::hDrawBitMap;
LKBitmap MapWindow::hDrawBitMapTmp;
LKBitmap MapWindow::hMaskBitMap;
LKBitmap MapWindow::mhbbuffer;
*/
LKWindowSurface MapWindow::ScreenSurface;


LKBitmapSurface MapWindow::hdcDrawWindow;
  
LKBitmapSurface MapWindow::hDCTempTask;
LKBitmapSurface MapWindow::hdcTempTerrainAbove;

LKBitmapSurface MapWindow::hdcTempAsp;
LKMaskBitmapSurface MapWindow::hdcMask;
LKBitmapSurface MapWindow::hdcbuffer;

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

DWORD MapWindow::targetPanSize = 0;

bool MapWindow::LandableReachable = false;

LKPen MapWindow::hSnailPens[NUMSNAILCOLORS];
LKColor MapWindow::hSnailColours[NUMSNAILCOLORS];

POINT MapWindow::Groundline[NUMTERRAINSWEEPS+1];
#ifdef GTL2
POINT MapWindow::Groundline2[NUMTERRAINSWEEPS+1];
#endif

// 16 is number of airspace types
int      MapWindow::iAirspaceBrush[AIRSPACECLASSCOUNT] = {2,0,0,0,3,3,3,3,0,3,2,3,3,3,3,3};
int      MapWindow::iAirspaceColour[AIRSPACECLASSCOUNT] = {5,0,0,10,0,0,10,2,0,10,9,3,7,7,7,10};
int      MapWindow::iAirspaceMode[AIRSPACECLASSCOUNT] = {0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0};

LKPen MapWindow::hAirspacePens[AIRSPACECLASSCOUNT];
LKPen MapWindow::hBigAirspacePens[AIRSPACECLASSCOUNT];
LKPen MapWindow::hAirspaceBorderPen;

LKBrush  MapWindow::hInvBackgroundBrush[LKMAXBACKGROUNDS];

LKBrush  MapWindow::hAirspaceBrushes[NUMAIRSPACEBRUSHES];

LKColor MapWindow::Colours[NUMAIRSPACECOLORS] =
  {LKColor(0xFF,0x00,0x00), LKColor(0x00,0xFF,0x00),
   LKColor(0x00,0x00,0xFF), LKColor(0xFF,0xFF,0x00),
   LKColor(0xFF,0x00,0xFF), LKColor(0x00,0xFF,0xFF),
   LKColor(0x7F,0x00,0x00), LKColor(0x00,0x7F,0x00),
   LKColor(0x00,0x00,0x7F), LKColor(0x7F,0x7F,0x00),
   LKColor(0x7F,0x00,0x7F), LKColor(0x00,0x7F,0x7F),
   LKColor(0xFF,0xFF,0xFF), LKColor(0xC0,0xC0,0xC0),
   LKColor(0x7F,0x7F,0x7F), LKColor(0x00,0x00,0x00)};


LKPen MapWindow::hpAircraft;
LKPen MapWindow::hpWindThick;

LKPen MapWindow::hpThermalBand;
LKPen MapWindow::hpThermalBandGlider;
LKPen MapWindow::hpFinalGlideAbove;
LKPen MapWindow::hpFinalGlideBelow;
LKPen MapWindow::hpMapScale2;
LKPen MapWindow::hpTerrainLine;
LKPen MapWindow::hpTerrainLineBg;
LKPen MapWindow::hpStartFinishThick;

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


#ifdef LONGCLICK_FEEDBACK
    static LPARAM  lOld = 0;
#endif
  //
  // Values to be remembered
  //
  static bool  pressed = false;
  static double Xstart, Ystart;
  static Poco::Timestamp tsDownTime= 0L, tsUpTime= 0L;
  static Poco::Timespan DownUpInterval= 0L;
  static double Xlat, Ylat;
  static double distance;

  //
  // Not needed to be static for remembering old values. Only for speedup.
  //
  static int lparam_X;
  static int lparam_Y;
  static int gestX, gestY, gestDir, gestDist;
  static bool dontdrawthemap;

  //
  // Candidates to be simplified, or to be made globals
  //
  static short Y_BottomBar;		  // this is different from BottomBarY
#ifdef CENTER_CUSTOMKEY
  static POINT P_HalfScreen;
#endif
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
#ifdef CENTER_CUSTOMKEY
	P_HalfScreen.y=ScreenSizeY/2;
	P_HalfScreen.x=ScreenSizeX/2;
#endif
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

    case WM_SETFOCUS:
		#if DEBUG_FOCUS
		StartupStore(_T("............ MAPWINDOWS HAS FOCUS\n"));
		#endif
		extern HWND hWndWithFocus;
		hWndWithFocus=hWnd;
		return 0;
		break;

    case WM_SIZE:
        if (!MapWindow::IsDisplayRunning()) {
            hdcDrawWindow.Resize(lparam_X, lparam_Y);

            hDCTempTask.Resize(lparam_X, lparam_Y);
            hdcTempTerrainAbove.Resize(lparam_X, lparam_Y);

            hdcTempAsp.Resize(lparam_X, lparam_Y);
            hdcbuffer.Resize(lparam_X, lparam_Y);
            hdcMask.Resize(lparam_X+1, lparam_Y+1);
        }
	break;

    case WM_CREATE:

    	ScreenSurface.Create(hWnd);

        hdcDrawWindow.Create(ScreenSurface, lparam_X, lparam_Y);
        
        hDCTempTask.Create(ScreenSurface, lparam_X, lparam_Y);
        hdcTempTerrainAbove.Create(ScreenSurface, lparam_X, lparam_Y);

        hdcTempAsp.Create(ScreenSurface, lparam_X, lparam_Y);
        hdcbuffer.Create(ScreenSurface, lparam_X, lparam_Y);
        hdcMask.Create(ScreenSurface, lparam_X+1, lparam_Y+1);

	// Signal that draw thread can run now
	Initialised = TRUE;

	break;


    case WM_DESTROY:

        ScreenSurface.Release();

        hdcDrawWindow.Release();
        
        hDCTempTask.Release();
        hdcTempTerrainAbove.Release();

        hdcTempAsp.Release();
        hdcbuffer.Release();
        hdcMask.Release();

	PostQuitMessage (0);
	break;


    case WM_LBUTTONDBLCLK: 
      //
      // Attention please: a DBLCLK is followed by a simple BUTTONUP with NO buttondown.
      //
      // 111110 there is no need to process buttondblclick if the map is unlocked.
      // So we go directly to buttondown, simulating a non-doubleclick.
      if (!LockModeStatus) goto _buttondown;

      tsDownTime.update();  
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
	// The idea was/is: break if we are in the nearest pages, or in main map but on the bottom bar.
	if ((DONTDRAWTHEMAP) || (IsMultiMap() && (YstartScreen >=BottomBarY))) {  
		// do not ignore next, let buttonup get the signal
		break;
      }

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
			// dwDownTime.update();
			break;
		}
		// set a min mouse move to trigger panning
		if ( (abs(XstartScreen-lparam_X)+abs(YstartScreen-lparam_Y)) > (ScreenScale+1)) {
			Screen2LatLon(lparam_X, lparam_Y, Xlat, Ylat);
			PanLongitude += (Xstart-Xlat);
			PanLatitude  += (Ystart-Ylat);

			if(ValidTaskPoint(PanTaskEdit))
			{
			  LockTaskData(); // protect from external task changes
			  WayPointList[RESWP_PANPOS].Latitude = PanLatitude;
			  WayPointList[RESWP_PANPOS].Longitude= PanLongitude;
			  static int iCnt =0;
			  if(iCnt++ > 10)
			  {
			    RefreshTask();
			    iCnt =0;
			  }
			  UnlockTaskData(); // protect from external task changes
			}


			ignorenext=true;
			MouseWasPanMoving=true;

			// With OnFastPanning we are saying: Since we are dragging the bitmap around,
			// forget the Redraw requests from other parts of LK, which would cause PanRefreshed.
			// We have no control on those requests issued for example by Calc thread.
			// However we force full map refresh after some time in ms

			// If time has passed  then force a MapDirty and redraw the whole screen.
			// This was previously not working in v3 because ThreadCalculations was forcing MapDirty 
			// in the background each second, and we were loosing control!
			if ( tsDownTime.isElapsed(Poco::Timespan(0,1000*FASTPANNING).totalMicroseconds()) ) {
				tsDownTime.update();
				OnFastPanning=false;
				RefreshMap();
			} else {
				XtargetScreen=lparam_X; YtargetScreen=lparam_Y;
				// Lets not forget that if we stop moving the mouse, or we exit the windows while
				// dragging, or we endup on another window -f.e. a button - then we will NOT
				// receive any more events in this loop! For this reason we must let the
				// DrawThread clear this flag. Not thread safe, I know.
				//
				// There is a situation to consider, here: we are setting OnFastPanning but
				// if we stop moving the mouse/finger, we shall never be back in MapWndProc .
				// Remember.
				OnFastPanning=true;

				// This will force BitBlt operation in RenderMapWindow
				RequestFastRefresh();
			}
		} // minimal movement detected
	} // mouse moved with Lbutton (dragging)


	      if(ValidTaskPoint(PanTaskEdit))
    	  {
	        LockTaskData();
		    PanLongitude =  WayPointList[ Task[PanTaskEdit].Index].Longitude ;
		    PanLatitude  =  WayPointList[ Task[PanTaskEdit].Index].Latitude ;

            if( (mode.Is(Mode::MODE_PAN)) || (mode.Is(Mode::MODE_TARGET_PAN)) )
		    {
		      {
		        if(Task[PanTaskEdit].Index != RESWP_PANPOS)
		        {
		 	      RealActiveWaypoint = Task[PanTaskEdit].Index;
			      LKASSERT(ValidWayPoint(Task[PanTaskEdit].Index));
		 	      WayPointList[RESWP_PANPOS].Latitude = WayPointList[RealActiveWaypoint].Latitude;
		 	      WayPointList[RESWP_PANPOS].Longitude = WayPointList[RealActiveWaypoint].Longitude;
		 	      WayPointList[RESWP_PANPOS].Altitude = WayPointList[RealActiveWaypoint].Altitude;

		          Task[PanTaskEdit].Index =RESWP_PANPOS;
				  RefreshMap();
		        }
		      }
		    }
            UnlockTaskData();
    	  }
	break;


#define WM_USER_LONGTIME_CLICK_TIMER  50
    case WM_LBUTTONDOWN:
_buttondown:

//StartupStore(_T("... Ulli: WM_LBUTTONDOWN: %i %i\n"),wParam,  lParam);
#ifdef LONGCLICK_FEEDBACK
      lOld = lParam;
#endif
      if (LockModeStatus) break;
      pressed = true;
      tsDownTime.update();
#ifdef LONGCLICK_FEEDBACK
      if(!mode.Is(Mode::MODE_PAN))
      {
  	    SetTimer(	hWnd,WM_USER_LONGTIME_CLICK_TIMER , AIRSPACECLICK , NULL);
      }
#endif
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

    case WM_TIMER:
#ifdef LONGCLICK_FEEDBACK
      if(wParam==WM_USER_LONGTIME_CLICK_TIMER)
	  {
	  	if (EnableSoundModes)
	 	  PlayResource(TEXT("IDR_WAV_MM1"));
	    KillTimer(hWnd,WM_USER_LONGTIME_CLICK_TIMER);
	//    StartupStore(_T("... Ulli: WM_TIMER: %i %i\n"),0,  lOld);

	    lparam_X = (int) LOWORD(lOld);
	    lparam_Y = (int) HIWORD(lOld);

	  }
#endif
	    break;

    case WM_LBUTTONUP:
  //  	StartupStore(_T("... Ulli: WM_LBUTTONUP! %i %i\n"),wParam,  lParam);
#ifdef LONGCLICK_FEEDBACK
	   KillTimer(hWnd, WM_USER_LONGTIME_CLICK_TIMER );
#endif
	if (LockModeStatus) break;
	if(!pressed) break;
	  pressed = false;
	// Mouse released DURING panning, full redraw requested.
	// Otherwise process virtual keys etc. as usual
	if (MouseWasPanMoving) {
		MouseWasPanMoving=false;
		OnFastPanning=false;
		ignorenext=false;
		RefreshMap();
		tsDownTime=0; // otherwise we shall get a fake click passthrough
		break;
	}
	if (ignorenext||tsDownTime==0) { 
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

	tsUpTime.update();
	DownUpInterval=tsUpTime-tsDownTime;
	tsDownTime=0; // do it once forever

	gestDir=LKGESTURE_NONE; gestDist=-1;

	gestY=YstartScreen-lparam_Y;
	gestX=XstartScreen-lparam_X;

	if ( ( dontdrawthemap && (lparam_Y <Y_BottomBar)) || ((MapSpaceMode==MSM_MAP)) ) { 

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
				if (IsMultiMapNoMain()) {
					if (gestY>0)
						gestDir=LKGESTURE_UP;
					else
						gestDir=LKGESTURE_DOWN;
				} else {
					if (gestY>0)
						gestDir=IphoneGestures?LKGESTURE_DOWN:LKGESTURE_UP;
					else
						gestDir=IphoneGestures?LKGESTURE_UP:LKGESTURE_DOWN;
				}
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
		if ( DownUpInterval.totalMilliseconds() <= DOUBLECLICKINTERVAL) {
goto_menu:
			#ifndef DISABLEAUDIO
                	if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			ShowMenu();
			break;
		} else
		// Long click on aircraft icon, toggle thermal mode
		//
		if ( DownUpInterval.totalMilliseconds() >=VKLONGCLICK) { // in Defines.h
			#if 0
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
			#endif
		} else {
			// We are here in any case only when dwInterval is <VKLONGCLICK
			if (DownUpInterval.totalMilliseconds() >= CustomKeyTime) {
				if (!CustomKeyHandler(CKI_BOTTOMICON)) goto goto_menu;
			}
			break;
		}

      // end aircraft icon check				
      } 

	// MultiMap custom specials, we use same geometry of MSM_MAP

if((DownUpInterval.totalMilliseconds() < AIRSPACECLICK) || ISPARAGLIDER)
{
	if (NOTANYPAN && IsMultiMapCustom() ) {
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
	if (NOTANYPAN && IsMultiMapShared()) {
	if ( (lparam_X <= P_UngestureLeft.x) && (lparam_Y <= P_UngestureLeft.y) ) {
		
		if (!CustomKeyHandler(CKI_TOPLEFT)) {
			// we click in any case to let the user have a response feeling
			if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			MapWindow::zoom.EventScaleZoom(1);
			return TRUE;
		}
		MapWindow::RefreshMap();
		break;
	}
	}
}
      if (NOTANYPAN && IsMultiMapShared()) {
      if (ISPARAGLIDER) {
	// Use the compass to pullup UTM informations to paragliders
	if ( (lparam_X > P_UngestureRight.x) && (lparam_Y <= P_UngestureRight.y) ) {

		if (DownUpInterval.totalMilliseconds() >= DOUBLECLICKINTERVAL) {

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
				// we click in any case to let the user have a response feeling
				if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				MapWindow::zoom.EventScaleZoom(1);
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
	if (dontdrawthemap||(NOTANYPAN && IsMultiMapShared())) {

		if ( gestDir == LKGESTURE_LEFT) {
			ProcessVirtualKey(lparam_X,lparam_Y,DownUpInterval.totalMilliseconds(),LKGESTURE_LEFT);
			break;
		}
		if ( gestDir == LKGESTURE_RIGHT) {
			ProcessVirtualKey(lparam_X,lparam_Y,DownUpInterval.totalMilliseconds(),LKGESTURE_RIGHT);
			break;
		}

		// basically we do this only to process left/right gestures in main map as well
		// In SIM mode, gestDir are not detected because we manage simulation movements
		if (MapSpaceMode==MSM_MAP) goto _continue;

		if ( gestDir == LKGESTURE_UP) {
			ProcessVirtualKey(lparam_X,lparam_Y,DownUpInterval.totalMilliseconds(),LKGESTURE_UP);
			break;
		}
		if ( gestDir == LKGESTURE_DOWN) {
			ProcessVirtualKey(lparam_X,lparam_Y,DownUpInterval.totalMilliseconds(),LKGESTURE_DOWN);
			break;
		}

		// We are here when lk8000, and NO moving map displayed: virtual enter, virtual up/down, or 
		// navbox operations including center key.
		wParam=ProcessVirtualKey(lparam_X,lparam_Y,DownUpInterval.totalMilliseconds(),LKGESTURE_NONE);
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

_continue:
      
	// if clicking on navboxes, process fast virtual keys
	// maybe check LK8000 active?
	// This point is selected when in MapSpaceMode==MSM_MAP, i.e. lk8000 with moving map on.
	if (  DrawBottom && (lparam_Y >= Y_BottomBar) && !mode.AnyPan() ) {
		wParam=ProcessVirtualKey(lparam_X,lparam_Y,DownUpInterval.totalMilliseconds(),LKGESTURE_NONE);
                #ifdef DEBUG_MAPINPUT
		DoStatusMessage(_T("DBG-034 navboxes")); 
                #endif
		if (wParam!=0) {
			DoStatusMessage(_T("ERR-034 Invalid Virtual Key")); 
			break;
		}
		break;
	}


      if (DownUpInterval == 0) {
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
	_stprintf(buf,_T("XY=%d,%d dist=%S Up=%ld Down=%ld Int=%ld"),lparam_X,lparam_Y,sbuf,dwUpTime,dwDownTime,dwInterval);
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
		if (DownUpInterval.totalMilliseconds() >= FASTDOUBLECLICK) { // fast dblclk required here.
			#ifdef DEBUG_VIRTUALKEYS // 100320
			wParam=ProcessVirtualKey(lparam_X,lparam_Y,dwInterval,LKGESTURE_NONE);
			if (wParam==0) {
				DoStatusMessage(_T("P00 Virtual Key 0")); 
				break;
			}
			#else
			ProcessVirtualKey(lparam_X,lparam_Y,DownUpInterval.totalMilliseconds(),LKGESTURE_NONE);
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
	//
	// Notice: Parser is not active because there is no real gps. We can use GPS_INFO.
	//
	double newbearing;
	double oldbearing = DrawInfo.TrackBearing;
	double minspeed = 1.1*GlidePolar::Vminsink;
	DistanceBearing(Ystart, Xstart, Ylat, Xlat, NULL, &newbearing);
	if ((fabs(AngleLimit180(newbearing-oldbearing))<30) || (DrawInfo.Speed<minspeed)) {
		// sink we shall be sinking, lets raise the altitude when using old simulator interface
		if ( (DerivedDrawInfo.TerrainValid) && ( DerivedDrawInfo.AltitudeAGL <0 ))
			GPS_INFO.Altitude=DerivedDrawInfo.TerrainAlt;
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
			if(DownUpInterval.totalMilliseconds() < AIRSPACECLICK) { // original and untouched interval
				{
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
						MapWindow::zoom.EventScaleZoom(1);
						if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
					} else {
						if (lparam_Y>Y_Down) {
							MapWindow::zoom.EventScaleZoom(-1);
							if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
						} else {
							// process center key, do nothing 
							// v5 obsoleted, should not happen
							break;
						}
					}
					tsDownTime= 0L;

					return TRUE; 
				}
			} else {

				// Select airspace on moving map only if they are visible
				// 120526 moved out of anypan, buggy because we want airspace selection with priority
				if (/*IsMultimapAirspace() &&*/ Event_InteriorAirspaceDetails(Xstart, Ystart))
					break;

				if (!mode.AnyPan()) {
					// match only center screen
#ifdef CENTER_CUSTOMKEY
					if (  (abs(lparam_X-P_HalfScreen.x) <NIBLSCALE(100)) && 
					      (abs(lparam_Y-P_HalfScreen.y) <NIBLSCALE(100)) ) {

						if (CustomKeyHandler(CKI_CENTERSCREEN)) break;
					}
#endif
				}
				break;
			}
      } // !TargetPan

      break;


    #if defined(PNA)
    case WM_KEYDOWN:
    #else
	#if (WINDOWSPC>0)
	case WM_KEYDOWN:
		if (!Debounce(50)) return FALSE;
	#else
    case WM_KEYUP: 
	#endif
    #endif

	#if 0	// Show keypressed for testing only
	#ifdef PNA
	TCHAR keypressed[30];
	_stprintf(keypressed,_T("Key=0x%x =d %d"),wParam,wParam);
	DoStatusMessage(keypressed);
	#endif
	#endif

	//
	// Special SIM mode keys for PC
	//
	#if (WINDOWSPC>0)
	extern void SimFastForward(void);
	if (SIMMODE && IsMultiMapShared() && (!ReplayLogger::IsEnabled())) {
		short nn;
		switch(wParam) {
			case 0x21:	// VK_PRIOR PAGE UP
				nn=GetKeyState(VK_SHIFT);
				if (nn<0) {
				  if (Units::GetUserAltitudeUnit() == unFeet)
					GPS_INFO.Altitude += 45.71999999*10;
				  else
					GPS_INFO.Altitude += 10*10;
				}
				else
				{
				  if (Units::GetUserAltitudeUnit() == unFeet)
					GPS_INFO.Altitude += 45.71999999;
				  else
					GPS_INFO.Altitude += 10;
				}
				TriggerGPSUpdate();
				return TRUE;
				break;
			case 0x22:	// VK_NEXT PAGE DOWN
				nn=GetKeyState(VK_SHIFT);
				if (nn<0) {
				  if (Units::GetUserAltitudeUnit() == unFeet)
					GPS_INFO.Altitude -= 45.71999999*10;
				  else
					GPS_INFO.Altitude -= 10*10;
				}
				else
				{
				  if (Units::GetUserAltitudeUnit() == unFeet)
					GPS_INFO.Altitude -= 45.71999999;
				  else
					GPS_INFO.Altitude -= 10;
				}
				if (GPS_INFO.Altitude<=0) GPS_INFO.Altitude=0;
				TriggerGPSUpdate();
				return TRUE;
				break;
			case 0x26:	// VK_UP
				nn=GetKeyState(VK_SHIFT);
				if (nn<0) {
					SimFastForward();
				} else {
					InputEvents::eventChangeGS(_T("kup"));
				}
				TriggerGPSUpdate();
				return TRUE;
				break;
			case 0x28:	// VK_DOWN
				InputEvents::eventChangeGS(_T("kdown"));
				TriggerGPSUpdate();
				return TRUE;
				break;
			case 0x25:	// VK_LEFT
				nn=GetKeyState(VK_SHIFT);
				if (nn<0) {
						GPS_INFO.TrackBearing -= 0.1;

				} else 
				{
					GPS_INFO.TrackBearing -= 5;
				}
				if (GPS_INFO.TrackBearing<0) GPS_INFO.TrackBearing+=360;
				else if (GPS_INFO.TrackBearing>359) GPS_INFO.TrackBearing-=360;

				TriggerGPSUpdate();
				return TRUE;
				break;
			case 0x27:	// VK_RIGHT

				nn=GetKeyState(VK_SHIFT);
				if (nn<0) {
					GPS_INFO.TrackBearing += 0.1;
				} else 
				{
					GPS_INFO.TrackBearing += 5;
				}
				if (GPS_INFO.TrackBearing<0) GPS_INFO.TrackBearing+=360;
				else if (GPS_INFO.TrackBearing>359) GPS_INFO.TrackBearing-=360;

				TriggerGPSUpdate();
				return TRUE;
				break;
		}
	}

	extern double ReplayTime;
	if (SIMMODE && IsMultiMapShared() && ReplayLogger::IsEnabled()) {
		switch(wParam) {
			case 0x21:	// VK_PRIOR PAGE UP
				ReplayTime+=300;
				return TRUE;
			case 0x26:	// VK_UP
				ReplayLogger::TimeScale++;
				return TRUE;
				break;
			case 0x28:	// VK_DOWN
				if (ReplayLogger::TimeScale>0) ReplayLogger::TimeScale--;
				if (ReplayLogger::TimeScale<0) ReplayLogger::TimeScale=0; // to be safe
				return TRUE;
				break;
			case 0x27:	// VK_RIGHT
				ReplayTime+=60;
				return TRUE;
		}
	}

	#endif


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
	if ( GlobalModelType == MODELTYPE_PNA_MINIMAP ) {
		switch(wParam) {

			// Button A is generating a C
			case 67:
				goto _key_enter;
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
			// v5> careful not the same as _key gotos.
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
				goto _key_previous_mode;
				break;
			case 39:
				goto _key_next_mode;
				break;

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
	if ( GlobalModelType == MODELTYPE_PNA_GENERIC_BTK1 ) {
		#if (WINDOWSPC<1)
		if (!Debounce(KEYDEBOUNCE)) return FALSE;
		#endif
		tsDownTime= 0L;
		switch(wParam) {
			//
			// THE BOTTOM BAR
			//
			case '1':
			case '2':
				goto _key_bottombar_previous;
				break;
				
			case 'a':
			case 'A':
			case 's':
			case 'S':
			case 'q':
			case 'Q':
			case 'W':
			case 'w':
				goto _key_next_mode;
				break;

			case 'z':
			case 'Z':
			case 'x':
			case 'X':
				goto _key_bottombar_next;
				break;

			//
			// THE LEFT SCREEN
			//
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				goto _key_previous_page;
				break;

			//
			// THE RIGHT SCREEN
			//
			case 0x20: // space
			case 172:
			case 191:
				goto _key_next_page;
				break;


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
				goto _key_up;
				break;

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
				goto _key_down;
				break;

			//
			// THE TOP-LEFT
			//
			case '0':
			case '9':
			case 'P':
			case 'p':
			case 'O':
			case 'o':
				goto _key_topleft;
				goto _key_topleft;
				break;

			//
			// THE TOP-RIGHT
			//
			case 13: // enter
			case  8: // BS! not the number
			case 'L':
			case 'l':
			case 190:
			case 186:
				goto _key_topright;
				break;

			//
			// the special Shift corner keys
			//
			case 16:
				goto _key_enter;
				break;

			default:
				break;
		}
	}

	//
	// BTK2 is representing the screen in portrait mode. Space is on the right.
	// Keyboard type 2.
	//
	if ( GlobalModelType == MODELTYPE_PNA_GENERIC_BTK2 ) {
		#if (WINDOWSPC<1)
		if (!Debounce(KEYDEBOUNCE)) return FALSE;
		#endif
		tsDownTime= 0L;
		switch(wParam) {
			//
			// THE BOTTOM BAR
			//
			case '1':
			case '2':
				goto _key_bottombar_previous;
				break;
				
			case 'a':
			case 'A':
			case 's':
			case 'S':
			case 'q':
			case 'Q':
			case 'W':
			case 'w':
				goto _key_next_mode;
				break;

			case 'z':
			case 'Z':
			case 'x':
			case 'X':
				goto _key_bottombar_next;
				break;

			//
			// THE LEFT SCREEN
			//
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				goto _key_previous_page;
				break;

			//
			// THE RIGHT SCREEN
			//
			case 0x20: // space
				goto _key_next_page;
				break;


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
				goto _key_up;
				break;

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
				goto _key_down;
				break;

			//
			// THE TOP-LEFT
			//
			case '0':
			case '9':
			case 'P':
			case 'p':
			case 'O':
			case 'o':
				goto _key_topleft;
				goto _key_topleft;
				break;

			//
			// THE TOP-RIGHT
			//
			case 13: // enter
			case 8: // Del
			case 'L':
			case 'l':
			case 190:
			case 186:
				goto _key_topright;
				break;

			//
			// the special Shift and Ctrol
			//
			case 0x10:
			case 0x11:
				goto _key_enter;
				break;

			default:
				break;
		}
	}
	//
	// BTK3 is representing the screen in portrait mode. Space is on the right.
	// Keyboard type 3.
	//
	if ( GlobalModelType == MODELTYPE_PNA_GENERIC_BTK3 ) {
		#if (WINDOWSPC<1)
		if (!Debounce(KEYDEBOUNCE)) return FALSE;
		#endif
		tsDownTime= 0L;
		switch(wParam) {
			//
			// THE BOTTOM BAR
			//
			case '1':
			case '2':
				goto _key_bottombar_previous;
				break;
				
			case 'a':
			case 'A':
			case 's':
			case 'S':
			case 'q':
			case 'Q':
			case 'W':
			case 'w':
				goto _key_next_mode;
				break;

			case 'z':
			case 'Z':
			case 'x':
			case 'X':
				goto _key_bottombar_next;
				break;

			//
			// THE LEFT SCREEN
			//
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				goto _key_previous_page;
				break;

			//
			// THE RIGHT SCREEN
			//
			case 0x20: // space
			case 0xbc: // 188
			case 0xbe: // 190
				goto _key_next_page;
				break;


			//
			// THE CENTER-UP
			//
			case 'I':
			case 'i':
			case 'K':
			case 'k':
			case  0xba: // 186

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
				goto _key_up;
				break;

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
				goto _key_down;
				break;

			//
			// THE TOP-LEFT
			//
			case '0':
			case '9':
			case 'P':
			case 'p':
			case 'O':
			case 'o':
				goto _key_topleft;
				goto _key_topleft;
				break;

			//
			// THE TOP-RIGHT
			//
			case 13: // enter
			case 8: // Del
			case 'L':
			case 'l':
			case 0xde: // 222
				goto _key_topright;
				break;

			//
			// the special Shift and Ctrol
			//
			case 0x10:
			case 0x11:
				goto _key_enter;
				break;

			default:
				break;
		}
	}

	//
	// BTKA is for using 1 2 3 4 5 6 7 8 9 0  assigned to customkeys of customenu.
	// universal Keyboard type
	//
	if ( GlobalModelType == MODELTYPE_PNA_GENERIC_BTKA ) {
		#if (WINDOWSPC<1)
		if (!Debounce(KEYDEBOUNCE)) return FALSE;
		#endif
		tsDownTime= 0L;
		switch(wParam) {
			case '1':
				CustomKeyHandler(CustomMenu1+1000);
				break;
			case '2':
				CustomKeyHandler(CustomMenu2+1000);
				break;
			case '3':
				CustomKeyHandler(CustomMenu3+1000);
				break;
			case '4':
				CustomKeyHandler(CustomMenu4+1000);
				break;
			case '5':
				CustomKeyHandler(CustomMenu5+1000);
				break;
			case '6':
				CustomKeyHandler(CustomMenu6+1000);
				break;
			case '7':
				CustomKeyHandler(CustomMenu7+1000);
				break;
			case '8':
				CustomKeyHandler(CustomMenu8+1000);
				break;
			case '9':
				CustomKeyHandler(CustomMenu9+1000);
				break;
			case '0':
				CustomKeyHandler(CustomMenu10+1000);
				break;
			default:
				break;
		}
	}

	//
	// BTKB is for using 1 2 3 4 5 6 7 8 9 0  assigned to customkeys of customenu.
	// universal Keyboard type
	//
	if ( GlobalModelType == MODELTYPE_PNA_GENERIC_BTKB ) {
		#if (WINDOWSPC<1)
		if (!Debounce(KEYDEBOUNCE)) return FALSE;
		#endif
		tsDownTime= 0L;
		switch(wParam) {
			case '1':
			case '2':
			case 'Q':
			case 'W':
			case 'q':
			case 'w':
				CustomKeyHandler(CustomMenu1+1000);
				break;
			case '3':
			case '4':
			case 'e':
			case 'E':
			case 'r':
			case 'R':
				CustomKeyHandler(CustomMenu2+1000);
				break;
			case '5':
			case '6':
			case 't':
			case 'T':
			case 'Y':
			case 'y':
				CustomKeyHandler(CustomMenu3+1000);
				break;
			case '7':
			case '8':
			case 'u':
			case 'i':
			case 'U':
			case 'I':
				CustomKeyHandler(CustomMenu4+1000);
				break;
			case '9':
			case '0':
			case 'O':
			case 'o':
			case 'P':
			case 'p':
				CustomKeyHandler(CustomMenu5+1000);
				break;
			case 'A':
			case 'Z':
			case 'S':
			case 'X':
			case 'a':
			case 'z':
			case 's':
			case 'x':
				CustomKeyHandler(CustomMenu6+1000);
				break;
			case 'D':
			case 'F':
			case 'C':
			case 'V':
			case 'd':
			case 'f':
			case 'c':
			case 'v':
				CustomKeyHandler(CustomMenu7+1000);
				break;
			case 'G':
			case 'H':
			case 'B':
			case 'N':
			case 'g':
			case 'h':
			case 'b':
			case 'n':
				CustomKeyHandler(CustomMenu8+1000);
				break;
			case 'J':
			case 'j':
			case 'K':
			case 'k':
			case 'M':
			case 'm':
			case 188:
				CustomKeyHandler(CustomMenu9+1000);
				break;
			case 13: // enter
			case 8: // Del
			case 'L':
			case 'l':
			case 190:
				CustomKeyHandler(CustomMenu10+1000);
				break;
			default:
				break;
		}
	}


// this goto line after the end of transcoding code!
goto _skipout;
	
	// -------------------------------------------------------------------------
	// Key functions, called from a case switch, must all end with a return or a break.
	// They are all accessed with an ugly goto. 
	// -------------------------------------------------------------------------

_key_next_mode:
	NextModeIndex();
	MapWindow::RefreshMap();
	SoundModeIndex();
	return TRUE;

_key_previous_mode:
	PreviousModeIndex();
	MapWindow::RefreshMap();
	SoundModeIndex();
	return TRUE;

_key_up:
	//
	// UP
	//
	if ( !MapWindow::mode.AnyPan()&&MapSpaceMode!=1) { // dontdrawthemap
		if (MapSpaceMode<=MSM_MAP) {
			return TRUE;
		}
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		LKevent=LKEVENT_UP;
		MapWindow::RefreshMap();
	} else {
		// careful! zoom works inverted!!
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		MapWindow::zoom.EventScaleZoom(1);
	}
	return TRUE;

_key_down:
	//
	// DOWN
	//
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
		// careful! zoom works inverted!!
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
		MapWindow::zoom.EventScaleZoom(-1);
	}
	return TRUE;


_key_next_page:
	// next page
	NextModeType();
	MapWindow::RefreshMap();
	if (ModeIndex!=LKMODE_MAP)
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
	return TRUE;

_key_previous_page:
	// previous page
	PreviousModeType();
	MapWindow::RefreshMap();
	if (ModeIndex!=LKMODE_MAP)
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
	return TRUE;

#if 0 // unused
_key_gesture_up:
	ProcessVirtualKey(lparam_X,lparam_Y,dwInterval,LKGESTURE_UP);
	MapWindow::RefreshMap();
	return TRUE;
#endif

_key_gesture_down:
	ProcessVirtualKey(lparam_X,lparam_Y,DownUpInterval.totalMilliseconds(),LKGESTURE_DOWN);
	MapWindow::RefreshMap();
	return TRUE;


_key_enter:
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
	} 
	#if 0 // no center screen ck anymore
	else {
		if (CustomKeyHandler(CKI_CENTERSCREEN)) {
			#ifndef DISABLEAUDIO
			if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
		}
		// MapWindow::RefreshMap();
		return TRUE;
	}
	#endif
	break;

_key_topright:
	if (NOTANYPAN && IsMultiMapCustom() ) {
		LKevent=LKEVENT_TOPRIGHT;
		MapWindow::RefreshMap();
		return TRUE;
	}
	if (NOTANYPAN && IsMultiMapShared()) {
		CustomKeyHandler(CKI_TOPRIGHT);
	}
	if ( isListPage() ) goto _key_gesture_down;
	break;

_key_topleft:
	if (NOTANYPAN && IsMultiMapCustom() ) {
		LKevent=LKEVENT_TOPLEFT;
		MapWindow::RefreshMap();
		return TRUE;
	}
	if (NOTANYPAN && IsMultiMapShared()) {
		CustomKeyHandler(CKI_TOPLEFT);
	}
	if ( isListPage() ) goto _key_gesture_down;

	break;

#if 0
_key_topcenter:
	if ( isListPage() )
		goto _key_gesture_down;
	else
		goto _key_topright;
	break;
#endif

#if 0
_key_overtarget_rotate:
	RotateOvertarget();
	MapWindow::RefreshMap();
	return TRUE;
#endif

_key_bottombar_next:
	BottomBarChange(true);  // next bb
	BottomSounds();
	MapWindow::RefreshMap();
	return TRUE;

_key_bottombar_previous:
	BottomBarChange(false); // prev bb
	BottomSounds();
	MapWindow::RefreshMap();
	return TRUE;




	// End of key manager

_skipout:
	tsDownTime= 0L; // removable? check
	InputEvents::processKey(wParam);
	tsDownTime= 0L;
	return TRUE; 
    }

  return (DefWindowProc (hWnd, uMsg, wParam, lParam));
}


static bool isListPage(void) {
  if ( 
	MapSpaceMode==MSM_COMMON || MapSpaceMode==MSM_RECENT ||
	MapSpaceMode==MSM_LANDABLE || MapSpaceMode==MSM_AIRPORTS ||
	MapSpaceMode==MSM_NEARTPS || MapSpaceMode==MSM_AIRSPACES ||
	MapSpaceMode==MSM_THERMALS || MapSpaceMode==MSM_TRAFFIC 
     )
	return true;
  else
	return false;
}
