/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: STScreenBuffer.cpp,v 8.2 2010/12/12 17:09:24 root Exp root $
*/

#include "externs.h"
#include "STScreenBuffer.h"

/////////////////////////////////////////////////////////////////////////////
// CSTScreenBuffer
CSTScreenBuffer::CSTScreenBuffer(int nWidth, int nHeight) : RawBitmap(nWidth, nHeight) {
#ifdef USE_TERRAIN_BLUR
    m_pBufferTmp = (BGRColor*)malloc(sizeof(BGRColor)*GetHeight()*GetCorrectedWidth());
    if(!m_pBufferTmp) {
        throw std::bad_alloc();
    }
#endif
    std::fill_n(GetBuffer(), GetHeight()*GetCorrectedWidth(), BGRColor(255, 255, 255));
}

CSTScreenBuffer::~CSTScreenBuffer() {
#ifdef USE_TERRAIN_BLUR
    if (m_pBufferTmp) {
        free(m_pBufferTmp);
        m_pBufferTmp = nullptr;
    }
#endif
}

void CSTScreenBuffer::DrawStretch(LKSurface& Surface, const RECT& rcDest, int scale) {
    const unsigned cx = rcDest.right - rcDest.left;
    const unsigned cy = rcDest.bottom - rcDest.top;
    int cropsize;
    if ((cy < GetWidth()) || ScreenLandscape) {
        cropsize = GetHeight() * cx / cy;
    } else {
        // NOT TESTED!
        cropsize = GetWidth();
    }
    StretchTo(cropsize/scale, GetHeight()/scale, Surface , rcDest.left, rcDest.top, cx, cy);
}

#ifdef USE_TERRAIN_BLUR
void CSTScreenBuffer::Blur(unsigned int boxw) {
    HorizontalBlur(boxw, GetBuffer(), m_pBufferTmp);
    VerticalBlur(boxw, m_pBufferTmp, GetBuffer());
}

void CSTScreenBuffer::HorizontalBlur(unsigned int boxw, BGRColor* src, BGRColor* dst) {
    const unsigned int muli = (boxw * 2 + 1);
    BGRColor *c;

    const unsigned int off1 = boxw + 1;
    const unsigned int off2 = GetCorrectedWidth() - boxw - 1;
    const unsigned int right = GetCorrectedWidth() - boxw;

    for (unsigned int y = GetHeight(); y--;) {
        unsigned int tot_r = 0;
        unsigned int tot_g = 0;
        unsigned int tot_b = 0;
        unsigned int x;

        c = src + boxw - 1;
        for (x = boxw; x--; --c) {
            tot_r += c->value.Red();
            tot_g += c->value.Green();
            tot_b += c->value.Blue();
        }

        for (x = 0; x < GetCorrectedWidth(); ++x) {
            unsigned int acc = muli;
            if (x > boxw) {
                c = src - off1;
                tot_r -= c->value.Red();
                tot_g -= c->value.Green();
                tot_b -= c->value.Blue();
            } else {
                acc += x - boxw;
            }
            if (x < right) {
                c = src + boxw;
                tot_r += c->value.Red();
                tot_g += c->value.Green();
                tot_b += c->value.Blue();
            } else {
                acc += off2 - x;
            }
            (*dst) = BGRColor((uint8_t) (tot_r / acc),(uint8_t) (tot_g / acc), (uint8_t) (tot_b / acc));

            src++;
            dst++;
        }
    }
}

void CSTScreenBuffer::VerticalBlur(unsigned int boxh, BGRColor* src, BGRColor* dst) {

    BGRColor *c, *d, *e;

    const unsigned int muli = (boxh * 2 + 1);
    const unsigned int iboxh = GetCorrectedWidth() * boxh;
    const unsigned int off1 = iboxh + GetCorrectedWidth();
    const unsigned int off2 = GetHeight() - boxh - 1;
    const unsigned int bottom = GetHeight() - boxh;

    for (unsigned int x = GetCorrectedWidth(); --x;) {
        unsigned int tot_r = 0;
        unsigned int tot_g = 0;
        unsigned int tot_b = 0;
        unsigned int y;

        c = d = src + x;
        e = dst + x;
        for (y = boxh; y--; c += GetCorrectedWidth()) {
            tot_r += c->value.Red();
            tot_g += c->value.Green();
            tot_b += c->value.Blue();
        }

        for (y = 0; y < GetHeight(); ++y) {
            unsigned int acc = muli;
            if (y > boxh) {
                c = d - off1;
                tot_r -= c->value.Red();
                tot_g -= c->value.Green();
                tot_b -= c->value.Blue();
            } else {
                acc += y - boxh;
            }
            if (y < bottom) {
                c = d + iboxh;
                tot_r += c->value.Red();
                tot_g += c->value.Green();
                tot_b += c->value.Blue();
            } else {
                acc += off2 - y;
            }
            (*e) = BGRColor((uint8_t) (tot_r / acc),(uint8_t) (tot_g / acc), (uint8_t) (tot_b / acc));
            d += GetCorrectedWidth();
            e += GetCorrectedWidth();
        }
    }
}
#endif
