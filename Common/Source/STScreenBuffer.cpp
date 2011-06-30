/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: STScreenBuffer.cpp,v 8.2 2010/12/12 17:09:24 root Exp root $
*/

#include "StdAfx.h"
#include "STScreenBuffer.h"
#include "lk8000.h"

#include "utils/heapcheck.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int CSTScreenBuffer::CorrectedWidth(int nWidth)
{
	return ( ( nWidth + 3 ) / 4 ) * 4;
}

struct DIBINFO : public BITMAPINFO
{
	RGBQUAD	 arColors[255];    // Color table info - adds an extra 255 entries to palette

	operator LPBITMAPINFO()          { return (LPBITMAPINFO) this; }
	operator LPBITMAPINFOHEADER()    { return &bmiHeader;          }
	RGBQUAD* ColorTable()            { return bmiColors;           }
};


/////////////////////////////////////////////////////////////////////////////
// CSTScreenBuffer

CSTScreenBuffer::CSTScreenBuffer()
	: memDc(NULL),
	  m_pBuffer(NULL),
	  m_pBufferTmp(NULL),
	  m_hBitmap(NULL),
	  m_pDC(NULL)
{
}

CSTScreenBuffer::~CSTScreenBuffer()
{
	if (m_hBitmap!=NULL) {
		ReleaseDC();
		::DeleteObject(m_hBitmap);
	}
	if (m_pBufferTmp) {
	  free(m_pBufferTmp);
	}
	if (memDc) {
	  DeleteDC(memDc); memDc = NULL;
	}
}

BOOL CSTScreenBuffer::CreateBitmap(int nWidth, int nHeight)
{
  ASSERT(nWidth>0);
  ASSERT(nHeight>0);
  
  if (m_hBitmap!=NULL) DeleteObject(m_hBitmap);
  
  m_nCorrectedWidth = CorrectedWidth(nWidth);
  m_nWidth = nWidth;
  m_nHeight = nHeight;
  
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
  if (!hDC) FailStore(_T("A001 GetDC")); // 100101
  ASSERT(hDC);
  BGRColor **pBuffer = &m_pBuffer;
  m_hBitmap = CreateDIBSection(hDC, (const BITMAPINFO*)dibInfo, DIB_RGB_COLORS, (void**)pBuffer, NULL, 0);
  ::ReleaseDC(NULL, hDC);
  if (!m_hBitmap) FailStore(_T("A002 hBitmap"));
  ASSERT(m_hBitmap);
  if (!m_pBuffer) FailStore(_T("A003 pBuffer"));
  ASSERT(m_pBuffer);
  
  m_pBufferTmp = (BGRColor*)malloc(sizeof(BGRColor)*m_nHeight*m_nCorrectedWidth);
  if (!m_pBufferTmp) FailStore(_T("A003/B pBuffer"));
  
  return TRUE;
}

void CSTScreenBuffer::Create(int nWidth, int nHeight)
{
	ASSERT(nWidth>0);
	ASSERT(nHeight>0);

	CreateBitmap(nWidth, nHeight);
}

void CSTScreenBuffer::Create(int nWidth, int nHeight, COLORREF clr)
{
	ASSERT(nWidth>0);
	ASSERT(nHeight>0);

	CreateBitmap(nWidth, nHeight);

	BGRColor bgrColor = BGRColor(GetBValue(clr), GetGValue(clr), GetRValue(clr));
	int nPosition = 0;

	for (int y=0; y<nHeight; y++) {
		nPosition = m_nCorrectedWidth*y;
		for (int x=0; x<nWidth; x++) {
			m_pBuffer[nPosition] = bgrColor;
			nPosition++;
		}
	}
}

void CSTScreenBuffer::Create(HBITMAP hBitmap)
{
  BITMAP bm;
  GetObject(hBitmap, sizeof(BITMAP), &bm);
  CreateBitmap(bm.bmWidth, bm.bmHeight);
  
  HDC targetDc;
  memDc = CreateCompatibleDC(NULL);
  targetDc = CreateCompatibleDC(NULL);
  
  HBITMAP hOldBitmap1 = (HBITMAP)::SelectObject(memDc, hBitmap);
  HBITMAP hOldBitmap2 = (HBITMAP)::SelectObject(targetDc, m_hBitmap);
  
  BitBlt(targetDc, 0, 0, bm.bmWidth, bm.bmHeight, memDc, 0, 0, SRCCOPY);
  
  ::SelectObject(memDc, hOldBitmap1);
  ::SelectObject(targetDc, hOldBitmap2);
  DeleteDC(memDc); memDc = NULL;
  DeleteDC(targetDc);
}

