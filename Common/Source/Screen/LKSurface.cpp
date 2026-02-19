/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKSurface.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 17 octobre 2014
 */

#include <tchar.h>
#include <cassert>
#include <math.h>
#include <vector>
#include <utility>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "LKSurface.h"
#include "MathFunctions.h"
#include "utils/2dpclip.h"
#include "utils/array_adaptor.h"
#include "utils/stl_utils.h"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Texture.hpp"
#include "Screen/OpenGL/Scope.hpp"

#ifdef USE_GLSL
#include "Screen/OpenGL/Shaders.hpp"
#include "Screen/OpenGL/Program.hpp"
#else
#include "Screen/OpenGL/Compatibility.hpp"
#endif
#else
#include "Screen/LKBitmapSurface.h"
#endif


#ifdef USE_GDI

LKSurface::LKSurface() : _OutputDC(), _AttribDC(), _TempDC() {

}

void LKSurface::SetAttribDC(HDC hDC) {
    _AttribDC = hDC;
}

void LKSurface::SetOutputDC(HDC hDC) {
    _OutputDC = hDC;
}

void LKSurface::ReleaseAttribDC() {
    _AttribDC = NULL;
}

void LKSurface::Release() {
    if(_OutputDC) {
        ::DeleteDC(Detach());
    }
}

void LKSurface::ReleaseTempDC() {
    if (_TempDC) {
        ::DeleteDC(_TempDC);
        _TempDC = NULL;
    }
}

HDC LKSurface::GetTempDC() {
    if (!_TempDC) {
        _TempDC = ::CreateCompatibleDC(GetAttribDC());
    }
    return _TempDC;
}

bool LKSurface::Attach(HDC hDC) {
    LKASSERT(NULL == _OutputDC); // only attach once.
    LKASSERT(NULL == _AttribDC);
    if (NULL == hDC) {
        return false;
    }
    _OutputDC = hDC;

    return true;
}

HDC LKSurface::Detach() {
    HDC hDC = _OutputDC;
    ReleaseAttribDC();
    ReleaseTempDC();
    _OutputDC = NULL;
    return hDC;
}
#else

#include "Screen/SubCanvas.hpp"


LKSurface::LKSurface() : _pCanvas() {
    
}

void LKSurface::Release() {

}

bool LKSurface::Attach(Canvas* pCanvas) {
    if(pCanvas && pCanvas->IsDefined()) {
        _pCanvas = pCanvas;
    }
    return _pCanvas;
}

Canvas* LKSurface::Detach() {
    Canvas* p = _pCanvas;
    _pCanvas = nullptr;
    return p;
}


#endif

LKSurface::~LKSurface() {
    Release();
}

LKColor LKSurface::SetTextColor(const LKColor& Color) {
#ifdef USE_GDI
    return LKColor(::SetTextColor(*this, Color));
#else
    if(_pCanvas) {
        _pCanvas->SetTextColor(Color);
    }    
    return LKColor();
#endif
}

LKColor LKSurface::SetBkColor(const LKColor& Color) {
#ifdef USE_GDI
    return LKColor(::SetBkColor(*this, Color));
#else
    if(_pCanvas) {
        _pCanvas->SetBackgroundColor(Color);
    }    
    return LKColor();
#endif
}

void LKSurface::DrawBitmapCopy(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap, const int cxSrc, const int cySrc) {
#ifdef USE_GDI
    HGDIOBJ old = ::SelectObject(GetTempDC(), (HBITMAP) Bitmap);

    if (cxSrc != cx || cySrc != cy) {
        ::StretchBlt(*this, x, y, cx, cy, GetTempDC(), 0, 0, cxSrc, cySrc, SRCCOPY);
    } else {
        ::BitBlt(*this, x, y, cx, cy, GetTempDC(), 0, 0, SRCCOPY);
    }

    ::SelectObject(GetTempDC(), old);
#else
    if(_pCanvas && Bitmap.IsDefined()) {
        if (cxSrc != cx || cySrc != cy) {
            _pCanvas->Stretch(x, y, cx, cy, Bitmap, 0, 0, cxSrc, cySrc);
        } else {
            _pCanvas->Copy(x, y, cx, cy, Bitmap, 0, 0);
        }
    }    
#endif
}

