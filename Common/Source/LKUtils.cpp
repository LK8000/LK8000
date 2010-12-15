/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKUtils.cpp,v 1.1 2010/12/11 23:45:28 root Exp root $
*/

#include "StdAfx.h"
#include <stdio.h>
#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#endif
#include "options.h"
#include "externs.h"
#include "XCSoar.h"
#include "InfoBoxLayout.h"
#include "Utils2.h"
#include "Cpustats.h"
#include "device.h"
#include "Logger.h"
#include "Parser.h"
#include "WaveThread.h"
#include "GaugeFLARM.h"
#include "LKUtils.h"
#include "Message.h"
#include "McReady.h"
#include "InputEvents.h"
#include "LKMapWindow.h"


extern int PDABatteryPercent;
extern int PDABatteryStatus;
extern int PDABatteryFlag;

extern bool GiveBatteryWarnings(int numwarn);

void LKBatteryManager() {

  static bool doinit=true;
  static bool invalid=false, recharging=false;
  static bool warn33=true, warn50=true, warn100=true;
  static double last_time=0, init_time=0;
  static int last_percent=0, last_status=0;
  static int numwarn=0;


  if (invalid) return;
  if (doinit) {

	if (PDABatteryPercent<1 || PDABatteryPercent>100) {
		StartupStore(_T("... LK BatteryManager V1: internal battery information not available, function disabled%s"), NEWLINE);
		invalid=true;
		doinit=false; // just to be sure
		return;
	}

	StartupStore(_T(". LK Battery Manager V1 started, current charge=%d%%%s"),PDABatteryPercent,NEWLINE);
	init_time=GPS_INFO.Time;
	doinit=false;
  }


  // if first run,  and not passed 30 seconds, do nothing
  if (last_percent==0 && (GPS_INFO.Time<(init_time+30))) {
	// StartupStore(_T("... first run, waiting for 30s\n"));
	return;
  }

  TCHAR mbuf[100];

  // first run after 30 seconds: give a message
  if (last_percent==0) {
	// StartupStore(_T("... first run, last percent=0\n"));
	if (PDABatteryPercent <=50) {
		last_time=GPS_INFO.Time;
		_stprintf(mbuf,_T("BATTERY LEVEL %d%%"),PDABatteryPercent);
		DoStatusMessage(mbuf);
		warn50=false;
	}
	// special case, pdabattery is 0...
	if (PDABatteryPercent <1) {
		StartupStore(_T("... LK Battery Manager disabled, low battery %s"),NEWLINE);
		DoStatusMessage(_T("BATTERY MANAGER DISABLED"));
		invalid=true;
		return;
	} else
		last_percent=PDABatteryPercent;

	if (PDABatteryStatus!=AC_LINE_UNKNOWN) {
		last_status=PDABatteryStatus;
	}
	// StartupStore(_T("... last_percent first assigned=%d\n"),last_percent);
	return;
  }

  if (PDABatteryStatus!=AC_LINE_UNKNOWN) {
	if (last_status != PDABatteryStatus) {
		if (PDABatteryStatus==AC_LINE_OFFLINE) {
			if (GiveBatteryWarnings(++numwarn))
	// LKTOKEN  _@M514_ = "POWER SUPPLY OFF" 
			DoStatusMessage(gettext(TEXT("_@M514_")));
		} else {
			if (PDABatteryStatus==AC_LINE_ONLINE) {
				if (GiveBatteryWarnings(++numwarn))
	// LKTOKEN  _@M515_ = "POWER SUPPLY ON" 
				DoStatusMessage(gettext(TEXT("_@M515_")));
			} else {
				if (PDABatteryStatus==AC_LINE_BACKUP_POWER) {
					if (GiveBatteryWarnings(++numwarn))
	// LKTOKEN  _@M119_ = "BACKUP POWER SUPPLY ON" 
					DoStatusMessage(gettext(TEXT("_@M119_")));
				}
			}
		}
	}
	last_status=PDABatteryStatus;
  }

  // Only check every 5 minutes normally
  if (GPS_INFO.Time<(last_time+(60*5))) return;

  // if battery is recharging, reset warnings and do nothing
  if (last_percent<PDABatteryPercent) {
	warn33=true;
	warn50=true;
	warn100=true;
	last_percent=PDABatteryPercent;
	if (!recharging) {
		recharging=true;
		if (PDABatteryFlag==BATTERY_FLAG_CHARGING || PDABatteryStatus==AC_LINE_ONLINE) {
			if (GiveBatteryWarnings(++numwarn))
	// LKTOKEN  _@M124_ = "BATTERY IS RECHARGING" 
			DoStatusMessage(gettext(TEXT("_@M124_")));
		}
	}
	return;
  }
	
  // if battery is same level, do nothing except when 100% during recharge
  if (last_percent == PDABatteryPercent) {
	if (recharging && (PDABatteryPercent==100) && warn100) {
		if (GiveBatteryWarnings(++numwarn))
	// LKTOKEN  _@M123_ = "BATTERY 100% CHARGED" 
		DoStatusMessage(gettext(TEXT("_@M123_")));
		warn100=false;
	}
	return;
  }

  // else battery is discharging
  recharging=false;

  // Time to give a message to the user, if necessary
  if (PDABatteryPercent <=5) {
	_stprintf(mbuf,_T("BATTERY LEVEL %d%% CRITIC!"),PDABatteryPercent);
	DoStatusMessage(mbuf);
	#ifndef DISABLEAUDIO
        PlayResource(TEXT("IDR_WAV_RED"));
	#endif

	// repeat after 1 minute, forced
	last_time=GPS_INFO.Time-(60*4);
	last_percent=PDABatteryPercent;
	return;
  }
  if (PDABatteryPercent <=10) {
	_stprintf(mbuf,_T("BATTERY LEVEL %d%% VERY LOW!"),PDABatteryPercent);
	DoStatusMessage(mbuf);
	// repeat after 2 minutes, forced
	last_time=GPS_INFO.Time-(60*3);
	last_percent=PDABatteryPercent;
	return;
  }
  if (PDABatteryPercent <=20) {
	_stprintf(mbuf,_T("BATTERY LEVEL %d%% LOW!"),PDABatteryPercent);
	DoStatusMessage(mbuf);
	last_time=GPS_INFO.Time;
	last_percent=PDABatteryPercent;
	return;
  }

  if (PDABatteryPercent <=30) {
	if (warn33) {
		_stprintf(mbuf,_T("BATTERY LEVEL %d%%"),PDABatteryPercent);
		DoStatusMessage(mbuf);
		warn33=false;
	}
	last_time=GPS_INFO.Time;
	last_percent=PDABatteryPercent;
	return;
  }
  // DISABLED
  if (PDABatteryPercent <=50) {
	if (warn50) {
		_stprintf(mbuf,_T("BATTERY LEVEL %d%%"),PDABatteryPercent);
	//	DoStatusMessage(mbuf);
		warn50=false;
	}
	last_time=GPS_INFO.Time;
	last_percent=PDABatteryPercent;
	return;
  }

}

