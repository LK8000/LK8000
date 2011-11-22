/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Utils.h"
#include "McReady.h"
#include "Modeltype.h"
#include "Calculations.h"
//#include "Parser.h"


#if NEWPROFILES
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
// * values must be normalized with ProfileAdjustVariables
// * Runtime values must be initialised with INITRUNTIME
//
void LKProfileResetDefault(void) {

  int i;

  #if TESTBENCH
  StartupStore(TEXT("... ProfileResetDefault%s"),NEWLINE);
  #endif

  AcknowledgementTime = 900;	// keep ack level for this time, [secs]
  ActiveMap_Config = 0;

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
  InfoType[0] = 1008146198;
  InfoType[1] = 1311715074;
  InfoType[2] = 923929365;
  InfoType[3] = 975776319;
  InfoType[4] = 956959267;
  InfoType[5] = 1178420506;
  InfoType[6] = 1410419993;
  InfoType[7] = 1396384771;
  InfoType[8] = 387389207;

  DisplayOrientation_Config=TRACKUP;

  DisplayTextType=0;

  AltitudeMode = ALLON;
  ClipAltitude = 1000;
  AltWarningMargin = 100;
  AIRSPACEWARNINGS = TRUE;
  WarningTime = 60;
  AirspaceWarningRepeatTime = 300;            // warning repeat time if not acknowledged after 5 minutes
  AirspaceWarningVerticalMargin = 100;        // vertical distance used to calculate too close condition
  AirspaceWarningDlgTimeout = 30;             // airspace warning dialog auto closing in x secs
  AirspaceWarningMapLabels = 1;               // airspace warning map labels showed

  SafetyAltitudeMode = 0;

  SAFETYALTITUDEARRIVAL = 300;
  SAFETYALTITUDETERRAIN = 50;
  SAFTEYSPEED = 50.0;

  WindCalcTime=WCALC_TIMEBACK;
  WindCalcSpeed=27.778;

  SectorType = 1;
  SectorRadius = 10000;


  for(i=0;i<AIRSPACECLASSCOUNT;i++) {
	MapWindow::iAirspaceMode[i] = 3; // Display + Warning
  } 

  MapWindow::SetAirSpaceFillType(MapWindow::asp_fill_patterns_full);
  MapWindow::SetAirSpaceOpacity(30);

  MapWindow::bAirspaceBlackOutline = false;

  TrailActive_Config = TRUE;

  MapWindow::EnableTrailDrift = false;

  EnableThermalLocator = 1;

  EnableTopology_Config = 1;

  EnableTerrain_Config = 1;

  FinalGlideTerrain = 1;

  AutoWindMode= D_AUTOWIND_CIRCLING;

  MapWindow::zoom.CircleZoom(1);

  HomeWaypoint = -1;

  Alternate1 = -1;

  Alternate2 = -1;

  MapWindow::SnailWidthScale = 16;

  TeamCodeRefWaypoint = -1;

  StartLine = 1;

  StartRadius = 1000;

  FinishLine = 1;

  FinishRadius = 1000;

  EnableAutoBacklight = 1;

  EnableAutoSoundVolume = 1;

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
  AutoOrientScale=10;

  PGOpenTimeH=12;
  PGOpenTimeM=0;
  PGNumberOfGates=0;
  PGGateIntervalTime=30;
  PGStartOut=0;

  LKTopoZoomCat05=9999;
  LKTopoZoomCat10=9999;
  LKTopoZoomCat20=9999;
  LKTopoZoomCat30=9999;
  LKTopoZoomCat40=9999;
  LKTopoZoomCat50=9999;
  LKTopoZoomCat60=9999;
  LKTopoZoomCat70=9999;
  LKTopoZoomCat80=9999;
  LKTopoZoomCat90=9999;
  LKTopoZoomCat100=9999;
  LKTopoZoomCat110=9999;

  LKMaxLabels=70;

  IphoneGestures = (IphoneGestures_t)iphDisabled;

  PollingMode = (PollingMode_t)pollmodeDisabled;

  LKVarioBar = (LKVarioBar_t)vBarDisabled;

  LKVarioVal = (LKVarioVal_t)vValVarioVario;

  OutlinedTp = (OutlinedTp_t)otLandable;

  TpFilter = (TpFilter_t)TfNoLandables;

  OverColor = (OverColor_t)OcBlack;

  DeclutterMode = (DeclutterMode_t)dmMedium;

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
  CustomKeyModeCenter = (CustomKeyMode_t)ckDisabled;

  CustomKeyModeLeft = (CustomKeyMode_t)ckDisabled;
  CustomKeyModeRight = (CustomKeyMode_t)ckDisabled;
  CustomKeyModeAircraftIcon = (CustomKeyMode_t)ckDisabled;
  CustomKeyModeLeftUpCorner = (CustomKeyMode_t)ckMultitargetRotate;
  CustomKeyModeRightUpCorner = (CustomKeyMode_t)ckMultitargetMenu;
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

  PGOptimizeRoute=1;

  GlideBarMode = (GlideBarMode_t)gbDisabled;

  ArrivalValue = (ArrivalValue_t)avAltitude;

  // 1 is showing all airports and declutter only unneeded outlandings
  NewMapDeclutter = 1;

  // This need AircraftCategory been set !
  AverEffTime = (AverEffTime_t)ae2minutes; 

  BgMapColor_Config = 2;

  debounceTimeout = 250;

  needclipping=false;

  Appearance.DefaultMapWidth=206;
  // Landables style
  Appearance.IndLandable=wpLandableDefault,
  // Black/White inversion
  InverseInfoBox_Config=false;
  Appearance.InfoBoxModel=apImPnaGeneric;


  AutoAdvance_Config = 1;

  AutoMcMode_Config = amcEquivalent;


  AutoMacCready_Config = true;

  UseTotalEnergy_Config= false;

  WaypointsOutOfRange = 1; // include also wps out of terrain

  EnableFAIFinishHeight = false;

  Handicap = 108;

  UTCOffset = 0;

  MapWindow::zoom.AutoZoom(false); // CHECK! TODO

  MenuTimeout_Config = MENUTIMEOUTMAX;

  LockSettingsInFlight = 0;

  LoggerShortName = 0;

  EnableFLARMMap = 1;

  TerrainContrast = 140;

  TerrainBrightness = 115;

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
  ConfBB1 = 1;
  ConfBB2 = 1;
  ConfBB3 = 1;
  ConfBB4 = 1;
  ConfBB5 = 1;
  ConfBB6 = 1;
  ConfBB7 = 1;
  ConfBB8 = 1;
  ConfBB9 = 1;

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

  GlidePolar::SafetyMacCready = 0.5;

  DisableAutoLogger = false;

  szPolarFile[0] = TEXT('\0');
  szAirspaceFile[0] = TEXT('\0');
  szAdditionalAirspaceFile[0] = TEXT('\0');
  szWaypointFile[0] = TEXT('\0');
  szAdditionalWaypointFile[0] = TEXT('\0');
  szTerrainFile[0] = TEXT('\0');
  szTopologyFile[0] = TEXT('\0');
  szAirfieldFile[0] = TEXT('\0');
  szLanguageFile[0] = TEXT('\0');
  szInputFile[0] = TEXT('\0');
  szMapFile[0] = TEXT('\0');

  // Ports and device settings
  dwDeviceName1[0]=_T('\0');
  dwPortIndex1 = 0;
  dwSpeedIndex1 = 2;
  dwBit1Index = (BitIndex_t)bit8N1;
  dwDeviceName2[0]=_T('\0');
  dwPortIndex2 = 0;
  dwSpeedIndex2 = 2;
  dwBit2Index = (BitIndex_t)bit8N1;

  FontDesc_MapWindow[0]=_T('\0');
  FontDesc_MapLabel [0]=_T('\0');

  _tcscpy(PilotName_Config,_T(""));
  _tcscpy(AircraftType_Config,_T(""));
  _tcscpy(AircraftRego_Config,_T(""));
  _tcscpy(CompetitionClass_Config,_T(""));
  _tcscpy(CompetitionID_Config,_T(""));

  LockSettingsInFlight = false;
  LoggerShortName = false;



  // ######### ADD NEW ITEMS ABOVE THIS LINE  #########

}



#if 0
#ifdef PNA
// LOAD ModelType directly at startup
extern bool SetModelName(DWORD Temp);

bool SetModelType() {

  TCHAR sTmp[100];
  TCHAR szRegistryInfoBoxModel[]= TEXT("AppInfoBoxModel");
  DWORD Temp=0;

  GetFromRegistry(szRegistryInfoBoxModel, &Temp);
  
  if ( SetModelName(Temp) != true ) {
	_stprintf(sTmp,_T(". SetModelType failed: probably no registry entry%s"), NEWLINE);
	StartupStore(sTmp);
	GlobalModelType=MODELTYPE_PNA_PNA;
	_tcscpy(GlobalModelName,_T("GENERIC"));  // 100820
	return false;
  } else {
	GlobalModelType = Temp;
  }
  
  _stprintf(sTmp,_T(". SetModelType: Name=<%s> Type=%d%s"),GlobalModelName, GlobalModelType,NEWLINE);
  StartupStore(sTmp);
  return true;
}
#endif
#endif // 0


#endif // NEWPROFILES
