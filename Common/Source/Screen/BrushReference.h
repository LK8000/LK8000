/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKBrush.h
 * Author: Max Kellermann
 */

#ifndef BRUSH_REFERENCE_H
#define	BRUSH_REFERENCE_H

#ifdef USE_GDI

#include <windows.h>

typedef HBRUSH BrushReference;

#else

class LKBrush;
typedef const LKBrush *BrushReference;

#endif

#endif
