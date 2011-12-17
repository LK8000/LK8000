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

#if 1 // Experimental work in progress

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

  // Detect here the current screen geometry
  WindowSize.left = 0;
  WindowSize.top = 0;
  WindowSize.right = GetSystemMetrics(SM_CXSCREEN);
  WindowSize.bottom = GetSystemMetrics(SM_CYSCREEN);

  // force a test resolution, for testing only!
  WindowSize.right = 480;	// switch to portrait mode 480x640
  WindowSize.bottom = 640;
  //


  SCREENWIDTH = WindowSize.right;
  SCREENHEIGHT= WindowSize.bottom;

#if (WINDOWSPC>0)
  WindowSize.right = SCREENWIDTH + 2*GetSystemMetrics( SM_CXFIXEDFRAME);
  WindowSize.left = (GetSystemMetrics(SM_CXSCREEN) - WindowSize.right) / 2;
  WindowSize.bottom = SCREENHEIGHT + 2*GetSystemMetrics( SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
  WindowSize.top = (GetSystemMetrics(SM_CYSCREEN) - WindowSize.bottom) / 2;
#endif


  MoveWindow(hWndMainWindow, WindowSize.left, WindowSize.top, SCREENWIDTH, SCREENHEIGHT, TRUE);
  UpdateWindow(hWndMainWindow);
  MoveWindow(hWndMapWindow, 0, 0, SCREENWIDTH, SCREENHEIGHT, TRUE);

  Reset_All_DoInits(); // this is wrong, we should be less drastic!!

  InitLKScreen();
  InitLKFonts();
  
  //Reset_All_DoInits(); // this is wrong, we should be less drastic!!
  Reset_Single_DoInits(MDI_DRAWLOOK8000);
  Reset_Single_DoInits(MDI_PROCESSVIRTUALKEY);
  Reset_Single_DoInits(MDI_ONPAINTLISTITEM);

  GetClientRect(hWndMainWindow, &rc);
#if (WINDOWSPC>0)
  rc.left = 0;
  rc.right = SCREENWIDTH;
  rc.top = 0;
  rc.bottom = SCREENHEIGHT;
#endif
  LKObjects_Delete();
  LKObjects_Create();
  ButtonLabel::CreateButtonLabels(rc);
  extern void InitialiseFonts(RECT rc);
  InitialiseFonts(rc);
  InitLKFonts();
  ButtonLabel::SetFont(MapWindowBoldFont);
  Message::Initialize(rc); // creates window, sets fonts

  UpdateWindow(hWndMapWindow);
  return;
}


#endif // experimental only
