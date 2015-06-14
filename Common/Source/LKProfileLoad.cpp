/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"
#include "Modeltype.h"
#include "LKProfiles.h"
#include "utils/stringext.h"
#include "Asset.hpp"
#include "Screen/Init.hpp"

// #define DEBUGPROF	1
void LKParseProfileString(const char *sname, const char *svalue);

static bool matchedstring=false;		// simple accelerator

//
// Overload read functions
//
void SetProfileVariable(const char *curname, const char *curvalue, const char *lookupname, bool *lookupvalue) {
  if (strcmp(curname,lookupname)) return;
  int ival= strtol(curvalue, NULL, 10);
  *lookupvalue= (bool)(ival==1?true:false);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> bool=%d\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}
void SetProfileVariable(const char *curname, const char *curvalue, const char *lookupname, double *lookupvalue) {
  if (strcmp(curname,lookupname)) return;
  *lookupvalue=(double) strtol(curvalue, NULL, 10);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> double=%.0f\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}
void SetProfileVariable(const char *curname, const char *curvalue, const char *lookupname, DWORD *lookupvalue) {
  if (strcmp(curname,lookupname)) return;
  *lookupvalue=(int) strtoul(curvalue, NULL, 10);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> DWORD=%d\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}
void SetProfileVariable(const char *curname, const char *curvalue, const char *lookupname, int *lookupvalue) {
  if (strcmp(curname,lookupname)) return;
  *lookupvalue=(int)strtol(curvalue, NULL, 10);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> int=%d\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}
void SetProfileVariable(const char *curname, const char *curvalue, const char *lookupname, unsigned int *lookupvalue) {
  if (strcmp(curname,lookupname)) return;
  *lookupvalue=(int) strtoul(curvalue, NULL, 10);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> int=%d\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}
void SetProfileVariable(const char *curname, const char *curvalue, const char *lookupname, short *lookupvalue) {
  if (strcmp(curname,lookupname)) return;
  *lookupvalue=(short) strtol(curvalue, NULL, 10);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> short=%d\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}
void SetProfileVariable(const char *curname, const char *curvalue, const char *lookupname, unsigned short *lookupvalue) {
  if (strcmp(curname,lookupname)) return;
  *lookupvalue=(unsigned short) strtoul(curvalue, NULL, 10);
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> unsigned short=%d\n"),
  curname,curvalue,lookupname,*lookupvalue);
  #endif
  matchedstring=true;
}

void SetProfileVariable(const char *curname, const char *curvalue, const char *lookupname, TCHAR* lookupvalue, size_t size) {
  if (strcmp(curname,lookupname)) return;
#ifdef UNICODE  
  utf2unicode(curvalue, lookupvalue, size);
  // REMEMBER TO CONVERT FROM UTF8 to UNICODE!!
  // char stmp[MAX_PATH];
  // unicode2utf((TCHAR*) varvalue, stmp, sizeof(stmp));
  // fprintf(pfp,"%S=\"%s\" (TCHAR)%s", varname, stmp ,PNEWLINE);
#else
  strncpy(lookupvalue, curvalue, size);
#endif
  #if DEBUGPROF 
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> tchar=<%s>\n"),
  curname,curvalue,lookupname,lookupvalue);
  #endif
  matchedstring=true;
}

void SetProfileVariable(const char *curname, const char *curvalue, const char *lookupname, Poco::Timespan* lookupvalue) {
  if (strcmp(curname,lookupname)) return;
  int ival= strtol(curvalue, NULL, 10);
 
  (*lookupvalue).assign(0, 1000*ival);
  matchedstring=true;
}

int nMaxValueValueSize = MAX_PATH*2 + 6;	// max regkey name is 256 chars + " = "

static bool isDefaultProfile=false; // needed to avoid screensize changes from custom profiles on PC

