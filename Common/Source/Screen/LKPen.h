/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKPen.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 16 octobre 2014
 */

#ifndef LKPEN_H
#define	LKPEN_H

#include "Screen/Pen.hpp"
#include "LKColor.h"

#define PEN_SOLID Pen::SOLID
#define PEN_DASH Pen::DASH

class LKPen : public Pen {
public:
    LKPen() = default;
    LKPen(Style Style, unsigned width, const LKColor c) : Pen(Style, width, c) { }

    LKPen(LKPen&& Pen) = delete;
    LKPen& operator= (LKPen&& Pen) = delete;

    ~LKPen() {}

    void Create(Style Type, unsigned Size, const LKColor& Color) { Set(Type, Size, Color); }
    void Release() { Reset(); }

	operator bool() const { return IsDefined(); }

#ifdef USE_GDI
public:
	explicit LKPen(HPEN Pen) { pen = Pen; }
	operator HPEN() const { return Native(); }
#else
    operator const LKPen*() const { return this; }
#endif
};

#ifdef USE_GDI
extern const HPEN LK_NULL_PEN;
extern const HPEN LK_BLACK_PEN;
extern const HPEN LK_WHITE_PEN;
#else
extern const LKPen LK_NULL_PEN;
extern const LKPen LK_BLACK_PEN;
extern const LKPen LK_WHITE_PEN;
#endif
#endif	/* LKPEN_H */