void LKSurface::DrawBitmap(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap, const int cxSrc, const int cySrc) {
#ifdef USE_GDI
    HGDIOBJ old = ::SelectObject(GetTempDC(), (HBITMAP) Bitmap);

    if (cxSrc != cx || cySrc != cy) {
        ::StretchBlt(*this, x, y, cx, cy, GetTempDC(), 0, 0, cxSrc, cySrc, SRCPAINT);
    } else {
        ::BitBlt(*this, x, y, cx, cy, GetTempDC(), 0, 0, SRCPAINT);
    }

    ::SelectObject(GetTempDC(), old);
#elif defined(ENABLE_OPENGL)
    if(_pCanvas && Bitmap.IsDefined()) {
        _pCanvas->Stretch(x, y, cx, cy, Bitmap, 0, 0, cxSrc, cySrc);
    }
#else
    if(_pCanvas && Bitmap.IsDefined()) {
        if (cxSrc != cx || cySrc != cy) {
            _pCanvas->StretchOr(x, y, cx, cy, Bitmap, 0, 0, cxSrc, cySrc);
        } else {
            _pCanvas->CopyOr(x, y, cx, cy, Bitmap, 0, 0);
        }
    }
#endif    
}

void LKSurface::DrawBitmapCopy(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap) {
#ifdef USE_GDI
    HGDIOBJ old = ::SelectObject(GetTempDC(), (HBITMAP) Bitmap);
    ::BitBlt(*this, x, y, cx, cy, GetTempDC(), 0, 0, SRCCOPY);
    ::SelectObject(GetTempDC(), old);
#else
    if(_pCanvas && Bitmap.IsDefined()) {
        _pCanvas->Copy(x, y, cx, cy, Bitmap, 0, 0);
    }
#endif    
}

void LKSurface::DrawBitmap(const int x, const int y, const int cx, const int cy, const LKBitmap& Bitmap) {
#ifdef USE_GDI
    HGDIOBJ old = ::SelectObject(GetTempDC(), (HBITMAP) Bitmap);
    ::BitBlt(*this, x, y, cx, cy, GetTempDC(), 0, 0, SRCPAINT);
    ::SelectObject(GetTempDC(), old);
#elif defined(ENABLE_OPENGL)
    if(_pCanvas && Bitmap.IsDefined()) {
        _pCanvas->Stretch(x, y, cx, cy, Bitmap, 0, 0, cx, cy);
    }
#else
    if(_pCanvas && Bitmap.IsDefined()) {
        _pCanvas->CopyOr(x, y, cx, cy, Bitmap, 0, 0);
    }
#endif    
}

void LKSurface::Polygon(const POINT *apt, int cpt) {
    if (cpt < 3) {
        return; // ignore empty polygon
    }
    assert(apt[0] == apt[cpt-1]);
#ifdef USE_GDI
    ::Polygon(*this, apt, cpt);
#else
    if(_pCanvas) {
        _pCanvas->DrawPolygon(apt, cpt);
    }    
#endif    
}

void LKSurface::Polygon(const RasterPoint *apt, int cpt, const RECT& ClipRect) {
    if(cpt < 3) {
        return;
    }
    assert(apt[0] == apt[cpt-1]);
#ifdef ENABLE_OPENGL
    const GLPushScissor push_scissor;
    const GLCanvasScissor scissor(ClipRect);
    Polygon(apt, cpt);
#else
    std::vector<RasterPoint> Clipped;
    Clipped.reserve(cpt);
    LKGeom::ClipPolygon(ClipRect, make_array(apt, cpt), Clipped);
    if(Clipped.size() >= 3) {
        Polygon(Clipped.data(), Clipped.size());
    }
#endif
}


void LKSurface::Polyline(const POINT *apt, int cpt) {
#ifdef USE_GDI
    ::Polyline(*this, apt, cpt);
#else
    if(_pCanvas) {
        _pCanvas->DrawPolyline(apt, cpt);
    }
#endif    
}

