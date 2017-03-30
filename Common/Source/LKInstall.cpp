/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Modeltype.h"
#ifdef WIN32
#include <shlobj.h>
#if defined(PNA) && defined(UNDER_CE)
#include "LKHolux.h"
#include "LKRoyaltek3200.h"
#endif
#endif
#include "utils/stringext.h"



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

  #if TESTBENCH
  StartupStore(_T(". Welcome to InstallSystem v1.2%s"),NEWLINE);
  #endif
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
	#if TESTBENCH
	StartupStore(_T(". InstallSystem source directory <%s> is available%s"),srcdir,NEWLINE);
	#endif
  }

#ifdef PNA
  if (  failure ) {
	StartupStore(_T("------ WARNING: NO font will be installed on device (and thus wrong text size displayed)%s"),NEWLINE);
  } else {

	if (GlobalModelType == MODELTYPE_PNA_HP31X) { // 091109

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
  SHGetSpecialFolderPath(MainWindow.Handle(), dstdir, CSIDL_FONTS, false);
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

  #if TESTBENCH
  StartupStore(_T(". Checking TAHOMA font%s"),NEWLINE);
  #endif
  _stprintf(srcfile,TEXT("%s\\TAHOMA.TTF"),srcdir);
  _stprintf(dstfile,TEXT("%s\\TAHOMA.TTF"),dstdir);
  if ( lk::filesystem::exist(dstfile) ) {
	#if TESTBENCH
	StartupStore(_T(". Font TAHOMA.TTF is already installed%s"),NEWLINE);
	#endif
  } else {
	if ( !lk::filesystem::copyFile(srcfile,dstfile,false) )  {
		StartupStore(_T("------ Could not copy TAHOMA.TTF on device, not good.%s"),NEWLINE);
		StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
	} else
		StartupStore(_T("... Font TAHOMA.TTF installed on device%s"),NEWLINE);
  }

  // not needed, cannot overwrite tahoma while in use! Tahoma bold not used for some reason in this case.
  // Problem solved, look at FontPath !!

  #if TESTBENCH
  StartupStore(_T(". Checking TAHOMABD font%s"),NEWLINE);
  #endif
  _stprintf(srcfile,TEXT("%s\\TAHOMABD.TTF"),srcdir);
  _stprintf(dstfile,TEXT("%s\\TAHOMABD.TTF"),dstdir);
  if ( lk::filesystem::exist(dstfile) ) {
	#if TESTBENCH
	StartupStore(_T(". Font TAHOMABD.TTF is already installed%s"),NEWLINE);
	#endif
  } else {
	if ( !lk::filesystem::copyFile(srcfile,dstfile,false))  {
		StartupStore(_T("------ Could not copy TAHOMABD.TTF on device, not good.%s"),NEWLINE);
		StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
	} else
		StartupStore(_T("... Font TAHOMABD.TTF installed on device%s"),NEWLINE);
  }

  #if TESTBENCH
  StartupStore(_T(". InstallSystem completed OK%s"),NEWLINE);
  #endif
#endif
}



bool CheckRootDir() {
  TCHAR rootdir[MAX_PATH];
  LocalPath(rootdir,_T(""));
  return lk::filesystem::isDirectory(rootdir);
}


bool CheckDataDir() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  SystemPath(srcdir,_T(LKD_SYSTEM));
  _stprintf(srcfile,TEXT("%s%s_SYSTEM"),srcdir, _T(DIRSEP));
  return lk::filesystem::exist(srcfile);
}

bool CheckLanguageDir() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_LANGUAGE));
  _stprintf(srcfile,TEXT("%s%s_LANGUAGE"),srcdir, _T(DIRSEP));
  return lk::filesystem::exist(srcfile);
}

bool CheckLanguageEngMsg() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_LANGUAGE));
  _stprintf(srcfile,TEXT("%s%sENG_MSG.TXT"),srcdir, _T(DIRSEP));
  return lk::filesystem::exist(srcfile);
}

bool CheckSystemDefaultMenu() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  SystemPath(srcdir, _T(LKD_SYSTEM));
  _stprintf(srcfile,TEXT("%s%sDEFAULT_MENU.TXT"),srcdir, _T(DIRSEP));
  return lk::filesystem::exist(srcfile);
}


bool CheckPolarsDir() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_POLARS));
  _stprintf(srcfile,TEXT("%s%s_POLARS"),srcdir, _T(DIRSEP));
  if ( !lk::filesystem::exist(srcfile) ) {
    return false;
  }

  LocalPath(srcdir, _T(LKD_POLARS));
  _stprintf(srcfile,TEXT("%s%sDefault.plr"),srcdir, _T(DIRSEP));
  return lk::filesystem::exist(srcfile);
}

