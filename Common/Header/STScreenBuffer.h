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

#include "Screen/RawBitmap.hpp"

/////////////////////////////////////////////////////////////////////////////
// CSTScreenBuffer class provides fast drawing methods and offscreen buffer.

class CSTScreenBuffer : public RawBitmap
{
public:
	// Creates uninitialized buffer. Call Create or CreateRGB to
	// initialize the buffer.
	CSTScreenBuffer(int nWidth, int nHeight, LKColor clr);

	virtual ~CSTScreenBuffer();

	// Creates buffer with the given size and fills it with 
	// the given color
	void Create(int nWidth, int nHeight, LKColor clr);

	void HorizontalBlur(unsigned int boxw);
	void VerticalBlur(unsigned int boxh);

	// Draws buffer into given device context within rectangle
	void DrawStretch(LKSurface& Surface, const RECT& rcDest, int scale);
	
protected:
	// Returns minimum width that is greater then the given width and
	// that is acceptable as image width (not all numbers are acceptable)
	static int CorrectedWidth(int nWidth);

	BGRColor *m_pBufferTmp;
    LKBitmap m_Bitmap;
};

#endif // !defined(AFX_STSCREENBUFFER_H__22D62F5D_32E2_4785_B3D9_2341C11F84A3__INCLUDED_)
