/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id
*/

#ifndef LKPROFILES_H
#define LKPROFILES_H

#if defined(STATIC_PGLOBALS)


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



TCHAR szRegistryAirspaceWarning[]= TEXT("AirspaceWarn");
TCHAR szRegistryAirspaceBlackOutline[]= TEXT("AirspaceBlackOutline1");
TCHAR szRegistryAirspaceFillType[]= TEXT("AirspaceFillType");
TCHAR szRegistryAirspaceOpacity[]= TEXT("AirspaceOpacity");
TCHAR szRegistryAirspaceWarningRepeatTime[]= TEXT("AirspaceWarningRepeatTime1");
TCHAR szRegistryAirspaceWarningVerticalMargin[]= TEXT("AirspaceWarningVerticalMargin");
TCHAR szRegistryAirspaceWarningDlgTimeout[]= TEXT("AirspaceWarningDlgTimeout");
TCHAR szRegistryAirspaceWarningMapLabels[]= TEXT("AirspaceWarningMapLabels");
TCHAR szRegistryAltMargin[]=	   TEXT("AltMargin");
TCHAR szRegistryAltMode[]=  TEXT("AltitudeMode");
TCHAR szRegistrySafetyAltitudeMode[]=  TEXT("SafetyAltitudeMode");
TCHAR szRegistryAltitudeUnitsValue[] = TEXT("Altitude");
TCHAR szRegistryCircleZoom[]= TEXT("CircleZoom");
TCHAR szRegistryClipAlt[]= TEXT("ClipAlt");
TCHAR szRegistryDisplayText[] = TEXT("DisplayText2");
TCHAR szRegistryDisplayUpValue[] = TEXT("DisplayUp");
TCHAR szRegistryDistanceUnitsValue[] = TEXT("Distance");
TCHAR szRegistryDrawTerrain[]= TEXT("DrawTerrain");
TCHAR szRegistryDrawTopology[]= TEXT("DrawTopology");
TCHAR szRegistryFAISector[] = TEXT("FAISector");
TCHAR szRegistryFinalGlideTerrain[]= TEXT("FinalGlideTerrain");
TCHAR szRegistryAutoWind[]= TEXT("AutoWind");
TCHAR szRegistryHomeWaypoint[]= TEXT("HomeWaypoint1");
TCHAR szRegistryAlternate1[]= TEXT("Alternate1a");
TCHAR szRegistryAlternate2[]= TEXT("Alternate2a");
TCHAR szRegistryLiftUnitsValue[] = TEXT("Lift");
TCHAR szRegistryLatLonUnits[] = TEXT("LatLonUnits");
TCHAR szRegistryPort1Index[]= TEXT("PortIndex");
TCHAR szRegistryPort2Index[]= TEXT("Port2Index");
TCHAR szRegistryRegKey[]=				 TEXT("RegKey");
TCHAR szRegistrySafetyAltitudeArrival[] =     TEXT("SafetyAltitudeArrival");
TCHAR szRegistrySafetyAltitudeTerrain[] =     TEXT("SafetyAltitudeTerrain");
TCHAR szRegistrySafteySpeed[] =          TEXT("SafteySpeed");
TCHAR szRegistryWindCalcSpeed[] =          TEXT("WindCalcSpeed");
TCHAR szRegistryWindCalcTime[] =          TEXT("WindCalcTime");
TCHAR szRegistrySectorRadius[]=          TEXT("Radius");
TCHAR szRegistrySnailTrail[]=		 TEXT("SnailTrail");
TCHAR szRegistryTrailDrift[]=		 TEXT("TrailDrift");
TCHAR szRegistryThermalLocator[]=	 TEXT("ThermalLocator");
TCHAR szRegistrySpeed1Index[]=		 TEXT("SpeedIndex");
TCHAR szRegistrySpeed2Index[]=		 TEXT("Speed2Index");
TCHAR szRegistryBit1Index[]=		 TEXT("Bit1Index");
TCHAR szRegistryBit2Index[]=		 TEXT("Bit2Index");
TCHAR szRegistryCheckSum[]=		 TEXT("CheckSum");
TCHAR szRegistrySpeedUnitsValue[] =      TEXT("Speed");
TCHAR szRegistryTaskSpeedUnitsValue[] =      TEXT("TaskSpeed");
TCHAR szRegistryStartLine[]=		 TEXT("StartLine");
TCHAR szRegistryStartRadius[]=		 TEXT("StartRadius");
TCHAR szRegistryFinishLine[]=		 TEXT("FinishLine");
TCHAR szRegistryFinishRadius[]=		 TEXT("FinishRadius");
TCHAR szRegistryWarningTime[]=		 TEXT("WarnTime");
TCHAR szRegistryAcknowledgementTime[]=	 TEXT("AcknowledgementTime");
TCHAR szRegistryWindUpdateMode[] =       TEXT("WindUpdateMode");
TCHAR szRegistryWindSpeed[] =            TEXT("WindSpeed");
TCHAR szRegistryWindBearing[] =          TEXT("WindBearing");
TCHAR szRegistryAirfieldFile[]=  TEXT("AirfieldFile");
TCHAR szRegistryAirspaceFile[]=  TEXT("AirspaceFile");
TCHAR szRegistryAdditionalAirspaceFile[]=  TEXT("AdditionalAirspaceFile");
TCHAR szRegistryPolarFile[] = TEXT("PolarFile");
TCHAR szRegistryTerrainFile[]=	 TEXT("TerrainFile");
TCHAR szRegistryTopologyFile[]=  TEXT("TopologyFile");
TCHAR szRegistryWayPointFile[]=  TEXT("WPFile");
TCHAR szRegistryAdditionalWayPointFile[]=  TEXT("AdditionalWPFile");
TCHAR szRegistryLanguageFile[]=  TEXT("LanguageFile");
TCHAR szRegistryStatusFile[]=  TEXT("StatusFile");
TCHAR szRegistryInputFile[]=  TEXT("InputFile");
TCHAR szRegistryPilotName[]=  TEXT("PilotName");
TCHAR szRegistryAircraftType[]=  TEXT("AircraftType");
TCHAR szRegistryAircraftRego[]=  TEXT("AircraftRego");
TCHAR szRegistryCompetitionClass[]=  TEXT("CompetitionClass");
TCHAR szRegistryCompetitionID[]=  TEXT("CompetitionID");
TCHAR szRegistryLoggerID[]=  TEXT("LoggerID");
TCHAR szRegistryLoggerShort[]=  TEXT("LoggerShortName");

