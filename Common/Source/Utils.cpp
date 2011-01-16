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
#endif

#include "Defines.h" // VENTA3

#include "Utils.h"

// #include "resource.h"
#include "externs.h"
#include "device.h"
#include "uniqueid.h"
#include "XCSoar.h"
#include "Topology.h"
#include "Terrain.h"
#include "Units.h"
#include "Calculations.h"
#include "GaugeFLARM.h"
#include "McReady.h"
#include "NavFunctions.h"
#include "WaveThread.h"

#ifdef FLARMNET
#include "FlarmIdFile.h"
// Warning, this is initialising class, loading flarmnet IDs before anything else in the LK is even started..
FlarmIdFile file; 
#endif

#if defined(PNA) || defined(FIVV)
#include "InfoBoxLayout.h"
#endif

// JMW not required in newer systems?
#ifdef __MINGW32__
#ifndef max
#define max(x, y)   (x > y ? x : y)
#define min(x, y)   (x < y ? x : y)
#endif
#endif

bool EnableAnimation=false;

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
				  TEXT("Colour14")
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
				  TEXT("Brush14")
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
					       TEXT("AirspaceMode14")
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
					 TEXT("AirspacePriority14")
}; // pL


const TCHAR szRegistryAirspaceWarning[]= TEXT("AirspaceWarn");
const TCHAR szRegistryAirspaceBlackOutline[]= TEXT("AirspaceBlackOutline");
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
const TCHAR szRegistryPolarID[] = TEXT("Polar"); // pL
const TCHAR szRegistryPort1Index[]= TEXT("PortIndex");
const TCHAR szRegistryPort2Index[]= TEXT("Port2Index");
// const TCHAR szRegistryPort3Index[]= TEXT("Port3Index");
const TCHAR szRegistryRegKey[]=				 TEXT("RegKey");
const TCHAR szRegistrySafetyAltitudeArrival[] =     TEXT("SafetyAltitudeArrival");
const TCHAR szRegistrySafetyAltitudeBreakOff[] =     TEXT("SafetyAltitudeBreakOff");
const TCHAR szRegistrySafetyAltitudeTerrain[] =     TEXT("SafetyAltitudeTerrain");
const TCHAR szRegistrySafteySpeed[] =          TEXT("SafteySpeed");
const TCHAR szRegistryWindCalcSpeed[] =          TEXT("WindCalcSpeed");
const TCHAR szRegistryWindCalcTime[] =          TEXT("WindCalcTime");
const TCHAR szRegistrySectorRadius[]=          TEXT("Radius");
const TCHAR szRegistrySnailTrail[]=		 TEXT("SnailTrail");
const TCHAR szRegistryTrailDrift[]=		 TEXT("TrailDrift");
const TCHAR szRegistryThermalLocator[]=	 TEXT("ThermalLocator");
const TCHAR szRegistryAnimation[]=		 TEXT("Animation");
const TCHAR szRegistrySpeed1Index[]=		 TEXT("SpeedIndex");
const TCHAR szRegistrySpeed2Index[]=		 TEXT("Speed2Index");
//const TCHAR szRegistrySpeed3Index[]=		 TEXT("Speed3Index");
const TCHAR szRegistryBit1Index[]=		 TEXT("Bit1Index");
const TCHAR szRegistryBit2Index[]=		 TEXT("Bit2Index");
//const TCHAR szRegistryBit3Index[]=		 TEXT("Bit3Index");
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
const TCHAR szRegistrySoundVolume[]=  TEXT("SoundVolume");
const TCHAR szRegistrySoundDeadband[]=  TEXT("SoundDeadband");
const TCHAR szRegistrySoundAudioVario[]=  TEXT("AudioVario");
const TCHAR szRegistrySoundTask[]=  TEXT("SoundTask");
const TCHAR szRegistrySoundModes[]=  TEXT("SoundModes");
const TCHAR szRegistryNettoSpeed[]= TEXT("NettoSpeed");
const TCHAR szRegistryAccelerometerZero[]= TEXT("AccelerometerZero");
const TCHAR szRegistryCDICruise[]= TEXT("CDICruise");
const TCHAR szRegistryCDICircling[]= TEXT("CDICircling");

const TCHAR szRegistryDeviceA[]= TEXT("DeviceA");
const TCHAR szRegistryDeviceB[]= TEXT("DeviceB");
const TCHAR szRegistryDeviceC[]= TEXT("DeviceC");

const TCHAR szRegistryAutoBlank[]= TEXT("AutoBlank");
const TCHAR szRegistryAutoBacklight[]= TEXT("AutoBacklight");
const TCHAR szRegistryAutoSoundVolume[]= TEXT("AutoSoundVolume");
const TCHAR szRegistryAircraftCategory[]= TEXT("AircraftCategory");
const TCHAR szRegistryExtendedVisualGlide[]= TEXT("ExtVisualGlide");
const TCHAR szRegistryLook8000[]= TEXT("Look8000");
const TCHAR szRegistryAltArrivMode[]= TEXT("AltArrivMode");
const TCHAR szRegistryUseMapLock[]= TEXT("UseMapLock");
const TCHAR szRegistryActiveMap[]= TEXT("ActiveMap"); // 100318
const TCHAR szRegistryBestWarning[]= TEXT("BestWarning");
const TCHAR szRegistryThermalBar[]= TEXT("ThermalBar");
const TCHAR szRegistryMcOverlay[]= TEXT("McOverlay");
const TCHAR szRegistryTrackBar[]= TEXT("TrackBar");
const TCHAR szRegistryNewMap[]= TEXT("NewMap");
const TCHAR szRegistryIphoneGestures[]= TEXT("IphoneGestures");
const TCHAR szRegistryPollingMode[]= TEXT("PollingMode");
const TCHAR szRegistryLKVarioBar[]= TEXT("LKVarioBar");
const TCHAR szRegistryHideUnits[]= TEXT("HideUnits");
const TCHAR szRegistryVirtualKeys[]= TEXT("VirtualKeys");
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
const TCHAR szRegistryVarioGauge[]= TEXT("VarioGauge");

const TCHAR szRegistryDebounceTimeout[]= TEXT("DebounceTimeout");

const TCHAR szRegistryPGCruiseZoom[]= TEXT("PGCruiseZoom");
const TCHAR szRegistryPGClimbZoom[]= TEXT("PGClimbZoom");
#if AUTORIENT
const TCHAR szRegistryAutoOrientScale[]= TEXT("AutoOrientScale");
#endif
const TCHAR szRegistryPGOpenTimeH[]= TEXT("PGOpenTimeH");
const TCHAR szRegistryPGOpenTimeM[]= TEXT("PGOpenTimeM");
const TCHAR szRegistryPGNumberOfGates[]= TEXT("PGNumberOfGates");
#if LKTOPO
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
#endif
const TCHAR szRegistryLKMaxLabels[]= TEXT("LKMaxLabels");
const TCHAR szRegistryOverlaySize[]= TEXT("OverlaySize");
const TCHAR szRegistryPGGateIntervalTime[]= TEXT("PGGateIntervalTime");
const TCHAR szRegistryPGStartOut[]= TEXT("PGStartOut");

const TCHAR szRegistryAppIndFinalGlide[] = TEXT("AppIndFinalGlide");
const TCHAR szRegistryAppIndLandable[] = TEXT("AppIndLandable");
const TCHAR szRegistryAppInverseInfoBox[] = TEXT("AppInverseInfoBox2");
const TCHAR szRegistryAppGaugeVarioSpeedToFly[] = TEXT("AppGaugeVarioSpeedToFly");
const TCHAR szRegistryAppGaugeVarioAvgText[] = TEXT("AppGaugeVarioAvgText");
const TCHAR szRegistryAppGaugeVarioMc[] = TEXT("AppGaugeVarioMc");
const TCHAR szRegistryAppGaugeVarioBugs[] = TEXT("AppGaugeVarioBugs");
const TCHAR szRegistryAppGaugeVarioBallast[] = TEXT("AppGaugeVarioBallast");
const TCHAR szRegistryAppGaugeVarioGross[] = TEXT("AppGaugeVarioGross");
const TCHAR szRegistryAppCompassAppearance[] = TEXT("AppCompassAppearance");
const TCHAR szRegistryAppStatusMessageAlignment[] = TEXT("AppStatusMessageAlignment");
const TCHAR szRegistryAppTextInputStyle[] = TEXT("AppTextInputStyle");
const TCHAR szRegistryAppInfoBoxColors[] = TEXT("AppInfoBoxColors");
const TCHAR szRegistryAppDefaultMapWidth[] = TEXT("AppDefaultMapWidth");
const TCHAR szRegistryTeamcodeRefWaypoint[] = TEXT("TeamcodeRefWaypoint");
const TCHAR szRegistryAppInfoBoxBorder[] = TEXT("AppInfoBoxBorder");

