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
  
bool ExtSound(void)
{
  for (int i=0 ; i < NUMDEV; i++)
    if(UseExtSound[i]) return true;
  return false;
}

void LKSound(const TCHAR *lpName) {
    TCHAR *ptrExt;
    TCHAR soundFileStr[100];
    LKASSERT(lpName);

    if(!lpName || !bSoundInit || !EnableSoundModes || (!ExtSound())
      ) {
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
    const bool bResult = EnumString<sound_code_t>::To( sound_code, soundFileStr );
    
    if (!bResult) {
        // No sound found, take default sound
        sound_code = sound_code_t::DEFAULT;
    }
    
    const tstring& nmeaStr = _sound_table.getNmeaStr(sound_code);

    if (!nmeaStr.empty()) {
        if (UseExtSound[0]) {
            devWriteNMEAString(devA(), nmeaStr.c_str());
        }
        if (UseExtSound[1]) {
            devWriteNMEAString(devB(), nmeaStr.c_str());
        }
        if (UseExtSound[2]) {
            devWriteNMEAString(devC(), nmeaStr.c_str());
        }
        if (UseExtSound[3]) {
            devWriteNMEAString(devD(), nmeaStr.c_str());
        }
        if (UseExtSound[4]) {
            devWriteNMEAString(devE(), nmeaStr.c_str());
        }
        if (UseExtSound[5]) {
            devWriteNMEAString(devF(), nmeaStr.c_str());
        }
    }
        
}


void PlayResource (const TCHAR* lpName) {

    LKASSERT(lpName);

    if(!lpName || !bSoundInit || !EnableSoundModes || (!ExtSound())) {
        return;
    }

    resource_sound_t resource_sound = resource_sound_t::IDR_WAV_TONE1;
    sound_code_t sound_code = sound_code_t::DEFAULT;
    const bool bResult = EnumString<resource_sound_t>::To( resource_sound, lpName );
    
    if (bResult) {
        // transform resource to sound_code
        // Cast is possible because resource enum is made with sound code enum, see resource_sound.h
        sound_code = static_cast<sound_code_t>(resource_sound);
    }
    
    const tstring& nmeaStr = _sound_table.getNmeaStr(sound_code);

    if (!nmeaStr.empty()) {
        if (UseExtSound[0]) {
            devWriteNMEAString(devA(), nmeaStr.c_str());
        }
        if (UseExtSound[1]) {
            devWriteNMEAString(devB(), nmeaStr.c_str());
        }
        if (UseExtSound[2]) {
            devWriteNMEAString(devC(), nmeaStr.c_str());
        }
        if (UseExtSound[3]) {
            devWriteNMEAString(devD(), nmeaStr.c_str());
        }
        if (UseExtSound[4]) {
            devWriteNMEAString(devE(), nmeaStr.c_str());
        }
        if (UseExtSound[5]) {
            devWriteNMEAString(devF(), nmeaStr.c_str());
        }
    }
        
}

