/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils2.cpp,v 8.31 2010/12/13 00:55:29 root Exp root $
*/

#include "StdAfx.h"
#include <stdio.h>
#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#endif
#include "options.h"
#include "externs.h"
#include "lk8000.h"
#include "InfoBoxLayout.h"
#include "Utils2.h"
#include "device.h"
#include "Logger.h"
#include "Parser.h"
#include "WaveThread.h"
#include "Message.h"
#include "McReady.h"
#include "InputEvents.h"
#include "MapWindow.h"
#include "LKMapWindow.h"
#include "RGB.h"

#include "Modeltype.h"

#ifdef PNA
#include "LKHolux.h"
#include "LKRoyaltek3200.h"
#endif

#include "utils/heapcheck.h"
using std::min;
using std::max;

extern void NextMapSpace();
extern void PreviousMapSpace();
extern void ShowMenu();
extern void ResetNearestTopology();

void BottomSounds();

extern void ApplyClearType(LOGFONT *logfont);



// InitLKScreen can be called anytime, and should be called upon screen changed from portrait to landscape,
// or windows size is changed for any reason. We dont support dynamic resize of windows, though, because each
// resolution has its own tuned settings. This is thought for real devices, not for PC emulations.
// Attention: after InitLKScreen, also InitLKFonts should be called. 
void InitLKScreen() {

#if (WINDOWSPC>0)
  int iWidth=SCREENWIDTH;
  int iHeight=SCREENHEIGHT;
#else
  int iWidth=GetSystemMetrics(SM_CXSCREEN);
  int iHeight=GetSystemMetrics(SM_CYSCREEN);
#endif

  ScreenSizeX=iWidth;
  ScreenSizeY=iHeight;
  ScreenSizeR.top=0;
  ScreenSizeR.bottom=iHeight-1;
  ScreenSizeR.left=0;
  ScreenSizeR.right=iWidth-1;

  int maxsize=0;
  int minsize=0;
  maxsize = max(ScreenSizeR.right-ScreenSizeR.left+1,ScreenSizeR.bottom-ScreenSizeR.top+1);
  minsize = min(ScreenSizeR.right-ScreenSizeR.left+1,ScreenSizeR.bottom-ScreenSizeR.top+1);

  ScreenDScale = max(1.0,minsize/240.0); // always start w/ shortest dimension

  if (maxsize == minsize) 
  {
    ScreenDScale *= 240.0 / 320.0;
  }

  ScreenScale = (int)ScreenDScale;

  #if (WINDOWSPC>0)
  if (maxsize==720) {
        ScreenScale=2; // force rescaling with Stretch
  }
  #endif

  if ( ((double)ScreenScale) == ScreenDScale)
	ScreenIntScale = true;
  else
	ScreenIntScale = false;

  int i;
  if ( ScreenIntScale ) {
        for (i=0; i<=MAXIBLSCALE; i++) LKIBLSCALE[i]=(int)(i*ScreenScale);
  } else {
        for (i=0; i<=MAXIBLSCALE;i++) LKIBLSCALE[i]=(int)(i*ScreenDScale);
  }

  // StartupStore(_T("...... ScreenScale=%d ScreenDScale=%.3f ScreenIntScale=%d\n"),ScreenScale,ScreenDScale,ScreenIntScale);

  ScreenSize=0;

  if (iWidth == 240 && iHeight == 320) ScreenSize=(ScreenSize_t)ss240x320; // QVGA      portrait
  if (iWidth == 234 && iHeight == 320) ScreenSize=(ScreenSize_t)ss240x320; // use the same config of 240x320
  if (iWidth == 272 && iHeight == 480) ScreenSize=(ScreenSize_t)ss272x480;
  if (iWidth == 240 && iHeight == 400) ScreenSize=(ScreenSize_t)ss240x320; //           portrait

  if (iWidth == 480 && iHeight == 640) ScreenSize=(ScreenSize_t)ss480x640; //  VGA
  if (iWidth == 640 && iHeight == 480) ScreenSize=(ScreenSize_t)ss640x480; //   VGA
  if (iWidth == 320 && iHeight == 240) ScreenSize=(ScreenSize_t)ss320x240; //  QVGA
  if (iWidth == 320 && iHeight == 234) ScreenSize=(ScreenSize_t)ss320x240; //  QVGA
  if (iWidth == 720 && iHeight == 408) ScreenSize=(ScreenSize_t)ss720x408;
  if (iWidth == 480 && iHeight == 800) ScreenSize=(ScreenSize_t)ss480x800;
  if (iWidth == 400 && iHeight == 240) ScreenSize=(ScreenSize_t)ss400x240; // landscape
  if (iWidth == 480 && iHeight == 272) ScreenSize=(ScreenSize_t)ss480x272; // WQVGA     landscape
  if (iWidth == 480 && iHeight == 234) ScreenSize=(ScreenSize_t)ss480x234; //   iGo
  if (iWidth == 800 && iHeight == 480) ScreenSize=(ScreenSize_t)ss800x480; //  WVGA
  if (iWidth == 896 && iHeight == 672) ScreenSize=(ScreenSize_t)ss896x672; //  PC version only

  TCHAR tbuf[80];
  if (ScreenSize==0) {
        wsprintf(tbuf,_T(". InitLKScreen: ++++++ ERROR UNKNOWN RESOLUTION %dx%d !%s"),iWidth,iHeight,NEWLINE); // 091119
        StartupStore(tbuf);
  } else {
        wsprintf(tbuf,_T(". InitLKScreen: %dx%d%s"),iWidth,iHeight,NEWLINE); // 091213
        StartupStore(tbuf);
  }

  if (ScreenSize > (ScreenSize_t)sslandscape) 
	ScreenLandscape=true;
  else
	ScreenLandscape=false;

  // By default, h=v=size/6 and here we set it better
  switch (ScreenSize) { 
	case (ScreenSize_t)ss800x480:
		GestureSize=50;
		LKVarioSize=50;
		// dscale=480/240=2  800/dscale=400 -(70+2+2)=  326 x dscale = 652
		LKwdlgConfig=652;
		break;
	case (ScreenSize_t)ss400x240:
		GestureSize=50;
		LKVarioSize=25;
		// dscale=240/240=1  400/dscale=400 -(70+2+2)=  326 x dscale = 326
		LKwdlgConfig=326;
		break;
	case (ScreenSize_t)ss640x480:
		GestureSize=50;
		LKVarioSize=40;
		// dscale=480/240=2  640/dscale=320 -(70+2+2)=  246 x dscale = 492
		LKwdlgConfig=492;
		break;
	case (ScreenSize_t)ss896x672:
		GestureSize=50;
		LKVarioSize=56;
		// dscale=672/240=2.8  896/dscale=320 -(70+2+2)=  246 x dscale = 689
		LKwdlgConfig=689;
		break;
	case (ScreenSize_t)ss480x272:
		GestureSize=50;
		LKVarioSize=30;
		// dscale=272/240=1.133  480/dscale=424 -(70+2+2)=  350 x dscale = 397
		LKwdlgConfig=395;
		break;
	case (ScreenSize_t)ss720x408:
		GestureSize=50;
		LKVarioSize=45;
		// dscale=408/240=1.133  720/dscale=423 -(70+2+2)=  350 x dscale = 594
		LKwdlgConfig=594;
		break;
	case (ScreenSize_t)ss480x234:
		GestureSize=50;
		LKVarioSize=30;
		// dscale=234/240=0.975  480/dscale=492 -(70+2+2)=  418 x dscale = 407
		LKwdlgConfig=405;
		break;
	case (ScreenSize_t)ss320x240:
		GestureSize=50;
		LKVarioSize=20;
		// dscale=240/240=1  320/dscale=320 -(70+2+2)=  246 x dscale = 246
		// but 246 is too long..
		LKwdlgConfig=244;
		break;
	// PORTRAIT MODES
	case (ScreenSize_t)ss480x640:
		GestureSize=50;
		LKVarioSize=30;
		break;
	case (ScreenSize_t)ss480x800:
		GestureSize=50;
		LKVarioSize=30;
		// dscale=240/240=1  400/dscale=400 -(70+2+2)=  326 x dscale = 326
		LKwdlgConfig=324;
		break;
	case (ScreenSize_t)ss240x320:
		GestureSize=50;
		LKVarioSize=15;
		break;
	default:
		GestureSize=50;
		LKVarioSize=30;
		break;
  }
}


