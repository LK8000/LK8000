/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKGeneralAviation.cpp,v 1.1 2010/12/11 19:06:34 root Exp root $

   LKGeneralAviation.cpp by Oren
*/

/////////////////////////////

#include "StdAfx.h"

#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#endif

#include "Defines.h"
#include "Utils.h"
#include "Utils2.h"
#include "LKUtils.h"
#include "LKObjects.h"
#include "externs.h"

#include "utils/heapcheck.h"



////////////////////////////////////////////////////////////////////////////////////
void drawOutlineText(HDC hdc,int x,int y,const TCHAR * textBuffer,COLORREF color )
{
	size_t len = _tcslen(textBuffer);


	SetTextColor(hdc, RGB_BLACK);
	ExtTextOut( hdc,x -1, y -1, ETO_OPAQUE, NULL, textBuffer , len, NULL );
	ExtTextOut( hdc,x +1, y +1, ETO_OPAQUE, NULL, textBuffer , len, NULL );
	ExtTextOut( hdc,x -1, y   , ETO_OPAQUE, NULL, textBuffer , len, NULL );
	ExtTextOut( hdc,x   , y +1, ETO_OPAQUE, NULL, textBuffer , len, NULL );

	SetTextColor(hdc, color);
	ExtTextOut( hdc,x , y , ETO_OPAQUE, NULL, textBuffer , len, NULL );

}


////////////////////////////////////////////////////////////////////////////////////
int DrawCompassArc(HDC hdc, long x, long y, int radius, RECT rc,
	    double bearing)

{
		const int indicatorStep = 10;

		// For Oren: remember to always DeleteObject you create with Create, or in 1 hour any
		// device will run out of GDI space, including your PC...
		// Meanwhile, I have created LKObjects, so anytime we should use them . No need to delete them.
		//HPEN hPenBlack = ::CreatePen(PS_SOLID, (5), RGB(0x00,0x0,0x0));  
		//HPEN hPenWhite = (HPEN)CreatePen(PS_SOLID, (2), RGB(0xff,0xff,0xff));
		HPEN hPenBlack = LKPen_Black_N5;
		HPEN hPenWhite = LKPen_White_N2;
		HFONT oldfont;
		HPEN oldpen;

		oldpen=(HPEN) SelectObject(hdc, hPenBlack);
		DrawArc(hdc, x, y,radius, rc, 300, 60);

		int heading = (int)(bearing +0.5);

		// -----------Draw the detents around the circle-----------------------------

		double angleDiff = heading % indicatorStep;

		int screenAngle = (int)(300 - angleDiff);
		int curHeading = (int)(heading - 60 - angleDiff);
		TCHAR textBuffer[32];

		POINT pt[2];
		int i;

		oldfont=(HFONT)SelectObject(hdc, LK8MediumFont); // always remember to save object or we miss font 

		for(i = - 60; i<= 60;
				i+=indicatorStep,screenAngle += indicatorStep,curHeading += indicatorStep)
		{

			if ( (screenAngle < 300) && (screenAngle > 60) )
 			{
				continue;
			}

			screenAngle  = (int)AngleLimit360(screenAngle);

			pt[0].x = x + (long) (radius * fastsine(screenAngle) );
			pt[0].y = y - (long) (radius * fastcosine(screenAngle) );

			// The length of the tickmarks on the compass rose
			double tickLength;

			// Make sure the display heading is between 0 and 360
			int displayHeading = (int)AngleLimit360(curHeading);

			// If the heading is a multiple of ten, it gets a long tick
			if(displayHeading%30==0)
			{
				tickLength = 15;

				if(displayHeading%30==0)
				{

					int drawHdg = displayHeading/10;
					switch ( drawHdg )
					{
					case 0:
						wsprintf( textBuffer, _T("N"));
						break;
					case 9:
						wsprintf( textBuffer, _T("E"));
						break;
					case 18:
						wsprintf( textBuffer, _T("S"));
						break;
					case 27:
						wsprintf( textBuffer, _T("W"));
						break;
					default:
						wsprintf( textBuffer, _T("%d"), displayHeading/10 );
					}

					SIZE textSize;
					GetTextExtentPoint(hdc, textBuffer, _tcslen(textBuffer), &textSize);

					int textX = x + (long) ((radius - (textSize.cy/2)-2) * fastsine(screenAngle) ) - textSize.cx/2;
					int textY = y - (long) ((radius - (textSize.cy/2)-2) * fastcosine(screenAngle) );
					drawOutlineText(hdc,textX,textY ,textBuffer,RGB_WHITE);
				}
			}
			else // Otherwise it gets a short tick
				tickLength = 10;

			pt[1].x = x + (long) ((radius -tickLength) * fastsine(screenAngle) );
			pt[1].y = y - (long) ((radius -tickLength) * fastcosine(screenAngle) );
			SelectObject(hdc, hPenBlack);
			::Polyline(hdc,pt,2);
			SelectObject(hdc, hPenWhite);
			::Polyline(hdc,pt,2);
		}

		SelectObject(hdc, hPenWhite);
		DrawArc(hdc, x, y,radius, rc, 300, 60);
		SelectObject(hdc, oldfont);
		SelectObject(hdc, oldpen);
		return 0;
}



