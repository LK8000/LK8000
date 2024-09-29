/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   sound_table.h
 * Author: Jack
 *
 * Created on January 29, 2015, 10:11 PM
 */


#include "externs.h"
#include "sound_table.h"
#include <functional>
#include "LocalPath.h"
using namespace std::placeholders;

void sound_table::set(sound_code_t code, const TCHAR * nmeaStr) {
    table[code] = nmeaStr;
}

/**
 * Init sound table
 * Read conversion sound table file (SOUND_TABLE.TXT): <root sound file name>=<Nmea sentence>
 * @return true if file is read
 */
bool sound_table::init() {
    
    reset(); // useless still table is initialized only one time. no impact on perf, so is better to leave it...

    TCHAR srcfile[MAX_PATH] = {};
    LocalPath(srcfile,_T(LKD_CONF), _T(LKSOUNDTABLE));
    
     FILE *fp;
	 if ( (fp=_tfopen(srcfile, _T("rt"))) == NULL ) {
		 StartupStore(_T("...Loading default Sound Table"));
		 for (int i=DEFAULT;i<last;i++) {
			 TCHAR str[200]; // Nmea string can have max (200 - soundCodeSize - 1)
			 str[std::size(str)-1] = _T('\0');  // added make sure the string is terminated
			 _stprintf(str,_T("LKALARM,%d"), i);
			 set((sound_code_t)i,str);
		 }
		 return true;
	 }



    TCHAR str[200]; // Nmea string can have max (200 - soundCodeSize - 1) 
    str[std::size(str)-1] = _T('\0');  // added make sure the string is terminated
    while ( (_fgetts(str, std::size(str)-1, fp))!=NULL) {
        if (str[0]==_T('#')) continue; // skip comments
        TrimRight(str);
        if (str[0]==_T('\0')) continue; // skip empty line

        TCHAR * ptrCode = str;
        TCHAR * ptrNmea = _tcschr(str, _T('='));
        if (ptrNmea==NULL) {
            StartupStore(_T("...Malformed line: %s"), str);
            continue;
        }
        *ptrNmea = _T('\0'); // replace '=' by '\0', now ptrCode are nts sound code
        
        sound_code_t soundCode;
        const bool bResult = EnumString<sound_code_t>::To(soundCode, to_utf8(ptrCode));
        if (!bResult) {
            // sound code not found ==> skip
            StartupStore(_T("... Cannot find sound: %s") , ptrCode);
            continue;
        }

        ptrNmea++; // advance to first Nmea string character
        size_t last = _tcslen(ptrNmea) -1;
        while(last >= 0 && (ptrNmea[last]==_T('\n') || ptrNmea[last]==_T('\r'))) {
            ptrNmea[last--] = _T('\0'); // remove end of line char
        }
        
        // Associate sound code and nmea sentence
        set(soundCode,ptrNmea);
    }

    return true;
}

void sound_table::reset() {
    std::for_each(table.begin(), table.end(), std::bind(&tstring::clear, _1) );
}

const tstring& sound_table::getNmeaStr(sound_code_t code) const {
    static const tstring empty;
    if(code > table.size()) {
        return empty;
    }
    return table[code];
}
