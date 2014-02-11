/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Modeltype.h"
#include <shlobj.h>

extern HWND hWndMainWindow;


#if defined(PNA) && defined(UNDER_CE)
#include "LKHolux.h"
#include "LKRoyaltek3200.h"
#endif



// This will NOT be called from PC versions
short InstallSystem() {

  TCHAR srcdir[MAX_PATH];
  TCHAR dstdir[MAX_PATH];
  TCHAR maindir[MAX_PATH];
  TCHAR fontdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  TCHAR dstfile[MAX_PATH];
  TCHAR tbuf[MAX_PATH*3];
#if (0)
  DWORD attrib;
#endif
  bool failure=false;

  #if TESTBENCH
  StartupStore(_T(". Welcome to InstallSystem v1.2%s"),NEWLINE);
  #endif
  LocalPath(srcdir,TEXT(LKD_SYSTEM));

  dstdir[0]='\0';

  // search for the main system directory on the real device
  // Remember that SHGetSpecialFolder works differently on CE platforms, and you cannot check for result.
  // We need to verify if directory does really exist.

//  SHGetSpecialFolderPath(hWndMainWindow, dstdir, CSIDL_WINDOWS, false);
  if ( _tcslen(dstdir) <6) {
	_stprintf(tbuf,_T("------ InstallSystem PROBLEM: cannot locate the Windows folder, got string:<%s>%s"),dstdir,NEWLINE);
	StartupStore(tbuf);
	StartupStore(_T("------ InstallSystem attempting to use default \"\\Windows\" but no warranty!%s"),NEWLINE);
	_stprintf(dstdir,TEXT("\\Windows")); // 091118
  } else {
	StartupStore(_T(". InstallSystem: Windows path reported from device is: <%s>%s"),dstdir,NEWLINE);
  }
  _tcscpy(maindir,dstdir);


  // We now test for a single file existing inside the directory, called _DIRECTORYNAME
  // because GetFileAttributes can be very slow or hang if checking a directory. In any case testing a file is 
  // much more faster.
  _stprintf(srcfile,TEXT("%s\\_SYSTEM"),srcdir);
  if ( !lk::filesystem::exist(srcfile) ) {
	StartupStore(_T("------ InstallSystem ERROR could not find valid system directory <%s>%s"),srcdir,NEWLINE); // 091104
	StartupStore(_T("------ Missing checkfile <%s>%s"),srcfile,NEWLINE);
	failure=true;
  } else {
	#if TESTBENCH
	StartupStore(_T(". InstallSystem source directory <%s> is available%s"),srcdir,NEWLINE);
	#endif
  }

  if (  failure ) {
	StartupStore(_T("------ WARNING: NO font will be installed on device (and thus wrong text size displayed)%s"),NEWLINE);
	return 5; // 091109
  } else {

#if defined(PNA) && defined(UNDER_CE)
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
#endif

  }

  // we are shure that \Windows does exist already.

  fontdir[0] = _T('\0');
  dstdir[0] = _T('\0');
  #ifdef PNA
  if ( GetFontPath(fontdir) == FALSE ) {
	StartupStore(_T(". Special RegKey for fonts not found on this PNA, using standard folder.%s"), NEWLINE);
//	SHGetSpecialFolderPath(hWndMainWindow, dstdir, CSIDL_FONTS, false);
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
  SHGetSpecialFolderPath(hWndMainWindow, dstdir, CSIDL_FONTS, false);
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

  return 0;

}



bool CheckRootDir() {
  TCHAR rootdir[MAX_PATH];
  LocalPath(rootdir,_T(""));
  return lk::filesystem::isDirectory(rootdir);
}


bool CheckDataDir() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir,_T(LKD_SYSTEM));
  _stprintf(srcfile,TEXT("%s\\_SYSTEM"),srcdir);
  return lk::filesystem::exist(srcfile);
}

bool CheckLanguageDir() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_LANGUAGE));
  _stprintf(srcfile,TEXT("%s\\_LANGUAGE"),srcdir);
  return lk::filesystem::exist(srcfile);
}

bool CheckLanguageEngMsg() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_LANGUAGE));
  _stprintf(srcfile,TEXT("%s\\ENG_MSG.TXT"),srcdir);
  return lk::filesystem::exist(srcfile);
}

bool CheckSystemDefaultMenu() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_SYSTEM));
  _stprintf(srcfile,TEXT("%s\\DEFAULT_MENU.TXT"),srcdir);
  return lk::filesystem::exist(srcfile);
}


bool CheckPolarsDir() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_POLARS));
  _stprintf(srcfile,TEXT("%s\\_POLARS"),srcdir);
  if ( !lk::filesystem::exist(srcfile) ) {
	return false;
  }

  LocalPath(srcdir, _T(LKD_POLARS));
  _stprintf(srcfile,TEXT("%s\\Default.plr"),srcdir);
  return lk::filesystem::exist(srcfile);
}

bool CheckRegistryProfile() {
	TCHAR srcpath[MAX_PATH];
	TCHAR profilePath[MAX_PATH];
	if ( GlobalModelType == MODELTYPE_PNA_HP31X ) return false;
	LocalPath(srcpath,TEXT(LKD_CONF)); // 091101
	_stprintf(profilePath,_T("%s\\%s"),srcpath,LKPROFILE); // 091101
    return lk::filesystem::exist(profilePath);
}


bool CheckSystemBitmaps() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_BITMAPS));
  _stprintf(srcfile,TEXT("%s\\_BITMAPSH"),srcdir);
  return lk::filesystem::exist(srcfile);
}

bool CheckFilesystemWritable() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_SYSTEM));
  _stprintf(srcfile,TEXT("%s\\EmptyTest.txt"),srcdir);

  FILE *stream;
  stream=_tfopen(srcfile,_T("a"));
  if (stream==NULL) return false;
  bool success = fprintf(stream,"FILESYSTEM WRITE CHECK, THIS FILE CAN BE REMOVED ANY TIME\n") >= 0;
  success &= fclose(stream) == 0;
  return(success);
}






