/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: lk8000.cpp,v 1.1 2010/12/15 11:30:56 root Exp root $
*/

#include "options.h"

#ifndef DOCTEST_CONFIG_DISABLE
#  define DOCTEST_CONFIG_IMPLEMENT
#  include <doctest/doctest.h>
#endif

#include "externs.h"
#include "LKInterface.h"
#include "resource.h"
#include "Waypointparser.h"
#include "Logger.h"
#include "McReady.h"
#include "AirfieldDetails.h"
#include "Dialogs/dlgProgress.h"
#include "Dialogs.h"
#include "Terrain.h"

#include "InputEvents.h"
#include "Geoid.h"
#include "RasterTerrain.h"

#include "LKObjects.h"
#include "Bitmaps.h"
#include "ComCheck.h"

#include "FlightDataRec.h"

#include "Screen/Init.hpp"
#include "Message.h"
#include "Sound/Sound.h"

#include "Kobo/System.hpp"
#include "Kobo/Kernel.hpp"
#include "Hardware/CPU.hpp"
#include "LKInterface/CScreenOrientation.h"
#include <time.h>

#include "Airspace/Sonar.h"
#include <OS/RotateScreen.h>
#include <dlgFlarmIGCDownload.h>
#include <memory>
#include "Calc/Vario.h"
#include "IO/Async/GlobalIOThread.hpp"
#include "Tracking/Tracking.h"
#include "Waypoints/SetHome.h"
#include "Baro.h"
#include "OS/Sleep.h"
#include "Comm/ExternalWind.h"
#include "LocalPath.h"
#include "Calc/LDRotaryBuffer.h"
#include "Thread/NamedMutex.hpp"
#include <mutex>

#ifdef __linux__
#include <sys/utsname.h>
#endif

#ifdef ANDROID
#include "Android/LK8000Activity.h"
#include "Android/NativeView.hpp"
#include "Android/Main.hpp"
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
extern void PreloadInitialisation(bool ask);
extern bool LKProfileLoad(const TCHAR *szFile);


static bool realexitforced=false;

void DetectKeyboardModel() {
#ifdef ANDROID
  LK8000Activity *activity = LK8000Activity::Get();
  if (activity) {
    activity->DetectKeyboardModel();
  }
#endif
}


