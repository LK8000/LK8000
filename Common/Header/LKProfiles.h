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

const char *szRegistryDisplayType[MAXINFOWINDOWS] =     { "Info0",
				       "Info1",
				       "Info2",
				       "Info3",
				       "Info4",
				       "Info5",
				       "Info6",
				       "Info7",
				       "Info8",
				       "Info9",
				       "Info10",
				       "Info11",
				       "Info12",
				       "Info13"
};

const char *szRegistryColour[] =     { "Colour0",
				  "Colour1",
				  "Colour2",
				  "Colour3",
				  "Colour4",
				  "Colour5",
				  "Colour6",
				  "Colour7",
				  "Colour8",
				  "Colour9",
				  "Colour10",
				  "Colour11",
				  "Colour12",
				  "Colour13",
                  "Colour14",
                  "Colour15",
                  "Colour16",
                  "Colour17",
                  "Colour18"
};

#ifdef HAVE_HATCHED_BRUSH
const char *szRegistryBrush[] =     {  "Brush0",
				  "Brush1",
				  "Brush2",
				  "Brush3",
				  "Brush4",
				  "Brush5",
				  "Brush6",
				  "Brush7",
				  "Brush8",
				  "Brush9",
				  "Brush10",
				  "Brush11",
				  "Brush12",
				  "Brush13",
                  "Brush14",
                  "Brush15",
                  "Brush16",
                  "Brush17",
                  "Brush18"
};
#endif

const char *szRegistryAirspaceMode[] =     {  "AirspaceMode0",
					       "AirspaceMode1",
					       "AirspaceMode2",
					       "AirspaceMode3",
					       "AirspaceMode4",
					       "AirspaceMode5",
					       "AirspaceMode6",
					       "AirspaceMode7",
					       "AirspaceMode8",
					       "AirspaceMode9",
					       "AirspaceMode10",
					       "AirspaceMode11",
					       "AirspaceMode12",
					       "AirspaceMode13",
                           "AirspaceMode14",
                           "AirspaceMode15",
                           "AirspaceMode16",
                           "AirspaceMode17",
                           "AirspaceMode18"
};

char szRegistryAcknowledgementTime[]=	 "AcknowledgementTime1";

char szRegistryAircraftCategory[]= "AircraftCategory1";
char szRegistryAircraftRego[]=  "AircraftRego1";
char szRegistryAircraftType[]=  "AircraftType1";
char szRegistryAirfieldFile[]=  "AirfieldFile";


const char *szRegistryAirspaceFile[]=  {"AirspaceFile","AdditionalAirspaceFile","AirspaceFile3","AirspaceFile4","AirspaceFile5",
                                              "AirspaceFile6","AirspaceFile7","AirspaceFile8","AirspaceFile9"};