void LKSurface::Polyline(const POINT *apt, int cpt, const RECT& ClipRect) {
    if(cpt >= 2) {
        POINT Line[2];
        const POINT * Start = NULL;
        size_t n=0;

        LKGeom::clipper<POINT> Clipper((POINT) {ClipRect.left, ClipRect.top}, (POINT) {ClipRect.right, ClipRect.bottom});
        
        const POINT * const ItFirst = std::next(apt);
        const POINT * const ItLast = std::next(apt, cpt);

        Line[0] = *(apt);
        for(const POINT * It = ItFirst; It != ItLast; ++It) {
            Line[1] = *(It);
            const unsigned j = Clipper.ClipLine(Line[0], Line[1]);
            if(j&LKGeom::clipper<POINT>::_SEGM) {
                if(j&LKGeom::clipper<POINT>::_CLIP) {
                    if(Start && n > 1) {
                        Polyline(Start, n);
                        Start = NULL;
                        n=0;
                    }
                    Polyline(Line, 2);
                } else {
                    if(!Start) {
                        Start = std::prev(It);
                        n = 2;
                    } else {
                        ++n;
                    }
                }
            }
            Line[0] = *(It);
        }
        if(Start) {
            Polyline(Start, n);
        }
    }
}

#ifdef ENABLE_OPENGL
void LKSurface::Polyline(const FloatPoint *apt, int cpt, const RECT& ClipRect) {
    if(_pCanvas) {
        const GLPushScissor push_scissor;
        const GLCanvasScissor scissor(ClipRect);
        _pCanvas->DrawPolyline(apt, cpt);
    }
}

void LKSurface::DrawDashPoly(const int width, const LKColor& color, const FloatPoint* pt, const unsigned npoints, const RECT& rc) {
    LKPen hpDash(PEN_DASH, width, color);
    SelectObject(hpDash);
    Polyline(pt, npoints, rc);
}
#endif

void LKSurface::DrawDashPoly(const int width, const LKColor& color, const RasterPoint *pt, const unsigned npoints, const RECT& rc) {
    for (unsigned Segment = 1; Segment < npoints; Segment++) {
        DrawDashLine(width, pt[Segment - 1], pt[Segment], color, rc);
    }
}

void LKSurface::Blackness(const int x, const int y, const int cx, const int cy) {
#ifdef USE_GDI
    ::BitBlt(*this, x, y, cx, cy, NULL, 0, 0, BLACKNESS);
#else
    if(_pCanvas) {
        _pCanvas->Clear(COLOR_BLACK);
    }
#endif
}

void LKSurface::Whiteness(const int x, const int y, const int cx, const int cy) {
#ifdef USE_GDI
    ::BitBlt(*this, x, y, cx, cy, NULL, 0, 0, WHITENESS);
#else
    if(_pCanvas) {
        _pCanvas->ClearWhite();
    }
#endif
}

void LKSurface::DrawSolidLine(const POINT &ptStart, const POINT &ptEnd, const RECT& rc) {
    const POINT pt[2] = {{ptStart.x, ptStart.y}, {ptEnd.x, ptEnd.y}};
    Polyline(pt, 2, rc);
}

void LKSurface::DrawLine(int x1, int y1, int x2, int y2) {
    const POINT pt[2] = {{x1, y1}, {x2, y2}};
    Polyline(pt, 2);
}

void LKSurface::DrawLine(int x1, int y1, int x2, int y2, int x3, int y3) {
    const POINT pt[3] = {{x1, y1}, {x2, y2}, {x3, y3}};
    Polyline(pt, 3);
}


void LKSurface::DrawDashLine(const int width, const POINT& ptStart, const POINT& ptEnd, const LKColor& cr, const RECT& rc) {
    int i;
    POINT pt[2];

    int Offset = ((width - 1) / 2) + 1;
    // amount to shift 1st line to center
    // the group of lines properly

    //Create a dot pen
    LKPen hpDash(PEN_DASH, 1, cr);
    const auto hpOld = SelectObject(hpDash);

    pt[0] = ptStart;
    pt[1] = ptEnd;

    //increment on smallest variance
    if (abs(ptStart.x - ptEnd.x) < abs(ptStart.y - ptEnd.y)) {
        pt[0].x -= Offset;
        pt[1].x -= Offset;
        for (i = 0; i < width; i++) {
            pt[0].x += 1;
            pt[1].x += 1;
            Polyline(pt, 2, rc);
        }
    } else {
        pt[0].y -= Offset;
        pt[1].y -= Offset;
        for (i = 0; i < width; i++) {
            pt[0].y += 1;
            pt[1].y += 1;
            Polyline(pt, 2, rc);
        }
    }

    SelectObject(hpOld);
}

