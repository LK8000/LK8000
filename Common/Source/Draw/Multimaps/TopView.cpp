/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"
#include "Multimap.h"
#include "Terrain.h"
#include "RasterTerrain.h"

extern LKColor  Sideview_TextColor;


int MapWindow::SharedTopView(LKSurface& Surface, DiagrammStruct* psDia , double fAS_Bearing, double fWP_Bearing)
{
  int iOldDisplayOrientation =  DisplayOrientation;
  DiagrammStruct m_Dia =	*psDia;
  RECT rct = m_Dia.rc;

  unsigned short getsideviewpage=GetSideviewPage();
  LKASSERT(getsideviewpage<NUMBER_OF_SHARED_MULTIMAPS);
  LKASSERT(Current_Multimap_SizeY!=SIZE0);

  DisplayOrientation = GetMMNorthUp(getsideviewpage);

  switch(GetMMNorthUp(getsideviewpage))
  {
   	case TRACKUP:
   		break;

   	case NORTHUP:
   	default:
		m_Dia.fXMin = -m_Dia.fXMax;
		break;
  }

  double fOldScale  =  zoom.Scale();
  const auto hfOld = Surface.SelectObject(LK8PanelUnitFont);

  if(zoom.AutoZoom())
	zoom.AutoZoom(false);
  double fFact = 1.25 ;
#ifdef INIT_CASE


  switch(ScreenSize) {

	case ss800x480:	fFact=0.750; break;
	case ss640x480:	fFact=0.938; break;
	case ss480x272:	fFact=0.708; break;
	case ss400x240:	fFact=0.750; break;
	case ss320x240:	fFact=0.938; break;
	case ss480x800:	fFact=1.250; break;
	case ss480x640:	fFact=1.250; break;
	case ss272x480:	fFact=1.250; break;
	case ss240x400:	fFact=1.250; break;
	case ss240x320:	fFact=1.250; break;
	default:	fFact=1.000; break;
  }
#endif


  if((ScreenSizeX > 0) && (ScreenSizeY > 0))
  {
    if(ScreenSizeX > ScreenSizeY)
      fFact = (double)ScreenSizeY/(double)ScreenSizeX * 1.250;

  }
  PanLatitude  = DrawInfo.Latitude;
  PanLongitude = DrawInfo.Longitude;

  switch(GetMMNorthUp(getsideviewpage))
  {
      case TRACKUP:
	    	DisplayAngle = AngleLimit360(fAS_Bearing  +270.0);
	    	DisplayAircraftAngle = AngleLimit360(fWP_Bearing);
		break;

      case NORTHUP:
      default:
		DisplayAngle = 0;
		if( getsideviewpage == IM_HEADING || getsideviewpage==IM_VISUALGLIDE)
			DisplayAircraftAngle = AngleLimit360(fAS_Bearing);
		else
			DisplayAircraftAngle = AngleLimit360(DrawInfo.TrackBearing);
		break;
  }

  int iOldLocator = EnableThermalLocator;
  EnableThermalLocator =0;

  MapWindow::ChangeDrawRect(rct);       // set new area for terrain and topology

  zoom.ModifyMapScale();
  zoom.RequestedScale((m_Dia.fXMax -m_Dia.fXMin)  * fFact *  (DISTANCEMODIFY)/10.0f);

  POINT Orig           =  { CalcDistanceCoordinat(0.0,  (DiagrammStruct*) &m_Dia),(rct.bottom-rct.top)/2};
  POINT Orig_Aircraft= {0,0};

  zoom.ModifyMapScale();
  zoom.UpdateMapScale();

  CalculateScreenPositions( Orig,  rct, &Orig_Aircraft);
  CalculateScreenPositionsAirspace(rct);

  // 
  // Expose variables in use for topview drawing
  // 
  Current_Multimap_TopOrig=Orig_Aircraft;
  Current_Multimap_TopZoom=GetInvDrawScale();
  Current_Multimap_TopAngle=DisplayAngle;

  bool terrainpainted=false;

  if (IsMultimapTerrain() &&  DerivedDrawInfo.TerrainValid && RasterTerrain::isTerrainLoaded() ) {
        LKTextBlack=false;
        BlackScreen=false;
	LockTerrainDataGraphics();
    /* workaround */
    RECT rcTerrain = rct;
    rcTerrain.right+=rcTerrain.left;
    rcTerrain.bottom+=rcTerrain.top;
    
	DrawTerrain(Surface, rcTerrain, GetAzimuth(), 40.0);
	UnlockTerrainDataGraphics();
	terrainpainted=true;
  } else {
	// We fill up the background wity chosen empty map color

	// display border and fill background..
        Surface.SelectObject(hInvBackgroundBrush[BgMapColor]);
        Surface.SelectObject(LK_WHITE_PEN);
        Surface.Rectangle(rct.left,rct.top,rct.right,rct.bottom);
        // We force LK painting black values on screen depending on the background color in use
        // blackscreen would force everything to be painted white, instead
        LKTextBlack=BgMapColorTextBlack[BgMapColor];
        if (BgMapColor>6 ) BlackScreen=true; else BlackScreen=false;
  } 

  ResetLabelDeclutter();

  // We reduce screen cluttering for some cases..
  short olddecluttermode=DeclutterMode;
  if (Current_Multimap_SizeY==SIZE4) goto _nomoredeclutter;
  if (Current_Multimap_SizeY<SIZE3) {
	DeclutterMode+=2;
  } else {
	if (Current_Multimap_SizeY==SIZE3)
		DeclutterMode++;
  }
  if (DeclutterMode>dmVeryHigh) DeclutterMode=dmVeryHigh;

_nomoredeclutter:

  if (IsMultimapTopology()) {
	// Do not print topology labels, to be used with another config later!
	// SaturateLabelDeclutter();
	RECT rc_red = rct;
	rc_red.bottom -= 3;
	DrawTopology(Surface, rc_red);
  } else {
	// No topology is desired, but terrain requires water areas nevertheless
	if (terrainpainted) {
		RECT rc_red = rct;
		rc_red.bottom -= 3;
		DrawTopology(Surface, rc_red,true); // water only!
	}
  }


  if (IsMultimapAirspace()) {
	if ( (GetAirSpaceFillType() == asp_fill_ablend_full) || (GetAirSpaceFillType() == asp_fill_ablend_borders) ) {
		DrawTptAirSpace(Surface, rct);
	} else {
		if ( GetAirSpaceFillType() == asp_fill_border_only)
			DrawAirSpaceBorders(Surface, rct); // full screen, to hide clipping effect on low border
		else
			DrawAirSpace(Surface, rct);   // full screen, to hide clipping effect on low border
	}
	// DrawAirspaceLabels( hdc,   rct, Orig_Aircraft);
  }

  if (Flags_DrawTask && MapSpaceMode!=MSM_MAPASP && ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) {
    DrawTaskAAT(Surface, DrawRect);
    DrawTask(Surface, DrawRect, Current_Multimap_TopOrig);
  }

  if (IsMultimapWaypoints()) {
	DrawWaypointsNew(Surface,DrawRect);
  }
  if (Flags_DrawFAI)
	DrawFAIOptimizer(Surface, DrawRect, Current_Multimap_TopOrig);

  DeclutterMode=olddecluttermode; // set it back correctly

 /* THIS STUFF DOES NOT WORK IN SHARED MAPS, YET
    NEED FIXING LatLon2Screen for shared maps using Sideview
    #ifdef GTL2
    if (((FinalGlideTerrain == 2) || (FinalGlideTerrain == 4)) &&
	DerivedDrawInfo.TerrainValid)
	DrawTerrainAbove(hdc, DrawRect);
    #endif
  */


  // 
  // Stuff for MAPTRK only (M1)
  if (MapSpaceMode==MSM_MAPTRK) {
	if(IsMultimapTerrain() || IsMultimapTopology() ) {
		if (FinalGlideTerrain && DerivedDrawInfo.TerrainValid)
			DrawGlideThroughTerrain(Surface, DrawRect);
	}
	if (extGPSCONNECT)
		DrawBearing(Surface, DrawRect);
	// Wind arrow
	if (IsMultimapOverlaysGauges())
		DrawWindAtAircraft2(Surface, Current_Multimap_TopOrig, DrawRect);
  }

  if (MapSpaceMode==MSM_MAPWPT) {
	if (extGPSCONNECT)
		DrawBearing(Surface, DrawRect);
  }

  switch(GetMMNorthUp(getsideviewpage)) {
	case NORTHUP:
	default:
		DrawCompass( Surface,  rct, 0);
	break;
	case TRACKUP:
		if(getsideviewpage ==  IM_HEADING || getsideviewpage == IM_VISUALGLIDE)
		  DrawCompass( Surface,  rct, DrawInfo.TrackBearing-90.0);
		else
		  DrawCompass( Surface,  rct, DisplayAngle);
	break;
  }


  /****************************************************************************************************
   * draw vertical line
   ****************************************************************************************************/
  POINT line[2];
  line[0].x = rct.left;
  line[0].y = Orig_Aircraft.y-1;
  line[1].x = rct.right;
  line[1].y = line[0].y;

  switch(GetMMNorthUp(getsideviewpage))
  {
     case TRACKUP:
	// Are we are not topview fullscreen?
	if (Current_Multimap_SizeY<SIZE4 && !MapSpaceMode==MSM_VISUALGLIDE) {
		Surface.DrawDashLine(NIBLSCALE(1), line[0], line[1],  Sideview_TextColor, rct);
	} else {
	    if (TrackBar) {
    	 	    DrawHeadUpLine(Surface, Orig, rct, psDia->fXMin ,psDia->fXMax);
    	 	    if (ISGAAIRCRAFT) DrawFuturePos(Surface, Orig, DrawRect, true);
    	 	}
	}
     break;

     case NORTHUP:
     default:
	if (TrackBar) {
		DrawHeadUpLine(Surface, Orig, rct, psDia->fXMin ,psDia->fXMax);
		if (ISGAAIRCRAFT) DrawFuturePos(Surface, Orig, DrawRect, true);
	}
	break;
  }
  DrawAircraft(Surface, Orig_Aircraft);

  // M3 has sideview always on, so wont apply here, and no need to check
  if (Current_Multimap_SizeY==SIZE4) {
	DrawMapScale(Surface,rct,0);
  }

  MapWindow::zoom.RequestedScale(fOldScale);
  EnableThermalLocator = iOldLocator;
  DisplayOrientation = iOldDisplayOrientation;
  Surface.SelectObject(hfOld);
  return 0;

}




void MapWindow::DrawHeadUpLine(LKSurface& Surface, const POINT& Orig, const RECT& rc, double fMin, double fMax  ) {

  const double tmp = fMax*zoom.ResScaleOverDistanceModify();
  const double trackbearing =  DisplayAircraftAngle+  (DerivedDrawInfo.Heading-DrawInfo.TrackBearing);
  
  const POINT p2 = { Orig.x + (int)(tmp*fastsine(trackbearing)), Orig.y - (int)(tmp*fastcosine(trackbearing)) };

  const LKColor rgbCol = BlackScreen?RGB_INVDRAW:RGB_BLACK;

  // Reduce the rectangle for a better effect
  const RECT ClipRect = {rc.left+NIBLSCALE(5), rc.top+NIBLSCALE(5), rc.right-NIBLSCALE(5), rc.bottom-NIBLSCALE(5) };
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), Orig, p2, rgbCol, ClipRect);
}




