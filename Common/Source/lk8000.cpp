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
#include "Dialogs/dlgProgress.h"
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
#include "devPVCOM.h"
#include "devCondor.h"
#include "devIlec.h"
#include "devDSX.h"
#include "devIMI.h"
#include "devEye.h"
#include "devWesterboer.h"
#include "devFlyNet.h"
#include "devKRT2.h"
#include "devLXNano3.h"
#include "devXCTracer.h"
#include "devGPSBip.h"
#include "InputEvents.h"
#include "Geoid.h"
#include "RasterTerrain.h"
#include "LiveTracker.h"

#include "LKObjects.h"
#include "Bitmaps.h"
#include "devCProbe.h"
#include "devBlueFlyVario.h"
#include "devLXV7easy.h"
#include "ComCheck.h"


#include "TraceThread.h"
#include "Poco/NamedEvent.h"

#include "FlightDataRec.h"

#include "Screen/Init.hpp"
#include "Message.h"
#include "Sound/Sound.h"

#include "Kobo/System.hpp"
#include "Kobo/Kernel.hpp"
#include "Hardware/CPU.hpp"
#include "LKInterface/CScreenOrientation.h"
#include <time.h>

#ifdef __linux__
#include <sys/utsname.h>
#endif

#ifdef INT_OVERFLOW
	#include <signal.h>
#endif

using std::min;
using std::max;

#define MSGDELAY  200  // delay after printing each progress dialog info line
#define ERRDELAY  3000  // delay after printing each progress dialog ERROR line


// Developers dedicates!
// Use rot13 under linux to code and decode strings
// char dedicated_by_{yourname}="....";
char dedicated_by_paolo[]="Qrqvpngrq gb zl sngure Ivggbevb";

BOOL	InitInstance    ();

extern void CreateCalculationThread();
extern void StartupLogFreeRamAndStorage();
extern void PreloadInitialisation(bool ask);
#ifdef PNA
extern bool LoadModelFromProfile(void);
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
#ifdef WIN32
HINSTANCE _hInstance;

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
    (void)hPrevInstance;
    
    _hInstance = hInstance; // this need to be first, always !
    #if (WINDOWSPC >0)
    const TCHAR* szCmdLine = GetCommandLine();
    #endif
    
