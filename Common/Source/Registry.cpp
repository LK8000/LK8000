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


#ifdef __MINGW32__
#ifndef max
#define max(x, y)   (x > y ? x : y)
#define min(x, y)   (x < y ? x : y)
#endif
#endif


const TCHAR szRegistryKey[] = TEXT(REGKEYNAME);
const TCHAR *szRegistryDisplayType[MAXINFOWINDOWS] =     { TEXT("Info0"),
				       TEXT("Info1"),
				       TEXT("Info2"),
				       TEXT("Info3"),
				       TEXT("Info4"),
				       TEXT("Info5"),
				       TEXT("Info6"),
				       TEXT("Info7"),
				       TEXT("Info8"),
				       TEXT("Info9")
				       TEXT("Info10")
				       TEXT("Info11")
				       TEXT("Info12")
				       TEXT("Info13")
}; // pL

const TCHAR *szRegistryColour[] =     { TEXT("Colour0"),
				  TEXT("Colour1"),
				  TEXT("Colour2"),
				  TEXT("Colour3"),
				  TEXT("Colour4"),
				  TEXT("Colour5"),
				  TEXT("Colour6"),
				  TEXT("Colour7"),
				  TEXT("Colour8"),
				  TEXT("Colour9"),
				  TEXT("Colour10"),
				  TEXT("Colour11"),
				  TEXT("Colour12"),
				  TEXT("Colour13"),
                  TEXT("Colour14"),
                  TEXT("Colour15")
}; // pL


const TCHAR *szRegistryBrush[] =     {  TEXT("Brush0"),
				  TEXT("Brush1"),
				  TEXT("Brush2"),
				  TEXT("Brush3"),
				  TEXT("Brush4"),
				  TEXT("Brush5"),
				  TEXT("Brush6"),
				  TEXT("Brush7"),
				  TEXT("Brush8"),
				  TEXT("Brush9"),
				  TEXT("Brush10"),
				  TEXT("Brush11"),
				  TEXT("Brush12"),
				  TEXT("Brush13"),
                  TEXT("Brush14"),
                  TEXT("Brush15")
}; // pL

const TCHAR *szRegistryAirspaceMode[] =     {  TEXT("AirspaceMode0"),
					       TEXT("AirspaceMode1"),
					       TEXT("AirspaceMode2"),
					       TEXT("AirspaceMode3"),
					       TEXT("AirspaceMode4"),
					       TEXT("AirspaceMode5"),
					       TEXT("AirspaceMode6"),
					       TEXT("AirspaceMode7"),
					       TEXT("AirspaceMode8"),
					       TEXT("AirspaceMode9"),
					       TEXT("AirspaceMode10"),
					       TEXT("AirspaceMode11"),
					       TEXT("AirspaceMode12"),
					       TEXT("AirspaceMode13"),
                           TEXT("AirspaceMode14"),
                           TEXT("AirspaceMode15")
}; // pL


const TCHAR *szRegistryAirspacePriority[] = {  TEXT("AirspacePriority0"),
					 TEXT("AirspacePriority1"),
					 TEXT("AirspacePriority2"),
					 TEXT("AirspacePriority3"),
					 TEXT("AirspacePriority4"),
					 TEXT("AirspacePriority5"),
					 TEXT("AirspacePriority6"),
					 TEXT("AirspacePriority7"),
					 TEXT("AirspacePriority8"),
					 TEXT("AirspacePriority9"),
					 TEXT("AirspacePriority10"),
					 TEXT("AirspacePriority11"),
					 TEXT("AirspacePriority12"),
					 TEXT("AirspacePriority13"),
                     TEXT("AirspacePriority14"),
                     TEXT("AirspacePriority15")
}; // pL


const TCHAR szRegistryAirspaceWarning[]= TEXT("AirspaceWarn");
const TCHAR szRegistryAirspaceBlackOutline[]= TEXT("AirspaceBlackOutline1");
const TCHAR szRegistryAirspaceFillType[]= TEXT("AirspaceFillType");
const TCHAR szRegistryAirspaceOpacity[]= TEXT("AirspaceOpacity");
const TCHAR szRegistryAirspaceWarningRepeatTime[]= TEXT("AirspaceWarningRepeatTime1");
const TCHAR szRegistryAirspaceWarningVerticalMargin[]= TEXT("AirspaceWarningVerticalMargin");
const TCHAR szRegistryAirspaceWarningDlgTimeout[]= TEXT("AirspaceWarningDlgTimeout");
const TCHAR szRegistryAirspaceWarningMapLabels[]= TEXT("AirspaceWarningMapLabels");
const TCHAR szRegistryAltMargin[]=	   TEXT("AltMargin");
const TCHAR szRegistryAltMode[]=  TEXT("AltitudeMode");
const TCHAR szRegistrySafetyAltitudeMode[]=  TEXT("SafetyAltitudeMode");
const TCHAR szRegistryAltitudeUnitsValue[] = TEXT("Altitude");
const TCHAR szRegistryCircleZoom[]= TEXT("CircleZoom");
const TCHAR szRegistryClipAlt[]= TEXT("ClipAlt");
const TCHAR szRegistryDisplayText[] = TEXT("DisplayText2");
const TCHAR szRegistryDisplayUpValue[] = TEXT("DisplayUp");
const TCHAR szRegistryDistanceUnitsValue[] = TEXT("Distance");
const TCHAR szRegistryDrawTerrain[]= TEXT("DrawTerrain");
const TCHAR szRegistryDrawTopology[]= TEXT("DrawTopology");
const TCHAR szRegistryFAISector[] = TEXT("FAISector");
const TCHAR szRegistryFinalGlideTerrain[]= TEXT("FinalGlideTerrain");
const TCHAR szRegistryAutoWind[]= TEXT("AutoWind");
const TCHAR szRegistryHomeWaypoint[]= TEXT("HomeWaypoint1");
const TCHAR szRegistryAlternate1[]= TEXT("Alternate1a");
const TCHAR szRegistryAlternate2[]= TEXT("Alternate2a");
const TCHAR szRegistryLiftUnitsValue[] = TEXT("Lift");
const TCHAR szRegistryLatLonUnits[] = TEXT("LatLonUnits");
const TCHAR szRegistryPort1Index[]= TEXT("PortIndex");
const TCHAR szRegistryPort2Index[]= TEXT("Port2Index");
const TCHAR szRegistryRegKey[]=				 TEXT("RegKey");
const TCHAR szRegistrySafetyAltitudeArrival[] =     TEXT("SafetyAltitudeArrival");
const TCHAR szRegistrySafetyAltitudeTerrain[] =     TEXT("SafetyAltitudeTerrain");
const TCHAR szRegistrySafteySpeed[] =          TEXT("SafteySpeed");
const TCHAR szRegistryWindCalcSpeed[] =          TEXT("WindCalcSpeed");
const TCHAR szRegistryWindCalcTime[] =          TEXT("WindCalcTime");
const TCHAR szRegistrySectorRadius[]=          TEXT("Radius");
const TCHAR szRegistrySnailTrail[]=		 TEXT("SnailTrail");
const TCHAR szRegistryTrailDrift[]=		 TEXT("TrailDrift");
const TCHAR szRegistryThermalLocator[]=	 TEXT("ThermalLocator");
const TCHAR szRegistrySpeed1Index[]=		 TEXT("SpeedIndex");
const TCHAR szRegistrySpeed2Index[]=		 TEXT("Speed2Index");
#if USEPORT3
const TCHAR szRegistryPort3Index[]= TEXT("Port3Index");
const TCHAR szRegistrySpeed3Index[]=		 TEXT("Speed3Index");
const TCHAR szRegistryBit3Index[]=		 TEXT("Bit3Index");
#endif
const TCHAR szRegistryBit1Index[]=		 TEXT("Bit1Index");
const TCHAR szRegistryBit2Index[]=		 TEXT("Bit2Index");
const TCHAR szRegistryCheckSum[]=		 TEXT("CheckSum");
const TCHAR szRegistrySpeedUnitsValue[] =      TEXT("Speed");
const TCHAR szRegistryTaskSpeedUnitsValue[] =      TEXT("TaskSpeed");
const TCHAR szRegistryStartLine[]=		 TEXT("StartLine");
const TCHAR szRegistryStartRadius[]=		 TEXT("StartRadius");
const TCHAR szRegistryFinishLine[]=		 TEXT("FinishLine");
const TCHAR szRegistryFinishRadius[]=		 TEXT("FinishRadius");
const TCHAR szRegistryWarningTime[]=		 TEXT("WarnTime");
const TCHAR szRegistryAcknowledgementTime[]=	 TEXT("AcknowledgementTime");
const TCHAR szRegistryWindUpdateMode[] =       TEXT("WindUpdateMode");
const TCHAR szRegistryWindSpeed[] =            TEXT("WindSpeed");
const TCHAR szRegistryWindBearing[] =          TEXT("WindBearing");
const TCHAR szRegistryAirfieldFile[]=  TEXT("AirfieldFile"); // pL
const TCHAR szRegistryAirspaceFile[]=  TEXT("AirspaceFile"); // pL
const TCHAR szRegistryAdditionalAirspaceFile[]=  TEXT("AdditionalAirspaceFile"); // pL
const TCHAR szRegistryPolarFile[] = TEXT("PolarFile"); // pL
const TCHAR szRegistryTerrainFile[]=	 TEXT("TerrainFile"); // pL
const TCHAR szRegistryTopologyFile[]=  TEXT("TopologyFile"); // pL
const TCHAR szRegistryWayPointFile[]=  TEXT("WPFile"); // pL
const TCHAR szRegistryAdditionalWayPointFile[]=  TEXT("AdditionalWPFile"); // pL
const TCHAR szRegistryLanguageFile[]=  TEXT("LanguageFile"); // pL
const TCHAR szRegistryStatusFile[]=  TEXT("StatusFile"); // pL
const TCHAR szRegistryInputFile[]=  TEXT("InputFile"); // pL
const TCHAR szRegistryPilotName[]=  TEXT("PilotName");
const TCHAR szRegistryAircraftType[]=  TEXT("AircraftType");
const TCHAR szRegistryAircraftRego[]=  TEXT("AircraftRego");
const TCHAR szRegistryCompetitionClass[]=  TEXT("CompetitionClass");
const TCHAR szRegistryCompetitionID[]=  TEXT("CompetitionID");
const TCHAR szRegistryLoggerID[]=  TEXT("LoggerID");
const TCHAR szRegistryLoggerShort[]=  TEXT("LoggerShortName");

