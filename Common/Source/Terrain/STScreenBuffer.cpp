/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: STScreenBuffer.cpp,v 8.2 2010/12/12 17:09:24 root Exp root $
*/

#include "externs.h"
#include "STScreenBuffer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int CSTScreenBuffer::CorrectedWidth(int nWidth)
{
	return ( ( nWidth + 3 ) / 4 ) * 4;
}

#ifdef WIN32
struct DIBINFO : public BITMAPINFO
{
	RGBQUAD	 arColors[255];    // Color table info - adds an extra 255 entries to palette

	operator LPBITMAPINFO()          { return (LPBITMAPINFO) this; }
	operator LPBITMAPINFOHEADER()    { return &bmiHeader;          }
	RGBQUAD* ColorTable()            { return bmiColors;           }
};
#endif

/////////////////////////////////////////////////////////////////////////////
// CSTScreenBuffer
CSTScreenBuffer::CSTScreenBuffer() : m_pBuffer(NULL), m_pBufferTmp(NULL) {

}

CSTScreenBuffer::~CSTScreenBuffer() {
    if (m_pBufferTmp) {
        free(m_pBufferTmp);
        m_pBufferTmp = NULL;
    }
}

BOOL CSTScreenBuffer::CreateBitmap(int nWidth, int nHeight)
{
  LKASSERT(nWidth>0);
  LKASSERT(nHeight>0);
  
  m_Bitmap.Release();
  
  m_nCorrectedWidth = CorrectedWidth(nWidth);
  m_nWidth = nWidth;
  m_nHeight = nHeight;

#ifdef WIN32
  DIBINFO  dibInfo;
  
  dibInfo.bmiHeader.biBitCount = 24;
  dibInfo.bmiHeader.biClrImportant = 0;
  dibInfo.bmiHeader.biClrUsed = 0;
  dibInfo.bmiHeader.biCompression = 0;
  dibInfo.bmiHeader.biHeight = m_nHeight;
  dibInfo.bmiHeader.biPlanes = 1;
  dibInfo.bmiHeader.biSize = 40;
  dibInfo.bmiHeader.biSizeImage = m_nCorrectedWidth*m_nHeight*3;
  dibInfo.bmiHeader.biWidth = m_nCorrectedWidth;
  dibInfo.bmiHeader.biXPelsPerMeter = 3780;
  dibInfo.bmiHeader.biYPelsPerMeter = 3780;
  dibInfo.bmiColors[0].rgbBlue = 0;
  dibInfo.bmiColors[0].rgbGreen = 0;
  dibInfo.bmiColors[0].rgbRed = 0;
  dibInfo.bmiColors[0].rgbReserved = 0;
  
  HDC hDC = ::GetDC(NULL);
  LKASSERT(hDC);
  BGRColor **pBuffer = &m_pBuffer;
  m_Bitmap = LKBitmap(CreateDIBSection(hDC, (const BITMAPINFO*)dibInfo, DIB_RGB_COLORS, (void**)pBuffer, NULL, 0));
  ::ReleaseDC(NULL, hDC);
  LKASSERT(m_Bitmap);
#endif

  LKASSERT(m_pBuffer);
  
  m_pBufferTmp = (BGRColor*)malloc(sizeof(BGRColor)*m_nHeight*m_nCorrectedWidth);
  LKASSERT(m_pBufferTmp);

  return TRUE;
}

void CSTScreenBuffer::Create(int nWidth, int nHeight, LKColor clr) {
    LKASSERT(nWidth > 0);
    LKASSERT(nHeight > 0);

    CreateBitmap(nWidth, nHeight);
    std::fill_n(m_pBuffer, m_nHeight*m_nCorrectedWidth, BGRColor(clr.Blue(), clr.Green(), clr.Red()));
}

void CSTScreenBuffer::DrawStretch(LKSurface& Surface, const RECT& rcDest) {
    LKASSERT(m_Bitmap);
    const unsigned cx = rcDest.right - rcDest.left;
    const unsigned cy = rcDest.bottom - rcDest.top;
    int cropsize;
    if ((cy < m_nWidth) || ScreenLandscape) {
        cropsize = m_nHeight * cx / cy;
    } else {
        // NOT TESTED!
        cropsize = m_nWidth;
    }

    Surface.DrawBitmapCopy(rcDest.left, rcDest.top, cx, cy, m_Bitmap, cropsize, m_nHeight);
}