#else
int main(int argc, char *argv[]) {
    
#endif

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


  // use mutex to avoid multiple instances of lk8000 be running
  #if (!((WINDOWSPC>0) && TESTBENCH))
   if (!Mutex.tryLock()) {
	  return(-2);
  }
  #endif

  #if TRACETHREAD
  _THREADID_WINMAIN=GetCurrentThreadId();
  StartupStore(_T("##############  WINMAIN threadid=%d\n"),GetCurrentThreadId());
  #endif
  _stprintf(LK8000_Version,_T("%s v%s.%s "), _T(LKFORK), _T(LKVERSION),_T(LKRELEASE));
  _tcscat(LK8000_Version, TEXT(__DATE__));
  StartupStore(_T("------------------------------------------------------------%s"),NEWLINE);
  #ifdef __linux__
  #ifdef KOBO
  StartupStore(TEXT(". Starting %s %s%s"), LK8000_Version,_T("KOBO"),NEWLINE);
  #else
  StartupStore(TEXT(". Starting %s %s%s"), LK8000_Version,_T("LINUX"),NEWLINE);
  #endif
 
  struct utsname sysinfo = {};
  if(uname(&sysinfo) == 0) {
    StartupStore(". System Name:    %s %s" NEWLINE, sysinfo.sysname, sysinfo.nodename);
    StartupStore(". Kernel Version: %s" NEWLINE, sysinfo.release);
    StartupStore(". Kernel Build:   %s" NEWLINE, sysinfo.version);
    StartupStore(". Machine Arch:   %s" NEWLINE, sysinfo.machine);
  }
  
  #else
  #ifdef PNA
  StartupStore(TEXT(". [%09u] Starting %s %s%s"),(unsigned int)GetTickCount(),LK8000_Version,_T("PNA"),NEWLINE);
  #else
  #if (WINDOWSPC>0)
  StartupStore(TEXT(". [%09u] Starting %s %s%s"),(unsigned int)GetTickCount(),LK8000_Version,_T("PC"),NEWLINE);
  #else
  StartupStore(TEXT(". [%09u] Starting %s %s%s"),(unsigned int)GetTickCount(),LK8000_Version,_T("PDA"),NEWLINE);
  #endif
  #endif
  #endif

  #if TESTBENCH
    #ifdef _MSC_VER
      StartupStore(TEXT(". Built with MSVC ver : %d %s"), _MSC_VER, NEWLINE);
    #endif
    #ifdef __GNUC__
        #ifdef __MINGW32__
          StartupStore(TEXT(". Built with mingw32 %d.%d (GCC %d.%d.%d) %s"), 
                  __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION, 
                  __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, 
                  NEWLINE);
        #else
          StartupStore(TEXT(". Built with GCC %d.%d.%d %s"), 
                  __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, 
                  NEWLINE);

        #endif
    #endif

    StartupStore(_T(". Compiler options:%s"),NEWLINE);
    #ifdef ENABLE_OPENGL
    StartupStore(_T("    + ENABLE_OPENGL%s"),NEWLINE);
    #endif
    #ifdef HAVE_GLES
    StartupStore(_T("    + HAVE_GLES%s"),NEWLINE);
    #endif
    #ifdef USE_WAYLAND
    StartupStore(_T("    + USE_WAYLAND%s"),NEWLINE);
    #endif
    #ifdef USE_X11
    StartupStore(_T("    + USE_X11%s"),NEWLINE);
    #endif
    #ifdef USE_CONSOLE
    StartupStore(_T("    + USE_CONSOLE%s"),NEWLINE);
    #endif
    #ifdef ENABLE_SDL
    StartupStore(_T("    + ENABLE_SDL%s"),NEWLINE);
    #endif
    #ifdef USE_EGL
    StartupStore(_T("    + USE_EGL%s"),NEWLINE);
    #endif
    #ifdef USE_FB
    StartupStore(_T("    + USE_FB%s"),NEWLINE);
    #endif
    #ifdef USE_MEMORY_CANVAS
    StartupStore(_T("    + USE_MEMORY_CANVAS%s"),NEWLINE);
    #endif
    #ifdef GREYSCALE
    StartupStore(_T("    + GREYSCALE%s"),NEWLINE);
    #endif
    #ifdef DITHER
    StartupStore(_T("    + DITHER%s"),NEWLINE);
    #endif
    #ifdef USE_ALSA
    StartupStore(_T("    + USE_ALSA%s"),NEWLINE);
    #endif
    #ifdef USE_FREETYPE
    StartupStore(_T("    + USE_FREETYPE%s"),NEWLINE);
    #endif
    #ifdef USE_FULLSCREEN
    StartupStore(_T("    + USE_FULLSCREEN%s"),NEWLINE);
    #endif
    #ifdef HC_MALLOC
    StartupStore(_T("    + HC_MALLOC%s"),NEWLINE);
    #endif
    #ifdef POCO_STATIC
    StartupStore(_T("    + POCO_STATIC%s"),NEWLINE);
    #endif
    #ifdef INT_OVERFLOW
    StartupStore(_T("    + INT_OVERFLOW%s"),NEWLINE);
    #endif

    StartupStore(TEXT(". TESTBENCH option enabled%s"),NEWLINE);
  #endif

  // WE NEED TO KNOW IN RUNTIME WHEN THESE OPTIONS ARE ENABLED, EVEN WITH NO TESTBENCH!
  #ifndef NDEBUG
  StartupStore(TEXT(". DEBUG enabled in makefile%s"),NEWLINE);
  #endif
  #if YDEBUG
  StartupStore(TEXT(". YDEBUG option enabled%s"),NEWLINE);
  #endif
  #if BUGSTOP
  StartupStore(TEXT(". BUGSTOP option enabled%s"),NEWLINE);
  #endif
  #if USELKASSERT
  StartupStore(TEXT(". USELKASSERT option enabled%s"),NEWLINE);
  #endif

  ScreenGlobalInit InitScreen;
  SoundGlobalInit InitSound;

  std::unique_ptr<CScreenOrientation> pSaveScreen(new CScreenOrientation(LKGetLocalPath()));

  
  // This is needed otherwise LKSound will be silent until we init Globals.
  EnableSoundModes=true;
 
  bool realexitforced=false;

  LKSound(_T("LK_CONNECT.WAV"));

  Globals_Init();

  StartupLogFreeRamAndStorage();

  lscpu_init();
  if (HaveSystemInfo) {
      StartupStore(_T(". Host: %s %s%s"),
          SystemInfo_Architecture()==NULL?_T("Unknown"):SystemInfo_Architecture(),
          SystemInfo_Vendor()==NULL?_T(""):SystemInfo_Vendor(),
          NEWLINE);
      StartupStore(_T(". CPUs: #%d running at %d Mhz, %d bogoMips%s"),SystemInfo_Cpus(),
          SystemInfo_Mhz(), SystemInfo_Bogomips(), NEWLINE);
  } else {
      StartupStore(_T(". Host and Cpu informations not available%s"),NEWLINE);
  }

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
  StartupStore(_T(". Program execution path is <%s>") NEWLINE,szPath);
  StartupStore(_T(". Program data directory is <%s>") NEWLINE,LKGetLocalPath());
  #endif

  InstallSystem(); 

  // These directories are needed if missing, as LK can run also with no maps and no waypoints..
  CreateDirectoryIfAbsent(TEXT(LKD_LOGS));
  CreateDirectoryIfAbsent(TEXT(LKD_CONF));
  CreateDirectoryIfAbsent(TEXT(LKD_TASKS));
  CreateDirectoryIfAbsent(TEXT(LKD_MAPS));
  CreateDirectoryIfAbsent(TEXT(LKD_WAYPOINTS));

  _stprintf(defaultProfileFile,_T("%s%s%s%s"),LKGetLocalPath(), _T(LKD_CONF), _T(DIRSEP), _T(LKPROFILE));
  _tcscpy(startProfileFile, defaultProfileFile);
  _stprintf(defaultAircraftFile,_T("%s%s%s%s"),LKGetLocalPath(), _T(LKD_CONF), _T(DIRSEP), _T(LKAIRCRAFT));
  _tcscpy(startAircraftFile, defaultAircraftFile);
  _stprintf(defaultPilotFile,_T("%s%s%s%s"),LKGetLocalPath(), _T(LKD_CONF), _T(DIRSEP), _T(LKPILOT));
  _tcscpy(startPilotFile, defaultPilotFile);
  _stprintf(defaultDeviceFile,_T("%s%s%s%s"),LKGetLocalPath(), _T(LKD_CONF), _T(DIRSEP), _T(LKDEVICE));
  _tcscpy(startDeviceFile, defaultDeviceFile);

  #ifdef __linux__
  extern void LKCmdLineArguments(int argc, char *argv[]);
  LKCmdLineArguments(argc,argv);
  #else
  #if (WINDOWSPC >0)
  LK8000GetOpts(szCmdLine);
  #endif
  #endif
  InitSineTable();

  // Perform application initialization: also ScreenGeometry and LKIBLSCALE, and Objects
  if (!InitInstance ())
    {
	StartupStore(_T("++++++ InitInstance failed, program terminated!%s"),NEWLINE);
	return -1;
    }
  
#ifdef RADIO_ACTIVE
  memset( &(RadioPara), 0, sizeof(Radio_t));
  RadioPara.Volume = 6;
  RadioPara.Squelch = 3;
  RadioPara.Vox = 5;
   RadioPara.Enabled = false; //devIsRadio(devA()) || devIsRadio(devB());
#endif  // RADIO_ACTIVE        
  
  // Initialise main blackboard data

  memset( &(Task), 0, sizeof(Task_t));
  memset( &(StartPoints), 0, sizeof(Start_t));
  ClearTask();
  memset( &(GPS_INFO), 0, sizeof(GPS_INFO));
  memset( &(CALCULATED_INFO), 0,sizeof(CALCULATED_INFO));
  memset( &SnailTrail[0],0,TRAILSIZE*sizeof(SNAIL_POINT));
  memset( &LongSnailTrail[0],0,(LONGTRAILSIZE+1)*sizeof(LONG_SNAIL_POINT));

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

  time_t  linux_time;
  linux_time = time(0);
  struct tm *pda_time;
  pda_time = gmtime(&linux_time);
  GPS_INFO.Time  = pda_time->tm_hour*3600+pda_time->tm_min*60+pda_time->tm_sec;
  GPS_INFO.Year  = pda_time->tm_year + 1900;
  GPS_INFO.Month = pda_time->tm_mon + 1;
  GPS_INFO.Day = pda_time->tm_mday;
  GPS_INFO.Hour  = pda_time->tm_hour;
  GPS_INFO.Minute = pda_time->tm_min;
  GPS_INFO.Second = pda_time->tm_sec;  

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
    Poco::Thread::sleep(ERRDELAY);
  }
