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


#if 0
void LKWriteFileRegistryString(HANDLE hFile, TCHAR *instring) {
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
  #if TESTBENCH
  StartupStore(_T("... WriteProfile <%s>%s"),szFile,NEWLINE);
  #endif
  SaveRegistryToFile(szFile);
}
#endif

// wind save TODO
// deviceA and B name TODO


extern int nMaxValueNameSize;
extern int nMaxValueValueSize;
extern int nMaxClassSize;
extern int nMaxKeyNameSize;

#define PNEWLINE  "\r\n"


void LKProfileSave(const TCHAR *szFile)
{
  #if TESTBENCH
  StartupStore(_T("... SaveProfile <%s>%s"),szFile,NEWLINE);
  #endif

  
  FILE *fp=NULL;
  if (_tcslen(szFile)>0)
	fp = _tfopen(szFile, TEXT("wb")); // 'w' will overwrite content, 'b' for no crlf translation

  if(fp == NULL) {
	StartupStore(_T("...... SaveProfile <%s> open for write FAILED!%s"),szFile,NEWLINE);
	return;
  }

  //
  // Standard header
  //
  fprintf(fp,"### LK8000 PROFILE - DO NOT EDIT%s",PNEWLINE);
  fprintf(fp,"### THIS FILE IS ENCODED IN UTF8%s",PNEWLINE);
  fprintf(fp,"LKVERSION=\"%s.%s\"%s",LKVERSION,LKRELEASE,PNEWLINE);
  fprintf(fp,"PROFILEVERSION=1%s",PNEWLINE);

  // 
  // RESPECT LKPROFILE.H ALPHA ORDER OR WE SHALL GET LOST SOON!
  // 
  fprintf(fp,"%S=%d%s", szRegistryAcknowledgementTime, AcknowledgementTime,PNEWLINE);
  fprintf(fp,"%S=%d%s", szRegistryActiveMap, ActiveMap,PNEWLINE);

// Todo:
// AdditionalAirspaceFile
// AdditionalWPFile

  fprintf(fp,"%S=%d%s", szRegistryAircraftCategory, AircraftCategory,PNEWLINE);

//  fprintf(fp,"%S=%d%s", szRegistryAircraftRego, AircraftRego,PNEWLINE); missing global
//  fprintf(fp,"%S=%d%s", szRegistryAircraftType, AircraftType,PNEWLINE); missing global
//  fprintf(fp,"%S=%d%s", szRegistryAirfieldFile, AirfieldFile,PNEWLINE); missing global

  fprintf(fp,"%S=%d%s", szRegistryAirspaceBlackOutline, MapWindow::bAirspaceBlackOutline,PNEWLINE);

//  todo: AirspaceFile
//  fprintf(fp,"%S=%d%s", szRegistryAirspaceFillType, AirspaceFillType,PNEWLINE);  missing global
//  fprintf(fp,"%S=%d%s", szRegistryAirspaceOpacity, AirspaceOpacity,PNEWLINE); missing global

  fprintf(fp,"%S=%d%s",szRegistryAirspaceWarningDlgTimeout, AirspaceWarningDlgTimeout,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAirspaceWarningMapLabels, AirspaceWarningMapLabels,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAirspaceWarningRepeatTime, AirspaceWarningRepeatTime,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAirspaceWarningVerticalMargin, AirspaceWarningVerticalMargin,PNEWLINE);

  fprintf(fp,"%S=%d%s",szRegistryAirspaceWarning, AIRSPACEWARNINGS,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAlarmMaxAltitude1, AlarmMaxAltitude1,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAlarmMaxAltitude2, AlarmMaxAltitude2,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAlarmMaxAltitude3, AlarmMaxAltitude3,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAltMargin, AltWarningMargin,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAltMode, AltitudeMode,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAlternate1, Alternate1,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAlternate2, Alternate2,PNEWLINE);

  // fprintf(fp,"%S=%d%s",szRegistryAltitudeUnitsValue, AltitudeUnit_Config,PNEWLINE); // todo global

  fprintf(fp,"%S=%d%s",szRegistryAppDefaultMapWidth, Appearance.DefaultMapWidth,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAppIndLandable,Appearance.IndLandable,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAppInfoBoxModel,Appearance.InfoBoxModel,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAppInverseInfoBox,Appearance.InverseInfoBox,PNEWLINE);

  fprintf(fp,"%S=%d%s",szRegistryArrivalValue,ArrivalValue,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAutoAdvance,AutoAdvance,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAutoBacklight,EnableAutoBacklight,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAutoForceFinalGlide,AutoForceFinalGlide,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAutoMcMode,AutoMcMode,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAutoMcStatus,AutoMacCready_Config,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAutoOrientScale,AutoOrientScale,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAutoSoundVolume,EnableAutoSoundVolume,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAutoWind,AutoWindMode,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAutoZoom,MapWindow::zoom.AutoZoom(),PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryAverEffTime,AverEffTime,PNEWLINE);

  fprintf(fp,"%S=%d%s",szRegistryBallastSecsToEmpty,BallastSecsToEmpty,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryBarOpacity,BarOpacity,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryBestWarning,BestWarning,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryBgMapColor,BgMapColor,PNEWLINE);

  //fprintf(fp,"%S=%d%s",szRegistryBit1Index,Bit1Index,PNEWLINE); // missing global
  //fprintf(fp,"%S=%d%s",szRegistryBit2Index,Bit2Index,PNEWLINE); // missing global

  fprintf(fp,"%S=%d%s",szRegistryCheckSum,CheckSum,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryCircleZoom,MapWindow::zoom.CircleZoom(),PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryClipAlt,ClipAltitude,PNEWLINE);

  // CompetitionClass  missing global
  // CompetitionID  missing global

  fprintf(fp,"%S=%d%s",szRegistryConfBB1,ConfBB1,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfBB2,ConfBB2,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfBB3,ConfBB3,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfBB4,ConfBB4,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfBB5,ConfBB5,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfBB6,ConfBB6,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfBB7,ConfBB7,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfBB8,ConfBB8,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfBB9,ConfBB9,PNEWLINE);

  fprintf(fp,"%S=%d%s",szRegistryConfIP11,ConfIP11,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfIP12,ConfIP12,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfIP13,ConfIP13,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfIP14,ConfIP14,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfIP15,ConfIP15,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfIP16,ConfIP16,PNEWLINE);

  fprintf(fp,"%S=%d%s",szRegistryConfIP21,ConfIP21,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfIP22,ConfIP22,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfIP23,ConfIP23,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfIP24,ConfIP24,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfIP31,ConfIP31,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfIP32,ConfIP32,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryConfIP33,ConfIP33,PNEWLINE);

  fprintf(fp,"%S=%d%s",szRegistryCustomKeyModeAircraftIcon,CustomKeyModeAircraftIcon,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryCustomKeyModeCenterScreen,CustomKeyModeCenterScreen,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryCustomKeyModeCenter,CustomKeyModeCenter,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryCustomKeyModeLeftUpCorner,CustomKeyModeLeftUpCorner,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryCustomKeyModeLeft,CustomKeyModeLeft,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryCustomKeyModeRightUpCorner,CustomKeyModeRightUpCorner,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryCustomKeyModeRight,CustomKeyModeRight,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryCustomKeyTime,CustomKeyTime,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryDebounceTimeout,debounceTimeout,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryDeclutterMode,DeclutterMode,PNEWLINE);

  // DeviceA  missing global
  // DeviceB  missing global

  fprintf(fp,"%S=%d%s",szRegistryDisableAutoLogger,DisableAutoLogger,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryDisplayText,DisplayTextType,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryDisplayUpValue,DisplayOrientation_Config,PNEWLINE);

  // fprintf(fp,"%S=%d%s",szRegistryDistanceUnitsValue,  ,PNEWLINE);   missing global
  
  fprintf(fp,"%S=%d%s",szRegistryDrawTerrain,EnableTerrain,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryDrawTopology,EnableTopology,PNEWLINE);

  fprintf(fp,"%S=%d%s",szRegistryEnableFLARMMap,EnableFLARMMap,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryEnableNavBaroAltitude,EnableNavBaroAltitude,PNEWLINE);

  // Extended visual to remove

  fprintf(fp,"%S=%d%s",szRegistryFAIFinishHeight,FAIFinishHeight,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryFAISector,SectorType,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryFinalGlideTerrain,FinalGlideTerrain,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryFinishLine,FinishLine,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryFinishMinHeight,FinishMinHeight,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryFinishRadius,FinishRadius,PNEWLINE);
  // fprintf(fp,"%S=%d%s",szRegistryFontMapLabelFont,MapLabelFont,PNEWLINE); // todo fix missing global
  // fprintf(fp,"%S=%d%s",szRegistryFontMapWindowFont,MapWindowFont,PNEWLINE); // todo fix missing global

  fprintf(fp,"%S=%d%s",szRegistryFontRenderer,FontRenderer,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryGlideBarMode,GlideBarMode,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryGliderScreenPosition,MapWindow::GliderScreenPosition,PNEWLINE);
  fprintf(fp,"%S=%.0f%s",szRegistryGpsAltitudeOffset,GPSAltitudeOffset,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryHandicap,Handicap,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryHideUnits,HideUnits,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryHomeWaypoint,HomeWaypoint,PNEWLINE);

  // fprintf(fp,"%S=%d%s",szRegistryInputFile,InputFile,PNEWLINE);  missing global

  fprintf(fp,"%S=%d%s",szRegistryIphoneGestures,IphoneGestures,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryLKMaxLabels,LKMaxLabels,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat05,LKTopoZoomCat05,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat100,LKTopoZoomCat100,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat10,LKTopoZoomCat10,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat110,LKTopoZoomCat110,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat20,LKTopoZoomCat20,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat30,LKTopoZoomCat30,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat40,LKTopoZoomCat40,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat50,LKTopoZoomCat50,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat60,LKTopoZoomCat60,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat70,LKTopoZoomCat70,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat80,LKTopoZoomCat80,PNEWLINE);
  fprintf(fp,"%S=%.1f%s",szRegistryLKTopoZoomCat90,LKTopoZoomCat90,PNEWLINE);

  fprintf(fp,"%S=%d%s",szRegistryLKVarioBar,LKVarioBar,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryLKVarioVal,LKVarioVal,PNEWLINE);

  // fprintf(fp,"%S=%d%s",szRegistryLanguageFile,,PNEWLINE); missing global

  fprintf(fp,"%S=%d%s", szRegistryLatLonUnits, Units::CoordinateFormat,PNEWLINE);

  // fprintf(fp,"%S=%d%s", szRegistryLiftUnitsValue, ,PNEWLINE);  missing global

  fprintf(fp,"%S=%d%s", szRegistryLockSettingsInFlight,LockSettingsInFlight,PNEWLINE);

  // fprintf(fp,"%S=%d%s", szRegistryLoggerID,,PNEWLINE); // LoggerID missing

  fprintf(fp,"%S=%d%s",szRegistryLoggerShort,LoggerShortName,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryLoggerTimeStepCircling,LoggerTimeStepCircling,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryLoggerTimeStepCruise,LoggerTimeStepCruise,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryLook8000,Look8000,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryMapBox,MapBox,PNEWLINE);
  // fprintf(fp,"%S=%d%s",szRegistryMapFile,,PNEWLINE); // missing MapFile  global

  fprintf(fp,"%S=%d%s",szRegistryMcOverlay,McOverlay,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryMenuTimeout,MenuTimeout_Config,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryNewMapDeclutter,NewMapDeclutter,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryOrbiter,Orbiter,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryOutlinedTp,OutlinedTp,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryOverColor,OverColor,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryOverlayClock,OverlayClock,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryOverlaySize,OverlaySize,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryPGAutoZoomThreshold,PGAutoZoomThreshold,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryPGClimbZoom,PGClimbZoom,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryPGCruiseZoom,PGCruiseZoom,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryPGGateIntervalTime,PGGateIntervalTime,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryPGNumberOfGates,PGNumberOfGates,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryPGOpenTimeH,PGOpenTimeH,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryPGOpenTimeM,PGOpenTimeM,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryPGOptimizeRoute,PGOptimizeRoute,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryPGStartOut,PGStartOut,PNEWLINE);

  // fprintf(fp,"%S=%d%s",szRegistryPilotName,,PNEWLINE);  missing global
  // fprintf(fp,"%S=%d%s",szRegistryPolarFile,,PNEWLINE);  missing global

  fprintf(fp,"%S=%d%s",szRegistryPollingMode,PollingMode,PNEWLINE);

  // fprintf(fp,"%S=%d%s",szRegistryPort1Index,,PNEWLINE);  missing global
  // fprintf(fp,"%S=%d%s",szRegistryPort2Index,,PNEWLINE);  missing global

  fprintf(fp,"%S=%d%s",szRegistryPressureHg,PressureHg,PNEWLINE);
  fprintf(fp,"%S=%.0f%s",szRegistrySafetyAltitudeArrival,SAFETYALTITUDEARRIVAL,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistrySafetyAltitudeMode,SafetyAltitudeMode,PNEWLINE);
  fprintf(fp,"%S=%.0f%s",szRegistrySafetyAltitudeTerrain,SAFETYALTITUDETERRAIN,PNEWLINE);
  fprintf(fp,"%S=%.0f%s",szRegistrySafetyMacCready,GlidePolar::SafetyMacCready,PNEWLINE);
  fprintf(fp,"%S=%.0f%s",szRegistrySafteySpeed,SAFTEYSPEED,PNEWLINE);
  fprintf(fp,"%S=%.0f%s",szRegistrySectorRadius,SectorRadius,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistrySetSystemTimeFromGPS,SetSystemTimeFromGPS,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryShading,Shading,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistrySnailTrail,SnailTrail,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistrySnailWidthScale,MapWindow::SnailWidthScale,PNEWLINE);

  // fprintf(fp,"%S=%d%s",szRegistrySpeed1Index,SpeedIndex,PNEWLINE); missing global
  // fprintf(fp,"%S=%d%s",szRegistrySpeed2Index,Speed2Index,PNEWLINE); missing global
  // fprintf(fp,"%S=%d%s",szRegistrySpeedUnitsValue,,PNEWLINE); // missing global

  fprintf(fp,"%S=%d%s",szRegistryStartHeightRef,StartHeightRef,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryStartLine,StartLine,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryStartMaxHeightMargin,StartMaxHeightMargin,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryStartMaxHeight,StartMaxHeight,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryStartMaxSpeedMargin,StartMaxSpeedMargin,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryStartMaxSpeed,StartMaxSpeed,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryStartRadius,StartRadius,PNEWLINE);

  // fprintf(fp,"%S=%d%s",szRegistryTaskSpeedUnitsValue,,PNEWLINE); missing global

  fprintf(fp,"%S=%d%s",szRegistryTeamcodeRefWaypoint,TeamCodeRefWaypoint,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryTerrainBrightness,TerrainBrightness,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryTerrainContrast,TerrainContrast,PNEWLINE);

  // fprintf(fp,"%S=%d%s",szRegistryTerrainFile,,PNEWLINE); missing global

  fprintf(fp,"%S=%d%s",szRegistryTerrainRamp,TerrainRamp,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryThermalBar,ThermalBar,PNEWLINE);

  fprintf(fp,"%S=%d%s",szRegistryThermalLocator,EnableThermalLocator,PNEWLINE);

  // fprintf(fp,"%S=%d%s",szRegistryTopologyFile,,PNEWLINE); missing global

  fprintf(fp,"%S=%d%s",szRegistryTpFilter,TpFilter,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryTrackBar,TrackBar,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryTrailDrift,MapWindow::EnableTrailDrift,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryUTCOffset,UTCOffset,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryUseCustomFonts,UseCustomFonts,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryUseGeoidSeparation,UseGeoidSeparation,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryUseTotalEnergy,UseTotalEnergy,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryWarningTime,WarningTime,PNEWLINE);

  // fprintf(fp,"%S=%d%s",szRegistryWayPointFile,,PNEWLINE); missing global

  fprintf(fp,"%S=%d%s",szRegistryWaypointsOutOfRange,WaypointsOutOfRange,PNEWLINE);
  fprintf(fp,"%S=%.0f%s",szRegistryWindCalcSpeed,WindCalcSpeed,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryWindCalcTime,WindCalcTime,PNEWLINE);
  fprintf(fp,"%S=%d%s",szRegistryWindUpdateMode,WindUpdateMode,PNEWLINE);

  /*
  // Anything containing non-ascii chars should be treated like this:
  // Unicode UTF8 converted
  static TCHAR  szMapFile[MAX_PATH] = TEXT("\0");
  GetRegistryString(szRegistryMapFile, szMapFile, MAX_PATH);
  ExpandLocalPath(szMapFile);
  char stmp[256];
  unicode2utf((TCHAR*) szMapFile, stmp, sizeof(stmp));
  fprintf(fp,"%S=\"%s\"%s", szRegistryMapFile, stmp ,PNEWLINE);
  */
end:
  fprintf(fp,"\r\n"); // end of file
  fflush(fp);
  fclose(fp);

}


#endif // NEWPROFILES
