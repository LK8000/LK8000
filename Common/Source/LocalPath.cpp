/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "DoInits.h"

// return Path including trailing directory separator.
const TCHAR * LKGetLocalPath(void) {

    if(IsKobo()) {
        return "/mnt/onboard/" LKDATADIR "/";
    }
    
    static TCHAR localpath[MAX_PATH + 1] = {0};

    if (!DoInit[MDI_GETLOCALPATH]) {
        return localpath;
    }

    DoInit[MDI_GETLOCALPATH] = false;

    if (lk::filesystem::getExePath(localpath, MAX_PATH)) {
        size_t nLen = _tcslen(localpath);
        _tcscat(localpath, _T(LKD_SYSTEM"\\_SYSTEM"));
        lk::filesystem::fixPath(localpath);
        if (lk::filesystem::exist(localpath)) {
            // Yes, so we use the current path folder
            localpath[nLen] = _T('\0');
            return localpath;
        }
    }

    // try to use MyDocuments directory
    localpath[0] = _T('\0');
    if (lk::filesystem::getUserPath(localpath, MAX_PATH)) {
        _tcscat(localpath, TEXT(LKDATADIR"\\"));

        size_t nLen = _tcslen(localpath);
        _tcscat(localpath, _T(LKD_SYSTEM"\\_SYSTEM"));
        lk::filesystem::fixPath(localpath);
        if (lk::filesystem::exist(localpath)) {
            // Yes, so we use the current path folder
            localpath[nLen] = _T('\0');
            return localpath;
        }
    }

    //
    // For PNAs the localpath is taken from the application exec path
    // example> \sdmmc\bin\Program.exe  results in localpath=\sdmmc\LK8000
    // Then the basename is searched for an underscore char, which is
    // used as a separator for getting the model type.  example>
    // program_pna.exe results in GlobalModelType=pna
    localpath[0] = _T('\0');
    if (lk::filesystem::getBasePath(localpath, MAX_PATH)) {
        _tcscat(localpath, TEXT(LKDATADIR"\\"));

        size_t nLen = _tcslen(localpath);
        _tcscat(localpath, _T(LKD_SYSTEM"\\_SYSTEM"));
        lk::filesystem::fixPath(localpath);
        if (lk::filesystem::exist(localpath)) {
            // Yes, so we use the current path folder
            localpath[nLen] = _T('\0');
            return localpath;
        }
    }
    localpath[0] = _T('\0');
    return localpath;
}


#ifdef PNA
#include <shlobj.h>
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


void LocalPath(TCHAR* buffer, const TCHAR* file) {
  // remove leading directory separator from file.
  const TCHAR* ptr2 = file;
  while( (*ptr2) == _T('\\') && (*ptr2) ) {
      ++ptr2;
  }

  if (_tcslen(file)>0)
    _stprintf(buffer,TEXT("%s%s"),LKGetLocalPath(),ptr2);
  else {
    _tcscpy(buffer,LKGetLocalPath());
  }

  lk::filesystem::fixPath(buffer);
}

// This is used by LoadFromXML function only
void LocalPathS(TCHAR *buffer, const TCHAR* file) {

  LocalPath(buffer, TEXT(LKD_DIALOGS));
  TCHAR* ptr = buffer + _tcslen(buffer) -1;
  if(*ptr != _T('\\')) {
      _tcscat(buffer, _T(DIRSEP));
  }

  switch(AircraftCategory) {
	case umGlider:
		_tcscat(buffer,_T("GLIDER\\"));
		break;
	case umParaglider:
		_tcscat(buffer,_T("PARAGLIDER\\"));
		break;
	case umCar:
		_tcscat(buffer,_T("CAR\\"));
		break;
	case umGAaircraft:
		_tcscat(buffer,_T("GA\\"));
		break;
	default:
		break;
  }

  const TCHAR* ptr2 = file;
  while( (*ptr2) == _T('\\') && (*ptr2) ) {
      ++ptr2;
  }

  _tcscat(buffer,ptr2);
  lk::filesystem::fixPath(buffer);  
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

  const TCHAR* ptr = _tcsstr(filein, code);
  if (!ptr) return;

  ptr += _tcslen(code);
   
  const TCHAR* ptr2 = lpath + _tcslen(lpath) -1;

  if( (*ptr2=='/')||(*ptr2=='\\') ) {
    if( ((*ptr=='/')||( *ptr=='\\')) ) {
      ++ptr;
    }
  } else {
    if( (*ptr!='/')||(*ptr!='\\') ) {
      _tcscat(lpath, _T(DIRSEP));
    }
  }

  if (_tcslen(ptr)>0) {
    _stprintf(output,TEXT("%s%s"),lpath, ptr);
    _tcscpy(filein, output);
  }
  lk::filesystem::fixPath(filein);  
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