//
// Returns true if at least one value was found,
// excluded comments and empty lines
//
bool LKProfileLoad(const TCHAR *szFile)
{
  #if TESTBENCH
  StartupStore(_T("... LoadProfile <%s>%s"),szFile,NEWLINE);
  #endif

  if(!IsEmbedded()) {
    if (_tcscmp(defaultProfileFile,szFile)==0) {
       isDefaultProfile=true;
    } else {
       isDefaultProfile=false;
    }
  }
  
  bool found = false;
  FILE *fp=NULL;
  int j;

  if (_tcslen(szFile)>0)
	fp = _tfopen(szFile, TEXT("rb"));

  if(fp == NULL) {
	StartupStore(_T(".... LoadProfile <%s> open failed%s"),szFile,NEWLINE);
	return false;
  }

  char inval[nMaxValueValueSize];
  char name [nMaxValueValueSize];
  char value [nMaxValueValueSize];

  // UTF8 file
  while (fgets(inval, nMaxValueValueSize, fp)) {
	matchedstring=false;
	if (sscanf(inval, "%[^#=\r\n ]=\"%[^\r\n\"]\"[\r\n]", name, value) == 2) {
		if (strlen(name)>0) {
			LKParseProfileString(name, value);
			found = true;
		}
	} else if (sscanf(inval, "%[^#=\r\n ]=%d[\r\n]", name, &j) == 2) {
		if (strlen(name)>0) {
			sprintf(value,"%d",j);
			LKParseProfileString(name, value);
			found = true;
		}
	} else if (sscanf(inval, "%[^#=\r\n ]=\"\"[\r\n]", name) == 1) {
		if (strlen(name)>0) {
			LKParseProfileString(name, "");
			found = true;
		}
	} 
	// else crlf, or comment, or invalid line
	// else StartupStore(_T("...... PARSE INVALID: <%S>\n"),inval);
  }

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
void LKParseProfileString(const char *sname, const char *svalue) {

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
  PREAD(sname,svalue,szRegistryAdditionalAirspaceFile, &*szAdditionalAirspaceFile, array_size(szAdditionalAirspaceFile));
  PREAD(sname,svalue,szRegistryAdditionalWayPointFile, &*szAdditionalWaypointFile, array_size(szAdditionalWaypointFile));
  PREAD(sname,svalue,szRegistryAircraftCategory, &AircraftCategory);
  PREAD(sname,svalue,szRegistryAircraftRego, &*AircraftRego_Config, array_size(AircraftRego_Config));
  PREAD(sname,svalue,szRegistryAircraftType, &*AircraftType_Config, array_size(AircraftType_Config));
  PREAD(sname,svalue,szRegistryAirfieldFile, &*szAirfieldFile, array_size(szAirfieldFile)); 
  PREAD(sname,svalue,szRegistryAirspaceFile, &*szAirspaceFile, array_size(szAirspaceFile));
  if (matchedstring) return; // every 10 or so PREADs we check for quick return

  // Special cases with no global variable and a function to access the private variable.
  // This is bad. We want a common global variable approach for the future.
  // We want a memory area with values, not with function calls.

  if (!strcmp(szRegistryAirspaceFillType,sname)) {
	ival=strtol(svalue, NULL, 10);
	MapWindow::SetAirSpaceFillType((MapWindow::EAirspaceFillType)ival);
	return;
  }
  if (!strcmp(szRegistryAirspaceOpacity,sname)) {
	ival=strtol(svalue, NULL, 10);
	MapWindow::SetAirSpaceOpacity(ival);
	return;
  }

  PREAD(sname,svalue,szRegistryAirspaceWarningDlgTimeout, &AirspaceWarningDlgTimeout);
  PREAD(sname,svalue,szRegistryAirspaceWarningMapLabels, &AirspaceWarningMapLabels);
  PREAD(sname,svalue,szRegistryAirspaceAckAllSame, &AirspaceAckAllSame);
  PREAD(sname,svalue,szRegistryAirspaceWarningRepeatTime, &AirspaceWarningRepeatTime);
  PREAD(sname,svalue,szRegistryAirspaceWarningVerticalMargin, &AirspaceWarningVerticalMargin);
  PREAD(sname,svalue,szRegistryAirspaceWarning, &AIRSPACEWARNINGS);
  PREAD(sname,svalue,szRegistryAlarmMaxAltitude1,&AlarmMaxAltitude1);
  PREAD(sname,svalue,szRegistryAlarmMaxAltitude2,&AlarmMaxAltitude2);
  PREAD(sname,svalue,szRegistryAlarmMaxAltitude3,&AlarmMaxAltitude3);
  PREAD(sname,svalue,szRegistryAlarmTakeoffSafety,&AlarmTakeoffSafety);
  PREAD(sname,svalue,szRegistryAltMargin,&AltWarningMargin);
  PREAD(sname,svalue,szRegistryAltMode,&AltitudeMode_Config);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryAlternate1,&Alternate1);
  PREAD(sname,svalue,szRegistryAlternate2,&Alternate2);
  PREAD(sname,svalue,szRegistryAltitudeUnitsValue,&AltitudeUnit_Config);
  PREAD(sname,svalue,szRegistryAppDefaultMapWidth,&Appearance.DefaultMapWidth);

  if (!strcmp(szRegistryAppIndLandable,sname)) {
	ival=strtol(svalue, NULL, 10);
	Appearance.IndLandable = (IndLandable_t)ival;
	return;
  }
  if (!strcmp(szRegistryAppInfoBoxModel,sname)) {
	ival=strtol(svalue, NULL, 10);
	Appearance.InfoBoxModel = (InfoBoxModelAppearance_t)ival;
	return;
  }

  PREAD(sname,svalue,szRegistryAppInverseInfoBox,&InverseInfoBox_Config);
  PREAD(sname,svalue,szRegistryArrivalValue,&ArrivalValue);
  PREAD(sname,svalue,szRegistryAutoAdvance,&AutoAdvance_Config);
  PREAD(sname,svalue,szRegistryAutoBacklight,&EnableAutoBacklight);
  PREAD(sname,svalue,szRegistryAutoForceFinalGlide,&AutoForceFinalGlide);
  PREAD(sname,svalue,szRegistryAutoMcMode,&AutoMcMode_Config);
/*
  PREAD(sname,svalue,szRegistryAutoMcMode,&AutoMcMode);
  DWORD McTmp=0;
  PREAD(sname,svalue,szRegistryMacCready,&McTmp);
  if (matchedstring)
  {
    MACCREADY = (double) McTmp;  MACCREADY /= 100.0;

  }*/
  PREAD(sname,svalue,szRegistryAutoMcStatus,&AutoMacCready_Config);
//  PREAD(sname,svalue,szRegistryAutoMcStatus,&AutoMacCready);

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

  if (!strcmp(szRegistryCircleZoom,sname)) {
	ival=strtol(svalue, NULL, 10);
	MapWindow::zoom.CircleZoom(ival == 1);
	return;
  }

  PREAD(sname,svalue,szRegistryClipAlt,&ClipAltitude);
  PREAD(sname,svalue,szRegistryCompetitionClass,&*CompetitionClass_Config, array_size(CompetitionClass_Config));
  PREAD(sname,svalue,szRegistryCompetitionID,&*CompetitionID_Config, array_size(CompetitionID_Config));
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
  PREAD(sname,svalue,szRegistryConfBB0Auto,&ConfBB0Auto);
  PREAD(sname,svalue,szRegistryConfIP11,&ConfIP11);
  PREAD(sname,svalue,szRegistryConfIP12,&ConfIP12);
  PREAD(sname,svalue,szRegistryConfIP13,&ConfIP13);
  PREAD(sname,svalue,szRegistryConfIP14,&ConfIP14);
  PREAD(sname,svalue,szRegistryConfIP15,&ConfIP15);
  PREAD(sname,svalue,szRegistryConfIP16,&ConfIP16);
  PREAD(sname,svalue,szRegistryConfIP17,&ConfIP17);
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

  PREAD(sname,svalue,szRegistryCustomMenu1,&CustomMenu1);
  PREAD(sname,svalue,szRegistryCustomMenu2,&CustomMenu2);
  PREAD(sname,svalue,szRegistryCustomMenu3,&CustomMenu3);
  PREAD(sname,svalue,szRegistryCustomMenu4,&CustomMenu4);
  PREAD(sname,svalue,szRegistryCustomMenu5,&CustomMenu5);
  PREAD(sname,svalue,szRegistryCustomMenu6,&CustomMenu6);
  PREAD(sname,svalue,szRegistryCustomMenu7,&CustomMenu7);
  PREAD(sname,svalue,szRegistryCustomMenu8,&CustomMenu8);
  PREAD(sname,svalue,szRegistryCustomMenu9,&CustomMenu9);
  PREAD(sname,svalue,szRegistryCustomMenu10,&CustomMenu10);

  PREAD(sname,svalue,szRegistryDebounceTimeout,&debounceTimeout);
  PREAD(sname,svalue,szRegistryDeclutterMode,&DeclutterMode);
  PREAD(sname,svalue,szRegistryDeviceA,&*dwDeviceName1, array_size(dwDeviceName1));
  PREAD(sname,svalue,szRegistryDeviceB,&*dwDeviceName2, array_size(dwDeviceName2));
  PREAD(sname,svalue,szRegistryDisableAutoLogger,&DisableAutoLogger);
  PREAD(sname,svalue,szRegistryLiveTrackerInterval,&LiveTrackerInterval);
  PREAD(sname,svalue,szRegistryDisplayText,&DisplayTextType);
  PREAD(sname,svalue,szRegistryDisplayUpValue,&DisplayOrientation_Config);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryDistanceUnitsValue,&DistanceUnit_Config );
  PREAD(sname,svalue,szRegistryEnableFLARMMap,&EnableFLARMMap);
  PREAD(sname,svalue,szRegistryEnableNavBaroAltitude,&EnableNavBaroAltitude_Config);
  PREAD(sname,svalue,szRegistryFAIFinishHeight,&EnableFAIFinishHeight);
  PREAD(sname,svalue,szRegistryFAISector,&SectorType);
  PREAD(sname,svalue,szRegistryFinalGlideTerrain,&FinalGlideTerrain);
  PREAD(sname,svalue,szRegistryFinishLine,&FinishLine);
  PREAD(sname,svalue,szRegistryFinishMinHeight,&FinishMinHeight);
  PREAD(sname,svalue,szRegistryFinishRadius,&FinishRadius);
  PREAD(sname,svalue,szRegistryFontRenderer,&FontRenderer);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryFontMapWaypoint,&FontMapWaypoint);
  PREAD(sname,svalue,szRegistryFontMapTopology,&FontMapTopology);
  PREAD(sname,svalue,szRegistryFontInfopage1L,&FontInfopage1L);
  PREAD(sname,svalue,szRegistryFontInfopage2L,&FontInfopage2L);
  PREAD(sname,svalue,szRegistryFontBottomBar,&FontBottomBar);
  PREAD(sname,svalue,szRegistryFontCustom1,&FontCustom1);
  PREAD(sname,svalue,szRegistryFontOverlayBig,&FontOverlayBig);
  PREAD(sname,svalue,szRegistryFontOverlayMedium,&FontOverlayMedium);
  if (matchedstring) return;

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
  PREAD(sname,svalue,szRegistryInputFile,&*szInputFile, array_size(szInputFile));
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
  PREAD(sname,svalue,szRegistryLanguageFile,&*szLanguageFile, array_size(szLanguageFile));

  if (!strcmp(szRegistryLatLonUnits,sname)) {
	ival=strtol(svalue, NULL, 10);
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
  PREAD(sname,svalue,szRegistryMapFile,&*szMapFile, array_size(szMapFile));
  PREAD(sname,svalue,szRegistryMcOverlay,&McOverlay);
  PREAD(sname,svalue,szRegistryMenuTimeout,&MenuTimeout_Config);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryNewMapDeclutter,&NewMapDeclutter);
  PREAD(sname,svalue,szRegistryOrbiter,&Orbiter_Config);
  PREAD(sname,svalue,szRegistryOutlinedTp,&OutlinedTp_Config);
  PREAD(sname,svalue,szRegistryOverColor,&OverColor);
  PREAD(sname,svalue,szRegistryOverlayClock,&OverlayClock);
  PREAD(sname,svalue,szRegistryUseTwoLines,&UseTwoLines);
  PREAD(sname,svalue,szRegistryOverlaySize,&OverlaySize);
  PREAD(sname,svalue,szRegistryPGAutoZoomThreshold,&PGAutoZoomThreshold);
  PREAD(sname,svalue,szRegistryPGClimbZoom,&PGClimbZoom);
  PREAD(sname,svalue,szRegistryPGCruiseZoom,&PGCruiseZoom);
 // PREAD(sname,svalue,szRegistryPGGateIntervalTime,&PGGateIntervalTime);
  if (matchedstring) return;
 // PREAD(sname,svalue,szRegistryPGNumberOfGates,&PGNumberOfGates);
 // PREAD(sname,svalue,szRegistryPGOpenTimeH,&PGOpenTimeH);
 // PREAD(sname,svalue,szRegistryPGOpenTimeM,&PGOpenTimeM);
  PREAD(sname,svalue,szRegistryPGOptimizeRoute,&PGOptimizeRoute_Config);
 // PREAD(sname,svalue,szRegistryPGStartOut,&PGStartOut);
  PREAD(sname,svalue,szRegistryPilotName,&*PilotName_Config, array_size(PilotName_Config));
  PREAD(sname,svalue,szRegistryLiveTrackersrv,&*LiveTrackersrv_Config, array_size(LiveTrackersrv_Config));
  PREAD(sname,svalue,szRegistryLiveTrackerport,&LiveTrackerport_Config);
  PREAD(sname,svalue,szRegistryLiveTrackerusr,&*LiveTrackerusr_Config, array_size(LiveTrackerusr_Config));
  PREAD(sname,svalue,szRegistryLiveTrackerpwd,&*LiveTrackerpwd_Config, array_size(LiveTrackerpwd_Config));
  PREAD(sname,svalue,szRegistryPolarFile,&*szPolarFile, array_size(szPolarFile));
  PREAD(sname,svalue,szRegistryPollingMode,&PollingMode);
  if (matchedstring) return;
  
  /***************************************************/
  /* for compatibilty with old file                  */
  DWORD dwIdxPort;
  PREAD(sname,svalue,szRegistryPort1Index,&dwIdxPort);
    if(matchedstring) {
        if(COMMPort.size() == 0) {
            RefreshComPortList();
        }
        if(dwIdxPort < COMMPort.size()) {
          _tcscpy(szPort1, COMMPort[dwIdxPort].GetName());
        }
        return;
    }
  
  PREAD(sname,svalue,szRegistryPort2Index,&dwIdxPort);
    if(matchedstring) {
        if(COMMPort.size() == 0) {
            RefreshComPortList();
        }
        if(dwIdxPort < COMMPort.size()) {
          _tcscpy(szPort2, COMMPort[dwIdxPort].GetName());
        }
        return;
    }
  /***************************************************/
  PREAD(sname,svalue,szRegistryPort1Name,szPort1, array_size(szPort1));
  PREAD(sname,svalue,szRegistryPort2Name,szPort2, array_size(szPort2));
    
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
  PREAD(sname,svalue,szRegistrySaveRuntime,&SaveRuntime);
  if (matchedstring) return;
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
  PREAD(sname,svalue,szRegistryTerrainFile,szTerrainFile, array_size(szTerrainFile));
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

  PREAD(sname,svalue,szRegistryUseGeoidSeparation,&UseGeoidSeparation);
  PREAD(sname,svalue,szRegistryUseExtSound1,&UseExtSound1);
  PREAD(sname,svalue,szRegistryUseExtSound2,&UseExtSound2);
  PREAD(sname,svalue,szRegistryUseUngestures,&UseUngestures);
  PREAD(sname,svalue,szRegistryUseTotalEnergy,&UseTotalEnergy_Config);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryWarningTime,&WarningTime);
  PREAD(sname,svalue,szRegistryWayPointFile,szWaypointFile, array_size(szWaypointFile));
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
	PREAD(sname,svalue,szRegistryAirspaceMode[i],&MapWindow::iAirspaceMode[i]);
	if (matchedstring) return;
	PREAD(sname,svalue,szRegistryColour[i],&MapWindow::iAirspaceColour[i]);
	if (matchedstring) return;
