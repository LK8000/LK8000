/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKSurface.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 17 octobre 2014
 */

#ifndef LKSURFACE_H
#define	LKSURFACE_H

#ifdef WIN32
#include <windows.h>
#endif

#include "boost/noncopyable.hpp"
#include <vector>

#include "LKColor.h"
#include "LKPen.h"
#include "LKBrush.h"
#include "LKBitmap.h"
#include "LKFont.h"
#include "BrushReference.h"
#include "PenReference.h"
#include "LKAssert.h"

#ifndef USE_GDI
#include "Screen/Canvas.hpp"
#endif

class LKBitmapSurface;

class LKSurface : public boost::noncopyable {
protected:
#ifndef USE_GDI
    Canvas* _pCanvas; // need to be first.
#endif
    
public:
    LKSurface();
    virtual ~LKSurface();

#ifdef USE_GDI
    typedef HFONT OldFont;
    typedef HBRUSH OldBrush;
    typedef HPEN OldPen;

    HFONT SelectObject(HFONT o) {
      return (HFONT)::SelectObject(*this, o);
    }

    HBRUSH SelectObject(HBRUSH o) {
      return (HBRUSH)::SelectObject(*this, o);
    }

    HPEN SelectObject(HPEN o) {
      return (HPEN)::SelectObject(*this, o);
    }
#else
    typedef const LKFont* OldFont;
    typedef BrushReference OldBrush;
    typedef PenReference OldPen;

    OldFont SelectObject(const LKFont &obj) {
        // we can't select invalid font, otherwise we have segfault when is used.
        if(_pCanvas && obj.IsDefined()) {
            _pCanvas->Select(obj);
        }
        return OldFont();
    }

    OldBrush SelectObject(const LKBrush &obj) {
        if(_pCanvas) {
            _pCanvas->Select(obj);
        }
        return OldBrush();
    }

    OldPen SelectObject(const LKPen &obj) {
        if(_pCanvas) {
            _pCanvas->Select(obj);
        }
        return OldPen();
    }

    OldFont SelectObject(OldFont o) { 
        if(_pCanvas && o && o->IsDefined()) {
            _pCanvas->Select(*o);
        }
        return OldFont(); 
    }
    OldBrush SelectObject(OldBrush o) { 
        if(_pCanvas && o) {
            _pCanvas->Select(*o);
        }
        return OldBrush(); 
    }
    OldPen SelectObject(OldPen o) { 
        if(_pCanvas && o) {
            _pCanvas->Select(*o);
        }
        return OldPen(); 
    }
    
    bool Attach(Canvas* pCanvas);
    Canvas* Detach();
    
    operator Canvas&() const {
        LKASSERT(_pCanvas);
        return (*_pCanvas);
    }
    
    bool IsDefined() const { return (_pCanvas && _pCanvas->IsDefined()); }
#endif

    LKColor SetTextColor(const LKColor& Color);
    LKColor SetBkColor(const LKColor& Color);
    
    inline void SetBackgroundTransparent();
    inline void SetBackgroundOpaque();

    void Blackness(const int x, const int y, const int cx, const int cy);
    void Whiteness(const int x, const int y, const int cx, const int cy);

    void DrawMaskedBitmap(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap, const int cxSrc, const int cySrc);
    void DrawBitmapCopy(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap, const int cxSrc, const int cySrc);
    void DrawBitmap(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap, const int cxSrc, const int cySrc);
    void DrawBitmapCopy(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap);
    void DrawBitmap(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap);

    void Polygon(const POINT *apt, int cpt, const RECT& ClipRect);
    void Polygon(const POINT *apt, int cpt);

    void Polyline(const POINT *apt, int cpt);
    void Polyline(const POINT *apt, int cpt, const RECT& ClipRect);

    void DrawDashPoly(const int width, const LKColor& color, const POINT* pt, const unsigned npoints, const RECT& rc);

    bool Circle(long x, long y, int radius, const RECT& rc, bool clip = false, bool fill = true);
    bool CircleNoCliping(long x, long y, int radius, const RECT& rc, bool fill);

    void DrawLine(int x1, int y1, int x2, int y2);
    void DrawLine(int x1, int y1, int x2, int y2, int x3, int y3);
    void DrawLine(Pen::Style PenStyle, const int width, const POINT& ptStart, const POINT& ptEnd, const LKColor& cr, const RECT& rc);

