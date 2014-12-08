/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

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
  // by default we shall ask for lk to be reset, unless it is 180 flip
  // because in that case the OS will do the magic with no need to do anything else.
  bool mustresetlk=true;

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
			#if TESTBENCH
			StartupStore(_T("... CURRENT ORIENT=0, rotate to 90\n"));
			#endif
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_90; 
			break;
		case DMDO_90:
			#if TESTBENCH
			StartupStore(_T("... CURRENT ORIENT=90, rotate back to 0\n"));
			#endif
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_0; 
			break;
		case DMDO_180:
			#if TESTBENCH
			StartupStore(_T("... CURRENT ORIENT=180, rotate to 270\n"));
			#endif
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_270; 
			break;
		case DMDO_270:
			#if TESTBENCH
			StartupStore(_T("... CURRENT ORIENT=270, rotate back to 180\n"));
			#endif
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_180; 
			break;
		default:
			#if TESTBENCH
			StartupStore(_T("... CURRENT ORIENT=UNKNOWN\n"));
			#endif
			mustresetlk=false;
			break;
	}
  }  // angle 90

  //
  // OS rotation only
  //
  if (angle==180) {
	switch (DeviceMode.dmDisplayOrientation) {
		case DMDO_0:
			#if TESTBENCH
			StartupStore(_T("... CURRENT ORIENT=0, flip to 180\n"));
			#endif
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_180; 
			break;
		case DMDO_90:
			#if TESTBENCH
			StartupStore(_T("... CURRENT ORIENT=90, flip to 270\n"));
			#endif
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_270; 
			break;
		case DMDO_180:
			#if TESTBENCH
			StartupStore(_T("... CURRENT ORIENT=180, flip back to 0\n"));
			#endif
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_0; 
			break;
		case DMDO_270:
			#if TESTBENCH
			StartupStore(_T("... CURRENT ORIENT=270, flip back to 90\n"));
			#endif
			memset(&DeviceMode, 0, sizeof(DeviceMode));
			DeviceMode.dmDisplayOrientation = DMDO_90; 
			break;
		default:
			#if TESTBENCH
			StartupStore(_T("... CURRENT ORIENT=UNKNOWN\n"));
			#endif
			break;
	}
	mustresetlk=false;
  } // angle 180

  DeviceMode.dmSize=sizeof(DeviceMode);
  DeviceMode.dmFields = DM_DISPLAYORIENTATION;

  if (DISP_CHANGE_SUCCESSFUL == ChangeDisplaySettingsEx(NULL, &DeviceMode, NULL, CDS_RESET, NULL)) {
	if (mustresetlk) {
		#if TESTBENCH
		StartupStore(_T("... Screen Rotation successful, setting to %d x %d\n"),ScreenSizeY,ScreenSizeX);
		#endif
		SetWindowPos(MainWindow.Handle(),HWND_TOPMOST,0,0,ScreenSizeY,ScreenSizeX,SWP_SHOWWINDOW);
	} else {
		#if TESTBENCH
		StartupStore(_T("... Screen Rotation successful, setting to %d x %d\n"),ScreenSizeX,ScreenSizeY);
		#endif
		SetWindowPos(MainWindow.Handle(),HWND_TOPMOST,0,0,ScreenSizeX,ScreenSizeY,SWP_SHOWWINDOW);
	}

	ShowWindow(MainWindow.Handle(), SW_SHOWNORMAL);
    BringWindowToTop(MainWindow.Handle());

	#ifdef HAVE_ACTIVATE_INFO
	SHFullScreen(MainWindow.Handle(),SHFS_HIDETASKBAR|SHFS_HIDESIPBUTTON|SHFS_HIDESTARTICON);
	#endif

//	UpdateWindow(MainWindow); No! No WM_PAINT please!

	return mustresetlk;
  } else {
	#if TESTBENCH
	StartupStore(_T("... Screen Rotation failed!\n"));
	#endif
	return false;
  }

#endif // !WINDOWSPC

}


