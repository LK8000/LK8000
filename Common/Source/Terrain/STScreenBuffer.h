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

#if (!defined(GREYSCALE) && !defined(_WIN32_WCE) && !defined(ENABLE_OPENGL))
#define USE_TERRAIN_BLUR
#endif
/////////////////////////////////////////////////////////////////////////////
// CSTScreenBuffer class provides fast drawing methods and offscreen buffer.

class CSTScreenBuffer final : public RawBitmap
{
public:
	// Creates uninitialized buffer. Call Create or CreateRGB to
	// initialize the buffer.
	CSTScreenBuffer(int nWidth, int nHeight);

#ifdef USE_TERRAIN_BLUR
    void Blur(unsigned int boxw);
#endif

	// Draws buffer into given device context within rectangle
	void DrawStretch(LKSurface& Surface, const RECT& rcDest, int scale);

protected:
#ifdef USE_TERRAIN_BLUR
	void HorizontalBlur(unsigned int boxw, BGRColor* src, BGRColor* dst);
	void VerticalBlur(unsigned int boxh, BGRColor* src, BGRColor* dst);

	std::unique_ptr<BGRColor[]> m_pBufferTmp;
#endif
};

#endif // !defined(AFX_STSCREENBUFFER_H__22D62F5D_32E2_4785_B3D9_2341C11F84A3__INCLUDED_)
