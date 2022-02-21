/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Globals.h,v 1.1 2011/12/21 10:35:29 root Exp root $

   PLEASE USE COMMENTS ALSO HERE TO DESCRIBE YOUR GLOBALS!
   YOU CAN INITIALIZE VALUES TO true,false,zero and NULL,
   or you can do it also inside Globals_Init.

*/

#ifndef GLOBALS_H
#define GLOBALS_H

#include "Poco/Event.h"
#include "Time/PeriodClock.hpp"
#include "Comm/PortConfig.h"


  #undef  GEXTERN
  #undef  GEXTTRUE
  #undef  GEXTFALSE
  #undef  GEXTNULL
  #undef  GEXTZERO

#if defined(STATIC_GLOBALS)
  #define GEXTERN

  #define GEXTTRUE       = true
  #define GEXTFALSE      = false
  #define GEXTNULL       = NULL
  #define GEXTZERO       = 0

#else
  #define GEXTERN extern
  #define GEXTTRUE
  #define GEXTFALSE
  #define GEXTNULL
  #define GEXTZERO

  extern void Globals_Init(void);
#endif

typedef struct _Radio_t
{
	double ActiveFrequency;    //active station frequency
	double PassiveFrequency;   // passive (or standby) station frequency
	TCHAR PassiveName[NAME_SIZE + 1] ;    // passive (or standby) station name
	TCHAR ActiveName[NAME_SIZE + 1] ;     //active station name
	int Volume ;               // Radio Volume
	int Squelch ;              // Radio Squelch
	int Vox ;                  // Radio Intercom Volume
	BOOL Changed;              // Parameter Changed Flag            (TRUE = parameter changed)
	BOOL Enabled;              // Radio Installed d Flag            (TRUE = Radio found)
	BOOL Dual;                 // Dual Channel mode active flag     (TRUE = on)
	BOOL Enabled8_33;          // 8,33kHz Radio enabled             (TRUE = 8,33kHz)
	BOOL RX;                   // Radio reception active            (TRUE = reception)
	BOOL TX;                   // Radio transmission active         (TRUE = transmission)
	BOOL RX_active;            // Radio reception on active station (TRUE = reception)
	BOOL RX_standy;            // Radio reception on passive        (standby) station
	BOOL lowBAT;               // Battery low flag                  (TRUE = Batt low)
	BOOL TXtimeout;            // Timeout while transmission (2Min)
	BOOL ActiveValid;          // active Frequency received flag
	BOOL PassiveValid;         // standy Frequency received flag
	BOOL VolValid;             // Volume received flag
	BOOL SqValid;              // Squelch received flag
	BOOL DualValid;            // Dual received flag
        
}Radio_t;

GEXTERN bool MenuActive GEXTFALSE;
GEXTERN Poco::Event dataTriggerEvent;

GEXTERN unsigned short HaveSystemInfo GEXTZERO; // Normally only on linux
GEXTERN bool HaveBatteryInfo GEXTFALSE;

GEXTERN TCHAR LK8000_Version[256];

GEXTERN StartupState_t ProgramStarted;

GEXTERN int UTCOffset;

GEXTERN ModelType::Type_t GlobalModelType;
GEXTERN TCHAR	GlobalModelName[MAX_PATH];


GEXTERN BYTE RUN_MODE;

// infoboxes
GEXTERN int  InfoType[MAXINFOWINDOWS];

GEXTERN DATAOPTIONS Data_Options[NUMDATAOPTIONS_MAX];

GEXTERN int NumDataOptions;
GEXTERN BOOL InfoBoxesHidden;
GEXTERN int numInfoWindows;

// waypoint data
GEXTERN int HomeWaypoint;
GEXTERN bool TakeOffWayPoint;
GEXTERN bool DeclTakeoffLanding; 

// TODO check
//force Airfields home to be HomeWaypoint if an H flag in waypoints file is not available..
GEXTERN int AirfieldsHomeWaypoint;

GEXTERN int Alternate1;
GEXTERN int Alternate2;
GEXTERN int BestAlternate;
GEXTERN bool DisableBestAlternate;

GEXTERN bool bAutoActive ;
GEXTERN bool bAutoPassiv ;

GEXTERN int ActiveAlternate;

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

#ifndef ANDROID
GEXTERN bool  WarningHomeDir;
#endif

// Specials
GEXTERN bool UseGeoidSeparation;
GEXTERN bool UseExtSound1;
GEXTERN bool UseExtSound2;
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

// used by auto QFE: do not reset QFE if previously in flight. So you can check QFE
// on the ground, otherwise it turns to zero at once!
GEXTERN bool WasFlying;

