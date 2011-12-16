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

  WindowSize.left = 0;
  WindowSize.top = 0;
  WindowSize.right = GetSystemMetrics(SM_CXSCREEN);
  WindowSize.bottom = GetSystemMetrics(SM_CYSCREEN);

  // testing only!
  WindowSize.right = 480;
  WindowSize.bottom = 272;
  SCREENWIDTH=480;
  SCREENHEIGHT=272;
  //

#if (WINDOWSPC>0)
  WindowSize.right = SCREENWIDTH + 2*GetSystemMetrics( SM_CXFIXEDFRAME);
  WindowSize.left = (GetSystemMetrics(SM_CXSCREEN) - WindowSize.right) / 2;
  WindowSize.bottom = SCREENHEIGHT + 2*GetSystemMetrics( SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
  WindowSize.top = (GetSystemMetrics(SM_CYSCREEN) - WindowSize.bottom) / 2;
#endif

  InitLKScreen();
  InitLKFonts();

  HWND hWndMainWindow;
  hWndMainWindow=GetActiveWindow();

  MoveWindow(hWndMainWindow, WindowSize.left, WindowSize.top, 640,480, TRUE);

  GetClientRect(hWndMainWindow, &rc);

#if (WINDOWSPC>0)
  rc.left = 0;
  rc.right = SCREENWIDTH;
  rc.top = 0;
  rc.bottom = SCREENHEIGHT;
#endif

  ButtonLabel::CreateButtonLabels(rc);

  extern void InitialiseFonts(RECT rc);
  InitialiseFonts(rc);
  InitLKFonts();	// reload updating LK fonts after loading profile for fontquality
  extern void Reset_All_DoInits(void);
  Reset_All_DoInits();

  ButtonLabel::SetFont(MapWindowBoldFont);

  Message::Initialize(rc); // creates window, sets fonts

#if 0
  ShowWindow(hWndMainWindow, SW_SHOW);

  StartupStore(TEXT(". Create map window%s"),NEWLINE);

  hWndMapWindow = CreateWindow(TEXT("MapWindowClass"),NULL,
			       WS_VISIBLE | WS_CHILD
			       | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                               0, 0, (rc.right - rc.left),
			       (rc.bottom-rc.top) ,
                               hWndMainWindow, NULL ,hInstance,NULL);

  ShowWindow(hWndMainWindow, nCmdShow);

  UpdateWindow(hWndMainWindow);
#endif
    
  return;
}


#endif // experimental only
