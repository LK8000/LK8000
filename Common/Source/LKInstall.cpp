/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Modeltype.h"
#ifdef WIN32
#include <shlobj.h>
#endif
#include "utils/openzip.h"
#include "LocalPath.h"


bool CheckRootDir() {
  TCHAR rootdir[MAX_PATH];
  LocalPath(rootdir,_T(""));
  return lk::filesystem::isDirectory(rootdir);
}


bool CheckDataDir() {
  TCHAR srcfile[MAX_PATH];
  SystemPath(srcfile,_T(LKD_SYSTEM),_T("_SYSTEM"));
  return lk::filesystem::exist(srcfile);
}

bool CheckLanguageDir() {
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcfile, _T(LKD_LANGUAGE), _T("language.json"));
  return lk::filesystem::exist(srcfile);
}

bool CheckLanguageEngMsg() {
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcfile, _T(LKD_LANGUAGE), _T("en.json"));
  return lk::filesystem::exist(srcfile);
}

bool CheckSystemDefaultMenu() {
  TCHAR srcfile[MAX_PATH];
  SystemPath(srcfile, _T(LKD_SYSTEM), _T("DEFAULT_MENU.TXT"));
  return lk::filesystem::exist(srcfile);
}


bool CheckPolarsDir() {
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcfile, _T(LKD_POLARS), _T("_POLARS"));
  if ( !lk::filesystem::exist(srcfile) ) {
    return false;
  }

  LocalPath(srcfile, _T(LKD_POLARS), _T("Default.plr"));
  return lk::filesystem::exist(srcfile);
}

#ifndef ANDROID
bool CheckSystemBitmaps() {
  TCHAR srcfile[MAX_PATH];
  SystemPath(srcfile, _T(LKD_BITMAPS), _T("_BITMAPSH"));
  return lk::filesystem::exist(srcfile);
}
#endif

bool CheckFilesystemWritable() {
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcfile, _T("EmptyTest.txt"));

  FILE *stream =_tfopen(srcfile,_T("w"));
  if (stream==NULL) return false;
  bool success = fprintf(stream,"FILESYSTEM WRITE CHECK, THIS FILE CAN BE REMOVED ANY TIME\n") >= 0;
  success &= fclose(stream) == 0;

  lk::filesystem::deleteFile(srcfile);

  return(success);
}

//
// The INFO system, previously CREDITS, can be updated anytime.
// We save the last version we have read inside _Configuration/LASTINFO.TXT. 
//--pv
//
bool CheckInfoUpdated() {

  TCHAR srcfile[MAX_PATH];

  bool have_old_version=false, have_new_version=false;
  char newversion[4], oldversion[4];
  size_t new_size = 0,  old_size = 0;

  // 
  // First we check the INFO has a version
  // on android platform, this file is inside APK ( zip file )
  // 
  SystemPath(srcfile, _T(LKD_SYSTEM), _T(LKF_CREDITS));
  ZZIP_FILE *fp_new=openzip(srcfile, "rb");
  if (fp_new) {
    new_size = zzip_fread(newversion, 1, sizeof(newversion), fp_new);
    // warning fread can return -1, new_size is size_t, casting result?
    // Situation managed here, anyway.
    if(new_size == sizeof(newversion)) {
      have_new_version= ( newversion[0] == _T('#') && newversion[3]==_T(';') );
    }
    zzip_fclose(fp_new);
  } else {
#ifdef TESTBENCH
     StartupStore(_T("... Info/Credits file <%s> not existing!"), srcfile);
#endif
  }

  if (!have_new_version) {
     StartupStore(_T(". LKInstall: INFO DOES NOT HAVE A VERSION??"));
     return false;
  }

  // Load Last version
  LocalPath(srcfile, _T(LKD_CONF), _T(LKF_LASTVERSION));
  FILE* fp_old = _tfopen(srcfile, _T("rb"));
  if (fp_old) {
    old_size = fread(oldversion, 1, sizeof(oldversion), fp_old);
    if(old_size == sizeof(oldversion)) {
      have_old_version= ( oldversion[0] == _T('#') && oldversion[3]==_T(';') );
    } 
    fclose(fp_old);
  } else {
#ifdef TESTBENCH
    StartupStore(_T("... Info version <%s> not existing, first time?"),srcfile);
#endif

  }

  if (have_new_version && have_old_version) {
    if(new_size == old_size) {
      if (!memcmp(newversion,oldversion, new_size)) {
        // same version, nothing to do
#ifdef TESTBENCH
        StartupStore(_T(".... Info/Credits old=new version, nothing to do."));
#endif
        return false;
      }
    }
  }

  // ... else we either dont have an old version, or the old and the new are different.
  // we save new version as old version. In both cases we return true
  FILE* stream=_tfopen(srcfile,_T("wb"));
  if (stream) {
    const size_t version_size = sizeof(newversion);
    const size_t write_size = fwrite (newversion , 1, sizeof(newversion), stream);
    fclose(stream);

    if(version_size == write_size) {
        StartupStore(_T(". LKInstall created <%s>"),srcfile);
        return true;
    }
  } 
  // In case of problems, we drop usage
  StartupStore(_T(". LKInstall cannot create %s"),srcfile);
  return false;
}


