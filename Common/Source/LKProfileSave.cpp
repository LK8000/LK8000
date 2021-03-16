/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */

#include "externs.h"
#include "McReady.h"
#include "Modeltype.h"
#include "utils/stringext.h"
#include "LKProfiles.h"
#include "Asset.hpp"
#include "Settings/write.h"

extern bool CommandResolution;

void LKProfileSave(const TCHAR *szFile) {
#if TESTBENCH
  StartupStore(_T("... SaveProfile <%s>%s"), szFile, NEWLINE);
#endif

  if (!szFile || !szFile[0]) {
    // nullptr or empty string.
    return;
  }
  settings::writer write_settings(szFile, "PROFILE");
  if (!write_settings) {
    StartupStore(_T("...... SaveProfile <%s> open for write FAILED!"), szFile);
    return;
  }


  //
  // RESPECT LKPROFILE.H ALPHA ORDER OR WE SHALL GET LOST SOON!
  //
  // -- USE _CONFIG VARIABLES WHEN A RUNTIME VALUE CAN BE CHANGED --
  // WE DONT WANT TO SAVE RUNTIME TEMPORARY CONFIGURATIONS, ONLY SYSTEM CONFIG!
  // FOR EXAMPLE: ActiveMap can be set by default in system config, but also changed
  // at runtime with a button and with a customkey. We must save in profile ONLY
  // the _Config, not the temporary setup!
  //



  write_settings(szRegistryAcknowledgementTime, AcknowledgementTime);

  write_settings(szRegistryAirfieldFile, szAirfieldFile);
  for (unsigned int i = 0; i < NO_AS_FILES; i++) {
    write_settings(szRegistryAirspaceFile[i], szAirspaceFile[i]);
  }
  write_settings(szRegistryAirspaceFillType, MapWindow::GetAirSpaceFillType());
  write_settings(szRegistryAirspaceOpacity, MapWindow::GetAirSpaceOpacity());
  write_settings(szRegistryAirspaceWarningDlgTimeout, AirspaceWarningDlgTimeout);
  write_settings(szRegistryAirspaceWarningMapLabels, AirspaceWarningMapLabels);
  write_settings(szRegistryAirspaceAckAllSame, AirspaceAckAllSame);
  write_settings(szRegistryAirspaceWarningRepeatTime, AirspaceWarningRepeatTime);
  write_settings(szRegistryAirspaceWarningVerticalMargin, AirspaceWarningVerticalMargin);
  write_settings(szRegistryAirspaceWarning, AIRSPACEWARNINGS);
  write_settings(szRegistryAlarmMaxAltitude1, AlarmMaxAltitude1); // saved *1000, /1000 when used
  write_settings(szRegistryAlarmMaxAltitude2, AlarmMaxAltitude2);
  write_settings(szRegistryAlarmMaxAltitude3, AlarmMaxAltitude3);
  write_settings(szRegistryAlarmTakeoffSafety, AlarmTakeoffSafety);
  write_settings(szRegistryAltMargin, AltWarningMargin);
  write_settings(szRegistryAltMode, AltitudeMode_Config);

  // these are not part of configuration, but saved all the same
  write_settings(szRegistryAlternate1, Alternate1);
  write_settings(szRegistryAlternate2, Alternate2);

  write_settings(szRegistryAltitudeUnitsValue, AltitudeUnit_Config);
  write_settings(szRegistryAppIndLandable, Appearance.IndLandable);
  write_settings(szRegistryUTF8Symbolsl, Appearance.UTF8Pictorials);

  write_settings(szRegistryAppInverseInfoBox, InverseInfoBox_Config);
  write_settings(szRegistryArrivalValue, ArrivalValue);
  write_settings(szRegistryAutoAdvance, AutoAdvance_Config);
  write_settings(szRegistryAutoBacklight, EnableAutoBacklight);
  write_settings(szRegistryAutoForceFinalGlide, AutoForceFinalGlide);
  write_settings(szRegistryAutoMcMode, AutoMcMode_Config);

  write_settings(szRegistryAutoMcStatus, AutoMacCready_Config);

  write_settings(szRegistryAutoOrientScale, AutoOrientScale * 10);
  write_settings(szRegistryAutoSoundVolume, EnableAutoSoundVolume);
  write_settings(szRegistryAutoWind, AutoWindMode_Config);
  write_settings(szRegistryAutoZoom, AutoZoom_Config);
  write_settings(szRegistryAverEffTime, AverEffTime);

  write_settings(szRegistryBarOpacity, BarOpacity);
  write_settings(szRegistryBestWarning, BestWarning);
  write_settings(szRegistryBgMapColor, BgMapColor_Config);

  write_settings(szRegistryBugs, BUGS_Config * 100);

  write_settings(szRegistryCircleZoom, MapWindow::zoom.CircleZoom());
  write_settings(szRegistryClipAlt, ClipAltitude);

  write_settings(szRegistryConfBB0, ConfBB0);
  write_settings(szRegistryConfBB1, ConfBB1);
  write_settings(szRegistryConfBB2, ConfBB2);
  write_settings(szRegistryConfBB3, ConfBB3);
  write_settings(szRegistryConfBB4, ConfBB4);
  write_settings(szRegistryConfBB5, ConfBB5);
  write_settings(szRegistryConfBB6, ConfBB6);
  write_settings(szRegistryConfBB7, ConfBB7);
  write_settings(szRegistryConfBB8, ConfBB8);
  write_settings(szRegistryConfBB9, ConfBB9);
  write_settings(szRegistryConfBB0Auto, ConfBB0Auto);
  write_settings(szRegistryConfIP11, ConfIP11);
  write_settings(szRegistryConfIP12, ConfIP12);
  write_settings(szRegistryConfIP13, ConfIP13);
  write_settings(szRegistryConfIP14, ConfIP14);
  write_settings(szRegistryConfIP15, ConfIP15);
  write_settings(szRegistryConfIP16, ConfIP16);
  write_settings(szRegistryConfIP17, ConfIP17);
  write_settings(szRegistryConfIP21, ConfIP21);
  write_settings(szRegistryConfIP22, ConfIP22);
  write_settings(szRegistryConfIP23, ConfIP23);
  write_settings(szRegistryConfIP24, ConfIP24);
  write_settings(szRegistryConfIP31, ConfIP31);
  write_settings(szRegistryConfIP32, ConfIP32);
  write_settings(szRegistryConfIP33, ConfIP33);
  write_settings(szRegistryCustomKeyModeAircraftIcon, CustomKeyModeAircraftIcon);
  write_settings(szRegistryCustomKeyModeCenterScreen, CustomKeyModeCenterScreen);
  write_settings(szRegistryCustomKeyModeCenter, CustomKeyModeCenter);
  write_settings(szRegistryCustomKeyModeLeftUpCorner, CustomKeyModeLeftUpCorner);
  write_settings(szRegistryCustomKeyModeLeft, CustomKeyModeLeft);
  write_settings(szRegistryCustomKeyModeRightUpCorner, CustomKeyModeRightUpCorner);
  write_settings(szRegistryCustomKeyModeRight, CustomKeyModeRight);
  write_settings(szRegistryCustomKeyTime, CustomKeyTime);
  write_settings(szRegistryCustomMenu1, CustomMenu1);
  write_settings(szRegistryCustomMenu2, CustomMenu2);
  write_settings(szRegistryCustomMenu3, CustomMenu3);
  write_settings(szRegistryCustomMenu4, CustomMenu4);
  write_settings(szRegistryCustomMenu5, CustomMenu5);
  write_settings(szRegistryCustomMenu6, CustomMenu6);
  write_settings(szRegistryCustomMenu7, CustomMenu7);
  write_settings(szRegistryCustomMenu8, CustomMenu8);
  write_settings(szRegistryCustomMenu9, CustomMenu9);
  write_settings(szRegistryCustomMenu10, CustomMenu10);
  write_settings(szRegistryDebounceTimeout, debounceTimeout);
  write_settings(szRegistryDeclutterMode, DeclutterMode);

  write_settings(szRegistryDisableAutoLogger, DisableAutoLogger);
  write_settings(szRegistryDisplayText, DisplayTextType);
  if (SaveRuntime) {
    write_settings(szRegistryDisplayUpValue, DisplayOrientation);
  } else {
    write_settings(szRegistryDisplayUpValue, DisplayOrientation_Config);
  }
  write_settings(szRegistryDistanceUnitsValue, DistanceUnit_Config);
  write_settings(szRegistryEnableFLARMMap, EnableFLARMMap);
  write_settings(szRegistryEnableNavBaroAltitude, EnableNavBaroAltitude_Config);
  write_settings(szRegistryFAIFinishHeight, EnableFAIFinishHeight);
  write_settings(szRegistryFAISector, SectorType);
  write_settings(szRegistryFinalGlideTerrain, FinalGlideTerrain);
  write_settings(szRegistryFinishLine, FinishLine);
  write_settings(szRegistryFinishMinHeight, FinishMinHeight); // saved *1000, /1000 when used
  write_settings(szRegistryFinishRadius, FinishRadius);
  write_settings(szRegistryFontRenderer, FontRenderer);

  write_settings(szRegistryFontMapWaypoint, FontMapWaypoint);
  write_settings(szRegistryFontMapTopology, FontMapTopology);
  write_settings(szRegistryFontInfopage1L, FontInfopage1L);
  write_settings(szRegistryFontInfopage2L, FontInfopage2L);
  write_settings(szRegistryFontBottomBar, FontBottomBar);
  write_settings(szRegistryFontOverlayBig, FontOverlayBig);
  write_settings(szRegistryFontOverlayMedium, FontOverlayMedium);
  write_settings(szRegistryFontCustom1, FontCustom1);
  write_settings(szRegistryFontVisualGlide, FontVisualGlide);

  write_settings(szRegistryGlideBarMode, GlideBarMode);
  write_settings(szRegistryGliderScreenPosition, MapWindow::GliderScreenPosition);
  write_settings(szRegistryGpsAltitudeOffset, GPSAltitudeOffset);

  write_settings(szRegistryHideUnits, HideUnits);
  write_settings(szRegistryHomeWaypoint, HomeWaypoint);
  write_settings(szRegistryDeclTakeOffLanding, DeclTakeoffLanding);

  // InfoType for infoboxes configuration
  for (int i = 0; i < MAXINFOWINDOWS; i++) {
    write_settings(szRegistryDisplayType[i], InfoType[i]);
  }

  write_settings(szRegistryInputFile, szInputFile);
  write_settings(szRegistryIphoneGestures, IphoneGestures);
  write_settings(szRegistryLKMaxLabels, LKMaxLabels);
  write_settings(szRegistryLKTopoZoomCat05, LKTopoZoomCat05 * 1000);
  write_settings(szRegistryLKTopoZoomCat100, LKTopoZoomCat100 * 1000);
  write_settings(szRegistryLKTopoZoomCat10, LKTopoZoomCat10 * 1000);
  write_settings(szRegistryLKTopoZoomCat110, LKTopoZoomCat110 * 1000);
  write_settings(szRegistryLKTopoZoomCat20, LKTopoZoomCat20 * 1000);
  write_settings(szRegistryLKTopoZoomCat30, LKTopoZoomCat30 * 1000);
  write_settings(szRegistryLKTopoZoomCat40, LKTopoZoomCat40 * 1000);
  write_settings(szRegistryLKTopoZoomCat50, LKTopoZoomCat50 * 1000);
  write_settings(szRegistryLKTopoZoomCat60, LKTopoZoomCat60 * 1000);
  write_settings(szRegistryLKTopoZoomCat70, LKTopoZoomCat70 * 1000);
  write_settings(szRegistryLKTopoZoomCat80, LKTopoZoomCat80 * 1000);
  write_settings(szRegistryLKTopoZoomCat90, LKTopoZoomCat90 * 1000);
  write_settings(szRegistryLKVarioBar, LKVarioBar);
  write_settings(szRegistryLKVarioVal, LKVarioVal);
  write_settings(szRegistryLanguageCode, szLanguageCode);
  write_settings(szRegistryLatLonUnits, Units::CoordinateFormat);
  write_settings(szRegistryLiftUnitsValue, LiftUnit_Config);
  write_settings(szRegistryLockSettingsInFlight, LockSettingsInFlight);
  write_settings(szRegistryLoggerShort, LoggerShortName);
  write_settings(szRegistryMapBox, MapBox);
  write_settings(szRegistryMapFile, szMapFile);
  write_settings(szRegistryMenuTimeout, MenuTimeout_Config);
  write_settings(szRegistryNewMapDeclutter, NewMapDeclutter);
  write_settings(szRegistryOrbiter, Orbiter_Config);
  write_settings(szRegistryOutlinedTp, OutlinedTp_Config);
  write_settings(szRegistryOverColor, OverColor);
  write_settings(szRegistryOverlayClock, OverlayClock);
  write_settings(szRegistryUseTwoLines, UseTwoLines);
  write_settings(szRegistryOverlaySize, OverlaySize);
  write_settings(szRegistryAutoZoomThreshold, AutoZoomThreshold);
  write_settings(szRegistryClimbZoom, ClimbZoom);
  write_settings(szRegistryCruiseZoom, CruiseZoom);
  write_settings(szRegistryMaxAutoZoom, MaxAutoZoom);
  write_settings(szRegistryTskOptimizeRoute, TskOptimizeRoute_Config);
  write_settings(szRegistryGliderSymbol, GliderSymbol);

  write_settings(szRegistryPressureHg, PressureHg);
  write_settings(szRegistrySafetyAltitudeArrival, SAFETYALTITUDEARRIVAL);
  write_settings(szRegistrySafetyAltitudeMode, SafetyAltitudeMode);
  write_settings(szRegistrySafetyAltitudeTerrain, SAFETYALTITUDETERRAIN);
  write_settings(szRegistrySafetyMacCready, GlidePolar::SafetyMacCready * 10);

  write_settings(szRegistrySectorRadius, SectorRadius);

#if defined(PPC2003) || defined(PNA)
  write_settings(szRegistrySetSystemTimeFromGPS, SetSystemTimeFromGPS);
#endif

  write_settings(szRegistrySaveRuntime, SaveRuntime);
  write_settings(szRegistryShading, Shading_Config);
  write_settings(szRegistryIsoLine, IsoLine_Config);
  write_settings(szRegistrySnailTrail, TrailActive_Config);
  write_settings(szRegistrySnailScale, SnailScale);

  write_settings(szRegistrySpeedUnitsValue, SpeedUnit_Config);
  write_settings(szRegistryStartHeightRef, StartHeightRef);
  write_settings(szRegistryStartLine, StartLine);
  write_settings(szRegistryStartMaxHeightMargin,
             StartMaxHeightMargin); // saved *1000, /1000 when used
  write_settings(szRegistryStartMaxHeight, StartMaxHeight); // saved *1000, /1000 when used
  write_settings(szRegistryStartMaxSpeedMargin,
             StartMaxSpeedMargin); // saved *1000, /1000 when used
  write_settings(szRegistryStartMaxSpeed, StartMaxSpeed); // saved *1000, /1000 when used
  write_settings(szRegistryStartRadius, StartRadius);
  write_settings(szRegistryTaskSpeedUnitsValue, TaskSpeedUnit_Config);
  write_settings(szRegistryTeamcodeRefWaypoint, TeamCodeRefWaypoint);
  write_settings(szRegistryTerrainBrightness, TerrainBrightness);
  write_settings(szRegistryTerrainContrast, TerrainContrast);
  write_settings(szRegistryTerrainFile, szTerrainFile);
  write_settings(szRegistryTerrainRamp, TerrainRamp_Config);

  if (SaveRuntime) {
    write_settings(szRegistryTerrainWhiteness, TerrainWhiteness * 100);
  }

  write_settings(szRegistryThermalBar, ThermalBar);
  write_settings(szRegistryThermalLocator, EnableThermalLocator);
  write_settings(szRegistryTpFilter, TpFilter);
  write_settings(szRegistryTrackBar, TrackBar);
  write_settings(szRegistryTrailDrift, EnableTrailDrift_Config);
  write_settings(szRegistryUTCOffset, UTCOffset);

  write_settings(szRegistryUseUngestures, UseUngestures);
  write_settings(szRegistryUseTotalEnergy, UseTotalEnergy_Config);
  write_settings(szRegistryWarningTime, WarningTime);

  for (unsigned int i = 0; i < NO_WP_FILES; i++) {
    write_settings(szRegistryWayPointFile[i], szWaypointFile[i]);
  }

  write_settings(szRegistryWaypointsOutOfRange, WaypointsOutOfRange);
  write_settings(szRegistryWindCalcSpeed, WindCalcSpeed * 1000); // m/s x1000
  write_settings(szRegistryWindCalcTime, WindCalcTime);

  for (int i = 0; i < AIRSPACECLASSCOUNT; i++) {
    write_settings(szRegistryAirspaceMode[i], MapWindow::iAirspaceMode[i]);
    write_settings(szRegistryColour[i], MapWindow::iAirspaceColour[i]);
#ifdef HAVE_HATCHED_BRUSH
    write_settings(szRegistryBrush[i],MapWindow::iAirspaceBrush[i]);
#endif
  }

  write_settings(szRegistryUseWindRose, UseWindRose);

  //
  // Multimaps added 121003
  //

  if (SaveRuntime) {
    write_settings(szRegistryMultiTerr0, Multimap_Flags_Terrain[MP_MOVING]);
    write_settings(szRegistryMultiTerr1, Multimap_Flags_Terrain[MP_MAPTRK]);
    write_settings(szRegistryMultiTerr2, Multimap_Flags_Terrain[MP_MAPWPT]);
    write_settings(szRegistryMultiTerr3, Multimap_Flags_Terrain[MP_MAPASP]);
    write_settings(szRegistryMultiTerr4, Multimap_Flags_Terrain[MP_VISUALGLIDE]);

    write_settings(szRegistryMultiTopo0, Multimap_Flags_Topology[MP_MOVING]);
    write_settings(szRegistryMultiTopo1, Multimap_Flags_Topology[MP_MAPTRK]);
    write_settings(szRegistryMultiTopo2, Multimap_Flags_Topology[MP_MAPWPT]);
    write_settings(szRegistryMultiTopo3, Multimap_Flags_Topology[MP_MAPASP]);
    write_settings(szRegistryMultiTopo4, Multimap_Flags_Topology[MP_VISUALGLIDE]);

    write_settings(szRegistryMultiAsp0, Multimap_Flags_Airspace[MP_MOVING]);
    write_settings(szRegistryMultiAsp1, Multimap_Flags_Airspace[MP_MAPTRK]);
    write_settings(szRegistryMultiAsp2, Multimap_Flags_Airspace[MP_MAPWPT]);
    write_settings(szRegistryMultiAsp3, Multimap_Flags_Airspace[MP_MAPASP]);
    write_settings(szRegistryMultiAsp4, Multimap_Flags_Airspace[MP_VISUALGLIDE]);

    write_settings(szRegistryMultiLab0, Multimap_Labels[MP_MOVING]);
    write_settings(szRegistryMultiLab1, Multimap_Labels[MP_MAPTRK]);
    write_settings(szRegistryMultiLab2, Multimap_Labels[MP_MAPWPT]);
    write_settings(szRegistryMultiLab3, Multimap_Labels[MP_MAPASP]);
    write_settings(szRegistryMultiLab4, Multimap_Labels[MP_VISUALGLIDE]);

    write_settings(szRegistryMultiWpt0, Multimap_Flags_Waypoints[MP_MOVING]);
    write_settings(szRegistryMultiWpt1, Multimap_Flags_Waypoints[MP_MAPTRK]);
    write_settings(szRegistryMultiWpt2, Multimap_Flags_Waypoints[MP_MAPWPT]);
    write_settings(szRegistryMultiWpt3, Multimap_Flags_Waypoints[MP_MAPASP]);
    write_settings(szRegistryMultiWpt4, Multimap_Flags_Waypoints[MP_VISUALGLIDE]);

    write_settings(szRegistryMultiOvrT0, Multimap_Flags_Overlays_Text[MP_MOVING]);
    write_settings(szRegistryMultiOvrT1, Multimap_Flags_Overlays_Text[MP_MAPTRK]);
    write_settings(szRegistryMultiOvrT2, Multimap_Flags_Overlays_Text[MP_MAPWPT]);
    write_settings(szRegistryMultiOvrT3, Multimap_Flags_Overlays_Text[MP_MAPASP]);
    write_settings(szRegistryMultiOvrT4, Multimap_Flags_Overlays_Text[MP_VISUALGLIDE]);

    write_settings(szRegistryMultiOvrG0, Multimap_Flags_Overlays_Gauges[MP_MOVING]);
    write_settings(szRegistryMultiOvrG1, Multimap_Flags_Overlays_Gauges[MP_MAPTRK]);
    write_settings(szRegistryMultiOvrG2, Multimap_Flags_Overlays_Gauges[MP_MAPWPT]);
    write_settings(szRegistryMultiOvrG3, Multimap_Flags_Overlays_Gauges[MP_MAPASP]);
    write_settings(szRegistryMultiOvrG4, Multimap_Flags_Overlays_Gauges[MP_VISUALGLIDE]);

    write_settings(szRegistryMultiSizeY1, Multimap_SizeY[MP_MAPTRK]);
    write_settings(szRegistryMultiSizeY2, Multimap_SizeY[MP_MAPWPT]);
    write_settings(szRegistryMultiSizeY3, Multimap_SizeY[MP_MAPASP]);
    write_settings(szRegistryMultiSizeY4, Multimap_SizeY[MP_VISUALGLIDE]);
  }

  write_settings(szRegistryMultimap1, Multimap1);
  write_settings(szRegistryMultimap2, Multimap2);
  write_settings(szRegistryMultimap3, Multimap3);
  write_settings(szRegistryMultimap4, Multimap4);
  write_settings(szRegistryMultimap5, Multimap5);

  if (SaveRuntime) {
    write_settings(szRegistryMMNorthUp1, MMNorthUp_Runtime[0]);
    write_settings(szRegistryMMNorthUp2, MMNorthUp_Runtime[1]);
    write_settings(szRegistryMMNorthUp3, MMNorthUp_Runtime[2]);
    write_settings(szRegistryMMNorthUp4, MMNorthUp_Runtime[3]);
  }

  write_settings(szRegistryAspPermanent, AspPermanentChanged);
  write_settings(szRegistryFlarmDirection, iFlarmDirection);

  write_settings(szRegistryDrawFAI, Flags_DrawFAI_config);
  write_settings(szRegistryDrawXC, Flags_DrawXC_config);

  write_settings(szRegistryGearMode, GearWarningMode);
  write_settings(szRegistryGearAltitude, GearWarningAltitude);
  write_settings(szRegistryBigFAIThreshold, FAI28_45Threshold);

  if (SaveRuntime) {
    write_settings(szRegistryBottomMode, BottomMode);
  }

  write_settings(szRegistrySonarWarning, SonarWarning_Config);

  write_settings(szRegistryOverlay_TopLeft, Overlay_TopLeft);
  write_settings(szRegistryOverlay_TopMid, Overlay_TopMid);
  write_settings(szRegistryOverlay_TopRight, Overlay_TopRight);
  write_settings(szRegistryOverlay_TopDown, Overlay_TopDown);
  write_settings(szRegistryOverlay_LeftTop, Overlay_LeftTop);
  write_settings(szRegistryOverlay_LeftMid, Overlay_LeftMid);
  write_settings(szRegistryOverlay_LeftBottom, Overlay_LeftBottom);
  write_settings(szRegistryOverlay_LeftDown, Overlay_LeftDown);
  write_settings(szRegistryOverlay_RightTop, Overlay_RightTop);
  write_settings(szRegistryOverlay_RightMid, Overlay_RightMid);
  write_settings(szRegistryOverlay_RightBottom, Overlay_RightBottom);
  write_settings(szRegistryOverlay_Title, Overlay_Title);

  write_settings(szRegistryAdditionalContestRule, AdditionalContestRule);
  write_settings(szRegistryEnableAudioVario, EnableAudioVario);

#ifdef _WGS84
  write_settings(szRegistry_earth_model_wgs84, earth_model_wgs84);
#endif

  write_settings(szRegistryAutoContrast, AutoContrast);

  if (SaveRuntime && !IsEmbedded() && !CommandResolution) {
    write_settings(szRegistryScreenSize, ScreenSize);
    write_settings(szRegistryScreenSizeX, ScreenSizeX);
    write_settings(szRegistryScreenSizeY, ScreenSizeY);
  }
  if (SaveRuntime) {
    write_settings(szRegistrySoundSwitch, EnableSoundModes);
  }
}