#ifdef HAVE_HATCHED_BRUSH
	PREAD(sname,svalue,szRegistryBrush[i],&MapWindow::iAirspaceBrush[i]);
	if (matchedstring) return;
#endif
  }

  PREAD(sname, svalue, szRegistryUseWindRose, &UseWindRose);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryMultiTerr0,&Multimap_Flags_Terrain[MP_MOVING]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiTerr1,&Multimap_Flags_Terrain[MP_MAPTRK]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiTerr2,&Multimap_Flags_Terrain[MP_MAPWPT]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiTerr3,&Multimap_Flags_Terrain[MP_MAPASP]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiTerr4,&Multimap_Flags_Terrain[MP_VISUALGLIDE]);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryMultiTopo0,&Multimap_Flags_Topology[MP_MOVING]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiTopo1,&Multimap_Flags_Topology[MP_MAPTRK]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiTopo2,&Multimap_Flags_Topology[MP_MAPWPT]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiTopo3,&Multimap_Flags_Topology[MP_MAPASP]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiTopo4,&Multimap_Flags_Topology[MP_VISUALGLIDE]);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryMultiAsp0,&Multimap_Flags_Airspace[MP_MOVING]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiAsp1,&Multimap_Flags_Airspace[MP_MAPTRK]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiAsp2,&Multimap_Flags_Airspace[MP_MAPWPT]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiAsp3,&Multimap_Flags_Airspace[MP_MAPASP]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiAsp4,&Multimap_Flags_Airspace[MP_VISUALGLIDE]);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryMultiLab0,&Multimap_Labels[MP_MOVING]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiLab1,&Multimap_Labels[MP_MAPTRK]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiLab2,&Multimap_Labels[MP_MAPWPT]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiLab3,&Multimap_Labels[MP_MAPASP]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiLab4,&Multimap_Labels[MP_VISUALGLIDE]);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryMultiWpt0,&Multimap_Flags_Waypoints[MP_MOVING]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiWpt1,&Multimap_Flags_Waypoints[MP_MAPTRK]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiWpt2,&Multimap_Flags_Waypoints[MP_MAPWPT]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiWpt3,&Multimap_Flags_Waypoints[MP_MAPASP]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiWpt4,&Multimap_Flags_Waypoints[MP_VISUALGLIDE]);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryMultiOvrT0,&Multimap_Flags_Overlays_Text[MP_MOVING]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiOvrT1,&Multimap_Flags_Overlays_Text[MP_MAPTRK]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiOvrT2,&Multimap_Flags_Overlays_Text[MP_MAPWPT]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiOvrT3,&Multimap_Flags_Overlays_Text[MP_MAPASP]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiOvrT4,&Multimap_Flags_Overlays_Text[MP_VISUALGLIDE]);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryMultiOvrG0,&Multimap_Flags_Overlays_Gauges[MP_MOVING]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiOvrG1,&Multimap_Flags_Overlays_Gauges[MP_MAPTRK]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiOvrG2,&Multimap_Flags_Overlays_Gauges[MP_MAPWPT]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiOvrG3,&Multimap_Flags_Overlays_Gauges[MP_MAPASP]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiOvrG4,&Multimap_Flags_Overlays_Gauges[MP_VISUALGLIDE]);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryMultiSizeY1,&Multimap_SizeY[MP_MAPTRK]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiSizeY2,&Multimap_SizeY[MP_MAPWPT]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiSizeY3,&Multimap_SizeY[MP_MAPASP]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultiSizeY4,&Multimap_SizeY[MP_VISUALGLIDE]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultimap1,&Multimap1);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultimap2,&Multimap2);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultimap3,&Multimap3);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMultimap4,&Multimap4);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryMMNorthUp1 ,&MMNorthUp_Runtime[0]);
  if (matchedstring) return;  
  PREAD(sname,svalue,szRegistryMMNorthUp2 ,&MMNorthUp_Runtime[1]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMMNorthUp3 ,&MMNorthUp_Runtime[2]);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryMMNorthUp4 ,&MMNorthUp_Runtime[3]);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryAspPermanent   , &AspPermanentChanged);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryFlarmDirection , &iFlarmDirection    );
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryDrawTask       , &Flags_DrawTask     );
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryDrawFAI        , &Flags_DrawFAI      );
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryGearMode       , &GearWarningMode    );
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryGearAltitude   , &GearWarningAltitude);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryBottomMode     , &BottomMode);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryBigFAIThreshold, &FAI28_45Threshold);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistrySonarWarning,&SonarWarning_Config);
  if (matchedstring) return;

  #if SAVESCREEN
  if(!IsEmbedded()) {
    extern bool CommandResolution;
    // Do NOT load resolution from profile, if we have requested a resolution from command line
    // And also, we can only load saved screen parameters from the default profile!
    // It does not work from custom profiles.
    if (!CommandResolution && isDefaultProfile) {
        PREAD(sname,svalue,szRegistryScreenSize, &ScreenSize);
        if (matchedstring) return;
        PREAD(sname,svalue,szRegistryScreenSizeX, &ScreenSizeX);
        if (matchedstring) {
            return;
        }
        PREAD(sname,svalue,szRegistryScreenSizeY, &ScreenSizeY);
        if (matchedstring) {
            return;
        }
    }
  }
  #endif

  #if TESTBENCH
  if (!strcmp(sname,"LKVERSION") && !strcmp(sname,"PROFILEVERSION")) {
      StartupStore(_T("... UNMANAGED PARAMETER inside profile: <%s>=<%s>\n"),sname,svalue);
  }
  #endif

  return;

}


void ReadDeviceSettings(const int devIdx, TCHAR *Name){
  Name[0] = '\0';
  if (devIdx == 0) _tcscpy(Name,dwDeviceName1);
  if (devIdx == 1) _tcscpy(Name,dwDeviceName2);
  if (_tcslen(Name)==0) _tcscpy(Name,_T(DEV_DISABLED_NAME));
}

void ReadPort1Settings(LPTSTR szPort, DWORD *SpeedIndex, DWORD *Bit1Index) {
    if (szPort) {
        _tcscpy(szPort, szPort1);
    }
    if (SpeedIndex) {
        *SpeedIndex = dwSpeedIndex1;
    }
    if (Bit1Index) {
        *Bit1Index = dwBit1Index;
    }
}

void ReadPort2Settings(LPTSTR szPort, DWORD *SpeedIndex, DWORD *Bit1Index) {
    if (szPort) {
        _tcscpy(szPort, szPort2);
    }
    if (SpeedIndex) {
        *SpeedIndex = dwSpeedIndex2;
    }
    if (Bit1Index) {
        *Bit1Index = dwBit2Index;
    }
}


