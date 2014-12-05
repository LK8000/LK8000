/////////////////////////////////////////////////////////////////////////////
// File name:      STScreenBuffer.h
// Author:         Vassili Philippov (vasja@spbteam.com)
// Created:        June 2001
// Last changed:   07 August 2001
// Version:        2.0
// Description:    Class for pixel working with images. One could get access
//                 to image bits using this library or create custom image.

#if !defined(AFX_STSCREENBUFFER_H__22D62F5D_32E2_4785_B3D9_2341C11F84A3__INCLUDED_)
#define AFX_STSCREENBUFFER_H__22D62F5D_32E2_4785_B3D9_2341C11F84A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "externs.h"

/////////////////////////////////////////////////////////////////////////////
// BGRColor structure encapsulates color information about one point. Color
// order is Blue, Green, Red (not RGB).

struct BGRColor
{
	BGRColor() {}
	BGRColor(byte R, byte G, byte B) : m_B(B), m_G(G), m_R(R) {}
	byte m_B;
	byte m_G;
	byte m_R;
};


/////////////////////////////////////////////////////////////////////////////
// CSTScreenBuffer class provides fast drawing methods and offscreen buffer.

class CSTScreenBuffer
{
public:
	// Creates uninitialized buffer. Call Create or CreateRGB to
	// initialize the buffer.
	CSTScreenBuffer();

	virtual ~CSTScreenBuffer();

	// Creates buffer with the given size and fills it with 
	// the given color
	void Create(int nWidth, int nHeight, LKColor clr);

    BGRColor *GetBuffer(void) {
        return m_pBuffer;
    }

	void HorizontalBlur(unsigned int boxw);
	void VerticalBlur(unsigned int boxh);
	void Zoom(unsigned int step);

	// Draws buffer into given device context within rectangle
	void DrawStretch(LKSurface& Surface, const RECT& rcDest);
	
	// Returns real width of the screen buffer. It could be slightly more then
	// requested width. This paramater is important only when you work with
	// points array directly (using GetPointsArray function).
	int GetCorrectedWidth() {
		return m_nCorrectedWidth;
	}

	// Returns screen buffer width.
	int GetWidth() {
		return m_nWidth;
	}

	// Returns screen buffer height.
	int GetHeight() {
		return m_nHeight;
	}



protected:
	// Returns minimum width that is greater then the given width and
	// that is acceptable as image width (not all numbers are acceptable)
	static int CorrectedWidth(int nWidth);

	// Creates internal bitmap and image buffer. Assignes width and
	// height properties
	BOOL CreateBitmap(int nWidth, int nHeight);


	unsigned int m_nWidth;
	unsigned int m_nHeight;
	unsigned int m_nCorrectedWidth;
	BGRColor *m_pBuffer;
	BGRColor *m_pBufferTmp;
    LKBitmap m_Bitmap;
};

#endif // !defined(AFX_STSCREENBUFFER_H__22D62F5D_32E2_4785_B3D9_2341C11F84A3__INCLUDED_)
