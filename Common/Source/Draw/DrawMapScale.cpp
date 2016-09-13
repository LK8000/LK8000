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
#include "utils/stl_utils.h"
#include "NavFunctions.h"
//
// SCALE SIZE: NIBLSCALE(42+4)
//
#define MAPSCALE_VSIZE  NIBLSCALE(42)
#define MAPSCALE_HSIZE  NIBLSCALE(5)
#define MAPSCALE_RIGHTMARGIN   (rc.right-NIBLSCALE(3))

/* in Landscape mode, we need to draw Scale upper for avoid menu overlap
 * this define are crap, but i don't have better way for do that now.
 * TODO : find right way for have menu size and visibility.
 * Bruno.
 */
#define MAPSCALE_BOTTOMMARGIN  (rc.bottom-NIBLSCALE(4) - (inpanmode? \
                                                                ( ScreenLandscape? \
                                                                    std::min<PixelScalar>(((rc.bottom-rc.top)-4)/5, NIBLSCALE(40)) \
                                                                    : 0) \
                                                                : BottomSize))

void MapWindow::DrawMapScale(LKSurface& Surface, const RECT& rc /* the Map Rect*/, 
                             const bool ScaleChangeFeedback)
{
    static short terrainwarning=0;
    
    static RasterPoint ScaleLine[4];
    static PixelRect   ScaleLineBck[3];
    static int   ytext;
    static bool flipflop=true;

    bool inpanmode= (!mode.Is(Mode::MODE_TARGET_PAN) && mode.Is(Mode::MODE_PAN));
    static bool prevmode = inpanmode;

    if (DoInit[MDI_DRAWMAPSCALE] || prevmode != inpanmode) {
        ScaleLine[0] = { 
            MAPSCALE_RIGHTMARGIN - MAPSCALE_HSIZE, 
            MAPSCALE_BOTTOMMARGIN - MAPSCALE_VSIZE 
        };
        ScaleLine[1] = { 
            MAPSCALE_RIGHTMARGIN, 
            MAPSCALE_BOTTOMMARGIN - MAPSCALE_VSIZE
        };
        ScaleLine[2] = { 
            MAPSCALE_RIGHTMARGIN, 
            MAPSCALE_BOTTOMMARGIN 
        };
        ScaleLine[3] = { 
            MAPSCALE_RIGHTMARGIN - MAPSCALE_HSIZE, 
            MAPSCALE_BOTTOMMARGIN 
        };

        ScaleLineBck[0] = {
            ScaleLine[0].x - 1, 
            ScaleLine[0].y - 1, 
            ScaleLine[1].x + 2, 
            ScaleLine[1].y + 2
        };
        ScaleLineBck[1] = {
            ScaleLine[1].x - 1, 
            ScaleLine[1].y - 1, 
            ScaleLine[2].x + 2, 
            ScaleLine[2].y + 2
        };
        ScaleLineBck[2] = {
            ScaleLine[3].x - 1, 
            ScaleLine[3].y - 1, 
            ScaleLine[2].x + 2, 
            ScaleLine[2].y + 2
        }; 
        
        SIZE tsize;
        const auto oldFont = Surface.SelectObject(MapScaleFont);
        Surface.GetTextSize(_T("M"),&tsize);
        int ofs=(MAPSCALE_VSIZE - (tsize.cy + tsize.cy))/2;
        ytext=ScaleLine[0].y+ofs;
        Surface.SelectObject(oldFont);


        prevmode = inpanmode;
    	DoInit[MDI_DRAWMAPSCALE]=false;
    }

    TCHAR Scale[200] = {};
    TCHAR Scale1[200] = {};
    TCHAR Scale2[200] = {};
    TCHAR TEMP[20] = {};

    Surface.FillRect(&ScaleLineBck[0], LKBrush_Black);
    Surface.FillRect(&ScaleLineBck[1], LKBrush_Black);
    Surface.FillRect(&ScaleLineBck[2], LKBrush_Black);
   
    const auto hpOld = Surface.SelectObject(LKPen_White_N0);
    Surface.Polyline(ScaleLine, array_size(ScaleLine));
    Surface.SelectObject(hpOld);

    flipflop=!flipflop;

    if (inpanmode) {
	if (DerivedDrawInfo.TerrainValid) {
		double alt= ALTITUDEMODIFY*RasterTerrain::GetTerrainHeight(GetPanLatitude(), GetPanLongitude());
		if (alt==TERRAIN_INVALID) alt=0.0;
		_stprintf(Scale1, _T(" %.0f%s "),alt,
		Units::GetUnitName(Units::GetUserAltitudeUnit()));
	}
	double pandistance, panbearing;


    DistanceBearing(DrawInfo.Latitude,DrawInfo.Longitude,GetPanLatitude(),GetPanLongitude(),&pandistance,&panbearing);
    if(ValidTaskPoint(PanTaskEdit))
    {
        double Dist = DerivedDrawInfo.TaskTotalDistance;
    	if( DerivedDrawInfo.TaskFAI)
    	{
    	  Dist = DerivedDrawInfo.TaskFAIDistance;
          _stprintf(Scale2, _T("FAI Task %.1f%s %.1f%s %s %.0f%s"),  Dist*DISTANCEMODIFY, Units::GetDistanceName(),pandistance*DISTANCEMODIFY,Units::GetDistanceName(), Scale1 ,panbearing,MsgToken(2179) );
    	}
        else
    	  _stprintf(Scale2, _T("     Task %.1f%s %.1f%s %s %.0f%s"),  Dist*DISTANCEMODIFY, Units::GetDistanceName(),pandistance*DISTANCEMODIFY, Units::GetDistanceName(), Scale1 ,panbearing,MsgToken(2179) );
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
	Units::CoordinateToString(GetPanLongitude(), GetPanLatitude(), sCoordinate, array_size(sCoordinate)-1);
	_tcscat(Scale, sCoordinate);
	_tcscat(Scale, _T(" "));
    }
    double mapScale=Units::ToSysDistance(zoom.Scale()*1.4);	// 1.4 for mapscale symbol size on map screen
    // zoom.Scale() gives user units, but FormatUserMapScale() needs system distance units
    Units::FormatUserMapScale(NULL, mapScale, Scale1, sizeof(Scale1)/sizeof(Scale1[0]));
    _tcscat(Scale,Scale1);

    SIZE tsize;

    Surface.SetBackgroundTransparent();
    const auto oldFont = Surface.SelectObject(MapScaleFont);
    const auto oldPen = Surface.SelectObject(LK_BLACK_PEN);
    const auto oldBrush = Surface.SelectObject(LKBrush_Black);

    Surface.GetTextSize(Scale, &tsize);

    LKColor mapscalecolor = OverColorRef;
    if (OverColorRef==RGB_SBLACK) mapscalecolor=RGB_WHITE;

    LKWriteText(Surface, Scale, rc.right-NIBLSCALE(7)-tsize.cx, ytext, WTMODE_OUTLINED, WTALIGN_LEFT, mapscalecolor, true);

    Surface.GetTextSize(Scale2, &tsize);

    if (!DerivedDrawInfo.TerrainValid) {
	if (terrainwarning>0 && terrainwarning<120) mapscalecolor=RGB_RED;
    }

    LKWriteText(Surface, Scale2, rc.right-NIBLSCALE(7)-tsize.cx, ytext+tsize.cy, WTMODE_OUTLINED, WTALIGN_LEFT, mapscalecolor, true);

    Surface.SelectObject(oldPen);
    Surface.SelectObject(oldBrush);
    Surface.SelectObject(oldFont);

}

