/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "McReady.h"
#include "Modeltype.h"
#include "LKProfiles.h"
#include "utils/stringext.h"
#include "Asset.hpp"
#include "Screen/Init.hpp"

extern long  StrTol(const  TCHAR *buff) ;
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
#else
  strncpy(lookupvalue, curvalue, size);
#endif
  #if DEBUGPROF
  StartupStore(_T(".... PREAD curname=<%s> curvalue=<%s> lookupname=<%s> tchar=<%s>\n"),
  curname,curvalue,lookupname,lookupvalue);
  #endif
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

  PREAD(sname,svalue,szRegistryAircraftCategory, &AircraftCategory);
  PREAD(sname,svalue,szRegistryAircraftRego, &*AircraftRego_Config, std::size(AircraftRego_Config));
  PREAD(sname,svalue,szRegistryAircraftType, &*AircraftType_Config, std::size(AircraftType_Config));
  if (matchedstring) {
      return;
  }

  PREAD(sname,svalue,szRegistryAirfieldFile, &*szAirfieldFile, std::size(szAirfieldFile));
  if (matchedstring) {
    RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szAirfieldFile);
    RemoveFilePathPrefix(_T(LKD_WAYPOINTS), szAirfieldFile);
    return;
  }

  for(unsigned int i = 0; i < NO_AS_FILES; i++)
  {
    PREAD(sname,svalue,szRegistryAirspaceFile[i], &*szAirspaceFile[i], std::size(szAirspaceFile[i]));
    if (matchedstring) { // every 10 or so PREADs we check for quick return
      RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szAirspaceFile[i]);
      RemoveFilePathPrefix(_T(LKD_AIRSPACES), szAirspaceFile[i]);
      return;
    }
  }

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

  if (!strcmp(szRegistryAppIndLandable,sname)) {
	ival=strtol(svalue, NULL, 10);
	Appearance.IndLandable = (IndLandable_t)ival;
	return;
  }

  if (!strcmp( szRegistryUTF8Symbolsl  ,sname)) {
        ival=strtol(svalue, NULL, 10);
        Appearance.UTF8Pictorials = (IndLandable_t)ival;
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
  PREAD(sname,svalue,szRegistryBit1Index,&dwBitIndex[0]);
  PREAD(sname,svalue,szRegistryBit2Index,&dwBitIndex[1]);
  PREAD(sname,svalue,szRegistryBit3Index,&dwBitIndex[2]);
  PREAD(sname,svalue,szRegistryBit4Index,&dwBitIndex[3]);
  PREAD(sname,svalue,szRegistryBit5Index,&dwBitIndex[4]);
  PREAD(sname,svalue,szRegistryBit6Index,&dwBitIndex[5]);
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
  PREAD(sname,svalue,szRegistryCompetitionClass,&*CompetitionClass_Config, std::size(CompetitionClass_Config));
  PREAD(sname,svalue,szRegistryCompetitionID,&*CompetitionID_Config, std::size(CompetitionID_Config));
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
  PREAD(sname,svalue,szRegistryDeviceA,&dwDeviceName[0][0], std::size(dwDeviceName[0]));
  PREAD(sname,svalue,szRegistryDeviceB,&dwDeviceName[1][0], std::size(dwDeviceName[1]));
  PREAD(sname,svalue,szRegistryDeviceC,&dwDeviceName[2][0], std::size(dwDeviceName[2]));
  PREAD(sname,svalue,szRegistryDeviceD,&dwDeviceName[3][0], std::size(dwDeviceName[3]));
  PREAD(sname,svalue,szRegistryDeviceE,&dwDeviceName[4][0], std::size(dwDeviceName[4]));
  PREAD(sname,svalue,szRegistryDeviceF,&dwDeviceName[5][0], std::size(dwDeviceName[5]));
  PREAD(sname,svalue,szRegistryDisableAutoLogger,&DisableAutoLogger);
  PREAD(sname,svalue,szRegistryLiveTrackerInterval,&LiveTrackerInterval);
  PREAD(sname,svalue,szRegistryLiveTrackerRadar_config,&LiveTrackerRadar_config);
  PREAD(sname,svalue,szRegistryLiveTrackerStart_config,&LiveTrackerStart_config);
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
  PREAD(sname,svalue,szRegistryFontVisualGlide,&FontVisualGlide);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryGlideBarMode,&GlideBarMode);
  PREAD(sname,svalue,szRegistryGliderScreenPosition,&MapWindow::GliderScreenPosition);
  PREAD(sname,svalue,szRegistryGpsAltitudeOffset,&GPSAltitudeOffset);
  PREAD(sname,svalue,szRegistryHandicap,&Handicap);
  PREAD(sname,svalue,szRegistryHideUnits,&HideUnits);
  PREAD(sname,svalue,szRegistryHomeWaypoint,&HomeWaypoint);
  PREAD(sname,svalue,szRegistryDeclTakeOffLanding,&DeclTakeoffLanding);

  // InfoType
  for (int i=0;i<MAXINFOWINDOWS;i++) {
        PREAD(sname,svalue,&*szRegistryDisplayType[i], &InfoType[i]);
	if (matchedstring) return;
  }
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryInputFile,&*szInputFile, std::size(szInputFile));
  if (matchedstring) { // every 10 or so PREADs we check for quick return
    RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szInputFile);
    RemoveFilePathPrefix(_T(LKD_CONF), szInputFile);
    return;
  }


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
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryLanguageCode,&*szLanguageCode, std::size(szLanguageCode));
  if (matchedstring) {
    return;
  }

  if (!strcmp(szRegistryLatLonUnits,sname)) {
	ival=strtol(svalue, NULL, 10);
	Units::CoordinateFormat = (CoordinateFormats_t)ival;
	return;
  }

  PREAD(sname,svalue,szRegistryLiftUnitsValue,&LiftUnit_Config );
  PREAD(sname,svalue,szRegistryLockSettingsInFlight,&LockSettingsInFlight);
  PREAD(sname,svalue,szRegistryLoggerShort,&LoggerShortName);
  PREAD(sname,svalue,szRegistryMapBox,&MapBox);
  if (matchedstring) {
    return;
  }
  
  PREAD(sname,svalue,szRegistryMapFile,&*szMapFile, std::size(szMapFile));
  if (matchedstring) {
    RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szMapFile);
    RemoveFilePathPrefix(_T(LKD_MAPS), szMapFile);
    return;
  }


  PREAD(sname,svalue,szRegistryMenuTimeout,&MenuTimeout_Config);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryNewMapDeclutter,&NewMapDeclutter);
  PREAD(sname,svalue,szRegistryOrbiter,&Orbiter_Config);
  PREAD(sname,svalue,szRegistryOutlinedTp,&OutlinedTp_Config);
  PREAD(sname,svalue,szRegistryOverColor,&OverColor);
  PREAD(sname,svalue,szRegistryOverlayClock,&OverlayClock);
  PREAD(sname,svalue,szRegistryUseTwoLines,&UseTwoLines);
  PREAD(sname,svalue,szRegistryOverlaySize,&OverlaySize);
  PREAD(sname,svalue,szRegistryAutoZoomThreshold,&AutoZoomThreshold);
  PREAD(sname,svalue,szRegistryClimbZoom,&ClimbZoom);
  PREAD(sname,svalue,szRegistryCruiseZoom,&CruiseZoom);
  PREAD(sname,svalue,szRegistryMaxAutoZoom,&MaxAutoZoom);
  // PREAD(sname,svalue,szRegistryPGGateIntervalTime,&PGGateIntervalTime);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryTskOptimizeRoute,&TskOptimizeRoute_Config);
  PREAD(sname,svalue,szRegistryGliderSymbol,&GliderSymbol);


 // PREAD(sname,svalue,szRegistryPGStartOut,&PGStartOut);
  PREAD(sname,svalue,szRegistryPilotName,&*PilotName_Config, std::size(PilotName_Config));
  PREAD(sname,svalue,szRegistryLiveTrackersrv,&*LiveTrackersrv_Config, std::size(LiveTrackersrv_Config));
  PREAD(sname,svalue,szRegistryLiveTrackerport,&LiveTrackerport_Config);
  PREAD(sname,svalue,szRegistryLiveTrackerusr,&*LiveTrackerusr_Config, std::size(LiveTrackerusr_Config));
  PREAD(sname,svalue,szRegistryLiveTrackerpwd,&*LiveTrackerpwd_Config, std::size(LiveTrackerpwd_Config));
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryPolarFile,&*szPolarFile, std::size(szPolarFile));
  if (matchedstring) {
    RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szPolarFile);
    RemoveFilePathPrefix(_T(LKD_POLARS), szPolarFile);
    RemoveFilePathPrefix(_T(LKD_SYS_POLAR), szPolarFile);
    return;
  }

  PREAD(sname,svalue,szRegistryPollingMode,&PollingMode);
  if (matchedstring) return;

  /***************************************************/
  /* for compatibilty with old file                  */
  unsigned dwIdxPort;
  PREAD(sname,svalue,szRegistryPort1Index,&dwIdxPort);
    if(matchedstring) {
#ifdef ANDROID
      ScopeLock lock(COMMPort_mutex);
#endif

      if(COMMPort.size() == 0) {
            RefreshComPortList();
        }
        if(dwIdxPort < COMMPort.size()) {
          _tcscpy(szPort[0], COMMPort[dwIdxPort].GetName());
        }
        return;
    }
  
  PREAD(sname,svalue,szRegistryPort2Index,&dwIdxPort);
    if(matchedstring) {
#ifdef ANDROID
      ScopeLock lock(COMMPort_mutex);
#endif
      if(COMMPort.size() == 0) {
            RefreshComPortList();
        }
        if(dwIdxPort < COMMPort.size()) {
          _tcscpy(szPort[1], COMMPort[dwIdxPort].GetName());
        }
        return;
    }
  /***************************************************/
  PREAD(sname,svalue,szRegistryPort1Name ,szPort[0]     , std::size(szPort[0])     );
  PREAD(sname,svalue,szRegistryPort2Name ,szPort[1]     , std::size(szPort[1])     );
  PREAD(sname,svalue,szRegistryPort3Name ,szPort[2]     , std::size(szPort[2])     );
  PREAD(sname,svalue,szRegistryPort4Name ,szPort[3]     , std::size(szPort[3])     );
  PREAD(sname,svalue,szRegistryPort5Name ,szPort[4]     , std::size(szPort[4])     );
  PREAD(sname,svalue,szRegistryPort6Name ,szPort[5]     , std::size(szPort[5])     );

  PREAD(sname,svalue,szRegistryIpAddress1,szIpAddress[0], std::size(szIpAddress[0]));
  PREAD(sname,svalue,szRegistryIpAddress2,szIpAddress[1], std::size(szIpAddress[1]));
  PREAD(sname,svalue,szRegistryIpAddress3,szIpAddress[2], std::size(szIpAddress[2]));
  PREAD(sname,svalue,szRegistryIpAddress4,szIpAddress[3], std::size(szIpAddress[3]));
  PREAD(sname,svalue,szRegistryIpAddress5,szIpAddress[4], std::size(szIpAddress[4]));
  PREAD(sname,svalue,szRegistryIpAddress6,szIpAddress[5], std::size(szIpAddress[5]));

  PREAD(sname,svalue,szRegistryIpPort1,&dwIpPort[0]);
  PREAD(sname,svalue,szRegistryIpPort2,&dwIpPort[1]);
  PREAD(sname,svalue,szRegistryIpPort3,&dwIpPort[2]);
  PREAD(sname,svalue,szRegistryIpPort4,&dwIpPort[3]);
  PREAD(sname,svalue,szRegistryIpPort5,&dwIpPort[4]);
  PREAD(sname,svalue,szRegistryIpPort6,&dwIpPort[5]);