// colorcode is taken from a 5 bit AsInt union
void MapWindow::TextColor(HDC hDC, short colorcode) {

	switch (colorcode) {
	case TEXTBLACK: 
		if (BlackScreen) // 091109
		  SetTextColor(hDC,RGB_WHITE);  // black 
		else
		  SetTextColor(hDC,RGB_BLACK);  // black 
	  break;
	case TEXTWHITE: 
		if (BlackScreen) // 091109
		  SetTextColor(hDC,RGB_LIGHTYELLOW);  // white
		else
		  SetTextColor(hDC,RGB_WHITE);  // white
	  break;
	case TEXTGREEN: 
	  SetTextColor(hDC,RGB_GREEN);  // green
	  break;
	case TEXTRED:
	  SetTextColor(hDC,RGB_RED);  // red
	  break;
	case TEXTBLUE:
	  SetTextColor(hDC,RGB_BLUE);  // blue
	  break;
	case TEXTYELLOW:
	  SetTextColor(hDC,RGB_YELLOW);  // yellow
	  break;
	case TEXTCYAN:
	  SetTextColor(hDC,RGB_CYAN);  // cyan
	  break;
	case TEXTMAGENTA:
	  SetTextColor(hDC,RGB_MAGENTA);  // magenta
	  break;
	case TEXTLIGHTGREY: 
	  SetTextColor(hDC,RGB_LIGHTGREY);  // light grey
	  break;
	case TEXTGREY: 
	  SetTextColor(hDC,RGB_GREY);  // grey
	  break;
	case TEXTLIGHTGREEN:
	  SetTextColor(hDC,RGB_LIGHTGREEN);  //  light green
	  break;
	case TEXTLIGHTRED:
	  SetTextColor(hDC,RGB_LIGHTRED);  // light red
	  break;
	case TEXTLIGHTYELLOW:
	  SetTextColor(hDC,RGB_LIGHTYELLOW);  // light yellow
	  break;
	case TEXTLIGHTCYAN:
	  SetTextColor(hDC,RGB_LIGHTCYAN);  // light cyan
	  break;
	case TEXTORANGE:
	  SetTextColor(hDC,RGB_ORANGE);  // orange
	  break;
	case TEXTLIGHTORANGE:
	  SetTextColor(hDC,RGB_LIGHTORANGE);  // light orange
	  break;
	case TEXTLIGHTBLUE:
	  SetTextColor(hDC,RGB_LIGHTBLUE);  // light blue
	  break;
	default:
	  SetTextColor(hDC,RGB_MAGENTA);  // magenta so we know it's wrong: nobody use magenta..
	  break;
	}

}


