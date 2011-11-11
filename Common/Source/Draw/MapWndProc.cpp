/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "MapWindow.h"
#include "McReady.h"
#include "InputEvents.h"
#include "Modeltype.h"
#include "LKObjects.h"
#include "Bitmaps.h"
#include "RGB.h"

using std::min;
using std::max;


#ifdef DEBUG
#define DEBUG_VIRTUALKEYS
#endif


#define DONTDRAWTHEMAP !mode.AnyPan()&&MapSpaceMode!=MSM_MAP
#define MAPMODE8000    !mode.AnyPan()&&MapSpaceMode==MSM_MAP


COLORREF taskcolor = RGB_TASKLINECOL; // 091216
static bool ignorenext=false;

MapWindow::Zoom MapWindow::zoom;
MapWindow::Mode MapWindow::mode;

HBRUSH  MapWindow::hAboveTerrainBrush;
HPEN    MapWindow::hpCompassBorder;

int MapWindow::SnailWidthScale = 16;
int MapWindow::ScaleListCount = 0;
double MapWindow::ScaleList[];
int MapWindow::ScaleCurrent;

POINT MapWindow::Orig_Screen;

RECT MapWindow::MapRect;

HBITMAP MapWindow::hDrawBitMap = NULL;
HBITMAP MapWindow::hDrawBitMapTmp = NULL;
HBITMAP MapWindow::hMaskBitMap = NULL;
HDC MapWindow::hdcDrawWindow = NULL;
HDC MapWindow::hdcScreen = NULL;
HDC MapWindow::hDCTemp = NULL;
HDC MapWindow::hDCMask = NULL;

double MapWindow::PanLatitude = 0.0;
double MapWindow::PanLongitude = 0.0;

bool MapWindow::targetMoved = false;
double MapWindow::targetMovedLat = 0;
double MapWindow::targetMovedLon = 0;

int MapWindow::TargetPanIndex = 0;
double MapWindow::TargetZoomDistance = 500.0;
bool MapWindow::EnableTrailDrift=false;
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

// 16 is number of airspace types
int      MapWindow::iAirspaceBrush[AIRSPACECLASSCOUNT] = 
  {2,0,0,0,3,3,3,3,0,3,2,3,3,3,3,3};
int      MapWindow::iAirspaceColour[AIRSPACECLASSCOUNT] = 
  {5,0,0,10,0,0,10,2,0,10,9,3,7,7,7,10};
int      MapWindow::iAirspaceMode[AIRSPACECLASSCOUNT] =
  {0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0};

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
HBRUSH MapWindow::hbFinalGlideBelowLandable;
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
HPEN MapWindow::hpFinalGlideBelowLandable;
HPEN MapWindow::hpMapScale;
HPEN MapWindow::hpMapScale2;
HPEN MapWindow::hpTerrainLine;
HPEN MapWindow::hpTerrainLineBg;
HPEN MapWindow::hpStartFinishThick;
HPEN MapWindow::hpStartFinishThin;
HPEN MapWindow::hpVisualGlideLightBlack; // VENTA3
HPEN MapWindow::hpVisualGlideHeavyBlack; // VENTA3
HPEN MapWindow::hpVisualGlideLightRed; // VENTA3
HPEN MapWindow::hpVisualGlideHeavyRed; // VENTA3

bool MapWindow::MapDirty = true;

bool MapWindow::ForceVisibilityScan = false;

NMEA_INFO MapWindow::DrawInfo;
DERIVED_INFO MapWindow::DerivedDrawInfo;

extern void ShowMenu();