const TCHAR szRegistryDeviceA[]= TEXT("DeviceA");
const TCHAR szRegistryDeviceB[]= TEXT("DeviceB");
const TCHAR szRegistryDeviceC[]= TEXT("DeviceC");

const TCHAR szRegistryAutoBacklight[]= TEXT("AutoBacklight");
const TCHAR szRegistryAutoSoundVolume[]= TEXT("AutoSoundVolume");
const TCHAR szRegistryAircraftCategory[]= TEXT("AircraftCategory");
const TCHAR szRegistryExtendedVisualGlide[]= TEXT("ExtVisualGlide");
const TCHAR szRegistryLook8000[]= TEXT("Look8000");
const TCHAR szRegistryAltArrivMode[]= TEXT("AltArrivMode");
const TCHAR szRegistryActiveMap[]= TEXT("ActiveMap"); // 100318
const TCHAR szRegistryBestWarning[]= TEXT("BestWarning");
const TCHAR szRegistryThermalBar[]= TEXT("ThermalBar");
const TCHAR szRegistryMcOverlay[]= TEXT("McOverlay2");
const TCHAR szRegistryTrackBar[]= TEXT("TrackBar");
const TCHAR szRegistryPGOptimizeRoute[]= TEXT("PGOptimizeRoute");
const TCHAR szRegistryIphoneGestures[]= TEXT("IphoneGestures");
const TCHAR szRegistryPollingMode[]= TEXT("PollingMode");
const TCHAR szRegistryLKVarioBar[]= TEXT("LKVarioBar");
const TCHAR szRegistryLKVarioVal[]= TEXT("LKVarioVal");
const TCHAR szRegistryHideUnits[]= TEXT("HideUnits");
const TCHAR szRegistryGlideBarMode[]= TEXT("GlideBarMode");
const TCHAR szRegistryOutlinedTp[]= TEXT("OutlinedTp");
const TCHAR szRegistryOverColor[]= TEXT("OverColor");
const TCHAR szRegistryTpFilter[]= TEXT("TpFilter");
const TCHAR szRegistryDeclutterMode[]= TEXT("DeclutterMode");
const TCHAR szRegistryMapBox[]= TEXT("MapBox");
const TCHAR szRegistryArrivalValue[]= TEXT("ArrivalValue");
const TCHAR szRegistryNewMapDeclutter[]= TEXT("NewMapDeclutter");
const TCHAR szRegistryAverEffTime[]= TEXT("AverEffTime");
const TCHAR szRegistryBgMapColor[]= TEXT("BgMapColor");

const TCHAR szRegistryDebounceTimeout[]= TEXT("DebounceTimeout");

const TCHAR szRegistryPGCruiseZoom[]= TEXT("PGCruiseZoom");
const TCHAR szRegistryPGClimbZoom[]= TEXT("PGClimbZoom");
const TCHAR szRegistryAutoOrientScale[]= TEXT("AutoOrientScale");
const TCHAR szRegistryPGOpenTimeH[]= TEXT("PGOpenTimeH");
const TCHAR szRegistryPGOpenTimeM[]= TEXT("PGOpenTimeM");
const TCHAR szRegistryPGNumberOfGates[]= TEXT("PGNumberOfGates");
const TCHAR szRegistryLKTopoZoomCat05[]= TEXT("LKTopoZoomCat05");
const TCHAR szRegistryLKTopoZoomCat10[]= TEXT("LKTopoZoomCat10");
const TCHAR szRegistryLKTopoZoomCat20[]= TEXT("LKTopoZoomCat20");
const TCHAR szRegistryLKTopoZoomCat30[]= TEXT("LKTopoZoomCat30");
const TCHAR szRegistryLKTopoZoomCat40[]= TEXT("LKTopoZoomCat40");
const TCHAR szRegistryLKTopoZoomCat50[]= TEXT("LKTopoZoomCat50");
const TCHAR szRegistryLKTopoZoomCat60[]= TEXT("LKTopoZoomCat60");
const TCHAR szRegistryLKTopoZoomCat70[]= TEXT("LKTopoZoomCat70");
const TCHAR szRegistryLKTopoZoomCat80[]= TEXT("LKTopoZoomCat80");
const TCHAR szRegistryLKTopoZoomCat90[]= TEXT("LKTopoZoomCat90");
const TCHAR szRegistryLKTopoZoomCat100[]= TEXT("LKTopoZoomCat100");
const TCHAR szRegistryLKTopoZoomCat110[]= TEXT("LKTopoZoomCat110");
const TCHAR szRegistryLKMaxLabels[]= TEXT("LKMaxLabels");
const TCHAR szRegistryOverlaySize[]= TEXT("OverlaySize");
const TCHAR szRegistryBarOpacity[]= TEXT("BarOpacity");
const TCHAR szRegistryFontRenderer[]= TEXT("FontRenderer2");
const TCHAR szRegistryPGGateIntervalTime[]= TEXT("PGGateIntervalTime");
const TCHAR szRegistryPGStartOut[]= TEXT("PGStartOut");

const TCHAR szRegistryAppIndLandable[] = TEXT("AppIndLandable");
const TCHAR szRegistryAppInverseInfoBox[] = TEXT("AppInverseInfoBox2");
const TCHAR szRegistryAppDefaultMapWidth[] = TEXT("AppDefaultMapWidth");
const TCHAR szRegistryTeamcodeRefWaypoint[] = TEXT("TeamcodeRefWaypoint1");

const TCHAR szRegistryAppInfoBoxModel[] = TEXT("AppInfoBoxModel"); 
const TCHAR szRegistryGpsAltitudeOffset[] = TEXT("GpsAltitudeOffset");
const TCHAR szRegistryUseGeoidSeparation[] = TEXT("UseGeoidSeparation");
const TCHAR szRegistryPressureHg[] = TEXT("PressureHg");
const TCHAR szRegistryCustomKeyTime[] = TEXT("CustomKeyTime");
const TCHAR szRegistryCustomKeyModeCenter[] = TEXT("CustomKeyModeCenter");
const TCHAR szRegistryCustomKeyModeLeft[] = TEXT("CustomKeyModeLeft");
const TCHAR szRegistryCustomKeyModeRight[] = TEXT("CustomKeyModeRight");
const TCHAR szRegistryCustomKeyModeAircraftIcon[] = TEXT("CustomKeyModeAircraftIcon");
const TCHAR szRegistryCustomKeyModeLeftUpCorner[] = TEXT("CustomKeyModeLeftUpCorner");
const TCHAR szRegistryCustomKeyModeRightUpCorner[] = TEXT("CustomKeyModeRightUpCorner");
const TCHAR szRegistryCustomKeyModeCenterScreen[] = TEXT("CustomKeyModeCenterScreen");

const TCHAR szRegistryAutoAdvance[] = TEXT("AutoAdvance");
const TCHAR szRegistryUTCOffset[] = TEXT("UTCOffset");
const TCHAR szRegistryAutoZoom[] = TEXT("AutoZoom");
const TCHAR szRegistryMenuTimeout[] = TEXT("MenuTimeout");
const TCHAR szRegistryLockSettingsInFlight[] = TEXT("LockSettingsInFlight");
const TCHAR szRegistryTerrainContrast[] = TEXT("TerrainContrast");
const TCHAR szRegistryTerrainBrightness[] = TEXT("TerrainBrightness");
const TCHAR szRegistryTerrainRamp[] = TEXT("TerrainRamp");
const TCHAR szRegistryEnableFLARMMap[] = TEXT("EnableFLARMDisplay1");
const TCHAR szRegistryGliderScreenPosition[] = TEXT("GliderScreenPosition");
const TCHAR szRegistrySetSystemTimeFromGPS[] = TEXT("SetSystemTimeFromGPS");
const TCHAR szRegistryAutoForceFinalGlide[] = TEXT("AutoForceFinalGlide");

const TCHAR szRegistryAlarmMaxAltitude1[]= TEXT("AlarmMaxAltitude1");
const TCHAR szRegistryAlarmMaxAltitude2[]= TEXT("AlarmMaxAltitude3");
const TCHAR szRegistryAlarmMaxAltitude3[]= TEXT("AlarmMaxAltitude4");

const TCHAR szRegistryFinishMinHeight[]= TEXT("FinishMinHeight");
const TCHAR szRegistryStartMaxHeight[]= TEXT("StartMaxHeight");
const TCHAR szRegistryStartMaxSpeed[]= TEXT("StartMaxSpeed");
const TCHAR szRegistryStartMaxHeightMargin[]= TEXT("StartMaxHeightMargin");
const TCHAR szRegistryStartMaxSpeedMargin[]= TEXT("StartMaxSpeedMargin");
const TCHAR szRegistryStartHeightRef[] = TEXT("StartHeightRef");
const TCHAR szRegistryEnableNavBaroAltitude[] = TEXT("EnableNavBaroAltitude");
const TCHAR szRegistryOrbiter[] = TEXT("Orbiter");
const TCHAR szRegistryShading[] = TEXT("Shading");
const TCHAR szRegistryConfBB1[] = TEXT("ConfBB1");
const TCHAR szRegistryConfBB2[] = TEXT("ConfBB2");
const TCHAR szRegistryConfBB3[] = TEXT("ConfBB3");
const TCHAR szRegistryConfBB4[] = TEXT("ConfBB4");
const TCHAR szRegistryConfBB5[] = TEXT("ConfBB5");
const TCHAR szRegistryConfBB6[] = TEXT("ConfBB6");
const TCHAR szRegistryConfBB7[] = TEXT("ConfBB7");
const TCHAR szRegistryConfBB8[] = TEXT("ConfBB8");
const TCHAR szRegistryConfBB9[] = TEXT("ConfBB9");

