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

  const PixelRect rc(main_window->GetClientRect());
  if (doinit) {

#if (WINDOWSPC>0) || defined(__linux__)
	oldSCREENWIDTH=rc.GetSize().cx;
	oldSCREENHEIGHT=rc.GetSize().cy;
#else
	oldSCREENWIDTH=GetSystemMetrics(SM_CXSCREEN);
	oldSCREENHEIGHT=GetSystemMetrics(SM_CYSCREEN);
#endif

        #ifdef TESTBENCH
        StartupStore(_T("... First ScreenHasChanged: %d x %d\n"),oldSCREENWIDTH, oldSCREENHEIGHT);
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

#ifndef ENABLE_OPENGL
  MapWindow::SuspendDrawingThread();
  ScopeLock Lock(MapWindow::Surface_Mutex);
#endif

  // MapWndProc will get a WM_SIZE

  //
  // Detect the current screen geometry
  //
  const PixelRect rc(main_window->GetClientRect());
  ScreenSizeX = rc.GetSize().cx;
  ScreenSizeY = rc.GetSize().cy;

  InitLKScreen();

  LKObjects_Delete();
  LKObjects_Create();

  ButtonLabel::Destroy();
  ButtonLabel::CreateButtonLabels(rc);

  InitLKFonts();
  ButtonLabel::SetFont(MapWindowBoldFont);

  Message::Destroy();
  Message::Initialize(rc); // creates window, sets fonts

  LockTerrainDataGraphics();
  CloseTerrainRenderer();
  UnlockTerrainDataGraphics();

  // DoInits will require new values (at least PROCESSVIRTUALKEYS)
  main_window->UpdateActiveScreenZone(rc);

  Reset_Single_DoInits(MDI_DRAWLOOK8000);
  Reset_Single_DoInits(MDI_DRAWTRI);
  Reset_Single_DoInits(MDI_DRAWHSI);
  Reset_Single_DoInits(MDI_DRAWFLARMTRAFFIC);
  Reset_Single_DoInits(MDI_DRAWINFOPAGE);
  Reset_Single_DoInits(MDI_WRITEINFO);
  Reset_Single_DoInits(MDI_DRAWLOOK8000);
  Reset_Single_DoInits(MDI_DRAWNEAREST);
  Reset_Single_DoInits(MDI_DRAWTARGET);
  Reset_Single_DoInits(MDI_DRAWVARIO);
  Reset_Single_DoInits(MDI_PROCESSVIRTUALKEY);
  Reset_Single_DoInits(MDI_CHECKLABELBLOCK);
  Reset_Single_DoInits(MDI_LOOKABLEND);
  Reset_Single_DoInits(MDI_MAPWPVECTORS);
  Reset_Single_DoInits(MDI_MAPASP);
  Reset_Single_DoInits(MDI_MAPRADAR); // doing nothing reallt
  Reset_Single_DoInits(MDI_FLARMRADAR);
  Reset_Single_DoInits(MDI_DRAWBOTTOMBAR);
  Reset_Single_DoInits(MDI_DRAWFLIGHTMODE);
  Reset_Single_DoInits(MDI_DRAWTASK);

  // Some parameters need to be reset in advance, otherwise they will be retuned only when the
  // relative DoInit is accomplished. This is a mistake of course.
  TopSize=0; // requires a DrawNearest. 0 is Ok on startup.

  #if TESTBENCH
  StartupStore(_T("... ChangeScreen resuming Draw Thread\n"));
  #endif


  MapWindow::Initialize();
  MapWindow::ResumeDrawingThread();
  main_window->SetToForeground();

  return;
}