#define IO_PARAM_SIZE 160
  for(int n = 0; n < NUMDEV; n++)
  {
    int i =0;

    TCHAR szTmp[IO_PARAM_SIZE] = _T("");
    TCHAR szItem[10];

    char szKey[20] = ("");
     sprintf(szKey , "%s%u",szRegistryIOValues,n+1);
    PREAD(sname,svalue,szKey, szTmp, IO_PARAM_SIZE);
    if (matchedstring)
    {
      if(_tcslen(szTmp) > 0)
      {
    //    StartupStore(TEXT(" Load : szRegistryIOValues[%u] (%s) %s  %s"), n, szKey, szTmp, NEWLINE);
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].MCDir     = (DataBiIoDir) StrTol(szItem);
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].BUGDir    = (DataBiIoDir) StrTol(szItem); // (String, NULL, 0)
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].BALDir    = (DataBiIoDir) StrTol(szItem);

	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].STFDir    = (DataBiIoDir) StrTol(szItem);
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].WINDDir   = (DataBiIoDir) StrTol(szItem);
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].BARODir   = (DataBiIoDir) StrTol(szItem);

	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].VARIODir  = (DataBiIoDir) StrTol(szItem);
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].SPEEDDir  = (DataBiIoDir) StrTol(szItem);
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].R_TRGTDir = (DataTP_Type) StrTol(szItem);

	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].RADIODir  = (DataBiIoDir) StrTol(szItem);
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].TRAFDir   = (DataBiIoDir) StrTol(szItem);
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].GYRODir   = (DataBiIoDir) StrTol(szItem);

	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].GFORCEDir = (DataBiIoDir) StrTol(szItem);
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].OATDir    = (DataBiIoDir) StrTol(szItem);
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].BAT1Dir   = (DataBiIoDir) StrTol(szItem);

	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].BAT2Dir   = (DataBiIoDir) StrTol(szItem);
	NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].POLARDir  = (DataBiIoDir) StrTol(szItem);
  NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].DirLink   = (DataBiIoDir) StrTol(szItem);
  NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].T_TRGTDir = (DataTP_Type) StrTol(szItem);
    NMEAParser::ExtractParameter(szTmp,szItem,i++); PortIO[n].QNHDir    = (DataBiIoDir) StrTol(szItem);


      }
      return;
    }
  }


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

