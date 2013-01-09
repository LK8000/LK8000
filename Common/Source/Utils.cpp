/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"
#include "RGB.h"
#include "Modeltype.h"
#include "dlgTools.h"
#include "TraceThread.h"

long GetUTCOffset(void) {
  return UTCOffset;
}


#if 0 // REMOVE ANIMATION
static RECT AnimationRectangle = {0,0,0,0};

void SetSourceRectangle(RECT fromRect) {
  AnimationRectangle = fromRect;
}


RECT WINAPI DrawWireRects(LPRECT lprcTo, UINT nMilliSecSpeed)
{
    return AnimationRectangle;
}
#endif 



int GetTextWidth(HDC hDC, TCHAR *text) {
  SIZE tsize;
  GetTextExtentPoint(hDC, text, _tcslen(text), &tsize);
  return tsize.cx;
}


void RestartCommPorts() {

  StartupStore(TEXT(". RestartCommPorts%s"),NEWLINE);

  LockComm();

  devClose(devA());
  devClose(devB());

  NMEAParser::Reset();

  devInit(TEXT(""));

  UnlockComm();

}


void TriggerGPSUpdate()
{
  SetEvent(dataTriggerEvent);
}

// This is currently doing nothing.
void TriggerVarioUpdate()
{
}

//
// When Debounce(int) was introduced, the old Debounce was incorrect, always returning true;
// Possible undebounced triggers could be issued> to check in WindowControls and many parts.
// No complaints so far, but this should be fixed. Otherwise the debounceTimeout was UNUSED
// and the simple Debounce(void) call was always true!!
static DWORD fpsTimeLast= 0;

bool Debounce(void) {
  DWORD fpsTimeThis = ::GetTickCount();
  DWORD dT = fpsTimeThis-fpsTimeLast;

  if (dT>(unsigned int)debounceTimeout) {
    fpsTimeLast = fpsTimeThis;
    return true;
  } else {
    return false;
  }
}

bool Debounce(int dtime) {
  DWORD fpsTimeThis = ::GetTickCount();
  DWORD dT = fpsTimeThis-fpsTimeLast;

  if (dT>(unsigned int)dtime) {
    fpsTimeLast = fpsTimeThis;
    return true;
  } else {
    return false;
  }
}

//
// Let's get rid of BOOOOls soon!!!
bool BOOL2bool(BOOL a) {
  if (a==TRUE) return true;
  return false;
}



// Get the infobox type from configuration, selecting position i
// From 1-8 auxiliaries
//     0-16 dynamic page
//
int GetInfoboxType(int i) {

	int retval = 0;
	if (i<1||i>16) return LK_ERROR;

	// it really starts from 0
	if (i<=8)
		retval = (InfoType[i-1] >> 24) & 0xff; // auxiliary
	else {
		switch ( MapWindow::mode.Fly() ) {
			case MapWindow::Mode::MODE_FLY_CRUISE:
				retval = (InfoType[i-9] >> 8) & 0xff;
				break;
			case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
				retval = (InfoType[i-9] >> 16) & 0xff;
				break;
			case MapWindow::Mode::MODE_FLY_CIRCLING:
				retval = (InfoType[i-9]) & 0xff; 
				break;
			default:
				// impossible case, show twice auxiliaries
				retval = (InfoType[i-9] >> 24) & 0xff;
				break;
		}
	}

	return min(NumDataOptions-1,retval);
}

// Returns the LKProcess index value for configured infobox (0-8) for dmCruise, dmFinalGlide, Auxiliary, dmCircling
// The function name is really stupid...
// dmMode is an enum, we simply use for commodity
int GetInfoboxIndex(int i, MapWindow::Mode::TModeFly dmMode) {
	int retval = 0;
	if (i<0||i>8) return LK_ERROR;

	switch(dmMode) {
		case MapWindow::Mode::MODE_FLY_CRUISE:
			retval = (InfoType[i-1] >> 8) & 0xff;
			break;
		case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
			retval = (InfoType[i-1] >> 16) & 0xff;
			break;
		case MapWindow::Mode::MODE_FLY_CIRCLING:
			retval = (InfoType[i-1]) & 0xff; 
			break;
		default:
			// default is auxiliary
			retval = (InfoType[i-1] >> 24) & 0xff; 
			break;
	}
	return min(NumDataOptions-1,retval);
}

// Used for calculation, but does not affect IsSafetyMacCreadyInUse so careful
double GetMacCready(int wpindex, short wpmode)
{
	if (WayPointCalc[wpindex].IsLandable) {
		if (MACCREADY>GlidePolar::SafetyMacCready) 
			return MACCREADY;
		else
			return GlidePolar::SafetyMacCready;
	}
	return MACCREADY;

}