char szRegistryAirspaceFillType[]= "AirspaceFillType";
char szRegistryAirspaceOpacity[]= "AirspaceOpacity";
char szRegistryAirspaceWarningDlgTimeout[]= "AirspaceWarningDlgTimeout";
char szRegistryAirspaceWarningMapLabels[]= "AirspaceWarningMapLabels";
char szRegistryAirspaceAckAllSame[]= "AirspaceAckAllSame";
char szRegistryAirspaceWarningRepeatTime[]= "AirspaceWarningRepeatTime1";
char szRegistryAirspaceWarningVerticalMargin[]= "AirspaceWarningVerticalMargin1";
char szRegistryAirspaceWarning[]= "AirspaceWarn";
char szRegistryAlarmMaxAltitude1[]= "AlarmMaxAltitude1";
char szRegistryAlarmMaxAltitude2[]= "AlarmMaxAltitude3";
char szRegistryAlarmMaxAltitude3[]= "AlarmMaxAltitude4";
char szRegistryAlarmTakeoffSafety[]= "AlarmTakeoffSafety";
char szRegistryAltMargin[]=	   "AltMargin1";
char szRegistryAltMode[]=  "AltitudeMode";
char szRegistryAlternate1[]= "Alternate1b";
char szRegistryAlternate2[]= "Alternate2b";
char szRegistryAltitudeUnitsValue[] = "AltitudeUnits";
char szRegistryAppIndLandable[] = "AppIndLandable1";
char szRegistryAppInfoBoxModel[] = "AppInfoBoxModel";
char szRegistryUTF8Symbolsl[] = "UTF8Symbolsl";
char szRegistryAppInverseInfoBox[] = "AppInverseInfoBox2";
char szRegistryArrivalValue[]= "ArrivalValue";
char szRegistryAutoAdvance[] = "AutoAdvance";
char szRegistryAutoBacklight[]= "AutoBacklight";
char szRegistryAutoContrast[]= "AutoContrast";
char szRegistryAutoForceFinalGlide[] = "AutoForceFinalGlide";
char szRegistryMacCready[] = "MacCready";
char szRegistryAutoMcMode[] = "AutoMcMode";
char szRegistryAutoMcStatus[] = "AutoMcStatus";
char szRegistryAutoOrientScale[]= "AutoOrientScale3";
char szRegistryAutoSoundVolume[]= "AutoSoundVolume";
char szRegistryAutoWind[]= "AutoWind";
char szRegistryAutoZoom[] = "AutoZoom";
char szRegistryAverEffTime[]= "AverEffTime1";
char szRegistryBallastSecsToEmpty[]=	 "BallastSecsToEmpty1";
char szRegistryBarOpacity[]= "BarOpacity";
char szRegistryBestWarning[]= "BestWarning";
char szRegistryBgMapColor[]= "BgMapColor";
char szRegistryBit1Index[]= "Bit1Index";
char szRegistryBit2Index[]= "Bit2Index";
char szRegistryBit3Index[]= "Bit3Index";
char szRegistryBit4Index[]= "Bit4Index";
char szRegistryBit5Index[]= "Bit5Index";
char szRegistryBit6Index[]= "Bit6Index";
char szRegistryBugs[]=		 "Bugs";
char szRegistryCheckSum[]=		 "CheckSum1";
char szRegistryCircleZoom[]= "CircleZoom";
char szRegistryClipAlt[]= "ClipAlt1";
char szRegistryCompetitionClass[]=  "CompetitionClass1";
char szRegistryCompetitionID[]=  "CompetitionID1";
char szRegistryConfBB0[] = "ConfBB0a";
char szRegistryConfBB1[] = "ConfBB1";
char szRegistryConfBB2[] = "ConfBB2";
char szRegistryConfBB3[] = "ConfBB3";
char szRegistryConfBB4[] = "ConfBB4";
char szRegistryConfBB5[] = "ConfBB5";
char szRegistryConfBB6[] = "ConfBB6";
char szRegistryConfBB7[] = "ConfBB7";
char szRegistryConfBB8[] = "ConfBB8";
char szRegistryConfBB9[] = "ConfBB9";
char szRegistryConfBB0Auto[] = "ConfBB0Auto";
char szRegistryConfIP11[] = "ConfIP11";
char szRegistryConfIP12[] = "ConfIP12";
char szRegistryConfIP13[] = "ConfIP13";
char szRegistryConfIP14[] = "ConfIP14";
char szRegistryConfIP15[] = "ConfIP15";
char szRegistryConfIP16[] = "ConfIP16";
char szRegistryConfIP17[] = "ConfIP17";
char szRegistryConfIP21[] = "ConfIP21";
char szRegistryConfIP22[] = "ConfIP22";
char szRegistryConfIP23[] = "ConfIP23";
char szRegistryConfIP24[] = "ConfIP24";
char szRegistryConfIP31[] = "ConfIP31";
char szRegistryConfIP32[] = "ConfIP32";
char szRegistryConfIP33[] = "ConfIP33";
char szRegistryCustomKeyModeAircraftIcon[] = "CustomKeyModeAircraftIcon";
char szRegistryCustomKeyModeCenterScreen[] = "CustomKeyModeCenterScreen";
char szRegistryCustomKeyModeCenter[] = "CustomKeyModeCenter";
char szRegistryCustomKeyModeLeftUpCorner[] = "CustomKeyModeLeftUpCorner";
char szRegistryCustomKeyModeLeft[] = "CustomKeyModeLeft";
char szRegistryCustomKeyModeRightUpCorner[] = "CustomKeyModeRightUpCorner";
char szRegistryCustomKeyModeRight[] = "CustomKeyModeRight";
char szRegistryCustomKeyTime[] = "CustomKeyTime";
char szRegistryDebounceTimeout[]= "DebounceTimeout1";
char szRegistryDeclutterMode[]= "DeclutterMode";
char szRegistryDeviceA[]= "DeviceA";
char szRegistryDeviceB[]= "DeviceB";
char szRegistryDeviceC[]= "DeviceC";
char szRegistryDeviceD[]= "DeviceD";
char szRegistryDeviceE[]= "DeviceE";
char szRegistryDeviceF[]= "DeviceF";
char szRegistryDisableAutoLogger[] = "DisableAutoLogger";
char szRegistryLiveTrackerInterval[] = "LiveTrackerInterval";
char szRegistryLiveTrackerRadar_config[] = "LiveTrackerRadar_config";
char szRegistryLiveTrackerStart_config[] = "LiveTrackerStart_config";
char szRegistryDisplayText[] = "DisplayText2";
char szRegistryDisplayUpValue[] = "DisplayUp";
char szRegistryDistanceUnitsValue[] = "DistanceUnits";
char szRegistryEnableFLARMMap[] = "EnableFLARMDisplay1";
char szRegistryEnableNavBaroAltitude[] = "EnableNavBaroAltitude";
char szRegistryFAIFinishHeight[] = "FAIFinishHeight";
char szRegistryFAISector[] = "FAISector";
char szRegistryFinalGlideTerrain[]= "FinalGlideTerrain";
char szRegistryFinishLine[]=		 "FinishLine";
char szRegistryFinishMinHeight[]= "FinishMinHeight";
char szRegistryFinishRadius[]=		 "FinishRadius";
char szRegistryFontRenderer[]= "FontRenderer2";
char szRegistryFontMapWaypoint[]= "FontMapWaypointB";
char szRegistryFontMapTopology[]= "FontMapTopologyB";
char szRegistryFontInfopage1L[]= "FontInfopage1LB";
char szRegistryFontInfopage2L[]= "FontInfopage2LB";
char szRegistryFontBottomBar[]= "FontBottomBarB";
char szRegistryFontOverlayBig[]= "FontOverlayBigB";
char szRegistryFontOverlayMedium[]= "FontOverlayMediumB";
char szRegistryFontVisualGlide[]= "FontVisualGlideB";
char szRegistryFontCustom1[]= "FontCustom1B";


