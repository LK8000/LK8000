/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: lk8000.cpp,v 1.1 2010/12/15 11:30:56 root Exp root $
*/
#include "StdAfx.h"
#include "wcecompat/ts_string.h"
#include "options.h"
#include "Defines.h"
#include "externs.h"
#include "compatibility.h"
#include "lk8000.h"
#include "buildnumber.h"
#include "MapWindow.h"
#include "Parser.h"
#include "Calculations.h"
#include "Calculations2.h"
#include "Task.h"
#include "Dialogs.h"

#include "Process.h"

#include "Modeltype.h"

#include "Utils.h"
#include "Utils2.h"
#include "Port.h"
#include "Waypointparser.h"
#include "Airspace.h"
#include "Logger.h"
#include "McReady.h"
#include "AirfieldDetails.h"
#include "InfoBoxLayout.h"

#include <commctrl.h>
#include <aygshell.h>
#if (WINDOWSPC<1)
#include <sipapi.h>
#endif

#include "Terrain.h"
#include "device.h"

#include "devCAI302.h"
#include "devCaiGpsNav.h"
#include "devEW.h"
#include "devGeneric.h"
#include "devDisabled.h"
#include "devNmeaOut.h"
#include "devPosiGraph.h"
#include "devBorgeltB50.h"
#include "devVolkslogger.h"
#include "devEWMicroRecorder.h"
#include "devLX.h"
#include "devLXNano.h"
#include "devZander.h"
#include "devFlymasterF1.h"
#include "devCompeo.h"
#include "devFlytec.h"
#include "devLK8EX1.h"
#include "devDigifly.h"
#include "devXCOM760.h"
#include "devCondor.h"
#include "devIlec.h"
#include "devDSX.h"
#include "devIMI.h"
#include "devWesterboer.h"

#include "Units.h"
#include "InputEvents.h"
#include "Message.h"
#include "Atmosphere.h"
#include "Geoid.h"
#ifdef PNA
#include "LKHolux.h"
#endif

#include "RasterTerrain.h"
extern void LKObjects_Create();
extern void LKObjects_Delete();
#include "LKMainObjects.h"

using std::min;
using std::max;

#ifdef DEBUG_TRANSLATIONS
#include <map>
static std::map<TCHAR*, TCHAR*> unusedTranslations;
#endif

#include "utils/heapcheck.h"


HBRUSH hBrushSelected;
HBRUSH hBrushUnselected;
HBRUSH hBrushButton;
COLORREF ColorSelected = RGB(0xC0,0xC0,0xC0);
COLORREF ColorUnselected = RGB_WHITE;
COLORREF ColorWarning = RGB_RED;
COLORREF ColorOK = RGB_BLUE;
COLORREF ColorButton = RGB_BUTTONS;  


//Local Static data
static int iTimerID= 0;

static bool MenuActive = false;

#if (((UNDER_CE >= 300)||(_WIN32_WCE >= 0x0300)) && (WINDOWSPC<1))
#define HAVE_ACTIVATE_INFO
static SHACTIVATEINFO s_sai;
static bool api_has_SHHandleWMActivate = false;
static bool api_has_SHHandleWMSettingChange = false;
#endif


void PopupBugsBallast(int updown);

// System boot specific flags 
// Give me a go/no-go 
bool goInstallSystem=false;
bool goCalculationThread=false;
#ifndef NOINSTHREAD
bool goInstrumentThread=false;
#endif

// Developers dedicates..
// Use rot13 under linux to code and decode strings
char dedicated_by_paolo[]="Qrqvpngrq gb zl sngure Ivggbevb";
// char dedicated_by_{yourname}="....";


CRITICAL_SECTION  CritSec_FlightData;
bool csFlightDataInitialized = false;
CRITICAL_SECTION  CritSec_EventQueue;
bool csEventQueueInitialized = false;
CRITICAL_SECTION  CritSec_TerrainDataGraphics;
bool csTerrainDataGraphicsInitialized = false;
CRITICAL_SECTION  CritSec_TerrainDataCalculations;
bool csTerrainDataCalculationsInitialized = false;
CRITICAL_SECTION  CritSec_NavBox;
bool csNavBoxInitialized = false;
CRITICAL_SECTION  CritSec_Comm;
bool csCommInitialized = false;
CRITICAL_SECTION  CritSec_TaskData;
bool csTaskDataInitialized = false;


static BOOL GpsUpdated;
static HANDLE dataTriggerEvent;
static BOOL VarioUpdated;
static HANDLE varioTriggerEvent;

// Forward declarations of functions included in this code module:
ATOM                                                    MyRegisterClass (HINSTANCE, LPTSTR);
BOOL                                                    InitInstance    (HINSTANCE, int);
LRESULT CALLBACK        WndProc                 (HWND, UINT, WPARAM, LPARAM);
LRESULT                                         MainMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void                                                    AssignValues(void);
void                                                    DisplayText(void);

void CommonProcessTimer    (void);
void SIMProcessTimer(void);
void ProcessTimer    (void);

#ifdef DEBUG
void                                            DebugStore(char *Str);
#endif

void TriggerGPSUpdate()
{
  GpsUpdated = true;
  SetEvent(dataTriggerEvent);
}

void TriggerVarioUpdate()
{
  VarioUpdated = true;
  PulseEvent(varioTriggerEvent);
}

void HideMenu() {
    MenuTimeOut = MenuTimeoutMax;
    DisplayTimeOut = 0;
}

void ShowMenu() {
  InputEvents::setMode(TEXT("Menu"));
  MenuTimeOut = 0;
  DisplayTimeOut = 0;
}


void SettingsEnter() {
  MenuActive = true;

  MapWindow::SuspendDrawingThread();
  // This prevents the map and calculation threads from doing anything
  // with shared data while it is being changed.

  MAPFILECHANGED = FALSE;
  AIRSPACEFILECHANGED = FALSE;
  AIRFIELDFILECHANGED = FALSE;
  WAYPOINTFILECHANGED = FALSE;
  TERRAINFILECHANGED = FALSE;
  TOPOLOGYFILECHANGED = FALSE;
  POLARFILECHANGED = FALSE;
  LANGUAGEFILECHANGED = FALSE;
  STATUSFILECHANGED = FALSE;
  INPUTFILECHANGED = FALSE;
  COMPORTCHANGED = FALSE;
}


void SettingsLeave() {
  if (!GlobalRunning) return; 

  SwitchToMapWindow();

  // Locking everything here prevents the calculation thread from running,
  // while shared data is potentially reloaded.
 
  LockFlightData();
  LockTaskData();
  LockNavBox();

  MenuActive = false;

  // 101020 LKmaps contain only topology , so no need to force total reload!
  if(MAPFILECHANGED) {
	if (LKTopo==0) {
		AIRSPACEFILECHANGED = TRUE;
		AIRFIELDFILECHANGED = TRUE;
		WAYPOINTFILECHANGED = TRUE;
		TERRAINFILECHANGED  = TRUE;
	}
	TOPOLOGYFILECHANGED = TRUE;
  } 

  if (TERRAINFILECHANGED) {
	RasterTerrain::CloseTerrain();
	RasterTerrain::OpenTerrain();
	SetHome(WAYPOINTFILECHANGED==TRUE);
	RasterTerrain::ServiceFullReload(GPS_INFO.Latitude, GPS_INFO.Longitude);
	MapWindow::ForceVisibilityScan = true;
  }

  if((WAYPOINTFILECHANGED) || (AIRFIELDFILECHANGED)) {
	SaveDefaultTask(); //@ 101020 BUGFIX
	ClearTask();
	ReadWayPoints();
	StartupStore(_T(". Total %d waypoints%s"),NumberOfWayPoints,NEWLINE);
	InitWayPointCalc();
	ReadAirfieldFile();
	SetHome(true); // force home reload

	if (WAYPOINTFILECHANGED) {
		SaveRecentList();
		LoadRecentList();
		RangeLandableNumber=0;
		RangeAirportNumber=0;
		RangeTurnpointNumber=0;
		CommonNumber=0;
		SortedNumber=0;
		// SortedTurnpointNumber=0; 101222
		LastDoRangeWaypointListTime=0;
		LKForceDoCommon=true;
		LKForceDoNearest=true;
		LKForceDoRecent=true;
		// LKForceDoNearestTurnpoint=true; 101222
	}
	InputEvents::eventTaskLoad(_T(LKF_DEFAULTASK)); //@ BUGFIX 101020
  } 

  if (TOPOLOGYFILECHANGED) {
	CloseTopology();
	OpenTopology();
	MapWindow::ForceVisibilityScan = true;
  }
  
  if(AIRSPACEFILECHANGED) {
	CAirspaceManager::Instance().CloseAirspaces();
	CAirspaceManager::Instance().ReadAirspaces();
	CAirspaceManager::Instance().SortAirspaces();
	MapWindow::ForceVisibilityScan = true;
  }  
  
  if (POLARFILECHANGED) {
	CalculateNewPolarCoef();
	GlidePolar::SetBallast();
  }
  
  if (AIRFIELDFILECHANGED
      || AIRSPACEFILECHANGED
      || WAYPOINTFILECHANGED
      || TERRAINFILECHANGED
      || TOPOLOGYFILECHANGED
      ) {
	CloseProgressDialog();
	SetFocus(hWndMapWindow);
  }
  
  UnlockNavBox();
  UnlockTaskData();
  UnlockFlightData();

  if(!SIMMODE && COMPORTCHANGED) {
      LKForceComPortReset=true;
      // RestartCommPorts(); 110605
  }

  MapWindow::ResumeDrawingThread();
  // allow map and calculations threads to continue on their merry way
}


void SystemConfiguration(void) {
  if (!SIMMODE) {
  	if (LockSettingsInFlight && CALCULATED_INFO.Flying) {
		DoStatusMessage(TEXT("Settings locked in flight"));
		return;
	}
  }

  SettingsEnter();
  dlgConfigurationShowModal(); 
  SettingsLeave();
}



void FullScreen() {
  if (!MenuActive) {
    SetForegroundWindow(hWndMainWindow);
#if (WINDOWSPC>0)
    SetWindowPos(hWndMainWindow,HWND_TOP,
                 0, 0, 0, 0,
                 SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);
#else
#ifndef CECORE
    SHFullScreen(hWndMainWindow, SHFS_HIDETASKBAR|SHFS_HIDESIPBUTTON|SHFS_HIDESTARTICON);
#endif
    SetWindowPos(hWndMainWindow,HWND_TOP,
                 0,0,
                 GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN),
                 SWP_SHOWWINDOW);
#endif
  }
  MapWindow::RequestFastRefresh();
}


void LockComm() {
#ifdef HAVEEXCEPTIONS
  if (!csCommInitialized) throw TEXT("LockComm Error");
#endif
  EnterCriticalSection(&CritSec_Comm);
}