TCHAR szRegistryDeviceA[]= TEXT("DeviceA");
TCHAR szRegistryDeviceB[]= TEXT("DeviceB");
TCHAR szRegistryDeviceC[]= TEXT("DeviceC");

TCHAR szRegistryAutoBacklight[]= TEXT("AutoBacklight");
TCHAR szRegistryAutoSoundVolume[]= TEXT("AutoSoundVolume");
TCHAR szRegistryAircraftCategory[]= TEXT("AircraftCategory");
TCHAR szRegistryExtendedVisualGlide[]= TEXT("ExtVisualGlide");
TCHAR szRegistryLook8000[]= TEXT("Look8000");
TCHAR szRegistryAltArrivMode[]= TEXT("AltArrivMode");
TCHAR szRegistryActiveMap[]= TEXT("ActiveMap");
TCHAR szRegistryBestWarning[]= TEXT("BestWarning");
TCHAR szRegistryThermalBar[]= TEXT("ThermalBar");
TCHAR szRegistryMcOverlay[]= TEXT("McOverlay2");
TCHAR szRegistryTrackBar[]= TEXT("TrackBar");
TCHAR szRegistryPGOptimizeRoute[]= TEXT("PGOptimizeRoute");
TCHAR szRegistryIphoneGestures[]= TEXT("IphoneGestures");
TCHAR szRegistryPollingMode[]= TEXT("PollingMode");
TCHAR szRegistryLKVarioBar[]= TEXT("LKVarioBar");
TCHAR szRegistryLKVarioVal[]= TEXT("LKVarioVal");
TCHAR szRegistryHideUnits[]= TEXT("HideUnits");
TCHAR szRegistryGlideBarMode[]= TEXT("GlideBarMode");
TCHAR szRegistryOutlinedTp[]= TEXT("OutlinedTp");
TCHAR szRegistryOverColor[]= TEXT("OverColor");
TCHAR szRegistryTpFilter[]= TEXT("TpFilter");
TCHAR szRegistryDeclutterMode[]= TEXT("DeclutterMode");
TCHAR szRegistryMapBox[]= TEXT("MapBox");
TCHAR szRegistryArrivalValue[]= TEXT("ArrivalValue");
TCHAR szRegistryNewMapDeclutter[]= TEXT("NewMapDeclutter");
TCHAR szRegistryAverEffTime[]= TEXT("AverEffTime");
TCHAR szRegistryBgMapColor[]= TEXT("BgMapColor");