#ifdef PNA
// VENTA-ADDON MODELTYPE

//
//	Check if the model type is encoded in the executable file name
//
//  GlobalModelName is a global variable, shown during startup and used for printouts only.
//  In order to know what model you are using, GlobalModelType is used.
// 
//  This "smartname" facility is used to override the registry/config Model setup to force
//  a model type to be used, just in case. The model types may not follow strictly those in
//  config menu, nor be updated. Does'nt hurt though.
//
void SmartGlobalModelType() {

	GlobalModelType=MODELTYPE_PNA;	// default for ifdef PNA by now!

	if ( GetGlobalModelName() ) 
	{
		ConvToUpper(GlobalModelName);
	
		if ( !_tcscmp(GlobalModelName,_T("PNA"))) {
					GlobalModelType=MODELTYPE_PNA_PNA;
					_tcscpy(GlobalModelName,_T("GENERIC") );
		}
		else 
			if ( !_tcscmp(GlobalModelName,_T("HP31X")))	{
					GlobalModelType=MODELTYPE_PNA_HP31X;
			}
		else	
			if ( !_tcscmp(GlobalModelName,_T("PN6000"))) {
					GlobalModelType=MODELTYPE_PNA_PN6000;
			}
		else	
			if ( !_tcscmp(GlobalModelName,_T("MIO"))) {
					GlobalModelType=MODELTYPE_PNA_MIO;
			}
		else
			if ( !_tcscmp(GlobalModelName,_T("FUNTREK"))) {
					GlobalModelType=MODELTYPE_PNA_FUNTREK;
			}
		else
			if ( !_tcscmp(GlobalModelName,_T("ROYALTEK3200"))) {
					GlobalModelType=MODELTYPE_PNA_ROYALTEK3200;
			}
		else
			_tcscpy(GlobalModelName,_T("UNKNOWN") );
	} else	
		_tcscpy(GlobalModelName, _T("UNKNOWN") );
}


// Parse a MODELTYPE value and set the equivalent model name. 
// If the modeltype is invalid or not yet handled, assume that
// the user changed it in the registry or in the profile, and 
// correct the error returning false: this will force a Generic Type.

