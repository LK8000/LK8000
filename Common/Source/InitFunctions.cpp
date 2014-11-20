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
#include "InputEvents.h"
#include "Message.h"
#include "Bitmaps.h"
#include "LKObjects.h"
#include "DoInits.h"


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
    LKSW_ReloadProfileBitmaps=true;

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
  InitLKScreen();
  InitLKFonts(); // causing problems with CreateButtonLabels?
  PreloadInitialisation(true);

  RECT WindowSize;

  WindowSize.left = 0;
  WindowSize.top = 0;
  WindowSize.right = GetSystemMetrics(SM_CXSCREEN);
  WindowSize.bottom = GetSystemMetrics(SM_CYSCREEN);


#if (WINDOWSPC>0)
  WindowSize.right = SCREENWIDTH + 2*GetSystemMetrics( SM_CXFIXEDFRAME);
  WindowSize.left = (GetSystemMetrics(SM_CXSCREEN) - WindowSize.right) / 2;
  WindowSize.right = WindowSize.right +WindowSize.left;
  WindowSize.bottom = SCREENHEIGHT + 2*GetSystemMetrics( SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
  WindowSize.top = (GetSystemMetrics(SM_CYSCREEN) - WindowSize.bottom) / 2;
  WindowSize.bottom = WindowSize.bottom +WindowSize.top;
  /*
  //
  // Custom setup for positioning the window , ready to be used
  //
  WindowSize.top=768;	// top and left corner coords
  WindowSize.left=1024;
  WindowSize.right = SCREENWIDTH + 2*GetSystemMetrics( SM_CXFIXEDFRAME);
  WindowSize.bottom = SCREENHEIGHT + 2*GetSystemMetrics( SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
  */
#endif

  if (!goInstallSystem) Poco::Thread::sleep(50); // 091119
  #if TESTBENCH
  StartupStore(TEXT(". Create main window%s"),NEWLINE);
  #endif

  if(!MainWindow.Create(WindowSize)) {
      return FALSE;
  }

  RECT rc = MainWindow.GetClientRect();

  LKLoadFixedBitmaps();
  LKLoadProfileBitmaps();
  LKObjects_Create(); 


  ButtonLabel::CreateButtonLabels(rc);
  ButtonLabel::SetLabelText(0,TEXT("MODE"));

  extern void InitialiseFonts(RECT rc);
  InitialiseFonts(rc);
  InitLKFonts();	// reload updating LK fonts after loading profile for fontquality

  ButtonLabel::SetFont(MapWindowBoldFont);

  Message::Initialize(rc); // creates window, sets fonts

  MainWindow.Visible(true);

  // Since MapWindow is doing static inits, we want them to be recalculated at the end of
  // initializations, since some values in use might have been not available yet, for example BottomSize.
  // maybe it's useless, already done by MainWindow::OnSize()
  MainWindow.UpdateActiveScreenZone(rc.right - rc.left, rc.bottom - rc.top);
   
  return TRUE;
}