void WriteDeviceSettings(const int devIdx, const TCHAR *Name) {
  if (devIdx >= 0 && devIdx < NUMDEV) {
    _tcscpy(dwDeviceName[devIdx], Name);
  }
}

//
// Save only Aircraft related parameters
//
void LKAircraftSave(const TCHAR *szFile) {
#if TESTBENCH
  StartupStore(_T("... AircraftSave <%s>%s"), szFile, NEWLINE);
#endif

  if (!szFile || !szFile[0]) {
    // nullptr or empty string.
    return;
  }
  settings::writer write_settings(szFile, "AIRCRAFT PROFILE");
  if (!write_settings) {
    StartupStore(_T("...... AircraftSaveProfile <%s> open for write FAILED!%s"), szFile, NEWLINE);
    return;
  }

  write_settings(szRegistryAircraftCategory, AircraftCategory);
  write_settings(szRegistryPolarFile, szPolarFile);
  write_settings(szRegistrySafteySpeed, SAFTEYSPEED * 1000); // Max speed V rough air m/s x1000
  write_settings(szRegistryHandicap, Handicap);
  write_settings(szRegistryBallastSecsToEmpty, BallastSecsToEmpty);

  write_settings(szRegistryAircraftType, AircraftType_Config);
  write_settings(szRegistryAircraftRego, AircraftRego_Config);
  write_settings(szRegistryCompetitionClass, CompetitionClass_Config);
  write_settings(szRegistryCompetitionID, CompetitionID_Config);
}