TCHAR szRegistryDebounceTimeout[]= TEXT("DebounceTimeout");

TCHAR szRegistryPGCruiseZoom[]= TEXT("PGCruiseZoom");
TCHAR szRegistryPGAutoZoomThreshold[]= TEXT("PGAutoZoomThreshold");
TCHAR szRegistryPGClimbZoom[]= TEXT("PGClimbZoom");
TCHAR szRegistryAutoOrientScale[]= TEXT("AutoOrientScale");
TCHAR szRegistryPGOpenTimeH[]= TEXT("PGOpenTimeH");
TCHAR szRegistryPGOpenTimeM[]= TEXT("PGOpenTimeM");
TCHAR szRegistryPGNumberOfGates[]= TEXT("PGNumberOfGates");
TCHAR szRegistryLKTopoZoomCat05[]= TEXT("LKTopoZoomCat05");
TCHAR szRegistryLKTopoZoomCat10[]= TEXT("LKTopoZoomCat10");
TCHAR szRegistryLKTopoZoomCat20[]= TEXT("LKTopoZoomCat20");
TCHAR szRegistryLKTopoZoomCat30[]= TEXT("LKTopoZoomCat30");
TCHAR szRegistryLKTopoZoomCat40[]= TEXT("LKTopoZoomCat40");
TCHAR szRegistryLKTopoZoomCat50[]= TEXT("LKTopoZoomCat50");
TCHAR szRegistryLKTopoZoomCat60[]= TEXT("LKTopoZoomCat60");
TCHAR szRegistryLKTopoZoomCat70[]= TEXT("LKTopoZoomCat70");
TCHAR szRegistryLKTopoZoomCat80[]= TEXT("LKTopoZoomCat80");
TCHAR szRegistryLKTopoZoomCat90[]= TEXT("LKTopoZoomCat90");
TCHAR szRegistryLKTopoZoomCat100[]= TEXT("LKTopoZoomCat100");
TCHAR szRegistryLKTopoZoomCat110[]= TEXT("LKTopoZoomCat110");
TCHAR szRegistryLKMaxLabels[]= TEXT("LKMaxLabels");
TCHAR szRegistryOverlaySize[]= TEXT("OverlaySize");
TCHAR szRegistryBarOpacity[]= TEXT("BarOpacity");
TCHAR szRegistryFontRenderer[]= TEXT("FontRenderer2");
TCHAR szRegistryPGGateIntervalTime[]= TEXT("PGGateIntervalTime");
TCHAR szRegistryPGStartOut[]= TEXT("PGStartOut");

TCHAR szRegistryAppIndLandable[] = TEXT("AppIndLandable");
TCHAR szRegistryAppInverseInfoBox[] = TEXT("AppInverseInfoBox2");
TCHAR szRegistryAppDefaultMapWidth[] = TEXT("AppDefaultMapWidth");
TCHAR szRegistryTeamcodeRefWaypoint[] = TEXT("TeamcodeRefWaypoint1");

