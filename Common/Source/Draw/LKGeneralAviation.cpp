/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: LKGeneralAviation.cpp, v1.1 2010/12/11 19:06:34 root Exp root $
   $Id: LKGeneralAviation.cpp, v1.2 2019/03/31 by Alus Fixed: aircraft now always at the center of the compass rose $
   $Id: LKGeneralAviation.cpp, v1.3 2019/04/10 by Alus added rate of turn indication on the compass$

   LKGeneralAviation.cpp original work by Oren
   Improved by Alberto Realis-Luc (Alus)
*/

#include "externs.h"
#include "LKObjects.h"
#include "RGB.h"

void drawOutlineText(LKSurface& Surface, int x, int y, const TCHAR * textBuffer) {
	Surface.SetTextColor(MapWindow::GetOutlineColor(OverColorRef));

	Surface.DrawText(x -1, y -1, textBuffer);
	Surface.DrawText(x +1, y +1, textBuffer);
	Surface.DrawText(x -1, y   , textBuffer);
	Surface.DrawText(x   , y +1, textBuffer);
	Surface.SetTextColor(OverColorRef);
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

	// The aircraft symbol must be horizontally in the center
	LKASSERT(AircraftPos.x == rc.right/2);

	// Print current heading
	TCHAR textBuffer[11];
	const auto oldfont = Surface.SelectObject(LK8InfoNormalFont); // always remember to save object or we miss font
	const int hdg = (int)round(DrawInfo.TrackBearing);
	_stprintf(textBuffer,_T("%03d"), hdg);
	SIZE textSize;
	Surface.GetTextSize(textBuffer, &textSize); // get size of heading printed digits
	const int halfBrgSize = textSize.cx/2;
	const int screenOrientYoffset = ScreenLandscape ? 0 : 44;
	const int brgYoffset = textSize.cy + screenOrientYoffset;
	drawOutlineText(Surface, AircraftPos.x - halfBrgSize, screenOrientYoffset, textBuffer);


	LKPen OverlayPen(PEN_SOLID, NIBLSCALE(1), OverColorRef);
	LKPen OutlinePen(PEN_SOLID, NIBLSCALE(3), GetOutlineColor(OverColorRef));

	// Calculate compass radius: consider the offset below heading pointer
	const int radius = AircraftPos.y - brgYoffset - 12;

	// Draw heading pointer on the top of the compass

	const auto oldpen=Surface.SelectObject(OutlinePen);

	const int deciBrgSize = textSize.cx/10;
	const POINT hdgPointer[7] = {
			{AircraftPos.x - halfBrgSize - 5, brgYoffset - 5},
			{AircraftPos.x - halfBrgSize - 5, brgYoffset + 2},
			{AircraftPos.x - deciBrgSize    , brgYoffset + 2},
			{AircraftPos.x                  , brgYoffset + 7},
			{AircraftPos.x + deciBrgSize    , brgYoffset + 2},
			{AircraftPos.x + halfBrgSize + 5, brgYoffset + 2},
			{AircraftPos.x + halfBrgSize + 5, brgYoffset - 5}
	};

	Surface.Polyline(hdgPointer,7);
	Surface.SelectObject(OverlayPen);
	Surface.Polyline(hdgPointer,7);

	// Calculate the angle covered by the compass on each side
	const int rightArc = ScreenLandscape ? 60 : (ScreenSize == ss272x480 || ScreenSize == ss480x800 ? 25 : 30) ;
	const int leftArc = (int)AngleLimit360(-rightArc);

	OverlayPen.Create(PEN_SOLID, NIBLSCALE(2), OverColorRef);
	OutlinePen.Create(PEN_SOLID, NIBLSCALE(5), GetOutlineColor(OverColorRef));

	Surface.SelectObject(OutlinePen);
	Surface.DrawArc(AircraftPos.x, AircraftPos.y,radius, rc, leftArc, rightArc);

	// Draw the dents around the circle
	const int step = ScreenLandscape ? 10 : 5;
	const int bigStep = ScreenLandscape ? 30 : 10;
	const int diff = (int)round(hdg % step);
	Surface.SelectObject(LK8MediumFont);
	for(int i = -rightArc, screenAngle = leftArc - diff, curHdg = hdg - rightArc - diff; i <= rightArc; i += step, screenAngle += step, curHdg += step) {
		screenAngle = (int)AngleLimit360(screenAngle);
		if (screenAngle < leftArc && screenAngle > rightArc) continue;
		const int displayHeading = (int)AngleLimit360(curHdg);
		int tickLength = radius - 10; // the length of the tickmarks on the compass rose
		const double& sinus = fastsine(screenAngle);
		const double& cosinus = fastcosine(screenAngle);
		if (displayHeading % bigStep == 0) {
			tickLength -= 8; // make longer ticks
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
			const int textX = AircraftPos.x + (int) ((radius - (textSize.cy/2)-2) * sinus) - textSize.cx/2;
			const int textY = AircraftPos.y - (int) ((radius - (textSize.cy/2)-2) * cosinus);
			drawOutlineText(Surface, textX, textY, textBuffer);
		}
		const POINT dent[2] = {
			{ AircraftPos.x + (int)(radius     * sinus), AircraftPos.y - (int)(radius     * cosinus) },
			{ AircraftPos.x + (int)(tickLength * sinus), AircraftPos.y - (int)(tickLength * cosinus) }
		};
		Surface.SelectObject(OutlinePen);
		Surface.Polyline(dent,2);
		Surface.SelectObject(OverlayPen);
		Surface.Polyline(dent,2);
	}

	// Draw overlay part of the compass arc

	const auto oldPen = Surface.SelectObject(OverlayPen);
	Surface.DrawArc(AircraftPos.x, AircraftPos.y, radius, rc, leftArc, rightArc);
	Surface.SelectObject(oldPen);

	// Draw rate of turn indication on the compass
	if (DerivedDrawInfo.TurnRate != 0) {
		double displayROTangle = fabs(DerivedDrawInfo.TurnRate);
		if (displayROTangle <= rightArc) {
			const double absTurn6sec = displayROTangle * 6; // to point to the heading we will have in 6 seconds
			const bool is6secTurnScale(absTurn6sec <= rightArc); // if the 6 seconds turn fits on the compass scale
			if (is6secTurnScale) displayROTangle = absTurn6sec;
			LKPen Pen(PEN_SOLID, NIBLSCALE(3), is6secTurnScale ? RGB_MAGENTA : RGB_RED); // Color: purple for the deg/6 sec turn otherwise red for deg/sec
			Surface.SelectObject(Pen);
			if (DerivedDrawInfo.TurnRate > 0) Surface.DrawArc(AircraftPos.x, AircraftPos.y, radius, rc, 0, displayROTangle); // turning right
			else Surface.DrawArc(AircraftPos.x, AircraftPos.y, radius, rc, -displayROTangle, 0); // turning left
		}
	}

	// Restore previous text color, pen and font
	Surface.SetTextColor(RGB_BLACK);
	Surface.SelectObject(oldpen);
	Surface.SelectObject(oldfont);
}