void LKSurface::FillRect(const RECT *lprc, const BrushReference Brush) {
#ifdef USE_GDI
    ::FillRect(*this, lprc, Brush);
#else
    if(_pCanvas && Brush && lprc) {
        _pCanvas->DrawFilledRectangle(*lprc, *Brush);
    }
#endif    
}

#ifndef ENABLE_OPENGL
bool LKSurface::Copy(int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, const LKSurface& Surface, int nXOriginSrc, int nYOriginSrc) {
#ifdef USE_GDI
    return ::BitBlt(*this, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, Surface, nXOriginSrc, nYOriginSrc, SRCCOPY);
#else
    if(_pCanvas && Surface.IsDefined()) {
        _pCanvas->Copy(nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, Surface, nXOriginSrc, nYOriginSrc);
        return true;
    }
    return false;
#endif
}
#endif


#ifdef __MINGW32CE__
#ifdef __cplusplus
extern "C" {
#endif
/* 
TransparentImage is not defined in arm-mingw32ce

  TransparentBlt (Windows CE 5.0)
		The TransparentBlt function is a wrapper for the TransparentImage function.
		http://msdn.microsoft.com/fr-fr/library/windows/apps/xaml/aa453778.aspx
*/
	WINGDIAPI WINBOOL WINAPI TransparentImage(HDC hdcDest,int xoriginDest,int yoriginDest,int wDest,int hDest,HDC hdcSrc,int xoriginSrc,int yoriginSrc,int wSrc,int hSrc,UINT crTransparent);
#ifdef __cplusplus
}
#endif
#endif

#ifndef ENABLE_OPENGL
bool LKSurface::TransparentCopy(int xoriginDest, int yoriginDest, int wDest, int hDest, const LKSurface& Surface, int xoriginSrc, int yoriginSrc) {
#ifdef USE_GDI
#ifdef UNDER_CE
    return ::TransparentImage(*this, xoriginDest, yoriginDest, wDest, hDest, Surface, xoriginSrc, yoriginSrc, wDest, hDest, COLOR_WHITE);
#else
    return ::TransparentBlt(*this, xoriginDest, yoriginDest, wDest, hDest, Surface, xoriginSrc, yoriginSrc, wDest, hDest, COLOR_WHITE);
#endif
#else
    if(_pCanvas && Surface.IsDefined()) {
        _pCanvas->CopyTransparentWhite(xoriginDest, yoriginDest, wDest, hDest, Surface, xoriginSrc, yoriginSrc);
        return true;
    }
    return false;
#endif
}

// for each white pixel in the mask, do nothing (NOP), and for each black pixel, do a Copy.
bool LKSurface::CopyWithMask(int nXDest, int nYDest, int nWidth, int nHeight, const LKSurface& hdcSrc, int nXSrc, int nYSrc, const LKBitmapSurface& bmpMask, int xMask, int yMask) {
#ifdef USE_GDI
    return ::MaskBlt(*this, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, bmpMask, xMask, yMask, MAKEROP4(SRCAND,  0x00AA0029));
#else
    Canvas& buffer = hdcSrc;
    buffer.CopyNotOr(nXDest, nYDest, nWidth, nHeight, bmpMask, xMask, yMask);
    _pCanvas->Copy(nXDest, nYDest, nWidth, nHeight, buffer, nXSrc, nYSrc);
    return true;
#endif
}

#ifdef UNDER_CE
LKSurface::TAlphaBlendF LKSurface::AlphaBlendF = NULL;
#endif
#endif
// tries to locate AlphaBlend() function
// sets pointer to AlphaBlend function (AlphaBlendF) 
// (returns false when AlphaBlending is not supported)
bool LKSurface::AlphaBlendSupported() {
#ifdef UNDER_CE
    static bool bInit = false;
    if(!bInit) {
        AlphaBlendF = (TAlphaBlendF) GetProcAddress(GetModuleHandle(TEXT("coredll.dll")), TEXT("AlphaBlend"));        
        bInit = true;
    }
#endif
    // always supported on all.
    return true;
}