GEXTERN double LastDoRangeWaypointListTime;
GEXTERN bool DeviceNeedClipping;

GEXTERN bool EnableAutoBacklight;
GEXTERN bool EnableAutoSoundVolume;
GEXTERN unsigned EnableFLARMMap;
GEXTERN short AircraftCategory;
GEXTERN bool CheckSum;
GEXTERN bool HideUnits;
GEXTERN short OutlinedTp;
GEXTERN short OutlinedTp_Config;
GEXTERN int  OverColor;
GEXTERN LKColor OverColorRef;
GEXTERN int TpFilter;
GEXTERN short MapBox;
GEXTERN short GlideBarMode;
GEXTERN short ArrivalValue;
GEXTERN short NewMapDeclutter;
GEXTERN short AverEffTime;
GEXTERN bool DrawBottom;
GEXTERN short BottomMode; // Stripe number
GEXTERN short BottomSize; // Height of bottom stripe
GEXTERN short TopSize;

// Overlay config
GEXTERN short Overlay_TopLeft;
GEXTERN short Overlay_TopMid;
GEXTERN short Overlay_TopRight;
GEXTERN short Overlay_TopDown;
GEXTERN short Overlay_LeftTop;
GEXTERN short Overlay_LeftMid;
GEXTERN short Overlay_LeftBottom;
GEXTERN short Overlay_LeftDown;
GEXTERN short Overlay_RightTop;
GEXTERN short Overlay_RightMid;
GEXTERN short Overlay_RightBottom;
GEXTERN bool  Overlay_Title;

#ifdef _WGS84
GEXTERN bool earth_model_wgs84;
#endif

// coordinates of the sort boxes. Each mapspace can have a different layout
GEXTERN short SortBoxY[MSM_TOP+1];
GEXTERN short SortBoxX[MSM_TOP+1][MAXSORTBOXES+1];
GEXTERN short BottomGeom; // registry saved lk8000 navboxes geometry
GEXTERN short GlideBarOffset; // offset to the right for drawing LK8000 with GB active
GEXTERN bool  EngineeringMenu; // not saved in registry
GEXTERN short DeclutterMode;

GEXTERN int ClimbZoom;
GEXTERN int CruiseZoom;
GEXTERN double AutoZoomThreshold;
GEXTERN int MaxAutoZoom;

// This is the gauge bar on the left for variometer
GEXTERN int LKVarioBar;
// This is the value to be used for painting the bar
GEXTERN int LKVarioVal;

GEXTERN bool TskOptimizeRoute;
GEXTERN bool TskOptimizeRoute_Config;
GEXTERN short  GliderSymbol;

GEXTERN short OverlaySize;
GEXTERN short BarOpacity;

GEXTERN short FontMapWaypoint;
GEXTERN short FontMapTopology;
GEXTERN short FontInfopage1L;
GEXTERN short FontInfopage2L;
GEXTERN short FontBottomBar;
GEXTERN short FontCustom1;
GEXTERN short FontOverlayBig;
GEXTERN short FontOverlayMedium;
GEXTERN short FontVisualGlide;

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
GEXTERN double LastDoTraffic;
GEXTERN double LastDoAirspaces;
GEXTERN double LastDoNearest;
GEXTERN double LastDoCommon;
GEXTERN double LastDoThermalH;

GEXTERN bool PollingMode;

GEXTERN bool BestWarning;
GEXTERN unsigned int ThermalBar;
GEXTERN bool TrackBar;
GEXTERN double WindCalcSpeed;
GEXTERN int WindCalcTime;
GEXTERN bool RepeatWindCalc;

// FLARM Traffic is real if <=1min, Shadow if <= etc. If >Zombie it is removed
GEXTERN int LKTime_Real, LKTime_Ghost, LKTime_Zombie;

// type of file format for waypoints files
GEXTERN int WpFileType[NO_WP_FILES];

GEXTERN TCHAR WpHome_Name[NAME_SIZE+1];
GEXTERN double WpHome_Lat;
GEXTERN double WpHome_Lon;

GEXTERN TCHAR TAKEOFFWP_Name[NAME_SIZE+1];
GEXTERN TCHAR LANDINGWP_Name[NAME_SIZE+1];

// LK8000 Hearth beats at 2Hz
GEXTERN unsigned LKHearthBeats;
// number of reporting messages from Portmonitor.
GEXTERN int PortMonitorMessages;

// Copy of runtime traffic for instant use
GEXTERN FLARM_TRAFFIC LKTraffic[FLARM_MAX_TRAFFIC];

