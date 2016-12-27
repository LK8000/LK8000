/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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

static bool bSoundInit = false;
static bool bSoundFile = false;
std::map<tstring, tstring> _resourceToWav;
/*
 * Sound can be play from more than one thread, so we need use mutex for protect audioChunkCache.
 */
static Mutex mutex_sound;

SoundGlobalInit::SoundGlobalInit() {

    _resourceToWav["IDR_WAV_MM0"] = "MM0.WAV";
    _resourceToWav["IDR_WAV_MM1"] = "MM1.WAV";
    _resourceToWav["IDR_WAV_MM2"] = "MM2.WAV";
    _resourceToWav["IDR_WAV_MM3"] = "MM3.WAV";
    _resourceToWav["IDR_WAV_MM4"] = "MM4.WAV";
    _resourceToWav["IDR_WAV_MM5"] = "MM5.WAV";
    _resourceToWav["IDR_WAV_MM6"] = "MM6.WAV";
    _resourceToWav["IDR_WAV_DRIP"] = "LKbeep-drip.WAV";
    _resourceToWav["IDR_WAV_CLICK"] = "LK_SHORTERCLICKM.WAV";
    _resourceToWav["IDR_WAV_HIGHCLICK"] = "LK_CLICKH.WAV";
    _resourceToWav["IDR_WAV_TONE1"] = "LK_T1.WAV";
    _resourceToWav["IDR_WAV_TONE2"] = "LK_T2.WAV";
    _resourceToWav["IDR_WAV_TONE3"] = "LK_T3.WAV";
    _resourceToWav["IDR_WAV_TONE4"] = "LK_T4.WAV";
    _resourceToWav["IDR_WAV_TONE7"] = "LK_T8.WAV";
    _resourceToWav["IDR_WAV_BTONE2"] = "LK_B2b.WAV";
    _resourceToWav["IDR_WAV_BTONE4"] = "LK_B4.WAV";
    _resourceToWav["IDR_WAV_BTONE5"] = "LK_B5.WAV";
    _resourceToWav["IDR_WAV_BTONE6"] = "LK_B5b.WAV";
    _resourceToWav["IDR_WAV_BTONE7"] = "LK_B8.WAV";
    _resourceToWav["IDR_WAV_OVERTONE0"] = "LK_S0.WAV";
    _resourceToWav["IDR_WAV_OVERTONE1"] = "LK_S1.WAV";
    _resourceToWav["IDR_WAV_OVERTONE2"] = "LK_S2.WAV";
    _resourceToWav["IDR_WAV_OVERTONE3"] = "LK_S3.WAV";
    _resourceToWav["IDR_WAV_OVERTONE4"] = "LK_S4.WAV";
    _resourceToWav["IDR_WAV_OVERTONE5"] = "LK_S5.WAV";
    _resourceToWav["IDR_WAV_OVERTONE6"] = "LK_S6.WAV";
    _resourceToWav["IDR_WAV_OVERTONE7"] = "LK_S6b.WAV";

    bSoundInit = true;
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

  
void LKSound(const TCHAR *lpName) {
    ScopeLock Lock(mutex_sound);
    SoundUtil::Play(Java::GetEnv(), context->Get(), lpName);
}


void PlayResource (const TCHAR* lpName) {
    LKASSERT(lpName);

    const tstring  szWav = _resourceToWav[lpName];
    TCHAR* lpWav = new TCHAR[szWav.size()+1];
    lpWav[szWav.size()]=0;
    std::copy(szWav.begin(),szWav.end(),lpWav);
    LKSound(lpWav);
    delete lpWav;
}

