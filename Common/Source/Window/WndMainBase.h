/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   WndMainBase.h
 * Author: Bruno de Lacheisserie
 *
 * Created on December 16, 2014, 9:59 PM
 */

#ifndef WNDMAINBASE_H
#define	WNDMAINBASE_H

#ifdef WIN32
#include "Win32/WndMainBase.h"
#endif

#ifdef __linux__
#include "Linux/WndMainBase.h"
#endif


#endif	/* WNDMAINBASE_H */

