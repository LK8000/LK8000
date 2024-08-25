/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "../RotateScreen.h"

bool CanRotateScreen() {
    return true;
}

// This is not working on PC because we are not rotating the whole desktop!
// We return true only if we need to perform a ChangeScreen inside LK.
bool RotateScreen(short angle) {
#if (WINDOWSPC>0)
  return false;
#else
  //
  // Change the orientation of the screen
  //
  DEVMODE DeviceMode;
  memset(&DeviceMode, 0, sizeof(DeviceMode));
  DeviceMode.dmSize=sizeof(DeviceMode);
  DeviceMode.dmFields = DM_DISPLAYORIENTATION;

  if (DISP_CHANGE_SUCCESSFUL != ChangeDisplaySettingsEx(NULL,&DeviceMode,NULL,CDS_TEST,NULL)) {
	StartupStore(_T("... ERROR RotateScreen cannot read current screen settings\n"));
	return false;
  }

  //
  // Real rotation
  //
  if (angle==90) {
	switch (DeviceMode.dmDisplayOrientation) {
		case DMDO_0:
			TestLog(_T("... CURRENT ORIENT=0, rotate to 90\n"));
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_90;
			break;
		case DMDO_90:
			TestLog(_T("... CURRENT ORIENT=90, rotate back to 0\n"));
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_0;
			break;
		case DMDO_180:
			TestLog(_T("... CURRENT ORIENT=180, rotate to 270\n"));
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_270;
			break;
		case DMDO_270:
			TestLog(_T("... CURRENT ORIENT=270, rotate back to 180\n"));
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_180;
			break;
		default:
			TestLog(_T("... CURRENT ORIENT=UNKNOWN\n"));
			break;
	}
  }  // angle 90

  //
  // OS rotation only
  //
  if (angle==180) {
	switch (DeviceMode.dmDisplayOrientation) {
		case DMDO_0:
			TestLog(_T("... CURRENT ORIENT=0, flip to 180\n"));
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_180;
			break;
		case DMDO_90:
			TestLog(_T("... CURRENT ORIENT=90, flip to 270\n"));
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_270;
			break;
		case DMDO_180:
			TestLog(_T("... CURRENT ORIENT=180, flip back to 0\n"));
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_0;
			break;
		case DMDO_270:
			TestLog(_T("... CURRENT ORIENT=270, flip back to 90\n"));
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_90;
			break;
		default:
			TestLog(_T("... CURRENT ORIENT=UNKNOWN\n"));
			break;
	}
  } // angle 180

  DeviceMode.dmSize=sizeof(DeviceMode);
  DeviceMode.dmFields = DM_DISPLAYORIENTATION;

  if (DISP_CHANGE_SUCCESSFUL == ChangeDisplaySettingsEx(NULL, &DeviceMode, NULL, CDS_RESET, NULL)) {
		StartupStore(_T("... Screen Rotation successful, setting to %d x %d\n"),ScreenSizeY,ScreenSizeX);

	ShowWindow(main_window->Handle(), SW_SHOWNORMAL);
    BringWindowToTop(main_window->Handle());

#ifdef HAVE_ACTIVATE_INFO
	SHFullScreen(main_window->Handle(),SHFS_HIDETASKBAR|SHFS_HIDESIPBUTTON|SHFS_HIDESTARTICON);
#endif
#ifdef UNDER_CE
    SetWindowPos(main_window->Handle(), HWND_TOP,
                 0, 0,
                 GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN),
                 SWP_SHOWWINDOW);
#endif

//	UpdateWindow(*main_window); No! No WM_PAINT please!

	return true;
  } else {
	TestLog(_T("... Screen Rotation failed!"));
	return false;
  }

#endif // !WINDOWSPC

}

ScopeLockScreen::ScopeLockScreen() :
            previous_state()
{
#ifndef UNDER_CE
    HWND hwnd = main_window->Handle();
    if(hwnd) {
        DWORD dwStyle = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
        previous_state = !!(dwStyle&WS_SIZEBOX);
        SetWindowLong(hwnd, GWL_STYLE, dwStyle&(~WS_SIZEBOX));
    }
#endif
}

ScopeLockScreen::~ScopeLockScreen() {
#ifndef UNDER_CE
    if(previous_state) {
        HWND hwnd = main_window->Handle();
        if(hwnd) {
            DWORD dwStyle = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
            SetWindowLong(hwnd, GWL_STYLE, dwStyle|WS_SIZEBOX);
        }
    }
#endif
}
