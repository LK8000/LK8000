/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Radio.h,v 1.1 2020/22/12 
*/

#ifndef __RADIO_H__
#define __RADIO_H__

#include "Sizes.h"

BOOL ValidFrequency(double Freq);
double ExtractFrequency(const TCHAR *text);
double ExtractFrequencyPos(const TCHAR *text, size_t *start, size_t *len);
bool UpdateStationName(TCHAR (&Name)[NAME_SIZE + 1], double Frequency);

int SearchBestStation();


#define ACTIVE_SYMBOL      _T("X")
#define ACTIVE_SYMBOL_UTF8 _T("↕")
#define STANDBY_SYMBOL      _T("v")
#define STANDBY_SYMBOL_UTF8 _T("↓")

const TCHAR* GetActiveStationSymbol(bool utf8_symbol) ;
const TCHAR* GetStandyStationSymbol(bool utf8_symbol) ;

#endif