const TCHAR szRegistryAppInfoBoxGeom[] = TEXT("AppInfoBoxGeom");
const TCHAR szRegistryAppInfoBoxModel[] = TEXT("AppInfoBoxModel"); 
const TCHAR szRegistryGpsAltitudeOffset[] = TEXT("GpsAltitudeOffset");
const TCHAR szRegistryUseGeoidSeparation[] = TEXT("UseGeoidSeparation");
const TCHAR szRegistryPressureHg[] = TEXT("PressureHg");
//const TCHAR szRegistryShortcutIbox[] = TEXT("ShortcutIbox");
const TCHAR szRegistryCustomKeyTime[] = TEXT("CustomKeyTime");
const TCHAR szRegistryCustomKeyModeCenter[] = TEXT("CustomKeyModeCenter");
const TCHAR szRegistryCustomKeyModeLeft[] = TEXT("CustomKeyModeLeft");
const TCHAR szRegistryCustomKeyModeRight[] = TEXT("CustomKeyModeRight");
const TCHAR szRegistryCustomKeyModeAircraftIcon[] = TEXT("CustomKeyModeAircraftIcon");
const TCHAR szRegistryCustomKeyModeLeftUpCorner[] = TEXT("CustomKeyModeLeftUpCorner");
const TCHAR szRegistryCustomKeyModeRightUpCorner[] = TEXT("CustomKeyModeRightUpCorner");

const TCHAR szRegistryAppAveNeedle[] = TEXT("AppAveNeedle");

const TCHAR szRegistryAutoAdvance[] = TEXT("AutoAdvance");
const TCHAR szRegistryUTCOffset[] = TEXT("UTCOffset");
const TCHAR szRegistryBlockSTF[] = TEXT("BlockSpeedToFly");
const TCHAR szRegistryAutoZoom[] = TEXT("AutoZoom");
const TCHAR szRegistryMenuTimeout[] = TEXT("MenuTimeout");
const TCHAR szRegistryLockSettingsInFlight[] = TEXT("LockSettingsInFlight");
const TCHAR szRegistryTerrainContrast[] = TEXT("TerrainContrast");
const TCHAR szRegistryTerrainBrightness[] = TEXT("TerrainBrightness");
const TCHAR szRegistryTerrainRamp[] = TEXT("TerrainRamp");
const TCHAR szRegistryEnableFLARMMap[] = TEXT("EnableFLARMDisplay");
const TCHAR szRegistryEnableFLARMGauge[] = TEXT("EnableFLARMGauge");
const TCHAR szRegistryFLARMGaugeBearing[] = TEXT("FLARMGaugeBearing");
const TCHAR szRegistryGliderScreenPosition[] = TEXT("GliderScreenPosition");
const TCHAR szRegistrySetSystemTimeFromGPS[] = TEXT("SetSystemTimeFromGPS");
const TCHAR szRegistryAutoForceFinalGlide[] = TEXT("AutoForceFinalGlide");

const TCHAR szRegistryVoiceClimbRate[]= TEXT("VoiceClimbRate");
const TCHAR szRegistryVoiceTerrain[]= TEXT("VoiceTerrain");
const TCHAR szRegistryVoiceWaypointDistance[]= TEXT("VoiceWaypointDistance");
const TCHAR szRegistryVoiceTaskAltitudeDifference[]= TEXT("VoiceTaskAltitudeDifference");
const TCHAR szRegistryVoiceMacCready[]= TEXT("VoiceMacCready");
const TCHAR szRegistryVoiceNewWaypoint[]= TEXT("VoiceNewWaypoint");
const TCHAR szRegistryVoiceInSector[]= TEXT("VoiceInSector");
const TCHAR szRegistryVoiceAirspace[]= TEXT("VoiceAirspace");

const TCHAR szRegistryFinishMinHeight[]= TEXT("FinishMinHeight");
const TCHAR szRegistryStartMaxHeight[]= TEXT("StartMaxHeight");
const TCHAR szRegistryStartMaxSpeed[]= TEXT("StartMaxSpeed");
const TCHAR szRegistryStartMaxHeightMargin[]= TEXT("StartMaxHeightMargin");
const TCHAR szRegistryStartMaxSpeedMargin[]= TEXT("StartMaxSpeedMargin");
const TCHAR szRegistryStartHeightRef[] = TEXT("StartHeightRef");
const TCHAR szRegistryEnableNavBaroAltitude[] = TEXT("EnableNavBaroAltitude");
const TCHAR szRegistryOrbiter[] = TEXT("Orbiter");
const TCHAR szRegistryShading[] = TEXT("Shading");
const TCHAR szRegistryOverlayClock[] = TEXT("OverlayClock");

const TCHAR szRegistryLoggerTimeStepCruise[]= TEXT("LoggerTimeStepCruise");
const TCHAR szRegistryLoggerTimeStepCircling[]= TEXT("LoggerTimeStepCircling");

const TCHAR szRegistrySafetyMacCready[] = TEXT("SafetyMacCready");
const TCHAR szRegistryAbortSafetyUseCurrent[] = TEXT("AbortSafetyUseCurrent");
const TCHAR szRegistryAutoMcMode[] = TEXT("AutoMcMode");
const TCHAR szRegistryWaypointsOutOfRange[] = TEXT("WaypointsOutOfRange2"); // 101020 2nd version, reset needed
const TCHAR szRegistryEnableExternalTriggerCruise[] = TEXT("EnableExternalTriggerCruise");
const TCHAR szRegistryFAIFinishHeight[] = TEXT("FAIFinishHeight");
const TCHAR szRegistryOLCRules[] = TEXT("OLCRules");
const TCHAR szRegistryHandicap[] = TEXT("Handicap");
const TCHAR szRegistrySnailWidthScale[] = TEXT("SnailWidthScale");
const TCHAR szRegistryUserLevel[] = TEXT("UserLevel");
const TCHAR szRegistryRiskGamma[] = TEXT("RiskGamma");
const TCHAR szRegistryWindArrowStyle[] = TEXT("WindArrowStyle");
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
#ifdef GNAV
  InfoType[0]=873336334;
  InfoType[1]=856820491;
  InfoType[2]=822280982;
  InfoType[3]=2829105;
  InfoType[4]=103166000;
  InfoType[5]=421601569;
  InfoType[6]=657002759;
  InfoType[7]=621743887;
  InfoType[8]=439168301;
#else
  InfoType[0] = 921102;
  InfoType[1] = 725525;
  InfoType[2] = 262144;
  InfoType[3] = 74518;
  InfoType[4] = 657930;
  InfoType[5] = 2236963;
  InfoType[6] = 394758;
  InfoType[7] = 1644825;
  InfoType[8] = 1644825; // TESTFIX BUGFIX 091015 missing
#endif
}

void SetRegistryStringIfAbsent(const TCHAR* name,
			       const TCHAR* value) {

  // VENTA force fonts registry rewrite in PNAs 
#if defined(PNA) || defined(FIVV) // VENTA TODO WARNING should really delete the key before creating it TODO
  SetRegistryString(name, value);
#else
  TCHAR temp[MAX_PATH];
  if (GetRegistryString(name, temp, MAX_PATH)) {  // 0==ERROR_SUCCESS 
    SetRegistryString(name, value);
  }
#endif 
}