TCHAR szRegistryAppInfoBoxModel[] = TEXT("AppInfoBoxModel"); 
TCHAR szRegistryGpsAltitudeOffset[] = TEXT("GpsAltitudeOffset");
TCHAR szRegistryUseGeoidSeparation[] = TEXT("UseGeoidSeparation");
TCHAR szRegistryPressureHg[] = TEXT("PressureHg");
TCHAR szRegistryCustomKeyTime[] = TEXT("CustomKeyTime");
TCHAR szRegistryCustomKeyModeCenter[] = TEXT("CustomKeyModeCenter");
TCHAR szRegistryCustomKeyModeLeft[] = TEXT("CustomKeyModeLeft");
TCHAR szRegistryCustomKeyModeRight[] = TEXT("CustomKeyModeRight");
TCHAR szRegistryCustomKeyModeAircraftIcon[] = TEXT("CustomKeyModeAircraftIcon");
TCHAR szRegistryCustomKeyModeLeftUpCorner[] = TEXT("CustomKeyModeLeftUpCorner");
TCHAR szRegistryCustomKeyModeRightUpCorner[] = TEXT("CustomKeyModeRightUpCorner");
TCHAR szRegistryCustomKeyModeCenterScreen[] = TEXT("CustomKeyModeCenterScreen");

TCHAR szRegistryAutoAdvance[] = TEXT("AutoAdvance");
TCHAR szRegistryUTCOffset[] = TEXT("UTCOffset");
TCHAR szRegistryAutoZoom[] = TEXT("AutoZoom");
TCHAR szRegistryMenuTimeout[] = TEXT("MenuTimeout");
TCHAR szRegistryLockSettingsInFlight[] = TEXT("LockSettingsInFlight");
TCHAR szRegistryTerrainContrast[] = TEXT("TerrainContrast");
TCHAR szRegistryTerrainBrightness[] = TEXT("TerrainBrightness");
TCHAR szRegistryTerrainRamp[] = TEXT("TerrainRamp");
TCHAR szRegistryEnableFLARMMap[] = TEXT("EnableFLARMDisplay1");
TCHAR szRegistryGliderScreenPosition[] = TEXT("GliderScreenPosition");
TCHAR szRegistrySetSystemTimeFromGPS[] = TEXT("SetSystemTimeFromGPS");
TCHAR szRegistryAutoForceFinalGlide[] = TEXT("AutoForceFinalGlide");

TCHAR szRegistryAlarmMaxAltitude1[]= TEXT("AlarmMaxAltitude1");
TCHAR szRegistryAlarmMaxAltitude2[]= TEXT("AlarmMaxAltitude3");
TCHAR szRegistryAlarmMaxAltitude3[]= TEXT("AlarmMaxAltitude4");

TCHAR szRegistryFinishMinHeight[]= TEXT("FinishMinHeight");
TCHAR szRegistryStartMaxHeight[]= TEXT("StartMaxHeight");
TCHAR szRegistryStartMaxSpeed[]= TEXT("StartMaxSpeed");
TCHAR szRegistryStartMaxHeightMargin[]= TEXT("StartMaxHeightMargin");
TCHAR szRegistryStartMaxSpeedMargin[]= TEXT("StartMaxSpeedMargin");
TCHAR szRegistryStartHeightRef[] = TEXT("StartHeightRef");
TCHAR szRegistryEnableNavBaroAltitude[] = TEXT("EnableNavBaroAltitude");
TCHAR szRegistryOrbiter[] = TEXT("Orbiter");
TCHAR szRegistryShading[] = TEXT("Shading");
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

TCHAR szRegistryOverlayClock[] = TEXT("OverlayClock");

TCHAR szRegistryLoggerTimeStepCruise[]= TEXT("LoggerTimeStepCruise");
TCHAR szRegistryLoggerTimeStepCircling[]= TEXT("LoggerTimeStepCircling");

