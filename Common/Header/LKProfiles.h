/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef LKPROFILES_H
#define LKPROFILES_H

#if defined(STATIC_PGLOBALS)

//
// >>> PLEASE KEEP ALPHA ORDER , BE TIDY! <<<
// 
// If you want to force a reset of a parameter in an old configuration,
// maybe because the software has changed and this old parameter must be
// manually reconfigured by the user, there is a simple solution:
// change the name of the parameter here, adding a number to keep versioning.
// This will not load the old parameter on Profileload, and will not save it of course.
// You are in fact ignoring that old profile token, which will disappear.
//

const TCHAR *szRegistryDisplayType[MAXINFOWINDOWS] =     { TEXT("Info0"),
				       TEXT("Info1"),
				       TEXT("Info2"),
				       TEXT("Info3"),
				       TEXT("Info4"),
				       TEXT("Info5"),
				       TEXT("Info6"),
				       TEXT("Info7"),
				       TEXT("Info8"),
				       TEXT("Info9"),
				       TEXT("Info10"),
				       TEXT("Info11"),
				       TEXT("Info12"),
				       TEXT("Info13")
};

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
                  TEXT("Colour15"),
                  TEXT("Colour16")
};


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
                  TEXT("Brush15"),
                  TEXT("Brush16")
};

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
                           TEXT("AirspaceMode15"),
                           TEXT("AirspaceMode16")
};

