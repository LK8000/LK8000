/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include "types.h"
#include "Screen/Canvas.hpp"
#endif

#ifndef ENABLE_OPENGL
class LKBitmapSurface;
#endif

class LKSurface {
protected:
#ifndef USE_GDI
    Canvas* _pCanvas; // need to be first.
#endif

public:
    LKSurface();
    virtual ~LKSurface();

    LKSurface( const LKSurface& ) = delete;
    LKSurface& operator=( const LKSurface& ) = delete;

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

    void DrawBitmapCopy(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap, const int cxSrc, const int cySrc);
    void DrawBitmap(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap, const int cxSrc, const int cySrc);
    void DrawBitmapCopy(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap);
    void DrawBitmap(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap);

    void Polygon(const RasterPoint *apt, int cpt, const RECT& ClipRect);
    void Polygon(const POINT *apt, int cpt);

    void Polyline(const POINT *apt, int cpt);
    void Polyline(const POINT *apt, int cpt, const RECT& ClipRect);

#ifdef ENABLE_OPENGL
    void Polyline(const FloatPoint *apt, int cpt, const RECT& ClipRect);

    void DrawDashPoly(const int width, const LKColor& color, const FloatPoint* pt, const unsigned npoints, const RECT& rc);
#endif

    void DrawDashPoly(const int width, const LKColor& color, const RasterPoint* pt, const unsigned npoints, const RECT& rc);

    /**
     * this is used only by DrawThread, not thread safe
     */
    void DrawCircle(long x, long y, int radius, const RECT& rc, bool fill);
    /**
     * this is used by DrawThread and MainThread for waypoint Picto and Statistics::RenderTask
     *   !! need to be thread safe on platform other than OpenGL
     */
    void DrawCircle(long x, long y, int radius, bool fill);

    void DrawLine(int x1, int y1, int x2, int y2);
    void DrawLine(int x1, int y1, int x2, int y2, int x3, int y3);

    template <typename PT>
    void DrawLine(Pen::Style PenStyle, const int width, const PT& ptStart, const PT& ptEnd, const LKColor& cr, const RECT& rc) {
        LKPen Pen(PenStyle, width, cr);
        const auto OldPen = SelectObject(Pen);

        const PT pt[2] = {ptStart, ptEnd};
        Polyline(pt, 2, rc);

        SelectObject(OldPen);
    }

    void DrawSolidLine(const POINT &ptStart, const POINT &ptEnd, const RECT& rc);

    void DrawDashLine(const int, const POINT&, const POINT&, const LKColor&, const RECT& rc);

    void Segment(long x, long y, int radius, const RECT& rc, double start, double end);

    int DrawArc(long x, long y, int radius, const RECT& rc, double start, double end);


    void FillRect(const RECT *lprc, BrushReference Brush);
    void Rectangle(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect);

    void DrawPushButton(const RECT& rc, bool bPushed);

    static bool AlphaBlendSupported();

 #ifndef ENABLE_OPENGL
    bool Copy(int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, const LKSurface& Surface, int nXOriginSrc, int nYOriginSrc);
    bool TransparentCopy(int xoriginDest, int yoriginDest, int wDest, int hDest, const LKSurface& Surface, int xoriginSrc, int yoriginSrc);

    bool CopyWithMask(int nXDest, int nYDest, int nWidth, int nHeight, const LKSurface& hdcSrc, int nXSrc, int nYSrc, const LKBitmapSurface& bmpMask, int xMask, int yMask);

    bool AlphaBlend(const RECT& dstRect, const LKSurface& Surface, const RECT& srcRect, uint8_t globalOpacity);

    bool InvertRect(const RECT& rc);

#endif

#ifdef USE_MEMORY_CANVAS
    void AlphaBlendNotWhite(const RECT& dstRect, const LKSurface& Surface, const RECT& srcRect, uint8_t globalOpacity);
#endif
    bool GetTextSize(const TCHAR* lpString, SIZE* lpSize);

    void DrawText(int X, int Y, const TCHAR* lpString, RECT* ClipRect = nullptr);

    void DrawText(const RasterPoint& p, const TCHAR* lpString, RECT* ClipRect = nullptr) {
        DrawText(p.x, p.y, lpString, ClipRect);
    }

    int DrawText(const TCHAR* lpchText, RECT *lpRect, UINT uFormat);

    int GetTextWidth(const TCHAR *text);
    int GetTextHeight(const TCHAR *text);
    void DrawTextClip(int x, int y, const TCHAR *text, PixelScalar width);


    void SetPixel(int X, int Y, const LKColor& Color);

    bool RoundRect(const RECT& rc, int nWidth, int nHeight);

    static void buildCircle(const RasterPoint& center, int radius, std::vector<RasterPoint>& list);

    void ExcludeClipRect(const RECT& rc);

    int SaveState();
    void RestoreState(int nSaved);

    virtual void Release();

#ifdef USE_GDI
    virtual void SetAttribDC(HDC hDC);
    virtual void SetOutputDC(HDC hDC);

    virtual void ReleaseAttribDC();

    void ReleaseTempDC();

    bool Attach(HDC hDC);
    HDC Detach();

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
    friend class LKIcon;

    HDC GetTempDC();

    HDC _OutputDC; // Drawing Surface

private:
    HDC _AttribDC;
    HDC _TempDC; // used for Draw Bitmap.

#endif

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
