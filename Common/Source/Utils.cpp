/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.cpp,v 8.17 2010/12/19 16:42:53 root Exp root $
*/

#include "StdAfx.h"

#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#else
#include "wcecompat/ts_string.h"
#endif

#include "Defines.h" // VENTA3

#include "Utils.h"
#include "Utils2.h"

// #include "resource.h"
#include "externs.h"
#include "device.h"
#include "uniqueid.h"
#include "lk8000.h"
#include "Topology.h"
#include "Terrain.h"
#include "Units.h"
#include "Calculations.h"
#include "McReady.h"
#include "NavFunctions.h"
#include "WaveThread.h"
#include "LKMapWindow.h"
#ifdef PNA
#include "LKHolux.h"
#endif

#include "FlarmIdFile.h"
// Warning, this is initialising class, loading flarmnet IDs before anything else in the LK is even started..
FlarmIdFile file; 

#if USEIBOX
#include "InfoBoxLayout.h"
#endif

#include "utils/heapcheck.h"

// JMW not required in newer systems?
#ifdef __MINGW32__
#ifndef max
#define max(x, y)   (x > y ? x : y)
#define min(x, y)   (x < y ? x : y)
#endif
#endif

bool ReadWinPilotPolarInternal(int i);

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
const TCHAR szRegistryHomeWaypoint[]= TEXT("HomeWaypoint");
const TCHAR szRegistryAlternate1[]= TEXT("Alternate1"); // VENTA3
const TCHAR szRegistryAlternate2[]= TEXT("Alternate2");
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
const TCHAR szRegistryTeamcodeRefWaypoint[] = TEXT("TeamcodeRefWaypoint");

#if USEIBOX
const TCHAR szRegistryAppInfoBoxGeom[] = TEXT("AppInfoBoxGeom");
#endif
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

const TCHAR szRegistryAutoAdvance[] = TEXT("AutoAdvance");
const TCHAR szRegistryUTCOffset[] = TEXT("UTCOffset");
const TCHAR szRegistryAutoZoom[] = TEXT("AutoZoom");
const TCHAR szRegistryMenuTimeout[] = TEXT("MenuTimeout");
const TCHAR szRegistryLockSettingsInFlight[] = TEXT("LockSettingsInFlight");
const TCHAR szRegistryTerrainContrast[] = TEXT("TerrainContrast");
const TCHAR szRegistryTerrainBrightness[] = TEXT("TerrainBrightness");
const TCHAR szRegistryTerrainRamp[] = TEXT("TerrainRamp");
const TCHAR szRegistryEnableFLARMMap[] = TEXT("EnableFLARMDisplay");
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
const TCHAR szRegistryWaypointsOutOfRange[] = TEXT("WaypointsOutOfRange2"); // 101020 2nd version, reset needed
const TCHAR szRegistryEnableExternalTriggerCruise[] = TEXT("EnableExternalTriggerCruise");
const TCHAR szRegistryFAIFinishHeight[] = TEXT("FAIFinishHeight");
const TCHAR szRegistryHandicap[] = TEXT("Handicap");
const TCHAR szRegistrySnailWidthScale[] = TEXT("SnailWidthScale");
const TCHAR szRegistryDisableAutoLogger[] = TEXT("DisableAutoLogger");
const TCHAR szRegistryMapFile[]=	 TEXT("MapFile"); // pL
const TCHAR szRegistryBallastSecsToEmpty[]=	 TEXT("BallastSecsToEmpty"); 
const TCHAR szRegistryUseCustomFonts[]=	 TEXT("UseCustomFonts"); 
const TCHAR szRegistryFontInfoWindowFont[]=	 TEXT("InfoWindowFont"); 
const TCHAR szRegistryFontTitleWindowFont[]=	 TEXT("TitleWindowFont"); 
const TCHAR szRegistryFontMapWindowFont[]=	 TEXT("MapWindowFont"); 
const TCHAR szRegistryFontTitleSmallWindowFont[]=	 TEXT("TeamCodeFont"); 
const TCHAR szRegistryFontMapWindowBoldFont[]=	 TEXT("MapWindowBoldFont"); 
const TCHAR szRegistryFontCDIWindowFont[]=	 TEXT("CDIWindowFont"); 
const TCHAR szRegistryFontMapLabelFont[]=	 TEXT("MapLabelFont"); 
const TCHAR szRegistryFontStatisticsFont[]=	 TEXT("StatisticsFont"); 

int UTCOffset = 0; // used for Altair
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




// This function checks to see if Final Glide mode infoboxes have been
// initialised.  If all are zero, then the current configuration was
// using XCSoarV3 infoboxes, so copy settings from cruise mode.
void CheckInfoTypes() {
  int i;
  char iszero=0;
  for (i=0; i<MAXINFOWINDOWS; i++) {
    iszero |= (InfoType[i] >> 16) & 0xff;
  }
  if (iszero==0) {
    for (i=0; i<MAXINFOWINDOWS; i++) {
      InfoType[i] += (InfoType[i] & 0xff)<<16;
      StoreType(i,InfoType[i]);
    }
  }
}


