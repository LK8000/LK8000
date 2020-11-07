/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKUtils.cpp,v 1.1 2010/12/11 23:45:28 root Exp root $
*/

#include "externs.h"
#include "LKProcess.h"
#if defined(PNA) && defined(UNDER_CE)
#include "LKHolux.h"
#endif
#include "utils/stringext.h"
#include "Dialogs.h"


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


void ChangeWindCalcSpeed(const int newspeed) {

  WindCalcSpeed += (double)newspeed/SPEEDMODIFY;

}

#ifdef _WIN32
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
	if (runmode==0) StartupStore(_T(". LKRun: exec <%s> background=%lu%s"),path,dwaitime,NEWLINE);

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si,sizeof(STARTUPINFO));
	si.cb=sizeof(STARTUPINFO);
	si.wShowWindow= SW_SHOWNORMAL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	// if (!::CreateProcess(_T("C:\\WINDOWS\\notepad.exe"),_T(""), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))

    /*cf. http://msdn.microsoft.com/en-us/library/windows/desktop/ms682425(v=vs.85).aspx
     * The Unicode version of this function, CreateProcessW, can modify the contents of this string. Therefore, this
     * parameter cannot be a pointer to read-only memory (such as a const variable or a literal string). If this parameter
     * is a constant string, the function may cause an access violation.
     * */
	if (!::CreateProcess(path,NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		if (runmode==0) StartupStore(_T("... LKRun exec FAILED%s"),NEWLINE);
		return false;
	}
	::WaitForSingleObject(pi.hProcess, dwaitime);
	::CloseHandle(pi.hProcess);
	::CloseHandle(pi.hThread);
	StartupStore(_T(". LKRun exec terminated%s"),NEWLINE);
	return true;
  }
  return false;
}
#endif

