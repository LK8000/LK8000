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
#include "Modeltype.h"
#include "Port.h"
#include "Waypointparser.h"
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
#include "Geoid.h"
#ifdef PNA
#include "LKHolux.h"
#endif
#include "RGB.h"

#include "RasterTerrain.h"

// extern void LKObjects_Create(); //GOEXT
// extern void LKObjects_Delete(); //GOEXT REMOVE
#include "LKObjects.h"

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

extern void StartupLogFreeRamAndStorage();	 // GOEXT

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

  extern void InitCriticalSections(void);
  InitCriticalSections();

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

  extern void InitialiseFonts(RECT rc);
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
  
  //  CommandBar_Destroy(hWndCB); // REMOVE

  // Kill graphics objects

  DeleteObject(hBrushSelected);
  DeleteObject(hBrushUnselected);
  DeleteObject(hBrushButton);

  extern void DeInitialiseFonts(void);
  DeInitialiseFonts();  
/* REMOVE
  DeleteObject(TitleWindowFont);
  DeleteObject(CDIWindowFont);
  DeleteObject(MapLabelFont);
  DeleteObject(MapWindowFont);
  DeleteObject(MapWindowBoldFont);
  DeleteObject(StatisticsFont);  
*/
  CAirspaceManager::Instance().CloseAirspaces();
  StartupStore(TEXT(". Delete Critical Sections%s"),NEWLINE);

  extern void DeInitCriticalSections(void);
  DeInitCriticalSections(); 

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

