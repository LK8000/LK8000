/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "DoInits.h"
#include "Asset.hpp"


#ifdef ANDROID
#include <android/log.h>
#include <unistd.h>
#include <sys/stat.h>
#include <Android/Main.hpp>
#include "Android/Environment.hpp"
#include "Android/NativeView.hpp"

/**
 * Determine whether a text file contains a given string
 *
 * If two strings are given, the second string is considered
 * as no-match for the given line (i.e. string1 AND !string2).
 */
static bool
fgrep(const char *fname, const char *string, const char *string2 = nullptr)
{
    char line[100];
    FILE *fp;

    if ((fp = fopen(fname, "r")) == nullptr)
        return false;
    while (fgets(line, sizeof(line), fp) != nullptr)
        if (strstr(line, string) != nullptr &&
            (string2 == nullptr || strstr(line, string2) == nullptr)) {
            fclose(fp);
            return true;
        }
    fclose(fp);
    return false;
}

/**
 * The default mount point of the SD card on Android.
 */
#define ANDROID_SDCARD "/sdcard"

/**
 * On the Samsung Galaxy Tab, the "external" SD card is mounted here.
 * Shame on the Samsung engineers, they didn't implement
 * Environment.getExternalStorageDirectory() properly.
 */
#define ANDROID_SAMSUNG_EXTERNAL_SD "/sdcard/external_sd"

/**
 * This is the partition that the Kobo software mounts on PCs
 */
#define KOBO_USER_DATA "/mnt/onboard"


static
void getExternalStoragePublicDirectory(TCHAR* szPath, size_t MaxSize) {
    /* on Samsung Galaxy S4 (and others), the "external" SD card is
       mounted here */
    if (lk::filesystem::isDirectory(_T("/mnt/extSdCard" LKDATADIR))
        && access(_T("/mnt/extSdCard" LKDATADIR),W_OK) == 0) {
        /* found writable XCSoarData: use this SD card */
        _tcsncpy(szPath, _T("/mnt/extSdCard"), MaxSize);
        return;
    }

    /* hack for Samsung Galaxy S and Samsung Galaxy Tab (which has a
       built-in and an external SD card) */
    struct stat st;
    if (stat(ANDROID_SAMSUNG_EXTERNAL_SD, &st) == 0 &&
        S_ISDIR(st.st_mode) &&
        fgrep("/proc/mounts", ANDROID_SAMSUNG_EXTERNAL_SD " ", "tmpfs ")) {

        __android_log_print(ANDROID_LOG_DEBUG, "LK8000", "Enable Samsung hack : " ANDROID_SAMSUNG_EXTERNAL_SD);

        _tcsncpy(szPath, _T(ANDROID_SAMSUNG_EXTERNAL_SD), MaxSize);
        return;
    }

    /* try Context.getExternalStoragePublicDirectory() */
    if (Environment::getExternalStoragePublicDirectory(szPath, MaxSize, LKDATADIR) != nullptr) {
        TCHAR* szEnd = _tcsrchr(szPath, '/');
        if(szEnd) {
            *szEnd = _T('\0');
        }
        __android_log_print(ANDROID_LOG_DEBUG, "LK8000", "Environment.getExternalStoragePublicDirectory()='%s'", szPath);
        return;
    }

    /* now try Context.getExternalStorageDirectory(), because
       getExternalStoragePublicDirectory() needs API level 8 */
    if (Environment::getExternalStorageDirectory(szPath, MaxSize - 32) != nullptr) {
        return;
    }

    /* hard-coded path for Android */
    __android_log_print(ANDROID_LOG_DEBUG, "L8000", "Fallback : " ANDROID_SDCARD);

    _tcsncpy(szPath, _T(ANDROID_SDCARD), MaxSize);
}


