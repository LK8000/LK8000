/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Globals.h,v 1.1 2011/12/21 10:35:29 root Exp root $

   PLEASE USE COMMENTS ALSO HERE TO DESCRIBE YOUR GLOBALS!
   YOU CAN INITIALIZE VALUES TO true,false,zero and NULL, 
   or you can do it also inside Globals_Init.

*/

#ifndef GLOBALS_H
#define GLOBALS_H

  #undef  GEXTERN
  #undef  GEXTTRUE
  #undef  GEXTFALSE
  #undef  GEXTNULL
  #undef  GEXTFONTNULL
  #undef  GEXTZERO

#if defined(STATIC_GLOBALS)
  #define GEXTERN 

  #define GEXTTRUE       = true
  #define GEXTFALSE      = false
  #define GEXTNULL       = NULL
  #define GEXTFONTNULL	 =(HFONT)NULL
  #define GEXTZERO       = 0

#else
  #define GEXTERN extern
  #define GEXTTRUE  
  #define GEXTFALSE 
  #define GEXTNULL 
  #define GEXTFONTNULL 
  #define GEXTZERO 

  extern void Globals_Init(void);
#endif

#ifdef GTL2
GEXTERN pointObj GlideFootPrint2[NUMTERRAINSWEEPS+1];
#endif

GEXTERN bool MenuActive GEXTFALSE;
GEXTERN HANDLE dataTriggerEvent;

// System boot specific flags
// Give me a go/no-go
GEXTERN bool goInstallSystem GEXTFALSE;
GEXTERN bool goCalculationThread GEXTFALSE;




GEXTERN TCHAR LK8000_Version[256];

// instance of main program
GEXTERN HINSTANCE hInst;

GEXTERN StartupState_t ProgramStarted;


GEXTERN int UTCOffset;

GEXTERN int	GlobalModelType; 
GEXTERN TCHAR	GlobalModelName[MAX_PATH];


GEXTERN BYTE RUN_MODE;

// asset/registration data
GEXTERN TCHAR strAssetNumber[MAX_ASSETIDSTRING];

// windows
GEXTERN HWND hWndMainWindow;           // HWND Main Window
GEXTERN HWND hWndMapWindow;            // HWND MapWindow

// infoboxes
GEXTERN int  InfoType[MAXINFOWINDOWS];

GEXTERN DATAOPTIONS Data_Options[NUMDATAOPTIONS_MAX];

GEXTERN int NumDataOptions;
GEXTERN BOOL InfoBoxesHidden;
GEXTERN int numInfoWindows;

// waypoint data
GEXTERN int HomeWaypoint;
GEXTERN bool TakeOffWayPoint;

// TODO check
//force Airfields home to be HomeWaypoint if an H flag in waypoints file is not available..
GEXTERN int AirfieldsHomeWaypoint;

GEXTERN int Alternate1;
GEXTERN int Alternate2;
GEXTERN int BestAlternate;
GEXTERN int ActiveAlternate;

GEXTERN unsigned int NumberOfWayPoints;
GEXTERN int  WaypointsOutOfRange;
GEXTERN int  RangeLandableIndex[MAXRANGELANDABLE+1]; 
GEXTERN int  RangeAirportIndex[MAXRANGELANDABLE+1];
GEXTERN int  RangeTurnpointIndex[MAXRANGETURNPOINT+1];

GEXTERN int  RangeLandableNumber;
GEXTERN int  RangeAirportNumber;
GEXTERN int  RangeTurnpointNumber;

// This list is sorted out of RangeLandableIndex, used by DoNearest
// cannot be used elsewhere, since it's only updated when in Nearest MapSpaceMode.
// +1 is for safety.
GEXTERN int  SortedLandableIndex[MAXNEAREST+1];
GEXTERN int  SortedAirportIndex[MAXNEAREST+1];
GEXTERN int  SortedTurnpointIndex[MAXNEAREST+1];

// Real number of NEAREST items contained in array after removing duplicates, or not enough to fill MAXNEAREST/MAX..
GEXTERN int  SortedNumber;
// GEXTERN int  SortedTurnpointNumber; 101222

// Commons are Home, best alternate, alternate1, 2, and task waypoints , all up to MAXCOMMON.
// It is reset when changing wp file
GEXTERN int  CommonIndex[MAXCOMMON+1];
GEXTERN int  CommonNumber;

// History of recent waypoints
GEXTERN int  RecentIndex[MAXCOMMON+1];
GEXTERN unsigned int RecentChecksum[MAXCOMMON+1];
GEXTERN int  RecentNumber;