#ifndef ENABLE_OPENGL
bool LKSurface::AlphaBlend(const RECT& dstRect, const LKSurface& Surface, const RECT& srcRect, uint8_t globalOpacity) {
    if(!AlphaBlendSupported()) {
        return false;
    }

#ifdef USE_GDI
#ifdef UNDER_CE

    extern BOOL DoAlphaBlend_internal(HDC,int,int,int,int,HDC,int,int,int,int, DWORD);

    static unsigned failedCount = 0;
    static bool Success = false;

    bool bOK = false;
    if (AlphaBlendF) {
        //BLENDFUNCTION bf = { AC_SRC_OVER, 0, globalOpacity, AC_SRC_ALPHA };
        // we are not using per-pixel alpha, so do not use AC_SRC_ALPHA flag
        BLENDFUNCTION bf = {AC_SRC_OVER, 0, globalOpacity, 0};
        bOK = AlphaBlendF(
                *this, dstRect.left, dstRect.top, dstRect.right - dstRect.left, dstRect.bottom - dstRect.top,
                Surface, srcRect.left, srcRect.top, srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, bf);

        if (!Success) {
            if (!bOK && dstRect.right - dstRect.left > 0 &&
                    dstRect.bottom - dstRect.top > 0 &&
                    srcRect.right - srcRect.left > 0 &&
                    srcRect.bottom - srcRect.top > 0) {

                // after more 10 consecutive failed, we assume AlphaBlend is not supported, don't use it anymore
                ++failedCount;
                if (failedCount > 10) {
                    AlphaBlendF = NULL;
                }
            }
            Success = bOK;
        }
    }

    if (!bOK) {
        bOK = DoAlphaBlend_internal(
                *this, dstRect.left, dstRect.top, dstRect.right - dstRect.left, dstRect.bottom - dstRect.top,
                Surface, srcRect.left, srcRect.top, srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, globalOpacity);
    }

    return bOK;

#else
    //BLENDFUNCTION bf = { AC_SRC_OVER, 0, globalOpacity, AC_SRC_ALPHA };
    // we are not using per-pixel alpha, so do not use AC_SRC_ALPHA flag
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, globalOpacity, 0};
    ::AlphaBlend(*this, dstRect.left, dstRect.top, dstRect.right - dstRect.left, dstRect.bottom - dstRect.top,
            Surface, srcRect.left, srcRect.top, srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, bf);

    return true; // always return true because always implemented on Windows PC
#endif
#else
    if(_pCanvas) {
        _pCanvas->AlphaBlend(dstRect.left, dstRect.top, dstRect.right - dstRect.left, dstRect.bottom - dstRect.top,
                        Surface, srcRect.left, srcRect.top, srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, globalOpacity);
        return true;
    }
    return false;
#endif    
}
#endif

#ifdef USE_MEMORY_CANVAS
void LKSurface::AlphaBlendNotWhite(const RECT& dstRect, const LKSurface& Surface, const RECT& srcRect, uint8_t globalOpacity) {
    if(_pCanvas) {
        _pCanvas->AlphaBlendNotWhite(dstRect.left, dstRect.top, dstRect.right - dstRect.left, dstRect.bottom - dstRect.top,
                        Surface, srcRect.left, srcRect.top, srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, globalOpacity);
    }
}
#endif    

bool LKSurface::GetTextSize(const TCHAR* lpString, SIZE* lpSize) const {
#ifdef USE_GDI
    return ::GetTextExtentPoint(*this, lpString, _tcslen(lpString), lpSize);
#else
    if(_pCanvas) {
        *lpSize = _pCanvas->CalcTextSize(lpString);
        return true;
    }
    return false;
#endif
}

PixelSize LKSurface::GetTextSize(const TCHAR *text) const {
    SIZE size = {};
    GetTextSize(text, &size);
    return { size.cx, size.cy };
}

