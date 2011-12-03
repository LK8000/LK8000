/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Modeltype.h"

#ifdef PNA
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

  #if ALPHADEBUG
  StartupStore(_T(". Welcome to InstallSystem v1.2%s"),NEWLINE);
  #endif
  LocalPath(srcdir,TEXT(LKD_SYSTEM));

  _stprintf(dstdir,_T(""));

  // search for the main system directory on the real device
  // Remember that SHGetSpecialFolder works differently on CE platforms, and you cannot check for result.
  // We need to verify if directory does really exist.

  SHGetSpecialFolderPath(hWndMainWindow, dstdir, CSIDL_WINDOWS, false);
  if ( wcslen(dstdir) <6) {
	_stprintf(tbuf,_T("------ InstallSystem PROBLEM: cannot locate the Windows folder, got string:<%s>%s"),dstdir,NEWLINE);
	StartupStore(tbuf);
	StartupStore(_T("------ InstallSystem attempting to use default \"\\Windows\" but no warranty!%s"),NEWLINE);
	_stprintf(dstdir,TEXT("\\Windows")); // 091118
  } else {
	StartupStore(_T(". InstallSystem: Windows path reported from device is: <%s>%s"),dstdir,NEWLINE);
  }
  _tcscpy(maindir,dstdir);

#if OLDLOGGER
  _stprintf(tbuf,_T(". InstallSystem: copy DLL from <%s> to <%s>%s"), srcdir, dstdir,NEWLINE);
  StartupStore(tbuf);
#endif

  // We now test for a single file existing inside the directory, called _DIRECTORYNAME
  // because GetFileAttributes can be very slow or hang if checking a directory. In any case testing a file is 
  // much more faster.
