/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: lk8000.cpp,v 1.1 2010/12/15 11:30:56 root Exp root $
*/

#include "StdAfx.h"
#include "wcecompat/ts_string.h"
#include "options.h"
#include "compatibility.h"
#include "XCSoar.h"
#include "buildnumber.h"
#include "Cpustats.h"
#include "MapWindow.h"
#include "Parser.h"
#include "Calculations.h"
#include "Calculations2.h"
#include "Task.h"
#include "Dialogs.h"

#ifdef OLDPPCx
#include "XCSoarProcess.h"
#else
#include "Process.h"
#endif

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

#include "externs.h"
#include "Units.h"
#include "InputEvents.h"
#include "Message.h"
#include "Atmosphere.h"
#include "Geoid.h"

#include "InfoBox.h"
#include "RasterTerrain.h"
extern void LKObjects_Create();
extern void LKObjects_Delete();
#include "LKMainObjects.h"

#if defined(LKAIRSPACE) || defined(NEW_OLC)
using std::min;
using std::max;
#endif

#ifdef DEBUG_TRANSLATIONS
#include <map>
static std::map<TCHAR*, TCHAR*> unusedTranslations;
#endif

#include "utils/heapcheck.h"

#if !defined(MapScale2)
  #define MapScale2  apMs2Default
#endif

#if SAMGI
Appearance_t Appearance = {
  apMsAltA,
  apMs2None,
  true,
  206,
  {0,-13},
  apFlightModeIconAltA,
  //apFlightModeIconDefault,
  {10,3},
  apCompassAltA,
  {0,0,0},
  {0,0,0},
  {0,0,0},
  {0,0,0},
  {0,0,0},
  ctBestCruiseTrackAltA,
  afAircraftAltA,
  true,
  fgFinalGlideAltA,
  wpLandableAltA,
  true,
  true,
  true,
  smAlligneTopLeft,
  true,
  true,
  true,
  true,
  true,
  gvnsDefault, 
  false,
  apIbBox,
  false,
  true,
  false
};
#else

Appearance_t Appearance = {
  apMsAltA, // mapscale
  MapScale2, 
  false, // don't show logger indicator
  206,
  {0,-13},
  apFlightModeIconDefault,
  {0,0},
  apCompassAltA,
  {0,0,0},
  {0,0,0},
  {0,0,0},
  {0,0,0},
  {0,0,0},
  {0,0,0},
  {0,0,0},
  {0,0,0},
  ctBestCruiseTrackAltA,
  afAircraftAltA,
  true, // don't show speed to fly
  fgFinalGlideDefault,
  wpLandableDefault,
  true,
  false,
  true,
  smAlligneCenter,
  tiHighScore,
  false,
  false,
  false,
  false,
  false,
  gvnsLongNeedle,
  true,
  apIbBox,
#if defined(PNA) || defined(FIVV)  // VENTA-ADDON Model type
  apIg0,  // VENTA-ADDON GEOM
  apImPnaGeneric,
#endif
  false,
  true,
  false
};

#endif


TCHAR XCSoar_Version[256] = TEXT("");

bool ForceShutdown = false;

HINSTANCE hInst; // The current instance
//HWND hWndCB; // The command bar handle
HWND hWndMainWindow; // Main Windows
HWND hWndMapWindow;  // MapWindow

#ifdef CPUSTATS
HANDLE hCalculationThread;
DWORD dwCalcThreadID;
HANDLE hInstrumentThread;
DWORD dwInstThreadID;
#endif

int numInfoWindows = 9;



InfoBox *InfoBoxes[MAXINFOWINDOWS];

int                                     InfoType[MAXINFOWINDOWS] = 
#ifdef GNAV
  {
    873336334,
    856820491,
    822280982,
    2829105,
    103166000,
    421601569,
    657002759,
    621743887,
    439168301
  };
#else
  {1008146198,
   1311715074,
   923929365,
   975776319,
   956959267,
   1178420506,
   1410419993,
   1396384771,
   387389207};
#endif
/* OLD
  {921102,
   725525,
   262144,
   74518,
   657930,
   2236963,
   394758,
   1644825,
   1644825};
*/


bool RequestAirspaceWarningDialog= false;
bool RequestAirspaceWarningForce=false;
bool                                    DisplayLocked = true;
bool                                    InfoWindowActive = true;
bool                                    EnableAuxiliaryInfo = false;
int                                     InfoBoxFocusTimeOut = 0;
int                                     MenuTimeOut = 0;
int                                     DisplayTimeOut = 0;
int                                     MenuTimeoutMax = MENUTIMEOUTMAX;


HBRUSH hBrushSelected;
HBRUSH hBrushUnselected;
HBRUSH hBrushButton;
COLORREF ColorSelected = RGB(0xC0,0xC0,0xC0);
COLORREF ColorUnselected = RGB_WHITE;
COLORREF ColorWarning = RGB_RED;
COLORREF ColorOK = RGB_BLUE;
/* Possible alternative colors for buttons:
	//COLORREF ColorButton = RGB(133,255,163); // ligher green 0
	//COLORREF ColorButton = RGB(158,231,255); // azure 2
	// COLORREF ColorButton = RGB(158,255,231); // azure 3
	// COLORREF ColorButton = RGB(158,181,255); // indigo 4
	// COLORREF ColorButton = RGB(138,255,173); // nice light green
	// COLORREF ColorButton = RGB(157,185,200); // light blue grey
	// COLORREF ColorButton = RGB(37,77,116); // dirty blue
 */
COLORREF ColorButton = RGB_BUTTONS;  

// Display Gobals
HFONT                                   InfoWindowFont;
HFONT                                   TitleWindowFont;
HFONT                                   MapWindowFont;
HFONT                                   TitleSmallWindowFont;
HFONT                                   MapWindowBoldFont;
HFONT                                   CDIWindowFont; // New
HFONT                                   MapLabelFont;
HFONT                                   StatisticsFont;

HFONT                                   LK8UnitFont;
HFONT                                   LK8TitleFont;
HFONT                                   LK8MapFont;
HFONT                                   LK8TitleNavboxFont;
HFONT                                   LK8ValueFont;
HFONT                                   LK8TargetFont;
HFONT                                   LK8BigFont;
HFONT                                   LK8MediumFont;
HFONT                                   LK8SmallFont;
HFONT					LK8SymbolFont;
HFONT					LK8InfoBigFont;
HFONT					LK8InfoBigItalicFont;
HFONT					LK8InfoNormalFont;
HFONT					LK8InfoSmallFont;
HFONT					LK8PanelBigFont;
HFONT					LK8PanelMediumFont;
HFONT					LK8PanelSmallFont;
HFONT					LK8PanelUnitFont;

LOGFONT                                   autoInfoWindowLogFont; // these are the non-custom parameters
LOGFONT                                   autoTitleWindowLogFont;
LOGFONT                                   autoMapWindowLogFont;
LOGFONT                                   autoTitleSmallWindowLogFont;
LOGFONT                                   autoMapWindowBoldLogFont;
LOGFONT                                   autoCDIWindowLogFont; // New
LOGFONT                                   autoMapLabelLogFont;
LOGFONT                                   autoStatisticsLogFont;

int  UseCustomFonts;
int                                             CurrentInfoType;
int                                             InfoFocus = 0;
int                                             DisplayOrientation = TRACKUP;
int                                             OldDisplayOrientation = TRACKUP;
int						AutoOrientScale = 10;
int                                             DisplayTextType = DISPLAYNONE;

int                                             AltitudeMode = ALLON;
int                                             ClipAltitude = 1000;
int                                             AltWarningMargin = 100;
int                                             AutoAdvance = 1;
bool                                            AdvanceArmed = false;

int                                             SafetyAltitudeMode = 0;
short	ScreenSize=0; // VENTA6

bool EnableBlockSTF = false;

bool GlobalRunning = false; 

#if defined(PNA) || defined(FIVV)  // VENTA-ADDON we call it model and not PNA for possible future usage even for custom PDAs
int	GlobalModelType=MODELTYPE_PNA_PNA;
TCHAR	GlobalModelName[MAX_PATH]; // there are currently no checks.. TODO check it fits here
float	GlobalEllipse=1.1f;	// default ellipse type VENTA2-ADDON
#endif


// this controls all displays, to make sure everything is
// properly initialised.


//SI to Local Units
double        SPEEDMODIFY = TOKNOTS;
double        LIFTMODIFY  = TOKNOTS;
double        DISTANCEMODIFY = TONAUTICALMILES;
double        ALTITUDEMODIFY = TOFEET;
double        TASKSPEEDMODIFY = TOKPH;

//Flight Data Globals
double        MACCREADY = 0; // JMW now in SI units (m/s) for consistency
double        QNH = (double)1013.25; // 100413 changed to .25
double        BUGS = 1;
double        BALLAST = 0;

bool          AutoMacCready = false;

int          NettoSpeed = 1000;

NMEA_INFO     GPS_INFO;
DERIVED_INFO  CALCULATED_INFO;

BOOL GPSCONNECT = FALSE;
BOOL extGPSCONNECT = FALSE; // this one used by external functions

bool InfoBoxesDirty= false;
bool DialogActive = false;

// 091011 Used by TakeoffLanding inside Calculation.cpp - limited values careful 
int time_in_flight=0;
int time_on_ground=0;
double TakeOffSpeedThreshold=0.0;

#if LKSTARTUP
BYTE RUN_MODE=RUN_WELCOME;
#endif

DWORD EnableFLARMMap = 1;

// 100210 Comport diagnostics, see Utils2.h
// Using 0,1, plus +1 for safety 
int ComPortStatus[NUMDEV+1];
long ComPortRx[NUMDEV+1];
long ComPortTx[NUMDEV+1];
long ComPortErrRx[NUMDEV+1];
long ComPortErrTx[NUMDEV+1];
long ComPortErrors[NUMDEV+1];
// Com ports hearth beats, based on LKHearthBeats
double ComPortHB[NUMDEV+1];

//Local Static data
static int iTimerID= 0;

// Final Glide Data
double SAFETYALTITUDEARRIVAL = 300;
double SAFETYALTITUDEBREAKOFF = 0;
double SAFETYALTITUDETERRAIN = 50;
double SAFTEYSPEED = 50.0;

// polar info
// int              POLARID = 0; REMOVE 110416
double POLAR[POLARSIZE] = {0,0,0};
double POLARV[POLARSIZE] = {21,27,40};
double POLARLD[POLARSIZE] = {33,30,20};
double WEIGHTS[POLARSIZE] = {250,70,100};
#ifdef NEW_OLC
int Handicap = 108; // LS-3
#endif /* NEW_OLC */

// Team code info
int TeamCodeRefWaypoint = -1;
TCHAR TeammateCode[10];
bool TeamFlarmTracking = false;
TCHAR TeamFlarmCNTarget[4]; // CN of the glider to track
int TeamFlarmIdTarget;      // FlarmId of the glider to track
double TeammateLatitude;
double TeammateLongitude;
bool TeammateCodeValid = false;


// Waypoint Database
WAYPOINT *WayPointList = NULL;
WPCALC *WayPointCalc = NULL; // VENTA3 additional infos calculated, parallel to WPs
unsigned int NumberOfWayPoints = 0;
int SectorType = 1; // FAI sector
DWORD SectorRadius = 500;
int StartLine = TRUE;
DWORD StartRadius = 3000;

int HomeWaypoint = -1;
bool TakeOffWayPoint=false;
int AirfieldsHomeWaypoint = -1; // VENTA3 force Airfields home to be HomeWaypoint if
                                // an H flag in waypoints file is not available..
// Alternates
int Alternate1 = -1; // VENTA3
int Alternate2 = -1; // VENTA3
int BestAlternate = -1; // VENTA3
int ActiveAlternate = -1; // VENTA3

// Specials
double GPSAltitudeOffset = 0; // VENTA3
bool	UseGeoidSeparation=false;
bool	PressureHg=false;
// 100413 shortcut on aircraft icon for ibox switching with medium click
// bool	ShortcutIbox=true;
int	CustomKeyTime=700;
int	CustomKeyModeCenter=(CustomKeyMode_t)ckDisabled;
int	CustomKeyModeLeft=(CustomKeyMode_t)ckDisabled;
int	CustomKeyModeRight=(CustomKeyMode_t)ckDisabled;
int	CustomKeyModeAircraftIcon=(CustomKeyMode_t)ckDisabled;
int	CustomKeyModeLeftUpCorner=(CustomKeyMode_t)ckDisabled;
int	CustomKeyModeRightUpCorner=(CustomKeyMode_t)ckDisabled;
bool ResumeSession=false;
double QFEAltitudeOffset = 0;
int OnAirSpace=1; // VENTA3 toggle DrawAirSpace, normal behaviour is "true"
bool WasFlying = false; // VENTA3 used by auto QFE: do not reset QFE if previously in flight. So you can check QFE
			//   on the ground, otherwise it turns to zero at once!
double LastFlipBoxTime = 0; // VENTA3 need this global for slowcalculations cycle
double LastRangeLandableTime=0;
#if defined(PNA) || defined(FIVV)
bool needclipping=false; // flag to activate extra clipping for some PNAs
#endif
bool EnableAutoBacklight=true;
bool EnableAutoSoundVolume=true;
short AircraftCategory=0;
bool ExtendedVisualGlide=false;
short Look8000=lxcAdvanced;
bool HideUnits=false;
bool VirtualKeys=false;
bool NewMap=true;
bool CheckSum=true;
short OutlinedTp=0;
int  OverColor=0;
COLORREF OverColorRef;
int  TpFilter=0;
short MapBox=0;
bool MapLock=false;
bool UseMapLock=false;
bool ActiveMap=true;
short GlideBarMode=0;
short OverlaySize=0;
short BarOpacity=255; // bottom bar transparency if available, black by default
short FontRenderer=0;
bool LockModeStatus=false;
short ArrivalValue=0;
short NewMapDeclutter=0;
short Shading=1;
bool ConfBB[10];
bool ConfIP[10][10];
bool ConfMP[10]={1,1,1,1,1,1,1,1,1,1};
bool ConfBB1=1, ConfBB2=1, ConfBB3=1, ConfBB4=1, ConfBB5=1, ConfBB6=1, ConfBB7=1, ConfBB8=1, ConfBB9=1;
bool ConfIP11=1, ConfIP12=1, ConfIP13=1, ConfIP14=1, ConfIP15=1, ConfIP16=1, ConfIP21=1, ConfIP22=1;
bool ConfIP23=1, ConfIP24=1, ConfIP31=1, ConfIP32=1;
short AverEffTime=0;
bool DrawBottom=false; // new map's bottom line in landscape mode fullscreen condition
short BottomMode=BM_FIRST; 
short BottomSize=1; // Init by MapWindow3  091213 0 to 1
short TopSize=0;
short BottomGeom=0; 
// coordinates of the sort boxes. Each mapspace can have a different layout
short SortBoxX[MSM_TOP+1][MAXSORTBOXES+1];
short SortBoxY[MSM_TOP+1];
// default initialization for gestures. InitLK8000 will fine tune it.
short GestureSize=60;
// xml dlgconfiguration value replacing 246 which became 278
int   LKwdlgConfig=0;
// normally we do it the unusual way
bool IphoneGestures=false;

int PGClimbZoom=1;
int PGCruiseZoom=1;
// This is the gauge bar on the left for variometer
int LKVarioBar=0;
// This is the value to be used for painting the bar
int LKVarioVal=0;
// moving map is all black and need white painting - not much used 091109
bool BlackScreen=false; 
// if true, LK specific text on map is painted black, otherwise white
bool LKTextBlack=false;;
// enumerated value for map background when no terrain is painted, valid for both normal and inverted mode
// note that all topology text is in black, so this should be a light colour in any case
short BgMapColor=0;
bool  BgMapColorTextBlack[LKMAXBACKGROUNDS]={ false, false, false, false, true, true, true, true, true, true };  // 101009
int LKVarioSize=2; // init by InitLK8000
// activated by Utils2 in virtual keys, used inside RenderMapWindowBg
bool PGZoomTrigger=false;
bool BestWarning=false;
bool ThermalBar=false;
bool McOverlay=true; // 101031 fixed true
bool TrackBar=false;

double WindCalcSpeed=0;
int WindCalcTime=WCALC_TIMEBACK; 
bool RepeatWindCalc=false;
// FLARM Traffic is real if <=1min, Shadow if <= etc. If >Zombie it is removed
int LKTime_Real=15, LKTime_Ghost=60, LKTime_Zombie=180;
// Copy of runtime traffic for instant use 
FLARM_TRAFFIC LKTraffic[FLARM_MAX_TRAFFIC+1];
// Number of IDs (items) of existing traffic updated from DoTraffic
int LKNumTraffic=0;
// Pointer to FLARM struct, ordered by DoTraffic, from 0 to LKNumTraffic-1
int LKSortedTraffic[FLARM_MAX_TRAFFIC+1];

// 100404 index inside FLARM_Traffic of our target, and its type as defined in Utils2
int LKTargetIndex=-1;
int LKTargetType=LKT_TYPE_NONE;

// Copy of runtime airspaces for instant use 
LKAirspace_Nearest_Item LKAirspaces[MAXNEARAIRSPACES+1];
// Number of asps (items) of existing airspaces updated from DoAirspaces
int LKNumAirspaces=0;
// Pointer to ASP struct, ordered by DoAirspaces, from 0 to LKNumAirspaces-1
int LKSortedAirspaces[MAXNEARAIRSPACES+1];

// type of file format for waypoints files
int WpFileType[3];
TCHAR WpHome_Name[NAME_SIZE+1];
double WpHome_Lat=0;
double WpHome_Lon=0;

// LK8000 Hearth beats at 2Hz
double LKHearthBeats=0;
// number of reporting messages from Portmonitor.
int PortMonitorMessages=0;

// Time in use by Nmea parser, updated realtime
double NmeaTime=0;
int NmeaHours=0, NmeaMinutes=0, NmeaSeconds=0;
bool PollingMode=false;
#if  (LK_CACHECALC && LK_CACHECALC_MCA_STAT)
int  Cache_Calls_MCA=0;
int  Cache_Hits_MCA=0;
int  Cache_Fail_MCA=0;
int  Cache_False_MCA=0;
int  Cache_Incomplete_MCA=0;
#endif
#if (LK_CACHECALC)
int  Cache_Calls_DBE=0;
int  Cache_Hits_DBE=0;
int  Cache_Fail_DBE=0;
int  Cache_False_DBE=0;
#endif

short GlideBarOffset=0;
bool	EngineeringMenu=false; // never saved to registry
short splitter=1; // 091213 0 to 1
bool iboxtoclick=true; // do a click on a new ibox focus
// it is called DeclutterMode but it has nothing to do with MapSpaces
short DeclutterMode;

