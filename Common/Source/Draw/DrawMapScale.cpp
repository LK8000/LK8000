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
#include "LKObjects.h"




BOOL IsFAI_Task(void)
{
BOOL fai = true;
int i;

if (CALCULATED_INFO.TaskDistanceToGo>0) {
  for (i=1; i<MAXTASKPOINTS; i++) {
    if (Task[i].Index != -1) {
	double lrat = Task[i].Leg/CALCULATED_INFO.TaskDistanceToGo;
	if(ValidWayPoint(i))
	{
	  if(CALCULATED_INFO.TaskDistanceToGo < FAI28_45Threshold)
	  {
	    if (lrat<FAI_NORMAL_PERCENTAGE)  fai = false;
	  }
	  else
	  {
	    if (lrat<FAI_BIG_PERCENTAGE)    fai = false;
	  }
	  if(lrat>FAI_BIG_MAX_PERCENTAGE)   fai = false;
	}
    }
  }
} else {
  fai = false;
}
return fai;
}

void MapWindow::DrawMapScale(HDC hDC, const RECT rc /* the Map Rect*/, 
                             const bool ScaleChangeFeedback)
{
    static short terrainwarning=0;
    static POINT lineOneStart, lineOneEnd,lineTwoStart,lineTwoEnd,lineThreeStart,lineThreeEnd;
    static POINT lineTwoStartB,lineThreeStartB;
    static bool flipflop=true;

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

	lineTwoStartB=lineTwoStart;
	lineTwoStartB.x++;
	lineThreeStartB=lineThreeStart;
	lineThreeStartB.x++;

	DoInit[MDI_DRAWMAPSCALE]=false;
    }

    TCHAR Scale[200];
    TCHAR Scale1[200];
    TCHAR Scale2[200];
    TCHAR TEMP[20];

    HPEN hpOld = (HPEN)SelectObject(hDC, hpMapScale2);

    DrawSolidLine(hDC,lineOneStart,lineOneEnd, rc);
    DrawSolidLine(hDC,lineTwoStart,lineTwoEnd, rc);
    DrawSolidLine(hDC,lineThreeStart,lineThreeEnd, rc);

    SelectObject(hDC, LKPen_White_N0);
    DrawSolidLine(hDC,lineOneStart,lineOneEnd, rc);
    DrawSolidLine(hDC,lineTwoStartB,lineTwoEnd, rc);
    DrawSolidLine(hDC,lineThreeStartB,lineThreeEnd, rc);

    SelectObject(hDC, hpOld);

    flipflop=!flipflop;

    _tcscpy(Scale2,TEXT(""));

    bool inpanmode= (!mode.Is(Mode::MODE_TARGET_PAN) && mode.Is(Mode::MODE_PAN));

    if (inpanmode) {
	if (DerivedDrawInfo.TerrainValid) {
		double alt= ALTITUDEMODIFY*RasterTerrain::GetTerrainHeight(GetPanLatitude(), GetPanLongitude());
		if (alt==TERRAIN_INVALID) alt=0.0;
		_stprintf(Scale2, _T(" %.0f%s "),alt,
		Units::GetUnitName(Units::GetUserAltitudeUnit()));
	}
	double pandistance, panbearing;

    if(ValidTaskPoint(PanTaskEdit))
    {
    	if( IsFAI_Task())
    	  _tcscpy(Scale1,TEXT("FAI"));
    	else
    	   _tcscpy(Scale1,TEXT(""));

    	_stprintf(Scale, _T("%s Task %.1f%s"), Scale1, CALCULATED_INFO.TaskDistanceToGo*DISTANCEMODIFY, Units::GetDistanceName()/*, panbearing,_T(DEG)*/ );

    }
    else
    {
	  DistanceBearing(DrawInfo.Latitude,DrawInfo.Longitude,GetPanLatitude(),GetPanLongitude(),&pandistance,&panbearing);
	  _stprintf(Scale, _T(" %.1f%s %.0f%s "), pandistance*DISTANCEMODIFY, Units::GetDistanceName(), panbearing,_T(DEG) );
    }


	_tcscat(Scale2,Scale);
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
	_stprintf(Scale,_T("%s %.0fX"),
		MsgToken(1350), // " REPLAY"
		ReplayLogger::TimeScale);
      _tcscat(Scale2, Scale);
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
	_tcscat(Scale, _T(" "));
    }
    double mapScale=Units::ToSysDistance(zoom.Scale()*1.4);	// 1.4 for mapscale symbol size on map screen
    // zoom.Scale() gives user units, but FormatUserMapScale() needs system distance units
    Units::FormatUserMapScale(NULL, mapScale, Scale1, sizeof(Scale1)/sizeof(Scale1[0]));
    _tcscat(Scale,Scale1);

    SIZE tsize;

    SetBkMode(hDC,TRANSPARENT);
    HFONT oldFont = (HFONT)SelectObject(hDC, MapWindowFont);
    HPEN  oldPen=(HPEN)SelectObject(hDC, GetStockObject(BLACK_PEN));
    HBRUSH oldBrush=(HBRUSH)SelectObject(hDC, GetStockObject(BLACK_BRUSH));

    GetTextExtentPoint(hDC, Scale, _tcslen(Scale), &tsize);
    COLORREF mapscalecolor=OverColorRef;
    if (mapscalecolor==RGB_SBLACK) mapscalecolor=RGB_WHITE;

    LKWriteText(hDC, Scale, rc.right-NIBLSCALE(11)-tsize.cx, lineThreeEnd.y+NIBLSCALE(3), 0, WTMODE_OUTLINED, WTALIGN_LEFT, mapscalecolor, true); 

    GetTextExtentPoint(hDC, Scale2, _tcslen(Scale2), &tsize);

    if (!DerivedDrawInfo.TerrainValid) {
	if (terrainwarning>0 && terrainwarning<120) mapscalecolor=RGB_RED;
    }

    LKWriteText(hDC, Scale2, rc.right-NIBLSCALE(11)-tsize.cx, lineThreeEnd.y+NIBLSCALE(3)+tsize.cy, 
	0, WTMODE_OUTLINED, WTALIGN_LEFT, mapscalecolor, true); 

    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);
    SelectObject(hDC,oldFont);

}

