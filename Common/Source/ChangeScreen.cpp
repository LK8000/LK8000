/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "LKInterface.h"
#include "resource.h"
#include "Waypointparser.h"
#include "InfoBoxLayout.h"
#include "LKProfiles.h"
#include "RasterTerrain.h"
#include "Terrain.h"

#include <commctrl.h>
#include <aygshell.h>
#if (WINDOWSPC<1)
#include <sipapi.h>
#endif

#include "InputEvents.h"
#include "Message.h"

#include "LKObjects.h"
#include "Bitmaps.h"
#include "DoInits.h"



//
// Detect if screen resolution and/or orientation has changed
//
bool ScreenHasChanged(void) {

  static int oldSCREENWIDTH=0;
  static int oldSCREENHEIGHT=0;
  static bool doinit=true;
  int x=0,y=0;

  if (doinit) {
	#if (WINDOWSPC>0)
	oldSCREENWIDTH=SCREENWIDTH;
	oldSCREENHEIGHT=SCREENHEIGHT;
	#else
	oldSCREENWIDTH=GetSystemMetrics(SM_CXSCREEN);
	oldSCREENHEIGHT=GetSystemMetrics(SM_CYSCREEN);
	#endif
	doinit=false;
	return false;
  }

  // On PC, simply check for WIDTH and HEIGHT changed
  #if (WINDOWSPC>0)
  x=SCREENWIDTH;
  y=SCREENHEIGHT;
  #else
  x=GetSystemMetrics(SM_CXSCREEN);
  y=GetSystemMetrics(SM_CYSCREEN);
  #endif
  if (x==oldSCREENWIDTH && y==oldSCREENHEIGHT) return false;

  oldSCREENWIDTH=x;
  oldSCREENHEIGHT=y;
  StartupStore(_T("... SCREEN RESOLUTION CHANGE DETECTED: %d x %d\n"),x,y);
  return true;
}