//  mapspace sort mode: 0 wp name  1 distance  2 bearing  3 reff  4 altarr
//  UNUSED on MSM_COMMON etc. however it is dimensioned on mapspacemodes
GEXTERN short  SortedMode[MSM_TOP+1];

GEXTERN bool  WarningHomeDir;
GEXTERN TCHAR LKLangSuffix[4];

// Specials
GEXTERN bool UseGeoidSeparation;
GEXTERN double GPSAltitudeOffset; 	// VENTA3
GEXTERN bool PressureHg;
//GEXTERN bool ShortcutIbox;
GEXTERN int  CustomKeyTime;
GEXTERN int  CustomKeyModeCenter;
GEXTERN int  CustomKeyModeLeft;
GEXTERN int  CustomKeyModeRight;
GEXTERN int  CustomKeyModeAircraftIcon;
GEXTERN int  CustomKeyModeLeftUpCorner;
GEXTERN int  CustomKeyModeRightUpCorner;
GEXTERN int  CustomKeyModeCenterScreen;
GEXTERN double QFEAltitudeOffset; // VENTA3
GEXTERN int OnAirSpace; // VENTA3 toggle DrawAirSpace

// used by auto QFE: do not reset QFE if previously in flight. So you can check QFE
// on the ground, otherwise it turns to zero at once!
GEXTERN bool WasFlying;

GEXTERN double LastDoRangeWaypointListTime;
GEXTERN bool needclipping;
GEXTERN bool EnableAutoBacklight;
GEXTERN bool EnableAutoSoundVolume;
GEXTERN DWORD EnableFLARMMap;
GEXTERN short AircraftCategory;
GEXTERN bool ExtendedVisualGlide;
GEXTERN short Look8000;
GEXTERN bool CheckSum;
GEXTERN bool HideUnits;
GEXTERN short OutlinedTp;
GEXTERN short OutlinedTp_Config;
GEXTERN int  OverColor;
GEXTERN COLORREF OverColorRef;
GEXTERN int TpFilter;
GEXTERN short MapBox;
GEXTERN short GlideBarMode; 
GEXTERN short ArrivalValue;
GEXTERN short NewMapDeclutter;
GEXTERN short AverEffTime;
GEXTERN bool ActiveMap;
GEXTERN bool ActiveMap_Config;
GEXTERN bool DrawBottom;
GEXTERN short BottomMode; // Stripe number
GEXTERN short BottomSize; // Height of bottom stripe
GEXTERN short TopSize;

// coordinates of the sort boxes. Each mapspace can have a different layout
GEXTERN short SortBoxY[MSM_TOP+1];
GEXTERN short SortBoxX[MSM_TOP+1][MAXSORTBOXES+1];
GEXTERN short BottomGeom; // registry saved lk8000 navboxes geometry
GEXTERN short GlideBarOffset; // offset to the right for drawing LK8000 with GB active
GEXTERN bool  EngineeringMenu; // not saved in registry
GEXTERN short splitter;	
GEXTERN short DeclutterMode;

GEXTERN int PGClimbZoom;
GEXTERN int PGCruiseZoom;
GEXTERN double PGAutoZoomThreshold;

// This is the gauge bar on the left for variometer
GEXTERN int LKVarioBar;
// This is the value to be used for painting the bar
GEXTERN int LKVarioVal;

GEXTERN bool PGOptimizeRoute;

GEXTERN short OverlaySize;
GEXTERN short BarOpacity;
GEXTERN short FontRenderer;
GEXTERN bool LockModeStatus;

// default initialization for gestures. InitLK8000 will fine tune it.
GEXTERN short GestureSize;

GEXTERN bool IphoneGestures;

// moving map is all black and need white painting - not much used 091109
GEXTERN bool BlackScreen; // white on black.. 
// if true, LK specific text on map is painted black, otherwise white
GEXTERN bool LKTextBlack;

// enumerated value for map background when no terrain is painted, valid for both normal and inverted mode
// note that all topology text is in black, so this should be a light colour in any case
GEXTERN short BgMapColor;
GEXTERN short BgMapColor_Config;
GEXTERN bool BgMapColorTextBlack[LKMAXBACKGROUNDS];

GEXTERN int LKVarioSize;

// activated by Utils2 in virtual keys, used inside RenderMapWindowBg
GEXTERN bool PGZoomTrigger;
GEXTERN double  LastZoomTrigger;

// traffic DoTraffic interval, also reset during key up and down to prevent wrong selections
GEXTERN double  LastDoTraffic;

GEXTERN double LastDoAirspaces;
GEXTERN double LastDoNearest;
GEXTERN double LastDoCommon;
GEXTERN double LastDoThermalH;