const TCHAR szRegistryConfIP11[] = TEXT("ConfIP11");
const TCHAR szRegistryConfIP12[] = TEXT("ConfIP12");
const TCHAR szRegistryConfIP13[] = TEXT("ConfIP13");
const TCHAR szRegistryConfIP14[] = TEXT("ConfIP14");
const TCHAR szRegistryConfIP15[] = TEXT("ConfIP15");
const TCHAR szRegistryConfIP16[] = TEXT("ConfIP16");
const TCHAR szRegistryConfIP21[] = TEXT("ConfIP21");
const TCHAR szRegistryConfIP22[] = TEXT("ConfIP22");
const TCHAR szRegistryConfIP23[] = TEXT("ConfIP23");
const TCHAR szRegistryConfIP24[] = TEXT("ConfIP24");
const TCHAR szRegistryConfIP31[] = TEXT("ConfIP31");
const TCHAR szRegistryConfIP32[] = TEXT("ConfIP32");
const TCHAR szRegistryConfIP33[] = TEXT("ConfIP33");

const TCHAR szRegistryOverlayClock[] = TEXT("OverlayClock");

const TCHAR szRegistryLoggerTimeStepCruise[]= TEXT("LoggerTimeStepCruise");
const TCHAR szRegistryLoggerTimeStepCircling[]= TEXT("LoggerTimeStepCircling");

const TCHAR szRegistrySafetyMacCready[] = TEXT("SafetyMacCready");
const TCHAR szRegistryAutoMcMode[] = TEXT("AutoMcMode");
const TCHAR szRegistryAutoMcStatus[] = TEXT("AutoMcStatus");
const TCHAR szRegistryUseTotalEnergy[] = TEXT("UseTotalEnergy");
const TCHAR szRegistryWaypointsOutOfRange[] = TEXT("WaypointsOutOfRange2"); // 101020 2nd version, reset needed
const TCHAR szRegistryEnableExternalTriggerCruise[] = TEXT("EnableExternalTriggerCruise");
const TCHAR szRegistryFAIFinishHeight[] = TEXT("FAIFinishHeight");
const TCHAR szRegistryHandicap[] = TEXT("Handicap");
const TCHAR szRegistrySnailWidthScale[] = TEXT("SnailWidthScale");
const TCHAR szRegistryDisableAutoLogger[] = TEXT("DisableAutoLogger");
const TCHAR szRegistryMapFile[]=	 TEXT("MapFile"); // pL
const TCHAR szRegistryBallastSecsToEmpty[]=	 TEXT("BallastSecsToEmpty"); 
const TCHAR szRegistryUseCustomFonts[]=	 TEXT("UseCustomFonts"); 
const TCHAR szRegistryFontMapLabelFont[]=	 TEXT("MapLabelFont"); 
const TCHAR szRegistryFontMapWindowFont[]=	 TEXT("MapWindowFont"); 

bool LockSettingsInFlight = true;
bool LoggerShortName = false;

double COSTABLE[4096];
double SINETABLE[4096];
double INVCOSINETABLE[4096];
int ISINETABLE[4096];
int ICOSTABLE[4096];

void StoreType(int Index,int infoType)
{
  SetToRegistry(szRegistryDisplayType[Index],(DWORD)infoType); // CHECK BUGFIX 091007
}


void SetRegistryStringIfAbsent(const TCHAR* name,
			       const TCHAR* value) {

  // VENTA force fonts registry rewrite in PNAs 
  // VENTA TODO WARNING should really delete the key before creating it TODO
  SetRegistryString(name, value);
}


void SaveRegistryAirspacePriority() {
  for (int i=0; i<AIRSPACECLASSCOUNT; i++) {
    SetToRegistry(szRegistryAirspacePriority[i], AirspacePriority[i]);
  }
}


