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
#include "lk8000.h"
#include "InfoBoxLayout.h"
#include "Utils2.h"
#include "Cpustats.h"
#include "device.h"
#include "Logger.h"
#include "Parser.h"
#include "WaveThread.h"
#include "LKUtils.h"
#include "Message.h"
#include "McReady.h"
#include "InputEvents.h"
#include "LKMapWindow.h"
#ifdef PNA
#include "LKHolux.h"
#endif

#include "utils/stringext.h"
#include "utils/heapcheck.h"


extern TCHAR LastTaskFileName[MAX_PATH];

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
		// LKTOKEN _@M1352_ "BATTERY LEVEL"
		_stprintf(mbuf,_T("%s %d%%"), gettext(TEXT("_@M1352_")), PDABatteryPercent);
		DoStatusMessage(mbuf);
		warn50=false;
	}
	// special case, pdabattery is 0...
	if (PDABatteryPercent <1) {
		StartupStore(_T("... LK Battery Manager disabled, low battery %s"),NEWLINE);
		// LKTOKEN _@M1353_ "BATTERY MANAGER DISABLED"
		DoStatusMessage(gettext(TEXT("_@M1353_")));
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
	// LKTOKEN _@M1354_ "BATTERY LEVEL CRITIC!"
	_stprintf(mbuf,_T("%d%% %s"), PDABatteryPercent, gettext(TEXT("_@M1354_")));
	DoStatusMessage(mbuf);
	#ifndef DISABLEAUDIO
        LKSound(TEXT("LK_RED.WAV"));
	#endif

	// repeat after 1 minute, forced
	last_time=GPS_INFO.Time-(60*4);
	last_percent=PDABatteryPercent;
	return;
  }
  if (PDABatteryPercent <=10) {
	// LKTOKEN _@M1355_ "BATTERY LEVEL VERY LOW!"
	_stprintf(mbuf,_T("%d%% %s"), PDABatteryPercent, gettext(TEXT("_@M1355_")));
	DoStatusMessage(mbuf);
	// repeat after 2 minutes, forced
	last_time=GPS_INFO.Time-(60*3);
	last_percent=PDABatteryPercent;
	return;
  }
  if (PDABatteryPercent <=20) {
	// LKTOKEN _@M1356_ "BATTERY LEVEL LOW!"
	_stprintf(mbuf,_T("%d%% %s"), PDABatteryPercent, gettext(TEXT("_@M1356_")));
	DoStatusMessage(mbuf);
	last_time=GPS_INFO.Time;
	last_percent=PDABatteryPercent;
	return;
  }

  if (PDABatteryPercent <=30) {
	if (warn33) {
		// LKTOKEN _@M1352_ "BATTERY LEVEL"
		_stprintf(mbuf, _T("%s %d%%"), gettext(TEXT("_@M1352_")), PDABatteryPercent);
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
		// LKTOKEN _@M1352_ "BATTERY LEVEL"
	//	_stprintf(mbuf, _T("%s %d%%"), gettext(TEXT("_@M1352_")), PDABatteryPercent);
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
	// LKTOKEN _@M1357_ "BATTERY WARNINGS DISABLED"
	DoStatusMessage(gettext(TEXT("_@M1357_")));
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

  #ifdef PNA
  if (DeviceIsGM130) {
	MessageBeep(0xffffffff); // default
	return;
  }
  #endif   
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
#if USEIBOX
	if (rWidth == (int)(-246*InfoBoxLayout::dscale)){
#else
	if (rWidth == (int)(-246*ScreenDScale)){
#endif
		return LKwdlgConfig;
	}
	double i=(246.0 / abs(rWidth));
	if (i==0) {
		FailStore(_T("INTERNAL ERROR RESCALEWIDTH rWidth=%d"),rWidth);
		DoStatusMessage(_T("RESCALE ERR-001"));
		return rWidth;
	}
#if USEIBOX
	int ri=(int)( (LKwdlgConfig/i) *InfoBoxLayout::dscale );
#else
	int ri=(int)( (LKwdlgConfig/i) *ScreenDScale );
#endif
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
		OvertargetMode=OVT_TASK;
		UnlockTaskData();
        }
  } else {
	LockTaskData();
	FlyDirectTo(wpnum);
	OvertargetMode=OVT_TASK;
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

TCHAR * GetSizeSuffix(void) {
  static TCHAR suffixname[12];
  _stprintf(suffixname,_T("%03dx%03d"),ScreenSizeX,ScreenSizeY);
  return(suffixname);
}


void LKRunStartEnd(bool start) {
  if (start) {
	LKRun(_T("PREROTATE1.EXE"),1,5000); 
	LKRun(_T("PREROTATE2.EXE"),1,5000);
	LKRun(_T("PREROTATE3.EXE"),1,5000);
	LKRun(_T("PRELOAD_00.EXE"),1,0);
	LKRun(_T("PRELOAD_05.EXE"),1,5000);
	LKRun(_T("PRELOAD_30.EXE"),1,30000);
	LKRun(_T("PRELOAD_60.EXE"),1,60000);
	LKRun(_T("PRELOAD_99.EXE"),1,INFINITE);
  } else {
	LKRun(_T("ENDLOAD_00.EXE"),1,0);
	LKRun(_T("ENDLOAD_05.EXE"),1,5000);
	LKRun(_T("ENDLOAD_30.EXE"),1,30000);
	LKRun(_T("ENDLOAD_60.EXE"),1,60000);
	LKRun(_T("ENDROTATE1.EXE"),1,5000); 
	LKRun(_T("ENDROTATE2.EXE"),1,5000);
	LKRun(_T("ENDROTATE3.EXE"),1,5000);
	LKRun(_T("ENDLOAD_99.EXE"),1,INFINITE);

  }
}


// Reads line from UTF-8 encoded text file.
// File must be open in binary read mode.
bool ReadULine(ZZIP_FILE* fp, TCHAR *unicode, int maxChars)
{
  unsigned char buf[READLINE_LENGTH * 2];

  long startPos = zzip_tell(fp);

  if (startPos < 0) {
    StartupStore(_T(". ftell() error = %d%s"), errno, NEWLINE);
    return(false);
  }

  size_t nbRead = zzip_fread(buf, 1, sizeof(buf) - 1, fp);
  
  if (nbRead == 0)
    return(false);

  buf[nbRead] = '\0';

  // find new line (CR/LF/CRLF) in the string and terminate string at that position
  size_t i;
  for (i = 0; i < nbRead; i++) {
    if (buf[i] == '\n')
    {
      buf[i++] = '\0';
      if (buf[i] == '\r')
        i++;
      break;
    }

    if (buf[i] == '\r')
    {
      buf[i++] = '\0';
      if (buf[i] == '\n')
        i++;
      break;
    }
  }

  // next reading will continue after new line
  zzip_seek(fp, startPos + i, SEEK_SET);

  // skip leading BOM
  char* begin = (char*) buf;
  if (buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF)
    begin += 3;

  return(utf2unicode(begin, unicode, maxChars) >= 0);
}


/** 
 * @brief Returns task file name
 * 
 * Function obtains task file path and strips the directory part and file
 * extension from it.
 * 
 * @param bufferLen The length of the buffer
 * @param buffer Buffer for the task file name 
 * 
 * @return Buffer with filled data
 */
const TCHAR *TaskFileName(unsigned bufferLen, TCHAR buffer[])
{
  TCHAR name[MAX_PATH] = { _T('\0') };
  
  LockTaskData();
  int len = _tcslen(LastTaskFileName);
  if(len > 0) {
    int index = 0;
    TCHAR *src = LastTaskFileName;
    while ((*src != _T('\0')) && (*src != _T('.'))) {
      if ((*src == _T('\\')) || (*src == _T('/'))) {
        index = 0;
      }
      else {
        name[index] = *src;
        index++;
      }
      src++;
    }
    name[index] = _T('\0');
  }
  UnlockTaskData();
  
  _sntprintf(buffer, bufferLen, name);
  buffer[bufferLen - 1] = _T('\0');
  
  return buffer;
}

//
// This is used globally to determine if Contest facility is in use or not
// We might use here also a configuration option shortly.
// Notice that engine Reset() is done in any case.
//
bool UseContestEngine(void) {

  // Gliding mode always have the engine running
  if (ISGLIDER) return true;
  // All other modes like paragliding, Car and GA, will need 1.5 page to be ON
  if (!ConfIP[LKMODE_INFOMODE][IM_CONTEST]) return false;

  return true;

}


/* 
 * Implementation of the _splitpath runtime library function with wide character strings
 * Copyright 2000, 2004 Martin Fuchs -- GPL licensed - WINE project
 */
void LK_wsplitpath(const WCHAR* path, WCHAR* drv, WCHAR* dir, WCHAR* name, WCHAR* ext)
{
	const WCHAR* end; /* end of processed string */
	const WCHAR* p;	  /* search pointer */
	const WCHAR* s;	  /* copy pointer */

	/* extract drive name */
	if (path[0] && path[1]==':') {
		if (drv) {
			*drv++ = *path++;
			*drv++ = *path++;
			*drv = '\0';
		}
	} else if (drv)
		*drv = '\0';

	/* search for end of string or stream separator */
	for(end=path; *end && *end!=':'; )
		end++;

	/* search for begin of file extension */
	for(p=end; p>path && *--p!='\\' && *p!='/'; )
		if (*p == '.') {
			end = p;
			break;
		}

	if (ext)
		for(s=end; (*ext=*s++); )
			ext++;

	/* search for end of directory name */
	for(p=end; p>path; )
		if (*--p=='\\' || *p=='/') {
			p++;
			break;
		}

	if (name) {
		for(s=p; s<end; )
			*name++ = *s++;

		*name = '\0';
	}

	if (dir) {
		for(s=path; s<p; )
			*dir++ = *s++;

		*dir = '\0';
	}
}

//
// Returns the LKW extension index of the incoming suffix, or -1
//
int GetWaypointFileFormatType(const wchar_t* wfilename) {

  TCHAR wextension[MAX_PATH];
  TCHAR wdrive[MAX_PATH];
  TCHAR wdir[MAX_PATH];
  TCHAR wname[MAX_PATH];
  LK_wsplitpath(wfilename, wdrive,wdir,wname,wextension);

  //StartupStore(_T("... wdrive=%s\n"),wdrive);
  //StartupStore(_T("... wdir=%s\n"),wdir);
  //StartupStore(_T("... wname=%s\n"),wname);
  //StartupStore(_T("... wext=%s\n"),wextension);

  if ( wcscmp(wextension,_T(".cup"))==0 ||
    wcscmp(wextension,_T(".CUP"))==0 ||
    wcscmp(wextension,_T(".Cup"))==0) {
       return LKW_CUP;
  }
  if ( wcscmp(wextension,_T(".dat"))==0 ||
    wcscmp(wextension,_T(".DAT"))==0 ||
    wcscmp(wextension,_T(".Dat"))==0) {
       return LKW_DAT;
  }
  if ( wcscmp(wextension,_T(".wpt"))==0 ||
    wcscmp(wextension,_T(".WPT"))==0 ||
    wcscmp(wextension,_T(".Wpt"))==0) {
       return LKW_COMPE;
  }

  return -1;

}


// #define DEBUG_LKALARMS	1

void InitAlarms(void) {

  #if DEBUG_LKALARMS
  StartupStore(_T("...... Alarms: InitAlarms\n"));
  #endif
  int i;
  for (i=0; i<MAXLKALARMS; i++) {
	LKalarms[i].triggervalue=0;
	LKalarms[i].lastvalue=0;
	LKalarms[i].lasttriggertime=0.0;
	LKalarms[i].triggerscount=0;
  }
	/* Test values
	LKalarms[0].triggervalue=500;
	LKalarms[1].triggervalue=0;
	LKalarms[2].triggervalue=1200;
	*/
}

#if DEBUG_LKALARMS
#undef LKALARMSINTERVAL
#undef MAXLKALARMSTRIGGERS
#define LKALARMSINTERVAL 10
#define MAXLKALARMSTRIGGERS 3
#endif

// alarms in range 0-(MAXLKALARMS-1), that is  0-2
bool CheckAlarms(unsigned short al) {

  int i;

  // safe check
  if (al>=MAXLKALARMS) return false;

  // Alarms are working only with a valid GPS fix. No navigator, no alarms.
  if (GPS_INFO.NAVWarning) return false;

  // do we have a valid alarm request?
  if ( LKalarms[al].triggervalue == 0) return false;

  // We check for duplicates. We could do it only when config is changing, right now.
  // However, maybe we can have LK set automatically alarms in the future.
  // Duplicates filter is working giving priority to the lowest element in the list
  // We don't want more than 1 alarm for the same trigger value
  for (i=0; i<=al; i++) {
	if (i==al) continue; // do not check against ourselves
	// if a previous alarm has the same value, we are a duplicate
	if (LKalarms[al].triggervalue == LKalarms[i].triggervalue) {
		#if DEBUG_LKALARMS
		StartupStore(_T("...... Alarms: duplicate value [%d]=[%d] =<%d>\n"), al, i, LKalarms[i].triggervalue);
		#endif
		return false;
	}
  }

  // ok so this is not a duplicated alarm, lets check if we have overcounted
  if (LKalarms[al].triggerscount >= MAXLKALARMSTRIGGERS) {
	#if DEBUG_LKALARMS
	StartupStore(_T("...... Alarms: count exceeded for [%d]\n"),al);
	#endif
	return false;
  }

  // if too early we ignore it in any case
  if (GPS_INFO.Time < (LKalarms[al].lasttriggertime + LKALARMSINTERVAL)) {
	#if DEBUG_LKALARMS
	StartupStore(_T("...... Alarms: too early for [%d], still %.0f seconds to go\n"),al,
	(LKalarms[al].lasttriggertime + LKALARMSINTERVAL)- GPS_INFO.Time);
	#endif
	return false;
  }

  // So this is a potentially valid alarm to check

  //
  // First we check for altitude alarms , 0-2
  //
  if (al<3) {

	int navaltitude=(int)CALCULATED_INFO.NavAltitude;

	// is this is the first valid sample?
	if (LKalarms[al].lastvalue==0) {
		LKalarms[al].lastvalue= navaltitude;
		#if DEBUG_LKALARMS
		StartupStore(_T("...... Alarms: init lastvalue [%d] = %d\n"),al,LKalarms[al].lastvalue);
		#endif
		return false;
	}

	// if we were previously below trigger altitude
	if (LKalarms[al].lastvalue< LKalarms[al].triggervalue) {
		#if DEBUG_LKALARMS
		StartupStore(_T("...... Alarms: armed lastvalue [%d] = %d < trigger <%d>\n"),al,
		LKalarms[al].lastvalue,LKalarms[al].triggervalue);
		#endif
		// if we are now over the trigger altitude
		if (navaltitude >= LKalarms[al].triggervalue) {
			#if DEBUG_LKALARMS
			StartupStore(_T("...... Alarms: RING [%d] = %d\n"),al,navaltitude);
			#endif
			// bingo. first reset last value , update lasttime and counter
			LKalarms[al].lastvalue=0;
			LKalarms[al].triggerscount++;
			LKalarms[al].lasttriggertime = GPS_INFO.Time;
			return true;
		}
	}

	// otherwise simply update lastvalue
	LKalarms[al].lastvalue=navaltitude;
	return false;

  } // end altitude alarms


  // other alarms here, or failed
  return false;

}


//
// Master Time Reset
// This function is called when a valid GPS time (or a time taken from an IGC replay log)
// is verified to be gone back in time, or more than 2 hours have passed since last GPS
// fix was received.
// This is normally happening when the device was switched off and back on some time later.
// What we do, is disable logging, resetting some functions etc.
//
void MasterTimeReset(void) {

  #include "./LKincludetexttime.cpp"
  StartupStore(_T("... Master Time Reset %s%s"), time_temp,NEWLINE);
  #if ALPHADEBUG
  DoStatusMessage(_T("MASTER TIME RESET")); // no translation please
  #endif

  // Remember to lock anything needed to be locked



}
