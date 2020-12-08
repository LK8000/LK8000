/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"
#include "Modeltype.h"
#include "utils/stringext.h"
#include "LKProfiles.h"
#include "Asset.hpp"


static FILE *pfp=NULL;

#define PNEWLINE  "\r\n"
#define rprintf LKWriteToProfile

//
// Overload write functions
//
void LKWriteToProfile(const char *varname, bool varvalue) {
  fprintf(pfp,"%s=%d%s", varname, varvalue,PNEWLINE); // check we dont have fake bools
}
void LKWriteToProfile(const char *varname, int varvalue) {
  fprintf(pfp,"%s=%d%s", varname, varvalue,PNEWLINE);
}
void LKWriteToProfile(const char *varname, unsigned int varvalue) {
  fprintf(pfp,"%s=%u%s", varname, varvalue,PNEWLINE);
}
void LKWriteToProfile(const char *varname, double varvalue) {
  fprintf(pfp,"%s=%.0f%s", varname, varvalue,PNEWLINE);
}
void LKWriteToProfile(const char *varname, TCHAR *varvalue) {
#ifdef UNICODE
  char stmp[MAX_PATH];
  unicode2utf(varvalue, stmp, sizeof(stmp));
  fprintf(pfp,"%s=\"%s\"%s", varname, stmp ,PNEWLINE);
#else
  fprintf(pfp,"%s=\"%s\"%s", varname, varvalue ,PNEWLINE);
#endif
}