// current mapspacemode: the internal identifier of a page type
// should not be used for turning pages, only for direct access
short MapSpaceMode; 
// telling you if you are in wpmode, infomode etc..
short ModeIndex;
// See Utils2.h for relationship
// pointers to MapSpacemodes
// MSM_TOP is used as max size also for each subsets
short ModeTable[LKMODE_TOP+1][MSM_TOP+1];
// top of the list inside each table. Could be a struct with ModeTable
short ModeTableTop[LKMODE_TOP+1];
// remembers for each mode (wp, infopage, map , etc.) the current type
short ModeType[LKMODE_TOP+1];

// current selected raw in mapspacemodes
short SelectedRaw[MSM_TOP+1]; 
// current page in each mapspacemode, reset entering new mapspace: no memory
// since it doesnt eat memory, it is also used for pages with currently no subpages
short SelectedPage[MSM_TOP+1];
// number of raws in mapspacemode screen
// TODO: check if they can be unsigned
short Numraws;
short CommonNumraws;
short Numpages;
short CommonNumpages;
short TrafficNumpages;
short AspNumpages;
//  mapspace sort mode: 0 wp name  1 distance  2 bearing  3 reff  4 altarr
//  UNUSED on MSM_COMMON etc. however it is dimensioned on mapspacemodes
short SortedMode[MSM_TOP+1];

TCHAR LKLangSuffix[4];
bool WarningHomeDir=false;

// Fixed Screen Parameters, initialised by InitScreen. Size
int  ScreenSizeX=0;
int  ScreenSizeY=0;
RECT ScreenSizeR;
bool ScreenLandscape=false;

// Default arrival mode calculation type
// 091016 currently not changed anymore
short AltArrivMode=ALTA_MC;

// zoomout trigger time handled by MapWindow
double  LastZoomTrigger=0;

// traffic DoTraffic interval, also reset during key up and down to prevent wrong selections
double  LastDoTraffic=0;
double  LastDoNearest=0;
double  LastDoAirspaces=0;
// double  LastDoNearestTp=0; 101222
double  LastDoCommon=0;
// double  LastDoTarget=0; unused 

// These are not globals to allow SetMapScales in Utils2 operate..
double  CruiseMapScale=1;
double  ClimbMapScale=1;

// Paraglider's time gates
// ------------------------------
// Open and close time, gate 0  ex. 12:00
// M and H for registry
int  PGOpenTimeH=0;
int  PGOpenTimeM=0;
int  PGOpenTime=0;
int  PGCloseTime=0;
// Interval, in minutes
int	PGGateIntervalTime=0;
// How many gates, 1-x
int	PGNumberOfGates=0;
// Start out or start in?
bool	PGStartOut=false;
// Current assigned gate 
int ActiveGate=-1;

// LKMAPS flag for topology: >0 means ON, and indicating how many topo files are loaded
int  LKTopo=0;
// This threshold used in Terrain.cpp to distinguish water altitude
short  LKWaterThreshold=0;
double LKTopoZoomCat05=0;
double LKTopoZoomCat10=0;
double LKTopoZoomCat20=0;
double LKTopoZoomCat30=0;
double LKTopoZoomCat40=0;
double LKTopoZoomCat50=0;
double LKTopoZoomCat60=0;
double LKTopoZoomCat70=0;
double LKTopoZoomCat80=0;
double LKTopoZoomCat90=0;
double LKTopoZoomCat100=0;
double LKTopoZoomCat110=0;
// max number of topo and wp labels painted on map, defined by default in Utils
int  LKMaxLabels=0;

// current mode of overtarget 0=task 1=alt1, 2=alt2, 3=best alt
short OvertargetMode=0;
double SimTurn=0;
double ThLatitude=1;
double ThLongitude=1;
double ThermalRadius=0;
double SinkRadius=0;

// LK8000 sync flags
bool NearestDataReady=false;
bool CommonDataReady=false;
bool RecentDataReady=false;
bool LKForceDoNearest=false;
bool LKForceDoCommon=false;
bool LKForceDoRecent=false;
short LKevent=LKEVENT_NONE;
bool LKForceComPortReset=false; 
bool LKDoNotResetComms=false;
ldrotary_s rotaryLD;
windrotary_s rotaryWind;

// Optimization  preprocessing
int  RangeLandableIndex[MAXRANGELANDABLE+1]; 
int  RangeLandableNumber=0;
int  RangeAirportIndex[MAXRANGELANDABLE+1];
int  RangeAirportNumber=0;
int  RangeTurnpointIndex[MAXRANGETURNPOINT+1];
int  RangeTurnpointNumber=0;
// This list is sorted out of RangeLandableIndex, used by DoNearest
// cannot be used elsewhere, since it's only updated when in Nearest MapSpaceMode.
// +1 is for safety...  
// Also in DoNearestTurnpoint for MSM_NEARTPS 
int  SortedLandableIndex[MAXNEAREST+1];
int  SortedAirportIndex[MAXNEAREST+1];
int  SortedTurnpointIndex[MAXNEAREST+1];
// Real number of NEAREST items contained in array after removing duplicates, or not enough to fill MAXNEAREST/MAX..
int  SortedNumber=0;

// Commons are Home, best alternate, alternate1, 2, and task waypoints , all up to MAXCOMMON.
// It is reset when changing wp file
int  CommonIndex[MAXCOMMON+1];
// Number of items 0-n inside CommonIndex
int  CommonNumber=0;

// History of recent waypoints
int RecentIndex[MAXCOMMON+1];
unsigned int RecentChecksum[MAXCOMMON+1];
int RecentNumber=0;
// Cpu stats
#ifdef CPUSTATS
int Cpu_Draw=0;
int Cpu_Calc=0;
int Cpu_Instrument=0;
int Cpu_Port=0;
int Cpu_Aver=0;
#endif

#ifdef NEWIBLSCALE
int LKIBLSCALE[MAXIBLSCALE+1];
#endif
double Experimental1=0, Experimental2=0;

double NearestAirspaceHDist=-1;
double NearestAirspaceVDist=0;
TCHAR NearestAirspaceName[NAME_SIZE+1] = {0};
#ifdef LKAIRSPACE
TCHAR NearestAirspaceVName[NAME_SIZE+1] = {0};
#endif

// Flarmnet tools
int FlarmNetCount=0;

// Airspace Database
#ifndef LKAIRSPACE
AIRSPACE_AREA *AirspaceArea = NULL;
AIRSPACE_POINT *AirspacePoint = NULL;
POINT *AirspaceScreenPoint = NULL;
AIRSPACE_CIRCLE *AirspaceCircle = NULL;
unsigned int NumberOfAirspacePoints = 0;
unsigned int NumberOfAirspaceAreas = 0;
unsigned int NumberOfAirspaceCircles = 0;
#endif

//Airspace Warnings
int AIRSPACEWARNINGS = TRUE;
int WarningTime = 60;
int AcknowledgementTime = 900;                  // keep ack level for this time, [secs]
#ifdef LKAIRSPACE
int AirspaceWarningRepeatTime = 300;			// warning repeat time if not acknowledged after 5 minutes
int AirspaceWarningVerticalMargin = 100;		// vertical distance used to calculate too close condition
int AirspaceWarningDlgTimeout = 30;             // airspace warning dialog auto closing in x secs
int AirspaceWarningMapLabels = 1;               // airspace warning map labels showed
#endif

// Registration Data
TCHAR strAssetNumber[MAX_LOADSTRING] = TEXT(""); //4G17DW31L0HY");
TCHAR strRegKey[MAX_LOADSTRING] = TEXT("");

// Interface Files
StatusMessageSTRUCT StatusMessageData[MAXSTATUSMESSAGECACHE];
int StatusMessageData_Size = 0;

//Snail Trial
SNAIL_POINT SnailTrail[TRAILSIZE];
int SnailNext = 0;

#ifdef NEW_OLC
// OLC COOKED VALUES
CContestMgr::CResult OlcResults[CContestMgr::TYPE_NUM];
#endif

// user interface settings
int WindUpdateMode = 0;
bool EnableTopology = true; // 091105
bool EnableTerrain = true;  // 091105
int FinalGlideTerrain = 1;
bool EnableSoundVario = true;
bool EnableSoundModes = true;
bool EnableSoundTask = true;
bool OverlayClock = false;
int SoundVolume = 80;
int SoundDeadband = 5;
bool EnableVarioGauge = false;
bool EnableAutoBlank = false;
bool ScreenBlanked = false;
bool LKLanguageReady = false;


//IGC Logger
bool LoggerActive = false;

// Others

BOOL COMPORTCHANGED = FALSE;
BOOL MAPFILECHANGED = FALSE;
BOOL AIRSPACEFILECHANGED = FALSE;
BOOL AIRFIELDFILECHANGED = FALSE;
BOOL WAYPOINTFILECHANGED = FALSE;
BOOL TERRAINFILECHANGED = FALSE;
BOOL TOPOLOGYFILECHANGED = FALSE;
BOOL POLARFILECHANGED = FALSE;
BOOL LANGUAGEFILECHANGED = FALSE;
BOOL STATUSFILECHANGED = FALSE;
BOOL INPUTFILECHANGED = FALSE;
static bool MenuActive = false;

//Task Information
Task_t Task = {{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0},{-1,0,0,0,0,0,0,0,0}};
Start_t StartPoints;
TaskStats_t TaskStats;
int ActiveWayPoint = -1;

// Assigned Area Task
double AATTaskLength = 120;
BOOL AATEnabled = FALSE;
DWORD FinishMinHeight = 0;
DWORD StartMaxHeight = 0;
DWORD StartMaxSpeed = 0;
DWORD StartMaxHeightMargin = 0;
DWORD StartMaxSpeedMargin = 0;


// Statistics
Statistics flightstats;

#if (((UNDER_CE >= 300)||(_WIN32_WCE >= 0x0300)) && (WINDOWSPC<1))
#define HAVE_ACTIVATE_INFO
static SHACTIVATEINFO s_sai;
static bool api_has_SHHandleWMActivate = false;
static bool api_has_SHHandleWMSettingChange = false;
#endif

BOOL InfoBoxesHidden = false; 

void PopupBugsBallast(int updown);

// System boot specific flags 
// Give me a go/no-go 
bool goInstallSystem=false;
bool goCalculationThread=false;
bool goInstrumentThread=false;
bool goInitDevice=false; 
// bool goCalculating=false;

#include "GaugeCDI.h"
#include "GaugeFLARM.h"

// Battery status for SIMULATOR mode
//	30% reminder, 20% exit, 30 second reminders on warnings

#ifndef GNAV
#define BATTERY_WARNING 30
#define BATTERY_EXIT 20
#define BATTERY_REMINDER 30000
DWORD BatteryWarningTime = 0;
#endif

char dedicated[]="Dedicated to my father Vittorio";

#define NUMSELECTSTRING_MAX			130
SCREEN_INFO Data_Options[NUMSELECTSTRING_MAX];
int NUMSELECTSTRINGS = 0;


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
void                                                    PopUpSelect(int i);

//HWND CreateRpCommandBar(HWND hwnd);

#ifdef DEBUG
void                                            DebugStore(char *Str);
#endif

bool SetDataOption( int index,
					UnitGroup_t UnitGroup,
					TCHAR *Description,
					TCHAR *Title,
					InfoBoxFormatter *Formatter,
					void (*Process)(int UpDown),
					char next_screen,
					char prev_screen)
{
	SCREEN_INFO tag;
	if (index>=NUMSELECTSTRING_MAX) return false;

	tag.UnitGroup = UnitGroup;
	_tcsncpy(tag.Description, gettext(Description), DESCRIPTION_SIZE); 
	tag.Description[DESCRIPTION_SIZE] = 0;		// buff allocated to DESCRIPITON_SIZE+1
	_tcsncpy(tag.Title, gettext(Title), TITLE_SIZE);
	tag.Title[TITLE_SIZE] = 0;;					// buff allocated to TITLE_SIZE+1
	tag.Formatter = Formatter;
	tag.Process = Process;
	tag.next_screen = next_screen;
	tag.prev_screen = prev_screen;

	memcpy(&Data_Options[index], &tag, sizeof(SCREEN_INFO));
	if (NUMSELECTSTRINGS<=index) NUMSELECTSTRINGS=index+1;				//No. of items = max index+1

#ifdef DEBUG
	DebugStore(TEXT("Add data option #%d %s%s"),index,tag.Description,NEWLINE);
#endif
	return true;
}



