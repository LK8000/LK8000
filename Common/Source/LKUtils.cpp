/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKUtils.cpp,v 1.1 2010/12/11 23:45:28 root Exp root $
*/

#include "externs.h"
#include "Process.h"
#ifdef PNA
#include "LKHolux.h"
#endif
#include "utils/stringext.h"

extern TCHAR LastTaskFileName[MAX_PATH];


void ResetNearestTopology(void) {
  #if TESTBENCH
  StartupStore(_T(". ResetNearestTopology%s"),NEWLINE);
  #endif
  NearestBigCity.Valid=false;
  NearestCity.Valid=false;
  NearestSmallCity.Valid=false;
  NearestWaterArea.Valid=false;
}


// Rescale automatically dialogs, using negative values to force rescaling
// Notice: SHOULD BE CALLED ONLY IF rWidth is negative, in order to avoid useless SetWindowPos
int RescaleWidth(const int rWidth) {

  // Always rescale negative widths
  if (rWidth <-1) {
	// Special case is when width is also the scale unit, which demonstrate we have a bug to fix here!
	if (rWidth == (int)(-246*ScreenDScale)){
		return LKwdlgConfig;
	}
	double i=(246.0 / abs(rWidth));
	if (i==0) {
		FailStore(_T("INTERNAL ERROR RESCALEWIDTH rWidth=%d"),rWidth);
		DoStatusMessage(_T("RESCALE ERR-001"));
		return rWidth;
	}
	int ri=(int)( (LKwdlgConfig/i) *ScreenDScale );
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
	DoStatusMessage(MsgToken(121)); // BARO ALTITUDE NOT AVAILABLE
	return;
  }
  EnableNavBaroAltitude=!EnableNavBaroAltitude;
  if (EnableNavBaroAltitude)
	DoStatusMessage(MsgToken(1796)); // USING BARO ALTITUDE
   else
	DoStatusMessage(MsgToken(757)); // USING GPS ALTITUDE

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


extern void LK_wsplitpath(const WCHAR* path, WCHAR* drv, WCHAR* dir, WCHAR* name, WCHAR* ext);

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


//
// Master Time Reset
//
// This function is called when a valid GPS time (or a time taken from an IGC replay log)
// is verified to be gone back in time.
// This was normally happening when the device was switched off and back on some time later, until 3.1f.
// Now the time in LK is advancing also considering the date through a full year, so it is unlikely to happen after a valid fix.
//
// But this can also happen  after switching ON a PNA with no time battery, and thus a full reset.
// Time is appearing as 1/1/2000 12:00am , some times. So we only log the event.
//
// What we might do, is disable logging, resetting some functions etc.
//
void MasterTimeReset(void) {

  static bool silent=false;

  //
  // We want to avoid the single situation: no gps fix, PNA awaken with old time,
  // and still no gps fix. In this case, we would get several resets, and we only
  // give one until the fix is received again. At that time, time will advance normally
  // and we shall not remain stuck in mastertimereset state.
  //

  // No fix, and silent = no warning.
  if (GPS_INFO.NAVWarning && silent) {
	#if TESTBENCH
	StartupStore(_T("... (silent!) Master Time Reset %s%s"), WhatTimeIsIt(),NEWLINE);
	#endif
	return;
  }

  if (GPS_INFO.NAVWarning && !silent) {
	#if TESTBENCH
	StartupStore(_T("... Master Time Reset going silent, no fix! %s%s"), WhatTimeIsIt(),NEWLINE);
	#endif
	silent=true;
  } else
	silent=false; // arm it 

  StartupStore(_T("... Master Time Reset %s%s"), WhatTimeIsIt(),NEWLINE);
  #if TESTBENCH
  DoStatusMessage(_T("MASTER TIME RESET")); // no translation please
  #endif

  // Future possible handlings of this exception:
  // . Battery manager

  // Remember to lock anything needed to be locked

}


bool DoOptimizeRoute() {

  if (AircraftCategory != (AircraftCategory_t)umParaglider) return false;
  if (!PGOptimizeRoute) return false;

  if (!ValidTaskPoint(0) || !ValidTaskPoint(1)) return false;
  if (!ValidTaskPoint(ActiveWayPoint)) return false;

  return true;

}

int CurrentMarker=RESWP_FIRST_MARKER-1;

// Returns the next waypoint index to use for marker
int GetVirtualWaypointMarkerSlot(void) {

  if (CurrentMarker==RESWP_LAST_MARKER) 
	CurrentMarker=RESWP_FIRST_MARKER-1;

  return ++CurrentMarker;
}


void TaskStartMessage(void) {

  TCHAR TempTime[40];
  TCHAR TempAlt[40];
  TCHAR TempSpeed[40];
  Units::TimeToText(TempTime, (int)TimeLocal((int)CALCULATED_INFO.TaskStartTime));
  _stprintf(TempAlt, TEXT("%.0f %s"), CALCULATED_INFO.TaskStartAltitude*ALTITUDEMODIFY, Units::GetAltitudeName());
  _stprintf(TempSpeed, TEXT("%.0f %s"), CALCULATED_INFO.TaskStartSpeed*TASKSPEEDMODIFY, Units::GetTaskSpeedName());

  TCHAR TempAll[300];
  _stprintf(TempAll, TEXT("\r\n%s: %s\r\n%s:%s\r\n%s: %s"),
  // Altitude
  gettext(TEXT("_@M89_")),
  TempAlt,
  // Speed
  gettext(TEXT("_@M632_")),
  TempSpeed,
  // Time
  gettext(TEXT("_@M720_")),
  TempTime);

  // ALWAYS issue DoStatusMessage BEFORE sounds, if possible.
  // LKTOKEN  _@M692_ = "Task Start"
  DoStatusMessage(gettext(TEXT("_@M692_")), TempAll);
}

void TaskFinishMessage(void) {

  TCHAR TempTime[40];
  TCHAR TempAlt[40];
  TCHAR TempSpeed[40];
  TCHAR TempTskSpeed[40];

  Units::TimeToText(TempTime, (int)TimeLocal((int)GPS_INFO.Time));
  _stprintf(TempAlt, TEXT("%.0f %s"), CALCULATED_INFO.NavAltitude*ALTITUDEMODIFY, Units::GetAltitudeName());
  _stprintf(TempSpeed, TEXT("%.0f %s"), GPS_INFO.Speed*TASKSPEEDMODIFY, Units::GetTaskSpeedName());
  _stprintf(TempTskSpeed, TEXT("%.2f %s"), CALCULATED_INFO.TaskSpeedAchieved*TASKSPEEDMODIFY, Units::GetTaskSpeedName());

  TCHAR TempAll[300];

  _stprintf(TempAll, TEXT("\r\n%s: %s\r\n%s:%s\r\n%s: %s\r\n%s: %s"),
  // Altitude
  gettext(TEXT("_@M89_")),
  TempAlt,
  // Speed
  gettext(TEXT("_@M632_")),
  TempSpeed,
  // Time
  gettext(TEXT("_@M720_")),
  TempTime,
  // task speed
  gettext(TEXT("_@M697_")),
  TempTskSpeed);

  // LKTOKEN  _@M687_ = "Task Finish"
  DoStatusMessage(gettext(TEXT("_@M687_")), TempAll);

}

extern void MSG_NotEnoughMemory(void);
void OutOfMemory(char *where, int line) {

  StartupStore(_T(">>> OUT OF MEMORY in <%S> line %d%s"),where,line,NEWLINE);
  MSG_NotEnoughMemory();

}
