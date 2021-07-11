/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
#include "Event/Globals.hpp"
#include "Event/Queue.hpp"
#include "Screen/Debug.hpp"
#include "Screen/Font.hpp"
#include "DisplayOrientation.hpp"
#include "Asset.hpp"
#include "ChangeScreen.h"

#ifdef USE_FREETYPE
#include "Screen/FreeType/Init.hpp"
#include "Screen/Init.hpp"
#endif

#ifdef USE_VIDEOCORE
#include <bcm_host.h>
#endif

#ifdef HAVE_MALI
#include "Screen/Sunxi/mali.h"
#endif

#ifdef ANDROID
#include "Android/Main.hpp"
#include "Android/NativeView.hpp"
#endif

#ifdef OPENVARIO
#include "LKInterface/CScreenOrientation.h"
#include "Hardware/RotateDisplay.hpp"
#endif

// windows
std::unique_ptr<WndMain> main_window; // Main Window singleton

BOOL	InitInstance    (int);

void PreloadInitialisation(bool ask) {
  LKLoadLanguageFile();

  if (ask) {
    // Load default profile and status file: we are at an early stage
    LKProfileResetDefault();
    LKProfileLoad(startAircraftFile);
    LKProfileLoad(startPilotFile);
    LKProfileLoad(startDeviceFile);
    LKProfileLoad(startProfileFile);

	LKProfileInitRuntime();
    
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
    	if(!LKProfileLoad(startProfileFile)) {
            #if TESTBENCH
            StartupStore(_T(". SystemFile RESET to defaults%s"),NEWLINE);
            #endif
        }
    }
	LKProfileInitRuntime();

    InitLKFonts();
    // We are sure that buttons have been created already
    ButtonLabel::SetFont(MapWindowBoldFont);

    // font change, we need to reset "Messge
    Message::InitFont();

    // LKTOKEN _@M1206_ "Initialising..."
	CreateProgressDialog(MsgToken(1206));
  }

  // Interface (before interface)
  if (!ask) {
    LKLoadLanguageFile();
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
  SDL_DisplayMode mode = {};
  if(SDL_GetCurrentDisplayMode(0, &mode) == 0) {
    ScreenSizeX = mode.w;
    ScreenSizeY = mode.h;
  } else {
    StartupStore("SDL_GetCurrentDisplayMode() has failed: %s", ::SDL_GetError());
  }
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

#ifdef HAVE_MALI
  const PixelSize size = mali::GetScreenSize();
  ScreenSizeX = size.cx;
  ScreenSizeY = size.cy;
#endif

#ifdef ANDROID
  const PixelSize Size = native_view->GetSize();
  ScreenSizeX=Size.cx;
  ScreenSizeY=Size.cy;
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

  if(!main_window->Create(WindowSize)) {
      StartupStore(TEXT(". FAILURE: Create main window%s"),NEWLINE);
      return FALSE;
  }
  const PixelRect rc(main_window->GetClientRect());
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

#ifdef OPENVARIO
  Display::Rotate(static_cast<DisplayOrientation_t>(CScreenOrientation::GetScreenSetting()));
#endif

  main_window->SetVisible(true);

  return TRUE;
}
