/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndPaint.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 17 novembre 2014, 23:21
 */

#ifndef WNDPAINT_H
#define	WNDPAINT_H
#include "LKWindow.h"

class LKSurface;

template<class _Base>
class WndPaint : public LKWindow<_Base> {
public:
    WndPaint() = default;

protected:
    virtual bool OnPaint(LKSurface& Surface, const RECT& Rect) = 0;
};

#endif	/* WNDPAINT_H */
