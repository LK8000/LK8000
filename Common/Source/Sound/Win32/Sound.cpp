/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.cpp,v 8.17 2010/12/19 16:42:53 root Exp root $
*/

#include "../Sound.h"
#include "externs.h"
#if defined(PNA) && defined(UNDER_CE)
#include "Modeltype.h"
#include "Devices/LKHolux.h"
#endif

#include "mmsystem.h"

static bool bSoundFile = false; // this is true only if "_System/_Sounds" directory exists.
static TCHAR szSoundPath[MAX_PATH] = {}; // path of Sound file, initialized by  #SoundGlobalInit end never change;

SoundGlobalInit::SoundGlobalInit() {
    TCHAR srcfile[MAX_PATH];
    SystemPath(szSoundPath,TEXT(LKD_SOUNDS));
    _stprintf(srcfile,TEXT("%s%s_SOUNDS"), szSoundPath, _T(DIRSEP));
    if ( lk::filesystem::exist(srcfile) ) {
        bSoundFile = true;
    } else {
        StartupStore(_T("ERROR NO SOUNDS DIRECTORY CHECKFILE <%s>%s"),srcfile,NEWLINE);
        StartupStore(_T("------ LK8000 SOUNDS NOT WORKING!%s"),NEWLINE);
    }
}

SoundGlobalInit::~SoundGlobalInit() {

}

bool IsSoundInit() {

    return(bSoundFile);
}


extern HINSTANCE _hInstance; // The current instance

void PlayResource(const TCHAR* lpName) {
    if(!EnableSoundModes) {
        return;
    }

#if defined(PNA) && defined(UNDER_CE)
    if (DeviceIsGM130) {
        MessageBeep(0xffffffff);
        return;
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
        MessageBeep(MB_ICONEXCLAMATION);
    }
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

  if (!bSoundFile) return;
  TCHAR sndfile[MAX_PATH];
  _stprintf(sndfile,_T("%s%s%s"),szSoundPath, _T(DIRSEP), lpName);
  sndPlaySound (sndfile, SND_ASYNC| SND_NODEFAULT );
}

bool SetSoundVolume() {

#if defined(PNA) && defined(UNDER_CE)
  if (EnableAutoSoundVolume == false ) return false;

  switch (ModelType::Get()) {
	case ModelType::FUNTREK:
		GM130MaxSoundVolume();
		break;

	default:
		// A general approach normally working fine.
		// (should we enter critical section ?  probably... )
		waveOutSetVolume(0, 0xffff); // this is working for all platforms
		break;
  }

  return true;
#else
  return false;
#endif
}