#if defined(PPC2003) || defined(PNA)
  PREAD(sname,svalue,szRegistrySetSystemTimeFromGPS,&SetSystemTimeFromGPS);
#endif

  PREAD(sname,svalue,szRegistrySaveRuntime,&SaveRuntime);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryShading,&Shading_Config);
  PREAD(sname,svalue,szRegistryIsoLine,&IsoLine_Config);
  PREAD(sname,svalue,szRegistrySnailTrail,&TrailActive_Config);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistrySnailScale,&SnailScale);
  PREAD(sname,svalue,szRegistrySpeed1Index,&dwSpeedIndex[0]);
  PREAD(sname,svalue,szRegistrySpeed2Index,&dwSpeedIndex[1]);
  PREAD(sname,svalue,szRegistrySpeed3Index,&dwSpeedIndex[2]);
  PREAD(sname,svalue,szRegistrySpeed4Index,&dwSpeedIndex[3]);
  PREAD(sname,svalue,szRegistrySpeed5Index,&dwSpeedIndex[4]);
  PREAD(sname,svalue,szRegistrySpeed6Index,&dwSpeedIndex[5]);
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
  if (matchedstring) {
    return;
  }
  PREAD(sname,svalue,szRegistryTerrainFile,szTerrainFile, std::size(szTerrainFile));
  if (matchedstring) {
    RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szTerrainFile);
    RemoveFilePathPrefix(_T(LKD_MAPS), szTerrainFile);
    return;
  }

  PREAD(sname,svalue,szRegistryTerrainRamp,&TerrainRamp_Config);
  if (matchedstring) return; // do not remove
  PREAD(sname,svalue,szRegistryTerrainWhiteness,&TerrainWhiteness);
  // we must be sure we are changing TerrainWhiteness only when loaded from profile
  // otherwise we would get 0.01  (1/100), black screen..
  if (matchedstring) {; TerrainWhiteness/=100; return;}  
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
  PREAD(sname,svalue,szRegistryUseExtSound1,&UseExtSound[0]);
  PREAD(sname,svalue,szRegistryUseExtSound2,&UseExtSound[1]);
  PREAD(sname,svalue,szRegistryUseExtSound3,&UseExtSound[2]);
  PREAD(sname,svalue,szRegistryUseExtSound4,&UseExtSound[3]);
  PREAD(sname,svalue,szRegistryUseExtSound5,&UseExtSound[4]);
  PREAD(sname,svalue,szRegistryUseExtSound6,&UseExtSound[5]);

  PREAD(sname,svalue,szRegistryUseUngestures,&UseUngestures);
  PREAD(sname,svalue,szRegistryUseTotalEnergy,&UseTotalEnergy_Config);
  PREAD(sname,svalue,szRegistryWarningTime,&WarningTime);
  if (matchedstring) return;

  for(unsigned int i = 0; i < NO_WP_FILES; i++)
  {
    PREAD(sname,svalue,szRegistryWayPointFile[i],szWaypointFile[i], std::size(szWaypointFile[i]));
    if (matchedstring) {
      RemoveFilePathPrefix(_T("%LOCAL_PATH%"), szWaypointFile[i]);
      RemoveFilePathPrefix(_T(LKD_WAYPOINTS), szWaypointFile[i]);
      return;
    }
  }

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
  PREAD(sname,svalue,szRegistryMultimap5,&Multimap5);
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
//  PREAD(sname,svalue,szRegistryDrawTask       , &Flags_DrawTask     );  // DrawTask not any more a runtime save option . v7.1
//  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryDrawFAI        , &Flags_DrawFAI_config      );
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryGearMode       , &GearWarningMode    );
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryGearAltitude   , &GearWarningAltitude);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryBottomMode     , &BottomMode);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryBigFAIThreshold, &FAI28_45Threshold);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryDrawXC    , &Flags_DrawXC_config);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistrySonarWarning,&SonarWarning_Config);
  if (matchedstring) return;

  PREAD(sname,svalue,szRegistryOverlay_TopLeft,&Overlay_TopLeft);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryOverlay_TopMid,&Overlay_TopMid);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryOverlay_TopRight,&Overlay_TopRight);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryOverlay_TopDown,&Overlay_TopDown);
  if (matchedstring) return;
  PREAD(sname,svalue,szRegistryOverlay_LeftTop,&Overlay_LeftTop);
  if (matchedstring) {
    if ( Overlay_LeftTop == 2 ) { // old Customs
      Overlay_LeftTop = GetInfoboxType(4)+1000;
    }
    return;
  }
  PREAD(sname,svalue,szRegistryOverlay_LeftMid,&Overlay_LeftMid);
  if (matchedstring) {
    if ( Overlay_LeftMid == 2 ) { // old Customs
      Overlay_LeftMid = GetInfoboxType(5)+1000;
    }
    return;
  }
  PREAD(sname,svalue,szRegistryOverlay_LeftBottom,&Overlay_LeftBottom);
  if (matchedstring) {
    if ( Overlay_LeftBottom == 2 ) { // old Customs
      Overlay_LeftBottom = GetInfoboxType(6)+1000;
    }
    return;
  }
  PREAD(sname,svalue,szRegistryOverlay_LeftDown,&Overlay_LeftDown);
  if (matchedstring) {
    if ( Overlay_LeftDown == 2 ) { // old Customs
      Overlay_LeftDown = GetInfoboxType(7)+1000;
    }
    return;
  }
  PREAD(sname,svalue,szRegistryOverlay_RightTop,&Overlay_RightTop);
  if (matchedstring) {
    if ( Overlay_RightTop == 2 ) { // old Customs
      Overlay_RightTop = GetInfoboxType(1)+1000;
    }
    return;
  }
  PREAD(sname,svalue,szRegistryOverlay_RightMid,&Overlay_RightMid);
  if (matchedstring) {
    if ( Overlay_RightMid == 2 ) { // old Customs
      Overlay_RightMid = GetInfoboxType(2)+1000;
    }
    return;
  }
  PREAD(sname,svalue,szRegistryOverlay_RightBottom,&Overlay_RightBottom);
  if (matchedstring) {
    if ( Overlay_RightBottom == 2 ) { // old Customs
      Overlay_RightBottom = GetInfoboxType(3)+1000;
    }
    return;
  }
  PREAD(sname,svalue,szRegistryAdditionalContestRule,&AdditionalContestRule);
  if (matchedstring) return;

