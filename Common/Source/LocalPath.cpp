/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LocalPath.h"
#include "DoInits.h"
#include "Asset.hpp"
#include "utils/printf.h"


#ifdef ANDROID

#include "Android/Main.hpp"
#include "Android/NativeView.hpp"
#include "Android/Context.hpp"
#include <android/log.h>


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
    const tstring path = context->GetExternalFilesDir(Java::GetEnv()).ToString();
    if (!path.empty()) {
        _tcscpy(localpath, path.c_str());
        _tcscat(localpath, _T("/"));
        lk::filesystem::createDirectory(localpath);
        return localpath;
    }

    /**
     * fallback to the default mount point of the SD card on Android.
     */
    __android_log_print(ANDROID_LOG_DEBUG, "L8000", "Fallback : " "/sdcard");
    _tcscpy(localpath, _T("/sdcard"));
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

namespace {

bool IsDirectorySeparator(TCHAR c) {
  return (c == _T('\\')) || (c == _T('/'));
}

const TCHAR* RemoveLeadingSeparator(const TCHAR* path) {
  while (IsDirectorySeparator(*path)) {
    ++path;
  }
  return path;
}

} // namespace

static gcc_nonnull_all
void GetPath(TCHAR* buffer, size_t size, const TCHAR* subpath, const TCHAR* file, const TCHAR* lkPath) {
  // remove leading directory separator from file and subpath.
  const TCHAR* pfile = RemoveLeadingSeparator(file);
  const TCHAR* psub = RemoveLeadingSeparator(subpath);

  size_t len = lk::snprintf(buffer, size, _T("%s"), lkPath);
  while (IsDirectorySeparator(buffer[len - 1])) {
    --len;
  }
  len += lk::snprintf(buffer + len, size - len, _T("%s"), _T(DIRSEP));

  if (psub[0] != _T('\0')) {
    len += lk::snprintf(buffer + len, size - len, _T("%s"), psub);
    while (IsDirectorySeparator(buffer[len - 1])) {
      --len;
    }
    len += lk::snprintf(buffer + len, size - len, _T("%s"), _T(DIRSEP));
  }

  if (pfile[0] != _T('\0')) {
    len += lk::snprintf(buffer + len, size - len, _T("%s"), pfile);
  }

  lk::filesystem::fixPath(buffer);
}

void SystemPath(TCHAR* buffer, size_t size, const TCHAR* SubPath, const TCHAR* file) {
  GetPath(buffer, size, SubPath, file,  LKGetSystemPath());
}

void LocalPath(TCHAR* buffer, size_t size, const TCHAR* SubPath, const TCHAR* file) {
  GetPath(buffer, size, SubPath, file, LKGetLocalPath());
}

void SystemPath(TCHAR* buffer, size_t size, const TCHAR* file) {
  GetPath(buffer, size, _T(""), file, LKGetSystemPath());
}

void LocalPath(TCHAR* buffer, size_t size, const TCHAR* file) {
  GetPath(buffer, size, _T(""), file, LKGetLocalPath());
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

  while (ptr && IsDirectorySeparator(*ptr)) {
    ++ptr;
    found = true;
  }
  if(found && ptr) {
    // memmove is required for overlapping src and dst
    std::memmove(szFilePath, ptr, (_tcslen(ptr) + 1) * sizeof(TCHAR));
  }
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include "Util/tstring.hpp"

TEST_CASE("LocalPath") {

	SUBCASE("RemoveFilePathPrefix") {
    TCHAR path[MAX_PATH] = { _T("prefix/file") };
    RemoveFilePathPrefix(_T("prefix"), path);
    CHECK(tstring_view(_T("file")) == path);
  }

  const tstring_view result(_T("path" DIRSEP "sub" DIRSEP "file"));
	SUBCASE("GetPath test 1") {
    TCHAR path[MAX_PATH];
    GetPath(path, MAX_PATH, _T("sub"), _T("file"), _T("path"));
    CHECK(result == path);
  }
	SUBCASE("GetPath test 2") {
    TCHAR path[MAX_PATH];
    GetPath(path, MAX_PATH, _T("/sub/"), _T("/file"), _T("path/"));
    CHECK(result == path);
  }
	SUBCASE("GetPath test 3") {
    TCHAR path[MAX_PATH];
    GetPath(path, MAX_PATH, _T("\\sub\\"), _T("\\file"), _T("path\\"));
    CHECK(result == path);
  }
	SUBCASE("GetPath test 4") {
    TCHAR path[MAX_PATH];
    GetPath(path, MAX_PATH, _T(""), _T("file"), _T("path"));
    CHECK(tstring_view(_T("path" DIRSEP "file")) == path);
  }
	SUBCASE("GetPath test 5") {
    TCHAR path[MAX_PATH];
    GetPath(path, MAX_PATH, _T("sub"), _T(""), _T("path"));
    CHECK(tstring_view(_T("path" DIRSEP "sub" DIRSEP)) == path);
  }
	SUBCASE("GetPath test 6") {
    TCHAR path[MAX_PATH];
    GetPath(path, MAX_PATH, _T(""), _T(""), _T("path"));
    CHECK(tstring_view(_T("path" DIRSEP)) == path);
  }
}

#endif