TCHAR szRegistryAcknowledgementTime[]=	 TEXT("AcknowledgementTime1");
TCHAR szRegistryAdditionalAirspaceFile[]=  TEXT("AdditionalAirspaceFile");
TCHAR szRegistryAdditionalWayPointFile[]=  TEXT("AdditionalWPFile");
TCHAR szRegistryAircraftCategory[]= TEXT("AircraftCategory1");
TCHAR szRegistryAircraftRego[]=  TEXT("AircraftRego1");
TCHAR szRegistryAircraftType[]=  TEXT("AircraftType1");
TCHAR szRegistryAirfieldFile[]=  TEXT("AirfieldFile");
TCHAR szRegistryAirspaceFile[]=  TEXT("AirspaceFile");
TCHAR szRegistryAirspaceFillType[]= TEXT("AirspaceFillType");
TCHAR szRegistryAirspaceOpacity[]= TEXT("AirspaceOpacity");
TCHAR szRegistryAirspaceWarningDlgTimeout[]= TEXT("AirspaceWarningDlgTimeout");
TCHAR szRegistryAirspaceWarningMapLabels[]= TEXT("AirspaceWarningMapLabels");
TCHAR szRegistryAirspaceAckAllSame[]= TEXT("AirspaceAckAllSame");
TCHAR szRegistryAirspaceWarningRepeatTime[]= TEXT("AirspaceWarningRepeatTime1");
TCHAR szRegistryAirspaceWarningVerticalMargin[]= TEXT("AirspaceWarningVerticalMargin1");
TCHAR szRegistryAirspaceWarning[]= TEXT("AirspaceWarn");
TCHAR szRegistryAlarmMaxAltitude1[]= TEXT("AlarmMaxAltitude1");
TCHAR szRegistryAlarmMaxAltitude2[]= TEXT("AlarmMaxAltitude3");
TCHAR szRegistryAlarmMaxAltitude3[]= TEXT("AlarmMaxAltitude4");
TCHAR szRegistryAlarmTakeoffSafety[]= TEXT("AlarmTakeoffSafety");
TCHAR szRegistryAltMargin[]=	   TEXT("AltMargin1");
TCHAR szRegistryAltMode[]=  TEXT("AltitudeMode");
TCHAR szRegistryAlternate1[]= TEXT("Alternate1b");
TCHAR szRegistryAlternate2[]= TEXT("Alternate2b");
TCHAR szRegistryAltitudeUnitsValue[] = TEXT("AltitudeUnits");
TCHAR szRegistryAppDefaultMapWidth[] = TEXT("AppDefaultMapWidth");
TCHAR szRegistryAppIndLandable[] = TEXT("AppIndLandable1");
TCHAR szRegistryAppInfoBoxModel[] = TEXT("AppInfoBoxModel"); 
TCHAR szRegistryAppInverseInfoBox[] = TEXT("AppInverseInfoBox2");
TCHAR szRegistryArrivalValue[]= TEXT("ArrivalValue");
TCHAR szRegistryAutoAdvance[] = TEXT("AutoAdvance");
TCHAR szRegistryAutoBacklight[]= TEXT("AutoBacklight");
TCHAR szRegistryAutoForceFinalGlide[] = TEXT("AutoForceFinalGlide");
TCHAR szRegistryMacCready[] = TEXT("MacCready");
TCHAR szRegistryAutoMcMode[] = TEXT("AutoMcMode");
TCHAR szRegistryAutoMcStatus[] = TEXT("AutoMcStatus");
TCHAR szRegistryAutoOrientScale[]= TEXT("AutoOrientScale3");
TCHAR szRegistryAutoSoundVolume[]= TEXT("AutoSoundVolume");
TCHAR szRegistryAutoWind[]= TEXT("AutoWind");
TCHAR szRegistryAutoZoom[] = TEXT("AutoZoom");
TCHAR szRegistryAverEffTime[]= TEXT("AverEffTime1");
TCHAR szRegistryBallastSecsToEmpty[]=	 TEXT("BallastSecsToEmpty1"); 
TCHAR szRegistryBarOpacity[]= TEXT("BarOpacity");
TCHAR szRegistryBestWarning[]= TEXT("BestWarning");
TCHAR szRegistryBgMapColor[]= TEXT("BgMapColor");
TCHAR szRegistryBit1Index[]=		 TEXT("Bit1Index");
TCHAR szRegistryBit2Index[]=		 TEXT("Bit2Index");
TCHAR szRegistryBugs[]=		 TEXT("Bugs");
TCHAR szRegistryCheckSum[]=		 TEXT("CheckSum");
TCHAR szRegistryCircleZoom[]= TEXT("CircleZoom");
TCHAR szRegistryClipAlt[]= TEXT("ClipAlt1");
TCHAR szRegistryCompetitionClass[]=  TEXT("CompetitionClass1");
TCHAR szRegistryCompetitionID[]=  TEXT("CompetitionID1");
TCHAR szRegistryConfBB0[] = TEXT("ConfBB0a");
TCHAR szRegistryConfBB1[] = TEXT("ConfBB1");
TCHAR szRegistryConfBB2[] = TEXT("ConfBB2");
TCHAR szRegistryConfBB3[] = TEXT("ConfBB3");
TCHAR szRegistryConfBB4[] = TEXT("ConfBB4");
TCHAR szRegistryConfBB5[] = TEXT("ConfBB5");
TCHAR szRegistryConfBB6[] = TEXT("ConfBB6");
TCHAR szRegistryConfBB7[] = TEXT("ConfBB7");
TCHAR szRegistryConfBB8[] = TEXT("ConfBB8");
TCHAR szRegistryConfBB9[] = TEXT("ConfBB9");
TCHAR szRegistryConfBB0Auto[] = TEXT("ConfBB0Auto");
TCHAR szRegistryConfIP11[] = TEXT("ConfIP11");
TCHAR szRegistryConfIP12[] = TEXT("ConfIP12");
TCHAR szRegistryConfIP13[] = TEXT("ConfIP13");
TCHAR szRegistryConfIP14[] = TEXT("ConfIP14");
TCHAR szRegistryConfIP15[] = TEXT("ConfIP15");
TCHAR szRegistryConfIP16[] = TEXT("ConfIP16");
TCHAR szRegistryConfIP17[] = TEXT("ConfIP17");
TCHAR szRegistryConfIP21[] = TEXT("ConfIP21");
TCHAR szRegistryConfIP22[] = TEXT("ConfIP22");
TCHAR szRegistryConfIP23[] = TEXT("ConfIP23");
TCHAR szRegistryConfIP24[] = TEXT("ConfIP24");
TCHAR szRegistryConfIP31[] = TEXT("ConfIP31");
TCHAR szRegistryConfIP32[] = TEXT("ConfIP32");
TCHAR szRegistryConfIP33[] = TEXT("ConfIP33");
TCHAR szRegistryCustomKeyModeAircraftIcon[] = TEXT("CustomKeyModeAircraftIcon");
TCHAR szRegistryCustomKeyModeCenterScreen[] = TEXT("CustomKeyModeCenterScreen");
TCHAR szRegistryCustomKeyModeCenter[] = TEXT("CustomKeyModeCenter");
TCHAR szRegistryCustomKeyModeLeftUpCorner[] = TEXT("CustomKeyModeLeftUpCorner");
TCHAR szRegistryCustomKeyModeLeft[] = TEXT("CustomKeyModeLeft");
TCHAR szRegistryCustomKeyModeRightUpCorner[] = TEXT("CustomKeyModeRightUpCorner");
TCHAR szRegistryCustomKeyModeRight[] = TEXT("CustomKeyModeRight");
TCHAR szRegistryCustomKeyTime[] = TEXT("CustomKeyTime");
TCHAR szRegistryDebounceTimeout[]= TEXT("DebounceTimeout");
TCHAR szRegistryDeclutterMode[]= TEXT("DeclutterMode");
TCHAR szRegistryDeviceA[]= TEXT("DeviceA");
TCHAR szRegistryDeviceB[]= TEXT("DeviceB");
TCHAR szRegistryDisableAutoLogger[] = TEXT("DisableAutoLogger");
TCHAR szRegistryLiveTrackerInterval[] = TEXT("LiveTrackerInterval");
TCHAR szRegistryDisplayText[] = TEXT("DisplayText2");
TCHAR szRegistryDisplayUpValue[] = TEXT("DisplayUp");
TCHAR szRegistryDistanceUnitsValue[] = TEXT("DistanceUnits");
TCHAR szRegistryEnableFLARMMap[] = TEXT("EnableFLARMDisplay1");
TCHAR szRegistryEnableNavBaroAltitude[] = TEXT("EnableNavBaroAltitude");
TCHAR szRegistryFAIFinishHeight[] = TEXT("FAIFinishHeight");
TCHAR szRegistryFAISector[] = TEXT("FAISector");
TCHAR szRegistryFinalGlideTerrain[]= TEXT("FinalGlideTerrain");
TCHAR szRegistryFinishLine[]=		 TEXT("FinishLine");
TCHAR szRegistryFinishMinHeight[]= TEXT("FinishMinHeight");
TCHAR szRegistryFinishRadius[]=		 TEXT("FinishRadius");
TCHAR szRegistryFontMapLabelFont[]=	 TEXT("MapLabelFont"); 
TCHAR szRegistryFontMapWindowFont[]=	 TEXT("MapWindowFont"); 
TCHAR szRegistryFontRenderer[]= TEXT("FontRenderer2");
TCHAR szRegistryGlideBarMode[]= TEXT("GlideBarMode");
TCHAR szRegistryGliderScreenPosition[] = TEXT("GliderScreenPosition");
TCHAR szRegistryGpsAltitudeOffset[] = TEXT("GpsAltitudeOffset");
TCHAR szRegistryHandicap[] = TEXT("Handicap1");
TCHAR szRegistryHideUnits[]= TEXT("HideUnits");
TCHAR szRegistryHomeWaypoint[]= TEXT("HomeWaypoint1b");
TCHAR szRegistryInputFile[]=  TEXT("InputFile");
TCHAR szRegistryIphoneGestures[]= TEXT("IphoneGestures");
TCHAR szRegistryLKMaxLabels[]= TEXT("LKMaxLabels");
TCHAR szRegistryLKTopoZoomCat05[]= TEXT("LKTopoZoomCat05d");
TCHAR szRegistryLKTopoZoomCat100[]= TEXT("LKTopoZoomCat100a");
TCHAR szRegistryLKTopoZoomCat10[]= TEXT("LKTopoZoomCat10d");
TCHAR szRegistryLKTopoZoomCat110[]= TEXT("LKTopoZoomCat110a");
TCHAR szRegistryLKTopoZoomCat20[]= TEXT("LKTopoZoomCat20a");
TCHAR szRegistryLKTopoZoomCat30[]= TEXT("LKTopoZoomCat30a");
TCHAR szRegistryLKTopoZoomCat40[]= TEXT("LKTopoZoomCat40a");
TCHAR szRegistryLKTopoZoomCat50[]= TEXT("LKTopoZoomCat50a");
TCHAR szRegistryLKTopoZoomCat60[]= TEXT("LKTopoZoomCat60a");
TCHAR szRegistryLKTopoZoomCat70[]= TEXT("LKTopoZoomCat70a");
TCHAR szRegistryLKTopoZoomCat80[]= TEXT("LKTopoZoomCat80a");
TCHAR szRegistryLKTopoZoomCat90[]= TEXT("LKTopoZoomCat90a");
TCHAR szRegistryLKVarioBar[]= TEXT("LKVarioBar");
TCHAR szRegistryLKVarioVal[]= TEXT("LKVarioVal");
TCHAR szRegistryLanguageFile[]=  TEXT("LanguageFile");
TCHAR szRegistryLatLonUnits[] = TEXT("LatLonUnits");
TCHAR szRegistryLiftUnitsValue[] = TEXT("LiftUnits");
TCHAR szRegistryLockSettingsInFlight[] = TEXT("LockSettingsInFlight");
TCHAR szRegistryLoggerShort[]=  TEXT("LoggerShortName");
TCHAR szRegistryLoggerTimeStepCircling[]= TEXT("LoggerTimeStepCircling");
TCHAR szRegistryLoggerTimeStepCruise[]= TEXT("LoggerTimeStepCruise");
TCHAR szRegistryLook8000[]= TEXT("Look8000a");
TCHAR szRegistryMapBox[]= TEXT("MapBox");
TCHAR szRegistryMapFile[]= TEXT("MapFile");
TCHAR szRegistryMcOverlay[]= TEXT("McOverlay2");
TCHAR szRegistryMenuTimeout[] = TEXT("MenuTimeout");
TCHAR szRegistryNewMapDeclutter[]= TEXT("NewMapDeclutter");
TCHAR szRegistryOrbiter[] = TEXT("Orbiter");
TCHAR szRegistryOutlinedTp[]= TEXT("OutlinedTp1");
TCHAR szRegistryOverColor[]= TEXT("OverColor");
TCHAR szRegistryOverlayClock[] = TEXT("OverlayClock");
TCHAR szRegistrySonarWarning[] = TEXT("SonarWarning");
TCHAR szRegistryOverlaySize[]= TEXT("OverlaySize");
TCHAR szRegistryPGAutoZoomThreshold[]= TEXT("PGAutoZoomThreshold1");
TCHAR szRegistryPGClimbZoom[]= TEXT("PGClimbZoom");
TCHAR szRegistryPGCruiseZoom[]= TEXT("PGCruiseZoom");
TCHAR szRegistryPGOptimizeRoute[]= TEXT("PGOptimizeRoute");
TCHAR szRegistryPilotName[]=  TEXT("PilotName1");
TCHAR szRegistryLiveTrackersrv[]=  TEXT("LiveTrackersrv");
TCHAR szRegistryLiveTrackerport[]=  TEXT("LiveTrackerport");
TCHAR szRegistryLiveTrackerusr[]=  TEXT("LiveTrackerusr");
TCHAR szRegistryLiveTrackerpwd[]=  TEXT("LiveTrackerpwd");
TCHAR szRegistryPolarFile[] = TEXT("PolarFile1");
TCHAR szRegistryPollingMode[]= TEXT("PollingMode");
TCHAR szRegistryPort1Index[]= TEXT("PortIndex");
TCHAR szRegistryPort2Index[]= TEXT("Port2Index");
TCHAR szRegistryPort1Name[]= TEXT("Port1Name");
TCHAR szRegistryPort2Name[]= TEXT("Port2Name");
TCHAR szRegistryPressureHg[] = TEXT("PressureHg");
TCHAR szRegistrySafetyAltitudeArrival[] =     TEXT("SafetyAltitudeArrival1");
TCHAR szRegistrySafetyAltitudeMode[]=  TEXT("SafetyAltitudeMode");
TCHAR szRegistrySafetyAltitudeTerrain[] =     TEXT("SafetyAltitudeTerrain1");
TCHAR szRegistrySafetyMacCready[] = TEXT("SafetyMacCready");
TCHAR szRegistrySafteySpeed[] =          TEXT("SafteySpeed1");
TCHAR szRegistrySectorRadius[]=          TEXT("Radius");
TCHAR szRegistrySetSystemTimeFromGPS[] = TEXT("SetSystemTimeFromGPS");
TCHAR szRegistryShading[] = TEXT("Shading");
TCHAR szRegistrySnailTrail[]=		 TEXT("SnailTrail");
TCHAR szRegistrySnailWidthScale[] = TEXT("SnailWidthScale");
TCHAR szRegistrySpeed1Index[]=		 TEXT("SpeedIndex");
TCHAR szRegistrySpeed2Index[]=		 TEXT("Speed2Index");
TCHAR szRegistrySpeedUnitsValue[] =      TEXT("SpeedUnits");
TCHAR szRegistryStartHeightRef[] = TEXT("StartHeightRef");
TCHAR szRegistryStartLine[]=		 TEXT("StartLine");
TCHAR szRegistryStartMaxHeightMargin[]= TEXT("StartMaxHeightMargin");
TCHAR szRegistryStartMaxHeight[]= TEXT("StartMaxHeight");
TCHAR szRegistryStartMaxSpeedMargin[]= TEXT("StartMaxSpeedMargin");
TCHAR szRegistryStartMaxSpeed[]= TEXT("StartMaxSpeed");
TCHAR szRegistryStartRadius[]=		 TEXT("StartRadius");
TCHAR szRegistryTaskSpeedUnitsValue[] =      TEXT("TaskSpeedUnits");
TCHAR szRegistryTeamcodeRefWaypoint[] = TEXT("TeamcodeRefWaypoint1");
TCHAR szRegistryTerrainBrightness[] = TEXT("TerrainBrightness1");
TCHAR szRegistryTerrainContrast[] = TEXT("TerrainContrast1");
TCHAR szRegistryTerrainFile[]=	 TEXT("TerrainFile");
TCHAR szRegistryTerrainRamp[] = TEXT("TerrainRamp");
TCHAR szRegistryThermalBar[]= TEXT("ThermalBar");
TCHAR szRegistryThermalLocator[]=	 TEXT("ThermalLocator");
TCHAR szRegistryTpFilter[]= TEXT("TpFilter");
TCHAR szRegistryTrackBar[]= TEXT("TrackBar");
TCHAR szRegistryTrailDrift[]=		 TEXT("TrailDrift");
TCHAR szRegistryUTCOffset[] = TEXT("UTCOffset");
TCHAR szRegistryUseCustomFonts[]=	 TEXT("UseCustomFonts"); 
TCHAR szRegistryUseGeoidSeparation[] = TEXT("UseGeoidSeparation");
TCHAR szRegistryUseUngestures[] = TEXT("UseUngestures");
TCHAR szRegistryUseTotalEnergy[] = TEXT("UseTotalEnergy");
TCHAR szRegistryWarningTime[]=		 TEXT("WarnTime");
TCHAR szRegistryWayPointFile[]=  TEXT("WPFile");
TCHAR szRegistryWaypointsOutOfRange[] = TEXT("WaypointsOutOfRange2");
TCHAR szRegistryWindCalcSpeed[] =          TEXT("WindCalcSpeed");
TCHAR szRegistryWindCalcTime[] =          TEXT("WindCalcTime");
TCHAR szRegistryCustomMenu1[] = _T("CustomMenu1a");
TCHAR szRegistryCustomMenu2[] = _T("CustomMenu2a");
TCHAR szRegistryCustomMenu3[] = _T("CustomMenu3a");
TCHAR szRegistryCustomMenu4[] = _T("CustomMenu4a");
TCHAR szRegistryCustomMenu5[] = _T("CustomMenu5a");
TCHAR szRegistryCustomMenu6[] = _T("CustomMenu6a");
TCHAR szRegistryCustomMenu7[] = _T("CustomMenu7a");
TCHAR szRegistryCustomMenu8[] = _T("CustomMenu8a");
TCHAR szRegistryCustomMenu9[] = _T("CustomMenu9a");
TCHAR szRegistryCustomMenu10[] = _T("CustomMenu10a");
TCHAR szRegistryUseWindRose[] = _T("UseWindRose");
TCHAR szRegistryMultiTerr0[] = _T("MultimapTerrain0");
TCHAR szRegistryMultiTerr1[] = _T("MultimapTerrain1");
TCHAR szRegistryMultiTerr2[] = _T("MultimapTerrain2");
TCHAR szRegistryMultiTerr3[] = _T("MultimapTerrain3");
TCHAR szRegistryMultiTerr4[] = _T("MultimapTerrain4");
TCHAR szRegistryMultiTopo0[] = _T("MultimapTopology0");
TCHAR szRegistryMultiTopo1[] = _T("MultimapTopology1");
TCHAR szRegistryMultiTopo2[] = _T("MultimapTopology2");
TCHAR szRegistryMultiTopo3[] = _T("MultimapTopology3");
TCHAR szRegistryMultiTopo4[] = _T("MultimapTopology4");
TCHAR szRegistryMultiAsp0[] = _T("MultimapAirspace0");
TCHAR szRegistryMultiAsp1[] = _T("MultimapAirspace1");
TCHAR szRegistryMultiAsp2[] = _T("MultimapAirspace2");
TCHAR szRegistryMultiAsp3[] = _T("MultimapAirspace3");
TCHAR szRegistryMultiAsp4[] = _T("MultimapAirspace4");
TCHAR szRegistryMultiLab0[] = _T("MultimapLab0");
TCHAR szRegistryMultiLab1[] = _T("MultimapLab1");
TCHAR szRegistryMultiLab2[] = _T("MultimapLab2");
TCHAR szRegistryMultiLab3[] = _T("MultimapLab3");
TCHAR szRegistryMultiLab4[] = _T("MultimapLab4");
TCHAR szRegistryMultiWpt0[] = _T("MultimapWpt0");
TCHAR szRegistryMultiWpt1[] = _T("MultimapWpt1");
TCHAR szRegistryMultiWpt2[] = _T("MultimapWpt2");
TCHAR szRegistryMultiWpt3[] = _T("MultimapWpt3");
TCHAR szRegistryMultiWpt4[] = _T("MultimapWpt4");
TCHAR szRegistryMultiOvrT0[] = _T("MultimapOvrT0");
TCHAR szRegistryMultiOvrT1[] = _T("MultimapOvrT1");
TCHAR szRegistryMultiOvrT2[] = _T("MultimapOvrT2");
TCHAR szRegistryMultiOvrT3[] = _T("MultimapOvrT3");
TCHAR szRegistryMultiOvrT4[] = _T("MultimapOvrT4");
TCHAR szRegistryMultiOvrG0[] = _T("MultimapOvrG0");
TCHAR szRegistryMultiOvrG1[] = _T("MultimapOvrG1");
TCHAR szRegistryMultiOvrG2[] = _T("MultimapOvrG2");
TCHAR szRegistryMultiOvrG3[] = _T("MultimapOvrG3");
TCHAR szRegistryMultiOvrG4[] = _T("MultimapOvrG4");
TCHAR szRegistryMultiSizeY1[]= _T("MultimapSizeY1");
TCHAR szRegistryMultiSizeY2[]= _T("MultimapSizeY2");
TCHAR szRegistryMultiSizeY3[]= _T("MultimapSizeY3");
TCHAR szRegistryMultiSizeY4[]= _T("MultimapSizeY4");

