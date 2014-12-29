/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.cpp,v 8.17 2010/12/19 16:42:53 root Exp root $
*/

#include "externs.h"
#if defined(PNA) && defined(UNDER_CE)
#include "Modeltype.h"
#include "LKHolux.h"
#endif
#include "DoInits.h"

#ifdef DISABLEAUDIO

BOOL PlayResource (const TCHAR* lpName) {
    return false;
}

void LKSound(const TCHAR *lpName) {
    
}

#else
#ifdef WIN32
#include "mmsystem.h"

extern HINSTANCE _hInstance; // The current instance

BOOL PlayResource(const TCHAR* lpName) {
    if(!EnableSoundModes) return true;
    
#if defined(PNA) && defined(UNDER_CE)
    if (DeviceIsGM130) {
        MessageBeep(0xffffffff);
        return true;
    }
#endif
    BOOL bRtn = false;
    // TODO code: Modify to allow use of WAV Files and/or Embedded files

    if (_tcsstr(lpName, TEXT(".wav"))) {
        bRtn = sndPlaySound(lpName, SND_ASYNC | SND_NODEFAULT);
    } else {

        // Find the wave resource.
        HRSRC hResInfo = FindResource(_hInstance, lpName, TEXT("WAVE"));
        if (hResInfo) {
            // Load the wave resource. 
            HGLOBAL hRes = LoadResource(_hInstance, hResInfo);
            if (hRes) {
                // Lock the wave resource and play it. 
                LPCTSTR lpRes = (LPCTSTR) LockResource(hRes);
                if (lpRes) {
                    bRtn = sndPlaySound(lpRes, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
                }
            }
        }
    }
    if (!bRtn) {
        bRtn = MessageBeep(MB_ICONEXCLAMATION);
    }
    return bRtn;
}



// Play a sound from filesystem
void LKSound(const TCHAR *lpName) {
    if(!EnableSoundModes) return;
    
  #if defined(PNA) && defined(UNDER_CE)
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
	_stprintf(srcfile,TEXT("%s%s_SOUNDS"), sDir, _T(DIRSEP));
	if ( !lk::filesystem::exist(srcfile) ) {
	    StartupStore(_T("ERROR NO SOUNDS DIRECTORY CHECKFILE <%s>%s"),srcfile,NEWLINE);
		StartupStore(_T("------ LK8000 SOUNDS NOT WORKING!%s"),NEWLINE);
    } else
		working=true;
	DoInit[MDI_LKSOUND]=false;
  }

  if (!working) return;
  TCHAR sndfile[MAX_PATH];
  _stprintf(sndfile,_T("%s%s%s"),sDir, _T(DIRSEP), lpName);
  sndPlaySound (sndfile, SND_ASYNC| SND_NODEFAULT );
}

#if defined(PNA) && defined(UNDER_CE)
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
#endif
#elif defined(ENABLE_SDL)
#include <SDL/SDL_audio.h>

SDL_AudioSpec spec;
Uint32 sound_len;
Uint8 *sound_buffer;
Uint8 *sound_pos = 0;

void Callback (void *userdata, Uint8 *stream, int len){
    /* Only play if we have data left */
    if ( sound_len == 0 )
        return;

    /* Mix as much data as possible */
    len = std::min((Uint32)len,sound_len);
    SDL_MixAudio(stream, sound_pos, (Uint32)len, SDL_MIX_MAXVOLUME);
    sound_pos += len;
    sound_len -= len;
}

void LKSound(const TCHAR *lpName) {
  if (IsKobo()) {
      // SDL_Init is Called without SDL_INIT_AUDIO if we are on Kobo Device
      // cf. "Sources/xcs/Screen/SDL/Init.cpp
      return; 
  }
    
  static bool working=false;
  static TCHAR sDir[MAX_PATH];

  if (DoInit[MDI_LKSOUND]) {
	working=false;
	TCHAR srcfile[MAX_PATH];
	LocalPath(sDir,TEXT(LKD_SOUNDS));
	_stprintf(srcfile,TEXT("%s%s_SOUNDS"), sDir, _T(DIRSEP));
	if ( !lk::filesystem::exist(srcfile) ) {
	    StartupStore(_T("ERROR NO SOUNDS DIRECTORY CHECKFILE <%s>%s"),srcfile,NEWLINE);
		StartupStore(_T("------ LK8000 SOUNDS NOT WORKING!%s"),NEWLINE);
    } else
		working=true;
	DoInit[MDI_LKSOUND]=false;
  }

  if (!working) return;
  TCHAR sndfile[MAX_PATH];
  _stprintf(sndfile,_T("%s%s%s"),sDir, _T(DIRSEP), lpName);
      
  if (SDL_LoadWAV (sndfile, &spec, &sound_buffer, &sound_len) == NULL) return;
  spec.callback = Callback;
  sound_pos = sound_buffer;
  if (SDL_OpenAudio (&spec, NULL) < 0)  {
      //Error message 
      return;
    }
  SDL_PauseAudio (0);    
  
  /* Wait for sound to complete */
  while ( sound_len > 0 ) {
    Poco::Thread::sleep(100);         /* Sleep 1/10 second */
  }
  SDL_CloseAudio();  
  SDL_FreeWAV(sound_buffer);
}


BOOL PlayResource (const TCHAR* lpName) {
    return false;
}

#endif
#endif // __linux__
