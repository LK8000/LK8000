/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKBitmapSurface.h
 * Author: Bruno
 *
 * Created on 16 octobre 2014, 21:49
 */

#ifndef LKBITMAPSURFACE_H
#define	LKBITMAPSURFACE_H

#include "LKSurface.h"

class LKBitmapSurface : public LKSurface {
public:
    LKBitmapSurface();
    virtual ~LKBitmapSurface();

    LKBitmapSurface(LKSurface& Surface, unsigned width, unsigned height);
        
    virtual void Create(const LKSurface& Surface, unsigned width, unsigned height);
    virtual void Release();

    virtual void Resize(unsigned width, unsigned height);

    operator LKBitmap& () { return _hBitmap; }

#ifdef WIN32
    operator HBITMAP () { return _hBitmap; }
#endif
    
protected:
    LKBitmap _hBitmap;
    LKBitmap _oldBitmap;
};

class LKMaskBitmapSurface : public LKBitmapSurface {
public:

    virtual void Resize(unsigned width, unsigned height);
    virtual void Create(const LKSurface& Surface, unsigned width, unsigned height);
};

#endif	/* LKBITMAPSURFACE_H */

