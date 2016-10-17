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
#include "Dialogs.h"
#include "Dialogs/dlgProgress.h"
#include "InputEvents.h"
#include "Message.h"
#include "Bitmaps.h"
#include "LKObjects.h"
#include "DoInits.h"

#ifdef USE_FREETYPE
#include "Screen/FreeType/Init.hpp"
#endif

#include "Event/Globals.hpp"
#include "Event/Queue.hpp"
#include "Screen/Debug.hpp"
#include "Screen/Font.hpp"
#include "DisplayOrientation.hpp"
#include "Asset.hpp"

#ifdef KOBO
#include "Hardware/RotateDisplay.hpp"
#endif

#ifdef USE_FREETYPE
#include "Screen/FreeType/Init.hpp"
#include "Screen/Init.hpp"
#endif

#ifdef USE_VIDEOCORE
#include <bcm_host.h>
#endif

// windows
WndMain MainWindow; // Main Window singleton

BOOL	InitInstance    (int);

extern void StartupLogFreeRamAndStorage();
extern bool ScreenHasChanged(void);

void PreloadInitialisation(bool ask) {
  LKLanguageReady=false;
  LKReadLanguageFile(szLanguageFile);

  if (ask) {
    // Load default profile and status file: we are at an early stage
    LKProfileResetDefault();
    LKProfileLoad(startAircraftFile);
    LKProfileLoad(startPilotFile);
    LKProfileLoad(startDeviceFile);
    // if DEFAULT PROFILE does not exist, initialize ResetDefaults!
    // This is because LKProfileLoad will do this at its end, normally.
    // Notice: aircraft and pilot files will not be overridden by defaults
    if (!LKProfileLoad(startProfileFile)) {
	LKProfileInitRuntime();
    }

    StatusFileInit();
  } else {
    // We are in the dialog startup phase
    FullScreen();
    short retstartup;
    do {
	retstartup=dlgStartupShowModal();
    } while (retstartup>0);

    if (retstartup<0) return;

    if (_tcscmp(startProfileFile,_T("PROFILE_RESET"))==0) {
	StartupStore(_T(". USER ASKED FOR PROFILE FULL RESET!%s"),NEWLINE);
	DoStatusMessage(MsgToken(1757)); // LK8000 PROFILES RESET
	LKProfileResetDefault();
	LKProfileInitRuntime();

	// Notice: this is also resetting the default Aircraft and Pilot profiles to demo settings
    } else  {
	if (!LKProfileLoad(startPilotFile)) {
		#if TESTBENCH
		StartupStore(_T(". PilotFile RESET to defaults%s"),NEWLINE);
		#endif
	}
	if (!LKProfileLoad(startDeviceFile)) {
		#if TESTBENCH
		StartupStore(_T(". DeviceFile RESET to defaults%s"),NEWLINE);
		#endif
	}
	if (!LKProfileLoad(startAircraftFile)) {
		#if TESTBENCH
		StartupStore(_T(". AircraftFile RESET to defaults%s"),NEWLINE);
		#endif
	}
	LKProfileLoad(startProfileFile); // this is calling adjust and InitRuntime itself
    }
    InitLKFonts();
    // We are sure that buttons have been created already
    ButtonLabel::SetFont(MapWindowBoldFont);
    // LKTOKEN _@M1206_ "Initialising..."
	CreateProgressDialog(MsgToken(1206)); 
  }

  // Interface (before interface)
  if (!ask) {
    LKReadLanguageFile(szLanguageFile);
    InputEvents::readFile();
  }
  
  InitCustomHardware();
}

//
//  FUNCTION: InitInstance()
//
//  PURPOSE: creates main window
//
//  COMMENTS:
//
//    In this function, we create and display the main program window.
//
BOOL InitInstance()
{
  extern bool CommandResolution;
  if(!IsEmbedded() && !CommandResolution) {
      ScreenSizeX = 800;
      ScreenSizeY = 480;
  }

#if defined(ENABLE_SDL) && defined(USE_FULLSCREEN)
 #if (SDL_MAJOR_VERSION >= 2)
  SDL_DisplayMode mode = {};
  if(SDL_GetCurrentDisplayMode(0, &mode) == 0) {
  	ScreenSizeX = mode.w;
    ScreenSizeY = mode.h;
  } else {
  	fprintf(stderr, "SDL_GetCurrentDisplayMode() has failed: %s\n", ::SDL_GetError());
  }
  #else
  	ScreenSizeX = 0;
    ScreenSizeY = 0;
  #endif
#endif

  PreloadInitialisation(true);

  RECT WindowSize;

#ifdef __linux__
#ifdef USE_VIDEOCORE
  uint32_t iWidth, iHeight;
  if(graphics_get_display_size(0, &iWidth, &iHeight) >= 0) {
    ScreenSizeX=iWidth;
    ScreenSizeY=iHeight;
  }
#endif  
  WindowSize=WindowResize(ScreenSizeX, ScreenSizeY);
#endif
#ifdef WIN32
#if defined(UNDER_CE) || defined(USE_FULLSCREEN)
  WindowSize=WindowResize( GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
#else
  WindowSize=WindowResize(ScreenSizeX, ScreenSizeY);
#endif
#endif


  #if TESTBENCH
  StartupStore(TEXT(". Create main window%s"),NEWLINE);
  #endif

  if(!MainWindow.Create(WindowSize)) {
      StartupStore(TEXT(". FAILURE: Create main window%s"),NEWLINE);
      return FALSE;
  } 
  const PixelRect rc(MainWindow.GetClientRect());
  ScreenSizeX = rc.GetSize().cx;
  ScreenSizeY = rc.GetSize().cy;
  ScreenHasChanged();
  
  InitLKScreen();
  InitLKFonts(); // causing problems with CreateButtonLabels?

  LKLoadFixedBitmaps();
  LKLoadProfileBitmaps();
  LKObjects_Create(); 

  ButtonLabel::CreateButtonLabels(rc);
  ButtonLabel::SetFont(MapWindowBoldFont);

  Message::Initialize(rc); // creates window, sets fonts

  MainWindow.SetVisible(true);

  return TRUE;
}



