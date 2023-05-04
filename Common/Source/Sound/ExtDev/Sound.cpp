/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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


static sound_table _sound_table; // sound_table singleton
static bool bSoundInit = false;

SoundGlobalInit::SoundGlobalInit() {
    
    bSoundInit = _sound_table.init();
    if(bSoundInit) {
        // this 2 line are needed for guarantees internal map init at startup ( before start any other thread ).
        std::string Init_resource_sound_Enum = EnumString<resource_sound_t>::From(resource_sound_t::IDR_WAV_CLICK);
        std::string Init_sound_code_Enum = EnumString<sound_code_t>::From(sound_code_t::DEFAULT);
    }    
}
  
SoundGlobalInit::~SoundGlobalInit() {
    
    bSoundInit = false;
}

bool IsSoundInit() {
    
    return(bSoundInit);
}

bool SetSoundVolume() {
    // TODO : need to implement ?
    return false;
}
  
static
bool ExtSound(const TCHAR *lpName) {
    if(!lpName || !bSoundInit || !EnableSoundModes) {
        return false;
    }    

    for (auto& Config : PortConfig) {
        if (Config.UseExtSound) {
            return true;
        }
    }
    return false;
}

static
void PlayExtSound(sound_code_t sound_code) {
    const tstring& nmeaStr = _sound_table.getNmeaStr(sound_code);
    if (!nmeaStr.empty()) {
        for(unsigned i = 0; i < NUMDEV; ++i) {
            if (PortConfig[i].UseExtSound) {
                devWriteNMEAString(&DeviceList[i], nmeaStr.c_str());
            }
        }
    }   
}

void LKSound(const TCHAR *lpName) {
    TCHAR *ptrExt;
    TCHAR soundFileStr[100];
    LKASSERT(lpName);

    if(!ExtSound(lpName)) {
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

    sound_code_t sound_code;
    const bool bResult = EnumString<sound_code_t>::To( sound_code, to_utf8(soundFileStr));
    
    if (!bResult) {
        // No sound found, take default sound
        sound_code = sound_code_t::DEFAULT;
    }
    
    PlayExtSound(sound_code);
}


void PlayResource (const TCHAR* lpName) {

    LKASSERT(lpName);

    if(!ExtSound(lpName)) {
        return;
    }

    resource_sound_t resource_sound = resource_sound_t::IDR_WAV_TONE1;
    sound_code_t sound_code = sound_code_t::DEFAULT;
    const bool bResult = EnumString<resource_sound_t>::To( resource_sound, to_utf8(lpName));
    
    if (bResult) {
        // transform resource to sound_code
        // Cast is possible because resource enum is made with sound code enum, see resource_sound.h
        sound_code = static_cast<sound_code_t>(resource_sound);
    }
    
    PlayExtSound(sound_code);
}

