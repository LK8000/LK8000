/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKIcon.h
 * Author: Bruno de Lacheisserie
 *
 * Created on June 19, 2015, 11:07 PM
 */

#ifndef LKICON_H
#define	LKICON_H

#include <utility>
#include "Screen/Point.hpp"
#include "LKBitmap.h"

class LKSurface;

class LKIcon {
public:
    LKIcon() : _size() {}
    ~LKIcon() {}

	LKIcon& operator=(LKBitmap&& orig);

    LKIcon(LKIcon&& orig) = delete;
	LKIcon(const LKIcon& orig) = delete;
	LKIcon& operator=(LKIcon&& orig) = delete;

    inline
	void Release() { 
        _bitmap.Release(); 
    }
    
    void Draw(LKSurface& Surface, const int x, const int y, const int cx, const int cy) const;

    void Draw(LKSurface& Surface, const RasterPoint& origin, const PixelSize& size) const {
        Draw(Surface, origin.x, origin.y, size.cx, size.cy);
    }

    void Draw(LKSurface& Surface, const PixelRect& rect) const {
        Draw(Surface, rect.GetOrigin(), rect.GetSize());
    }
    
    inline
    PixelSize GetSize() const { 
        assert(_bitmap.IsDefined());
        return _size; 
    }
    
    inline
    operator bool() const { 
        return _bitmap; 
    }
    
    bool LoadFromResource(const TCHAR* ResourceName);
    
private:
    LKBitmap _bitmap;
    PixelSize _size;
};

#endif	/* LKICON_H */