void LKProfileSave(const TCHAR *szFile)
{
  #if TESTBENCH
  StartupStore(_T("... SaveProfile <%s>%s"),szFile,NEWLINE);
  #endif


  if (_tcslen(szFile)>0)
	pfp = _tfopen(szFile, TEXT("wb")); // 'w' will overwrite content, 'b' for no crlf translation

  if(pfp == NULL) {
	StartupStore(_T("...... SaveProfile <%s> open for write FAILED!%s"),szFile,NEWLINE);
	return;
  }

  //
  // Standard header
  //
  fprintf(pfp,"### LK8000 PROFILE - DO NOT EDIT%s",PNEWLINE);
  fprintf(pfp,"### THIS FILE IS ENCODED IN UTF8%s",PNEWLINE);
  fprintf(pfp,"LKVERSION=\"%s.%s\"%s",LKVERSION,LKRELEASE,PNEWLINE);
  fprintf(pfp,"PROFILEVERSION=2%s",PNEWLINE);

  //
  // RESPECT LKPROFILE.H ALPHA ORDER OR WE SHALL GET LOST SOON!
  //
  // -- USE _CONFIG VARIABLES WHEN A RUNTIME VALUE CAN BE CHANGED --
  // WE DONT WANT TO SAVE RUNTIME TEMPORARY CONFIGURATIONS, ONLY SYSTEM CONFIG!
  // FOR EXAMPLE: ActiveMap can be set by default in system config, but also changed
  // at runtime with a button and with a customkey. We must save in profile ONLY
  // the _Config, not the temporary setup!
  //



  rprintf(szRegistryAcknowledgementTime, AcknowledgementTime);

//  >> Moved to AircraftFile <<
//  rprintf(szRegistryAircraftCategory, AircraftCategory);
//  rprintf(szRegistryAircraftRego, AircraftRego_Config);
//  rprintf(szRegistryAircraftType, AircraftType_Config);
  rprintf(szRegistryAirfieldFile, szAirfieldFile);
  for(unsigned int i = 0; i < NO_AS_FILES; i++) {
    rprintf(szRegistryAirspaceFile[i], szAirspaceFile[i]);
  }
  rprintf(szRegistryAirspaceFillType, MapWindow::GetAirSpaceFillType());
  rprintf(szRegistryAirspaceOpacity, MapWindow::GetAirSpaceOpacity());
  rprintf(szRegistryAirspaceWarningDlgTimeout, AirspaceWarningDlgTimeout);
  rprintf(szRegistryAirspaceWarningMapLabels, AirspaceWarningMapLabels);
  rprintf(szRegistryAirspaceAckAllSame, AirspaceAckAllSame);
  rprintf(szRegistryAirspaceWarningRepeatTime, AirspaceWarningRepeatTime);
  rprintf(szRegistryAirspaceWarningVerticalMargin, AirspaceWarningVerticalMargin);
  rprintf(szRegistryAirspaceWarning, AIRSPACEWARNINGS);
  rprintf(szRegistryAlarmMaxAltitude1, AlarmMaxAltitude1); // saved *1000, /1000 when used
  rprintf(szRegistryAlarmMaxAltitude2, AlarmMaxAltitude2);
  rprintf(szRegistryAlarmMaxAltitude3, AlarmMaxAltitude3);
  rprintf(szRegistryAlarmTakeoffSafety, AlarmTakeoffSafety);
  rprintf(szRegistryAltMargin, AltWarningMargin);
  rprintf(szRegistryAltMode, AltitudeMode_Config);
  rprintf(szRegistryAlternate1, Alternate1); // these are not part of configuration, but saved all the same
  rprintf(szRegistryAlternate2, Alternate2);
  rprintf(szRegistryAltitudeUnitsValue, AltitudeUnit_Config);
  rprintf(szRegistryAppIndLandable,Appearance.IndLandable);
  rprintf(szRegistryUTF8Symbolsl,Appearance.UTF8Pictorials);

//  rprintf(szRegistryAppInfoBoxModel,GlobalModelType); // We save GlobalModelType, not InfoBoxModel
  rprintf(szRegistryAppInverseInfoBox,InverseInfoBox_Config);
  rprintf(szRegistryArrivalValue,ArrivalValue);
  rprintf(szRegistryAutoAdvance,AutoAdvance_Config);
  rprintf(szRegistryAutoBacklight,EnableAutoBacklight);
  rprintf(szRegistryAutoForceFinalGlide,AutoForceFinalGlide);
  rprintf(szRegistryAutoMcMode,AutoMcMode_Config);
/*
  rprintf(szRegistryAutoMcMode,AutoMcMode);
  int tmp = (int)(MACCREADY*100.0);
  rprintf(szRegistryMacCready,tmp);*/
  rprintf(szRegistryAutoMcStatus,AutoMacCready_Config);
 // rprintf(szRegistryAutoMcStatus,AutoMacCready);
  rprintf(szRegistryAutoOrientScale,AutoOrientScale*10);
  rprintf(szRegistryAutoSoundVolume,EnableAutoSoundVolume);
  rprintf(szRegistryAutoWind,AutoWindMode_Config);
  rprintf(szRegistryAutoZoom,AutoZoom_Config);
  rprintf(szRegistryAverEffTime,AverEffTime);
//  >> Moved to AircraftFile <<
//  rprintf(szRegistryBallastSecsToEmpty,BallastSecsToEmpty);
  rprintf(szRegistryBarOpacity,BarOpacity);
  rprintf(szRegistryBestWarning,BestWarning);
  rprintf(szRegistryBgMapColor,BgMapColor_Config);
//  rprintf(szRegistryBit1Index,dwBit1Index);
//  rprintf(szRegistryBit2Index,dwBit2Index);
  rprintf(szRegistryBugs,BUGS_Config*100);
//  rprintf(szRegistryCheckSum,CheckSum);
  rprintf(szRegistryCircleZoom,MapWindow::zoom.CircleZoom());
  rprintf(szRegistryClipAlt,ClipAltitude);
//  >> Moved to AircraftFile <<
//  rprintf(szRegistryCompetitionClass,CompetitionClass_Config);
//  rprintf(szRegistryCompetitionID,CompetitionID_Config);
  rprintf(szRegistryConfBB0,ConfBB0);
  rprintf(szRegistryConfBB1,ConfBB1);
  rprintf(szRegistryConfBB2,ConfBB2);
  rprintf(szRegistryConfBB3,ConfBB3);
  rprintf(szRegistryConfBB4,ConfBB4);
  rprintf(szRegistryConfBB5,ConfBB5);
  rprintf(szRegistryConfBB6,ConfBB6);
  rprintf(szRegistryConfBB7,ConfBB7);
  rprintf(szRegistryConfBB8,ConfBB8);
  rprintf(szRegistryConfBB9,ConfBB9);
  rprintf(szRegistryConfBB0Auto,ConfBB0Auto);
  rprintf(szRegistryConfIP11,ConfIP11);
  rprintf(szRegistryConfIP12,ConfIP12);
  rprintf(szRegistryConfIP13,ConfIP13);
  rprintf(szRegistryConfIP14,ConfIP14);
  rprintf(szRegistryConfIP15,ConfIP15);
  rprintf(szRegistryConfIP16,ConfIP16);
  rprintf(szRegistryConfIP17,ConfIP17);
  rprintf(szRegistryConfIP21,ConfIP21);
  rprintf(szRegistryConfIP22,ConfIP22);
  rprintf(szRegistryConfIP23,ConfIP23);
  rprintf(szRegistryConfIP24,ConfIP24);
  rprintf(szRegistryConfIP31,ConfIP31);
  rprintf(szRegistryConfIP32,ConfIP32);
  rprintf(szRegistryConfIP33,ConfIP33);
  rprintf(szRegistryCustomKeyModeAircraftIcon,CustomKeyModeAircraftIcon);
  rprintf(szRegistryCustomKeyModeCenterScreen,CustomKeyModeCenterScreen);
  rprintf(szRegistryCustomKeyModeCenter,CustomKeyModeCenter);
  rprintf(szRegistryCustomKeyModeLeftUpCorner,CustomKeyModeLeftUpCorner);
  rprintf(szRegistryCustomKeyModeLeft,CustomKeyModeLeft);
  rprintf(szRegistryCustomKeyModeRightUpCorner,CustomKeyModeRightUpCorner);
  rprintf(szRegistryCustomKeyModeRight,CustomKeyModeRight);
  rprintf(szRegistryCustomKeyTime,CustomKeyTime);
  rprintf(szRegistryCustomMenu1,CustomMenu1);
  rprintf(szRegistryCustomMenu2,CustomMenu2);
  rprintf(szRegistryCustomMenu3,CustomMenu3);
  rprintf(szRegistryCustomMenu4,CustomMenu4);
  rprintf(szRegistryCustomMenu5,CustomMenu5);
  rprintf(szRegistryCustomMenu6,CustomMenu6);
  rprintf(szRegistryCustomMenu7,CustomMenu7);
  rprintf(szRegistryCustomMenu8,CustomMenu8);
  rprintf(szRegistryCustomMenu9,CustomMenu9);
  rprintf(szRegistryCustomMenu10,CustomMenu10);
  rprintf(szRegistryDebounceTimeout,debounceTimeout);
  rprintf(szRegistryDeclutterMode,DeclutterMode);
  ///rprintf(szRegistryDeviceA,dwDeviceName1);
  ///rprintf(szRegistryDeviceB,dwDeviceName2);
  rprintf(szRegistryDisableAutoLogger,DisableAutoLogger);
  rprintf(szRegistryDisplayText,DisplayTextType);
  if (SaveRuntime) {
      rprintf(szRegistryDisplayUpValue, DisplayOrientation);
  } else {
      rprintf(szRegistryDisplayUpValue, DisplayOrientation_Config);
  }
  rprintf(szRegistryDistanceUnitsValue,DistanceUnit_Config );
  rprintf(szRegistryEnableFLARMMap,EnableFLARMMap);
  rprintf(szRegistryEnableNavBaroAltitude,EnableNavBaroAltitude_Config);
  rprintf(szRegistryFAIFinishHeight,EnableFAIFinishHeight);
  rprintf(szRegistryFAISector,SectorType);
  rprintf(szRegistryFinalGlideTerrain,FinalGlideTerrain);
  rprintf(szRegistryFinishLine,FinishLine);
  rprintf(szRegistryFinishMinHeight,FinishMinHeight); // saved *1000, /1000 when used
  rprintf(szRegistryFinishRadius,FinishRadius);
  rprintf(szRegistryFontRenderer,FontRenderer);

  rprintf(szRegistryFontMapWaypoint,FontMapWaypoint);
  rprintf(szRegistryFontMapTopology,FontMapTopology);
  rprintf(szRegistryFontInfopage1L,FontInfopage1L);
  rprintf(szRegistryFontInfopage2L,FontInfopage2L);
  rprintf(szRegistryFontBottomBar,FontBottomBar);
  rprintf(szRegistryFontOverlayBig,FontOverlayBig);
  rprintf(szRegistryFontOverlayMedium,FontOverlayMedium);
  rprintf(szRegistryFontCustom1,FontCustom1);
  rprintf(szRegistryFontVisualGlide,FontVisualGlide);

  rprintf(szRegistryGlideBarMode,GlideBarMode);
  rprintf(szRegistryGliderScreenPosition,MapWindow::GliderScreenPosition);
  rprintf(szRegistryGpsAltitudeOffset,GPSAltitudeOffset);
//  >> Moved to AircraftFile <<
//  rprintf(szRegistryHandicap,Handicap);
  rprintf(szRegistryHideUnits,HideUnits);
  rprintf(szRegistryHomeWaypoint,HomeWaypoint);
  rprintf(szRegistryDeclTakeOffLanding,DeclTakeoffLanding);

  // InfoType for infoboxes configuration
  for (int i=0;i<MAXINFOWINDOWS;i++) rprintf(szRegistryDisplayType[i], InfoType[i]);

  rprintf(szRegistryInputFile,szInputFile);
  rprintf(szRegistryIphoneGestures,IphoneGestures);
  rprintf(szRegistryLKMaxLabels,LKMaxLabels);
  rprintf(szRegistryLKTopoZoomCat05,LKTopoZoomCat05*1000);
  rprintf(szRegistryLKTopoZoomCat100,LKTopoZoomCat100*1000);
  rprintf(szRegistryLKTopoZoomCat10,LKTopoZoomCat10*1000);
  rprintf(szRegistryLKTopoZoomCat110,LKTopoZoomCat110*1000);
  rprintf(szRegistryLKTopoZoomCat20,LKTopoZoomCat20*1000);
  rprintf(szRegistryLKTopoZoomCat30,LKTopoZoomCat30*1000);
  rprintf(szRegistryLKTopoZoomCat40,LKTopoZoomCat40*1000);
  rprintf(szRegistryLKTopoZoomCat50,LKTopoZoomCat50*1000);
  rprintf(szRegistryLKTopoZoomCat60,LKTopoZoomCat60*1000);
  rprintf(szRegistryLKTopoZoomCat70,LKTopoZoomCat70*1000);
  rprintf(szRegistryLKTopoZoomCat80,LKTopoZoomCat80*1000);
  rprintf(szRegistryLKTopoZoomCat90,LKTopoZoomCat90*1000);
  rprintf(szRegistryLKVarioBar,LKVarioBar);
  rprintf(szRegistryLKVarioVal,LKVarioVal);
  rprintf(szRegistryLanguageCode,szLanguageCode);
  rprintf(szRegistryLatLonUnits, Units::CoordinateFormat);
  rprintf(szRegistryLiftUnitsValue,LiftUnit_Config );
  rprintf(szRegistryLockSettingsInFlight,LockSettingsInFlight);
  rprintf(szRegistryLoggerShort,LoggerShortName);
  rprintf(szRegistryMapBox,MapBox);
  rprintf(szRegistryMapFile,szMapFile);
  rprintf(szRegistryMenuTimeout,MenuTimeout_Config);
  rprintf(szRegistryNewMapDeclutter,NewMapDeclutter);
  rprintf(szRegistryOrbiter,Orbiter_Config);
  rprintf(szRegistryOutlinedTp,OutlinedTp_Config);
  rprintf(szRegistryOverColor,OverColor);
  rprintf(szRegistryOverlayClock,OverlayClock);
  rprintf(szRegistryUseTwoLines,UseTwoLines);
  rprintf(szRegistryOverlaySize,OverlaySize);
  rprintf(szRegistryAutoZoomThreshold,AutoZoomThreshold);
  rprintf(szRegistryClimbZoom,ClimbZoom);
  rprintf(szRegistryCruiseZoom,CruiseZoom);
  rprintf(szRegistryMaxAutoZoom,MaxAutoZoom);
  rprintf(szRegistryTskOptimizeRoute,TskOptimizeRoute_Config);
  rprintf(szRegistryGliderSymbol,GliderSymbol);

// >> Moved to PilotFile <<
//  rprintf(szRegistryPilotName,PilotName_Config);
//  >> Moved to AircraftFile <<
//  rprintf(szRegistryPolarFile,szPolarFile);
//  rprintf(szRegistryPollingMode,PollingMode);
//  rprintf(szRegistryPort1Index,dwPortIndex1);
//  rprintf(szRegistryPort2Index,dwPortIndex2);
  rprintf(szRegistryPressureHg,PressureHg);
  rprintf(szRegistrySafetyAltitudeArrival,SAFETYALTITUDEARRIVAL);
  rprintf(szRegistrySafetyAltitudeMode,SafetyAltitudeMode);
  rprintf(szRegistrySafetyAltitudeTerrain,SAFETYALTITUDETERRAIN);
  rprintf(szRegistrySafetyMacCready,GlidePolar::SafetyMacCready*10);
//  >> Moved to AircraftFile <<
//  rprintf(szRegistrySafteySpeed,SAFTEYSPEED*1000); // m/s x1000
  rprintf(szRegistrySectorRadius,SectorRadius);

#if defined(PPC2003) || defined(PNA)
  rprintf(szRegistrySetSystemTimeFromGPS,SetSystemTimeFromGPS);
#endif

  rprintf(szRegistrySaveRuntime,SaveRuntime);
  rprintf(szRegistryShading,Shading_Config);
  rprintf(szRegistryIsoLine,IsoLine_Config);
  rprintf(szRegistrySnailTrail,TrailActive_Config);
  rprintf(szRegistrySnailScale,SnailScale);
//  rprintf(szRegistrySpeed1Index,dwSpeedIndex1);
//  rprintf(szRegistrySpeed2Index,dwSpeedIndex2);
  rprintf(szRegistrySpeedUnitsValue,SpeedUnit_Config);
  rprintf(szRegistryStartHeightRef,StartHeightRef);
  rprintf(szRegistryStartLine,StartLine);
  rprintf(szRegistryStartMaxHeightMargin,StartMaxHeightMargin);	// saved *1000, /1000 when used
  rprintf(szRegistryStartMaxHeight,StartMaxHeight);		// saved *1000, /1000 when used
  rprintf(szRegistryStartMaxSpeedMargin,StartMaxSpeedMargin);	// saved *1000, /1000 when used
  rprintf(szRegistryStartMaxSpeed,StartMaxSpeed);		// saved *1000, /1000 when used
  rprintf(szRegistryStartRadius,StartRadius);
  rprintf(szRegistryTaskSpeedUnitsValue,TaskSpeedUnit_Config);
  rprintf(szRegistryTeamcodeRefWaypoint,TeamCodeRefWaypoint);
  rprintf(szRegistryTerrainBrightness,TerrainBrightness);
  rprintf(szRegistryTerrainContrast,TerrainContrast);
  rprintf(szRegistryTerrainFile,szTerrainFile);
  rprintf(szRegistryTerrainRamp,TerrainRamp_Config);

  if (SaveRuntime) rprintf(szRegistryTerrainWhiteness,TerrainWhiteness*100);

  rprintf(szRegistryThermalBar,ThermalBar);
  rprintf(szRegistryThermalLocator,EnableThermalLocator);
  rprintf(szRegistryTpFilter,TpFilter);
  rprintf(szRegistryTrackBar,TrackBar);
  rprintf(szRegistryTrailDrift,EnableTrailDrift_Config);
  rprintf(szRegistryUTCOffset,UTCOffset);
//  rprintf(szRegistryUseGeoidSeparation,UseGeoidSeparation);
  rprintf(szRegistryUseUngestures,UseUngestures);
  rprintf(szRegistryUseTotalEnergy,UseTotalEnergy_Config);
  rprintf(szRegistryWarningTime,WarningTime);
  for(unsigned int i = 0; i < NO_WP_FILES; i++) {
    rprintf(szRegistryWayPointFile[i],szWaypointFile[i]);
  }
  rprintf(szRegistryWaypointsOutOfRange,WaypointsOutOfRange);
  rprintf(szRegistryWindCalcSpeed,WindCalcSpeed*1000); // m/s x1000
  rprintf(szRegistryWindCalcTime,WindCalcTime);

  for(int i=0;i<AIRSPACECLASSCOUNT;i++) {
	rprintf(szRegistryAirspaceMode[i],MapWindow::iAirspaceMode[i]);
	rprintf(szRegistryColour[i],MapWindow::iAirspaceColour[i]);
#ifdef HAVE_HATCHED_BRUSH
	rprintf(szRegistryBrush[i],MapWindow::iAirspaceBrush[i]);
#endif
  }


  rprintf(szRegistryUseWindRose,UseWindRose);

  //
  // Multimaps added 121003
  //

 if (SaveRuntime) {
  rprintf(szRegistryMultiTerr0,Multimap_Flags_Terrain[MP_MOVING]);
  rprintf(szRegistryMultiTerr1,Multimap_Flags_Terrain[MP_MAPTRK]);
  rprintf(szRegistryMultiTerr2,Multimap_Flags_Terrain[MP_MAPWPT]);
  rprintf(szRegistryMultiTerr3,Multimap_Flags_Terrain[MP_MAPASP]);
  rprintf(szRegistryMultiTerr4,Multimap_Flags_Terrain[MP_VISUALGLIDE]);

  rprintf(szRegistryMultiTopo0,Multimap_Flags_Topology[MP_MOVING]);
  rprintf(szRegistryMultiTopo1,Multimap_Flags_Topology[MP_MAPTRK]);
  rprintf(szRegistryMultiTopo2,Multimap_Flags_Topology[MP_MAPWPT]);
  rprintf(szRegistryMultiTopo3,Multimap_Flags_Topology[MP_MAPASP]);
  rprintf(szRegistryMultiTopo4,Multimap_Flags_Topology[MP_VISUALGLIDE]);

  rprintf(szRegistryMultiAsp0,Multimap_Flags_Airspace[MP_MOVING]);
  rprintf(szRegistryMultiAsp1,Multimap_Flags_Airspace[MP_MAPTRK]);
  rprintf(szRegistryMultiAsp2,Multimap_Flags_Airspace[MP_MAPWPT]);
  rprintf(szRegistryMultiAsp3,Multimap_Flags_Airspace[MP_MAPASP]);
  rprintf(szRegistryMultiAsp4,Multimap_Flags_Airspace[MP_VISUALGLIDE]);

  rprintf(szRegistryMultiLab0,Multimap_Labels[MP_MOVING]);
  rprintf(szRegistryMultiLab1,Multimap_Labels[MP_MAPTRK]);
  rprintf(szRegistryMultiLab2,Multimap_Labels[MP_MAPWPT]);
  rprintf(szRegistryMultiLab3,Multimap_Labels[MP_MAPASP]);
  rprintf(szRegistryMultiLab4,Multimap_Labels[MP_VISUALGLIDE]);

  rprintf(szRegistryMultiWpt0,Multimap_Flags_Waypoints[MP_MOVING]);
  rprintf(szRegistryMultiWpt1,Multimap_Flags_Waypoints[MP_MAPTRK]);
  rprintf(szRegistryMultiWpt2,Multimap_Flags_Waypoints[MP_MAPWPT]);
  rprintf(szRegistryMultiWpt3,Multimap_Flags_Waypoints[MP_MAPASP]);
  rprintf(szRegistryMultiWpt4,Multimap_Flags_Waypoints[MP_VISUALGLIDE]);

  rprintf(szRegistryMultiOvrT0,Multimap_Flags_Overlays_Text[MP_MOVING]);
  rprintf(szRegistryMultiOvrT1,Multimap_Flags_Overlays_Text[MP_MAPTRK]);
  rprintf(szRegistryMultiOvrT2,Multimap_Flags_Overlays_Text[MP_MAPWPT]);
  rprintf(szRegistryMultiOvrT3,Multimap_Flags_Overlays_Text[MP_MAPASP]);
  rprintf(szRegistryMultiOvrT4,Multimap_Flags_Overlays_Text[MP_VISUALGLIDE]);

  rprintf(szRegistryMultiOvrG0,Multimap_Flags_Overlays_Gauges[MP_MOVING]);
  rprintf(szRegistryMultiOvrG1,Multimap_Flags_Overlays_Gauges[MP_MAPTRK]);
  rprintf(szRegistryMultiOvrG2,Multimap_Flags_Overlays_Gauges[MP_MAPWPT]);
  rprintf(szRegistryMultiOvrG3,Multimap_Flags_Overlays_Gauges[MP_MAPASP]);
  rprintf(szRegistryMultiOvrG4,Multimap_Flags_Overlays_Gauges[MP_VISUALGLIDE]);

  rprintf(szRegistryMultiSizeY1,Multimap_SizeY[MP_MAPTRK]);
  rprintf(szRegistryMultiSizeY2,Multimap_SizeY[MP_MAPWPT]);
  rprintf(szRegistryMultiSizeY3,Multimap_SizeY[MP_MAPASP]);
  rprintf(szRegistryMultiSizeY4,Multimap_SizeY[MP_VISUALGLIDE]);
 }

  rprintf(szRegistryMultimap1,Multimap1);
  rprintf(szRegistryMultimap2,Multimap2);
  rprintf(szRegistryMultimap3,Multimap3);
  rprintf(szRegistryMultimap4,Multimap4);
  rprintf(szRegistryMultimap5,Multimap5);

 if (SaveRuntime) {
  rprintf(szRegistryMMNorthUp1,MMNorthUp_Runtime[0]);
  rprintf(szRegistryMMNorthUp2,MMNorthUp_Runtime[1]);
  rprintf(szRegistryMMNorthUp3,MMNorthUp_Runtime[2]);
  rprintf(szRegistryMMNorthUp4,MMNorthUp_Runtime[3]);
 }

  rprintf(szRegistryAspPermanent  ,AspPermanentChanged);
  rprintf(szRegistryFlarmDirection,iFlarmDirection);
  //rprintf(szRegistryDrawTask      ,Flags_DrawTask);  // DrawTask not any more a runtime save option . v7.1
  rprintf(szRegistryDrawFAI       ,Flags_DrawFAI_config);
  rprintf(szRegistryDrawXC   ,Flags_DrawXC_config);

  rprintf(szRegistryGearMode      ,GearWarningMode);
  rprintf(szRegistryGearAltitude  ,GearWarningAltitude);
  rprintf(szRegistryBigFAIThreshold,FAI28_45Threshold);

  if (SaveRuntime) rprintf(szRegistryBottomMode    ,BottomMode);
  rprintf(szRegistrySonarWarning    ,SonarWarning_Config);

  rprintf(szRegistryOverlay_TopLeft, Overlay_TopLeft);
  rprintf(szRegistryOverlay_TopMid, Overlay_TopMid);
  rprintf(szRegistryOverlay_TopRight, Overlay_TopRight);
  rprintf(szRegistryOverlay_TopDown, Overlay_TopDown);
  rprintf(szRegistryOverlay_LeftTop, Overlay_LeftTop);
  rprintf(szRegistryOverlay_LeftMid, Overlay_LeftMid);
  rprintf(szRegistryOverlay_LeftBottom, Overlay_LeftBottom);
  rprintf(szRegistryOverlay_LeftDown, Overlay_LeftDown);
  rprintf(szRegistryOverlay_RightTop, Overlay_RightTop);
  rprintf(szRegistryOverlay_RightMid, Overlay_RightMid);
  rprintf(szRegistryOverlay_RightBottom, Overlay_RightBottom);

  rprintf(szRegistryAdditionalContestRule, AdditionalContestRule);


#ifdef _WGS84
  rprintf(szRegistry_earth_model_wgs84,earth_model_wgs84);
#endif

  rprintf(szRegistryAutoContrast,AutoContrast);
  



  extern bool CommandResolution;
  if (SaveRuntime) if(!IsEmbedded() && !CommandResolution) {
    rprintf(szRegistryScreenSize   ,ScreenSize);
    rprintf(szRegistryScreenSizeX  ,ScreenSizeX);
    rprintf(szRegistryScreenSizeY  ,ScreenSizeY);
  }
  if (SaveRuntime)
    rprintf(szRegistrySoundSwitch,EnableSoundModes);

  fprintf(pfp,PNEWLINE); // end of file
  fflush(pfp);
  fclose(pfp);

}