void GotoWaypoint(const int wpnum) {
  if (!ValidWayPoint(wpnum)) {
	DoStatusMessage(_T("ERR-639 INVALID GOTO WPT"));
	return;
  }
  if (ValidTaskPoint(ActiveTaskPoint) && ValidTaskPoint(1)) {
	TCHAR wpname[NAME_SIZE+1];
	_tcscpy(wpname,WayPointList[wpnum].Name);
	wpname[10] = '\0';

	if (MessageBoxX(
	// LKTOKEN  _@M158_ = "CONFIRM GOTO, ABORTING TASK?"
	MsgToken(158),
	// LKTOKEN  _@M40_ = "A task is running!"
	MsgToken(40),
	mbYesNo) == IdYes) {
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

void LKRunStartEnd(bool start) {
#ifdef WIN32
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
#endif
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
  BUGSTOP_LKASSERT(buffer!=NULL);
  if (buffer==NULL) return NULL;

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

  _tcsncpy(buffer, name, bufferLen);
  #if BUGSTOP
  LKASSERT(bufferLen>0);
  #endif
  if (bufferLen>0)
  buffer[bufferLen - 1] = _T('\0');

  return buffer;
}

//
// This is used globally to determine if Contest facility is in use or not
// We might use here also a configuration option shortly.
// Notice that engine Reset() is done in any case.
//
bool UseContestEngine(void) {
  // Gliding and Paragliding mode always have the engine running
  if (ISGLIDER || ISPARAGLIDER) return true;
  // All other modes like Car and GA, will need 1.5 page to be ON
  if (!ConfIP[LKMODE_INFOMODE][IM_CONTEST]) return false;

  return true;

}


extern void LK_tsplitpath(const TCHAR* path, TCHAR* drv, TCHAR* dir, TCHAR* name, TCHAR* ext);

//
// Returns the LKW extension index of the incoming suffix, or -1
//
int GetWaypointFileFormatType(const TCHAR* wfilename) {

  TCHAR wextension[MAX_PATH];
  LK_tsplitpath(wfilename, nullptr,nullptr,nullptr,wextension);

  if ( _tcsicmp(wextension,_T(LKS_WP_CUP))==0) {
       return LKW_CUP;
  }
  if ( _tcsicmp(wextension,_T(LKS_WP_WINPILOT))==0 ) {
       return LKW_DAT;
  }
  if ( _tcsicmp(wextension,_T(LKS_WP_COMPE))==0 ) {
       return LKW_COMPE;
  }
  if ( _tcsicmp(wextension,_T(LKS_OPENAIP))==0 ) {
       return LKW_OPENAIP;
  }

  return -1;

}


//
// Master Time Reset -- only for diagnostics in TESTBENCH mode
//
// This function is called when a valid GPS time (or a time taken from an IGC replay log)
// is verified to be gone back in time.
// This was normally happening when the device was switched off and back on some time later, until 3.1f.
// Now the time in LK is advancing also considering the date through a full year,
// so it is unlikely to happen after a valid fix.
//
// But this can also happen  after switching ON a PNA with no time battery, and thus a full reset.
// Time is appearing as 1/1/2000 12:00am , some times. So we only log the event.
//
// It can also happen when two gps feed are mixed through a multiplexer providing two gps fix at the same time,
// one of them 1 second late. Very bad indeed.
//
// What we might do, is disable logging, resetting some functions etc.
//
void MasterTimeReset(void) {
#if TESTBENCH
  static bool silent=false;

  //
  // We want to avoid the single situation: no gps fix, PNA awaken with old time,
  // and still no gps fix. In this case, we would get several resets, and we only
  // give one until the fix is received again. At that time, time will advance normally
  // and we shall not remain stuck in mastertimereset state.
  //

  // No fix, and silent = no warning.
  if (GPS_INFO.NAVWarning && silent) {
	StartupStore(_T("... (silent!) Master Time Reset %s%s"), WhatTimeIsIt(),NEWLINE);
	return;
  }

  if (GPS_INFO.NAVWarning && !silent) {
	StartupStore(_T("... Master Time Reset going silent, no fix! %s%s"), WhatTimeIsIt(),NEWLINE);
	silent=true;
  } else
	silent=false; // arm it

  StartupStore(_T("... Master Time Reset %s%s"), WhatTimeIsIt(),NEWLINE);
  DoStatusMessage(_T("MASTER TIME RESET")); // no translation please

  // Future possible handlings of this exception:
  // . Battery manager
  // . Trip computer

  // Remember to lock anything needed to be locked
#else
  static unsigned short count=0;
  if (count>9) return;
  if (GPS_INFO.NAVWarning) return;
  count++;
  StartupStore(_T("... Master Time Reset %s%s"), WhatTimeIsIt(),NEWLINE);
  DoStatusMessage(_T("MASTER TIME RESET")); // no translation please
#endif // MasterTimeReset
}

bool DoOptimizeRoute() {

  if (AircraftCategory != (AircraftCategory_t)umParaglider) return false;
  if (!PGOptimizeRoute) return false;

  if (!ValidTaskPoint(0) || !ValidTaskPoint(1)) return false;
  if (!ValidTaskPoint(ActiveTaskPoint)) return false;

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
  Units::TimeToText(TempTime, LocalTime(CALCULATED_INFO.TaskStartTime));
  _stprintf(TempAlt, TEXT("%.0f %s"), CALCULATED_INFO.TaskStartAltitude*ALTITUDEMODIFY, Units::GetAltitudeName());
  _stprintf(TempSpeed, TEXT("%.0f %s"), CALCULATED_INFO.TaskStartSpeed*TASKSPEEDMODIFY, Units::GetTaskSpeedName());

  TCHAR TempAll[300];
  _stprintf(TempAll, TEXT("\r\n%s: %s\r\n%s:%s\r\n%s: %s"),
  // Altitude
  MsgToken(89),
  TempAlt,
  // Speed
  MsgToken(632),
  TempSpeed,
  // Time
  MsgToken(720),
  TempTime);

  // ALWAYS issue DoStatusMessage BEFORE sounds, if possible.
  // LKTOKEN  _@M692_ = "Task Start"
  DoStatusMessage(MsgToken(692), TempAll);
}

void TaskFinishMessage(void) {

  TCHAR TempTime[40];
  TCHAR TempAlt[40];
  TCHAR TempSpeed[40];
  TCHAR TempTskSpeed[40];

  Units::TimeToText(TempTime, LocalTime());
  _stprintf(TempAlt, TEXT("%.0f %s"), CALCULATED_INFO.NavAltitude*ALTITUDEMODIFY, Units::GetAltitudeName());
  _stprintf(TempSpeed, TEXT("%.0f %s"), GPS_INFO.Speed*TASKSPEEDMODIFY, Units::GetTaskSpeedName());
  _stprintf(TempTskSpeed, TEXT("%.2f %s"), CALCULATED_INFO.TaskSpeedAchieved*TASKSPEEDMODIFY, Units::GetTaskSpeedName());

  TCHAR TempAll[300];

  _stprintf(TempAll, TEXT("\r\n%s: %s\r\n%s:%s\r\n%s: %s\r\n%s: %s"),
  // Altitude
  MsgToken(89),
  TempAlt,
  // Speed
  MsgToken(632),
  TempSpeed,
  // Time
  MsgToken(720),
  TempTime,
  // task speed
  MsgToken(697),
  TempTskSpeed);

  // LKTOKEN  _@M687_ = "Task Finish"
  DoStatusMessage(MsgToken(687), TempAll);

}

extern void MSG_NotEnoughMemory(void);
void OutOfMemory(const TCHAR *where, int line) {

  StartupStore(_T(">>> OUT OF MEMORY in <%s> line %d%s"),where,line,NEWLINE);
  MSG_NotEnoughMemory();

}