void DefaultRegistrySettingsAltair(void)
{  // RLD left GNAV Altair settings untouched.  
   // these are redundant b/c they're also added to "InitialiseFontsHardCoded"
  SetRegistryStringIfAbsent(TEXT("InfoWindowFont"),
   TEXT("24,0,0,0,700,0,0,0,0,0,0,3,2,RasterGothicTwentyFourCond"));
  SetRegistryStringIfAbsent(TEXT("TitleWindowFont"),
   TEXT("10,0,0,0,500,0,0,0,0,0,0,3,2,RasterGothicNineCond"));
  SetRegistryStringIfAbsent(TEXT("CDIWindowFont"),
   TEXT("19,0,0,0,700,0,0,0,0,0,0,3,2,RasterGothicEighteenCond"));
  SetRegistryStringIfAbsent(TEXT("MapLabelFont"),
   TEXT("13,0,0,0,500,0,0,0,0,0,0,3,2,RasterGothicTwelveCond"));
  SetRegistryStringIfAbsent(TEXT("StatisticsFont"),
   TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,RasterGothicFourteenCond"));
  SetRegistryStringIfAbsent(TEXT("MapWindowFont"),
   TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,RasterGothicFourteenCond"));
  SetRegistryStringIfAbsent(TEXT("MapWindowBoldFont"),
   TEXT("15,0,0,0,700,0,0,0,0,0,0,3,2,RasterGothicFourteenCond"));
  SetRegistryStringIfAbsent(TEXT("BugsBallastFont"),
   TEXT("24,0,0,0,750,0,0,0,0,0,0,3,2,RasterGothicTwentyFourCond"));
  SetRegistryStringIfAbsent(TEXT("AirspacePressFont"),
   TEXT("24,0,0,0,750,0,0,0,0,0,0,3,2,RasterGothicTwentyFourCond"));
  SetRegistryStringIfAbsent(TEXT("AirspaceColourDlgFont"),
   TEXT("14,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"));
  SetRegistryStringIfAbsent(TEXT("TeamCodeFont"),
   TEXT("19,0,0,0,700,0,0,0,0,0,0,3,2,RasterGothicEighteenCond"));
#if 0
  SetRegistryStringIfAbsent(TEXT("ScaleList"),
   TEXT("0.5,1,2,5,10,20,50,100,150,200,500,1000"));
#endif
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

#if defined(GNAV) || defined(PCGNAV) || defined(GNAV_FONTEST)
  DefaultRegistrySettingsAltair();
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

  Temp=SafetyAltitudeMode;
  if(GetFromRegistry(szRegistrySafetyAltitudeMode,&Temp)==ERROR_SUCCESS)
    SafetyAltitudeMode = Temp;

  Temp=(DWORD)SAFETYALTITUDEARRIVAL;
  GetFromRegistry(szRegistrySafetyAltitudeArrival,&Temp);
  SAFETYALTITUDEARRIVAL = (double)Temp;

  Temp=(DWORD)SAFETYALTITUDEBREAKOFF;
  GetFromRegistry(szRegistrySafetyAltitudeBreakOff,&Temp);
  SAFETYALTITUDEBREAKOFF = (double)Temp;

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

  Temp=37; // discus 2a
  GetFromRegistry(szRegistryPolarID,
		  &Temp); 
  POLARID = (int)Temp;

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

  //Temp = EnableAnimation;
  Temp=0; // disabled by default VNT9
  GetFromRegistry(szRegistryAnimation,&Temp);
  EnableAnimation = (Temp==1);

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

  Temp  = CircleZoom;
  GetFromRegistry(szRegistryCircleZoom,&Temp);
  CircleZoom = (Temp == 1);

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
    OnAlternate1=true;
  } else {
    Alternate1 = -1;
    OnAlternate1=false;
  }

  Temp = Alternate2;
  if (GetFromRegistry(szRegistryAlternate2,&Temp)==ERROR_SUCCESS) {
    Alternate2 = (signed int) Temp; // 100429
    OnAlternate2=true;
  } else {
    Alternate2 = -1;
    OnAlternate2=false;
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

  Temp = 0;
  GetFromRegistry(szRegistryAirspaceWarning,&Temp);
  AIRSPACEWARNINGS = Temp;

  Temp = 30;
  GetFromRegistry(szRegistryWarningTime,&Temp);
  WarningTime = max(10,Temp);

  Temp = 30;
  GetFromRegistry(szRegistryAcknowledgementTime,&Temp);
  AcknowledgementTime = max(10,Temp);

  Temp = 80;
  GetFromRegistry(szRegistrySoundVolume,&Temp);
  SoundVolume = Temp;

  Temp = 4;
  GetFromRegistry(szRegistrySoundDeadband,&Temp);
  SoundDeadband = Temp;

  Temp = 1;
  GetFromRegistry(szRegistrySoundAudioVario,&Temp);
  EnableSoundVario = (Temp == 1);

  Temp = 1;
  GetFromRegistry(szRegistrySoundTask,&Temp);
  EnableSoundTask = (Temp == 1);

  Temp = 1;
  GetFromRegistry(szRegistrySoundModes,&Temp);
  EnableSoundModes = (Temp == 1);

  Temp = 500;
  GetFromRegistry(szRegistryNettoSpeed,&Temp);
  NettoSpeed = Temp;

  EnableCDICruise = 0;
  EnableCDICircling = 0;

  /* JMW temporarily disabled these because they are not updated for 4.7+
  Temp = 0;
  GetFromRegistry(szRegistryCDICruise,&Temp);
  EnableCDICruise = (Temp == 1);

  Temp = 0;
  GetFromRegistry(szRegistryCDICircling,&Temp);
  EnableCDICircling = (Temp == 1);
  */

  Temp = 0;
  GetFromRegistry(szRegistryAutoBlank,&Temp);
  EnableAutoBlank = (Temp == 1);

  Temp = 1;
  GetFromRegistry(szRegistryAutoBacklight,&Temp); // VENTA4
  EnableAutoBacklight = (Temp == 1);

  Temp = 1;
  GetFromRegistry(szRegistryAutoSoundVolume,&Temp); // VENTA4
  EnableAutoSoundVolume = (Temp == 1);

  Temp = 0;
  GetFromRegistry(szRegistryAircraftCategory,&Temp); // VENTA4
  AircraftCategory = Temp;

  Temp = 0;
  GetFromRegistry(szRegistryExtendedVisualGlide,&Temp); // VENTA4
  ExtendedVisualGlide = Temp;

  // Do not allow LK8000 mode be disabled. Allow only temporary disabling, and permanent setting for normal/advanced
  #if NEWPNAV
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
  #else
  Temp = (Look8000_t)lxcAdvanced;  
  GetFromRegistry(szRegistryLook8000,&Temp);
  if (Temp == 0) Temp = (Look8000_t)lxcAdvanced;  // FIX 091020
  Look8000 = Temp;
  #endif

  // AltArriv registry disabled 
#if (0)
  Temp = ALTA_SMC;
  GetFromRegistry(szRegistryAltArrivMode,&Temp);
  AltArrivMode = Temp;
#endif

  // NewMap forced on, ignore registry setting
#if (0)
  Temp = 1; 
  GetFromRegistry(szRegistryNewMap,&Temp); 
  NewMap = Temp;
#else
  NewMap = 1; 
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
  #if AUTORIENT
  Temp=10;
  GetFromRegistry(szRegistryAutoOrientScale,&Temp);
  AutoOrientScale=Temp;
  #endif

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

  #if LKTOPO
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
  #endif

  Temp=70;
  GetFromRegistry(szRegistryLKMaxLabels,&Temp);
  LKMaxLabels=Temp;

  Temp = (IphoneGestures_t)iphDisabled;
  GetFromRegistry(szRegistryIphoneGestures,&Temp);
  IphoneGestures = Temp;

  Temp = (PollingMode_t)pollmodeDisabled;
  GetFromRegistry(szRegistryPollingMode,&Temp);
  PollingMode = Temp;

  #if NEWPNAV
  if (ScreenLandscape)
	//Temp = (LKVarioBar_t)vBarVarioColor;
	Temp = (LKVarioBar_t)vBarDisabled;
  else
	Temp = (LKVarioBar_t)vBarDisabled;
  #else
  Temp = (LKVarioBar_t)vBarVarioColor;
  #endif
  GetFromRegistry(szRegistryLKVarioBar,&Temp);
  LKVarioBar = Temp;

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

  #if 0
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

  Temp=0; // 100724 OFF by default
  GetFromRegistry(szRegistryVirtualKeys,&Temp); 
  VirtualKeys = Temp;

  if ( AircraftCategory == (AircraftCategory_t)umParaglider )
  	Temp = 1;
  else
  	Temp = 0;
  GetFromRegistry(szRegistryUseMapLock,&Temp); 
  UseMapLock = (Temp==1);

  Temp = 0;
  GetFromRegistry(szRegistryActiveMap,&Temp); 
  ActiveMap = (Temp==1);

  Temp=1;
  GetFromRegistry(szRegistryBestWarning,&Temp); 
  BestWarning=Temp;

  Temp=0; // not active by default 100219
  GetFromRegistry(szRegistryThermalBar,&Temp); 
  ThermalBar=Temp;

  #if 0
  Temp=1;
  GetFromRegistry(szRegistryMcOverlay,&Temp); 
  McOverlay=Temp;
  #else
  McOverlay=true;
  #endif

  Temp=1;
  GetFromRegistry(szRegistryTrackBar,&Temp); 
  TrackBar=Temp;

  #if NEWPNAV
  if (ScreenLandscape)
	//Temp = (GlideBarMode_t)gbFinish;
	Temp = (GlideBarMode_t)gbDisabled;
  else
	Temp = (GlideBarMode_t)gbDisabled;
  #else
  // For most of us flying cross country with long tasks, this is the correct settings.
  Temp = (GlideBarMode_t)gbFinish;  // 100219
  #endif
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

  /*
  Temp = 0;
  GetFromRegistry(szRegistryVarioGauge,&Temp);
  EnableVarioGauge = (Temp == 1);
  */

  Temp = 250;
  GetFromRegistry(szRegistryDebounceTimeout, &Temp);
  debounceTimeout = Temp;

  Temp = 100;
  GetFromRegistry(szRegistryAccelerometerZero,&Temp);
  AccelerometerZero = Temp;
  if (AccelerometerZero==0.0) {
    AccelerometerZero= 100.0;
    Temp = 100;
    SetToRegistry(szRegistryAccelerometerZero,Temp);
  }

  // new appearance variables

  //Temp = Appearance.IndFinalGlide;
  Temp=(IndFinalGlide_t)fgFinalGlideDefault; // VNT9 default
  GetFromRegistry(szRegistryAppIndFinalGlide, &Temp);
  //Appearance.IndFinalGlide = (IndFinalGlide_t)Temp;
  Appearance.IndFinalGlide = (IndFinalGlide_t)fgFinalGlideDefault; // VENTA9 FIX final glide bar mode set to default

  Temp = (IndLandable_t)wpLandableAltA;
  GetFromRegistry(szRegistryAppIndLandable, &Temp);
  Appearance.IndLandable = (IndLandable_t)Temp;

  Temp = 1; 	// DEFAULT LK8000 091019
  GetFromRegistry(szRegistryAppInverseInfoBox, &Temp);
  Appearance.InverseInfoBox = (Temp != 0);

  Temp = Appearance.GaugeVarioSpeedToFly;
  GetFromRegistry(szRegistryAppGaugeVarioSpeedToFly, &Temp);
  Appearance.GaugeVarioSpeedToFly = (Temp != 0);

  Temp = Appearance.GaugeVarioAvgText;
  GetFromRegistry(szRegistryAppGaugeVarioAvgText, &Temp);
  Appearance.GaugeVarioAvgText = (Temp != 0);

  Temp = Appearance.GaugeVarioMc;
  GetFromRegistry(szRegistryAppGaugeVarioMc, &Temp);
  Appearance.GaugeVarioMc = (Temp != 0);

  Temp = Appearance.GaugeVarioBugs;
  GetFromRegistry(szRegistryAppGaugeVarioBugs, &Temp);
  Appearance.GaugeVarioBugs = (Temp != 0);

  Temp = Appearance.GaugeVarioBallast;
  GetFromRegistry(szRegistryAppGaugeVarioBallast, &Temp);
  Appearance.GaugeVarioBallast = (Temp != 0);

  Temp = Appearance.GaugeVarioGross;
  GetFromRegistry(szRegistryAppGaugeVarioGross, &Temp);
  Appearance.GaugeVarioGross = (Temp != 0);

  Appearance.CompassAppearance = (CompassAppearance_t)apCompassAltA; // 091114 FIXED SMALL OUTLINED COMPASS
  

  //Temp = Appearance.InfoBoxBorder;
  Temp=(InfoBoxBorderAppearance_t)apIbBox; // VNT9 default
  GetFromRegistry(szRegistryAppInfoBoxBorder, &Temp);
  Appearance.InfoBoxBorder = (InfoBoxBorderAppearance_t)Temp;

// VENTA2-ADDON Geometry change and PNA custom font settings
// depending on infobox geometry and model type
// I had to move here the font setting because I needed first to 
// know the screen geometry, in the registry!
#if defined(PNA) || defined(FIVV)
  Temp = Appearance.InfoBoxGeom;
  GetFromRegistry(szRegistryAppInfoBoxGeom, &Temp);
  Appearance.InfoBoxGeom = (InfoBoxGeomAppearance_t)Temp;

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
#endif

  Temp = Appearance.StateMessageAlligne;
  GetFromRegistry(szRegistryAppStatusMessageAlignment, &Temp);
  Appearance.StateMessageAlligne = (StateMessageAlligne_t)Temp;

  Temp = (TextInputStyle_t)tiKeyboard;
  GetFromRegistry(szRegistryAppTextInputStyle, &Temp);
  Appearance.TextInputStyle = (TextInputStyle_t)Temp;

  Temp = Appearance.DefaultMapWidth;
  GetFromRegistry(szRegistryAppDefaultMapWidth, &Temp);
  Appearance.DefaultMapWidth = Temp;

  //Temp = Appearance.InfoBoxColors;
  Temp=1; // true as default VNT9
  GetFromRegistry(szRegistryAppInfoBoxColors, &Temp);
  Appearance.InfoBoxColors = (Temp != 0);

  Temp = Appearance.GaugeVarioAveNeedle;
  GetFromRegistry(szRegistryAppAveNeedle, &Temp);
  Appearance.GaugeVarioAveNeedle = (Temp != 0);

  // StateMessageAlligne : center, topleft
  // DefaultMapWidth: 206?
  // CompassAppearance (north arrow)
  // 
  // DontShowLoggerIndicator
  // FlightModeIcon
  // DontShowAutoMacCready
  // MapScale
  // MapScale2
  // BestCruiseTrack
  // Aircraft
  // IndFinalGlide
  // IndLandable

  Temp = 1;
  GetFromRegistry(szRegistryAutoAdvance,&Temp);
  AutoAdvance = (Temp == 1);

  Temp = AutoMcMode;
  GetFromRegistry(szRegistryAutoMcMode,&Temp);
  AutoMcMode = Temp;

  Temp = WaypointsOutOfRange;
  GetFromRegistry(szRegistryWaypointsOutOfRange,&Temp);
  WaypointsOutOfRange = Temp;

  Temp = OLCRules;
  GetFromRegistry(szRegistryOLCRules,&Temp);
  OLCRules = Temp;

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
  GetFromRegistry(szRegistryBlockSTF,&Temp);
  EnableBlockSTF = (Temp == 1);

  Temp = 0;
  GetFromRegistry(szRegistryAutoZoom,&Temp);
  MapWindow::AutoZoom = (Temp == 1);

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

  #ifndef NOFLARMGAUGE
  Temp = EnableFLARMGauge;
  GetFromRegistry(szRegistryEnableFLARMGauge,&Temp);
  EnableFLARMGauge = false;
  #endif

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
  #ifdef NEWMOVEICON
  MapWindow::GliderScreenPositionY = MapWindow::GliderScreenPosition;
  #endif


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


#ifdef VEGAVOICE
  Temp = EnableVoiceClimbRate; 
  GetFromRegistry(szRegistryVoiceClimbRate,&Temp);
  EnableVoiceClimbRate = (Temp!=0);

  Temp = EnableVoiceTerrain; 
  GetFromRegistry(szRegistryVoiceTerrain,&Temp);
  EnableVoiceTerrain = (Temp!=0);

  Temp = EnableVoiceWaypointDistance; 
  GetFromRegistry(szRegistryVoiceWaypointDistance,&Temp);
  EnableVoiceWaypointDistance = (Temp!=0);

  Temp = EnableVoiceTaskAltitudeDifference; 
  GetFromRegistry(szRegistryVoiceTaskAltitudeDifference,&Temp);
  EnableVoiceTaskAltitudeDifference = (Temp!=0);

  Temp = EnableVoiceMacCready; 
  GetFromRegistry(szRegistryVoiceMacCready,&Temp);
  EnableVoiceMacCready = (Temp!=0);

  Temp = EnableVoiceNewWaypoint; 
  GetFromRegistry(szRegistryVoiceNewWaypoint,&Temp);
  EnableVoiceNewWaypoint = (Temp!=0);

  Temp = EnableVoiceInSector; 
  GetFromRegistry(szRegistryVoiceInSector,&Temp);
  EnableVoiceInSector = (Temp!=0);

  Temp = EnableVoiceAirspace; 
  GetFromRegistry(szRegistryVoiceAirspace,&Temp);
  EnableVoiceAirspace = (Temp!=0);
#endif

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

  #if ORBITER
  Temp = 1;
  GetFromRegistry(szRegistryOrbiter,&Temp);
  Orbiter = (Temp!=0);
  #endif
  Temp = 1;
  GetFromRegistry(szRegistryShading,&Temp);
  Shading = Temp;
  Temp = 0;
  GetFromRegistry(szRegistryOverlayClock,&Temp);
  OverlayClock = Temp;


  Temp = LoggerTimeStepCruise;
  GetFromRegistry(szRegistryLoggerTimeStepCruise,&Temp);
  LoggerTimeStepCruise = Temp;

  Temp = LoggerTimeStepCircling;
  GetFromRegistry(szRegistryLoggerTimeStepCircling,&Temp);
  LoggerTimeStepCircling = Temp;

  Temp = GlidePolar::AbortSafetyUseCurrent;
  GetFromRegistry(szRegistryAbortSafetyUseCurrent, &Temp);
  GlidePolar::AbortSafetyUseCurrent = (Temp != 0);

  Temp = iround(GlidePolar::SafetyMacCready*10);
  GetFromRegistry(szRegistrySafetyMacCready,&Temp);
  GlidePolar::SafetyMacCready = Temp/10.0;

  Temp  = UserLevel;
  GetFromRegistry(szRegistryUserLevel,&Temp);
  UserLevel = Temp;

  Temp  = iround(GlidePolar::RiskGamma*10);
  GetFromRegistry(szRegistryRiskGamma,&Temp);
  GlidePolar::RiskGamma = Temp/10.0;

  //Temp = MapWindow::WindArrowStyle;
  Temp=(CompassAppearance_t)apCompassAltA; // VNT9 default
  GetFromRegistry(szRegistryWindArrowStyle,&Temp);
  MapWindow::WindArrowStyle = Temp;

  Temp = DisableAutoLogger;
  GetFromRegistry(szRegistryDisableAutoLogger,&Temp);
  if (Temp) 
    DisableAutoLogger = true;
  else 
    DisableAutoLogger = false;
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

#if 0
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



/*
double Distance(double lat1, double lon1, double lat2, double lon2)
{
    R = earth's radius = 6371000
    dlat = lat2-lat1;
    dlon = long2-long1
    a= sin^2(dlat/2)+cos(lat1)*cos(lat2)*sin^2(dlong/2)
    c= 2*atan2(sqrt(a),sqrt(1.0-a));
    d = R.c

  lat1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;

  double dlat = lat2-lat1;
  double dlon = lon2-lon1;
  double s1 = sin(dlat/2);
  double s2 = sin(dlon/2);
  double a= s1*s1+cos(lat1)*cos(lat2)*s2*s2;
  double c= 2.0*atan2(sqrt(a),sqrt(1.0-a));
  return 6371000.0*c;

  // Old code... broken
  double distance, dTmp;

  dTmp =  sin(lat1)*sin(lat2) +
			cos(lat1)*cos(lat2) * cos(lon1-lon2);

  if (dTmp > 1.0)         // be shure we dont call acos with
    distance = 0;         // values greater than 1 (like 1.0000000000001)
  else
    distance = (double)acos(dTmp) * (double)(RAD_TO_DEG * 111194.9267);
  return (double)(distance);
}
  */

/*
double Bearing(double lat1, double lon1, double lat2, double lon2)
{
//    theta = atan2(sin(dlong)*cos(lat2),
  //    cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(dlong));

  lat1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;

  double clat1 = cos(lat1);
  double clat2 = cos(lat2);
  double slat1 = sin(lat1);
  double slat2 = sin(lat2);
  double dlat = lat2-lat1;
  double dlon = lon2-lon1;

  double theta = 
    atan2(sin(dlon)*clat2,
          clat1*slat2-slat1*clat2*cos(dlon))*RAD_TO_DEG;
  while (theta>360.0) {
    theta-= 360.0;
  }
  while (theta<0.0) {
    theta+= 360.0;
  }
  return theta;

  // old code
  #ifdef HAVEEXCEPTIONS
  __try{
  #endif

  double angle;
  double d;

  d = (slat1*slat2 +  clat1*clat2 * cos(lon1-lon2) );
  if(d>1) d = 0.99999999999999;
  if(d<-1) d = -0.99999999999999;
  d = acos(d);

  if(sin(lon1-lon2)<0 )
    {
      angle = (((slat2-slat1)
		* cos(d) ) / (sin(d)*clat1));

      if(angle >1) angle = 1;
      if(angle<-1) angle = -1;
      angle = acos(angle);

      // JMW Redundant code?
      //if(lat1>lat2)
//	angle = angle * (180/pi);
  //    else
	//angle = angle * (180/pi);
      //
      angle *= RAD_TO_DEG;
    }
  else
    {
      if (d != 0 && clat1 != 0){
        angle=(( (slat2-slat1)
          * cos(d) ) / (sin(d)*clat1));
        if(angle >1) angle = 1;
        if(angle<-1) angle = -1;
        angle = acos(angle);
      } else
        angle = 0;

      angle = 360 - (angle * RAD_TO_DEG);
    }
  #ifdef HAVEEXCEPTIONS
  }__except(EXCEPTION_EXECUTE_HANDLER ){
    return(0);
  }
  #endif

  return (double)angle;
  
}
*/


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

    ww[0]= 403.0; // 383
    ww[1]= 101.0; // 121
    POLARV[0]= 115.03;
    POLARW[0]= -0.86;
    POLARV[1]= 174.04;
    POLARW[1]= -1.76;
    POLARV[2]= 212.72;
    POLARW[2]= -3.4;

    GetRegistryString(szRegistryPolarFile, szFile, MAX_PATH);
    ExpandLocalPath(szFile);

    #ifndef HAVEEXCEPTIONS
    SetRegistryString(szRegistryPolarFile, TEXT("\0"));
    #endif

    hFile = CreateFile(szFile,GENERIC_READ,0,(LPSECURITY_ATTRIBUTES)NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

    if (hFile != INVALID_HANDLE_VALUE ){

#ifdef HAVEEXCEPTIONS
      __try{
#endif
      int *p=NULL; // test, force an exception
      p=0;

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
#if 100201
		if ( _tcscmp(ctemp,_T("")) != 0) {
              		GlidePolar::WingArea = StrToDouble(ctemp,NULL);
			StartupStore(_T(". Winpilot custom polar has special wing area=%f\n"),GlidePolar::WingArea);
		} else
	      		GlidePolar::WingArea = 0.0;
#endif
		
              PolarWinPilot2XCSoar(POLARV, POLARW, ww);
#if 0
	      GlidePolar::WingArea = 0.0;
#endif

              foundline = true;
            }
        }

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
#ifdef HAVEEXCEPTIONS
  }__except(EXCEPTION_EXECUTE_HANDLER){
    foundline = false;
  }
#endif
  return(foundline);

}

// *LS-3	WinPilot POLAR file: MassDryGross[kg], MaxWaterBallast[liters], Speed1[km/h], Sink1[m/s], Speed2, Sink2, Speed3, Sink3
// 403, 101, 115.03, -0.86, 174.04, -1.76, 212.72,	-3.4


typedef double PolarCoefficients_t[3];
typedef double WeightCoefficients_t[3];


void CalculateNewPolarCoef(void)
{

  StartupStore(TEXT(". Calculate New Polar Coef%s"),NEWLINE);

  static PolarCoefficients_t Polars[7] =
    {
      {-0.0538770500225782443497, 0.1323114348, -0.1273364037098239098543},
      {-0.0532456270195884696748, 0.1509454717, -0.1474304674787072275183},
      {-0.0598306909918491529791, 0.1896480967, -0.1883344146619101871894},
      {-0.0303118230885946660507, 0.0771466019, -0.0799469636558217515699},
      {-0.0222929913566948641563, 0.0318771616, -0.0307925896846546928318},
      {-0.0430828898445299480353, 0.0746938776, -0.0487285153053357557183},
      {0.0, 0.0, 0.0}

    };


  /* Weights:
     0 Pilot Weight?
     1 Glider Weight
     2 BallastWeight
  */

  static WeightCoefficients_t Weights[7] = { {70,190,1},
                                             {70,250,100},
                                             {70,240,285},
                                             {70,287,165},  // w ok!
                                             {70,400,120},  //
                                             {70,527,303},
                                             {0,0,0}
  };
  static double WingAreas[7] = { 
    12.4,  // Ka6
    11.0,  // ASW19
    10.5,  // LS8
    9.0,   // ASW27
    11.4,  // LS6C-18
    16.31, // ASW22
    0};
  int i;

  ASSERT(sizeof(Polars)/sizeof(Polars[0]) == sizeof(Weights)/sizeof(Weights[0]));

  GlidePolar::WeightOffset=0; // 100131 

  if (POLARID < (int)(sizeof(Polars)/sizeof(Polars[0])) ){ // 100207 fixed (int)
    for(i=0;i<3;i++){
      POLAR[i] = Polars[POLARID][i];
      WEIGHTS[i] = Weights[POLARID][i];
    }
    GlidePolar::WingArea = WingAreas[POLARID];
  }
  if (POLARID==POLARUSEWINPILOTFILE) {
    if (ReadWinPilotPolar())
    // polar data gets from winpilot file
      return;
  } else if (POLARID>POLARUSEWINPILOTFILE){
    if (ReadWinPilotPolarInternal(POLARID-7))
      // polar data get from build in table
      return;
  } else if (POLARID<POLARUSEWINPILOTFILE){
    // polar data get from historical table
    return;
  }

  // ups
  // error reading winpilot file

  POLARID = 2;              // do it again with default polar (LS8)

  CalculateNewPolarCoef();
  MessageBoxX(NULL, 
              gettext(TEXT("_@M920_")), // Error loading Polar file!\r\nUse LS8 Polar.
	// LKTOKEN  _@M791_ = "Warning" 
              gettext(TEXT("_@M791_")),
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


void ConvertFlightLevels(void)
{
  unsigned i;

  // TODO accuracy: Convert flightlevels is inaccurate!

  for(i=0;i<NumberOfAirspaceCircles;i++)
    {
      if(AirspaceCircle[i].Base.FL  != 0)
	{
	  AirspaceCircle[i].Base.Altitude = (AirspaceCircle[i].Base.FL * 100) + ((QNH-1013)*30);
	  AirspaceCircle[i].Base.Altitude = AirspaceCircle[i].Base.Altitude / TOFEET;
	}
      if(AirspaceCircle[i].Top.FL  != 0)
	{
	  AirspaceCircle[i].Top.Altitude = (AirspaceCircle[i].Top.FL * 100) + ((QNH-1013)*30);
	  AirspaceCircle[i].Top.Altitude = AirspaceCircle[i].Top.Altitude / TOFEET;
	}
    }


  for(i=0;i<NumberOfAirspaceAreas;i++)
    {
      if(AirspaceArea[i].Base.FL  != 0)
	{
	  AirspaceArea[i].Base.Altitude = (AirspaceArea[i].Base.FL * 100) + ((QNH-1013)*30);
	  AirspaceArea[i].Base.Altitude = AirspaceArea[i].Base.Altitude / TOFEET;
	}
      if(AirspaceArea[i].Top.FL  != 0)
	{
	  AirspaceArea[i].Top.Altitude = (AirspaceArea[i].Top.FL * 100) + ((QNH-1013)*30);
	  AirspaceArea[i].Top.Altitude = AirspaceArea[i].Top.Altitude / TOFEET;
	}
    }
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
	StartupStore(strAssetNumber);
	StartupStore(TEXT(" (PC)%s"),NEWLINE);
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

  ReadCompaqID();
  if(strAssetNumber[0] != '\0')
    {
      StartupStore(strAssetNumber);
      StartupStore(TEXT(" (compaq)%s"),NEWLINE);
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

void ReadCompaqID(void)
{
  PROCESS_INFORMATION pi;
  HANDLE hInFile;// = INVALID_HANDLE_VALUE;
  DWORD dwBytesRead;

  if(strAssetNumber[0] != '\0')
    {
      return;
    }

  CreateProcess(TEXT("\\windows\\CreateAssetFile.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);

  hInFile = CreateFile(TEXT("\\windows\\cpqAssetData.dat"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  if (hInFile == INVALID_HANDLE_VALUE)
    {
      //	    MessageBox(hWnd, TEXT("Unable to open asset data file."), TEXT("Error!"), MB_OK);
      return;
    }
  SetFilePointer(hInFile, 976, NULL, FILE_BEGIN);
  memset(strAssetNumber, 0, 64 * sizeof(TCHAR));
  ReadFile(hInFile, &strAssetNumber, 64, &dwBytesRead, (OVERLAPPED *)NULL);
  CloseHandle(hInFile);
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


#if 0
void ReadUUIDold(void)
{
#ifndef __MINGW32__
  BOOL fRes;
  DWORD dwBytesReturned =0;
  DEVICE_ID DevID;
  int wSize;
  int i;

  GUID Guid;

  unsigned long temp, Asset;

  memset(&Guid, 0, sizeof(GUID));

  memset(&DevID, 0, sizeof(DEVICE_ID));
  DevID.dwSize = sizeof(DEVICE_ID);

  fRes = KernelIoControl( IOCTL_HAL_GET_DEVICEID, NULL, 0,
			  &DevID, sizeof( DEVICE_ID ), &dwBytesReturned );

  wSize = DevID.dwSize;

  if( (FALSE != fRes) || (ERROR_INSUFFICIENT_BUFFER != GetLastError()))
    return;

  memset(&DevID, 0, sizeof(wSize));
  DevID.dwSize = wSize;

  fRes = KernelIoControl( IOCTL_HAL_GET_DEVICEID, NULL, 0,
			  &DevID, wSize, &dwBytesReturned );

  if((FALSE == fRes) || (ERROR_INSUFFICIENT_BUFFER == GetLastError()) )
    return;

  BYTE* pDat = (BYTE*)&Guid.Data1;
  BYTE* pSrc = (BYTE*)(&DevID) + DevID.dwPresetIDOffset;
  memcpy(pDat, pSrc, DevID.dwPresetIDBytes);
  pDat +=  DevID.dwPresetIDBytes;
  pSrc =  (BYTE*)(&DevID) + DevID.dwPlatformIDOffset;
  memcpy(pDat, pSrc, DevID.dwPlatformIDBytes);

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
  return;
#endif
}
#endif

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
    case NOGLIDER:		
	// LKTOKEN  _@M464_ = "No Glider" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M464_"))); break;
    case CTR:					
      _tcscpy(szTitleBuffer,TEXT("CTR")); break;
    case WAVE:				
	// LKTOKEN  _@M794_ = "Wave" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M794_"))); break;
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
    return 0.0; // error!
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


void SaveSoundSettings()
{
  SetToRegistry(szRegistrySoundVolume, (DWORD)SoundVolume);
  SetToRegistry(szRegistrySoundDeadband, (DWORD)SoundDeadband);
  SetToRegistry(szRegistrySoundAudioVario, EnableSoundVario);
  SetToRegistry(szRegistrySoundTask, EnableSoundTask);
  SetToRegistry(szRegistrySoundModes, EnableSoundModes);
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
    return;
  }

  if (devIdx == 1){
    GetRegistryString(szRegistryDeviceB , Name, DEVNAMESIZE);
    return;
  }

  if (devIdx == 2){
    GetRegistryString(szRegistryDeviceC , Name, DEVNAMESIZE);
    return;
  }

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
    if ((found != 2) || !key || !value) {
      
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
	    mbstowcs(wname, name, strlen(name)+1);
	    mbstowcs(wvalue, value, strlen(value)+1);
	    SetRegistryString(wname, wvalue);
	    found = true;
	  }
        } else if (sscanf(inval, "%[^#=\r\n ]=%d[\r\n]", name, &j) == 2) {
	  if (strlen(name)>0) {
	    mbstowcs(wname, name, strlen(name)+1);
	    SetToRegistry(wname, j);
	    found = true;
	  }
        } else if (sscanf(inval, "%[^#=\r\n ]=\"\"[\r\n]", name) == 1) {
	  if (strlen(name)>0) {
	    mbstowcs(wname, name, strlen(name)+1);
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
	  uValue.pValue[nValueSize]= 0; // null terminate, just in case
	  uValue.pValue[nValueSize+1]= 0; // null terminate, just in case
	  if (_tcslen((TCHAR*)uValue.pValue)>0) {
	    fprintf(fp,"%S=\"%S\"\r\n", lpstrName, (wchar_t *) uValue.pValue); // 100228 FIX
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
  for (i = 0; i < wcslen(old_string); i++) {
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
  
  new_string = (TCHAR *)malloc((wcslen(buffer)+1)*sizeof(TCHAR));
  wcscpy(new_string, buffer);
  
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

#if defined (PNA) && (!defined(WINDOWSPC) || (WINDOWSPC <=0) )

// check this is still valid 101212
// For PNAs the localpath is taken from the application exec path
// example> \sdmmc\bin\Program.exe  results in localpath=\sdmmc\XCSoarData
// Then the basename is searched for an underscore char, which is
// used as a separator for getting the model type.  example>
// program_pna.exe results in GlobalModelType=pna
  
	_stprintf(buffer,TEXT("%s%S"),gmfpathname(), XCSDATADIR );

#elif defined (FIVV) && (!defined(WINDOWSPC) || (WINDOWSPC <=0) )

  _stprintf(buffer,TEXT("%s%S"),gmfpathname(), XCSDATADIR );

#else
	// get the MyDocuments directory
	SHGetSpecialFolderPath(hWndMainWindow, buffer, loc, false);

	_tcscat(buffer,TEXT("\\"));
	_tcscat(buffer,TEXT(XCSDATADIR));
#endif

	if (_tcslen(file)>0) {
		wcsncat(buffer, TEXT("\\"), MAX_PATH);    
		wcsncat(buffer, file, MAX_PATH);
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
  // lfTmp.lfQuality = CLEARTYPE_COMPAT_QUALITY; // 110106 FIX TEST

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

#if (WINDOWSPC>0) && !defined(PCGNAV)
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
/*
#ifndef GNAV
  long utcoffset=0;
  // returns offset in seconds
  TIME_ZONE_INFORMATION TimeZoneInformation;
  DWORD tzi = GetTimeZoneInformation(&TimeZoneInformation);

  utcoffset = -TimeZoneInformation.Bias*60;

  if (tzi==TIME_ZONE_ID_STANDARD) {
    utcoffset -= TimeZoneInformation.StandardBias*60;
  }
  if (tzi==TIME_ZONE_ID_DAYLIGHT) {
    utcoffset -= TimeZoneInformation.DaylightBias*60;
  }
#if (WINDOWSPC>0)
  return UTCOffset;
#else
  return utcoffset;
#endif
#else
  return UTCOffset;
#endif
*/
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
  SaveRegistryToFile(startProfileFile);
  SaveRegistryToFile(defaultProfileFile);
}

void XCSoarGetOpts(LPTSTR CommandLine) {
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
    pC = _tcsstr(MyCommandLine, TEXT("-square"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=480;
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
    pC = _tcsstr(MyCommandLine, TEXT("-240x320"));
    if (pC != NULL){
      SCREENWIDTH=240;
      SCREENHEIGHT=320;
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


#if !defined(GNAV) || (WINDOWSPC>0)
typedef DWORD (_stdcall *GetIdleTimeProc) (void);
GetIdleTimeProc GetIdleTime;
#endif

int MeasureCPULoad() {
#if (!defined(GNAV) || (WINDOWSPC>0)) && !defined(__MINGW32__)
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

#if defined(GNAV) && defined(__MINGW32__)
  // JMW GetIdleTime() not defined?
  return 100;
#else
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
#endif
}



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
// Also remember, currently 200 items limit in WindowControls.h DFE_  enums.
WinPilotPolarInternal WinPilotPolars[] = 
{
  {TEXT("1-26E"), 315, 0, 82.3, -1.04, 117.73, -1.88, 156.86, -3.8, 14.87},
  {TEXT("1-34"), 354, 0, 89.82, -0.8, 143.71, -2.1, 179.64, -3.8, 14.03},
  {TEXT("1-35A"), 381, 179, 98.68, -0.74, 151.82, -1.8, 202.87, -3.9, 9.64},
  {TEXT("1-36 Sprite"), 322, 0, 75.98, -0.68, 132.96, -2, 170.95, -4.1, 13.10},
  {TEXT("604"), 570, 100, 112.97, 0.72, 150.64, -1.42, 207.13, -4.1, 16.26},
  {TEXT("ASH-25M 2"), 750, 121, 130.01, -0.78, 169.96, -1.4, 219.94, -2.6, 16.31},
  {TEXT("ASH-25M 1"), 660, 121, 121.3, -0.73, 159.35, -1.31, 206.22, -2.4, 16.31},
  {TEXT("ASH-25 (PAS)"), 693, 120, 105.67, -0.56, 163.25, -1.34, 211.26, -2.5, 16.31},
  {TEXT("ASH-25 (PIL)"), 602, 120, 98.5, -0.52, 152.18, -1.25, 196.93, -2.3, 16.31},
  {TEXT("Astir CS"), 330, 90, 75.0, -0.7, 93.0, -0.74, 185.00, -3.1, 12.40},
  {TEXT("ASK-21"), 450, 0, 100.0, -0.82, 120.0, -1.10, 150.00, -1.9, 17.95},
  {TEXT("ASK-23"), 330, 0, 100.0, -0.85, 120.0, -1.19, 150.00, -2.02, 12.9},
  {TEXT("ASW-12"), 948, 189, 95, -0.57, 148, -1.48, 183.09, -2.6, 13.00},
  {TEXT("ASW-15"), 349, 91, 97.56, -0.77, 156.12, -1.9, 195.15, -3.4, 11.0},
  {TEXT("ASW-17"), 522, 151, 114.5, -0.7, 169.05, -1.68, 206.5, -2.9, 14.84},
  {TEXT("ASW-19"), 363, 125, 97.47, -0.74, 155.96, -1.64, 194.96, -3.1, 11.0},
  {TEXT("ASW-20"), 377, 159, 116.2, -0.77, 174.3, -1.89, 213.04, -3.3, 10.5},
  {TEXT("ASW-24"), 350, 159, 108.82, -0.73, 142.25, -1.21, 167.41, -1.8, 10.0},
  {TEXT("ASW-27 Wnglts"), 357, 165, 108.8, -0.64, 156.4, -1.18, 211.13, -2.5, 9.0},
  {TEXT("Std Cirrus"), 337, 80, 93.23, -0.74, 149.17, -1.71, 205.1, -4.2, 10.04},
  {TEXT("Cobra (SZD-36)"), 350, 30, 70.8, -0.60, 94.5, -0.69, 148.1, -1.83, 11.6},
  {TEXT("DG-400 (15m)"), 440, 90, 115, -0.76, 160.53, -1.22, 210.22, -2.3, 10.0},
  {TEXT("DG-400 (17m)"), 444, 90, 118.28, -0.68, 163.77, -1.15, 198.35, -1.8, 10.57},
  {TEXT("DG-500M PAS"), 750, 100, 121.6, -0.75, 162.12, -1.37, 202.66, -2.5, 18.29},
  {TEXT("DG-500M PIL"), 659, 100, 115.4, -0.71, 152.01, -1.28, 190.02, -2.3, 18.29},
  {TEXT("DG-500 PAS"), 660, 160, 115.5, -0.72, 152.16, -1.28, 190.22, -2.3, 18.29},
  {TEXT("DG-500 PIL"), 570, 160, 107.5, -0.66, 141.33, -1.19, 176.66, -2.1, 18.29},
  {TEXT("DG-800 15m"), 468, 120, 133.9, -0.88, 178.87, -1.53, 223.59, -2.5, 10.68},
  {TEXT("DG-800 18m Wnglts"), 472, 120, 106, -0.62, 171.75, -1.47, 214.83, -2.4, 11.81},
  {TEXT("Discus A"), 350, 182, 103.77, -0.72, 155.65, -1.55, 190.24, -3.1, 10.58},
  {TEXT("Discus 2a"), 330, 195, 110.0, -0.728, 155.00, -1.26, 200.00, -2.26, 10.16},
  {TEXT("Duo Discus (PAS)"), 628, 201, 106.5, -0.79, 168.11, -1.54, 201.31, -2.9, 16.40},
  {TEXT("Duo Discus (PIL)"), 537, 201, 94.06, -0.72, 155.49, -1.43, 188.21, -2.7, 16.40},
  {TEXT("Genesis II"), 374, 151, 94, -0.61, 141.05, -1.18, 172.4, -2.0, 11.24},
  {TEXT("Grob G-103 Twin II (PAS)"), 580, 0, 99, -0.8, 175.01, -1.95, 225.02, -3.8, 17.52},
  {TEXT("Grob G-103 Twin II (PIL)"), 494, 0, 90.75, -0.74, 161.42, -1.8, 207.54, -3.5, 17.52},
  {TEXT("H-201 Std Libelle"), 304, 50, 97, -0.79, 152.43, -1.91, 190.54, -3.3, 9.8},
  {TEXT("H-301 Libelle"), 300, 50, 94, -0.68, 147.71, -2.03, 184.64, -4.1, 9.8},
  {TEXT("IS-29D2 Lark"), 360, 0, 100, -0.82, 135.67, -1.55, 184.12, -3.3, 10.4},
  {TEXT("Jantar 2 (SZD-42A)"), 482, 191, 109.5, -0.66, 157.14, -1.47, 196.42, -2.7, 14.27},
  {TEXT("Janus B (18.2m PAS)"), 603, 170, 115.5, -0.76, 171.79, -1.98, 209.96, -4.0, 17.4},
  {TEXT("Janus B (18.2m PIL)"), 508, 170, 105.7, -0.7, 157.65, -1.82, 192.68, -3.6, 17.4},
  {TEXT("Ka-6CR"), 310, 0, 87.35, -0.81, 141.92, -2.03, 174.68, -3.5, 12.4},
  {TEXT("L-33 SOLO"), 330, 0, 87.2, -0.8, 135.64, -1.73, 174.4, -3.4, 11.0},
  {TEXT("LS-1C"), 350, 91, 115.87, -1.02, 154.49, -1.84, 193.12, -3.3, 9.74},
  {TEXT("LS-3"),  383, 121, 93.0, -0.64, 127.0, -0.93, 148.2, -1.28, 10.5},
  {TEXT("LS-4a"), 361, 121, 114.9, -0.80, 172.3, -2.33, 210.59, -4.5, 10.35},
  {TEXT("LS7wl"), 350, 150, 103.77, -0.73, 155.65, -1.47, 180.00, -2.66, 9.80},
  {TEXT("Nimbus 2"), 493, 159, 119.83, -0.75, 179.75, -2.14, 219.69, -3.8, 14.41},
  {TEXT("Nimbus 3DM (PAS)"), 820, 168, 114.97, -0.57, 157.42, -0.98, 222.24, -2.3, 16.70},
  {TEXT("Nimbus 3D (PAS)"), 712, 168, 93.64, -0.46, 175.42, -1.48, 218.69, -2.5, 16.70},
  {TEXT("Nimbus 3D (PIL)"), 621, 168, 87.47, -0.43, 163.86, -1.38, 204.27, -2.3, 16.70},
  {TEXT("Nimbus 3"), 527, 159, 116.18, -0.67, 174.28, -1.81, 232.37, -3.8, 16.70},
  {TEXT("Nimbus 3T"), 577, 310, 141.7, -0.99, 182.35, -1.89, 243.13, -4.0, 16.70},
  {TEXT("Nimbus 4DM (PAS)"), 820, 168, 100.01, -0.48, 150.01, -0.87, 190.76, -1.6, 17.8},
  {TEXT("Nimbus 4DM (PIL)"), 729, 168, 94.31, -0.46, 141.47, -0.82, 179.9, -1.5, 17.8},
  {TEXT("Nimbus 4D PAS"), 743, 303, 107.5, -0.5, 142.74, -0.83, 181.51, -1.6, 17.8},
  {TEXT("Nimbus 4D PIL"), 652, 303, 99, -0.46, 133.73, -0.78, 170.07, -1.5, 17.8},
  {TEXT("Nimbus 4"), 597, 303, 85.1, -0.41, 127.98, -0.75, 162.74, -1.4, 17.8},
  {TEXT("PIK-20B"), 354, 144, 102.5, -0.69, 157.76, -1.59, 216.91, -3.6, 10.0},
  {TEXT("PIK-20D"), 348, 144, 100, -0.69, 156.54, -1.78, 215.24, -4.2, 10.0},
  {TEXT("PIK-20E"), 437, 80, 109.61, -0.83, 166.68, -2, 241.15, -4.7, 10.0},
  {TEXT("PIK-30M"), 460, 0, 123.6, -0.78, 152.04, -1.12, 200.22, -2.2, 10.63},
  {TEXT("PW-5 Smyk"), 300, 0, 99.5, -0.95, 158.48, -2.85, 198.1, -5.1, 10.16},
  {TEXT("Russia AC-4"), 250, 0, 99.3, -0.92, 140.01, -1.8, 170.01, -2.9, 7.70},
  {TEXT("Stemme S-10 PAS"), 850, 0, 133.47, -0.83, 167.75, -1.41, 205.03, -2.3, 18.70},
  {TEXT("Stemme S-10 PIL"), 759, 0, 125.8, -0.82, 158.51, -1.33, 193.74, -2.2, 18.70},
  {TEXT("SZD-55-1"), 350, 200, 100.0, -0.66, 120, -0.86, 150, -1.4, 9.60},
  {TEXT("Ventus A/B (16.6m)"), 358, 151, 100.17, -0.64, 159.69, -1.47, 239.54, -4.3, 9.96},
  {TEXT("Ventus B (15m)"), 341, 151, 97.69, -0.68, 156.3, -1.46, 234.45, -3.9, 9.51},
  {TEXT("Ventus 2C (18m)"), 385, 180, 80.0, -0.5, 120.0, -0.73, 180.0, -2.0, 11.03},
  {TEXT("Ventus 2Cx (18m)"), 385, 215, 80.0, -0.5, 120.0, -0.73, 180.0, -2.0, 11.03},
  {TEXT("Zuni II"), 358, 182, 110, -0.88, 167, -2.21, 203.72, -3.6, 10.13},
  {TEXT("Speed Astir"),351,  90,  90, -0.63, 105, -0.72, 157, -2.00, 11.5},   // BestLD40@105
  {TEXT("LS-6-18W"),   330, 140,  90, -0.51, 100, -0.57, 183, -2.00, 11.4},   // BestLD48@100
  {TEXT("LS-8-15"),    325, 185,  70, -0.51, 115, -0.85, 173, -2.00, 10.5},   // BestLD42.5@97kph
  {TEXT("LS-8-18"),    325, 185,  80, -0.51,  94, -0.56, 173, -2.00, 11.4},   // BestLD48
  {TEXT("ASH-26E"),    435,  90,  90, -0.51,  96, -0.53, 185, -2.00, 11.7},   // BestLD50@96kph
  {TEXT("ASG29-18"),   355, 225,  85, -0.47,  90, -0.48, 185, -2.00, 10.5},   // BestLD52@90kph
  {TEXT("ASW28-18"),   345, 190,  65, -0.47, 107, -0.67, 165, -2.00, 10.5},   // BestLD48@90kph
  {TEXT("LS-6-15"),    327, 160,  90, -0.6,  100, -0.658, 183, -1.965, 10.53},   // BestLD42@?
  {TEXT("ASG29E-18"),   400, 200,  90, -0.499,  95.5, -0.510, 196.4, -2.12, 10.5},   // BestLD52@90kph
  {TEXT("Ventus CM (17.6m)"),   430, 0,  100.17, -0.6,  159.7, -1.32, 210.54, -2.5, 10.14}, 
  {TEXT("Duo Discus XT (PIL)"), 580,	170,	100,	-0.605,	150,	-1.271,	200,	-2.668, 16.40},
  {TEXT("Duo Discus XT (PAS)"), 700,	50,	110,	-0.664,	155,	-1.206,	200,	-2.287, 16.40},
  {TEXT("Lak17A-18"), 298,	180,	115,	-0.680,	158,	-1.379,	200,	-2.975, 9.80},
  {TEXT("Lak17A-15"), 285,	180,	95,	-0.574,	148,	-1.310,	200,	-2.885, 9.06},
  {TEXT("DG-300"), 340,	65,	95.0,	-0.65,	140.0,	-1.29,	160.0,	-1.84, 10.27},
  {TEXT("Para EN A/DHV1"), 100,	0, 29.0, -1.1,	34.0,	-1.3,	44.0,	-2.30, 0},
  {TEXT("Para EN B/DHV12"), 100, 0, 29.5, -1.1,	37.0,	-1.2,	50.0,	-2.30, 0},
  {TEXT("Para EN C/DHV2"), 100,	0, 33.0, -1.1,	39.0,	-1.2,	56.0,	-2.30, 0},
  {TEXT("Para EN D/DHV23"), 100, 0, 33.0, -1.1,	41.0,	-1.2,	58.0,	-2.30,  0},
  {TEXT("Para Competition"), 100, 0, 39.0, -1.0, 45.0,	-1.1,	64.0,	-2.30,  0},
  {TEXT("Delta USHPA-2"), 100, 0, 30, -1.10, 44.3,	-1.52,	58.0,	-3.60,  0}, // 091217
  {TEXT("Delta USHPA-3"), 100, 0, 37, -0.95, 48.1,	-1.15,	73.0,	-3.60,  0}, // 091217
  {TEXT("Delta USHPA-4"), 100, 0, 37, -0.89, 48.3,	-1.02,	76.5,	-3.30,  0}, // 091217
  {TEXT("ASG29-15"), 362,	165,	108.8,	-0.635,	156.4,	-1.182,	211.13,	-2.540, 9.20},
  {TEXT("Ventus 2CT (18m)"), 410, 110, 110.0, -0.62, 150.0, -1.2, 214.0, -2.3, 11.03},
  {TEXT("Ventus 2CxT (18m)"), 470, 130, 100.0, -0.56, 150.0, -1.13, 200.0, -2.28, 11.03},
  {TEXT("Blanik L13"), 472, 0, 85.0, -0.84, 143.0, -3.32, 200.0, -9.61, 19.1},
  {TEXT("Blanik L23"), 510, 0, 95.0, -0.94, 148.0, -2.60, 200.0, -6.37, 19.1},
  {TEXT("DG1000-20M (PIL)"), 490, 160, 100.0, -0.59, 120.0, -0.86, 150.0, -1.65, 17.51},
  {TEXT("DG1000-20M (PAS)"), 613, 160, 106.0, -0.62, 153.0, -1.53, 200.0, -3.2, 17.51},
  {TEXT("Mosquito"), 450, 0, 100.0, -0.68, 120.0, -0.92, 150.0, -1.45, 9.85},
  {TEXT("DuoDiscus T (PAS)"), 615, 80, 103, -0.64, 152, -1.25, 200, -2.51, 16.4},
  {TEXT("Jantar Std 48-2"), 375, 150, 100, -0.73, 120, -0.95, 150, -1.60, 10.66},
  {TEXT("Mini Nimbus"), 345, 155, 100, -0.69, 120, -0.92, 150, -1.45, 9.86},
  {TEXT("Pegase 101A"), 344, 120, 85.0, -0.62, 105, -0.75, 175, -2.54, 10.5},

  {TEXT("SZD-30 Pirat"), 340,0,75,-0.7,83,-0.739,150,-2.46,13.8},
  {TEXT("SZD-48-3 Jantar 3"),  326,150,95,-0.66,180,-2.24,220,-3.85,10.66},
  {TEXT("SZD-51-1 Junior"), 333,0,70,-0.58,130,-1.6,180,-3.6,12.51},
  {TEXT("SZD-9bis 1E Bocian"), 540,0,70,-0.83,90,-1.00,140,-2.53,20},
  }; //   0-x
  // 101219 Total: 112
  // 101212 Total: 108


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

////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:    DrawWireRects
//
// DESCRIPTION: Creates exploding wire rectanges
//
// INPUTS:  LPRECT lprcFrom      Source Rectangle
//          LPRECT lprcTo        Destination Rectangle
//          UINT nMilliSecSpeed  Speed in millisecs for animation
//
// RETURN:    None
// NOTES:    None
//
//  Maintenance Log
//  Author      Date    Version     Notes
//  NT Almond   011199  1.0         Origin
//  CJ Maunder  010899  1.1         Modified rectangle transition code
//
/////////////////////////////////////////////////////////////////////////

static RECT AnimationRectangle = {0,0,0,0};

void SetSourceRectangle(RECT fromRect) {
  AnimationRectangle = fromRect;
}


RECT WINAPI DrawWireRects(LPRECT lprcTo, UINT nMilliSecSpeed)
{
  if (!EnableAnimation)
    return AnimationRectangle;

  LPRECT lprcFrom = &AnimationRectangle;
  const int nNumSteps = 10;
  
  GdiFlush();
  Sleep(10);  // Let the desktop window sort itself out
  
  // if hwnd is null - "you have the CON".
  HDC hDC = ::GetDC(NULL);
  
  // Pen size, urmmm not too thick
  HPEN hPen = ::CreatePen(PS_SOLID, 2, RGB(0,0,0));
  
  int nMode = ::SetROP2(hDC, R2_NOT);
  HPEN hOldPen = (HPEN) ::SelectObject(hDC, hPen);
  
  for (int i = 0; i < nNumSteps; i++)
    {
      double dFraction = (double) i / (double) nNumSteps;
      
      RECT transition;
      transition.left   = lprcFrom->left + 
	(int)((lprcTo->left - lprcFrom->left) * dFraction);
      transition.right  = lprcFrom->right + 
	(int)((lprcTo->right - lprcFrom->right) * dFraction);
      transition.top    = lprcFrom->top + 
	(int)((lprcTo->top - lprcFrom->top) * dFraction);
      transition.bottom = lprcFrom->bottom + 
	(int)((lprcTo->bottom - lprcFrom->bottom) * dFraction);
      
      POINT pt[5];
      pt[0].x = transition.left; pt[0].y= transition.top;
      pt[1].x = transition.right; pt[1].y= transition.top;
      pt[2].x = transition.right; pt[2].y= transition.bottom;
      pt[3].x = transition.left; pt[3].y= transition.bottom;
      pt[4].x = transition.left; pt[4].y= transition.top;
      
      // We use Polyline because we can determine our own pen size
      // Draw Sides
      ::Polyline(hDC,pt,5);

      GdiFlush();
      
      Sleep(nMilliSecSpeed);
      
      // UnDraw Sides
      ::Polyline(hDC,pt,5);
      
      GdiFlush();
    }
  
  ::SetROP2(hDC, nMode);
  ::SelectObject(hDC, hOldPen);
  ::DeleteObject(hPen);
  ::ReleaseDC(NULL,hDC);
  return AnimationRectangle;
}

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
  
#ifdef FLARMNET
  // try to find flarm from FLARMNet.org File
  FlarmId* flarmId = file.GetFlarmIdItem(id);
  if (flarmId != NULL)
    {
      return flarmId->cn;
    }
#endif
  return NULL;
}

TCHAR* LookupFLARMDetails(long id) {
  
  // try to find flarm from userFile
  int index = LookupSecondaryFLARMId(id);
  if (index != -1)
    {
      return FLARM_Names[index].Name;
    }
  
#ifdef FLARMNET
  // try to find flarm from FLARMNet.org File
  FlarmId* flarmId = file.GetFlarmIdItem(id);
  if (flarmId != NULL)
    {
      // return flarmId->cn;
      return flarmId->reg;
    }
#endif
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
  
#ifdef FLARMNET
  // try to find flarm from FLARMNet.org File
  FlarmId* flarmId = file.GetFlarmIdItem(cn);
  if (flarmId != NULL)
    {
      return flarmId->GetId();
    }
#endif  
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
#if (WINDOWSPC>0)||(defined(GNAV) && !defined(__MINGW32__))
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
  BOOL bRtn;
  LPTSTR lpRes;
  HANDLE hResInfo, hRes;

  // TODO code: Modify to allow use of WAV Files and/or Embedded files

  if (wcsstr(lpName, TEXT(".wav"))) {
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

bool FileExistsW(TCHAR *FileName){

  HANDLE hFile = CreateFileW(FileName, GENERIC_READ, 0, NULL,
                 OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

  if( hFile == INVALID_HANDLE_VALUE)
    return(FALSE);

  CloseHandle(hFile);

  return(TRUE);

}

bool FileExistsA(char *FileName){

#if (WINDOWSPC>0)
  HANDLE hFile = CreateFileA(FileName, GENERIC_READ, 0, NULL,
                 OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if( hFile == INVALID_HANDLE_VALUE)
    return(FALSE);

  CloseHandle(hFile);

  return(TRUE);
#else
  FILE *file = fopen(FileName, "r");
  if (file != NULL) {
    fclose(file);
    return(TRUE);
  }
  return FALSE;
#endif

}



bool RotateScreen() {
#if (WINDOWSPC>0)
  return false;
#else 
  //
  // Change the orientation of the screen
  //
#ifdef GNAV  
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


