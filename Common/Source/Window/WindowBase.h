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
#include "Win32/WndMainBase.h"
#include "Win32/WndPaint.h"
#include "Win32/WndTextEdit.h"
#include "Win32/WndTextLabel.h"
#include "Win32/WndCtrlBase.h"

typedef Window ContainerWindow;
#endif

#ifdef __linux__
#include "types.h"
#include "KeyCode.h"
#include "Screen/Window.hpp"
#include "Screen/TextWindow.hpp"
#include "Screen/SingleWindow.hpp"
#include "Linux/LKWindow.h"
#include "Linux/LKWndMainBase.h"
#include "Linux/LKWndPaint.h"
#include "Linux/LKWndTextEdit.h"
#include "Linux/LKWndTextLabel.h"
#include "Linux/LKWndCtrlBase.h"


typedef LKWndMainBase< SingleWindow > WndMainBase;
typedef LKWndTextLabel< Window > WndTextLabel;
typedef LKWndTextEdit< Window > WndTextEdit;
typedef LKWndCtrlBase< ContainerWindow > WndCtrlBase;

#endif

#endif	/* WINDOWBASE_H */

