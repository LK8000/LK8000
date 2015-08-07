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


// windows
WndMain MainWindow; // Main Window singleton

BOOL	InitInstance    (int);

extern void FillDataOptions(void);
extern void StartupLogFreeRamAndStorage();

void PreloadInitialisation(bool ask) {
  LKLanguageReady=false;
  LKReadLanguageFile(szLanguageFile);
  FillDataOptions(); // Load infobox list

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
	DoStatusMessage(gettext(_T("_@M1757_"))); // LK8000 PROFILES RESET
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

    // LKTOKEN _@M1206_ "Initialising..."
	CreateProgressDialog(gettext(TEXT("_@M1206_"))); 
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
  WindowSize=WindowResize(ScreenSizeX, ScreenSizeY);
#endif
#ifdef WIN32
#ifdef UNDER_CE
  WindowSize=WindowResize( GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
#else
  WindowSize=WindowResize(ScreenSizeX, ScreenSizeY);
#endif
#endif


  if (!goInstallSystem) Poco::Thread::sleep(50); // 091119
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
  
  InitLKScreen();
  InitLKFonts(); // causing problems with CreateButtonLabels?

  LKLoadFixedBitmaps();
  LKLoadProfileBitmaps();
  LKObjects_Create(); 


  ButtonLabel::CreateButtonLabels(rc);
  ButtonLabel::SetLabelText(0,TEXT("MODE"));

  extern void InitLKFonts();
  // reload updating LK fonts after loading profile for fontquality
  InitLKFonts();	

  ButtonLabel::SetFont(MapWindowBoldFont);

  Message::Initialize(rc); // creates window, sets fonts

  MainWindow.SetVisible(true);

  return TRUE;
}