bool SetModelName(DWORD Temp) {
  switch (Temp) {
  case MODELTYPE_PNA_PNA:
    _tcscpy(GlobalModelName,_T("GENERIC")); 
    return true;
    break;
  case MODELTYPE_PNA_HP31X:
    _tcscpy(GlobalModelName,_T("HP31X"));
    return true;
    break;
  case MODELTYPE_PNA_PN6000:
    _tcscpy(GlobalModelName,_T("PN6000"));
    return true;
  case MODELTYPE_PNA_MIO:
    _tcscpy(GlobalModelName,_T("MIO"));
    return true;
  case  MODELTYPE_PNA_MEDION_P5:
    _tcscpy(GlobalModelName,_T("MEDION P5"));
    return true;
  case MODELTYPE_PNA_NOKIA_500:
    _tcscpy(GlobalModelName,_T("NOKIA500"));
    return true;
  case MODELTYPE_PNA_NAVIGON:
    _tcscpy(GlobalModelName,_T("NAVIGON"));
    return true;
  case MODELTYPE_PNA_FUNTREK:
    _tcscpy(GlobalModelName,_T("FUNTREK"));
    return true;
  case MODELTYPE_PNA_ROYALTEK3200:
    _tcscpy(GlobalModelName,_T("ROYALTEK3200"));
    return true;
  default:
    _tcscpy(GlobalModelName,_T("UNKNOWN"));
    return false;
  }

}

#endif



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
//    StartupStore(TEXT("CRITIC- gmfpathname returned null GetModuleFileName\n")); // rob bughunt
    return(_T("\\ERROR_01\\") );
  }
  if (gmfpathname_buffer[0] != '\\' ) {
//   StartupStore(TEXT("CRITIC- gmfpathname starting without a leading backslash\n"));
    return(_T("\\ERROR_02\\"));
  }
  gmfpathname_buffer[MAXPATHBASENAME-1] = '\0';	// truncate for safety
  
  for (p=gmfpathname_buffer+1; *p != '\0'; p++)
    if ( *p == '\\' ) break;	// search for the very first "\"
  
  if ( *p == '\0') {
//    StartupStore(TEXT("CRITIC- gmfpathname no backslash found\n"));
    return(_T("\\ERROR_03\\"));
  }
  *++p = '\0';
  
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
//    StartupStore(TEXT("++++++ CRITIC- gmfbasename returned null GetModuleFileName%s"),NEWLINE); // 091119
    return(_T("ERROR_04") );
  }
  if (gmfbasename_buffer[0] != '\\' ) {
//    StartupStore(TEXT("++++++ CRITIC- gmfbasename starting without a leading backslash%s"),NEWLINE); // 091119
    return(_T("ERROR_05"));
  }
  for (p=gmfbasename_buffer+1, lp=NULL; *p != '\0'; p++)
    {
      if ( *p == '\\' ) {
	lp=++p;
	continue;
      }
    }
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
    return(_T("ERROR_04") );
  }
  for (p=gmfbasename_buffer+1, lp=NULL; *p != '\0'; p++)
    {
      if ( *p == '\\' ) {
	lp=++p;
	continue;
      }
    }

  *lp=_T('\0');
  *--lp=_T('\0'); // also remove trailing backslash
  return  gmfbasename_buffer;
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
  
  if (GetModuleFileName(NULL, modelname_buffer, MAXPATHBASENAME) <= 0) {
    StartupStore(TEXT("++++++ CRITIC- GetGlobalFileName returned NULL%s"),NEWLINE); // 091119
    return 0;
  }
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
    } // assuming \sd\path\xcsoar_pna.exe  we are now at \xcsoar..
  
  for (p=lp, np=NULL; *p != '\0'; p++)
    {
      if (*p == '_' ) {
	np=++p;
	break;
      }
    } // assuming xcsoar_pna.exe we are now at _pna..
  
  if ( np == NULL ) {
    return 0;	// VENTA2-bugfix , null deleted
  }
  
  for ( p=np, lp=NULL; *p != '\0'; p++) 
    {
      if (*p == '.' ) {
	lp=p;
	break;
      }
    } // we found the . in pna.exe
  
  if (lp == NULL) return 0; // VENTA2-bugfix null return
  *lp='\0'; // we cut .exe
  
  _tcscpy(GlobalModelName, np);
  
  return 1;  // we say ok
  
}


