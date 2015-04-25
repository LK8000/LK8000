/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Sound.h
 * Author: Jack
 *
 * Created on January 29, 2015, 10:11 PM
 */
#include "../Sound.h"
#include "externs.h"
#include <map>
#include "Util/ConstBuffer.hpp"
#include "resource_data.h"
#include "LKAssert.h"
#include "utils/EnumString.h"
#include "sound_table.h"
#include "resource_sound.h"


static bool bSoundInit = false; // this is true only if #SDL_Mixer is initialized.

SoundGlobalInit::SoundGlobalInit() {

    sound_table::init();
    bSoundInit = true;
}
  
SoundGlobalInit::~SoundGlobalInit() {
    
    bSoundInit = false;
}

bool SetSoundVolume() {
    // TODO : need to implement ?
    return false;
}
  
void LKSound(const TCHAR *lpName) {
    TCHAR *ptrExt;
    TCHAR soundFileStr[100];
    LKASSERT(lpName);

    if(!lpName || !bSoundInit || !EnableSoundModes || (!UseExtSound1 && !UseExtSound2)) {
        return;
    }
    
    // Suppress file name extension
    _tcscpy(soundFileStr,lpName);
    ptrExt = _tcsrchr(soundFileStr, '.');
    if (ptrExt==NULL) {
        // malformed sound file name
        return;
    }
    *ptrExt=0;

    std::tstring nmeaStr;
    sound_code_t sound_code;
    const bool bResult = EnumString<sound_code_t>::To( sound_code, soundFileStr );
    
    if (!bResult) {
        // No sound found, take default sound
        sound_code = sound_code_t::DEFAULT;
    }
    
    nmeaStr = sound_table::getNmeaStr(sound_code);

    if (!nmeaStr.empty()) {
        if (UseExtSound1) {
            devWriteNMEAString(devA(), nmeaStr.data());
        }
        if (UseExtSound2) {
            devWriteNMEAString(devB(), nmeaStr.data());
        }
    }
        
}

void PlayResource (const TCHAR* lpName) {

    LKASSERT(lpName);

    if(!lpName || !bSoundInit || !EnableSoundModes || (!UseExtSound1 && !UseExtSound2)) {
        return;
    }

    std::tstring nmeaStr;
    resource_sound_t resource_sound = resource_sound_t::IDR_WAV_TONE1;
    sound_code_t sound_code = sound_code_t::DEFAULT;
    const bool bResult = EnumString<resource_sound_t>::To( resource_sound, lpName );
    
    if (bResult) {
        // transform resource to sound_code
        // Cast is possible because resource enum is made with sound code enum, see resource_sound.h
        sound_code = static_cast<sound_code_t>(resource_sound);
    }
    
    nmeaStr = sound_table::getNmeaStr(sound_code);

    if (!nmeaStr.empty()) {
        if (UseExtSound1) {
            devWriteNMEAString(devA(), nmeaStr.data());
        }
        if (UseExtSound2) {
            devWriteNMEAString(devB(), nmeaStr.data());
        }
    }
        
}