    void DrawSolidLine(const POINT &ptStart, const POINT &ptEnd, const RECT& rc);

    void DrawDashLine(const int, const POINT&, const POINT&, const LKColor&, const RECT& rc);

    int Segment(long x, long y, int radius, const RECT& rc, double start, double end, bool horizon = false);

    int DrawArc(long x, long y, int radius, const RECT& rc, double start, double end);


    void FillRect(const RECT *lprc, BrushReference Brush);
    void Rectangle(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect);

    void DrawPushButton(const RECT& rc, bool bPushed);

    bool Copy(int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, const LKSurface& Surface, int nXOriginSrc, int nYOriginSrc);
	bool StretchCopy(int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, const LKSurface& Surface, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc);
    bool TransparentCopy(int xoriginDest, int yoriginDest, int wDest, int hDest, const LKSurface& Surface, int xoriginSrc, int yoriginSrc);

    bool CopyWithMask(int nXDest, int nYDest, int nWidth, int nHeight, const LKSurface& hdcSrc, int nXSrc, int nYSrc, const LKBitmapSurface& bmpMask, int xMask, int yMask);
    
    static bool AlphaBlendSupported();
    bool AlphaBlend(const RECT& dstRect, const LKSurface& Surface, const RECT& srcRect, uint8_t globalOpacity);
#ifdef USE_MEMORY_CANVAS
    void AlphaBlendNotWhite(const RECT& dstRect, const LKSurface& Surface, const RECT& srcRect, uint8_t globalOpacity);
#endif    
    bool GetTextSize(const TCHAR* lpString, int cbString, SIZE* lpSize);
    void DrawText(int X, int Y, const TCHAR* lpString, UINT cbCount, RECT* ClipRect = nullptr);
    int DrawText(const TCHAR* lpchText, int nCount, RECT *lpRect, UINT uFormat);

    int GetTextWidth(const TCHAR *text);
    int GetTextHeight(const TCHAR *text);
    void DrawTextClip(int x, int y, const TCHAR *text, PixelScalar width);


    void SetPixel(int X, int Y, const LKColor& Color);

    bool InvertRect(const RECT& rc);
    bool RoundRect(const RECT& rc, int nWidth, int nHeight);

    static void buildCircle(const POINT& center, int radius, std::vector<POINT>& list);

    void ExcludeClipRect(const RECT& rc);

    int SaveState();
    void RestoreState(int nSaved);

    virtual void Release();
    
#ifdef WIN32
    virtual void SetAttribDC(HDC hDC);
    virtual void SetOutputDC(HDC hDC);

    virtual void ReleaseAttribDC();

    void ReleaseTempDC();

    bool Attach(HDC hDC);
    HDC Detach();

protected:
    friend class RawBitmap;

    operator HDC() const {
        return _OutputDC;
    }

    operator HDC() {
        return _OutputDC;
    }

public:
    HDC GetAttribDC() const {
        return _AttribDC ? _AttribDC : _OutputDC;
    }
	
protected:

    HDC GetTempDC();

    HDC _OutputDC; // Drawing Surface

private:
    HDC _AttribDC;
    HDC _TempDC; // used for Draw Bitmap.

#endif


    static const double xcoords[];
    static const double ycoords[];

#ifdef UNDER_CE
// pointer to AlphaBlend() function (initialized by first call of AlphaBlendSupported())
typedef BOOL (WINAPI *TAlphaBlendF)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION); 
static TAlphaBlendF AlphaBlendF;
#endif

};

#ifndef USE_GDI
#include "Screen/Canvas.hpp"
#endif

inline void LKSurface::SetBackgroundTransparent() {
#ifdef USE_GDI
    ::SetBkMode(*this, TRANSPARENT);
#else
    if(_pCanvas) {
        _pCanvas->SetBackgroundTransparent();
    }
#endif
}

inline void LKSurface::SetBackgroundOpaque() {
#ifdef USE_GDI
    ::SetBkMode(*this, OPAQUE);
#else
    if(_pCanvas) {
        _pCanvas->SetBackgroundOpaque();
    }
#endif
}

#endif	/* LKSURFACE_H */