// xml dlgconfiguration value replacing 246 which became 278
GEXTERN int LKwdlgConfig;

GEXTERN bool PollingMode;

GEXTERN bool BestWarning;
GEXTERN bool ThermalBar;
GEXTERN bool McOverlay;
GEXTERN bool TrackBar;
GEXTERN double WindCalcSpeed;
GEXTERN int WindCalcTime;
GEXTERN bool RepeatWindCalc;

// FLARM Traffic is real if <=1min, Shadow if <= etc. If >Zombie it is removed
GEXTERN int LKTime_Real, LKTime_Ghost, LKTime_Zombie;

// type of file format for waypoints files
GEXTERN int WpFileType[3]; 

GEXTERN TCHAR WpHome_Name[NAME_SIZE+1];
GEXTERN double WpHome_Lat;
GEXTERN double WpHome_Lon;

GEXTERN TCHAR TAKEOFFWP_Name[NAME_SIZE+1];
GEXTERN TCHAR LANDINGWP_Name[NAME_SIZE+1];

// LK8000 Hearth beats at 2Hz
GEXTERN double LKHearthBeats;
// number of reporting messages from Portmonitor.
GEXTERN int PortMonitorMessages;

GEXTERN int LKIBLSCALE[MAXIBLSCALE+1];
GEXTERN int FlarmNetCount;

// Copy of runtime traffic for instant use
GEXTERN FLARM_TRAFFIC LKTraffic[FLARM_MAX_TRAFFIC+1];

// Number of IDs (items) of existing traffic updated from DoTraffic
GEXTERN int LKNumTraffic;

// Pointer to FLARM struct, ordered by DoTraffic, from 0 to LKNumTraffic-1
GEXTERN int LKSortedTraffic[FLARM_MAX_TRAFFIC+1];

GEXTERN int LKTargetIndex;
GEXTERN int LKTargetType;

// Copy of runtime airspaces for instant use
GEXTERN LKAirspace_Nearest_Item LKAirspaces[MAXNEARAIRSPACES+1];

// Number of asps (items) of existing airspaces updated from DoAirspaces
GEXTERN int LKNumAirspaces;

// Pointer to ASP struct, ordered by DoAirspaces, from 0 to LKNumAirspaces-1
GEXTERN int LKSortedAirspaces[MAXNEARAIRSPACES+1];

// The Thermal History internal database
GEXTERN THERMAL_HISTORY	ThermalHistory[MAX_THERMAL_HISTORY+1];
// Copy of runtime thermal history structure for instant use
GEXTERN THERMAL_HISTORY	CopyThermalHistory[MAX_THERMAL_HISTORY+1];
// Number of Thermals updated from DoThermalHistory
GEXTERN int LKNumThermals;
GEXTERN int LKSortedThermals[MAX_THERMAL_HISTORY+1];

GEXTERN NearestTopoItem NearestBigCity;
GEXTERN NearestTopoItem NearestCity;
GEXTERN NearestTopoItem NearestSmallCity;
GEXTERN NearestTopoItem NearestWaterArea;

// Paraglider's time gates
// ------------------------------
// Open and close time, gate 0  ex. 12:00
// M and H for registry

GEXTERN int PGOpenTimeH;
GEXTERN int PGOpenTimeM;
GEXTERN int PGOpenTime;
GEXTERN int PGCloseTime;

// Interval, in minutes
GEXTERN int PGGateIntervalTime;
// How many gates, 1-x
GEXTERN int PGNumberOfGates;
// Start out or start in?
GEXTERN bool PGStartOut;
// Current assigned gate
GEXTERN int ActiveGate;

// LKMAPS flag for topology: >0 means ON, and indicating how many topo files are loaded
GEXTERN int LKTopo;

// This threshold used in Terrain.cpp to distinguish water altitude
GEXTERN short LKWaterThreshold;
GEXTERN double LKTopoZoomCat05;
GEXTERN double LKTopoZoomCat10;
GEXTERN double LKTopoZoomCat20;
GEXTERN double LKTopoZoomCat30;
GEXTERN double LKTopoZoomCat40;
GEXTERN double LKTopoZoomCat50;
GEXTERN double LKTopoZoomCat60;
GEXTERN double LKTopoZoomCat70;
GEXTERN double LKTopoZoomCat80;
GEXTERN double LKTopoZoomCat90;
GEXTERN double LKTopoZoomCat100;
GEXTERN double LKTopoZoomCat110;

// max number of topo and wp labels painted on map, defined by default in Utils
GEXTERN int LKMaxLabels;