TCHAR szRegistrySafetyMacCready[] = TEXT("SafetyMacCready");
TCHAR szRegistryAutoMcMode[] = TEXT("AutoMcMode");
TCHAR szRegistryAutoMcStatus[] = TEXT("AutoMcStatus");
TCHAR szRegistryUseTotalEnergy[] = TEXT("UseTotalEnergy");
TCHAR szRegistryWaypointsOutOfRange[] = TEXT("WaypointsOutOfRange2");
TCHAR szRegistryFAIFinishHeight[] = TEXT("FAIFinishHeight");
TCHAR szRegistryHandicap[] = TEXT("Handicap");
TCHAR szRegistrySnailWidthScale[] = TEXT("SnailWidthScale");
TCHAR szRegistryDisableAutoLogger[] = TEXT("DisableAutoLogger");
TCHAR szRegistryMapFile[]=	 TEXT("MapFile"); // pL
TCHAR szRegistryBallastSecsToEmpty[]=	 TEXT("BallastSecsToEmpty"); 
TCHAR szRegistryUseCustomFonts[]=	 TEXT("UseCustomFonts"); 
TCHAR szRegistryFontMapLabelFont[]=	 TEXT("MapLabelFont"); 
TCHAR szRegistryFontMapWindowFont[]=	 TEXT("MapWindowFont"); 

//
//
#else 
//
// ------------------------------------- externals ------------------------------------------
//

extern TCHAR *szRegistryDisplayType[];	// MAXINFOWINDOWS
extern TCHAR *szRegistryColour[];	// 16
extern TCHAR *szRegistryBrush[];	// 16
extern TCHAR *szRegistryAirspaceMode[];	// 16

extern TCHAR szRegistryAirspaceWarning[];
extern TCHAR szRegistryAirspaceBlackOutline[];
extern TCHAR szRegistryAirspaceFillType[];
extern TCHAR szRegistryAirspaceOpacity[];
extern TCHAR szRegistryAirspaceWarningRepeatTime[];
extern TCHAR szRegistryAirspaceWarningVerticalMargin[];
extern TCHAR szRegistryAirspaceWarningDlgTimeout[];
extern TCHAR szRegistryAirspaceWarningMapLabels[];
extern TCHAR szRegistryAltMargin[];
extern TCHAR szRegistryAltMode[];
extern TCHAR szRegistrySafetyAltitudeMode[];
extern TCHAR szRegistryAltitudeUnitsValue[];
extern TCHAR szRegistryCircleZoom[];
extern TCHAR szRegistryClipAlt[];
extern TCHAR szRegistryDisplayText[];
extern TCHAR szRegistryDisplayUpValue[];
extern TCHAR szRegistryDistanceUnitsValue[];
extern TCHAR szRegistryDrawTerrain[];
extern TCHAR szRegistryDrawTopology[];
extern TCHAR szRegistryFAISector[];
extern TCHAR szRegistryFinalGlideTerrain[];
extern TCHAR szRegistryAutoWind[];
extern TCHAR szRegistryHomeWaypoint[];
extern TCHAR szRegistryAlternate1[];
extern TCHAR szRegistryAlternate2[];
extern TCHAR szRegistryLiftUnitsValue[];
extern TCHAR szRegistryLatLonUnits[];
extern TCHAR szRegistryPort1Index[];
extern TCHAR szRegistryPort2Index[];
extern TCHAR szRegistryRegKey[];
extern TCHAR szRegistrySafetyAltitudeArrival[];
extern TCHAR szRegistrySafetyAltitudeTerrain[];
extern TCHAR szRegistrySafteySpeed[];
extern TCHAR szRegistryWindCalcSpeed[];
extern TCHAR szRegistryWindCalcTime[];
extern TCHAR szRegistrySectorRadius[];
extern TCHAR szRegistrySnailTrail[];
extern TCHAR szRegistryTrailDrift[];
extern TCHAR szRegistryThermalLocator[];
extern TCHAR szRegistrySpeed1Index[];
extern TCHAR szRegistrySpeed2Index[];
extern TCHAR szRegistryBit1Index[];
extern TCHAR szRegistryBit2Index[];
extern TCHAR szRegistryCheckSum[];
extern TCHAR szRegistrySpeedUnitsValue[];
extern TCHAR szRegistryTaskSpeedUnitsValue[];
extern TCHAR szRegistryStartLine[];
extern TCHAR szRegistryStartRadius[];
extern TCHAR szRegistryFinishLine[];
extern TCHAR szRegistryFinishRadius[];
extern TCHAR szRegistryWarningTime[];
extern TCHAR szRegistryAcknowledgementTime[];
extern TCHAR szRegistryWindUpdateMode[];
extern TCHAR szRegistryWindSpeed[];
extern TCHAR szRegistryWindBearing[];
extern TCHAR szRegistryAirfieldFile[];
extern TCHAR szRegistryAirspaceFile[];
extern TCHAR szRegistryAdditionalAirspaceFile[];
extern TCHAR szRegistryPolarFile[];
extern TCHAR szRegistryTerrainFile[];
extern TCHAR szRegistryTopologyFile[];
extern TCHAR szRegistryWayPointFile[];
extern TCHAR szRegistryAdditionalWayPointFile[];
extern TCHAR szRegistryLanguageFile[];
extern TCHAR szRegistryStatusFile[];
extern TCHAR szRegistryInputFile[];
extern TCHAR szRegistryPilotName[];
extern TCHAR szRegistryAircraftType[];
extern TCHAR szRegistryAircraftRego[];
extern TCHAR szRegistryCompetitionClass[];
extern TCHAR szRegistryCompetitionID[];
extern TCHAR szRegistryLoggerID[];
extern TCHAR szRegistryLoggerShort[];
extern TCHAR szRegistryDeviceA[];
extern TCHAR szRegistryDeviceB[];