TCHAR szRegistryMultimap1[]= _T("Multimap1a");
TCHAR szRegistryMultimap2[]= _T("Multimap2a");
TCHAR szRegistryMultimap3[]= _T("Multimap3a");
TCHAR szRegistryMultimap4[]= _T("Multimap4a");

TCHAR szRegistryMMNorthUp1[]= _T("MultimapNorthUp1");
TCHAR szRegistryMMNorthUp2[]= _T("MultimapNorthUp2");
TCHAR szRegistryMMNorthUp3[]= _T("MultimapNorthUp3");
TCHAR szRegistryMMNorthUp4[]= _T("MultimapNorthUp4");

TCHAR szRegistryAspPermanent[]     = _T("AirspacePermMod");
TCHAR szRegistryFlarmDirection[]   = _T("FlarmDirection");
TCHAR szRegistryDrawTask[]         = _T("DrawTask");
TCHAR szRegistryDrawFAI[]          = _T("DrawFAI");
TCHAR szRegistryGearMode[]         = _T("GearMode");
TCHAR szRegistryGearAltitude[]     = _T("GearAltitude");
TCHAR szRegistryBottomMode[]       = _T("ActiveBottomBar");
TCHAR szRegistryBigFAIThreshold[]  = _T("FAI_28_45_Threshold");