//
// Reinit screen upon resolution/orientation change detected
//
// Test is possible from VirtualKeys.cpp, activating the customkey at line 229
// In this case, enable a testbench development option.
//
void ReinitScreen(void) {

  static int oldSCREENWIDTH=0;
  static int oldSCREENHEIGHT=0;

  RECT WindowSize, rc;

  // This is needed to hide any menu currently on, as first thing.
  InputEvents::setMode(TEXT("default"));

  #if TESTBENCH
  StartupStore(_T("... ChangeScreen suspending Draw Thread\n"));
  #endif
  MapWindow::SuspendDrawingThread();


  // MapWndProc will get a WM_SIZE 

  //
  // Detect the current screen geometry
  //
  #if (WINDOWSPC>0)
  // For PC we assume that the desired resolution is in SCREENxx
  WindowSize.left = 0;
  WindowSize.top = 0;
  WindowSize.right = SCREENWIDTH;
  WindowSize.bottom = SCREENHEIGHT;
  #else
  WindowSize.left = 0;
  WindowSize.top = 0;
  WindowSize.right = GetSystemMetrics(SM_CXSCREEN);
  WindowSize.bottom = GetSystemMetrics(SM_CYSCREEN);
  #endif

  // 
  // ----------- DEVELOPMENT TESTBENCH OPTIONS -------------
  //
  #if 0
  // Force a test resolution, for testing only!
  // Using always the same resolution will not work when asking for the same resolution again.
  // dont know why (yet)...
  WindowSize.left = 0;
  WindowSize.top = 0;
  WindowSize.right = 480;	
  WindowSize.bottom = 272;
  #endif
  #if 0
  // Simulate changin one resolution to another
  static bool vhflip=true;
  if (vhflip) {
	WindowSize.left = 0;
	WindowSize.top = 0;
	WindowSize.right = 480;
	WindowSize.bottom = 272;
	vhflip=false;
  } else {
	WindowSize.left = 0;
	WindowSize.top = 0;
	WindowSize.right = 800;
	WindowSize.bottom = 480;
	vhflip=true;;
  }
  #endif
  #if 0
  // Simulate Portrait<>Landscape flip/flop
  WindowSize.left = 0;
  WindowSize.top = 0;
  WindowSize.right = SCREENHEIGHT;
  WindowSize.bottom = SCREENWIDTH;
  #endif
  // 
  // ---------------------------------------------------------
  //

  if (oldSCREENWIDTH!=WindowSize.right || oldSCREENHEIGHT!=WindowSize.bottom) {
	#if TESTBENCH
	StartupStore(_T(".... CHANGING RESOLUTION\n"));
	#endif
	#if (WINDOWSPC>0)
	SCREENWIDTH = WindowSize.right;
	SCREENHEIGHT= WindowSize.bottom;
	#endif
	oldSCREENWIDTH = WindowSize.right;
	oldSCREENHEIGHT= WindowSize.bottom;
  } else {
	// THIS DOES NOT STILL WORK! NO EFFECT.
	#if TESTBENCH
	StartupStore(_T(".... CHANGE RESOLUTION, SAME SIZE, WM_SIZE FORCED\n"));
	#endif
	#if (WINDOWSPC>0)
	SCREENWIDTH = WindowSize.right;
	SCREENHEIGHT= WindowSize.bottom;
	#endif
	SendMessage(hWndMapWindow, WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM(0,0));
  }

#if (WINDOWSPC>0)
  WindowSize.right = SCREENWIDTH + 2*GetSystemMetrics( SM_CXFIXEDFRAME);
  WindowSize.left = (GetSystemMetrics(SM_CXSCREEN) - WindowSize.right) / 2;
  WindowSize.bottom = SCREENHEIGHT + 2*GetSystemMetrics( SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
  WindowSize.top = (GetSystemMetrics(SM_CYSCREEN) - WindowSize.bottom) / 2;

  // We must consider the command bar size on PC window
  MoveWindow(hWndMainWindow, WindowSize.left, WindowSize.top, WindowSize.right, WindowSize.bottom, TRUE);
  MoveWindow(hWndMapWindow, 0, 0, SCREENWIDTH, SCREENHEIGHT, FALSE); // also TRUE?
#else

  // Still to be tested!
  MoveWindow(hWndMainWindow, WindowSize.left, WindowSize.top, WindowSize.right, WindowSize.bottom, TRUE);
  MoveWindow(hWndMapWindow, 0, 0, WindowSize.right, WindowSize.bottom, FALSE); 


#endif

  GetClientRect(hWndMainWindow, &rc);
#if (WINDOWSPC>0)
  rc.left = 0;
  rc.right = SCREENWIDTH;
  rc.top = 0;
  rc.bottom = SCREENHEIGHT;
#endif
  InitLKScreen();

  LKSW_ReloadProfileBitmaps=true;
  LKObjects_Delete();
  LKObjects_Create();

  ButtonLabel::Destroy();
  ButtonLabel::CreateButtonLabels(rc);

  extern void InitialiseFonts(RECT rc);
  InitialiseFonts(rc);
  InitLKFonts();
  ButtonLabel::SetFont(MapWindowBoldFont);
  Message::Destroy();
  Message::Initialize(rc); // creates window, sets fonts

  LockTerrainDataGraphics();
  CloseTerrainRenderer();
  UnlockTerrainDataGraphics();

  Reset_Single_DoInits(MDI_DRAWLOOK8000);
  Reset_Single_DoInits(MDI_DRAWTRI);
  Reset_Single_DoInits(MDI_DRAWASPNEAREST);
  Reset_Single_DoInits(MDI_DRAWCOMMON);
  Reset_Single_DoInits(MDI_DRAWFLARMTRAFFIC);
  Reset_Single_DoInits(MDI_DRAWINFOPAGE);
  Reset_Single_DoInits(MDI_WRITEINFO);
  Reset_Single_DoInits(MDI_DRAWLOOK8000);
  Reset_Single_DoInits(MDI_DRAWMAPSPACE);
  Reset_Single_DoInits(MDI_DRAWNEAREST);
  Reset_Single_DoInits(MDI_DRAWTARGET);
  Reset_Single_DoInits(MDI_DRAWTHERMALHISTORY);
  Reset_Single_DoInits(MDI_DRAWTRAFFIC);
  Reset_Single_DoInits(MDI_DRAWVARIO);
  Reset_Single_DoInits(MDI_PROCESSVIRTUALKEY);
  Reset_Single_DoInits(MDI_ONPAINTLISTITEM);
  Reset_Single_DoInits(MDI_DRAWMAPSCALE);
  Reset_Single_DoInits(MDI_MAPWPLABELADD);
  Reset_Single_DoInits(MDI_CHECKLABELBLOCK);
  Reset_Single_DoInits(MDI_LKPROCESS);
  Reset_Single_DoInits(MDI_COMPASS);
  Reset_Single_DoInits(MDI_LOOKABLEND);
  Reset_Single_DoInits(MDI_MAPWPVECTORS);
  Reset_Single_DoInits(MDI_MAPASP);
  Reset_Single_DoInits(MDI_MAPRADAR);
  Reset_Single_DoInits(MDI_MAPWNDPROC);

  #if TESTBENCH
  StartupStore(_T("... ChangeScreen resuming Draw Thread\n"));
  #endif
  MapWindow::ResumeDrawingThread();

  ShowWindow(hWndMainWindow, SW_SHOWNORMAL);
  BringWindowToTop(hWndMainWindow);

  return;
}