LRESULT CALLBACK MapWindow::MapWndProc (HWND hWnd, UINT uMsg, WPARAM wParam,
                                        LPARAM lParam)
{
  int i;
  static double Xstart, Ystart;
  static int XstartScreen, YstartScreen;
  int X,Y;
  int gestX, gestY, gestDir=LKGESTURE_NONE, gestDist=-1;
  double Xlat, Ylat;
  double distance;
  int width = (int) LOWORD(lParam);
  int height = (int) HIWORD(lParam);
  
  static DWORD dwDownTime= 0L, dwUpTime= 0L, dwInterval= 0L;

  bool dontdrawthemap=(DONTDRAWTHEMAP);
  bool mapmode8000=(MAPMODE8000);

  static short navboxesY;
  // Attention... this is duplicated inside Utils2, I am lazy 
  // apparently only #include is duplicated, so no problems
  static bool doinit=true;
  static int AIRCRAFTMENUSIZE=0, COMPASSMENUSIZE=0;

  navboxesY=(MapWindow::MapRect.bottom-MapWindow::MapRect.top)-BottomSize-NIBLSCALE(2); // BUGFIX 091125


  switch (uMsg)
    {
    case WM_ERASEBKGND:
      return TRUE;
    case WM_SIZE:

      hDrawBitMap = CreateCompatibleBitmap (hdcScreen, width, height);
      SelectObject(hdcDrawWindow, (HBITMAP)hDrawBitMap);

      hDrawBitMapTmp = CreateCompatibleBitmap (hdcScreen, width, height);
      SelectObject(hDCTemp, (HBITMAP)hDrawBitMapTmp);

      hMaskBitMap = CreateBitmap(width+1, height+1, 1, 1, NULL);
      SelectObject(hDCMask, (HBITMAP)hMaskBitMap);

      break;

    case WM_CREATE:

      hdcScreen = GetDC(hWnd);
      hdcDrawWindow = CreateCompatibleDC(hdcScreen);
      hDCTemp = CreateCompatibleDC(hdcDrawWindow);
      hDCMask = CreateCompatibleDC(hdcDrawWindow);
  
      AlphaBlendInit();

      LKLoadFixedBitmaps();
    
      hInvBackgroundBrush[0] = LKBrush_White;
      hInvBackgroundBrush[1] = LKBrush_LightGrey;
      hInvBackgroundBrush[2] = LKBrush_LcdGreen;
      hInvBackgroundBrush[3] = LKBrush_LcdDarkGreen;
      hInvBackgroundBrush[4] = LKBrush_Grey; 
      hInvBackgroundBrush[5] = LKBrush_Lake;
      hInvBackgroundBrush[6] = LKBrush_Emerald;
      hInvBackgroundBrush[7] = LKBrush_DarkSlate;
      hInvBackgroundBrush[8] = LKBrush_RifleGrey;
      hInvBackgroundBrush[9] = LKBrush_Black;

      for (i=0; i<NUMAIRSPACEBRUSHES; i++) {
	hAirspaceBrushes[i] =
	  CreatePatternBrush((HBITMAP)hAirspaceBitmap[i]);
      }
      hAboveTerrainBrush = CreatePatternBrush((HBITMAP)hAboveTerrainBitmap);

	int iwidth;
	iwidth=IBLSCALE(SnailWidthScale);
	hSnailColours[0] = RGB_BLACK;
	hSnailColours[1] = RGB_INDIGO;
	hSnailColours[2] = RGB_INDIGO;
	hSnailColours[3] = RGB_BLUE;
	hSnailColours[4] = RGB_BLUE;
	hSnailColours[5] = RGB_LAKE;
	hSnailColours[6] = RGB_LAKE;
	hSnailColours[7] = RGB_GREY;
	hSnailColours[8] = RGB_GREEN;
	hSnailColours[9] = RGB_GREEN;
	hSnailColours[10] = RGB_ORANGE;
	hSnailColours[11] = RGB_ORANGE;
	hSnailColours[12] = RGB_RED;
	hSnailColours[13] = RGB_RED;
	hSnailColours[14] = RGB_DARKRED;

	hSnailPens[0] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[0]);
	hSnailPens[1] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[1]);
	hSnailPens[2] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[2]);
	hSnailPens[3] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[3]);
	hSnailPens[4] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(2), hSnailColours[4]);
	hSnailPens[5] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), hSnailColours[5]);
	hSnailPens[6] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), hSnailColours[6]);
	hSnailPens[7] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(6), hSnailColours[7]);
	hSnailPens[8] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), hSnailColours[8]);
	hSnailPens[9] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), hSnailColours[9]);
	hSnailPens[10] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[10]);
	hSnailPens[11] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[11]);
	hSnailPens[12] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[12]);
	hSnailPens[13] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[13]);
	hSnailPens[14] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), hSnailColours[14]);

      hpCompassBorder = LKPen_Black_N2;
      hpAircraft = LKPen_White_N3;
      hpAircraftBorder = LKPen_Black_N1;

      hpWind = LKPen_Black_N2;

      hpWindThick = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(4), RGB(255,220,220));

      if (ISGAAIRCRAFT)
	hpBearing = LKPen_GABRG;
      else
	hpBearing = LKPen_Black_N2;

      hpBestCruiseTrack = LKPen_Blue_N1;
      hpCompass = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_BLACK);
      hpThermalBand = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB(0x40,0x40,0xFF));
      hpThermalBandGlider = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB(0x00,0x00,0x30));

      hpFinalGlideBelow = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(0xFF,0xA0,0xA0));	// another light red
      hpFinalGlideBelowLandable = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(255,196,0));	// another dark yellow

      // TODO enhancement: support red/green Color blind
      hpFinalGlideAbove = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(0xA0,0xFF,0xA0));	// another light green

      hpStartFinishThin=LKPen_Red_N1;
      hpMapScale = LKPen_Black_N1;


      hpStartFinishThick=(HPEN)CreatePen(PS_SOLID, NIBLSCALE(5), taskcolor);
      hpMapScale2 = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1)+1, RGB_BLACK);
      // TerrainLine is for shade, Bg is for perimeter
      hpTerrainLine = (HPEN)CreatePen(PS_DASH, (1), RGB(0x30,0x30,0x30));
      hpTerrainLineBg = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB_LCDDARKGREEN);
      hpVisualGlideLightBlack = (HPEN)CreatePen(PS_DASH, (1), RGB_BLACK);
      hpVisualGlideHeavyBlack = (HPEN)CreatePen(PS_DASH, (2), RGB_BLACK);
      hpVisualGlideLightRed = (HPEN)CreatePen(PS_DASH, (1), RGB_RED);
      hpVisualGlideHeavyRed = (HPEN)CreatePen(PS_DASH, (2), RGB_RED);

      hbThermalBand=LKBrush_Emerald;
      hbCompass=LKBrush_White;
      hbBestCruiseTrack=LKBrush_Blue;
      hbFinalGlideBelow=LKBrush_Red;
      hbFinalGlideAbove=LKBrush_Green;
      hbFinalGlideBelowLandable=LKBrush_Orange;
      hbWind=LKBrush_Grey;

      // Scalelist filled up in DrawThread, in function Mapwindow::FillScaleListForEngineeringUnits()
      //ScaleListCount = propGetScaleList(ScaleList, sizeof(ScaleList)/sizeof(ScaleList[0]));
      //zoom.RequestedScale(LimitMapScale(zoom.RequestedScale()));


      // Signal that draw thread can run now
      Initialised = TRUE;

      break;

    case WM_DESTROY:

      ReleaseDC(hWnd, hdcScreen);
      DeleteDC(hdcDrawWindow);
      DeleteDC(hDCTemp);
      DeleteDC(hDCMask);
      DeleteObject(hDrawBitMap);
      DeleteObject(hMaskBitMap);

      AlphaBlendDestroy();

      DeleteObject((HPEN)hpWindThick);

      DeleteObject((HPEN)hpThermalBand);
      DeleteObject((HPEN)hpThermalBandGlider);
      DeleteObject((HPEN)hpFinalGlideAbove);
      DeleteObject((HPEN)hpFinalGlideBelow);
      DeleteObject((HPEN)hpTerrainLine);
      DeleteObject((HPEN)hpTerrainLineBg);
      DeleteObject((HPEN)hpStartFinishThick);

      DeleteObject((HPEN)hpVisualGlideLightBlack); // VENTA3
      DeleteObject((HPEN)hpVisualGlideLightRed); // VENTA3
      DeleteObject((HPEN)hpVisualGlideHeavyRed); // VENTA3
      DeleteObject((HPEN)hpVisualGlideHeavyBlack); // VENTA3
      DeleteObject((HPEN)hpFinalGlideBelowLandable);

      DeleteObject((HPEN)hpCompassBorder);

      for(i=0;i<NUMAIRSPACEBRUSHES;i++)
	{
	  DeleteObject(hAirspaceBrushes[i]);
	}

      for (i=0; i<AIRSPACECLASSCOUNT; i++) {
	DeleteObject(hAirspacePens[i]);
      }
      DeleteObject(hAirspaceBorderPen);

      for (i=0; i<NUMSNAILCOLORS; i++) {
	DeleteObject(hSnailPens[i]);
      }
    
      LKUnloadFixedBitmaps(); // After removing brushes using Bitmaps
      LKUnloadProfileBitmaps(); 
      PostQuitMessage (0);

      break;

    case WM_LBUTTONDBLCLK: 
      // VNT TODO: do not handle this event and remove CS_DBLCLKS in register class.
      // Only handle timed clicks in BUTTONDOWN with no proximity. 
      //
      // Attention please: a DBLCLK is followed by a simple BUTTONUP with NO buttondown.

      dwDownTime = GetTickCount();  
      XstartScreen = LOWORD(lParam); YstartScreen = HIWORD(lParam);

	if (LockModeStatus) {
        	ignorenext=true; // do not interpret the second click of the doubleclick as a real click.
		if (XstartScreen <(MapRect.right-BottomSize-NIBLSCALE(15))) break;
		if (YstartScreen<(MapRect.bottom-MapRect.top-BottomSize-NIBLSCALE(15))) break;
		LockMode(2);
		DoStatusMessage(gettext(_T("_@M964_"))); // SCREEN IS UNLOCKED
	}

      // Careful! If you ignorenext, any event timed as double click of course will be affected.
      // and this means also fast clicking on bottombar!!
      // so first lets see if we are in lk8000 text screens.. 
      if (dontdrawthemap || (mapmode8000 && (YstartScreen >=navboxesY))) {  
		// do not ignore next, let buttonup get the signal
		break;
      }

	// only when activemap (which means newmap is on) is off
      if (ActiveMap) ignorenext=true;
      break;

    case WM_LBUTTONDOWN:
      #ifdef DEBUG_DBLCLK
      DoStatusMessage(_T("BUTTONDOWN MapWindow")); 
      #endif
      if (LockModeStatus) break;
      dwDownTime = GetTickCount();
      // After calling a menu, on exit as we touch the screen we fall back here
      if (ignorenext) {
                #ifdef DEBUG_MAPINPUT
		DoStatusMessage(TEXT("DBG-055 BUTTONDOWN with ignorenext"));
                #endif
		break;
      }
      XstartScreen = LOWORD(lParam); YstartScreen = HIWORD(lParam);
      // TODO VNT move Screen2LatLon in LBUTTONUP after making sure we really need Xstart and Ystart
      // so we save precious milliseconds waiting for BUTTONUP GetTickCount
      Screen2LatLon(XstartScreen, YstartScreen, Xstart, Ystart);

      LKevent=LKEVENT_NONE; // CHECK FIX TODO VENTA10  probably useless 090915

      FullScreen();
      break;

    case WM_LBUTTONUP:
	if (LockModeStatus) break;
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
      RECT rc;
      dwUpTime = GetTickCount(); 
      dwInterval=dwUpTime-dwDownTime;
      dwDownTime=0; // do it once forever

      GetClientRect(hWnd,&rc);

      X = LOWORD(lParam); Y = HIWORD(lParam); 

	gestY=YstartScreen-Y;
	gestX=XstartScreen-X;

	if (  dontdrawthemap && (Y <(rc.bottom-BottomSize)) ) { 

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

        if (AATEnabled && mode.Is(Mode::MODE_TARGET_PAN)) {
          Screen2LatLon(X, Y, Xlat, Ylat);
          LockTaskData();
          targetMoved = true;
          targetMovedLat = Ylat;
          targetMovedLon = Xlat;
          UnlockTaskData();
          break;
        }

      // Process Active Icons
	if (doinit) {
		#include "./LKinclude_menusize.cpp"
		doinit=false;
	}

	short topicon;
	if (DrawBottom) topicon=MapRect.bottom-MapRect.top-BottomSize-14; // 100305
		else
			topicon=MapRect.bottom-MapRect.top-AIRCRAFTMENUSIZE;

if ( (X > ((MapRect.right-MapRect.left)- AIRCRAFTMENUSIZE)) &&
   (Y > topicon) ) {

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
	if (mapmode8000) { 
	if ( (X <= (MapRect.left + COMPASSMENUSIZE)) && (Y <= (MapRect.top+COMPASSMENUSIZE)) ) {
		if (!CustomKeyHandler(CKI_TOPLEFT)) {
			#ifndef DISABLEAUDIO
         		 if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
			#endif
			wParam = 0x26; 
			// we can have problems if fast double clicks?
			// zoom in
			InputEvents::processKey(wParam);
			return TRUE;
		}
		MapWindow::RefreshMap();
		break;
	}

      if (ISPARAGLIDER) {
	// Use the compass to pullup UTM informations to paragliders
	if ( (X > ((MapRect.right-MapRect.left)- COMPASSMENUSIZE)) && (Y <= MapRect.top+COMPASSMENUSIZE) ) {

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
	else { 
		if ( (X > ((MapRect.right-MapRect.left)- COMPASSMENUSIZE)) && (Y <= MapRect.top+COMPASSMENUSIZE) ) {
			if (!CustomKeyHandler(CKI_TOPRIGHT)) {
				#ifndef DISABLEAUDIO
         			if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
				#endif
				wParam = 0x26; 
				// we can have problems if fast double clicks?
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
			ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_UP);
			break;
		}
		if ( gestDir == LKGESTURE_DOWN) {
			ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_DOWN);
			break;
		}
		if ( gestDir == LKGESTURE_LEFT) {
			ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_LEFT);
			break;
		}
		if ( gestDir == LKGESTURE_RIGHT) {
			ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_RIGHT);
			break;
		}


		// We are here when lk8000, and NO moving map displayed: virtual enter, virtual up/down, or 
		// navbox operations including center key.
		wParam=ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_NONE);
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
	if (  DrawBottom && (Y >= (rc.bottom-BottomSize)) && !mode.AnyPan() ) {
		wParam=ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_NONE);
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
		distance = isqrt4((long)((XstartScreen-X)*(XstartScreen-X)+ (YstartScreen-Y)*(YstartScreen-Y))) /ScreenScale;

	#ifdef DEBUG_VIRTUALKEYS
	TCHAR buf[80]; char sbuf[80];
	sprintf(sbuf,"%.0f",distance);
	wsprintf(buf,_T("XY=%d,%d dist=%S Up=%ld Down=%ld Int=%ld"),X,Y,sbuf,dwUpTime,dwDownTime,dwInterval);
        DoStatusMessage(buf);
	#endif

	// Handling double click passthrough
	// Caution, timed clicks from PC with a mouse are different from real touchscreen devices

      // Process faster clicks here and no precision, but let DBLCLK pass through
      // VK are used in the bottom line in this case, forced on for this situation.
      if (  DrawBottom && (Y >= (rc.bottom-BottomSize)) ) {
		// DoStatusMessage(_T("Click on hidden map ignored")); 

		// do not process virtual key if it is timed as a DBLCLK
		// we want users to get used to double clicking only on infoboxes
		// and avoid triggering unwanted waypoints details
		if (dwInterval >= ( (DOUBLECLICKINTERVAL/2-30) )) { // fast dblclk required here.
			#ifdef DEBUG_VIRTUALKEYS // 100320
			wParam=ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_NONE);
			if (wParam==0) {
				DoStatusMessage(_T("P00 Virtual Key 0")); 
				break;
			}
			#else
			ProcessVirtualKey(X,Y,dwInterval,LKGESTURE_NONE);
			#endif
			break; 
		}
		// do not process click on the underneath window
		break;
      }
      if (dontdrawthemap) break;

      Screen2LatLon(X, Y, Xlat, Ylat);
    
      if (!mode.Is(Mode::MODE_TARGET_PAN) && mode.Is(Mode::MODE_PAN) && (distance>36)) { // TODO FIX should be IBLSCALE 36 instead?
	PanLongitude += Xstart-Xlat;
	PanLatitude  += Ystart-Ylat;
	RefreshMap();
	// disable picking when in pan mode
	break; 
      } 
      else if (SIMMODE && (!mode.Is(Mode::MODE_TARGET_PAN) && (distance>NIBLSCALE(36)))) {
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
	GPS_INFO.TrackBearing = newbearing;
	TriggerGPSUpdate();
      
	break;
      }

      if (!mode.Is(Mode::MODE_TARGET_PAN)) {
		
		//
		// Finally process normally a click on the moving map.
		//
			if(dwInterval < AIRSPACECLICK) { // original and untouched interval
				if (ActiveMap) {
                                  if (Event_NearestWaypointDetails(Xstart, Ystart, 500*zoom.Scale(), false)) {
						break;
					}
				} else {
					int yup, ydown, ytmp;
					ytmp=(int)((MapWindow::MapRect.bottom-MapWindow::MapRect.top-BottomSize)/2);
					yup=ytmp+MapWindow::MapRect.top;
					ydown=MapWindow::MapRect.bottom-BottomSize-ytmp;

					if (Y<yup) {
						// pg UP = zoom in
						wParam = 0x26;
					} else {
						if (Y>ydown) {
							// pg DOWN = zoom out
							wParam = 0x28;
						} 
						else {
							// process center key, do nothing 
							break;
						}
					}
					#ifndef DISABLEAUDIO
					if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
					#endif

					InputEvents::processKey(wParam);
					dwDownTime= 0L;
					return TRUE; 
				}
			} else {
				if (SIMMODE) {
					if (mode.AnyPan()) {
						// match only center screen
						if (  (abs(X-((rc.left+rc.right)/2)) <NIBLSCALE(5)) && 
						      (abs(Y-((rc.bottom+rc.top)/2)) <NIBLSCALE(5)) ) {
							// LKTOKEN  _@M204_ = "Current position updated" 
							DoStatusMessage(gettext(TEXT("_@M204_")));
							GPS_INFO.Latitude=PanLatitude;
							GPS_INFO.Longitude=PanLongitude;
							LastDoRangeWaypointListTime=0; // force DoRange
							break;
						}
						// if we are not long clicking in center screen, before setting new position
						// we check if we are on an airspace for informations, if activemap is on
						if (OnAirSpace && Event_InteriorAirspaceDetails(Xstart, Ystart)) {
							break;
						}
						// Ok, so we reposition the aircraft
						Screen2LatLon(X,Y,PanLongitude,PanLatitude);
						// LKTOKEN  _@M204_ = "Current position updated" 
						DoStatusMessage(gettext(TEXT("_@M204_")));
						GPS_INFO.Latitude=PanLatitude;
						GPS_INFO.Longitude=PanLongitude;
						LastDoRangeWaypointListTime=0;
						break;
					}
				}
				if (!mode.AnyPan()) {
					// Select airspace on moving map only if they are visible, (and activemap optionally!)
					if (OnAirSpace && Event_InteriorAirspaceDetails(Xstart, Ystart))
						break;

					// match only center screen
					if (  (abs(X-((rc.left+rc.right)/2)) <NIBLSCALE(100)) && 
					      (abs(Y-((rc.bottom+rc.top)/2)) <NIBLSCALE(100)) ) {

						if (CustomKeyHandler(CKI_CENTERSCREEN)) break;
					}
				}
				// else ignore
				/*
				// Pan mode airspace details
				if (!OnAirSpace) break; 
				if (Event_InteriorAirspaceDetails(Xstart, Ystart)) {
					break;
				}
				*/
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

      #ifdef VENTA_DEBUG_KEY
      TCHAR ventabuffer[80];
      wsprintf(ventabuffer,TEXT("WMKEY uMsg=%d wParam=%ld lParam=%ld"), uMsg, wParam,lParam);
      DoStatusMessage(ventabuffer);
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

      dwDownTime= 0L;

      InputEvents::processKey(wParam);
      dwDownTime= 0L;
      return TRUE; 
    }

  return (DefWindowProc (hWnd, uMsg, wParam, lParam));
}