TCHAR szRegistryScreenSize[]       = _T("ScreenSize");
TCHAR szRegistryScreenSizeX[]      = _T("ScreenSizeX");
TCHAR szRegistryScreenSizeY[]      = _T("ScreenSizeY");

//
//
//
#else 
//
// ------------------------------------- externals ------------------------------------------
//
extern const TCHAR *szRegistryAirspaceMode[];	// 17
extern const TCHAR *szRegistryBrush[];	// 17
extern const TCHAR *szRegistryColour[];	// 17
extern const TCHAR *szRegistryDisplayType[];	// MAXINFOWINDOWS
extern const TCHAR szRegistryAcknowledgementTime[];
extern const TCHAR szRegistryAdditionalAirspaceFile[];
extern const TCHAR szRegistryAdditionalWayPointFile[];
extern const TCHAR szRegistryAircraftCategory[];
extern const TCHAR szRegistryAircraftRego[];
extern const TCHAR szRegistryAircraftType[];
extern const TCHAR szRegistryAirfieldFile[];
extern const TCHAR szRegistryAirspaceFile[];
extern const TCHAR szRegistryAirspaceFillType[];
extern const TCHAR szRegistryAirspaceOpacity[];
extern const TCHAR szRegistryAirspaceWarningDlgTimeout[];
extern const TCHAR szRegistryAirspaceWarningMapLabels[];
extern const TCHAR szRegistryAirspaceAckAllSame[];
extern const TCHAR szRegistryAirspaceWarningRepeatTime[];
extern const TCHAR szRegistryAirspaceWarningVerticalMargin[];
extern const TCHAR szRegistryAirspaceWarning[];
extern const TCHAR szRegistryAlarmMaxAltitude1[];
extern const TCHAR szRegistryAlarmMaxAltitude2[];
extern const TCHAR szRegistryAlarmMaxAltitude3[];
extern const TCHAR szRegistryAlarmTakeoffSafety[];
extern const TCHAR szRegistryAltMargin[];
extern const TCHAR szRegistryAltMode[];
extern const TCHAR szRegistryAlternate1[];
extern const TCHAR szRegistryAlternate2[];
extern const TCHAR szRegistryAltitudeUnitsValue[];
extern const TCHAR szRegistryAppDefaultMapWidth[];
extern const TCHAR szRegistryAppIndLandable[];
extern const TCHAR szRegistryAppInfoBoxModel[];
extern const TCHAR szRegistryAppInverseInfoBox[];
extern const TCHAR szRegistryArrivalValue[];
extern const TCHAR szRegistryAutoAdvance[];
extern const TCHAR szRegistryAutoBacklight[];
extern const TCHAR szRegistryAutoForceFinalGlide[];
extern const TCHAR szRegistryAutoMcMode[];
extern const TCHAR szRegistryMacCready[];
extern const TCHAR szRegistryAutoMcStatus[];
extern const TCHAR szRegistryAutoOrientScale[];
extern const TCHAR szRegistryAutoSoundVolume[];
extern const TCHAR szRegistryAutoWind[];
extern const TCHAR szRegistryAutoZoom[];
extern const TCHAR szRegistryAverEffTime[];
extern const TCHAR szRegistryBallastSecsToEmpty[];
extern const TCHAR szRegistryBarOpacity[];
extern const TCHAR szRegistryBestWarning[];
extern const TCHAR szRegistryBgMapColor[];
extern const TCHAR szRegistryBit1Index[];
extern const TCHAR szRegistryBit2Index[];
extern const TCHAR szRegistryBugs[];
extern const TCHAR szRegistryCheckSum[];
extern const TCHAR szRegistryCircleZoom[];
extern const TCHAR szRegistryClipAlt[];
extern const TCHAR szRegistryCompetitionClass[];
extern const TCHAR szRegistryCompetitionID[];
extern const TCHAR szRegistryConfBB0[];
extern const TCHAR szRegistryConfBB1[];
extern const TCHAR szRegistryConfBB2[];
extern const TCHAR szRegistryConfBB3[];
extern const TCHAR szRegistryConfBB4[];
extern const TCHAR szRegistryConfBB5[];
extern const TCHAR szRegistryConfBB6[];
extern const TCHAR szRegistryConfBB7[];
extern const TCHAR szRegistryConfBB8[];
extern const TCHAR szRegistryConfBB9[];
extern const TCHAR szRegistryConfBB0Auto[];
extern const TCHAR szRegistryConfIP11[];
extern const TCHAR szRegistryConfIP12[];
extern const TCHAR szRegistryConfIP13[];
extern const TCHAR szRegistryConfIP14[];
extern const TCHAR szRegistryConfIP15[];
extern const TCHAR szRegistryConfIP16[];
extern const TCHAR szRegistryConfIP17[];
extern const TCHAR szRegistryConfIP21[];
extern const TCHAR szRegistryConfIP22[];
extern const TCHAR szRegistryConfIP23[];
extern const TCHAR szRegistryConfIP24[];
extern const TCHAR szRegistryConfIP31[];
extern const TCHAR szRegistryConfIP32[];
extern const TCHAR szRegistryConfIP33[];
extern const TCHAR szRegistryCustomKeyModeAircraftIcon[];
extern const TCHAR szRegistryCustomKeyModeCenterScreen[];
extern const TCHAR szRegistryCustomKeyModeCenter[];
extern const TCHAR szRegistryCustomKeyModeLeftUpCorner[];
extern const TCHAR szRegistryCustomKeyModeLeft[];
extern const TCHAR szRegistryCustomKeyModeRightUpCorner[];
extern const TCHAR szRegistryCustomKeyModeRight[];
extern const TCHAR szRegistryCustomKeyTime[];
extern const TCHAR szRegistryDebounceTimeout[];
extern const TCHAR szRegistryDeclutterMode[];
extern const TCHAR szRegistryDeviceA[];
extern const TCHAR szRegistryDeviceB[];
extern const TCHAR szRegistryDisableAutoLogger[];
extern const TCHAR szRegistryLiveTrackerInterval[];
extern const TCHAR szRegistryDisplayText[];
extern const TCHAR szRegistryDisplayUpValue[];
extern const TCHAR szRegistryDistanceUnitsValue[];
extern const TCHAR szRegistryEnableFLARMMap[];
extern const TCHAR szRegistryEnableNavBaroAltitude[];
extern const TCHAR szRegistryFAIFinishHeight[];
extern const TCHAR szRegistryFAISector[];
extern const TCHAR szRegistryFinalGlideTerrain[];
extern const TCHAR szRegistryFinishLine[];
extern const TCHAR szRegistryFinishMinHeight[];
extern const TCHAR szRegistryFinishRadius[];
extern const TCHAR szRegistryFontMapLabelFont[];
extern const TCHAR szRegistryFontMapWindowFont[];
extern const TCHAR szRegistryFontRenderer[];
extern const TCHAR szRegistryGlideBarMode[];
extern const TCHAR szRegistryGliderScreenPosition[];
extern const TCHAR szRegistryGpsAltitudeOffset[];
extern const TCHAR szRegistryHandicap[];
extern const TCHAR szRegistryHideUnits[];
extern const TCHAR szRegistryHomeWaypoint[];
extern const TCHAR szRegistryInputFile[];
extern const TCHAR szRegistryIphoneGestures[];
extern const TCHAR szRegistryLKMaxLabels[];
extern const TCHAR szRegistryLKTopoZoomCat05[];
extern const TCHAR szRegistryLKTopoZoomCat100[];
extern const TCHAR szRegistryLKTopoZoomCat10[];
extern const TCHAR szRegistryLKTopoZoomCat110[];
extern const TCHAR szRegistryLKTopoZoomCat20[];
extern const TCHAR szRegistryLKTopoZoomCat30[];
extern const TCHAR szRegistryLKTopoZoomCat40[];
extern const TCHAR szRegistryLKTopoZoomCat50[];
extern const TCHAR szRegistryLKTopoZoomCat60[];
extern const TCHAR szRegistryLKTopoZoomCat70[];
extern const TCHAR szRegistryLKTopoZoomCat80[];
extern const TCHAR szRegistryLKTopoZoomCat90[];
extern const TCHAR szRegistryLKVarioBar[];
extern const TCHAR szRegistryLKVarioVal[];
extern const TCHAR szRegistryLanguageFile[];
extern const TCHAR szRegistryLatLonUnits[];
extern const TCHAR szRegistryLiftUnitsValue[];
extern const TCHAR szRegistryLockSettingsInFlight[];
extern const TCHAR szRegistryLoggerShort[];
extern const TCHAR szRegistryLoggerTimeStepCircling[];
extern const TCHAR szRegistryLoggerTimeStepCruise[];
extern const TCHAR szRegistryLook8000[];
extern const TCHAR szRegistryMapBox[];
extern const TCHAR szRegistryMapFile[];
extern const TCHAR szRegistryMcOverlay[];
extern const TCHAR szRegistryMenuTimeout[];
extern const TCHAR szRegistryNewMapDeclutter[];
extern const TCHAR szRegistryOrbiter[];
extern const TCHAR szRegistryOutlinedTp[];
extern const TCHAR szRegistryOverColor[];
extern const TCHAR szRegistryOverlayClock[];
extern const TCHAR szRegistrySonarWarning[];
extern const TCHAR szRegistryOverlaySize[];
extern const TCHAR szRegistryPGAutoZoomThreshold[];
extern const TCHAR szRegistryPGClimbZoom[];
extern const TCHAR szRegistryPGCruiseZoom[];
extern const TCHAR szRegistryPGOptimizeRoute[];
extern const TCHAR szRegistryPilotName[];
extern const TCHAR szRegistryLiveTrackersrv[];
extern const TCHAR szRegistryLiveTrackerport[];
extern const TCHAR szRegistryLiveTrackerusr[];
extern const TCHAR szRegistryLiveTrackerpwd[];
extern const TCHAR szRegistryPolarFile[];
extern const TCHAR szRegistryPollingMode[];
extern const TCHAR szRegistryPort1Index[];
extern const TCHAR szRegistryPort2Index[];
extern const TCHAR szRegistryPort1Name[];
extern const TCHAR szRegistryPort2Name[];
extern const TCHAR szRegistryPressureHg[];
extern const TCHAR szRegistrySafetyAltitudeArrival[];
extern const TCHAR szRegistrySafetyAltitudeMode[];
extern const TCHAR szRegistrySafetyAltitudeTerrain[];
extern const TCHAR szRegistrySafetyMacCready[];
extern const TCHAR szRegistrySafteySpeed[];
extern const TCHAR szRegistrySectorRadius[];
extern const TCHAR szRegistrySetSystemTimeFromGPS[];
extern const TCHAR szRegistryShading[];
extern const TCHAR szRegistrySnailTrail[];
extern const TCHAR szRegistrySnailWidthScale[];
extern const TCHAR szRegistrySpeed1Index[];
extern const TCHAR szRegistrySpeed2Index[];
extern const TCHAR szRegistrySpeedUnitsValue[];
extern const TCHAR szRegistryStartHeightRef[];
extern const TCHAR szRegistryStartLine[];
extern const TCHAR szRegistryStartMaxHeightMargin[];
extern const TCHAR szRegistryStartMaxHeight[];
extern const TCHAR szRegistryStartMaxSpeedMargin[];
extern const TCHAR szRegistryStartMaxSpeed[];
extern const TCHAR szRegistryStartRadius[];
extern const TCHAR szRegistryStatusFile[];
extern const TCHAR szRegistryTaskSpeedUnitsValue[];
extern const TCHAR szRegistryTeamcodeRefWaypoint[];
extern const TCHAR szRegistryTerrainBrightness[];
extern const TCHAR szRegistryTerrainContrast[];
extern const TCHAR szRegistryTerrainFile[];
extern const TCHAR szRegistryTerrainRamp[];
extern const TCHAR szRegistryThermalBar[];
extern const TCHAR szRegistryThermalLocator[];
extern const TCHAR szRegistryTpFilter[];
extern const TCHAR szRegistryTrackBar[];
extern const TCHAR szRegistryTrailDrift[];
extern const TCHAR szRegistryUTCOffset[];
extern const TCHAR szRegistryUseCustomFonts[];
extern const TCHAR szRegistryUseGeoidSeparation[];
extern const TCHAR szRegistryUseUngestures[];
extern const TCHAR szRegistryUseTotalEnergy[];
extern const TCHAR szRegistryWarningTime[];
extern const TCHAR szRegistryWayPointFile[];
extern const TCHAR szRegistryWaypointsOutOfRange[];
extern const TCHAR szRegistryWindCalcSpeed[];
extern const TCHAR szRegistryWindCalcTime[];
extern const TCHAR szRegistryCustomMenu1[];
extern const TCHAR szRegistryCustomMenu2[];
extern const TCHAR szRegistryCustomMenu3[];
extern const TCHAR szRegistryCustomMenu4[];
extern const TCHAR szRegistryCustomMenu5[];
extern const TCHAR szRegistryCustomMenu6[];
extern const TCHAR szRegistryCustomMenu7[];
extern const TCHAR szRegistryCustomMenu8[];
extern const TCHAR szRegistryCustomMenu9[];
extern const TCHAR szRegistryCustomMenu10[];
extern const TCHAR szRegistryUseWindRose[];
extern const TCHAR szRegistryMultiTerr0[];
extern const TCHAR szRegistryMultiTerr1[];
extern const TCHAR szRegistryMultiTerr2[];
extern const TCHAR szRegistryMultiTerr3[];
extern const TCHAR szRegistryMultiTerr4[];
extern const TCHAR szRegistryMultiTopo0[];
extern const TCHAR szRegistryMultiTopo1[];
extern const TCHAR szRegistryMultiTopo2[];
extern const TCHAR szRegistryMultiTopo3[];
extern const TCHAR szRegistryMultiTopo4[];
extern const TCHAR szRegistryMultiAsp0[];
extern const TCHAR szRegistryMultiAsp1[];
extern const TCHAR szRegistryMultiAsp2[];
extern const TCHAR szRegistryMultiAsp3[];
extern const TCHAR szRegistryMultiAsp4[];
extern const TCHAR szRegistryMultiLab0[];
extern const TCHAR szRegistryMultiLab1[];
extern const TCHAR szRegistryMultiLab2[];
extern const TCHAR szRegistryMultiLab3[];
extern const TCHAR szRegistryMultiLab4[];
extern const TCHAR szRegistryMultiWpt0[];
extern const TCHAR szRegistryMultiWpt1[];
extern const TCHAR szRegistryMultiWpt2[];
extern const TCHAR szRegistryMultiWpt3[];
extern const TCHAR szRegistryMultiWpt4[];
extern const TCHAR szRegistryMultiOvrT0[];
extern const TCHAR szRegistryMultiOvrT1[];
extern const TCHAR szRegistryMultiOvrT2[];
extern const TCHAR szRegistryMultiOvrT3[];
extern const TCHAR szRegistryMultiOvrT4[];
extern const TCHAR szRegistryMultiOvrG0[];
extern const TCHAR szRegistryMultiOvrG1[];
extern const TCHAR szRegistryMultiOvrG2[];
extern const TCHAR szRegistryMultiOvrG3[];
extern const TCHAR szRegistryMultiOvrG4[];
extern const TCHAR szRegistryMultiSizeY1[];
extern const TCHAR szRegistryMultiSizeY2[];
extern const TCHAR szRegistryMultiSizeY3[];
extern const TCHAR szRegistryMultiSizeY4[];