// current mode of overtarget 0=task 1=alt1, 2=alt2, 3=best alt
GEXTERN short OvertargetMode;

// Simulator mode Turn rate, degrees per second (positive or negative)
// Simulator has one thermal at a time with these values
GEXTERN double	SimTurn;
GEXTERN double ThLatitude;
GEXTERN double ThLongitude;
GEXTERN double ThermalRadius;
GEXTERN double SinkRadius;

// Append over here NEW

#if LK_CACHECALC

#if (LK_CACHECALC_MCA && LK_CACHECALC_MCA_STAT)
GEXTERN int  Cache_Calls_MCA;
GEXTERN int  Cache_Hits_MCA;
GEXTERN int  Cache_Fail_MCA;
GEXTERN int  Cache_False_MCA;
GEXTERN int  Cache_Incomplete_MCA;
#endif

#if (LK_CACHECALC_DBE && LK_CACHECALC_DBE_STAT)
GEXTERN int  Cache_Calls_DBE;
GEXTERN int  Cache_Hits_DBE;
GEXTERN int  Cache_Fail_DBE;
GEXTERN int  Cache_False_DBE;
#endif

#endif

// General index of all pages
// current mapspacemode: the internal identifier of a page type
// should not be used for turning pages, only for direct access
GEXTERN short MapSpaceMode;

// current selected raw in mapspacemodes
GEXTERN short SelectedRaw[MSM_TOP+1];

// current page in each mapspacemode, reset entering new mapspace: no memory
// since it doesnt eat memory, it is also used for pages with currently no subpages
GEXTERN short SelectedPage[MSM_TOP+1];

GEXTERN short Numraws;
GEXTERN short Numpages;
GEXTERN short CommonNumraws;
GEXTERN short CommonNumpages;
// GEXTERN short TurnpointNumraws; 101222
// GEXTERN short TurnpointNumpages;
GEXTERN short TrafficNumpages;
GEXTERN short AspNumpages;
GEXTERN short THistoryNumpages;
GEXTERN short ModeIndex;

// LK8000 sync flags
GEXTERN bool NearestDataReady;
GEXTERN bool CommonDataReady;
GEXTERN bool RecentDataReady;
// GEXTERN bool NearestTurnpointDataReady; 101222
GEXTERN bool LKForceDoNearest;
GEXTERN bool LKForceDoCommon;
GEXTERN bool LKForceDoRecent;
// GEXTERN bool LKForceDoNearestTurnpoint; 101222
GEXTERN short LKevent;
GEXTERN bool LKForceComPortReset;
GEXTERN bool LKDoNotResetComms;

GEXTERN ldrotary_s rotaryLD;
GEXTERN windrotary_s rotaryWind;
GEXTERN lkalarms_s LKalarms[MAXLKALARMS];
// airspace data

GEXTERN short AltArrivMode;
GEXTERN bool GlobalRunning;

GEXTERN short ScreenSize;
GEXTERN int ScreenSizeX;
GEXTERN int ScreenSizeY;
GEXTERN RECT ScreenSizeR;
GEXTERN bool ScreenLandscape;
GEXTERN double ScreenDScale;
GEXTERN int ScreenScale;
GEXTERN bool ScreenIntScale;
// Used as a delimiter in MapWnd for clicks on screen.
GEXTERN int BottomBarY;
GEXTERN int AircraftMenuSize;
GEXTERN int CompassMenuSize;

// 091011 Used by TakeoffLanding inside Calculation.cpp - limited values careful
GEXTERN int time_in_flight;
GEXTERN int time_on_ground;
GEXTERN double TakeOffSpeedThreshold;

// Com port diagnostic - see Utils2.h
GEXTERN int ComPortStatus[NUMDEV+1];
GEXTERN long ComPortRx[NUMDEV+1];
GEXTERN long ComPortErrRx[NUMDEV+1];
GEXTERN long ComPortTx[NUMDEV+1];
GEXTERN long ComPortErrTx[NUMDEV+1];
GEXTERN long ComPortErrors[NUMDEV+1];
// Com ports hearth beats, based on LKHearthBeats
GEXTERN double ComPortHB[NUMDEV+1];

// Cpu stats
#ifdef CPUSTATS
GEXTERN int Cpu_Draw;
GEXTERN int Cpu_Calc;
GEXTERN int Cpu_Instrument;
GEXTERN int Cpu_PortA;
GEXTERN int Cpu_PortB;
GEXTERN int Cpu_Aver;
#endif

GEXTERN double	NearestAirspaceHDist;
GEXTERN double	NearestAirspaceVDist;
GEXTERN TCHAR    NearestAirspaceName[NAME_SIZE+1];
GEXTERN TCHAR    NearestAirspaceVName[NAME_SIZE+1];