// Number of IDs (items) of existing traffic updated from DoTraffic
GEXTERN int LKNumTraffic;

// Pointer to FLARM struct, ordered by DoTraffic, from 0 to LKNumTraffic-1
GEXTERN int LKSortedTraffic[FLARM_MAX_TRAFFIC];

GEXTERN int LKTargetIndex;
GEXTERN int LKTargetType;

// Copy of runtime airspaces for instant use (Shared Ressource, Lock is needed)
GEXTERN LKAirspace_Nearest_Item LKAirspaces[MAXNEARAIRSPACES+1];

// Number of asps (items) of existing airspaces updated from DoAirspaces
GEXTERN int LKNumAirspaces;

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

GEXTERN int PGCloseTimeH;
GEXTERN int PGCloseTimeM;
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

// This used in Terrain.cpp to draw water if "Coast Area" not available in topology file
GEXTERN bool LKWaterTopology;


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
GEXTERN double SimNettoVario;

GEXTERN bool AutoContrast; // automatic contrast of terrain
GEXTERN short SnailScale;  // user choice for rescaling the snail trail
GEXTERN double TerrainWhiteness; // HSV luminance

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

GEXTERN short Numpages;
GEXTERN short ModeIndex;

// LK8000 sync flags
GEXTERN bool NearestDataReady;
GEXTERN bool CommonDataReady;
GEXTERN bool RecentDataReady;
GEXTERN bool LKForceDoNearest;
GEXTERN bool LKForceDoCommon;
GEXTERN bool LKForceDoRecent;
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
GEXTERN bool ScreenLandscape;
GEXTERN double Screen0Ratio;
GEXTERN short ScreenGeometry;
GEXTERN int ScreenDensity;
GEXTERN int ScreenThinSize;

GEXTERN int AircraftMenuSize;
GEXTERN int CompassMenuSize;

// 091011 Used by TakeoffLanding inside Calculation.cpp - limited values careful
GEXTERN int time_in_flight;
GEXTERN int time_on_ground;
GEXTERN double TakeOffSpeedThreshold;

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
Radio_t RadioPara ;
Task_t Task = {};

std::vector<WAYPOINT> WayPointList;
std::vector<WPCALC> WayPointCalc;

#undef STATIC_GLOBALS
#else
extern Start_t StartPoints;
extern Task_t Task;
extern TaskStats_t TaskStats;
extern Radio_t RadioPara ;
extern std::vector<WAYPOINT> WayPointList;
extern std::vector<WPCALC> WayPointCalc;
#endif

GEXTERN int PanTaskEdit;
GEXTERN int RealActiveWaypoint;
GEXTERN int ActiveTaskPoint;
GEXTERN bool TaskAborted;
GEXTERN int SelectedWaypoint;
GEXTERN int SectorType;
GEXTERN double SectorRadius;

GEXTERN bool EnableMultipleStartPoints;
GEXTERN int StartLine;
GEXTERN double StartRadius;
GEXTERN int FinishLine;
GEXTERN double FinishRadius;
GEXTERN double AATTaskLength;
GEXTERN int gTaskType GEXTZERO;
GEXTERN bool EnableFAIFinishHeight;
GEXTERN unsigned FinishMinHeight;
GEXTERN unsigned StartMaxHeight;
GEXTERN unsigned StartMaxHeightMargin;
GEXTERN unsigned StartMaxSpeed;
GEXTERN unsigned StartMaxSpeedMargin;
GEXTERN int StartHeightRef;
GEXTERN unsigned FAI28_45Threshold;

GEXTERN unsigned AlarmMaxAltitude1;
GEXTERN unsigned AlarmMaxAltitude2;
GEXTERN unsigned AlarmMaxAltitude3;
GEXTERN unsigned AlarmTakeoffSafety;
GEXTERN unsigned GearWarningAltitude;
GEXTERN int GearWarningMode;

// master flight data
GEXTERN NMEA_INFO GPS_INFO;
GEXTERN DERIVED_INFO CALCULATED_INFO;

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
GEXTERN double WW[2];

GEXTERN int BallastSecsToEmpty;
GEXTERN bool BallastTimerActive;
GEXTERN int Handicap;

GEXTERN bool InfoWindowActive;

// snail trail
GEXTERN SNAIL_POINT SnailTrail[TRAILSIZE];
GEXTERN	int SnailNext;
GEXTERN LONG_SNAIL_POINT LongSnailTrail[LONGTRAILSIZE+1];
GEXTERN	int LongSnailNext;
GEXTERN int TrailLock;

// Logger
GEXTERN bool LoggerActive;

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
GEXTERN unsigned debounceTimeout;