/*
 * Convert to uppercase a TCHAR array
 */
void ConvToUpper( TCHAR *str )
{
	if ( str )
	{
		for ( ; *str; ++str )
		*str = towupper(*str);

	}

	return ;
}


#ifdef PNA 	
/* 
 * SetBacklight for PNA devices. There is no standard way of managing backlight on CE,
 * and every device may have different value names and settings. Microsoft did not set 
 * a standard and thus we need a custom solution for each device.
 * But the approach is always the same: change a value and call an event.
 */
bool SetBacklight() // VENTA4
{
  HKEY    hKey;
  DWORD   Disp=0;
  HRESULT hRes;
  HANDLE BLEvent;

  if (EnableAutoBacklight == false ) return false;


  switch (GlobalModelType)
  {
	case MODELTYPE_PNA_HP31X:

 		hRes = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ControlPanel\\Backlight"), 0,  0, &hKey);
 		if (hRes != ERROR_SUCCESS) return false;

		Disp=20; // max backlight
		// currently we ignore hres, if registry entries are spoiled out user is already in deep troubles
		hRes = RegSetValueEx(hKey, _T("BackLightCurrentACLevel"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("BackLightCurrentBatteryLevel"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("TotalLevels"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=0;
		hRes = RegSetValueEx(hKey, _T("UseExt"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		RegDeleteValue(hKey,_T("ACTimeout"));
  		RegCloseKey(hKey);
		BLEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("BacklightChangeEvent")); 
		if ( SetEvent(BLEvent) == 0)
			return false;
		else
			CloseHandle(BLEvent);

		break;

	case MODELTYPE_PNA_ROYALTEK3200:

 		hRes = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ControlPanel\\Backlight"), 0,  0, &hKey);
 		if (hRes != ERROR_SUCCESS) return false;

		// currently we ignore hres, if registry entries are spoiled out user is already in deep troubles

		Disp=0; // disable timeouts
		hRes = RegSetValueEx(hKey, _T("ACTimeout"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("BatteryTimeout"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("AutoChageBrightness"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));

		Disp=60; // max backlight
		hRes = RegSetValueEx(hKey, _T("BattBrightness"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("ACBrightness"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=0;
  		RegCloseKey(hKey);
		BLEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("BacklightChangeEvent")); 
		if ( SetEvent(BLEvent) == 0)
			return false;
		else
			CloseHandle(BLEvent);

		break;

	case MODELTYPE_PNA_FUNTREK:

		GM130MaxBacklight();

		break;
	default:
		return false;
		break;
  }

  #if TESTBENCH
  StartupStore(_T("...... Backlight set ok!\n"));
  #endif
  return true;

}

bool SetSoundVolume() // VENTA4
{

  if (EnableAutoSoundVolume == false ) return false;

  switch (GlobalModelType)
  {
	#if 0 // does not work, no idea why - paolo
	case MODELTYPE_PNA_HP31X:
		HKEY    hKey;
		DWORD   Disp=0;
		HRESULT hRes;

		hRes = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ControlPanel\\Volume"), 0,  0, &hKey);
		if (hRes != ERROR_SUCCESS) return false;

		Disp=0xFFFFFFFF; // max volume
		hRes = RegSetValueEx(hKey, _T("Volume"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=65538;
		hRes = RegSetValueEx(hKey, _T("Screen"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=0;
		hRes = RegSetValueEx(hKey, _T("Key"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=7;
		hRes = RegSetValueEx(hKey, _T("Mute"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0xF2, 0); 
	        RegCloseKey(hKey); 
		break;
	#endif // disabled code for HP314

	case MODELTYPE_PNA_FUNTREK:
		GM130MaxSoundVolume();
		break;

	default:
		// A general approach normally working fine.
		// (should we enter critical section ?  probably... )
		waveOutSetVolume(0, 0xffff); // this is working for all platforms
		break;
  }


  return true;
}


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

  _stprintf(tbuf,_T(". InstallSystem: copy DLL from <%s> to <%s>%s"), srcdir, dstdir,NEWLINE);
  StartupStore(tbuf);

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
	StartupStore(_T(". InstallSystem source directory <%s> is available%s"),srcdir,NEWLINE);
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
	StartupStore(_T("------ WARNING: NO DLL install available (and thus NO G-RECORD FOR VALIDATING IGC FILES)%s"),NEWLINE);
	StartupStore(_T("------ WARNING: NO font will be installed on device (and thus wrong text size displayed)%s"),NEWLINE);
	return 5; // 091109
  } else {
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


  _stprintf(tbuf,_T(". InstallSystem: Copy Fonts from <%s> to <%s>%s"), srcdir, dstdir,NEWLINE);
  StartupStore(tbuf);
  // on PNAs sometimes FolderPath is reported correctly, but the directory is not existing!
  // this is not needed really on PNA, but doesnt hurt
  CreateDirectory(dstdir, NULL); // 100820


  // we cannot check directory existance without the risk of hanging for many seconds
  // we can only rely on singe real file existance, not on directories

  #if ALPHADEBUG
  StartupStore(_T(". Checking TAHOMA font%s"),NEWLINE);
  #endif
  _stprintf(srcfile,TEXT("%s\\TAHOMA.TTF"),srcdir);
  _stprintf(dstfile,TEXT("%s\\TAHOMA.TTF"),dstdir);
  if (  GetFileAttributes(dstfile) != 0xffffffff ) {
	StartupStore(_T(". Font TAHOMA.TTF is already installed%s"),NEWLINE);
  } else {
	if ( !CopyFile(srcfile,dstfile,TRUE))  {
		StartupStore(_T("------ Could not copy TAHOMA.TTF on device, not good.%s"),NEWLINE);
		StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
	} else
		StartupStore(_T("... Font TAHOMA.TTF installed on device%s"),NEWLINE);
  }

  // not needed, cannot overwrite tahoma while in use! Tahoma bold not used for some reason in this case.
  // Problem solved, look at FontPath !!

  #if ALPHADEBUG
  StartupStore(_T(". Checking TAHOMABD font%s"),NEWLINE);
  #endif
  _stprintf(srcfile,TEXT("%s\\TAHOMABD.TTF"),srcdir);
  _stprintf(dstfile,TEXT("%s\\TAHOMABD.TTF"),dstdir);
  if (  GetFileAttributes(dstfile) != 0xffffffff ) {
	StartupStore(_T(". Font TAHOMABD.TTF is already installed%s"),NEWLINE);
  } else {
	if ( !CopyFile(srcfile,dstfile,TRUE))  {
		StartupStore(_T("------ Could not copy TAHOMABD.TTF on device, not good.%s"),NEWLINE);
		StartupStore(_T("------ Error code was: %ld%s"),GetLastError(),NEWLINE);
	} else
		StartupStore(_T("... Font TAHOMABD.TTF installed on device%s"),NEWLINE);
  }

  #if ALPHADEBUG
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
	_stprintf(srcpath,_T("%s\\%s"),srcpath,XCSPROFILE); // 091101
	if (  GetFileAttributes(srcpath) == 0xffffffff) return false;
	return true;
}


int roundupdivision(int a, int b) {
   int c=a / b;
   if ( (a%b) >0) return ++c;
   else return c;
}

#ifdef CPUSTATS
// Warning this is called by several concurrent threads, no static variables here
void Cpustats(int *accounting, FILETIME *kernel_old, FILETIME *kernel_new, FILETIME *user_old, FILETIME *user_new) {
   __int64 knew=0, kold=0, unew=0, uold=0;
   int total=2; // show evident problem

   knew = kernel_new->dwHighDateTime;
   knew <<= 32;
   knew += kernel_new->dwLowDateTime;
    
   unew=user_new->dwHighDateTime;
   unew <<=32;
   unew+=user_new->dwLowDateTime;

   kold = kernel_old->dwHighDateTime;
   kold <<= 32;
   kold += kernel_old->dwLowDateTime;
   
   uold=user_old->dwHighDateTime;
   uold <<=32;
   uold+=user_old->dwLowDateTime;

#if (WINDOWSPC>0)   
   total = (int) ((knew+unew-kold-uold)/10.0);
   //if (total==0) return;
#else
   total = (int) ((knew+unew-kold-uold)/10000.0);
#endif
   *accounting=total;

}

#endif


// Get the infobox type from configuration, selecting position i
// From 1-8 auxiliaries
//     0-16 dynamic page
//
int GetInfoboxType(int i) {

	int retval = 0;
	if (i<1||i>16) return LK_ERROR;

	// it really starts from 0
	if (i<=8)
		retval = (InfoType[i-1] >> 24) & 0xff; // auxiliary
	else {
		switch ( MapWindow::mode.Fly() ) {
			case MapWindow::Mode::MODE_FLY_CRUISE:
				retval = (InfoType[i-9] >> 8) & 0xff;
				break;
			case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
				retval = (InfoType[i-9] >> 16) & 0xff;
				break;
			case MapWindow::Mode::MODE_FLY_CIRCLING:
				retval = (InfoType[i-9]) & 0xff; 
				break;
			default:
				// impossible case, show twice auxiliaries
				retval = (InfoType[i-9] >> 24) & 0xff;
				break;
		}
	}

	return min(NumDataOptions-1,retval);
}

// Returns the LKProcess index value for configured infobox (0-8) for dmCruise, dmFinalGlide, Auxiliary, dmCircling
// The function name is really stupid...
// dmMode is an enum, we simply use for commodity
int GetInfoboxIndex(int i, MapWindow::Mode::TModeFly dmMode) {
	int retval = 0;
	if (i<0||i>8) return LK_ERROR;

	switch(dmMode) {
		case MapWindow::Mode::MODE_FLY_CRUISE:
			retval = (InfoType[i-1] >> 8) & 0xff;
			break;
		case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
			retval = (InfoType[i-1] >> 16) & 0xff;
			break;
		case MapWindow::Mode::MODE_FLY_CIRCLING:
			retval = (InfoType[i-1]) & 0xff; 
			break;
		default:
			// default is auxiliary
			retval = (InfoType[i-1] >> 24) & 0xff; 
			break;
	}
	return min(NumDataOptions-1,retval);
}


double GetMacCready(int wpindex, short wpmode)
{
	if (WayPointCalc[wpindex].IsLandable) {
		if (MACCREADY>GlidePolar::SafetyMacCready) 
			return MACCREADY;
		else
			return GlidePolar::SafetyMacCready;
	}
	return MACCREADY;

}

void unicodetoascii(TCHAR *utext, int utextsize, char *atext) {

	int i,j;
	if (utextsize==0) {
		atext[0]=0;
		return;
	}

	for (i=0,j=0; i<utextsize; i++) {
		if (utext[i]==0) continue;
		atext[j++]=utext[i];
	}
	atext[j]=0;

}


void SetOverColorRef() {
  switch(OverColor) {
	case OcWhite:
		OverColorRef=RGB_WHITE;
		break;
	case OcBlack:
		OverColorRef=RGB_SBLACK;
		break;
	case OcBlue:
		OverColorRef=RGB_DARKBLUE;
		break;
	case OcGreen:
		OverColorRef=RGB_GREEN;
		break;
	case OcYellow:
		OverColorRef=RGB_YELLOW;
		break;
	case OcCyan:
		OverColorRef=RGB_CYAN;
		break;
	case OcOrange:
		OverColorRef=RGB_ORANGE;
		break;
	case OcGrey:
		OverColorRef=RGB_GREY;
		break;
	case OcDarkGrey:
		OverColorRef=RGB_DARKGREY;
		break;
	case OcDarkWhite:
		OverColorRef=RGB_DARKWHITE;
		break;
	case OcAmber:
		OverColorRef=RGB_AMBER;
		break;
	case OcLightGreen:
		OverColorRef=RGB_LIGHTGREEN;
		break;
	case OcPetrol:
		OverColorRef=RGB_PETROL;
		break;
	default:
		OverColorRef=RGB_MAGENTA;
		break;
  }
}


#ifdef PNA
bool LoadModelFromProfile()
{

  TCHAR tmpTbuf[MAX_PATH*2];
  char  tmpbuf[MAX_PATH*2];

  LocalPath(tmpTbuf,_T(LKD_CONF));
  _tcscat(tmpTbuf,_T("\\"));
  _tcscat(tmpTbuf,_T(XCSPROFILE));

  StartupStore(_T(". Searching modeltype inside default profile <%s>%s"),tmpTbuf,NEWLINE);

  FILE *fp=NULL;
  fp = _tfopen(tmpTbuf, _T("rb"));
  if(fp == NULL) {
	StartupStore(_T("... No default profile found%s"),NEWLINE);
	return false;
  }

  while (fgets(tmpbuf, (MAX_PATH*2)-1, fp) != NULL ) {

	if (strlen(tmpbuf)<21) continue;

	if (strncmp(tmpbuf,"AppInfoBoxModel",15) == 0) {
		int val=atoi(&tmpbuf[16]);
		GlobalModelType=val;
		SetModelName(val);
		StartupStore(_T("... ModelType found: <%s> val=%d%s"),GlobalModelName,GlobalModelType,NEWLINE);
		fclose(fp);
		return true;
	}
  }

  StartupStore(_T(". Modeltype not found in profile, probably Generic PNA is used.\n"));
  fclose(fp);
  return false;
}
#endif

#ifdef PNA
void CreateRecursiveDirectory(TCHAR *fullpath)
{
  TCHAR tmpbuf[MAX_PATH];
  TCHAR *p;
  TCHAR *lastslash;
  bool found;
  
  if ( _tcslen(fullpath) <10 || _tcslen(fullpath)>=MAX_PATH) {
	StartupStore(_T("... FontPath too short or too long, cannot create folders%s"),NEWLINE);
	return;
  }

  if (*fullpath != '\\' ) {
	StartupStore(TEXT("... FontPath <%s> has no leading backslash, cannot create folders on a relative path.%s"),fullpath,NEWLINE);
	return;
  }

  lastslash=tmpbuf;

  do {
	// we copy the full path in tmpbuf as a working copy 
	_tcscpy(tmpbuf,fullpath);
	found=false;
	// we are looking after a slash. like in /Disk/
	// special case: a simple / remaining which we ignore, because either it is the first and only (like in \)
	// or it is a trailing slash with a null following
	if (*(lastslash+1)=='\0') {
		break;
	}
	
	// no eol, so lets look for another slash, starting from the char after last
	for (p=lastslash+1; *p != '\0'; p++) {
		if ( *p == '\\' ) {
			*p='\0';
			found=true;
			lastslash=p;
			break;
		}
	}
	if (_tcscmp(tmpbuf,_T("\\Windows"))==0) {
		continue;
	}
	CreateDirectory(tmpbuf, NULL);
  } while (found);
			
  return;
}
#endif


bool CheckClubVersion() {
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcfile, _T("CLUB"));
  if (  GetFileAttributes(srcfile) == 0xffffffff ) return false;
  return true;
}

void ClubForbiddenMsg() {
  MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M503_ = "Operation forbidden on CLUB devices" 
	gettext(TEXT("_@M503_")),
	_T("CLUB DEVICE"), 
	MB_OK|MB_ICONEXCLAMATION);
        return;
}

int GetFontRenderer() { 


  switch(FontRenderer) {
	case 0:
		return CLEARTYPE_COMPAT_QUALITY;
		break;
	case 1:
		return ANTIALIASED_QUALITY;
		break;
	case 2:
		return DEFAULT_QUALITY;
		break;
	case 3:
		return NONANTIALIASED_QUALITY;
		break;
	default:
		return CLEARTYPE_COMPAT_QUALITY;
		break;
  }

}

// Are we using lockmode? What is the current status?
bool LockMode(const short lmode) {

  switch(lmode) {
	case 0:		// query availability of LockMode
		return true;
		break;

	case 1:		// query lock/unlock status
		return LockModeStatus;
		break;

	case 2:		// invert lock status
		LockModeStatus = !LockModeStatus;
		return LockModeStatus;
		break;

	case 3:		// query button is usable or not
		if (ISPARAGLIDER)
			// Positive if not flying
			return CALCULATED_INFO.Flying==TRUE?false:true;
		else return true;
		break;

	case 9:		// Check if we can unlock the screen
		if (ISPARAGLIDER) {
			// Automatic unlock
			if (CALCULATED_INFO.Flying == TRUE) {
				if ( (GPS_INFO.Time - CALCULATED_INFO.TakeOffTime)>10) {
					LockModeStatus=false;
				}
			}
		}
		return LockModeStatus;
		break;

	default:
		return false;
		break;
  }

  return 0;

}



//
// This is called by lk8000.cpp on init, only once
//
void InitCustomHardware(void) {

  #ifdef PNA
  if (GlobalModelType == MODELTYPE_PNA_FUNTREK) {
	Init_GM130();
	// if (!DeviceIsGM130) return;
	// todo set to General devicetype if Init failed
  }
  if (GlobalModelType == MODELTYPE_PNA_ROYALTEK3200) {
	Init_Royaltek3200();
  }
  #endif

  return;
}

//
// This is called by lk8000.cpp on exit, only once
//
void DeInitCustomHardware(void) {

  #ifdef PNA
  if (DeviceIsGM130) DeInit_GM130();
  if (DeviceIsRoyaltek3200) DeInit_Royaltek3200();
  #endif

  return;
}