void FillDataOptions()
{
	// Groups:
	//   Altitude 0,1,20,33
	//   Aircraft info 3,6,23,32,37,47,54
	//   LD 4,5,19,38,53, 66    VENTA-ADDON added 66 for GR final
	//   Vario 2,7,8,9,21,22,24,44
	//   Wind 25,26,48,49,50
	//   Mcready 10,34,35,43
	//   Nav 11,12,13,15,16,17,18,27,28,29,30,31
	//   Waypoint 14,36,39,40,41,42,45,46

	// LKTOKEN  _@M1001_ = "Altitude QNH", _@M1002_ = "Alt"
	SetDataOption(0, ugAltitude,		TEXT("_@M1001_"), TEXT("_@M1002_"), new InfoBoxFormatter(TEXT("%2.0f")), AltitudeProcessing, 1, 33);
	// LKTOKEN  _@M1003_ = "Altitude AGL", _@M1004_ = "HAGL"
	SetDataOption(1, ugAltitude,		TEXT("_@M1003_"), TEXT("_@M1004_"), new FormatterLowWarning(TEXT("%2.0f"),0.0), NoProcessing, 20, 0);
	// LKTOKEN  _@M1005_ = "Thermal last 30 sec", _@M1006_ = "TC.30\""
	SetDataOption(2, ugVerticalSpeed,	TEXT("_@M1005_"), TEXT("_@M1006_"), new FormatterLowWarning(TEXT("%-2.1f"),0.0), NoProcessing, 7, 44);
	// LKTOKEN  _@M1007_ = "Bearing", _@M1008_ = "Brg"
#ifdef FIVV
	SetDataOption(3, ugNone,			TEXT("_@M1007_"), TEXT("_@M1008_"), new InfoBoxFormatter(TEXT("%2.0f")TEXT(DEG)), NoProcessing, 6, 54);
#else
	SetDataOption((3, ugNone,			TEXT("_@M1007_"), TEXT("_@M1008_"), new InfoBoxFormatter(TEXT("%2.0f")TEXT(DEG)TEXT("T")), NoProcessing, 6, 54);
#endif
	// LKTOKEN  _@M1009_ = "Eff.last 20 sec", _@M1010_ = "E.20\""
	SetDataOption(4, ugNone,			TEXT("_@M1009_"), TEXT("_@M1010_"), new InfoBoxFormatter(TEXT("%2.0f")), PopupBugsBallast, 5, 38);
	// LKTOKEN  _@M1011_ = "Eff.cruise last therm", _@M1012_ = "E.Cru"
	SetDataOption(5, ugNone,            TEXT("_@M1011_"), TEXT("_@M1012_"), new InfoBoxFormatter(TEXT("%2.0f")), PopupBugsBallast, 19, 4);
	// LKTOKEN  _@M1013_ = "Speed ground", _@M1014_ = "GS"
	SetDataOption(6, ugHorizontalSpeed, TEXT("_@M1013_"), TEXT("_@M1014_"), new InfoBoxFormatter(TEXT("%2.0f")), SpeedProcessing, 23, 3);
	// LKTOKEN  _@M1015_ = "Thermal Average Last", _@M1016_ = "TL.Avg"
	SetDataOption(7, ugVerticalSpeed,   TEXT("_@M1015_"), TEXT("_@M1016_"), new InfoBoxFormatter(TEXT("%-2.1f")), NoProcessing, 8, 2);
	// LKTOKEN  _@M1017_ = "Thermal Gain Last", _@M1018_ = "TL.Gain"
	SetDataOption(8, ugAltitude,        TEXT("_@M1017_"), TEXT("_@M1018_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 9, 7);
	// LKTOKEN  _@M1019_ = "Thermal Time Last", _@M1020_ = "TL.Time"
	SetDataOption(9, ugNone,            TEXT("_@M1019_"), TEXT("_@M1020_"), new FormatterTime(TEXT("%04.0f")), NoProcessing, 21, 8);
	// LKTOKEN  _@M1021_ = "MacCready Setting", _@M1022_ = "MCready"
	SetDataOption(10, ugVerticalSpeed,  TEXT("_@M1021_"), TEXT("_@M1022_"), new InfoBoxFormatter(TEXT("%2.1f")), MacCreadyProcessing, 34, 43);
	// LKTOKEN  _@M1023_ = "Next Distance", _@M1024_ = "Dist"
	SetDataOption(11, ugDistance,       TEXT("_@M1023_"), TEXT("_@M1024_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 12, 31);
	// LKTOKEN  _@M1025_ = "Next Alt.Arrival", _@M1026_ = "NxtArr"
	SetDataOption(12, ugAltitude,       TEXT("_@M1025_"), TEXT("_@M1026_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 13, 11);
	// LKTOKEN  _@M1027_ = "Next Alt.Required", _@M1028_ = "NxtAltR"
	SetDataOption(13, ugAltitude,       TEXT("_@M1027_"), TEXT("_@M1028_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 15, 12);
	// LKTOKEN  _@M1029_ = "Next Waypoint", _@M1030_ = "Next"
	SetDataOption(14, ugNone,           TEXT("_@M1029_"), TEXT("_@M1030_"), new FormatterWaypoint(TEXT("\0")), NextUpDown, 36, 46);
	// LKTOKEN  _@M1031_ = "Task Alt.Arrival", _@M1032_ = "TskArr"
	SetDataOption(15, ugAltitude,       TEXT("_@M1031_"), TEXT("_@M1032_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 16, 13);
	// LKTOKEN  _@M1033_ = "Task Alt.Required", _@M1034_ = "TskAltR"
	SetDataOption(16, ugAltitude,       TEXT("_@M1033_"), TEXT("_@M1034_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 17, 15);
	// LKTOKEN  _@M1035_ = "Task Speed Average", _@M1036_ = "TskSpAv"
	SetDataOption(17, ugTaskSpeed,		TEXT("_@M1035_"), TEXT("_@M1036_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 18, 16);
	// LKTOKEN  _@M1037_ = "Task Distance", _@M1038_ = "TskDis"
	SetDataOption(18, ugDistance,       TEXT("_@M1037_"), TEXT("_@M1038_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 27, 17);
	// LKTOKEN  _@M1039_ = "_Reserved 1", _@M1040_ = "OLD fLD"
	SetDataOption(19, ugNone,           TEXT("_@M1039_"), TEXT("_@M1040_"), new InfoBoxFormatter(TEXT("%1.0f")), NoProcessing, 38, 5);
	// LKTOKEN  _@M1041_ = "Terrain Elevation", _@M1042_ = "Gnd"
	SetDataOption(20, ugAltitude,       TEXT("_@M1041_"), TEXT("_@M1042_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 33, 1);
	// LKTOKEN  _@M1043_ = "Thermal Average", _@M1044_ = "TC.Avg"
	SetDataOption(21, ugVerticalSpeed,  TEXT("_@M1043_"), TEXT("_@M1044_"), new FormatterLowWarning(TEXT("%-2.1f"),0.0), NoProcessing, 22, 9);
	// LKTOKEN  _@M1045_ = "Thermal Gain", _@M1046_ = "TC.Gain"
	SetDataOption(22, ugAltitude,       TEXT("_@M1045_"), TEXT("_@M1046_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 24, 21);
	// LKTOKEN  _@M1047_ = "Track", _@M1048_ = "Track"
#ifdef FIVV
	SetDataOption(23, ugNone,           TEXT("_@M1047_"), TEXT("_@M1048_"), new InfoBoxFormatter(TEXT("%2.0f")TEXT(DEG)), DirectionProcessing, 32, 6);
#else
	SetDataOption(23, ugNone,           TEXT("_@M1047_"), TEXT("_@M1048_"), new InfoBoxFormatter(TEXT("%2.0f")TEXT(DEG)TEXT("T")), DirectionProcessing, 32, 6);
#endif
	// LKTOKEN  _@M1049_ = "Vario", _@M1050_ = "Vario"
	SetDataOption(24, ugVerticalSpeed,  TEXT("_@M1049_"), TEXT("_@M1050_"), new InfoBoxFormatter(TEXT("%-2.1f")), NoProcessing, 44, 22);
	// LKTOKEN  _@M1051_ = "Wind Speed", _@M1052_ = "WindV"
	SetDataOption(25, ugWindSpeed,      TEXT("_@M1051_"), TEXT("_@M1052_"), new InfoBoxFormatter(TEXT("%2.0f")), WindSpeedProcessing, 26, 50);
	// LKTOKEN  _@M1053_ = "Wind Bearing", _@M1054_ = "WindB"
#ifdef FIVV
	SetDataOption(26, ugNone,           TEXT("_@M1053_"), TEXT("_@M1054_"), new InfoBoxFormatter(TEXT("%2.0f")TEXT(DEG)), WindDirectionProcessing, 48, 25);
#else
	SetDataOption(26, ugNone,           TEXT("_@M1053_"), TEXT("_@M1054_"), new InfoBoxFormatter(TEXT("%2.0f")TEXT(DEG)TEXT("T")), WindDirectionProcessing, 48, 25);
#endif
	// LKTOKEN  _@M1055_ = "AA Time", _@M1056_ = "AATime"
	SetDataOption(27, ugNone,           TEXT("_@M1055_"), TEXT("_@M1056_"), new FormatterAATTime(TEXT("%2.0f")), NoProcessing, 28, 18);
	// LKTOKEN  _@M1057_ = "AA Distance Max", _@M1058_ = "AADmax"
	SetDataOption(28, ugDistance,       TEXT("_@M1057_"), TEXT("_@M1058_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 29, 27);
	// LKTOKEN  _@M1059_ = "AA Distance Min", _@M1060_ = "AADmin"
	SetDataOption(29, ugDistance,       TEXT("_@M1059_"), TEXT("_@M1060_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 30, 28);
	// LKTOKEN  _@M1061_ = "AA Speed Max", _@M1062_ = "AAVmax"
	SetDataOption(30, ugTaskSpeed,      TEXT("_@M1061_"), TEXT("_@M1062_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 31, 29);
	// LKTOKEN  _@M1063_ = "AA Speed Min", _@M1064_ = "AAVmin"
	SetDataOption(31, ugTaskSpeed,      TEXT("_@M1063_"), TEXT("_@M1064_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 51, 30);
	// LKTOKEN  _@M1065_ = "Airspeed IAS", _@M1066_ = "IAS"
	SetDataOption(32, ugHorizontalSpeed,TEXT("_@M1065_"), TEXT("_@M1066_"), new InfoBoxFormatter(TEXT("%2.0f")), AirspeedProcessing, 37, 23);
	// LKTOKEN  _@M1067_ = "Altitude BARO", _@M1068_ = "HBAR"
	SetDataOption(33, ugAltitude,       TEXT("_@M1067_"), TEXT("_@M1068_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 0, 20);
	// LKTOKEN  _@M1069_ = "Speed MacReady", _@M1070_ = "SpMc"
	SetDataOption(34, ugHorizontalSpeed,TEXT("_@M1069_"), TEXT("_@M1070_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 35, 10);
	// LKTOKEN  _@M1071_ = "Percentage clim", _@M1072_ = "%Climb"
	SetDataOption(35, ugNone,           TEXT("_@M1071_"), TEXT("_@M1072_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 43, 34);
	// LKTOKEN  _@M1073_ = "Time of flight", _@M1074_ = "FlyTime"
	SetDataOption(36, ugNone,           TEXT("_@M1073_"), TEXT("_@M1074_"), new FormatterTime(TEXT("%04.0f")), NoProcessing, 39, 14);
	// LKTOKEN  _@M1075_ = "G load", _@M1076_ = "G"
	SetDataOption(37, ugNone,           TEXT("_@M1075_"), TEXT("_@M1076_"), new InfoBoxFormatter(TEXT("%2.2f")), AccelerometerProcessing, 47, 32);
	// LKTOKEN  _@M1077_ = "_Reserved 2", _@M1078_ = "OLD nLD"
	SetDataOption(38, ugNone,           TEXT("_@M1077_"), TEXT("_@M1078_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 53, 19);
	// LKTOKEN  _@M1079_ = "Time local", _@M1080_ = "Time"
	SetDataOption(39, ugNone,           TEXT("_@M1079_"), TEXT("_@M1080_"), new FormatterTime(TEXT("%04.0f")), NoProcessing, 40, 36);
	// LKTOKEN  _@M1081_ = "Time UTC", _@M1082_ = "UTC"
	SetDataOption(40, ugNone,           TEXT("_@M1081_"), TEXT("_@M1082_"), new FormatterTime(TEXT("%04.0f")), NoProcessing, 41, 39);
	// LKTOKEN  _@M1083_ = "Task Time To Go", _@M1084_ = "TskETE"
	SetDataOption(41, ugNone,           TEXT("_@M1083_"), TEXT("_@M1084_"), new FormatterAATTime(TEXT("%04.0f")), NoProcessing, 42, 40);
	// LKTOKEN  _@M1085_ = "Next Time To Go", _@M1086_ = "NextETE"
	SetDataOption(42, ugNone,           TEXT("_@M1085_"), TEXT("_@M1086_"), new FormatterAATTime(TEXT("%04.0f")), NoProcessing, 45, 41);
	// LKTOKEN  _@M1087_ = "Speed To Fly", _@M1088_ = "STF"
	SetDataOption(43, ugHorizontalSpeed,TEXT("_@M1087_"), TEXT("_@M1088_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 10, 35);
	// LKTOKEN  _@M1089_ = "Netto Vario", _@M1090_ = "Netto"
	SetDataOption(44, ugVerticalSpeed,  TEXT("_@M1089_"), TEXT("_@M1090_"), new InfoBoxFormatter(TEXT("%-2.1f")), NoProcessing, 2, 24);
	// LKTOKEN  _@M1091_ = "Task Arrival Time", _@M1092_ = "TskETA"
	SetDataOption(45, ugNone,           TEXT("_@M1091_"), TEXT("_@M1092_"), new FormatterAATTime(TEXT("%04.0f")), NoProcessing, 46, 42);
	// LKTOKEN  _@M1093_ = "Next Arrival Time", _@M1094_ = "NextETA"
	SetDataOption(46, ugNone,           TEXT("_@M1093_"), TEXT("_@M1094_"), new FormatterTime(TEXT("%04.0f")), NoProcessing, 14, 45);
	// LKTOKEN  _@M1095_ = "Bearing Difference", _@M1096_ = "To"
	SetDataOption(47, ugNone,           TEXT("_@M1095_"), TEXT("_@M1096_"), new FormatterDiffBearing(TEXT("")), NoProcessing, 54, 37);
	// LKTOKEN  _@M1097_ = "Outside Air Temperature", _@M1098_ = "OAT"
	SetDataOption(48, ugNone,           TEXT("_@M1097_"), TEXT("_@M1098_"), new InfoBoxFormatter(TEXT("%2.1f")TEXT(DEG)), NoProcessing, 49, 26);
	// LKTOKEN  _@M1099_ = "Relative Humidity", _@M1100_ = "RelHum"
	SetDataOption(49, ugNone,           TEXT("_@M1099_"), TEXT("_@M1100_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 50, 48);
	// LKTOKEN  _@M1101_ = "Forecast Temperature", _@M1102_ = "MaxTemp"
	SetDataOption(50, ugNone,           TEXT("_@M1101_"), TEXT("_@M1102_"), new InfoBoxFormatter(TEXT("%2.1f")TEXT(DEG)), ForecastTemperatureProcessing, 49, 25);
	// LKTOKEN  _@M1103_ = "AA Distance Tg", _@M1104_ = "AADtgt"
	SetDataOption(51, ugDistance,       TEXT("_@M1103_"), TEXT("_@M1104_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 52, 31);
	// LKTOKEN  _@M1105_ = "AA Speed Tg", _@M1106_ = "AAVtgt"
	SetDataOption(52, ugTaskSpeed, TEXT("_@M1105_"), TEXT("_@M1106_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 11, 51);
	// LKTOKEN  _@M1107_ = "L/D vario", _@M1108_ = "L/D vario"
	SetDataOption(53, ugNone,           TEXT("_@M1107_"), TEXT("_@M1108_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 4, 38);
	// LKTOKEN  _@M1109_ = "Airspeed TAS", _@M1110_ = "TAS"
	SetDataOption(54, ugHorizontalSpeed,TEXT("_@M1109_"), TEXT("_@M1110_"), new InfoBoxFormatter(TEXT("%2.0f")), AirspeedProcessing, 3, 47);
	// LKTOKEN  _@M1111_ = "Team Code", _@M1112_ = "TeamCode"
	SetDataOption(55, ugNone,           TEXT("_@M1111_"), TEXT("_@M1112_"), new FormatterTeamCode(TEXT("\0")), TeamCodeProcessing, 56, 54);
	// LKTOKEN  _@M1113_ = "Team Bearing", _@M1114_ = "TmBrng"
#ifdef FIVV
	SetDataOption(56, ugNone,           TEXT("_@M1113_"), TEXT("_@M1114_"), new InfoBoxFormatter(TEXT("%2.0f")TEXT(DEG)), NoProcessing, 57, 55);
#else
	SetDataOption(56, ugNone,           TEXT("_@M1113_"), TEXT("_@M1114_"), new InfoBoxFormatter(TEXT("%2.0f")TEXT(DEG)TEXT("T")), NoProcessing, 57, 55);
#endif
	// LKTOKEN  _@M1115_ = "Team Bearing Diff", _@M1116_ = "TeamBd"
	SetDataOption(57, ugNone,           TEXT("_@M1115_"), TEXT("_@M1116_"), new FormatterDiffTeamBearing(TEXT("")), NoProcessing, 58, 56);
	// LKTOKEN  _@M1117_ = "Team Range", _@M1118_ = "TeamDis"
	SetDataOption(58, ugNone,           TEXT("_@M1117_"), TEXT("_@M1118_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 55, 57);
	// LKTOKEN  _@M1119_ = "Task Speed Instantaneous", _@M1120_ = "TskSpI"
	SetDataOption(59, ugTaskSpeed,      TEXT("_@M1119_"), TEXT("_@M1120_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 18, 16);
	// LKTOKEN  _@M1121_ = "Home Distance", _@M1122_ = "HomeDis"
	SetDataOption(60, ugDistance,       TEXT("_@M1121_"), TEXT("_@M1122_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 18, 16);
	// LKTOKEN  _@M1123_ = "Task Speed", _@M1124_ = "TskSp"
	SetDataOption(61, ugTaskSpeed,      TEXT("_@M1123_"), TEXT("_@M1124_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 18, 16);
	// LKTOKEN  _@M1125_ = "AA Delta Time", _@M1126_ = "AAdT"
	SetDataOption(62, ugNone,           TEXT("_@M1125_"), TEXT("_@M1126_"), new FormatterAATTime(TEXT("%2.0f")), NoProcessing, 28, 18);
	// LKTOKEN  _@M1127_ = "Thermal All", _@M1128_ = "Th.All"
	SetDataOption(63, ugVerticalSpeed,  TEXT("_@M1127_"), TEXT("_@M1128_"), new InfoBoxFormatter(TEXT("%-2.1f")), NoProcessing, 8, 2);
	// LKTOKEN  _@M1129_ = "Distance Vario", _@M1130_ = "DVario"
	SetDataOption(64, ugVerticalSpeed,  TEXT("_@M1129_"), TEXT("_@M1130_"), new InfoBoxFormatter(TEXT("%-2.1f")), NoProcessing, 8, 2);
#ifndef GNAV
	// LKTOKEN  _@M1131_ = "Battery Percent", _@M1132_ = "Battery"
	SetDataOption(65, ugNone,           TEXT("_@M1131_"), TEXT("_@M1132_"), new InfoBoxFormatter(TEXT("%2.0f%%")), NoProcessing, 49, 26);
#else
	// LKTOKEN  _@M1181_ = "Battery Voltage", _@M1182_ = "Batt"
	SetDataOption(65, ugNone,           TEXT("_@M1181_"), TEXT("_@M1182_"), new InfoBoxFormatter(TEXT("%2.1fV")), NoProcessing, 49, 26);
#endif
	// LKTOKEN  _@M1133_ = "Task Req.Efficiency", _@M1134_ = "TskReqE"
	SetDataOption(66, ugNone,           TEXT("_@M1133_"), TEXT("_@M1134_"), new InfoBoxFormatter(TEXT("%1.1f")), NoProcessing, 38, 5);
	// LKTOKEN  _@M1135_ = "Alternate1 Req.Efficiency", _@M1136_ = "Atn1.E"
	SetDataOption(67, ugNone,           TEXT("_@M1135_"), TEXT("_@M1136_"), new FormatterAlternate(TEXT("\0")), Alternate1Processing, 36, 46);
	// LKTOKEN  _@M1137_ = "Alternate2 Req.Efficiency", _@M1138_ = "Atn2.E"
	SetDataOption(68, ugNone,           TEXT("_@M1137_"), TEXT("_@M1138_"), new FormatterAlternate(TEXT("\0")), Alternate2Processing, 36, 46);
	// LKTOKEN  _@M1139_ = "BestAltern Req.Efficiency", _@M1140_ = "BAtn.E"
	SetDataOption(69, ugNone,           TEXT("_@M1139_"), TEXT("_@M1140_"), new FormatterAlternate(TEXT("\0")), BestAlternateProcessing, 36, 46);
	// LKTOKEN  _@M1141_ = "Altitude QFE", _@M1142_ = "QFE"
	SetDataOption(70, ugAltitude,       TEXT("_@M1141_"), TEXT("_@M1142_"), new InfoBoxFormatter(TEXT("%2.0f")), QFEAltitudeProcessing, 1, 33);
	// LKTOKEN  _@M1143_ = "Average Efficiency", _@M1144_ = "E.Avg"
	SetDataOption(71, ugNone,           TEXT("_@M1143_"), TEXT("_@M1144_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 19, 4);
	// LKTOKEN  _@M1145_ = "Next Req.Efficiency", _@M1146_ = "Req.E"
	SetDataOption(72, ugNone,           TEXT("_@M1145_"), TEXT("_@M1146_"), new InfoBoxFormatter(TEXT("%1.1f")), NoProcessing, 38, 5);
	// LKTOKEN  _@M1147_ = "Flight Level", _@M1148_ = "FL"
	SetDataOption(73, ugNone,           TEXT("_@M1147_"), TEXT("_@M1148_"), new InfoBoxFormatter(TEXT("%1.1f")), NoProcessing, 38, 5);
	// LKTOKEN  _@M1149_ = "Task Covered distance", _@M1150_ = "TskCov"
	SetDataOption(74, ugDistance,       TEXT("_@M1149_"), TEXT("_@M1150_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 38, 5);
	// LKTOKEN  _@M1151_ = "Alternate1 Arrival", _@M1152_ = "Atn1Arr"
	SetDataOption(75, ugAltitude,       TEXT("_@M1151_"), TEXT("_@M1152_"), new FormatterAlternate(TEXT("%2.0f")), Alternate1Processing, 36, 46);
	// LKTOKEN  _@M1153_ = "Alternate2 Arrival", _@M1154_ = "Atn2Arr"
	SetDataOption(76, ugAltitude,       TEXT("_@M1153_"), TEXT("_@M1154_"), new FormatterAlternate(TEXT("%2.0f")), Alternate2Processing, 36, 46);
	// LKTOKEN  _@M1155_ = "BestAlternate Arrival", _@M1156_ = "BAtnArr"
	SetDataOption(77, ugAltitude,       TEXT("_@M1155_"), TEXT("_@M1156_"), new FormatterAlternate(TEXT("%2.0f")), BestAlternateProcessing, 36, 46);
	// LKTOKEN  _@M1157_ = "Home Radial", _@M1158_ = "Radial"
	SetDataOption(78, ugNone,           TEXT("_@M1157_"), TEXT("_@M1158_"), new InfoBoxFormatter(TEXT("%.0f")TEXT(DEG)), NoProcessing, 6, 54);
	// LKTOKEN  _@M1159_ "Airspace Horizontal Dist", _@M1160_ "ArSpcH"
	SetDataOption(79, ugDistance,       TEXT("_@M1159_"), TEXT("_@M1160_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 38, 5);
	// LKTOKEN  _@M1161_ = "Ext.Batt.Bank", _@M1162_ = "xBnk#"
	SetDataOption(80, ugNone,           TEXT("_@M1161_"), TEXT("_@M1162_"), new InfoBoxFormatter(TEXT("%1.0f")), NoProcessing, 38, 5);
	// LKTOKEN  _@M1163_ = "Ext.Batt.1 Voltage", _@M1164_ = "xBat1"
	SetDataOption(81, ugNone,           TEXT("_@M1163_"), TEXT("_@M1164_"), new InfoBoxFormatter(TEXT("%2.1fV")), NoProcessing, 49, 26);
	// LKTOKEN  _@M1165_ = "Ext.Batt.2 Voltage", _@M1166_ = "xBat2"
	SetDataOption(82, ugNone,           TEXT("_@M1165_"), TEXT("_@M1166_"), new InfoBoxFormatter(TEXT("%2.1fV")), NoProcessing, 49, 26);
	// LKTOKEN  _@M1167_ = "Odometer", _@M1168_ = "Odo"
	SetDataOption(83, ugDistance,       TEXT("_@M1167_"), TEXT("_@M1168_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 49, 26);
	// LKTOKEN  _@M1169_ = "Altern QNH", _@M1170_ = "aAlt"
	SetDataOption(84, ugInvAltitude,    TEXT("_@M1169_"), TEXT("_@M1170_"), new InfoBoxFormatter(TEXT("%2.0f")), AltitudeProcessing, 1, 33);
	// LKTOKEN  _@M1171_ = "Altern AGL", _@M1172_ = "aHAGL"
	SetDataOption(85, ugInvAltitude,    TEXT("_@M1171_"), TEXT("_@M1172_"), new FormatterLowWarning(TEXT("%2.0f"),0.0), NoProcessing, 20, 0);
	// LKTOKEN  _@M1173_ = "Altitude GPS", _@M1174_ = "HGPS"
	SetDataOption(86, ugAltitude,       TEXT("_@M1173_"), TEXT("_@M1174_"), new InfoBoxFormatter(TEXT("%2.0f")), AltitudeProcessing, 1, 33);
	// LKTOKEN  _@M1175_ = "MacCready Equivalent", _@M1176_ = "eqMC"
	SetDataOption(87, ugVerticalSpeed,  TEXT("_@M1175_"), TEXT("_@M1176_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 34, 43);
	// LKTOKEN  _@M1177_ = "_Experimental1", _@M1178_ = "Exp1"
	SetDataOption(88, ugNone,           TEXT("_@M1177_"), TEXT("_@M1178_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// LKTOKEN  _@M1179_ = "_Experimental2", _@M1180_ = "Exp2"
	SetDataOption(89, ugNone,           TEXT("_@M1179_"), TEXT("_@M1180_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);

	// Distance OLC
	SetDataOption(90, ugNone,           TEXT("_@M1455_"), TEXT("_@M1456_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Distance FAI triangle
	SetDataOption(91, ugNone,           TEXT("_@M1457_"), TEXT("_@M1458_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Distance League
	SetDataOption(92, ugNone,           TEXT("_@M1459_"), TEXT("_@M1460_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Distance FAI 3 TPs
	SetDataOption(93, ugNone,           TEXT("_@M1461_"), TEXT("_@M1462_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	
	////////  PREDICTED ////////
	// Distance OLC
	SetDataOption(94, ugNone,           TEXT("_@M1463_"), TEXT("_@M1464_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Distance FAI triangle
	SetDataOption(95, ugNone,           TEXT("_@M1465_"), TEXT("_@M1466_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Distance FAI 3 TPs
	SetDataOption(96, ugNone,           TEXT("_@M1469_"), TEXT("_@M1470_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	
	// Speed OLC
	SetDataOption(97, ugNone,           TEXT("_@M1471_"), TEXT("_@M1472_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Speed FAI triangle
	SetDataOption(98, ugNone,           TEXT("_@M1473_"), TEXT("_@M1474_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Speed League
	SetDataOption(99, ugNone,           TEXT("_@M1475_"), TEXT("_@M1476_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Speed FAI 3 TPs
	SetDataOption(100, ugNone,           TEXT("_@M1477_"), TEXT("_@M1478_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);

	////////  PREDICTED ////////
	// Speed OLC
	SetDataOption(101, ugNone,           TEXT("_@M1479_"), TEXT("_@M1480_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Speed FAI triangle
	SetDataOption(102, ugNone,           TEXT("_@M1481_"), TEXT("_@M1482_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Speed FAI 3 TPs
	SetDataOption(103, ugNone,           TEXT("_@M1485_"), TEXT("_@M1486_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);

	// Score OLC
	SetDataOption(104, ugNone,           TEXT("_@M1487_"), TEXT("_@M1488_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Score FAI triangle
	SetDataOption(105, ugNone,           TEXT("_@M1489_"), TEXT("_@M1490_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Score League
	SetDataOption(106, ugNone,           TEXT("_@M1491_"), TEXT("_@M1492_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// FAI 3 TPs currently has no score
	SetDataOption(107, ugNone,           TEXT("_Reserved 3"), TEXT("_@M1494_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);

	////////  PREDICTED ////////
	// Score OLC
	SetDataOption(108, ugNone,           TEXT("_@M1495_"), TEXT("_@M1496_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Score FAI triangle
	SetDataOption(109, ugNone,           TEXT("_@M1497_"), TEXT("_@M1498_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// FAI 3 TPs currently has no score
	SetDataOption(110, ugNone,           TEXT("_Reserved 4"), TEXT("_@M1502_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);

	// Olc Plus Score
	SetDataOption(111, ugNone,           TEXT("_@M1503_"), TEXT("_@M1504_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);
	// Olc Plus Score Predicted
	SetDataOption(112, ugNone,           TEXT("_@M1505_"), TEXT("_@M1506_"), new InfoBoxFormatter(TEXT("%2.1f")), NoProcessing, 8, 2);

  // Flaps
  SetDataOption(113, ugNone,           TEXT("_@M1640_"), TEXT("_@M1641_"), new InfoBoxFormatter(TEXT("")), AirspeedProcessing, 8, 2);

  // Vertical distance to airspace
  SetDataOption(114, ugNone,           TEXT("_@M1285_"), TEXT("_@M1286_"), new InfoBoxFormatter(TEXT("")), NoProcessing, 8, 2);

	// LKTOKEN  _@M1644_ = "Home Alt.Arrival", _@M1645_ = "HomeArr"
	SetDataOption(115, ugAltitude,       TEXT("_@M1644_"), TEXT("_@M1645_"), new InfoBoxFormatter(TEXT("%2.0f")), NoProcessing, 8, 2);

	//Before adding new items, consider changing NUMSELECTSTRING_MAX

}

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
  // ignore this if the display isn't locked -- must keep menu visible
  if (DisplayLocked) {
    MenuTimeOut = MenuTimeoutMax;
    DisplayTimeOut = 0;
  }
}

void ShowMenu() {
#if !defined(GNAV) && !defined(PCGNAV)
  // Popup exit button if in .xci
  //InputEvents::setMode(TEXT("Exit"));
  InputEvents::setMode(TEXT("Menu")); // VENTA3
#endif
  MenuTimeOut = 0;
  DisplayTimeOut = 0;
}


#if (EXPERIMENTAL > 0)
BlueDialupSMS bsms;
#endif

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
		LastRangeLandableTime=0; // 110102
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
#ifdef LKAIRSPACE
	CAirspaceManager::Instance().CloseAirspaces();
	CAirspaceManager::Instance().ReadAirspaces();
	CAirspaceManager::Instance().SortAirspaces();
#else
	CloseAirspace();
	ReadAirspace();
	SortAirspace();
#endif
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
  InfoBoxesDirty = true;
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
 /*
  static bool first = true;
#if (WINDOWSPC>0)
  if (!first) {
    NMEAParser::Reset();
    return;
  }
#endif
 */

  StartupStore(TEXT(". RestartCommPorts%s"),NEWLINE);

  #ifdef DEBUG_DEVSETTINGS
  if (!goInitDevice) StartupStore(_T(".......... RestartCommPorts waiting for goInit\n"));
  #endif
  while(!goInitDevice) Sleep(50); // 100118 110605 this is potentially a deadlock!
  LockComm();

  devClose(devA());
  devClose(devB());

  NMEAParser::Reset();

//  first = false;

  devInit(TEXT(""));      

  UnlockComm();

}



void DefocusInfoBox() {
  FocusOnWindow(InfoFocus,false);
  InfoFocus = -1;
  if(MapWindow::mode.Is(MapWindow::Mode::MODE_PAN)) {
    InputEvents::setMode(TEXT("pan"));
  } else {
    InputEvents::setMode(TEXT("default"));
  }
  InfoWindowActive = FALSE;
}


void FocusOnWindow(int i, bool selected) {
    //hWndTitleWindow

  if (i<0) return; // error

  InfoBoxes[i]->SetFocus(selected);
  // todo defocus all other?

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

#ifndef NOVARIOGAUGE
	// VNT This thread was eating cpu in landscape mode although vario not used. 
	#ifdef LK8000_OPTIMIZE
	if ( (InfoBoxLayout::landscape == true) && ( InfoBoxLayout::InfoBoxGeometry == 6) ) {
		if (VarioUpdated && !InfoBoxLayout::fullscreen) { // VNT 090814 fix 
	#else
	if (VarioUpdated) { 
	#endif
		VarioUpdated = false;
		if (MapWindow::IsDisplayRunning()) {
			if (EnableVarioGauge) {
				GaugeVario::Render();
			}
		}
	}
	#ifdef LK8000_OPTIMIZE
		} else {
			// VNT Not sure this is busy-wait, but normally the thread could even be suspended
			Sleep(1000);
		}
	#endif
#else
	// DO NOTHING BY NOW
	Sleep(10000);
#endif
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
      // if (VarioUpdated) {  20060511/sgi commented out dueto asynchronus reset of VarioUpdate in InstrumentThread
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
      InfoBoxesDirty = true;
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


void PreloadInitialisation(bool ask) {
  //SetToRegistry(TEXT("XCV"), 1);
  SetToRegistry(TEXT("LKV"), 3);
  LKLanguageReady=false;
  LKReadLanguageFile();
  FillDataOptions();

  // Registery (early)

  if (ask) {
    RestoreRegistry();
    ReadRegistrySettings();
    StatusFileInit();


  } else {
    #if LKSTARTUP
    FullScreen();
    while (dlgStartupShowModal());
    #else
    dlgStartupShowModal();
    #endif
    RestoreRegistry();
    ReadRegistrySettings();

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

HANDLE drawTriggerEvent;

StartupState_t ProgramStarted = psInitInProgress; 
// 0: not started at all
// 1: everything is alive
// 2: done first draw
// 3: normal operation

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

#ifdef _INPUTDEBUG_
  InputEvents::showErrors();
#endif

  // Create default task if none exists
  StartupStore(TEXT(". Create default task%s"),NEWLINE);
//  Sleep(500); // 091212
  DefaultTask();

  // Trigger first redraw
  GpsUpdated = true;
  MapWindow::MapDirty = true;
  FullScreen();
  SetEvent(drawTriggerEvent);
}


extern int testmain();

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
  
  wsprintf(XCSoar_Version,_T("%S v%S.%S "), LKFORK, LKVERSION,LKRELEASE);
  wcscat(XCSoar_Version, TEXT(__DATE__));
  StartupStore(_T("%s------------------------------------------------------------%s"),NEWLINE,NEWLINE);
  #ifdef PNA
  StartupStore(TEXT(". Starting %s %s build#%d%s"), XCSoar_Version,_T("PNA"),BUILDNUMBER,NEWLINE);
  #else
  #if (WINDOWSPC>0)
  StartupStore(TEXT(". Starting %s %s build#%d%s"), XCSoar_Version,_T("PC"),BUILDNUMBER,NEWLINE);
  #else
  StartupStore(TEXT(". Starting %s %s build#%d%s"), XCSoar_Version,_T("PDA"),BUILDNUMBER,NEWLINE);
  #endif
  #endif

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
  StartupStore(_T(". Deleting registry key%s"),NEWLINE);
  if ( RegDeleteKey(HKEY_CURRENT_USER, _T(REGKEYNAME))== ERROR_SUCCESS )  // 091213
	StartupStore(_T(". Registry key was correctly deleted%s"),NEWLINE);
  else
	StartupStore(_T(". Registry key could NOT be deleted, this is normal after a reset.%s"),NEWLINE);


  bool datadir;
  datadir=CheckDataDir();
  if (!datadir) {
	// we cannot call startupstore, no place to store log!
	WarningHomeDir=true;
  }

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
    
  // We shall NOT create the LK8000 root folder if not existing
  // otherwise users will get confused on errors
  // CreateDirectoryIfAbsent(TEXT("")); 

  // These directories are needed if missing, as LK can run also with no maps and no waypoints..
  CreateDirectoryIfAbsent(TEXT(LKD_LOGS));
  CreateDirectoryIfAbsent(TEXT(LKD_CONF));
  CreateDirectoryIfAbsent(TEXT(LKD_TASKS));
  CreateDirectoryIfAbsent(TEXT(LKD_MAPS));
  CreateDirectoryIfAbsent(TEXT(LKD_WAYPOINTS));

  XCSoarGetOpts(lpCmdLine);

  icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icc.dwICC = ICC_UPDOWN_CLASS;
  InitCommonControls();
  InitSineTable();

  StartupStore(TEXT(". Initialize application instance%s"),NEWLINE);

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
  // Typical Record Button
  //	Why you can't always get this to work
  //	http://forums.devbuzz.com/m_1185/mpage_1/key_/tm.htm
  //	To do with the fact it is a global hotkey, but you can with code above
  //	Also APPA is record key on some systems
  SHSetAppKeyWndAssoc(VK_APP5, hWndMainWindow);
  SHSetAppKeyWndAssoc(VK_APP6, hWndMainWindow);
  #endif

  StartupStore(TEXT(". Initializing critical sections and events%s"),NEWLINE);

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
  StartupStore(_T(". ClearTask%s"),NEWLINE);
  ClearTask();
  memset( &(GPS_INFO), 0, sizeof(GPS_INFO));
  memset( &(CALCULATED_INFO), 0,sizeof(CALCULATED_INFO));
  memset( &SnailTrail[0],0,TRAILSIZE*sizeof(SNAIL_POINT));

  InitCalculations(&GPS_INFO,&CALCULATED_INFO);

  LinkGRecordDLL(); // try to link DLL if it exists

  OpenGeoid();

  PreloadInitialisation(false); // calls dlgStartup

  #ifndef NOCDIGAUGE
  GaugeCDI::Create();
  #endif
  #ifndef NOVARIOGAUGE
  GaugeVario::Create();
  #endif

  GPS_INFO.NAVWarning = true; // default, no gps at all!

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
  StartupStore(TEXT(". GlidePolar::SetBallast%s"),NEWLINE);
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
#if defined(FIVV) && ( !defined(WINDOWSPC) || WINDOWSPC==0 )
  if ( !datadir ) {
	// LKTOKEN _@M1208_ "ERROR NO DIRECTORY:"
    CreateProgressDialog(gettext(TEXT("_@M1208_")));
    Sleep(3000);
  }
#endif
  wsprintf(sTmpB, TEXT("Conf=%s"),sTmpA);
  CreateProgressDialog(sTmpB); 
#if defined(FIVV) && ( !defined(WINDOWSPC) || WINDOWSPC==0 )
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
  InitWayPointCalc(); 
  InitLDRotary(&rotaryLD); 
  InitWindRotary(&rotaryWind); // 100103
  // InitNewMap(); was moved in InitInstance 
  MapWindow::zoom.Reset();
  InitLK8000();
  ReadAirfieldFile();
  SetHome(false);
  LKReadLanguageFile();
  LKLanguageReady=true;

  RasterTerrain::ServiceFullReload(GPS_INFO.Latitude, 
                                   GPS_INFO.Longitude);

  // LKTOKEN _@M1216_ "Scanning weather forecast"  
  // CreateProgressDialog(gettext(TEXT("_@M1216_")));
  StartupStore(TEXT(". RASP load%s"),NEWLINE);
  RASP.Scan(GPS_INFO.Latitude, GPS_INFO.Longitude);
#ifdef LKAIRSPACE
  CAirspaceManager::Instance().ReadAirspaces();
  CAirspaceManager::Instance().SortAirspaces();
#else
  ReadAirspace();
  SortAirspace();
#endif
  OpenTopology();
  TopologyInitialiseMarks();

  OpenFLARMDetails();


#ifndef DISABLEAUDIOVARIO
  /*
  VarioSound_Init();
  VarioSound_EnableSound(EnableSoundVario);
  VarioSound_SetVdead(SoundDeadband);
  VarioSound_SetV(0);
  VarioSound_SetSoundVolume(SoundVolume);
  */
#endif

  // ... register all supported devices
  // IMPORTANT: ADD NEW ONES TO BOTTOM OF THIS LIST
  // LKTOKEN _@M1217_ "Starting devices"
  // Please check that the number of devices is not exceeding NUMREGDEV in device.h
  CreateProgressDialog(gettext(TEXT("_@M1217_")));
  StartupStore(TEXT(". Register serial devices%s"),NEWLINE);
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

// WINDOWSPC _SIM_ devInit called twice missing devA name
// on PC nonSIM we cannot use devInit here! Generic device is used until next port reset!

#if 110530
  // we need devInit for all devices. Missing initialization otherwise.
  LockComm();
  devInit(TEXT("")); 
  UnlockComm();
  // we want to be sure that RestartCommPort works on startup ONLY after all devices are inititalized
  goInitDevice=true; // 100118
#else
  // I dont remember anymore WHY! Probably it has been fixed already! paolo
  #if (WINDOWSPC>0)
  if (SIMMODE) devInit(TEXT(""));      
  #endif
#endif


  // re-set polar in case devices need the data
  StartupStore(TEXT(". GlidePolar::SetBallast%s"),NEWLINE);
  GlidePolar::SetBallast();

#if (EXPERIMENTAL > 0)
  CreateProgressDialog(TEXT("Bluetooth dialup SMS"));
  bsms.Initialise();
#endif
  // LKTOKEN _@M1218_ "Initialising display"
  CreateProgressDialog(gettext(TEXT("_@M1218_")));

  // just about done....

  DoSunEphemeris(GPS_INFO.Longitude, GPS_INFO.Latitude);

  // Finally ready to go
  StartupStore(TEXT(". CreateDrawingThread%s"),NEWLINE);
  MapWindow::CreateDrawingThread();
  Sleep(100);
  StartupStore(TEXT(". ShowInfoBoxes%s"),NEWLINE);
  #ifndef NOIBOX
  ShowInfoBoxes();
  #endif

  SwitchToMapWindow();
  StartupStore(TEXT(". CreateCalculationThread%s"),NEWLINE);
  CreateCalculationThread();
  #ifndef NOINSTHREAD
  while(!(goCalculationThread && goInstrumentThread)) Sleep(50); // 091119
  #else
  while(!(goCalculationThread)) Sleep(50); // 091119
  #endif
  // Sleep(500); 091119
#ifndef LKAIRSPACE
  StartupStore(TEXT(". AirspaceWarnListInit%s"),NEWLINE);
  AirspaceWarnListInit();
#endif
  StartupStore(TEXT(". dlgAirspaceWarningInit%s"),NEWLINE);
  dlgAirspaceWarningInit();

  // find unique ID of this PDA
  ReadAssetNumber();


  MapWindow::RequestOnFullScreen(); // VENTA10 EXPERIMENTAL

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
#if defined(GNAV) && !defined(PCGNAV)
  wc.hIcon = NULL;
#else
  wc.hIcon                      = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_XCSOARSWIFT));
#endif
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
  logfont->lfQuality = GetFontRenderer(); //was logfont->lfQuality = LKFONT_QUALITY;
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
                        LOGFONT * ptrhardInfoWindowLogFont,
                        LOGFONT * ptrhardTitleWindowLogFont,
                        LOGFONT * ptrhardMapWindowLogFont,
                        LOGFONT * ptrhardTitleSmallWindowLogFont,
                        LOGFONT * ptrhardMapWindowBoldLogFont,
                        LOGFONT * ptrhardCDIWindowLogFont, // New
                        LOGFONT * ptrhardMapLabelLogFont,
                        LOGFONT * ptrhardStatisticsLogFont) {



  memset ((char *)ptrhardInfoWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardTitleWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardMapWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardTitleSmallWindowLogFont, 0, sizeof (LOGFONT));
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
    propGetFontSettingsFromString(TEXT("34,0,0,0,800,0,0,0,0,0,0,4,2,TahomaBD"), ptrhardInfoWindowLogFont); // 28 091120
    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardTitleWindowLogFont); // 16 091120
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,4,2,Tahoma"), ptrhardTitleSmallWindowLogFont); 
    propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,4,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,700,1,0,0,0,0,0,4,2,Tahoma"), ptrhardMapLabelLogFont); // 100709
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    // propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardMapWindowLogFont); 091120
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardMapWindowLogFont);
    // propGetFontSettingsFromString(TEXT("16,0,0,0,500,0,0,0,0,0,0,4,2,TahomaBD"), ptrhardMapWindowBoldLogFont); 091120
    propGetFontSettingsFromString(TEXT("19,0,0,0,500,0,0,0,0,0,0,6,2,Tahoma"), ptrhardMapWindowBoldLogFont); 
    if (Appearance.InfoBoxGeom == 5) {
      GlobalEllipse=1.32f; // We don't use vario gauge in landscape geo5 anymore.. but doesn't hurt.
    }
    else {
      GlobalEllipse=1.1f;
    }
  }
  else if (ScreenSize==(ScreenSize_t)ss720x408) { // WQVGA  e.g. MIO
    propGetFontSettingsFromString(TEXT("51,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont); // 28 091120
    propGetFontSettingsFromString(TEXT("21,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont); // 16 091120
    propGetFontSettingsFromString(TEXT("42,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleSmallWindowLogFont); 
    propGetFontSettingsFromString(TEXT("23,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("23,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 100709
    propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("33,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("30,0,0,0,700,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardMapWindowBoldLogFont);
    if (Appearance.InfoBoxGeom == 5) {
      GlobalEllipse=1.32f; // We don't use vario gauge in landscape geo5 anymore.. but doesn't hurt.
    }
    else {
      GlobalEllipse=1.1f;
    }
  }

  else if (ScreenSize==(ScreenSize_t)ss480x234) { // e.g. Messada 2440
    propGetFontSettingsFromString(TEXT("32,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleSmallWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    //propGetFontSettingsFromString(TEXT("13,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 14 091120
    propGetFontSettingsFromString(TEXT("16,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 100709
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    // propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont); 091120
    propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    // propGetFontSettingsFromString(TEXT("16,0,0,0,500,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardMapWindowBoldLogFont); 091120
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
    GlobalEllipse=1.1f; // to be checked, TODO
  }

  else if (ScreenSize==(ScreenSize_t)ss800x480) {// e.g. ipaq 31x {

    switch (Appearance.InfoBoxGeom) {	
      case 0:
      case 1:
      case 2:
      case 3:
      case 6: // standard landscape
            propGetFontSettingsFromString(TEXT("56,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont);
            propGetFontSettingsFromString(TEXT("20,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
            GlobalEllipse=1.1f;	// standard VENTA2-addon
            break;
      case 4:       
      case 5:   
            propGetFontSettingsFromString(TEXT("62,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont); // 64 091120
            propGetFontSettingsFromString(TEXT("24,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont); // 26 091120
            GlobalEllipse=1.32f;	// VENTA2-addon
            break;
      case 7:
            propGetFontSettingsFromString(TEXT("66,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont);
            propGetFontSettingsFromString(TEXT("23,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
		  break;

       // This is a failsafe with an impossible setting so that you know 
      // something is going very wrong.
       default:
            propGetFontSettingsFromString(TEXT("30,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont);
            propGetFontSettingsFromString(TEXT("10,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
          //}
            break;
    } // special geometry cases for 31x


    propGetFontSettingsFromString(TEXT("16,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleSmallWindowLogFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardCDIWindowLogFont);
    // propGetFontSettingsFromString(TEXT("26,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); pre 100709
    propGetFontSettingsFromString(TEXT("28,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 100709
    propGetFontSettingsFromString(TEXT("48,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("36,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);


  }
  // added 091204
  else if (ScreenSize==(ScreenSize_t)ss400x240) {

    switch (Appearance.InfoBoxGeom) {	
      case 0:
      case 1:
      case 2:
      case 3:
      case 6: // standard landscape
            propGetFontSettingsFromString(TEXT("28,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont);
            propGetFontSettingsFromString(TEXT("10,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
            GlobalEllipse=1.1f;	// standard VENTA2-addon
            break;
      case 4:       
      case 5:   
            propGetFontSettingsFromString(TEXT("31,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont); // 64 091120
            propGetFontSettingsFromString(TEXT("12,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont); // 26 091120
            GlobalEllipse=1.32f;	// VENTA2-addon
            break;
      case 7:
            propGetFontSettingsFromString(TEXT("33,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont);
            propGetFontSettingsFromString(TEXT("11,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
		  break;

       // This is a failsafe with an impossible setting so that you know 
      // something is going very wrong.
       default:
            propGetFontSettingsFromString(TEXT("15,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont);
            propGetFontSettingsFromString(TEXT("5,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
          //}
            break;
    } 


    propGetFontSettingsFromString(TEXT("8,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleSmallWindowLogFont);
    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("13,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont);
    propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);


  }
  else if (ScreenSize==(ScreenSize_t)ss640x480) { // real VGA, not fake VGA
    propGetFontSettingsFromString(TEXT("54,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont); // infobox values
    propGetFontSettingsFromString(TEXT("19,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);  // infobox titles
    propGetFontSettingsFromString(TEXT("40,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleSmallWindowLogFont); // teamcode?
    propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont); // waynotes
    // propGetFontSettingsFromString(TEXT("23,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // old topo labels
    propGetFontSettingsFromString(TEXT("26,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 100709 topo labels
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);  // wps and mapscale
    propGetFontSettingsFromString(TEXT("28,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont); // bold version of MapW
										// and all messages and menus
    GlobalEllipse=1.1f; // to be checked, TODO but unused, now
  }
  else if (ScreenSize==(ScreenSize_t)ss896x672) { // real VGA, not fake VGA
    propGetFontSettingsFromString(TEXT("75,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont); // infobox values
    propGetFontSettingsFromString(TEXT("25,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);  // infobox titles
    propGetFontSettingsFromString(TEXT("56,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleSmallWindowLogFont); // teamcode?
    propGetFontSettingsFromString(TEXT("33,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont); // waynotes
    propGetFontSettingsFromString(TEXT("32,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // topo labels
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);  // wps and mapscale
    propGetFontSettingsFromString(TEXT("39,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont); // bold version of MapW
										// and all messages and menus
    GlobalEllipse=1.1f; // to be checked, TODO but unused, now
  }
  else if (ScreenSize==(ScreenSize_t)ss320x240) { // also applies for fake VGA where all values are doubled stretched
    propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("21,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleSmallWindowLogFont);
    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,500,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // 100819
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
    GlobalEllipse=1.1f; // to be checked, TODO
  }
  else if (ScreenSize==(ScreenSize_t)ss240x320) { // also applies for fake VGA where all values are doubled stretched
    propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("21,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleSmallWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("13,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // RLD 16 works well too
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,500,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardMapWindowBoldLogFont);
    GlobalEllipse=1.1f; // to be checked, TODO
  }
  else if (ScreenSize==(ScreenSize_t)ss272x480) { 
    propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardInfoWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("21,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleSmallWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); 
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
    GlobalEllipse=1.1f; 
  }
  else if (ScreenSize==(ScreenSize_t)ss480x640) { // real VGA, not fake VGA
    propGetFontSettingsFromString(TEXT("48,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont); // infobox values
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);  // infobox titles
    propGetFontSettingsFromString(TEXT("40,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleSmallWindowLogFont); // teamcode?
    propGetFontSettingsFromString(TEXT("26,0,0,0,100,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont); // waynotes
    propGetFontSettingsFromString(TEXT("23,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // topo labels
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);  // wps and mapscale
    propGetFontSettingsFromString(TEXT("28,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont); // bold version of MapW
										// and all messages and menus
    GlobalEllipse=1.1f; // to be checked, TODO but unused, now
  }
  else if (ScreenSize==(ScreenSize_t)ss480x800) { // real VGA, not fake VGA
    propGetFontSettingsFromString(TEXT("48,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardInfoWindowLogFont); // infobox values
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);  // infobox titles
    propGetFontSettingsFromString(TEXT("40,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleSmallWindowLogFont); // teamcode?
    propGetFontSettingsFromString(TEXT("26,0,0,0,100,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont); // waynotes
    propGetFontSettingsFromString(TEXT("23,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); // topo labels
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);//  (RLD is this used?)
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);  // wps and mapscale
    propGetFontSettingsFromString(TEXT("30,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont); // bold version of MapW
										// and all messages and menus
    GlobalEllipse=1.1f; // to be checked, TODO but unused, now
  }




}

void InitialiseFontsAuto(RECT rc,
                        LOGFONT * ptrautoInfoWindowLogFont,
                        LOGFONT * ptrautoTitleWindowLogFont,
                        LOGFONT * ptrautoMapWindowLogFont,
                        LOGFONT * ptrautoTitleSmallWindowLogFont,
                        LOGFONT * ptrautoMapWindowBoldLogFont,
                        LOGFONT * ptrautoCDIWindowLogFont, // New
                        LOGFONT * ptrautoMapLabelLogFont,
                        LOGFONT * ptrautoStatisticsLogFont) {
  LOGFONT logfont;
  int FontHeight, FontWidth;
  int fontsz1 = (rc.bottom - rc.top );
  int fontsz2 = (rc.right - rc.left );

  memset ((char *)ptrautoInfoWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrautoTitleWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrautoMapWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrautoTitleSmallWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrautoMapWindowBoldLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrautoCDIWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrautoMapLabelLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrautoStatisticsLogFont, 0, sizeof (LOGFONT));

  if (fontsz1<fontsz2) { // portrait
    FontHeight = (int)(fontsz1/FONTHEIGHTRATIO*1.33);  // use small dimension, to work for widscreens and adjust so it works for 4x3 screens too.
    FontWidth = (int)(FontHeight*0.4);
  } 
  else if (fontsz1==fontsz2)
  {  // square
    FontHeight = (int)(fontsz2/FONTHEIGHTRATIO);
    FontWidth = (int)(FontHeight*0.4);
  }
  else 
  { // landscape 
    FontHeight = (int)(fontsz2/FONTHEIGHTRATIO*1.33);
    FontWidth = (int)(FontHeight*0.4);
  }

  int iFontHeight = (int)(FontHeight*1.4);
  // oversize first so can then scale down

  FontWidth = 0; // JMW this should be done so closest font is found

  // sgi todo

  memset ((char *)&logfont, 0, sizeof (logfont));

// #if defined(PNA) || defined(FIVV)  // Only for PNA, since we still do not copy Fonts in their Windows memory.
				      // though we could already do it automatically. 
// #if defined(PNA) //FIX 090925 QUI
  _tcscpy(logfont.lfFaceName, _T("Tahoma")); 

  logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE  ;
  logfont.lfHeight = iFontHeight;
  logfont.lfWidth =  FontWidth;
  logfont.lfWeight = FW_BOLD;
  logfont.lfItalic = TRUE;
  logfont.lfCharSet = ANSI_CHARSET;
  ApplyClearType(&logfont);

  // JMW algorithm to auto-size info window font.
  // this is still required in case title font property doesn't exist.
  SIZE tsize;
  HDC iwhdc = GetDC(hWndMainWindow);
  do {
    HFONT TempWindowFont;
    HFONT hfOld;

    iFontHeight--;
    logfont.lfHeight = iFontHeight;
//    InfoWindowFont = CreateFontIndirect (&logfont);
//    SelectObject(iwhdc, InfoWindowFont);

    TempWindowFont = CreateFontIndirect (&logfont);
    hfOld=(HFONT)SelectObject(iwhdc, TempWindowFont);


    GetTextExtentPoint(iwhdc, TEXT("00:00"), 5, &tsize);
//    DeleteObject(InfoWindowFont);
    SelectObject(iwhdc, hfOld); // unselect it before deleting it
    DeleteObject(TempWindowFont);

  } while (tsize.cx>InfoBoxLayout::ControlWidth);
  ReleaseDC(hWndMainWindow, iwhdc);

  iFontHeight++;
  logfont.lfHeight = iFontHeight;

//  propGetFontSettings(TEXT("InfoWindowFont"), &logfont);
//  InfoWindowFont = CreateFontIndirect (&logfont);
 memcpy ((void *)ptrautoInfoWindowLogFont, &logfont, sizeof (LOGFONT));


  // next font..

  memset ((char *)&logfont, 0, sizeof (logfont));

  _tcscpy(logfont.lfFaceName, _T("Tahoma"));
  logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE  ;
  logfont.lfHeight = (int)(FontHeight/TITLEFONTHEIGHTRATIO);
  logfont.lfWidth =  (int)(FontWidth/TITLEFONTWIDTHRATIO);
  logfont.lfWeight = FW_BOLD;
    ApplyClearType(&logfont); // 110106
  // RLD this was the only auto font to not have "ApplyClearType()".  It does not apply to very small fonts
  // we now apply ApplyClearType to all fonts in CreateOneFont(). 

//  propGetFontSettings(TEXT("TitleWindowFont"), &logfont);
//  TitleWindowFont = CreateFontIndirect (&logfont);
  memcpy ((void *)ptrautoTitleWindowLogFont, &logfont, sizeof (LOGFONT));

  memset ((char *)&logfont, 0, sizeof (logfont));

  // new font for CDI Scale

  _tcscpy(logfont.lfFaceName, _T(GLOBALFONT));
  logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE  ;
  logfont.lfHeight = (int)(FontHeight*CDIFONTHEIGHTRATIO);
  logfont.lfWidth =  (int)(FontWidth*CDIFONTWIDTHRATIO);
  logfont.lfWeight = FW_MEDIUM;
  ApplyClearType(&logfont); // 110106

//  propGetFontSettings(TEXT("CDIWindowFont"), &logfont);
//  CDIWindowFont = CreateFontIndirect (&logfont);
  memcpy ((void *)ptrautoCDIWindowLogFont, &logfont, sizeof (LOGFONT));

  // new font for map labels
  memset ((char *)&logfont, 0, sizeof (logfont));

  _tcscpy(logfont.lfFaceName, _T("Tahoma"));
  logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE  ;
  logfont.lfHeight = (int)(FontHeight*MAPFONTHEIGHTRATIO);
  logfont.lfWidth =  (int)(FontWidth*MAPFONTWIDTHRATIO);
  logfont.lfWeight = FW_MEDIUM;
  logfont.lfItalic = TRUE; 
  ApplyClearType(&logfont); // 110106

//  propGetFontSettings(TEXT("MapLabelFont"), &logfont);
//  MapLabelFont = CreateFontIndirect (&logfont);
  memcpy ((void *)ptrautoMapLabelLogFont, &logfont, sizeof (LOGFONT));


  // Font for map other text
  memset ((char *)&logfont, 0, sizeof (logfont));

  _tcscpy(logfont.lfFaceName, _T("Tahoma"));
  logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE  ;
  logfont.lfHeight = (int)(FontHeight*STATISTICSFONTHEIGHTRATIO);
  logfont.lfWidth =  (int)(FontWidth*STATISTICSFONTWIDTHRATIO);
  logfont.lfWeight = FW_MEDIUM;
  ApplyClearType(&logfont); // 110106

//  propGetFontSettings(TEXT("StatisticsFont"), &logfont);
//  StatisticsFont = CreateFontIndirect (&logfont);
  memcpy ((void *)ptrautoStatisticsLogFont, &logfont, sizeof (LOGFONT));

  // new font for map labels

  _tcscpy(logfont.lfFaceName, _T("Tahoma"));
  logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE  ;
  logfont.lfHeight = (int)(FontHeight*MAPFONTHEIGHTRATIO*1.3);
  logfont.lfWidth =  (int)(FontWidth*MAPFONTWIDTHRATIO*1.3);
  logfont.lfWeight = FW_MEDIUM;
  ApplyClearType(&logfont); // 110106

//  propGetFontSettings(TEXT("MapWindowFont"), &logfont);
//  MapWindowFont = CreateFontIndirect (&logfont);

//  SendMessage(hWndMapWindow,WM_SETFONT,
//        (WPARAM)MapWindowFont,MAKELPARAM(TRUE,0));
  memcpy ((void *)ptrautoMapWindowLogFont, &logfont, sizeof (LOGFONT));

  // Font for map bold text

  _tcscpy(logfont.lfFaceName, _T("Tahoma"));
  logfont.lfWeight = FW_BOLD;
  logfont.lfWidth =  0; // JMW (int)(FontWidth*MAPFONTWIDTHRATIO*1.3) +2;
  ApplyClearType(&logfont); // 110106 missing

//  propGetFontSettings(TEXT("MapWindowBoldFont"), &logfont);
//  MapWindowBoldFont = CreateFontIndirect (&logfont);
  memcpy ((void *)ptrautoMapWindowBoldLogFont, &logfont, sizeof (LOGFONT));

  // TODO code: create font settings for this one...
  memset((char *)&logfont, 0, sizeof (logfont));
  _tcscpy(logfont.lfFaceName, _T(GLOBALFONT));
  
  logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE  ;
  logfont.lfHeight = IBLSCALE(20);
  logfont.lfWidth =  IBLSCALE(8);
  logfont.lfWeight = FW_MEDIUM;
  ApplyClearType(&logfont); // 110106 missing
  
  memcpy ((void *)ptrautoTitleSmallWindowLogFont, &logfont, sizeof (LOGFONT));
}

//  propGetFontSettings(TEXT("TeamCodeFont"), &logfont);
//  TitleSmallWindowFont = CreateFontIndirect (&logfont);


void InitialiseFonts(RECT rc)
{ //this routine must be called only at start/restart of XCSoar b/c there are many pointers to these fonts
 
  DeleteObject(InfoWindowFont);  
  DeleteObject(TitleWindowFont);
  DeleteObject(MapWindowFont);
  DeleteObject(TitleSmallWindowFont);
  DeleteObject(MapWindowBoldFont);
  DeleteObject(CDIWindowFont);
  DeleteObject(MapLabelFont);
  DeleteObject(StatisticsFont);

  memset ((char *)&autoInfoWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&autoTitleWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&autoMapWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&autoTitleSmallWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&autoMapWindowBoldLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&autoCDIWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&autoMapLabelLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&autoStatisticsLogFont, 0, sizeof (LOGFONT));


  InitialiseFontsAuto(rc, 
                        &autoInfoWindowLogFont,
                        &autoTitleWindowLogFont,
                        &autoMapWindowLogFont,
                        &autoTitleSmallWindowLogFont,
                        &autoMapWindowBoldLogFont,
                        &autoCDIWindowLogFont, // New
                        &autoMapLabelLogFont,
                        &autoStatisticsLogFont);


  LOGFONT hardInfoWindowLogFont;
  LOGFONT hardTitleWindowLogFont;
  LOGFONT hardMapWindowLogFont;
  LOGFONT hardTitleSmallWindowLogFont;
  LOGFONT hardMapWindowBoldLogFont;
  LOGFONT hardCDIWindowLogFont; 
  LOGFONT hardMapLabelLogFont;
  LOGFONT hardStatisticsLogFont;

  memset ((char *)&hardInfoWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardTitleWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardMapWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardTitleSmallWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardMapWindowBoldLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardCDIWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardMapLabelLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardStatisticsLogFont, 0, sizeof (LOGFONT));

  InitialiseFontsHardCoded(rc,
                        &hardInfoWindowLogFont,
                        &hardTitleWindowLogFont,
                        &hardMapWindowLogFont,
                        &hardTitleSmallWindowLogFont,
                        &hardMapWindowBoldLogFont,
                        &hardCDIWindowLogFont, // New
                        &hardMapLabelLogFont,
                        &hardStatisticsLogFont);

// for PNA & GNAV, merge the "hard" into the "auto" if one exists 
  if (!IsNullLogFont(hardInfoWindowLogFont))
    autoInfoWindowLogFont = hardInfoWindowLogFont;

  if (!IsNullLogFont(hardTitleWindowLogFont))
    autoTitleWindowLogFont = hardTitleWindowLogFont;

  if (!IsNullLogFont(hardMapWindowLogFont))
    autoMapWindowLogFont = hardMapWindowLogFont;

  if (!IsNullLogFont(hardTitleSmallWindowLogFont))
    autoTitleSmallWindowLogFont = hardTitleSmallWindowLogFont;

  if (!IsNullLogFont(hardMapWindowBoldLogFont))
    autoMapWindowBoldLogFont = hardMapWindowBoldLogFont;

  if (!IsNullLogFont(hardCDIWindowLogFont))
    autoCDIWindowLogFont = hardCDIWindowLogFont;

  if (!IsNullLogFont(hardMapLabelLogFont))
    autoMapLabelLogFont = hardMapLabelLogFont;

  if (!IsNullLogFont(hardStatisticsLogFont))
    autoStatisticsLogFont = hardStatisticsLogFont;


  InitializeOneFont (&InfoWindowFont, 
                        szRegistryFontInfoWindowFont, 
                        autoInfoWindowLogFont,
                        NULL);

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
  SendMessage(hWndMapWindow,WM_SETFONT,
              (WPARAM)MapWindowFont,MAKELPARAM(TRUE,0));

  InitializeOneFont (&MapWindowBoldFont, 
                        szRegistryFontMapWindowBoldFont, 
                        autoMapWindowBoldLogFont,
                        NULL);

  InitializeOneFont (&TitleSmallWindowFont, 
                        szRegistryFontTitleSmallWindowFont, 
                        autoTitleSmallWindowLogFont,
                        NULL);
}


#if (WINDOWSPC>0) 
int SCREENWIDTH=640;
int SCREENHEIGHT=480;
#endif

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
  InitScreenSize();
  InitNewMap(); // causing problems with CreateButtonLabels?
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

#if defined(GNAV) && !defined(PCGNAV)
  // TODO code: release the handle?
  HANDLE hTmp = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_XCSOARSWIFT));
  SendMessage(hWndMainWindow, WM_SETICON,
	      (WPARAM)ICON_BIG, (LPARAM)hTmp);
  SendMessage(hWndMainWindow, WM_SETICON,
	      (WPARAM)ICON_SMALL, (LPARAM)hTmp);
#endif

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

  StartupStore(TEXT(". InfoBox geometry%s"),NEWLINE);

  InfoBoxLayout::ScreenGeometry(rc);
  StartupStore(TEXT(". Create Objects%s"),NEWLINE);
  LKObjects_Create(); //@ 101124

  StartupStore(TEXT(". Load unit bitmaps%s"),NEWLINE);

  Units::LoadUnitBitmap(hInstance);

  StartupStore(TEXT(". Create info boxes%s"),NEWLINE);

  #ifndef NOIBOX
  InfoBoxLayout::CreateInfoBoxes(rc);
  #endif

  #ifndef NOFLARMGAUGE
  StartupStore(TEXT(". Create FLARM gauge%s"),NEWLINE);
  GaugeFLARM::Create();
  #endif

  StartupStore(TEXT(". Create button labels%s"),NEWLINE);
  ButtonLabel::CreateButtonLabels(rc);
  ButtonLabel::SetLabelText(0,TEXT("MODE"));

  StartupStore(TEXT(". Initialize fonts%s"),NEWLINE);
  InitialiseFonts(rc);
  InitNewMap();	// reload updating LK fonts after loading profile

  ButtonLabel::SetFont(MapWindowBoldFont);

  StartupStore(TEXT(". Initialise message system%s"),NEWLINE);
  Message::Initialize(rc); // creates window, sets fonts

  ShowWindow(hWndMainWindow, SW_SHOW);

  StartupStore(TEXT(". Create map window%s"),NEWLINE);

  hWndMapWindow = CreateWindow(TEXT("MapWindowClass"),NULL,
			       WS_VISIBLE | WS_CHILD
			       | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                               0, 0, (rc.right - rc.left),
			       (rc.bottom-rc.top) ,
                               hWndMainWindow, NULL ,hInstance,NULL);

  // JMW gauge creation was here
  ShowWindow(hWndMainWindow, nCmdShow);

  UpdateWindow(hWndMainWindow);
    
  return TRUE;
}


// Obfuscated C code contest winner
int getInfoType(int i) {
  int retval = 0;
  if (i<0) return 0; // error

  if (EnableAuxiliaryInfo) {
    retval = (InfoType[i] >> 24) & 0xff; // auxiliary
  } else {
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
  }
  return min(NUMSELECTSTRINGS-1,retval);
}


void setInfoType(int i, char j) {
  if (i<0) return; // error

  if (EnableAuxiliaryInfo) {
    InfoType[i] &= 0x00ffffff;
    InfoType[i] += (j<<24);
  } else {
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
}


void DoInfoKey(int keycode) {
  int i;

  if (InfoFocus<0) return; // paranoid

  HideMenu();

  LockNavBox();
  i = getInfoType(InfoFocus);

  // XXX This could crash if MapWindow does not capture

  LockFlightData();
  Data_Options[min(NUMSELECTSTRINGS-1,i)].Process(keycode);
  UnlockFlightData();

  UnlockNavBox();

  InfoBoxesDirty = true;

  TriggerGPSUpdate(); // emulate update to trigger calculations

  InfoBoxFocusTimeOut = 0;
  DisplayTimeOut = 0;

}


// Debounce input buttons (does not matter which button is pressed)
// VNT 090702 FIX Careful here: synthetic double clicks and virtual keys require some timing.
//				See Defines.h DOUBLECLICKINTERVAL . Not sure they are 100% independent.

int debounceTimeout=200;

bool Debounce(void) {
  static DWORD fpsTimeLast= 0;
  DWORD fpsTimeThis = ::GetTickCount();
  DWORD dT = fpsTimeThis-fpsTimeLast;

  DisplayTimeOut = 0;
  InterfaceTimeoutReset();

  if (ScreenBlanked) {
    // prevent key presses working if screen is blanked,
    // so a key press just triggers turning the display on again
    return false;
  }

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

  StartupStore(TEXT(". Entering shutdown...%s"),NEWLINE);
  StartupLogFreeRamAndStorage();

  // turn off all displays
  GlobalRunning = false;

  StartupStore(TEXT(". dlgAirspaceWarningDeInit%s"),NEWLINE);
  dlgAirspaceWarningDeInit();
#ifndef LKAIRSPACE
  StartupStore(TEXT(". AirspaceWarnListDeInit%s"),NEWLINE);
  AirspaceWarnListDeInit();
#endif
  
  // LKTOKEN _@M1220_ "Shutdown, saving logs..."
  CreateProgressDialog(gettext(TEXT("_@M1220_")));
  // stop logger
  guiStopLogger(true);

  // LKTOKEN _@M1221_ "Shutdown, saving profile..."
  CreateProgressDialog(gettext(TEXT("_@M1221_")));
  // Save settings
  StoreRegistry();

  StartupStore(TEXT(". Save_Recent_WP_history%s"),NEWLINE);
  SaveRecentList();
  // Stop sound

  StartupStore(TEXT(". SaveSoundSettings%s"),NEWLINE);
  SaveSoundSettings();

#ifndef DISABLEAUDIOVARIO  
  //  VarioSound_EnableSound(false);
  //  VarioSound_Close();
#endif

  // Stop SMS device
#if (EXPERIMENTAL > 0)
  bsms.Shutdown();
#endif

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
  #ifndef NOVARIOGAUGE
  SetEvent(varioTriggerEvent);
  #endif

  // Clear data
  // LKTOKEN _@M1222_ "Shutdown, saving task..."
  CreateProgressDialog(gettext(TEXT("_@M1222_")));
  StartupStore(TEXT(". Save default task%s"),NEWLINE);
  SaveDefaultTask();

  StartupStore(TEXT(". Clear task data%s"),NEWLINE);

  LockTaskData();
  Task[0].Index = -1;  ActiveWayPoint = -1; 
  AATEnabled = FALSE;
#ifndef LKAIRSPACE
  NumberOfAirspacePoints = 0; NumberOfAirspaceAreas = 0; 
  NumberOfAirspaceCircles = 0;
#endif
  CloseWayPoints();
  UnlockTaskData();

  // LKTOKEN _@M1219_ "Shutdown, please wait..."
  CreateProgressDialog(gettext(TEXT("_@M1219_")));
  StartupStore(TEXT(". CloseTerrainTopology%s"),NEWLINE);

  RASP.Close();
  RasterTerrain::CloseTerrain();

  CloseTopology();

  TopologyCloseMarks();

  CloseTerrainRenderer();

  // Stop COM devices
  StartupStore(TEXT(". Stop COM devices%s"),NEWLINE);
  devCloseAll();

  // SaveCalculationsPersist(&CALCULATED_INFO); // 091119 DISABLED
#if (EXPERIMENTAL > 0)
  //  CalibrationSave();
#endif

  #if defined(GNAV) && !defined(PCGNAV)
    StartupStore(TEXT(". Altair shutdown%s"),NEWLINE);
    Sleep(2500);
    StopHourglassCursor();
    InputEvents::eventDLLExecute(TEXT("altairplatform.dll SetShutdown 1"));
    while(1) {
      Sleep(100); // free time up for processor to perform shutdown
    }
  #endif

  CloseFLARMDetails();

  ProgramStarted = psInitInProgress;

  // Kill windows

  #ifndef LK8000_OPTIMIZE
  StartupStore(TEXT(". Close Gauges%s"),NEWLINE);
  #endif
  #ifndef NOCDIGAUGE
  GaugeCDI::Destroy();
  #endif
  #ifndef NOVARIOGAUGE
  GaugeVario::Destroy();
  #endif
  #ifndef NOFLARMGAUGE
  GaugeFLARM::Destroy();
  #endif
  
  StartupStore(TEXT(". Close Messages%s"),NEWLINE);
  Message::Destroy();
  
  Units::UnLoadUnitBitmap();

  #ifndef NOIBOX  
  StartupStore(TEXT(". Destroy Info Boxes%s"),NEWLINE);
  InfoBoxLayout::DestroyInfoBoxes();
  #endif
  
  StartupStore(TEXT(". Destroy Button Labels%s"),NEWLINE);
  ButtonLabel::Destroy();

  StartupStore(TEXT(". Delete Objects%s"),NEWLINE);
  
  //  CommandBar_Destroy(hWndCB);
  for (i=0; i<NUMSELECTSTRINGS; i++) {
    delete Data_Options[i].Formatter;
  }

  // Kill graphics objects

  DeleteObject(hBrushSelected);
  DeleteObject(hBrushUnselected);
  DeleteObject(hBrushButton);
  
  DeleteObject(InfoWindowFont);
  DeleteObject(TitleWindowFont);
  DeleteObject(CDIWindowFont);
  DeleteObject(MapLabelFont);
  DeleteObject(MapWindowFont);
  DeleteObject(MapWindowBoldFont);
  DeleteObject(StatisticsFont);  
  DeleteObject(TitleSmallWindowFont);
#ifdef LKAIRSPACE  
  CAirspaceManager::Instance().CloseAirspaces();
#else
  if(AirspaceArea != NULL)   LocalFree((HLOCAL)AirspaceArea);
  if(AirspacePoint != NULL)  LocalFree((HLOCAL)AirspacePoint);
  if(AirspaceScreenPoint != NULL)  LocalFree((HLOCAL)AirspaceScreenPoint);
  if(AirspaceCircle != NULL) LocalFree((HLOCAL)AirspaceCircle);
#endif
  StartupStore(TEXT(". Delete Critical Sections%s"),NEWLINE);
  
  DeleteCriticalSection(&CritSec_EventQueue);
  csEventQueueInitialized = false;
  #if ALPHADEBUG
  StartupStore(TEXT(". Deleted EventQueue%s"),NEWLINE); // REMOVE 101121
  #endif
  DeleteCriticalSection(&CritSec_TaskData);
  csTaskDataInitialized = false;
  #if ALPHADEBUG
  StartupStore(TEXT(". Deleted TaskData%s"),NEWLINE); // REMOVE 101121
  #endif
  DeleteCriticalSection(&CritSec_FlightData);
  csFlightDataInitialized = false;
  #if ALPHADEBUG
  StartupStore(TEXT(". Deleted FlightData%s"),NEWLINE); // REMOVE 101121
  #endif
  DeleteCriticalSection(&CritSec_NavBox);
  csNavBoxInitialized = false;
  #if ALPHADEBUG
  StartupStore(TEXT(". Deleted NavBox%s"),NEWLINE); // REMOVE 101121
  #endif
  DeleteCriticalSection(&CritSec_Comm);
  csCommInitialized = false;
  #if ALPHADEBUG
  StartupStore(TEXT(". Deleted Comm%s"),NEWLINE); // REMOVE 101121
  #endif
  DeleteCriticalSection(&CritSec_TerrainDataCalculations);
  csTerrainDataGraphicsInitialized = false;
  #if ALPHADEBUG
  StartupStore(TEXT(". Deleted TerrainData%s"),NEWLINE); // REMOVE 101121
  #endif
  DeleteCriticalSection(&CritSec_TerrainDataGraphics);
  csTerrainDataCalculationsInitialized = false;

  StartupStore(TEXT(". Close Progress Dialog%s"),NEWLINE);

  CloseProgressDialog();

  StartupStore(TEXT(". Close Calculations%s"),NEWLINE);
  CloseCalculations();

  CloseGeoid();

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
  StartupStore(TEXT(". Finished shutdown%s"),NEWLINE);
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
        StartupStore(TEXT("SHHandleWMActivate not available%s"),NEWLINE);
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
        StartupStore(TEXT("SHHandleWMSettingChange not available%s"),NEWLINE);
        #endif
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
#endif
      break;

    case WM_SETFOCUS:
      // JMW not sure this ever does anything useful..
      if (ProgramStarted > psInitInProgress) {

	if(InfoWindowActive) {
	  
	  if(DisplayLocked) {
	    FocusOnWindow(InfoFocus,true);
	  } else {
	    FocusOnWindow(InfoFocus,true);
	  }
	} else {
	  DefocusInfoBox();
	  HideMenu();
	  SetFocus(hWndMapWindow);
	}
      }
      break;
      // TODO enhancement: Capture KEYDOWN time
      // 	- Pass that (otpionally) to processKey, allowing
      // 	  processKey to handle long events - at any length
      // 	- Not sure how to do double click... (need timer call back
      // 	process unless reset etc... tricky)
      // we do this in WindowControls
#if defined(GNAV) || defined(PCGNAV)
    case WM_KEYDOWN: // JMW was keyup
#else
    case WM_KEYUP: // JMW was keyup
#endif

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
          StartupStore(TEXT(". ProgramStarted=NormalOp%s"),NEWLINE);
          StartupLogFreeRamAndStorage();
	}
      }
      break;

    case WM_INITMENUPOPUP:
      if (ProgramStarted > psInitInProgress) {
	if(DisplayLocked)
	  CheckMenuItem((HMENU) wParam,IDM_FILE_LOCK,MF_CHECKED|MF_BYCOMMAND);
	else
	  CheckMenuItem((HMENU) wParam,IDM_FILE_LOCK,MF_UNCHECKED|MF_BYCOMMAND);
	
	if(LoggerActive)
	  CheckMenuItem((HMENU) wParam,IDM_FILE_LOGGER,MF_CHECKED|MF_BYCOMMAND);
	else
	  CheckMenuItem((HMENU) wParam,IDM_FILE_LOGGER,MF_UNCHECKED|MF_BYCOMMAND);
      }
      break;

    case WM_CLOSE:

#ifndef GNAV
      ASSERT(hWnd==hWndMainWindow);
      if(ForceShutdown || ((hWnd==hWndMainWindow) && 
         (MessageBoxX(hWndMainWindow,
		// LKTOKEN  _@M198_ = "Confirm Exit?"
               	gettext(TEXT("_@M198_")),
                      TEXT("LK8000"),
                      MB_YESNO|MB_ICONQUESTION) == IDYES))) 
#endif
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
  int i;

  wmId    = LOWORD(wParam);
  wmEvent = HIWORD(wParam);
  wmControl = (HWND)lParam;

  if(wmControl != NULL) {
    if (ProgramStarted==psNormalOp) {

      DialogActive = false;

      FullScreen();
      
      InfoBoxFocusTimeOut = 0;
      /*
      if (!InfoWindowActive) {
        ShowMenu();
      }
      */
      #ifndef NOIBOX 
      for(i=0;i<numInfoWindows;i++) {
        if(wmControl == InfoBoxes[i]->GetHandle()) {
          InfoWindowActive = TRUE;
                
          if(DisplayLocked) {
            if( i!= InfoFocus) {
              FocusOnWindow(i,true);
              FocusOnWindow(InfoFocus,false);
              
              InfoFocus = i;
              InfoWindowActive = TRUE;
            }
            DisplayText();
            InputEvents::setMode(TEXT("infobox"));
            //InputEvents::setMode(TEXT("fullscreen")); 091128 to test
                    
          } else {
            PopUpSelect(i);
            DisplayText();
          }
          return 0;
        }
      }
      #endif
      Message::CheckTouch(wmControl);
        
      if (ButtonLabel::CheckButtonPress(wmControl)) {
        return TRUE; // don't continue processing..
      }
      
    }
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}


void    AssignValues(void)
{
  if (InfoBoxesHidden) {
    // no need to assign values
    return;
  }

  //  DetectStartTime(); moved to Calculations

  // nothing to do here now!
}

extern int PDABatteryTemperature;
void DisplayText(void)
{
#ifdef NOIBOX
  return;
#endif
  if (InfoBoxesHidden)
    return;

  int i;
  static int DisplayType[MAXINFOWINDOWS];
  static bool first=true;
  static int InfoFocusLast = -1;
  static int DisplayTypeLast[MAXINFOWINDOWS];
// static double LastFlipBoxTime = 0; //  now global for SlowCalculations
  static bool FlipBoxValue = false;


  // VENTA3 - Dynamic box values
  if ( GPS_INFO.Time > LastFlipBoxTime + DYNABOXTIME ) {
/*
	static TCHAR ventabuffer[200];
	FILE *fp;
        wsprintf(ventabuffer,TEXT("GPS_INFO.Time=%d LastFlipBoxTime=%d Flip=%d"),(int)GPS_INFO.Time, (int)LastFlipBoxTime,
	FlipBoxValue == true ? 1 : 0);
        if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL){;fprintf(fp,"%S\n",ventabuffer);fclose(fp)
;}
*/
	FlipBoxValue = ( FlipBoxValue == false );
	LastFlipBoxTime = GPS_INFO.Time;
  }
	
  

  LockNavBox();

  // JMW note: this is updated every GPS time step

  if (InfoFocus != InfoFocusLast) {
    first = true; // force re-setting title
  }
  if ((InfoFocusLast>=0)&&(!InfoWindowActive)) {
    first = true;
  }
  InfoFocusLast = InfoFocus;

  for(i=0;i<numInfoWindows;i++) {

    // VENTA3
    // All calculations are made in a separate thread. Slow calculations should apply to
    // the function DoCalculationsSlow() . Do not put calculations here! 
    
    DisplayType[i] = getInfoType(i);
    Data_Options[DisplayType[i]].Formatter->AssignValue(DisplayType[i]);
    
    TCHAR sTmp[32];
    
    int color = 0;

    bool needupdate = ((DisplayType[i] != DisplayTypeLast[i])||first);
    
    int theactive = ActiveWayPoint;
    if (!ValidTaskPoint(theactive)) {
      theactive = -1;
    }

    //
    // Set Infobox title and middle value. Bottom line comes next
    //
    switch (DisplayType[i]) {

    case 67: // VENTA3 alternate1 and 2
    case 68: 
    case 69:
    case 75: // VENTA3 alternate1 and 2
    case 76: 
    case 77:
	if (DisplayType[i]==67 || DisplayType[i]==75) ActiveAlternate=Alternate1; else
	if (DisplayType[i]==68 || DisplayType[i]==76) ActiveAlternate=Alternate2; 
		else ActiveAlternate=BestAlternate;
	InfoBoxes[i]->SetSmallerFont(false);
	if ( ActiveAlternate != -1 ) {
		InfoBoxes[i]->SetTitle(Data_Options[DisplayType[i]].Formatter->
			   RenderTitle(&color));
		InfoBoxes[i]->SetColor(color);
		InfoBoxes[i]->SetValue(Data_Options[DisplayType[i]].Formatter->
			   Render(&color));
		InfoBoxes[i]->SetColor(color);
	} else {
		switch(DisplayType[i]) {
			case 67:
				// LKTOKEN  _@M1135_ = "Alternate1 Req.Efficiency", _@M1136_ = "Atn1.E"
				InfoBoxes[i]->SetTitle(gettext(TEXT("_@M1136_")));
				break;
			case 68:
				// LKTOKEN  _@M1137_ = "Alternate2 Req.Efficiency", _@M1138_ = "Atn2.E"
				InfoBoxes[i]->SetTitle(gettext(TEXT("_@M1138_")));
				break;
			case 69:
				// LKTOKEN  _@M1139_ = "BestAltern Req.Efficiency", _@M1140_ = "BAtn.E"
				InfoBoxes[i]->SetTitle(gettext(TEXT("_@M1140_")));
				break;
			case 75:
				// LKTOKEN  _@M1151_ = "Alternate1 Arrival", _@M1152_ = "Atn1Arr"
				InfoBoxes[i]->SetTitle(gettext(TEXT("_@M1152_")));
				break;
			case 76:
				// LKTOKEN  _@M1153_ = "Alternate2 Arrival", _@M1154_ = "Atn2Arr"
				InfoBoxes[i]->SetTitle(gettext(TEXT("_@M1154_")));
				break;
			case 77:
				// LKTOKEN  _@M1155_ = "BestAlternate Arrival", _@M1156_ = "BAtnArr"
				InfoBoxes[i]->SetTitle(gettext(TEXT("_@M1156_")));
				break;
			default:
				InfoBoxes[i]->SetTitle(TEXT("ERR234"));
				break;
		}
		InfoBoxes[i]->SetValue(TEXT("---"));
		InfoBoxes[i]->SetColor(-1);
	}
      if (needupdate)
	InfoBoxes[i]->SetValueUnit(Units::GetUserUnitByGroup(
          Data_Options[DisplayType[i]].UnitGroup));
	break;	
/*
    case 75: // VENTA3 alternate1 and 2
    case 76: 
    case 77:
	if (DisplayType[i]==75) ActiveAlternate=Alternate1; else
	if (DisplayType[i]==76) ActiveAlternate=Alternate2; 
		else ActiveAlternate=BestAlternate;
	InfoBoxes[i]->SetSmallerFont(false);
	if ( ActiveAlternate != -1 ) {
		InfoBoxes[i]->SetTitle(Data_Options[DisplayType[i]].Formatter->
			   RenderTitle(&color));
		InfoBoxes[i]->SetColor(color);
		InfoBoxes[i]->SetValue(Data_Options[DisplayType[i]].Formatter->
			   Render(&color));
		InfoBoxes[i]->SetColor(color);
	} else {
		if ( DisplayType[i]==75 ) 
			InfoBoxes[i]->SetTitle(TEXT("Alt1Arr"));
		else if ( DisplayType[i]==76 ) 
			InfoBoxes[i]->SetTitle(TEXT("Alt2Arr"));
		else	InfoBoxes[i]->SetTitle(TEXT("BAltArr"));
		InfoBoxes[i]->SetValue(TEXT("---"));
		InfoBoxes[i]->SetColor(-1);
	}
      if (needupdate)
	InfoBoxes[i]->SetValueUnit(Units::GetUserUnitByGroup(
          Data_Options[DisplayType[i]].UnitGroup));
	break;	
*/
    case 55:
      InfoBoxes[i]->SetSmallerFont(true);
      if (needupdate)
	InfoBoxes[i]->SetTitle(Data_Options[DisplayType[i]].Title);
      
      InfoBoxes[i]->
	SetValue(Data_Options[DisplayType[i]].Formatter->Render(&color));
      
      // to be optimized!
      if (needupdate)
	InfoBoxes[i]->
	  SetValueUnit(Units::GetUserUnitByGroup(
              Data_Options[DisplayType[i]].UnitGroup)
	  );
      InfoBoxes[i]->SetColor(color);
      break; 
    case 14: // Next waypoint
      InfoBoxes[i]->SetSmallerFont(false);
      if (theactive != -1){
	InfoBoxes[i]->
	  SetTitle(Data_Options[DisplayType[i]].Formatter->
		   Render(&color));
	InfoBoxes[i]->SetColor(color);
	InfoBoxes[i]->
	  SetValue(Data_Options[47].Formatter->Render(&color));
      }else{
	// LKTOKEN _@M1030_ "Next"
	InfoBoxes[i]->SetTitle(gettext(TEXT("_@M1030_")));
	InfoBoxes[i]->SetValue(TEXT("---"));
	InfoBoxes[i]->SetColor(-1);
      }
      if (needupdate)
	InfoBoxes[i]->SetValueUnit(Units::GetUserUnitByGroup(
          Data_Options[DisplayType[i]].UnitGroup)
      );
      break;
    default:
      InfoBoxes[i]->SetSmallerFont(false);
      if (needupdate)
	InfoBoxes[i]->SetTitle(Data_Options[DisplayType[i]].Title);
      
      InfoBoxes[i]->
          SetValue(Data_Options[DisplayType[i]].Formatter->Render(&color));
      
      // to be optimized!
      if (needupdate)
	InfoBoxes[i]->
	  SetValueUnit(Units::GetUserUnitByGroup(
            Data_Options[DisplayType[i]].UnitGroup)
	  );

      InfoBoxes[i]->SetColor(color);
    };
  
    // 
    // Infobox bottom line
    //
    switch (DisplayType[i]) {
    case 14: // Next waypoint
      if (theactive != -1){
        int index;
        index = Task[theactive].Index;
        if (ValidWayPoint(index)) { 
		if (WayPointList[index].Comment == NULL)
			InfoBoxes[i]-> SetComment(_T(""));
		else
			InfoBoxes[i]-> SetComment(WayPointList[index].Comment);
        }  else
		InfoBoxes[i]->SetComment(TEXT(""));
        break;
      }
      InfoBoxes[i]->SetComment(TEXT(""));
      break;
    case 79:
	if (NearestAirspaceHDist>0)
          InfoBoxes[i]->SetComment(NearestAirspaceName);
	  break;
#ifdef LKAIRSPACE
    case 114:
    if (NearestAirspaceVDist>0)
          InfoBoxes[i]->SetComment(NearestAirspaceVName);
      break;
#endif      
    case 10:
      if (CALCULATED_INFO.AutoMacCready)
		// LKTOKEN _@M1184_ "AutMC"
		InfoBoxes[i]->SetComment(gettext(TEXT("_@M1184_")));
      else
		// LKTOKEN _@M1183_ "ManMC"
		InfoBoxes[i]->SetComment(gettext(TEXT("_@M1183_")));
      break;
    case 0: // GPS Alt
      Units::FormatAlternateUserAltitude(GPS_INFO.Altitude, 
					 sTmp, sizeof(sTmp)/sizeof(sTmp[0]));
      InfoBoxes[i]->SetComment(sTmp);
      break;
    case 1: // AGL
      Units::FormatAlternateUserAltitude(CALCULATED_INFO.AltitudeAGL, 
					 sTmp, sizeof(sTmp)/sizeof(sTmp[0]));
      InfoBoxes[i]->SetComment(sTmp);
      break;
    case 33:
      Units::FormatAlternateUserAltitude(GPS_INFO.BaroAltitude, 
					 sTmp, sizeof(sTmp)/sizeof(sTmp[0]));
      InfoBoxes[i]->SetComment(sTmp);
      break;
    case 27: // AAT time to go
    case 36: // flight time
    case 39: // current time
    case 40: // gps time
    case 41: // task time to go
    case 42: // task time to go
    case 45: // ete 
    case 46: // leg ete 
    case 62: // ete 
      if (Data_Options[DisplayType[i]].Formatter->isValid()) {
        InfoBoxes[i]->
          SetComment(Data_Options[DisplayType[i]].Formatter->GetCommentText());
      } else {
        InfoBoxes[i]->
          SetComment(TEXT(""));
      }
      break;
    case 43:
      if (EnableBlockSTF) {
		// LKTOKEN _@M1195_ "BLOCK"
		InfoBoxes[i]->SetComment(gettext(TEXT("_@M1195_")));
      } else {
		// LKTOKEN _@M1196_ "DOLPHIN"
		InfoBoxes[i]->SetComment(gettext(TEXT("_@M1196_")));
      }
      break;
    case 55: // own team code      
      InfoBoxes[i]->SetComment(TeammateCode);
      if (TeamFlarmTracking)
	{
	  if (IsFlarmTargetCNInRange())
	    {				
	      InfoBoxes[i]->SetColorBottom(2);
	    }
	  else
	    {
	      InfoBoxes[i]->SetColorBottom(1);
	    }
	}
      else
	{
	  InfoBoxes[i]->SetColorBottom(0);
	}           
      break;
    case 56: // team bearing

      if (TeamFlarmIdTarget != 0)
	{
	  if (wcslen(TeamFlarmCNTarget) != 0)
	    {
	      InfoBoxes[i]->SetComment(TeamFlarmCNTarget);
	    }
	  else
	    {
	      InfoBoxes[i]->SetComment(TEXT("???"));
	    }
	}
      else
	{
	  InfoBoxes[i]->SetComment(TEXT("---"));
	}

      if (IsFlarmTargetCNInRange())
	{				
	  InfoBoxes[i]->SetColorBottom(2);
	}
      else
	{
	  InfoBoxes[i]->SetColorBottom(1);
	}

      break;
    case 57: // team bearing dif

      if (TeamFlarmIdTarget != 0)
	{
	  if (wcslen(TeamFlarmCNTarget) != 0)
	    {
	      InfoBoxes[i]->SetComment(TeamFlarmCNTarget);
	    }
	  else
	    {
	      InfoBoxes[i]->SetComment(TEXT("???"));
	    }
	}
      else
	{
	  InfoBoxes[i]->SetComment(TEXT("---"));
	}
      if (IsFlarmTargetCNInRange())
	{				
	  InfoBoxes[i]->SetColorBottom(2);
	}
      else
	{
	  InfoBoxes[i]->SetColorBottom(1);
	}

      break;
    case 58: // team range

      if (TeamFlarmIdTarget != 0)
	{
	  if (wcslen(TeamFlarmCNTarget) != 0)
	    {
	      InfoBoxes[i]->SetComment(TeamFlarmCNTarget);
	    }
	  else
	    {
	      InfoBoxes[i]->SetComment(TEXT("???"));
	    }
	}
      else
	{
	  InfoBoxes[i]->SetComment(TEXT("---"));
	}
      if (IsFlarmTargetCNInRange())
	{				
	  InfoBoxes[i]->SetColorBottom(2);
	}
      else
	{
	  InfoBoxes[i]->SetColorBottom(1);
	}

      break;
	// VENTA3 wind speed + bearing bottom line
	case 25:
		if ( CALCULATED_INFO.WindBearing == 0 )
		wsprintf(sTmp,_T("0%s"),_T(DEG)); else
		wsprintf(sTmp,_T("%1.0d%s"),(int)CALCULATED_INFO.WindBearing,_T(DEG));
		InfoBoxes[i]->SetComment(sTmp);
		break;
		
	// VENTA3 radial
	case 60: 
		if ( HomeWaypoint == -1 ) {  // should be redundant
      			InfoBoxes[i]->SetComment(TEXT(""));
			break; 
		}
		if ( CALCULATED_INFO.HomeRadial == 0 )
		wsprintf(sTmp,_T("0%s"),_T(DEG)); else
		wsprintf(sTmp,_T("%1.0d%s"),(int)CALCULATED_INFO.HomeRadial,_T(DEG));
		InfoBoxes[i]->SetComment(sTmp);
		break;

	// VENTA3 battery temperature under voltage. There is a good reason to see the temperature,
	// if available: many PNA/PDA will switch OFF during flight under direct sunlight for several
	// hours due to battery temperature too high!! The 314 does!

	// TODO: check temperature too high and set a warning flag to be used by an event or something
	#if (WINDOWSPC<1)
	case 65:
		if ( PDABatteryTemperature >0 ) {
			wsprintf(sTmp,_T("%1.0d%SC"),(int)PDABatteryTemperature,_T(DEG));
			InfoBoxes[i]->SetComment(sTmp);
		} else
      			InfoBoxes[i]->SetComment(TEXT(""));
		break;
	#endif

	// VENTA3 alternates
	case 67:
	case 68:
	case 69: 
		if ( ActiveAlternate == -1 ) {  // should be redundant
      			InfoBoxes[i]->SetComment(TEXT(""));
			break; 
		}
		if (FlipBoxValue == true) {
			Units::FormatUserDistance(WayPointCalc[ActiveAlternate].Distance,
					 sTmp, sizeof(sTmp)/sizeof(sTmp[0]));
			InfoBoxes[i]->SetComment(sTmp);
		} else {
			Units::FormatUserArrival(WayPointCalc[ActiveAlternate].AltArriv[AltArrivMode],
					 sTmp, sizeof(sTmp)/sizeof(sTmp[0]));
			InfoBoxes[i]->SetComment(sTmp);
		}
		break;
	// VENTA3 alternates
	case 75:
	case 76:
	case 77: 
		if ( ActiveAlternate == -1 ) {  // should be redundant
      			InfoBoxes[i]->SetComment(TEXT(""));
			break; 
		}
		if (FlipBoxValue == true) {
			Units::FormatUserDistance(WayPointCalc[ActiveAlternate].Distance,
					 sTmp, sizeof(sTmp)/sizeof(sTmp[0]));
			InfoBoxes[i]->SetComment(sTmp);
		} else {
			if (WayPointCalc[ActiveAlternate].GR == INVALID_GR) 
				wsprintf(sTmp,_T("---"));
			else
				wsprintf(sTmp,_T("%d"),(int)WayPointCalc[ActiveAlternate].GR);
/*
			Units::FormatUserArrival(WayPointCalc[ActiveAlternate].GR,
					 sTmp, sizeof(sTmp)/sizeof(sTmp[0]));
*/
			InfoBoxes[i]->SetComment(sTmp);
		}
		break;
	case 70: // QFE
		/*
		 // Showing the diff value offset was just interesting ;-)
		if (FlipBoxValue == true) {
			//Units::FormatUserArrival(QFEAltitudeOffset,
			Units::FormatUserAltitude(QFEAltitudeOffset,
				 sTmp, sizeof(sTmp)/sizeof(sTmp[0]));
			InfoBoxes[i]->SetComment(sTmp);
		} else {
		*/
		//Units::FormatUserArrival(GPS_INFO.Altitude,
		Units::FormatUserAltitude(GPS_INFO.Altitude,
			 sTmp, sizeof(sTmp)/sizeof(sTmp[0]));
		InfoBoxes[i]->SetComment(sTmp);
		break;

    default:
      InfoBoxes[i]->SetComment(TEXT(""));
    };

    DisplayTypeLast[i] = DisplayType[i];
    
  }
  InfoBoxLayout::Paint();

  first = false;

  UnlockNavBox();

}

#include "winbase.h"


void CommonProcessTimer()
{

  // service the GCE and NMEA queue
  if (ProgramStarted==psNormalOp) {
    InputEvents::DoQueuedEvents();
#ifdef LKAIRSPACE
	  // only shows the dialog if needed.
	  ShowAirspaceWarningsToUser();
#else
    if (RequestAirspaceWarningDialog) {
      DisplayTimeOut=0;
      RequestAirspaceWarningDialog= false;
      dlgAirspaceWarningShowDlg(RequestAirspaceWarningForce);
      RequestAirspaceWarningForce = false;
    }
#endif
    #ifndef NOFLARMGAUGE
    // update FLARM display (show/hide)
    GaugeFLARM::Show();
    #endif
  }

#if (WINDOWSPC<1)
  SystemIdleTimerReset();
#endif

  // VNT Maplock now has full control on focus/defocus on infoboxes
  if(InfoWindowActive && !UseMapLock) 
    {
      if (!dlgAirspaceWarningVisible()) {
	// JMW prevent switching to map window if in airspace warning dialog

	if(InfoBoxFocusTimeOut >= FOCUSTIMEOUTMAX)
	  {
	    SwitchToMapWindow();
	  }
	InfoBoxFocusTimeOut ++;
      }
    }

  if (DisplayLocked) {
    if(MenuTimeOut==MenuTimeoutMax) {
      if (!MapWindow::mode.AnyPan()) {
	InputEvents::setMode(TEXT("default"));
      }
    }
    MenuTimeOut++;
  }

  if (DisplayTimeOut >= DISPLAYTIMEOUTMAX) {
    BlankDisplay(true);
  } else {
    BlankDisplay(false);
  }
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
    // No need to redraw map or infoboxes if screen is blanked.
    // This should save lots of battery power due to CPU usage
    // of drawing the screen
    if (InfoBoxesDirty) {
      //JMWTEST    LockFlightData();
      AssignValues();
      DisplayText();
      InfoBoxesDirty = false;
      //JMWTEST    UnlockFlightData();
    }
  }

  //
  // maybe block/delay this if a dialog is active?
  // JMW: is done in the message function now.
  if (!dlgAirspaceWarningVisible()) {
    if (Message::Render()) {
      // turn screen on if blanked and receive a new message 
      DisplayTimeOut=0;
    }
  }

#if (EXPERIMENTAL > 0)

  if (bsms.Poll()) {
    // turn screen on if blanked and receive a new message
    DisplayTimeOut = 0;
  }

#endif

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
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_GREEN")); // 100404
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
			#if (EXPERIMENTAL > 0)
			// if comm port shut down, probably so did bluetooth dialup
			// so restart it here also.
			bsms.Shutdown();
			bsms.Initialise();
			#endif
	  
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
		if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_GREEN")); // 100404
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
  
  // write settings to vario every second
  if (itimeout % 2==0) {
    VarioWriteSettings();
  }
    
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

#if 101015
    extern void LKSimulator(void);
    LKSimulator();
#else
    LockFlightData();

    GPS_INFO.NAVWarning = FALSE;
    GPS_INFO.SatellitesUsed = 6;
    FindLatitudeLongitude(GPS_INFO.Latitude, GPS_INFO.Longitude, 
                          GPS_INFO.TrackBearing, GPS_INFO.Speed*1.0,
                          &GPS_INFO.Latitude,
                          &GPS_INFO.Longitude);
    GPS_INFO.Time+= 1.0;
    long tsec = (long)GPS_INFO.Time;
    GPS_INFO.Hour = tsec/3600;
    GPS_INFO.Minute = (tsec-GPS_INFO.Hour*3600)/60;
    GPS_INFO.Second = (tsec-GPS_INFO.Hour*3600-GPS_INFO.Minute*60);

    UnlockFlightData();
#endif
  }

  if (i%2==0) return;

#ifdef DEBUG
  // use this to test FLARM parsing/display
#ifndef GNAV
  NMEAParser::TestRoutine(&GPS_INFO);
#endif
#endif

  TriggerGPSUpdate();

  VarioWriteSettings();
}


void SwitchToMapWindow(void)
{
  DefocusInfoBox();

  SetFocus(hWndMapWindow);
  if (  MenuTimeOut< MenuTimeoutMax) {
    MenuTimeOut = MenuTimeoutMax;
  }
  if (  InfoBoxFocusTimeOut< FOCUSTIMEOUTMAX) {
    InfoBoxFocusTimeOut = FOCUSTIMEOUTMAX;
  }
}


void PopupAnalysis()
{
  DialogActive = true;
  dlgAnalysisShowModal();
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


void PopUpSelect(int Index)
{
  DialogActive = true;
  CurrentInfoType = InfoType[Index];
  StoreType(Index, InfoType[Index]);
  //  ShowWindow(hWndCB,SW_HIDE);
  FullScreen();
  SwitchToMapWindow();
  DialogActive = false;
}

#include <stdio.h>

void DebugStore(const char *Str, ...)
{
#if defined(DEBUG) && !defined(GNAV)
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
  	#if 0
	startupStoreFile = _tfopen(szFileName, TEXT("wb")); 100422
	if (startupStoreFile) {
		fclose(startupStoreFile);
	}
  	#endif
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



void HideInfoBoxes() {
  int i;
  InfoBoxesHidden = true;
  for (i=0; i<numInfoWindows+1; i++) {
    InfoBoxes[i]->SetVisible(false);
  }
}


void ShowInfoBoxes() {
  int i;
  InfoBoxesHidden = false;
  for (i=0; i<numInfoWindows; i++) {
    InfoBoxes[i]->SetVisible(true);
  }
}

#if (WINDOWSPC<1)
#ifndef GNAV
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
	// VENTA get everything ready for PNAs battery control
	pBatteryInfo->BatteryVoltage = sps.BatteryVoltage;
	pBatteryInfo->BatteryAverageCurrent = sps.BatteryAverageCurrent;
	pBatteryInfo->BatteryCurrent = sps.BatteryCurrent;
	pBatteryInfo->BatterymAHourConsumed = sps.BatterymAHourConsumed;
	pBatteryInfo->BatteryTemperature = sps.BatteryTemperature;
    }

    return result;
}
#endif
#endif

// GDI Escapes for ExtEscape()
#define QUERYESCSUPPORT    8
// The following are unique to CE
#define GETVFRAMEPHYSICAL   6144
#define GETVFRAMELEN    6145
#define DBGDRIVERSTAT    6146
#define SETPOWERMANAGEMENT   6147
#define GETPOWERMANAGEMENT   6148


typedef enum _VIDEO_POWER_STATE {
    VideoPowerOn = 1,
    VideoPowerStandBy,
    VideoPowerSuspend,
    VideoPowerOff
} VIDEO_POWER_STATE, *PVIDEO_POWER_STATE;


typedef struct _VIDEO_POWER_MANAGEMENT {
    ULONG Length;
    ULONG DPMSVersion;
    ULONG PowerState;
} VIDEO_POWER_MANAGEMENT, *PVIDEO_POWER_MANAGEMENT;

int PDABatteryPercent = 100;
int PDABatteryTemperature = 0;
int PDABatteryStatus=0; // 100125
int PDABatteryFlag=0;

void BlankDisplay(bool doblank) {

#if (WINDOWSPC>0)
  return;
#else
#ifdef GNAV
  return;
#else
  static bool oldblank = false;

  BATTERYINFO BatteryInfo; 
  BatteryInfo.acStatus = 0; // JMW initialise

  if (GetBatteryInfo(&BatteryInfo)) {
    PDABatteryPercent = BatteryInfo.BatteryLifePercent;
    PDABatteryTemperature = BatteryInfo.BatteryTemperature; 
    PDABatteryStatus=BatteryInfo.acStatus; // 100125
    PDABatteryFlag=BatteryInfo.chargeStatus; // 100125
/*
	// All you need to display extra Battery informations...

	TCHAR vtemp[1000];
	_stprintf(vtemp,_T("Battpercent=%d Volt=%d Curr=%d AvCurr=%d mAhC=%d Temp=%d Lifetime=%d Fulllife=%d\n"),
 		BatteryInfo.BatteryLifePercent, BatteryInfo.BatteryVoltage, 
 		BatteryInfo.BatteryCurrent, BatteryInfo.BatteryAverageCurrent,
 		BatteryInfo.BatterymAHourConsumed,
		BatteryInfo.BatteryTemperature, BatteryInfo.BatteryLifeTime, BatteryInfo.BatteryFullLifeTime);
	StartupStore( vtemp );
 */
  } 

  if (!EnableAutoBlank) {
    return;
  }
  if (doblank == oldblank) {
    return;
  }

  HDC gdc;
  int iESC=SETPOWERMANAGEMENT;

  gdc = ::GetDC(NULL);
  if (ExtEscape(gdc, QUERYESCSUPPORT, sizeof(int), (LPCSTR)&iESC,
                0, NULL)==0) {
    // can't do it, not supported
  } else {

    VIDEO_POWER_MANAGEMENT vpm;
    vpm.Length = sizeof(VIDEO_POWER_MANAGEMENT);
    vpm.DPMSVersion = 0x0001;

    // TODO feature: Trigger a GCE (Glide Computer Event) when
    // switching to battery mode This can be used to warn users that
    // power has been lost and you are now on battery power - ie:
    // something else is wrong

    if (doblank) {

      /* Battery status - simulator only - for safety of battery data
         note: Simulator only - more important to keep running in your plane
      */

      // JMW, maybe this should be active always...
      // we don't want the PDA to be completely depleted.

      if (BatteryInfo.acStatus==0) {
          if (PDABatteryPercent < BATTERY_WARNING) {
            DWORD LocalWarningTime = ::GetTickCount();
            if ((LocalWarningTime - BatteryWarningTime) > BATTERY_REMINDER) {
              BatteryWarningTime = LocalWarningTime;
              // TODO feature: Show the user what the batt status is.
              DoStatusMessage(TEXT("..... Organiser Battery Low"));
            }
          } else {
            BatteryWarningTime = 0;
          }
      }

      if (BatteryInfo.acStatus==0) {

        // Power off the display
        vpm.PowerState = VideoPowerOff;
        ExtEscape(gdc, SETPOWERMANAGEMENT, vpm.Length, (LPCSTR) &vpm,
                  0, NULL);
        oldblank = true;
        ScreenBlanked = true;
      } else {
        DisplayTimeOut = 0;
      }
    } else {
      if (oldblank) { // was blanked
        // Power on the display
        vpm.PowerState = VideoPowerOn;
        ExtEscape(gdc, SETPOWERMANAGEMENT, vpm.Length, (LPCSTR) &vpm,
                  0, NULL);
        oldblank = false;
        ScreenBlanked = false;
      }
    }

  }
  ::ReleaseDC(NULL, gdc);
#endif
#endif
}



void Event_SelectInfoBox(int i) {
//  int oldinfofocus = InfoFocus;

  // must do this
  InfoBoxFocusTimeOut = 0;

  if (InfoFocus>= 0) {
    FocusOnWindow(InfoFocus,false);
  }
  InfoFocus+= i;
  if (InfoFocus>=numInfoWindows) {
    InfoFocus = -1; // deactivate if wrap around
  }
  if (InfoFocus<0) {
    InfoFocus = -1; // deactivate if wrap around
  }
  if (InfoFocus<0) {
    DefocusInfoBox();
    SwitchToMapWindow();
    return;
  }

  //  SetFocus(hWndInfoWindow[InfoFocus]);
  FocusOnWindow(InfoFocus,true);
  InfoWindowActive = TRUE;
  DisplayText();

  InputEvents::setMode(TEXT("infobox"));
}


void Event_ChangeInfoBoxType(int i) {
  int j=0, k;

  if (InfoFocus<0) {
    return;
  }

  k = getInfoType(InfoFocus);
  if (i>0) {
    j = Data_Options[k].next_screen;
  }
  if (i<0) {
    j = Data_Options[k].prev_screen;
  }

  // TODO code: if i==0, go to default or reset

  setInfoType(InfoFocus, j);
  DisplayText();

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

#if 100517
  if (_tcsstr(OutBuffer, TEXT("$(")) == NULL) return false;
  a =_tcsstr(OutBuffer, TEXT("&("));
  if (a != NULL) {
	*a=_T('$');
	items=2;
  }
#else
  if (_tcsstr(OutBuffer, TEXT("$(")) == NULL) return false;
#endif

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
	if (CALCULATED_INFO.AutoMacCready) 
		// LKTOKEN _@M1202_ "Auto"
		_stprintf(tbuf,_T("%s"), gettext(TEXT("_@M1202_")));
	else
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
#ifdef LKAIRSPACE
	if (!CAirspaceManager::Instance().ValidAirspaces()) {
#else
	if (!ValidAirspace()) {
#endif
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

  // if using Final glide or "both", available only when we have a goto active
  if (_tcsstr(OutBuffer, TEXT("$(CheckAutoMc)"))) {
    if (!ValidTaskPoint(ActiveWayPoint) 
        && ((AutoMcMode==0)
	    || (AutoMcMode==2))) {
      invalid = true;
    }
    ReplaceInString(OutBuffer, TEXT("$(CheckAutoMc)"), TEXT(""), Size);
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

  if (_tcsstr(OutBuffer, TEXT("$(BoxMode)"))) {
	if ( MapWindow::IsMapFullScreen() ) invalid = true;
	ReplaceInString(OutBuffer, TEXT("$(BoxMode)"), TEXT(""), Size);
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
#if ORBITER
  if (_tcsstr(OutBuffer, TEXT("$(Orbiter"))) {
	if (!Orbiter)
		ReplaceInString(OutBuffer, TEXT("$(Orbiter)"), gettext(TEXT("_@M894_")), Size);
	else
		ReplaceInString(OutBuffer, TEXT("$(Orbiter)"), gettext(TEXT("_@M491_")), Size);

	if (!EnableThermalLocator) invalid = true;
	if (--items<=0) goto label_ret; 
  }
#endif

#if 0 // 100517
  if (_tcsstr(OutBuffer, TEXT("$(TerrainTopologyToggleName)"))) {
    char val;
    val = 0;
    if (EnableTopology) val++;
    if (EnableTerrain) val += (char)2;
    switch(val) {
    case 0:
      ReplaceInString(OutBuffer, TEXT("$(TerrainTopologyToggleName)"), 
                      TEXT("Topology\nON"), Size);
      break;
    case 1:
      ReplaceInString(OutBuffer, TEXT("$(TerrainTopologyToggleName)"), 
                      TEXT("Terrain\nON"), Size);
      break;
    case 2:
      ReplaceInString(OutBuffer, TEXT("$(TerrainTopologyToggleName)"), 
                      TEXT("Terrain\nTopology"), Size);
      break;
    case 3:
      ReplaceInString(OutBuffer, TEXT("$(TerrainTopologyToggleName)"), 
	// LKTOKEN  _@M709_ = "Terrain\nOFF" 
                      gettext(TEXT("_@M709_")), Size);
      break;
    }
	if (--items<=0) goto label_ret; // 100517
  }
  #endif


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

  CondReplaceInString(MapWindow::IsMapFullScreen(), OutBuffer, TEXT("$(FullScreenToggleActionName)"), gettext(TEXT("_@M894_")), gettext(TEXT("_@M491_")), Size);
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
  CondReplaceInString(EnableAuxiliaryInfo, OutBuffer, TEXT("$(AuxInfoToggleActionName)"), gettext(TEXT("_@M491_")), gettext(TEXT("_@M894_")), Size);
  {
  MapWindow::Mode::TModeFly userForcedMode = MapWindow::mode.UserForcedMode();
  CondReplaceInString(userForcedMode == MapWindow::Mode::MODE_FLY_CIRCLING, OutBuffer, TEXT("$(DispModeClimbShortIndicator)"), TEXT("_"), TEXT(""), Size);
  CondReplaceInString(userForcedMode == MapWindow::Mode::MODE_FLY_CRUISE, OutBuffer, TEXT("$(DispModeCruiseShortIndicator)"), TEXT("_"), TEXT(""), Size);
  CondReplaceInString(userForcedMode == MapWindow::Mode::MODE_FLY_NONE, OutBuffer, TEXT("$(DispModeAutoShortIndicator)"), TEXT("_"), TEXT(""), Size);
  CondReplaceInString(userForcedMode == MapWindow::Mode::MODE_FLY_FINAL_GLIDE, OutBuffer, TEXT("$(DispModeFinalShortIndicator)"), TEXT("_"), TEXT(""), Size);
  }

#if 0
  CondReplaceInString(AltitudeMode == ALLON, OutBuffer, TEXT("$(AirspaceModeAllShortIndicator)"), TEXT("|"), TEXT(""), Size);

  CondReplaceInString(AltitudeMode == CLIP,  OutBuffer, TEXT("$(AirspaceModeClipShortIndicator)"), TEXT("(�)"), TEXT(""), Size);
  CondReplaceInString(AltitudeMode == AUTO,  OutBuffer, TEXT("$(AirspaceModeAutoShortIndicator)"), TEXT("(*)"), TEXT(""), Size);
  CondReplaceInString(AltitudeMode == ALLBELOW, OutBuffer, TEXT("$(AirspaceModeBelowShortIndicator)"), TEXT("(*)"), TEXT(""), Size);
  CondReplaceInString(AltitudeMode == ALLOFF, OutBuffer, TEXT("$(AirspaceModeAllOffIndicator)"), TEXT("(*)"), TEXT(""), Size);
#else  // 091211
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
#endif

#if 0  //  100517 NOUSE
  CondReplaceInString(TrailActive == 0, OutBuffer, TEXT("$(SnailTrailOffShortIndicator)"), TEXT("(*)"), TEXT(""), Size);
  CondReplaceInString(TrailActive == 2, OutBuffer, TEXT("$(SnailTrailShortShortIndicator)"), TEXT("(*)"), TEXT(""), Size);
  CondReplaceInString(TrailActive == 1, OutBuffer, TEXT("$(SnailTrailLongShortIndicator)"), TEXT("(*)"), TEXT(""), Size);
  CondReplaceInString(TrailActive == 3, OutBuffer, TEXT("$(SnailTrailFullShortIndicator)"), TEXT("(*)"), TEXT(""), Size);

// VENTA3 VisualGlide
  CondReplaceInString(VisualGlide == 0, OutBuffer, TEXT("$(VisualGlideOffShortIndicator)"), TEXT("(*)"), TEXT(""), Size);
  CondReplaceInString(VisualGlide == 1, OutBuffer, TEXT("$(VisualGlideLightShortIndicator)"), TEXT("(*)"), TEXT(""), Size);
  CondReplaceInString(VisualGlide == 2, OutBuffer, TEXT("$(VisualGlideHeavyShortIndicator)"), TEXT("(*)"), TEXT(""), Size);
// VENTA3 AirSpace
  CondReplaceInString(OnAirSpace  == 0, OutBuffer, TEXT("$(AirSpaceOffShortIndicator)"), TEXT("(*)"), TEXT(""), Size);
  CondReplaceInString(OnAirSpace  == 1, OutBuffer, TEXT("$(AirSpaceOnShortIndicator)"), TEXT("(*)"), TEXT(""), Size);
#endif

  #ifndef NOFLARMGAUGE
  CondReplaceInString(EnableFLARMGauge != 0, OutBuffer, TEXT("$(FlarmDispToggleActionName)"), TEXT("OFF"), TEXT("ON"), Size);
  #endif

label_ret:

  return invalid;
}

