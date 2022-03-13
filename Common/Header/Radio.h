/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: Radio.h,v 1.1 2020/22/12 
 */

#ifndef __RADIO_H__
#define __RADIO_H__

#include "Sizes.h"

bool ValidFrequency(unsigned khz);

unsigned ExtractFrequency(const TCHAR *text, size_t *start = nullptr, size_t *len = nullptr);

bool UpdateStationName(TCHAR (&Name)[NAME_SIZE + 1], unsigned khz);

int SearchBestStation();

const TCHAR* GetActiveStationSymbol(bool unicode_symbol);
const TCHAR* GetStandyStationSymbol(bool unicode_symbol);

#endif