#endif
  _stprintf(sTmpB, TEXT("Conf=%s"),sTmpA);
  CreateProgressDialog(sTmpB); 
#if ( WINDOWSPC==0 )
  if ( !datadir ) {
    Poco::Thread::sleep(ERRDELAY);
    // LKTOKEN _@M1209_ "CHECK INSTALLATION!"
	CreateProgressDialog(gettext(TEXT("_@M1209_")));
    Poco::Thread::sleep(ERRDELAY);
  }
#endif
#endif // non PNA

// TODO until startup graphics are settled, no need to delay PC start
    if (AircraftCategory == (AircraftCategory_t) umParaglider) {
        // LKTOKEN _@M1210_ "PARAGLIDING MODE"
        CreateProgressDialog(gettext(TEXT("_@M1210_")));
        Poco::Thread::sleep(MSGDELAY);
    }
    if (SIMMODE) {
        // LKTOKEN _@M1211_ "SIMULATION"
        CreateProgressDialog(gettext(TEXT("_@M1211_")));
        Poco::Thread::sleep(MSGDELAY);
    }

#ifdef PNA
  if ( SetBacklight() == true ) 
	// LKTOKEN _@M1212_ "AUTOMATIC BACKLIGHT CONTROL"
	CreateProgressDialog(gettext(TEXT("_@M1212_")));
  else
	// LKTOKEN _@M1213_ "NO BACKLIGHT CONTROL"
	CreateProgressDialog(gettext(TEXT("_@M1213_")));