// Ready to use for experiments
GEXTERN double Experimental1;
GEXTERN double Experimental2;


#if defined(STATIC_GLOBALS)
// task data
Start_t StartPoints;
TaskStats_t TaskStats;
Task_t Task = {{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0
,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0}};

WAYPOINT *WayPointList=NULL;
WPCALC   *WayPointCalc=NULL;
#undef STATIC_GLOBALS
#else
extern START_POINT StartPoints[];
extern TASK_POINT Task[];
extern TASKSTATS_POINT TaskStats[];
extern WAYPOINT *WayPointList;
extern WPCALC   *WayPointCalc;
#endif

GEXTERN int ActiveWayPoint;
GEXTERN bool TaskAborted;
GEXTERN int SelectedWaypoint;
GEXTERN int SectorType;
GEXTERN DWORD SectorRadius;

GEXTERN bool EnableMultipleStartPoints;
GEXTERN int StartLine;
GEXTERN DWORD StartRadius;
GEXTERN int FinishLine;
GEXTERN DWORD FinishRadius;
GEXTERN double AATTaskLength;
GEXTERN BOOL AATEnabled;
GEXTERN bool EnableFAIFinishHeight;
GEXTERN DWORD FinishMinHeight;
GEXTERN DWORD StartMaxHeight;
GEXTERN DWORD StartMaxHeightMargin;
GEXTERN DWORD StartMaxSpeed;
GEXTERN DWORD StartMaxSpeedMargin;
GEXTERN int StartHeightRef;

GEXTERN DWORD AlarmMaxAltitude1;
GEXTERN DWORD AlarmMaxAltitude2;
GEXTERN DWORD AlarmMaxAltitude3;
GEXTERN DWORD AlarmTakeoffSafety;

// master flight data
GEXTERN NMEA_INFO GPS_INFO;
GEXTERN DERIVED_INFO CALCULATED_INFO;

// gps detection
GEXTERN BOOL GPSCONNECT;
GEXTERN BOOL VARIOCONNECT;

// units
GEXTERN double SPEEDMODIFY;
GEXTERN double LIFTMODIFY;
GEXTERN double DISTANCEMODIFY;
GEXTERN double ALTITUDEMODIFY; 
GEXTERN double TASKSPEEDMODIFY;

// polar info
GEXTERN double BUGS;
GEXTERN double BUGS_Config;
GEXTERN double BALLAST;
GEXTERN int POLARID;
GEXTERN double POLAR[POLARSIZE];
GEXTERN double WEIGHTS[POLARSIZE];
GEXTERN double POLARV[POLARSIZE];
GEXTERN double POLARLD[POLARSIZE];

GEXTERN int BallastSecsToEmpty;
GEXTERN bool BallastTimerActive;
GEXTERN int Handicap;

GEXTERN bool InfoWindowActive;

// snail trail
GEXTERN SNAIL_POINT SnailTrail[TRAILSIZE];
GEXTERN	int SnailNext;
GEXTERN int TrailLock;

// Logger
GEXTERN bool LoggerActive;
GEXTERN int LoggerTimeStepCruise;
GEXTERN int LoggerTimeStepCircling;
GEXTERN bool IGCWriteLock;

// user controls/parameters
GEXTERN double MACCREADY;
GEXTERN bool   AutoMacCready_Config;
GEXTERN int    AutoMcMode;
GEXTERN int    AutoMcMode_Config;
GEXTERN bool   UseTotalEnergy;
GEXTERN bool   UseTotalEnergy_Config;
GEXTERN double SAFETYALTITUDEARRIVAL;
GEXTERN double SAFETYALTITUDETERRAIN;
GEXTERN double SAFTEYSPEED;

// Terrain
GEXTERN short TerrainRamp_Config;

GEXTERN double QNH;
GEXTERN int NettoSpeed;
GEXTERN int debounceTimeout;
GEXTERN bool SetSystemTimeFromGPS;
GEXTERN bool ForceFinalGlide;
GEXTERN bool AutoForceFinalGlide;

GEXTERN CContestMgr::CResult OlcResults[CContestMgr::TYPE_NUM];

// user interface options
GEXTERN bool bAirspaceBlackOutline;
GEXTERN int TrailActive;
GEXTERN int TrailActive_Config;
GEXTERN int VisualGlide;
GEXTERN bool EnableTopology;
GEXTERN bool EnableTopology_Config;
GEXTERN bool EnableTerrain;
GEXTERN bool EnableTerrain_Config;
GEXTERN int FinalGlideTerrain;

