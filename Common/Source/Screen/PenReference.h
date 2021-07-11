/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKPen.h
 * Author: Max Kellermann
 */

#ifndef PEN_REFERENCE_H
#define	PEN_REFERENCE_H

#ifdef USE_GDI
#include <windows.h>
typedef HPEN PenReference;
#else
class LKPen;
typedef const LKPen *PenReference;
#endif

#endif