void ReadRegistrySettings(void)
{
  DWORD Speed = 0;
  DWORD Distance = 0;
  DWORD TaskSpeed = 0;
  DWORD Lift = 0;
  DWORD Altitude = 0;
//  DWORD DisplayUp = 0;
  DWORD Temp = 0;
  //DWORD Temp2 = 0; // 091207
  int i;

  #if TESTBENCH
  StartupStore(TEXT(". Read registry settings%s"),NEWLINE);
  #endif

  for (i=0; i<AIRSPACECLASSCOUNT; i++) {
    Temp=0;
    GetFromRegistry(szRegistryAirspacePriority[i], &Temp);
    AirspacePriority[i] = Temp;
  }

  Temp = 0;
  GetFromRegistry(szRegistryLatLonUnits, &Temp);
  Units::CoordinateFormat = (CoordinateFormats_t)Temp;

  // default Speed unit 
  Speed=2; // 100219
  GetFromRegistry(szRegistrySpeedUnitsValue,&Speed);
  switch(Speed)
    {
    case 0 :
      SPEEDMODIFY = TOMPH;
      break;
    case 1 :
      SPEEDMODIFY = TOKNOTS;
      break;
    case 2 :
      SPEEDMODIFY = TOKPH;
      break;
    }

  TaskSpeed = 2;
  GetFromRegistry(szRegistryTaskSpeedUnitsValue,&TaskSpeed);
  switch(TaskSpeed)
    {
    case 0 :
      TASKSPEEDMODIFY = TOMPH;
      break;
    case 1 :
      TASKSPEEDMODIFY = TOKNOTS;
      break;
    case 2 :
      TASKSPEEDMODIFY = TOKPH;
      break;
    }

  Distance=2; // 100219
  GetFromRegistry(szRegistryDistanceUnitsValue,&Distance);
  switch(Distance)
    {
    case 0 : DISTANCEMODIFY = TOMILES; break;
    case 1 : DISTANCEMODIFY = TONAUTICALMILES; break;
    case 2 : DISTANCEMODIFY = TOKILOMETER; break;
    }

  Altitude=1; // 100219
  GetFromRegistry(szRegistryAltitudeUnitsValue,&Altitude);
  switch(Altitude)
    {
    case 0 : ALTITUDEMODIFY = TOFEET; break;
    case 1 : ALTITUDEMODIFY = TOMETER; break;
    }

  Lift=1; // 100219
  GetFromRegistry(szRegistryLiftUnitsValue,&Lift);
  switch(Lift)
    {
    case 0 : LIFTMODIFY = TOKNOTS; break;
    case 1 : LIFTMODIFY = TOMETER; break;
    case 2 : LIFTMODIFY = TOKNOTS; break; // 100314
    }

  Units::NotifyUnitChanged();

  for (i=0;i<MAXINFOWINDOWS;i++)
    {
      Temp = InfoType[i];
      GetFromRegistry(szRegistryDisplayType[i],&Temp);
      InfoType[i] = Temp;
    }

  Temp=0;
  GetFromRegistry(szRegistryDisplayUpValue,&Temp);
  switch(Temp)
    {
    case TRACKUP : DisplayOrientation = TRACKUP; break;
    case NORTHUP : DisplayOrientation = NORTHUP;break;
    case NORTHSMART : 
		DisplayOrientation = NORTHSMART;
		/*
		if (InfoBoxLayout::landscape) // 100417
			DisplayOrientation = NORTHSMART;
		else
			DisplayOrientation = NORTHUP;
		*/
		break;
    case NORTHCIRCLE : DisplayOrientation = NORTHCIRCLE;break;
    case TRACKCIRCLE : DisplayOrientation = TRACKCIRCLE;break;
    case NORTHTRACK : DisplayOrientation = NORTHTRACK;break;
    }
  OldDisplayOrientation=DisplayOrientation; // 101008

  Temp=0;
  GetFromRegistry(szRegistryDisplayText,&Temp);
  DisplayTextType=Temp;

  Temp=AltitudeMode;
  if(GetFromRegistry(szRegistryAltMode,&Temp)==ERROR_SUCCESS)
    AltitudeMode = Temp;

  Temp=ClipAltitude;
  if(GetFromRegistry(szRegistryClipAlt,&Temp)==ERROR_SUCCESS)
    ClipAltitude = Temp;

  Temp=AltWarningMargin;
  if(GetFromRegistry(szRegistryAltMargin,&Temp)==ERROR_SUCCESS)
    AltWarningMargin = Temp;

  Temp=AirspaceWarningRepeatTime;
  if(GetFromRegistry(szRegistryAirspaceWarningRepeatTime,&Temp)==ERROR_SUCCESS)
    AirspaceWarningRepeatTime = Temp;

  Temp=AirspaceWarningVerticalMargin;
  if(GetFromRegistry(szRegistryAirspaceWarningVerticalMargin,&Temp)==ERROR_SUCCESS)
    AirspaceWarningVerticalMargin = Temp;

  Temp=AirspaceWarningDlgTimeout;
  if(GetFromRegistry(szRegistryAirspaceWarningDlgTimeout,&Temp)==ERROR_SUCCESS)
    AirspaceWarningDlgTimeout = Temp;

  Temp=AirspaceWarningMapLabels;
  if(GetFromRegistry(szRegistryAirspaceWarningMapLabels,&Temp)==ERROR_SUCCESS)
    AirspaceWarningMapLabels = Temp;
	
  Temp=SafetyAltitudeMode;
  if(GetFromRegistry(szRegistrySafetyAltitudeMode,&Temp)==ERROR_SUCCESS)
    SafetyAltitudeMode = Temp;

  Temp=(DWORD)SAFETYALTITUDEARRIVAL;
  GetFromRegistry(szRegistrySafetyAltitudeArrival,&Temp);
  SAFETYALTITUDEARRIVAL = (double)Temp;

  Temp=(DWORD)SAFETYALTITUDETERRAIN;
  GetFromRegistry(szRegistrySafetyAltitudeTerrain,&Temp);
  SAFETYALTITUDETERRAIN = (double)Temp;

  // SAFTEYSPEED is in ms, default 50.0 ie 180kmh
  // Value is saved multiplied by 1000 to preserve decimal calculations for conversion to knots
  Temp=1234567; // 091207
  GetFromRegistry(szRegistrySafteySpeed,&Temp);
  if (Temp!=1234567) SAFTEYSPEED = (double)Temp/1000.0; // 091207
  if (SAFTEYSPEED <8) SAFTEYSPEED=50.0; // 091215 do not let SAFTEYSPEED be below 30kmh

  Temp=1234567; // 100112
  GetFromRegistry(szRegistryWindCalcSpeed,&Temp);
  if (Temp!=1234567) WindCalcSpeed = (double)Temp/1000.0; 
  // WindCalcSpeed is INDICATED AIR SPEED in m/s , minimum is 8kmh default is 100kmh
  if (WindCalcSpeed <2) WindCalcSpeed=27.778;

  // time is in seconds
  Temp=WindCalcTime;
  GetFromRegistry(szRegistryWindCalcTime,&Temp);
  WindCalcTime=Temp;

  Temp = SectorType;
  GetFromRegistry(szRegistryFAISector,&Temp);
  SectorType = Temp;

  SectorRadius = 10000;
  GetFromRegistry(szRegistrySectorRadius,
		  &SectorRadius);

  GetRegistryString(szRegistryRegKey, strRegKey, 65);

  for(i=0;i<AIRSPACECLASSCOUNT;i++)
    {
      MapWindow::iAirspaceMode[i] = GetRegistryAirspaceMode(i);

      Temp= MapWindow::iAirspaceBrush[i];
      if(GetFromRegistry(szRegistryBrush[i],&Temp)==ERROR_SUCCESS)
        MapWindow::iAirspaceBrush[i] =			(int)Temp;

      Temp= MapWindow::iAirspaceColour[i];
      if(GetFromRegistry(szRegistryColour[i],&Temp)==ERROR_SUCCESS)
        MapWindow::iAirspaceColour[i] =			(int)Temp;

      if (MapWindow::iAirspaceColour[i]>= NUMAIRSPACECOLORS) {
        MapWindow::iAirspaceColour[i]= 0;
      }

      if (MapWindow::iAirspaceBrush[i]>= NUMAIRSPACEBRUSHES) {
        MapWindow::iAirspaceBrush[i]= 0;
      }

    }

  Temp = MapWindow::GetAirSpaceFillType();
  if(GetFromRegistry(szRegistryAirspaceFillType,&Temp) == ERROR_SUCCESS)
    MapWindow::SetAirSpaceFillType((MapWindow::EAirspaceFillType)Temp);

  Temp = MapWindow::GetAirSpaceOpacity();
  if(GetFromRegistry(szRegistryAirspaceOpacity,&Temp) == ERROR_SUCCESS)
    MapWindow::SetAirSpaceOpacity(Temp);

  Temp = MapWindow::bAirspaceBlackOutline;
  GetFromRegistry(szRegistryAirspaceBlackOutline,&Temp);
  MapWindow::bAirspaceBlackOutline = (Temp == 1);

  Temp = TrailActive;
  GetFromRegistry(szRegistrySnailTrail,&Temp);
  TrailActive = Temp;

  Temp = MapWindow::EnableTrailDrift;
  GetFromRegistry(szRegistryTrailDrift,&Temp);
  MapWindow::EnableTrailDrift = (Temp==1);

  Temp = EnableThermalLocator;
  GetFromRegistry(szRegistryThermalLocator,&Temp);
  EnableThermalLocator = Temp;

  Temp  = EnableTopology;
  GetFromRegistry(szRegistryDrawTopology,&Temp);
  EnableTopology = (Temp == 1);

  Temp  = EnableTerrain;
  GetFromRegistry(szRegistryDrawTerrain,&Temp);
  EnableTerrain = (Temp == 1);

  Temp  = FinalGlideTerrain;
  GetFromRegistry(szRegistryFinalGlideTerrain,&Temp);
  FinalGlideTerrain = Temp;

  Temp  = AutoWindMode;
  GetFromRegistry(szRegistryAutoWind,&Temp);
  AutoWindMode = Temp;

  Temp  = MapWindow::zoom.CircleZoom();
  GetFromRegistry(szRegistryCircleZoom,&Temp);
  MapWindow::zoom.CircleZoom(Temp == 1);

  Temp  = WindUpdateMode;
  GetFromRegistry(szRegistryWindUpdateMode,&Temp);
  WindUpdateMode = Temp;

  Temp = HomeWaypoint;
  if (GetFromRegistry(szRegistryHomeWaypoint,&Temp)==ERROR_SUCCESS) {
    HomeWaypoint = Temp;
  } else {
    HomeWaypoint = -1;
  }

  // alternate can be -1 , signed!!
  Temp = Alternate1;
  if (GetFromRegistry(szRegistryAlternate1,&Temp)==ERROR_SUCCESS) {
    Alternate1 = (signed int) Temp;
  } else {
    Alternate1 = -1;
  }

  Temp = Alternate2;
  if (GetFromRegistry(szRegistryAlternate2,&Temp)==ERROR_SUCCESS) {
    Alternate2 = (signed int) Temp; // 100429
  } else {
    Alternate2 = -1;
  }


  Temp = MapWindow::SnailWidthScale;
  GetFromRegistry(szRegistrySnailWidthScale,&Temp);
  MapWindow::SnailWidthScale = Temp;

  Temp = TeamCodeRefWaypoint;
  GetFromRegistry(szRegistryTeamcodeRefWaypoint,&Temp);
  TeamCodeRefWaypoint = Temp;

  Temp = 1;
  GetFromRegistry(szRegistryStartLine,&Temp);
  StartLine = Temp;

  Temp = 1000;
  GetFromRegistry(szRegistryStartRadius,&Temp);
  StartRadius = Temp;

  Temp = 1;
  GetFromRegistry(szRegistryFinishLine,&Temp);
  FinishLine = Temp;

  Temp = 1000;
  GetFromRegistry(szRegistryFinishRadius,&Temp);
  FinishRadius = Temp;

  Temp = AIRSPACEWARNINGS;
  GetFromRegistry(szRegistryAirspaceWarning,&Temp);
  AIRSPACEWARNINGS = Temp;

  Temp = WarningTime;
  GetFromRegistry(szRegistryWarningTime,&Temp);
  WarningTime = max(10,Temp);

  Temp = AcknowledgementTime;
  GetFromRegistry(szRegistryAcknowledgementTime,&Temp);
  AcknowledgementTime = max(10,Temp);

  Temp = 1;
  GetFromRegistry(szRegistryAutoBacklight,&Temp); // VENTA4
  EnableAutoBacklight = (Temp == 1);

  Temp = 1;
  GetFromRegistry(szRegistryAutoSoundVolume,&Temp); // VENTA4
  EnableAutoSoundVolume = (Temp == 1);

  Temp = 0;
  GetFromRegistry(szRegistryAircraftCategory,&Temp);
  AircraftCategory = Temp;
  if (ISPARAGLIDER) {
	AATEnabled=TRUE;
  }

  Temp = 0;
  GetFromRegistry(szRegistryExtendedVisualGlide,&Temp); // VENTA4
  ExtendedVisualGlide = Temp;

  // Do not allow LK8000 mode be disabled. The reserved (0) mode is unused right now.
  if (ScreenLandscape) {
	Temp = (Look8000_t)lxcAdvanced;  
	GetFromRegistry(szRegistryLook8000,&Temp);
	if (Temp == 0) Temp = (Look8000_t)lxcAdvanced;
  } else {
	Temp = (Look8000_t)lxcStandard;  
	GetFromRegistry(szRegistryLook8000,&Temp);
	if (Temp == 0) Temp = (Look8000_t)lxcStandard;
  }
  Look8000 = Temp;

  // AltArriv registry disabled 
#if (0)
  Temp = ALTA_SMC;
  GetFromRegistry(szRegistryAltArrivMode,&Temp);
  AltArrivMode = Temp;
#endif

  Temp = 1; 
  GetFromRegistry(szRegistryCheckSum,&Temp); 
  CheckSum = Temp;

  Temp=4;
  GetFromRegistry(szRegistryPGCruiseZoom,&Temp);
  PGCruiseZoom=Temp;
  Temp=1;
  GetFromRegistry(szRegistryPGClimbZoom,&Temp);
  PGClimbZoom=Temp;
  Temp=10;
  GetFromRegistry(szRegistryAutoOrientScale,&Temp);
  AutoOrientScale=Temp;

  Temp=12;
  GetFromRegistry(szRegistryPGOpenTimeH,&Temp);
  PGOpenTimeH=Temp;
  Temp=0;
  GetFromRegistry(szRegistryPGOpenTimeM,&Temp);
  PGOpenTimeM=Temp;
  Temp=0;
  GetFromRegistry(szRegistryPGNumberOfGates,&Temp);
  PGNumberOfGates=Temp;
  Temp=30;
  GetFromRegistry(szRegistryPGGateIntervalTime,&Temp);
  PGGateIntervalTime=Temp;
  Temp=0;
  GetFromRegistry(szRegistryPGStartOut,&Temp);
  PGStartOut=(Temp==1);

  PGOpenTime=((PGOpenTimeH*60)+PGOpenTimeM)*60;
  PGCloseTime=PGOpenTime+(PGGateIntervalTime*PGNumberOfGates*60);
  if (PGCloseTime>86399) PGCloseTime=86399; // 23:59:59

  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat05,&Temp);
  LKTopoZoomCat05=Temp;
  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat10,&Temp);
  LKTopoZoomCat10=Temp;
  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat20,&Temp);
  LKTopoZoomCat20=Temp;
  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat30,&Temp);
  LKTopoZoomCat30=Temp;
  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat40,&Temp);
  LKTopoZoomCat40=Temp;
  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat50,&Temp);
  LKTopoZoomCat50=Temp;
  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat60,&Temp);
  LKTopoZoomCat60=Temp;
  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat70,&Temp);
  LKTopoZoomCat70=Temp;
  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat80,&Temp);
  LKTopoZoomCat80=Temp;
  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat90,&Temp);
  LKTopoZoomCat90=Temp;
  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat100,&Temp);
  LKTopoZoomCat100=Temp;
  Temp=9999;
  GetFromRegistry(szRegistryLKTopoZoomCat110,&Temp);
  LKTopoZoomCat110=Temp;

  Temp=70;
  GetFromRegistry(szRegistryLKMaxLabels,&Temp);
  LKMaxLabels=Temp;

  Temp = (IphoneGestures_t)iphDisabled;
  GetFromRegistry(szRegistryIphoneGestures,&Temp);
  IphoneGestures = Temp;

  Temp = (PollingMode_t)pollmodeDisabled;
  GetFromRegistry(szRegistryPollingMode,&Temp);
  PollingMode = Temp;

  if (ScreenLandscape)
	//Temp = (LKVarioBar_t)vBarVarioColor;
	Temp = (LKVarioBar_t)vBarDisabled;
  else
	Temp = (LKVarioBar_t)vBarDisabled;
  GetFromRegistry(szRegistryLKVarioBar,&Temp);
  LKVarioBar = Temp;

  Temp = (LKVarioVal_t)vValVarioVario;
  GetFromRegistry(szRegistryLKVarioVal,&Temp);
  LKVarioVal = Temp;

  Temp = (OutlinedTp_t)otLandable;  // VENTA6
  GetFromRegistry(szRegistryOutlinedTp,&Temp); 
  OutlinedTp = Temp;

  Temp = (TpFilter_t)TfNoLandables;
  GetFromRegistry(szRegistryTpFilter,&Temp); 
  TpFilter = Temp;

  Temp = (OverColor_t)OcBlack;
  GetFromRegistry(szRegistryOverColor,&Temp); 
  OverColor = Temp;
  SetOverColorRef();

  Temp = (DeclutterMode_t)dmMedium;  // 100724
  GetFromRegistry(szRegistryDeclutterMode,&Temp); 
  DeclutterMode = Temp;

  Temp = 0; // full size overlay by default
  GetFromRegistry(szRegistryOverlaySize,&Temp); 
  OverlaySize = Temp;

  Temp = 60; // black bottom bar by default
  GetFromRegistry(szRegistryBarOpacity,&Temp); 
  BarOpacity = Temp;

  #ifdef PPC2002  
  Temp = 1;  // AntiAliasing
  #else
  Temp = 0;  // ClearType Compatible
  #endif
  GetFromRegistry(szRegistryFontRenderer,&Temp); 
  FontRenderer = Temp;

  Temp = 0;
  GetFromRegistry(szRegistryGpsAltitudeOffset,&Temp); 
  // USE SIGNED CASTINGO CONVERSION FOR SIGNED VALUES IN THE REGISTRY!!
  if (Temp>9999999) {
	int sword;
	sword=(Temp-9999999)*-1;
	GPSAltitudeOffset = sword;
  } else {
	GPSAltitudeOffset = Temp;
  }

  Temp = 1;
  GetFromRegistry(szRegistryUseGeoidSeparation,&Temp); 
  UseGeoidSeparation = Temp;

  Temp = 0;
  GetFromRegistry(szRegistryPressureHg,&Temp); 
  PressureHg = Temp;

  Temp = 700;
  GetFromRegistry(szRegistryCustomKeyTime,&Temp); 
  CustomKeyTime = Temp;
  Temp = (CustomKeyMode_t)ckDisabled;
  GetFromRegistry(szRegistryCustomKeyModeCenter,&Temp); 
  CustomKeyModeCenter = Temp;
  Temp = (CustomKeyMode_t)ckDisabled;
  GetFromRegistry(szRegistryCustomKeyModeLeft,&Temp); 
  CustomKeyModeLeft = Temp;
  Temp = (CustomKeyMode_t)ckDisabled;
  GetFromRegistry(szRegistryCustomKeyModeRight,&Temp); 
  CustomKeyModeRight = Temp;
  Temp = (CustomKeyMode_t)ckDisabled;
  GetFromRegistry(szRegistryCustomKeyModeAircraftIcon,&Temp); 
  CustomKeyModeAircraftIcon = Temp;
  Temp = (CustomKeyMode_t)ckMultitargetRotate;
  GetFromRegistry(szRegistryCustomKeyModeLeftUpCorner,&Temp); 
  CustomKeyModeLeftUpCorner = Temp;
  Temp = (CustomKeyMode_t)ckMultitargetMenu;
  GetFromRegistry(szRegistryCustomKeyModeRightUpCorner,&Temp); 
  CustomKeyModeRightUpCorner = Temp;
  Temp = (CustomKeyMode_t)ckWhereAmI;
  GetFromRegistry(szRegistryCustomKeyModeCenterScreen,&Temp); 
  CustomKeyModeCenterScreen = Temp;

  Temp = (MapBox_t)mbBoxed;  // VENTA6
  GetFromRegistry(szRegistryMapBox,&Temp); 
  MapBox = Temp;

  if ((ScreenSize == (ScreenSize_t)ss240x320) ||
      (ScreenSize == (ScreenSize_t)ss272x480) ||
      (ScreenSize == (ScreenSize_t)ss320x240) )
	Temp=1;
  else
  	Temp = 0; 
  GetFromRegistry(szRegistryHideUnits,&Temp); 
  HideUnits = (Temp==1);

  Temp = 0;
  GetFromRegistry(szRegistryActiveMap,&Temp); 
  ActiveMap = (Temp==1);

  Temp=1;
  GetFromRegistry(szRegistryBestWarning,&Temp); 
  BestWarning=Temp;

  Temp=0; // not active by default 100219
  GetFromRegistry(szRegistryThermalBar,&Temp); 
  ThermalBar=Temp;

  Temp=1;
  GetFromRegistry(szRegistryMcOverlay,&Temp); 
  McOverlay=Temp;

  Temp=1;
  GetFromRegistry(szRegistryTrackBar,&Temp); 
  TrackBar=Temp;

  Temp=1;
  GetFromRegistry(szRegistryPGOptimizeRoute,&Temp); 
  PGOptimizeRoute=Temp;

  if (ScreenLandscape)
	//Temp = (GlideBarMode_t)gbFinish;
	Temp = (GlideBarMode_t)gbDisabled;
  else
	Temp = (GlideBarMode_t)gbDisabled;
  GetFromRegistry(szRegistryGlideBarMode,&Temp); 
  GlideBarMode = Temp;

  Temp = (ArrivalValue_t)avAltitude;  // VENTA6
  GetFromRegistry(szRegistryArrivalValue,&Temp); 
  ArrivalValue = Temp;

  Temp = 1;  //  no enums here. 1 is show all airports and declutter only unneeded outlandings
  GetFromRegistry(szRegistryNewMapDeclutter,&Temp); 
  NewMapDeclutter = Temp;

  if ( AircraftCategory == (AircraftCategory_t)umParaglider )
  	Temp = (AverEffTime_t)ae15seconds; 
  else
  	Temp = (AverEffTime_t)ae2minutes; 
  GetFromRegistry(szRegistryAverEffTime,&Temp); 
  AverEffTime = Temp;

  if ( AircraftCategory == (AircraftCategory_t)umParaglider )
  	Temp = 2; 
  else
  	Temp = 2; 
  GetFromRegistry(szRegistryBgMapColor,&Temp); 
  BgMapColor = Temp;

  Temp = 250;
  GetFromRegistry(szRegistryDebounceTimeout, &Temp);
  debounceTimeout = Temp;

  // new appearance variables

  Temp = (IndLandable_t)wpLandableAltA;
  GetFromRegistry(szRegistryAppIndLandable, &Temp);
  Appearance.IndLandable = (IndLandable_t)Temp;

  Temp = 1; 	// DEFAULT LK8000 091019
  GetFromRegistry(szRegistryAppInverseInfoBox, &Temp);
  Appearance.InverseInfoBox = (Temp != 0);


