/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKBrush.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 23 octobre 2014, 20:28
 */

#ifndef LKBRUSH_H
#define	LKBRUSH_H

#include "LKBitmap.h"
#include "Screen/Brush.hpp"

class LKColor;
class LKBitmap;


class LKBrush : public Brush {
public:
    LKBrush(LKBrush&& Brush); // tranfert ownership
    LKBrush& operator= (LKBrush&& Brush); // tranfert ownership

    explicit LKBrush(const LKColor& Color) : Brush(Color) { }

    virtual ~LKBrush();

    void Create(const LKColor& Color) { Set(Color); }
#ifdef HAVE_HATCHED_BRUSH
    void Create(const LKBitmap& Bitmap) { Set(Bitmap); }
#endif
    void Release() { Reset(); }

    operator bool() const;

#ifdef USE_GDI
public:
    LKBrush(): Brush() {}
    explicit LKBrush(HBRUSH Brush) {
        brush = Brush;
    }

    operator HBRUSH() const { return brush; }

protected:
//    HBRUSH _Brush;
#else
    LKBrush() {}
    operator const LKBrush*() const { return this; }

#endif

};

inline LKBrush::operator bool() const { return IsDefined(); }

#ifdef USE_GDI
extern const HBRUSH  LK_WHITE_BRUSH;
extern const HBRUSH  LK_BLACK_BRUSH;
extern const HBRUSH  LK_HOLLOW_BRUSH;
#else
extern const LKBrush  LK_WHITE_BRUSH;
extern const LKBrush  LK_BLACK_BRUSH;
extern const LKBrush  LK_HOLLOW_BRUSH;
#endif

#endif	/* LKBRUSH_H */