void LKSurface::DrawText(int X, int Y, const TCHAR* lpString, RECT* ClipRect) {
#ifdef USE_GDI
    ::ExtTextOut(*this, X, Y,ETO_CLIPPED, ClipRect, lpString, _tcslen(lpString), NULL);
#else
    if(_pCanvas) {
        if(ClipRect) {
            SubCanvas ClipCanvas(*_pCanvas, ClipRect->GetOrigin(), ClipRect->GetSize() );
            ClipCanvas.DrawText(X-ClipRect->left, Y-ClipRect->top, lpString);
        } else {
            _pCanvas->DrawText(X, Y, lpString);
        }
    }    
#endif    
}

void LKSurface::DrawPushButton(const RECT& rc, bool bPushed) {
#ifdef USE_GDI
    const UINT Style = bPushed? DFCS_BUTTONPUSH | DFCS_PUSHED : DFCS_BUTTONPUSH;
    RECT Rect = rc;
    ::DrawFrameControl(*this, &Rect, DFC_BUTTON, Style);
#else
    if(_pCanvas) {
        _pCanvas->DrawButton(rc, bPushed);
    }
#endif    
}

int LKSurface::DrawText(const TCHAR* lpchText, RECT *lpRect, UINT uFormat) {
    assert(lpRect);
#ifdef USE_GDI
    return ::DrawText(*this, lpchText, _tcslen(lpchText), lpRect, uFormat);
#else
    if(_pCanvas) {
        if(!(uFormat&(DT_NOCLIP|DT_CALCRECT))) {
            PixelRect rcText(*lpRect);
            SubCanvas ClipCanvas(*_pCanvas, rcText.GetOrigin(), rcText.GetSize() );
            rcText.Offset(-rcText.left, -rcText.top);
            ClipCanvas.DrawFormattedText(&rcText, lpchText, uFormat);
        } else {
            _pCanvas->DrawFormattedText(lpRect, lpchText, uFormat);
        }
    }
    return false;
#endif    
}

void LKSurface::DrawTextClip(int x, int y, const TCHAR *text, PixelScalar width) {
    int len = _tcslen(text);
    if (len > 0) {
#ifdef USE_GDI
        const RECT rc = {x, 0, x + width, std::numeric_limits<PixelScalar>::max()};
        ::ExtTextOut(*this, x, y, ETO_CLIPPED, &rc, text, len, NULL);
#else
        if(_pCanvas) {
            SubCanvas ClipCanvas(*_pCanvas, {x, 0}, {(unsigned)width, _pCanvas->GetHeight()});
            ClipCanvas.DrawText(0,y, text);
        }
#endif
    }
}

int LKSurface::GetTextWidth(const TCHAR *text) {
    SIZE tsize;
    if(GetTextSize(text, &tsize)) {
        return tsize.cx;
    }
    return 0;
}

int LKSurface::GetTextHeight(const TCHAR *text) {
    SIZE tsize{};
    if(GetTextSize(text, &tsize)) {
        return tsize.cy;
    }
    return 0;
}

void LKSurface::SetPixel(int X, int Y, const LKColor& Color) {
#ifdef USE_GDI
    ::SetPixel(*this, X, Y, Color);
#else
    if(_pCanvas) {
        _pCanvas->DrawFilledRectangle(X,Y,X+1,Y+1,Color);
    }
#endif
}

void LKSurface::Rectangle(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect) {
#ifdef USE_GDI
    ::Rectangle(*this, nLeftRect, nTopRect, nRightRect, nBottomRect);
#else
    if(_pCanvas) {
        _pCanvas->Rectangle(nLeftRect, nTopRect, nRightRect, nBottomRect);
    }    
#endif
}


#ifndef ENABLE_OPENGL
// Do not use InvertRect with OPENGL, yet
bool LKSurface::InvertRect(const RECT& rc) {
#ifdef USE_GDI
    return ::InvertRect(*this, &rc);
#elif !defined(ENABLE_OPENGL)
    if(_pCanvas) {
        _pCanvas->InvertRectangle(rc);
        return true;
    }
    return false;
#else
#warning "Not Implemented"
    return false;
#endif
}
#endif



bool LKSurface::RoundRect(const RECT& rc, int nWidth, int nHeight) {
#ifdef USE_GDI
    return ::RoundRect(*this, rc.left, rc.top, rc.right, rc.bottom, nWidth, nHeight);
#else
    if(_pCanvas) {
        _pCanvas->DrawRoundRectangle(rc.left, rc.top, rc.right, rc.bottom, nWidth, nHeight);
        return true;
    }
    return false;
#endif
}

