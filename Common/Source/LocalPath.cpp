/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"

#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#else
#include "wcecompat/ts_string.h"
#endif

#include "options.h"
#include "Defines.h"
#include "externs.h"
#include "lk8000.h"


#include "utils/heapcheck.h"


#ifdef PNA
BOOL GetFontPath(TCHAR *pPos)
{
  HKEY    hKey;
  DWORD   dwType = REG_SZ;
  DWORD dwSize = MAX_PATH;
  long    hRes;
  unsigned int i;
  for (i=0; i<dwSize; i++) {
    pPos[i]=0;
  }

  pPos[0]= '\0';
  hRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\FontPath"), 0, KEY_READ /*KEY_ALL_ACCESS*/, &hKey);
  if (hRes != ERROR_SUCCESS) {
	RegCloseKey(hKey);
	return FALSE;
  }

  dwSize *= 2;  // BUGFIX 100913 ?? to remove? check

  hRes = RegQueryValueEx(hKey, _T("FontPath"), 0, &dwType, (LPBYTE)pPos, &dwSize);

  RegCloseKey(hKey);
  if (hRes==ERROR_SUCCESS) return TRUE;
  else return FALSE;
}
#endif

//
//	Get local My Documents path - optionally include file to add and location
//	loc = CSIDL_PROGRAMS
//	File system directory that contains the user's program groups (which are also file system directories).
//	CSIDL_PERSONAL               File system directory that serves as a common
//	                             repository for documents.
//	CSIDL_PROGRAM_FILES 0x0026   The program files folder.
//
void LocalPath(TCHAR* buffer, const TCHAR* file, int loc) {

  #if (!defined(WINDOWSPC) || (WINDOWSPC <=0) )

  // For PNAs the localpath is taken from the application exec path
  // example> \sdmmc\bin\Program.exe  results in localpath=\sdmmc\LK8000
  // Then the basename is searched for an underscore char, which is
  // used as a separator for getting the model type.  example>
  // program_pna.exe results in GlobalModelType=pna
  
	_stprintf(buffer,TEXT("%s%S"),gmfpathname(), XCSDATADIR );
  #else
	//
	// Windows PC environment
	//
	// Do we have a valid _System/_SYSTEM locally?
	extern TCHAR *gmfcurrentpath();
	_stprintf(buffer,_T("%s\\%S\\_SYSTEM"),gmfcurrentpath(),LKD_SYSTEM);
	if (  GetFileAttributes(buffer) != 0xffffffff )  {
		// Yes, so we use the current path folder
		_tcscpy(buffer,gmfcurrentpath());
	} else {
		// No, we use MyDocuments directory
		SHGetSpecialFolderPath(hWndMainWindow, buffer, loc, false);

		_tcscat(buffer,TEXT("\\"));
		_tcscat(buffer,TEXT(XCSDATADIR));
	}
  #endif

  if (_tcslen(file)>0) {
	_tcsncat(buffer, TEXT("\\"), MAX_PATH);    
	_tcsncat(buffer, file, MAX_PATH);
  }
}

// This is used by LoadFromXML function only
void LocalPathS(char *buffer, const TCHAR* file, int loc) {
  TCHAR wbuffer[MAX_PATH];

  LocalPath(wbuffer, TEXT(LKD_DIALOGS));
  _tcscat(wbuffer,_T("\\"));

  switch(AircraftCategory) {
	case umGlider:
		_tcscat(wbuffer,L"GLIDER\\");
		break;
	case umParaglider:
		_tcscat(wbuffer,L"PARAGLIDER\\");
		break;
	case umCar:
		_tcscat(wbuffer,L"CAR\\");
		break;
	case umGAaircraft:
		_tcscat(wbuffer,L"GA\\");
		break;
	default:
		break;
  }
  _tcscat(wbuffer,file);

  sprintf(buffer,"%S",wbuffer);

}


void ExpandLocalPath(TCHAR* filein) {
  // Convert %LOCALPATH% to Local Path

  if (_tcslen(filein)==0) {
    return;
  }

  TCHAR lpath[MAX_PATH];
  TCHAR code[] = TEXT("%LOCAL_PATH%\\");
  TCHAR output[MAX_PATH];
  LocalPath(lpath);

  TCHAR* ptr;
  ptr = _tcsstr(filein, code);
  if (!ptr) return;

  ptr += _tcslen(code);
  if (_tcslen(ptr)>0) {
    _stprintf(output,TEXT("%s%s"),lpath, ptr);
    _tcscpy(filein, output);
  }
}


void ContractLocalPath(TCHAR* filein) {
  // Convert Local Path part to %LOCALPATH%

  if (_tcslen(filein)==0) {
    return;
  }

  TCHAR lpath[MAX_PATH];
  TCHAR code[] = TEXT("%LOCAL_PATH%\\");
  TCHAR output[MAX_PATH];
  LocalPath(lpath);

  TCHAR* ptr;
  ptr = _tcsstr(filein, lpath);
  if (!ptr) return;

  ptr += _tcslen(lpath);
  if (_tcslen(ptr)>0) {
    _stprintf(output,TEXT("%s%s"),code, ptr);
    _tcscpy(filein, output);
  }
}