void WriteDeviceSettings(const int devIdx, const TCHAR *Name){
  if (devIdx >= 0)
    if (devIdx < NUMDEV)
      _tcscpy(dwDeviceName[devIdx],Name);
}

//
// Save only Aircraft related parameters
//
void LKAircraftSave(const TCHAR *szFile)
{
  #if TESTBENCH
  StartupStore(_T("... AircraftSave <%s>%s"),szFile,NEWLINE);
  #endif


  if (_tcslen(szFile)>0)
	pfp = _tfopen(szFile, TEXT("wb")); // 'w' will overwrite content, 'b' for no crlf translation

  if(pfp == NULL) {
	StartupStore(_T("......  AircraftSaveProfile <%s> open for write FAILED!%s"),szFile,NEWLINE);
	return;
  }

  //
  // Standard header
  //
  fprintf(pfp,"### LK8000 AICRAFT PROFILE - DO NOT EDIT%s",PNEWLINE);
  fprintf(pfp,"### THIS FILE IS ENCODED IN UTF8%s",PNEWLINE);
  fprintf(pfp,"LKVERSION=\"%s.%s\"%s",LKVERSION,LKRELEASE,PNEWLINE);
  fprintf(pfp,"PROFILEVERSION=2%s",PNEWLINE);

  rprintf(szRegistryAircraftCategory, AircraftCategory);
  rprintf(szRegistryPolarFile,szPolarFile);
  rprintf(szRegistrySafteySpeed,SAFTEYSPEED*1000); // Max speed V rough air m/s x1000
  rprintf(szRegistryHandicap,Handicap);
  rprintf(szRegistryBallastSecsToEmpty,BallastSecsToEmpty);

  rprintf(szRegistryAircraftType, AircraftType_Config);
  rprintf(szRegistryAircraftRego, AircraftRego_Config);
  rprintf(szRegistryCompetitionClass,CompetitionClass_Config);
  rprintf(szRegistryCompetitionID,CompetitionID_Config);


  fprintf(pfp,PNEWLINE); // end of file
  fflush(pfp);
  fclose(pfp);

}