void CSTScreenBuffer::Zoom(unsigned int step) {
  BGRColor* src = m_pBuffer;
  BGRColor* dst = m_pBufferTmp;
  BGRColor* dst_start = m_pBufferTmp;

  const unsigned int smallx = m_nCorrectedWidth/step;
  const unsigned int smally = m_nHeight/step;
  const unsigned int rowsize = m_nCorrectedWidth*sizeof(BGRColor);
  const unsigned int wstep = m_nCorrectedWidth*step;
  const unsigned int stepmo = step-1;

  dst_start = m_pBufferTmp+(smally-1)*wstep;
  for (unsigned int y= smally; y--; dst_start-= wstep) {
    dst = dst_start;
    for (unsigned int x= smallx; x--; src++) {
      for (unsigned int j= step; j--; ) {
	*dst++ = *src;
      }
    }
    // done first row, now copy each row
    for (unsigned int k= stepmo; k--; dst+= m_nCorrectedWidth) {
      memcpy((char*)dst, (char*)dst_start, rowsize);
    }
  }

  // copy it back to main buffer
  memcpy((char*)m_pBuffer, (char*)m_pBufferTmp, 
	 rowsize*m_nHeight);

}

void CSTScreenBuffer::HorizontalBlur(unsigned int boxw) {

  const unsigned int muli = (boxw*2+1);
  BGRColor* src = m_pBuffer;
  BGRColor* dst = m_pBufferTmp;
  BGRColor *c;

  const unsigned int off1 = boxw+1;
  const unsigned int off2 = m_nCorrectedWidth-boxw-1;
  const unsigned int right = m_nCorrectedWidth-boxw;

  for (unsigned int y=m_nHeight;y--; )
    {
      unsigned int tot_r=0;
      unsigned int tot_g=0;
      unsigned int tot_b=0;
      unsigned int x;

      c = src+boxw-1;
      for (x=boxw;x--; c--) {
        tot_r+= c->m_R;
        tot_g+= c->m_G;
        tot_b+= c->m_B;
      }
      for (x=0;x< m_nCorrectedWidth; x++)
	{
	  unsigned int acc = muli;
	  if (x>boxw) {
	    c = src-off1;
	    tot_r-= c->m_R;
	    tot_g-= c->m_G;
	    tot_b-= c->m_B;
	  }  else {
	    acc += x-boxw;
	  }
	  if (x< right) {
	    c = src+boxw;
	    tot_r+= c->m_R;
	    tot_g+= c->m_G;
	    tot_b+= c->m_B;
	  } else {
	    acc += off2-x;
	  }
	  dst->m_R=(unsigned char)(tot_r/acc);
	  dst->m_G=(unsigned char)(tot_g/acc);
	  dst->m_B=(unsigned char)(tot_b/acc);

	  src++;
	  dst++;
	}
    }	

  // copy it back to main buffer
  memcpy((char*)m_pBuffer, (char*)m_pBufferTmp, 
	 m_nCorrectedWidth*m_nHeight*sizeof(BGRColor));

}

void CSTScreenBuffer::VerticalBlur(unsigned int boxh) {

  BGRColor* src = m_pBuffer;
  BGRColor* dst = m_pBufferTmp;
  BGRColor *c, *d, *e;

  const unsigned int muli = (boxh*2+1);
  const unsigned int iboxh = m_nCorrectedWidth*boxh;
  const unsigned int off1 = iboxh+m_nCorrectedWidth;
  const unsigned int off2 = m_nHeight-boxh-1;
  const unsigned int bottom = m_nHeight-boxh;

  for (unsigned int x= m_nCorrectedWidth; x--;)
    {
      unsigned int tot_r=0;
      unsigned int tot_g=0;
      unsigned int tot_b=0;
      unsigned int y;
      
      c = d = src+x;
      e = dst+x;
      for (y=boxh;y--; c+= m_nCorrectedWidth) {
	tot_r+= c->m_R;
	tot_g+= c->m_G;
	tot_b+= c->m_B;
      }

      for (y=0;y< m_nHeight; y++) {
	unsigned int acc = muli;
        if (y>boxh) {
          c = d-off1;
          tot_r-= c->m_R;
          tot_g-= c->m_G;
          tot_b-= c->m_B;
        }  else {
          acc += y-boxh;
        }
        if (y< bottom) {
          c = d+iboxh;
          tot_r+= c->m_R;
          tot_g+= c->m_G;
          tot_b+= c->m_B;
        } else {
          acc += off2-y;
        }
        e->m_R=(unsigned char)(tot_r/acc);
        e->m_G=(unsigned char)(tot_g/acc);
        e->m_B=(unsigned char)(tot_b/acc);
        d+= m_nCorrectedWidth;
        e+= m_nCorrectedWidth;
      }
    }

  // copy it back to main buffer
  memcpy((char*)m_pBuffer, (char*)m_pBufferTmp, 
	 m_nCorrectedWidth*m_nHeight*sizeof(BGRColor));

}
