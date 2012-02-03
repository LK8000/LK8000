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
#include "LKProfiles.h"

#if NEWPROFILES

// #define DEBUGPROF	1
extern void LKParseProfileString(TCHAR *sname, TCHAR *svalue);

static bool matchedstring=false;		// simple accelerator

//
// Overload read functions
//
void SetProfileVariable(const TCHAR *curname, TCHAR *curvalue, TCHAR *lookupname, bool *lookupvalue) {
  if (_tcscmp(curname,lookupname)) return;
  int ival= wcstol(curvalue, NULL, 10);
  *lookupvalue= (bool)(ival==1?true:false);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> bool=%d\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}
void SetProfileVariable(const TCHAR *curname, TCHAR *curvalue, TCHAR *lookupname, double *lookupvalue) {
  if (_tcscmp(curname,lookupname)) return;
  *lookupvalue=(double) wcstol(curvalue, NULL, 10);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> double=%.0f\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}
void SetProfileVariable(const TCHAR *curname, TCHAR *curvalue, TCHAR *lookupname, DWORD *lookupvalue) {
  if (_tcscmp(curname,lookupname)) return;
  *lookupvalue=(int) wcstol(curvalue, NULL, 10);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> DWORD=%d\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}
void SetProfileVariable(const TCHAR *curname, TCHAR *curvalue, TCHAR *lookupname, int *lookupvalue) {
  if (_tcscmp(curname,lookupname)) return;
  *lookupvalue=(int) wcstol(curvalue, NULL, 10);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> int=%d\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}
void SetProfileVariable(const TCHAR *curname, TCHAR *curvalue, TCHAR *lookupname, short *lookupvalue) {
  if (_tcscmp(curname,lookupname)) return;
  *lookupvalue=(short) wcstol(curvalue, NULL, 10);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> short=%d\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}
void SetProfileVariable(const TCHAR *curname, TCHAR *curvalue, TCHAR *lookupname, TCHAR *lookupvalue) {
  if (_tcscmp(curname,lookupname)) return;
  _tcscpy(lookupvalue,curvalue);
  // REMEMBER TO CONVERT FROM UTF8 to UNICODE!!
  // char stmp[MAX_PATH];
  // unicode2utf((TCHAR*) varvalue, stmp, sizeof(stmp));
  // fprintf(pfp,"%S=\"%s\" (TCHAR)%s", varname, stmp ,PNEWLINE);
  #if DEBUGPROF 
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> tchar=<%s>\n"),
  curname,curvalue,lookupname,lookupvalue);
  #endif
  matchedstring=true;
}


int nMaxValueValueSize = MAX_PATH*2 + 6;	// max regkey name is 256 chars + " = "

