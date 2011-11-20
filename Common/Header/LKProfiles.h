/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id
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
//

TCHAR *szRegistryDisplayType[MAXINFOWINDOWS] =     { TEXT("Info0"),
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
};

TCHAR *szRegistryColour[] =     { TEXT("Colour0"),
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
};


TCHAR *szRegistryBrush[] =     {  TEXT("Brush0"),
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
};

TCHAR *szRegistryAirspaceMode[] =     {  TEXT("AirspaceMode0"),
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
};

TCHAR szRegistryAcknowledgementTime[]=	 TEXT("AcknowledgementTime");
TCHAR szRegistryActiveMap[]= TEXT("ActiveMap");
TCHAR szRegistryAdditionalAirspaceFile[]=  TEXT("AdditionalAirspaceFile");
TCHAR szRegistryAdditionalWayPointFile[]=  TEXT("AdditionalWPFile");
TCHAR szRegistryAircraftCategory[]= TEXT("AircraftCategory");
TCHAR szRegistryAircraftRego[]=  TEXT("AircraftRego");
TCHAR szRegistryAircraftType[]=  TEXT("AircraftType");
TCHAR szRegistryAirfieldFile[]=  TEXT("AirfieldFile");
TCHAR szRegistryAirspaceBlackOutline[]= TEXT("AirspaceBlackOutline1");
TCHAR szRegistryAirspaceFile[]=  TEXT("AirspaceFile");
TCHAR szRegistryAirspaceFillType[]= TEXT("AirspaceFillType");
TCHAR szRegistryAirspaceOpacity[]= TEXT("AirspaceOpacity");
TCHAR szRegistryAirspaceWarningDlgTimeout[]= TEXT("AirspaceWarningDlgTimeout");
TCHAR szRegistryAirspaceWarningMapLabels[]= TEXT("AirspaceWarningMapLabels");
TCHAR szRegistryAirspaceWarningRepeatTime[]= TEXT("AirspaceWarningRepeatTime1");
TCHAR szRegistryAirspaceWarningVerticalMargin[]= TEXT("AirspaceWarningVerticalMargin");
TCHAR szRegistryAirspaceWarning[]= TEXT("AirspaceWarn");
TCHAR szRegistryAlarmMaxAltitude1[]= TEXT("AlarmMaxAltitude1");
TCHAR szRegistryAlarmMaxAltitude2[]= TEXT("AlarmMaxAltitude3");
TCHAR szRegistryAlarmMaxAltitude3[]= TEXT("AlarmMaxAltitude4");
TCHAR szRegistryAltMargin[]=	   TEXT("AltMargin");
TCHAR szRegistryAltMode[]=  TEXT("AltitudeMode");
TCHAR szRegistryAlternate1[]= TEXT("Alternate1a");
TCHAR szRegistryAlternate2[]= TEXT("Alternate2a");
TCHAR szRegistryAltitudeUnitsValue[] = TEXT("AltitudeUnits");
TCHAR szRegistryAppDefaultMapWidth[] = TEXT("AppDefaultMapWidth");
TCHAR szRegistryAppIndLandable[] = TEXT("AppIndLandable");
TCHAR szRegistryAppInfoBoxModel[] = TEXT("AppInfoBoxModel"); 
TCHAR szRegistryAppInverseInfoBox[] = TEXT("AppInverseInfoBox2");
TCHAR szRegistryArrivalValue[]= TEXT("ArrivalValue");
TCHAR szRegistryAutoAdvance[] = TEXT("AutoAdvance");
TCHAR szRegistryAutoBacklight[]= TEXT("AutoBacklight");
TCHAR szRegistryAutoForceFinalGlide[] = TEXT("AutoForceFinalGlide");
TCHAR szRegistryAutoMcMode[] = TEXT("AutoMcMode");
TCHAR szRegistryAutoMcStatus[] = TEXT("AutoMcStatus");
TCHAR szRegistryAutoOrientScale[]= TEXT("AutoOrientScale");
TCHAR szRegistryAutoSoundVolume[]= TEXT("AutoSoundVolume");
TCHAR szRegistryAutoWind[]= TEXT("AutoWind");
TCHAR szRegistryAutoZoom[] = TEXT("AutoZoom");
TCHAR szRegistryAverEffTime[]= TEXT("AverEffTime");
TCHAR szRegistryBallastSecsToEmpty[]=	 TEXT("BallastSecsToEmpty"); 
TCHAR szRegistryBarOpacity[]= TEXT("BarOpacity");
TCHAR szRegistryBestWarning[]= TEXT("BestWarning");
TCHAR szRegistryBgMapColor[]= TEXT("BgMapColor");
TCHAR szRegistryBit1Index[]=		 TEXT("Bit1Index");
TCHAR szRegistryBit2Index[]=		 TEXT("Bit2Index");
TCHAR szRegistryCheckSum[]=		 TEXT("CheckSum");
TCHAR szRegistryCircleZoom[]= TEXT("CircleZoom");
TCHAR szRegistryClipAlt[]= TEXT("ClipAlt");
TCHAR szRegistryCompetitionClass[]=  TEXT("CompetitionClass");
TCHAR szRegistryCompetitionID[]=  TEXT("CompetitionID");
TCHAR szRegistryConfBB1[] = TEXT("ConfBB1");
TCHAR szRegistryConfBB2[] = TEXT("ConfBB2");
TCHAR szRegistryConfBB3[] = TEXT("ConfBB3");
TCHAR szRegistryConfBB4[] = TEXT("ConfBB4");
TCHAR szRegistryConfBB5[] = TEXT("ConfBB5");
TCHAR szRegistryConfBB6[] = TEXT("ConfBB6");
TCHAR szRegistryConfBB7[] = TEXT("ConfBB7");
TCHAR szRegistryConfBB8[] = TEXT("ConfBB8");
TCHAR szRegistryConfBB9[] = TEXT("ConfBB9");
TCHAR szRegistryConfIP11[] = TEXT("ConfIP11");
TCHAR szRegistryConfIP12[] = TEXT("ConfIP12");
TCHAR szRegistryConfIP13[] = TEXT("ConfIP13");
TCHAR szRegistryConfIP14[] = TEXT("ConfIP14");
TCHAR szRegistryConfIP15[] = TEXT("ConfIP15");
TCHAR szRegistryConfIP16[] = TEXT("ConfIP16");
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
TCHAR szRegistryDisplayText[] = TEXT("DisplayText2");
TCHAR szRegistryDisplayUpValue[] = TEXT("DisplayUp");
TCHAR szRegistryDistanceUnitsValue[] = TEXT("DistanceUnits");
TCHAR szRegistryDrawTerrain[]= TEXT("DrawTerrain");
TCHAR szRegistryDrawTopology[]= TEXT("DrawTopology");
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
TCHAR szRegistryHandicap[] = TEXT("Handicap");
TCHAR szRegistryHideUnits[]= TEXT("HideUnits");
TCHAR szRegistryHomeWaypoint[]= TEXT("HomeWaypoint1");
TCHAR szRegistryInputFile[]=  TEXT("InputFile");
TCHAR szRegistryIphoneGestures[]= TEXT("IphoneGestures");
TCHAR szRegistryLKMaxLabels[]= TEXT("LKMaxLabels");
TCHAR szRegistryLKTopoZoomCat05[]= TEXT("LKTopoZoomCat05");
TCHAR szRegistryLKTopoZoomCat100[]= TEXT("LKTopoZoomCat100");
TCHAR szRegistryLKTopoZoomCat10[]= TEXT("LKTopoZoomCat10");
TCHAR szRegistryLKTopoZoomCat110[]= TEXT("LKTopoZoomCat110");
TCHAR szRegistryLKTopoZoomCat20[]= TEXT("LKTopoZoomCat20");
TCHAR szRegistryLKTopoZoomCat30[]= TEXT("LKTopoZoomCat30");
TCHAR szRegistryLKTopoZoomCat40[]= TEXT("LKTopoZoomCat40");
TCHAR szRegistryLKTopoZoomCat50[]= TEXT("LKTopoZoomCat50");
TCHAR szRegistryLKTopoZoomCat60[]= TEXT("LKTopoZoomCat60");
TCHAR szRegistryLKTopoZoomCat70[]= TEXT("LKTopoZoomCat70");
TCHAR szRegistryLKTopoZoomCat80[]= TEXT("LKTopoZoomCat80");
TCHAR szRegistryLKTopoZoomCat90[]= TEXT("LKTopoZoomCat90");
TCHAR szRegistryLKVarioBar[]= TEXT("LKVarioBar");
TCHAR szRegistryLKVarioVal[]= TEXT("LKVarioVal");
TCHAR szRegistryLanguageFile[]=  TEXT("LanguageFile");
TCHAR szRegistryLatLonUnits[] = TEXT("LatLonUnits");
TCHAR szRegistryLiftUnitsValue[] = TEXT("LiftUnits");
TCHAR szRegistryLockSettingsInFlight[] = TEXT("LockSettingsInFlight");
TCHAR szRegistryLoggerID[]=  TEXT("LoggerID");
TCHAR szRegistryLoggerShort[]=  TEXT("LoggerShortName");
TCHAR szRegistryLoggerTimeStepCircling[]= TEXT("LoggerTimeStepCircling");
TCHAR szRegistryLoggerTimeStepCruise[]= TEXT("LoggerTimeStepCruise");
TCHAR szRegistryLook8000[]= TEXT("Look8000");
TCHAR szRegistryMapBox[]= TEXT("MapBox");
TCHAR szRegistryMapFile[]= TEXT("MapFile");
TCHAR szRegistryMcOverlay[]= TEXT("McOverlay2");
TCHAR szRegistryMenuTimeout[] = TEXT("MenuTimeout");
TCHAR szRegistryNewMapDeclutter[]= TEXT("NewMapDeclutter");
TCHAR szRegistryOrbiter[] = TEXT("Orbiter");
TCHAR szRegistryOutlinedTp[]= TEXT("OutlinedTp");
TCHAR szRegistryOverColor[]= TEXT("OverColor");
TCHAR szRegistryOverlayClock[] = TEXT("OverlayClock");
TCHAR szRegistryOverlaySize[]= TEXT("OverlaySize");
TCHAR szRegistryPGAutoZoomThreshold[]= TEXT("PGAutoZoomThreshold");
TCHAR szRegistryPGClimbZoom[]= TEXT("PGClimbZoom");
TCHAR szRegistryPGCruiseZoom[]= TEXT("PGCruiseZoom");
TCHAR szRegistryPGGateIntervalTime[]= TEXT("PGGateIntervalTime");
TCHAR szRegistryPGNumberOfGates[]= TEXT("PGNumberOfGates");
TCHAR szRegistryPGOpenTimeH[]= TEXT("PGOpenTimeH");
TCHAR szRegistryPGOpenTimeM[]= TEXT("PGOpenTimeM");
TCHAR szRegistryPGOptimizeRoute[]= TEXT("PGOptimizeRoute");
TCHAR szRegistryPGStartOut[]= TEXT("PGStartOut");
TCHAR szRegistryPilotName[]=  TEXT("PilotName");
TCHAR szRegistryPolarFile[] = TEXT("PolarFile");
TCHAR szRegistryPollingMode[]= TEXT("PollingMode");
TCHAR szRegistryPort1Index[]= TEXT("PortIndex");
TCHAR szRegistryPort2Index[]= TEXT("Port2Index");
TCHAR szRegistryPressureHg[] = TEXT("PressureHg");
TCHAR szRegistryRegKey[]=				 TEXT("RegKey");
TCHAR szRegistrySafetyAltitudeArrival[] =     TEXT("SafetyAltitudeArrival");
TCHAR szRegistrySafetyAltitudeMode[]=  TEXT("SafetyAltitudeMode");
TCHAR szRegistrySafetyAltitudeTerrain[] =     TEXT("SafetyAltitudeTerrain");
TCHAR szRegistrySafetyMacCready[] = TEXT("SafetyMacCready");
TCHAR szRegistrySafteySpeed[] =          TEXT("SafteySpeed");
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
TCHAR szRegistryTerrainBrightness[] = TEXT("TerrainBrightness");
TCHAR szRegistryTerrainContrast[] = TEXT("TerrainContrast");
TCHAR szRegistryTerrainFile[]=	 TEXT("TerrainFile");
TCHAR szRegistryTerrainRamp[] = TEXT("TerrainRamp");
TCHAR szRegistryThermalBar[]= TEXT("ThermalBar");
TCHAR szRegistryThermalLocator[]=	 TEXT("ThermalLocator");
TCHAR szRegistryTopologyFile[]=  TEXT("TopologyFile");
TCHAR szRegistryTpFilter[]= TEXT("TpFilter");
TCHAR szRegistryTrackBar[]= TEXT("TrackBar");
TCHAR szRegistryTrailDrift[]=		 TEXT("TrailDrift");
TCHAR szRegistryUTCOffset[] = TEXT("UTCOffset");
TCHAR szRegistryUseCustomFonts[]=	 TEXT("UseCustomFonts"); 
TCHAR szRegistryUseGeoidSeparation[] = TEXT("UseGeoidSeparation");
TCHAR szRegistryUseTotalEnergy[] = TEXT("UseTotalEnergy");
TCHAR szRegistryWarningTime[]=		 TEXT("WarnTime");
TCHAR szRegistryWayPointFile[]=  TEXT("WPFile");
TCHAR szRegistryWaypointsOutOfRange[] = TEXT("WaypointsOutOfRange2");
TCHAR szRegistryWindCalcSpeed[] =          TEXT("WindCalcSpeed");
TCHAR szRegistryWindCalcTime[] =          TEXT("WindCalcTime");
TCHAR szRegistryWindUpdateMode[] =       TEXT("WindUpdateMode");
//
//
//
#else 
//
// ------------------------------------- externals ------------------------------------------
//
extern TCHAR *szRegistryAirspaceMode[];	// 16
extern TCHAR *szRegistryBrush[];	// 16
extern TCHAR *szRegistryColour[];	// 16
extern TCHAR *szRegistryDisplayType[];	// MAXINFOWINDOWS
extern TCHAR szRegistryAcknowledgementTime[];
extern TCHAR szRegistryActiveMap[];
extern TCHAR szRegistryAdditionalAirspaceFile[];
extern TCHAR szRegistryAdditionalWayPointFile[];
extern TCHAR szRegistryAircraftCategory[];
extern TCHAR szRegistryAircraftRego[];
extern TCHAR szRegistryAircraftType[];
extern TCHAR szRegistryAirfieldFile[];
extern TCHAR szRegistryAirspaceBlackOutline[];
extern TCHAR szRegistryAirspaceFile[];
extern TCHAR szRegistryAirspaceFillType[];
extern TCHAR szRegistryAirspaceOpacity[];
extern TCHAR szRegistryAirspaceWarningDlgTimeout[];
extern TCHAR szRegistryAirspaceWarningMapLabels[];
extern TCHAR szRegistryAirspaceWarningRepeatTime[];
extern TCHAR szRegistryAirspaceWarningVerticalMargin[];
extern TCHAR szRegistryAirspaceWarning[];
extern TCHAR szRegistryAlarmMaxAltitude1[];
extern TCHAR szRegistryAlarmMaxAltitude2[];
extern TCHAR szRegistryAlarmMaxAltitude3[];
extern TCHAR szRegistryAltMargin[];
extern TCHAR szRegistryAltMode[];
extern TCHAR szRegistryAlternate1[];
extern TCHAR szRegistryAlternate2[];
extern TCHAR szRegistryAltitudeUnitsValue[];
extern TCHAR szRegistryAppDefaultMapWidth[];
extern TCHAR szRegistryAppIndLandable[];
extern TCHAR szRegistryAppInfoBoxModel[];
extern TCHAR szRegistryAppInverseInfoBox[];
extern TCHAR szRegistryArrivalValue[];
extern TCHAR szRegistryAutoAdvance[];
extern TCHAR szRegistryAutoBacklight[];
extern TCHAR szRegistryAutoForceFinalGlide[];
extern TCHAR szRegistryAutoMcMode[];
extern TCHAR szRegistryAutoMcStatus[];
extern TCHAR szRegistryAutoOrientScale[];
extern TCHAR szRegistryAutoSoundVolume[];
extern TCHAR szRegistryAutoWind[];
extern TCHAR szRegistryAutoZoom[];
extern TCHAR szRegistryAverEffTime[];
extern TCHAR szRegistryBallastSecsToEmpty[];
extern TCHAR szRegistryBarOpacity[];
extern TCHAR szRegistryBestWarning[];
extern TCHAR szRegistryBgMapColor[];
extern TCHAR szRegistryBit1Index[];
extern TCHAR szRegistryBit2Index[];
extern TCHAR szRegistryCheckSum[];
extern TCHAR szRegistryCircleZoom[];
extern TCHAR szRegistryClipAlt[];
extern TCHAR szRegistryCompetitionClass[];
extern TCHAR szRegistryCompetitionID[];
extern TCHAR szRegistryConfBB1[];
extern TCHAR szRegistryConfBB2[];
extern TCHAR szRegistryConfBB3[];
extern TCHAR szRegistryConfBB4[];
extern TCHAR szRegistryConfBB5[];
extern TCHAR szRegistryConfBB6[];
extern TCHAR szRegistryConfBB7[];
extern TCHAR szRegistryConfBB8[];
extern TCHAR szRegistryConfBB9[];
extern TCHAR szRegistryConfIP11[];
extern TCHAR szRegistryConfIP12[];
extern TCHAR szRegistryConfIP13[];
extern TCHAR szRegistryConfIP14[];
extern TCHAR szRegistryConfIP15[];
extern TCHAR szRegistryConfIP16[];
extern TCHAR szRegistryConfIP21[];
extern TCHAR szRegistryConfIP22[];
extern TCHAR szRegistryConfIP23[];
extern TCHAR szRegistryConfIP24[];
extern TCHAR szRegistryConfIP31[];
extern TCHAR szRegistryConfIP32[];
extern TCHAR szRegistryConfIP33[];
extern TCHAR szRegistryCustomKeyModeAircraftIcon[];
extern TCHAR szRegistryCustomKeyModeCenterScreen[];
extern TCHAR szRegistryCustomKeyModeCenter[];
extern TCHAR szRegistryCustomKeyModeLeftUpCorner[];
extern TCHAR szRegistryCustomKeyModeLeft[];
extern TCHAR szRegistryCustomKeyModeRightUpCorner[];
extern TCHAR szRegistryCustomKeyModeRight[];
extern TCHAR szRegistryCustomKeyTime[];
extern TCHAR szRegistryDebounceTimeout[];
extern TCHAR szRegistryDeclutterMode[];
extern TCHAR szRegistryDeviceA[];
extern TCHAR szRegistryDeviceB[];
extern TCHAR szRegistryDisableAutoLogger[];
extern TCHAR szRegistryDisplayText[];
extern TCHAR szRegistryDisplayUpValue[];
extern TCHAR szRegistryDistanceUnitsValue[];
extern TCHAR szRegistryDrawTerrain[];
extern TCHAR szRegistryDrawTopology[];
extern TCHAR szRegistryEnableFLARMMap[];
extern TCHAR szRegistryEnableNavBaroAltitude[];
extern TCHAR szRegistryFAIFinishHeight[];
extern TCHAR szRegistryFAISector[];
extern TCHAR szRegistryFinalGlideTerrain[];
extern TCHAR szRegistryFinishLine[];
extern TCHAR szRegistryFinishMinHeight[];
extern TCHAR szRegistryFinishRadius[];
extern TCHAR szRegistryFontMapLabelFont[];
extern TCHAR szRegistryFontMapWindowFont[];
extern TCHAR szRegistryFontRenderer[];
extern TCHAR szRegistryGlideBarMode[];
extern TCHAR szRegistryGliderScreenPosition[];
extern TCHAR szRegistryGpsAltitudeOffset[];
extern TCHAR szRegistryHandicap[];
extern TCHAR szRegistryHideUnits[];
extern TCHAR szRegistryHomeWaypoint[];
extern TCHAR szRegistryInputFile[];
extern TCHAR szRegistryIphoneGestures[];
extern TCHAR szRegistryLKMaxLabels[];
extern TCHAR szRegistryLKTopoZoomCat05[];
extern TCHAR szRegistryLKTopoZoomCat100[];
extern TCHAR szRegistryLKTopoZoomCat10[];
extern TCHAR szRegistryLKTopoZoomCat110[];
extern TCHAR szRegistryLKTopoZoomCat20[];
extern TCHAR szRegistryLKTopoZoomCat30[];
extern TCHAR szRegistryLKTopoZoomCat40[];
extern TCHAR szRegistryLKTopoZoomCat50[];
extern TCHAR szRegistryLKTopoZoomCat60[];
extern TCHAR szRegistryLKTopoZoomCat70[];
extern TCHAR szRegistryLKTopoZoomCat80[];
extern TCHAR szRegistryLKTopoZoomCat90[];
extern TCHAR szRegistryLKVarioBar[];
extern TCHAR szRegistryLKVarioVal[];
extern TCHAR szRegistryLanguageFile[];
extern TCHAR szRegistryLatLonUnits[];
extern TCHAR szRegistryLiftUnitsValue[];
extern TCHAR szRegistryLockSettingsInFlight[];
extern TCHAR szRegistryLoggerID[];
extern TCHAR szRegistryLoggerShort[];
extern TCHAR szRegistryLoggerTimeStepCircling[];
extern TCHAR szRegistryLoggerTimeStepCruise[];
extern TCHAR szRegistryLook8000[];
extern TCHAR szRegistryMapBox[];
extern TCHAR szRegistryMapFile[];
extern TCHAR szRegistryMcOverlay[];
extern TCHAR szRegistryMenuTimeout[];
extern TCHAR szRegistryNewMapDeclutter[];
extern TCHAR szRegistryOrbiter[];
extern TCHAR szRegistryOutlinedTp[];
extern TCHAR szRegistryOverColor[];
extern TCHAR szRegistryOverlayClock[];
extern TCHAR szRegistryOverlaySize[];
extern TCHAR szRegistryPGAutoZoomThreshold[];
extern TCHAR szRegistryPGClimbZoom[];
extern TCHAR szRegistryPGCruiseZoom[];
extern TCHAR szRegistryPGGateIntervalTime[];
extern TCHAR szRegistryPGNumberOfGates[];
extern TCHAR szRegistryPGOpenTimeH[];
extern TCHAR szRegistryPGOpenTimeM[];
extern TCHAR szRegistryPGOptimizeRoute[];
extern TCHAR szRegistryPGStartOut[];
extern TCHAR szRegistryPilotName[];
extern TCHAR szRegistryPolarFile[];
extern TCHAR szRegistryPollingMode[];
extern TCHAR szRegistryPort1Index[];
extern TCHAR szRegistryPort2Index[];
extern TCHAR szRegistryPressureHg[];
extern TCHAR szRegistryRegKey[];
extern TCHAR szRegistrySafetyAltitudeArrival[];
extern TCHAR szRegistrySafetyAltitudeMode[];
extern TCHAR szRegistrySafetyAltitudeTerrain[];
extern TCHAR szRegistrySafetyMacCready[];
extern TCHAR szRegistrySafteySpeed[];
extern TCHAR szRegistrySectorRadius[];
extern TCHAR szRegistrySetSystemTimeFromGPS[];
extern TCHAR szRegistryShading[];
extern TCHAR szRegistrySnailTrail[];
extern TCHAR szRegistrySnailWidthScale[];
extern TCHAR szRegistrySpeed1Index[];
extern TCHAR szRegistrySpeed2Index[];
extern TCHAR szRegistrySpeedUnitsValue[];
extern TCHAR szRegistryStartHeightRef[];
extern TCHAR szRegistryStartLine[];
extern TCHAR szRegistryStartMaxHeightMargin[];
extern TCHAR szRegistryStartMaxHeight[];
extern TCHAR szRegistryStartMaxSpeedMargin[];
extern TCHAR szRegistryStartMaxSpeed[];
extern TCHAR szRegistryStartRadius[];
extern TCHAR szRegistryStatusFile[];
extern TCHAR szRegistryTaskSpeedUnitsValue[];
extern TCHAR szRegistryTeamcodeRefWaypoint[];
extern TCHAR szRegistryTerrainBrightness[];
extern TCHAR szRegistryTerrainContrast[];
extern TCHAR szRegistryTerrainFile[];
extern TCHAR szRegistryTerrainRamp[];
extern TCHAR szRegistryThermalBar[];
extern TCHAR szRegistryThermalLocator[];
extern TCHAR szRegistryTopologyFile[];
extern TCHAR szRegistryTpFilter[];
extern TCHAR szRegistryTrackBar[];
extern TCHAR szRegistryTrailDrift[];
extern TCHAR szRegistryUTCOffset[];
extern TCHAR szRegistryUseCustomFonts[];
extern TCHAR szRegistryUseGeoidSeparation[];
extern TCHAR szRegistryUseTotalEnergy[];
extern TCHAR szRegistryWarningTime[];
extern TCHAR szRegistryWayPointFile[];
extern TCHAR szRegistryWaypointsOutOfRange[];
extern TCHAR szRegistryWindCalcSpeed[];
extern TCHAR szRegistryWindCalcTime[];
extern TCHAR szRegistryWindUpdateMode[];
//
//
#endif

#endif // LKPROFILES_H
