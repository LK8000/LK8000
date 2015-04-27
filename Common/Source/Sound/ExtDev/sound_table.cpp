/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   sound_table.h
 * Author: Jack
 *
 * Created on January 29, 2015, 10:11 PM
 */


#include "externs.h"
#include "sound_table.h"
#include <array>

// This array is loaded at init phase and contain association between enum sound code and nmea string
static std::array<sound_assoc_t, sound_code_t::last> table;

void sound_table::set(sound_code_t code, const std::tstring& nmeaStr) {
    
    table[code].code = code;
    table[code].nmeaStr = nmeaStr;
}

/**
 * Init sound table
 * Read conversion sound table file (SOUND_TABLE.TXT): <root sound file name>=<Nmea sentence>
 * @return true if file is read
 */
bool sound_table::init() {
    FILE *fp;
    TCHAR srcfile[MAX_PATH];
    TCHAR str[100];
    TCHAR soundCodeStr[80];
    sound_code_t soundCode;
    TCHAR *ptrNmea;
    
    _stprintf(srcfile,_T("%s%s%s%s"),LKGetLocalPath(), _T(LKD_CONF), _T(DIRSEP), _T(LKSOUNDTABLE));
    
    if ( (fp=_tfopen(srcfile, _T("r"))) == NULL ) {
	StartupStore(_T("... Cannot load conversion sound table file: %s%s"),srcfile,NEWLINE);
	return false;
    }
    
    while ( (_fgetts(str, 80, fp))!=NULL) {
	if (str[0]=='#') continue; // skip comments
        
        ptrNmea = _tcschr(str, '=');
        if (ptrNmea==NULL) {
            StartupStore(_T("Malformed line: %s%s"),str,NEWLINE);
            continue;
        }
        _tcsncpy(soundCodeStr, str, ptrNmea - str);
        soundCodeStr[ptrNmea - str] = 0;
        const bool bResult = EnumString<sound_code_t>::To(soundCode, soundCodeStr);
        if (!bResult) {
            // sound code not found ==> skip
            StartupStore(_T("... Cannot find sound: %s%s"),soundCodeStr,NEWLINE);
            continue;
        }

        ptrNmea++;
        // remove end of line char
        ptrNmea[_tcslen(ptrNmea)-1] = 0;
        
        // Associate sound code and nmea sentence
        set(soundCode,ptrNmea);
    }

    return true;
}

const std::tstring sound_table::getNmeaStr(sound_code_t code) {
    return table[code].nmeaStr;
}