void LKSurface::ExcludeClipRect(const RECT& rc) {
#ifdef USE_GDI
    ::ExcludeClipRect(*this, rc.left, rc.top, rc.right, rc.bottom);
#else
    // only used in WndForm, Not really needed...
#endif
}

int LKSurface::SaveState() {
#ifdef USE_GDI
    return ::SaveDC(*this);
#else
    // Not Needed...
    return 0;
#endif
}

void LKSurface::RestoreState(int nSaved) {
#ifdef USE_GDI
    ::RestoreDC(*this, nSaved);
#else
    // Not Needed...
#endif
}


#define  _COS(x) cos( ((4*atan(1))/2)-(x*2.0*(4*atan(1))/64.0))
static const double xcoords[] = {
    _COS(0), _COS(1), _COS(2), _COS(3), _COS(4), _COS(5), _COS(6), _COS(7),
    _COS(8), _COS(9), _COS(10), _COS(11), _COS(12), _COS(13), _COS(14), _COS(15),
    _COS(16), _COS(17), _COS(18), _COS(19), _COS(20), _COS(21), _COS(22), _COS(23),
    _COS(24), _COS(25), _COS(26), _COS(27), _COS(28), _COS(29), _COS(30), _COS(31),
    _COS(32), _COS(33), _COS(34), _COS(35), _COS(36), _COS(37), _COS(38), _COS(39),
    _COS(40), _COS(41), _COS(42), _COS(43), _COS(44), _COS(45), _COS(46), _COS(47),
    _COS(48), _COS(49), _COS(50), _COS(51), _COS(52), _COS(53), _COS(54), _COS(55),
    _COS(56), _COS(57), _COS(58), _COS(59), _COS(60), _COS(61), _COS(62), _COS(63)
};

#define  _SIN(x) sin(((4*atan(1))/2)-(x*2.0*(4*atan(1))/64.0))
static const double ycoords[] = {
    _SIN(0), _SIN(1), _SIN(2), _SIN(3), _SIN(4), _SIN(5), _SIN(6), _SIN(7),
    _SIN(8), _SIN(9), _SIN(10), _SIN(11), _SIN(12), _SIN(13), _SIN(14), _SIN(15),
    _SIN(16), _SIN(17), _SIN(18), _SIN(19), _SIN(20), _SIN(21), _SIN(22), _SIN(23),
    _SIN(24), _SIN(25), _SIN(26), _SIN(27), _SIN(28), _SIN(29), _SIN(30), _SIN(31),
    _SIN(32), _SIN(33), _SIN(34), _SIN(35), _SIN(36), _SIN(37), _SIN(38), _SIN(39),
    _SIN(40), _SIN(41), _SIN(42), _SIN(43), _SIN(44), _SIN(45), _SIN(46), _SIN(47),
    _SIN(48), _SIN(49), _SIN(50), _SIN(51), _SIN(52), _SIN(53), _SIN(54), _SIN(55),
    _SIN(56), _SIN(57), _SIN(58), _SIN(59), _SIN(60), _SIN(61), _SIN(62), _SIN(63)
};

void LKSurface::buildCircle(const RasterPoint& center, int radius, std::vector<RasterPoint>& list) {
  
    typedef RasterPoint::scalar_type scalar_type;
  
    int step = ((radius<20)?2:1);
    list.clear();
    list.reserve((64/step)+1);
    list.emplace_back( (RasterPoint){ 
        center.x + static_cast<scalar_type>(radius * xcoords[0]), 
        center.y + static_cast<scalar_type>(radius * ycoords[0]) 
    });
    for(int i=64-step; i>=0; i-=step) {
        list.emplace_back( (RasterPoint){ 
            center.x + static_cast<scalar_type>(radius * xcoords[i]), 
            center.y + static_cast<scalar_type>(radius * ycoords[i]) 
        });
    }
}