//
// Returns true if at least one value was found,
// excluded comments and empty lines
//
bool LKProfileLoad(TCHAR *szFile)
{
  #if TESTBENCH
  StartupStore(_T("... LoadProfile <%s>%s"),szFile,NEWLINE);
  #endif

  bool found = false;
  FILE *fp=NULL;
  int j;

  if (_tcslen(szFile)>0)
	fp = _tfopen(szFile, TEXT("rb"));

  if(fp == NULL) {
	StartupStore(_T(".... LoadProfile <%s> open failed%s"),szFile,NEWLINE);
	return false;
  }

  TCHAR winval[nMaxValueValueSize];
  TCHAR wname[nMaxValueValueSize];
  TCHAR wvalue[nMaxValueValueSize];

  char inval[nMaxValueValueSize];
  char name [nMaxValueValueSize];
  char value [nMaxValueValueSize];

  // if using mingw, parse utf8 first
  #ifdef __MINGW32__
  goto parse_utf8;
  #endif
 
parse_wide:
 
  // Wide Chars file
  while (_fgetts(winval, nMaxValueValueSize, fp)) {
	if (winval[0] > 255) { // not reading corectly, probably narrow file.
		break;
	}
	matchedstring=false;
	if (_stscanf(winval, TEXT("%[^#=\r\n ]=\"%[^\r\n\"]\"[\r\n]"), wname, wvalue) == 2) {
		if (_tcslen(wname)>0) {
			LKParseProfileString(wname, wvalue);
			found = true;
		}
	} else if (_stscanf(winval, TEXT("%[^#=\r\n ]=%d[\r\n]"), wname, &j) == 2) {
		if (_tcslen(wname)>0) {
			_stprintf(wvalue,_T("%d"),j);
			LKParseProfileString(wname, wvalue);
			found = true;
		}
	} else if (_stscanf(winval, TEXT("%[^#=\r\n ]=\"\"[\r\n]"), wname) == 1) {
		if (_tcslen(wname)>0) {
			LKParseProfileString(wname, TEXT(""));
			found = true;
		}
	}
	// else crlf, or comment, or invalid line
  }

  // if using mingw, this is a second attempt already so return
  #ifdef __MINGW32__
  goto go_return;
  #endif

parse_utf8:

  // UTF8 file
  while (fgets(inval, nMaxValueValueSize, fp)) {
	matchedstring=false;
	if (sscanf(inval, "%[^#=\r\n ]=\"%[^\r\n\"]\"[\r\n]", name, value) == 2) {
		if (strlen(name)>0) {
			utf2unicode(name, wname, nMaxValueValueSize);
			utf2unicode(value, wvalue, nMaxValueValueSize);
			LKParseProfileString(wname, wvalue);
			found = true;
		}
	} else if (sscanf(inval, "%[^#=\r\n ]=%d[\r\n]", name, &j) == 2) {
		if (strlen(name)>0) {
			utf2unicode(name, wname, nMaxValueValueSize);
			_stprintf(wvalue,_T("%d"),j);
			LKParseProfileString(wname, wvalue);
			found = true;
		}
	} else if (sscanf(inval, "%[^#=\r\n ]=\"\"[\r\n]", name) == 1) {
		if (strlen(name)>0) {
			utf2unicode(name, wname, nMaxValueValueSize);
			LKParseProfileString(wname, TEXT(""));
			found = true;
		}
	} 
	// else crlf, or comment, or invalid line
	// else StartupStore(_T("...... PARSE INVALID: <%S>\n"),inval);
  }

  // if using mingw and nothing found in utf8 file, try with wide chars
  #ifdef __MINGW32__
  if (!found) goto parse_wide;
  #endif

go_return:

  fclose(fp);
  if (found) {
	LKProfileInitRuntime();
  }
  return found;
}


using std::max;
#define PREAD SetProfileVariable