void UnlockComm() {
#ifdef HAVEEXCEPTIONS
  if (!csCommInitialized) throw TEXT("LockComm Error");
#endif
  LeaveCriticalSection(&CritSec_Comm);
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



void TriggerRedraws(NMEA_INFO *nmea_info,
		    DERIVED_INFO *derived_info) {
	(void)nmea_info;
	(void)derived_info;
  if (MapWindow::IsDisplayRunning()) {
    if (GpsUpdated) {
      MapWindow::MapDirty = true;
      PulseEvent(drawTriggerEvent); 
      // only ask for redraw if the thread was waiting,
      // this causes the map thread to try to synchronise
      // with the calculation thread, which is desirable
      // to reduce latency
      // it also ensures that if the display is lagging,
      // it will have a chance to catch up.
    }
  }
}

#ifndef NOINSTHREAD
// this thread currently does nothing. Soon used for background parallel calculations or new gauges
DWORD InstrumentThread (LPVOID lpvoid) {
	(void)lpvoid;
  #ifdef CPUSTATS
  FILETIME CreationTime, ExitTime, StartKernelTime, EndKernelTime, StartUserTime, EndUserTime ;
  #endif

  // watch out for a deadlock here. This has to be done before waiting for DisplayRunning..
  goInstrumentThread=true; // 091119

  // wait for proper startup signal
  while (!MapWindow::IsDisplayRunning()) {
	Sleep(100);
  }

  while (!MapWindow::CLOSETHREAD) {

	#ifdef CPUSTATS
	GetThreadTimes( hInstrumentThread, &CreationTime, &ExitTime,&StartKernelTime,&StartUserTime);
	#endif
	WaitForSingleObject(varioTriggerEvent, 5000);
	ResetEvent(varioTriggerEvent);
	if (MapWindow::CLOSETHREAD) break; // drop out on exit

	// DO NOTHING BY NOW
	// if triggervario, render vario update eventually here
	Sleep(10000);
	#ifdef CPUSTATS
	if ( (GetThreadTimes( hInstrumentThread, &CreationTime, &ExitTime,&EndKernelTime,&EndUserTime)) == 0) {
		Cpu_Instrument=9999;
	} else {
		Cpustats(&Cpu_Instrument,&StartKernelTime, &EndKernelTime, &StartUserTime, &EndUserTime);
	}
	#endif
  }
  return 0;
}
#endif

DWORD CalculationThread (LPVOID lpvoid) {
	(void)lpvoid;
  bool needcalculationsslow;

  NMEA_INFO     tmp_GPS_INFO;
  DERIVED_INFO  tmp_CALCULATED_INFO;
#ifdef CPUSTATS
  FILETIME CreationTime, ExitTime, StartKernelTime, EndKernelTime, StartUserTime, EndUserTime ;
#endif
  needcalculationsslow = false;

  // let's not create a deadlock here, setting the go after another race condition
  goCalculationThread=true; // 091119 CHECK
  // wait for proper startup signal
  while (!MapWindow::IsDisplayRunning()) {
    Sleep(100);
  }

  // while (!goCalculating) Sleep(100);
  Sleep(1000); // 091213  BUGFIX need to syncronize !!! TOFIX02 TODO

  while (!MapWindow::CLOSETHREAD) {

    WaitForSingleObject(dataTriggerEvent, 5000);
    ResetEvent(dataTriggerEvent);
    if (MapWindow::CLOSETHREAD) break; // drop out on exit

#ifdef CPUSTATS
    GetThreadTimes( hCalculationThread, &CreationTime, &ExitTime,&StartKernelTime,&StartUserTime);
#endif
    // set timer to determine latency (including calculations)
    // the UpdateTimeStats was unused and commented, so no reason to keep the if
    // if (GpsUpdated) { 
      //      MapWindow::UpdateTimeStats(true);
    // }
    // make local copy before editing...
    LockFlightData();
    if (GpsUpdated) { // timeout on FLARM objects
      FLARM_RefreshSlots(&GPS_INFO);
    }
    memcpy(&tmp_GPS_INFO,&GPS_INFO,sizeof(NMEA_INFO));
    memcpy(&tmp_CALCULATED_INFO,&CALCULATED_INFO,sizeof(DERIVED_INFO));

    UnlockFlightData();

    // Do vario first to reduce audio latency
    if (GPS_INFO.VarioAvailable) {
      if (DoCalculationsVario(&tmp_GPS_INFO,&tmp_CALCULATED_INFO)) {
	        
      }
      // assume new vario data has arrived, so infoboxes
      // need to be redrawn
      //} 20060511/sgi commented out 
    } else {
      // run the function anyway, because this gives audio functions
      // if no vario connected
      if (GpsUpdated) {
	if (DoCalculationsVario(&tmp_GPS_INFO,&tmp_CALCULATED_INFO)) {
	}
	TriggerVarioUpdate(); // emulate vario update
      }
    }
    
    if (GpsUpdated) {
      if(DoCalculations(&tmp_GPS_INFO,&tmp_CALCULATED_INFO)){
        MapWindow::MapDirty = true;
        needcalculationsslow = true;

        if (tmp_CALCULATED_INFO.Circling)
          MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_CIRCLING);
        else if (tmp_CALCULATED_INFO.FinalGlide)
          MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
        else
          MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_CRUISE);
      }
    }
        
    if (MapWindow::CLOSETHREAD) break; // drop out on exit

    TriggerRedraws(&tmp_GPS_INFO, &tmp_CALCULATED_INFO);

    if (MapWindow::CLOSETHREAD) break; // drop out on exit

    if (SIMMODE) {
	if (needcalculationsslow || ( ReplayLogger::IsEnabled() ) ) { 
		DoCalculationsSlow(&tmp_GPS_INFO,&tmp_CALCULATED_INFO);
		needcalculationsslow = false;
	}
    } else {
	if (needcalculationsslow) {
		DoCalculationsSlow(&tmp_GPS_INFO,&tmp_CALCULATED_INFO);
		needcalculationsslow = false;
	}
    }

    if (MapWindow::CLOSETHREAD) break; // drop out on exit

    // values changed, so copy them back now: ONLY CALCULATED INFO
    // should be changed in DoCalculations, so we only need to write
    // that one back (otherwise we may write over new data)
    LockFlightData();
    memcpy(&CALCULATED_INFO,&tmp_CALCULATED_INFO,sizeof(DERIVED_INFO));
    UnlockFlightData();

    GpsUpdated = false;

#ifdef CPUSTATS
    if ( (GetThreadTimes( hCalculationThread, &CreationTime, &ExitTime,&EndKernelTime,&EndUserTime)) == 0) {
               Cpu_Calc=9999;
    } else {
               Cpustats(&Cpu_Calc,&StartKernelTime, &EndKernelTime, &StartUserTime, &EndUserTime);
    }
#endif
  }
  return 0;
}

// Since the calling function want to be sure that threads are created, they now flag a go status
// and we save 500ms at startup. 
// At the end of thread creation, we expect goCalc and goInst flags are true
void CreateCalculationThread() {
  #ifndef CPUSTATS
  // Need to keep them global to make them accessible from GetThreadTimes if in use
  HANDLE hCalculationThread;
  DWORD dwCalcThreadID;
  #endif

  // Create a read thread for performing calculations
  if ((hCalculationThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE )CalculationThread, 0, 0, &dwCalcThreadID)) != NULL)
  {
	SetThreadPriority(hCalculationThread, THREAD_PRIORITY_NORMAL); 
	#ifndef CPUSTATS
	// Do not close if we need to use the handle 
	CloseHandle (hCalculationThread); 
	#endif
  } else {
	ASSERT(1);
  }

#ifndef NOINSTHREAD

  #ifndef CPUSTATS
  HANDLE hInstrumentThread;
  DWORD dwInstThreadID;
  #endif

  if ((hInstrumentThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE )InstrumentThread, 0, 0, &dwInstThreadID)) != NULL)
  {
	SetThreadPriority(hInstrumentThread, THREAD_PRIORITY_NORMAL); 
	#ifndef CPUSTATS
	CloseHandle (hInstrumentThread);
	#endif
  } else {
	ASSERT(1);
  }
#endif

}

extern bool SetDataOption( int index, UnitGroup_t UnitGroup, TCHAR *Description, TCHAR *Title);
extern void FillDataOptions(void);


void PreloadInitialisation(bool ask) {
  SetToRegistry(TEXT("LKV"), 3);
  LKLanguageReady=false;
  LKReadLanguageFile();
  FillDataOptions(); // Load infobox list

  if (ask) {
    // Load default profile and status file: we are at an early stage
    RestoreRegistry();
    ReadRegistrySettings();
    StatusFileInit();
  } else {
    FullScreen();
    while (dlgStartupShowModal());

    RestoreRegistry();
    ReadRegistrySettings();

    // Force reload of bitmaps in the Draw thread 
    LKSW_ReloadProfileBitmaps=true;

    // LKTOKEN _@M1206_ "Initialising..."
	CreateProgressDialog(gettext(TEXT("_@M1206_"))); 
  }

  // Interface (before interface)
  if (!ask) {
    LKReadLanguageFile();
    ReadStatusFile();
    InputEvents::readFile();
  }

}

void AfterStartup() {

  StartupStore(TEXT(". CloseProgressDialog%s"),NEWLINE);
  CloseProgressDialog();

  // NOTE: Must show errors AFTER all windows ready
  int olddelay = StatusMessageData[0].delay_ms;
  StatusMessageData[0].delay_ms = 20000; // 20 seconds

  if (SIMMODE) {
	StartupStore(TEXT(". GCE_STARTUP_SIMULATOR%s"),NEWLINE);
	InputEvents::processGlideComputer(GCE_STARTUP_SIMULATOR);
  } else {
	StartupStore(TEXT(". GCE_STARTUP_REAL%s"),NEWLINE);
	InputEvents::processGlideComputer(GCE_STARTUP_REAL);
  }
  StatusMessageData[0].delay_ms = olddelay; 

  // Create default task if none exists
  StartupStore(TEXT(". Create default task%s"),NEWLINE);
  DefaultTask();

  // Trigger first redraw
  GpsUpdated = true;
  MapWindow::MapDirty = true;
  MapWindow::zoom.Reset(); 
  FullScreen();
  SetEvent(drawTriggerEvent);
}



void StartupLogFreeRamAndStorage() {
  unsigned long freeram = CheckFreeRam()/1024;
  TCHAR buffer[MAX_PATH];
  LocalPath(buffer);
  unsigned long freestorage = FindFreeSpace(buffer);
  StartupStore(TEXT(". Free ram=%ld K  storage=%ld K%s"), freeram,freestorage,NEWLINE);
}


int WINAPI WinMain(     HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPTSTR    lpCmdLine,
                        int       nCmdShow)
{
  MSG msg;
  HACCEL hAccelTable;
  INITCOMMONCONTROLSEX icc;
  (void)hPrevInstance;

  // use mutex to avoid multiple instances of lk8000 be running
  CreateMutex(NULL,FALSE,_T("LOCK8000"));
  if (GetLastError() == ERROR_ALREADY_EXISTS) return(0);

  
  wsprintf(LK8000_Version,_T("%S v%S.%S "), LKFORK, LKVERSION,LKRELEASE);
  wcscat(LK8000_Version, TEXT(__DATE__));
  StartupStore(_T("%s------------------------------------------------------------%s"),NEWLINE,NEWLINE);
  #ifdef PNA
  StartupStore(TEXT(". Starting %s %s build#%d%s"), LK8000_Version,_T("PNA"),BUILDNUMBER,NEWLINE);
  #else
  #if (WINDOWSPC>0)
  StartupStore(TEXT(". Starting %s %s build#%d%s"), LK8000_Version,_T("PC"),BUILDNUMBER,NEWLINE);
  #else
  StartupStore(TEXT(". Starting %s %s build#%d%s"), LK8000_Version,_T("PDA"),BUILDNUMBER,NEWLINE);
  #endif
  #endif

  #if TESTBENCH
  StartupStore(TEXT(". TESTBENCH option enabled%s"),NEWLINE);
  #endif
  Globals_Init();

  StartupLogFreeRamAndStorage();

  // PRELOAD ANYTHING HERE
  LKRunStartEnd(true);
  // END OF PRELOAD, PROGRAM GO!

  #ifdef PNA 
  //  LocalPath is called for the very first time by CreateDirectoryIfAbsent.
  //  In order to be able in the future to behave differently for each PNA device
  //  and maybe also for common PDAs, we need to know the PNA/PDA Model Type 
  //  BEFORE calling LocalPath. This was critical.
  //  First we check the exec filename, which has priority over registry values.
  SmartGlobalModelType();

  // Huston we have a problem
  // At this point we still havent loaded profile. Loading profile will also reload registry.
  // If registry was deleted in PNA, model type is not configured. It is configured in profile, but
  // it is too early here. So no ModelType .
  //
  // if we found no embedded name, try from registry
  if (  !wcscmp(GlobalModelName, _T("UNKNOWN")) ) {
	if (  !SetModelType() ) {
		// last chance: try from default profile
		LoadModelFromProfile();
	}
  }
  #endif
  

  // registry deleted at startup also for PC
  if ( RegDeleteKey(HKEY_CURRENT_USER, _T(REGKEYNAME))== ERROR_SUCCESS )  // 091213
        {
	#if TESTBENCH
	StartupStore(_T(". Registry key was correctly deleted%s"),NEWLINE);
	#endif
	}
  else
	StartupStore(_T(". Registry key could NOT be deleted, this is normal after a reset.%s"),NEWLINE);


  bool datadir;
  datadir=CheckDataDir();
  if (!datadir) {
	// we cannot call startupstore, no place to store log!
	WarningHomeDir=true;
  }

  #if ALPHADEBUG
  extern TCHAR *gmfcurrentpath();
  StartupStore(_T(". Program execution path is <%s>\n"),gmfcurrentpath());
  TCHAR lpath[MAX_PATH];
  LocalPath(lpath);
  StartupStore(_T(". Program data directory is <%s>\n"),lpath);
  #endif

  #if ( !defined(WINDOWSPC) || WINDOWSPC==0 )
  #if ALPHADEBUG
  StartupStore(TEXT(". Install/copy system objects in device memory%s"),NEWLINE);
  #endif
  short didsystem;
  didsystem=InstallSystem(); 
  goInstallSystem=true;
  #if ALPHADEBUG
  StartupStore(_T(". InstallSystem ended, code=%d%s"),didsystem,NEWLINE);
  #endif
  #endif

  #ifdef HAVE_ACTIVATE_INFO
  FARPROC ptr;
  ptr = GetProcAddress(GetModuleHandle(TEXT("AYGSHELL")), TEXT("SHHandleWMActivate"));
  if (ptr != NULL) api_has_SHHandleWMActivate = true;
  ptr = GetProcAddress(GetModuleHandle(TEXT("AYGSHELL")), TEXT("SHHandleWMSettingChange"));
  if (ptr != NULL) api_has_SHHandleWMSettingChange = true;
  #endif
    
  // These directories are needed if missing, as LK can run also with no maps and no waypoints..
  CreateDirectoryIfAbsent(TEXT(LKD_LOGS));
  CreateDirectoryIfAbsent(TEXT(LKD_CONF));
  CreateDirectoryIfAbsent(TEXT(LKD_TASKS));
  CreateDirectoryIfAbsent(TEXT(LKD_MAPS));
  CreateDirectoryIfAbsent(TEXT(LKD_WAYPOINTS));

  LK8000GetOpts(lpCmdLine);

  icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icc.dwICC = ICC_UPDOWN_CLASS;
  InitCommonControls();
  InitSineTable();

  // Perform application initialization: also ScreenGeometry and LKIBLSCALE, and Objects
  if (!InitInstance (hInstance, nCmdShow))
    {
	StartupStore(_T("++++++ InitInstance failed, program terminated!%s"),NEWLINE);
	return FALSE;
    }

  hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_XCSOAR);

  #ifdef HAVE_ACTIVATE_INFO
  SHSetAppKeyWndAssoc(VK_APP1, hWndMainWindow);
  SHSetAppKeyWndAssoc(VK_APP2, hWndMainWindow);
  SHSetAppKeyWndAssoc(VK_APP3, hWndMainWindow);
  SHSetAppKeyWndAssoc(VK_APP4, hWndMainWindow);
  SHSetAppKeyWndAssoc(VK_APP5, hWndMainWindow);
  SHSetAppKeyWndAssoc(VK_APP6, hWndMainWindow);
  #endif

  InitializeCriticalSection(&CritSec_EventQueue);
  csEventQueueInitialized = true;
  InitializeCriticalSection(&CritSec_TaskData);
  csTaskDataInitialized = true;
  InitializeCriticalSection(&CritSec_FlightData);
  csFlightDataInitialized = true;
  InitializeCriticalSection(&CritSec_NavBox);
  csNavBoxInitialized = true;
  InitializeCriticalSection(&CritSec_Comm);
  csCommInitialized = true;
  InitializeCriticalSection(&CritSec_TerrainDataGraphics);
  csTerrainDataGraphicsInitialized = true;
  InitializeCriticalSection(&CritSec_TerrainDataCalculations);
  csTerrainDataCalculationsInitialized = true;

  drawTriggerEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("drawTriggerEvent"));
  dataTriggerEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("dataTriggerEvent"));
  varioTriggerEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("varioTriggerEvent"));

  // Initialise main blackboard data

  memset( &(Task), 0, sizeof(Task_t));
  memset( &(StartPoints), 0, sizeof(Start_t));
  ClearTask();
  memset( &(GPS_INFO), 0, sizeof(GPS_INFO));
  memset( &(CALCULATED_INFO), 0,sizeof(CALCULATED_INFO));
  memset( &SnailTrail[0],0,TRAILSIZE*sizeof(SNAIL_POINT));

  InitCalculations(&GPS_INFO,&CALCULATED_INFO);

  LinkGRecordDLL(); // try to link DLL if it exists

  OpenGeoid();

  // Load special libraries and init hardware. Anything custom should happen after here.
  // Note that this is subjected to GlobalModelType.
  // To initialise low level devices such as screen, we should do it much earlier.
  InitCustomHardware();

  PreloadInitialisation(false); // calls dlgStartup

  GPS_INFO.NAVWarning = true; // default, no gps at all!

  #if USESWITCHES
  GPS_INFO.SwitchState.AirbrakeLocked = false;
  GPS_INFO.SwitchState.FlapPositive = false;
  GPS_INFO.SwitchState.FlapNeutral = false;
  GPS_INFO.SwitchState.FlapNegative = false;
  GPS_INFO.SwitchState.GearExtended = false;
  GPS_INFO.SwitchState.Acknowledge = false;
  GPS_INFO.SwitchState.Repeat = false;
  GPS_INFO.SwitchState.SpeedCommand = false;
  GPS_INFO.SwitchState.UserSwitchUp = false;
  GPS_INFO.SwitchState.UserSwitchMiddle = false;
  GPS_INFO.SwitchState.UserSwitchDown = false;
  GPS_INFO.SwitchState.VarioCircling = false;
  #endif

  SYSTEMTIME pda_time;
  GetSystemTime(&pda_time);
  GPS_INFO.Time  = pda_time.wHour*3600+pda_time.wMinute*60+pda_time.wSecond;
  GPS_INFO.Year  = pda_time.wYear;
  GPS_INFO.Month = pda_time.wMonth;
  GPS_INFO.Day	 = pda_time.wDay;
  GPS_INFO.Hour  = pda_time.wHour;
  GPS_INFO.Minute = pda_time.wMinute;
  GPS_INFO.Second = pda_time.wSecond;

