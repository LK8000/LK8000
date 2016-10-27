/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKGeneralAviation.cpp,v 1.1 2010/12/11 19:06:34 root Exp root $

   LKGeneralAviation.cpp by Oren
*/

/////////////////////////////

#include "externs.h"
#include "LKObjects.h"
#include "RGB.h"




////////////////////////////////////////////////////////////////////////////////////
void drawOutlineText(LKSurface& Surface,int x,int y,const TCHAR * textBuffer, const LKColor& color )
{
#ifdef USE_FREETYPE
#warning "to slow, rewrite using freetype outline"
#endif
	Surface.SetTextColor(RGB_BLACK);
	Surface.DrawText(x -1, y -1, textBuffer);
	Surface.DrawText(x +1, y +1, textBuffer);
	Surface.DrawText(x -1, y   , textBuffer);
	Surface.DrawText(x   , y +1, textBuffer);

	Surface.SetTextColor(color);
	Surface.DrawText(x , y , textBuffer);
}


////////////////////////////////////////////////////////////////////////////////////
int MapWindow::DrawCompassArc(LKSurface& Surface, long x, long y, int radius, const RECT& rc,
	    double bearing)

{
		const int indicatorStep = 10;

		// For Oren: remember to always DeleteObject you create with Create, or in 1 hour any
		// device will run out of GDI space, including your PC...
		// Meanwhile, I have created LKObjects, so anytime we should use them . No need to delete them.
		//HPEN hPenBlack = ::CreatePen(PEN_SOLID, (5), LKColor(0x00,0x0,0x0));
		//HPEN hPenWhite = (HPEN)CreatePen(PEN_SOLID, (2), LKColor(0xff,0xff,0xff));
		const PenReference hPenBlack = LKPen_Black_N5;
		const PenReference hPenWhite = LKPen_White_N2;

		const auto oldpen=Surface.SelectObject(hPenBlack);
		Surface.DrawArc(x, y,radius, rc, 300, 60);

		int heading = (int)(bearing +0.5);

		// -----------Draw the detents around the circle-----------------------------

		double angleDiff = heading % indicatorStep;

		int screenAngle = (int)(300 - angleDiff);
		int curHeading = (int)(heading - 60 - angleDiff);
		TCHAR textBuffer[32];

		POINT pt[2];
		int i;

		const auto oldfont = Surface.SelectObject(LK8MediumFont); // always remember to save object or we miss font

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
						_stprintf( textBuffer, _T("N"));
						break;
					case 9:
						_stprintf( textBuffer, _T("E"));
						break;
					case 18:
						_stprintf( textBuffer, _T("S"));
						break;
					case 27:
						_stprintf( textBuffer, _T("W"));
						break;
					default:
						_stprintf( textBuffer, _T("%d"), displayHeading/10 );
						break;
					}

					SIZE textSize;
					Surface.GetTextSize(textBuffer, &textSize);

					int textX = x + (long) ((radius - (textSize.cy/2)-2) * fastsine(screenAngle) ) - textSize.cx/2;
					int textY = y - (long) ((radius - (textSize.cy/2)-2) * fastcosine(screenAngle) );
					drawOutlineText(Surface, textX,textY ,textBuffer,RGB_WHITE);
				}
			}
			else // Otherwise it gets a short tick
				tickLength = 10;

			pt[1].x = x + (long) ((radius -tickLength) * fastsine(screenAngle) );
			pt[1].y = y - (long) ((radius -tickLength) * fastcosine(screenAngle) );
			Surface.SelectObject(hPenBlack);
			Surface.Polyline(pt,2);
			Surface.SelectObject(hPenWhite);
			Surface.Polyline(pt,2);
		}

		Surface.SelectObject(hPenWhite);
		Surface.DrawArc(x, y,radius, rc, 300, 60);
		Surface.SelectObject(oldfont);
		Surface.SelectObject(oldpen);
		return 0;
}



////////////////////////////////////////////////////////////////////////////////////
void MapWindow::DrawHSIarc(LKSurface& Surface, const POINT& Orig, const RECT& rc )
{
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
	//HPEN hPenBlack = ::CreatePen(PEN_SOLID, (5), LKColor(0x00,0x0,0x0));
	//HPEN hPenWhite = (HPEN)CreatePen(PEN_SOLID, (2), LKColor(0xff,0xff,0xff));
	const PenReference hPenBlack = LKPen_Black_N3;
	const PenReference hPenWhite = LKPen_White_N2;

	const auto oldfont = Surface.SelectObject(LK8InfoNormalFont);

	//Draw current heading
	///////////////////

	TCHAR brgText[LKSIZEBUFFERLARGE];
	int bearing= (int) (DrawInfo.TrackBearing+0.5);
	_stprintf(brgText,_T("%03d"),bearing);

	SIZE brgSize;
	Surface.GetTextSize(brgText, &brgSize);
	drawOutlineText(Surface, rcx -(brgSize.cx/2), 0 ,brgText,RGB_WHITE);

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

	const auto oldpen=Surface.SelectObject(hPenBlack);
	Surface.Polyline(pt,7);
	Surface.SelectObject(hPenWhite);
	Surface.Polyline(pt,7);

	//Offset arc below heading
	rcy += brgSize.cy +10;

	DrawCompassArc(Surface,Orig.x,rcy,rad,rc,DrawInfo.TrackBearing);

	Surface.SetTextColor(RGB_BLACK);
	Surface.SelectObject(oldpen);
	Surface.SelectObject(oldfont);

}