#endif
  
  // this should work ok for all pdas as well
  if ( SetSoundVolume() == true ) 
	// LKTOKEN _@M1214_ "AUTOMATIC SOUND LEVEL CONTROL"
	CreateProgressDialog(gettext(TEXT("_@M1214_")));
  else
	// LKTOKENS _@M1215_ "NO SOUND LEVEL CONTROL"
	CreateProgressDialog(gettext(TEXT("_@M1215_")));


  RasterTerrain::OpenTerrain();

  ReadWayPoints();
  StartupStore(_T(". LOADED %d WAYPOINTS + %u virtuals%s"),(unsigned)WayPointList.size()-NUMRESWP,NUMRESWP,NEWLINE);
  InitLDRotary(&rotaryLD); 
  InitWindRotary(&rotaryWind); // 100103
  MapWindow::zoom.Reset();
  InitLK8000();
  ReadAirfieldFile();
  SetHome(false);
  LKReadLanguageFile(szLanguageFile);
  LKLanguageReady=true;

  CreateProgressDialog(gettext(TEXT("_@M399_")));
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
  flymasterGPSRegister();
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
  DevLXNanoIII::Register();  
  XCTracerRegister();
  GPSBipRegister ();
#ifdef RADIO_ACTIVE  
  PVCOMRegister();
  KRT2Register();
#endif  // RADIO_ACTIVE        
    
  // REPETITION REMINDER ..
  // IMPORTANT: ADD NEW ONES TO BOTTOM OF THIS LIST
  // >>> Please check that the number of devices is not exceeding NUMREGDEV in device.h <<<
  // or you get an assertion error in device.cpp 

  ComCheck_Init();

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

#ifndef NO_DATARECORDER
  InitFlightDataRecorder();
#endif

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


    if (WarningHomeDir) {
        TCHAR nopath[MAX_PATH];
        LocalPath(nopath, _T(""));
        // LKTOKEN _@M1209_ "CHECK INSTALLATION!"
        MessageBoxX(nopath, gettext(TEXT("_@M1209_")), mbOk);
        WarningHomeDir = false;
    }
#ifdef UNDER_CE
    static bool checktickcountbug = true; // 100510
    if (checktickcountbug) {
        DWORD counts = GetTickCount();
        if (counts > (unsigned) 2073600000l) {
            // LKTOKEN  _@M527_ = "Please exit LK8000 and reset your device.\n"
            MessageBoxX(gettext(TEXT("_@M527_")),
                    TEXT("Device need reset!"),
                    mbOk);
        }
        checktickcountbug = false;
    }
#endif
    if (!ISPARAGLIDER && !ISCAR) { // 100925
        if (SAFETYALTITUDEARRIVAL < 500) { // SAFETY is *10, so we check <50 really
            // LKTOKEN  _@M155_ = "CHECK safety arrival altitude\n"
            MessageBoxX(gettext(TEXT("_@M155_")),
                    TEXT("Warning!"),
                    mbOk);
        }
    }

 //
 // Main message loop
 //
  MainWindow.RunModalLoop();

_Shutdown:
  CleanupForShutdown();
  #if TESTBENCH
  StartupStore(_T(".... WinMain terminated, realexitforced=%d%s"),realexitforced,NEWLINE);
  #endif

  #if (WINDOWSPC>0)
  #if _DEBUG
  _CrtCheckMemory();
  //  _CrtDumpMemoryLeaks(); generate False positive, use _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF); instead
  #endif
  #endif

  pSaveScreen = nullptr;
#ifdef KOBO
  extern bool RestartToNickel;
  if(RestartToNickel) {
    KoboExecNickel();
  } else {
#ifdef NDEBUG
    /* in case of crash, device will reboot on nickel */
    lk::filesystem::deleteFile("/mnt/onboard/LK8000/kobo/start_nickel");
    KoboPowerOff();
#endif
  }
#endif  
  
  if (realexitforced) return 222;
  else return 111;
}

extern void DeInitLKFonts(void);

void CleanupForShutdown(void) {

  MainWindow.Destroy();
  Message::Destroy();

  DeInitLKFonts();  
  LKObjects_Delete();
  LKUnloadProfileBitmaps();
  LKUnloadFixedBitmaps();

  LKUnloadMessage();
  InputEvents::UnloadString();
  // This is freeing char *slot in TextInBox
  MapWindow::FreeSlot();
  
  Mutex.unlock();
}