#ifdef DEBUG
  DebugStore("# Start\r\n");
#endif
  #ifndef NOWINDREGISTRY	// 100404
  LoadWindFromRegistry();
  #endif
  CalculateNewPolarCoef();
  #if TESTBENCH
  StartupStore(TEXT(". GlidePolar::SetBallast%s"),NEWLINE);
  #endif
  GlidePolar::SetBallast();

// VENTA-ADDON
#ifdef VENTA_DEBUG_KEY
  CreateProgressDialog(TEXT("DEBUG KEY MODE ACTIVE"));
  Sleep(1000);
#endif
#ifdef VENTA_DEBUG_EVENT
  CreateProgressDialog(TEXT("DEBUG EVENT MODE ACTIVE"));
  Sleep(1000);
#endif


if (ScreenSize==0) {
// LKTOKENS _@M1207_ "ERROR UNKNOWN RESOLUTION!"
CreateProgressDialog(gettext(TEXT("_@M1207_")));
 Sleep(3000);
}
#ifdef PNA // VENTA-ADDON 

	TCHAR sTmp[MAX_PATH];
	wsprintf(sTmp,TEXT("Conf=%s%S"), gmfpathname(),XCSDATADIR ); // VENTA2 FIX double backslash
	CreateProgressDialog(sTmp); 

	wsprintf(sTmp, TEXT("PNA MODEL=%s (%d)"), GlobalModelName, GlobalModelType);
	CreateProgressDialog(sTmp); Sleep(300);
#else
  TCHAR sTmpA[MAX_PATH], sTmpB[MAX_PATH];
  LocalPath(sTmpA,_T(""));
#if ( !defined(WINDOWSPC) || WINDOWSPC==0 )
  if ( !datadir ) {
	// LKTOKEN _@M1208_ "ERROR NO DIRECTORY:"
    CreateProgressDialog(gettext(TEXT("_@M1208_")));
    Sleep(3000);
  }
#endif
  wsprintf(sTmpB, TEXT("Conf=%s"),sTmpA);
  CreateProgressDialog(sTmpB); 
#if ( !defined(WINDOWSPC) || WINDOWSPC==0 )
  if ( !datadir ) {
    Sleep(3000);
    // LKTOKEN _@M1209_ "CHECK INSTALLATION!"
	CreateProgressDialog(gettext(TEXT("_@M1209_")));
    Sleep(3000);
  }
#endif
#endif // non PNA

// TODO until startup graphics are settled, no need to delay PC start
  if ( AircraftCategory == (AircraftCategory_t)umParaglider )
	// LKTOKEN _@M1210_ "PARAGLIDING MODE"
	CreateProgressDialog(gettext(TEXT("_@M1210_"))); 
	// LKTOKEN _@M1211_ "SIMULATION"
  if (SIMMODE) CreateProgressDialog(gettext(TEXT("_@M1211_"))); 

#ifdef PNA
  if ( SetBacklight() == true ) 
	// LKTOKEN _@M1212_ "AUTOMATIC BACKLIGHT CONTROL"
	CreateProgressDialog(gettext(TEXT("_@M1212_")));
  else
	// LKTOKEN _@M1213_ "NO BACKLIGHT CONTROL"
	CreateProgressDialog(gettext(TEXT("_@M1213_")));

  // this should work ok for all pdas as well
  if ( SetSoundVolume() == true ) 
	// LKTOKEN _@M1214_ "AUTOMATIC SOUND LEVEL CONTROL"
	CreateProgressDialog(gettext(TEXT("_@M1214_")));
  else
	// LKTOKENS _@M1215_ "NO SOUND LEVEL CONTROL"
	CreateProgressDialog(gettext(TEXT("_@M1215_")));
#endif

  RasterTerrain::OpenTerrain();

  ReadWayPoints();
  StartupStore(_T(". Total %d waypoints%s"),NumberOfWayPoints,NEWLINE);
  InitWayPointCalc(); 
  InitLDRotary(&rotaryLD); 
  InitWindRotary(&rotaryWind); // 100103
  MapWindow::zoom.Reset();
  InitLK8000();
  ReadAirfieldFile();
  SetHome(false);
  LKReadLanguageFile();
  LKLanguageReady=true;

  RasterTerrain::ServiceFullReload(GPS_INFO.Latitude, 
                                   GPS_INFO.Longitude);

  CAirspaceManager::Instance().ReadAirspaces();
  CAirspaceManager::Instance().SortAirspaces();
  OpenTopology();
  #if USETOPOMARKS
  TopologyInitialiseMarks();
  #endif

  OpenFLARMDetails();

  // ... register all supported devices
  // IMPORTANT: ADD NEW ONES TO BOTTOM OF THIS LIST
  // LKTOKEN _@M1217_ "Starting devices"
  // Please check that the number of devices is not exceeding NUMREGDEV in device.h
  CreateProgressDialog(gettext(TEXT("_@M1217_")));
  #if TESTBENCH
  StartupStore(TEXT(". Register serial devices%s"),NEWLINE);
  #endif
  disRegister(); // must be first
  genRegister(); // must be second, since we Sort(2) in dlgConfiguration
  cai302Register();
  ewRegister();
  #if !110101
  atrRegister();
  vgaRegister();
  #endif
  caiGpsNavRegister();
  nmoRegister();
  pgRegister();
  b50Register();
  vlRegister();
  ewMicroRecorderRegister();
  DevLX::Register();
  DevLXNano::Register();
  zanderRegister();
  flymasterf1Register();
  CompeoRegister();
  xcom760Register();
  condorRegister();
  DigiflyRegister(); // 100209
  IlecRegister();
  DSXRegister();
  CDevIMI::Register();
  FlytecRegister();
  LK8EX1Register();
  WesterboerRegister();

// WINDOWSPC _SIM_ devInit called twice missing devA name
// on PC nonSIM we cannot use devInit here! Generic device is used until next port reset!

#if 110530
  // we need devInit for all devices. Missing initialization otherwise.
  LockComm();
  devInit(TEXT("")); 
  UnlockComm();
#else
  // I dont remember anymore WHY! Probably it has been fixed already! paolo
  #if (WINDOWSPC>0)
  if (SIMMODE) devInit(TEXT(""));      
  #endif
#endif


  // re-set polar in case devices need the data
  #if TESTBENCH
  StartupStore(TEXT(". GlidePolar::SetBallast%s"),NEWLINE);
  #endif
  GlidePolar::SetBallast();

  // LKTOKEN _@M1218_ "Initialising display"
  CreateProgressDialog(gettext(TEXT("_@M1218_")));

  // just about done....

  DoSunEphemeris(GPS_INFO.Longitude, GPS_INFO.Latitude);

  // Finally ready to go
  StartupStore(TEXT(". CreateDrawingThread%s"),NEWLINE);
  MapWindow::CreateDrawingThread();
  Sleep(100);

  SwitchToMapWindow();
  StartupStore(TEXT(". CreateCalculationThread%s"),NEWLINE);
  CreateCalculationThread();
  #ifndef NOINSTHREAD
  while(!(goCalculationThread && goInstrumentThread)) Sleep(50); // 091119
  #else
  while(!(goCalculationThread)) Sleep(50); // 091119
  #endif

  // find unique ID of this PDA
  ReadAssetNumber();


  // Da-da, start everything now
  StartupStore(TEXT(". ProgramStarted=InitDone%s"),NEWLINE);
  ProgramStarted = psInitDone;

  GlobalRunning = true;

#if _DEBUG
 // _crtBreakAlloc = -1;     // Set this to the number in {} brackets to
                           // break on a memory leak
#endif

  // Main message loop:
  /* GlobalRunning && */
  while ( GetMessage(&msg, NULL, 0, 0)) {
	if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
  }
  LKObjects_Delete(); //@ 101124
  StartupStore(_T(". WinMain terminated%s"),NEWLINE);

#if (WINDOWSPC>0)
#if _DEBUG
  _CrtCheckMemory();
  _CrtDumpMemoryLeaks();
#endif
#endif

  return msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    It is important to call this function so that the application
//    will get 'well formed' small icons associated with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{

  WNDCLASS wc;
  WNDCLASS dc;

  GetClassInfo(hInstance,TEXT("DIALOG"),&dc);

   wc.style                      = CS_HREDRAW | CS_VREDRAW;
//  wc.style                      = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; // VENTA3 NO USE
  wc.lpfnWndProc                = (WNDPROC) WndProc;
  wc.cbClsExtra                 = 0;
#if (WINDOWSPC>0)
  wc.cbWndExtra = 0;
#else
  wc.cbWndExtra                 = dc.cbWndExtra ;
#endif
  wc.hInstance                  = hInstance;
  wc.hIcon                      = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_XCSOARSWIFT));
  wc.hCursor                    = 0;
  wc.hbrBackground              = (HBRUSH) GetStockObject(BLACK_BRUSH); 
  wc.lpszMenuName               = 0;
  wc.lpszClassName              = szWindowClass;

  if (!RegisterClass (&wc))
    return FALSE;

  // disabling DBLCLK here will make it not working in map window and maplocking failure
  wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
  wc.lpfnWndProc = (WNDPROC)MapWindow::MapWndProc;
  wc.cbClsExtra = 0;

#if (WINDOWSPC>0)
  wc.cbWndExtra = 0 ;
#else
  wc.cbWndExtra = dc.cbWndExtra ;
#endif

  wc.hInstance = hInstance;
  wc.hIcon = (HICON)NULL;
  wc.hCursor = NULL;
  wc.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH); // refixed 100101
  wc.lpszMenuName = 0;
  wc.lpszClassName = TEXT("MapWindowClass");

  return RegisterClass(&wc);

}


void ApplyClearType(LOGFONT *logfont) {

  // this has to be checked on PPC and old 2002 CE devices: using ANTIALIASED quality could be better
  // 110120  .. and in fact on ppc2002 no cleartype available
  logfont->lfQuality = GetFontRenderer();
}

bool IsNullLogFont(LOGFONT logfont) {
  bool bRetVal=false;

  LOGFONT LogFontBlank;
  memset ((char *)&LogFontBlank, 0, sizeof (LOGFONT));

  if ( memcmp(&logfont, &LogFontBlank, sizeof(LOGFONT)) == 0) {
    bRetVal=true;
  }
  return bRetVal;
}

void InitializeOneFont (HFONT * theFont, 
                               const TCHAR FontRegKey[] , 
                               LOGFONT autoLogFont, 
                               LOGFONT * LogFontUsed)
{
  LOGFONT logfont;
  int iDelStatus = 0;
  if (GetObjectType(*theFont) == OBJ_FONT) {
    iDelStatus=DeleteObject(*theFont); // RLD the EditFont screens use the Delete
  }

  memset ((char *)&logfont, 0, sizeof (LOGFONT));

  if (UseCustomFonts) {
    propGetFontSettings((TCHAR * )FontRegKey, &logfont);
    if (!IsNullLogFont(logfont)) {
      *theFont = CreateFontIndirect (&logfont);
      if (GetObjectType(*theFont) == OBJ_FONT) {
        if (LogFontUsed != NULL) *LogFontUsed = logfont; // RLD save for custom font GUI
      }
    }
  }

  if (GetObjectType(*theFont) != OBJ_FONT) {
    if (!IsNullLogFont(autoLogFont)) {
      ApplyClearType(&autoLogFont);
      *theFont = CreateFontIndirect (&autoLogFont);
      if (GetObjectType(*theFont) == OBJ_FONT) {
        if (LogFontUsed != NULL) *LogFontUsed = autoLogFont; // RLD save for custom font GUI
      }
    }
  }
}

