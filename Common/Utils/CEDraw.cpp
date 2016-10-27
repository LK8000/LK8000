/* ---------------------------------------------------------------------------------------------------
 *
 *                          Windows CE Graphics Libary v1.00.0000
 *
 *
 *    Written by James.
 *    Bug report : jiet@msn.com
 *                                                             Copyright 2001
 */
// File: CEDraw.cpp
// Graphics library
//-----------------------------------------------------------------------------------------------------
//										Update Information.
//-----------------------------------------------------------------------------------------------------

/*
 * Created by James D. 2001.
 * Date: 01/11/2001
 */

#include "StdAfx.h"
#include "CEDraw.h"
#include "Polygon.h"
#include "CEGDefine.h"

//
// Function : CEDraw()
// Purpose  : Construction function.
//
CCEDraw::CCEDraw()
{

	m_pFont  = NULL;
	m_pBrush = NULL;
	m_pPen   = NULL;

	// Read the font to the memory...
	// Unsuitable code
	FILE* fp;
	UINT  nFileSize;
	fp=fopen("HZK16","rb++");
	if( fp )
	{
		fseek( fp, 0, SEEK_END );
		nFileSize = ftell( fp );
		fseek( fp, 0, SEEK_SET );
		m_pFontFile = (LPBYTE)malloc( nFileSize );
		if( m_pFontFile )
			fread( m_pFontFile, nFileSize, 1, fp );
		fclose( fp );
	}

}

//
// Function : CEDraw()
// Purpose  : Destruction function.
//
CCEDraw::~CCEDraw()
{
	// Release the font file alloc memory...
	if( NULL != m_pFontFile ) free( m_pFontFile );

}

int CCEDraw::MulDiv(int a, int b, int c)
{
    int nMul = a * b,
        nMod = nMul % c,
        nRes = nMul / c;

    if(nMod >= c / 2)    // Round up if >= 0.5
        nRes++;
    return nRes;
}

//
// Function : DrawPixel()
// Putpose  : Draw a Point
//
inline VOID CCEDraw::DrawPixel( DWORD dwX, DWORD dwY, unsigned short Color )
{
	if( IsPointOutside( dwX, dwY ) )
		return;

	if( m_gxdp.cBPP == 8 )
		*( m_pVB + dwY*m_cbyPitch + m_cbxPitch*dwX ) = (BYTE)Color;

	*(unsigned short*)( m_pVB + dwY*m_cbyPitch + m_cbxPitch*dwX ) = Color;
}

//
// Function : IsAllPointInside()
// Check the point in side
//
inline BOOL CCEDraw::IsAllPointInside( POINT* ptPoint, int nNumber )
{
	if( NULL == ptPoint ) return FALSE;
	for( int nIndex = 0 ; nIndex < nNumber ; nIndex ++ )
	{
		if( ptPoint[nIndex].x < 0 && ptPoint[nIndex].y < 0 ) return FALSE;
		if( ptPoint[nIndex].x >= (LONG)m_gxdp.cxWidth && ptPoint[nIndex].y >= (LONG)m_gxdp.cyHeight ) return FALSE;
	}
	return TRUE;
}

//
// Function : IsAllIsPointOutside()
// Check the point out side
//
inline BOOL CCEDraw::IsAllPointOutside( POINT* ptPoint, int nNumber )
{
	if( NULL == ptPoint ) return FALSE;
	for( int nIndex = 0 ; nIndex < nNumber ; nIndex ++ )
	{
		if( ptPoint[nIndex].x < (LONG)m_gxdp.cxWidth && ptPoint[nIndex].y < (LONG)m_gxdp.cyHeight
			&& ptPoint[nIndex].x >= 0 && ptPoint[nIndex].y >= 0 ) return FALSE;
	}
	return TRUE;
}

//
// Function : GetBitmapPointColor()
// Get the point color from the bitmap
//
inline unsigned short CCEDraw::GetBitmapPointColor( unsigned char * pBuffer ) {
	unsigned short Color;
	unsigned char nColorRed   = *(pBuffer+2);
	unsigned char nColorGreen = *(pBuffer+1);
	unsigned char nColorBlue  = *pBuffer;

	switch( m_gxdp.cBPP ) {
		case 16:
			if ( m_gxdp.ffFormat & kfDirect555 ) {
				nColorRed >>= 3;
				nColorGreen >>= 3;
				nColorBlue >>= 3;
				Color = (unsigned short)( ( nColorRed & 0xff ) << 10 |
										  ( nColorGreen & 0xff ) << 5 |
										  ( nColorBlue & 0xff ) );
			}
			else if( m_gxdp.ffFormat & kfDirect565) {
				nColorRed >>= 3;
				nColorGreen >>= 2;
				nColorBlue >>= 3;
				Color = (unsigned short)( ( nColorRed & 0xff ) << 11 |
										  ( nColorGreen & 0xff ) << 5 |
										  ( nColorBlue & 0xff ) );

			}
			else if( m_gxdp.ffFormat & kfDirect444 ) {
				nColorRed >>= 4;
				nColorGreen >>= 4;
				nColorBlue >>= 4;
				Color = (unsigned short)( ( nColorRed & 0xf0 ) << 8 |
										  ( nColorGreen & 0xf0 ) << 4 |
										  ( nColorBlue & 0xf0 ) );

			}
			break;
		case 8:
			Color = (unsigned short)( ( nColorRed & 0xf ) << 5 |
									  ( nColorGreen & 0x3 ) <<3 |
									  ( nColorBlue & 0xF ) );
			break;
	}
	return Color;
}