//
// Save only Pilot related parameters
//
void LKPilotSave(const TCHAR *szFile)
{
  #if TESTBENCH
  StartupStore(_T("... PilotSave <%s>%s"),szFile,NEWLINE);
  #endif

  if (_tcslen(szFile)>0)
	pfp = _tfopen(szFile, TEXT("wb")); // 'w' will overwrite content, 'b' for no crlf translation

  if(pfp == NULL) {
	StartupStore(_T("......  PilotSaveProfile <%s> open for write FAILED!%s"),szFile,NEWLINE);
	return;
  }

  //
  // Standard header
  //
  fprintf(pfp,"### LK8000 PILOT PROFILE - DO NOT EDIT%s",PNEWLINE);
  fprintf(pfp,"### THIS FILE IS ENCODED IN UTF8%s",PNEWLINE);
  fprintf(pfp,"LKVERSION=\"%s.%s\"%s",LKVERSION,LKRELEASE,PNEWLINE);
  fprintf(pfp,"PROFILEVERSION=2%s",PNEWLINE);

  rprintf(szRegistryPilotName,PilotName_Config);
  rprintf(szRegistryLiveTrackerInterval,LiveTrackerInterval);
  rprintf(szRegistryLiveTrackerRadar_config,LiveTrackerRadar_config);
  rprintf(szRegistryLiveTrackerStart_config,LiveTrackerStart_config);
  rprintf(szRegistryLiveTrackersrv,LiveTrackersrv_Config);
  rprintf(szRegistryLiveTrackerport,LiveTrackerport_Config);
  rprintf(szRegistryLiveTrackerusr,LiveTrackerusr_Config);
  rprintf(szRegistryLiveTrackerpwd,LiveTrackerpwd_Config);

  fprintf(pfp,PNEWLINE); // end of file
  fflush(pfp);
  fclose(pfp);

}

