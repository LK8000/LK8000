/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndCtrlBase.h
 * Author: Bruno de Lacheisserie
 *
 * Created on December 16, 2014, 9:50 PM
 */

#ifndef WNDCTRLBASE_H
#define	WNDCTRLBASE_H

#ifdef __linux__
#include "Linux/WndCtrlBase.h"
#endif

#ifdef WIN32
#include "Win32/WndCtrlBase.h"
#endif

#endif	/* WNDCTRLBASE_H */

