/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.cpp,v 8.17 2010/12/19 16:42:53 root Exp root $
*/

#include "externs.h"
#include "WaveThread.h"
#ifdef PNA
#include "Modeltype.h"
#include "LKHolux.h"
#endif
#include "DoInits.h"

#include "mmsystem.h"

extern HINSTANCE                       hInst; // The current instance

BOOL PlayResource (const TCHAR* lpName)
{
#ifdef DISABLEAUDIO
  return false;
#else
  #ifdef PNA
  if (DeviceIsGM130) {
	MessageBeep(0xffffffff);
	return true;
  }
  #endif
  BOOL bRtn;
  LPTSTR lpRes;
  HANDLE hResInfo, hRes;

  // TODO code: Modify to allow use of WAV Files and/or Embedded files

  if (_tcsstr(lpName, TEXT(".wav"))) {
    bRtn = sndPlaySound (lpName, SND_ASYNC | SND_NODEFAULT ); 

  } else {
    
    // Find the wave resource.
    hResInfo = FindResource (hInst, lpName, TEXT("WAVE")); 
    
    if (hResInfo == NULL) 
      return FALSE; 
    
    // Load the wave resource. 
    hRes = LoadResource (hInst, (HRSRC)hResInfo); 
    
    if (hRes == NULL) 
      return FALSE; 
    
    // Lock the wave resource and play it. 
    lpRes = (LPTSTR)LockResource ((HGLOBAL)hRes);
    
    if (lpRes != NULL) 
      { 
	bRtn = sndPlaySound (lpRes, SND_MEMORY | SND_ASYNC | SND_NODEFAULT ); 
      } 
    else 
      bRtn = 0;
  }
  return bRtn; 
#endif
}



// Play a sound from filesystem
void LKSound(const TCHAR *lpName) {
  #ifdef DISABLEAUDIO
  return false;
  #else

  #ifdef PNA
  if (DeviceIsGM130) {
	MessageBeep(0xffffffff); // default
	return;
  }
  #endif   

  static bool working=false;
  static TCHAR sDir[MAX_PATH];

  if (DoInit[MDI_LKSOUND]) {
	working=false;
	TCHAR srcfile[MAX_PATH];
	LocalPath(sDir,TEXT(LKD_SOUNDS));
	_stprintf(srcfile,TEXT("%s\\_SOUNDS"),sDir);
	if ( !lk::filesystem::exist(srcfile) ) {
	    StartupStore(_T("ERROR NO SOUNDS DIRECTORY CHECKFILE <%s>%s"),srcfile,NEWLINE);
		StartupStore(_T("------ LK8000 SOUNDS NOT WORKING!%s"),NEWLINE);
    } else
		working=true;
	DoInit[MDI_LKSOUND]=false;
  }

  if (!working) return;
  TCHAR sndfile[MAX_PATH];
  _stprintf(sndfile,_T("%s\\%s"),sDir,lpName);
  sndPlaySound (sndfile, SND_ASYNC| SND_NODEFAULT );
  return;

  #endif
}

#ifdef PNA
bool SetSoundVolume()
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
#endif // PNA
