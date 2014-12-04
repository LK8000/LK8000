/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WindowBase.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 9 novembre 2014, 14:08
 */

#ifndef WINDOWBASE_H
#define	WINDOWBASE_H

#ifdef WIN32
#include "Win32/Window.h"
#include "Win32/EventLoop.h"
#include "Win32/WndMainBase.h"
#include "Win32/WndPaint.h"
#include "Win32/WndTextEdit.h"
#include "Win32/WndTextLabel.h"
#include "Win32/WndCtrlBase.h"
#endif

#ifdef __linux__
#include "types.h"
#include "KeyCode.h"
#include "Linux/Window.h"
#include "Linux/EventLoop.h"
#include "Linux/WndMainBase.h"
#include "Linux/WndPaint.h"
#include "Linux/WndTextEdit.h"
#include "Linux/WndTextLabel.h"
#include "Linux/WndCtrlBase.h"
#endif

#endif	/* WINDOWBASE_H */