//
// Function : DrawLine()
// Purpose  : This method draws lines
//
BOOL CCEDraw::DrawLine( POINT &ptOrig, POINT &ptDes )
{
	DrawLine( ptOrig.x, ptOrig.y, ptDes.x, ptDes.y );
	return TRUE;
}

BOOL CCEDraw::DrawHLine( LONG lOrigX, LONG lOrigY, LONG lDestX, LONG lDestY, short nColor )
{
	if( lOrigY != lDestY ) return TRUE;  // Because Draw H Line...
	if( lOrigY < 0 || lOrigY >= (LONG)m_gxdp.cyHeight ) return TRUE;
	if( ( lOrigX < 0 && lDestX < 0 ) || ( lOrigX >= (LONG)m_gxdp.cxWidth && lDestX >= (LONG)m_gxdp.cxWidth ) ) return TRUE;

	LONG lTemp;
	if( lOrigX > lDestX )
	{
		lTemp = lOrigX;
		lOrigX = lDestX;
		lDestX = lTemp;
	}
	if( lOrigX < 0 ) lOrigX = 0;
	if( lDestX >= (LONG)m_gxdp.cxWidth ) lDestX = m_gxdp.cxWidth - 1;
	if( nColor == -1 && m_pPen != NULL ) nColor = m_pPen->m_Color;

	unsigned char* pBuffer = ( m_pVB + lOrigY*m_cbyPitch + m_cbxPitch*lOrigX );
	if( m_gxdp.cBPP == 8 )
	{
		for( int nX = lOrigX ; nX < lDestX ; nX ++ )
		{
			*pBuffer = (BYTE)nColor;
			pBuffer += m_cbxPitch;
		}
	}
	else if( m_gxdp.cBPP == 16 )
	{
		for( int nX = lOrigX ; nX < lDestX ; nX ++ )
		{
			*(unsigned short*)pBuffer = (unsigned short)nColor;
			pBuffer += m_cbxPitch;
		}
	}
	return TRUE;
}

BOOL CCEDraw::DrawVLine( LONG lOrigX, LONG lOrigY, LONG lDestX, LONG lDestY, short nColor )
{
	if( lOrigX != lDestX ) return TRUE;  // Because Draw V Line...
	if( lOrigX < 0 || lOrigX >= (LONG)m_gxdp.cxWidth ) return TRUE;
	if( ( lOrigY < 0 && lDestY < 0 ) || ( lOrigY >= (LONG)m_gxdp.cyHeight && lDestY >= (LONG)m_gxdp.cyHeight ) ) return TRUE;

	LONG lTemp;
	if( lOrigY > lDestY )
	{
		lTemp = lOrigY;
		lOrigY = lDestY;
		lDestY = lTemp;
	}
	if( lOrigY < 0 ) lOrigY = 0;
	if( lDestY >= (LONG)m_gxdp.cyHeight ) lDestY = m_gxdp.cyHeight - 1;
	if( nColor == -1 && m_pPen != NULL ) nColor = m_pPen->m_Color;

	unsigned char* pBuffer = ( m_pVB + lOrigY*m_cbyPitch + m_cbxPitch*lOrigX );
	if( m_gxdp.cBPP == 8 )
	{
		for( int nY = lOrigY ; nY < lDestY ; nY ++ )
		{
			*pBuffer = (BYTE)nColor;
			pBuffer += m_cbyPitch;
		}
	}
	else if( m_gxdp.cBPP == 16 )
	{
		for( int nY = lOrigY ; nY < lDestY ; nY ++ )
		{
			*(unsigned short*)pBuffer = (unsigned short)nColor;
			pBuffer += m_cbyPitch;
		}
	}
	return TRUE;
}