// 0: Manual
// 1: Circling
// 2: ZigZag
// 3: Both
GEXTERN int AutoWindMode;
GEXTERN int AutoWindMode_Config;
GEXTERN bool EnableTrailDrift_Config;
GEXTERN bool AutoZoom_Config;

GEXTERN bool EnableNavBaroAltitude;
GEXTERN bool EnableNavBaroAltitude_Config;
GEXTERN short Orbiter;
GEXTERN short Orbiter_Config;
GEXTERN short Shading;
GEXTERN short Shading_Config;
GEXTERN bool ConfBB[10];
GEXTERN bool ConfBB0;
GEXTERN bool ConfBB1;
GEXTERN bool ConfBB2;
GEXTERN bool ConfBB3;
GEXTERN bool ConfBB4;
GEXTERN bool ConfBB5;
GEXTERN bool ConfBB6;
GEXTERN bool ConfBB7;
GEXTERN bool ConfBB8;
GEXTERN bool ConfBB9;
GEXTERN bool ConfMP[10];
GEXTERN bool ConfIP[10][10];
GEXTERN bool ConfIP11;
GEXTERN bool ConfIP12;
GEXTERN bool ConfIP13;
GEXTERN bool ConfIP14;
GEXTERN bool ConfIP15;
GEXTERN bool ConfIP16;
GEXTERN bool ConfIP21;
GEXTERN bool ConfIP22;
GEXTERN bool ConfIP23;
GEXTERN bool ConfIP24;
GEXTERN bool ConfIP31;
GEXTERN bool ConfIP32;
GEXTERN bool ConfIP33;

GEXTERN unsigned short CustomMenu1;
GEXTERN unsigned short CustomMenu2;
GEXTERN unsigned short CustomMenu3;
GEXTERN unsigned short CustomMenu4;
GEXTERN unsigned short CustomMenu5;
GEXTERN unsigned short CustomMenu6;
GEXTERN unsigned short CustomMenu7;
GEXTERN unsigned short CustomMenu8;
GEXTERN unsigned short CustomMenu9;
GEXTERN unsigned short CustomMenu10;

GEXTERN bool OverlayClock;
GEXTERN bool EnableSoundModes;
GEXTERN int DisplayOrientation;
GEXTERN int DisplayOrientation_Config;
GEXTERN double AutoOrientScale;
GEXTERN int DisplayTextType;
GEXTERN int AIRSPACEWARNINGS;
GEXTERN int WarningTime;

GEXTERN int AcknowledgementTime;		// keep ack level for this time, [secs]
GEXTERN int AirspaceWarningRepeatTime;		// warning repeat time if not acknowledged
GEXTERN int AirspaceWarningVerticalMargin;	// vertical distance used to calculate too close condition
GEXTERN int AirspaceWarningDlgTimeout;          // airspace warning dialog auto closing in x secs
GEXTERN int AirspaceWarningMapLabels;           // airspace warning labels showed on map
GEXTERN int AltitudeMode;
GEXTERN int AltitudeMode_Config;
GEXTERN int SafetyAltitudeMode;
GEXTERN int ClipAltitude;
GEXTERN int AltWarningMargin;
GEXTERN int AutoAdvance;
GEXTERN int AutoAdvance_Config;
GEXTERN bool AdvanceArmed;


GEXTERN int MenuTimeout_Config;   // config
GEXTERN int MenuTimeOut;      // runtime


GEXTERN int EnableThermalLocator;

GEXTERN bool ExternalTriggerCruise;
GEXTERN bool ExternalTriggerCircling;
GEXTERN bool EnableExternalTriggerCruise;
// statistics
GEXTERN Statistics flightstats;

// used in settings dialog 
GEXTERN BOOL COMPORTCHANGED;
GEXTERN BOOL AIRSPACEFILECHANGED;
GEXTERN BOOL WAYPOINTFILECHANGED;
GEXTERN BOOL TERRAINFILECHANGED;
GEXTERN BOOL AIRFIELDFILECHANGED;
GEXTERN BOOL TOPOLOGYFILECHANGED;
GEXTERN BOOL POLARFILECHANGED;
GEXTERN BOOL LANGUAGEFILECHANGED;
GEXTERN BOOL INPUTFILECHANGED;
GEXTERN BOOL MAPFILECHANGED;
GEXTERN bool NEWWAYPOINTFILE;