#if (WINDOWSPC<1)
  if (GlobalModelType == MODELTYPE_PNA_HP31X ) {
			needclipping=true;
			// key transcoding for this one
			StartupStore(TEXT(". Loading HP31X settings%s"),NEWLINE);
	}
	else 
	if (GlobalModelType == MODELTYPE_PNA_PN6000 ) {
			StartupStore(TEXT(". Loading PN6000 settings%s"),NEWLINE);
			// key transcoding for this one
	}
	else 
	if (GlobalModelType == MODELTYPE_PNA_MIO ) {
			StartupStore(TEXT(". Loading MIO settings%s"),NEWLINE);
			// currently no special settings from MIO but need to handle hw keys
	}
	else
	if (GlobalModelType == MODELTYPE_PNA_NOKIA_500 ) {
			StartupStore(TEXT(". Loading Nokia500 settings%s"),NEWLINE);
			// key transcoding is made
	}
	else
	if (GlobalModelType == MODELTYPE_PNA_MEDION_P5 ) {
			StartupStore(TEXT(".Loading Medion settings%s"),NEWLINE);
			needclipping=true;
	}
	if (GlobalModelType == MODELTYPE_PNA_NAVIGON ) {
			StartupStore(TEXT(".Loading Navigon settings%s"),NEWLINE);
			needclipping=true;
	}
	else
	if (GlobalModelType == MODELTYPE_PNA_PNA ) {
		StartupStore(TEXT(". Loading default PNA settings%s"),NEWLINE);
	}
	else
		StartupStore(TEXT(". No special regsets for this device%s"),NEWLINE); // VENTA2
#endif