extern TCHAR szRegistryAutoBacklight[];
extern TCHAR szRegistryAutoSoundVolume[];
extern TCHAR szRegistryAircraftCategory[];
extern TCHAR szRegistryExtendedVisualGlide[];
extern TCHAR szRegistryLook8000[];
extern TCHAR szRegistryAltArrivMode[];
extern TCHAR szRegistryActiveMap[];
extern TCHAR szRegistryBestWarning[];
extern TCHAR szRegistryThermalBar[];
extern TCHAR szRegistryMcOverlay[];
extern TCHAR szRegistryTrackBar[];
extern TCHAR szRegistryPGOptimizeRoute[];
extern TCHAR szRegistryIphoneGestures[];
extern TCHAR szRegistryPollingMode[];
extern TCHAR szRegistryLKVarioBar[];
extern TCHAR szRegistryLKVarioVal[];
extern TCHAR szRegistryHideUnits[];
extern TCHAR szRegistryGlideBarMode[];
extern TCHAR szRegistryOutlinedTp[];
extern TCHAR szRegistryOverColor[];
extern TCHAR szRegistryTpFilter[];
extern TCHAR szRegistryDeclutterMode[];
extern TCHAR szRegistryMapBox[];
extern TCHAR szRegistryArrivalValue[];
extern TCHAR szRegistryNewMapDeclutter[];
extern TCHAR szRegistryAverEffTime[];
extern TCHAR szRegistryBgMapColor[];

extern TCHAR szRegistryDebounceTimeout[];

extern TCHAR szRegistryPGCruiseZoom[];
extern TCHAR szRegistryPGAutoZoomThreshold[];
extern TCHAR szRegistryPGClimbZoom[];
extern TCHAR szRegistryAutoOrientScale[];
extern TCHAR szRegistryPGOpenTimeH[];
extern TCHAR szRegistryPGOpenTimeM[];
extern TCHAR szRegistryPGNumberOfGates[];
extern TCHAR szRegistryLKTopoZoomCat05[];
extern TCHAR szRegistryLKTopoZoomCat10[];
extern TCHAR szRegistryLKTopoZoomCat20[];
extern TCHAR szRegistryLKTopoZoomCat30[];
extern TCHAR szRegistryLKTopoZoomCat40[];
extern TCHAR szRegistryLKTopoZoomCat50[];
extern TCHAR szRegistryLKTopoZoomCat60[];
extern TCHAR szRegistryLKTopoZoomCat70[];
extern TCHAR szRegistryLKTopoZoomCat80[];
extern TCHAR szRegistryLKTopoZoomCat90[];
extern TCHAR szRegistryLKTopoZoomCat100[];
extern TCHAR szRegistryLKTopoZoomCat110[];
extern TCHAR szRegistryLKMaxLabels[];
extern TCHAR szRegistryOverlaySize[];
extern TCHAR szRegistryBarOpacity[];
extern TCHAR szRegistryFontRenderer[];
extern TCHAR szRegistryPGGateIntervalTime[];
extern TCHAR szRegistryPGStartOut[];