// Team code
GEXTERN int TeamCodeRefWaypoint;
GEXTERN TCHAR TeammateCode[10];
GEXTERN double TeammateLatitude;
GEXTERN double TeammateLongitude;
GEXTERN bool TeammateCodeValid;
GEXTERN bool TeamFlarmTracking;
GEXTERN TCHAR TeamFlarmCNTarget[4]; // CN of the glider to track
GEXTERN int TeamFlarmIdTarget;    // FlarmId of the glider to track

GEXTERN bool DisableAutoLogger;
GEXTERN int LiveTrackerInterval;

// Interface Globals
GEXTERN StatusMessageSTRUCT StatusMessageData[MAXSTATUSMESSAGECACHE];
GEXTERN int StatusMessageData_Size;

GEXTERN bool LKLanguageReady;

GEXTERN int UseCustomFonts;

#if (WINDOWSPC>0) 
GEXTERN int SCREENWIDTH;
GEXTERN int SCREENHEIGHT;
#endif

GEXTERN short TerrainContrast;
GEXTERN short TerrainBrightness;
GEXTERN short TerrainRamp;


GEXTERN Appearance_t Appearance;
GEXTERN bool InverseInfoBox_Config;

#ifdef CPUSTATS
GEXTERN HANDLE hCalculationThread;
GEXTERN DWORD dwCalcThreadID;
#endif

GEXTERN BOOL extGPSCONNECT;
GEXTERN bool DialogActive;

GEXTERN HANDLE drawTriggerEvent;


#if  (LK_CACHECALC && LK_CACHECALC_MCA_STAT)
GEXTERN int  Cache_Calls_MCA GEXTZERO;
GEXTERN int  Cache_Hits_MCA  GEXTZERO;
GEXTERN int  Cache_Fail_MCA  GEXTZERO;
GEXTERN int  Cache_False_MCA GEXTZERO;
GEXTERN int  Cache_Incomplete_MCA GEXTZERO;
#endif
#if (LK_CACHECALC)
GEXTERN int  Cache_Calls_DBE GEXTZERO;
GEXTERN int  Cache_Hits_DBE  GEXTZERO;
GEXTERN int  Cache_Fail_DBE  GEXTZERO;
GEXTERN int  Cache_False_DBE GEXTZERO;
#endif


// Pointers to MapSpacemodes, init by InitModeTable()
// MSM_TOP is used as max size also for each subsets
GEXTERN short ModeTable[LKMODE_TOP+1][MSM_TOP+1];
// top of the list inside each table. Could be a struct with ModeTable
GEXTERN short ModeTableTop[LKMODE_TOP+1];
// remembers for each mode (wp, infopage, map , etc.) the current type
GEXTERN short ModeType[LKMODE_TOP+1];

GEXTERN int PDABatteryPercent;
GEXTERN int PDABatteryTemperature;
GEXTERN int PDABatteryStatus;
GEXTERN int PDABatteryFlag;

GEXTERN TCHAR startProfileFile[MAX_PATH];
GEXTERN TCHAR defaultProfileFile[MAX_PATH];
GEXTERN TCHAR startAircraftFile[MAX_PATH];
GEXTERN TCHAR defaultAircraftFile[MAX_PATH];
GEXTERN TCHAR startPilotFile[MAX_PATH];
GEXTERN TCHAR defaultPilotFile[MAX_PATH];

//
// Fonts
//
GEXTERN HFONT	TitleWindowFont;
GEXTERN HFONT   MapWindowFont;
GEXTERN HFONT   MapWindowBoldFont;
GEXTERN HFONT   CDIWindowFont;
GEXTERN HFONT   MapLabelFont;
GEXTERN HFONT   StatisticsFont;

GEXTERN HFONT   LK8UnitFont GEXTFONTNULL;
GEXTERN HFONT   LK8TitleFont GEXTFONTNULL;
GEXTERN HFONT   LK8MapFont GEXTFONTNULL;
GEXTERN HFONT   LK8TitleNavboxFont GEXTFONTNULL;
GEXTERN HFONT   LK8ValueFont GEXTFONTNULL;
GEXTERN HFONT   LK8TargetFont GEXTFONTNULL;
GEXTERN HFONT   LK8BigFont GEXTFONTNULL;
GEXTERN HFONT   LK8MediumFont GEXTFONTNULL;
GEXTERN HFONT   LK8SmallFont GEXTFONTNULL;
GEXTERN HFONT   LK8InfoBigFont GEXTFONTNULL;
GEXTERN HFONT   LK8InfoBigItalicFont GEXTFONTNULL;
GEXTERN HFONT   LK8InfoNormalFont GEXTFONTNULL;
GEXTERN HFONT   LK8InfoSmallFont GEXTFONTNULL;
GEXTERN HFONT   LK8PanelBigFont GEXTFONTNULL;
GEXTERN HFONT   LK8PanelMediumFont GEXTFONTNULL;
GEXTERN HFONT   LK8PanelSmallFont GEXTFONTNULL;
GEXTERN HFONT   LK8PanelUnitFont GEXTFONTNULL;