void SetOverColorRef() {
  switch(OverColor) {
	case OcWhite:
		OverColorRef=RGB_WHITE;
		break;
	case OcBlack:
		OverColorRef=RGB_SBLACK;
		break;
	case OcBlue:
		OverColorRef=RGB_DARKBLUE;
		break;
	case OcGreen:
		OverColorRef=RGB_GREEN;
		break;
	case OcYellow:
		OverColorRef=RGB_YELLOW;
		break;
	case OcCyan:
		OverColorRef=RGB_CYAN;
		break;
	case OcOrange:
		OverColorRef=RGB_ORANGE;
		break;
	case OcGrey:
		OverColorRef=RGB_GREY;
		break;
	case OcDarkGrey:
		OverColorRef=RGB_DARKGREY;
		break;
	case OcDarkWhite:
		OverColorRef=RGB_DARKWHITE;
		break;
	case OcAmber:
		OverColorRef=RGB_AMBER;
		break;
	case OcLightGreen:
		OverColorRef=RGB_LIGHTGREEN;
		break;
	case OcPetrol:
		OverColorRef=RGB_PETROL;
		break;
	default:
		OverColorRef=RGB_MAGENTA;
		break;
  }
}



bool CheckClubVersion() {
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcfile, _T("CLUB"));
  if (  GetFileAttributes(srcfile) == 0xffffffff ) return false;
  return true;
}

void ClubForbiddenMsg() {
  MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M503_ = "Operation forbidden on CLUB devices" 
	gettext(TEXT("_@M503_")),
	_T("CLUB DEVICE"), 
	MB_OK|MB_ICONEXCLAMATION);
        return;
}


// Are we using lockmode? What is the current status?
bool LockMode(const short lmode) {

  switch(lmode) {
	case 0:		// query availability of LockMode
		return true;
		break;

	case 1:		// query lock/unlock status
		return LockModeStatus;
		break;

	case 2:		// invert lock status
		LockModeStatus = !LockModeStatus;
		return LockModeStatus;
		break;

	case 3:		// query button is usable or not
		if (ISPARAGLIDER)
			// Positive if not flying
			return (!CALCULATED_INFO.Flying);
		else return true;
		break;

	case 9:		// Check if we can unlock the screen
		if (ISPARAGLIDER) {
			// Automatic unlock
			if (CALCULATED_INFO.Flying) {
				if ( (GPS_INFO.Time - CALCULATED_INFO.TakeOffTime)>10) {
					LockModeStatus=false;
				}
			}
		}
		return LockModeStatus;
		break;

	default:
		return false;
		break;
  }

  return 0;

}

//
// Rotate flags: TASK, FAI, ALLOFF
//
void ToggleDrawTaskFAI(void) {

  #if 0
  // AllOn -> Task
  if (Flags_DrawTask&&Flags_DrawFAI) {
	Flags_DrawFAI=false;
	return;
  }
  #endif 

  // Task -> FAI
  if (Flags_DrawTask&&!Flags_DrawFAI) {
	Flags_DrawTask=false;
	Flags_DrawFAI=true;
	return;
  }
  // FAI --> ALLOFF
  if (!Flags_DrawTask&&Flags_DrawFAI) {
	Flags_DrawTask=false;
	Flags_DrawFAI=false;
	return;
  }
  // ALLOFF -> Task
  Flags_DrawTask=true;
  Flags_DrawFAI=false;

}

#if TESTBENCH
int Test_NIBLSCALE(short x, const int line, const char *file) {
  if (x<0||x>MAXIBLSCALE) {
	StartupStore(_T("[ASSERT FAILURE] in %S line %d\n"),__FILE__,__LINE__);
	MSG_ASSERTION(__LINE__,__FILE__); 
	exit(0);
  }
  return LKIBLSCALE[x];
}
#endif

#if TRACETHREAD

int _THREADID_WINMAIN=0;
int _THREADID_CALC=0;
int _THREADID_DRAW=0;
int _THREADID_PORT1=0;
int _THREADID_PORT2=0;
int _THREADID_UNKNOWNPORT=0;

void TraceThread(const TCHAR *mes) {

  int id=GetCurrentThreadId();
  TCHAR tname[20];
  tname[0]='\0';

  if (id==_THREADID_WINMAIN)	_tcscpy(tname,_T("WINMAIN"));
  if (id==_THREADID_DRAW)	_tcscpy(tname,_T("DRAW"));
  if (id==_THREADID_CALC)	_tcscpy(tname,_T("CALC"));
  if (id==_THREADID_PORT1)	_tcscpy(tname,_T("PORT1"));
  if (id==_THREADID_PORT2)	_tcscpy(tname,_T("PORT2"));
  if (id==_THREADID_UNKNOWNPORT)	_tcscpy(tname,_T("UNKNOWN PORT"));

  if (_tcslen(tname)>0)
	StartupStore(_T("##############  [%s] in thread: %s\n"),mes,tname);
  else
	StartupStore(_T("##############  [%s] in unkown thread, id=%d\n"),mes,id);

}

#endif