char szRegistryGlideBarMode[]= "GlideBarMode";
char szRegistryGliderScreenPosition[] = "GliderScreenPosition";
char szRegistryGpsAltitudeOffset[] = "GpsAltitudeOffset1";
char szRegistryHandicap[] = "Handicap1";
char szRegistryHideUnits[]= "HideUnits";
char szRegistryHomeWaypoint[]= "HomeWaypoint1b";
char szRegistryInputFile[]=  "InputFile";
char szRegistryIphoneGestures[]= "IphoneGestures1";
char szRegistryLKMaxLabels[]= "LKMaxLabels";
char szRegistryLKTopoZoomCat05[]= "LKTopoZoomCat05d";
char szRegistryLKTopoZoomCat100[]= "LKTopoZoomCat100a";
char szRegistryLKTopoZoomCat10[]= "LKTopoZoomCat10d";
char szRegistryLKTopoZoomCat110[]= "LKTopoZoomCat110a";
char szRegistryLKTopoZoomCat20[]= "LKTopoZoomCat20a";
char szRegistryLKTopoZoomCat30[]= "LKTopoZoomCat30a";
char szRegistryLKTopoZoomCat40[]= "LKTopoZoomCat40a";
char szRegistryLKTopoZoomCat50[]= "LKTopoZoomCat50a";
char szRegistryLKTopoZoomCat60[]= "LKTopoZoomCat60a";
char szRegistryLKTopoZoomCat70[]= "LKTopoZoomCat70a";
char szRegistryLKTopoZoomCat80[]= "LKTopoZoomCat80a";
char szRegistryLKTopoZoomCat90[]= "LKTopoZoomCat90a";
char szRegistryLKVarioBar[]= "LKVarioBar";
char szRegistryLKVarioVal[]= "LKVarioVal";
char szRegistryLanguageFile[]=  "LanguageFile";
char szRegistryLatLonUnits[] = "LatLonUnits";
char szRegistryLiftUnitsValue[] = "LiftUnits";
char szRegistryLockSettingsInFlight[] = "LockSettingsInFlight";
char szRegistryLoggerShort[]=  "LoggerShortName";
char szRegistryMapBox[]= "MapBox";
char szRegistryMapFile[]= "MapFile";
char szRegistryMenuTimeout[] = "MenuTimeout";
char szRegistryNewMapDeclutter[]= "NewMapDeclutter";
char szRegistryOrbiter[] = "Orbiter";
char szRegistryOutlinedTp[]= "OutlinedTp1";
char szRegistryOverColor[]= "OverColor";
char szRegistryOverlayClock[] = "OverlayClock";
char szRegistryUseTwoLines[] = "UseTwoLines1";
char szRegistrySonarWarning[] = "SonarWarning";
char szRegistryOverlaySize[]= "OverlaySize";
char szRegistryAutoZoomThreshold[]= "AutoZoomThreshold";
char szRegistryClimbZoom[]= "ClimbZoom";
char szRegistryCruiseZoom[]= "CruiseZoom";
char szRegistryMaxAutoZoom[]= "MaxAutoZoom";
char szRegistryPGOptimizeRoute[]= "PGOptimizeRoute";
char szRegistryGliderSymbol[]= "GliderSymbol";
char szRegistryPilotName[]=  "PilotName1";
char szRegistryLiveTrackersrv[]=  "LiveTrackersrv";
char szRegistryLiveTrackerport[]=  "LiveTrackerport";
char szRegistryLiveTrackerusr[]=  "LiveTrackerusr";
char szRegistryLiveTrackerpwd[]=  "LiveTrackerpwd";
char szRegistryPolarFile[] = "PolarFile1";
char szRegistryPollingMode[]= "PollingMode";
char szRegistryPort1Index[]= "PortIndex";
char szRegistryPort2Index[]= "Port2Index";
char szRegistryPort1Name[]= "Port1Name";
char szRegistryPort2Name[]= "Port2Name";
char szRegistryPort3Name[]= "Port3Name";
char szRegistryPort4Name[]= "Port4Name";
char szRegistryPort5Name[]= "Port5Name";
char szRegistryPort6Name[]= "Port6Name";
char szRegistryIpAddress1[]= "IpAddress1";
char szRegistryIpAddress2[]= "IpAddress2";
char szRegistryIpAddress3[]= "IpAddress3";
char szRegistryIpAddress4[]= "IpAddress4";
char szRegistryIpAddress5[]= "IpAddress5";
char szRegistryIpAddress6[]= "IpAddress6";
char szRegistryIOValues[]= "Val_IO_Dir";
char szRegistryIpPort1[]= "IpPort1";
char szRegistryIpPort2[]= "IpPort2";
char szRegistryIpPort3[]= "IpPort3";
char szRegistryIpPort4[]= "IpPort4";
char szRegistryIpPort5[]= "IpPort5";
char szRegistryIpPort6[]= "IpPort6";
char szRegistryPressureHg[] = "PressureHg";
char szRegistrySafetyAltitudeArrival[] =     "SafetyAltitudeArrival1";
char szRegistrySafetyAltitudeMode[]=  "SafetyAltitudeMode";
char szRegistrySafetyAltitudeTerrain[] =     "SafetyAltitudeTerrain1";
char szRegistrySafetyMacCready[] = "SafetyMacCready";
char szRegistrySafteySpeed[] =          "SafteySpeed1";
char szRegistrySectorRadius[]=          "Radius";
char szRegistrySetSystemTimeFromGPS[] = "SetSystemTimeFromGPS";
char szRegistrySaveRuntime[] = "SaveRuntime";
char szRegistryShading[] = "Shading";
char szRegistryIsoLine[] = "IsoLine";
char szRegistrySnailTrail[]=		 "SnailTrail";
char szRegistrySnailScale[] = "SnailScale";
char szRegistrySpeed1Index[]=		 "SpeedIndex";
char szRegistrySpeed2Index[]=		 "Speed2Index";
char szRegistrySpeed3Index[]=            "Speed3Index";
char szRegistrySpeed4Index[]=            "Speed4Index";
char szRegistrySpeed5Index[]=            "Speed5Index";
char szRegistrySpeed6Index[]=            "Speed6Index";
char szRegistrySpeedUnitsValue[] =      "SpeedUnits";
char szRegistryStartHeightRef[] = "StartHeightRef";
char szRegistryStartLine[]=		 "StartLine";
char szRegistryStartMaxHeightMargin[]= "StartMaxHeightMargin";
char szRegistryStartMaxHeight[]= "StartMaxHeight";
char szRegistryStartMaxSpeedMargin[]= "StartMaxSpeedMargin";
char szRegistryStartMaxSpeed[]= "StartMaxSpeed";
char szRegistryStartRadius[]=		 "StartRadius";
char szRegistryTaskSpeedUnitsValue[] =      "TaskSpeedUnits";
char szRegistryTeamcodeRefWaypoint[] = "TeamcodeRefWaypoint1";
char szRegistryTerrainBrightness[] = "TerrainBrightness1";
char szRegistryTerrainContrast[] = "TerrainContrast1";
char szRegistryTerrainFile[]=	 "TerrainFile";
char szRegistryTerrainRamp[] = "TerrainRamp";
char szRegistryTerrainWhiteness[] = "TerrainWhiteness";
char szRegistryThermalBar[]= "ThermalBar";
char szRegistryThermalLocator[]=	 "ThermalLocator";
char szRegistryTpFilter[]= "TpFilter";
char szRegistryTrackBar[]= "TrackBar";
char szRegistryTrailDrift[]=		 "TrailDrift";
char szRegistryUTCOffset[] = "UTCOffset";
char szRegistryUseGeoidSeparation[] = "UseGeoidSeparation";
char szRegistryUseExtSound1[] = "UseExtSound1";
char szRegistryUseExtSound2[] = "UseExtSound2";
char szRegistryUseExtSound3[] = "UseExtSound3";
char szRegistryUseExtSound4[] = "UseExtSound4";
char szRegistryUseExtSound5[] = "UseExtSound5";
char szRegistryUseExtSound6[] = "UseExtSound6";
char szRegistryUseUngestures[] = "UseUngestures";
char szRegistryUseTotalEnergy[] = "UseTotalEnergy";
char szRegistryWarningTime[]=		 "WarnTime";
const char *szRegistryWayPointFile[]= { "WPFile", "AdditionalWPFile","WPFile3","WPFile4","WPFile5","WPFile6","WPFile7","WPFile8","WPFile9"};
char szRegistryWaypointsOutOfRange[] = "WaypointsOutOfRange2";
char szRegistryWindCalcSpeed[] =          "WindCalcSpeed";
char szRegistryWindCalcTime[] =          "WindCalcTime";
char szRegistryCustomMenu1[] = "CustomMenu1a";
char szRegistryCustomMenu2[] = "CustomMenu2a";
char szRegistryCustomMenu3[] = "CustomMenu3a";
char szRegistryCustomMenu4[] = "CustomMenu4a";
char szRegistryCustomMenu5[] = "CustomMenu5a";
char szRegistryCustomMenu6[] = "CustomMenu6a";
char szRegistryCustomMenu7[] = "CustomMenu7a";
char szRegistryCustomMenu8[] = "CustomMenu8a";
char szRegistryCustomMenu9[] = "CustomMenu9a";
char szRegistryCustomMenu10[] = "CustomMenu10a";
char szRegistryUseWindRose[] = "UseWindRose";
char szRegistryMultiTerr0[] = "MultimapTerrain0";
char szRegistryMultiTerr1[] = "MultimapTerrain1";
char szRegistryMultiTerr2[] = "MultimapTerrain2";
char szRegistryMultiTerr3[] = "MultimapTerrain3";
char szRegistryMultiTerr4[] = "MultimapTerrain4";
char szRegistryMultiTopo0[] = "MultimapTopology0";
char szRegistryMultiTopo1[] = "MultimapTopology1";
char szRegistryMultiTopo2[] = "MultimapTopology2";
char szRegistryMultiTopo3[] = "MultimapTopology3";
char szRegistryMultiTopo4[] = "MultimapTopology4";
char szRegistryMultiAsp0[] = "MultimapAirspace0";
char szRegistryMultiAsp1[] = "MultimapAirspace1";
char szRegistryMultiAsp2[] = "MultimapAirspace2";
char szRegistryMultiAsp3[] = "MultimapAirspace3";
char szRegistryMultiAsp4[] = "MultimapAirspace4";
char szRegistryMultiLab0[] = "MultimapLab0";
char szRegistryMultiLab1[] = "MultimapLab1";
char szRegistryMultiLab2[] = "MultimapLab2";
char szRegistryMultiLab3[] = "MultimapLab3";
char szRegistryMultiLab4[] = "MultimapLab4";
char szRegistryMultiWpt0[] = "MultimapWpt0";
char szRegistryMultiWpt1[] = "MultimapWpt1";
char szRegistryMultiWpt2[] = "MultimapWpt2";
char szRegistryMultiWpt3[] = "MultimapWpt3";
char szRegistryMultiWpt4[] = "MultimapWpt4";
char szRegistryMultiOvrT0[] = "MultimapOvrT0";
char szRegistryMultiOvrT1[] = "MultimapOvrT1";
char szRegistryMultiOvrT2[] = "MultimapOvrT2";
char szRegistryMultiOvrT3[] = "MultimapOvrT3";
char szRegistryMultiOvrT4[] = "MultimapOvrT4";
char szRegistryMultiOvrG0[] = "MultimapOvrG0";
char szRegistryMultiOvrG1[] = "MultimapOvrG1";
char szRegistryMultiOvrG2[] = "MultimapOvrG2";
char szRegistryMultiOvrG3[] = "MultimapOvrG3";
char szRegistryMultiOvrG4[] = "MultimapOvrG4";
char szRegistryMultiSizeY1[]= "MultimapSizeY1";
char szRegistryMultiSizeY2[]= "MultimapSizeY2";
char szRegistryMultiSizeY3[]= "MultimapSizeY3";
char szRegistryMultiSizeY4[]= "MultimapSizeY4";

