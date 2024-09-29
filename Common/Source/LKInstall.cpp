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
#if defined(PNA) && defined(UNDER_CE)
#include "Devices/LKHolux.h"
#include "Devices/LKRoyaltek3200.h"
#endif
#endif
#include "utils/openzip.h"
#include "LocalPath.h"



// This will NOT be called from PC versions
void InstallSystem() {
#ifdef UNDER_CE
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];


  TCHAR dstdir[MAX_PATH];
  TCHAR maindir[MAX_PATH];
  TCHAR dstfile[MAX_PATH];
  TCHAR tbuf[MAX_PATH*3];

  dstdir[0]='\0';

  bool failure=false;

  TestLog(_T(". Welcome to InstallSystem v1.2"));
  SystemPath(srcdir,TEXT(LKD_SYSTEM));



  // We now test for a single file existing inside the directory, called _DIRECTORYNAME
  // because GetFileAttributes can be very slow or hang if checking a directory. In any case testing a file is
  // much more faster.
  _stprintf(srcfile,TEXT("%s%s_SYSTEM"),srcdir, _T(DIRSEP));
  if ( !lk::filesystem::exist(srcfile) ) {
	StartupStore(_T("------ InstallSystem ERROR could not find valid system directory <%s>%s"),srcdir,NEWLINE); // 091104
	StartupStore(_T("------ Missing checkfile <%s>%s"),srcfile,NEWLINE);
	failure=true;
  } else {
	TestLog(_T(". InstallSystem source directory <%s> is available"), srcdir);
  }

