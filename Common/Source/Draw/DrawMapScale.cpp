/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "RGB.h"
#include "RasterTerrain.h"
#include "LKObjects.h"
#include "utils/stl_utils.h"
#include "ScreenGeometry.h"
#include "ScreenProjection.h"


void MapWindow::DrawMapScale(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj)
{
    static short terrainwarning=0;
    bool inpanmode= (!mode.Is(Mode::MODE_TARGET_PAN) && mode.Is(Mode::MODE_PAN));

    const GeoPoint geo_1(PanLatitude, PanLongitude);
    const GeoPoint geo_2 = geo_1.Direct(DisplayAngle, zoom.Scale() * 1000);
    const GeoToScreen<RasterPoint> ToScreen(_Proj);
    PixelScalar mapscale_vsize = std::abs(Distance(ToScreen(geo_1), ToScreen(geo_2)));
    PixelScalar mapscale_hsize = NIBLSCALE(5);
    PixelScalar mapscale_right_margin = (rc.right - NIBLSCALE(3));
    PixelScalar mapscale_bottom_margin = (rc.bottom - NIBLSCALE(4));

    if (inpanmode) {
        if(ScreenLandscape) {
            /* in Landscape mode, we need to draw Scale upper for avoid menu overlap
             * TODO : find right way for have menu size and visibility.
             */
            mapscale_bottom_margin -= std::min<PixelScalar>(((rc.bottom-rc.top)-4)/5, NIBLSCALE(40));
        }
    } else {
        mapscale_bottom_margin -= BottomSize;
    }

    const RasterPoint ScaleLine[] = {
            {
                    mapscale_right_margin - mapscale_hsize,
                    mapscale_bottom_margin - mapscale_vsize
            },
            {
                    mapscale_right_margin,
                    mapscale_bottom_margin - mapscale_vsize
            },
            {
                    mapscale_right_margin,
                    mapscale_bottom_margin
            },
            {
                    mapscale_right_margin - mapscale_hsize,
                    mapscale_bottom_margin
            }
    };

    const PixelRect ScaleLineBck[] = {
            {
                    ScaleLine[0].x - IBLSCALE(1),
                    ScaleLine[0].y - IBLSCALE(1),
                    ScaleLine[1].x + IBLSCALE(2),
                    ScaleLine[1].y + IBLSCALE(2)
            },
            {
                    ScaleLine[1].x - IBLSCALE(1),
                    ScaleLine[1].y - IBLSCALE(1),
                    ScaleLine[2].x + IBLSCALE(2),
                    ScaleLine[2].y + IBLSCALE(2)
            },
            {
                    ScaleLine[3].x - IBLSCALE(1),
                    ScaleLine[3].y - IBLSCALE(1),
                    ScaleLine[2].x + IBLSCALE(2),
                    ScaleLine[2].y + IBLSCALE(2)
            }
    };

    Surface.FillRect(&ScaleLineBck[0], LKBrush_Black);
    Surface.FillRect(&ScaleLineBck[1], LKBrush_Black);
    Surface.FillRect(&ScaleLineBck[2], LKBrush_Black);

    const auto hpOld = Surface.SelectObject(LKPen_White_N0);
    Surface.Polyline(ScaleLine, std::size(ScaleLine));
    Surface.SelectObject(hpOld);

    TCHAR Scale[200] = {};
    TCHAR Scale1[200] = {};
    TCHAR Scale2[200] = {};
    TCHAR TEMP[40] = {};


    if (inpanmode) {
	if (DerivedDrawInfo.TerrainValid) {
        RasterTerrain::Lock();
		double alt= ALTITUDEMODIFY*RasterTerrain::GetTerrainHeight(GetPanLatitude(), GetPanLongitude());
        RasterTerrain::Unlock();
		if (alt==TERRAIN_INVALID) alt=0.0;
		_stprintf(Scale1, _T(" %.0f%s "),alt,
		Units::GetUnitName(Units::GetUserAltitudeUnit()));
	}
	double pandistance, panbearing;


    DistanceBearing(DrawInfo.Latitude,DrawInfo.Longitude,GetPanLatitude(),GetPanLongitude(),&pandistance,&panbearing);
    if(ValidTaskPoint(PanTaskEdit))
    {  RefreshTask();
        double Dist = DerivedDrawInfo.TaskTotalDistance;
    	if( DerivedDrawInfo.TaskFAI)
    	{
    	  Dist = DerivedDrawInfo.TaskFAIDistance;
          _stprintf(Scale2, _T("FAI Task %.1f%s %s %.0f%s"),  Dist*DISTANCEMODIFY, Units::GetDistanceName(), Scale1 ,panbearing,MsgToken(2179) );
    	}
        else
    	  _stprintf(Scale2, _T("     Task %.1f%s %s %.0f%s"),  Dist*DISTANCEMODIFY, Units::GetDistanceName(), Scale1 ,panbearing,MsgToken(2179) );
    }
    else
    {

	  _stprintf(Scale2, _T(" %.1f%s %s %.0f%s "), pandistance*DISTANCEMODIFY, Units::GetDistanceName(),Scale1, panbearing, MsgToken(2179) );
    }

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
	Units::CoordinateToString(GetPanLongitude(), GetPanLatitude(), sCoordinate, std::size(sCoordinate)-1);
	_tcscat(Scale, sCoordinate);
	_tcscat(Scale, _T(" "));
    }
    double mapScale=Units::ToSysDistance(zoom.Scale());
    // zoom.Scale() gives user units, but FormatUserMapScale() needs system distance units
    Units::FormatUserMapScale(NULL, mapScale, Scale1, sizeof(Scale1)/sizeof(Scale1[0]));
    _tcscat(Scale,Scale1);

    SIZE tsize;

    Surface.SetBackgroundTransparent();
    const auto oldFont = Surface.SelectObject(MapScaleFont);
    const auto oldPen = Surface.SelectObject(LK_BLACK_PEN);
    const auto oldBrush = Surface.SelectObject(LKBrush_Black);

    LKColor mapscalecolor = ((OverColorRef==RGB_SBLACK) ? OverColorRef : RGB_WHITE);

    PixelScalar text_y = mapscale_bottom_margin + NIBLSCALE(4);
    PixelScalar text_x = ScaleLineBck[0].left - NIBLSCALE(1);

    Surface.GetTextSize(Scale, &tsize);
    LKWriteText(Surface, Scale, text_x - tsize.cx, text_y - (tsize.cy * 2), WTMODE_OUTLINED, WTALIGN_LEFT, mapscalecolor, true);

    if (!DerivedDrawInfo.TerrainValid) {
    	if (terrainwarning>0 && terrainwarning<120) {
            mapscalecolor = RGB_RED;
        }
    }

    Surface.GetTextSize(Scale2, &tsize);
    LKWriteText(Surface, Scale2, text_x - tsize.cx, text_y - tsize.cy, WTMODE_OUTLINED, WTALIGN_LEFT, mapscalecolor, true);

    Surface.SelectObject(oldPen);
    Surface.SelectObject(oldBrush);
    Surface.SelectObject(oldFont);
}