char szRegistryMultimap1[]= "Multimap1a";
char szRegistryMultimap2[]= "Multimap2a";
char szRegistryMultimap3[]= "Multimap3a";
char szRegistryMultimap4[]= "Multimap4a";
char szRegistryMultimap5[]= "Multimap5a";

char szRegistryMMNorthUp1[]= "MultimapNorthUp1";
char szRegistryMMNorthUp2[]= "MultimapNorthUp2";
char szRegistryMMNorthUp3[]= "MultimapNorthUp3";
char szRegistryMMNorthUp4[]= "MultimapNorthUp4";

char szRegistryAspPermanent[]     = "AirspacePermMod";
char szRegistryFlarmDirection[]   = "FlarmDirection";
char szRegistryDrawTask[]         = "DrawTask";
char szRegistryDrawFAI[]          = "DrawFAI";
char szRegistryGearMode[]         = "GearMode";
char szRegistryGearAltitude[]     = "GearAltitude";
char szRegistryBottomMode[]       = "ActiveBottomBar";
char szRegistryBigFAIThreshold[]  = "FAI_28_45_Threshold";
char szRegistryDrawXC[]      = "DrawXC";

char szRegistryScreenSize[]       = "ScreenSize";
char szRegistryScreenSizeX[]      = "ScreenSizeX";
char szRegistryScreenSizeY[]      = "ScreenSizeY";