//
// Search for a match of the keyname. Profile is NOT necessarily sorted!
// So we must check against all possible values until we find the good one.
// As soon as we find the match, we can return.
// Important: some parameters are saved multiplied by 10 or 1000, so they must
// be adjusted here. Example SafetyMacCready
// Notice that we must check that we are not getting a matchedstring of another match!
// This is why we do it twice, before and after the PREAD. 
// Another approach is to use for example  if (!_tcscmp(szRegistryCircleZoom,sname)) {
// We shall make PREAD return a bool to tell us, next time, and get rid of this terrible stuff.
// 
void LKParseProfileString(TCHAR *sname, TCHAR *svalue) {

  //#if DEBUGPROF
  //StartupStore(_T("... Parse: <%s> = <%s>\n"),sname,svalue);
  //#endif

  int ival;
  // 
  // RESPECT LKPROFILE.H ALPHA ORDER OR WE SHALL GET LOST SOON!
  // 
  // -- USE _CONFIG VARIABLES WHEN A RUNTIME VALUE CAN BE CHANGED --
  // WE DONT WANT TO SAVE RUNTIME TEMPORARY CONFIGURATIONS, ONLY SYSTEM CONFIG!
  // FOR EXAMPLE: ActiveMap can be set by default in system config, but also changed
  // at runtime with a button and with a customkey. We must save in profile ONLY
  // the _Config, not the temporary setup!
  //

  PREAD(sname,svalue,szRegistryAcknowledgementTime, &AcknowledgementTime);
  if (matchedstring) {
	AcknowledgementTime = max(10, AcknowledgementTime);
	return;
  }
  PREAD(sname,svalue,szRegistryActiveMap, &ActiveMap_Config);
  PREAD(sname,svalue,szRegistryAdditionalAirspaceFile, &*szAdditionalAirspaceFile);
  PREAD(sname,svalue,szRegistryAdditionalWayPointFile, &*szAdditionalWaypointFile);
  PREAD(sname,svalue,szRegistryAircraftCategory, &AircraftCategory);
  PREAD(sname,svalue,szRegistryAircraftRego, &*AircraftRego_Config);
  PREAD(sname,svalue,szRegistryAircraftType, &*AircraftType_Config);
  PREAD(sname,svalue,szRegistryAirfieldFile, &*szAirfieldFile); 
  PREAD(sname,svalue,szRegistryAirspaceBlackOutline, &MapWindow::bAirspaceBlackOutline);
  PREAD(sname,svalue,szRegistryAirspaceFile, &*szAirspaceFile);
  if (matchedstring) return; // every 10 or so PREADs we check for quick return

  // Special cases with no global variable and a function to access the private variable.
  // This is bad. We want a common global variable approach for the future.
  // We want a memory area with values, not with function calls.

  if (!_tcscmp(szRegistryAirspaceFillType,sname)) {
	ival=wcstol(svalue, NULL, 10);
	MapWindow::SetAirSpaceFillType((MapWindow::EAirspaceFillType)ival);
	return;
  }
  if (!_tcscmp(szRegistryAirspaceOpacity,sname)) {
	ival=wcstol(svalue, NULL, 10);
	MapWindow::SetAirSpaceOpacity(ival);
	return;
  }

  PREAD(sname,svalue,szRegistryAirspaceWarningDlgTimeout, &AirspaceWarningDlgTimeout);
  PREAD(sname,svalue,szRegistryAirspaceWarningMapLabels, &AirspaceWarningMapLabels);
  PREAD(sname,svalue,szRegistryAirspaceWarningRepeatTime, &AirspaceWarningRepeatTime);
  PREAD(sname,svalue,szRegistryAirspaceWarningVerticalMargin, &AirspaceWarningVerticalMargin);
  PREAD(sname,svalue,szRegistryAirspaceWarning, &AIRSPACEWARNINGS);
  PREAD(sname,svalue,szRegistryAlarmMaxAltitude1,&AlarmMaxAltitude1);
  PREAD(sname,svalue,szRegistryAlarmMaxAltitude2,&AlarmMaxAltitude2);
  PREAD(sname,svalue,szRegistryAlarmMaxAltitude3,&AlarmMaxAltitude3);
  PREAD(sname,svalue,szRegistryAltMargin,&AltWarningMargin);
  PREAD(sname,svalue,szRegistryAltMode,&AltitudeMode_Config);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryAlternate1,&Alternate1);
  PREAD(sname,svalue,szRegistryAlternate2,&Alternate2);
  PREAD(sname,svalue,szRegistryAltitudeUnitsValue,&AltitudeUnit_Config);
  PREAD(sname,svalue,szRegistryAppDefaultMapWidth,&Appearance.DefaultMapWidth);

  if (!_tcscmp(szRegistryAppIndLandable,sname)) {
	ival=wcstol(svalue, NULL, 10);
	Appearance.IndLandable = (IndLandable_t)ival;
	return;
  }
  if (!_tcscmp(szRegistryAppInfoBoxModel,sname)) {
	ival=wcstol(svalue, NULL, 10);
	Appearance.InfoBoxModel = (InfoBoxModelAppearance_t)ival;
	return;
  }

  PREAD(sname,svalue,szRegistryAppInverseInfoBox,&InverseInfoBox_Config);
  PREAD(sname,svalue,szRegistryArrivalValue,&ArrivalValue);
  PREAD(sname,svalue,szRegistryAutoAdvance,&AutoAdvance_Config);
  PREAD(sname,svalue,szRegistryAutoBacklight,&EnableAutoBacklight);
  PREAD(sname,svalue,szRegistryAutoForceFinalGlide,&AutoForceFinalGlide);
  PREAD(sname,svalue,szRegistryAutoMcMode,&AutoMcMode_Config);
  PREAD(sname,svalue,szRegistryAutoMcStatus,&AutoMacCready_Config);

  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryAutoOrientScale,&AutoOrientScale);
  if (matchedstring) {
	AutoOrientScale/=10;
	return;
  }


  PREAD(sname,svalue,szRegistryAutoSoundVolume,&EnableAutoSoundVolume);
  PREAD(sname,svalue,szRegistryAutoWind,&AutoWindMode_Config);

  PREAD(sname,svalue,szRegistryAutoZoom,&AutoZoom_Config);

  PREAD(sname,svalue,szRegistryAverEffTime,&AverEffTime);
  PREAD(sname,svalue,szRegistryBallastSecsToEmpty,&BallastSecsToEmpty);
  PREAD(sname,svalue,szRegistryBarOpacity,&BarOpacity);
  PREAD(sname,svalue,szRegistryBestWarning,&BestWarning);
  PREAD(sname,svalue,szRegistryBgMapColor,&BgMapColor_Config);
  PREAD(sname,svalue,szRegistryBit1Index,&dwBit1Index);
  PREAD(sname,svalue,szRegistryBit2Index,&dwBit2Index);

  // We save multiplied by 100, so we adjust it back after loading
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryBugs,&BUGS_Config);
  if (matchedstring) {
	BUGS_Config /= 100;
	return;
  }

  PREAD(sname,svalue,szRegistryCheckSum,&CheckSum);

  if (!_tcscmp(szRegistryCircleZoom,sname)) {
	ival=wcstol(svalue, NULL, 10);
	MapWindow::zoom.CircleZoom(ival == 1);
	return;
  }

  PREAD(sname,svalue,szRegistryClipAlt,&ClipAltitude);
  PREAD(sname,svalue,szRegistryCompetitionClass,&*CompetitionClass_Config);
  PREAD(sname,svalue,szRegistryCompetitionID,&*CompetitionID_Config);
  PREAD(sname,svalue,szRegistryConfBB0,&ConfBB0);
  PREAD(sname,svalue,szRegistryConfBB1,&ConfBB1);
  PREAD(sname,svalue,szRegistryConfBB2,&ConfBB2);
  PREAD(sname,svalue,szRegistryConfBB3,&ConfBB3);
  PREAD(sname,svalue,szRegistryConfBB4,&ConfBB4);
  PREAD(sname,svalue,szRegistryConfBB5,&ConfBB5);
  PREAD(sname,svalue,szRegistryConfBB6,&ConfBB6);
  PREAD(sname,svalue,szRegistryConfBB7,&ConfBB7);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryConfBB8,&ConfBB8);
  PREAD(sname,svalue,szRegistryConfBB9,&ConfBB9);
  PREAD(sname,svalue,szRegistryConfIP11,&ConfIP11);
  PREAD(sname,svalue,szRegistryConfIP12,&ConfIP12);
  PREAD(sname,svalue,szRegistryConfIP13,&ConfIP13);
  PREAD(sname,svalue,szRegistryConfIP14,&ConfIP14);
  PREAD(sname,svalue,szRegistryConfIP15,&ConfIP15);
  PREAD(sname,svalue,szRegistryConfIP16,&ConfIP16);
  PREAD(sname,svalue,szRegistryConfIP21,&ConfIP21);
  PREAD(sname,svalue,szRegistryConfIP22,&ConfIP22);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryConfIP23,&ConfIP23);
  PREAD(sname,svalue,szRegistryConfIP24,&ConfIP24);
  PREAD(sname,svalue,szRegistryConfIP31,&ConfIP31);
  PREAD(sname,svalue,szRegistryConfIP32,&ConfIP32);
  PREAD(sname,svalue,szRegistryConfIP33,&ConfIP33);
  PREAD(sname,svalue,szRegistryCustomKeyModeAircraftIcon,&CustomKeyModeAircraftIcon);
  PREAD(sname,svalue,szRegistryCustomKeyModeCenterScreen,&CustomKeyModeCenterScreen);
  PREAD(sname,svalue,szRegistryCustomKeyModeCenter,&CustomKeyModeCenter);
  PREAD(sname,svalue,szRegistryCustomKeyModeLeftUpCorner,&CustomKeyModeLeftUpCorner);
  PREAD(sname,svalue,szRegistryCustomKeyModeLeft,&CustomKeyModeLeft);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryCustomKeyModeRightUpCorner,&CustomKeyModeRightUpCorner);
  PREAD(sname,svalue,szRegistryCustomKeyModeRight,&CustomKeyModeRight);
  PREAD(sname,svalue,szRegistryCustomKeyTime,&CustomKeyTime);
  PREAD(sname,svalue,szRegistryDebounceTimeout,&debounceTimeout);
  PREAD(sname,svalue,szRegistryDeclutterMode,&DeclutterMode);
  PREAD(sname,svalue,szRegistryDeviceA,&*dwDeviceName1);
  PREAD(sname,svalue,szRegistryDeviceB,&*dwDeviceName2);
  PREAD(sname,svalue,szRegistryDisableAutoLogger,&DisableAutoLogger);
  PREAD(sname,svalue,szRegistryEnableLiveTracker,&EnableLiveTracker);
  PREAD(sname,svalue,szRegistryDisplayText,&DisplayTextType);
  PREAD(sname,svalue,szRegistryDisplayUpValue,&DisplayOrientation_Config);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryDistanceUnitsValue,&DistanceUnit_Config );
  PREAD(sname,svalue,szRegistryDrawTerrain,&EnableTerrain_Config);
  PREAD(sname,svalue,szRegistryDrawTopology,&EnableTopology_Config);
  PREAD(sname,svalue,szRegistryEnableFLARMMap,&EnableFLARMMap);
  PREAD(sname,svalue,szRegistryEnableNavBaroAltitude,&EnableNavBaroAltitude_Config);
  PREAD(sname,svalue,szRegistryFAIFinishHeight,&EnableFAIFinishHeight);
  PREAD(sname,svalue,szRegistryFAISector,&SectorType);
  PREAD(sname,svalue,szRegistryFinalGlideTerrain,&FinalGlideTerrain);
  PREAD(sname,svalue,szRegistryFinishLine,&FinishLine);
  PREAD(sname,svalue,szRegistryFinishMinHeight,&FinishMinHeight);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryFinishRadius,&FinishRadius);
  PREAD(sname,svalue,szRegistryFontMapLabelFont,&*FontDesc_MapLabel);
  PREAD(sname,svalue,szRegistryFontMapWindowFont,&*FontDesc_MapWindow);
  PREAD(sname,svalue,szRegistryFontRenderer,&FontRenderer);
  PREAD(sname,svalue,szRegistryGlideBarMode,&GlideBarMode);
  PREAD(sname,svalue,szRegistryGliderScreenPosition,&MapWindow::GliderScreenPosition);
  PREAD(sname,svalue,szRegistryGpsAltitudeOffset,&GPSAltitudeOffset);
  PREAD(sname,svalue,szRegistryHandicap,&Handicap);
  PREAD(sname,svalue,szRegistryHideUnits,&HideUnits);
  PREAD(sname,svalue,szRegistryHomeWaypoint,&HomeWaypoint);

  // InfoType 
  for (int i=0;i<MAXINFOWINDOWS;i++) {
        PREAD(sname,svalue,&*szRegistryDisplayType[i], &InfoType[i]);
	if (matchedstring) return;
  }

  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryInputFile,&*szInputFile);
  PREAD(sname,svalue,szRegistryIphoneGestures,&IphoneGestures);
  PREAD(sname,svalue,szRegistryLKMaxLabels,&LKMaxLabels);

  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryLKTopoZoomCat05,&LKTopoZoomCat05);
  if (matchedstring) {; LKTopoZoomCat05/=1000; return;}
  PREAD(sname,svalue,szRegistryLKTopoZoomCat100,&LKTopoZoomCat100);
  if (matchedstring) {; LKTopoZoomCat100/=1000; return;}
  PREAD(sname,svalue,szRegistryLKTopoZoomCat10,&LKTopoZoomCat10);
  if (matchedstring) {; LKTopoZoomCat10/=1000; return;}
  PREAD(sname,svalue,szRegistryLKTopoZoomCat110,&LKTopoZoomCat110);
  if (matchedstring) {; LKTopoZoomCat110/=1000; return;}
  PREAD(sname,svalue,szRegistryLKTopoZoomCat20,&LKTopoZoomCat20);
  if (matchedstring) {; LKTopoZoomCat20/=1000; return;}
  PREAD(sname,svalue,szRegistryLKTopoZoomCat30,&LKTopoZoomCat30);
  if (matchedstring) {; LKTopoZoomCat30/=1000; return;}
  PREAD(sname,svalue,szRegistryLKTopoZoomCat40,&LKTopoZoomCat40);
  if (matchedstring) {; LKTopoZoomCat40/=1000; return;}
  PREAD(sname,svalue,szRegistryLKTopoZoomCat50,&LKTopoZoomCat50);
  if (matchedstring) {; LKTopoZoomCat50/=1000; return;}
  PREAD(sname,svalue,szRegistryLKTopoZoomCat60,&LKTopoZoomCat60);
  if (matchedstring) {; LKTopoZoomCat60/=1000; return;}
  PREAD(sname,svalue,szRegistryLKTopoZoomCat70,&LKTopoZoomCat70);
  if (matchedstring) {; LKTopoZoomCat70/=1000; return;}
  PREAD(sname,svalue,szRegistryLKTopoZoomCat80,&LKTopoZoomCat80);
  if (matchedstring) {; LKTopoZoomCat80/=1000; return;}
  PREAD(sname,svalue,szRegistryLKTopoZoomCat90,&LKTopoZoomCat90);
  if (matchedstring) {; LKTopoZoomCat90/=1000; return;}
  PREAD(sname,svalue,szRegistryLKVarioBar,&LKVarioBar);
  PREAD(sname,svalue,szRegistryLKVarioVal,&LKVarioVal);
  PREAD(sname,svalue,szRegistryLanguageFile,&*szLanguageFile);

  if (!_tcscmp(szRegistryLatLonUnits,sname)) {
	ival=wcstol(svalue, NULL, 10);
	Units::CoordinateFormat = (CoordinateFormats_t)ival;
	return;
  }

  PREAD(sname,svalue,szRegistryLiftUnitsValue,&LiftUnit_Config );
  PREAD(sname,svalue,szRegistryLockSettingsInFlight,&LockSettingsInFlight);
  PREAD(sname,svalue,szRegistryLoggerShort,&LoggerShortName);
  PREAD(sname,svalue,szRegistryLoggerTimeStepCircling,&LoggerTimeStepCircling);
  PREAD(sname,svalue,szRegistryLoggerTimeStepCruise,&LoggerTimeStepCruise);
  PREAD(sname,svalue,szRegistryLook8000,&Look8000);
  PREAD(sname,svalue,szRegistryMapBox,&MapBox);
  PREAD(sname,svalue,szRegistryMapFile,&*szMapFile);
  PREAD(sname,svalue,szRegistryMcOverlay,&McOverlay);
  PREAD(sname,svalue,szRegistryMenuTimeout,&MenuTimeout_Config);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryNewMapDeclutter,&NewMapDeclutter);
  PREAD(sname,svalue,szRegistryOrbiter,&Orbiter_Config);
  PREAD(sname,svalue,szRegistryOutlinedTp,&OutlinedTp);
  PREAD(sname,svalue,szRegistryOverColor,&OverColor);
  PREAD(sname,svalue,szRegistryOverlayClock,&OverlayClock);
  PREAD(sname,svalue,szRegistryOverlaySize,&OverlaySize);
  PREAD(sname,svalue,szRegistryPGAutoZoomThreshold,&PGAutoZoomThreshold);
  PREAD(sname,svalue,szRegistryPGClimbZoom,&PGClimbZoom);
  PREAD(sname,svalue,szRegistryPGCruiseZoom,&PGCruiseZoom);
  PREAD(sname,svalue,szRegistryPGGateIntervalTime,&PGGateIntervalTime);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryPGNumberOfGates,&PGNumberOfGates);
  PREAD(sname,svalue,szRegistryPGOpenTimeH,&PGOpenTimeH);
  PREAD(sname,svalue,szRegistryPGOpenTimeM,&PGOpenTimeM);
  PREAD(sname,svalue,szRegistryPGOptimizeRoute,&PGOptimizeRoute);
  PREAD(sname,svalue,szRegistryPGStartOut,&PGStartOut);
  PREAD(sname,svalue,szRegistryPilotName,&*PilotName_Config);
  PREAD(sname,svalue,szRegistryLiveTrackerpwd,&*LiveTrackerpwd_Config);
  PREAD(sname,svalue,szRegistryPolarFile,&*szPolarFile);
  PREAD(sname,svalue,szRegistryPollingMode,&PollingMode);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryPort1Index,&dwPortIndex1);
  PREAD(sname,svalue,szRegistryPort2Index,&dwPortIndex2);
  PREAD(sname,svalue,szRegistryPressureHg,&PressureHg);
  PREAD(sname,svalue,szRegistrySafetyAltitudeArrival,&SAFETYALTITUDEARRIVAL);
  PREAD(sname,svalue,szRegistrySafetyAltitudeMode,&SafetyAltitudeMode);
  PREAD(sname,svalue,szRegistrySafetyAltitudeTerrain,&SAFETYALTITUDETERRAIN);

  // We save SafetyMacCready multiplied by 10, so we adjust it back after loading
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistrySafetyMacCready,&GlidePolar::SafetyMacCready);
  if (matchedstring) {
	GlidePolar::SafetyMacCready /= 10;
	return;
  }

  if (matchedstring) return;
  PREAD(sname,svalue,szRegistrySafteySpeed,&SAFTEYSPEED);
  if (matchedstring) {
	SAFTEYSPEED = (double)SAFTEYSPEED/1000.0;
	if (SAFTEYSPEED <8.0) {
		#if TESTBENCH
		StartupStore(_T("... SAFTEYSPEED<8 set to 50 = 180kmh\n"));
		#endif
		SAFTEYSPEED=50.0;
	}
	return;
  }

  PREAD(sname,svalue,szRegistrySectorRadius,&SectorRadius);
  PREAD(sname,svalue,szRegistrySetSystemTimeFromGPS,&SetSystemTimeFromGPS);
  PREAD(sname,svalue,szRegistryShading,&Shading_Config);
  PREAD(sname,svalue,szRegistrySnailTrail,&TrailActive_Config);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistrySnailWidthScale,&MapWindow::SnailWidthScale);
  PREAD(sname,svalue,szRegistrySpeed1Index,&dwSpeedIndex1);
  PREAD(sname,svalue,szRegistrySpeed2Index,&dwSpeedIndex2);
  PREAD(sname,svalue,szRegistrySpeedUnitsValue,&SpeedUnit_Config);
  PREAD(sname,svalue,szRegistryStartHeightRef,&StartHeightRef);
  PREAD(sname,svalue,szRegistryStartLine,&StartLine);
  PREAD(sname,svalue,szRegistryStartMaxHeightMargin,&StartMaxHeightMargin);
  PREAD(sname,svalue,szRegistryStartMaxHeight,&StartMaxHeight);
  PREAD(sname,svalue,szRegistryStartMaxSpeedMargin,&StartMaxSpeedMargin);
  PREAD(sname,svalue,szRegistryStartMaxSpeed,&StartMaxSpeed);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryStartRadius,&StartRadius);
  PREAD(sname,svalue,szRegistryTaskSpeedUnitsValue,&TaskSpeedUnit_Config);
  PREAD(sname,svalue,szRegistryTeamcodeRefWaypoint,&TeamCodeRefWaypoint);
  PREAD(sname,svalue,szRegistryTerrainBrightness,&TerrainBrightness);
  PREAD(sname,svalue,szRegistryTerrainContrast,&TerrainContrast);
  PREAD(sname,svalue,szRegistryTerrainFile,&*szTerrainFile);
  PREAD(sname,svalue,szRegistryTerrainRamp,&TerrainRamp_Config);
  PREAD(sname,svalue,szRegistryThermalBar,&ThermalBar);
  PREAD(sname,svalue,szRegistryThermalLocator,&EnableThermalLocator);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryTpFilter,&TpFilter);
  PREAD(sname,svalue,szRegistryTrackBar,&TrackBar);
  PREAD(sname,svalue,szRegistryTrailDrift,&EnableTrailDrift_Config);

  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryUTCOffset,&UTCOffset);
  if (matchedstring) {
	if (UTCOffset>12*3600)
		UTCOffset-= 24*3600;
	return;
  }

  PREAD(sname,svalue,szRegistryUseCustomFonts,&UseCustomFonts);
  PREAD(sname,svalue,szRegistryUseGeoidSeparation,&UseGeoidSeparation);
  PREAD(sname,svalue,szRegistryUseTotalEnergy,&UseTotalEnergy_Config);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryWarningTime,&WarningTime);
  PREAD(sname,svalue,szRegistryWayPointFile,&*szWaypointFile);
  PREAD(sname,svalue,szRegistryWaypointsOutOfRange,&WaypointsOutOfRange);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryWindCalcSpeed,&WindCalcSpeed);
  if (matchedstring) {
	WindCalcSpeed = (double)WindCalcSpeed/1000.0;
	if (WindCalcSpeed <2)
		WindCalcSpeed=27.778;
	return;
  }
  PREAD(sname,svalue,szRegistryWindCalcTime,&WindCalcTime);

  if (matchedstring) return;
  for(int i=0;i<AIRSPACECLASSCOUNT;i++) {
	PREAD(sname,svalue,&*szRegistryAirspaceMode[i],&MapWindow::iAirspaceMode[i]);
	if (matchedstring) return;
	PREAD(sname,svalue,&*szRegistryColour[i],&MapWindow::iAirspaceColour[i]);
	if (matchedstring) return;
	PREAD(sname,svalue,&*szRegistryBrush[i],&MapWindow::iAirspaceBrush[i]);
	if (matchedstring) return;
  }

  return;

}


void ReadDeviceSettings(const int devIdx, TCHAR *Name){
  Name[0] = '\0';
  if (devIdx == 0) _tcscpy(Name,dwDeviceName1);
  if (devIdx == 1) _tcscpy(Name,dwDeviceName2);
  if (_tcslen(Name)==0) _tcscpy(Name,_T(DEV_DISABLED_NAME));
}
void ReadPort1Settings(DWORD *PortIndex, DWORD *SpeedIndex, DWORD *Bit1Index) {
  *PortIndex	=dwPortIndex1;
  *SpeedIndex	=dwSpeedIndex1;
  *Bit1Index	=dwBit1Index;
}
void ReadPort2Settings(DWORD *PortIndex, DWORD *SpeedIndex, DWORD *Bit1Index) {
  *PortIndex	=dwPortIndex2;
  *SpeedIndex	=dwSpeedIndex2;
  *Bit1Index	=dwBit2Index;
}




#endif // NEWPROFILES