#if defined(PPC2003) || defined(PNA)
GEXTERN bool SetSystemTimeFromGPS;
#endif

GEXTERN bool SaveRuntime;
GEXTERN bool ForceFinalGlide;
GEXTERN bool AutoForceFinalGlide;

GEXTERN CContestMgr::CResult OlcResults[CContestMgr::TYPE_NUM];

// user interface options
GEXTERN int TrailActive;
GEXTERN int TrailActive_Config;
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
GEXTERN bool IsoLine_Config;
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
GEXTERN short ConfBB0Auto;
GEXTERN bool ConfMP[10];
GEXTERN bool ConfIP[10][10];
GEXTERN bool ConfIP11;
GEXTERN bool ConfIP12;
GEXTERN bool ConfIP13;
GEXTERN bool ConfIP14;
GEXTERN bool ConfIP15;
GEXTERN bool ConfIP16;
GEXTERN bool ConfIP17;
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

void Reset_CustomMenu();

GEXTERN bool OverlayClock;
GEXTERN bool UseTwoLines;
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
GEXTERN int AirspaceAckAllSame;        		// acknowledge all airspaces with same name and class
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
GEXTERN bool FONTSCHANGED;
GEXTERN bool AIRCRAFTTYPECHANGED;
GEXTERN bool SNAILCHANGED;

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

GEXTERN int AdditionalContestRule;  	// Enum to Rules to use for the addition contest CContestMgr::ContestRule


GEXTERN short TerrainContrast;
GEXTERN short TerrainBrightness;
GEXTERN short TerrainRamp;


GEXTERN Appearance_t Appearance;
GEXTERN bool InverseInfoBox_Config;

GEXTERN BOOL extGPSCONNECT;
GEXTERN bool DialogActive;

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
GEXTERN TCHAR startDeviceFile[MAX_PATH];
GEXTERN TCHAR defaultDeviceFile[MAX_PATH];
GEXTERN TCHAR startPilotFile[MAX_PATH];
GEXTERN TCHAR defaultPilotFile[MAX_PATH];

//
// Fonts
//
GEXTERN LKFont	TitleWindowFont;
GEXTERN LKFont   MapWindowFont;
GEXTERN LKFont   MapWindowBoldFont;
GEXTERN LKFont   CDIWindowFont;
GEXTERN LKFont   LK8GenericVar03Font;

GEXTERN LKFont   Custom1Font;
GEXTERN LKFont   MapWaypointFont;
GEXTERN LKFont   MapWaypointBoldFont;
GEXTERN LKFont   MapScaleFont;
GEXTERN LKFont   MapTopologyFont;

GEXTERN LKFont   LK8TitleFont;
GEXTERN LKFont   LK8MapFont;
GEXTERN LKFont   LK8GenericVar01Font;
GEXTERN LKFont   LK8GenericVar02Font;
GEXTERN LKFont   LK8BottomBarTitleFont;
GEXTERN LKFont   LK8BottomBarValueFont;
GEXTERN LKFont   LK8BottomBarUnitFont;
GEXTERN LKFont   LK8TargetFont;
GEXTERN LKFont   LK8BigFont;
GEXTERN LKFont   LK8OverlayBigFont;
GEXTERN LKFont   LK8OverlayMediumFont;
GEXTERN LKFont   LK8OverlaySmallFont;
GEXTERN LKFont   LK8OverlayGatesFont;
GEXTERN LKFont   LK8OverlayMcModeFont;
GEXTERN LKFont   LK8VisualTopFont;
GEXTERN LKFont   LK8VisualBotFont;
GEXTERN LKFont   LK8MediumFont;
GEXTERN LKFont   LK8SmallFont;
GEXTERN LKFont   LK8InfoBigFont;
GEXTERN LKFont   LK8InfoBigItalicFont;
GEXTERN LKFont   LK8InfoBig2LFont;
GEXTERN LKFont   LK8InfoBigItalic2LFont;
GEXTERN LKFont   LK8InfoNormalFont;
GEXTERN LKFont   LK8InfoNearestFont;
GEXTERN LKFont   LK8InfoSmallFont;
GEXTERN LKFont   LK8PanelBigFont;
GEXTERN LKFont   LK8PanelMediumFont;
GEXTERN LKFont   LK8PanelSmallFont;
GEXTERN LKFont   LK8PanelUnitFont;

