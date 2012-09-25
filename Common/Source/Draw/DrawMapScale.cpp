/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "RGB.h"
#include "DoInits.h"
#include "RasterTerrain.h"


void MapWindow::DrawMapScale(HDC hDC, const RECT rc /* the Map Rect*/, 
                             const bool ScaleChangeFeedback)
{
    static short terrainwarning=0;
    static POINT lineOneStart, lineOneEnd,lineTwoStart,lineTwoEnd,lineThreeStart,lineThreeEnd;

    if (DoInit[MDI_DRAWMAPSCALE]) {
	lineOneStart.x = rc.right-NIBLSCALE(6); 
	lineOneEnd.x   = rc.right-NIBLSCALE(6);
	lineOneStart.y = rc.bottom-BottomSize-NIBLSCALE(4);
	lineOneEnd.y = lineOneStart.y - NIBLSCALE(42);

	lineTwoStart.x = rc.right-NIBLSCALE(11); 
	lineTwoEnd.x = rc.right-NIBLSCALE(6);
	lineTwoEnd.y = lineOneStart.y;
	lineTwoStart.y = lineOneStart.y;

	lineThreeStart.y = lineTwoStart.y - NIBLSCALE(42);
	lineThreeEnd.y = lineThreeStart.y;
	lineThreeStart.x = lineTwoStart.x;
	lineThreeEnd.x = lineTwoEnd.x;

	DoInit[MDI_DRAWMAPSCALE]=false;
    }

    TCHAR Scale[200];
    TCHAR Scale1[200];
    TCHAR Scale2[200];
    TCHAR TEMP[20];
    COLORREF origcolor = SetTextColor(hDC, OverColorRef);

    HPEN hpOld;
    hpOld = (HPEN)SelectObject(hDC, hpMapScale2);

    DrawSolidLine(hDC,lineOneStart,lineOneEnd, rc);
    DrawSolidLine(hDC,lineTwoStart,lineTwoEnd, rc);
    DrawSolidLine(hDC,lineThreeStart,lineThreeEnd, rc);

    SelectObject(hDC, hpOld);

    _tcscpy(Scale2,TEXT(""));

    bool inpanmode= (!mode.Is(Mode::MODE_TARGET_PAN) && mode.Is(Mode::MODE_PAN));

    if (inpanmode) {
	double pandistance, panbearing;
	DistanceBearing(GPS_INFO.Latitude,GPS_INFO.Longitude,GetPanLatitude(),GetPanLongitude(),&pandistance,&panbearing);
	_stprintf(Scale2, _T(" %.1f%s %.0f%s "), pandistance*DISTANCEMODIFY, Units::GetDistanceName(), panbearing,_T(DEG) );
	goto _skip1;
    }

    //
    // This stuff is not painted while panning, to save space on screen
    //

    // warn about missing terrain
    if (!DerivedDrawInfo.TerrainValid) {
	if (terrainwarning < 120) {
		// LKTOKEN _@M1335_ " TERRAIN?"
		_tcscat(Scale2, MsgToken(1335));
		terrainwarning++;
	} else  {
		// LKTOKEN _@M1336_ " T?"
		_tcscat(Scale2, MsgToken(1336));
		terrainwarning=120;
	}
    } else terrainwarning=0;

    if (ActiveMap) {
      _tcscat(Scale2, MsgToken(1661)); // ACT
    }
    if (UseTotalEnergy) {
      _tcscat(Scale2, TEXT("[TE]")); // Total Energy indicator
    }
    if (zoom.AutoZoom()) {
		// LKTOKEN _@M1337_ " AZM"
      _tcscat(Scale2, MsgToken(1337));
    }

_skip1:

    //
    // Back painting stuff even in PAN mode
    //

    if (mode.AnyPan()) {
		// LKTOKEN _@M1338_ " PAN"
      _tcscat(Scale2, MsgToken(1338));
    }

    if (DrawBottom) {
	switch(BottomMode) {
		case BM_TRM:
				// LKTOKEN _@M1340_ " TRM0"
      			_tcscat(Scale2, MsgToken(1340));
			break;
		case BM_CRU:
				// LKTOKEN _@M1341_ " NAV1"
      			_tcscat(Scale2, MsgToken(1341));
			break;
		case BM_HGH:
				// LKTOKEN _@M1342_ " ALT2"
      			_tcscat(Scale2, MsgToken(1342));
			break;
		case BM_AUX:
				// LKTOKEN _@M1343_ " STA3"
      			_tcscat(Scale2, MsgToken(1343));
			break;
		case BM_TSK:
				// LKTOKEN _@M1344_ " TSK4"
      			_tcscat(Scale2, MsgToken(1344));
			break;
		case BM_ALT:
				// LKTOKEN _@M1345_ " ATN5"
      			_tcscat(Scale2, MsgToken(1345));
			break;
		case BM_SYS:
				// LKTOKEN _@M1346_ " SYS6"
      			_tcscat(Scale2, MsgToken(1346));
			break;
		case BM_CUS2:
				// LKTOKEN _@M1347_ " CRU7"
      			_tcscat(Scale2, MsgToken(1347));
			break;
		case BM_CUS3:
				// LKTOKEN _@M1348_ " FIN8"
      			_tcscat(Scale2, MsgToken(1348));
			break;
		case BM_CUS:
				// LKTOKEN _@M1349_ " AUX9"
      			_tcscat(Scale2, MsgToken(1349));
			break;
		default:
			break;
	}
    }

    if (inpanmode) goto _skip2;

    if (ReplayLogger::IsEnabled()) {
		// LKTOKEN _@M1350_ " REPLAY"
      _tcscat(Scale2, MsgToken(1350));
    }
    if (BallastTimerActive) {
		// LKTOKEN _@M1351_ " BALLAST"
      _stprintf(TEMP,TEXT("%s %3.0fL"), MsgToken(1351), WEIGHTS[2]*BALLAST);
      _tcscat(Scale2, TEMP);
    }

_skip2:

    _tcscpy(Scale,TEXT(""));
    _tcscpy(Scale1,TEXT(""));

    //if (SIMMODE && (!mode.Is(Mode::MODE_TARGET_PAN) && mode.Is(Mode::MODE_PAN)) ) {
    if (inpanmode) {

	TCHAR sCoordinate[32]={0};
	Units::CoordinateToString(GetPanLongitude(), GetPanLatitude(), sCoordinate, sizeof(sCoordinate)-1);
	_tcscat(Scale, sCoordinate);
	// Paint terrain altitude only if valid terrain!
	if (DerivedDrawInfo.TerrainValid) {
	   _stprintf(Scale1, _T(" %.0f%s "),ALTITUDEMODIFY*RasterTerrain::GetTerrainHeight(GetPanLatitude(), GetPanLongitude()),
		Units::GetUnitName(Units::GetUserAltitudeUnit()));
	   _tcscat(Scale, Scale1);
	}
    }
    double mapScale=Units::ToSysDistance(zoom.Scale()*1.4);	// 1.4 for mapscale symbol size on map screen
    // zoom.Scale() gives user units, but FormatUserMapScale() needs system distance units
    Units::FormatUserMapScale(NULL, mapScale, Scale1, sizeof(Scale1)/sizeof(Scale1[0]));
    _tcscat(Scale,Scale1);

    SIZE tsize;

    GetTextExtentPoint(hDC, Scale, _tcslen(Scale), &tsize);
    LKWriteText(hDC, Scale, rc.right-NIBLSCALE(11)-tsize.cx, lineThreeEnd.y+NIBLSCALE(3), 0, WTMODE_OUTLINED, WTALIGN_LEFT, OverColorRef, true); 

    GetTextExtentPoint(hDC, Scale2, _tcslen(Scale2), &tsize);
    COLORREF mapscalecolor=OverColorRef;

    if (!DerivedDrawInfo.TerrainValid) {
	if (terrainwarning>0 && terrainwarning<120) mapscalecolor=RGB_RED;
    } else {
	if (mapscalecolor==RGB_SBLACK) mapscalecolor=RGB_WHITE;
    }

		
    LKWriteText(hDC, Scale2, rc.right-NIBLSCALE(11)-tsize.cx, lineThreeEnd.y+NIBLSCALE(3)+tsize.cy, 
	0, WTMODE_OUTLINED, WTALIGN_LEFT, mapscalecolor, true); 


    // restore original color
    SetTextColor(hDC, origcolor);

    SelectObject(hDC, hpOld);


}

