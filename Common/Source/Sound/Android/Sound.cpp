/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Sound.h
 * Author: Tonino Tarsi
 *
 * Created on Dicember 27, 2016, 10:11 AM
 */
#include "../Sound.h"
#include "externs.h"
#include "Android/SoundUtil.hpp"
#include "Android/Context.hpp"
#include "Android/Main.hpp"
/*
 * Sound can be play from more than one thread, so we need use mutex for protect audioChunkCache.
 */
static Mutex mutex_sound;


static const struct {
    const TCHAR* szName;
    const TCHAR* szFile;
} _resourceToWav[] = {
        { _T("IDR_WAV_MM0"), _T("MM0.WAV") },
        { _T("IDR_WAV_MM1"), _T("MM1.WAV") },
        { _T("IDR_WAV_MM2"), _T("MM2.WAV") },
        { _T("IDR_WAV_MM3"), _T("MM3.WAV") },
        { _T("IDR_WAV_MM4"), _T("MM4.WAV") },
        { _T("IDR_WAV_MM5"), _T("MM5.WAV") },
        { _T("IDR_WAV_MM6"), _T("MM6.WAV") },
        { _T("IDR_WAV_DRIP"), _T("LKbeep-drip.wav") },
        { _T("IDR_WAV_CLICK"), _T("LK_SHORTERCLICKM.WAV") },
        { _T("IDR_WAV_HIGHCLICK"), _T("LK_CLICKH.WAV") },
        { _T("IDR_WAV_TONE1"), _T("LK_T1.WAV") },
        { _T("IDR_WAV_TONE2"), _T("LK_T2.WAV") },
        { _T("IDR_WAV_TONE3"), _T("LK_T3.WAV") },
        { _T("IDR_WAV_TONE4"), _T("LK_T4.WAV") },
        { _T("IDR_WAV_TONE7"), _T("LK_T8.WAV") },
        { _T("IDR_WAV_BTONE2"), _T("LK_B2b.wav") },
        { _T("IDR_WAV_BTONE4"), _T("LK_B4.wav") },
        { _T("IDR_WAV_BTONE5"), _T("LK_B5.wav") },
        { _T("IDR_WAV_BTONE6"), _T("LK_B5b.wav") },
        { _T("IDR_WAV_BTONE7"), _T("LK_B8.wav") },
        { _T("IDR_WAV_OVERTONE0"), _T("LK_S0.WAV") },
        { _T("IDR_WAV_OVERTONE1"), _T("LK_S1.WAV") },
        { _T("IDR_WAV_OVERTONE2"), _T("LK_S2.WAV") },
        { _T("IDR_WAV_OVERTONE3"), _T("LK_S3.WAV") },
        { _T("IDR_WAV_OVERTONE4"), _T("LK_S4.WAV") },
        { _T("IDR_WAV_OVERTONE5"), _T("LK_S5.WAV") },
        { _T("IDR_WAV_OVERTONE6"), _T("LK_S6.WAV") },
        { _T("IDR_WAV_OVERTONE7"), _T("LK_S6b.WAV") },
};

static
const TCHAR* FindWave(const TCHAR* szName) {
    for (const auto &Resource : _resourceToWav) {
        if (_tcscmp(Resource.szName, szName) == 0) {
            return Resource.szFile;
        }
    }
    return nullptr;
}

SoundGlobalInit::SoundGlobalInit() {
}

  
SoundGlobalInit::~SoundGlobalInit() {
}

bool IsSoundInit() {
    return(true);
}

bool SetSoundVolume() {
    // TODO : need to implement ?
    return false;
}

  
void LKSound(const TCHAR *lpName) {
    if(!lpName || !EnableSoundModes) {
        return;
    }

    ScopeLock Lock(mutex_sound);
    if(!SoundUtil::Play(Java::GetEnv(), context->Get(), lpName)) {
        StartupStore(_T("Sound : error playing '%s'" NEWLINE), lpName);
    }
}


void PlayResource (const TCHAR* lpName) {
    LKASSERT(lpName);

    if(!lpName || !EnableSoundModes) {
        return;
    }

    const TCHAR* szName =  FindWave(lpName);
    if ( szName != nullptr ) {
        LKSound(szName);
    } else {
        StartupStore(_T("Sound : Missing resource '%s'" NEWLINE), lpName);
    }
}

