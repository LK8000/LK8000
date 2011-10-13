/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.cpp,v 8.17 2010/12/19 16:42:53 root Exp root $
*/

#include "StdAfx.h"

#include "options.h"
#include "Defines.h"
#include "externs.h"
#include "lk8000.h"
#include "WaveThread.h"
#ifdef PNA
#include "LKHolux.h"
#endif

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
  static bool doinit=true;
  static bool working=false;
  static TCHAR sDir[MAX_PATH];

  if (doinit) {
	TCHAR srcfile[MAX_PATH];
	LocalPath(sDir,TEXT(LKD_SOUNDS));
	_stprintf(srcfile,TEXT("%s\\_SOUNDS"),sDir);
	if (  GetFileAttributes(srcfile) == 0xffffffff ) {
	        FailStore(_T("ERROR NO SOUNDS DIRECTORY CHECKFILE <%s>%s"),srcfile,NEWLINE);
		StartupStore(_T("------ LK8000 SOUNDS NOT WORKING!%s"),NEWLINE);
        } else
		working=true;
	doinit=false;
  }

  if (!working) return;
  TCHAR sndfile[MAX_PATH];
  _stprintf(sndfile,_T("%s\\%s"),sDir,lpName);
  sndPlaySound (sndfile, SND_ASYNC| SND_NODEFAULT );
  return;

  #endif
}

