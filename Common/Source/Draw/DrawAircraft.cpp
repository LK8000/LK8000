/*
 LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 Released under GNU/GPL License v.2
 See CREDITS.TXT file for authors and copyrights

 $Id$
 */

#include "externs.h"
#include "LKObjects.h"
#include "Multimap.h"

void MapWindow::DrawAircraft(LKSurface& Surface, const POINT& Orig) {

	const double angle = DisplayAircraftAngle
			+ (DerivedDrawInfo.Heading - DrawInfo.TrackBearing);

	if ((ISPARAGLIDER) || ISCAR) {

		switch (PGGliderSymbol) {

		case 0:  // Triangle
		{
			POINT AircraftInner[] =
					{ { 0, -5 }, { 5, 9 }, { -5, 9 }, { 0, -5 } };
			POINT AircraftOuter[] = { { 0, -6 }, { 6, 10 }, { -6, 10 },
					{ 0, -6 } };

			BrushReference hbPAircraftSolid;
			BrushReference hbPAircraftSolidBg;

			if (BlackScreen) {
				hbPAircraftSolid = LK_WHITE_BRUSH;
				hbPAircraftSolidBg = LKBrush_LightCyan;
			} else {
				hbPAircraftSolid = LK_WHITE_BRUSH;
				hbPAircraftSolidBg = LKBrush_Blue;
			}

			PolygonRotateShift(AircraftInner, array_size(AircraftInner), Orig.x,
					Orig.y, angle);
			PolygonRotateShift(AircraftOuter, array_size(AircraftOuter), Orig.x,
					Orig.y, angle);

			const auto oldPen = Surface.SelectObject(LK_NULL_PEN);

			const auto hbOld = Surface.SelectObject(hbPAircraftSolid);
			Surface.Polygon(AircraftOuter, array_size(AircraftOuter));

			Surface.SelectObject(hbPAircraftSolidBg);
			Surface.Polygon(AircraftInner, array_size(AircraftInner));

			Surface.SelectObject(oldPen);
			Surface.SelectObject(hbOld);
		}
			break;

		case 1: // Paraglider
		{
			POINT AircraftInner[] = { { -5, 3 }, { -11, 2 }, { -14, 1 }, { -14,
					-1 }, { -11, -2 }, { -5, -3 }, { 5, -3 }, { 11, -2 }, { 14,
					-1 }, { 14, 1 }, { 11, 2 }, { 5, 3 }, };

			PolygonRotateShift(AircraftInner, array_size(AircraftInner), Orig.x,
					Orig.y, angle);

			const auto oldPen = Surface.SelectObject(LK_NULL_PEN);

			const auto hbOld = Surface.SelectObject(LK_BLACK_BRUSH);

			Surface.SelectObject(LK_BLACK_BRUSH);
			Surface.Polygon(AircraftInner, array_size(AircraftInner));

			Surface.SelectObject(oldPen);
			Surface.SelectObject(hbOld);

		}
			break;

		case 2: // hanglider
		{
			POINT AircraftInner[] = { { 1, -3 }, { 7, 0 }, { 13, 4 }, { 13, 6 },
					{ 6, 3 }, { 1, 2 }, { -1, 2 }, { -6, 3 }, { -13, 6 }, { -13,
							4 }, { -7, 0 }, { -1, -3 }, };

			PolygonRotateShift(AircraftInner, array_size(AircraftInner), Orig.x,
					Orig.y, angle);

			const auto oldPen = Surface.SelectObject(LK_NULL_PEN);

			const auto hbOld = Surface.SelectObject(LK_BLACK_BRUSH);

			Surface.SelectObject(LK_BLACK_BRUSH);
			Surface.Polygon(AircraftInner, array_size(AircraftInner));

			Surface.SelectObject(oldPen);
			Surface.SelectObject(hbOld);

		}
			break;

		case 3: // Glider
		{
			POINT AircraftInner[] = { { 2, -6 }, { 2, -1 }, { 15, -1 },
					{ 15, 2 }, { 2, 2 }, { 2, 7 }, { 5, 7 }, { 5, 10 },
					{ -4, 10 }, { -4, 7 }, { -1, 7 }, { -1, 2 }, { -14, 2 }, {
							-14, -1 }, { -1, -1 }, { -1, -6 }, { 2, -6 }, };

			PolygonRotateShift(AircraftInner, array_size(AircraftInner), Orig.x,
					Orig.y, angle);

			const auto oldPen = Surface.SelectObject(LK_NULL_PEN);

			const auto hbOld = Surface.SelectObject(LK_BLACK_BRUSH);

			Surface.SelectObject(LK_BLACK_BRUSH);
			Surface.Polygon(AircraftInner, array_size(AircraftInner));

			Surface.SelectObject(oldPen);
			Surface.SelectObject(hbOld);
		}
			break;

		case 4: // Aircraft
		{
			POINT Aircraft[] = { { 1, -6 }, { 2, -1 }, { 15, 0 }, { 15, 2 }, {
					1, 2 }, { 0, 10 }, { 4, 11 }, { 4, 12 }, { -4, 12 }, { -4,
					11 }, { 0, 10 }, { -1, 2 }, { -15, 2 }, { -15, 0 },
					{ -2, -1 }, { -1, -6 }, { 1, -6 }

			};

			BrushReference hbAircraftSolid = LKBrush_Black;
			BrushReference hbAircraftSolidBg = LKBrush_White;

			const auto hbOld = Surface.SelectObject(hbAircraftSolidBg);
			const auto hpOld = Surface.SelectObject(hpAircraft);

			PolygonRotateShift(Aircraft, array_size(Aircraft), Orig.x, Orig.y,
					angle);

			Surface.Polygon(Aircraft, array_size(Aircraft));

			// draw it again so can get white border
			Surface.SelectObject(LKPen_White_N2);
			Surface.SelectObject(hbAircraftSolid);

			for (unsigned i = 0; i < array_size(Aircraft); i++) {
				Aircraft[i].x -= 1;
				Aircraft[i].y -= 1;
			}

			Surface.Polygon(Aircraft, array_size(Aircraft));

			Surface.SelectObject(hpOld);
			Surface.SelectObject(hbOld);
		}

			break;

		default:
			break;
		}

	} else if (ISGAAIRCRAFT) {

		POINT Aircraft[] = { { 1, -6 }, { 2, -1 }, { 15, 0 }, { 15, 2 },
				{ 1, 2 }, { 0, 10 }, { 4, 11 }, { 4, 12 }, { -4, 12 },
				{ -4, 11 }, { 0, 10 }, { -1, 2 }, { -15, 2 }, { -15, 0 }, { -2,
						-1 }, { -1, -6 }, { 1, -6 }

		};

		BrushReference hbAircraftSolid = LKBrush_Black;
		BrushReference hbAircraftSolidBg = LKBrush_White;

		const auto hbOld = Surface.SelectObject(hbAircraftSolidBg);
		const auto hpOld = Surface.SelectObject(hpAircraft);

		PolygonRotateShift(Aircraft, array_size(Aircraft), Orig.x, Orig.y,
				angle);

		Surface.Polygon(Aircraft, array_size(Aircraft));

		// draw it again so can get white border
		Surface.SelectObject(LKPen_White_N2);
		Surface.SelectObject(hbAircraftSolid);

		for (unsigned i = 0; i < array_size(Aircraft); i++) {
			Aircraft[i].x -= 1;
			Aircraft[i].y -= 1;
		}

		Surface.Polygon(Aircraft, array_size(Aircraft));

		Surface.SelectObject(hpOld);
		Surface.SelectObject(hbOld);

		return;

	} else {

		// GLIDER AICRAFT NORMAL ICON

		POINT AircraftInner[] = { { 1, -5 }, { 1, 0 }, { 14, 0 }, { 14, 1 }, {
				1, 1 }, { 1, 8 }, { 4, 8 }, { 4, 9 }, { -3, 9 }, { -3, 8 }, { 0,
				8 }, { 0, 1 }, { -13, 1 }, { -13, 0 }, { 0, 0 }, { 0, -5 }, { 1,
				-5 }, };
		POINT AircraftOuter[] = { { 2, -6 }, { 2, -1 }, { 15, -1 }, { 15, 2 }, {
				2, 2 }, { 2, 7 }, { 5, 7 }, { 5, 10 }, { -4, 10 }, { -4, 7 }, {
				-1, 7 }, { -1, 2 }, { -14, 2 }, { -14, -1 }, { -1, -1 }, { -1,
				-6 }, { 2, -6 }, };

		PolygonRotateShift(AircraftInner, array_size(AircraftInner), Orig.x,
				Orig.y, angle);
		PolygonRotateShift(AircraftOuter, array_size(AircraftOuter), Orig.x,
				Orig.y, angle);

		const auto oldPen = Surface.SelectObject(LK_NULL_PEN);

		const auto hbOld = Surface.SelectObject(LK_BLACK_BRUSH);
		Surface.Polygon(AircraftOuter, array_size(AircraftOuter));

		Surface.SelectObject(LK_WHITE_BRUSH);
		Surface.Polygon(AircraftInner, array_size(AircraftInner));

		Surface.SelectObject(oldPen);
		Surface.SelectObject(hbOld);

	}
}