////////////////////////////////////////////////////////////////////////////////////
void DrawHSI(HDC hdc, POINT Orig, RECT rc )
{
	HFONT oldfont;
	HPEN oldpen;

	// short rcHeight = rc.bottom;
	short rcx=rc.left+rc.right/2;
	short rad=(rc.right/2) - (rcx/10);
	short rcy= rad;

	if ( DisplayOrientation == NORTHSMART || 
		DisplayOrientation == NORTHTRACK ||
		DisplayOrientation == NORTHUP ||
		MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)
		)
	{
		return; //Only for Trackup and compatible modes
	}


	//XXXOREN - move to globals
	//HPEN hPenBlack = ::CreatePen(PS_SOLID, (5), RGB(0x00,0x0,0x0));
	//HPEN hPenWhite = (HPEN)CreatePen(PS_SOLID, (2), RGB(0xff,0xff,0xff));
	HPEN hPenBlack = LKPen_Black_N3;
	HPEN hPenWhite = LKPen_White_N2;

	oldfont = (HFONT)SelectObject(hdc, LK8InfoNormalFont);

	//Draw current heading
	///////////////////

	TCHAR brgText[LKSIZEBUFFERLARGE];
	int bearing= (int) (GPS_INFO.TrackBearing+0.5);
	wsprintf(brgText,_T("%03d"),bearing);

	SIZE brgSize;
	GetTextExtentPoint(hdc, brgText, _tcslen(brgText), &brgSize);
	drawOutlineText(hdc,rcx -(brgSize.cx/2), 0 ,brgText,RGB_WHITE);

	//Draw pointer
	POINT pt[7];
	pt[0].x = rcx - (brgSize.cx/2) -5;
	pt[0].y = brgSize.cy - 5;

	pt[1].x = rcx - (brgSize.cx/2) -5;
	pt[1].y = brgSize.cy + 2;

	pt[2].x = rcx - (brgSize.cx/10);
	pt[2].y = brgSize.cy + 2;

	pt[3].x = rcx;
	pt[3].y = brgSize.cy + 7;

	pt[4].x = rcx + (brgSize.cx/10);
	pt[4].y = brgSize.cy + 2;

	pt[5].x = rcx + (brgSize.cx/2) +5;
	pt[5].y = brgSize.cy + 2;

	pt[6].x = rcx + (brgSize.cx/2) +5;
	pt[6].y = brgSize.cy - 5;

	oldpen=(HPEN) SelectObject(hdc, hPenBlack);
	::Polyline(hdc,pt,7);
	SelectObject(hdc, hPenWhite);
	::Polyline(hdc,pt,7);

	//Offset arc below heading
	rcy += brgSize.cy +10;

	DrawCompassArc(hdc,Orig.x,rcy,rad,rc,GPS_INFO.TrackBearing);

	SetTextColor(hdc,RGB_BLACK);
	SelectObject(hdc,(HPEN)oldpen);
	SelectObject(hdc,(HFONT) oldfont);

}



