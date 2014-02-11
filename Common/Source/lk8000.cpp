/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: lk8000.cpp,v 1.1 2010/12/15 11:30:56 root Exp root $
*/
#include "externs.h"
#include "LKInterface.h"
#include "resource.h"
#include "Waypointparser.h"
#include "Logger.h"
#include "McReady.h"
#include "AirfieldDetails.h"
#include "Dialogs.h"
#include "Poco/NamedMutex.h"
#include "Terrain.h"

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
#include "devLXMiniMap.h"
#include "devLX16xx.h"
#include "devLXV7.h"
#include "devLXV7_EXP.h"
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
#include "devEye.h"
#include "devWesterboer.h"
#include "devFlyNet.h"
#include "InputEvents.h"
#include "Geoid.h"
#include "RasterTerrain.h"
#include "LiveTracker.h"

#include "LKObjects.h"
#include "Bitmaps.h"
#include "devCProbe.h"
#include "devBlueFlyVario.h"
#include "devLXV7easy.h"


#include "TraceThread.h"
#include "Poco/NamedEvent.h"

#ifdef INT_OVERFLOW
	#include <signal.h>
#endif

using std::min;
using std::max;



// Developers dedicates!
// Use rot13 under linux to code and decode strings
// char dedicated_by_{yourname}="....";
char dedicated_by_paolo[]="Qrqvpngrq gb zl sngure Ivggbevb";

extern HWND hWndMainWindow;

ATOM	MyRegisterClass (HINSTANCE, LPTSTR);
BOOL	InitInstance    (HINSTANCE, int);
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

extern void CreateCalculationThread();
extern void StartupLogFreeRamAndStorage();
extern void PreloadInitialisation(bool ask);
#ifdef PNA
extern bool LoadModelFromProfile(void);
#endif

#if (((UNDER_CE >= 300)||(_WIN32_WCE >= 0x0300)) && (WINDOWSPC<1))
#define HAVE_ACTIVATE_INFO
SHACTIVATEINFO s_sai;
bool api_has_SHHandleWMActivate = false;
bool api_has_SHHandleWMSettingChange = false;
#endif

void CleanupForShutdown(void);
Poco::NamedMutex Mutex("LOCK8000");

#ifdef INT_OVERFLOW
void handler(int /*signal*/) {
    LKASSERT(FALSE);
}
#endif

//
// WINMAIN RETURNS CODE ARE:
//
// -1	for init instance error
// -2	for mutex error return
// 
//  111 for normal program termination with automatic relaunch, if using lkrun
//  222 for normal program termination, with request to quit lkrun if running
// 
//  259 is reserved by OS (STILL_ACTIVE) status

#ifndef UNDER_CE
int WINAPI WinMain(     HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPSTR     lpCmdLine,
                        int       nCmdShow)
#else
int WINAPI WinMain(     HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPWSTR     lpCmdLine,
                        int       nCmdShow)
