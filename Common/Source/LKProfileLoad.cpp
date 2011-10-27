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

const static int nMaxValueNameSize = MAX_PATH + 6;	//255 + 1 + /r/n
const static int nMaxValueValueSize = MAX_PATH*2 + 6;	// max regkey name is 256 chars + " = "
const static int nMaxClassSize = MAX_PATH + 6;
const static int nMaxKeyNameSize = MAX_PATH + 6;


//
// Returns true if at least one value was found,
// excluded comments and empty lines
//
bool LKProfileLoad(TCHAR *szFile)
{
  #if TESTBENCH
  StartupStore(_T(".... LoadProfile <%s>%s"),szFile,NEWLINE);
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
  }

  // if using mingw and nothing found in utf8 file, try with wide chars
  #ifdef __MINGW32__
  if (!found) goto parse_wide;
  #endif

go_return:

  fclose(fp);
  return found;
}




void LKParseProfileString(TCHAR *sname, TCHAR *svalue) {

  StartupStore(_T("... Parse: <%s> = <%s>\n"),sname,svalue);


}


#endif // NEWPROFILES