char szRegistryOverlay_TopLeft[]       = "Overlay_TopLeft";
char szRegistryOverlay_TopMid[]       = "Overlay_TopMid";
char szRegistryOverlay_TopRight[]       = "Overlay_TopRight";
char szRegistryOverlay_TopDown[]       = "Overlay_TopDown";
char szRegistryOverlay_LeftTop[]       = "Overlay_LeftTop";
char szRegistryOverlay_LeftMid[]       = "Overlay_LeftMid";
char szRegistryOverlay_LeftBottom[]       = "Overlay_LeftBottom";
char szRegistryOverlay_LeftDown[]       = "Overlay_LeftDown";
char szRegistryOverlay_RightTop[]       = "Overlay_RightTop";
char szRegistryOverlay_RightMid[]       = "Overlay_RightMid";
char szRegistryOverlay_RightBottom[]       = "Overlay_RightBottom";
char szRegistryAdditionalContestRule[]       = "Additional_Contest_Rule";
#ifdef _WGS84
char szRegistry_earth_model_wgs84[] = "earth_model_wgs84";
#endif
//
//
//
#else
//
// ------------------------------------- externals ------------------------------------------
//
extern const char *szRegistryAirspaceMode[];	// 18
extern const char *szRegistryBrush[];	// 18
extern const char *szRegistryColour[];	// 18
extern const char *szRegistryDisplayType[];	// MAXINFOWINDOWS
extern const char szRegistryAcknowledgementTime[];
extern const char szRegistryAdditionalAirspaceFile[];
extern const char szRegistryAdditionalWayPointFile[];
extern const char szRegistryAircraftCategory[];
extern const char szRegistryAircraftRego[];
extern const char szRegistryAircraftType[];
extern const char szRegistryAirfieldFile[];
extern const char *szRegistryAirspaceFile[];
extern const char szRegistryAirspaceFillType[];
extern const char szRegistryAirspaceOpacity[];
extern const char szRegistryAirspaceWarningDlgTimeout[];
extern const char szRegistryAirspaceWarningMapLabels[];
extern const char szRegistryAirspaceAckAllSame[];
extern const char szRegistryAirspaceWarningRepeatTime[];
extern const char szRegistryAirspaceWarningVerticalMargin[];
extern const char szRegistryAirspaceWarning[];
extern const char szRegistryAlarmMaxAltitude1[];
extern const char szRegistryAlarmMaxAltitude2[];
extern const char szRegistryAlarmMaxAltitude3[];
extern const char szRegistryAlarmTakeoffSafety[];
extern const char szRegistryAltMargin[];
extern const char szRegistryAltMode[];
extern const char szRegistryAlternate1[];
extern const char szRegistryAlternate2[];
extern const char szRegistryAltitudeUnitsValue[];
extern const char szRegistryAppIndLandable[];
extern const char szRegistryUTF8Symbolsl[];
extern const char szRegistryAppInfoBoxModel[];
extern const char szRegistryAppInverseInfoBox[];
extern const char szRegistryArrivalValue[];
extern const char szRegistryAutoAdvance[];
extern const char szRegistryAutoBacklight[];
extern const char szRegistryAutoContrast[];
extern const char szRegistryAutoForceFinalGlide[];
extern const char szRegistryAutoMcMode[];
extern const char szRegistryMacCready[];
extern const char szRegistryAutoMcStatus[];
extern const char szRegistryAutoOrientScale[];
extern const char szRegistryAutoSoundVolume[];
extern const char szRegistryAutoWind[];
extern const char szRegistryAutoZoom[];
extern const char szRegistryAverEffTime[];
extern const char szRegistryBallastSecsToEmpty[];
extern const char szRegistryBarOpacity[];
extern const char szRegistryBestWarning[];
extern const char szRegistryBgMapColor[];
extern const char szRegistryBit1Index[];
extern const char szRegistryBit2Index[];
extern const char szRegistryBit3Index[];
extern const char szRegistryBit4Index[];
extern const char szRegistryBit5Index[];
extern const char szRegistryBit6Index[];
extern const char szRegistryBugs[];
extern const char szRegistryCheckSum[];
extern const char szRegistryCircleZoom[];
extern const char szRegistryClipAlt[];
extern const char szRegistryCompetitionClass[];
extern const char szRegistryCompetitionID[];
extern const char szRegistryConfBB0[];
extern const char szRegistryConfBB1[];
extern const char szRegistryConfBB2[];
extern const char szRegistryConfBB3[];
extern const char szRegistryConfBB4[];
extern const char szRegistryConfBB5[];
extern const char szRegistryConfBB6[];
extern const char szRegistryConfBB7[];
extern const char szRegistryConfBB8[];
extern const char szRegistryConfBB9[];
extern const char szRegistryConfBB0Auto[];
extern const char szRegistryConfIP11[];
extern const char szRegistryConfIP12[];
extern const char szRegistryConfIP13[];
extern const char szRegistryConfIP14[];
extern const char szRegistryConfIP15[];
extern const char szRegistryConfIP16[];
extern const char szRegistryConfIP17[];
extern const char szRegistryConfIP21[];
extern const char szRegistryConfIP22[];
extern const char szRegistryConfIP23[];
extern const char szRegistryConfIP24[];
extern const char szRegistryConfIP31[];
extern const char szRegistryConfIP32[];
extern const char szRegistryConfIP33[];
extern const char szRegistryCustomKeyModeAircraftIcon[];
extern const char szRegistryCustomKeyModeCenterScreen[];
extern const char szRegistryCustomKeyModeCenter[];
extern const char szRegistryCustomKeyModeLeftUpCorner[];
extern const char szRegistryCustomKeyModeLeft[];
extern const char szRegistryCustomKeyModeRightUpCorner[];
extern const char szRegistryCustomKeyModeRight[];
extern const char szRegistryCustomKeyTime[];
extern const char szRegistryDebounceTimeout[];
extern const char szRegistryDeclutterMode[];
extern const char szRegistryDeviceA[];
extern const char szRegistryDeviceB[];
extern const char szRegistryDeviceC[];
extern const char szRegistryDeviceD[];
extern const char szRegistryDeviceE[];
extern const char szRegistryDeviceF[];
extern const char szRegistryDisableAutoLogger[];
extern const char szRegistryLiveTrackerInterval[];
extern const char szRegistryLiveTrackerRadar_config[];
extern const char szRegistryLiveTrackerStart_config[];
extern const char szRegistryDisplayText[];
extern const char szRegistryDisplayUpValue[];
extern const char szRegistryDistanceUnitsValue[];
extern const char szRegistryEnableFLARMMap[];
extern const char szRegistryEnableNavBaroAltitude[];
extern const char szRegistryFAIFinishHeight[];
extern const char szRegistryFAISector[];
extern const char szRegistryFinalGlideTerrain[];
extern const char szRegistryFinishLine[];
extern const char szRegistryFinishMinHeight[];
extern const char szRegistryFinishRadius[];
extern const char szRegistryFontRenderer[];
extern const char szRegistryFontMapWaypoint[];
extern const char szRegistryFontMapTopology[];
extern const char szRegistryFontInfopage1L[];
extern const char szRegistryFontInfopage2L[];
extern const char szRegistryFontBottomBar[];
extern const char szRegistryFontOverlayBig[];
extern const char szRegistryFontOverlayMedium[];
extern const char szRegistryFontCustom1[];
extern const char szRegistryFontVisualGlide[];
extern const char szRegistryGlideBarMode[];
extern const char szRegistryGliderScreenPosition[];
extern const char szRegistryGpsAltitudeOffset[];
extern const char szRegistryHandicap[];
extern const char szRegistryHideUnits[];
extern const char szRegistryHomeWaypoint[];
extern const char szRegistryInputFile[];
extern const char szRegistryIphoneGestures[];
extern const char szRegistryLKMaxLabels[];
extern const char szRegistryLKTopoZoomCat05[];
extern const char szRegistryLKTopoZoomCat100[];
extern const char szRegistryLKTopoZoomCat10[];
extern const char szRegistryLKTopoZoomCat110[];
extern const char szRegistryLKTopoZoomCat20[];
extern const char szRegistryLKTopoZoomCat30[];
extern const char szRegistryLKTopoZoomCat40[];
extern const char szRegistryLKTopoZoomCat50[];
extern const char szRegistryLKTopoZoomCat60[];
extern const char szRegistryLKTopoZoomCat70[];
extern const char szRegistryLKTopoZoomCat80[];
extern const char szRegistryLKTopoZoomCat90[];
extern const char szRegistryLKVarioBar[];
extern const char szRegistryLKVarioVal[];
extern const char szRegistryLanguageFile[];
extern const char szRegistryLatLonUnits[];
extern const char szRegistryLiftUnitsValue[];
extern const char szRegistryLockSettingsInFlight[];
extern const char szRegistryLoggerShort[];
extern const char szRegistryMapBox[];
extern const char szRegistryMapFile[];
extern const char szRegistryMenuTimeout[];
extern const char szRegistryNewMapDeclutter[];
extern const char szRegistryOrbiter[];
extern const char szRegistryOutlinedTp[];
extern const char szRegistryOverColor[];
extern const char szRegistryOverlayClock[];
extern const char szRegistryUseTwoLines[];
extern const char szRegistrySonarWarning[];
extern const char szRegistryOverlaySize[];
extern const char szRegistryAutoZoomThreshold[];
extern const char szRegistryClimbZoom[];
extern const char szRegistryCruiseZoom[];
extern const char szRegistryMaxAutoZoom[];
extern const char szRegistryPGOptimizeRoute[];
extern const char szRegistryGliderSymbol[];
extern const char szRegistryPilotName[];
extern const char szRegistryLiveTrackersrv[];
extern const char szRegistryLiveTrackerport[];
extern const char szRegistryLiveTrackerusr[];
extern const char szRegistryLiveTrackerpwd[];
extern const char szRegistryPolarFile[];
extern const char szRegistryPollingMode[];
extern const char szRegistryPort1Index[];
extern const char szRegistryPort2Index[];
extern const char szRegistryPort3Index[];
extern const char szRegistryPort4Index[];
extern const char szRegistryPort5Index[];
extern const char szRegistryPort6Index[];
extern const char szRegistryPort1Name[];
extern const char szRegistryPort2Name[];
extern const char szRegistryPort3Name[];
extern const char szRegistryPort4Name[];
extern const char szRegistryPort5Name[];
extern const char szRegistryPort6Name[];
extern const char szRegistryIpAddress1[];
extern const char szRegistryIpAddress2[];
extern const char szRegistryIpAddress3[];
extern const char szRegistryIpAddress4[];
extern const char szRegistryIpAddress5[];
extern const char szRegistryIpAddress6[];
extern const char szRegistryIpPort1[];
extern const char szRegistryIpPort2[];
extern const char szRegistryIpPort3[];
extern const char szRegistryIpPort4[];
extern const char szRegistryIpPort5[];
extern const char szRegistryIpPort6[];