//
// File names and paths
//
GEXTERN TCHAR szPolarFile[MAX_PATH];
GEXTERN TCHAR szPolarName[80];
GEXTERN TCHAR szAirspaceFile[NO_AS_FILES][MAX_PATH];
GEXTERN TCHAR szWaypointFile[NO_WP_FILES][MAX_PATH];
GEXTERN TCHAR szAdditionalWaypointFile[MAX_PATH];
GEXTERN TCHAR szTerrainFile[MAX_PATH];
GEXTERN TCHAR szAirfieldFile[MAX_PATH];
GEXTERN TCHAR szLanguageCode[16];
GEXTERN TCHAR szInputFile[MAX_PATH];
GEXTERN TCHAR szMapFile[MAX_PATH];

// Ports and devices
GEXTERN unsigned  SelectedDevice;
GEXTERN PortConfig_t PortConfig[NUMDEV];

GEXTERN double LastFlarmCommandTime;
GEXTERN bool  DevIsCondor;
// Units , configurable only in system config

GEXTERN unsigned SpeedUnit_Config;
GEXTERN unsigned TaskSpeedUnit_Config;
GEXTERN unsigned DistanceUnit_Config;
GEXTERN unsigned LiftUnit_Config;
GEXTERN unsigned AltitudeUnit_Config;

// Logger
GEXTERN TCHAR PilotName_Config[100];
GEXTERN TCHAR AircraftType_Config[50];
GEXTERN TCHAR AircraftRego_Config[50];
GEXTERN TCHAR CompetitionClass_Config[50];
GEXTERN TCHAR CompetitionID_Config[50];

GEXTERN bool LockSettingsInFlight;
GEXTERN bool LoggerShortName;

GEXTERN bool UseHiresBitmap;
GEXTERN bool UseUngestures;
GEXTERN bool UseWindRose;

// Multimap stuff
GEXTERN bool Multimap_Flags_Terrain[MP_TOP+1];
GEXTERN bool Multimap_Flags_Topology[MP_TOP+1];
GEXTERN bool Multimap_Flags_Airspace[MP_TOP+1];
GEXTERN bool Multimap_Flags_Waypoints[MP_TOP+1];
GEXTERN bool Multimap_Flags_Overlays_Text[MP_TOP+1];
GEXTERN bool Multimap_Flags_Overlays_Gauges[MP_TOP+1];

GEXTERN unsigned short Multimap_Labels[MP_TOP+1];

GEXTERN int  Multimap_SizeY[MP_TOP+1];

GEXTERN bool Flags_DrawFAI_config;
GEXTERN bool Flags_DrawXC_config;

// Global, not saved to profile
GEXTERN bool Flags_DrawTask;
GEXTERN bool Flags_DrawFAI;
GEXTERN bool Flags_DrawXC;

GEXTERN unsigned int Trip_Moving_Time;
GEXTERN unsigned int Trip_Steady_Time;

GEXTERN double Rotary_Speed;
GEXTERN double Rotary_Distance;

GEXTERN unsigned short Multimap1;
GEXTERN unsigned short Multimap2;
GEXTERN unsigned short Multimap3;
GEXTERN unsigned short Multimap4;
GEXTERN unsigned short Multimap5;

GEXTERN int MMNorthUp_Runtime[NUMBER_OF_SHARED_MULTIMAPS];
GEXTERN int AspPermanentChanged;
GEXTERN int iFlarmDirection;

GEXTERN bool SonarWarning;
GEXTERN bool SonarWarning_Config;

GEXTERN bool EnableAudioVario;

//
// ---------------------------------------------------------------------------
// SWITCHES: switch them on, and something happens. Thread safe.
// Once the action is performed, they are automatically cleared.
// ---------------------------------------------------------------------------
//

// Force freeflight restart for motorgliders
GEXTERN bool LKSW_ForceFreeFlightRestart GEXTFALSE;

// Reset Odometer, by Calc thread
GEXTERN bool LKSW_ResetOdometer GEXTFALSE;

// Reset Trip computer values, by Calc
GEXTERN bool LKSW_ResetTripComputer GEXTFALSE;

// Reset LD Rotary buffer
GEXTERN bool LKSW_ResetLDRotary GEXTFALSE;

// Force landing if we are stationary, or below 3kmh, otherwise ignore
GEXTERN bool LKSW_ForceLanding GEXTFALSE;

////////////////////////////////////////////////////////////////////////////////
// use this two inline form when we already lock Task Data...
//  avoid recusive Lock overhead
inline bool ValidWayPointFast(int i) {
    return ((i>=0)&&(i<(int)WayPointList.size()));
}

inline bool ValidTaskPointFast(int i) {
  return ((i>=0) && (i<MAXTASKPOINTS)) ? ValidWayPointFast(Task[i].Index):false;
}
////////////////////////////////////////////////////////////////////////////////

#endif
