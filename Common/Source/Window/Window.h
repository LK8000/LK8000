/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Window.h
 * Author: Bruno de Lacheisserie
 *
 * Created on December 16, 2014, 9:55 PM
 */

#ifndef WINDOW_H
#define	WINDOW_H

#ifdef __linux__
#include "Screen/Window.hpp"
#endif

#ifdef WIN32
#include "Win32/Window.h"
#endif


#endif	/* WINDOW_H */