BOOL CCEDraw::DrawLine( LONG lOrigX, LONG lOrigY, LONG lDestX, LONG lDestY )
{
	//int dX = abs( lDestX-lOrigX );	// store the change in X and Y of the line endpoints
	//int dY = abs( lDestY-lOrigY );
	int dX, dY;
	int CurrentX = lOrigX;		// store the starting point (just point A)
	int CurrentY = lOrigY;

	// Draw none if all point outside the screen...
	if( IsPointOutside( lOrigX, lOrigY ) && IsPointOutside( lDestX, lDestY ) ) return TRUE;

	int Xincr, Yincr;
	if( lOrigX > lDestX ) { Xincr=-1; dX = lOrigX - lDestX; } else { Xincr=1; dX = lDestX - lOrigX; }	// which direction in X?
	if( lOrigY > lDestY ) { Yincr=-1; dY = lOrigY - lDestY; } else { Yincr=1; dY = lDestY - lOrigY; }	// which direction in Y?

	if( dX >= dY )	// if X is the independent variable
	{
		int dPr 	= dY << 1;
		int dPru 	= dPr - ( dX << 1 );
		int P 		= dPr - dX;

		for ( ; dX >= 0; dX -- )
		{
			DrawPixel( CurrentX, CurrentY, m_pPen->m_Color );
			if (P > 0)
			{
				CurrentX += Xincr;
				CurrentY += Yincr;
				P += dPru;
			}
			else
			{
				CurrentX += Xincr;
				P += dPr;
			}
		}
	}
	else
	{
		int dPr 	= dX << 1;
		int dPru 	= dPr - ( dY << 1 );
		int P 		= dPr - dY;

		for( ; dY >= 0; dY -- )
		{
			DrawPixel( CurrentX, CurrentY, m_pPen->m_Color );
			if ( P > 0 )
			{
				CurrentX += Xincr;
				CurrentY += Yincr;
				P += dPru;
			}
			else
			{
				CurrentY += Yincr;
				P += dPr;
			}
		}
	}
	return TRUE;
}

//
// Function : ConvertColor()
// Convert the COLORREF to CE support color
//
inline unsigned short CCEDraw::ConvertColor( COLORREF crColor )
{
	unsigned short Color = 0;
	int      nColorRed   = ((int)crColor)&0xff;
	int      nColorGreen = ((int)crColor>>8)&0xff;
	int      nColorBlue  = ((int)crColor>>16)&0xff;

	switch( m_gxdp.cBPP )
    {
	case 16:
        if ( m_gxdp.ffFormat & kfDirect555 )
		{
			nColorRed >>= 3;
			nColorGreen >>= 3;
			nColorBlue >>= 3;
            Color = (unsigned short)( ( nColorRed & 0xff ) << 10 |
                                      ( nColorGreen & 0xff ) << 5 |
                                      ( nColorBlue & 0xff ) );
		}
		else if( m_gxdp.ffFormat & kfDirect565)
		{
			nColorRed >>= 3;
			nColorGreen >>= 2;
			nColorBlue >>= 3;
			Color = (unsigned short)( ( nColorRed & 0xff ) << 11 |
                                      ( nColorGreen & 0xff ) << 5 |
                                      ( nColorBlue & 0xff ) );

        }
		else if( m_gxdp.ffFormat & kfDirect444 )
		{
			nColorRed >>= 4;
			nColorGreen >>= 4;
			nColorBlue >>= 4;
			Color = (unsigned short)( ( nColorRed & 0xf0 ) << 8 |
                                      ( nColorGreen & 0xf0 ) << 4 |
                                      ( nColorBlue & 0xf0 ) );

        }
		break;
	case 8:
		Color = (unsigned short)( ( nColorRed & 0xf ) << 5 |
                                  ( nColorGreen & 0x3 ) <<3 |
                                  ( nColorBlue & 0xF ) );
		break;
	}
	return Color;

}

//
// Function : ConvertColor()
// Convert the COLORREF to CE support color
//
unsigned short CCEDraw::ConvertColor( int nColorRed, int nColorGreen, int nColorBlue )
{
	unsigned short Color = 0;

	switch( m_gxdp.cBPP )
    {
	case 16:
        if ( m_gxdp.ffFormat & kfDirect555 )
		{
			nColorRed >>= 3;
			nColorGreen >>= 3;
			nColorBlue >>= 3;
            Color = (unsigned short)( ( nColorRed & 0xff ) << 10 |
                                      ( nColorGreen & 0xff ) << 5 |
                                      ( nColorBlue & 0xff ) );
		}
		else if( m_gxdp.ffFormat & kfDirect565)
		{
			nColorRed >>= 3;
			nColorGreen >>= 2;
			nColorBlue >>= 3;
			Color = (unsigned short)( ( nColorRed & 0xff ) << 11 |
                                      ( nColorGreen & 0xff ) << 5 |
                                      ( nColorBlue & 0xff ) );

        }
		else if( m_gxdp.ffFormat & kfDirect444 )
		{
			nColorRed >>= 4;
			nColorGreen >>= 4;
			nColorBlue >>= 4;
			Color = (unsigned short)( ( nColorRed & 0xf0 ) << 8 |
                                      ( nColorGreen & 0xf0 ) << 4 |
                                      ( nColorBlue & 0xf0 ) << 0 );

        }
		break;
	case 8:
		Color = (unsigned short)( ( nColorRed & 0xf ) << 5 |
                                  ( nColorGreen & 0x3 ) <<3 |
                                  ( nColorBlue & 0xF ) );
		break;
	}
	return Color;
}