// VENTA-ADDON Model change
  Temp = Appearance.InfoBoxModel;
  GetFromRegistry(szRegistryAppInfoBoxModel, &Temp);
  Appearance.InfoBoxModel = (InfoBoxModelAppearance_t)Temp;

  Temp = Appearance.DefaultMapWidth;
  GetFromRegistry(szRegistryAppDefaultMapWidth, &Temp);
  Appearance.DefaultMapWidth = Temp;

  Temp = 1;
  GetFromRegistry(szRegistryAutoAdvance,&Temp);
  AutoAdvance = (Temp == 1);

  Temp = AutoMcMode_Config;
  GetFromRegistry(szRegistryAutoMcMode,&Temp);
  AutoMcMode_Config = Temp;
  AutoMcMode = Temp;

  Temp = AutoMacCready_Config;
  GetFromRegistry(szRegistryAutoMcStatus,&Temp);
  AutoMacCready_Config = Temp;
  // AutoMacCready in calculations.h is an int, should be a bool
  CALCULATED_INFO.AutoMacCready = AutoMacCready_Config==true?1:0;

  Temp = UseTotalEnergy_Config;
  GetFromRegistry(szRegistryUseTotalEnergy,&Temp);
  UseTotalEnergy_Config=Temp;
  UseTotalEnergy = UseTotalEnergy_Config;

  Temp = WaypointsOutOfRange;
  GetFromRegistry(szRegistryWaypointsOutOfRange,&Temp);
  WaypointsOutOfRange = Temp;

  Temp = EnableFAIFinishHeight;
  GetFromRegistry(szRegistryFAIFinishHeight,&Temp);
  EnableFAIFinishHeight = (Temp==1);

  Temp = Handicap;
  GetFromRegistry(szRegistryHandicap,&Temp);
  Handicap = Temp;

  Temp = EnableExternalTriggerCruise;
  GetFromRegistry(szRegistryEnableExternalTriggerCruise,&Temp);
  EnableExternalTriggerCruise = Temp;

  Temp = 0;
  GetFromRegistry(szRegistryUTCOffset,&Temp);
  UTCOffset = Temp;
  if (UTCOffset>12*3600) {
    UTCOffset-= 24*3600;
  }

  Temp = 0;
  GetFromRegistry(szRegistryAutoZoom,&Temp);
  MapWindow::zoom.AutoZoom(Temp == 1);

  Temp = MenuTimeoutMax;
  GetFromRegistry(szRegistryMenuTimeout,&Temp);
  MenuTimeoutMax = Temp;

  Temp = 0;
  GetFromRegistry(szRegistryLockSettingsInFlight,&Temp);
  LockSettingsInFlight = (Temp == 1);

  Temp = 0;
  GetFromRegistry(szRegistryLoggerShort,&Temp);
  LoggerShortName = (Temp == 1);

  Temp = EnableFLARMMap;
  GetFromRegistry(szRegistryEnableFLARMMap,&Temp);
  EnableFLARMMap = Temp;

  Temp = TerrainContrast;
  GetFromRegistry(szRegistryTerrainContrast,&Temp);
  TerrainContrast = (short)Temp;

  Temp = TerrainBrightness;
  GetFromRegistry(szRegistryTerrainBrightness,&Temp);
  TerrainBrightness = (short)Temp;

  Temp = TerrainRamp_Config;
  GetFromRegistry(szRegistryTerrainRamp,&Temp);
  TerrainRamp_Config = (short)Temp;
  TerrainRamp=TerrainRamp_Config;

  Temp = MapWindow::GliderScreenPosition;
  GetFromRegistry(szRegistryGliderScreenPosition,&Temp);
  MapWindow::GliderScreenPosition = (int)Temp;
  MapWindow::GliderScreenPositionY = MapWindow::GliderScreenPosition;


  Temp = BallastSecsToEmpty;
  GetFromRegistry(szRegistryBallastSecsToEmpty,&Temp);
  BallastSecsToEmpty = Temp;

  Temp = SetSystemTimeFromGPS;
  GetFromRegistry(szRegistrySetSystemTimeFromGPS,&Temp);
  SetSystemTimeFromGPS = (Temp!=0);

  Temp = AutoForceFinalGlide;
  GetFromRegistry(szRegistryAutoForceFinalGlide,&Temp);
  AutoForceFinalGlide = (Temp!=0);


  Temp = 0; // fonts
  GetFromRegistry(szRegistryUseCustomFonts,&Temp);
  UseCustomFonts = (Temp == 0 ? 0 : 1);
  SetToRegistry(szRegistryUseCustomFonts, UseCustomFonts);  

  Temp = AlarmMaxAltitude1;
  GetFromRegistry(szRegistryAlarmMaxAltitude1,&Temp);
  AlarmMaxAltitude1 = Temp; // saved *1000, /1000 when used

  Temp = AlarmMaxAltitude2;
  GetFromRegistry(szRegistryAlarmMaxAltitude2,&Temp);
  AlarmMaxAltitude2 = Temp; // saved *1000, /1000 when used

  Temp = AlarmMaxAltitude3;
  GetFromRegistry(szRegistryAlarmMaxAltitude3,&Temp);
  AlarmMaxAltitude3 = Temp; // saved *1000, /1000 when used

  Temp = FinishMinHeight;
  GetFromRegistry(szRegistryFinishMinHeight,&Temp);
  FinishMinHeight = Temp; // 100315 saved *1000, /1000 when used

  Temp = StartHeightRef;
  GetFromRegistry(szRegistryStartHeightRef,&Temp);
  StartHeightRef = Temp;

  Temp = StartMaxHeight;
  GetFromRegistry(szRegistryStartMaxHeight,&Temp);
  StartMaxHeight = Temp; // 100315 saved *1000, /1000 when used
  
  Temp = StartMaxHeightMargin;
  GetFromRegistry(szRegistryStartMaxHeightMargin,&Temp);
  StartMaxHeightMargin = Temp; // 100315 now saved *1000, and divided /1000 when used

  // StartMaxSpeed is saved in ms * 1000, but used in ms!
  Temp=1234567;
  GetFromRegistry(szRegistryStartMaxSpeed,&Temp);
  if (Temp!=1234567) StartMaxSpeed = Temp;

  //  StartMaxSpeedMargin is now in ms*1000
  Temp = 1234567;
  GetFromRegistry(szRegistryStartMaxSpeedMargin,&Temp);
  if (Temp!=1234567) StartMaxSpeedMargin = Temp;

  Temp = 1; // 100219
  GetFromRegistry(szRegistryEnableNavBaroAltitude,&Temp);
  EnableNavBaroAltitude = (Temp!=0);

  Temp = 1;
  GetFromRegistry(szRegistryOrbiter,&Temp);
  Orbiter = (Temp!=0);
  Temp = 1;
  GetFromRegistry(szRegistryShading,&Temp);
  Shading = Temp;
  Temp = 0;
  GetFromRegistry(szRegistryOverlayClock,&Temp);
  OverlayClock = Temp;

  // default BB and IP is all ON
  Temp = 1;
  GetFromRegistry(szRegistryConfBB1,&Temp);
  ConfBB1 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfBB2,&Temp);
  ConfBB2 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfBB3,&Temp);
  ConfBB3 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfBB4,&Temp);
  ConfBB4 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfBB5,&Temp);
  ConfBB5 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfBB6,&Temp);
  ConfBB6 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfBB7,&Temp);
  ConfBB7 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfBB8,&Temp);
  ConfBB8 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfBB9,&Temp);
  ConfBB9 = Temp;

  Temp = 1;
  GetFromRegistry(szRegistryConfIP11,&Temp);
  ConfIP11 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP12,&Temp);
  ConfIP12 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP13,&Temp);
  ConfIP13 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP14,&Temp);
  ConfIP14 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP15,&Temp);
  ConfIP15 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP16,&Temp);
  ConfIP16 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP21,&Temp);
  ConfIP21 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP22,&Temp);
  ConfIP22 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP23,&Temp);
  ConfIP23 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP24,&Temp);
  ConfIP24 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP31,&Temp);
  ConfIP31 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP32,&Temp);
  ConfIP32 = Temp;
  Temp = 1;
  GetFromRegistry(szRegistryConfIP33,&Temp);
  ConfIP33 = Temp;

  Temp = LoggerTimeStepCruise;
  GetFromRegistry(szRegistryLoggerTimeStepCruise,&Temp);
  LoggerTimeStepCruise = Temp;

  Temp = LoggerTimeStepCircling;
  GetFromRegistry(szRegistryLoggerTimeStepCircling,&Temp);
  LoggerTimeStepCircling = Temp;

  Temp = iround(GlidePolar::SafetyMacCready*10);
  GetFromRegistry(szRegistrySafetyMacCready,&Temp);
  GlidePolar::SafetyMacCready = Temp/10.0;

  Temp = DisableAutoLogger;
  GetFromRegistry(szRegistryDisableAutoLogger,&Temp);
  if (Temp) 
    DisableAutoLogger = true;
  else 
    DisableAutoLogger = false;

  LKalarms[0].triggervalue=(int)AlarmMaxAltitude1/1000;
  LKalarms[1].triggervalue=(int)AlarmMaxAltitude2/1000;
  LKalarms[2].triggervalue=(int)AlarmMaxAltitude3/1000;
  UpdateConfBB();
  UpdateConfIP();

}

//
// NOTE: all registry variables are unsigned!
//
BOOL GetFromRegistry(const TCHAR *szRegValue, DWORD *pPos)
{  // returns 0 on SUCCESS, else the non-zero error code
  HKEY    hKey;
  DWORD    dwSize, dwType;
  long    hRes;
  DWORD defaultVal;

  hRes = RegOpenKeyEx(HKEY_CURRENT_USER, szRegistryKey, 0, KEY_ALL_ACCESS, &hKey);
  if (hRes != ERROR_SUCCESS)
    {
      RegCloseKey(hKey);
      return hRes;
    }

  defaultVal = *pPos;
  dwSize = sizeof(DWORD);
  hRes = RegQueryValueEx(hKey, szRegValue, 0, &dwType, (LPBYTE)pPos, &dwSize);
  if (hRes != ERROR_SUCCESS) {
    *pPos = defaultVal;
  }

  RegCloseKey(hKey);
  return hRes;
}


// Implement your code to save value to the registry

