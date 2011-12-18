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
#include "DoInits.h"

using std::min;
using std::max;

#if 0 // Experimental work in progress

//
// Detect if screen resolution and/or orientation has changed
//
bool ScreenHasChanged(void) {
	return false;
}



// TODO TODO WORK IN PROGRESS
// Reinit screen upon resolution/orientation change detected
//
// Test is possible from VirtualKeys.cpp, activating the customkey at line 229
//
void ReinitScreen(void) {

  RECT WindowSize, rc;

  //
  // Detect here the current screen geometry
  //
  WindowSize.left = 0;
  WindowSize.top = 0;
  WindowSize.right = GetSystemMetrics(SM_CXSCREEN);
  WindowSize.bottom = GetSystemMetrics(SM_CYSCREEN);

  #if 1
  // Force a test resolution, for testing only!
  WindowSize.right = 480;	
  WindowSize.bottom = 272;
  // 
  #endif

  #if 0
  // Toggle landcape portrait test only
  static bool vhflip=true;
  if (vhflip) {
	WindowSize.right = 480;
	WindowSize.bottom = 800;
	vhflip=false;
  } else {
	WindowSize.right = 800;
	WindowSize.bottom = 480;
	vhflip=true;;
  }
  #endif


  SCREENWIDTH = WindowSize.right;
  SCREENHEIGHT= WindowSize.bottom;

#if (WINDOWSPC>0)
  WindowSize.right = SCREENWIDTH + 2*GetSystemMetrics( SM_CXFIXEDFRAME);
  WindowSize.left = (GetSystemMetrics(SM_CXSCREEN) - WindowSize.right) / 2;
  WindowSize.bottom = SCREENHEIGHT + 2*GetSystemMetrics( SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
  WindowSize.top = (GetSystemMetrics(SM_CYSCREEN) - WindowSize.bottom) / 2;

  // We must consider the command bar size on PC window
  MoveWindow(hWndMainWindow, WindowSize.left, WindowSize.top, WindowSize.right,WindowSize.bottom, TRUE);
  MoveWindow(hWndMapWindow, 0, 0, SCREENWIDTH, SCREENHEIGHT, FALSE);
#else

  // Still to be tested!
  MoveWindow(hWndMainWindow, WindowSize.left, WindowSize.top, SCREENWIDTH, SCREENHEIGHT, TRUE);
  MoveWindow(hWndMapWindow, 0, 0, SCREENWIDTH, SCREENHEIGHT, FALSE);


#endif

//  This is not good for fast changes between portrait and landscape mode.
//  MapWindow::CloseDrawingThread();
//  MapWindow::CreateDrawingThread();
//  SwitchToMapWindow();

  Reset_All_DoInits(); // this is wrong, we should be less drastic!!

  InitLKScreen();
  LKSW_ReloadProfileBitmaps=true;

  GetClientRect(hWndMainWindow, &rc);
#if (WINDOWSPC>0)
  rc.left = 0;
  rc.right = SCREENWIDTH;
  rc.top = 0;
  rc.bottom = SCREENHEIGHT;
#endif
  LKObjects_Delete();
  LKObjects_Create();

  ButtonLabel::Destroy();
  ButtonLabel::CreateButtonLabels(rc);

  extern void InitialiseFonts(RECT rc);
  InitialiseFonts(rc);
  InitLKFonts();
  ButtonLabel::SetFont(MapWindowBoldFont);
  Message::Initialize(rc); // creates window, sets fonts

  LockTerrainDataGraphics();
  CloseTerrainRenderer();
  UnlockTerrainDataGraphics();


  return;
}


#endif // experimental only
