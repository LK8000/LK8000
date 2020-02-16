/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKLanguage.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 12 août 2014, 17:36
 */

#ifndef LKLANGUAGE_H
#define	LKLANGUAGE_H

#include <map>
#include "tchar.h"
#include "Util/tstring.hpp"

void LKReadLanguageFile();
void LKUnloadMessage();

std::map<tstring, tstring> LoadLanguageList();

tstring LKgethelptext(const TCHAR *TextIn);
const TCHAR *LKGetText(const TCHAR *TextIn);
const TCHAR *MsgToken(unsigned tindex);

#endif	/* LKLANGUAGE_H */
