/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "DoInits.h"
#include <shlobj.h>

extern TCHAR *gmfcurrentpath();


TCHAR * LKGetLocalPath(void) {

  static TCHAR localpath[MAX_PATH+1];

  if (DoInit[MDI_GETLOCALPATH]) {
	#if ((WINDOWSPC <=0) )
	//
	// For PNAs the localpath is taken from the application exec path
	// example> \sdmmc\bin\Program.exe  results in localpath=\sdmmc\LK8000
	// Then the basename is searched for an underscore char, which is
	// used as a separator for getting the model type.  example>
	// program_pna.exe results in GlobalModelType=pna
	//
	_stprintf(localpath,TEXT("%s%S"),gmfpathname(), LKDATADIR );
	#else
	//
	// Windows PC environment
	//
	// Do we have a valid _System/_SYSTEM locally?
	_stprintf(localpath,_T("%s\\%S\\_SYSTEM"),gmfcurrentpath(),LKD_SYSTEM);
	if (  GetFileAttributes(localpath) != 0xffffffff )  {
		// Yes, so we use the current path folder
		_tcscpy(localpath,gmfcurrentpath());
	} else {
		// No, we use MyDocuments directory
		SHGetSpecialFolderPath(hWndMainWindow, localpath, CSIDL_PERSONAL, false);

		_tcscat(localpath,TEXT("\\"));
		_tcscat(localpath,TEXT(LKDATADIR));
	}
	#endif

	DoInit[MDI_GETLOCALPATH]=false;
  }
  return localpath; 
}


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


void LocalPath(TCHAR* buffer, const TCHAR* file) {

  if (_tcslen(file)>0)
	_stprintf(buffer,TEXT("%s\\%s"),LKGetLocalPath(),file);
  else
	_tcscpy(buffer,LKGetLocalPath());
}

// This is used by LoadFromXML function only
void LocalPathS(char *buffer, const TCHAR* file) {
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
  if(*ptr != _T('\\')) {
      _tcscat(lpath, _T("\\"));
  }
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


#define MAXPATHBASENAME MAX_PATH

/*
 * gmfpathname returns the pathname of the current executed program, with leading and trailing slash
 * example:  \sdmmc\   \SD CARD\
 * In case of double slash, it is assumed currently as a single "\" .
 */
TCHAR * gmfpathname ()
{
  static TCHAR gmfpathname_buffer[MAXPATHBASENAME];
  TCHAR  *p; 
  
  if (GetModuleFileName(NULL, gmfpathname_buffer, MAXPATHBASENAME) <= 0) {
    _tcscpy(gmfpathname_buffer,_T("\\ERROR_01\\"));
    goto _out;
  }

  if (gmfpathname_buffer[0] != '\\' ) {
    _tcscpy(gmfpathname_buffer,_T("\\ERROR_02\\"));
    goto _out;
  }
  gmfpathname_buffer[MAXPATHBASENAME-1] = '\0';	// truncate for safety
  
  for (p=gmfpathname_buffer+1; *p != '\0'; p++)
    if ( *p == '\\' ) break;	// search for the very first "\"
  
  if ( *p == '\0') {
    _tcscpy(gmfpathname_buffer,_T("\\ERROR_03\\"));
    goto _out;
  }
  *++p = '\0';

_out:  
  return (TCHAR *) gmfpathname_buffer;
}

/*
 * gmfbasename returns the filename of the current executed program, without leading path.
 * Example:  lk8000.exe
 */
TCHAR * gmfbasename ()
{
  static TCHAR gmfbasename_buffer[MAXPATHBASENAME];
  TCHAR *p, *lp;
  
  if (GetModuleFileName(NULL, gmfbasename_buffer, MAXPATHBASENAME) <= 0) {
    _tcscpy(gmfbasename_buffer,_T("ERROR_04.EXE"));
    lp=gmfbasename_buffer;
    goto _out;
  }
  gmfbasename_buffer[MAXPATHBASENAME-1]='\0';

  if (gmfbasename_buffer[0] != '\\' ) {
    _tcscpy(gmfbasename_buffer,_T("ERROR_05.EXE"));
    lp=gmfbasename_buffer;
    goto _out;
  }
  for (p=gmfbasename_buffer+1, lp=NULL; *p != '\0'; p++)
    {
      if ( *p == '\\' ) {
	lp=++p;
	continue;
      }
    }
_out:
  return  lp;
}

// Windows CE does not have a "current path" concept, and there is no library function to know
// where is the program running. 
// Returns the current execution path like C:\Documents and Settings\username\Desktop
TCHAR * gmfcurrentpath ()
{
  static TCHAR gmfbasename_buffer[MAXPATHBASENAME];
  TCHAR *p, *lp;
  
  if (GetModuleFileName(NULL, gmfbasename_buffer, MAXPATHBASENAME) <= 0) {
    _tcscpy(gmfbasename_buffer,_T("\\ERROR_04"));
    goto _out;
  }
  gmfbasename_buffer[MAXPATHBASENAME-1]='\0';

  for (p=gmfbasename_buffer+1, lp=NULL; *p != '\0'; p++)
    {
      if ( *p == '\\' ) {
	lp=++p;
	continue;
      }
    }

  if (lp!=NULL) {
	*lp=_T('\0');
	*--lp=_T('\0'); // also remove trailing backslash
  }
_out:
  return (TCHAR *) gmfbasename_buffer;
}


/*
 *	A little hack in the executable filename: if it contains an
 *	underscore, then the following chars up to the .exe is
 *	considered a modelname
 *  Returns 0 if failed, 1 if name found
 */
int GetGlobalModelName ()
{
  TCHAR modelname_buffer[MAXPATHBASENAME];
  TCHAR *p, *lp, *np;
  
  _tcscpy(GlobalModelName, _T(""));
  
  if (GetModuleFileName(NULL, modelname_buffer, MAXPATHBASENAME-1) <= 0) {
    StartupStore(TEXT("++++++ CRITIC- GetGlobalFileName returned NULL%s"),NEWLINE); // 091119
    return 0;
  }
  modelname_buffer[MAXPATHBASENAME-1]='\0';

  if (modelname_buffer[0] != '\\' ) {
    StartupStore(TEXT("++++++ CRITIC- GetGlobalFileName starting without a leading backslash%s"),NEWLINE); // 091119
    return 0;
  }
  for (p=modelname_buffer+1, lp=NULL; *p != '\0'; p++) 
    {
      if ( *p == '\\' ) {
	lp=++p;
	continue;
      }
    } // assuming \sd\path\LK8000_pna.exe  we are now at \LK8000..
 
  if (lp == NULL ) return 0;
 
  for (p=lp, np=NULL; *p != '\0'; p++)
    {
      if (*p == '_' ) {
	np=++p;
	break;
      }
    } // assuming lk8000_pna.exe we are now at _pna..
  
  if ( np == NULL ) {
    return 0;
  }
  
  for ( p=np, lp=NULL; *p != '\0'; p++) 
    {
      if (*p == '.' ) {
	lp=p;
	break;
      }
    } // we found the . in pna.exe
  
  if (lp == NULL) return 0;
  *lp='\0'; // we cut .exe
  
  _tcscpy(GlobalModelName, np);
  
  return 1;  // we say ok
  
}

