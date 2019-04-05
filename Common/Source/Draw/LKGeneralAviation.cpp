/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKGeneralAviation.cpp,v 1.1 2010/12/11 19:06:34 root Exp root $
   $Id: LKGeneralAviation.cpp,v 1.2 2019/03/31 21:12:56 aircaft now at the center of the compass rose $

   LKGeneralAviation.cpp original work by Oren
   Improved by Alberto Realis-Luc
*/

#include "externs.h"
#include "LKObjects.h"
#include "RGB.h"

void drawOutlineText(LKSurface& Surface, int x, int y, const TCHAR * textBuffer) {
#ifdef USE_FREETYPE
#warning "to slow, rewrite using freetype outline"
#endif
	Surface.SetTextColor(RGB_BLACK);
	Surface.DrawText(x -1, y -1, textBuffer);
	Surface.DrawText(x +1, y +1, textBuffer);
	Surface.DrawText(x -1, y   , textBuffer);
	Surface.DrawText(x   , y +1, textBuffer);
	Surface.SetTextColor(RGB_WHITE);
	Surface.DrawText(x, y, textBuffer);
}

void MapWindow::DrawGAscreen(LKSurface& Surface, const POINT& AircraftPos, const RECT& rc) {

	// Only for Trackup and compatible modes
	if (DisplayOrientation == NORTHSMART ||
		DisplayOrientation == NORTHTRACK ||
		DisplayOrientation == NORTHUP ||
		MapWindow::mode.autoNorthUP() ||
		MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)
	) return;

	// Print current heading
	TCHAR textBuffer[4];
	const auto oldfont = Surface.SelectObject(LK8InfoNormalFont); // always remember to save object or we miss font
	const int hdg = (int)round(DrawInfo.TrackBearing);
	_stprintf(textBuffer,_T("%03d"), hdg);
	SIZE textSize;
	Surface.GetTextSize(textBuffer, &textSize); // get size of heading printed digits

	// Calculate center and radius of compass rose
	const int cx = AircraftPos.x; // center of compass arc : the plane
	const int cy = AircraftPos.y;
	const int radius = cy - textSize.cy - 12; // to calculate the radius consider the offset below heading pointer

	// Draw heading pointer on the top of the compass
	const int halfBrgSize = textSize.cx/2;
	const int deciBrgSize = textSize.cx/10;
	drawOutlineText(Surface, cx - halfBrgSize, 0, textBuffer);
	const POINT hdgPointer[7] = {
			{cx - halfBrgSize - 5, textSize.cy - 5},
			{cx - halfBrgSize - 5, textSize.cy + 2},
			{cx - deciBrgSize, textSize.cy + 2},
			{cx , textSize.cy + 7},
			{cx + deciBrgSize, textSize.cy + 2},
			{cx + halfBrgSize +5, textSize.cy + 2},
			{cx + halfBrgSize +5, textSize.cy - 5}
	};
	const auto oldpen=Surface.SelectObject(LKPen_Black_N3);
	Surface.Polyline(hdgPointer,7);
	Surface.SelectObject(LKPen_White_N2);
	Surface.Polyline(hdgPointer,7);

	// Draw black part of 120 degree arac
	static const int rightArc = 60;
	static const int leftArc = (int)AngleLimit360(-rightArc);
	Surface.SelectObject(LKPen_Black_N5);
	Surface.DrawArc(cx, cy,radius, rc, leftArc, rightArc);

	// Draw the dents around the circle
	static const int step = 10;
	const int diff = (int)round(hdg % step);
	Surface.SelectObject(LK8MediumFont);
	for(int i = -rightArc, screenAngle = leftArc - diff, curHdg = hdg - rightArc - diff; i <= rightArc; i += step, screenAngle += step, curHdg += step) {
		screenAngle = (int)AngleLimit360(screenAngle);
		if (screenAngle < leftArc && screenAngle > rightArc) continue;
		const int displayHeading = (int)AngleLimit360(curHdg);
		int tickLength = radius - 10; // the length of the tickmarks on the compass rose
		if(displayHeading % 30 == 0) {
			tickLength -= 8; // if the heading is a multiple of ten, it gets a long tick
			const int drawHdg = displayHeading/10;
			switch (drawHdg) {
			case 0:
				_stprintf(textBuffer, _T("N"));
				break;
			case 9:
				_stprintf(textBuffer, _T("E"));
				break;
			case 18:
				_stprintf(textBuffer, _T("S"));
				break;
			case 27:
				_stprintf(textBuffer, _T("W"));
				break;
			default:
				_stprintf(textBuffer, _T("%d"), drawHdg);
				break;
			}
			Surface.GetTextSize(textBuffer, &textSize);
			const int textX = cx + (int) ((radius - (textSize.cy/2)-2) * fastsine(screenAngle)) - textSize.cx/2;
			const int textY = cy - (int) ((radius - (textSize.cy/2)-2) * fastcosine(screenAngle));
			drawOutlineText(Surface, textX, textY, textBuffer);
		}
		const POINT dent[2] = {
			{ cx + (int)(radius     * fastsine(screenAngle)), cy - (int)(radius     * fastcosine(screenAngle)) },
			{ cx + (int)(tickLength * fastsine(screenAngle)), cy - (int)(tickLength * fastcosine(screenAngle)) }
		};
		Surface.SelectObject(LKPen_Black_N5);
		Surface.Polyline(dent,2);
		Surface.SelectObject(LKPen_White_N2);
		Surface.Polyline(dent,2);
	}

	// Draw white part of the 120 degree arc
	Surface.SelectObject(LKPen_White_N2);
	Surface.DrawArc(cx, cy, radius, rc, leftArc, rightArc);

	// Restore previous text color, pen and font
	Surface.SetTextColor(RGB_BLACK);
	Surface.SelectObject(oldpen);
	Surface.SelectObject(oldfont);
}