bool Startup(const TCHAR* szCmdLine) {

  lk::strcpy(LK8000_Version, _T(LKFORK " v" LKVERSION "." LKRELEASE " " __DATE__));
  StartupStore(_T("------------------------------------------------------------"));
#ifdef KOBO
  StartupStore(TEXT(". Starting %s %s"), LK8000_Version,_T("KOBO"));
#elif defined(__linux__)
  StartupStore(TEXT(". Starting %s %s"), LK8000_Version,_T("LINUX"));

  struct utsname sysinfo = {};
  if(uname(&sysinfo) == 0) {
    StartupStore(". System Name:    %s %s" NEWLINE, sysinfo.sysname, sysinfo.nodename);
    StartupStore(". Kernel Version: %s" NEWLINE, sysinfo.release);
    StartupStore(". Kernel Build:   %s" NEWLINE, sysinfo.version);
    StartupStore(". Machine Arch:   %s" NEWLINE, sysinfo.machine);
  }
#ifdef ANDROID
  StartupStore(". Android Product:%s",  native_view->GetProduct());
#endif

#elif defined(WIN32)
  StartupStore(TEXT(". Starting %s %s"),LK8000_Version,_T("PC"));
#endif

  DebugLog(_T(". DebugLog Enabled"));
  TestLog(_T(". TestLog Enabled"));

#ifdef __clang_version__
  #ifdef __MINGW32__
    StartupStore(_T(". Built with llvm-mingw32 ") __clang_version__);  
  #else
    StartupStore(_T(". Built with clang ") __clang_version__);
  #endif
#elif defined(__GNUC__)
  #ifdef __MINGW32__
    StartupStore(_T(". Built with mingw32 %d.%d (GCC %d.%d.%d)"),
            __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION,
            __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
  #else
    StartupStore(_T(". Built with GCC %d.%d.%d"),
            __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
  #endif
#endif

  TestLog(_T(". Compiler options:"));
#ifdef ENABLE_OPENGL
  TestLog(_T("    + ENABLE_OPENGL"));
#endif
#ifdef HAVE_GLES
  TestLog(_T("    + HAVE_GLES"));
#endif
#ifdef USE_WAYLAND
  TestLog(_T("    + USE_WAYLAND"));
#endif
#ifdef USE_X11
  TestLog(_T("    + USE_X11"));
#endif
#ifdef USE_CONSOLE
  TestLog(_T("    + USE_CONSOLE"));
#endif
#ifdef ENABLE_SDL
  TestLog(_T("    + ENABLE_SDL"));
#endif
#ifdef USE_EGL
  TestLog(_T("    + USE_EGL"));
#endif
#ifdef USE_FB
  TestLog(_T("    + USE_FB"));
#endif
#ifdef USE_MEMORY_CANVAS
  TestLog(_T("    + USE_MEMORY_CANVAS"));
#endif
#ifdef GREYSCALE
  TestLog(_T("    + GREYSCALE"));
#endif
#ifdef DITHER
  TestLog(_T("    + DITHER"));
#endif
#ifdef USE_ALSA
  TestLog(_T("    + USE_ALSA"));
#endif
#ifdef USE_FREETYPE
  TestLog(_T("    + USE_FREETYPE"));
#endif
#ifdef USE_FULLSCREEN
  TestLog(_T("    + USE_FULLSCREEN"));
#endif
#ifdef POCO_STATIC
  TestLog(_T("    + POCO_STATIC"));
#endif
#ifdef NO_DASH_LINES
  TestLog(_T("    + NO_DASH_LINES"));
#endif

  TestLog(TEXT(". TESTBENCH option enabled"));

  // WE NEED TO KNOW IN RUNTIME WHEN THESE OPTIONS ARE ENABLED, EVEN WITH NO TESTBENCH!
#ifndef NDEBUG
  StartupStore(TEXT(". DEBUG enabled in makefile"));
#endif
#if YDEBUG
  StartupStore(TEXT(". YDEBUG option enabled"));
#endif
#if BUGSTOP
  StartupStore(TEXT(". BUGSTOP option enabled"));
#endif
#if USELKASSERT
  StartupStore(TEXT(". USELKASSERT option enabled"));
#endif


  Globals_Init();

  StartupLogFreeRamAndStorage();

#ifdef PRELOAD_SOUND_SWITCH
  LocalPath(defaultProfileFile, _T(LKD_CONF), _T(LKPROFILE));
  LKProfileLoad(defaultProfileFile);
#endif  
  LKSound(_T("LK_CONNECT.WAV"));

  lscpu_init();
  if (HaveSystemInfo) {
      StartupStore(_T(". Host: %s %s"),
          SystemInfo_Architecture()==NULL?_T("Unknown"):SystemInfo_Architecture(),
          SystemInfo_Vendor()==NULL?_T(""):SystemInfo_Vendor());
      StartupStore(_T(". CPUs: #%d running at %d Mhz, %d bogoMips"),SystemInfo_Cpus(),
          SystemInfo_Mhz(), SystemInfo_Bogomips());
  } else {
      StartupStore(_T(". Host and Cpu informations not available"));
  }

  // PRELOAD ANYTHING HERE
  LKRunStartEnd(true);
  // END OF PRELOAD, PROGRAM GO!

#ifndef ANDROID
  bool datadir = CheckDataDir();
  if (!datadir) {
    // we cannot call startupstore, no place to store log!
    WarningHomeDir=true;
  }
#endif

  TCHAR szPath[MAX_PATH] = {0};
  lk::filesystem::getExePath(szPath, MAX_PATH);
  StartupStore(_T(". Program execution path :   <%s>"), szPath);
  StartupStore(_T(". Program system directory : <%s>"), LKGetSystemPath());
  StartupStore(_T(". Program data directory :   <%s>"), LKGetLocalPath());

  LocalPath(defaultProfileFile, _T(LKD_CONF), _T(LKPROFILE));
  lk::strcpy(startProfileFile, defaultProfileFile);
  LocalPath(defaultAircraftFile,_T(LKD_CONF), _T(LKAIRCRAFT));
  lk::strcpy(startAircraftFile, defaultAircraftFile);
  LocalPath(defaultPilotFile,_T(LKD_CONF), _T(LKPILOT));
  lk::strcpy(startPilotFile, defaultPilotFile);
  LocalPath(defaultDeviceFile,_T(LKD_CONF), _T(LKDEVICE));
  lk::strcpy(startDeviceFile, defaultDeviceFile);

#if !defined(ANDROID)
  if (!LK8000GetOpts(szCmdLine)) return 0; 
#endif

  InitSineTable();

  main_window = std::make_unique<WndMain>();

  // Perform application initialization: also ScreenGeometry and LKIBLSCALE, and Objects
  if (!InitInstance ()) {
    StartupStore(_T("++++++ InitInstance failed, program terminated!"));
    return -1;
  }

#ifdef ANDROID
  LK8000Activity* activity = LK8000Activity::Get();
  assert(activity);
  if(activity) {
    main_window->OnStartEventLoop();
    activity->RequestPermission();
    activity->WaitPermission();
    main_window->OnStopEventLoop();
  }
#endif

  // These directories are needed if missing, as LK can run also with no maps and no waypoints..
  CreateDirectoryIfAbsent(TEXT(LKD_LOGS));
  CreateDirectoryIfAbsent(TEXT(LKD_CONF));
  CreateDirectoryIfAbsent(TEXT(LKD_TASKS));
  CreateDirectoryIfAbsent(TEXT(LKD_MAPS));
  CreateDirectoryIfAbsent(TEXT(LKD_WAYPOINTS));
  CreateDirectoryIfAbsent(TEXT(LKD_AIRSPACES));
  CreateDirectoryIfAbsent(TEXT(LKD_POLARS));

  memset( &(RadioPara), 0, sizeof(Radio_t));
  RadioPara.Volume = 6;
  RadioPara.Squelch = 3;
  RadioPara.Vox = 5;
  RadioPara.Enabled = false; //devIsRadio(devA()) || devIsRadio(devB());
  RadioPara.ActiveKhz  = 118000;
  RadioPara.PassiveKhz = 118000;
  RadioPara.Enabled8_33      = true;

  // Initialise main blackboard data

  memset( &(Task), 0, sizeof(Task_t));
  memset( &(StartPoints), 0, sizeof(Start_t));
  ClearTask();

  GPS_INFO = {};
  CALCULATED_INFO = {};

  memset( SnailTrail, 0, sizeof(SnailTrail));
  memset( LongSnailTrail, 0, sizeof(LongSnailTrail));

  InitCalculations(&GPS_INFO,&CALCULATED_INFO);

  OpenGeoid();
  ScopeLockScreen LockSreen;

  PreloadInitialisation(false); // calls dlgStartup
  if(RUN_MODE == RUN_EXIT || RUN_MODE == RUN_SHUTDOWN) {
    realexitforced=true;
    return false;
  }

  GPS_INFO.NAVWarning = true; // default, no gps at all!

  time_t  linux_time;
  linux_time = time(0);
  tm utc_tm = {};
  struct tm *pda_time;
  pda_time = gmtime_r(&linux_time, &utc_tm);
  GPS_INFO.Time  = pda_time->tm_hour*3600+pda_time->tm_min*60+pda_time->tm_sec;
  GPS_INFO.Year  = pda_time->tm_year + 1900;
  GPS_INFO.Month = pda_time->tm_mon + 1;
  GPS_INFO.Day = pda_time->tm_mday;
  GPS_INFO.Hour  = pda_time->tm_hour;
  GPS_INFO.Minute = pda_time->tm_min;
  GPS_INFO.Second = pda_time->tm_sec;

  ReadWinPilotPolar();

// TODO until startup graphics are settled, no need to delay PC start
    if (AircraftCategory == AircraftCategory_t::umParaglider) {
        // LKTOKEN _@M1210_ "PARAGLIDING MODE"
        CreateProgressDialog(MsgToken<1210>());
        Sleep(MSGDELAY);
    }
    if (SIMMODE) {
        // LKTOKEN _@M1211_ "SIMULATION"
        CreateProgressDialog(MsgToken<1211>());
        Sleep(MSGDELAY);
    }

  // this should work ok for all pdas as well
  if ( SetSoundVolume() == true ) {
    // LKTOKEN _@M1214_ "AUTOMATIC SOUND LEVEL CONTROL"
    CreateProgressDialog(MsgToken<1214>());
  } else {
    // LKTOKENS _@M1215_ "NO SOUND LEVEL CONTROL"
    CreateProgressDialog(MsgToken<1215>());
  }

  LockTerrainDataGraphics();
  RasterTerrain::OpenTerrain();
  UnlockTerrainDataGraphics();

  ReadWayPoints();
  StartupStore(_T(". LOADED %d WAYPOINTS + %u virtuals%s"),(unsigned)WayPointList.size()-NUMRESWP,NUMRESWP,NEWLINE);
  rotaryLD.Init();
  InitWindRotary(&rotaryWind); // 100103
  MapWindow::zoom.Reset();
  InitLK8000();
  ReadAirfieldFile();
  SetHome(false);
  LKLoadLanguageFile();

  CreateProgressDialog(MsgToken<399>());
  CAirspaceManager::Instance().ReadAirspaces();
  CAirspaceManager::Instance().SortAirspaces();
  OpenTopology();

  CreateProgressDialog(MsgToken<1808>());	// Loading FLARMNET database
  OpenFLARMDetails();

  // LKTOKEN _@M1217_ "Starting devices"
  CreateProgressDialog(MsgToken<1217>());

  ComCheck_Init();
  // we need devInit for all devices. Missing initialization otherwise.
  devInit();

  InitialiseIOThread();
  tracking::Initialize(tracking::GetPlatform());

  InitFlightDataRecorder();

  // re-set polar in case devices need the data
  GlidePolar::SetBallast();

  // LKTOKEN _@M1218_ "Initialising display"
  CreateProgressDialog(MsgToken<1218>());

  // Finally ready to go
  TestLog(TEXT(".... WinMain CreateDrawingThread"));
  MapWindow::CreateDrawingThread();
  Sleep(50);

  SwitchToMapWindow();
  TestLog(TEXT(".... CreateCalculationThread"));
  CreateCalculationThread();

  // find unique ID of this PDA or create a new one
  // Currently disabled in LK, we use a dummy ID
  // CreateAssetNumber();

  TestLog(TEXT(".... ProgramStarted=InitDone"));
  ProgramStarted = psInitDone;
  GlobalRunning = true;
#ifdef ENABLE_OPENGL
  main_window->Invalidate();
#else
  MapWindow::_draw_cv.notify_one();
#endif
	
	InitAirspaceSonar();

#ifndef ANDROID
    if (WarningHomeDir) {
        TCHAR nopath[MAX_PATH];
        LocalPath(nopath, _T(""));
        // LKTOKEN _@M1209_ "CHECK INSTALLATION!"
        MessageBoxX(nopath, MsgToken<1209>(), mbOk);
        WarningHomeDir = false;
    }
#endif

    if (!ISPARAGLIDER && !ISCAR) { // 100925
        if (SAFETYALTITUDEARRIVAL < 500) { // SAFETY is *10, so we check <50 really
            // LKTOKEN  _@M155_ = "CHECK safety arrival altitude\n"
            MessageBoxX(MsgToken<155>(), TEXT("Warning!"), mbOk);
        }
    }

    DetectKeyboardModel();

    return true;
}

void Shutdown() {
  main_window->Destroy();
  Message::Destroy();

  TestLog(TEXT(".... Close Progress Dialog"));
  CloseProgressDialog();


  DeInitLKFonts();
  LKObjects_Delete();
  LKUnloadProfileBitmaps();
  LKUnloadFixedBitmaps();

  LKUnloadLanguageFile();
  InputEvents::UnloadString();

  main_window = nullptr;

  TestLog(_T(".... WinMain terminated, realexitforced=%d"),realexitforced);
}

#ifndef ANDROID
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

int WINAPI WinMain(     HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPSTR     lpCmdLine,
                        int       nCmdShow)
{
    (void)hPrevInstance;

    _hInstance = hInstance; // this need to be first, always !
    const TCHAR* szCmdLine = GetCommandLine();

#ifndef DOCTEST_CONFIG_DISABLE
  auto argc = __argc;
  auto argv = __argv;
#endif

#else
int main(int argc, char *argv[]) {

  std::string strCmdLine("");
  for (int i=1;i<argc;i++) {
    strCmdLine.append(std::string(argv[i]).append(" "));
  }
  const TCHAR* szCmdLine = strCmdLine.c_str();

#endif

#if !defined(ANDROID)
  // use mutex to avoid multiple instances of lk8000 be running
  std::unique_ptr<NamedMutex> single_instance;
  std::unique_ptr<std::unique_lock<NamedMutex>> single_instance_lock;

  try {
    single_instance = std::make_unique<NamedMutex>(_T("LOCK8000"));
    single_instance_lock = std::make_unique<std::unique_lock<NamedMutex>>(*single_instance, std::defer_lock);
    if (!single_instance_lock->try_lock()) {
      return(-2);
    }
  }
  catch (std::exception& e) {
    const tstring error = to_tstring(e.what());
    StartupStore(_T("%s"), error.c_str());
  }
#endif

  ScreenGlobalInit InitScreen;
  SoundGlobalInit InitSound;
  
  auto pSaveScreen = std::make_unique<CScreenOrientation>(LKGetLocalPath());

#ifndef DOCTEST_CONFIG_DISABLE
  {
    doctest::Context test_context(argc, argv);
    startup_store_ostream<char> out;
    test_context.setCout(&out);
    test_context.setOption("no-intro", true);
    test_context.setOption("no-colors", true);
    int test_ret = test_context.run();
    if (test_context.shouldExit()) {
      return test_ret;
    }
  }
#endif

  if(Startup(szCmdLine)) {
    //
    // Main message loop
    //
     main_window->RunModalLoop();
  }

  Shutdown();
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

#endif
