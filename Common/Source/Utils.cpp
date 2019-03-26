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
#include "Dialogs.h"
#include "TraceThread.h"

long GetUTCOffset(void) {
    return UTCOffset;
}

void TriggerGPSUpdate() {
    dataTriggerEvent.set();
}

// This is currently doing nothing.

void TriggerVarioUpdate() {
}

//
// When Debounce(int) was introduced, the old Debounce was incorrect, always returning true;
// Possible undebounced triggers could be issued> to check in WindowControls and many parts.
// No complaints so far, but this should be fixed. Otherwise the debounceTimeout was UNUSED
// and the simple Debounce(void) call was always true!!
static PeriodClock fpsTimeLast;

bool Debounce(void) {
    return fpsTimeLast.CheckUpdate(debounceTimeout);
}

bool Debounce(int dtime) {
    return fpsTimeLast.CheckUpdate(dtime);
}

// Get the infobox type from configuration, selecting position i
// From 1-8 auxiliaries
//     0-16 dynamic page
//

int GetInfoboxType(int i) {

    int retval = 0;
    if (i < 1 || i > 16) return LK_ERROR;

    // it really starts from 0
    if (i <= 8)
        retval = (InfoType[i - 1] >> 24) & 0xff; // auxiliary
    else {
        switch (MapWindow::mode.Fly()) {
            case MapWindow::Mode::MODE_FLY_CRUISE:
                retval = (InfoType[i - 9] >> 8) & 0xff;
                break;
            case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
                retval = (InfoType[i - 9] >> 16) & 0xff;
                break;
            case MapWindow::Mode::MODE_FLY_CIRCLING:
                retval = (InfoType[i - 9]) & 0xff;
                break;
            default:
                // impossible case, show twice auxiliaries
                retval = (InfoType[i - 9] >> 24) & 0xff;
                break;
        }
    }

    return min(NumDataOptions - 1, retval);
}

// Returns the LKProcess index value for configured infobox (0-8) for dmCruise, dmFinalGlide, Auxiliary, dmCircling
// The function name is really stupid...
// dmMode is an enum, we simply use for commodity

int GetInfoboxIndex(int i, MapWindow::Mode::TModeFly dmMode) {
    int retval = 0;
    if (i < 0 || i > 8) return LK_ERROR;

    switch (dmMode) {
        case MapWindow::Mode::MODE_FLY_CRUISE:
            retval = (InfoType[i - 1] >> 8) & 0xff;
            break;
        case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
            retval = (InfoType[i - 1] >> 16) & 0xff;
            break;
        case MapWindow::Mode::MODE_FLY_CIRCLING:
            retval = (InfoType[i - 1]) & 0xff;
            break;
        default:
            // default is auxiliary
            retval = (InfoType[i - 1] >> 24) & 0xff;
            break;
    }
    return min(NumDataOptions - 1, retval);
}

// Used for calculation, but does not affect IsSafetyMacCreadyInUse so careful

double GetMacCready(int wpindex, short wpmode) {

#if BUGSTOP
    LKASSERT(ValidWayPoint(wpindex));
#endif
    if (!ValidWayPoint(wpindex)) return 0;

    if (WayPointCalc[wpindex].IsLandable) {
        if (MACCREADY > GlidePolar::SafetyMacCready)
            return MACCREADY;
        else
            return GlidePolar::SafetyMacCready;
    }
    return MACCREADY;

}

//
// This is not used on startup by profiles
//
void CheckSetMACCREADY(const double value) {
    if (value<0)
        MACCREADY=0;
    else
        if (value>12)
            MACCREADY=12;
        else
            MACCREADY=value;
}

void SetOverColorRef() {
    switch (OverColor) {
        case OcWhite:
            OverColorRef = RGB_WHITE;
            break;
        case OcBlack:
            OverColorRef = RGB_SBLACK;
            break;
        case OcBlue:
            OverColorRef = RGB_DARKBLUE;
            break;
        case OcGreen:
            OverColorRef = RGB_GREEN;
            break;
        case OcYellow:
            OverColorRef = RGB_YELLOW;
            break;
        case OcCyan:
            OverColorRef = RGB_CYAN;
            break;
        case OcOrange:
            OverColorRef = RGB_ORANGE;
            break;
        case OcGrey:
            OverColorRef = RGB_GREY;
            break;
        case OcDarkGrey:
            OverColorRef = RGB_DARKGREY;
            break;
        case OcDarkWhite:
            OverColorRef = RGB_DARKWHITE;
            break;
        case OcAmber:
            OverColorRef = RGB_AMBER;
            break;
        case OcLightGreen:
            OverColorRef = RGB_LIGHTGREEN;
            break;
        case OcPetrol:
            OverColorRef = RGB_PETROL;
            break;
        default:
            OverColorRef = RGB_MAGENTA;
            break;
    }
}

bool CheckClubVersion() {
    TCHAR srcfile[MAX_PATH];
    LocalPath(srcfile, _T("CLUB"));
    return lk::filesystem::exist(srcfile);
}

void ClubForbiddenMsg() {
    MessageBoxX(
            // LKTOKEN  _@M503_ = "Operation forbidden on CLUB devices"
            MsgToken(503),
            _T("CLUB DEVICE"),
            mbOk);
    return;
}


// Are we using lockmode? What is the current status?