// returns true if no problems with too many warnings
#define MAXBATTWARN   15
bool GiveBatteryWarnings(int numwarn)
{
  static bool toomany=false;

  if (toomany) return false;

  if (numwarn>MAXBATTWARN) {
	DoStatusMessage(_T("BATTERY WARNINGS DISABLED"));
	StartupStore(_T("... Too many battery warnings, disabling Battery Manager%s"),NEWLINE);
	toomany=true;
	return false;
  }
  return true;
}

// Play a sound from filesystem
void LKSound(const TCHAR *lpName) {
  #ifdef DISABLEAUDIO
  return false;
  #else
  static bool doinit=true;
  static bool working=false;
  static TCHAR sDir[MAX_PATH];

  if (doinit) {
	TCHAR srcfile[MAX_PATH];
	LocalPath(sDir,TEXT(LKD_SOUNDS));
	_stprintf(srcfile,TEXT("%s\\_SOUNDS"),sDir);
	if (  GetFileAttributes(srcfile) == 0xffffffff ) {
	        FailStore(_T("ERROR NO SOUNDS DIRECTORY CHECKFILE <%s>%s"),srcfile,NEWLINE);
		StartupStore(_T("------ LK8000 SOUNDS NOT WORKING!%s"),NEWLINE);
        } else
		working=true;
	doinit=false;
  }

  if (!working) return;
  TCHAR sndfile[MAX_PATH];
  _stprintf(sndfile,_T("%s\\%s"),sDir,lpName);
  sndPlaySound (sndfile, SND_ASYNC| SND_NODEFAULT );
  return;

  #endif
}