extern const TCHAR szRegistryMultimap1[];
extern const TCHAR szRegistryMultimap2[];
extern const TCHAR szRegistryMultimap3[];
extern const TCHAR szRegistryMultimap4[];

extern const TCHAR szRegistryMMNorthUp1[];
extern const TCHAR szRegistryMMNorthUp2[];
extern const TCHAR szRegistryMMNorthUp3[];
extern const TCHAR szRegistryMMNorthUp4[];

extern const TCHAR szRegistryAspPermanent[];
extern const TCHAR szRegistryFlarmDirection[];
extern const TCHAR szRegistryDrawTask[];
extern const TCHAR szRegistryDrawFAI[] ;
extern const TCHAR szRegistryGearMode[];
extern const TCHAR szRegistryGearAltitude[];
extern const TCHAR szRegistryBottomMode[];
extern const TCHAR szRegistryBigFAIThreshold[];


extern const TCHAR szRegistryScreenSize[];
extern const TCHAR szRegistryScreenSizeX[];
extern const TCHAR szRegistryScreenSizeY[];

//
//
#endif

extern void LKProfileResetDefault(void);
extern void LKProfileInitRuntime(void);
extern bool LKProfileLoad(const TCHAR *szFile);
extern void LKProfileSave(const TCHAR *szFile);

#endif // LKPROFILES_H
