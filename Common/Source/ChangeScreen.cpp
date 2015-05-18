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
#include "InputEvents.h"
#include "Message.h"
#include "LKObjects.h"
#include "Bitmaps.h"
#include "DoInits.h"
#include "Screen/Point.hpp"

//
// Detect if screen resolution and/or orientation has changed
//
bool ScreenHasChanged(void) {

  static int oldSCREENWIDTH=0;
  static int oldSCREENHEIGHT=0;
  static bool doinit=true;
  int x=0,y=0;

  const PixelRect rc(MainWindow.GetClientRect());
  if (doinit) {
      
#if (WINDOWSPC>0) || defined(__linux__)
	oldSCREENWIDTH=rc.GetSize().cx;
	oldSCREENHEIGHT=rc.GetSize().cy;
#else
	oldSCREENWIDTH=GetSystemMetrics(SM_CXSCREEN);
	oldSCREENHEIGHT=GetSystemMetrics(SM_CYSCREEN);
#endif
	doinit=false;
	return false;
  }

  // On PC, simply check for WIDTH and HEIGHT changed
  #if (WINDOWSPC>0) || defined(__linux__)
  x=rc.GetSize().cx;
  y=rc.GetSize().cy;
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

  // This is needed to hide any menu currently on, as first thing.
  InputEvents::setMode(TEXT("default"));

  #if TESTBENCH
  StartupStore(_T("... ChangeScreen suspending Draw Thread\n"));
  #endif
  MapWindow::SuspendDrawingThread();
  Poco::FastMutex::ScopedLock Lock(MapWindow::Surface_Mutex);


  // MapWndProc will get a WM_SIZE 

  //
  // Detect the current screen geometry
  //
  const PixelRect rc(MainWindow.GetClientRect());
  ScreenSizeX = rc.GetSize().cx;
  ScreenSizeY = rc.GetSize().cy;
  
  InitLKScreen();

  LKSW_ReloadProfileBitmaps=true;
  LKObjects_Delete();
  LKObjects_Create();

  ButtonLabel::Destroy();
  ButtonLabel::CreateButtonLabels(rc);

  extern void InitLKFonts();
  InitLKFonts();
  ButtonLabel::SetFont(MapWindowBoldFont);
  Message::Destroy();
  Message::Initialize(rc); // creates window, sets fonts

  LockTerrainDataGraphics();
  CloseTerrainRenderer();
  UnlockTerrainDataGraphics();

  Reset_Single_DoInits(MDI_DRAWLOOK8000);
  Reset_Single_DoInits(MDI_DRAWTRI);
  Reset_Single_DoInits(MDI_DRAWHSI);
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
  Reset_Single_DoInits(MDI_MAPRADAR); // doing nothing reallt
  Reset_Single_DoInits(MDI_FLARMRADAR);
  Reset_Single_DoInits(MDI_DRAWBOTTOMBAR);
  Reset_Single_DoInits(MDI_DRAWTASK);

  #if TESTBENCH
  StartupStore(_T("... ChangeScreen resuming Draw Thread\n"));
  #endif

  // Since MapWindow is doing static inits, we want them to be recalculated at the end of
  // initializations, since some values in use might have been not available yet, for example BottomSize.
  // maybe useless, already done by MainWindow::OnSize()
  MainWindow.UpdateActiveScreenZone(rc.right - rc.left, rc.bottom - rc.top);


  MapWindow::ResumeDrawingThread();
  MainWindow.SetToForeground();

  return;
}


