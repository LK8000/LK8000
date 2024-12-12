/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */

#include "utils/stl_utils.h"
#include "externs.h"
#include "LKProfiles.h"
#include "MapWindow.h"

const char *const szRegistryDisplayType[] = {
        "Info0",
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
static_assert(std::size(InfoType) == std::size(szRegistryDisplayType),
              "invalid array size");

const char *const szRegistryColour[] = {
        "Colour0",
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
static_assert(std::size(MapWindow::iAirspaceColour) == std::size(szRegistryColour),
              "invalid array size");


#ifdef HAVE_HATCHED_BRUSH
const char * const szRegistryBrush[] = {
        "Brush0",
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
static_assert(std::size(MapWindow::iAirspaceBrush) == std::size(szRegistryBrush),
              "invalid array size");
#endif

const char *const szRegistryAirspaceMode[] = {
        "AirspaceMode0",
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
static_assert(std::size(MapWindow::iAirspaceMode) == std::size(szRegistryAirspaceMode),
              "invalid array size");

const char szRegistryAcknowledgementTime[] = "AcknowledgementTime1";

const char szRegistryAircraftCategory[] = "AircraftCategory1";
const char szRegistryAircraftRego[] = "AircraftRego1";
const char szRegistryAircraftType[] = "AircraftType1";
const char szRegistryAirfieldFile[] = "AirfieldFile";


const char *const szRegistryAirspaceFile[] = {
        "AirspaceFile",
        "AdditionalAirspaceFile",
        "AirspaceFile3",
        "AirspaceFile4",
        "AirspaceFile5",
        "AirspaceFile6",
        "AirspaceFile7",
        "AirspaceFile8",
        "AirspaceFile9"
};
static_assert(std::size(szAirspaceFile) == std::size(szRegistryAirspaceFile),
              "invalid array size");


const char szRegistryAirspaceFillType[] = "AirspaceFillType";
const char szRegistryAirspaceOpacity[] = "AirspaceOpacity";
const char szRegistryAirspaceWarningDlgTimeout[] = "AirspaceWarningDlgTimeout";
const char szRegistryAirspaceWarningMapLabels[] = "AirspaceWarningMapLabels";
const char szRegistryAirspaceAckAllSame[] = "AirspaceAckAllSame";
const char szRegistryAirspaceWarningRepeatTime[] = "AirspaceWarningRepeatTime1";
const char szRegistryAirspaceWarningVerticalMargin[] = "AirspaceWarningVerticalMargin1";
const char szRegistryAirspaceWarning[] = "AirspaceWarn";
const char szRegistryAlarmMaxAltitude1[] = "AlarmMaxAltitude1";
const char szRegistryAlarmMaxAltitude2[] = "AlarmMaxAltitude3";
const char szRegistryAlarmMaxAltitude3[] = "AlarmMaxAltitude4";
const char szRegistryAlarmTakeoffSafety[] = "AlarmTakeoffSafety";
const char szRegistryAltMargin[] = "AltMargin1";
const char szRegistryAltMode[] = "AltitudeMode";
const char szRegistryAlternate1[] = "Alternate1b";
const char szRegistryAlternate2[] = "Alternate2b";
const char szRegistryAppIndLandable[] = "AppIndLandable1";
const char szRegistryUTF8Symbolsl[] = "UTF8Symbolsl";
const char szRegistryAppInverseInfoBox[] = "AppInverseInfoBox2";
const char szRegistryArrivalValue[] = "ArrivalValue";
const char szRegistryAutoAdvance[] = "AutoAdvance";
const char szRegistryAutoBacklight[] = "AutoBacklight";
const char szRegistryAutoContrast[] = "AutoContrast";
const char szRegistryAutoForceFinalGlide[] = "AutoForceFinalGlide";
const char szRegistryMacCready[] = "MacCready";
const char szRegistryAutoMcMode[] = "AutoMcMode";
const char szRegistryAutoMcStatus[] = "AutoMcStatus";
const char szRegistryAutoOrientScale[] = "AutoOrientScale3";
const char szRegistryAutoSoundVolume[] = "AutoSoundVolume";
const char szRegistryAutoWind[] = "AutoWind";
const char szRegistryAutoZoom[] = "AutoZoom";
const char szRegistryAverEffTime[] = "AverEffTime1";
const char szRegistryBallastSecsToEmpty[] = "BallastSecsToEmpty1";
const char szRegistryBarOpacity[] = "BarOpacity";
const char szRegistryBestWarning[] = "BestWarning";
const char szRegistryBgMapColor[] = "BgMapColor";
const char szRegistryBugs[] = "Bugs";
const char szRegistryCheckSum[] = "CheckSum1";
const char szRegistryCircleZoom[] = "CircleZoom";
const char szRegistryClipAlt[] = "ClipAlt1";
const char szRegistryCompetitionClass[] = "CompetitionClass1";
const char szRegistryCompetitionID[] = "CompetitionID1";
const char szRegistryConfBB0[] = "ConfBB0a";
const char szRegistryConfBB1[] = "ConfBB1";
const char szRegistryConfBB2[] = "ConfBB2";
const char szRegistryConfBB3[] = "ConfBB3";
const char szRegistryConfBB4[] = "ConfBB4";
const char szRegistryConfBB5[] = "ConfBB5";
const char szRegistryConfBB6[] = "ConfBB6";
const char szRegistryConfBB7[] = "ConfBB7";
const char szRegistryConfBB8[] = "ConfBB8";
const char szRegistryConfBB9[] = "ConfBB9";
const char szRegistryConfBB0Auto[] = "ConfBB0Auto";
const char szRegistryConfIP11[] = "ConfIP11";
const char szRegistryConfIP12[] = "ConfIP12";
const char szRegistryConfIP13[] = "ConfIP13";
const char szRegistryConfIP14[] = "ConfIP14";
const char szRegistryConfIP15[] = "ConfIP15";
const char szRegistryConfIP16[] = "ConfIP16";
const char szRegistryConfIP17[] = "ConfIP17";
const char szRegistryConfIP21[] = "ConfIP21";
const char szRegistryConfIP22[] = "ConfIP22";
const char szRegistryConfIP23[] = "ConfIP23";
const char szRegistryConfIP24[] = "ConfIP24";
const char szRegistryConfIP31[] = "ConfIP31";
const char szRegistryConfIP32[] = "ConfIP32";
const char szRegistryConfIP33[] = "ConfIP33";
const char szRegistryCustomKeyModeAircraftIcon[] = "CustomKeyModeAircraftIcon";
const char szRegistryCustomKeyModeCenter[] = "CustomKeyModeCenter";
const char szRegistryCustomKeyModeLeftUpCorner[] = "CustomKeyModeLeftUpCorner";
const char szRegistryCustomKeyModeLeft[] = "CustomKeyModeLeft";
const char szRegistryCustomKeyModeRightUpCorner[] = "CustomKeyModeRightUpCorner";
const char szRegistryCustomKeyModeRight[] = "CustomKeyModeRight";
const char szRegistryCustomKeyTime[] = "CustomKeyTime";
const char szRegistryDebounceTimeout[] = "DebounceTimeout1";
const char szRegistryDeclutterMode[] = "DeclutterMode";

const char* szRegistryReplayFile[] = {
        "NMEA_Replay_FileA",
        "NMEA_Replay_FileB",
        "NMEA_Replay_FileC",
        "NMEA_Replay_FileD",
        "NMEA_Replay_FileE",
        "NMEA_Replay_FileF"
};

static_assert(std::size(PortConfig) == std::size(szRegistryReplayFile),
              "invalid array size");

const char* szRegistryReplaySpeed[] = {
        "NMEA_Replay_SpeedA",
        "NMEA_Replay_SpeedB",
        "NMEA_Replay_SpeedC",
        "NMEA_Replay_SpeedD",
        "NMEA_Replay_SpeedE",
        "NMEA_Replay_SpeedF"
};

static_assert(std::size(PortConfig) == std::size(szRegistryReplaySpeed), "invalid array size");

const char* szRegistryReplayRaw[] = {
        "NMEA_Replay_RawA",
        "NMEA_Replay_RawB",
        "NMEA_Replay_RawC",
        "NMEA_Replay_RawD",
        "NMEA_Replay_RawE",
        "NMEA_Replay_RawF"
};

static_assert(std::size(PortConfig) == std::size(szRegistryReplayRaw), "invalid array size");

const char* szRegistryReplaySync[] = {
        "NMEA_Replay_SyncA",
        "NMEA_Replay_SyncB",
        "NMEA_Replay_SyncC",
        "NMEA_Replay_SyncD",
        "NMEA_Replay_SyncE",
        "NMEA_Replay_SyncF"
};

static_assert(std::size(PortConfig) == std::size(szRegistryReplaySync), "invalid array size");

const char szRegistryDisableAutoLogger[] = "DisableAutoLogger";
const char szRegistryDisplayText[] = "DisplayText2";
const char szRegistryDisplayUpValue[] = "DisplayUp";
const char szRegistryEnableFLARMMap[] = "EnableFLARMDisplay1";
const char szRegistryEnableNavBaroAltitude[] = "EnableNavBaroAltitude";
const char szRegistryFAIFinishHeight[] = "FAIFinishHeight";
const char szRegistryFAISector[] = "FAISector";
const char szRegistryFinalGlideTerrain[] = "FinalGlideTerrain";
const char szRegistryFinishLine[] = "FinishLine";
const char szRegistryFinishMinHeight[] = "FinishMinHeight";
const char szRegistryFinishRadius[] = "FinishRadius";
const char szRegistryFontRenderer[] = "FontRenderer2";
const char szRegistryFontMapWaypoint[] = "FontMapWaypointB";
const char szRegistryFontMapTopology[] = "FontMapTopologyB";
const char szRegistryFontInfopage1L[] = "FontInfopage1LB";
const char szRegistryFontInfopage2L[] = "FontInfopage2LB";
const char szRegistryFontBottomBar[] = "FontBottomBarB";
const char szRegistryFontOverlayBig[] = "FontOverlayBigB";
const char szRegistryFontOverlayMedium[] = "FontOverlayMediumB";
const char szRegistryFontVisualGlide[] = "FontVisualGlideB";
const char szRegistryFontCustom1[] = "FontCustom1B";


const char szRegistryGlideBarMode[] = "GlideBarMode";
const char szRegistryGliderScreenPosition[] = "GliderScreenPosition";
const char szRegistryGpsAltitudeOffset[] = "GpsAltitudeOffset1";
const char szRegistryHandicap[] = "Handicap1";
const char szRegistryHideUnits[] = "HideUnits";
const char szRegistryHomeWaypoint[] = "HomeWaypoint1b";
const char szRegistryDeclTakeOffLanding [] = "DeclareTakeoffLanding";
const char szRegistryInputFile[] = "InputFile";
const char szRegistryIphoneGestures[] = "IphoneGestures1";
const char szRegistryLKMaxLabels[] = "LKMaxLabels";
const char szRegistryLKTopoZoomCat05[] = "LKTopoZoomCat05d";
const char szRegistryLKTopoZoomCat100[] = "LKTopoZoomCat100a";
const char szRegistryLKTopoZoomCat10[] = "LKTopoZoomCat10d";
const char szRegistryLKTopoZoomCat110[] = "LKTopoZoomCat110a";
const char szRegistryLKTopoZoomCat20[] = "LKTopoZoomCat20a";
const char szRegistryLKTopoZoomCat30[] = "LKTopoZoomCat30a";
const char szRegistryLKTopoZoomCat40[] = "LKTopoZoomCat40a";
const char szRegistryLKTopoZoomCat50[] = "LKTopoZoomCat50a";
const char szRegistryLKTopoZoomCat60[] = "LKTopoZoomCat60a";
const char szRegistryLKTopoZoomCat70[] = "LKTopoZoomCat70a";
const char szRegistryLKTopoZoomCat80[] = "LKTopoZoomCat80a";
const char szRegistryLKTopoZoomCat90[] = "LKTopoZoomCat90a";
const char szRegistryLKVarioBar[] = "LKVarioBar";
const char szRegistryLKVarioVal[] = "LKVarioVal";
const char szRegistryLanguageCode[] = "LanguageCode";
const char szRegistryLockSettingsInFlight[] = "LockSettingsInFlight";
const char szRegistryLoggerShort[] = "LoggerShortName";
const char szRegistryMapBox[] = "MapBox";
const char szRegistryMapFile[] = "MapFile";
const char szRegistryMenuTimeout[] = "MenuTimeout";
const char szRegistryNewMapDeclutter[] = "NewMapDeclutter";
const char szRegistryOrbiter[] = "Orbiter";
const char szRegistryOutlinedTp[] = "OutlinedTp1";
const char szRegistryOverColor[] = "OverColor";
const char szRegistryOverlayClock[] = "OverlayClock";
const char szRegistryUseTwoLines[] = "UseTwoLines1";
const char szRegistrySonarWarning[] = "SonarWarning";
const char szRegistryOverlaySize[] = "OverlaySize";
const char szRegistryAutoZoomThreshold[] = "AutoZoomThreshold";
const char szRegistryClimbZoom[] = "ClimbZoom";
const char szRegistryCruiseZoom[] = "CruiseZoom";
const char szRegistryMaxAutoZoom[] = "MaxAutoZoom";
const char szRegistryTskOptimizeRoute[] = "TskOptimizeRoute";
const char szRegistryGliderSymbol[] = "GliderSymbol";
const char szRegistryPilotName[] = "PilotName1";
const char szRegistryPolarFile[] = "PolarFile1";
const char szRegistryPollingMode[] = "PollingMode";
const char szRegistryPort1Index[] = "PortIndex";
const char szRegistryPort2Index[] = "Port2Index";

const char* szRegistryDevice[] = {
        "DeviceA",
        "DeviceB",
        "DeviceC",
        "DeviceD",
        "DeviceE",
        "DeviceF"
};

static_assert(std::size(PortConfig) == std::size(szRegistryDevice), "invalid array size");

const char* szRegistryPortName[] = {
        "Port1Name",
        "Port2Name",
        "Port3Name",
        "Port4Name",
        "Port5Name",
        "Port6Name"
};

static_assert(std::size(PortConfig) == std::size(szRegistryPortName), "invalid array size");

const char* szRegistrySpeedIndex[] = {
        "SpeedIndex",
        "Speed2Index",
        "Speed3Index",
        "Speed4Index",
        "Speed5Index",
        "Speed6Index"
};

static_assert(std::size(PortConfig) == std::size(szRegistrySpeedIndex), "invalid array size");

const char* szRegistryBitIndex[] = {
        "Bit1Index",
        "Bit2Index",
        "Bit3Index",
        "Bit4Index",
        "Bit5Index",
        "Bit6Index"
};

static_assert(std::size(PortConfig) == std::size(szRegistryBitIndex), "invalid array size");

const char* szRegistryIpAddress[] = {
        "IpAddress1",
        "IpAddress2",
        "IpAddress3",
        "IpAddress4",
        "IpAddress5",
        "IpAddress6"
};

static_assert(std::size(PortConfig) == std::size(szRegistryIpAddress), "invalid array size");

const char* szRegistryIpPort[] = {
        "IpPort1",
        "IpPort2",
        "IpPort3",
        "IpPort4",
        "IpPort5",
        "IpPort6"
};

static_assert(std::size(PortConfig) == std::size(szRegistryIpPort), "invalid array size");

const char szRegistryIOValues[] = "Val_IO_Dir";
const char szRegistrySafetyAltitudeArrival[] = "SafetyAltitudeArrival1";
const char szRegistrySafetyAltitudeMode[] = "SafetyAltitudeMode";
const char szRegistrySafetyAltitudeTerrain[] = "SafetyAltitudeTerrain1";
const char szRegistrySafetyMacCready[] = "SafetyMacCready";
const char szRegistrySafteySpeed[] = "SafteySpeed1";
const char szRegistrySectorRadius[] = "Radius";
const char szRegistrySetSystemTimeFromGPS[] = "SetSystemTimeFromGPS";
const char szRegistrySaveRuntime[] = "SaveRuntime";
const char szRegistryShading[] = "Shading";
const char szRegistryIsoLine[] = "IsoLine";
const char szRegistrySnailTrail[] = "SnailTrail";
const char szRegistrySnailScale[] = "SnailScale";
const char szRegistryStartHeightRef[] = "StartHeightRef";
const char szRegistryStartLine[] = "StartLine";
const char szRegistryStartMaxHeightMargin[] = "StartMaxHeightMargin";
const char szRegistryStartMaxHeight[] = "StartMaxHeight";
const char szRegistryStartMaxSpeedMargin[] = "StartMaxSpeedMargin";
const char szRegistryStartMaxSpeed[] = "StartMaxSpeed";
const char szRegistryStartRadius[] = "StartRadius";
const char szRegistryTeamcodeRefWaypoint[] = "TeamcodeRefWaypoint1";
const char szRegistryTerrainBrightness[] = "TerrainBrightness1";
const char szRegistryTerrainContrast[] = "TerrainContrast1";
const char szRegistryTerrainFile[] = "TerrainFile";
const char szRegistryTerrainRamp[] = "TerrainRamp";
const char szRegistryTerrainWhiteness[] = "TerrainWhiteness";
const char szRegistryThermalBar[] = "ThermalBar";
const char szRegistryThermalLocator[] = "ThermalLocator";
const char szRegistryTpFilter[] = "TpFilter";
const char szRegistryTrackBar[] = "TrackBar";
const char szRegistryTrailDrift[] = "TrailDrift";
const char szRegistryUTCOffset[] = "UTCOffset";
const char szRegistryUseGeoidSeparation[] = "UseGeoidSeparation";

const char* szRegistryUseExtSound[] = {
        "UseExtSound1",
        "UseExtSound2",
        "UseExtSound3",
        "UseExtSound4",
        "UseExtSound5",
        "UseExtSound6"
};

static_assert(std::size(PortConfig) == std::size(szRegistryUseExtSound),
              "invalid array size");

const char szRegistryUseUngestures[] = "UseUngestures";
const char szRegistryUseTotalEnergy[] = "UseTotalEnergy";
const char szRegistryWarningTime[] = "WarnTime";

const char *const szRegistryWayPointFile[] = {
        "WPFile",
        "AdditionalWPFile",
        "WPFile3",
        "WPFile4",
        "WPFile5",
        "WPFile6",
        "WPFile7",
        "WPFile8",
        "WPFile9"
};
static_assert(std::size(szWaypointFile) == std::size(szRegistryWayPointFile),
              "invalid array size");

const char szRegistryWaypointsOutOfRange[] = "WaypointsOutOfRange2";
const char szRegistryWindCalcSpeed[] = "WindCalcSpeed";
const char szRegistryWindCalcTime[] = "WindCalcTime";
const char szRegistryCustomMenu1[] = "CustomMenu1a";
const char szRegistryCustomMenu2[] = "CustomMenu2a";
const char szRegistryCustomMenu3[] = "CustomMenu3a";
const char szRegistryCustomMenu4[] = "CustomMenu4a";
const char szRegistryCustomMenu5[] = "CustomMenu5a";
const char szRegistryCustomMenu6[] = "CustomMenu6a";
const char szRegistryCustomMenu7[] = "CustomMenu7a";
const char szRegistryCustomMenu8[] = "CustomMenu8a";
const char szRegistryCustomMenu9[] = "CustomMenu9a";
const char szRegistryCustomMenu10[] = "CustomMenu10a";
const char szRegistryUseWindRose[] = "UseWindRose";
const char szRegistryMultiTerr0[] = "MultimapTerrain0";
const char szRegistryMultiTerr1[] = "MultimapTerrain1";
const char szRegistryMultiTerr2[] = "MultimapTerrain2";
const char szRegistryMultiTerr3[] = "MultimapTerrain3";
const char szRegistryMultiTerr4[] = "MultimapTerrain4";
const char szRegistryMultiTopo0[] = "MultimapTopology0";
const char szRegistryMultiTopo1[] = "MultimapTopology1";
const char szRegistryMultiTopo2[] = "MultimapTopology2";
const char szRegistryMultiTopo3[] = "MultimapTopology3";
const char szRegistryMultiTopo4[] = "MultimapTopology4";
const char szRegistryMultiAsp0[] = "MultimapAirspace0";
const char szRegistryMultiAsp1[] = "MultimapAirspace1";
const char szRegistryMultiAsp2[] = "MultimapAirspace2";
const char szRegistryMultiAsp3[] = "MultimapAirspace3";
const char szRegistryMultiAsp4[] = "MultimapAirspace4";
const char szRegistryMultiLab0[] = "MultimapLab0";
const char szRegistryMultiLab1[] = "MultimapLab1";
const char szRegistryMultiLab2[] = "MultimapLab2";
const char szRegistryMultiLab3[] = "MultimapLab3";
const char szRegistryMultiLab4[] = "MultimapLab4";
const char szRegistryMultiWpt0[] = "MultimapWpt0";
const char szRegistryMultiWpt1[] = "MultimapWpt1";
const char szRegistryMultiWpt2[] = "MultimapWpt2";
const char szRegistryMultiWpt3[] = "MultimapWpt3";
const char szRegistryMultiWpt4[] = "MultimapWpt4";
const char szRegistryMultiOvrT0[] = "MultimapOvrT0";
const char szRegistryMultiOvrT1[] = "MultimapOvrT1";
const char szRegistryMultiOvrT2[] = "MultimapOvrT2";
const char szRegistryMultiOvrT3[] = "MultimapOvrT3";
const char szRegistryMultiOvrT4[] = "MultimapOvrT4";
const char szRegistryMultiOvrG0[] = "MultimapOvrG0";
const char szRegistryMultiOvrG1[] = "MultimapOvrG1";
const char szRegistryMultiOvrG2[] = "MultimapOvrG2";
const char szRegistryMultiOvrG3[] = "MultimapOvrG3";
const char szRegistryMultiOvrG4[] = "MultimapOvrG4";
const char szRegistryMultiSizeY1[] = "MultimapSizeY1";
const char szRegistryMultiSizeY2[] = "MultimapSizeY2";
const char szRegistryMultiSizeY3[] = "MultimapSizeY3";
const char szRegistryMultiSizeY4[] = "MultimapSizeY4";

const char szRegistryMultimap1[] = "Multimap1a";
const char szRegistryMultimap2[] = "Multimap2a";
const char szRegistryMultimap3[] = "Multimap3a";
const char szRegistryMultimap4[] = "Multimap4a";
const char szRegistryMultimap5[] = "Multimap5a";

const char szRegistryMMNorthUp1[] = "MultimapNorthUp1";
const char szRegistryMMNorthUp2[] = "MultimapNorthUp2";
const char szRegistryMMNorthUp3[] = "MultimapNorthUp3";
const char szRegistryMMNorthUp4[] = "MultimapNorthUp4";

const char szRegistryAspPermanent[] = "AirspacePermMod";
const char szRegistryFlarmDirection[] = "FlarmDirection";
//const char szRegistryDrawTask[] = "DrawTask";
const char szRegistryDrawFAI[] = "DrawFAI";
const char szRegistryGearMode[] = "GearMode";
const char szRegistryGearAltitude[] = "GearAltitude";
const char szRegistryBottomMode[] = "ActiveBottomBar";
const char szRegistryBigFAIThreshold[] = "FAI_28_45_Threshold";
const char szRegistryDrawXC[] = "DrawXC";

const char szRegistryScreenSize[] = "ScreenSize";
const char szRegistryScreenSizeX[] = "ScreenSizeX";
const char szRegistryScreenSizeY[] = "ScreenSizeY";

const char szRegistryOverlay_TopLeft[] = "Overlay_TopLeft";
const char szRegistryOverlay_TopMid[] = "Overlay_TopMid";
const char szRegistryOverlay_TopRight[] = "Overlay_TopRight";
const char szRegistryOverlay_TopDown[] = "Overlay_TopDown";
const char szRegistryOverlay_LeftTop[] = "Overlay_LeftTop";
const char szRegistryOverlay_LeftMid[] = "Overlay_LeftMid";
const char szRegistryOverlay_LeftBottom[] = "Overlay_LeftBottom";
const char szRegistryOverlay_LeftDown[] = "Overlay_LeftDown";
const char szRegistryOverlay_RightTop[] = "Overlay_RightTop";
const char szRegistryOverlay_RightMid[] = "Overlay_RightMid";
const char szRegistryOverlay_RightBottom[] = "Overlay_RightBottom";
const char szRegistryOverlay_Title[] = "Overlay_Title";
const char szRegistryAdditionalContestRule[] = "Additional_Contest_Rule";
#ifdef _WGS84
const char szRegistry_earth_model_wgs84[] = "earth_model_wgs84";
#endif
const char szRegistrySoundSwitch[] = "SoundSwitch";
const char szRegistryEnableAudioVario[] = "EnableAudioVario";