#if (0)
  if (  GetFileAttributes(srcdir) != FILE_ATTRIBUTE_DIRECTORY) { // TODO FIX &= 
	StartupStore(_T("------ InstallSystem ERROR could not find source directory <%s> !%s"),srcdir,NEWLINE); // 091104
	failure=true;
#else
  _stprintf(srcfile,TEXT("%s\\_SYSTEM"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) {
	StartupStore(_T("------ InstallSystem ERROR could not find valid system directory <%s>%s"),srcdir,NEWLINE); // 091104
	StartupStore(_T("------ Missing checkfile <%s>%s"),srcfile,NEWLINE);
	failure=true;
#endif
  } else {
	#if TESTBENCH
	StartupStore(_T(". InstallSystem source directory <%s> is available%s"),srcdir,NEWLINE);
	#endif
  }

#if (0)
  attrib=GetFileAttributes(dstdir);
  if ( attrib == 0xffffffff ) {
	StartupStore(_T("------ InstallSystem ERROR Directory <%s> does not exist ???%s"),dstdir,NEWLINE);
	failure=true;
  }
  if ( attrib &= FILE_ATTRIBUTE_SYSTEM ) {
	StartupStore(_T(". Directory <%s> is identified as a system folder%s"),dstdir,NEWLINE);
  }
  if ( attrib &= FILE_ATTRIBUTE_COMPRESSED ) {
	StartupStore(_T(". Directory <%s> is a compressed folder%s"),dstdir,NEWLINE);
  }
  if ( attrib &= FILE_ATTRIBUTE_HIDDEN ) {
	StartupStore(_T(". Directory <%s> is a hidden folder%s"),dstdir,NEWLINE);
  }
  /* These attributes are not available
  if ( attrib &= FILE_ATTRIBUTE_INROM ) {
	StartupStore(_T("------ InstallSystem ERROR Directory <%s> is in ROM%s"),dstdir,NEWLINE);
	failure=true;
  }
  if ( attrib &= FILE_ATTRIBUTE_ROMMODULE ) {
	StartupStore(_T("------ InstallSystem ERROR Directory <%s> is a ROM MODULE%s"),dstdir,NEWLINE);
	failure=true;
  }
  */
  if ( attrib &= FILE_ATTRIBUTE_READONLY ) {
	StartupStore(_T("------ InstallSystem ERROR Directory <%s> is READ ONLY%s"),dstdir,NEWLINE);
	failure=true;
  }
#endif

  if (  failure ) {
	#if OLDLOGGER
	StartupStore(_T("------ WARNING: NO DLL install available (and thus NO G-RECORD FOR VALIDATING IGC FILES)%s"),NEWLINE);
	#endif
	StartupStore(_T("------ WARNING: NO font will be installed on device (and thus wrong text size displayed)%s"),NEWLINE);
	return 5; // 091109
  } else {
#if OLDLOGGER
#ifdef PPC2002
	_stprintf(srcfile,TEXT("%s\\GRECORD2002.XCS"),srcdir);
#endif
#ifdef PPC2003
	_stprintf(srcfile,TEXT("%s\\GRECORD2003.XCS"),srcdir);
#endif
#ifdef PNA
	_stprintf(srcfile,TEXT("%s\\GRECORDPNA.XCS"),srcdir);
#endif

	_stprintf(dstfile,TEXT("%s\\GRecordDll.dll"),dstdir);

	if (  GetFileAttributes(dstfile) != 0xffffffff ) {
		StartupStore(_T(". GRecordDll.dll already installed in device, very well.%s"),NEWLINE);
	} else {
		if (!CopyFile(srcfile,dstfile,TRUE)) {
			StartupStore(_T("++++++ COULD NOT INSTALL <%s> inside device. BAD!%s"),srcfile,NEWLINE);
			StartupStore(_T("++++++ Error code was: %ld%s"),GetLastError(),NEWLINE);
		} else {
			StartupStore(_T("... GRecordDll.dll installed using <%s>. Great.%s"),srcfile,NEWLINE);
		}
	}
#endif // OLDLOGGER

#ifdef PNA
	if (GlobalModelType == MODELTYPE_PNA_HP31X) { // 091109

		StartupStore(_T(". InstallSystem checking desktop links for HP31X%s"),NEWLINE);

		_stprintf(dstdir,TEXT("\\Windows\\Desktop"));
		if (  GetFileAttributes(dstdir) != FILE_ATTRIBUTE_DIRECTORY) { // FIX
			StartupStore(_T("------ Desktop directory <%s> NOT found! Is this REALLY an HP31X?%s"),dstdir,NEWLINE);
		} else {
			_stprintf(srcfile,TEXT("%s\\LK8_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\LK8000.lnk"),dstdir);
			if (  GetFileAttributes(dstfile) != 0xffffffff ) {
				StartupStore(_T(". Link to LK8000 already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!CopyFile(srcfile,dstfile,FALSE))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			#if 0
			_stprintf(srcfile,TEXT("%s\\LK8SIM_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\SIM.lnk"),dstdir);
			if (  GetFileAttributes(dstfile) != 0xffffffff ) {
				StartupStore(_T(". Link to SIM LK8000 already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!CopyFile(srcfile,dstfile,FALSE))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			#endif
			_stprintf(srcfile,TEXT("%s\\BT_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\BlueTooth.lnk"),dstdir);
			if (  GetFileAttributes(dstfile) != 0xffffffff ) {
				StartupStore(_T(". Link to BlueTooth already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!CopyFile(srcfile,dstfile,FALSE))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			_stprintf(srcfile,TEXT("%s\\NAV_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\CarNav.lnk"),dstdir);
			if (  GetFileAttributes(dstfile) != 0xffffffff ) {
				StartupStore(_T(". Link to Car Navigator already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!CopyFile(srcfile,dstfile,FALSE))  {
					StartupStore(_T("------ Could not install in <%s>. Strange.%s"),dstfile,NEWLINE);
					StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
				} else
					StartupStore(_T(". Installed <%s> link.%s"),dstfile,NEWLINE);
			}
			_stprintf(srcfile,TEXT("%s\\TLOCK_HP310.lnk"),srcdir);
			_stprintf(dstfile,TEXT("%s\\TouchLock.lnk"),dstdir);
			if (  GetFileAttributes(dstfile) != 0xffffffff ) {
				StartupStore(_T(". Link to TouchLock already found on the desktop, ok.%s"),NEWLINE);
			} else {
				StartupStore(_T(". Installing <%s>%s"),srcfile,NEWLINE);
				if (!CopyFile(srcfile,dstfile,FALSE))  {
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

  _stprintf(fontdir,_T(""));
  _stprintf(dstdir,_T(""));
  #ifdef PNA
  if ( GetFontPath(fontdir) == FALSE ) {
	StartupStore(_T(". Special RegKey for fonts not found on this PNA, using standard folder.%s"), NEWLINE);
	SHGetSpecialFolderPath(hWndMainWindow, dstdir, CSIDL_FONTS, false);
	if ( wcslen(dstdir) <5 ) {
		_stprintf(tbuf,_T("------ PROBLEM: cannot locate the Fonts folder, got string:<%s>%s"),dstdir,NEWLINE);
		StartupStore(tbuf);
		_stprintf(tbuf,_T("------ Attempting to use directory <%s> as a fallback%s"),maindir,NEWLINE);
		StartupStore(tbuf);
		_tcscpy(dstdir,maindir);
	}
  } else {
	StartupStore(_T(". RegKey Font directory is <%s>%s"),fontdir,NEWLINE);
	CreateRecursiveDirectory(fontdir);
	_tcscpy(dstdir,fontdir); 
  }
  #else
  // this is not working correctly on PNA, it is reporting Windows Fonts even with another value in regkey
  SHGetSpecialFolderPath(hWndMainWindow, dstdir, CSIDL_FONTS, false);
  if ( wcslen(dstdir) <5 ) {
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
  CreateDirectory(dstdir, NULL); // 100820


  // we cannot check directory existance without the risk of hanging for many seconds
  // we can only rely on singe real file existance, not on directories

  #if TESTBENCH
  StartupStore(_T(". Checking TAHOMA font%s"),NEWLINE);
  #endif
  _stprintf(srcfile,TEXT("%s\\TAHOMA.TTF"),srcdir);
  _stprintf(dstfile,TEXT("%s\\TAHOMA.TTF"),dstdir);
  if (  GetFileAttributes(dstfile) != 0xffffffff ) {
	#if TESTBENCH
	StartupStore(_T(". Font TAHOMA.TTF is already installed%s"),NEWLINE);
	#endif
  } else {
	if ( !CopyFile(srcfile,dstfile,TRUE))  {
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
  if (  GetFileAttributes(dstfile) != 0xffffffff ) {
	#if TESTBENCH
	StartupStore(_T(". Font TAHOMABD.TTF is already installed%s"),NEWLINE);
	#endif
  } else {
	if ( !CopyFile(srcfile,dstfile,TRUE))  {
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
  DWORD fattr = GetFileAttributes(rootdir);
  if ((fattr != 0xFFFFFFFF) && (fattr & FILE_ATTRIBUTE_DIRECTORY)) return true;
  return false;
}


bool CheckDataDir() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir,_T(LKD_SYSTEM));
  _stprintf(srcfile,TEXT("%s\\_SYSTEM"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) return false;
  return true;
}

bool CheckLanguageDir() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_LANGUAGE));
  _stprintf(srcfile,TEXT("%s\\_LANGUAGE"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) {
	return false;
  }
  return true;
}

bool CheckLanguageEngMsg() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_LANGUAGE));
  _stprintf(srcfile,TEXT("%s\\ENG_MSG.TXT"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) return false;
  return true;
}

bool CheckSystemDefaultMenu() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_SYSTEM));
  _stprintf(srcfile,TEXT("%s\\DEFAULT_MENU.TXT"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) return false;
  return true;
}

// We only check for general installation of GRecord files
bool CheckSystemGRecord() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_SYSTEM));
  _stprintf(srcfile,TEXT("%s\\_GRECORD"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) return false;
  return true;
}

bool CheckPolarsDir() {
  TCHAR srcdir[MAX_PATH];
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcdir, _T(LKD_POLARS));
  _stprintf(srcfile,TEXT("%s\\_POLARS"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) {
	return false;
  }

  LocalPath(srcdir, _T(LKD_POLARS));
  _stprintf(srcfile,TEXT("%s\\Default.plr"),srcdir);
  if (  GetFileAttributes(srcfile) == 0xffffffff ) return false;

  return true;
}

bool CheckRegistryProfile() {
	TCHAR srcpath[MAX_PATH];
	if ( GlobalModelType == MODELTYPE_PNA_HP31X ) return false;
	LocalPath(srcpath,TEXT(LKD_CONF)); // 091101
	_stprintf(srcpath,_T("%s\\%s"),srcpath,LKPROFILE); // 091101
	if (  GetFileAttributes(srcpath) == 0xffffffff) return false;
	return true;
}

