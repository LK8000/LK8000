/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"
#include "Modeltype.h"
#include "LKInterface.h"



//
// Set all default values for configuration.
// We need to set runtime variables later, that make use of 
// configuration values. One setting for configuration, and the
// runtime equivalent of the same setting, for the case when it
// is possible to change such runtime value with a button.
//
// ALL configurable parameters MUST be listed here. Add new items at the bottom!
//
// Let's keep this list alpha sorted like in LKPROFILES.h and load/save functions
//
// AFTER this function is called:
// * values must be normalized with ProfileAdjustVariables()
// * Runtime values must be initialised with InitRuntime()
//
void LKProfileResetDefault(void) {

  int i;

  #if TESTBENCH
  StartupStore(TEXT("... ProfileResetDefault%s"),NEWLINE);
  #endif

  AcknowledgementTime = 1800;	// keep ack level for this time, [secs]

  Units::CoordinateFormat = (CoordinateFormats_t)cfDDMMSS;

  // Units
  SpeedUnit_Config = 2;         // default is kmh
  TaskSpeedUnit_Config = 2;     // default is kph
  DistanceUnit_Config = 2;      // default is km
  LiftUnit_Config = 1;          // default m/s
  AltitudeUnit_Config = 1;      // default m

  //
  // Default infobox groups configuration
  // Should be different for each aircraft category
  //
  InfoType[0] = 1326913302;
  InfoType[1] = 1915694850;
  InfoType[2] = 403835669;
  InfoType[3] = 740895295;
  InfoType[4] = 1460275747;
  InfoType[5] = 725435674;
  InfoType[6] = 168906009;
  InfoType[7] = 387389207;

  InfoType[8] = 387389207; // totally unused!

  DisplayOrientation_Config=NORTHCIRCLE;

  DisplayTextType=0;

  AltitudeMode_Config = ALLON;
  ClipAltitude = 10000;	 // * 10
  AltWarningMargin = 1000; // * 10
  AIRSPACEWARNINGS = TRUE;
  WarningTime = 60;
  AirspaceWarningRepeatTime = 300;            // warning repeat time if not acknowledged after 5 minutes
  AirspaceWarningVerticalMargin = 1000;        // vertical distance used to calculate too close condition *10
  AirspaceWarningDlgTimeout = 30;             // airspace warning dialog auto closing in x secs
  AirspaceWarningMapLabels = 1;               // airspace warning map labels showed

  SafetyAltitudeMode = 0;

  SAFETYALTITUDEARRIVAL = 3000; // * 10 
  SAFETYALTITUDETERRAIN = 500; // *10
  SAFTEYSPEED = 55.556;

  WindCalcTime=WCALC_TIMEBACK;
  WindCalcSpeed=27.778;

  SectorType = 1;
  SectorRadius = 3000;


  for(i=0;i<AIRSPACECLASSCOUNT;i++) {
	MapWindow::iAirspaceMode[i] = 3; // Display + Warning
  } 

  MapWindow::SetAirSpaceFillType(MapWindow::asp_fill_patterns_full);
  MapWindow::SetAirSpaceOpacity(30);

  MapWindow::bAirspaceBlackOutline = false;

  TrailActive_Config = TRUE;

  EnableTrailDrift_Config = false;

  EnableThermalLocator = 1;

  FinalGlideTerrain = 1;

  AutoWindMode_Config = D_AUTOWIND_CIRCLING;

  MapWindow::zoom.CircleZoom(1);

  HomeWaypoint = -1;

  Alternate1 = -1;

  Alternate2 = -1;

  MapWindow::SnailWidthScale = 16;

  TeamCodeRefWaypoint = -1;

  StartLine = 1;

  StartRadius = 3000;

  FinishLine = 1;

  FinishRadius = 3000;

  EnableAutoBacklight = 1;

  EnableAutoSoundVolume = 0;

  AircraftCategory = 0;

  AATEnabled=FALSE;

  ExtendedVisualGlide = 0;

  if (ScreenLandscape)
	Look8000 = (Look8000_t)lxcAdvanced;
  else
	Look8000 = (Look8000_t)lxcStandard;  

  CheckSum = 1;

  PGCruiseZoom=4;
  PGAutoZoomThreshold = 5000;
  PGClimbZoom=1;
  AutoOrientScale=100;

  PGOpenTimeH=12;
  PGOpenTimeM=0;
  PGNumberOfGates=0;
  PGGateIntervalTime=30;
  PGStartOut=0;

  // These values are used on startup, but on reset change also OpenCloseTopology
  LKTopoZoomCat05=9999;		// coast area
  LKTopoZoomCat10=12;		// water labels threshold, over this realscale, no water labels are printed
  LKTopoZoomCat20=9999;		// water line
  LKTopoZoomCat30=25;		// Big Roads
  LKTopoZoomCat40=6;		// Medium road
  LKTopoZoomCat50=3;		// Small road
  LKTopoZoomCat60=8;		// Railroad
  LKTopoZoomCat70=15;		// Big cities
  LKTopoZoomCat80=9999;		// Med city
  LKTopoZoomCat90=9999;		// Small city
  LKTopoZoomCat100=3;		// Very small cities
  LKTopoZoomCat110=9999;	// city polyline area

  LKMaxLabels=70;

  IphoneGestures = (IphoneGestures_t)iphDisabled;

  PollingMode = (PollingMode_t)pollmodeDisabled;

  LKVarioBar = (LKVarioBar_t)vBarDisabled;

  LKVarioVal = (LKVarioVal_t)vValVarioVario;

  OutlinedTp_Config = (OutlinedTp_t)otLandable;

  TpFilter = (TpFilter_t)TfNoLandables;

  OverColor = (OverColor_t)OcBlack;

  DeclutterMode = (DeclutterMode_t)dmHigh;

  // full size overlay by default
  OverlaySize = 0;

  BarOpacity = 65;

  #ifdef PPC2002  
  FontRenderer = 1; // AntiAliasing
  #else
  FontRenderer = 0; // ClearType Compatible
  #endif

  GPSAltitudeOffset = 0;

  UseGeoidSeparation = 1;

  PressureHg = 0;

  CustomKeyTime = 700;
  CustomKeyModeCenter = (CustomKeyMode_t)ckToggleMap;

  CustomKeyModeLeft = (CustomKeyMode_t)ckDisabled;
  CustomKeyModeRight = (CustomKeyMode_t)ckDisabled;
  CustomKeyModeAircraftIcon = (CustomKeyMode_t)ckDisabled;
  CustomKeyModeLeftUpCorner = (CustomKeyMode_t)ckMultitargetRotate;
  CustomKeyModeRightUpCorner = (CustomKeyMode_t)ckToggleOverlays;
  CustomKeyModeCenterScreen = (CustomKeyMode_t)ckWhereAmI;

  MapBox = (MapBox_t)mbBoxed; 

  // Units labels printout
  if ((ScreenSize == (ScreenSize_t)ss240x320) ||
      (ScreenSize == (ScreenSize_t)ss272x480) ||
      (ScreenSize == (ScreenSize_t)ss320x240) )
	HideUnits = 1;
  else
	HideUnits = 0;



  BestWarning=1;

  ThermalBar=0;

  McOverlay=1;

  TrackBar=1;

  PGOptimizeRoute=true;

  GlideBarMode = (GlideBarMode_t)gbDisabled;

  ArrivalValue = (ArrivalValue_t)avAltitude;

  // 1 is showing all airports and declutter only unneeded outlandings
  NewMapDeclutter = 1;

  AverEffTime = (AverEffTime_t)ae30seconds; 

  BgMapColor_Config = 2;

  debounceTimeout = 250;

  DeviceNeedClipping=false;

  Appearance.DefaultMapWidth=206;
  // Landables style
  Appearance.IndLandable=wpLandableDefault;
  // White/Black inversion
  InverseInfoBox_Config=true; // black
  Appearance.InfoBoxModel=apImPnaGeneric;


  AutoAdvance_Config = 1;

  AutoMcMode_Config = amcEquivalent;


  AutoMacCready_Config = true;

  UseTotalEnergy_Config= false;

  WaypointsOutOfRange = 1; // include also wps out of terrain

  EnableFAIFinishHeight = false;

  Handicap = 100; // Std Cirrus

  UTCOffset = 0;

  AutoZoom_Config=false;

  MenuTimeout_Config = MENUTIMEOUTMAX;

  LockSettingsInFlight = 0;

  LoggerShortName = 0;

  EnableFLARMMap = 0;

  TerrainContrast = 128;

  TerrainBrightness = 128;

  TerrainRamp_Config = 0;

  MapWindow::GliderScreenPosition = 40;

  BallastSecsToEmpty =  120;

  #if (!defined(WINDOWSPC) || (WINDOWSPC==0))
  SetSystemTimeFromGPS = true;
  #else
  SetSystemTimeFromGPS = false;
  #endif

  AutoForceFinalGlide = false;

  UseCustomFonts = 0;

  AlarmMaxAltitude1 = 0;

  AlarmMaxAltitude2 = 0;

  AlarmMaxAltitude3 = 0;

  AlarmTakeoffSafety = 0;

  FinishMinHeight = 0;

  StartHeightRef = 0;

  StartMaxHeight = 0;
  
  StartMaxHeightMargin = 0;

  StartMaxSpeed = 0;

  StartMaxSpeedMargin = 0;

  EnableNavBaroAltitude_Config = 1;

  Orbiter_Config = 1;
  Shading_Config = 1;
  OverlayClock = 0;

  // default BB and IP is all ON
  ConfBB0 = 0; // TRM is off by default on v4
  ConfBB1 = 1;
  ConfBB2 = 1;
  ConfBB3 = 1;
  ConfBB4 = 1;
  ConfBB5 = 1;
  ConfBB6 = 1;
  ConfBB7 = 1;
  ConfBB8 = 1;
  ConfBB9 = 1;
  ConfBB0Auto = 1;

  ConfIP11 = 1;
  ConfIP12 = 1;
  ConfIP13 = 1;
  ConfIP14 = 1;
  ConfIP15 = 1;
  ConfIP16 = 1;
  ConfIP21 = 1;
  ConfIP22 = 1;
  ConfIP23 = 1;
  ConfIP24 = 1;
  ConfIP31 = 1;
  ConfIP32 = 1;
  ConfIP33 = 1;

  LoggerTimeStepCruise = 1;

  LoggerTimeStepCircling = 1;

  GlidePolar::SafetyMacCready = 0.5; // This is saved *10 and loaded /10 in Adjust! 

  DisableAutoLogger = false;
  
  LiveTrackerInterval = 0;
  
  // empty or demo versions
  //szAirspaceFile[0] = TEXT('\0');
  //szWaypointFile[0] = TEXT('\0');
  //szTerrainFile[0] = TEXT('\0');
  //szAirfieldFile[0] = TEXT('\0');
  //szMapFile[0] = TEXT('\0');
  //szPolarFile[0] = TEXT('\0');


  _tcscpy(szPolarFile,_T("%LOCAL_PATH%\\\\_Polars\\Std Cirrus.plr"));
  _tcscpy(szAirspaceFile,_T("%LOCAL_PATH%\\\\_Airspaces\\DEMO.txt"));
  szAdditionalAirspaceFile[0] = TEXT('\0');
  _tcscpy(szWaypointFile,_T("%LOCAL_PATH%\\\\_Waypoints\\DEMO.cup"));
  szAdditionalWaypointFile[0] = TEXT('\0');
  _tcscpy(szTerrainFile,_T("%LOCAL_PATH%\\\\_Maps\\DEMO.DEM"));
  _tcscpy(szAirfieldFile,_T("%LOCAL_PATH%\\\\_Waypoints\\WAYNOTES.txt"));
  _tcscpy(szLanguageFile,_T("%LOCAL_PATH%\\\\_Language\\ENGLISH.LNG"));

  szInputFile[0] = TEXT('\0');
  _tcscpy(szMapFile,_T("%LOCAL_PATH%\\\\_Maps\\DEMO.LKM"));

  // Ports and device settings
  dwDeviceName1[0]=_T('\0');
  dwPortIndex1 = 0;
  dwSpeedIndex1 = 2;
  dwBit1Index = (BitIndex_t)bit8N1;
  dwDeviceName2[0]=_T('\0');
  dwPortIndex2 = 1;
  dwSpeedIndex2 = 2;
  dwBit2Index = (BitIndex_t)bit8N1;

  FontDesc_MapWindow[0]=_T('\0');
  FontDesc_MapLabel [0]=_T('\0');

  _tcscpy(PilotName_Config,_T("WOLF.HIRTH"));
  _tcscpy(LiveTrackersrv_Config,_T("www.livetrack24.com"));
  _tcscpy(LiveTrackerusr_Config,_T("LK8000"));
  _tcscpy(LiveTrackerpwd_Config,_T(""));

  _tcscpy(AircraftType_Config,_T("CIRRUS-STD"));
  _tcscpy(AircraftRego_Config,_T("D-1900"));
  _tcscpy(CompetitionClass_Config,_T("CLUB"));
  _tcscpy(CompetitionID_Config,_T("WH"));

  LockSettingsInFlight = false;
  LoggerShortName = false;

  BUGS_Config=1; // 1=100%, 0.5 = 50% .. FLOATS!

  UseUngestures=true;

  // This is also reset by global init, but never mind. Done twice.
  extern void Reset_CustomMenu(void);
  Reset_CustomMenu();

  extern void Reset_Multimap_Flags(void);
  Reset_Multimap_Flags();

  extern void Reset_Multimap_Mode(void);
  Reset_Multimap_Mode();

   UseWindRose=false;	// use wind rose (ex: NNE) for wind direction, instead of degrees
                        // only Changed by custom Key

  // ######### ADD NEW ITEMS ABOVE THIS LINE  #########

}