//
// Function : Clear()
// Purpose  : Clear the background with the specify pen
//
BOOL CCEDraw::Clear( CCEPen* pPen )
{
	if ( NULL == pPen )	pPen = m_pPen;
	if ( NULL == pPen )
	{
		m_dwLastError = CEG_DRAW_PEN_NULL;
		return FALSE;     // Not Select Pen Object;
	}

	unsigned char * pusLine = (unsigned char*)m_pVB;
	if( m_gxdp.cBPP == 8 ) {
		for( unsigned int y = 0; y < m_gxdp.cyHeight - 1; y ++ )
		{
			unsigned char * pusDest = pusLine;
			for( unsigned int x = 0; x < m_gxdp.cxWidth - 1; x ++ )
			{
				*pusDest = (BYTE)pPen->m_Color;
				pusDest += m_cbxPitch;
			}
			pusLine += m_cbyPitch;
		}
	}
	else if( m_gxdp.cBPP == 16 )
	{
		for( unsigned int y = 0; y < m_gxdp.cyHeight - 1; y ++ )
		{
			unsigned char * pusDest = pusLine;
			for( unsigned int x = 0; x < m_gxdp.cxWidth - 1; x ++ )
			{

				//*(unsigned short*)pusDest = pPen->m_Color;
				DrawPixel( x, y, pPen->m_Color);
				pusDest += m_cbxPitch;
			}
			pusLine += m_cbyPitch;
		}
	}

	return TRUE;
}

//
// Function : Clear()
// Purpose  : Clear the background with the specify Color
//
BOOL CCEDraw::Clear( DWORD dwColor )
{
	// The dwColor must be 0~256
	memset( m_pDoubleBuffer, dwColor, m_gxdp.cxWidth*m_gxdp.cyHeight*m_gxdp.cBPP/8 );
	return TRUE;
}

//
// Function : FadeOut()
// Purpose  : Fade Out the Screen
//
VOID CCEDraw::FadeOut( DWORD dwStep )
{
	register unsigned char * pusLine = m_pVB;
	register char ColorR, ColorG, ColorB;
	register unsigned char step=(unsigned char) dwStep;
	register unsigned short Color;
	register bool allPixelsBlack;

	// Set drawmode to directdraw for faster execution
	//SetDrawMode(CEG_DRAWMODE_DIRECTDRAW);
	do {
		allPixelsBlack=true;

		BeginDraw();
		pusLine = m_pVB;
		for( unsigned int y = 0; y < m_gxdp.cyHeight; y ++ ) {
			register unsigned char *pusDest = pusLine;
			for( register unsigned short x = 0; x < m_gxdp.cxWidth; x ++ )
			{
				if( m_gxdp.cBPP == 8 ) {
					if(*pusDest > step) {
						*pusDest -= step;

						if(allPixelsBlack)
							allPixelsBlack = false;
					}
					else
						*pusDest = 0;
				}
				else {
					if( m_gxdp.ffFormat & kfDirect565 ) {
						/* n% ALPHA */
						Color = *(unsigned short*)pusDest;
						ColorR = ((Color >> 11) & 0x1f) - step;
						ColorG = ((Color >> 5) & 0x3f) - (step<<1);
						ColorB = (Color & 0x1f) - step;
						if( ColorR < 0 ) ColorR = 0;
						if( ColorG < 0 ) ColorG = 0;
						if( ColorB < 0 ) ColorB = 0;

						Color = (unsigned short)( ( ColorR & 0xff ) << 11 |
										  ( ColorG & 0xff ) << 5 |
										  ( ColorB & 0xff ) );
						*(unsigned short*)pusDest = Color;

						if(allPixelsBlack && Color != 0)
							allPixelsBlack = false;
					}
					else if( m_gxdp.ffFormat & kfDirect555 )
					{
						/* n% ALPHA */
						Color = *(unsigned short*)pusDest;
						ColorR = (unsigned short)((Color >> 10) & 0x1f) - step;
						ColorG = (unsigned short)((Color >> 5) & 0x3f) - step;
						ColorB = (unsigned short)(Color & 0x1f) - step;
						if( ColorR < 0 ) ColorR = 0;
						if( ColorG < 0 ) ColorG = 0;
						if( ColorB < 0 ) ColorB = 0;

						Color = (unsigned short)( ( ColorR & 0xff ) << 11 |
										  ( ColorG & 0xff ) << 5 |
										  ( ColorB & 0xff ) );
						*(unsigned short*)pusDest = Color;

						if(allPixelsBlack && Color != 0)
							allPixelsBlack = false;
					}
				}
				pusDest += m_cbxPitch;
			}
			pusLine += m_cbyPitch;
		}
		EndDraw();
		Flip();
	}
	while(!allPixelsBlack);

	// Restore drawmode to double buffered
	//SetDrawMode(CEG_DRAWMODE_DIRECTDRAW);
}