void CSTScreenBuffer::Create(HDC *pDC, RECT rect)
{
  if (!m_pDC) FailStore(_T("A004 pDC"));
  ASSERT(m_pDC);

  CreateBitmap(rect.right-rect.left, rect.bottom-rect.top);
  BitBlt(m_pDC, 0,0, rect.right-rect.left, rect.bottom-rect.top, 
	 *pDC, rect.left, rect.top, SRCCOPY);
}

void CSTScreenBuffer::CreateRGB(void *pData, int nWidth, int nHeight)
{
  ASSERT(pData);
  ASSERT(nWidth>0);
  ASSERT(nHeight>0);
  
  CreateBitmap(nWidth, nHeight);
  
  byte *pByteData = (byte*)pData;
  int nPosition = 0;
  int nDataPosition = 0;
  
  for (int y=0; y<nHeight; y++) {
    nPosition = m_nCorrectedWidth*(m_nHeight-y-1);
    nDataPosition = nWidth*3*y;
    for (int x=0; x<nWidth; x++) {
      m_pBuffer[nPosition].m_R = pByteData[nDataPosition++];
      m_pBuffer[nPosition].m_G = pByteData[nDataPosition++];
      m_pBuffer[nPosition].m_B = pByteData[nDataPosition++];
      nPosition++;
    }
  }
}

BOOL CSTScreenBuffer::Draw(HDC* pDC, POINT ptDest)
{
  if (!m_hBitmap) FailStore(_T("A005"));
  ASSERT(m_hBitmap);
  ReleaseDC();
  
  POINT Origin = {0,0};
  
  BOOL bResult = FALSE;
  
  if (!memDc) {
    memDc = CreateCompatibleDC(*pDC);
  }
  if (!memDc) {
    return FALSE;
  }
  
  HBITMAP m_hOldBitmap = (HBITMAP)::SelectObject(memDc, m_hBitmap);
  bResult = BitBlt(*pDC, ptDest.x, ptDest.y, 
		   m_nWidth, m_nHeight, 
		   memDc, 
		   Origin.x, Origin.y, SRCCOPY);
  ::SelectObject(memDc, m_hOldBitmap);
  //  DeleteDC(memDc); memDc = NULL;
  
  return bResult;
}

BOOL CSTScreenBuffer::DrawStretch(HDC* pDC, RECT rcDest)
{
  POINT ptDest;
  unsigned int cx;
  unsigned int cy;

  ptDest.x = rcDest.left;
  ptDest.y = rcDest.top;
  cx = rcDest.right-rcDest.left;
  cy = rcDest.bottom-rcDest.top;
  return DrawStretch(pDC, ptDest, cx, cy);
}

#include "InfoBoxLayout.h"

BOOL CSTScreenBuffer::DrawStretch(HDC* pDC, POINT ptDest, 
                                  unsigned int cx, 
                                  unsigned int cy)
{
  if (!m_hBitmap) FailStore(_T("A006"));
  ASSERT(m_hBitmap);
  ReleaseDC();
  
  POINT Origin = {0,0};
  
  if (!memDc) {
    memDc = CreateCompatibleDC(*pDC);
  }
  if (!memDc) {
    return FALSE;
  }

  HBITMAP m_hOldBitmap = (HBITMAP)::SelectObject(memDc, m_hBitmap);

  int cropsize;
  if ((cy<m_nWidth)||(InfoBoxLayout::landscape)) {
    cropsize = m_nHeight*cx/cy;
  } else {
    // NOT TESTED!
    cropsize = m_nWidth;
  }
  
  BOOL bResult = StretchBlt(*pDC, 
                       ptDest.x, ptDest.y, 
		       cx, cy, 
                       memDc, 
		       Origin.x, Origin.y, 
		       cropsize, m_nHeight, SRCCOPY);
  /*
  BitBlt(*pDC,
         ptDest.x, ptDest.y,
         cx, cy, memDc, 0, 0, SRCCOPY);
  */

  ::SelectObject(memDc, m_hOldBitmap);
  
  return bResult;
}


HBITMAP CSTScreenBuffer::CreateBitmapByRGBArray(void *pData, int nWidth, int nHeight)
{
  HBITMAP hResult = NULL;	
  CSTScreenBuffer sb;
  
  sb.CreateRGB(pData, nWidth, nHeight);
  hResult = sb.m_hBitmap;
  
  sb.m_hBitmap = NULL;
  sb.m_pBuffer = NULL;
  
  return hResult;
}

HDC CSTScreenBuffer::GetDC()
{
  if (m_pDC) return m_pDC;

  m_pDC = CreateCompatibleDC(NULL);
  if (!m_pDC) {
    return NULL;
  }
  
  m_hSaveBitmap = (HBITMAP)SelectObject(m_pDC,GetHBitmap());
  return m_pDC;
}

void CSTScreenBuffer::ReleaseDC()
{
  if (m_pDC) {
    SelectObject(m_pDC, m_hSaveBitmap);
    DeleteDC(m_pDC);
    m_pDC = NULL;
  }
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
