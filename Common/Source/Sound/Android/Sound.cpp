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
#include "utils/lookup_table.h"

/*
 * Sound can be play from more than one thread, so we need use mutex for protect audioChunkCache.
 */

namespace {

Mutex mutex_sound;

constexpr auto _resourceToWav = lookup_table<std::string_view, const char*>({
    {"IDR_WAV_MM0", "MM0.WAV"},
    {"IDR_WAV_MM1", "MM1.WAV"},
    {"IDR_WAV_MM2", "MM2.WAV"},
    {"IDR_WAV_MM3", "MM3.WAV"},
    {"IDR_WAV_MM4", "MM4.WAV"},
    {"IDR_WAV_MM5", "MM5.WAV"},
    {"IDR_WAV_MM6", "MM6.WAV"},
    {"IDR_WAV_DRIP", "LKbeep-drip.wav"},
    {"IDR_WAV_CLICK", "LK_SHORTERCLICKM.WAV"},
    {"IDR_WAV_HIGHCLICK", "LK_CLICKH.WAV"},
    {"IDR_WAV_TONE1", "LK_T1.WAV"},
    {"IDR_WAV_TONE2", "LK_T2.WAV"},
    {"IDR_WAV_TONE3", "LK_T3.WAV"},
    {"IDR_WAV_TONE4", "LK_T4.WAV"},
    {"IDR_WAV_TONE7", "LK_T8.WAV"},
    {"IDR_WAV_BTONE2", "LK_B2b.wav"},
    {"IDR_WAV_BTONE4", "LK_B4.wav"},
    {"IDR_WAV_BTONE5", "LK_B5.wav"},
    {"IDR_WAV_BTONE6", "LK_B5b.wav"},
    {"IDR_WAV_BTONE7", "LK_B8.wav"},
    {"IDR_WAV_OVERTONE0", "LK_S0.WAV"},
    {"IDR_WAV_OVERTONE1", "LK_S1.WAV"},
    {"IDR_WAV_OVERTONE2", "LK_S2.WAV"},
    {"IDR_WAV_OVERTONE3", "LK_S3.WAV"},
    {"IDR_WAV_OVERTONE4", "LK_S4.WAV"},
    {"IDR_WAV_OVERTONE5", "LK_S5.WAV"},
    {"IDR_WAV_OVERTONE6", "LK_S6.WAV"},
    {"IDR_WAV_OVERTONE7", "LK_S6b.WAV"},
});

} // namespace

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
    if(!lpName || !EnableSoundModes) {
        return;
    }

    const char* szName = _resourceToWav.get(lpName, nullptr);
    if ( szName != nullptr ) {
        LKSound(szName);
    } else {
        StartupStore("Sound : Missing resource '%s'", lpName);
    }
}