#ifdef _WGS84
  PREAD(sname,svalue,szRegistry_earth_model_wgs84,&earth_model_wgs84);
  if (matchedstring) return;
#endif
  PREAD(sname,svalue,szRegistryAutoContrast,&AutoContrast);
  if (matchedstring) return;


  if (SaveRuntime) if(!IsEmbedded()) {
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

  if(!SaveRuntime) {
    EnableSoundModes =true;
  }
  else
  {
    PREAD(sname,svalue,szRegistrySoundSwitch,&EnableSoundModes);
    if (matchedstring) {
     return;
    }
  }

  #if TESTBENCH
  if (!strcmp(sname,"LKVERSION") && !strcmp(sname,"PROFILEVERSION")) {
      StartupStore(_T("... UNMANAGED PARAMETER inside profile: <%s>=<%s>\n"),sname,svalue);
  }
  #endif

  return;

}


void ReadDeviceSettings(const int devIdx, TCHAR *Name){
  Name[0] = '\0';
  if (devIdx >=  0)
    if (devIdx <  NUMDEV)
      _tcscpy(Name,dwDeviceName[devIdx]);

  if (_tcslen(Name)==0) _tcscpy(Name,_T(DEV_DISABLED_NAME));
}




void ReadPortSettings( int devIdx,LPTSTR szPort_n, unsigned *SpeedIndex, BitIndex_t *Bit1Index) {
  if (devIdx < 0)  devIdx =0;
  if (devIdx >= NUMDEV) devIdx =NUMDEV-1;

    if (szPort_n) {
        _tcscpy(szPort_n, &szPort[devIdx][0]);
    }
    if (SpeedIndex) {
        *SpeedIndex = dwSpeedIndex[devIdx];
    }
    if (Bit1Index) {
        *Bit1Index = static_cast<BitIndex_t>(dwBitIndex[devIdx]);
    }
}
