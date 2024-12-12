/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKProfiles.h"
#include "McReady.h"
#include "Modeltype.h"
#include "LKInterface.h"
#include "Asset.hpp"
#include "Multimap.h"
#include "Tracking/Tracking.h"
#include "Devices/devFanet.h"
#include "Util/StringAPI.hxx"
#ifdef ANDROID
  #include "Android/Main.hpp"
  #include "Android/NativeView.hpp"
#endif

void InitDefaultComPort() {
#ifdef ANDROID
  _tcscpy(PortConfig[0].szDeviceName, DEV_INTERNAL_NAME);

  if (StringIsEqual(native_view->GetProduct(), "AIR3")) {
    // Configure Fanet on Air3 7.3+
    _tcscpy(PortConfig[1].szDeviceName, Fanet::DeviceName);
    PortConfig[1].SetPort(_T("/dev/ttyMT2"));
    PortConfig[1].dwSpeedIndex = std::distance(std::begin(baudrate), std::find(std::begin(baudrate), std::end(baudrate), 115200));
    PortConfig[1].dwBitIndex = bit8N1;
  }
#endif
}

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
void LKProfileResetDefault() {

  int i;

  TestLog(TEXT("... ProfileResetDefault"));

  Units::ResetSettings();
  
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

  AcknowledgementTime = 1800;	// keep ack level for this time, [secs]
  AirspaceWarningRepeatTime = 900;            // warning repeat time if not acknowledged after 15 minutes
  AirspaceWarningVerticalMargin = 1000;        // vertical distance used to calculate too close condition *10
  AirspaceWarningDlgTimeout = 10;             // airspace warning dialog auto closing in x secs
  AirspaceWarningMapLabels = 1;               // airspace warning map labels showed
  AirspaceAckAllSame = 0;

  SafetyAltitudeMode = 0;

  SAFETYALTITUDEARRIVAL = 3000; // * 10
  SAFETYALTITUDETERRAIN = 500; // *10
  SAFTEYSPEED = 55.556;

  WindCalcTime=WCALC_TIMEBACK;
  WindCalcSpeed=27.778;

  SectorType = sector_type_t::SECTOR;
  SectorRadius = 3000;


  for(i=0;i<AIRSPACECLASSCOUNT;i++) {
	MapWindow::iAirspaceMode[i] = 3; // Display + Warning
  }
#ifdef HAVE_HATCHED_BRUSH
  MapWindow::SetAirSpaceFillType(MapWindow::asp_fill_patterns_full);
#else
  MapWindow::SetAirSpaceFillType(MapWindow::asp_fill_ablend_borders);
#endif
  MapWindow::SetAirSpaceOpacity(30);

  TrailActive_Config = 1; // long

  EnableTrailDrift_Config = false;

  EnableThermalLocator = 1;

  FinalGlideTerrain = 1;

  AutoWindMode_Config = D_AUTOWIND_CIRCLING;

  MapWindow::zoom.CircleZoom(1);

  HomeWaypoint = -1;

  Alternate1 = -1;

  Alternate2 = -1;

  SnailScale = MAXSNAILRESIZE;

  TeamCodeRefWaypoint = -1;

  StartLine = sector_type_t::LINE;

  StartRadius = 3000;

  FinishLine = sector_type_t::LINE;

  FinishRadius = 3000;

  EnableAutoBacklight = 1;

  EnableAutoSoundVolume = 0;

  AircraftCategory = AircraftCategory_t::umGlider;

  gTaskType=TSK_DEFAULT;

  CheckSum = 1;

  ClimbZoom=5;
  CruiseZoom=14;
  AutoZoomThreshold = 8000;
  MaxAutoZoom = 9;

  AutoOrientScale=100;

  PGOpenTimeH=12;
  PGOpenTimeM=0;
  PGCloseTimeH=23;
  PGCloseTimeM=59;

  PGNumberOfGates=0;
  PGGateIntervalTime=30;

  // These values are used on startup, but on reset change also OpenCloseTopology
  LKTopoZoomCat05=DEFAULT_WATER_LABELS_THRESHOLD;	// coast area share the same label management of cat10
  LKTopoZoomCat10=DEFAULT_WATER_LABELS_THRESHOLD;	// water labels threshold, over this realscale, no water labels are printed)
  LKTopoZoomCat20=9999;		// water lines
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

#if defined(ANDROID) || defined(KOBO)
  IphoneGestures = (IphoneGestures_t)iphEnabled;
#else
  IphoneGestures = (IphoneGestures_t)iphDisabled;
#endif

  PollingMode = (PollingMode_t)pollmodeDisabled;

  LKVarioBar = (LKVarioBar_t)vBarDisabled;

  LKVarioVal = (LKVarioVal_t)vValVarioVario;

  OutlinedTp_Config = (OutlinedTp_t)otLandable;

  TpFilter = (TpFilter_t)TfNoLandables;

  OverColor = (OverColor_t)OcBlack;

  DeclutterMode = dmHigh;

  // full size overlay by default
  OverlaySize = 0;
  if (IsDithered())
      BarOpacity = 100;
  else
      BarOpacity = 75;

  #ifdef PPC2002
  FontRenderer = 1; // AntiAliasing
  #else
  FontRenderer = 0; // ClearType Compatible
  #endif

  GPSAltitudeOffset = 0;

  UseGeoidSeparation = true;

  CustomKeyTime = 700;

  CustomKeyModeCenter = CustomKeyMode_t::ckToggleMap;
  CustomKeyModeLeft = CustomKeyMode_t::ckDisabled;
  CustomKeyModeRight = CustomKeyMode_t::ckDisabled;
  CustomKeyModeAircraftIcon = CustomKeyMode_t::ckDisabled;
  CustomKeyModeLeftUpCorner = CustomKeyMode_t::ckMultitargetRotate;
  CustomKeyModeRightUpCorner = CustomKeyMode_t::ckToggleOverlays;

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

  TrackBar=1;

  TskOptimizeRoute=true;
  TskOptimizeRoute_Config = true;
  GliderSymbol = 0;  // Default depending on mode type

  GlideBarMode = (GlideBarMode_t)gbDisabled;

  ArrivalValue = (ArrivalValue_t)avAltitude;

  // 1 is showing all airports and declutter only unneeded outlandings
  NewMapDeclutter = dmLow;

  AverEffTime = (AverEffTime_t)ae30seconds;

  if (IsDithered())
      BgMapColor_Config = 0; // white
  else
      BgMapColor_Config = 2; // LCD green

  debounceTimeout = 180; // 180ms;

  DeviceNeedClipping=false;

  // Landables style
  Appearance.IndLandable=wpLandableDefault;
  // White/Black inversion
  InverseInfoBox_Config=true; // black

  Appearance.UTF8Pictorials = false;

  AutoAdvance_Config = 1;

  AutoMcMode_Config = amcEquivalent;


  AutoMacCready_Config = true;

  UseTotalEnergy_Config= false;

  WaypointsOutOfRange = 1; // include also wps out of terrain

  EnableFAIFinishHeight = false;

  Handicap = 100; // Std Cirrus

  UTCOffset = GetSystemUTCOffset();

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

#if defined(PPC2003) || defined(PNA)
  SetSystemTimeFromGPS = true;
#endif
  SaveRuntime = false;

  AutoForceFinalGlide = false;

  AlarmMaxAltitude1 = 0;

  AlarmMaxAltitude2 = 0;

  AlarmMaxAltitude3 = 0;

  AlarmTakeoffSafety = 0;

  GearWarningMode=0;

  GearWarningAltitude=200;

  FinishMinHeight = 0;

  StartHeightRef = 0;

  StartMaxHeight = 0;

  StartMaxHeightMargin = 0;

  StartMaxSpeed = 0;

  StartMaxSpeedMargin = 0;

  EnableNavBaroAltitude_Config = 1;

  Orbiter_Config = 1;
  Shading_Config = 1;
  IsoLine_Config = false;
  OverlayClock = 0;
  UseTwoLines = 1;
  SonarWarning_Config = 1; // sonar enabled by default on reset
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
  ConfIP17 = 1;
  ConfIP21 = 1;
  ConfIP22 = 1;
  ConfIP23 = 1;
  ConfIP24 = 1;
  ConfIP31 = 1;
  ConfIP32 = 1;
  ConfIP33 = 1;

  GlidePolar::SafetyMacCready = 0.5; // This is saved *10 and loaded /10 in Adjust!

  DisableAutoLogger = false;

  // empty or demo versions
  //szAirspaceFile[0] = TEXT('\0');
  //szWaypointFile[0] = TEXT('\0');
  //szTerrainFile[0] = TEXT('\0');
  //szAirfieldFile[0] = TEXT('\0');
  //szMapFile[0] = TEXT('\0');
  //szPolarFile[0] = TEXT('\0');


  _tcscpy(szPolarFile,_T(LKD_DEFAULT_POLAR));
  for(unsigned int i = 0; i < NO_AS_FILES; i++)
    _tcscpy(szAirspaceFile[0],_T(""));
  _tcscpy(szAirspaceFile[0],_T("DEMO.txt"));

  for(unsigned int i = 0; i < NO_WP_FILES; i++)
    _tcscpy(szWaypointFile[0],_T(""));
  _tcscpy(szWaypointFile[0],_T("DEMO.cup"));
  szAdditionalWaypointFile[0] = TEXT('\0');
  _tcscpy(szTerrainFile,_T("DEMO.DEM"));
  _tcscpy(szAirfieldFile,_T("WAYNOTES.txt"));
  szLanguageCode[0] = TEXT('\0');

  szInputFile[0] = TEXT('\0');
  _tcscpy(szMapFile,_T("DEMO.LKM"));

  // Ports and device settings
  for (auto& Port : PortConfig) {
    Port = PortConfig_t();
  }

  InitDefaultComPort();

  _tcscpy(PilotName_Config,_T("WOLF.HIRTH"));

  tracking::ResetSettings();

  _tcscpy(AircraftType_Config,_T("CIRRUS-STD"));
  _tcscpy(AircraftRego_Config,_T("D-1900"));
  _tcscpy(CompetitionClass_Config,_T("CLUB"));
  _tcscpy(CompetitionID_Config,_T("WH"));

  LockSettingsInFlight = false;
  LoggerShortName = false;

  BUGS_Config=1; // 1=100%, 0.5 = 50% .. FLOATS!

  UseUngestures=true;

  // This is also reset by global init, but never mind. Done twice.
  Reset_CustomMenu();
  Reset_Multimap_Flags();
  Reset_Multimap_Mode();

   UseWindRose=false;	// use wind rose (ex: NNE) for wind direction, instead of degrees
                        // only Changed by custom Key

  Flags_DrawTask = true;
  Flags_DrawFAI_config = false;
  Flags_DrawXC_config = false;
  FAI28_45Threshold = FAI_BIG_THRESHOLD;
  BottomMode=BM_CRU;
  iFlarmDirection=0;
  AspPermanentChanged=0;

  FontMapWaypoint=MAXFONTRESIZE;
  FontMapTopology=MAXFONTRESIZE;
  FontInfopage1L=MAXFONTRESIZE;
  FontInfopage2L=MAXFONTRESIZE;
  FontBottomBar=MAXFONTRESIZE;
  FontCustom1=MAXFONTRESIZE;
  FontOverlayBig=MAXFONTRESIZE;
  FontOverlayMedium=MAXFONTRESIZE;
  FontVisualGlide=MAXFONTRESIZE;

  // Overlay config
  Overlay_TopLeft=1;
  Overlay_TopMid=1;
  Overlay_TopRight=1;
  Overlay_TopDown=1;
  Overlay_LeftTop=1;
  Overlay_LeftMid=1;
  Overlay_LeftBottom=1;
  Overlay_LeftDown=1;
  Overlay_RightTop=1;
  Overlay_RightMid=1;
  Overlay_RightBottom=1;

  AdditionalContestRule = CContestMgr::ContestRule::OLC;  // OLC by default


#ifdef _WGS84
  earth_model_wgs84 = true;
#endif

  AutoContrast=true;
  TerrainWhiteness=1;

  EnableAudioVario = false;

  ModelType::ResetSettings();

  // ######### ADD NEW ITEMS ABOVE THIS LINE  #########

}
