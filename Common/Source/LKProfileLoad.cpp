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

#define NEWPROFILES 1
#if NEWPROFILES

// Set default registry values
// Open profile, read line, parse line and compare with list

#define DEBUGPROF	1
extern void LKParseProfileString(TCHAR *sname, TCHAR *svalue);


void LKReadStringFromProfile(HANDLE hFile, TCHAR *instring) {
    TCHAR tempFile[MAX_PATH+1];
    ReadString(hFile, MAX_PATH, tempFile);
    tempFile[_tcslen(tempFile)]= 0;
}

int nMaxValueNameSize = MAX_PATH + 6;	//255 + 1 + /r/n
int nMaxValueValueSize = MAX_PATH*2 + 6;	// max regkey name is 256 chars + " = "
int nMaxClassSize = MAX_PATH + 6;
int nMaxKeyNameSize = MAX_PATH + 6;


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

  if(fp == NULL)
	return false;

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
  return found;
}


using std::max;

void LKParseProfileString(TCHAR *sname, TCHAR *svalue) {

  #if TESTBENCH
  StartupStore(_T("... Parse: <%s> = <%s>\n"),sname,svalue);
  #endif

  int ival;

  if (!_tcscmp(szRegistryAcknowledgementTime,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AcknowledgementTime = max(10,ival);
	return;
  }
  if (!_tcscmp(szRegistryActiveMap,sname)) {
	ival=wcstol(svalue, NULL, 10);
	ActiveMap = ival;
	return;
  }

// Todo:
// AdditionalAirspaceFile
// AdditionalWPFile

  if (!_tcscmp(szRegistryAircraftCategory,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AircraftCategory = ival;
	return;
  }

//  fprintf(fp,"%S=%d%s", szRegistryAircraftRego, AircraftRego,PNEWLINE); missing global
//  fprintf(fp,"%S=%d%s", szRegistryAircraftType, AircraftType,PNEWLINE); missing global
//  fprintf(fp,"%S=%d%s", szRegistryAirfieldFile, AirfieldFile,PNEWLINE); missing global

  if (!_tcscmp(szRegistryAirspaceBlackOutline,sname)) {
	ival=wcstol(svalue, NULL, 10);
	MapWindow::bAirspaceBlackOutline = ival;
	return;
  }

  if (!_tcscmp(szRegistryAirspaceWarningDlgTimeout,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AirspaceWarningDlgTimeout = ival;
	return;
  }

  if (!_tcscmp(szRegistryAirspaceWarningMapLabels,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AirspaceWarningMapLabels = ival;
	return;
  }

  if (!_tcscmp(szRegistryAirspaceWarningRepeatTime,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AirspaceWarningRepeatTime = ival;
	return;
  }

  if (!_tcscmp(szRegistryAirspaceWarningVerticalMargin,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AirspaceWarningVerticalMargin = ival;
	return;
  }

  if (!_tcscmp(szRegistryAirspaceWarning,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AIRSPACEWARNINGS = ival;
	return;
  }

  if (!_tcscmp(szRegistryAlarmMaxAltitude1,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AlarmMaxAltitude1 = ival;
	return;
  }

  if (!_tcscmp(szRegistryAlarmMaxAltitude2,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AlarmMaxAltitude2 = ival;
	return;
  }

  if (!_tcscmp(szRegistryAlarmMaxAltitude3,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AlarmMaxAltitude3 = ival;
	return;
  }

  if (!_tcscmp(szRegistryAltMargin,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AltWarningMargin = ival;
	return;
  }

  if (!_tcscmp(szRegistryAltMode,sname)) {
	ival=wcstol(svalue, NULL, 10);
	AltitudeMode = ival;
	return;
  }

  if (!_tcscmp(szRegistryAlternate1,sname)) {
	ival=wcstol(svalue, NULL, 10);
	Alternate1 = ival;
	return;
  }
  if (!_tcscmp(szRegistryAlternate2,sname)) {
	ival=wcstol(svalue, NULL, 10);
	Alternate2 = ival;
	return;
  }




  if (!_tcscmp(szRegistryLatLonUnits,sname)) {
	ival=wcstol(svalue, NULL, 10);
	Units::CoordinateFormat = (CoordinateFormats_t)ival;
	return;
  }

// Todo:
// AdditionalAirspaceFile
// AdditionalWPFile





}


#endif // NEWPROFILES