//
// Save only Pilot related parameters
//
void LKPilotSave(const TCHAR *szFile) {
#if TESTBENCH
  StartupStore(_T("... PilotSave <%s>%s"), szFile, NEWLINE);
#endif

  if (!szFile || !szFile[0]) {
    // nullptr or empty string.
    return;
  }
  settings::writer write_settings(szFile, "PILOT PROFILE");
  if (!write_settings) {
    StartupStore(_T("...... PilotSaveProfile <%s> open for write FAILED!"), szFile);
    return;
  }

  write_settings(szRegistryPilotName, PilotName_Config);
  write_settings(szRegistryLiveTrackerInterval, LiveTrackerInterval);
  write_settings(szRegistryLiveTrackerRadar_config, LiveTrackerRadar_config);
  write_settings(szRegistryLiveTrackerStart_config, LiveTrackerStart_config);
  write_settings(szRegistryLiveTrackersrv, LiveTrackersrv_Config);
  write_settings(szRegistryLiveTrackerport, LiveTrackerport_Config);
  write_settings(szRegistryLiveTrackerusr, LiveTrackerusr_Config);
  write_settings(szRegistryLiveTrackerpwd, LiveTrackerpwd_Config);
}

#define IO_PARAM_SIZE 160

//
// Save only Device related parameters
//
void LKDeviceSave(const TCHAR *szFile) {
#if TESTBENCH
  StartupStore(_T("... DeviceSave <%s>%s"), szFile, NEWLINE);
#endif

  if (!szFile || !szFile[0]) {
    // nullptr or empty string.
    return;
  }
  settings::writer write_settings(szFile, "DEVICE PROFILE");
  if (!write_settings) {
    StartupStore(_T("...... DeviceSaveProfile <%s> open for write FAILED!%s"), szFile, NEWLINE);
    return;
  }

  for (int n = 0; n < NUMDEV; n++) {

    write_settings(szRegistryDevice[n], dwDeviceName[n]);
    write_settings(szRegistryPortName[n], szPort[n]);
    write_settings(szRegistrySpeedIndex[n], dwSpeedIndex[n]);
    write_settings(szRegistryBitIndex[n], dwBitIndex[n]);
    write_settings(szRegistryIpAddress[n], szIpAddress[n]);
    write_settings(szRegistryIpPort[n], dwIpPort[n]);

    write_settings(szRegistryUseExtSound[n], UseExtSound[n]);

    write_settings(szRegistryReplayFile[n], Replay_FileName[n]);
    write_settings(szRegistryReplaySpeed[n], ReplaySpeed[n]);
    write_settings(szRegistryReplayRaw[n], RawByteData[n]);  
    write_settings(szRegistryReplaySync[n], ReplaySync[n]);

    TCHAR szTmp[IO_PARAM_SIZE];
    _sntprintf(szTmp,IO_PARAM_SIZE, _T("%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u"),
	       (uint)PortIO[n].MCDir    ,(uint)PortIO[n].BUGDir  ,(uint)PortIO[n].BALDir   ,
	       (uint)PortIO[n].STFDir   ,(uint)PortIO[n].WINDDir ,(uint)PortIO[n].BARODir  ,
	       (uint)PortIO[n].VARIODir ,(uint)PortIO[n].SPEEDDir,(uint)PortIO[n].R_TRGTDir,
	       (uint)PortIO[n].RADIODir ,(uint)PortIO[n].TRAFDir ,(uint)PortIO[n].GYRODir  ,
	       (uint)PortIO[n].GFORCEDir,(uint)PortIO[n].OATDir  ,(uint)PortIO[n].BAT1Dir  ,
	       (uint)PortIO[n].BAT2Dir  ,(uint)PortIO[n].POLARDir,(uint)PortIO[n].DirLink  ,
	       (uint)PortIO[n].T_TRGTDir,(uint)PortIO[n].QNHDir
	     );

    char szKey[20];
    sprintf(szKey, ("%s%u"), szRegistryIOValues, n + 1);
    write_settings(szKey, szTmp);
  }

  write_settings(szRegistryUseGeoidSeparation, UseGeoidSeparation);
  write_settings(szRegistryPollingMode, PollingMode);
  write_settings(szRegistryCheckSum, CheckSum);

  // We save GlobalModelType, not InfoBoxModel
  write_settings(szRegistryAppInfoBoxModel, GlobalModelType);
}