extern const char szRegistryIOValues[];

extern const char szRegistryPressureHg[];
extern const char szRegistrySafetyAltitudeArrival[];
extern const char szRegistrySafetyAltitudeMode[];
extern const char szRegistrySafetyAltitudeTerrain[];
extern const char szRegistrySafetyMacCready[];
extern const char szRegistrySafteySpeed[];
extern const char szRegistrySectorRadius[];
extern const char szRegistrySetSystemTimeFromGPS[];
extern const char szRegistrySaveRuntime[];
extern const char szRegistryShading[];
extern const char szRegistryIsoLine[];
extern const char szRegistrySnailTrail[];
extern const char szRegistrySnailScale[];
extern const char szRegistrySpeed1Index[];
extern const char szRegistrySpeed2Index[];
extern const char szRegistrySpeed3Index[];
extern const char szRegistrySpeed4Index[];
extern const char szRegistrySpeed5Index[];
extern const char szRegistrySpeed6Index[];
extern const char szRegistrySpeedUnitsValue[];
extern const char szRegistryStartHeightRef[];
extern const char szRegistryStartLine[];
extern const char szRegistryStartMaxHeightMargin[];
extern const char szRegistryStartMaxHeight[];
extern const char szRegistryStartMaxSpeedMargin[];
extern const char szRegistryStartMaxSpeed[];
extern const char szRegistryStartRadius[];
extern const char szRegistryStatusFile[];
extern const char szRegistryTaskSpeedUnitsValue[];
extern const char szRegistryTeamcodeRefWaypoint[];
extern const char szRegistryTerrainBrightness[];
extern const char szRegistryTerrainContrast[];
extern const char szRegistryTerrainFile[];
extern const char szRegistryTerrainRamp[];
extern const char szRegistryTerrainWhiteness[];
extern const char szRegistryThermalBar[];
extern const char szRegistryThermalLocator[];
extern const char szRegistryTpFilter[];
extern const char szRegistryTrackBar[];
extern const char szRegistryTrailDrift[];
extern const char szRegistryUTCOffset[];
extern const char szRegistryUseGeoidSeparation[];
extern const char szRegistryUseExtSound1[];
extern const char szRegistryUseExtSound2[];
extern const char szRegistryUseExtSound3[];
extern const char szRegistryUseExtSound4[];
extern const char szRegistryUseExtSound5[];
extern const char szRegistryUseExtSound6[];
extern const char szRegistryUseUngestures[];
extern const char szRegistryUseTotalEnergy[];
extern const char szRegistryWarningTime[];
extern const char *szRegistryWayPointFile[];
extern const char szRegistryWaypointsOutOfRange[];
extern const char szRegistryWindCalcSpeed[];
extern const char szRegistryWindCalcTime[];
extern const char szRegistryCustomMenu1[];
extern const char szRegistryCustomMenu2[];
extern const char szRegistryCustomMenu3[];
extern const char szRegistryCustomMenu4[];
extern const char szRegistryCustomMenu5[];
extern const char szRegistryCustomMenu6[];
extern const char szRegistryCustomMenu7[];
extern const char szRegistryCustomMenu8[];
extern const char szRegistryCustomMenu9[];
extern const char szRegistryCustomMenu10[];
extern const char szRegistryUseWindRose[];
extern const char szRegistryMultiTerr0[];
extern const char szRegistryMultiTerr1[];
extern const char szRegistryMultiTerr2[];
extern const char szRegistryMultiTerr3[];
extern const char szRegistryMultiTerr4[];
extern const char szRegistryMultiTopo0[];
extern const char szRegistryMultiTopo1[];
extern const char szRegistryMultiTopo2[];
extern const char szRegistryMultiTopo3[];
extern const char szRegistryMultiTopo4[];
extern const char szRegistryMultiAsp0[];
extern const char szRegistryMultiAsp1[];
extern const char szRegistryMultiAsp2[];
extern const char szRegistryMultiAsp3[];
extern const char szRegistryMultiAsp4[];
extern const char szRegistryMultiLab0[];
extern const char szRegistryMultiLab1[];
extern const char szRegistryMultiLab2[];
extern const char szRegistryMultiLab3[];
extern const char szRegistryMultiLab4[];
extern const char szRegistryMultiWpt0[];
extern const char szRegistryMultiWpt1[];
extern const char szRegistryMultiWpt2[];
extern const char szRegistryMultiWpt3[];
extern const char szRegistryMultiWpt4[];
extern const char szRegistryMultiOvrT0[];
extern const char szRegistryMultiOvrT1[];
extern const char szRegistryMultiOvrT2[];
extern const char szRegistryMultiOvrT3[];
extern const char szRegistryMultiOvrT4[];
extern const char szRegistryMultiOvrG0[];
extern const char szRegistryMultiOvrG1[];
extern const char szRegistryMultiOvrG2[];
extern const char szRegistryMultiOvrG3[];
extern const char szRegistryMultiOvrG4[];
extern const char szRegistryMultiSizeY1[];
extern const char szRegistryMultiSizeY2[];
extern const char szRegistryMultiSizeY3[];
extern const char szRegistryMultiSizeY4[];