// Rescale automatically dialogs, using negative values to force rescaling
// Notice: SHOULD BE CALLED ONLY IF rWidth is negative, in order to avoid useless SetWindowPos
int RescaleWidth(const int rWidth) {

  // Always rescale negative widths
  if (rWidth <-1) {
	// Special case is when width is also the scale unit, which demonstrate we have a bug to fix here!
	if (rWidth == (int)(-246*InfoBoxLayout::dscale)){
		return LKwdlgConfig;
	}
	double i=(246.0 / abs(rWidth));
	if (i==0) {
		FailStore(_T("INTERNAL ERROR RESCALEWIDTH rWidth=%d"),rWidth);
		DoStatusMessage(_T("RESCALE ERR-001"));
		return rWidth;
	}
	int ri=(int)( (LKwdlgConfig/i) *InfoBoxLayout::dscale );
	// StartupStore(_T("... RescaleWidth(): rescale %d to %d\n"),rWidth, ri);
	if (ri>ScreenSizeX) return(ScreenSizeX);
	return (ri);
  }
  // else use the incoming rWidth but it is clearly an error
  DoStatusMessage(_T("RESCALE WARN-001"));
  return rWidth;
}

void ChangeWindCalcSpeed(const int newspeed) {

  WindCalcSpeed += (double)newspeed/SPEEDMODIFY;

}

// runmode 0: exec inside LocalPath home of LK8000
// runmode 1: exec inside 
bool LKRun(const TCHAR *prog, const int runmode, const DWORD dwaitime) {

  if (_tcslen(prog) <5) {
	StartupStore(_T("... LKRun failure: invalid exec path <%s>%s"),prog,NEWLINE);
	return false;
  }

  TCHAR path[MAX_PATH];

  if (runmode<0 || runmode>1) {
	StartupStore(_T("... LKRun failure: invalid runmode=%d %s"),runmode,NEWLINE);
	return false;
  }

  // mode 0: localpath , forced execution, with warnings if something goes wrong
  // mode 1: optional execution, no warnings if nothing found
  if (runmode<2) {
	LocalPath(path,prog);
	if (runmode==0) StartupStore(_T(". LKRun: exec <%s> background=%u%s"),path,dwaitime,NEWLINE);

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si,sizeof(STARTUPINFO));
	si.cb=sizeof(STARTUPINFO);
	si.wShowWindow= SW_SHOWNORMAL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	// if (!::CreateProcess(_T("C:\\WINDOWS\\notepad.exe"),_T(""), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) 
	if (!::CreateProcess(path,_T(""), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		if (runmode==0) StartupStore(_T("... LKRun exec FAILED%s"),NEWLINE);
		return false;
	}
	::WaitForSingleObject(pi.hProcess, dwaitime);
	StartupStore(_T(". LKRun exec terminated%s"),NEWLINE);
	return true;
  }

  return false;
}

void GotoWaypoint(const int wpnum) {
  if (!ValidWayPoint(wpnum)) {
	DoStatusMessage(_T("ERR-639 INVALID GOTO WPT"));
	return;
  }
  if (ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) {
	TCHAR wpname[NAME_SIZE+1];
	_tcscpy(wpname,WayPointList[wpnum].Name);
	wpname[10] = '\0';

	if (MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M158_ = "CONFIRM GOTO, ABORTING TASK?" 
	gettext(TEXT("_@M158_")),
	// LKTOKEN  _@M40_ = "A task is running!" 
	gettext(TEXT("_@M40_")),
	MB_YESNO|MB_ICONQUESTION) == IDYES) {
		LockTaskData();
		FlyDirectTo(wpnum);
		UnlockTaskData();
        }
  } else {
	LockTaskData();
	FlyDirectTo(wpnum);
	UnlockTaskData();
  }
}

void ToggleBaroAltitude() {
  if (!GPS_INFO.BaroAltitudeAvailable) {
	// LKTOKEN  _@M121_ = "BARO ALTITUDE NOT AVAILABLE" 
	DoStatusMessage(gettext(TEXT("_@M121_")));
	return;
  }
  EnableNavBaroAltitude=!EnableNavBaroAltitude;
  if (EnableNavBaroAltitude)
	// LKTOKEN  _@M756_ = "USING BARO ALTITUDE" 
	DoStatusMessage(gettext(TEXT("_@M756_")));
  else
	// LKTOKEN  _@M757_ = "USING GPS ALTITUDE" 
	DoStatusMessage(gettext(TEXT("_@M757_")));
}

#if FIX_MAPSIZE
bool ReducedMapSize() {
  if (MapWindow::IsMapFullScreen()) {
	if (MapWindow::isPan()) return false;
	return true;
  } else
	return false;
}
#endif

TCHAR * GetSizeSuffix(void) {
  static TCHAR suffixname[12];
  _stprintf(suffixname,_T("%03dx%03d"),ScreenSizeX,ScreenSizeY);
  return(suffixname);
}