bool LockMode(const short lmode) {

    switch (lmode) {
        case 0: // query availability of LockMode
            return true;
            break;

        case 1: // query lock/unlock status
            return LockModeStatus;
            break;

        case 2: // invert lock status
            LockModeStatus = !LockModeStatus;
            return LockModeStatus;
            break;

        case 3: // query button is usable or not
            if (ISPARAGLIDER)
                // Positive if not flying
                return (!CALCULATED_INFO.Flying);
            else return true;
            break;

        case 9: // Check if we can unlock the screen
            if (ISPARAGLIDER) {
                // Automatic unlock
                if (CALCULATED_INFO.Flying) {
                    if ((GPS_INFO.Time - CALCULATED_INFO.TakeOffTime) > 10) {
                        LockModeStatus = false;
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
// Rotate flags: ALL ON, TASK, FAI, ALLOFF
//

void ToggleDrawTaskFAI(void) {

    // AllOn -> Task
    if (Flags_DrawTask && Flags_DrawFAI) {
        Flags_DrawFAI = false;
        return;
    }
    // Task -> FAI
    if (Flags_DrawTask&&!Flags_DrawFAI) {
        Flags_DrawTask = false;
        Flags_DrawFAI = true;
        return;
    }
    // FAI --> ALLOFF
    if (!Flags_DrawTask && Flags_DrawFAI) {
        Flags_DrawTask = false;
        Flags_DrawFAI = false;
        return;
    }
    // ALLOFF -> ALLON
    Flags_DrawTask = true;
    Flags_DrawFAI = true;

}

#if TRACETHREAD

int _THREADID_WINMAIN = 0;
int _THREADID_CALC = 0;
int _THREADID_DRAW = 0;
int _THREADID_PORT1 = 0;
int _THREADID_PORT2 = 0;
int _THREADID_UNKNOWNPORT = 0;

void TraceThread(const TCHAR *mes) {

    int id = GetCurrentThreadId();
    TCHAR tname[20];
    tname[0] = '\0';

    if (id == _THREADID_WINMAIN) _tcscpy(tname, _T("WINMAIN"));
    if (id == _THREADID_DRAW) _tcscpy(tname, _T("DRAW"));
    if (id == _THREADID_CALC) _tcscpy(tname, _T("CALC"));
    if (id == _THREADID_PORT1) _tcscpy(tname, _T("PORT1"));
    if (id == _THREADID_PORT2) _tcscpy(tname, _T("PORT2"));
    if (id == _THREADID_UNKNOWNPORT) _tcscpy(tname, _T("UNKNOWN PORT"));

    if (_tcslen(tname) > 0)
        StartupStore(_T("##############  [%s] in thread: %s\n"), mes, tname);
    else
        StartupStore(_T("##############  [%s] in unkown thread, id=%d\n"), mes, id);

}

#endif


double CalculateLXBalastFactor(double Ballast)
{
	double CurrentWeight = WEIGHTS[0] +WEIGHTS[1] + (WEIGHTS[2]*Ballast) +  GlidePolar::WeightOffset;
	double WithoutBallastWeight =  WEIGHTS[0] +WEIGHTS[1] +  GlidePolar::WeightOffset;

	if(WithoutBallastWeight == 0)
		WithoutBallastWeight = 1;

	return   CurrentWeight/WithoutBallastWeight;


}
double CalculateBalastFromLX(double Factor)
{
	double TotalAvailableBallast  = WEIGHTS[2];
	if(TotalAvailableBallast == 0)
		TotalAvailableBallast = 1;

	return ((Factor-1) * (WEIGHTS[0] +WEIGHTS[1] +  GlidePolar::WeightOffset))/ TotalAvailableBallast;
}

double CalculateLXBugs(double Bugs)
{
  return (1.0-Bugs )*100;
}

double CalculateBugsFromLX(double LXBug)
{
  return (100.0 - LXBug)/100.0;
}



// All values in the range 100% (1) to 0% (0);

double CheckSetBallast(double val) {
    if ((val >= 0) && (val <= 1)) {
        BALLAST = val;
    }
    else
    {
#if TESTBENCH
      static short counter = 0;
      if (counter < 10) {
	  StartupStore(_T(". CHECKSETBALLAST ERROR, input=%f output=%f\n"), val, BALLAST);
	  counter++;
      }
#endif
    }

    if (val < 0) BALLAST = 0.0;
    else if (val > 1) BALLAST = 1.0;


    GlidePolar::SetBallast();
    return BALLAST;
}

// BUGS is really EFFICIENCY. In the range 100% to 50%, i.e. 1 to 0.5 .
// It cannot be 0.
#define MINBUGS 0.5

double CheckSetBugs(double val) {
    if ((val >= MINBUGS) && (val <= 1)) {
        BUGS = val;
        return val;
    }

    if (val < MINBUGS) BUGS = MINBUGS;
    else if (val > 1) BUGS = 1.0;

#if TESTBENCH
    static short counter = 0;
    if (counter < 10) {
        StartupStore(_T(". CHECKSETBUGS ERROR, input=%f output=%f\n"), val, BUGS);
        counter++;
    }
#endif
    return BUGS;
}

// is thermal bar visible or not ?

bool IsThermalBarVisible(void) {

    switch (ThermalBar) {

        case 0: // Disabled
            break;
        case 1: // in thermal
            return MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING);
            break;
        case 2: // in thermal and cruise
            return ( MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) ||
                    MapWindow::mode.Is(MapWindow::Mode::MODE_CRUISE));
            break;
        case 3: // Always
            return true;
            break;
        default:
            break;

    }
    return false;

}