HRESULT SetToRegistry(const TCHAR *szRegValue, DWORD Pos)
{
  HKEY    hKey;
  DWORD    Disp;
  HRESULT hRes;

  hRes = RegCreateKeyEx(HKEY_CURRENT_USER, szRegistryKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &Disp);
  if (hRes != ERROR_SUCCESS)
    {
      return FALSE;
    }

  hRes = RegSetValueEx(hKey, szRegValue,0,REG_DWORD, (LPBYTE)&Pos, sizeof(DWORD));
  RegCloseKey(hKey);

  return hRes;
}

// Set bool value to registry as 1 or 0 - JG
HRESULT SetToRegistry(const TCHAR *szRegValue, bool bVal)
{
	return SetToRegistry(szRegValue, bVal ? DWORD(1) : DWORD(0));
}

// Set int value to registry - JG
HRESULT SetToRegistry(const TCHAR *szRegValue, int nVal)
{
	return SetToRegistry(szRegValue, DWORD(nVal));
}

BOOL GetRegistryString(const TCHAR *szRegValue, TCHAR *pPos, DWORD dwSize)
{
  HKEY    hKey;
  DWORD   dwType = REG_SZ;
  long    hRes;
  unsigned int i;
  for (i=0; i<dwSize; i++) {
    pPos[i]=0;
  }

  pPos[0]= '\0';
  hRes = RegOpenKeyEx(HKEY_CURRENT_USER, szRegistryKey, 0, KEY_READ /*KEY_ALL_ACCESS*/, &hKey);
  if (hRes != ERROR_SUCCESS)
    {
      RegCloseKey(hKey);
      return FALSE;
    }

  dwSize *= 2;

  hRes = RegQueryValueEx(hKey, szRegValue, 0, &dwType, (LPBYTE)pPos, &dwSize);

  RegCloseKey(hKey);
  return hRes;
}


HRESULT SetRegistryString(const TCHAR *szRegValue, const TCHAR *Pos)
{
  HKEY    hKey;
  DWORD    Disp;
  HRESULT hRes;

  hRes = RegCreateKeyEx(HKEY_CURRENT_USER, szRegistryKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &Disp);
  if (hRes != ERROR_SUCCESS)
    {
      return FALSE;
    }

  hRes = RegSetValueEx(hKey, szRegValue,0,REG_SZ, (LPBYTE)Pos, (_tcslen(Pos)+1)*sizeof(TCHAR));
  RegCloseKey(hKey);

  return hRes;
}

void ReadPort1Settings(DWORD *PortIndex, DWORD *SpeedIndex, DWORD *Bit1Index)
{
  DWORD Temp=0;

  if(GetFromRegistry(szRegistryPort1Index,&Temp)==ERROR_SUCCESS)
    (*PortIndex) = Temp;

  if(GetFromRegistry(szRegistrySpeed1Index,&Temp)==ERROR_SUCCESS)
    (*SpeedIndex) = Temp;

  if(GetFromRegistry(szRegistryBit1Index,&Temp)==ERROR_SUCCESS)
    (*Bit1Index) = Temp;
}


void WritePort1Settings(DWORD PortIndex, DWORD SpeedIndex, DWORD Bit1Index)
{
  SetToRegistry(szRegistryPort1Index, PortIndex);
  SetToRegistry(szRegistrySpeed1Index, SpeedIndex);
  SetToRegistry(szRegistryBit1Index, Bit1Index);
}

void ReadPort2Settings(DWORD *PortIndex, DWORD *SpeedIndex, DWORD *Bit2Index)
{
  DWORD Temp=0;

  if(GetFromRegistry(szRegistryPort2Index,&Temp)==ERROR_SUCCESS)
    (*PortIndex) = Temp;

  if(GetFromRegistry(szRegistrySpeed2Index,&Temp)==ERROR_SUCCESS)
    (*SpeedIndex) = Temp;

  if(GetFromRegistry(szRegistryBit2Index,&Temp)==ERROR_SUCCESS)
    (*Bit2Index) = Temp;
}


void WritePort2Settings(DWORD PortIndex, DWORD SpeedIndex, DWORD Bit2Index)
{
  SetToRegistry(szRegistryPort2Index, PortIndex);
  SetToRegistry(szRegistrySpeed2Index, SpeedIndex);
  SetToRegistry(szRegistryBit2Index, Bit2Index);
}

#if USEPORT3
void ReadPort3Settings(DWORD *PortIndex, DWORD *SpeedIndex, DWORD *Bit3Index)
{
  DWORD Temp=0;

  if(GetFromRegistry(szRegistryPort3Index,&Temp)==ERROR_SUCCESS)
    (*PortIndex) = Temp;

  if(GetFromRegistry(szRegistrySpeed3Index,&Temp)==ERROR_SUCCESS)
    (*SpeedIndex) = Temp;

  if(GetFromRegistry(szRegistryBit3Index,&Temp)==ERROR_SUCCESS)
    (*Bit3Index) = Temp;
}


void WritePort3Settings(DWORD PortIndex, DWORD SpeedIndex, DWORD Bit3Index)
{
  SetToRegistry(szRegistryPort3Index, PortIndex);
  SetToRegistry(szRegistrySpeed3Index, SpeedIndex);
  SetToRegistry(szRegistryBit3Index, Bit3Index);
}
#endif



void SetRegistryColour(int i, DWORD c)
{


  SetToRegistry(szRegistryColour[i] ,c) ;
}


void SetRegistryBrush(int i, DWORD c)
{

  SetToRegistry(szRegistryBrush[i] ,c) ;
}



void SetRegistryAirspaceMode(int i)
{


  DWORD val = MapWindow::iAirspaceMode[i];
  SetToRegistry(szRegistryAirspaceMode[i], val);
}

int GetRegistryAirspaceMode(int i) {
  DWORD Temp= 3; // display + warnings
  GetFromRegistry(szRegistryAirspaceMode[i],&Temp);
  return Temp;
}


void WriteFileRegistryString(HANDLE hFile, TCHAR *instring) {
    int len;
    char ctempFile[MAX_PATH];
    TCHAR tempFile[MAX_PATH];
    DWORD dwBytesWritten;
    int i;

    tempFile[0]=0;
    for (i=0; i<MAX_PATH; i++) {
      tempFile[i]= 0;
    }
    GetRegistryString(instring, tempFile, MAX_PATH);
    WideCharToMultiByte( CP_ACP, 0, tempFile,
			 _tcslen(tempFile)+1,
			 ctempFile,
			 MAX_PATH, NULL, NULL);
    for (i=0; i<MAX_PATH; i++) {
      if (ctempFile[i]=='\?') {
	ctempFile[i]=0;
      }
    }
    len = strlen(ctempFile)+1;
    ctempFile[len-1]= '\n';
    WriteFile(hFile,ctempFile,len, &dwBytesWritten, (OVERLAPPED *)NULL);
}


void WriteProfile(const TCHAR *szFile)
{
  #if TESTBENCH
  StartupStore(_T("... WriteProfile <%s>%s"),szFile,NEWLINE);
  #endif
  SaveRegistryToFile(szFile);
}


void ReadFileRegistryString(HANDLE hFile, TCHAR *instring) {
    int i;
    TCHAR tempFile[MAX_PATH];

    for (i=0; i<MAX_PATH; i++) {
      tempFile[i]= 0;
    }
    ReadString(hFile, MAX_PATH, tempFile);
    tempFile[_tcslen(tempFile)]= 0; // 110101  this is useless
    SetRegistryString(instring, tempFile);
}


void ReadProfile(const TCHAR *szFile)
{
  #if TESTBENCH
  StartupStore(_T("... ReadProfile <%s>%s"),szFile,NEWLINE);
  #endif
  LoadRegistryFromFile(szFile);

  WAYPOINTFILECHANGED = TRUE;
  TERRAINFILECHANGED = TRUE;
  TOPOLOGYFILECHANGED = TRUE;
  AIRSPACEFILECHANGED = TRUE;
  AIRFIELDFILECHANGED = TRUE;
  POLARFILECHANGED = TRUE;
  
  // assuming all is ok, we can...
  ReadRegistrySettings();
}


#ifndef NOWINDREGISTRY
void SaveWindToRegistry() {
  DWORD Temp;
  Temp = iround(CALCULATED_INFO.WindSpeed);
  SetToRegistry(szRegistryWindSpeed,Temp);
  Temp = iround(CALCULATED_INFO.WindBearing);
  SetToRegistry(szRegistryWindBearing,Temp);
  SetWindEstimate(CALCULATED_INFO.WindSpeed, CALCULATED_INFO.WindBearing);

}


void LoadWindFromRegistry() {
  StartupStore(TEXT(". Load wind from registry%s"),NEWLINE);

  DWORD Temp;
  Temp=0;
  GetFromRegistry(szRegistryWindSpeed,&Temp);
  CALCULATED_INFO.WindSpeed = Temp;
  Temp=0;
  GetFromRegistry(szRegistryWindBearing,&Temp);
  CALCULATED_INFO.WindBearing = Temp;
}
#endif

void ReadDeviceSettings(const int devIdx, TCHAR *Name){

  Name[0] = '\0';

  if (devIdx == 0){
    GetRegistryString(szRegistryDeviceA , Name, DEVNAMESIZE);
  }

  if (devIdx == 1){
    GetRegistryString(szRegistryDeviceB , Name, DEVNAMESIZE);
  }
    if (_tcslen(Name)==0) _tcscpy(Name,_T(DEV_DISABLED_NAME));
    return;

}


void WriteDeviceSettings(const int devIdx, const TCHAR *Name){

  if (devIdx == 0)
    SetRegistryString(szRegistryDeviceA , Name);

  if (devIdx == 1)
    SetRegistryString(szRegistryDeviceB , Name);

  if (devIdx == 2)
    SetRegistryString(szRegistryDeviceC , Name);
}



const static int nMaxValueNameSize = MAX_PATH + 6; //255 + 1 + /r/n
const static int nMaxValueValueSize = MAX_PATH * 2 + 6; // max regkey name is 256 chars + " = " 
const static int nMaxClassSize = MAX_PATH + 6;
const static int nMaxKeyNameSize = MAX_PATH + 6;