bool CheckRegistryProfile() {
    TCHAR srcpath[MAX_PATH];
    TCHAR profilePath[MAX_PATH];
    if ( GlobalModelType == MODELTYPE_PNA_HP31X ) return false;
    LocalPath(srcpath,TEXT(LKD_CONF)); // 091101
    _stprintf(profilePath,_T("%s%s%s"),srcpath, _T(DIRSEP),LKPROFILE); // 091101
    return lk::filesystem::exist(profilePath);
}

#ifndef ANDROID
bool CheckSystemBitmaps() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  SystemPath(srcdir, _T(LKD_BITMAPS));
  _stprintf(srcfile,TEXT("%s%s_BITMAPSH"),srcdir, _T(DIRSEP));
  return lk::filesystem::exist(srcfile);
}
#endif

bool CheckFilesystemWritable() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(""));
  _stprintf(srcfile,TEXT("%s%sEmptyTest.txt"),srcdir, _T(DIRSEP));

  FILE *stream;
  stream=_tfopen(srcfile,_T("a"));
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
#include "utils/openzip.h"
#define LKF_LASTVERSION "LASTINFO.TXT"
bool CheckInfoUpdated() {

  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  SystemPath(srcdir, _T(LKD_SYSTEM));
  _stprintf(srcfile,TEXT("%s%s%s"),srcdir, _T(DIRSEP), _T(LKF_CREDITS));

  bool have_old_version=false, have_new_version=false;
  TCHAR newversion[3], oldversion[3];

  // 
  // First we check the INFO has a version
  // 
  if (lk::filesystem::exist(srcfile)) {

     ZZIP_FILE *fp=openzip(srcfile, "rb");
     if (!fp) {
        return false;
     }

     #define MAXHEADER 10
     TCHAR line[MAXHEADER+1]; line[0]=0;

     charset cs = charset::unknown;
     ReadString(fp,MAXHEADER,line, cs);

     if (_tcslen(line)==4) {
        if ( line[0] == _T('#') && line[3]==_T(';') ) {
           newversion[0]=line[1];
           newversion[1]=line[2];
           newversion[2]=_T('\0');
           have_new_version=true;
        } 
     }
     zzip_fclose(fp);
  } 
  #ifdef TESTBENCH
     else StartupStore(_T("... Info/Credits file <%s> not existing!%s"),srcfile,NEWLINE);
  #endif

  if (!have_new_version) {
     StartupStore(_T(". LKInstall: INFO DOES NOT HAVE A VERSION??%s"),NEWLINE);
     return false;
  }

  LocalPath(srcdir, _T(LKD_CONF));
  _stprintf(srcfile,TEXT("%s%s%s"),srcdir,_T(DIRSEP),_T(LKF_LASTVERSION));

  if ( !lk::filesystem::exist(srcfile) ) {
     #ifdef TESTBENCH
     StartupStore(_T("... Info version <%s> not existing, first time?%s"),srcfile,NEWLINE);
     #endif
  } else {
     ZZIP_FILE *fp=openzip(srcfile, "rb");
     if (!fp) return false;

     #define MAXHEADER 10
     TCHAR line[MAXHEADER+1]; line[0]=0;

     charset cs = charset::unknown;
     ReadString(fp,MAXHEADER,line, cs);

     if (_tcslen(line)==4) {
        if ( line[0] == _T('#') && line[3]==_T(';') ) {
           oldversion[0]=line[1];
           oldversion[1]=line[2];
           oldversion[2]=_T('\0');
           have_old_version=true;
        }
     }
     zzip_fclose(fp);
  }

  if (have_old_version) {
     if (!_tcscmp(newversion,oldversion)) {
        // same version, nothing to do
        #ifdef TESTBENCH
        StartupStore(_T(".... Info/Credits old=new version, nothing to do.\n"));
        #endif
        return false;
     } 
  }
  // ... else we either dont have an old version, or the old and the new are different.
  // we save new version as old version. In both cases we return true
  // First we remove the existing version file
  lk::filesystem::deleteFile(srcfile); 

  // then we create a new one with the current version in the header
  FILE *stream =_tfopen(srcfile,_T("a"));
  if (stream==NULL) {
     // In case of problems, we drop usage
     StartupStore(_T(". LKInstall cannot create lastversion.txt%s"),NEWLINE);
     return false;
  }
  char sbuf[20]; TCHAR tbuf[10];
  _stprintf(tbuf,_T("#%s;\n"),newversion);
  TCHAR2utf(tbuf,sbuf,sizeof(sbuf));
  fprintf(stream,sbuf);

  StartupStore(_T(". LKInstall created <%s> header=<%s>%s"),srcfile,tbuf,NEWLINE);
  fclose(stream);
  return true;
}