void InitialiseFontsHardCoded(RECT rc,
                        LOGFONT * ptrhardTitleWindowLogFont,
                        LOGFONT * ptrhardMapWindowLogFont,
                        LOGFONT * ptrhardMapWindowBoldLogFont,
                        LOGFONT * ptrhardCDIWindowLogFont, // New
                        LOGFONT * ptrhardMapLabelLogFont,
                        LOGFONT * ptrhardStatisticsLogFont) {



  memset ((char *)ptrhardTitleWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardMapWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardMapWindowBoldLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardCDIWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardMapLabelLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardStatisticsLogFont, 0, sizeof (LOGFONT));


/*
 * VENTA-ADDON 2/2/08 
 * Adding custom font settings for PNAs
 *
 * InfoWindowFont	= values inside infoboxes  like numbers, etc.
 * TitleWindowFont	= Titles of infoboxes like Next, WP L/D etc.
 * TitleSmallWindowFont = 
 * CDIWindowFont	= vario display, runway informations
 * MapLabelFont		= Flarm Traffic draweing and stats, map labels in italic
 * StatisticsFont
 * MapWindowFont	= text names on the map
 * MapWindowBoldFont = menu buttons, waypoint selection, messages, etc.
 *
 *
 */


   // If you set a font here for a specific resolution, no automatic font generation is used.
  if (ScreenSize==(ScreenSize_t)ss480x272) { // WQVGA  e.g. MIO
    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardTitleWindowLogFont); // 16 091120
    propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,4,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,700,1,0,0,0,0,0,4,2,Tahoma"), ptrhardMapLabelLogFont); // 100709
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    // propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardMapWindowLogFont); 091120
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardMapWindowLogFont);
    // propGetFontSettingsFromString(TEXT("16,0,0,0,500,0,0,0,0,0,0,4,2,TahomaBD"), ptrhardMapWindowBoldLogFont); 091120
    propGetFontSettingsFromString(TEXT("19,0,0,0,500,0,0,0,0,0,0,6,2,Tahoma"), ptrhardMapWindowBoldLogFont); 
  }
  else if (ScreenSize==(ScreenSize_t)ss720x408) { // WQVGA  e.g. MIO
    propGetFontSettingsFromString(TEXT("21,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont); // 16 091120
    propGetFontSettingsFromString(TEXT("23,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("23,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 100709
    propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("33,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("30,0,0,0,700,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardMapWindowBoldLogFont);
  }

  else if (ScreenSize==(ScreenSize_t)ss480x234) { // e.g. Messada 2440
    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    //propGetFontSettingsFromString(TEXT("13,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 14 091120
    propGetFontSettingsFromString(TEXT("16,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 100709
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    // propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont); 091120
    propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    // propGetFontSettingsFromString(TEXT("16,0,0,0,500,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardMapWindowBoldLogFont); 091120
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
  }

  else if (ScreenSize==(ScreenSize_t)ss800x480) {// e.g. ipaq 31x {

    propGetFontSettingsFromString(TEXT("20,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardCDIWindowLogFont);
    // propGetFontSettingsFromString(TEXT("26,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); pre 100709
    propGetFontSettingsFromString(TEXT("28,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 100709
    propGetFontSettingsFromString(TEXT("48,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("36,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);


  }
  // added 091204
  else if (ScreenSize==(ScreenSize_t)ss400x240) {

    propGetFontSettingsFromString(TEXT("10,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardCDIWindowLogFont);
    // propGetFontSettingsFromString(TEXT("13,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); //v2.2 topology
    propGetFontSettingsFromString(TEXT("15,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont);
    propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);


  }
  else if (ScreenSize==(ScreenSize_t)ss640x480) { // real VGA, not fake VGA
    propGetFontSettingsFromString(TEXT("19,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);  // infobox titles
    propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont); // waynotes
    // propGetFontSettingsFromString(TEXT("23,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // old topo labels
    propGetFontSettingsFromString(TEXT("26,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 100709 topo labels
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);  // wps and mapscale
    propGetFontSettingsFromString(TEXT("28,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont); // bold version of MapW
										// and all messages and menus
  }
  else if (ScreenSize==(ScreenSize_t)ss896x672) { // real VGA, not fake VGA
    propGetFontSettingsFromString(TEXT("25,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);  // infobox titles
    propGetFontSettingsFromString(TEXT("33,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont); // waynotes
    propGetFontSettingsFromString(TEXT("32,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // topo labels
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);  // wps and mapscale
    propGetFontSettingsFromString(TEXT("39,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont); // bold version of MapW
										// and all messages and menus
  }
  else if (ScreenSize==(ScreenSize_t)ss320x240) { // also applies for fake VGA where all values are doubled stretched
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,500,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 100819
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss240x320) { // also applies for fake VGA where all values are doubled stretched
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("13,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // RLD 16 works well too
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,500,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss272x480) { 
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); 
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss480x640) { // real VGA, not fake VGA
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);  // infobox titles
    propGetFontSettingsFromString(TEXT("26,0,0,0,100,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont); // waynotes
    propGetFontSettingsFromString(TEXT("23,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // topo labels
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);  // wps and mapscale
    propGetFontSettingsFromString(TEXT("28,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont); // bold version of MapW
										// and all messages and menus
  }
  else if (ScreenSize==(ScreenSize_t)ss480x800) { // real VGA, not fake VGA
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);  // infobox titles
    propGetFontSettingsFromString(TEXT("26,0,0,0,100,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont); // waynotes
    propGetFontSettingsFromString(TEXT("23,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // topo labels
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);  // wps and mapscale
    propGetFontSettingsFromString(TEXT("30,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont); // bold version of MapW
										// and all messages and menus
  }




}



void InitialiseFonts(RECT rc)
{ //this routine must be called only at start/restart b/c there are many pointers to these fonts

  DeleteObject(TitleWindowFont);
  DeleteObject(MapWindowFont);
  DeleteObject(MapWindowBoldFont);
  DeleteObject(CDIWindowFont);
  DeleteObject(MapLabelFont);
  DeleteObject(StatisticsFont);

  LOGFONT hardTitleWindowLogFont;
  LOGFONT hardMapWindowLogFont;
  LOGFONT hardMapWindowBoldLogFont;
  LOGFONT hardCDIWindowLogFont; 
  LOGFONT hardMapLabelLogFont;
  LOGFONT hardStatisticsLogFont;

  memset ((char *)&hardTitleWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardMapWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardMapWindowBoldLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardCDIWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardMapLabelLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardStatisticsLogFont, 0, sizeof (LOGFONT));

  InitialiseFontsHardCoded(rc,
                        &hardTitleWindowLogFont,
                        &hardMapWindowLogFont,
                        &hardMapWindowBoldLogFont,
                        &hardCDIWindowLogFont, // New
                        &hardMapLabelLogFont,
                        &hardStatisticsLogFont);

  //
  // Merge the "hard" into the "auto" if one exists 
  //


  if (!IsNullLogFont(hardTitleWindowLogFont))
    autoTitleWindowLogFont = hardTitleWindowLogFont;

  if (!IsNullLogFont(hardMapWindowLogFont))
    autoMapWindowLogFont = hardMapWindowLogFont;


  if (!IsNullLogFont(hardMapWindowBoldLogFont))
    autoMapWindowBoldLogFont = hardMapWindowBoldLogFont;

  if (!IsNullLogFont(hardCDIWindowLogFont))
    autoCDIWindowLogFont = hardCDIWindowLogFont;

  if (!IsNullLogFont(hardMapLabelLogFont))
    autoMapLabelLogFont = hardMapLabelLogFont;

  if (!IsNullLogFont(hardStatisticsLogFont))
    autoStatisticsLogFont = hardStatisticsLogFont;


  InitializeOneFont (&TitleWindowFont, 
                        szRegistryFontTitleWindowFont, 
                        autoTitleWindowLogFont,
                        NULL);

  InitializeOneFont (&CDIWindowFont, 
                        szRegistryFontCDIWindowFont, 
                        autoCDIWindowLogFont,
                        NULL);

  InitializeOneFont (&MapLabelFont, 
                        szRegistryFontMapLabelFont, 
                        autoMapLabelLogFont,
                        NULL);

  InitializeOneFont (&StatisticsFont, 
                        szRegistryFontStatisticsFont, 
                        autoStatisticsLogFont,
                        NULL);

  InitializeOneFont (&MapWindowFont, 
                        szRegistryFontMapWindowFont, 
                        autoMapWindowLogFont,
                        NULL);

  InitializeOneFont (&MapWindowBoldFont, 
                        szRegistryFontMapWindowBoldFont, 
                        autoMapWindowBoldLogFont,
                        NULL);

  SendMessage(hWndMapWindow,WM_SETFONT,
              (WPARAM)MapWindowFont,MAKELPARAM(TRUE,0));

}



//
//  FUNCTION: InitInstance(HANDLE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//  COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  TCHAR szTitle[MAX_LOADSTRING];                        // The title bar text
  TCHAR szWindowClass[MAX_LOADSTRING];                  // The window class name
  RECT rc;

  hInst = hInstance;            // Store instance handle in our global variable


  LoadString(hInstance, IDC_XCSOAR, szWindowClass, MAX_LOADSTRING);
  LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

  // If it is already running, then focus on the window
  // problem was  that if two instances are started within a few seconds, both will survive!
  // We enforceed this with mutex at the beginning of WinMain
  hWndMainWindow = FindWindow(szWindowClass, szTitle);
  if (hWndMainWindow)
    {
      SetForegroundWindow((HWND)((ULONG) hWndMainWindow | 0x00000001));
      return 0;
    }
  InitLKScreen();
  InitLKFonts(); // causing problems with CreateButtonLabels?
  PreloadInitialisation(true);

  MyRegisterClass(hInst, szWindowClass);

  RECT WindowSize;

  WindowSize.left = 0;
  WindowSize.top = 0;
  WindowSize.right = GetSystemMetrics(SM_CXSCREEN);
  WindowSize.bottom = GetSystemMetrics(SM_CYSCREEN);


#if (WINDOWSPC>0)
  WindowSize.right = SCREENWIDTH 
    + 2*GetSystemMetrics( SM_CXFIXEDFRAME);
  WindowSize.left = (GetSystemMetrics(SM_CXSCREEN) - WindowSize.right) / 2;
  WindowSize.bottom = SCREENHEIGHT 
    + 2*GetSystemMetrics( SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
  WindowSize.top = (GetSystemMetrics(SM_CYSCREEN) - WindowSize.bottom) / 2;
#endif

  if (!goInstallSystem) Sleep(50); // 091119
  StartupStore(TEXT(". Create main window%s"),NEWLINE);

  hWndMainWindow = CreateWindow(szWindowClass, szTitle,
                                WS_SYSMENU
                                | WS_CLIPCHILDREN
				| WS_CLIPSIBLINGS,
                                WindowSize.left, WindowSize.top,
				WindowSize.right, WindowSize.bottom,
                                NULL, NULL,
				hInstance, NULL);

  if (!hWndMainWindow)
    {
      return FALSE;
    }


  hBrushSelected = (HBRUSH)CreateSolidBrush(ColorSelected);
  hBrushUnselected = (HBRUSH)CreateSolidBrush(ColorUnselected);
  hBrushButton = (HBRUSH)CreateSolidBrush(ColorButton);

  GetClientRect(hWndMainWindow, &rc);

#if (WINDOWSPC>0)
  rc.left = 0;
  rc.right = SCREENWIDTH;
  rc.top = 0;
  rc.bottom = SCREENHEIGHT;
#endif


  LKObjects_Create(); 

  ButtonLabel::CreateButtonLabels(rc);
  ButtonLabel::SetLabelText(0,TEXT("MODE"));

  InitialiseFonts(rc);
  InitLKFonts();	// reload updating LK fonts after loading profile for fontquality

  ButtonLabel::SetFont(MapWindowBoldFont);

  Message::Initialize(rc); // creates window, sets fonts

  ShowWindow(hWndMainWindow, SW_SHOW);

  StartupStore(TEXT(". Create map window%s"),NEWLINE);

  hWndMapWindow = CreateWindow(TEXT("MapWindowClass"),NULL,
			       WS_VISIBLE | WS_CHILD
			       | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                               0, 0, (rc.right - rc.left),
			       (rc.bottom-rc.top) ,
                               hWndMainWindow, NULL ,hInstance,NULL);

  ShowWindow(hWndMainWindow, nCmdShow);

  UpdateWindow(hWndMainWindow);
    
  return TRUE;
}


// Obfuscated C code contest winner
int getInfoType(int i) {
  int retval = 0;
  if (i<0) return 0; // error

    switch(MapWindow::mode.Fly()) {
    case MapWindow::Mode::MODE_FLY_CIRCLING:
      retval = InfoType[i] & 0xff; // climb
      break;
    case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
      retval = (InfoType[i] >> 16) & 0xff; //final glide
      break;
    case MapWindow::Mode::MODE_FLY_CRUISE:
      retval = (InfoType[i] >> 8) & 0xff; // cruise
      break;
    case MapWindow::Mode::MODE_FLY_NONE:
      break;
    }
  return min(NumDataOptions-1,retval);
}


void setInfoType(int i, char j) {
  if (i<0) return; // error

    switch(MapWindow::mode.Fly()) {
    case MapWindow::Mode::MODE_FLY_CIRCLING:
      InfoType[i] &= 0xffffff00;
      InfoType[i] += (j);
      break;
    case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
      InfoType[i] &= 0xff00ffff;
      InfoType[i] += (j<<16);
      break;
    case MapWindow::Mode::MODE_FLY_CRUISE:
      InfoType[i] &= 0xffff00ff;
      InfoType[i] += (j<<8);
      break;
    case MapWindow::Mode::MODE_FLY_NONE:
      break;
    }
}


bool Debounce(void) {
  static DWORD fpsTimeLast= 0;
  DWORD fpsTimeThis = ::GetTickCount();
  DWORD dT = fpsTimeThis-fpsTimeLast;

  DisplayTimeOut = 0;
  InterfaceTimeoutReset();

  if (dT>(unsigned int)debounceTimeout) {
    fpsTimeLast = fpsTimeThis;
    return true;
  } else {
    return false;
  }
}


void Shutdown(void) {
  int i;

  LKSound(_T("LK_DISCONNECT.WAV")); Sleep(500); // real WAV length is 410+ms
  if (!GlobalRunning) { // shutdown on startup clicking on the X
	StartupStore(_T(". Quick shutdown requested before terminating startup%s"),NEWLINE);
	LKRunStartEnd(false);
	exit(0);
  }
  // LKTOKEN _@M1219_ "Shutdown, please wait..."
  CreateProgressDialog(gettext(TEXT("_@M1219_")));

  StartupStore(_T(". Entering shutdown %s%s"), WhatTimeIsIt(),NEWLINE);
  StartupLogFreeRamAndStorage();

  // turn off all displays
  GlobalRunning = false;

  // LKTOKEN _@M1220_ "Shutdown, saving logs..."
  CreateProgressDialog(gettext(TEXT("_@M1220_")));
  // stop logger
  guiStopLogger(true);

  // LKTOKEN _@M1221_ "Shutdown, saving profile..."
  CreateProgressDialog(gettext(TEXT("_@M1221_")));
  // Save settings
  StoreRegistry();

  #if TESTBENCH
  StartupStore(TEXT(". Save_Recent_WP_history%s"),NEWLINE);
  #endif
  SaveRecentList();
  // Stop sound

  // Stop drawing
  // LKTOKEN _@M1219_ "Shutdown, please wait..."
  CreateProgressDialog(gettext(TEXT("_@M1219_")));
  
  StartupStore(TEXT(". CloseDrawingThread%s"),NEWLINE);
  // 100526 this is creating problem in SIM mode when quit is called from X button, and we are in waypoint details
  // or probably in other menu related screens. However it cannot happen from real PNA or PDA because we don't have
  // that X button.
  MapWindow::CloseDrawingThread();

  // Stop calculating too (wake up)
  SetEvent(dataTriggerEvent);
  SetEvent(drawTriggerEvent);

  // Clear data
  // LKTOKEN _@M1222_ "Shutdown, saving task..."
  CreateProgressDialog(gettext(TEXT("_@M1222_")));
  #if TESTBENCH
  StartupStore(TEXT(". Save default task%s"),NEWLINE);
  #endif
  SaveDefaultTask();

  #if TESTBENCH
  StartupStore(TEXT(". Clear task data%s"),NEWLINE);
  #endif

  LockTaskData();
  Task[0].Index = -1;  ActiveWayPoint = -1; 
  AATEnabled = FALSE;
  CloseWayPoints();
  UnlockTaskData();

  // LKTOKEN _@M1219_ "Shutdown, please wait..."
  CreateProgressDialog(gettext(TEXT("_@M1219_")));
  #if TESTBENCH
  StartupStore(TEXT(". CloseTerrainTopology%s"),NEWLINE);
  #endif

  RasterTerrain::CloseTerrain();

  CloseTopology();
  #if USETOPOMARKS
  TopologyCloseMarks();
  #endif
  CloseTerrainRenderer();

  // Stop COM devices
  StartupStore(TEXT(". Stop COM devices%s"),NEWLINE);
  devCloseAll();

  CloseFLARMDetails();

  ProgramStarted = psInitInProgress;

  // Kill windows
  #if TESTBENCH
  StartupStore(TEXT(". Close Messages%s"),NEWLINE);
  #endif
  Message::Destroy();
  #if TESTBENCH 
  StartupStore(TEXT(". Destroy Button Labels%s"),NEWLINE);
  #endif
  ButtonLabel::Destroy();

  #if TESTBENCH
  StartupStore(TEXT(". Delete Objects%s"),NEWLINE);
  #endif
  
  //  CommandBar_Destroy(hWndCB);

  // Kill graphics objects

  DeleteObject(hBrushSelected);
  DeleteObject(hBrushUnselected);
  DeleteObject(hBrushButton);
  
  DeleteObject(TitleWindowFont);
  DeleteObject(CDIWindowFont);
  DeleteObject(MapLabelFont);
  DeleteObject(MapWindowFont);
  DeleteObject(MapWindowBoldFont);
  DeleteObject(StatisticsFont);  
  CAirspaceManager::Instance().CloseAirspaces();
  StartupStore(TEXT(". Delete Critical Sections%s"),NEWLINE);
  
  DeleteCriticalSection(&CritSec_EventQueue);
  csEventQueueInitialized = false;
  DeleteCriticalSection(&CritSec_TaskData);
  csTaskDataInitialized = false;
  DeleteCriticalSection(&CritSec_FlightData);
  csFlightDataInitialized = false;
  DeleteCriticalSection(&CritSec_NavBox);
  csNavBoxInitialized = false;
  DeleteCriticalSection(&CritSec_Comm);
  csCommInitialized = false;
  DeleteCriticalSection(&CritSec_TerrainDataCalculations);
  csTerrainDataGraphicsInitialized = false;
  DeleteCriticalSection(&CritSec_TerrainDataGraphics);
  csTerrainDataCalculationsInitialized = false;

  StartupStore(TEXT(". Close Progress Dialog%s"),NEWLINE);

  CloseProgressDialog();
  #if TESTBENCH
  StartupStore(TEXT(". Close Calculations%s"),NEWLINE);
  #endif
  CloseCalculations();

  CloseGeoid();
  DeInitCustomHardware();

  StartupStore(TEXT(". Close Windows%s"),NEWLINE);
  DestroyWindow(hWndMapWindow);
  DestroyWindow(hWndMainWindow);
      
  StartupStore(TEXT(". Close Event Handles%s"),NEWLINE);
  CloseHandle(drawTriggerEvent);
  CloseHandle(dataTriggerEvent);
  CloseHandle(varioTriggerEvent);

#ifdef DEBUG_TRANSLATIONS
  StartupStore(TEXT(".. Writing missing translations%s"),NEWLINE);
  WriteMissingTranslations();
#endif

  StartupLogFreeRamAndStorage();
  for (i=0;i<NUMDEV;i++) {
	if (ComPortStatus[i]!=0) {
		StartupStore(_T(". ComPort %d: status=%d Rx=%d Tx=%d ErrRx=%d + ErrTx=%d (==%d)%s"), i,
		ComPortStatus[i], ComPortRx[i],ComPortTx[i], ComPortErrRx[i],ComPortErrTx[i],ComPortErrors[i],NEWLINE);
	}
  }
  StartupStore(_T(". Finished shutdown %s%s"), WhatTimeIsIt(),NEWLINE);
  LKRunStartEnd(false);
  // quitting PC version while menus are up will not terminate correctly. this is a workaround
  #if (WINDOWSPC>0)
  StartupStore(TEXT(". Program terminated%s"),NEWLINE); 
  exit(0);
  #endif 

#ifdef DEBUG
  TCHAR foop[80];
  TASK_POINT wp;
  TASK_POINT *wpr = &wp;
  _stprintf(foop,TEXT(". Sizes %d %d %d%s"),
	    sizeof(TASK_POINT), 
	    ((long)&wpr->AATTargetLocked)-((long)wpr),
	    ((long)&wpr->Target)-((long)wpr), NEWLINE
	    );
  StartupStore(foop);
#endif
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  long wdata;

  switch (message)
    {

    case WM_ERASEBKGND:
      return TRUE; // JMW trying to reduce screen flicker
      break;
    case WM_COMMAND:
      return MainMenu(hWnd, message, wParam, lParam);
      break;
    case WM_CTLCOLORSTATIC:
      wdata = GetWindowLong((HWND)lParam, GWL_USERDATA);
      switch(wdata) {
      case 0:
        SetBkColor((HDC)wParam, ColorUnselected);
        SetTextColor((HDC)wParam, RGB(0x00,0x00,0x00));
        return (LRESULT)hBrushUnselected;
      case 1:
        SetBkColor((HDC)wParam, ColorSelected);
        SetTextColor((HDC)wParam, RGB(0x00,0x00,0x00));
        return (LRESULT)hBrushSelected;
      case 2:
	SetBkColor((HDC)wParam, ColorUnselected);
        SetTextColor((HDC)wParam, ColorWarning);
	return (LRESULT)hBrushUnselected;
      case 3:
	SetBkColor((HDC)wParam, ColorUnselected);
        SetTextColor((HDC)wParam, ColorOK);
	return (LRESULT)hBrushUnselected;
      case 4:
	// black on light green
        SetTextColor((HDC)wParam, RGB_BLACK); 
	SetBkColor((HDC)wParam, ColorButton);
	return (LRESULT)hBrushButton;
      case 5:
	// grey on light green
	SetBkColor((HDC)wParam, ColorButton);
        SetTextColor((HDC)wParam, RGB(0x80,0x80,0x80));
	return (LRESULT)hBrushButton;
/*
      default: // added 091230
	SetBkColor((HDC)wParam, ColorButton);
        SetTextColor((HDC)wParam, RGB(0xff,0xbe,0x00));
	return (LRESULT)hBrushButton;
*/
      }
      break;
    case WM_CREATE:
#ifdef HAVE_ACTIVATE_INFO
      memset (&s_sai, 0, sizeof (s_sai));
      s_sai.cbSize = sizeof (s_sai);
#endif
      //if (hWnd==hWndMainWindow) {
      if (iTimerID == 0) {
        iTimerID = SetTimer(hWnd,1000,500,NULL); // 500ms  2 times per second
      }

      //      hWndCB = CreateRpCommandBar(hWnd);

      break;

    case WM_ACTIVATE:
      if(LOWORD(wParam) != WA_INACTIVE)
        {
          SetWindowPos(hWndMainWindow,HWND_TOP,
                 0, 0, 0, 0,
                 SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);

#ifdef HAVE_ACTIVATE_INFO
         SHFullScreen(hWndMainWindow,SHFS_HIDETASKBAR|SHFS_HIDESIPBUTTON|SHFS_HIDESTARTICON);
#endif

        }
#ifdef HAVE_ACTIVATE_INFO
      if (api_has_SHHandleWMActivate) {
        SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
      } else {
        #ifdef ALPHADEBUG
        StartupStore(TEXT("... SHHandleWMActivate not available%s"),NEWLINE);
        #endif
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
#endif
      break;

    case WM_SETTINGCHANGE:
#ifdef HAVE_ACTIVATE_INFO
      if (api_has_SHHandleWMSettingChange) {
        SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
      } else {
        #ifdef ALPHADEBUG
        StartupStore(TEXT("... SHHandleWMSettingChange not available%s"),NEWLINE);
        #endif
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
#endif
      break;

    case WM_SETFOCUS:
      // JMW not sure this ever does anything useful..
      if (ProgramStarted > psInitInProgress) {

      }
      break;
      // TODO enhancement: Capture KEYDOWN time
      // 	- Pass that (otpionally) to processKey, allowing
      // 	  processKey to handle long events - at any length
      // 	- Not sure how to do double click... (need timer call back
      // 	process unless reset etc... tricky)
      // we do this in WindowControls
    case WM_KEYUP: // JMW was keyup

      InterfaceTimeoutReset();

      /* DON'T PROCESS KEYS HERE WITH NEWINFOBOX, IT CAUSES CRASHES! */
      break;
	  //VENTA DBG
#ifdef VENTA_DEBUG_EVENT
	case WM_KEYDOWN:	

		DoStatusMessage(TEXT("DBG KDOWN 1")); // VENTA
		InterfaceTimeoutReset();
	      break;
	case WM_SYSKEYDOWN:	
		DoStatusMessage(TEXT("DBG SYSKDOWN 1")); // VENTA
		InterfaceTimeoutReset();
	      break;
#endif
	//END VENTA DBG

    case WM_TIMER:
	// WM_TIMER is run at about 2hz.
	LKHearthBeats++; // 100213
      //      ASSERT(hWnd==hWndMainWindow);
      if (ProgramStarted > psInitInProgress) {
	if (SIMMODE)
		SIMProcessTimer();
	else
		ProcessTimer();
	if (ProgramStarted==psFirstDrawDone) {
	  AfterStartup();
	  ProgramStarted = psNormalOp;
          StartupStore(_T(". ProgramStarted=NormalOp %s%s"), WhatTimeIsIt(),NEWLINE);
          StartupLogFreeRamAndStorage();

	}
      }
      break;

    case WM_INITMENUPOPUP:
      if (ProgramStarted > psInitInProgress) {
	  CheckMenuItem((HMENU) wParam,IDM_FILE_LOCK,MF_CHECKED|MF_BYCOMMAND);
	
	if(LoggerActive)
	  CheckMenuItem((HMENU) wParam,IDM_FILE_LOGGER,MF_CHECKED|MF_BYCOMMAND);
	else
	  CheckMenuItem((HMENU) wParam,IDM_FILE_LOGGER,MF_UNCHECKED|MF_BYCOMMAND);
      }
      break;

    case WM_CLOSE:

      ASSERT(hWnd==hWndMainWindow);
      if((hWnd==hWndMainWindow) && 
         (MessageBoxX(hWndMainWindow,
		// LKTOKEN  _@M198_ = "Confirm Exit?"
               	gettext(TEXT("_@M198_")),
                      TEXT("LK8000"),
                      MB_YESNO|MB_ICONQUESTION) == IDYES)) 
        {
          if(iTimerID) {
            KillTimer(hWnd,iTimerID);
            iTimerID = 0;
          }
          
          Shutdown();
        }
      break;

    case WM_DESTROY:
      if (hWnd==hWndMainWindow) {
        PostQuitMessage(0);
      }
      break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  return 0;
}


/* JMW no longer needed
HWND CreateRpCommandBar(HWND hwnd)
{
  SHMENUBARINFO mbi;

  memset(&mbi, 0, sizeof(SHMENUBARINFO));
  mbi.cbSize     = sizeof(SHMENUBARINFO);
  mbi.hwndParent = hwnd;
  mbi.dwFlags = SHCMBF_EMPTYBAR|SHCMBF_HIDDEN;
  mbi.nToolBarId = IDM_MENU;
  mbi.hInstRes   = hInst;
  mbi.nBmpId     = 0;
  mbi.cBmpImages = 0;

  if (!SHCreateMenuBar(&mbi))
    return NULL;

  return mbi.hwndMB;
}
*/

LRESULT MainMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int wmId, wmEvent;
  HWND wmControl;

  wmId    = LOWORD(wParam);
  wmEvent = HIWORD(wParam);
  wmControl = (HWND)lParam;

  if(wmControl != NULL) {
    if (ProgramStarted==psNormalOp) {

      DialogActive = false;

      FullScreen();

      /*
      if (!InfoWindowActive) {
        ShowMenu();
      }
      */
      Message::CheckTouch(wmControl);
        
      if (ButtonLabel::CheckButtonPress(wmControl)) {
        return TRUE; // don't continue processing..
      }
      
    }
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}



#include "winbase.h"


void CommonProcessTimer()
{

  // service the GCE and NMEA queue
  if (ProgramStarted==psNormalOp) {
    InputEvents::DoQueuedEvents();
	  // only shows the dialog if needed.
	  ShowAirspaceWarningsToUser();
  }

#if (WINDOWSPC<1)
  SystemIdleTimerReset();
#endif

    if(MenuTimeOut==MenuTimeoutMax) {
      if (!MapWindow::mode.AnyPan()) {
	InputEvents::setMode(TEXT("default"));
      }
    }
    MenuTimeOut++;

  UpdateBatteryInfos();

  if (!DialogActive) {
    DisplayTimeOut++;
  } else {
    // JMW don't let display timeout while a dialog is active,
    // but allow button presses to trigger redisplay
    if (DisplayTimeOut>1) {
      DisplayTimeOut=1;
    }
  }

  if (MapWindow::IsDisplayRunning()) {
  }

  //
  // maybe block/delay this if a dialog is active?
  // JMW: is done in the message function now.
    if (Message::Render()) {
      // turn screen on if blanked and receive a new message 
      DisplayTimeOut=0;
    }

  static int iheapcompact = 0;
  // called 2 times per second, compact heap every minute.
  iheapcompact++;
  if (iheapcompact == 120) {
    MyCompactHeaps();
    iheapcompact = 0;
  }
}

// this part should be rewritten
int ConnectionProcessTimer(int itimeout) {
  LockComm();
  NMEAParser::UpdateMonitor();
  UnlockComm();
  
  static BOOL LastGPSCONNECT = FALSE;
  static BOOL CONNECTWAIT = FALSE;
  static BOOL LOCKWAIT = FALSE;
  
  //
  // replace bool with BOOL to correct warnings and match variable
  // declarations RB
  //
  BOOL gpsconnect = GPSCONNECT;
  
  if (GPSCONNECT) {
    extGPSCONNECT = TRUE;
  } 

  if (!extGPSCONNECT) {
    // if gps is not connected, set navwarning to true so
    // calculations flight timers don't get updated
    LockFlightData();
    GPS_INFO.NAVWarning = true;
    UnlockFlightData();
  }

  GPSCONNECT = FALSE;
  BOOL navwarning = (BOOL)(GPS_INFO.NAVWarning);

  if (gpsconnect && navwarning) {
	if (InterfaceTimeoutCheck()) {
		// do something when no gps fix since 1 hour.. *see Utils
	}
  }

  if((gpsconnect == FALSE) && (LastGPSCONNECT == FALSE)) {
	// re-draw screen every five seconds even if no GPS
	TriggerGPSUpdate();
      
	devLinkTimeout(devAll());

	if(LOCKWAIT == TRUE) {
		// gps was waiting for fix, now waiting for connection
		LOCKWAIT = FALSE;
	}
	if(!CONNECTWAIT) {
		// gps is waiting for connection first time
		extGPSCONNECT = FALSE;
  
		CONNECTWAIT = TRUE;
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) LKSound(TEXT("LK_GREEN.WAV"));
		#endif
		FullScreen();
	} else {
		// restart comm ports on timeouts, but not during managed special communications with devices
		// that will not provide NMEA stream, for example during a binary conversation for task declaration
		// or during a restart. Very careful, it shall be set to zero by the same function who
		// set it to true.
		if ((itimeout % 60 == 0) && !LKDoNotResetComms ) { 
			// no activity for 60/2 seconds (running at 2Hz), then reset.
			// This is needed only for virtual com ports..
			extGPSCONNECT = FALSE;
			if (!(devIsDisabled(0) && devIsDisabled(1))) {
			  InputEvents::processGlideComputer(GCE_COMMPORT_RESTART);
			  RestartCommPorts();
			}
	  
			itimeout = 0;
		}
	}
  }

  // Force RESET of comm ports on demand
  if (LKForceComPortReset) {
	StartupStore(_T(". ComPort RESET ordered%s"),NEWLINE);
	LKForceComPortReset=false;
	LKDoNotResetComms=false;
	if (MapSpaceMode != MSM_WELCOME)
		InputEvents::processGlideComputer(GCE_COMMPORT_RESTART);

	RestartCommPorts();
  }
  
  if((gpsconnect == TRUE) && (LastGPSCONNECT == FALSE)) {
	itimeout = 0; // reset timeout
      
	if(CONNECTWAIT) {
		TriggerGPSUpdate();
		CONNECTWAIT = FALSE;
	}
  }
  
  if((gpsconnect == TRUE) && (LastGPSCONNECT == TRUE)) {
	if((navwarning == TRUE) && (LOCKWAIT == FALSE)) {
		TriggerGPSUpdate();
	  
		LOCKWAIT = TRUE;
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) LKSound(TEXT("LK_GREEN.WAV")); // 100404
		#endif
		FullScreen();
	} else {
		if((navwarning == FALSE) && (LOCKWAIT == TRUE)) {
			TriggerGPSUpdate();
			LOCKWAIT = FALSE;
		}
	}
  }
  
  LastGPSCONNECT = gpsconnect;
  return itimeout;
}

void ProcessTimer(void)
{

  if (!GPSCONNECT && (DisplayTimeOut==0)) {
    // JMW 20071207
    // re-draw screen every five seconds even if no GPS
    // this prevents sluggish screen when inside hangar..
    TriggerGPSUpdate();
    DisplayTimeOut=1;
  }

  CommonProcessTimer();

  // now check GPS status

  static int itimeout = -1;
  itimeout++;
  
  // also service replay logger
  ReplayLogger::Update();
  if (ReplayLogger::IsEnabled()) {
    static double timeLast = 0;
	if (GPS_INFO.Time-timeLast>=1.0) {
	TriggerGPSUpdate();
    }
    timeLast = GPS_INFO.Time;
    GPSCONNECT = TRUE;
    extGPSCONNECT = TRUE;
    GPS_INFO.NAVWarning = FALSE;
    GPS_INFO.SatellitesUsed = 6;
    return;
  }
  
  if (itimeout % 10 == 0) {
    // check connection status every 5 seconds
    itimeout = ConnectionProcessTimer(itimeout);
  }
}

void SIMProcessTimer(void)
{

  CommonProcessTimer();

  GPSCONNECT = TRUE;
  extGPSCONNECT = TRUE;
  static int i=0;
  i++;

  if (!ReplayLogger::Update()) {

    // Process timer is run at 2hz, so this is bringing it back to 1hz
    if (i%2==0) return;

    extern void LKSimulator(void);
    LKSimulator();
  }

  if (i%2==0) return;

#ifdef DEBUG
  // use this to test FLARM parsing/display
  NMEAParser::TestRoutine(&GPS_INFO);
#endif

  TriggerGPSUpdate();

}


void SwitchToMapWindow(void)
{
  SetFocus(hWndMapWindow);
  if (  MenuTimeOut< MenuTimeoutMax) {
    MenuTimeOut = MenuTimeoutMax;
  }
}


void PopupAnalysis()
{
  DialogActive = true;
  dlgAnalysisShowModal(ANALYSYS_PAGE_DEFAULT);
  DialogActive = false;
}


void PopupWaypointDetails()
{
  // Quick is returning  0 for cancel or error, 1 for details, 2 for goto, 3 and 4 for alternates
  short ret= dlgWayQuickShowModal();
  // StartupStore(_T("... Quick ret=%d\n"),ret);
  switch(ret) {
	case 1:
		dlgWayPointDetailsShowModal();
		break;
	case 2:
		SetModeType(LKMODE_MAP,MP_MOVING);
		break;
	default:
		break;
  }
}


void PopupBugsBallast(int UpDown)
{
	(void)UpDown;
  DialogActive = true;
  //  ShowWindow(hWndCB,SW_HIDE);
  FullScreen();
  SwitchToMapWindow();
  DialogActive = false;
}



#include <stdio.h>

void DebugStore(const char *Str, ...)
{
#if defined(DEBUG)
  char buf[MAX_PATH];
  va_list ap;
  int len;

  va_start(ap, Str);
  len = vsprintf(buf, Str, ap);
  va_end(ap);

  LockFlightData();
  FILE *stream;
  TCHAR szFileName[] = TEXT(LKF_DEBUG);
  static bool initialised = false;
  if (!initialised) {
    initialised = true;
    stream = _wfopen(szFileName,TEXT("w"));
  } else {
    stream = _wfopen(szFileName,TEXT("a+"));
  }

  fwrite(buf,len,1,stream);

  fclose(stream);
  UnlockFlightData();
#endif
}

void FailStore(const TCHAR *Str, ...)
{
  TCHAR buf[MAX_PATH];
  va_list ap;

  va_start(ap, Str);
  _vstprintf(buf, Str, ap);
  va_end(ap);

  FILE *stream=NULL;
  static TCHAR szFileName[MAX_PATH];
  static bool initialised = false;

  if (!initialised) {
	LocalPath(szFileName, TEXT(LKF_FAILLOG));
	stream = _tfopen(szFileName, TEXT("ab+"));
	if (stream) {
		fclose(stream);
	}
	initialised = true;
  } 
  stream = _tfopen(szFileName,TEXT("ab+"));
  if (stream == NULL) {
	StartupStore(_T("------ FailStore failed, cannot open <%s>%s"), szFileName, NEWLINE);
	return;
  }
  fprintf(stream, "------%s%04d%02d%02d-%02d:%02d:%02d [%09u] FailStore Start, Version %s%s (%s %s) FreeRam=%ld %s",SNEWLINE,
	GPS_INFO.Year,GPS_INFO.Month,GPS_INFO.Day, GPS_INFO.Hour,GPS_INFO.Minute,GPS_INFO.Second,
	(unsigned int)GetTickCount(),LKVERSION, LKRELEASE,
	"",
#if WINDOWSPC >0
	"PC",
#else
	#ifdef PNA
	"PNA",
	#else
	"PDA",
	#endif
#endif

CheckFreeRam(),SNEWLINE); 
  fprintf(stream, "Message: %S%s", buf, SNEWLINE);
  fprintf(stream, "GPSINFO: Latitude=%f Longitude=%f Altitude=%f Speed=%f %s", 
	GPS_INFO.Latitude, GPS_INFO.Longitude, GPS_INFO.Altitude, GPS_INFO.Speed, SNEWLINE);

  fclose(stream);
  StartupStore(_T("------ %s%s"),buf,NEWLINE);
}



void StartupStore(const TCHAR *Str, ...)
{
  TCHAR buf[(MAX_PATH*2)+1]; // 260 chars normally  FIX 100205
  va_list ap;

  va_start(ap, Str);
  _vstprintf(buf, Str, ap);
  va_end(ap);

  if (csFlightDataInitialized) {
	LockFlightData();
  }
  FILE *startupStoreFile = NULL;
  static TCHAR szFileName[MAX_PATH];

  static bool initialised = false;
  if (!initialised) {
	LocalPath(szFileName, TEXT(LKF_RUNLOG));
	initialised = true;
  } 

  startupStoreFile = _tfopen(szFileName, TEXT("ab+"));
  if (startupStoreFile != NULL) {
    char sbuf[(MAX_PATH*2)+1]; // FIX 100205
    
    int i = unicode2utf(buf, sbuf, sizeof(sbuf));
    
    if (i > 0) {
      if (sbuf[i - 1] == 0x0a && (i == 1 || (i > 1 && sbuf[i-2] != 0x0d)))
        sprintf(sbuf + i - 1, SNEWLINE);
      fprintf(startupStoreFile, "[%09u] %s", (unsigned int)GetTickCount(), sbuf);
    }
    fclose(startupStoreFile);
  }
  if (csFlightDataInitialized) {
    UnlockFlightData();
  }
}


void LockNavBox() {
}

void UnlockNavBox() {
}

static int csCount_TaskData = 0;
static int csCount_FlightData = 0;
static int csCount_EventQueue = 0;

void LockTaskData() {
#ifdef HAVEEXCEPTIONS
  if (!csTaskDataInitialized) throw TEXT("LockTaskData Error");
#endif
  EnterCriticalSection(&CritSec_TaskData);
  csCount_TaskData++;
}

void UnlockTaskData() {
#ifdef HAVEEXCEPTIONS
  if (!csTaskDataInitialized) throw TEXT("LockTaskData Error");
#endif
  if (csCount_TaskData) 
    csCount_TaskData--;
  LeaveCriticalSection(&CritSec_TaskData);
}


void LockFlightData() {
#ifdef HAVEEXCEPTIONS
  if (!csFlightDataInitialized) throw TEXT("LockFlightData Error");
#endif
  EnterCriticalSection(&CritSec_FlightData);
  csCount_FlightData++;
}

void UnlockFlightData() {
#ifdef HAVEEXCEPTIONS
  if (!csFlightDataInitialized) throw TEXT("LockFlightData Error");
#endif
  if (csCount_FlightData)
    csCount_FlightData--;
  LeaveCriticalSection(&CritSec_FlightData);
}

void LockTerrainDataCalculations() {
#ifdef HAVEEXCEPTIONS
  if (!csTerrainDataCalculationsInitialized) throw TEXT("LockTerrainDataCalculations Error");
#endif
  EnterCriticalSection(&CritSec_TerrainDataCalculations);
}

void UnlockTerrainDataCalculations() {
#ifdef HAVEEXCEPTIONS
  if (!csTerrainDataCalculationsInitialized) throw TEXT("LockTerrainDataCalculations Error");
#endif
  LeaveCriticalSection(&CritSec_TerrainDataCalculations);
}

void LockTerrainDataGraphics() {
#ifdef HAVEEXCEPTIONS
  if (!csTerrainDataGraphicsInitialized) throw TEXT("LockTerrainDataGraphics Error");
#endif
  EnterCriticalSection(&CritSec_TerrainDataGraphics);
}

void UnlockTerrainDataGraphics() {
#ifdef HAVEEXCEPTIONS
  if (!csTerrainDataGraphicsInitialized) throw TEXT("LockTerrainDataGraphics Error");
#endif
  LeaveCriticalSection(&CritSec_TerrainDataGraphics);
}


void LockEventQueue() {
#ifdef HAVEEXCEPTIONS
  if (!csEventQueueInitialized) throw TEXT("LockEventQueue Error");
#endif
  EnterCriticalSection(&CritSec_EventQueue);
  csCount_EventQueue++;
}

void UnlockEventQueue() {
#ifdef HAVEEXCEPTIONS
  if (!csEventQueueInitialized) throw TEXT("LockEventQueue Error");
#endif
  if (csCount_EventQueue) 
    csCount_EventQueue--;
  LeaveCriticalSection(&CritSec_EventQueue);
}


#if (WINDOWSPC<1)
DWORD GetBatteryInfo(BATTERYINFO* pBatteryInfo)
{
    // set default return value
    DWORD result = 0;

    // check incoming pointer
    if(NULL == pBatteryInfo)
    {
        return 0;
    }

    SYSTEM_POWER_STATUS_EX2 sps;

    // request the power status
    result = GetSystemPowerStatusEx2(&sps, sizeof(sps), TRUE);

    // only update the caller if the previous call succeeded
    if(0 != result)
    {
        pBatteryInfo->acStatus = sps.ACLineStatus;
        pBatteryInfo->chargeStatus = sps.BatteryFlag;
        pBatteryInfo->BatteryLifePercent = sps.BatteryLifePercent;
	pBatteryInfo->BatteryVoltage = sps.BatteryVoltage;
	pBatteryInfo->BatteryAverageCurrent = sps.BatteryAverageCurrent;
	pBatteryInfo->BatteryCurrent = sps.BatteryCurrent;
	pBatteryInfo->BatterymAHourConsumed = sps.BatterymAHourConsumed;
	pBatteryInfo->BatteryTemperature = sps.BatteryTemperature;
    }

    return result;
}
#endif


void UpdateBatteryInfos(void) {

  #if (WINDOWSPC>0)
  return;
  #else

  BATTERYINFO BatteryInfo; 
  BatteryInfo.acStatus = 0;

  #ifdef PNA
  if (DeviceIsGM130) {
	PDABatteryPercent = GM130PowerLevel();
	PDABatteryStatus =  GM130PowerStatus();
	PDABatteryFlag =    GM130PowerFlag();

	PDABatteryTemperature = 0;
  } else 
  #endif
  if (GetBatteryInfo(&BatteryInfo)) {
    PDABatteryPercent = BatteryInfo.BatteryLifePercent;
    PDABatteryTemperature = BatteryInfo.BatteryTemperature; 
    PDABatteryStatus=BatteryInfo.acStatus;
    PDABatteryFlag=BatteryInfo.chargeStatus;

    // All you need to display extra Battery informations...
    //	TCHAR vtemp[1000];
    //	_stprintf(vtemp,_T("Battpercent=%d Volt=%d Curr=%d AvCurr=%d mAhC=%d Temp=%d Lifetime=%d Fulllife=%d\n"),
    //		BatteryInfo.BatteryLifePercent, BatteryInfo.BatteryVoltage, 
    //		BatteryInfo.BatteryCurrent, BatteryInfo.BatteryAverageCurrent,
    //		BatteryInfo.BatterymAHourConsumed,
    //		BatteryInfo.BatteryTemperature, BatteryInfo.BatteryLifeTime, BatteryInfo.BatteryFullLifeTime);
    //	StartupStore( vtemp );
  } 
  #endif
}


static void ReplaceInString(TCHAR *String, TCHAR *ToReplace, 
                            TCHAR *ReplaceWith, size_t Size){
  TCHAR TmpBuf[MAX_PATH];
  int   iR, iW;
  TCHAR *pC;

  while((pC = _tcsstr(String, ToReplace)) != NULL){
    iR = _tcsclen(ToReplace);
    iW = _tcsclen(ReplaceWith);
    _tcscpy(TmpBuf, pC + iR);
    _tcscpy(pC, ReplaceWith);
    _tcscat(pC, TmpBuf);
  }

}

static void CondReplaceInString(bool Condition, TCHAR *Buffer, 
                                TCHAR *Macro, TCHAR *TrueText, 
                                TCHAR *FalseText, size_t Size){
  if (Condition)
    ReplaceInString(Buffer, Macro, TrueText, Size);
  else
    ReplaceInString(Buffer, Macro, FalseText, Size);
}

bool ExpandMacros(const TCHAR *In, TCHAR *OutBuffer, size_t Size){
  // ToDo, check Buffer Size
  bool invalid = false;
  _tcsncpy(OutBuffer, In, Size);
  OutBuffer[Size-1] = '\0';
  TCHAR *a;
  short items=1;

  if (_tcsstr(OutBuffer, TEXT("$(")) == NULL) return false;
  a =_tcsstr(OutBuffer, TEXT("&("));
  if (a != NULL) {
	*a=_T('$');
	items=2;
  }

  if (_tcsstr(OutBuffer, TEXT("$(LOCKMODE"))) {
	if (LockMode(0)) {	// query availability
		TCHAR tbuf[10];
		_tcscpy(tbuf,_T(""));
		ReplaceInString(OutBuffer, TEXT("$(LOCKMODE)"), tbuf, Size);
		if (LockMode(1)) // query status
			_tcscpy(OutBuffer,gettext(_T("_@M965_"))); // UNLOCK\nSCREEN
		else
			_tcscpy(OutBuffer,gettext(_T("_@M966_"))); // LOCK\nSCREEN
		if (!LockMode(3)) invalid=true; // button not usable
	} else {
		// This will make the button invisible
		_tcscpy(OutBuffer,_T(""));
	}
	if (--items<=0) goto label_ret;
  }

  if (_tcsstr(OutBuffer, TEXT("$(MacCreadyValue)"))) { // 091214

	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%2.1lf"), iround(LIFTMODIFY*MACCREADY*10)/10.0);
	ReplaceInString(OutBuffer, TEXT("$(MacCreadyValue)"), tbuf, Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(MacCreadyMode)"))) { // 091214

	TCHAR tbuf[10];
	if (CALCULATED_INFO.AutoMacCready)  {
		switch(AutoMcMode) {
			case amcFinalGlide:
				_stprintf(tbuf,_T("%s"), gettext(TEXT("_@M1681_")));
				break;
			case amcAverageClimb:
				_stprintf(tbuf,_T("%s"), gettext(TEXT("_@M1682_")));
				break;
			case amcEquivalent:
				_stprintf(tbuf,_T("%s"), gettext(TEXT("_@M1683_")));
				break;
			case amcFinalAndClimb:
				if (CALCULATED_INFO.FinalGlide)
					_stprintf(tbuf,_T("%s"), gettext(TEXT("_@M1681_")));
				else
					_stprintf(tbuf,_T("%s"), gettext(TEXT("_@M1682_")));
				break;
			default:
				// LKTOKEN _@M1202_ "Auto"
				_stprintf(tbuf,_T("%s"), gettext(TEXT("_@M1202_")));
				break;
		}
	} else
		// LKTOKEN _@M1201_ "Man"
		_stprintf(tbuf,_T("%s"), gettext(TEXT("_@M1201_")));
	ReplaceInString(OutBuffer, TEXT("$(MacCreadyMode)"), tbuf, Size);
	if (--items<=0) goto label_ret; // 100517
  }

    if (_tcsstr(OutBuffer, TEXT("$(WaypointNext)"))) {
      // Waypoint\nNext
      invalid = !ValidTaskPoint(ActiveWayPoint+1);
      CondReplaceInString(!ValidTaskPoint(ActiveWayPoint+2), 
                          OutBuffer,
                          TEXT("$(WaypointNext)"), 
	// LKTOKEN  _@M801_ = "Waypoint\nFinish" 
                          gettext(TEXT("_@M801_")), 
	// LKTOKEN  _@M802_ = "Waypoint\nNext" 
                          gettext(TEXT("_@M802_")), Size);
	if (--items<=0) goto label_ret; // 100517
      
    } else
    if (_tcsstr(OutBuffer, TEXT("$(WaypointPrevious)"))) {
      if (ActiveWayPoint==1) {
        invalid = !ValidTaskPoint(ActiveWayPoint-1);
        ReplaceInString(OutBuffer, TEXT("$(WaypointPrevious)"), 
	// LKTOKEN  _@M804_ = "Waypoint\nStart" 
                        gettext(TEXT("_@M804_")), Size);
	if (--items<=0) goto label_ret; // 100517
      } else if (EnableMultipleStartPoints) {
        invalid = !ValidTaskPoint(0);
        CondReplaceInString((ActiveWayPoint==0), 
                            OutBuffer, 
                            TEXT("$(WaypointPrevious)"), 
	// LKTOKEN  _@M803_ = "Waypoint\nPrevious" 
                            TEXT("StartPoint\nCycle"), gettext(TEXT("_@M803_")), Size);
	if (--items<=0) goto label_ret; // 100517
      } else {
        invalid = (ActiveWayPoint<=0);
	// LKTOKEN  _@M803_ = "Waypoint\nPrevious" 
        ReplaceInString(OutBuffer, TEXT("$(WaypointPrevious)"), gettext(TEXT("_@M803_")), Size); 
	if (--items<=0) goto label_ret; // 100517
      }
    }

  if (_tcsstr(OutBuffer, TEXT("$(RealTask)"))) {
	if (! (ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1))) {
		invalid=true;
	}
	ReplaceInString(OutBuffer, TEXT("$(RealTask)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }


  if (_tcsstr(OutBuffer, TEXT("$(AdvanceArmed)"))) {
    switch (AutoAdvance) {
    case 0:
      ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), gettext(TEXT("_@M892_")), Size); // (manual)
      invalid = true;
      break;
    case 1:
      ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), gettext(TEXT("_@M893_")), Size); // (auto)
      invalid = true;
      break;
    case 2:
      if (ActiveWayPoint>0) {
        if (ValidTaskPoint(ActiveWayPoint+1)) {
          CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"), 
	// LKTOKEN  _@M161_ = "Cancel" 
                              gettext(TEXT("_@M161_")), 
	// LKTOKEN  _@M678_ = "TURN" 
				gettext(TEXT("_@M678_")), Size);
        } else {
          ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), 
	// LKTOKEN  _@M8_ = "(finish)" 
                          gettext(TEXT("_@M8_")), Size);
          invalid = true;
        }
      } else {
        CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"), 
	// LKTOKEN  _@M161_ = "Cancel" 
                            gettext(TEXT("_@M161_")), 
	// LKTOKEN  _@M571_ = "START" 
			gettext(TEXT("_@M571_")), Size);
      }
      break;
    case 3:
      if (ActiveWayPoint==0) {
        CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"), 
	// LKTOKEN  _@M161_ = "Cancel" 
                            gettext(TEXT("_@M161_")), 
	// LKTOKEN  _@M571_ = "START" 
			gettext(TEXT("_@M571_")), Size);
      } else if (ActiveWayPoint==1) {
        CondReplaceInString(AdvanceArmed, OutBuffer, TEXT("$(AdvanceArmed)"), 
	// LKTOKEN  _@M161_ = "Cancel" 
                            gettext(TEXT("_@M161_")), 
	// LKTOKEN  _@M539_ = "RESTART" 
			gettext(TEXT("_@M539_")), Size);
      } else {
        ReplaceInString(OutBuffer, TEXT("$(AdvanceArmed)"), gettext(TEXT("_@M893_")), Size); // (auto)
        invalid = true;
      }
      // TODO bug: no need to arm finish
    default:
      break;
    }
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(CheckFlying)"))) { // 100203
    if (!CALCULATED_INFO.Flying) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckFlying)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(CheckNotFlying)"))) { // 100223
    if (CALCULATED_INFO.Flying) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckNotFlying)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(CheckReplay)"))) {
    if (!ReplayLogger::IsEnabled() && GPS_INFO.MovementDetected) {
      invalid = true;
    } 
    ReplaceInString(OutBuffer, TEXT("$(CheckReplay)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(NotInReplay)"))) {
    if (ReplayLogger::IsEnabled()) {
      invalid = true;
    } 
    ReplaceInString(OutBuffer, TEXT("$(NotInReplay)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(CheckWaypointFile)"))) {
    if (!ValidWayPoint(NUMRESWP)) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckWaypointFile)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(CheckSettingsLockout)"))) {
    if (LockSettingsInFlight && CALCULATED_INFO.Flying) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckSettingsLockout)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(CheckTask)"))) {
    if (!ValidTaskPoint(ActiveWayPoint)) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckTask)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(CheckAirspace)"))) {
	if (!CAirspaceManager::Instance().ValidAirspaces()) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckAirspace)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(CheckFLARM)"))) {
    if (!GPS_INFO.FLARM_Available) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckFLARM)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(CheckTerrain)"))) {
    if (!CALCULATED_INFO.TerrainValid) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckTerrain)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(TerrainVisible)"))) {
    if (CALCULATED_INFO.TerrainValid && EnableTerrain && !LKVarioBar) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(TerrainVisible)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }

  // If it is not SIM mode, it is invalid
  if (_tcsstr(OutBuffer, TEXT("$(OnlyInSim)"))) {
	if (!SIMMODE) invalid = true;
	ReplaceInString(OutBuffer, TEXT("$(OnlyInSim)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(OnlyInFly)"))) {
	if (SIMMODE) invalid = true;
	ReplaceInString(OutBuffer, TEXT("$(OnlyInFly)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(DISABLED)"))) {
	invalid = true;
	ReplaceInString(OutBuffer, TEXT("$(DISABLED)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(HBARAVAILABLE)"))) {
    if (!GPS_INFO.BaroAltitudeAvailable) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(HBARAVAILABLE)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; 
  }
  if (_tcsstr(OutBuffer, TEXT("$(TOGGLEHBAR)"))) {
	if (!GPS_INFO.BaroAltitudeAvailable) {
		// LKTOKEN _@M1068_ "HBAR"
		ReplaceInString(OutBuffer, TEXT("$(TOGGLEHBAR)"), gettext(TEXT("_@M1068_")), Size);
	} else {
		if (EnableNavBaroAltitude)
			// LKTOKEN _@M1174_ "HGPS"
			ReplaceInString(OutBuffer, TEXT("$(TOGGLEHBAR)"), gettext(TEXT("_@M1174_")), Size);
		else
			// LKTOKEN _@M1068_ "HBAR"
			ReplaceInString(OutBuffer, TEXT("$(TOGGLEHBAR)"), gettext(TEXT("_@M1068_")), Size);
	}
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(WCSpeed)"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.0f%s"),SPEEDMODIFY*WindCalcSpeed,Units::GetUnitName(Units::GetUserHorizontalSpeedUnit()) );
	ReplaceInString(OutBuffer, TEXT("$(WCSpeed)"), tbuf, Size);
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(GS"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.0f%s"),SPEEDMODIFY*GPS_INFO.Speed,Units::GetUnitName(Units::GetUserHorizontalSpeedUnit()) );
	ReplaceInString(OutBuffer, TEXT("$(GS)"), tbuf, Size);
	if (--items<=0) goto label_ret;
  }
  if (_tcsstr(OutBuffer, TEXT("$(HGPS"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.0f%s"),ALTITUDEMODIFY*GPS_INFO.Altitude,Units::GetUnitName(Units::GetUserAltitudeUnit()) );
	ReplaceInString(OutBuffer, TEXT("$(HGPS)"), tbuf, Size);
	if (--items<=0) goto label_ret;
  }
  if (_tcsstr(OutBuffer, TEXT("$(TURN"))) {
	TCHAR tbuf[10];
	_stprintf(tbuf,_T("%.0f"),SimTurn);
	ReplaceInString(OutBuffer, TEXT("$(TURN)"), tbuf, Size);
	if (--items<=0) goto label_ret;
  }

  // This will make the button invisible
  if (_tcsstr(OutBuffer, TEXT("$(SIMONLY"))) {
	if (SIMMODE) {
		TCHAR tbuf[10];
		_tcscpy(tbuf,_T(""));
		ReplaceInString(OutBuffer, TEXT("$(SIMONLY)"), tbuf, Size);
	} else {
		_tcscpy(OutBuffer,_T(""));
	}
	if (--items<=0) goto label_ret;
  }

  if (_tcsstr(OutBuffer, TEXT("$(LoggerActive)"))) {
	CondReplaceInString(LoggerActive, OutBuffer, TEXT("$(LoggerActive)"), gettext(TEXT("_@M670_")), gettext(TEXT("_@M657_")), Size); // Stop Start
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(SnailTrailToggleName)"))) {
    switch(TrailActive) {
    case 0:
	// LKTOKEN  _@M410_ = "Long" 
      ReplaceInString(OutBuffer, TEXT("$(SnailTrailToggleName)"), gettext(TEXT("_@M410_")), Size);
      break;
    case 1:
	// LKTOKEN  _@M612_ = "Short" 
      ReplaceInString(OutBuffer, TEXT("$(SnailTrailToggleName)"), gettext(TEXT("_@M612_")), Size);
      break;
    case 2:
	// LKTOKEN  _@M312_ = "Full" 
      ReplaceInString(OutBuffer, TEXT("$(SnailTrailToggleName)"), gettext(TEXT("_@M312_")), Size);
      break;
    case 3:
	// LKTOKEN  _@M491_ = "OFF" 
      ReplaceInString(OutBuffer, TEXT("$(SnailTrailToggleName)"), gettext(TEXT("_@M491_")), Size);
      break;
    }
	if (--items<=0) goto label_ret; // 100517
  }
// VENTA3 VisualGlide
  if (_tcsstr(OutBuffer, TEXT("$(VisualGlideToggleName)"))) {
    switch(VisualGlide) {
    case 0:
      ReplaceInString(OutBuffer, TEXT("$(VisualGlideToggleName)"), gettext(TEXT("_@M894_")), Size); // ON
      break;
    case 1:
	if (ExtendedVisualGlide)
		// LKTOKEN _@M1205_ "Moving"
		ReplaceInString(OutBuffer, TEXT("$(VisualGlideToggleName)"), gettext(TEXT("_@M1205_")), Size);
	else
      		ReplaceInString(OutBuffer, TEXT("$(VisualGlideToggleName)"), gettext(TEXT("_@M491_")), Size); // OFF
      break;
    case 2:
      ReplaceInString(OutBuffer, TEXT("$(VisualGlideToggleName)"), gettext(TEXT("_@M491_")), Size); // OFF
      break;
    }
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(UseTE)"))) {
    switch(UseTotalEnergy) {
    case 0:
      ReplaceInString(OutBuffer, TEXT("$(UseTE)"), gettext(TEXT("_@M894_")), Size); // ON
      break;
    case 1:
      ReplaceInString(OutBuffer, TEXT("$(UseTE)"), gettext(TEXT("_@M491_")), Size); // OFF
      break;
    }
	if (--items<=0) goto label_ret;
  }

// VENTA3 AirSpace event
  if (_tcsstr(OutBuffer, TEXT("$(AirSpaceToggleName)"))) {
    switch(OnAirSpace) {
    case 0:
      ReplaceInString(OutBuffer, TEXT("$(AirSpaceToggleName)"), gettext(TEXT("_@M894_")), Size); // ON
      break;
    case 1:
      ReplaceInString(OutBuffer, TEXT("$(AirSpaceToggleName)"), gettext(TEXT("_@M491_")), Size); // OFF
      break;
    }
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(SHADING)"))) {
    if ( Shading )
      ReplaceInString(OutBuffer, TEXT("$(SHADING)"), gettext(TEXT("_@M491_")), Size); // OFF
    else
      ReplaceInString(OutBuffer, TEXT("$(SHADING)"), gettext(TEXT("_@M894_")), Size); // ON
	if (--items<=0) goto label_ret;
  }

  if (_tcsstr(OutBuffer, TEXT("$(PanModeStatus)"))) {
    if ( MapWindow::mode.AnyPan() )
      ReplaceInString(OutBuffer, TEXT("$(PanModeStatus)"), gettext(TEXT("_@M491_")), Size); // OFF
    else
      ReplaceInString(OutBuffer, TEXT("$(PanModeStatus)"), gettext(TEXT("_@M894_")), Size); // ON
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(EnableSoundModes)"))) {
    if (EnableSoundModes)
      ReplaceInString(OutBuffer, TEXT("$(EnableSoundModes)"), gettext(TEXT("_@M491_")), Size);
    else
      ReplaceInString(OutBuffer, TEXT("$(EnableSoundModes)"), gettext(TEXT("_@M894_")), Size);
	if (--items<=0) goto label_ret; // 100517
  }

  if (_tcsstr(OutBuffer, TEXT("$(ActiveMap)"))) {
    if (ActiveMap)
      ReplaceInString(OutBuffer, TEXT("$(ActiveMap)"), gettext(TEXT("_@M491_")), Size);
    else
      ReplaceInString(OutBuffer, TEXT("$(ActiveMap)"), gettext(TEXT("_@M894_")), Size);
	if (--items<=0) goto label_ret; // 100517
  }


  if (_tcsstr(OutBuffer, TEXT("$(NoSmart)"))) {
	if (DisplayOrientation == NORTHSMART) invalid = true;
	ReplaceInString(OutBuffer, TEXT("$(NoSmart)"), TEXT(""), Size);
	if (--items<=0) goto label_ret; // 100517
  }
  if (_tcsstr(OutBuffer, TEXT("$(OVERLAY"))) {
	if (Look8000==(Look8000_t)lxcNoOverlay)
		ReplaceInString(OutBuffer, TEXT("$(OVERLAY)"), gettext(TEXT("_@M894_")), Size);
	else
		ReplaceInString(OutBuffer, TEXT("$(OVERLAY)"), gettext(TEXT("_@M491_")), Size);
	if (--items<=0) goto label_ret; 
  }
  if (_tcsstr(OutBuffer, TEXT("$(Orbiter"))) {
	if (!Orbiter)
		ReplaceInString(OutBuffer, TEXT("$(Orbiter)"), gettext(TEXT("_@M894_")), Size);
	else
		ReplaceInString(OutBuffer, TEXT("$(Orbiter)"), gettext(TEXT("_@M491_")), Size);

	if (!EnableThermalLocator) invalid = true;
	if (--items<=0) goto label_ret; 
  }


  if (_tcsstr(OutBuffer, TEXT("$(FinalForceToggleActionName)"))) {
    CondReplaceInString(ForceFinalGlide, OutBuffer, 
                        TEXT("$(FinalForceToggleActionName)"), 
                        gettext(TEXT("_@M896_")), // Unforce
                        gettext(TEXT("_@M895_")), // Force
			Size);
    if (AutoForceFinalGlide) {
      invalid = true;
    }
	if (--items<=0) goto label_ret; // 100517
  }

  CondReplaceInString(MapWindow::zoom.AutoZoom(), OutBuffer, TEXT("$(ZoomAutoToggleActionName)"), gettext(TEXT("_@M418_")), gettext(TEXT("_@M897_")), Size);
  CondReplaceInString(EnableTopology, OutBuffer, TEXT("$(TopologyToggleActionName)"), gettext(TEXT("_@M491_")), gettext(TEXT("_@M894_")), Size);
  CondReplaceInString(EnableTerrain, OutBuffer, TEXT("$(TerrainToggleActionName)"), gettext(TEXT("_@M491_")), gettext(TEXT("_@M894_")), Size);

  if (_tcsstr(OutBuffer, TEXT("$(MapLabelsToggleActionName)"))) {
    switch(MapWindow::DeclutterLabels) {
    case MAPLABELS_ALLON:
		// LKTOKEN _@M1203_ "WPTS"
      ReplaceInString(OutBuffer, TEXT("$(MapLabelsToggleActionName)"), 
                      gettext(TEXT("_@M1203_")), Size);

      break;
    case MAPLABELS_ONLYWPS:
		// LKTOKEN _@M1204_ "TOPO"
      ReplaceInString(OutBuffer, TEXT("$(MapLabelsToggleActionName)"), 
                      gettext(TEXT("_@M1204_")), Size);
      break;
    case MAPLABELS_ONLYTOPO:
      ReplaceInString(OutBuffer, TEXT("$(MapLabelsToggleActionName)"), 
                      gettext(TEXT("_@M898_")), Size);
      break;
    case MAPLABELS_ALLOFF:
      ReplaceInString(OutBuffer, TEXT("$(MapLabelsToggleActionName)"), 
                      gettext(TEXT("_@M899_")), Size);
      break;
    }
	if (--items<=0) goto label_ret; // 100517
  }

  CondReplaceInString(CALCULATED_INFO.AutoMacCready != 0, OutBuffer, TEXT("$(MacCreadyToggleActionName)"), gettext(TEXT("_@M418_")), gettext(TEXT("_@M897_")), Size);
  {
  MapWindow::Mode::TModeFly userForcedMode = MapWindow::mode.UserForcedMode();
  CondReplaceInString(userForcedMode == MapWindow::Mode::MODE_FLY_CIRCLING, OutBuffer, TEXT("$(DispModeClimbShortIndicator)"), TEXT("_"), TEXT(""), Size);
  CondReplaceInString(userForcedMode == MapWindow::Mode::MODE_FLY_CRUISE, OutBuffer, TEXT("$(DispModeCruiseShortIndicator)"), TEXT("_"), TEXT(""), Size);
  CondReplaceInString(userForcedMode == MapWindow::Mode::MODE_FLY_NONE, OutBuffer, TEXT("$(DispModeAutoShortIndicator)"), TEXT("_"), TEXT(""), Size);
  CondReplaceInString(userForcedMode == MapWindow::Mode::MODE_FLY_FINAL_GLIDE, OutBuffer, TEXT("$(DispModeFinalShortIndicator)"), TEXT("_"), TEXT(""), Size);
  }

    CondReplaceInString(CALCULATED_INFO.AutoMacCready && AutoMcMode==amcFinalGlide, OutBuffer, TEXT("$(amcIsFinal)"), TEXT("_"), TEXT(""), Size);
    CondReplaceInString(CALCULATED_INFO.AutoMacCready && AutoMcMode==amcAverageClimb, OutBuffer, TEXT("$(amcIsClimb)"), TEXT("_"), TEXT(""), Size);
    CondReplaceInString(CALCULATED_INFO.AutoMacCready && AutoMcMode==amcEquivalent, OutBuffer, TEXT("$(amcIsEquiv)"), TEXT("_"), TEXT(""), Size);
    CondReplaceInString(CALCULATED_INFO.AutoMacCready, OutBuffer, TEXT("$(CheckManMc)"), TEXT(""),TEXT("_"), Size);


  if (_tcsstr(OutBuffer, TEXT("$(AirspaceMode)"))) {
    switch(AltitudeMode) {
    case 0:
	// LKTOKEN  _@M184_ = "Clip" 
      ReplaceInString(OutBuffer, TEXT("$(AirspaceMode)"), gettext(TEXT("_@M184_")), Size);
      break;
    case 1:
      ReplaceInString(OutBuffer, TEXT("$(AirspaceMode)"), gettext(TEXT("_@M897_")), Size); // Auto
      break;
    case 2:
	// LKTOKEN  _@M139_ = "Below" 
      ReplaceInString(OutBuffer, TEXT("$(AirspaceMode)"), gettext(TEXT("_@M139_")), Size);
      break;
    case 3:
	// LKTOKEN  _@M359_ = "Inside" 
      ReplaceInString(OutBuffer, TEXT("$(AirspaceMode)"), gettext(TEXT("_@M359_")), Size);
      break;
    case 4:
	// LKTOKEN  _@M75_ = "All OFF" 
      ReplaceInString(OutBuffer, TEXT("$(AirspaceMode)"), gettext(TEXT("_@M75_")), Size);
      break;
    case 5:
	// LKTOKEN  _@M76_ = "All ON" 
      ReplaceInString(OutBuffer, TEXT("$(AirspaceMode)"), gettext(TEXT("_@M76_")), Size);
      break;
    }
	if (--items<=0) goto label_ret; // 100517
  }

label_ret:

  return invalid;
}

