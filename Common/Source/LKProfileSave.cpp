/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Utils.h"
#include "McReady.h"
#include "Modeltype.h"

#define NEWPROFILES 1

#if NEWPROFILES

#include "LKProfiles.h"


static FILE *pfp=NULL;

#define PNEWLINE  "\r\n"
#define rprintf LKWriteToProfile

//
// Overload write functions
//
void LKWriteToProfile(const TCHAR *varname, bool varvalue) {
  // pfprintf(pfp,"%S=%d (bool)%s", varname, varvalue==1?1:0,PNEWLINE); correct
  fprintf(pfp,"%S=%d%s", varname, varvalue,PNEWLINE); // check we dont have fake bools
}
void LKWriteToProfile(const TCHAR *varname, int varvalue) {
  fprintf(pfp,"%S=%d%s", varname, varvalue,PNEWLINE);
}
void LKWriteToProfile(const TCHAR *varname, DWORD varvalue) {
  fprintf(pfp,"%S=%d%s", varname, (unsigned int) varvalue,PNEWLINE);
}
void LKWriteToProfile(const TCHAR *varname, double varvalue) {
  fprintf(pfp,"%S=%.0f%s", varname, varvalue,PNEWLINE);
}
void LKWriteToProfile(const TCHAR *varname, TCHAR *varvalue) {
  char stmp[MAX_PATH];
  unicode2utf((TCHAR*) varvalue, stmp, sizeof(stmp));
  fprintf(pfp,"%S=\"%s\"%s", varname, stmp ,PNEWLINE);
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
  fprintf(pfp,"PROFILEVERSION=1%s",PNEWLINE);

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
  rprintf(szRegistryActiveMap, ActiveMap_Config);
  rprintf(szRegistryAdditionalAirspaceFile, szAdditionalAirspaceFile);
  rprintf(szRegistryAdditionalWayPointFile, szAdditionalWaypointFile);
  rprintf(szRegistryAircraftCategory, AircraftCategory);
  rprintf(szRegistryAircraftRego, AircraftRego_Config);
  rprintf(szRegistryAircraftType, AircraftType_Config);
  rprintf(szRegistryAirfieldFile, szAirfieldFile); 
  rprintf(szRegistryAirspaceBlackOutline, MapWindow::bAirspaceBlackOutline);
  rprintf(szRegistryAirspaceFile, szAirspaceFile);
  rprintf(szRegistryAirspaceFillType, MapWindow::GetAirSpaceFillType()); 
  rprintf(szRegistryAirspaceOpacity, MapWindow::GetAirSpaceOpacity()); 
  rprintf(szRegistryAirspaceWarningDlgTimeout, AirspaceWarningDlgTimeout);
  rprintf(szRegistryAirspaceWarningMapLabels, AirspaceWarningMapLabels);
  rprintf(szRegistryAirspaceWarningRepeatTime, AirspaceWarningRepeatTime);
  rprintf(szRegistryAirspaceWarningVerticalMargin, AirspaceWarningVerticalMargin);
  rprintf(szRegistryAirspaceWarning, AIRSPACEWARNINGS);
  rprintf(szRegistryAlarmMaxAltitude1, AlarmMaxAltitude1); // saved *1000, /1000 when used
  rprintf(szRegistryAlarmMaxAltitude2, AlarmMaxAltitude2);
  rprintf(szRegistryAlarmMaxAltitude3, AlarmMaxAltitude3);
  rprintf(szRegistryAltMargin, AltWarningMargin);
  rprintf(szRegistryAltMode, AltitudeMode);
  rprintf(szRegistryAlternate1, Alternate1); // these are not part of configuration, but saved all the same
  rprintf(szRegistryAlternate2, Alternate2);
  rprintf(szRegistryAltitudeUnitsValue, AltitudeUnit_Config);
  rprintf(szRegistryAppDefaultMapWidth, Appearance.DefaultMapWidth);
  rprintf(szRegistryAppIndLandable,Appearance.IndLandable);
  rprintf(szRegistryAppInfoBoxModel,Appearance.InfoBoxModel);
  rprintf(szRegistryAppInverseInfoBox,InverseInfoBox_Config);
  rprintf(szRegistryArrivalValue,ArrivalValue);
  rprintf(szRegistryAutoAdvance,AutoAdvance_Config);
  rprintf(szRegistryAutoBacklight,EnableAutoBacklight);
  rprintf(szRegistryAutoForceFinalGlide,AutoForceFinalGlide);
  rprintf(szRegistryAutoMcMode,AutoMcMode_Config);
  rprintf(szRegistryAutoMcStatus,AutoMacCready_Config);
  rprintf(szRegistryAutoOrientScale,AutoOrientScale);
  rprintf(szRegistryAutoSoundVolume,EnableAutoSoundVolume);
  rprintf(szRegistryAutoWind,AutoWindMode);
  rprintf(szRegistryAutoZoom,MapWindow::zoom.AutoZoom()); // TODO CHECK IF OK
  rprintf(szRegistryAverEffTime,AverEffTime);
  rprintf(szRegistryBallastSecsToEmpty,BallastSecsToEmpty);
  rprintf(szRegistryBarOpacity,BarOpacity);
  rprintf(szRegistryBestWarning,BestWarning);
  rprintf(szRegistryBgMapColor,BgMapColor_Config);
  rprintf(szRegistryBit1Index,dwBit1Index);
  rprintf(szRegistryBit2Index,dwBit2Index);
  rprintf(szRegistryCheckSum,CheckSum);
  rprintf(szRegistryCircleZoom,MapWindow::zoom.CircleZoom());
  rprintf(szRegistryClipAlt,ClipAltitude);
  rprintf(szRegistryCompetitionClass,CompetitionClass_Config);
  rprintf(szRegistryCompetitionID,CompetitionID_Config);
  rprintf(szRegistryConfBB1,ConfBB1);
  rprintf(szRegistryConfBB2,ConfBB2);
  rprintf(szRegistryConfBB3,ConfBB3);
  rprintf(szRegistryConfBB4,ConfBB4);
  rprintf(szRegistryConfBB5,ConfBB5);
  rprintf(szRegistryConfBB6,ConfBB6);
  rprintf(szRegistryConfBB7,ConfBB7);
  rprintf(szRegistryConfBB8,ConfBB8);
  rprintf(szRegistryConfBB9,ConfBB9);
  rprintf(szRegistryConfIP11,ConfIP11);
  rprintf(szRegistryConfIP12,ConfIP12);
  rprintf(szRegistryConfIP13,ConfIP13);
  rprintf(szRegistryConfIP14,ConfIP14);
  rprintf(szRegistryConfIP15,ConfIP15);
  rprintf(szRegistryConfIP16,ConfIP16);
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
  rprintf(szRegistryDebounceTimeout,debounceTimeout);
  rprintf(szRegistryDeclutterMode,DeclutterMode);
  rprintf(szRegistryDeviceA,dwDeviceName1);
  rprintf(szRegistryDeviceB,dwDeviceName2);
  rprintf(szRegistryDisableAutoLogger,DisableAutoLogger);
  rprintf(szRegistryDisplayText,DisplayTextType);
  rprintf(szRegistryDisplayUpValue,DisplayOrientation_Config);
  rprintf(szRegistryDistanceUnitsValue,DistanceUnit_Config );
  rprintf(szRegistryDrawTerrain,EnableTerrain_Config);
  rprintf(szRegistryDrawTopology,EnableTopology_Config);
  rprintf(szRegistryEnableFLARMMap,EnableFLARMMap);
  rprintf(szRegistryEnableNavBaroAltitude,EnableNavBaroAltitude);
  rprintf(szRegistryFAIFinishHeight,EnableFAIFinishHeight);
  rprintf(szRegistryFAISector,SectorType);
  rprintf(szRegistryFinalGlideTerrain,FinalGlideTerrain);
  rprintf(szRegistryFinishLine,FinishLine);
  rprintf(szRegistryFinishMinHeight,FinishMinHeight); // saved *1000, /1000 when used
  rprintf(szRegistryFinishRadius,FinishRadius);
  rprintf(szRegistryFontMapLabelFont,FontDesc_MapLabel);
  rprintf(szRegistryFontMapWindowFont,FontDesc_MapWindow);
  rprintf(szRegistryFontRenderer,FontRenderer);
  rprintf(szRegistryGlideBarMode,GlideBarMode);
  rprintf(szRegistryGliderScreenPosition,MapWindow::GliderScreenPosition);
  rprintf(szRegistryGpsAltitudeOffset,GPSAltitudeOffset);
  rprintf(szRegistryHandicap,Handicap);
  rprintf(szRegistryHideUnits,HideUnits);
  rprintf(szRegistryHomeWaypoint,HomeWaypoint);

  // InfoType for infoboxes configuration
  for (int i=0;i<MAXINFOWINDOWS;i++) rprintf(szRegistryDisplayType[i], InfoType[i]);

  rprintf(szRegistryInputFile,szInputFile);
  rprintf(szRegistryIphoneGestures,IphoneGestures);
  rprintf(szRegistryLKMaxLabels,LKMaxLabels);
  rprintf(szRegistryLKTopoZoomCat05,LKTopoZoomCat05);
  rprintf(szRegistryLKTopoZoomCat100,LKTopoZoomCat100);
  rprintf(szRegistryLKTopoZoomCat10,LKTopoZoomCat10);
  rprintf(szRegistryLKTopoZoomCat110,LKTopoZoomCat110);
  rprintf(szRegistryLKTopoZoomCat20,LKTopoZoomCat20);
  rprintf(szRegistryLKTopoZoomCat30,LKTopoZoomCat30);
  rprintf(szRegistryLKTopoZoomCat40,LKTopoZoomCat40);
  rprintf(szRegistryLKTopoZoomCat50,LKTopoZoomCat50);
  rprintf(szRegistryLKTopoZoomCat60,LKTopoZoomCat60);
  rprintf(szRegistryLKTopoZoomCat70,LKTopoZoomCat70);
  rprintf(szRegistryLKTopoZoomCat80,LKTopoZoomCat80);
  rprintf(szRegistryLKTopoZoomCat90,LKTopoZoomCat90);
  rprintf(szRegistryLKVarioBar,LKVarioBar);
  rprintf(szRegistryLKVarioVal,LKVarioVal);
  rprintf(szRegistryLanguageFile,szLanguageFile);
  rprintf(szRegistryLatLonUnits, Units::CoordinateFormat);
  rprintf(szRegistryLiftUnitsValue,LiftUnit_Config );
  rprintf(szRegistryLockSettingsInFlight,LockSettingsInFlight);
  rprintf(szRegistryLoggerShort,LoggerShortName);
  rprintf(szRegistryLoggerTimeStepCircling,LoggerTimeStepCircling);
  rprintf(szRegistryLoggerTimeStepCruise,LoggerTimeStepCruise);
  rprintf(szRegistryLook8000,Look8000);
  rprintf(szRegistryMapBox,MapBox);
  rprintf(szRegistryMapFile,szMapFile);
  rprintf(szRegistryMcOverlay,McOverlay);
  rprintf(szRegistryMenuTimeout,MenuTimeout_Config);
  rprintf(szRegistryNewMapDeclutter,NewMapDeclutter);
  rprintf(szRegistryOrbiter,Orbiter_Config);
  rprintf(szRegistryOutlinedTp,OutlinedTp);
  rprintf(szRegistryOverColor,OverColor);
  rprintf(szRegistryOverlayClock,OverlayClock);
  rprintf(szRegistryOverlaySize,OverlaySize);
  rprintf(szRegistryPGAutoZoomThreshold,PGAutoZoomThreshold);
  rprintf(szRegistryPGClimbZoom,PGClimbZoom);
  rprintf(szRegistryPGCruiseZoom,PGCruiseZoom);
  rprintf(szRegistryPGGateIntervalTime,PGGateIntervalTime);
  rprintf(szRegistryPGNumberOfGates,PGNumberOfGates);
  rprintf(szRegistryPGOpenTimeH,PGOpenTimeH);
  rprintf(szRegistryPGOpenTimeM,PGOpenTimeM);
  rprintf(szRegistryPGOptimizeRoute,PGOptimizeRoute);
  rprintf(szRegistryPGStartOut,PGStartOut);
  rprintf(szRegistryPilotName,PilotName_Config);
  rprintf(szRegistryPolarFile,szPolarFile);
  rprintf(szRegistryPollingMode,PollingMode);
  rprintf(szRegistryPort1Index,dwPortIndex1);
  rprintf(szRegistryPort2Index,dwPortIndex2);
  rprintf(szRegistryPressureHg,PressureHg);
  rprintf(szRegistrySafetyAltitudeArrival,SAFETYALTITUDEARRIVAL);
  rprintf(szRegistrySafetyAltitudeMode,SafetyAltitudeMode);
  rprintf(szRegistrySafetyAltitudeTerrain,SAFETYALTITUDETERRAIN);
  rprintf(szRegistrySafetyMacCready,GlidePolar::SafetyMacCready*10);
  rprintf(szRegistrySafteySpeed,SAFTEYSPEED*1000); // m/s x1000
  rprintf(szRegistrySectorRadius,SectorRadius);
  rprintf(szRegistrySetSystemTimeFromGPS,SetSystemTimeFromGPS);
  rprintf(szRegistryShading,Shading_Config);
  rprintf(szRegistrySnailTrail,TrailActive_Config);
  rprintf(szRegistrySnailWidthScale,MapWindow::SnailWidthScale);
  rprintf(szRegistrySpeed1Index,dwSpeedIndex1);
  rprintf(szRegistrySpeed2Index,dwSpeedIndex2);
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
  rprintf(szRegistryThermalBar,ThermalBar);
  rprintf(szRegistryThermalLocator,EnableThermalLocator);
  rprintf(szRegistryTopologyFile,szTopologyFile);
  rprintf(szRegistryTpFilter,TpFilter);
  rprintf(szRegistryTrackBar,TrackBar);
  rprintf(szRegistryTrailDrift,MapWindow::EnableTrailDrift);
  rprintf(szRegistryUTCOffset,UTCOffset);
  rprintf(szRegistryUseCustomFonts,UseCustomFonts);
  rprintf(szRegistryUseGeoidSeparation,UseGeoidSeparation);
  rprintf(szRegistryUseTotalEnergy,UseTotalEnergy_Config);
  rprintf(szRegistryWarningTime,WarningTime);
  rprintf(szRegistryWayPointFile,szWaypointFile);
  rprintf(szRegistryWaypointsOutOfRange,WaypointsOutOfRange);
  rprintf(szRegistryWindCalcSpeed,WindCalcSpeed*1000); // m/s x1000
  rprintf(szRegistryWindCalcTime,WindCalcTime);

  for(int i=0;i<AIRSPACECLASSCOUNT;i++) {
	rprintf(szRegistryAirspaceMode[i],MapWindow::iAirspaceMode[i]);
	rprintf(szRegistryColour[i],MapWindow::iAirspaceColour[i]);
	rprintf(szRegistryBrush[i],MapWindow::iAirspaceBrush[i]);
  }

  /*
  // Anything containing non-ascii chars should be treated like this:
  // Unicode UTF8 converted
  static TCHAR  szMapFile[MAX_PATH] = TEXT("\0");
  GetRegistryString(szRegistryMapFile, szMapFile, MAX_PATH);
  ExpandLocalPath(szMapFile);
  char stmp[256];
  unicode2utf((TCHAR*) szMapFile, stmp, sizeof(stmp));
  rprintf("%S=\"%s\"%s", szRegistryMapFile, stmp );
  */

  fprintf(pfp,PNEWLINE); // end of file
  fflush(pfp);
  fclose(pfp);

}


#endif // NEWPROFILES