extern TCHAR szRegistryAppIndLandable[];
extern TCHAR szRegistryAppInverseInfoBox[];
extern TCHAR szRegistryAppDefaultMapWidth[];
extern TCHAR szRegistryTeamcodeRefWaypoint[];

extern TCHAR szRegistryAppInfoBoxModel[];
extern TCHAR szRegistryGpsAltitudeOffset[];
extern TCHAR szRegistryUseGeoidSeparation[];
extern TCHAR szRegistryPressureHg[];
extern TCHAR szRegistryCustomKeyTime[];
extern TCHAR szRegistryCustomKeyModeCenter[];
extern TCHAR szRegistryCustomKeyModeLeft[];
extern TCHAR szRegistryCustomKeyModeRight[];
extern TCHAR szRegistryCustomKeyModeAircraftIcon[];
extern TCHAR szRegistryCustomKeyModeLeftUpCorner[];
extern TCHAR szRegistryCustomKeyModeRightUpCorner[];
extern TCHAR szRegistryCustomKeyModeCenterScreen[];

extern TCHAR szRegistryAutoAdvance[];
extern TCHAR szRegistryUTCOffset[];
extern TCHAR szRegistryAutoZoom[];
extern TCHAR szRegistryMenuTimeout[];
extern TCHAR szRegistryLockSettingsInFlight[];
extern TCHAR szRegistryTerrainContrast[];
extern TCHAR szRegistryTerrainBrightness[];
extern TCHAR szRegistryTerrainRamp[];
extern TCHAR szRegistryEnableFLARMMap[];
extern TCHAR szRegistryGliderScreenPosition[];
extern TCHAR szRegistrySetSystemTimeFromGPS[];
extern TCHAR szRegistryAutoForceFinalGlide[];

extern TCHAR szRegistryAlarmMaxAltitude1[];
extern TCHAR szRegistryAlarmMaxAltitude2[];
extern TCHAR szRegistryAlarmMaxAltitude3[];

extern TCHAR szRegistryFinishMinHeight[];
extern TCHAR szRegistryStartMaxHeight[];
extern TCHAR szRegistryStartMaxSpeed[];
extern TCHAR szRegistryStartMaxHeightMargin[];
extern TCHAR szRegistryStartMaxSpeedMargin[];
extern TCHAR szRegistryStartHeightRef[];
extern TCHAR szRegistryEnableNavBaroAltitude[];
extern TCHAR szRegistryOrbiter[];
extern TCHAR szRegistryShading[];
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

extern TCHAR szRegistryOverlayClock[];

extern TCHAR szRegistryLoggerTimeStepCruise[];
extern TCHAR szRegistryLoggerTimeStepCircling[];

extern TCHAR szRegistrySafetyMacCready[];
extern TCHAR szRegistryAutoMcMode[];
extern TCHAR szRegistryAutoMcStatus[];
extern TCHAR szRegistryUseTotalEnergy[];
extern TCHAR szRegistryWaypointsOutOfRange[];
extern TCHAR szRegistryFAIFinishHeight[];
extern TCHAR szRegistryHandicap[];
extern TCHAR szRegistrySnailWidthScale[];
extern TCHAR szRegistryDisableAutoLogger[];
extern TCHAR szRegistryMapFile[];
extern TCHAR szRegistryBallastSecsToEmpty[];
extern TCHAR szRegistryUseCustomFonts[];
extern TCHAR szRegistryFontMapLabelFont[];
extern TCHAR szRegistryFontMapWindowFont[];


#endif

#endif
