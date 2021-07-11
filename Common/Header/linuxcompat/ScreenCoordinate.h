/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ScreenCoordinate.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 3 novembre 2014, 22:25
 */

#ifndef SCREENCOORDINATE_H
#define	SCREENCOORDINATE_H

#include "Screen/Point.hpp"

typedef RasterPoint POINT;
typedef PixelSize SIZE;
typedef PixelRect RECT;

inline bool PtInRect( const RECT *rect, POINT pt )
{
    if (!rect) return false;
    return ((pt.x >= rect->left) && (pt.x < rect->right) &&
            (pt.y >= rect->top) && (pt.y < rect->bottom));
}

inline bool EqualRect( const RECT* rect1, const RECT* rect2 ) {
    if (!rect1 || !rect2) return false;
    return ((rect1->left == rect2->left) && (rect1->right == rect2->right) &&
            (rect1->top == rect2->top) && (rect1->bottom == rect2->bottom));
}

inline bool OffsetRect( RECT* rect, int x, int y )
{
    if (!rect) return false;
    rect->left   += x;
    rect->right  += x;
    rect->top    += y;
    rect->bottom += y;
    return true;
}

inline bool InflateRect( RECT* rect, int x, int y )
{
    if (!rect) return false;
    rect->left   -= x;
    rect->top    -= y;
    rect->right  += x;
    rect->bottom += y;
    return true;
}

#endif	/* SCREENCOORDINATE_H */