extern const char szRegistryMultimap1[];
extern const char szRegistryMultimap2[];
extern const char szRegistryMultimap3[];
extern const char szRegistryMultimap4[];
extern const char szRegistryMultimap5[];

extern const char szRegistryMMNorthUp1[];
extern const char szRegistryMMNorthUp2[];
extern const char szRegistryMMNorthUp3[];
extern const char szRegistryMMNorthUp4[];

extern const char szRegistryAspPermanent[];
extern const char szRegistryFlarmDirection[];
extern const char szRegistryDrawTask[];
extern const char szRegistryDrawFAI[] ;
extern const char szRegistryGearMode[];
extern const char szRegistryGearAltitude[];
extern const char szRegistryBottomMode[];
extern const char szRegistryBigFAIThreshold[];
extern const char szRegistryDrawXC[];

extern const char szRegistryScreenSize[];
extern const char szRegistryScreenSizeX[];
extern const char szRegistryScreenSizeY[];

extern const char szRegistryOverlay_TopLeft[];
extern const char szRegistryOverlay_TopMid[];
extern const char szRegistryOverlay_TopRight[];
extern const char szRegistryOverlay_TopDown[];
extern const char szRegistryOverlay_LeftTop[];
extern const char szRegistryOverlay_LeftMid[];
extern const char szRegistryOverlay_LeftBottom[];
extern const char szRegistryOverlay_LeftDown[];
extern const char szRegistryOverlay_RightTop[];
extern const char szRegistryOverlay_RightMid[];
extern const char szRegistryOverlay_RightBottom[];
extern const char szRegistryAdditionalContestRule[];

#ifdef _WGS84
extern const char szRegistry_earth_model_wgs84[];
#endif

//
//
#endif

extern void LKProfileResetDefault(void);
extern void LKProfileInitRuntime(void);
extern bool LKProfileLoad(const TCHAR *szFile);
extern void LKProfileSave(const TCHAR *szFile);

#endif // LKPROFILES_H