void LKSurface::DrawCircle(long x, long y, int radius, const RECT& rc, bool fill) {

    if ((x - radius) > rc.right) return;
    if ((x + radius) < rc.left) return;
    if ((y - radius) > rc.bottom) return;
    if ((y + radius) < rc.top) return;

    // Only called by ThreadDraw, so static vector can be used.
    static std::vector<RasterPoint> CirclePt;
    buildCircle(RasterPoint(x,y), radius, CirclePt);
      
    if (fill) {
        Polygon(CirclePt.data(), CirclePt.size(), rc);
    } else {
        Polyline(CirclePt.data(), CirclePt.size(), rc);
    }
}

void LKSurface::DrawCircle(long x, long y, int radius, bool fill) {
    std::vector<RasterPoint> CirclePt;
    buildCircle((RasterPoint){x,y}, radius, CirclePt);
      
    if (fill) {
        Polygon(CirclePt.data(), CirclePt.size());
    } else {
        Polyline(CirclePt.data(), CirclePt.size());
    }
}

void LKSurface::Segment(long x, long y, int radius, const RECT& rc, double start, double end) {

    // JMW added faster checking...
    if ((x - radius) > rc.right) return;
    if ((x + radius) < rc.left) return;
    if ((y - radius) > rc.bottom) return;
    if ((y + radius) < rc.top) return;

    
    bool EmptySector = end == start;
    if(EmptySector) {
      RasterPoint pt[2];
      
      start = AngleLimit360(start);

      pt[0].x = x;
      pt[0].y = y;
      
      pt[1].x = x + (long) (radius * fastsine(start));
      pt[1].y = y - (long) (radius * fastcosine(start));
      
      Polyline(pt, std::size(pt), rc);
      
    } else {
      RasterPoint pt[68];

      start = AngleLimit360(start);
      end = AngleLimit360(end);

      const int istart = std::ceil(start / 360.0 * 64);
      const int iend = std::floor(((end > start) ? end : end + 360) / 360 * 64);

      int npoly = 0;

      // Center point
      pt[npoly].x = x;
      pt[npoly].y = y;
      npoly++;

      // first Segment point
      pt[npoly].x = x + (long) (radius * fastsine(start));
      pt[npoly].y = y - (long) (radius * fastcosine(start));
      npoly++;

      for (int i = 0; i < 64; i++) {
          if (i <= iend - istart) {
              pt[npoly].x = x + (long) (radius * xcoords[(i + istart) % 64]);
              pt[npoly].y = y - (long) (radius * ycoords[(i + istart) % 64]);
              npoly++;
          }
      }

      // Last Segment Point
      pt[npoly].x = x + (long) (radius * fastsine(end));
      pt[npoly].y = y - (long) (radius * fastcosine(end));
      npoly++;

      // Close Polygon ( center point )
      pt[npoly].x = x;
      pt[npoly].y = y;
      npoly++;
      
      assert(npoly <= (int)std::size(pt));
      
      Polygon(pt, npoly, rc);
    }
}

/*
 * VENTA3 This is a modified Segment()
 */
int LKSurface::DrawArc(long x, long y, int radius, const RECT& rc, double start, double end) {
    POINT pt[66];
    int i;
    int istart;
    int iend;

    if ((x - radius) > rc.right) return false;
    if ((x + radius) < rc.left) return false;
    if ((y - radius) > rc.bottom) return false;
    if ((y + radius) < rc.top) return false;

    // JMW added faster checking...

    start = AngleLimit360(start);
    end = AngleLimit360(end);

    istart = iround(start / 360.0 * 64);
    iend = iround(end / 360.0 * 64);

    int npoly = 0;

    if (istart > iend) {
        iend += 64;
    }
    istart++;
    iend--;

    pt[npoly].x = x + (long) (radius * fastsine(start));
    pt[npoly].y = y - (long) (radius * fastcosine(start));
    npoly++;

    for (i = 0; i < 64; i++) {
        if (i <= iend - istart) {
            pt[npoly].x = x + (long) (radius * xcoords[(i + istart) % 64]);
            pt[npoly].y = y - (long) (radius * ycoords[(i + istart) % 64]);
            npoly++;
        }
    }
    pt[npoly].x = x + (long) (radius * fastsine(end));
    pt[npoly].y = y - (long) (radius * fastcosine(end));
    npoly++;

    Polyline(pt, npoly, rc);

    return true;
}