#elif !defined(KOBO) && !defined(OPENVARIO)
// return Path including trailing directory separator.
static
const TCHAR * LKGetPath(TCHAR *localpath, TCHAR const *fileToSearch) {

   if (lk::filesystem::getExePath(localpath, MAX_PATH)) {
        size_t nLen = _tcslen(localpath);
        _tcscat(localpath, fileToSearch);
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
        _tcscat(localpath, fileToSearch);
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
        _tcscat(localpath, fileToSearch);
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
#endif

const TCHAR * LKGetSystemPath(void) {
#if defined(KOBO) || defined(OPENVARIO)
    return _T("/opt/" LKDATADIR "/share/");
#elif defined(ANDROID)
    // we use assets stored in apk, so, system directory is apk file....

    static TCHAR szPakagePath[MAX_PATH] = {0};
    if(szPakagePath[0] == '\0') {

        native_view->getPackagePath(szPakagePath, sizeof(szPakagePath));
        strcat(szPakagePath, "/");
    }
    return szPakagePath;

#else
    return LKGetLocalPath();
#endif
}

const TCHAR * LKGetLocalPath(void) {
#ifdef KOBO
    return _T("/mnt/onboard/" LKDATADIR "/");
#elif defined(OPENVARIO)
    return _T("/home/root/" LKDATADIR "/");
#else

    static TCHAR localpath[MAX_PATH + 1] = {0};

    if (!DoInit[MDI_GETLOCALPATH]) {
        return localpath;
    }

    DoInit[MDI_GETLOCALPATH] = false;

#ifdef ANDROID

    getExternalStoragePublicDirectory(localpath, sizeof(localpath));
    _tcscat(localpath, _T( "/" LKDATADIR "/"));
    lk::filesystem::createDirectory(localpath);
    return localpath;

#else

    const TCHAR *fileToSearch = _T(LKD_LANGUAGE"\\en.json");
    return LKGetPath(localpath, fileToSearch);

#endif // ! ANDROID
#endif
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

static gcc_nonnull_all
void GetPath(TCHAR* buffer, const TCHAR* subpath, const TCHAR* file, const TCHAR* lkPath) {
  // remove leading directory separator from file.
  const TCHAR* pfile = file;
  while( (*pfile) == _T('\\') && (*pfile) == _T('/') ) {
      ++pfile;
  }

  // remove leading directory separator from subpath.
  const TCHAR* psub = subpath;
  while( (*psub) == _T('\\') || (*psub) == _T('/')) {
    ++psub;
  }

  _tcscpy(buffer,lkPath);

  if( _tcslen(psub) > 0) {
      _tcscat(buffer,psub);
      // remove trailing directory separator;
      size_t len = _tcslen(buffer);
      while(len > 1 && ( buffer[len-1] == _T('\\') || buffer[len-1] == _T('/'))) {
          buffer[--len] = _T('\0');
      }
      // add right (platform depends) trailing separator
      _tcscat(buffer, _T(DIRSEP));
  }

  if (_tcslen(pfile)>0) {
      _tcscat(buffer,pfile);
  }

  lk::filesystem::fixPath(buffer);
}

void SystemPath(TCHAR* buffer, const TCHAR* SubPath, const TCHAR* file) {
    GetPath(buffer, SubPath, file,  LKGetSystemPath());
}

void LocalPath(TCHAR* buffer, const TCHAR* SubPath, const TCHAR* file) {
    GetPath(buffer, SubPath, file, LKGetLocalPath());
}

void SystemPath(TCHAR* buffer, const TCHAR* file) {
    GetPath(buffer, _T(""), file, LKGetSystemPath());
}

void LocalPath(TCHAR* buffer, const TCHAR* file) {
    GetPath(buffer, _T(""), file, LKGetLocalPath());
}

/**
 * * some file are loaded from LocalPath or SystemPath
 *   ( in v6 Polar and language )
 *
 *  this function remove Prefix from path.
 *  required for compatibility with old config file
 */
void RemoveFilePathPrefix(const TCHAR* szPrefix, TCHAR* szFilePath) {
    const TCHAR* ptr = _tcsstr(szFilePath, szPrefix);
    if(ptr && szFilePath == ptr) {
        ptr += _tcslen(szPrefix);
    } else {
        ptr = nullptr;
    }

    // remove prefix only if followed by directory separator
    bool found = false;
    while (ptr && ((*ptr) == '\\' || (*ptr) == '/')) {
        ++ptr;
        found = true;
    }
    if(found && ptr) {
        _tcscpy(szFilePath, ptr);
    }
}
