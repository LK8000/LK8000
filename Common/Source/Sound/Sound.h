/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Sound.h
 * Author: Bruno de Lacheisserie
 *
 * Created on January 29, 2015, 10:11 PM
 */

#ifndef SOUND_H
#define	SOUND_H

#include <tchar.h>

class SoundGlobalInit {
public:
  SoundGlobalInit();
  ~SoundGlobalInit();
};

bool IsSoundInit();

bool SetSoundVolume();

void LKSound(const TCHAR *lpName);
void PlayResource (const TCHAR* lpName);

#if defined(DISABLEAUDIO) && defined(DISABLEEXTAUDIO)
// For external device, sounds can be possible by NMEA sentences

inline SoundGlobalInit::SoundGlobalInit() {}
inline SoundGlobalInit::~SoundGlobalInit() {}

inline bool IsSoundInit() { return false; }

inline bool SetSoundVolume() { return false; }

inline void LKSound(const TCHAR *lpName) { }
inline void PlayResource (const TCHAR* lpName) { }
#endif

#endif	/* SOUND_H */