BOOL CCEDraw::ScreenSnapshot(unsigned char* dest)
{
	m_pDisplayMemory = (unsigned char*)GXBeginDraw();
	if( NULL == m_pDisplayMemory )
	{
		// Return false
		m_dwLastError = CEG_GRAPH_BEGINDRAW_FAILED;
		return FALSE;
	}

	unsigned int size = m_gxdp.cxWidth * m_gxdp.cyHeight * (m_gxdp.cBPP >> 3);
	memcpy(dest, m_pDisplayMemory, size);

	// End the gx draw.
	if( GXEndDraw() == 0 )
	{
		// Error...
		m_dwLastError = CEG_GRAPH_ENDDRAW_FAILED;
		return FALSE;
	}

	return TRUE;
}

//
// Function : SetGDIObject()
// Purpose  : Select the gdi object
//
CCEGDIObject* CCEDraw::SetGDIObject( CCEGDIObject* pGDIObject )
{
	CCEGDIObject* pOldObject = NULL;
	// Cannot be null
	if( NULL == pGDIObject )
	{
		m_dwLastError = CEG_DRAW_OBJECT_NULL;
		return NULL;
	}
	switch ( pGDIObject->m_dwObjectType )
	{
	case CEG_GDIOBJECT_TYPE_PEN:
		// The Object type is Pen...
		pOldObject = m_pPen;
		m_pPen = (CCEPen*) pGDIObject;
		break;
	case CEG_GDIOBJECT_TYPE_BRUSH:
		// The Object type is Brush...
		pOldObject = m_pBrush;
		m_pBrush   = (CCEBrush*)pGDIObject;
		break;
	case CEG_GDIOBJECT_TYPE_FONT:
		// The Object type is Font...
		pOldObject = m_pFont;
		m_pFont   = (CCEFont*)pGDIObject;
		break;
	}
	return pOldObject;
}

//
// Function : GetGDIObject()
// Get the GDI Object
//
CCEGDIObject* CCEDraw::GetGDIObject( DWORD dwType )
{
	CCEGDIObject* pObject = NULL;
	switch ( dwType )
	{
	case CEG_GDIOBJECT_TYPE_PEN:
		// The Object type is Pen...
		pObject = m_pPen;
		break;
	case CEG_GDIOBJECT_TYPE_BRUSH:
		// The Object type is Brush...
		pObject = m_pBrush;
		break;
	case CEG_GDIOBJECT_TYPE_FONT:
		// The Object type is Font...
		pObject = m_pFont;
		break;
	default:
		pObject = NULL;
		break;
	}
	return pObject;
}

//
// Function : Polygon()
// This method draws a polygon consisting of two or more points connected by lines.
// The system closes the polygon automatically by drawing a line from the last vertex to the first.
//
BOOL CCEDraw::DrawPolygon( LPPOINT lpPoints, int nCount )
{
	if( NULL == lpPoints ) return FALSE;
	if( IsAllPointOutside( lpPoints, nCount ) ) return TRUE;

	LPPOINT pPoint = lpPoints;
	for( int nIndex = 1 ; nIndex < nCount ; nIndex ++ )
	{
		DrawLine( pPoint->x, pPoint->y, (pPoint+1)->x, (pPoint+1)->y );
		pPoint ++;
	}
	// Draw The Last line...
	DrawLine( lpPoints->x, lpPoints->y, pPoint->x, pPoint->y );

	return TRUE;
}

//
// Function : Polyline()
// This method draws a polygon consisting of two or more points connected by lines.
//
BOOL CCEDraw::DrawPolyline( LPPOINT lpPoints, int nCount )
{
	if( NULL == lpPoints ) return FALSE;
	if( IsAllPointOutside( lpPoints, nCount ) ) return TRUE;

	LPPOINT pPoint = lpPoints;
	for( int nIndex = 1 ; nIndex < nCount ; nIndex ++ )
	{
		DrawLine( pPoint->x, pPoint->y, (pPoint+1)->x, (pPoint+1)->y );
		pPoint ++;
	}

	return TRUE;
}

//
// Function : FillPolygon()
// This method draws a polygon with the brush fill.
// The system closes the polygon automatically by drawing a line from the last vertex to the first.
//
BOOL CCEDraw::FillPolygon( LPPOINT lpPoints, int nCount, int nOffsetX, int nOffsetY )
{
	if( NULL == lpPoints ) return FALSE;
	if( IsAllPointOutside( lpPoints, nCount ) ) return TRUE;
	PtListHeader Header;
	Header.Length   = nCount;
	Header.PointPtr = lpPoints;

	::FillPolygon( &Header, COMPLEX, nOffsetX, nOffsetY, this );
	return TRUE;
}