#ifdef PNA
  if (  failure ) {
	StartupStore(_T("------ WARNING: NO font will be installed on device (and thus wrong text size displayed)%s"),NEWLINE);
  } else {

	if (ModelType::Get() == ModelType::HP31X) { // 091109

		StartupStore(_T(". InstallSystem checking desktop links for HP31X%s"),NEWLINE);

		_stprintf(dstdir,TEXT("\\Windows\\Desktop"));
		if ( !lk::filesystem::isDirectory(dstdir) ) { // FIX
			StartupStore(_T("------ Desktop directory <%s> NOT found! Is this REALLY an HP31X?%s"),dstdir,NEWLINE);
		} else {
			_stprintf(srcfile,TEXT("%s\\LK8_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\LK8000.lnk"),dstdir);
			if ( lk::filesystem::exist(dstfile) ) {
				StartupStore(_T(". Link to LK8000 already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!lk::filesystem::copyFile(srcfile,dstfile,true))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			#if 0
			_stprintf(srcfile,TEXT("%s\\LK8SIM_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\SIM.lnk"),dstdir);
			if ( lk::filesystem::exist(dstfile) ) {
				StartupStore(_T(". Link to SIM LK8000 already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!lk::filesystem::copyFile(srcfile,dstfile,true))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			#endif
			_stprintf(srcfile,TEXT("%s\\BT_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\BlueTooth.lnk"),dstdir);
			if ( lk::filesystem::exist(dstfile) ) {
				StartupStore(_T(". Link to BlueTooth already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!lk::filesystem::copyFile(srcfile,dstfile,true))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			_stprintf(srcfile,TEXT("%s\\NAV_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\CarNav.lnk"),dstdir);
			if ( lk::filesystem::exist(dstfile) ) {
				StartupStore(_T(". Link to Car Navigator already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!lk::filesystem::copyFile(srcfile,dstfile,true))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			_stprintf(srcfile,TEXT("%s\\TLOCK_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\TouchLock.lnk"),dstdir);
			if ( lk::filesystem::exist(dstfile) ) {
				StartupStore(_T(". Link to TouchLock already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!lk::filesystem::copyFile(srcfile,dstfile,true))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
		}
	}
  }
#endif

  // search for the main system directory on the real device
  // Remember that SHGetSpecialFolder works differently on CE platforms, and you cannot check for result.
  // We need to verify if directory does really exist.

//  SHGetSpecialFolderPath(MainWindow, dstdir, CSIDL_WINDOWS, false);
  if ( _tcslen(dstdir) <6) {
	_stprintf(tbuf,_T("------ InstallSystem PROBLEM: cannot locate the Windows folder, got string:<%s>%s"),dstdir,NEWLINE);
	StartupStore(tbuf);
	StartupStore(_T("------ InstallSystem attempting to use default \"\\Windows\" but no warranty!%s"),NEWLINE);
	_stprintf(dstdir,TEXT("\\Windows")); // 091118
  } else {
	StartupStore(_T(". InstallSystem: Windows path reported from device is: <%s>%s"),dstdir,NEWLINE);
  }
  _tcscpy(maindir,dstdir);

  // we are shure that \Windows does exist already.
  TCHAR fontdir[MAX_PATH];
  fontdir[0] = _T('\0');
  dstdir[0] = _T('\0');
  #ifdef PNA
  if ( GetFontPath(fontdir) == FALSE ) {
	StartupStore(_T(". Special RegKey for fonts not found on this PNA, using standard folder.%s"), NEWLINE);
//	SHGetSpecialFolderPath(MainWindow, dstdir, CSIDL_FONTS, false);
	if ( _tcslen(dstdir) <5 ) {
		_stprintf(tbuf,_T("------ PROBLEM: cannot locate the Fonts folder, got string:<%s>%s"),dstdir,NEWLINE);
		StartupStore(tbuf);
		_stprintf(tbuf,_T("------ Attempting to use directory <%s> as a fallback%s"),maindir,NEWLINE);
		StartupStore(tbuf);
		_tcscpy(dstdir,maindir);
	}
  } else {
	StartupStore(_T(". RegKey Font directory is <%s>%s"),fontdir,NEWLINE);
	lk::filesystem::createDirectory(fontdir);
	_tcscpy(dstdir,fontdir);
  }
  #else
  UNUSED(fontdir);
  // this is not working correctly on PNA, it is reporting Windows Fonts even with another value in regkey
  SHGetSpecialFolderPath(main_window->Handle(), dstdir, CSIDL_FONTS, false);
  if ( _tcslen(dstdir) <5 ) {
	_stprintf(tbuf,_T("------ PROBLEM: cannot locate the Fonts folder, got string:<%s>%s"),dstdir,NEWLINE);
	StartupStore(tbuf);
	_stprintf(tbuf,_T("------ Attempting to use directory <%s> as a fallback%s"),maindir,NEWLINE);
	StartupStore(tbuf);
	_tcscpy(dstdir,maindir);
  }
  #endif

  _stprintf(tbuf,_T(". InstallSystem: Copy/Check Fonts from <%s> to <%s>%s"), srcdir, dstdir,NEWLINE);
  StartupStore(tbuf);
  // on PNAs sometimes FolderPath is reported correctly, but the directory is not existing!
  // this is not needed really on PNA, but doesnt hurt
  lk::filesystem::createDirectory(dstdir); // 100820


  // we cannot check directory existance without the risk of hanging for many seconds
  // we can only rely on singe real file existance, not on directories

  TestLog(_T(". Checking TAHOMA font"));
  _stprintf(srcfile,TEXT("%s\\TAHOMA.TTF"),srcdir);
  _stprintf(dstfile,TEXT("%s\\TAHOMA.TTF"),dstdir);
  if ( lk::filesystem::exist(dstfile) ) {
	TestLog(_T(". Font TAHOMA.TTF is already installed"));
  } else {
	if ( !lk::filesystem::copyFile(srcfile,dstfile,false) )  {
		StartupStore(_T("------ Could not copy TAHOMA.TTF on device, not good.%s"),NEWLINE);
		StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
	} else
		StartupStore(_T("... Font TAHOMA.TTF installed on device%s"),NEWLINE);
  }

  // not needed, cannot overwrite tahoma while in use! Tahoma bold not used for some reason in this case.
  // Problem solved, look at FontPath !!

  TestLog(_T(". Checking TAHOMABD font"));
  _stprintf(srcfile,TEXT("%s\\TAHOMABD.TTF"),srcdir);
  _stprintf(dstfile,TEXT("%s\\TAHOMABD.TTF"),dstdir);
  if ( lk::filesystem::exist(dstfile) ) {
	TestLog(_T(". Font TAHOMABD.TTF is already installed"));
  } else {
	if ( !lk::filesystem::copyFile(srcfile,dstfile,false))  {
		StartupStore(_T("------ Could not copy TAHOMABD.TTF on device, not good.%s"),NEWLINE);
		StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
	} else
		StartupStore(_T("... Font TAHOMABD.TTF installed on device%s"),NEWLINE);
  }

  TestLog(_T(". InstallSystem completed OK"));
#endif
}



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


