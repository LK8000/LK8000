/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef _LKPROFILES_H
#define _LKPROFILES_H

#include "tchar.h"

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

extern const char * const szRegistryAirspaceMode[];	// 18
extern const char * const szRegistryBrush[];	// 18
extern const char * const szRegistryColour[];	// 18
extern const char * const szRegistryDisplayType[];	// MAXINFOWINDOWS
extern const char szRegistryAcknowledgementTime[];
extern const char szRegistryAdditionalAirspaceFile[];
extern const char szRegistryAdditionalWayPointFile[];
extern const char szRegistryAircraftCategory[];
extern const char szRegistryAircraftRego[];
extern const char szRegistryAircraftType[];
extern const char szRegistryAirfieldFile[];

extern const char * const szRegistryAirspaceFile[];

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
extern const char szRegistryDeclTakeOffLanding[];
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
extern const char szRegistryLanguageCode[];
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
extern const char * const szRegistryWayPointFile[];
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

extern const char szRegistrySoundSwitch[];

void LKProfileResetDefault();
void LKProfileInitRuntime();
bool LKProfileLoad(const TCHAR *szFile);
void LKProfileSave(const TCHAR *szFile);

void LKAircraftSave(const TCHAR *szFile);
void LKPilotSave(const TCHAR *szFile);
void LKDeviceSave(const TCHAR *szFile);
#endif // _LKPROFILES_H