void ResetInfoBoxes(void) {
  InfoType[0] = 921102;
  InfoType[1] = 725525;
  InfoType[2] = 262144;
  InfoType[3] = 74518;
  InfoType[4] = 657930;
  InfoType[5] = 2236963;
  InfoType[6] = 394758;
  InfoType[7] = 1644825;
  InfoType[8] = 1644825;
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

  StartupStore(TEXT(". Read registry settings%s"),NEWLINE);

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

  // check against V3 infotypes
  CheckInfoTypes();

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

  #if 0  // REMOVE
  Temp = 0;
  GetFromRegistry(szRegistryShortcutIbox,&Temp); 
  ShortcutIbox = Temp;
  #endif

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

 #if USEIBOX
// VENTA2-ADDON Geometry change and PNA custom font settings
// depending on infobox geometry and model type
// I had to move here the font setting because I needed first to 
// know the screen geometry, in the registry!
  Temp = Appearance.InfoBoxGeom;
  GetFromRegistry(szRegistryAppInfoBoxGeom, &Temp);
  Appearance.InfoBoxGeom = (InfoBoxGeomAppearance_t)Temp;
 #endif // USEIBOX

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

  Temp = TerrainRamp;
  GetFromRegistry(szRegistryTerrainRamp,&Temp);
  TerrainRamp = (short)Temp;

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

#ifdef PNA
BOOL GetFontPath(TCHAR *pPos)
{
  HKEY    hKey;
  DWORD   dwType = REG_SZ;
  DWORD dwSize = MAX_PATH;
  long    hRes;
  unsigned int i;
  for (i=0; i<dwSize; i++) {
    pPos[i]=0;
  }

  pPos[0]= '\0';
  hRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\FontPath"), 0, KEY_READ /*KEY_ALL_ACCESS*/, &hKey);
  if (hRes != ERROR_SUCCESS) {
	RegCloseKey(hKey);
	return FALSE;
  }

  dwSize *= 2;  // BUGFIX 100913 ?? to remove? check

  hRes = RegQueryValueEx(hKey, _T("FontPath"), 0, &dwType, (LPBYTE)pPos, &dwSize);

  RegCloseKey(hKey);
  if (hRes==ERROR_SUCCESS) return TRUE;
  else return FALSE;
}
#endif

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


void rotate(double &xin, double &yin, const double &angle)
{
  double x= xin;
  double y= yin;
  static double lastangle = 0;
  static double cost=1,sint=0;

  if(angle != lastangle)
    {
      lastangle = angle;
      cost = (double)fastcosine(angle);
      sint = (double)fastsine(angle);
    }
  xin = x*cost - y*sint;
  yin = y*cost + x*sint;
}




void frotate(float &xin, float &yin, const float &angle)
{
  float x= xin;
  float y= yin;
  static float lastangle = 0;
  static float cost=1,sint=0;

  if(angle != lastangle)
    {
      lastangle = angle;
      cost = (float)fastcosine(angle);
      sint = (float)fastsine(angle);
    }
  xin = x*cost - y*sint;
  yin = y*cost + x*sint;
}


void protate(POINT &pin, const double &angle)
{
  int x= pin.x;
  int y= pin.y;
  static double lastangle = 0;
  static int cost=1024,sint=0;

  if(angle != lastangle)
    {
      lastangle = angle;
      cost = ifastcosine(angle);
      sint = ifastsine(angle);
    }
  pin.x = (x*cost - y*sint + 512 )/1024;
  pin.y = (y*cost + x*sint + 512 )/1024;

  // round (x/b) = (x+b/2)/b;
  // b = 2; x = 10 -> (10+1)/2=5
  // b = 2; x = 11 -> (11+1)/2=6
  // b = 2; x = -10 -> (-10+1)/2=4
}


void protateshift(POINT &pin, const double &angle, 
		  const int &xs, const int &ys)
{
  int x= pin.x;
  int y= pin.y;
  static double lastangle = 0;
  static int cost=1024,sint=0;

  if(angle != lastangle)
    {
      lastangle = angle;
      cost = ifastcosine(angle);
      sint = ifastsine(angle);
    }
  pin.x = (x*cost - y*sint + 512 + (xs*1024))/1024;
  pin.y = (y*cost + x*sint + 512 + (ys*1024))/1024;

}


void irotatescale(int &xin, int &yin, const double &angle,
                  const double &scale, double &x, double &y)
{
  static double lastangle = 0;
  static double lastscale = 0;
  static int cost=1024,sint=0;
  if((angle != lastangle)||(scale != lastscale))
    {
      lastscale = scale/1024;
      lastangle = angle;
      cost = ifastcosine(angle);
      sint = ifastsine(angle);
    }
  x = (xin*cost - yin*sint + 512)*lastscale;
  y = (yin*cost + xin*sint + 512)*lastscale;
}


void irotate(int &xin, int &yin, const double &angle)
{
  int x= xin;
  int y= yin;
  static double lastangle = 0;
  static int cost=1024,sint=0;

  if(angle != lastangle)
    {
      lastangle = angle;
      cost = ifastcosine(angle);
      sint = ifastsine(angle);
    }
  xin = (x*cost - y*sint + 512)/1024;
  yin = (y*cost + x*sint + 512)/1024;
}


void rotatescale(double &xin, double &yin, 
		 const double &angle, const double &scale)
{
  double x= xin;
  double y= yin;
  static double lastangle = 0;
  static double lastscale = 0;
  static double cost=1,sint=0;

  if((angle != lastangle)||(scale != lastscale))
    {
      lastangle = angle;
      lastscale = scale;
      cost = (double)fastcosine(angle)*scale;
      sint = (double)fastsine(angle)*scale;
    }
  xin = x*cost - y*sint;
  yin = y*cost + x*sint;
}


void frotatescale(float &xin, float &yin, const float &angle, const float &scale)
{
  float x= xin;
  float y= yin;
  static float lastangle = 0;
  static float lastscale = 0;
  static float cost=1,sint=0;

  if((angle != lastangle)||(scale != lastscale))
    {
      lastangle = angle;
      lastscale = scale;
      cost = (float)fastcosine(angle)*scale;
      sint = (float)fastsine(angle)*scale;
    }
  xin = x*cost - y*sint;
  yin = y*cost + x*sint;
}


double AngleLimit360(double theta) {
  while (theta>=360.0) {
    theta-= 360.0;
  }
  while (theta<0.0) {
    theta+= 360.0;
  }
  return theta;
}


double AngleLimit180(double theta) {
  while (theta>180.0) {
    theta-= 360.0;
  }
  while (theta<-180.0) {
    theta+= 360.0;
  }
  return theta;
}


void DistanceBearing(double lat1, double lon1, double lat2, double lon2,
                     double *Distance, double *Bearing) {

// incomplete, test does not show benefits, low hits
#if (LK_CACHECALC && LK_CACHECALC_DBE)
  #define CASIZE_DBE 50
  static bool doinit=true;
  static int  cacheIndex;
  static double cur_checksum;
  static double cur_lat1, cur_lat2, cur_lon1, cur_lon2;
  bool cacheFound;
  int i;

  static double cache_checksum[CASIZE_DBE];
  static double cache_lat1[CASIZE_DBE];
  static double cache_lat2[CASIZE_DBE];
  static double cache_lon1[CASIZE_DBE];
  static double cache_lon2[CASIZE_DBE];
  static double cache_Distance[CASIZE_DBE];
  static double cache_Bearing[CASIZE_DBE];

  if (doinit) {
	cacheIndex=0;
	for (i=0; i<CASIZE_DBE; i++) {
		cache_checksum[i]=0;
		cache_lat1[i]=0;
		cache_lat2[i]=0;
		cache_lon1[i]=0;
		cache_lon2[i]=0;
		cache_Distance[i]=0;
		cache_Bearing[i]=0;
	}
	doinit=false;
  }

  Cache_Calls_DBE++;
  cur_checksum=lat1+lat2+lon1+lon2;
  cacheFound=false;

  for (i=0; i<CASIZE_DBE; i++) {
	if ( cache_checksum[i] != cur_checksum ) continue;
	if ( cache_lat1[i] != lat1 ) {
		Cache_False_DBE++;
		continue;
	}
	if ( cache_lat2[i] != lat2 ) {
		Cache_False_DBE++;
		continue;
	}
	if ( cache_lon1[i] != lon1 ) {
		Cache_False_DBE++;
		continue;
	}
	if ( cache_lon2[i] != lon2 ) {
		Cache_False_DBE++;
		continue;
	}
	cacheFound=true;
	break;
  }

  if (cacheFound) {
	Cache_Hits_DBE++;
  }  else {
	cur_lat1=lat1;
	cur_lat2=lat2;
	cur_lon1=lon1;
	cur_lon2=lon2;
  }
#endif
   

  lat1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;

  double clat1 = cos(lat1);
  double clat2 = cos(lat2);
  double dlon = lon2-lon1;

  if (Distance) {
    double s1 = sin((lat2-lat1)/2);
    double s2 = sin(dlon/2);
    double a= max(0.0,min(1.0,s1*s1+clat1*clat2*s2*s2));
    *Distance = 6371000.0*2.0*atan2(sqrt(a),sqrt(1.0-a));
  }
  if (Bearing) {
    double y = sin(dlon)*clat2;
    double x = clat1*sin(lat2)-sin(lat1)*clat2*cos(dlon);
    *Bearing = (x==0 && y==0) ? 0:AngleLimit360(atan2(y,x)*RAD_TO_DEG);
  }

#if (LK_CACHECALC && LK_CACHECALC_DBE)
  if (!cacheFound) {
	if (++cacheIndex==CASIZE_DBE) cacheIndex=0;
	cache_checksum[cacheIndex]=cur_checksum;
	cache_lat1[cacheIndex]=cur_lat1;
	cache_lat2[cacheIndex]=cur_lat2;
	cache_lon1[cacheIndex]=cur_lon1;
	cache_lon2[cacheIndex]=cur_lon2;
  }
#endif

}


double DoubleDistance(double lat1, double lon1, double lat2, double lon2,
		      double lat3, double lon3) {

  lat1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lat3 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;
  lon3 *= DEG_TO_RAD;

  double clat1 = cos(lat1);
  double clat2 = cos(lat2);
  double clat3 = cos(lat3);
  double dlon21 = lon2-lon1;
  double dlon32 = lon3-lon2;

  double s21 = sin((lat2-lat1)/2);
  double sl21 = sin(dlon21/2);
  double s32 = sin((lat3-lat2)/2);
  double sl32 = sin(dlon32/2);

  double a12 = max(0.0,min(1.0,s21*s21+clat1*clat2*sl21*sl21));
  double a23 = max(0.0,min(1.0,s32*s32+clat2*clat3*sl32*sl32));
  return 6371000.0*2.0*(atan2(sqrt(a12),sqrt(1.0-a12))
			+atan2(sqrt(a23),sqrt(1.0-a23)));

}


double Reciprocal(double InBound)
{
  return AngleLimit360(InBound+180);
}


bool AngleInRange(double Angle0, double Angle1, double x, bool is_signed) {
  Angle0 = AngleLimit360(Angle0);
  Angle1 = AngleLimit360(Angle1);
  x = AngleLimit360(x);

  if (Angle1>= Angle0) {
    if ((x>=Angle0) && (x<= Angle1)) {
      return true;
    }
  } else {
    if (is_signed) {
      if ((x>=Angle0) || (x<= Angle1)) {
        return true;
      }
    } else {
      if ((x<=Angle0) || (x>= Angle1)) {
        return true;
      }
    }
  }
  return false;
}

// Use only for AAT bisector calculations!
double HalfAngle(double Angle0, double Angle1) {
  Angle0 = AngleLimit360(Angle0);
  Angle1 = AngleLimit360(Angle1);

  // TODO code: check/test this? thankfully only occurs in one spot in AAT
  if (Angle1>= Angle0) {
    return (Angle0+Angle1)/2;
  } else {
    return (Angle0+Angle1+360)/2;
  }
}


double BiSector(double InBound, double OutBound)
{
  double result;

  InBound = Reciprocal(InBound);

  if(InBound == OutBound)
    {
      result = Reciprocal(InBound);
    }

  else if (InBound > OutBound)
    {
      if( (InBound - OutBound) < 180)
	{
	  result = Reciprocal((InBound+OutBound)/2);
	}
      else
	{
	  result = (InBound+OutBound)/2;
	}
    }
  else
    {
      if( (OutBound - InBound) < 180)
	{
	  result = Reciprocal((InBound+OutBound)/2);
	}
      else
	{
	  result = (InBound+OutBound)/2;
	}
    }
  return result;
}

// This is calculating the weight difference for the chosen wingloading
// Remember that polar files have a weight indicated that includes the pilot..
// Check that WingArea is NOT zero! Winpilot polars had no WingArea configurable!
void WeightOffset(double wload) {
  double calcweight;

  if (GlidePolar::WingArea<1) { // 100131
	// we should lock calculation thread probably
	if (GlidePolar::WeightOffset!=0) GlidePolar::WeightOffset=0;
	return;
  }

  // WEIGHTS[2] is full ballast
  // BALLAST is percentage of full ballast
  // new weight = (wingload * wingarea) - ballast
  calcweight=(wload*GlidePolar::WingArea) - (WEIGHTS[2]*BALLAST);
  GlidePolar::WeightOffset = calcweight-(WEIGHTS[0]+WEIGHTS[1]);

  GlidePolar::SetBallast(); // BUGFIX 101002
}


void PolarWinPilot2XCSoar(double POLARV[3], double POLARW[3], double ww[2]) {
  double d;
  double v1,v2,v3;
  double w1,w2,w3;

  v1 = POLARV[0]/3.6; v2 = POLARV[1]/3.6; v3 = POLARV[2]/3.6;
  //	w1 = -POLARV[0]/POLARLD[0];
  //    w2 = -POLARV[1]/POLARLD[1];
  //    w3 = -POLARV[2]/POLARLD[2];
  w1 = POLARW[0]; w2 = POLARW[1]; w3 = POLARW[2];

  d = v1*v1*(v2-v3)+v2*v2*(v3-v1)+v3*v3*(v1-v2);
  if (d == 0.0)
    {
      POLAR[0]=0;
    }
  else
    {
      POLAR[0]=((v2-v3)*(w1-w3)+(v3-v1)*(w2-w3))/d;
    }
  d = v2-v3;
  if (d == 0.0)
    {
      POLAR[1]=0;
    }
  else
    {
      POLAR[1] = (w2-w3-POLAR[0]*(v2*v2-v3*v3))/d;
    }

  // these 0 and 1 are always used as a single weight: always 0+1 everywhere
  // If WEIGHT 0 is used also WEIGHT 1 is used together, so it is unnecessary to keep both values.
  // however it doesnt hurt .
  // For this reason, the 70kg pilot weight is not important.
  // If we want to adjust wingloading, we just need to change gross weight.
  WEIGHTS[0] = 70;                      // Pilot weight 
  WEIGHTS[1] = ww[0]-WEIGHTS[0];        // Glider empty weight
  WEIGHTS[2] = ww[1];                   // Ballast weight

  POLAR[2] = (double)(w3 - POLAR[0] *v3*v3 - POLAR[1]*v3);

  // now scale off weight
  POLAR[0] = POLAR[0] * (double)sqrt(WEIGHTS[0] + WEIGHTS[1]);
  POLAR[2] = POLAR[2] / (double)sqrt(WEIGHTS[0] + WEIGHTS[1]);

}




void PExtractParameter(TCHAR *Source, TCHAR *Destination, int DesiredFieldNumber)
{
  int index = 0;
  int dest_index = 0;
  int CurrentFieldNumber = 0;
  int StringLength        = 0;

  StringLength = _tcslen(Source);

  while( (CurrentFieldNumber < DesiredFieldNumber) && (index < StringLength) )
    {
      if ( Source[ index ] == ',' )
	{
	  CurrentFieldNumber++;
	}
      index++;
    }

  if ( CurrentFieldNumber == DesiredFieldNumber )
    {
      while( (index < StringLength)    &&
	     (Source[ index ] != ',') &&
	     (Source[ index ] != 0x00) )
	{
	  Destination[dest_index] = Source[ index ];
	  index++; dest_index++;
	}
      Destination[dest_index] = '\0';
    }
}


bool ReadWinPilotPolar(void) {

  TCHAR	szFile[MAX_PATH] = TEXT("\0");
  TCHAR ctemp[80];
  TCHAR TempString[READLINE_LENGTH+1];
  HANDLE hFile;

  double POLARV[3];
  double POLARW[3];
  double ww[2];
  bool foundline = false;

#ifdef HAVEEXCEPTIONS
  __try{
#endif

    // LS3 values, overwritten by loaded values
    // *LS-3  WinPilot POLAR file: MassDryGross[kg], MaxWaterBallast[liters], Speed1[km/h], Sink1[m/s], Speed2, Sink2, Speed3, Sink3
    // 403, 101, 115.03, -0.86, 174.04, -1.76, 212.72,	-3.4
    ww[0]= 403.0; // 383
    ww[1]= 101.0; // 121
    POLARV[0]= 115.03;
    POLARW[0]= -0.86;
    POLARV[1]= 174.04;
    POLARW[1]= -1.76;
    POLARV[2]= 212.72;
    POLARW[2]= -3.4;

    GetRegistryString(szRegistryPolarFile, szFile, MAX_PATH);
    if (_tcscmp(szFile,_T(""))==0) {
	StartupStore(_T("... Empty polar file, using Default%s"),NEWLINE);
	wcscpy(szFile,_T("%LOCAL_PATH%\\\\_Polars\\Default.plr"));
    }

    ExpandLocalPath(szFile);
    StartupStore(_T(". Loading polar file <%s>%s"),szFile,NEWLINE);

    hFile = CreateFile(szFile,GENERIC_READ,0,(LPSECURITY_ATTRIBUTES)NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

    if (hFile != INVALID_HANDLE_VALUE ){
#ifdef HAVEEXCEPTIONS
      __try{
#endif

        while(ReadString(hFile,READLINE_LENGTH,TempString) && (!foundline)){

		if (_tcslen(TempString) <10) continue;

          if(_tcsstr(TempString,TEXT("*")) != TempString) // Look For Comment
            {
              PExtractParameter(TempString, ctemp, 0);
		// weight of glider + pilot
              ww[0] = StrToDouble(ctemp,NULL);
		// weight of loadable ballast
              PExtractParameter(TempString, ctemp, 1);
              ww[1] = StrToDouble(ctemp,NULL);

              PExtractParameter(TempString, ctemp, 2);
              POLARV[0] = StrToDouble(ctemp,NULL);
              PExtractParameter(TempString, ctemp, 3);
              POLARW[0] = StrToDouble(ctemp,NULL);

              PExtractParameter(TempString, ctemp, 4);
              POLARV[1] = StrToDouble(ctemp,NULL);
              PExtractParameter(TempString, ctemp, 5);
              POLARW[1] = StrToDouble(ctemp,NULL);

              PExtractParameter(TempString, ctemp, 6);
              POLARV[2] = StrToDouble(ctemp,NULL);
              PExtractParameter(TempString, ctemp, 7);
              POLARW[2] = StrToDouble(ctemp,NULL);

		_stprintf(ctemp,_T(""));
              	PExtractParameter(TempString, ctemp, 8);
		if ( _tcscmp(ctemp,_T("")) != 0) {
              		GlidePolar::WingArea = StrToDouble(ctemp,NULL);
			// StartupStore(_T(". Polar file has wing area=%f%s"),GlidePolar::WingArea,NEWLINE);
		} else {
	      		GlidePolar::WingArea = 0.0;
			StartupStore(_T("... WARNING Polar file has NO wing area%s"),NEWLINE);
		}
		
              PolarWinPilot2XCSoar(POLARV, POLARW, ww);

              foundline = true;
            }
        }

	int i;

	// Reset flaps values after loading a new polar, and init FlapsPos for the first time
	for (i=0; i<MAX_FLAPS; i++) {
		GlidePolar::FlapsPos[i]=0.0;
		wcscpy(GlidePolar::FlapsName[i],_T("???"));
	}
	GlidePolar::FlapsPosCount=0;
	GlidePolar::FlapsMass=0.0;

	// Unless we check valid string, even with empty string currentFlapsPos will be positive,
	// and thus force Flaps calculations even with no extended polar.
	// Let's allow empty lines and comments in the polar file, before the flaps line is found.
	// 
	do {
	   if (_tcslen(TempString) <10) continue;
	   if(_tcsstr(TempString,TEXT("*")) == TempString) continue;
    	   // try to read flaps configuration line
     	   PExtractParameter(TempString, ctemp, 0);
     	   GlidePolar::FlapsMass = StrToDouble(ctemp,NULL);
     	   PExtractParameter(TempString, ctemp, 1);
     	   int flapsCount = (int) StrToDouble(ctemp,NULL);

     	   // int currentFlapsPos = 0;
     	   // GlidePolar::FlapsPos[currentFlapsPos][0] = 0.0;  // no need, already initialised

     	   int currentFlapsPos=1;
     	   for (i=2; i <= flapsCount*2; i=i+2) {
	        PExtractParameter(TempString, ctemp, i);
	        GlidePolar::FlapsPos[currentFlapsPos] = StrToDouble(ctemp,NULL);	
	        PExtractParameter(TempString, ctemp, i+1);
		ctemp[MAXFLAPSNAME]='\0';
		if (ctemp[_tcslen(ctemp)-1]=='\r' || ctemp[_tcslen(ctemp)-1]=='\n')
			ctemp[_tcslen(ctemp)-1]='\0'; // remove trailing cr
		wcscpy(GlidePolar::FlapsName[currentFlapsPos],ctemp);
		if (currentFlapsPos >= (MAX_FLAPS-1)) break; // safe check
	        currentFlapsPos++;
       	   }
	   wcscpy(GlidePolar::FlapsName[0],GlidePolar::FlapsName[1]);
           GlidePolar::FlapsPos[currentFlapsPos] = SPEEDMODIFY*MAXSPEED;
           wcscpy(GlidePolar::FlapsName[currentFlapsPos],ctemp);
           currentFlapsPos++;
           GlidePolar::FlapsPosCount = currentFlapsPos; 
	   break;
	} while(ReadString(hFile,READLINE_LENGTH,TempString));

        // file was OK, so save it
        if (foundline) {
          ContractLocalPath(szFile);
          SetRegistryString(szRegistryPolarFile, szFile);
        }
#ifdef HAVEEXCEPTIONS
      }__finally
#endif
      {
        CloseHandle (hFile);
      }
    } 
    else {
	StartupStore(_T("... Polar file <%s> not found!%s"),szFile,NEWLINE);
    }
#ifdef HAVEEXCEPTIONS
  }__except(EXCEPTION_EXECUTE_HANDLER){
    foundline = false;
  }
#endif
  return(foundline);

}



typedef double PolarCoefficients_t[3];
typedef double WeightCoefficients_t[3];


void CalculateNewPolarCoef(void)
{

  // StartupStore(TEXT(". Calculate New Polar Coef%s"),NEWLINE);

  GlidePolar::WeightOffset=0; // 100131 

  // Load polar file
  if (ReadWinPilotPolar()) return;

  // Polar File error, we load a Ka6 just to be safe

  POLAR[0]=-0.0538770500225782443497;
  POLAR[1]=0.1323114348;
  POLAR[2]=-0.1273364037098239098543;
  WEIGHTS[0]=70;
  WEIGHTS[1]=190;
  WEIGHTS[2]=1;
  GlidePolar::WingArea = 12.4;

  // Probably called from wrong thread - check
  MessageBoxX(NULL, 
              gettext(TEXT("_@M920_")), // Error loading Polar file!
              gettext(TEXT("_@M791_")), // Warning
              MB_OK|MB_ICONERROR);

}




void FindLatitudeLongitude(double Lat, double Lon, 
                           double Bearing, double Distance,
                           double *lat_out, double *lon_out)
{
  double result;

  Lat *= DEG_TO_RAD;
  Lon *= DEG_TO_RAD;
  Bearing *= DEG_TO_RAD;
  Distance = Distance/6371000;

  double sinDistance = sin(Distance);
  double cosLat = cos(Lat);

  if (lat_out) {
    result = (double)asin(sin(Lat)*cos(Distance)
                          +cosLat*sinDistance*cos(Bearing));
    result *= RAD_TO_DEG;
    *lat_out = result;
  }
  if (lon_out) {
    if(cosLat==0)
      result = Lon;
    else {
      result = Lon+(double)asin(sin(Bearing)*sinDistance/cosLat);
      result = (double)fmod((result+M_PI),(M_2PI));
      result = result - M_PI;
    }
    result *= RAD_TO_DEG;
    *lon_out = result;
  }
}


static double xcoords[64] = {
  0,			0.09801714,		0.195090322,	0.290284677,	0.382683432,	0.471396737,	0.555570233,	0.634393284,
  0.707106781,	0.773010453,	0.831469612,	0.881921264,	0.923879533,	0.956940336,	0.98078528,		0.995184727,
  1,			0.995184727,	0.98078528,		0.956940336,	0.923879533,	0.881921264,	0.831469612,	0.773010453,
  0.707106781,	0.634393284,	0.555570233,	0.471396737,	0.382683432,	0.290284677,	0.195090322,	0.09801714,
  0,			-0.09801714,	-0.195090322,	-0.290284677,	-0.382683432,	-0.471396737,	-0.555570233,	-0.634393284,
  -0.707106781,	-0.773010453,	-0.831469612,	-0.881921264,	-0.923879533,	-0.956940336,	-0.98078528,	-0.995184727,
  -1,			-0.995184727,	-0.98078528,	-0.956940336,	-0.923879533,	-0.881921264,	-0.831469612,	-0.773010453,
  -0.707106781,	-0.634393284,	-0.555570233,	-0.471396737,	-0.382683432,	-0.290284677,	-0.195090322,	-0.09801714
};

static double ycoords[64] = {
  1,			0.995184727,	0.98078528,		0.956940336,	0.923879533,	0.881921264,	0.831469612,	0.773010453,
  0.707106781,	0.634393284,	0.555570233,	0.471396737,	0.382683432,	0.290284677,	0.195090322,	0.09801714,
  0,			-0.09801714,	-0.195090322,	-0.290284677,	-0.382683432,	-0.471396737,	-0.555570233,	-0.634393284,
  -0.707106781,	-0.773010453,	-0.831469612,	-0.881921264,	-0.923879533,	-0.956940336,	-0.98078528,	-0.995184727,
  -1,			-0.995184727,	-0.98078528,	-0.956940336,	-0.923879533,	-0.881921264,	-0.831469612,	-0.773010453,
  -0.707106781,	-0.634393284,	-0.555570233,	-0.471396737,	-0.382683432,	-0.290284677,	-0.195090322,	-0.09801714,
  0,			0.09801714,		0.195090322,	0.290284677,	0.382683432,	0.471396737,	0.555570233,	0.634393284,
  0.707106781,	0.773010453,	0.831469612,	0.881921264,	0.923879533,	0.956940336,	0.98078528,		0.995184727
};


void StartArc(HDC hdc, 
	      double longitude0, double latitude0,
	      double longitude1, double latitude1, 
	      double arclength) {

  double radius, bearing;
  DistanceBearing(latitude0, longitude0, 
                  latitude1, longitude1,
                  &radius,
                  &bearing);
  double angle = 360*min(1, arclength/(2.0*3.1415926*radius));
  int i0 = (int)(bearing+angle/2);
  int i1 = (int)(bearing-angle/2);
  int i;
  if (i0<0) { i1+= 360; }
  if (i1<0) { i1+= 360; }
  if (i0>360) {i0-= 360; }
  if (i1>360) {i1-= 360; }
  i0 = i0*64/360;
  i1 = i1*64/360;
  POINT pt[2];
//  double lat, lon;
  int x=0;
  int y=0;

  if (i1<i0) {
    for (i=i0; i<64-1; i++) {
      //      MapWindow::LatLon2Screen(lon, lat, &scx, &scy);
      pt[0].x = x + (long) (radius * xcoords[i]);
      pt[0].y = y + (long) (radius * ycoords[i]);
      pt[1].x = x + (long) (radius * xcoords[i+1]);
      pt[1].y = y + (long) (radius * ycoords[i+1]);
      Polygon(hdc,pt,2);
    }
    for (i=0; i<i1-1; i++) {
      pt[0].x = x + (long) (radius * xcoords[i]);
      pt[0].y = y + (long) (radius * ycoords[i]);
      pt[1].x = x + (long) (radius * xcoords[i+1]);
      pt[1].y = y + (long) (radius * ycoords[i+1]);
      Polygon(hdc,pt,2);
    }
  } else {
    for (i=i0; i<i1-1; i++) {
      pt[0].x = x + (long) (radius * xcoords[i]);
      pt[0].y = y + (long) (radius * ycoords[i]);
      pt[1].x = x + (long) (radius * xcoords[i+1]);
      pt[1].y = y + (long) (radius * ycoords[i+1]);
      Polygon(hdc,pt,2);
    }
  }

}


int Circle(HDC hdc, long x, long y, int radius, RECT rc, bool clip, bool fill)
{
  POINT pt[65];
  unsigned int i;

  rectObj rect;
  rect.minx = x-radius;
  rect.maxx = x+radius;
  rect.miny = y-radius;
  rect.maxy = y+radius;
  rectObj rcrect;
  rcrect.minx = rc.left;
  rcrect.maxx = rc.right;
  rcrect.miny = rc.top;
  rcrect.maxy = rc.bottom;

  if (msRectOverlap(&rect, &rcrect)!=MS_TRUE) {
    return FALSE;
  }
  // JMW added faster checking...

  unsigned int step = 1;
  if (radius<20) {
    step = 2;
  }
  for(i=64/step;i--;) {
    pt[i].x = x + (long) (radius * xcoords[i*step]);
    pt[i].y = y + (long) (radius * ycoords[i*step]);
  }
  step = 64/step;
  pt[step].x = x + (long) (radius * xcoords[0]);
  pt[step].y = y + (long) (radius * ycoords[0]);

  if (clip) {
    ClipPolygon(hdc,pt,step+1,rc, fill);
  } else {
    if (fill) {
#ifdef PNA
    if (needclipping==true)
      	ClipPolygon(hdc,pt,step+1,rc, true); // VNT10 090909 fixed bug was false FIX CHECK IF WORKING 
    else
        Polygon(hdc,pt,step+1);
#else
      Polygon(hdc,pt,step+1); 
#endif 
    } else {
// VENTA3 FIX HP clipping bug
#ifdef PNA
      if (needclipping==true)
      	MapWindow::_Polyline(hdc,pt,step+1,rc);
      else
        Polyline(hdc,pt,step+1);
#else
      Polyline(hdc,pt,step+1);
#endif
    }
  }
  return TRUE;
}


int Segment(HDC hdc, long x, long y, int radius, RECT rc, 
	    double start,
	    double end,
            bool horizon)
{
  POINT pt[66];
  int i;
  int istart;
  int iend;

  rectObj rect;
  rect.minx = x-radius;
  rect.maxx = x+radius;
  rect.miny = y-radius;
  rect.maxy = y+radius;
  rectObj rcrect;
  rcrect.minx = rc.left;
  rcrect.maxx = rc.right;
  rcrect.miny = rc.top;
  rcrect.maxy = rc.bottom;

  if (msRectOverlap(&rect, &rcrect)!=MS_TRUE) {
    return FALSE;
  }

  // JMW added faster checking...

  start = AngleLimit360(start);
  end = AngleLimit360(end);

  istart = iround(start/360.0*64);
  iend = iround(end/360.0*64);

  int npoly = 0;

  if (istart>iend) {
    iend+= 64;
  }
  istart++;
  iend--;

  if (!horizon) {
    pt[0].x = x; pt[0].y = y; npoly=1;
  }
  pt[npoly].x = x + (long) (radius * fastsine(start));
  pt[npoly].y = y - (long) (radius * fastcosine(start));
  npoly++;

  for(i=0;i<64;i++) {
    if (i<=iend-istart) {
      pt[npoly].x = x + (long) (radius * xcoords[(i+istart)%64]);
      pt[npoly].y = y - (long) (radius * ycoords[(i+istart)%64]);
      npoly++;
    }
  }
  pt[npoly].x = x + (long) (radius * fastsine(end));
  pt[npoly].y = y - (long) (radius * fastcosine(end));
  npoly++;

  if (!horizon) {
    pt[npoly].x = x; 
    pt[npoly].y = y; npoly++;
  } else {
    pt[npoly].x = pt[0].x;
    pt[npoly].y = pt[0].y;
    npoly++;
  }
  if (npoly) {
    Polygon(hdc,pt,npoly);
  }
  
  return TRUE;
}

/*
 * VENTA3 This is a modified Segment() 
 */
int DrawArc(HDC hdc, long x, long y, int radius, RECT rc, 
	    double start,
	    double end)
{
  POINT pt[66];
  int i;
  int istart;
  int iend;

  rectObj rect;
  rect.minx = x-radius;
  rect.maxx = x+radius;
  rect.miny = y-radius;
  rect.maxy = y+radius;
  rectObj rcrect;
  rcrect.minx = rc.left;
  rcrect.maxx = rc.right;
  rcrect.miny = rc.top;
  rcrect.maxy = rc.bottom;

  if (msRectOverlap(&rect, &rcrect)!=MS_TRUE) {
    return FALSE;
  }

  // JMW added faster checking...

  start = AngleLimit360(start);
  end = AngleLimit360(end);

  istart = iround(start/360.0*64);
  iend = iround(end/360.0*64);

  int npoly = 0;

  if (istart>iend) {
    iend+= 64;
  }
  istart++;
  iend--;

  pt[npoly].x = x + (long) (radius * fastsine(start));
  pt[npoly].y = y - (long) (radius * fastcosine(start));
  npoly++;

  for(i=0;i<64;i++) {
    if (i<=iend-istart) {
      pt[npoly].x = x + (long) (radius * xcoords[(i+istart)%64]);
      pt[npoly].y = y - (long) (radius * ycoords[(i+istart)%64]);
      npoly++;
    }
  }
  pt[npoly].x = x + (long) (radius * fastsine(end));
  pt[npoly].y = y - (long) (radius * fastcosine(end));
  npoly++;
  if (npoly) {
    Polyline(hdc,pt,npoly); // TODO check ClipPolygon for HP31X
  }
  
  return TRUE;
}

BOOL PolygonVisible(const POINT *lpPoints, int nCount, RECT rc)
{
  BOOL Sector[9] = {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE};
  int i;
  int Count = 0;
  (void)rc;
  //return TRUE;

  for(i=0;i<nCount;i++)
    {
      if(lpPoints[i].y < MapWindow::MapRect.top)
	{
	  if(lpPoints[i].x < MapWindow::MapRect.left)
	    {
	      Sector[0] = TRUE;
	    }
	  else if((lpPoints[i].x >=MapWindow::MapRect.left) 
		  && (lpPoints[i].x <MapWindow::MapRect.right))
	    {
	      Sector[1] = TRUE;
	    }
	  else if(lpPoints[i].x >=MapWindow::MapRect.right)
	    {
	      Sector[2] = TRUE;
	    }
	}
      else if((lpPoints[i].y >=MapWindow::MapRect.top) 
	      && (lpPoints[i].y <MapWindow::MapRect.bottom))
	{
	  if(lpPoints[i].x <MapWindow::MapRect.left)
	    {
	      Sector[3] = TRUE;
	    }
	  else if((lpPoints[i].x >=MapWindow::MapRect.left) 
		  && (lpPoints[i].x <MapWindow::MapRect.right))
	    {
	      Sector[4] = TRUE;
	      return TRUE;
	    }
	  else if(lpPoints[i].x >=MapWindow::MapRect.right)
	    {
	      Sector[5] = TRUE;
	    }
	}
      else if(lpPoints[i].y >=MapWindow::MapRect.bottom)
	{
	  if(lpPoints[i].x <MapWindow::MapRect.left)
	    {
	      Sector[6] = TRUE;
	    }
	  else if((lpPoints[i].x >=MapWindow::MapRect.left) 
		  && (lpPoints[i].x <MapWindow::MapRect.right))
	    {
	      Sector[7] = TRUE;
	    }
	  else if(lpPoints[i].x >=MapWindow::MapRect.right)
	    {
	      Sector[8] = TRUE;
	    }
	}
    }

  for(i=0;i<9;i++)
    {
      if(Sector[i])
	{
	  Count ++;
	}
    }

  if(Count>= 2)
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

#define CheckIndex(x, i)    ASSERT((i>=0) && (sizeof(x)/sizeof(x[0]) > i));

void SetRegistryColour(int i, DWORD c)
{

  CheckIndex(szRegistryColour, i);

  SetToRegistry(szRegistryColour[i] ,c) ;
}


void SetRegistryBrush(int i, DWORD c)
{
  CheckIndex(szRegistryBrush, i);

  SetToRegistry(szRegistryBrush[i] ,c) ;
}



void SetRegistryAirspaceMode(int i)
{

  CheckIndex(MapWindow::iAirspaceMode, i);
  CheckIndex(szRegistryAirspaceMode, i);

  DWORD val = MapWindow::iAirspaceMode[i];
  SetToRegistry(szRegistryAirspaceMode[i], val);
}

int GetRegistryAirspaceMode(int i) {
  DWORD Temp= 3; // display + warnings
  CheckIndex(szRegistryAirspaceMode, i);
  GetFromRegistry(szRegistryAirspaceMode[i],&Temp);
  return Temp;
}


void ReadAssetNumber(void)
{
  TCHAR val[MAX_PATH];

  val[0]= _T('\0');

  memset(strAssetNumber, 0, MAX_LOADSTRING*sizeof(TCHAR));
  // JMW clear this first just to be safe.

  StartupStore(TEXT(". Asset ID: "));

#if (WINDOWSPC>0)
	strAssetNumber[0]= _T('L');
	strAssetNumber[1]= _T('K');
	strAssetNumber[2]= _T('8');
	#if TESTBENCH
	StartupStore(strAssetNumber);
	StartupStore(TEXT(" (PC)%s"),NEWLINE);
	#endif
	return;
#endif

  GetRegistryString(szRegistryLoggerID, val, 100);
  int ifound=0;
  int len = _tcslen(val);
  for (int i=0; i< len; i++) {
    if (((val[i] >= _T('A'))&&(val[i] <= _T('Z')))
        ||((val[i] >= _T('0'))&&(val[i] <= _T('9')))) {
      strAssetNumber[ifound]= val[i];
      ifound++;
    }
    if (ifound>=3) {
// TODO metti a 0 l'ultimo byte per chiudere la stringa
      StartupStore(strAssetNumber);
      StartupStore(TEXT(" (reg)%s"),NEWLINE);
      return;
    }
  }

  if (ifound>0 && ifound<3) {
	if (ifound==1) strAssetNumber[1]= _T('A');
	strAssetNumber[2]= _T('A');
  }

  if(strAssetNumber[0] != '\0')
    {
      StartupStore(strAssetNumber);
      StartupStore(TEXT(" (?)%s"),NEWLINE);
      return;
    }

  ReadUUID();
  if(strAssetNumber[0] != '\0')
    {
      StartupStore(strAssetNumber);
      StartupStore(TEXT(" (uuid)%s"),NEWLINE);
      return;
    }
  
  strAssetNumber[0]= _T('A');
  strAssetNumber[1]= _T('A');
  strAssetNumber[2]= _T('A');

  StartupStore(strAssetNumber);
  StartupStore(TEXT(" (fallback)%s"),NEWLINE);
  
  return;
}


void ReadUUID(void) 
{
#if !(defined(__MINGW32__) && (WINDOWSPC>0))
  BOOL fRes;
  
#define GUIDBuffsize 100
  unsigned char GUIDbuffer[GUIDBuffsize];

  int eLast=0;
  int i;
  unsigned long uNumReturned=0;
  int iBuffSizeIn=0;
  unsigned long temp, Asset;


  GUID Guid;

  
  // approach followed: http://blogs.msdn.com/jehance/archive/2004/07/12/181116.aspx
  // 1) send 16 byte buffer - some older devices need this
  // 2) if buffer is wrong size, resize buffer accordingly and retry
  // 3) take first 16 bytes of buffer and process.  Buffer returned may be any size 
  // First try exactly 16 bytes, some older PDAs require exactly 16 byte buffer

      #ifdef HAVEEXCEPTIONS
    __try {
      #else
	  strAssetNumber[0]= '\0';
      #endif

	  iBuffSizeIn=sizeof(Guid);
	  memset(GUIDbuffer, 0, iBuffSizeIn);
	  fRes = KernelIoControl(IOCTL_HAL_GET_DEVICEID, 0, 0, GUIDbuffer, iBuffSizeIn, &uNumReturned);
	  if(fRes == FALSE)
	  { // try larger buffer
		  eLast = GetLastError();
		  if (ERROR_INSUFFICIENT_BUFFER != eLast)
		  {
			return;
		  }
		  else
		  { // wrong buffer
			iBuffSizeIn = uNumReturned;
			memset(GUIDbuffer, 0, iBuffSizeIn);
			fRes = KernelIoControl(IOCTL_HAL_GET_DEVICEID, 0, 0, GUIDbuffer, iBuffSizeIn, &uNumReturned);
  			eLast = GetLastError();
			if(FALSE == fRes)
				return;
		  }
	  }

	  // here assume we have data in GUIDbuffer of length uNumReturned
	  memcpy(&Guid,GUIDbuffer, sizeof(Guid));
  

	  temp = Guid.Data2; temp = temp << 16;
	  temp += Guid.Data3 ;

	  Asset = temp ^ Guid.Data1 ;

	  temp = 0;
	  for(i=0;i<4;i++)
		{
		  temp = temp << 8;
		  temp += Guid.Data4[i];
		}

	  Asset = Asset ^ temp;

	  temp = 0;
	  for(i=0;i<4;i++)
		{
		  temp = temp << 8;
		  temp += Guid.Data4[i+4];
		}

	  Asset = Asset ^ temp;

	  _stprintf(strAssetNumber,TEXT("%08X%08X"),Asset,Guid.Data1 );

#ifdef HAVEEXCEPTIONS
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
	  strAssetNumber[0]= '\0';
  }
#endif
#endif
  return;
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


double ScreenAngle(int x1, int y1, int x2, int y2)
{
  return atan2((double)y2-y1, (double)x2-x1)*RAD_TO_DEG;
}

void FormatWarningString(int Type, TCHAR *Name , AIRSPACE_ALT Base, AIRSPACE_ALT Top, TCHAR *szMessageBuffer, TCHAR *szTitleBuffer )
{
  TCHAR BaseStr[512];
  TCHAR TopStr[512];

  switch (Type)
    {
    case RESTRICT:	  
	// LKTOKEN  _@M565_ = "Restricted" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M565_"))); break;
    case PROHIBITED:	  
	// LKTOKEN  _@M537_ = "Prohibited" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M537_"))); break;
    case DANGER:          
	// LKTOKEN  _@M213_ = "Danger Area" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M213_"))); break;
    case CLASSA:          
      _tcscpy(szTitleBuffer,TEXT("Class A")); break;
    case CLASSB:          
      _tcscpy(szTitleBuffer,TEXT("Class B")); break;
    case CLASSC:          
      _tcscpy(szTitleBuffer,TEXT("Class C")); break;
    case CLASSD:          
      _tcscpy(szTitleBuffer,TEXT("Class D")); break;
    case CLASSE:			
      _tcscpy(szTitleBuffer,TEXT("Class E")); break;
    case CLASSF:			
      _tcscpy(szTitleBuffer,TEXT("Class F")); break;
    case CLASSG:			
      _tcscpy(szTitleBuffer,TEXT("Class G")); break;
    case NOGLIDER:		
	// LKTOKEN  _@M464_ = "No Glider" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M464_"))); break;
    case CTR:					
      _tcscpy(szTitleBuffer,TEXT("CTR")); break;
    case WAVE:				
	// LKTOKEN  _@M794_ = "Wave" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M794_"))); break;
    case CLASSTMZ:            
      _tcscpy(szTitleBuffer,TEXT("TMZ")); break;
    default:					
	// LKTOKEN  _@M765_ = "Unknown" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M765_")));
    }

  if(Base.FL == 0)
    {
      if (Base.AGL > 0) {
        _stprintf(BaseStr,TEXT("%1.0f%s %s"), 
                  ALTITUDEMODIFY * Base.AGL, 
                  Units::GetUnitName(Units::GetUserAltitudeUnit()),
                  TEXT("AGL"));
      } else if (Base.Altitude > 0)
        _stprintf(BaseStr,TEXT("%1.0f%s %s"), 
                  ALTITUDEMODIFY * Base.Altitude, 
                  Units::GetUnitName(Units::GetUserAltitudeUnit()),
                  TEXT("MSL"));
      else
        _stprintf(BaseStr,TEXT("GND"));
    }
  else
    {
      _stprintf(BaseStr,TEXT("FL %1.0f"),Base.FL );
    }

  if(Top.FL == 0)
    {
      if (Top.AGL > 0) {
        _stprintf(TopStr,TEXT("%1.0f%s %s"), 
                  ALTITUDEMODIFY * Top.AGL, 
                  Units::GetUnitName(Units::GetUserAltitudeUnit()),
                  TEXT("AGL"));
      } else {
	_stprintf(TopStr,TEXT("%1.0f%s %s"), ALTITUDEMODIFY * Top.Altitude, 
		  Units::GetUnitName(Units::GetUserAltitudeUnit()),
		  TEXT("MSL"));
      }
    }
  else
    {
      _stprintf(TopStr,TEXT("FL %1.0f"),Top.FL );
    }

  _stprintf(szMessageBuffer,TEXT("%s: %s\r\n%s: %s\r\n%s: %s\r\n"),
            szTitleBuffer, 
            Name, 
	// LKTOKEN  _@M729_ = "Top" 
            gettext(TEXT("_@M729_")),
            TopStr,
	// LKTOKEN  _@M128_ = "Base" 
            gettext(TEXT("_@M128_")),
            BaseStr 
            );
}


// JMW added support for zzip files

BOOL ReadString(ZZIP_FILE *zFile, int Max, TCHAR *String)
{
  char sTmp[READLINE_LENGTH+1];
  char FileBuffer[READLINE_LENGTH+1];
  long dwNumBytesRead=0;
  long dwTotalNumBytesRead=0;
  long dwFilePos;

  String[0] = '\0';
  sTmp[0] = 0;

  ASSERT(Max<sizeof(sTmp));

  if (Max >= (int)(sizeof(sTmp))) // 100207 fixed signed unsigned (int)
    return(FALSE);
  if (!zFile)
    return(FALSE);

  dwFilePos = zzip_tell(zFile);

  dwNumBytesRead = zzip_fread(FileBuffer, 1, Max, zFile);
  if (dwNumBytesRead <= 0)
    return(FALSE);

  int i = 0;
  int j = 0;
  while((i<Max) && (j<(int)dwNumBytesRead)) {

    char c = FileBuffer[j];
    j++;
    dwTotalNumBytesRead++;

    if((c == '\n')){
      break;
    }

    sTmp[i] = c;
    i++;
  }

  sTmp[i] = 0;
  zzip_seek(zFile, dwFilePos+j, SEEK_SET);
  sTmp[Max-1] = '\0';
  mbstowcs(String, sTmp, strlen(sTmp)+1);
  return (dwTotalNumBytesRead>0);
}


// read string from file
// support national codepage
// hFile:  file handle
// Max:    max chars to fit in Buffer
// String: pointer to string buffer
// return: True if at least one byte was read from file
//         False Max > MAX_PATH or EOF or read error
BOOL ReadString(HANDLE hFile, int Max, TCHAR *String)
{
  char sTmp[READLINE_LENGTH+1];
  DWORD dwNumBytesRead=0;
  DWORD dwTotalNumBytesRead=0;
  char  FileBuffer[READLINE_LENGTH+1];
  DWORD dwFilePos;

  String[0] = '\0';
  sTmp[0] = 0;

  ASSERT(Max<sizeof(sTmp));

  if (Max >= (int)(sizeof(sTmp)))  
    return(FALSE);

  dwFilePos = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

  if (hFile == INVALID_HANDLE_VALUE)
    return(FALSE);

  if (ReadFile(hFile, FileBuffer, sizeof(FileBuffer), 
	       &dwNumBytesRead, (OVERLAPPED *)NULL) == 0)
    return(FALSE);

  int i = 0;
  int j = 0;
  while(i<Max && j<(int)dwNumBytesRead){

    char c = FileBuffer[j];
    j++;
    dwTotalNumBytesRead++;

    if((c == '\n')){
      break;
    }

    sTmp[i] = c;
    i++;
    continue;
  }

  sTmp[i] = 0;
  SetFilePointer(hFile, dwFilePos+j, NULL, FILE_BEGIN);
  sTmp[Max-1] = '\0';
  mbstowcs(String, sTmp, strlen(sTmp)+1);
  return (dwTotalNumBytesRead>0);

}

BOOL ReadStringX(FILE *fp, int Max, TCHAR *String){
  if (fp == NULL || Max < 1 || String == NULL) {
    if (String) {
      String[0]= '\0';
    }
    return (0);
  }

  if (_fgetts(String, Max, fp) != NULL){     // 20060512/sgi change 200 to max

    String[Max-1] = '\0';                    // 20060512/sgi added make shure the  string is terminated
    TCHAR *pWC = &String[max(0,_tcslen(String)-1)]; 
    // 20060512/sgi change add -1 to set pWC at the end of the string

    while (pWC > String && (*pWC == '\r' || *pWC == '\n')){
      *pWC = '\0';
      pWC--;
    }

    return (1);
  }

  return (0);

}



void InitSineTable(void)
{
  int i;
  double angle;
  double cosa, sina;

  for(i=0;i<4096; i++)
    {
      angle = DEG_TO_RAD*((double)i*360)/4096;
      cosa = cos(angle);
      sina = sin(angle);
      SINETABLE[i] = sina;
      COSTABLE[i] = cosa;
      ISINETABLE[i] = iround(sina*1024);
      ICOSTABLE[i] = iround(cosa*1024);
      if ((cosa>0) && (cosa<1.0e-8)) {
	cosa = 1.0e-8;
      }
      if ((cosa<0) && (cosa>-1.0e-8)) {
	cosa = -1.0e-8;
      }
      INVCOSINETABLE[i] = 1.0/cosa;
    }
}

/*
 * WARNING: if you are using Stop pointer, remember that you MUST set it equal to *Source before
   calling this function, because in case of null result, it will not be set!!
 */
double StrToDouble(TCHAR *Source, TCHAR **Stop)
{
  int index = 0;
  int StringLength        = 0;
  double Sum = 0;
  double Divisor = 10;
  int neg = 0;

  StringLength = _tcslen(Source);

  while(((Source[index] == ' ')||(Source[index]=='+')||(Source[index]==9)) 
        && (index<StringLength))
    // JMW added skip for tab stop
    // JMW added skip for "+"
    {
      index ++;
    }
  if (index>= StringLength) {
	// WARNING> we are not setting Stop here!!
    return 0.0; // Set 0.0 as error, probably not the best thing to do. TOFIX 110307
  }
  if (Source[index]=='-') {
    neg=1;
    index++;
  }

  while( (index < StringLength)
	 &&
	 (
	  (Source[index]>= '0') && (Source [index] <= '9')
          )
	 )
    {
      Sum = (Sum*10) + (Source[ index ] - '0');
      index ++;
    }
  if(Source[index] == '.')
    {
      index ++;
      while( (index < StringLength)
	     &&
	     (
	      (Source[index]>= '0') && (Source [index] <= '9')
	      )
	     )
	{
	  Sum = (Sum) + (double)(Source[ index ] - '0')/Divisor;
	  index ++;Divisor = Divisor * 10;
	}
    }
  if(Stop != NULL)
    *Stop = &Source[index];

  if (neg) {
    return -Sum;
  } else {
    return Sum;
  }
}


// RMN: Volkslogger outputs data in hex-strings.  Function copied from StrToDouble
// Note: Decimal-point and decimals disregarded.  Assuming integer amounts only.
double HexStrToDouble(TCHAR *Source, TCHAR **Stop) 
{
  int index = 0;
  int StringLength        = 0;
  double Sum = 0;
  int neg = 0;

  StringLength = _tcslen(Source);

  while((Source[index] == ' ')||(Source[index]==9))
    // JMW added skip for tab stop
    {
      index ++;
    }
  if (Source[index]=='-') {
    neg=1;
    index++;
  }

  while( 
  (index < StringLength)	 &&	 
	(	( (Source[index]>= '0') && (Source [index] <= '9')  ) || 
		( (Source[index]>= 'A') && (Source [index] <= 'F')  ) || 
		( (Source[index]>= 'a') && (Source [index] <= 'f')  )
		)	 
	)
    {
      if((Source[index]>= '0') && (Source [index] <= '9'))	  {
		Sum = (Sum*16) + (Source[ index ] - '0');
		index ++;
	  }
	  if((Source[index]>= 'A') && (Source [index] <= 'F'))	  {
		Sum = (Sum*16) + (Source[ index ] - 'A' + 10);
		index ++;
	  }
	  if((Source[index]>= 'a') && (Source [index] <= 'f'))	  {
		Sum = (Sum*16) + (Source[ index ] - 'a' + 10);
		index ++;
	  }
    }
  
  if(Stop != NULL)
    *Stop = &Source[index];

  if (neg) {
    return -Sum;
  } else {
    return Sum;
  }
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


unsigned int isqrt4(unsigned long val) {
  unsigned int temp, g=0;

  if (val >= 0x40000000) {
    g = 0x8000;
    val -= 0x40000000;
  }

#define INNER_MBGSQRT(s)                      \
  temp = (g << (s)) + (1 << ((s) * 2 - 2));   \
  if (val >= temp) {                          \
    g += 1 << ((s)-1);                        \
    val -= temp;                              \
  }

  INNER_MBGSQRT (15)
  INNER_MBGSQRT (14)
  INNER_MBGSQRT (13)
  INNER_MBGSQRT (12)
  INNER_MBGSQRT (11)
  INNER_MBGSQRT (10)
  INNER_MBGSQRT ( 9)
  INNER_MBGSQRT ( 8)
  INNER_MBGSQRT ( 7)
  INNER_MBGSQRT ( 6)
  INNER_MBGSQRT ( 5)
  INNER_MBGSQRT ( 4)
  INNER_MBGSQRT ( 3)
  INNER_MBGSQRT ( 2)

#undef INNER_MBGSQRT

  temp = g+g+1;
  if (val >= temp) g++;
  return g;
}

// http://www.azillionmonkeys.com/qed/sqroot.html




static int ByteCRC16(int value, int crcin)
{
    int k = (((crcin >> 8) ^ value) & 255) << 8;
    int crc = 0;
    int bits = 8;
    do
    {
        if (( crc ^ k ) & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc <<= 1;
        k <<= 1;
    }
    while (--bits);
    return ((crcin << 8) ^ crc);
}

WORD crcCalc(void *Buffer, size_t size){

  int crc = 0;
  unsigned char *pB = (unsigned char *)Buffer;

  do {
    int value = *pB++;
    crc = ByteCRC16(value, crc);
  } while (--size);

  return((WORD)crc);
}

void ExtractDirectory(TCHAR *Dest, TCHAR *Source) {
  int len = _tcslen(Source);
  int found = -1;
  int i;
  if (len==0) {
    Dest[0]= 0;
    return;
  }
  for (i=0; i<len; i++) {
    if ((Source[i]=='/')||(Source[i]=='\\')) {
      found = i;
    }
  }
  for (i=0; i<=found; i++) {
    Dest[i]= Source[i];
  }
  Dest[i]= 0;
}


/*
 * Copyright (c) 1990 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. [rescinded 22 July 1999]
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Perform a binary search.
 *
 * The code below is a bit sneaky.  After a comparison fails, we
 * divide the work in half by moving either left or right. If lim
 * is odd, moving left simply involves halving lim: e.g., when lim
 * is 5 we look at item 2, so we change lim to 2 so that we will
 * look at items 0 & 1.  If lim is even, the same applies.  If lim
 * is odd, moving right again involes halving lim, this time moving
 * the base up one item past p: e.g., when lim is 5 we change base
 * to item 3 and make lim 2 so that we will look at items 3 and 4.
 * If lim is even, however, we have to shrink it by one before
 * halving: e.g., when lim is 4, we still looked at item 2, so we
 * have to make lim 3, then halve, obtaining 1, so that we will only
 * look at item 3.
 */

void *bsearch(void *key, void *base0, size_t nmemb, size_t size, int (*compar)(const void *elem1, const void *elem2)){
	void *base = base0;
	int lim, cmp;
	void *p;

	for (lim = nmemb; lim != 0; lim >>= 1) {
		p = (char *)base + (lim >> 1) * size;
		cmp = (*compar)(key, p);
		if (cmp == 0)
			return (p);
		if (cmp > 0) {	/* key > p: move right */
			base = (char *)p + size;
			lim--;
		} /* else move left */
	}
	return (NULL);
}



TCHAR *strtok_r(TCHAR *s, TCHAR *delim, TCHAR **lasts){
// "s" MUST be a pointer to an array, not to a string!!! 
// (ARM92, Win emulator cause access violation if not)

  TCHAR *spanp;
	int   c, sc;
	TCHAR *tok;


	if (s == NULL && (s = *lasts) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */

cont:
	c = *s++;
	for (spanp = (TCHAR *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		*lasts = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (TCHAR *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;  // causes access violation in some configs if s is a pointer instead of an array
				*lasts = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}


void StatusFileInit() {
  StartupStore(TEXT(". StatusFileInit%s"),NEWLINE);

  // DEFAULT - 0 is loaded as default, and assumed to exist
  StatusMessageData[0].key = TEXT("DEFAULT");
  StatusMessageData[0].doStatus = true;
  StatusMessageData[0].doSound = true; 
  StatusMessageData[0].sound = TEXT("IDR_WAV_DRIP");
  StatusMessageData_Size=1;
#ifdef VENTA_DEBUG_EVENT // VENTA- longer statusmessage delay in event debug mode
	StatusMessageData[0].delay_ms = 10000;  // 10 s
#else
    StatusMessageData[0].delay_ms = 2500; // 2.5 s
#endif

  // Load up other defaults - allow overwrite in config file
#include "Status_defaults.cpp"

}

void ReadStatusFile() {
  
  StartupStore(TEXT(". Loading status file%s"),NEWLINE);

  TCHAR szFile1[MAX_PATH] = TEXT("\0");
  FILE *fp=NULL;
  
  // Open file from registry
  GetRegistryString(szRegistryStatusFile, szFile1, MAX_PATH);
  ExpandLocalPath(szFile1);

  SetRegistryString(szRegistryStatusFile, TEXT("\0"));
  
  if (_tcslen(szFile1)>0)
    fp  = _tfopen(szFile1, TEXT("rt"));
  
  // Unable to open file
  if (fp == NULL)
    return;
  
  // TODO code: Safer sizes, strings etc - use C++ (can scanf restrict length?)
  TCHAR buffer[2049];	// Buffer for all
  TCHAR key[2049];	// key from scanf
  TCHAR value[2049];	// value from scanf
  int ms;				// Found ms for delay
  TCHAR **location;	// Where to put the data
  int found;			// Entries found from scanf
  bool some_data;		// Did we find some in the last loop...
  
  // Init first entry
  _init_Status(StatusMessageData_Size);
  some_data = false;
  
  /* Read from the file */
  while (
	 (StatusMessageData_Size < MAXSTATUSMESSAGECACHE)
	 && fgetws(buffer, 2048, fp)
	 && ((found = swscanf(buffer, TEXT("%[^#=]=%[^\n]\n"), key, value)) != EOF)
	 ) {
    // Check valid line? If not valid, assume next record (primative, but works ok!)
    if ((found != 2) || key[0] == '\0' || value == '\0') {
      
      // Global counter (only if the last entry had some data)
      if (some_data) {
	StatusMessageData_Size++;
	some_data = false;
	_init_Status(StatusMessageData_Size);
      }
      
    } else {
      
      location = NULL;
      
      if (wcscmp(key, TEXT("key")) == 0) {
	some_data = true;	// Success, we have a real entry
	location = &StatusMessageData[StatusMessageData_Size].key;
      } else if (wcscmp(key, TEXT("sound")) == 0) {
	StatusMessageData[StatusMessageData_Size].doSound = true;
	location = &StatusMessageData[StatusMessageData_Size].sound;
      } else if (wcscmp(key, TEXT("delay")) == 0) {
	if (swscanf(value, TEXT("%d"), &ms) == 1)
	  StatusMessageData[StatusMessageData_Size].delay_ms = ms;
      } else if (wcscmp(key, TEXT("hide")) == 0) {
	if (wcscmp(value, TEXT("yes")) == 0)
	  StatusMessageData[StatusMessageData_Size].doStatus = false;
      }
      
      // Do we have somewhere to put this && is it currently empty ? (prevent lost at startup)
      if (location && (wcscmp(*location, TEXT("")) == 0)) {
	// TODO code: this picks up memory lost from no entry, but not duplicates - fix.
	if (*location) {
	  // JMW fix memory leak
	  free(*location);
	}
	*location = StringMallocParse(value);
      }
    }
    
  }
  
  // How many we really got (blank next just in case)
  StatusMessageData_Size++;
  _init_Status(StatusMessageData_Size);
  
  // file was ok, so save it to registry
  ContractLocalPath(szFile1);
  SetRegistryString(szRegistryStatusFile, szFile1);
  
  fclose(fp);
}

// Create a blank entry (not actually used)
void _init_Status(int num) {
	StatusMessageData[num].key = TEXT("");
	StatusMessageData[num].doStatus = true;
	StatusMessageData[num].doSound = false;
	StatusMessageData[num].sound = TEXT("");
	StatusMessageData[num].delay_ms = 2500;  // 2.5 s
}


//////////////////////////
// Registry file handling
/////////////////

const static int nMaxValueNameSize = MAX_PATH + 6; //255 + 1 + /r/n
const static int nMaxValueValueSize = MAX_PATH * 2 + 6; // max regkey name is 256 chars + " = " 
const static int nMaxClassSize = MAX_PATH + 6;
const static int nMaxKeyNameSize = MAX_PATH + 6;

static bool LoadRegistryFromFile_inner(const TCHAR *szFile, bool wide=true)
{
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

TCHAR* StringMallocParse(TCHAR* old_string) {
  TCHAR buffer[2048];	// Note - max size of any string we cope with here !
  TCHAR* new_string;
  unsigned int used = 0;
  unsigned int i;
  for (i = 0; i < _tcslen(old_string); i++) {
    if (used < 2045) {
      if (old_string[i] == '\\' ) {
        if (old_string[i + 1] == 'r') {
          buffer[used++] = '\r';
          i++;
        } else if (old_string[i + 1] == 'n') { 
          buffer[used++] = '\n';
          i++;
        } else if (old_string[i + 1] == '\\') {
          buffer[used++] = '\\';
          i++;
        } else {
          buffer[used++] = old_string[i];
        }
      } else {
	buffer[used++] = old_string[i];
      }
    }
  };
  buffer[used++] =_T('\0');
  
  new_string = (TCHAR *)malloc((_tcslen(buffer)+1)*sizeof(TCHAR));
  _tcscpy(new_string, buffer);
  
  return new_string;
}

//
//	Get local My Documents path - optionally include file to add and location
//	loc = CSIDL_PROGRAMS
//	File system directory that contains the user's program groups (which are also file system directories).
//	CSIDL_PERSONAL               File system directory that serves as a common
//	                             repository for documents.
//	CSIDL_PROGRAM_FILES 0x0026   The program files folder.
//
void LocalPath(TCHAR* buffer, const TCHAR* file, int loc) {

  #if (!defined(WINDOWSPC) || (WINDOWSPC <=0) )

  // For PNAs the localpath is taken from the application exec path
  // example> \sdmmc\bin\Program.exe  results in localpath=\sdmmc\LK8000
  // Then the basename is searched for an underscore char, which is
  // used as a separator for getting the model type.  example>
  // program_pna.exe results in GlobalModelType=pna
  
	_stprintf(buffer,TEXT("%s%S"),gmfpathname(), XCSDATADIR );
  #else
	// get the MyDocuments directory
	SHGetSpecialFolderPath(hWndMainWindow, buffer, loc, false);

	_tcscat(buffer,TEXT("\\"));
	_tcscat(buffer,TEXT(XCSDATADIR));
  #endif

  if (_tcslen(file)>0) {
	_tcsncat(buffer, TEXT("\\"), MAX_PATH);    
	_tcsncat(buffer, file, MAX_PATH);
  }
}


void LocalPathS(char *buffer, const TCHAR* file, int loc) {
  TCHAR wbuffer[MAX_PATH];
  LocalPath(wbuffer,file,loc);
  sprintf(buffer,"%S",wbuffer);
}


void ExpandLocalPath(TCHAR* filein) {
  // Convert %LOCALPATH% to Local Path

  if (_tcslen(filein)==0) {
    return;
  }

  TCHAR lpath[MAX_PATH];
  TCHAR code[] = TEXT("%LOCAL_PATH%\\");
  TCHAR output[MAX_PATH];
  LocalPath(lpath);

  TCHAR* ptr;
  ptr = _tcsstr(filein, code);
  if (!ptr) return;

  ptr += _tcslen(code);
  if (_tcslen(ptr)>0) {
    _stprintf(output,TEXT("%s%s"),lpath, ptr);
    _tcscpy(filein, output);
  }
}


void ContractLocalPath(TCHAR* filein) {
  // Convert Local Path part to %LOCALPATH%

  if (_tcslen(filein)==0) {
    return;
  }

  TCHAR lpath[MAX_PATH];
  TCHAR code[] = TEXT("%LOCAL_PATH%\\");
  TCHAR output[MAX_PATH];
  LocalPath(lpath);

  TCHAR* ptr;
  ptr = _tcsstr(filein, lpath);
  if (!ptr) return;

  ptr += _tcslen(lpath);
  if (_tcslen(ptr)>0) {
    _stprintf(output,TEXT("%s%s"),code, ptr);
    _tcscpy(filein, output);
  }
}


void ConvertTToC(CHAR* pszDest, const TCHAR* pszSrc)
{
	for(unsigned int i = 0; i < _tcslen(pszSrc); i++)
		pszDest[i] = (CHAR) pszSrc[i];
}

void ConvertCToT(TCHAR* pszDest, const CHAR* pszSrc)
{
	for(unsigned int i = 0; i < strlen(pszSrc); i++)
		pszDest[i] = (TCHAR) pszSrc[i];
}


void propGetFontSettingsFromString(TCHAR *Buffer1, LOGFONT* lplf)
{
#define propGetFontSettingsMAX_SIZE 128
  TCHAR Buffer[propGetFontSettingsMAX_SIZE]; // RLD need a buffer (not sz) for strtok_r w/ gcc optimized ARM920

  TCHAR *pWClast, *pToken;
  LOGFONT lfTmp;
  _tcsncpy(Buffer, Buffer1, propGetFontSettingsMAX_SIZE);
    // FontDescription of format:
    // typical font entry
    // 26,0,0,0,700,1,0,0,0,0,0,4,2,<fontname>

    //FW_THIN   100
    //FW_NORMAL 400
    //FW_MEDIUM 500
    //FW_BOLD   700
    //FW_HEAVY  900

  ASSERT(lplf != NULL);
  memset ((void *)&lfTmp, 0, sizeof (LOGFONT));

  if ((pToken = strtok_r(Buffer, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfHeight = _tcstol(pToken, NULL, 10);
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfWidth = _tcstol(pToken, NULL, 10);
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfEscapement = _tcstol(pToken, NULL, 10);
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfOrientation = _tcstol(pToken, NULL, 10);
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfWeight = _tcstol(pToken, NULL, 10);
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfItalic = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfUnderline = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfStrikeOut = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfCharSet = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfOutPrecision = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfClipPrecision = (unsigned char)_tcstol(pToken, NULL, 10);

  // DEFAULT_QUALITY			   0
  // RASTER_FONTTYPE			   0x0001
  // DRAFT_QUALITY			     1
  // NONANTIALIASED_QUALITY  3
  // ANTIALIASED_QUALITY     4
  // CLEARTYPE_QUALITY       5
  // CLEARTYPE_COMPAT_QUALITY 6

  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfQuality = (unsigned char)_tcstol(pToken, NULL, 10); 

  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfPitchAndFamily = (unsigned char)_tcstol(pToken, NULL, 10);

  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL) return;

  _tcscpy(lfTmp.lfFaceName, pToken);

  memcpy((void *)lplf, (void *)&lfTmp, sizeof (LOGFONT));

  return;
}


void propGetFontSettings(TCHAR *Name, LOGFONT* lplf) {

  TCHAR Buffer[128];

  ASSERT(Name != NULL);
  ASSERT(Name[0] != '\0');
  ASSERT(lplf != NULL);

#if (WINDOWSPC>0) 
  // Don't load font settings from registry values for windows version
  return; // TODO FIX let font settings be read by windows PC also! 091021 
#endif

  if (GetRegistryString(Name, Buffer, sizeof(Buffer)/sizeof(TCHAR)) == 0) {
    propGetFontSettingsFromString(Buffer, lplf);
  }
}


int propGetScaleList(double *List, size_t Size){

  TCHAR Buffer[128];
  TCHAR Name[] = TEXT("ScaleList");
  TCHAR *pWClast, *pToken;
  int   Idx = 0;
  double vlast=0;
  double val;

  ASSERT(List != NULL);
  ASSERT(Size > 0);

  SetRegistryString(TEXT("ScaleList"),
   TEXT("0.5,1,2,5,10,20,50,100,150,200,500,1000"));

  if (GetRegistryString(Name, Buffer, sizeof(Buffer)/sizeof(TCHAR)) == 0){

    pToken = strtok_r(Buffer, TEXT(","), &pWClast);
    
    while(Idx < (int)Size && pToken != NULL){
      val = _tcstod(pToken, NULL);
      if (Idx>0) {
        List[Idx] = (val+vlast)/2;
        Idx++;
      }
      List[Idx] = val;
      Idx++;
      vlast = val;
      pToken = strtok_r(NULL, TEXT(","), &pWClast);
    }
    
    return(Idx);
    
  } else {
    return(0);
  }
  
}

long GetUTCOffset(void) {
  return UTCOffset;
}


int TextToLineOffsets(TCHAR* text, int* LineOffsets, int maxLines) {
  int nTextLines=0;
  LineOffsets[0]= 0;
  if (text) {
    if (_tcslen(text)>0) {

      int delta = 0;
      int cumul = 0;
      TCHAR* vind = text;

      while (nTextLines<maxLines) {
	delta = _tcscspn(vind+cumul, TEXT("\n"));
	if (!delta) {
	  break;
	}
	if (_tcslen(vind+cumul+delta)>0) {
	  delta++;
	} else {
	  break;
	}
	cumul += delta;
	nTextLines++;
	LineOffsets[nTextLines]= cumul;
      }
      nTextLines++;

    }
  }
  return nTextLines;
}



TCHAR startProfileFile[MAX_PATH];
TCHAR defaultProfileFile[MAX_PATH];
TCHAR failsafeProfileFile[MAX_PATH];

void RestoreRegistry(void) {
  StartupStore(TEXT(". Restore registry%s"),NEWLINE);
  // load registry backup if it exists
  LoadRegistryFromFile(failsafeProfileFile);
  LoadRegistryFromFile(startProfileFile);
}

void StoreRegistry(void) {
  StartupStore(TEXT(". Store registry%s"),NEWLINE);
  // save registry backup first (try a few places)
  if (!CheckClubVersion())
	SaveRegistryToFile(startProfileFile);
  SaveRegistryToFile(defaultProfileFile);
}

void LK8000GetOpts(LPTSTR CommandLine) {
  (void)CommandLine;

//  LocalPath(defaultProfileFile,TEXT(XCSPROFILE));  
//  LocalPath(failsafeProfileFile,TEXT(XCSPROFILE));  // CHECK, STRANO
//  _tcscpy(startProfileFile, defaultProfileFile);

/*
  091101 this approach does NOT work!
  TCHAR ubuffer[LKSIZEBUFFERPATH];
  _stprintf(ubuffer,_T("%s\\%s%s"),LKD_LOGS,XCSPROFILE,NEWLINE);
  StartupStore(ubuffer);
*/

  TCHAR buffer[MAX_PATH];
#if (!defined(WINDOWSPC) || (WINDOWSPC <=0) )
  LocalPath(buffer,TEXT(LKD_CONF));
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,_T(XCSPROFILE)); // 091101

#else
  SHGetSpecialFolderPath(hWndMainWindow, buffer, CSIDL_PERSONAL, false);
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,TEXT(XCSDATADIR));
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,TEXT(LKD_CONF)); // 091101
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,_T(XCSPROFILE)); // 091101
#endif
  _tcscpy(defaultProfileFile,buffer);
  _tcscpy(failsafeProfileFile, defaultProfileFile);
  _tcscpy(startProfileFile, defaultProfileFile);

#if (WINDOWSPC>0) 
  SCREENWIDTH=800;
  SCREENHEIGHT=480;

#if defined(SCREENWIDTH_)
  SCREENWIDTH=SCREENWIDTH_;
#endif
#if defined(SCREENHEIGHT_)
  SCREENHEIGHT=SCREENHEIGHT_;
#endif

#else
  return; // don't do anything for PDA platforms
#endif

  TCHAR *MyCommandLine = GetCommandLine();

  if (MyCommandLine != NULL){
    TCHAR *pC, *pCe;

    pC = _tcsstr(MyCommandLine, TEXT("-profile="));
    if (pC != NULL){
      pC += strlen("-profile=");
      if (*pC == '"'){
        pC++;
        pCe = pC;
        while (*pCe != '"' && *pCe != '\0') pCe++;
      } else{
        pCe = pC;
        while (*pCe != ' ' && *pCe != '\0') pCe++;
      }
      if (pCe != NULL && pCe-1 > pC){

        _tcsncpy(startProfileFile, pC, pCe-pC);
        startProfileFile[pCe-pC] = '\0';
      }
    }
#if (WINDOWSPC>0) 
    pC = _tcsstr(MyCommandLine, TEXT("-640x480"));
    if (pC != NULL){
      SCREENWIDTH=640;
      SCREENHEIGHT=480;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-800x480"));
    if (pC != NULL){
      SCREENWIDTH=800;
      SCREENHEIGHT=480;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-720x408"));
    if (pC != NULL){
      SCREENWIDTH=720;
      SCREENHEIGHT=408;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-800x600"));
    if (pC != NULL){
      SCREENWIDTH=800;
      SCREENHEIGHT=600;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-896x672"));
    if (pC != NULL){
      SCREENWIDTH=896;
      SCREENHEIGHT=672;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-big"));
    if (pC != NULL){
      SCREENWIDTH=896;
      SCREENHEIGHT=672;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-400x240"));
    if (pC != NULL){
      SCREENWIDTH=400;
      SCREENHEIGHT=240;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x272"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=272;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x234"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=234;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x800"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=800;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-portrait"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=640;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x640"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=640;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-small"));
    if (pC != NULL){
      SCREENWIDTH/= 2;
      SCREENHEIGHT/= 2;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-320x240"));
    if (pC != NULL){
      SCREENWIDTH=320;
      SCREENHEIGHT=240;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-320x234"));
    if (pC != NULL){
      SCREENWIDTH=320;
      SCREENHEIGHT=234;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-240x320"));
    if (pC != NULL){
      SCREENWIDTH=240;
      SCREENHEIGHT=320;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-234x320"));
    if (pC != NULL){
      SCREENWIDTH=234;
      SCREENHEIGHT=320;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-240x400"));
    if (pC != NULL){
      SCREENWIDTH=240;
      SCREENHEIGHT=400;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-272x480"));
    if (pC != NULL){
      SCREENWIDTH=272;
      SCREENHEIGHT=480;
    }

#endif
  }
}


#if TOPOFASTLABEL
bool CheckRectOverlap(const RECT *rc1, const RECT *rc2) {
  if(rc1->left >= rc2->right) return(false);
  if(rc1->right <= rc2->left) return(false);
  if(rc1->top >= rc2->bottom) return(false);
  if(rc1->bottom <= rc2->top) return(false);
  return(true);
}
#else
bool CheckRectOverlap(RECT rc1, RECT rc2) {
  if(rc1.left >= rc2.right) return(false);
  if(rc1.right <= rc2.left) return(false);
  if(rc1.top >= rc2.bottom) return(false);
  if(rc1.bottom <= rc2.top) return(false);
  return(true);
}
#endif


#if (WINDOWSPC>0)
typedef DWORD (_stdcall *GetIdleTimeProc) (void);
GetIdleTimeProc GetIdleTime;
#endif

#if DEBUG_MEM
int MeasureCPULoad() {
#if (WINDOWSPC>0) && !defined(__MINGW32__)
  static bool init=false;
  if (!init) {
    // get the pointer to the function
    GetIdleTime = (GetIdleTimeProc) 
      GetProcAddress(LoadLibrary(_T("coredll.dll")),
		     _T("GetIdleTime"));
    init=true;
  }
  if (!GetIdleTime) return 0;
#endif

  static int pi;
  static int PercentIdle;
  static int PercentLoad;
  static bool start=true;
  static DWORD dwStartTick;
  static DWORD dwIdleSt;
  static DWORD dwStopTick;
  static DWORD dwIdleEd;
  if (start) {
    dwStartTick = GetTickCount();
    dwIdleSt = GetIdleTime();
  }
  if (!start) {
    dwStopTick = GetTickCount();
    dwIdleEd = GetIdleTime();
    pi = ((100 * (dwIdleEd - dwIdleSt))/(dwStopTick - dwStartTick));
    PercentIdle = (PercentIdle+pi)/2;
  }
  start = !start;
  PercentLoad = 100-PercentIdle;
  return PercentLoad;
}
#endif



typedef struct WinPilotPolarInternal {
  TCHAR name[50];
  double ww0;
  double ww1;
  double v0;
  double w0;
  double v1;
  double w1;
  double v2;
  double w2;
  double wing_area;
} WinPilotPolarInternal;


// REMEMBER: add new polars at the bottom, or old configuration will get a different polar value
// Also remember, currently 300 items limit in WindowControls.h DFE_  enums.
// THIS IS NOW UNUSED
WinPilotPolarInternal WinPilotPolars[] = 
{
  {TEXT("ERROR"), 670,100,100,-1.29,120,-1.61,150,-2.45,15.3},
  }; //   0-x

TCHAR* GetWinPilotPolarInternalName(int i) {
  if ( (unsigned) i >= (sizeof(WinPilotPolars)/sizeof(WinPilotPolarInternal)) ) {
    return NULL; // error
  }
  return WinPilotPolars[i].name;
}

bool ReadWinPilotPolarInternal(int i) {
  double POLARV[3];
  double POLARW[3];
  double ww[2];

  if (!(i < (int)(sizeof(WinPilotPolars) / sizeof(WinPilotPolars[0])))) { // 100207 fix signed unsigned (int)
	return(FALSE);
  }

  // 0 is glider+pilot,  1 is ballast
  ww[0] = WinPilotPolars[i].ww0;
  ww[1] = WinPilotPolars[i].ww1;
  POLARV[0] = WinPilotPolars[i].v0;
  POLARV[1] = WinPilotPolars[i].v1;
  POLARV[2] = WinPilotPolars[i].v2;
  POLARW[0] = WinPilotPolars[i].w0;
  POLARW[1] = WinPilotPolars[i].w1;
  POLARW[2] = WinPilotPolars[i].w2;
  PolarWinPilot2XCSoar(POLARV, POLARW, ww);
  GlidePolar::WingArea = WinPilotPolars[i].wing_area;

  return(TRUE);

}

#if (WINDOWSPC<1)
#define GdiFlush() do { } while (0)
#endif


#if 0 // REMOVE ANIMATION
static RECT AnimationRectangle = {0,0,0,0};

void SetSourceRectangle(RECT fromRect) {
  AnimationRectangle = fromRect;
}


RECT WINAPI DrawWireRects(LPRECT lprcTo, UINT nMilliSecSpeed)
{
    return AnimationRectangle;
}
#endif // REMOVE

// FLARM FUNCTIONS

int NumberOfFLARMNames = 0;

typedef struct {
  long ID;
  TCHAR Name[MAXFLARMNAME+1]; // 100322 was 20
} FLARM_Names_t;


FLARM_Names_t FLARM_Names[MAXFLARMLOCALS+1];

void CloseFLARMDetails() {
  int i;
  for (i=0; i<NumberOfFLARMNames; i++) {
    //    free(FLARM_Names[i]);
  }
  NumberOfFLARMNames = 0;
}

void OpenFLARMDetails() {

  StartupStore(_T(". FlarmNet ids found: %d%s"),FlarmNetCount,NEWLINE);

  StartupStore(TEXT(". OpenFLARMDetails: \""));

  if (NumberOfFLARMNames) {
    CloseFLARMDetails(); // BUGFIX TESTFIX 091020 attempt NOT to reset flarmnet preloaded list 100321 NO
  }

  TCHAR filename[MAX_PATH];
  LocalPath(filename,TEXT(LKD_CONF)); // 091103
  _tcscat(filename,_T("\\"));
  _tcscat(filename,_T(LKF_FLARMIDS));
  
  StartupStore(filename);
  StartupStore(_T("\"%s"),NEWLINE);
  HANDLE hFile = CreateFile(filename,GENERIC_READ,0,NULL,
			    OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if( hFile == INVALID_HANDLE_VALUE) {
	StartupStore(_T("... No flarm details local file found%s"),NEWLINE);
	return;
  }

  TCHAR line[READLINE_LENGTH];
  while (ReadString(hFile,READLINE_LENGTH, line)) {
    long id;
    TCHAR Name[MAX_PATH];

    if (_stscanf(line, TEXT("%lx=%s"), &id, Name) == 2) {
      if (AddFlarmLookupItem(id, Name, false) == false)
	{
	  break; // cant add anymore items !
	}
    }
  }

  _stprintf(filename,_T(". Local Flarm ids found=%d%s"),NumberOfFLARMNames,NEWLINE);
  StartupStore(filename);

  CloseHandle(hFile);

}


void SaveFLARMDetails(void)
{
  DWORD bytesWritten;
  TCHAR filename[MAX_PATH];
  LocalPath(filename,TEXT(LKD_CONF)); // 091103
  _tcscat(filename,_T("\\"));
  _tcscat(filename,_T(LKF_FLARMIDS));
  
  HANDLE hFile = CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if( hFile == INVALID_HANDLE_VALUE) {
	// StartupStore(_T("-- Cannot save FLARM details, error --\n"));
	return;
  }
  
  TCHAR wsline[READLINE_LENGTH];
  char cline[READLINE_LENGTH];
  
  for (int z = 0; z < NumberOfFLARMNames; z++)
    {   
      wsprintf(wsline, TEXT("%lx=%s\r\n"), FLARM_Names[z].ID,FLARM_Names[z].Name);
      
      WideCharToMultiByte( CP_ACP, 0, wsline,
			   _tcslen(wsline)+1,
			   cline,
			   READLINE_LENGTH, NULL, NULL);
      
      WriteFile(hFile, cline, strlen(cline), &bytesWritten, NULL);
    }
  _stprintf(filename,_T("... Saved %d FLARM names%s"),NumberOfFLARMNames,NEWLINE);
  StartupStore(filename);
  CloseHandle(hFile);
}


int LookupSecondaryFLARMId(int id)
{
  for (int i=0; i<NumberOfFLARMNames; i++) 
    {
      if (FLARM_Names[i].ID == id) 
	{
	  return i;
	}
    }
  return -1;
}

int LookupSecondaryFLARMId(TCHAR *cn)
{
  for (int i=0; i<NumberOfFLARMNames; i++) 
    {
      if (wcscmp(FLARM_Names[i].Name, cn) == 0) 
	{
	  return i;
	}
    }
  return -1;
}

// returns Name or Cn to be used
TCHAR* LookupFLARMCn(long id) {
  
  // try to find flarm from userFile
  int index = LookupSecondaryFLARMId(id);
  if (index != -1)
    {
      return FLARM_Names[index].Name;
    }
  
  // try to find flarm from FLARMNet.org File
  FlarmId* flarmId = file.GetFlarmIdItem(id);
  if (flarmId != NULL)
    {
      return flarmId->cn;
    }
  return NULL;
}

TCHAR* LookupFLARMDetails(long id) {
  
  // try to find flarm from userFile
  int index = LookupSecondaryFLARMId(id);
  if (index != -1)
    {
      return FLARM_Names[index].Name;
    }
  
  // try to find flarm from FLARMNet.org File
  FlarmId* flarmId = file.GetFlarmIdItem(id);
  if (flarmId != NULL)
    {
      // return flarmId->cn;
      return flarmId->reg;
    }
  return NULL;
}

// Used by TeamCode, to select a CN and get back the Id
int LookupFLARMDetails(TCHAR *cn) 
{
  // try to find flarm from userFile
  int index = LookupSecondaryFLARMId(cn);
  if (index != -1)
    {
      return FLARM_Names[index].ID;
    }
  
  // try to find flarm from FLARMNet.org File
  FlarmId* flarmId = file.GetFlarmIdItem(cn);
  if (flarmId != NULL)
    {
      return flarmId->GetId();
    }
  return 0;
}


bool AddFlarmLookupItem(int id, TCHAR *name, bool saveFile)
{
  int index = LookupSecondaryFLARMId(id);

  #ifdef DEBUG_LKT
  StartupStore(_T("... LookupSecondary id=%d result index=%d\n"),id,index);
  #endif
  if (index == -1)
    {
      if (NumberOfFLARMNames < MAXFLARMLOCALS)  // 100322 
	{
	  // create new record
	  FLARM_Names[NumberOfFLARMNames].ID = id;
	  _tcsncpy(FLARM_Names[NumberOfFLARMNames].Name, name,MAXFLARMNAME); // was 20 100322
	  FLARM_Names[NumberOfFLARMNames].Name[MAXFLARMNAME]=0; // was 20
	  NumberOfFLARMNames++;
	  SaveFLARMDetails();
	  return true;
	}
    }
  else
    {
      // modify existing record
      FLARM_Names[index].ID = id;
      _tcsncpy(FLARM_Names[index].Name, name,MAXFLARMNAME);
      FLARM_Names[index].Name[MAXFLARMNAME]=0;	
      if (saveFile)
	{
	  SaveFLARMDetails();
	}
      return true;
    }
  return false;
}



double QNHAltitudeToStaticPressure(double alt) {
  // http://wahiduddin.net/calc/density_altitude.htm
  const double k1=0.190263;
  const double k2=8.417286e-5;
  return 100.0*pow((pow(QNH,k1)-k2*alt),1.0/k1);
  // example, alt= 100, QNH=1014
  // ps = 100203 Pa
}


double StaticPressureToAltitude(double ps) {
  // http://wahiduddin.net/calc/density_altitude.htm
  const double k1=0.190263;
  const double k2=8.417286e-5;
  return (pow(QNH,k1) - pow(ps/100.0, k1))/k2;
  // example, QNH=1014, ps=100203
  // alt= 100
}


// Converts altitude with QNH=1013.25 reference to QNH adjusted altitude
double AltitudeToQNHAltitude(double alt) {
  const double k1=0.190263;
  double ps = pow((44330.8-alt)/4946.54,1.0/k1);
  return StaticPressureToAltitude(ps);
}


double FindQNH(double alt_raw, double alt_known) {
  // find QNH so that the static pressure gives the specified altitude
  // (altitude can come from GPS or known airfield altitude or terrain
  // height on ground)

  // This function assumes the barometric altitude (alt_raw) is 
  // already adjusted for QNH ---> the function returns the
  // QNH value to make the barometric altitude equal to the
  // alt_known value.

  const double k1=0.190263;
  const double k2=8.417286e-5;

  // step 1, find static pressure from device assuming it's QNH adjusted
  double psraw = QNHAltitudeToStaticPressure(alt_raw);
  // step 2, calculate QNH so that reported alt will be known alt 
  return pow(pow(psraw/100.0,k1) + k2*alt_known,1/k1);

  // example, QNH=1014, ps=100203
  // alt= 100
  // alt_known = 120
  // qnh= 1016
}



double AirDensity(double altitude) {
  double rho = pow((44330.8-altitude)/42266.5,1.0/0.234969);
  return rho;
}


// divide TAS by this number to get IAS
double AirDensityRatio(double altitude) {
  double rho = AirDensity(altitude);
  double rho_rat = sqrt(1.225/rho);
  return rho_rat;
}


unsigned long CheckFreeRam(void) {
  MEMORYSTATUS    memInfo;
  // Program memory
  memInfo.dwLength = sizeof(memInfo);
  GlobalMemoryStatus(&memInfo);

  //	   memInfo.dwTotalPhys, 
  //	   memInfo.dwAvailPhys, 
  //	   memInfo.dwTotalPhys- memInfo.dwAvailPhys);

  return memInfo.dwAvailPhys;
}

// check maximum allocatable heap block
unsigned long CheckMaxHeapBlock(void) {
  #if defined(HC_DMALLOC) ||  defined(HC_DUMA)
    // when using heap checker, do not try allocate maximum size - malloc() can
    // return NULL which heap checker recognizes as an error and will terminate
    // program immediately when configured so (can be confusing for developer)
    return(0xFFFFFFFF);
  #else  
    // try allocate maximum block (of course on PC with disk swapping, we will not
    // try maximum block, function just returns something near to initial top value)
    size_t top = 100*1024*1024; // start with 100MB/2
    size_t btm = 0;
    
    void*  addr;
    size_t size;
    
    while ((size = (btm + top) / 2) != 0) { // ~ btm + (top - btm) / 2
      addr = malloc(size);
      if (addr == NULL)
        top = size;
      else {
        free(addr);
        if ((top - btm) < 1024) // 1 KB accuracy
          return(size);
        btm = size;
      }
    }
    
    return(0);
  #endif
}


#if (WINDOWSPC>0)
#if _DEBUG
_CrtMemState memstate_s1;
#endif
#endif

void MemCheckPoint()
{
#if (WINDOWSPC>0)
#if _DEBUG
  _CrtMemCheckpoint( &memstate_s1 );
#endif
#endif
}


void MemLeakCheck() {
#if (WINDOWSPC>0)
#if _DEBUG
  _CrtMemState memstate_s2, memstate_s3;

   // Store a 2nd memory checkpoint in s2
   _CrtMemCheckpoint( &memstate_s2 );

   if ( _CrtMemDifference( &memstate_s3, &memstate_s1, &memstate_s2 ) ) {
     _CrtMemDumpStatistics( &memstate_s3 );
     _CrtMemDumpAllObjectsSince(&memstate_s1);
   }

  _CrtCheckMemory();
#endif
#endif
}


// This is necessary to be called periodically to get rid of 
// memory defragmentation, since on pocket pc platforms there is no
// automatic defragmentation.
void MyCompactHeaps() {
#if (WINDOWSPC>0)
  HeapCompact(GetProcessHeap(),0);
#else
  typedef DWORD (_stdcall *CompactAllHeapsFn) (void);
  static CompactAllHeapsFn CompactAllHeaps = NULL;
  static bool init=false;
  if (!init) {
    // get the pointer to the function
    CompactAllHeaps = (CompactAllHeapsFn) 
      GetProcAddress(LoadLibrary(_T("coredll.dll")),
		     _T("CompactAllHeaps"));
    init=true;
  }
  if (CompactAllHeaps) {
    CompactAllHeaps();
  }
#endif
}


unsigned long FindFreeSpace(const TCHAR *path) {
  // returns number of kb free on destination drive

  ULARGE_INTEGER FreeBytesAvailableToCaller;
  ULARGE_INTEGER TotalNumberOfBytes;
  ULARGE_INTEGER TotalNumberOfFreeBytes;
  if (GetDiskFreeSpaceEx(path, 
			 &FreeBytesAvailableToCaller,
			 &TotalNumberOfBytes,
			 &TotalNumberOfFreeBytes)) {
    return FreeBytesAvailableToCaller.LowPart/1024;
  } else {
    return 0;
  }
}


bool MatchesExtension(const TCHAR *filename, const TCHAR* extension) {
  TCHAR *ptr;
  ptr = _tcsstr((TCHAR*)filename, extension);
  if (ptr != filename+_tcslen(filename)-_tcslen(extension)) {
    return false;
  } else {
    return true;
  }
}


#include "mmsystem.h"

extern HINSTANCE                       hInst; // The current instance

BOOL PlayResource (const TCHAR* lpName)
{
#ifdef DISABLEAUDIO
  return false;
#else
  #ifdef PNA
  if (DeviceIsGM130) {
	MessageBeep(0xffffffff);
	return true;
  }
  #endif
  BOOL bRtn;
  LPTSTR lpRes;
  HANDLE hResInfo, hRes;

  // TODO code: Modify to allow use of WAV Files and/or Embedded files

  if (_tcsstr(lpName, TEXT(".wav"))) {
    bRtn = sndPlaySound (lpName, SND_ASYNC | SND_NODEFAULT ); 

  } else {
    
    // Find the wave resource.
    hResInfo = FindResource (hInst, lpName, TEXT("WAVE")); 
    
    if (hResInfo == NULL) 
      return FALSE; 
    
    // Load the wave resource. 
    hRes = LoadResource (hInst, (HRSRC)hResInfo); 
    
    if (hRes == NULL) 
      return FALSE; 
    
    // Lock the wave resource and play it. 
    lpRes = (LPTSTR)LockResource ((HGLOBAL)hRes);
    
    if (lpRes != NULL) 
      { 
	bRtn = sndPlaySound (lpRes, SND_MEMORY | SND_ASYNC | SND_NODEFAULT ); 
      } 
    else 
      bRtn = 0;
  }
  return bRtn; 
#endif
}

void CreateDirectoryIfAbsent(TCHAR *filename) {
  TCHAR fullname[MAX_PATH];

  LocalPath(fullname, filename);

  DWORD fattr = GetFileAttributes(fullname);

  if ((fattr != 0xFFFFFFFF) &&
      (fattr & FILE_ATTRIBUTE_DIRECTORY)) {
    // directory exists
  } else {
    CreateDirectory(fullname, NULL);
  }

}

static int interface_timeout;

bool InterfaceTimeoutZero(void) {
  return (interface_timeout==0);
}

void InterfaceTimeoutReset(void) {
  interface_timeout = 0;
}


bool InterfaceTimeoutCheck(void) {
  if (interface_timeout > 60*10) {
    interface_timeout = 0;
    return true;
  } else {
    interface_timeout++;
    return false;
  }
}

bool FileExists(TCHAR *FileName){

  HANDLE hFile = CreateFileW(FileName, GENERIC_READ, 0, NULL,
                 OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

  if( hFile == INVALID_HANDLE_VALUE)
    return(FALSE);

  CloseHandle(hFile);

  return(TRUE);
  
  /*
  FILE *file = _tfopen(FileName, _T("r"));
  if (file != NULL) {
    fclose(file);
    return(TRUE);
  }
  return FALSE;
  */
}

bool RotateScreen() {
#if (WINDOWSPC>0)
  return false;
#else 
  //
  // Change the orientation of the screen
  //
#if 0
  DEVMODE DeviceMode;
    
  memset(&DeviceMode, 0, sizeof(DeviceMode));
  DeviceMode.dmSize=sizeof(DeviceMode);
  DeviceMode.dmFields = DM_DISPLAYORIENTATION;
  DeviceMode.dmDisplayOrientation = DMDO_90; 
  //Put your desired position right here.

  if (DISP_CHANGE_SUCCESSFUL == 
      ChangeDisplaySettingsEx(NULL, &DeviceMode, NULL, CDS_RESET, NULL))
    return true;
  else
    return false;
#else
  return false;
#endif
#endif

}


int GetTextWidth(HDC hDC, TCHAR *text) {
  SIZE tsize;
  GetTextExtentPoint(hDC, text, _tcslen(text), &tsize);
  return tsize.cx;
}


void ExtTextOutClip(HDC hDC, int x, int y, TCHAR *text, int width) {
  int len = _tcslen(text);
  if (len <=0 ) {
    return;
  }
  SIZE tsize;
  GetTextExtentPoint(hDC, text, len, &tsize);
  RECT rc;
  rc.left = x;
  rc.top = y;
  rc.right = x + min(width,tsize.cx);
  rc.bottom = y + tsize.cy;

  ExtTextOut(hDC, x, y, /* ETO_OPAQUE | */ ETO_CLIPPED, &rc,
             text, len, NULL);
}

void UpdateConfBB(void) {

  ConfBB[0]=true; // thermal always on automatically
  ConfBB[1]=ConfBB1;
  ConfBB[2]=ConfBB2;
  ConfBB[3]=ConfBB3;
  ConfBB[4]=ConfBB4;
  ConfBB[5]=ConfBB5;
  ConfBB[6]=ConfBB6;
  ConfBB[7]=ConfBB7;
  ConfBB[8]=ConfBB8;
  ConfBB[9]=ConfBB9;

  if (ConfBB2==false && ConfBB3==false &&
      ConfBB4==false && ConfBB5==false &&
      ConfBB6==false && ConfBB7==false &&
      ConfBB8==false && ConfBB9==false)

		// we need at least one bottom bar stripe available (thermal apart)
		ConfBB[1]=true;

}

void UpdateConfIP(void) {

  // MAP MODE always available
  ConfIP[0][0]=true; 
  ConfIP[0][1]=true; 
  ConfMP[0]=true; // map mode

  // LKMODE_INFOMODE is 1
  ConfIP[1][0]=ConfIP11;
  ConfIP[1][1]=ConfIP12;
  ConfIP[1][2]=ConfIP13;
  ConfIP[1][3]=ConfIP14;
  ConfIP[1][4]=ConfIP15;
  ConfIP[1][5]=ConfIP16;

  // WPMODE
  ConfIP[2][0]=ConfIP21;
  ConfIP[2][1]=ConfIP22;
  ConfIP[2][2]=ConfIP23;
  ConfIP[2][3]=ConfIP24;

  // COMMONS
  ConfIP[3][0]=ConfIP31;
  ConfIP[3][1]=ConfIP32;
  ConfIP[3][2]=ConfIP33;

  // TRAFFIC always on if available
  ConfIP[4][0]=true;
  ConfIP[4][1]=true;
  ConfIP[4][2]=true;
  ConfMP[4]=true; // traffic mode

  // Check if we have INFOMODE
  if (ConfIP[1][0]==false && ConfIP[1][1]==false 
	&& ConfIP[1][2]==false && ConfIP[1][3]==false 
	&& ConfIP[1][4]==false && ConfIP[1][5]==false) {
	ConfMP[1]=false;
  } else
	ConfMP[1]=true;

  // Check if we have NEAREST pages
  if (ConfIP[2][0]==false && ConfIP[2][1]==false 
	&& ConfIP[2][2]==false && ConfIP[2][3]==false ) {
	ConfMP[2]=false;
  } else
	ConfMP[2]=true;

  // Check if we have COMMONS
  if (ConfIP[3][0]==false && ConfIP[3][1]==false && ConfIP[3][2]==false ) {
	ConfMP[3]=false;
  } else
	ConfMP[3]=true;

  /*
  // Verify that we have at least one menu
  if (ConfMP[1]==false && ConfMP[2]==false && ConfMP[3]==false ) {
	ConfIP[1][0]=true;
	ConfMP[1]=true;
  }
  */
  SetInitialModeTypes();

}

void SetInitialModeTypes(void) {

  // Update the initial values for each mapspace, keeping the first valid value. We search backwards.
  // INFOMODE 1  
  if (ConfIP[LKMODE_INFOMODE][IM_TRI]) ModeType[LKMODE_INFOMODE]=IM_TRI;
  if (ConfIP[LKMODE_INFOMODE][IM_CONTEST]) ModeType[LKMODE_INFOMODE]=IM_CONTEST;
  if (ConfIP[LKMODE_INFOMODE][IM_AUX]) ModeType[LKMODE_INFOMODE]=IM_AUX;
  if (ConfIP[LKMODE_INFOMODE][IM_TASK]) ModeType[LKMODE_INFOMODE]=IM_TASK;
  if (ConfIP[LKMODE_INFOMODE][IM_THERMAL]) ModeType[LKMODE_INFOMODE]=IM_THERMAL;
  if (ConfIP[LKMODE_INFOMODE][IM_CRUISE]) ModeType[LKMODE_INFOMODE]=IM_CRUISE;

  // WP NEAREST MODE 2  
  if (ConfIP[LKMODE_WP][WP_NEARTPS]) ModeType[LKMODE_WP]=WP_NEARTPS;
  if (ConfIP[LKMODE_WP][WP_LANDABLE]) ModeType[LKMODE_WP]=WP_LANDABLE;
  if (ConfIP[LKMODE_WP][WP_AIRPORTS]) ModeType[LKMODE_WP]=WP_AIRPORTS;

  // COMMONS MODE 3
  if (ConfIP[LKMODE_NAV][NV_HISTORY]) ModeType[LKMODE_WP]=NV_HISTORY;
  if (ConfIP[LKMODE_NAV][NV_COMMONS]) ModeType[LKMODE_WP]=NV_COMMONS;


}