BOOL CCEDraw::FillPolygon( FLOATPOINT* lpPoints, int nCount, int nOffsetX, int nOffsetY )
{
	if( NULL == lpPoints ) return FALSE;
	// Check...
	BOOL bIn = FALSE;
	for( int nIndex = 0 ; nIndex < nCount ; nIndex ++ )
	{
		if( (lpPoints[nIndex].x+nOffsetX) < (LONG)m_gxdp.cxWidth && (lpPoints[nIndex].y + nOffsetY) < (LONG)m_gxdp.cyHeight
			&& (lpPoints[nIndex].x+nOffsetX) >= 0 && (lpPoints[nIndex].y + nOffsetY) >= 0 )
		{
			bIn = TRUE;
			break;
		}
	}
	if(!bIn)return TRUE;
	// Check End.
	FloatPtListHeader Header;
	Header.Length   = nCount;
	Header.PointPtr = lpPoints;

	::FillFloatPolygon( &Header, COMPLEX, nOffsetX, nOffsetY, this );
	return TRUE;
}

//
// Function : DrawText()
// This method draws string to the screen.
//
BOOL CCEDraw::DrawText( LPPOINT lpPoint, LPCTSTR lpString, int nCount, UINT uFormat )
{
	// Create Font Bitmap
	HBITMAP     hOldBitmap;       // Handle to the old bitmap
	HDC         hWindowDC;
	HBITMAP     hBitmap;
	BITMAPINFO  BitmapInfo;
	RECT        rectText;
	SIZE        sizeText;
	unsigned short nColor = 0;
	LPBYTE      pBitmap;
	LPRECT      lpRect;

	// Get the windows device content...
	hWindowDC = GetDC( NULL );
	CCEFont font;

	font.CreateFont(
		-CCEDraw::MulDiv(30, GetDeviceCaps(hWindowDC, LOGPIXELSY), 72),
		ANTIALIASED_QUALITY,
		FW_BOLD);

	HGDIOBJ oldFont = SelectObject(hWindowDC, font.m_hFont);
	GetTextExtentPoint32( hWindowDC, lpString, nCount, &sizeText );
	rectText.left = 0;
	rectText.top  = 0;
	rectText.right = sizeText.cx;
	rectText.bottom = sizeText.cy;
	lpRect = &rectText;
	// Set the bitmap info...
	BitmapInfo.bmiHeader.biSize          = sizeof( BITMAPINFOHEADER );
	BitmapInfo.bmiHeader.biWidth         = lpRect->right - lpRect->left;
	BitmapInfo.bmiHeader.biHeight        = lpRect->bottom - lpRect->top;
	BitmapInfo.bmiHeader.biPlanes        = 1;
	BitmapInfo.bmiHeader.biBitCount      = 24;
	BitmapInfo.bmiHeader.biCompression   = BI_RGB;
	BitmapInfo.bmiHeader.biSizeImage     = 0;
	BitmapInfo.bmiHeader.biClrImportant  = 0;
	BitmapInfo.bmiHeader.biClrUsed       = 0;
	BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	hBitmap = CreateDIBSection( hWindowDC, &BitmapInfo, DIB_RGB_COLORS, (void**)&pBitmap, NULL, 0 );
	SelectObject(hWindowDC, oldFont);
	::ReleaseDC( NULL, hWindowDC );

	memset( pBitmap, 0xff, BitmapInfo.bmiHeader.biHeight * BitmapInfo.bmiHeader.biWidth * 3 );

	hWindowDC = CreateCompatibleDC( NULL );

	if( IsPointOutside( lpPoint->x, lpPoint->y ) &&
		IsPointOutside( lpPoint->x + sizeText.cx, lpPoint->y + sizeText.cy ) &&
		IsPointOutside( lpPoint->x, lpPoint->y + sizeText.cy ) &&
		IsPointOutside( lpPoint->x + sizeText.cx, lpPoint->y ) ) return TRUE;

	oldFont = SelectObject(hWindowDC, font.m_hFont);
	hOldBitmap = (HBITMAP)::SelectObject( hWindowDC, hBitmap );

	// Draw the text to the dib section...
	::DrawText( hWindowDC, lpString, nCount, &rectText, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

	unsigned char* pVideoBuffer = m_pVB;
	// Copy it to the back buffer...
	// Set the font...
	SetBkMode( hWindowDC, TRANSPARENT );

	COLORREF crTransparent = GetBitmapPointColor( pBitmap );
	for( int nX = 0; nX < BitmapInfo.bmiHeader.biWidth ; nX ++ )
		for( int nY = 1; nY < BitmapInfo.bmiHeader.biHeight ; nY ++ )
		{
			if( GetBitmapPointColor( pBitmap +  ( nX ) * 3 +
											    ( BitmapInfo.bmiHeader.biHeight - nY ) *
											     BitmapInfo.bmiHeader.biWidth * 3
								   ) == crTransparent
			  ) continue;

			DrawPixel( nX + lpPoint->x, nY + lpPoint->y, nColor );
		}

	// cleanup
	::SelectObject( hWindowDC, hOldBitmap );
	SelectObject(hWindowDC, oldFont);
	DeleteObject( hBitmap );
    DeleteDC( hWindowDC );
	return TRUE;
}

inline int CCEDraw::TestPoint( unsigned int a,int k )
{
	a<<=(k-1);
	if(a&0x80)
	return (1);
	else
	return (0);
}

//
// Function : DrawText()
// This method draws string to the screen use hzk16 or 24
//
BOOL CCEDraw::DrawText( LPPOINT lpPoint, LPSTR lpString, int nCount, UINT uFormat )
{
	int X = lpPoint->x;
	int Y = lpPoint->y;
	unsigned int n=0;
	char zw[256];

	if( NULL == m_pFontFile )
	{
		m_dwLastError = CEG_DRAW_FONTFILE_NULL;
		return FALSE;
	}
	if(  IsPointOutside( lpPoint->x, lpPoint->y ) &&
		 IsPointOutside( lpPoint->x + nCount* 16, lpPoint->y + 16 ) &&
		 IsPointOutside( lpPoint->x + nCount* 16, lpPoint->y ) &&
		 IsPointOutside( lpPoint->x, lpPoint->y + 16 ) ) return TRUE;

	strcpy( zw, lpString );
    while( n != strlen( zw ) )
	{   register int i,j;
		register int xx=0;
		register int yy=0;
		char wm[32];
		long num;

		if((zw[n]&0x80)==0)
		{
			num=188+zw[n]-33;
			memcpy( wm, (m_pFontFile + num*32), 32 );
			for(i=0;i<32;i++)
			{
				for(j=1;j<=8;j++)
				   if(TestPoint(wm[i],j)==0) xx++;
				   else
				   {
					   DrawPixel((xx)+X,Y+yy,m_pPen->m_Color);xx++;
				   }
				   if( (i+1)%2==0 )
				   {
					   xx=0;
					   yy++;
				   }

			}
			X=X+16;
			n=n+1;
		}
		else
		{
		    zw[n]=zw[n]&0x7f;
			zw[n+1]=zw[n+1]&0x7f;
			zw[n]=zw[n]-0x20;
			zw[n+1]=zw[n+1]-0x20;
			num=(zw[n]-1)*94+(zw[n+1]-1);
			memcpy( wm, (m_pFontFile + num*32), 32 );
			for(i=0;i<32;i++)
			{
				for(j=1;j<=8;j++)
					if(TestPoint(wm[i],j)==0) xx++;
					else
					{
						DrawPixel((xx)+X,Y+yy,m_pPen->m_Color);
						xx++;
					}
					if((i+1)%2==0)
					{
						xx=0;
						yy++;
					}
			}
			X=X+16;n=n+2;
		}
	}
   return TRUE;
}

//
// Function : DrawBitmap()
// Draw the bitmap to the screen...
//
BOOL CCEDraw::DrawBitmap( CCEBitmap* pBitmap, int nXDest, int nYDest, int nWidth, int nHeight, int nXSrc, int nYSrc, DWORD dwRop, float fAlpha )
{
	return pBitmap->BitBlt( this, nXDest, nYDest, nWidth, nHeight, nXSrc, nYSrc, dwRop, fAlpha );
}

//
// Function : DrawRect()
// Draw the rectangle
//
BOOL CCEDraw::DrawRect( LONG lStartX, LONG lStartY, LONG lEndX, LONG lEndY )
{
	LONG lTemp;
	if( IsPointOutside( lStartX, lStartY ) && IsPointOutside( lEndX, lEndY ) ) return TRUE;
	if( lStartX > lEndX )
	{
		lTemp = lStartX;
		lStartX = lEndX;
		lEndX = lTemp;
	}
	if( lStartY > lEndY )
	{
		lTemp = lStartY;
		lStartY = lEndY;
		lEndY = lTemp;
	}

	// Draw hor line
	DrawHLine( lStartX, lStartY, lEndX, lStartY );
	DrawHLine( lStartX, lEndY, lEndX, lEndY );
	// Draw v line
	DrawVLine( lStartX, lStartY, lStartX, lEndY );
	DrawVLine( lEndX, lStartY, lEndX, lEndY );

	return TRUE;
}

BOOL CCEDraw::DrawRect( RECT &rectDraw )
{
	return DrawRect( rectDraw.left, rectDraw.top , rectDraw.right, rectDraw.bottom );
}

//
// Function : FillRect()
// Draw a filled rectangle
//
BOOL CCEDraw::FillRect( LONG lStartX, LONG lStartY, LONG lEndX, LONG lEndY, FLOAT fAlpha )
{
	LONG lTemp;
	if( IsPointOutside( lStartX, lStartY ) && IsPointOutside( lEndX, lEndY ) ) return TRUE;
	if( lStartX > lEndX )
	{
		lTemp = lStartX;
		lStartX = lEndX;
		lEndX = lTemp;
	}
	if( lStartY > lEndY )
	{
		lTemp = lStartY;
		lStartY = lEndY;
		lEndY = lTemp;
	}
	if( lStartY < 0 ) lStartY = 0;
	if( lEndY >= (LONG)m_gxdp.cyHeight ) lEndY = (LONG)m_gxdp.cyHeight - 1;
	if( lStartX < 0 ) lStartX = 0;
	if( lEndX >= (LONG)m_gxdp.cxWidth ) lEndX = m_gxdp.cxWidth - 1;

	unsigned char* pBuffer = ( m_pVB + lStartY*m_cbyPitch + m_cbxPitch*lStartX );
	unsigned char* pTempBuffer = pBuffer;
	unsigned short ColorR, ColorG, ColorB, Color;
	// First draw if the color deepth is 8
	if( m_gxdp.cBPP == 8 )
	{
		for( int nY = lStartY ; nY < lEndY ; nY ++ )
		{
			pBuffer = pTempBuffer;
			for( int nX = lStartX ; nX < lEndX ; nX ++ )
			{
				*pBuffer = (BYTE)m_pBrush->m_Color;
				pBuffer += m_cbxPitch;
			}
			pTempBuffer += m_cbyPitch;
		}
	}
	else if( m_gxdp.cBPP == 16 )
	{
		if( m_gxdp.ffFormat & kfDirect565 && fAlpha != 1 )
		{
			for( int nY = lStartY ; nY < lEndY ; nY ++ )
			{
				pBuffer = pTempBuffer;
				for( int nX = lStartX ; nX < lEndX ; nX ++ )
				{
					/* n% ALPHA */
					Color  = *(unsigned short*)pBuffer;
					ColorR = (unsigned short)((Color >> 11) & 0x1f) * ( 1 - fAlpha );
					ColorG = (unsigned short)((Color >> 5) & 0x3f) * ( 1 - fAlpha );
					ColorB = (unsigned short)(Color & 0x1f) * ( 1 - fAlpha );
					Color  = m_pBrush->m_Color;
					ColorR = (unsigned short)(((Color >> 11)&0x1f) * fAlpha + ColorR);
					ColorG = (unsigned short)(((Color >> 5)&0x3f) * fAlpha + ColorG);
					ColorB = (unsigned short)((Color & 0x1f)* fAlpha + ColorB);
					Color  = (unsigned short)( ( ColorR & 0xff ) << 11 |
											  ( ColorG & 0xff ) << 5 |
											  ( ColorB & 0xff ) );
					*(unsigned short*)pBuffer = Color;
					pBuffer += m_cbxPitch;
				}
				pTempBuffer += m_cbyPitch;
			}
		}
		else if( m_gxdp.ffFormat & kfDirect555 && fAlpha != 1 )
		{
			for( int nY = lStartY ; nY < lEndY ; nY ++ )
			{
				pBuffer = pTempBuffer;
				for( int nX = lStartX ; nX < lEndX ; nX ++ )
				{
					/* n% ALPHA */
					Color  = *(unsigned short*)pBuffer;
					ColorR = (unsigned short)((Color >> 10) & 0x1f) * ( 1 - fAlpha );
					ColorG = (unsigned short)((Color >> 5) & 0x1f) * ( 1 - fAlpha );
					ColorB = (unsigned short)(Color & 0x1f) * ( 1 - fAlpha );
					Color  = m_pBrush->m_Color;
					ColorR = (unsigned short)(((Color >> 10)&0x1f) * fAlpha + ColorR);
					ColorG = (unsigned short)(((Color >> 5)&0x1f) * fAlpha + ColorG);
					ColorB = (unsigned short)((Color & 0x1f)* fAlpha + ColorB);
					Color  = (unsigned short)( ( ColorR & 0xff ) << 10 |
											  ( ColorG & 0xff ) << 5 |
											  ( ColorB & 0xff ) );
					*(unsigned short*)pBuffer = Color;
					pBuffer += m_cbxPitch;
				}
				pTempBuffer += m_cbyPitch;
			}
		}
		else
		{
			for( int nY = lStartY ; nY < lEndY ; nY ++ )
			{
				pBuffer = pTempBuffer;
				for( int nX = lStartX ; nX < lEndX ; nX ++ )
				{
					*(unsigned short*)pBuffer = m_pBrush->m_Color;
					pBuffer += m_cbxPitch;
				}
				pTempBuffer += m_cbyPitch;
			}
		}
	}
	return TRUE;
}

BOOL CCEDraw::FillRect( RECT &rectDraw, FLOAT fAlpha )
{
	return FillRect( rectDraw.left, rectDraw.top , rectDraw.right, rectDraw.bottom, fAlpha );
}