static bool LoadRegistryFromFile_inner(const TCHAR *szFile, bool wide=true)
{
  #if TESTBENCH
  StartupStore(_T(".... LoadRegistryFromFile <%s>%s"),szFile,NEWLINE);
  #endif
  bool found = false;
  FILE *fp=NULL;
  if (_tcslen(szFile)>0)
#ifndef __MINGW32__
    if (wide) {
      fp = _tfopen(szFile, TEXT("rb"));    
    } else {
      fp = _tfopen(szFile, TEXT("rt"));    
    }
#else
    fp = _tfopen(szFile, TEXT("rb"));    //20060515:sgi add b
#endif
  if(fp == NULL) {
    // error
    return false;
  }
  TCHAR winval[nMaxValueValueSize];
  TCHAR wname[nMaxValueValueSize];
  TCHAR wvalue[nMaxValueValueSize];
  int j;

#ifdef __MINGW32__
  char inval[nMaxValueValueSize];
  char name [nMaxValueValueSize];
  char value [nMaxValueValueSize];
    if (wide) {
#endif
      while (_fgetts(winval, nMaxValueValueSize, fp)) {
        if (winval[0] > 255) { // not reading corectly, probably narrow file.
          break;
        }
        if (_stscanf(winval, TEXT("%[^#=\r\n ]=\"%[^\r\n\"]\"[\r\n]"), wname, wvalue) == 2) {
	  if (_tcslen(wname)>0) {
	    SetRegistryString(wname, wvalue);
	    found = true;
	  }
        } else if (_stscanf(winval, TEXT("%[^#=\r\n ]=%d[\r\n]"), wname, &j) == 2) {
	  if (_tcslen(wname)>0) {
	    SetToRegistry(wname, j);
	    found = true;
	  }
        } else if (_stscanf(winval, TEXT("%[^#=\r\n ]=\"\"[\r\n]"), wname) == 1) {
	  if (_tcslen(wname)>0) {
	    SetRegistryString(wname, TEXT(""));
	    found = true;
	  }
        } else {
	  //		ASSERT(false);	// Invalid line reached
        }
      }

#ifdef __MINGW32__
    } else {
      while (fgets(inval, nMaxValueValueSize, fp)) {
        if (sscanf(inval, "%[^#=\r\n ]=\"%[^\r\n\"]\"[\r\n]", name, value) == 2) {
          if (strlen(name)>0) {
            utf2unicode(name, wname, nMaxValueValueSize);
            utf2unicode(value, wvalue, nMaxValueValueSize);
            SetRegistryString(wname, wvalue);
            found = true;
          }
        } else if (sscanf(inval, "%[^#=\r\n ]=%d[\r\n]", name, &j) == 2) {
          if (strlen(name)>0) {
            utf2unicode(name, wname, nMaxValueValueSize);
            SetToRegistry(wname, j);
            found = true;
          }
        } else if (sscanf(inval, "%[^#=\r\n ]=\"\"[\r\n]", name) == 1) {
          if (strlen(name)>0) {
            utf2unicode(name, wname, nMaxValueValueSize);
            SetRegistryString(wname, TEXT(""));
            found = true;
          }
        } else {
	  //		ASSERT(false);	// Invalid line reached
        }
      }
    }
#endif

  fclose(fp);

  return found;
}

void LoadRegistryFromFile(const TCHAR *szFile) {
#ifndef __MINGW32__
  if (!LoadRegistryFromFile_inner(szFile,true)) { // legacy, wide chars
    LoadRegistryFromFile_inner(szFile,false);       // new, non-wide chars
  }
#else
  if (!LoadRegistryFromFile_inner(szFile,false)) { // new, non-wide chars
    LoadRegistryFromFile_inner(szFile,true);       // legacy, wide chars
  }
#endif
}

void SaveRegistryToFile(const TCHAR *szFile)
{
  TCHAR lpstrName[nMaxKeyNameSize+1];
//  char sName[nMaxKeyNameSize+1];
//  char sValue[nMaxValueValueSize+1];
  #if TESTBENCH
  StartupStore(_T(".... SaveRegistryToFile <%s>%s"),szFile,NEWLINE);
  #endif
  
  //  TCHAR lpstrClass[nMaxClassSize+1];
#ifdef __MINGW32__
  union {
    BYTE pValue[nMaxValueValueSize+4];
    DWORD dValue;
  } uValue;
#else
  BYTE pValue[nMaxValueValueSize+1];
#endif

  HKEY hkFrom;
  LONG res = ::RegOpenKeyEx(HKEY_CURRENT_USER, szRegistryKey, 0, KEY_ALL_ACCESS, &hkFrom);

  if (ERROR_SUCCESS != res) {
	return;
  }

  FILE *fp=NULL;
  if (_tcslen(szFile)>0)
	fp = _tfopen(szFile, TEXT("wb"));  //20060515:sgi add b
  if(fp == NULL) {
	// error
	::RegCloseKey(hkFrom);
	return;
  }

  for (int i = 0;;i++) {
    DWORD nType;
    DWORD nValueSize = nMaxValueValueSize;
    DWORD nNameSize = nMaxKeyNameSize;
//    DWORD nClassSize = nMaxClassSize;

    lpstrName[0] = _T('\0'); // null terminate, just in case

    res = ::RegEnumValue(hkFrom, i, lpstrName,
			      &nNameSize, 0,
#ifdef __MINGW32__
			      &nType, uValue.pValue,
#else
			      &nType, pValue,
#endif
			      &nValueSize);

    if (ERROR_NO_MORE_ITEMS == res) {
      break;
    }
    if ((nNameSize<=0)|| (nNameSize>(DWORD)nMaxKeyNameSize)) { // 100207 fixed signed unsigned (DWORD)
      continue; // in case things get wierd
    }

    lpstrName[nNameSize] = _T('\0'); // null terminate, just in case

    if (_tcslen(lpstrName)>0) {

      // type 1 text
      // type 4 integer (valuesize 4)
      
      if (nType==4) { // data
#ifdef __MINGW32__
	fprintf(fp,"%S=%d\r\n", lpstrName, (int)uValue.dValue); // FIX 100228
#else
	wcstombs(sName,lpstrName,nMaxKeyNameSize+1);
	fprintf(fp,"%s=%d\r\n", sName, *((DWORD*)pValue));
#endif
      } else
      // XXX SCOTT - Check that the output data (lpstrName and pValue) do not contain \r or \n
      if (nType==1) { // text
        if (nValueSize>0) {
#ifdef __MINGW32__
          if (_tcslen((TCHAR*)uValue.pValue)>0) {
            char sValue[nMaxValueValueSize+1];
            
            uValue.pValue[nValueSize]= 0; // null terminate, just in case
            uValue.pValue[nValueSize+1]= 0; // null terminate, just in case
            unicode2utf((TCHAR*) uValue.pValue, sValue, sizeof(sValue));
            fprintf(fp,"%S=\"%s\"\r\n", lpstrName, sValue);
          } else {
            fprintf(fp,"%S=\"\"\r\n", lpstrName);
          }
#else
          if (_tcslen((TCHAR*)pValue)>0) {
            pValue[nValueSize]= 0; // null terminate, just in case
            pValue[nValueSize+1]= 0; // null terminate, just in case
            wcstombs(sName,lpstrName,nMaxKeyNameSize+1);
            wcstombs(sValue,(TCHAR*)pValue,nMaxKeyNameSize+1);
            fprintf(fp,"%s=\"%s\"\r\n", sName, sValue);
          } else {
            wcstombs(sName,lpstrName,nMaxKeyNameSize+1);
            fprintf(fp,"%s=\"\"\r\n", sName);
          }
#endif
        } else {
#ifdef __MINGW32__
          fprintf(fp,"%S=\"\"\r\n", lpstrName);
#else
          fprintf(fp,"%s=\"\"\r\n", lpstrName);
#endif
        }
      }
    }

  }
#ifdef __MINGW32__
  // JMW why flush agressively?
  fflush(fp);
#endif

#ifdef __MINGW32__
  fprintf(fp,"\r\n"); // end of file
#endif

  fclose(fp);

  ::RegCloseKey(hkFrom);
}



void RestoreRegistry(void) {
  #if TESTBENCH
  StartupStore(TEXT(". Restore registry from startProfile <%s>%s"),startProfileFile,NEWLINE);
  #endif
  LoadRegistryFromFile(startProfileFile);
  #ifdef NEWPROFILES
  // purely testing purposes
  extern bool LKProfileLoad(TCHAR *file);
  LKProfileLoad(startProfileFile);
  #endif
}

void StoreRegistry(void) {
  StartupStore(TEXT(". Store registry%s"),NEWLINE);
  #if 0 
  // DO NOT SAVE to the startup chosen profile, anymore
  if (!CheckClubVersion())
	SaveRegistryToFile(startProfileFile);
  #endif
  SaveRegistryToFile(defaultProfileFile);
}


#ifdef PNA
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

BOOL DelRegistryKey(const TCHAR *szDelKey)
{
   HKEY tKey;
   RegOpenKeyEx(HKEY_CURRENT_USER, _T(REGKEYNAME),0,0,&tKey);
   if ( RegDeleteValue(tKey, szDelKey) != ERROR_SUCCESS ) {
	return false;
   }
   RegCloseKey(tKey);
   return true;
}

#ifdef PNA
void CleanRegistry()
{
   HKEY tKey;
   RegOpenKeyEx(HKEY_CURRENT_USER, szRegistryKey ,0,0,&tKey);

	RegDeleteValue(tKey,_T("CDIWindowFont"));
	RegDeleteValue(tKey,_T("MapLabelFont"));
	RegDeleteValue(tKey,_T("MapWindowBoldFont"));
	RegDeleteValue(tKey,_T("MapWindowFont"));
	RegDeleteValue(tKey,_T("StatisticsFont"));
	RegDeleteValue(tKey,_T("TitleWindowFont"));
	RegDeleteValue(tKey,_T("BugsBallastFont"));
	RegDeleteValue(tKey,_T("TeamCodeFont"));

   RegCloseKey(tKey);
}
#endif