//
// Save only Device related parameters
//
void LKDeviceSave(const TCHAR *szFile)
{
  #if TESTBENCH
  StartupStore(_T("... DeviceSave <%s>%s"),szFile,NEWLINE);
  #endif

  if (_tcslen(szFile)>0)
	pfp = _tfopen(szFile, TEXT("wb")); // 'w' will overwrite content, 'b' for no crlf translation

  if(pfp == NULL) {
	StartupStore(_T("......  DeviceSaveProfile <%s> open for write FAILED!%s"),szFile,NEWLINE);
	return;
  }

  //
  // Standard header
  //
  fprintf(pfp,"### LK8000 DEVICE PROFILE - DO NOT EDIT%s",PNEWLINE);
  fprintf(pfp,"### THIS FILE IS ENCODED IN UTF8%s",PNEWLINE);
  fprintf(pfp,"LKVERSION=\"%s.%s\"%s",LKVERSION,LKRELEASE,PNEWLINE);
  fprintf(pfp,"PROFILEVERSION=2%s",PNEWLINE);

  rprintf(szRegistryDeviceA,dwDeviceName[0]);
  rprintf(szRegistryDeviceB,dwDeviceName[1]);
  rprintf(szRegistryDeviceC,dwDeviceName[2]);
  rprintf(szRegistryDeviceD,dwDeviceName[3]);
  rprintf(szRegistryDeviceE,dwDeviceName[4]);
  rprintf(szRegistryDeviceF,dwDeviceName[5]);

  rprintf(szRegistryPort1Name,szPort[0]);
  rprintf(szRegistryPort2Name,szPort[1]);
  rprintf(szRegistryPort3Name,szPort[2]);
  rprintf(szRegistryPort4Name,szPort[3]);
  rprintf(szRegistryPort5Name,szPort[4]);
  rprintf(szRegistryPort6Name,szPort[5]);

  rprintf(szRegistrySpeed1Index,dwSpeedIndex[0]);
  rprintf(szRegistrySpeed2Index,dwSpeedIndex[1]);
  rprintf(szRegistrySpeed3Index,dwSpeedIndex[2]);
  rprintf(szRegistrySpeed4Index,dwSpeedIndex[3]);
  rprintf(szRegistrySpeed5Index,dwSpeedIndex[4]);
  rprintf(szRegistrySpeed6Index,dwSpeedIndex[5]);

  rprintf(szRegistryBit1Index,dwBitIndex[0]);
  rprintf(szRegistryBit2Index,dwBitIndex[1]);
  rprintf(szRegistryBit3Index,dwBitIndex[2]);
  rprintf(szRegistryBit4Index,dwBitIndex[3]);
  rprintf(szRegistryBit5Index,dwBitIndex[4]);
  rprintf(szRegistryBit6Index,dwBitIndex[5]);

  rprintf(szRegistryIpAddress1,szIpAddress[0]);
  rprintf(szRegistryIpAddress2,szIpAddress[1]);
  rprintf(szRegistryIpAddress3,szIpAddress[2]);
  rprintf(szRegistryIpAddress4,szIpAddress[3]);
  rprintf(szRegistryIpAddress5,szIpAddress[4]);
  rprintf(szRegistryIpAddress6,szIpAddress[5]);

  rprintf(szRegistryIpPort1,dwIpPort[0]);
  rprintf(szRegistryIpPort2,dwIpPort[1]);
  rprintf(szRegistryIpPort3,dwIpPort[2]);
  rprintf(szRegistryIpPort4,dwIpPort[3]);
  rprintf(szRegistryIpPort5,dwIpPort[4]);
  rprintf(szRegistryIpPort6,dwIpPort[5]);

#define IO_PARAM_SIZE 160

  for(int n = 0; n < NUMDEV; n++)
  {
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

    char szKey[20] = ("");
    sprintf(szKey ,("%s%u"),szRegistryIOValues,n+1);
    rprintf(szKey,szTmp);
  }

  rprintf(szRegistryUseExtSound1,UseExtSound[0]);
  rprintf(szRegistryUseExtSound2,UseExtSound[1]);
  rprintf(szRegistryUseExtSound3,UseExtSound[2]);
  rprintf(szRegistryUseExtSound4,UseExtSound[3]);
  rprintf(szRegistryUseExtSound5,UseExtSound[4]);
  rprintf(szRegistryUseExtSound6,UseExtSound[5]);

  rprintf(szRegistryUseGeoidSeparation,UseGeoidSeparation);
  rprintf(szRegistryPollingMode,PollingMode);
  rprintf(szRegistryCheckSum,CheckSum);

  rprintf(szRegistryAppInfoBoxModel,GlobalModelType); // We save GlobalModelType, not InfoBoxModel

  fprintf(pfp,PNEWLINE); // end of file
  fflush(pfp);
  fclose(pfp);

}