#endif
{   
#ifdef INT_OVERFLOW
  SetErrorMode(SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
  // when we get a SIGABRT, call handler
  signal(SIGABRT, &handler);
#endif
#if (WINDOWSPC>0)
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
#endif
#endif

	MSG msg = {0};
  (void)hPrevInstance;
  // use mutex to avoid multiple instances of lk8000 be running
  #if (!((WINDOWSPC>0) && TESTBENCH))
   if (!Mutex.tryLock()) {
	  return(-2);
  }
  #endif
  bool realexitforced=false;

  LKSound(_T("LK_CONNECT.WAV"));

  #if TRACETHREAD
  _THREADID_WINMAIN=GetCurrentThreadId();
  StartupStore(_T("##############  WINMAIN threadid=%d\n"),GetCurrentThreadId());
  #endif
  _stprintf(LK8000_Version,_T("%s v%s.%s "), _T(LKFORK), _T(LKVERSION),_T(LKRELEASE));
  _tcscat(LK8000_Version, TEXT(__DATE__));
  StartupStore(_T("------------------------------------------------------------%s"),NEWLINE);
  #ifdef PNA
  StartupStore(TEXT(". Starting %s %s%s"), LK8000_Version,_T("PNA"),NEWLINE);
  #else
  #if (WINDOWSPC>0)
  StartupStore(TEXT(". Starting %s %s%s"), LK8000_Version,_T("PC"),NEWLINE);
  #else
  StartupStore(TEXT(". Starting %s %s%s"), LK8000_Version,_T("PDA"),NEWLINE);
  #endif
  #endif

  #if TESTBENCH
    #ifdef _MSC_VER
      StartupStore(TEXT(". Built with MSVC ver : %d %s"), _MSC_VER, NEWLINE);
    #endif
    #ifdef __MINGW32__
      StartupStore(TEXT(". Built with mingw32 %d.%d (GCC %d.%d.%d) %s"), 
              __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION, 
              __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, 
              NEWLINE);
    #endif
  StartupStore(TEXT(". TESTBENCH option enabled%s"),NEWLINE);
  #endif
  Globals_Init();

  StartupLogFreeRamAndStorage();

  // PRELOAD ANYTHING HERE
  LKRunStartEnd(true);
  // END OF PRELOAD, PROGRAM GO!

  #ifdef PNA 
    // At this point we still havent loaded profile. Loading profile will also reload registry.
    // If registry was deleted in PNA, model type is not configured. It is configured in profile, but
    // it is too early here. So no ModelType .
    //
    // if we found no embedded name, try from registry
    if (  !SetModelType() ) {
        // last chance: try from default profile
        LoadModelFromProfile();
    }
  #endif
  
  bool datadir;
  datadir=CheckDataDir();

  if (!datadir) {
	// we cannot call startupstore, no place to store log!
	WarningHomeDir=true;
  }

  #if TESTBENCH
  TCHAR szPath[MAX_PATH] = {0};
  lk::filesystem::getExePath(szPath, MAX_PATH);
  StartupStore(_T(". Program execution path is <%s>\n"),szPath);
  StartupStore(_T(". Program data directory is <%s>\n"),LKGetLocalPath());
  #endif

  #if ( WINDOWSPC==0 )
  #if TESTBENCH
  StartupStore(TEXT(". Install/copy system objects in device memory%s"),NEWLINE);
  #endif
  short didsystem;
  didsystem=InstallSystem(); 
  goInstallSystem=true;
  #if TESTBENCH
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

  _stprintf(defaultProfileFile,_T("%s\\%s\\%s"),LKGetLocalPath(),_T(LKD_CONF),_T(LKPROFILE));
  _tcscpy(startProfileFile, defaultProfileFile);
  _stprintf(defaultAircraftFile,_T("%s\\%s\\%s"),LKGetLocalPath(),_T(LKD_CONF),_T(LKAIRCRAFT));
  _tcscpy(startAircraftFile, defaultAircraftFile);
  _stprintf(defaultPilotFile,_T("%s\\%s\\%s"),LKGetLocalPath(),_T(LKD_CONF),_T(LKPILOT));
  _tcscpy(startPilotFile, defaultPilotFile);
  _stprintf(defaultDeviceFile,_T("%s\\%s\\%s"),LKGetLocalPath(),_T(LKD_CONF),_T(LKDEVICE));
  _tcscpy(startDeviceFile, defaultDeviceFile);


  LK8000GetOpts();

  InitCommonControls();
  InitSineTable();

  // Perform application initialization: also ScreenGeometry and LKIBLSCALE, and Objects
  if (!InitInstance (hInstance, nCmdShow))
    {
	StartupStore(_T("++++++ InitInstance failed, program terminated!%s"),NEWLINE);
	return -1;
    }

  #ifdef HAVE_ACTIVATE_INFO
  SHSetAppKeyWndAssoc(VK_APP1, hWndMainWindow);
  SHSetAppKeyWndAssoc(VK_APP2, hWndMainWindow);
  SHSetAppKeyWndAssoc(VK_APP3, hWndMainWindow);
  SHSetAppKeyWndAssoc(VK_APP4, hWndMainWindow);
  SHSetAppKeyWndAssoc(VK_APP5, hWndMainWindow);
  SHSetAppKeyWndAssoc(VK_APP6, hWndMainWindow);
  #endif

  // Initialise main blackboard data

  memset( &(Task), 0, sizeof(Task_t));
  memset( &(StartPoints), 0, sizeof(Start_t));
  ClearTask();
  memset( &(GPS_INFO), 0, sizeof(GPS_INFO));
  memset( &(CALCULATED_INFO), 0,sizeof(CALCULATED_INFO));
  memset( &SnailTrail[0],0,TRAILSIZE*sizeof(SNAIL_POINT));
  #if LONGSNAIL
  memset( &LongSnailTrail[0],0,(LONGTRAILSIZE+1)*sizeof(LONG_SNAIL_POINT));
  #endif

  InitCalculations(&GPS_INFO,&CALCULATED_INFO);

  OpenGeoid();

  PreloadInitialisation(false); // calls dlgStartup
  if(RUN_MODE == RUN_EXIT || RUN_MODE == RUN_SHUTDOWN) {
	realexitforced=true;
	goto _Shutdown;
  }

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

  CalculateNewPolarCoef();
  #if TESTBENCH
  StartupStore(TEXT(". GlidePolar::SetBallast%s"),NEWLINE);
  #endif
  GlidePolar::SetBallast();


#ifdef PNA // VENTA-ADDON
    TCHAR sTmp[250];
	_stprintf(sTmp, TEXT("PNA MODEL=%s (%d)"), GlobalModelName, GlobalModelType);
	CreateProgressDialog(sTmp); 
#else
  TCHAR sTmpA[MAX_PATH], sTmpB[MAX_PATH];
  LocalPath(sTmpA,_T(""));
#if ( WINDOWSPC==0 )
  if ( !datadir ) {
	// LKTOKEN _@M1208_ "ERROR NO DIRECTORY:"
    CreateProgressDialog(gettext(TEXT("_@M1208_")));
    Poco::Thread::sleep(3000);
  }
#endif
  _stprintf(sTmpB, TEXT("Conf=%s"),sTmpA);
  CreateProgressDialog(sTmpB); 
#if ( WINDOWSPC==0 )
  if ( !datadir ) {
    Poco::Thread::sleep(3000);
    // LKTOKEN _@M1209_ "CHECK INSTALLATION!"
	CreateProgressDialog(gettext(TEXT("_@M1209_")));
    Poco::Thread::sleep(3000);
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
  StartupStore(_T(". LOADED %d WAYPOINTS + %d virtuals%s"),WayPointList.size()-NUMRESWP,NUMRESWP,NEWLINE);
  InitLDRotary(&rotaryLD); 
  InitWindRotary(&rotaryWind); // 100103
  MapWindow::zoom.Reset();
  InitLK8000();
  ReadAirfieldFile();
  SetHome(false);
  LKReadLanguageFile(szLanguageFile);
  LKLanguageReady=true;

  RasterTerrain::ServiceFullReload(GPS_INFO.Latitude, 
                                   GPS_INFO.Longitude);

  CAirspaceManager::Instance().ReadAirspaces();
  CAirspaceManager::Instance().SortAirspaces();
  OpenTopology();
  #if USETOPOMARKS
  TopologyInitialiseMarks();
  #endif

  CreateProgressDialog(MsgToken(1808));	// Loading FLARMNET database
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
  InternalRegister(); // must be second
  genRegister(); // must be three, since we Sort(3) in dlgConfiguration
  cai302Register();
  ewRegister();
  #if !110101
  atrRegister();
  vgaRegister();
  #endif
  CDevCAIGpsNav::Register();
  nmoRegister();
  pgRegister();
  b50Register();
  vlRegister();
  ewMicroRecorderRegister();
  DevLX::Register();
  DevLXMiniMap::Register();
  DevLX16xx::Register();
  DevLXV7::Register();
  DevLXV7_EXP::Register();
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
  CDevEye::Register();
  FlytecRegister();
  LK8EX1Register();
  WesterboerRegister();
  FlyNetRegister();
  CDevCProbe::Register();
  BlueFlyRegister();
  LXV7easyRegister();
  // REPETITION REMINDER ..
  // IMPORTANT: ADD NEW ONES TO BOTTOM OF THIS LIST
  // >>> Please check that the number of devices is not exceeding NUMREGDEV in device.h <<<
  // or you get an assertion error in device.cpp 

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

  LiveTrackerInit();

  extern void InitFlightDataRecorder(void);
  InitFlightDataRecorder();

  // re-set polar in case devices need the data
  #if TESTBENCH
  StartupStore(TEXT(".... GlidePolar::SetBallast%s"),NEWLINE);
  #endif
  GlidePolar::SetBallast();

  // LKTOKEN _@M1218_ "Initialising display"
  CreateProgressDialog(gettext(TEXT("_@M1218_")));

  // just about done....

  DoSunEphemeris(GPS_INFO.Longitude, GPS_INFO.Latitude);

  // Finally ready to go
  #if TESTBENCH
  StartupStore(TEXT(".... WinMain CreateDrawingThread%s"),NEWLINE);
  #endif
  MapWindow::CreateDrawingThread();
  Poco::Thread::sleep(50);

  SwitchToMapWindow();
  #if TESTBENCH
  StartupStore(TEXT(".... CreateCalculationThread%s"),NEWLINE);
  #endif
  CreateCalculationThread();
  while(!(goCalculationThread)) Poco::Thread::sleep(50);

  // find unique ID of this PDA or create a new one
  // Currently disabled in LK, we use a dummy ID
  // CreateAssetNumber();

  #if TESTBENCH
  StartupStore(TEXT(".... ProgramStarted=InitDone%s"),NEWLINE);
  #endif
  ProgramStarted = psInitDone;

  GlobalRunning = true;

#if _DEBUG
 // _crtBreakAlloc = -1;     // Set this to the number in {} brackets to
                           // break on a memory leak
#endif

 //
 // Main message loop
 //

  #if (1)
  // Simple approach
  BOOL bRet;
  while ( (bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
	LKASSERT(bRet!=-1);
	TranslateMessage(&msg);
	DispatchMessage(&msg);
  }
  #else
  // This is an alternate approach.
  bool bQuit=false;
  do {
	while ( PeekMessage(&msg, NULL, 0, 0,PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT) bQuit=true;
	}
	if (bQuit) break;
  } while(1);
  #endif

_Shutdown:
  CleanupForShutdown();
  #if TESTBENCH
  StartupStore(_T(".... WinMain terminated, wParamreturn code=%d realexitforced=%d%s"),msg.wParam,realexitforced,NEWLINE);
  #endif

  #if (WINDOWSPC>0)
  #if _DEBUG
  _CrtCheckMemory();
  //  _CrtDumpMemoryLeaks(); generate False positive, use _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF); instead
  #endif
  #endif

  if (realexitforced) return 222;
  else return 111;
}

void CleanupForShutdown(void) {

  LKObjects_Delete();
  LKUnloadProfileBitmaps();
  LKUnloadFixedBitmaps();

  LKUnloadMessage();
  InputEvents::UnloadString();
  // This is freeing char *slot in TextInBox
  MapWindow::FreeSlot();
  
  Mutex.unlock();
}