GEXTERN LOGFONT  autoTitleWindowLogFont;
GEXTERN LOGFONT  autoMapWindowLogFont;
GEXTERN LOGFONT  autoMapWindowBoldLogFont;
GEXTERN LOGFONT  autoCDIWindowLogFont;
GEXTERN LOGFONT  autoMapLabelLogFont;
GEXTERN LOGFONT  autoStatisticsLogFont;

//
// File names and paths
//
GEXTERN TCHAR szPolarFile[MAX_PATH];
GEXTERN TCHAR szAirspaceFile[MAX_PATH];
GEXTERN TCHAR szAdditionalAirspaceFile[MAX_PATH];
GEXTERN TCHAR szWaypointFile[MAX_PATH];
GEXTERN TCHAR szAdditionalWaypointFile[MAX_PATH];
GEXTERN TCHAR szTerrainFile[MAX_PATH];
GEXTERN TCHAR szAirfieldFile[MAX_PATH];
GEXTERN TCHAR szLanguageFile[MAX_PATH];
GEXTERN TCHAR szInputFile[MAX_PATH];
GEXTERN TCHAR szMapFile[MAX_PATH];

// Ports and devices

GEXTERN DWORD dwPortIndex1;
GEXTERN DWORD dwSpeedIndex1;
GEXTERN DWORD dwBit1Index;
GEXTERN DWORD dwPortIndex2;
GEXTERN DWORD dwSpeedIndex2;
GEXTERN DWORD dwBit2Index;
GEXTERN TCHAR dwDeviceName1[DEVNAMESIZE+1];
GEXTERN TCHAR dwDeviceName2[DEVNAMESIZE+1];

// Units , configurable only in system config

GEXTERN DWORD SpeedUnit_Config;
GEXTERN DWORD TaskSpeedUnit_Config;
GEXTERN DWORD DistanceUnit_Config;
GEXTERN DWORD LiftUnit_Config;
GEXTERN DWORD AltitudeUnit_Config;

// Editable fonts, verbose string
GEXTERN TCHAR FontDesc_MapWindow[256];
GEXTERN TCHAR FontDesc_MapLabel[256];

// Logger
GEXTERN TCHAR PilotName_Config[100];
GEXTERN TCHAR LiveTrackersrv_Config[100];
GEXTERN TCHAR LiveTrackerusr_Config[100];
GEXTERN TCHAR LiveTrackerpwd_Config[100];
GEXTERN TCHAR AircraftType_Config[50];
GEXTERN TCHAR AircraftRego_Config[50];
GEXTERN TCHAR CompetitionClass_Config[50];
GEXTERN TCHAR CompetitionID_Config[50];

GEXTERN bool LockSettingsInFlight;
GEXTERN bool LoggerShortName;

GEXTERN double COSTABLE[4096];
GEXTERN double SINETABLE[4096];
GEXTERN double INVCOSINETABLE[4096];
GEXTERN int ISINETABLE[4096];
GEXTERN int ICOSTABLE[4096];

GEXTERN short TouchContext;
GEXTERN bool UseHiresBitmap;
GEXTERN bool UseUngestures;
GEXTERN bool UseWindRose;

//
// ---------------------------------------------------------------------------
// SWITCHES: switch them on, and something happens. Thread safe.
// Once the action is performed, they are automatically cleared.
// ---------------------------------------------------------------------------
//

// Tell Draw thread to reload bitmaps that are affected by some 
// changes in profile, such as aircraft icons.
GEXTERN bool LKSW_ReloadProfileBitmaps GEXTFALSE;

// This will calculate nearest topology without painting it, for 1s only.
// It will be automatically cleared by Terrain  DrawTopology()
GEXTERN bool LKSW_ForceNearestTopologyCalculation GEXTFALSE;

// Force freeflight restart for motorgliders
GEXTERN bool LKSW_ForceFreeFlightRestart GEXTFALSE;

// Reset Odometer, by Calc thread
GEXTERN bool LKSW_ResetOdometer GEXTFALSE;

// Force landing if we are stationary, or below 3kmh, otherwise ignore
GEXTERN bool LKSW_ForceLanding GEXTFALSE;

// Close and reopen TerrainRenderer, to quickly use a new screen resolution
GEXTERN bool LKSW_ResetTerrainRenderer GEXTFALSE;

#endif

