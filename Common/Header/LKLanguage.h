/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKLanguage.h
 *
 * Created on 12 august 2014, 17:36
 */

#ifndef LKLANGUAGE_H
#define	LKLANGUAGE_H

#include <map>
#include "tchar.h"
#include "Util/tstring.hpp"

/**
 * Load must be done in main thread before all others thread start
 * Unload must be done after all other thread end
 *
 * that's avoid to add mutex to protect all string pointers
 */
void LKLoadLanguageFile();
void LKUnloadLanguageFile();

/**
 * @return list of available language files < code , names >
 */
std::map<tstring, tstring> LoadLanguageList();

/**
 * @param TextIn help token, must be "_@Hxxx_"
 * @return localized help text or help token if it is is not available.
 *
 * string is loaded from json files without cache, so to avoid useless overhead
 * don't call this function from draw thread or inside loop.
 */
tstring LKgethelptext(const TCHAR *TextIn);

/**
 * @param TextIn msg token, must be "_@Mxxx_"
 * @return localized msg text or msg token if it is not available.
 */
const TCHAR *LKGetText(const TCHAR *TextIn);

/**
 * Direct token access, with range check, faster than LKGetText
 * @return empty string if token is not found
 */
const TCHAR *MsgToken(unsigned tindex);

#endif	/* LKLANGUAGE_H */
