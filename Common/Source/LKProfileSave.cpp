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

  char stmp[256];
  
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


  fprintf(fp,"%S=%d%s", szRegistryLatLonUnits, Units::CoordinateFormat,PNEWLINE);



  // Anything containing non-ascii chars should be treated like this:
  // Unicode UTF8 converted
  static TCHAR  szMapFile[MAX_PATH] = TEXT("\0");
  GetRegistryString(szRegistryMapFile, szMapFile, MAX_PATH);
  ExpandLocalPath(szMapFile);
  unicode2utf((TCHAR*) szMapFile, stmp, sizeof(stmp));
  fprintf(fp,"%S=\"%s\"%s", szRegistryMapFile, stmp ,PNEWLINE);


  fprintf(fp,"\r\n"); // end of file
  fflush(fp);
  fclose(fp);

  #if TESTBENCH
  StartupStore(_T("... SaveProfile <%s> OK%s"),szFile,NEWLINE);
  #endif
}


#endif // NEWPROFILES
