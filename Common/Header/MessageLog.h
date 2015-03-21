/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   MessageLog.h
 * Author: Bruno de Lacheisserie
 *
 * Created on March 21, 2015, 1:22 PM
 */

#ifndef MESSAGELOG_H
#define	MESSAGELOG_H

#include <tchar.h>

extern "C" {
void DebugStore(const char *Str, ...);
}

void StartupStore(const TCHAR *Str, ...);

#endif	/* MESSAGELOG_H */

