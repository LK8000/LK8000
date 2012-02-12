/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RasterTerrain.h"
#include "LKAirspace.h"
#include "RGB.h"
#include "Sideview.h"

using std::min;
using std::max;

#define RGB_ROYAL_BLUE  RGB(18,32,139)
#define RGB_STEEL_BLUE  RGB(70,130,180)

extern AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS];
extern COLORREF Sideview_TextColor;
extern int Sideview_iNoHandeldSpaces;



void RenderAirspaceTerrain(HDC hdc, const RECT rc,double PosLat, double PosLon,  double brg,  DiagrammStruct* psDiag )
{

  double range =psDiag->fXMax - psDiag->fXMin; // km

  double hmin = psDiag->fYMin;
  double hmax = psDiag->fYMax;
  double lat, lon;

  int i,j;

#if (WINDOWSPC>0)
  if(INVERTCOLORS)
    RenderSky( hdc, rc, SKY_HORIZON_COL , SKY_SPACE_COL , GC_NO_COLOR_STEPS);
#endif

  FindLatitudeLongitude(PosLat, PosLon, brg  , psDiag->fXMin , &lat, &lon);

  double d_lat[AIRSPACE_SCANSIZE_X];
  double d_lon[AIRSPACE_SCANSIZE_X];
  double d_h[AIRSPACE_SCANSIZE_X];
//  double dfi = 1.0/(AIRSPACE_SCANSIZE_H-1);
  double dfj = 1.0/(AIRSPACE_SCANSIZE_X-1);



#define FRACT 0.75
  RasterTerrain::Lock(); // want most accurate rounding here
  RasterTerrain::SetTerrainRounding(0,0);
  double fj;
  for (j=0; j< AIRSPACE_SCANSIZE_X; j++)
  { // scan range
    fj = j*1.0/(AIRSPACE_SCANSIZE_X-1);
    FindLatitudeLongitude(lat, lon, brg, range*fj, &d_lat[j], &d_lon[j]);
    d_h[j] = RasterTerrain::GetTerrainHeight(d_lat[j], d_lon[j]);
    if (d_h[j] == TERRAIN_INVALID) d_h[j]=0; //@ 101027 BUGFIX
    hmax = max(hmax, d_h[j]);
  }

  RasterTerrain::Unlock();


  /********************************************************************************
   * scan line
   ********************************************************************************/
  Sideview_iNoHandeldSpaces =  CAirspaceManager::Instance().ScanAirspaceLineList(d_lat, d_lon, d_h, Sideview_pHandeled,MAX_NO_SIDE_AS); //  Sideview_pHandeled[GC_MAX_NO];
  /********************************************************************************
   * bubble sort to start with biggest airspaces
   ********************************************************************************/
  int iSizeLookupTable[MAX_NO_SIDE_AS];
  for( i = 0 ; i < Sideview_iNoHandeldSpaces ;i++)
	iSizeLookupTable[i] = i;

  for( i = 0 ; i < Sideview_iNoHandeldSpaces ;i++)
	for( j = i ; j < Sideview_iNoHandeldSpaces ;j++)
      if(Sideview_pHandeled[iSizeLookupTable[i]].iAreaSize < Sideview_pHandeled[iSizeLookupTable[j]].iAreaSize )
      {
    	int iTmp = iSizeLookupTable[i];
    	iSizeLookupTable[i] = iSizeLookupTable[j];
    	iSizeLookupTable[j] = iTmp;
      }

  /**********************************************************************************
   * transform into diagram coordinates
   **********************************************************************************/
  double dx = dfj*(rc.right-rc.left-BORDER_X);
  int x0 = rc.left+BORDER_X;
  int y0 = rc.bottom-BORDER_Y;

  for( i = 0 ; i < Sideview_iNoHandeldSpaces ;i++)
  {
	Sideview_pHandeled[i].rc.left   = iround((Sideview_pHandeled[i].rc.left    -FRACT)*dx)+x0;
	Sideview_pHandeled[i].rc.right  = iround((Sideview_pHandeled[i].rc.right   +FRACT)*dx)+x0;
    if( Sideview_pHandeled[i].rc.bottom  > 0)
	  Sideview_pHandeled[i].rc.bottom  = CalcHeightCoordinat((double)  Sideview_pHandeled[i].rc.bottom,  rc, psDiag);
    else
      Sideview_pHandeled[i].rc.bottom  =  rc.bottom;

	Sideview_pHandeled[i].rc.top     = CalcHeightCoordinat((double)  Sideview_pHandeled[i].rc.top,     rc, psDiag);
 //   if(Sideview_pHandeled[i].rc.top  > y0)
  //    Sideview_pHandeled[i].rc.top  = y0;

	if(Sideview_pHandeled[i].bRectAllowed == false)
      for(j =0 ; j < Sideview_pHandeled[i].iNoPolyPts ; j++)
      {
        Sideview_pHandeled[i].apPolygon[j].x = iround(Sideview_pHandeled[i].apPolygon[j].x * dx)+x0;
        if( Sideview_pHandeled[i].apPolygon[j].y > 0)
          Sideview_pHandeled[i].apPolygon[j].y = CalcHeightCoordinat((double)   Sideview_pHandeled[i].apPolygon[j].y,  rc, psDiag);
        else
          Sideview_pHandeled[i].apPolygon[j].y =  rc.bottom;
 //       if( Sideview_pHandeled[i].apPolygon[j].y  > y0)
  //        Sideview_pHandeled[i].apPolygon[j].y = y0;
      }
  }


  /**********************************************************************************
   * draw airspaces
   **********************************************************************************/
  HPEN mpen = (HPEN)CreatePen(PS_NULL, 0, RGB(0xf0,0xf0,0xb0));
  HPEN oldpen = (HPEN)SelectObject(hdc, (HPEN)mpen);
  _TCHAR text [80];
  SIZE tsize;
  for (int m=0 ; m < Sideview_iNoHandeldSpaces; m++)
  {
	int iSizeIdx =  iSizeLookupTable[m];
    {
	  int  type = Sideview_pHandeled[iSizeIdx].iType;
	  RECT rcd  = Sideview_pHandeled[iSizeIdx].rc;
	  if(Sideview_pHandeled[iSizeIdx].bEnabled)
	  {
		SelectObject(hdc, MapWindow::GetAirspaceBrushByClass(type));
		SetTextColor(hdc, MapWindow::GetAirspaceColourByClass(type));
		if(Sideview_pHandeled[iSizeIdx].bRectAllowed == true)
		{
		  Rectangle(hdc,rcd.left+1,rcd.top,rcd.right,rcd.bottom);
		}
		else
		{
		  Polygon(hdc,Sideview_pHandeled[iSizeIdx].apPolygon ,Sideview_pHandeled[iSizeIdx].iNoPolyPts );
		}
	  }

	  {
		//	NULL_BRUSH
		SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		double fFrameColFact = 1.0;
    	if(INVERTCOLORS)
    	  fFrameColFact = 0.8;
    	else
      	  fFrameColFact = 1.2;
		long lDisabledColor = ChangeBrightness( MapWindow::GetAirspaceColourByClass(type), fFrameColFact);


	//	HPEN Newpen = (HPEN)CreatePen(PS_DOT, 3, lDisabledColor);
		HPEN Newpen = (HPEN)CreatePen(PS_SOLID, 3, lDisabledColor);
		HPEN Oldpen = (HPEN)SelectObject(hdc, Newpen);
#define LINE_DIFF 2
		if(Sideview_pHandeled[iSizeIdx].bRectAllowed == true)
		  Rectangle(hdc,rcd.left+LINE_DIFF ,rcd.top - LINE_DIFF,rcd.right-LINE_DIFF,rcd.bottom+LINE_DIFF-1);
		else
		{
		  Polygon(hdc,Sideview_pHandeled[iSizeIdx].apPolygon ,Sideview_pHandeled[iSizeIdx].iNoPolyPts );
		}
#if TESTBENCH > 0
		SelectObject(hdc, GetStockObject(BLACK_PEN));
		Rectangle(hdc,rcd.left+1,rcd.top,rcd.right,rcd.bottom);
#endif
		SelectObject(hdc, Oldpen);
		DeleteObject (Newpen);
	  }


	  if(Sideview_pHandeled[iSizeIdx].bEnabled)
        SetTextColor(hdc, Sideview_TextColor); // RGB_MENUTITLEFG
      else
        SetTextColor(hdc, RGB_GGREY);

      int x = rcd.left + (rcd.right - rcd.left)/2;
      int y = rcd.top  - (rcd.top   - rcd.bottom)/2;

      _tcsncpy(text, Sideview_pHandeled[iSizeIdx].szAS_Name,NAME_SIZE-1/* sizeof(text)/sizeof(text[0])*/);
      GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
      x -= tsize.cx/2; // - NIBLSCALE(5);
      y -= tsize.cy;   // - NIBLSCALE(5);
      if (
           (tsize.cx < (rcd.right-rcd.left)) &&
           ((y)  < rcd.top) &&
           ((y + tsize.cy)  > rcd.bottom)
         )
      {
        ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
        y = rcd.top  - (rcd.top   - rcd.bottom)/2;
      }

      _tcsncpy((wchar_t*)text, (wchar_t*) CAirspaceManager::Instance().GetAirspaceTypeShortText( Sideview_pHandeled[iSizeIdx].iType), NAME_SIZE);
      GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
      x = rcd.left + (rcd.right - rcd.left)/2;
      if (
           (tsize.cx < (rcd.right-rcd.left)) &&
           ((y)  < rcd.top) &&
           ((y + tsize.cy)  > rcd.bottom)
         )
      {
        x -= tsize.cx/2; // - NIBLSCALE(5);
        ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
      }
    }
  }





  int iBottom = rc.bottom;
  // draw ground
  POINT ground[4];
  HPEN   hpHorizonGround;
  HBRUSH hbHorizonGround;
  int itemp;
  hpHorizonGround = (HPEN)CreatePen(PS_SOLID, IBLSCALE(1), GROUND_COLOUR);
  hbHorizonGround = (HBRUSH)CreateSolidBrush(GROUND_COLOUR);
  SelectObject(hdc, hpHorizonGround);
  SelectObject(hdc, hbHorizonGround);


#ifdef MSL_SEA_DRAW
  // draw sea
  if(psDiag->fYMin < GC_SEA_LEVEL_TOLERANCE)
  {
	RECT sea= {rc.left,rc.bottom,rc.right,rc.bottom-BORDER_Y};
	RenderSky( hdc,   sea, RGB_STEEL_BLUE, RGB_ROYAL_BLUE  , 7);
	iBottom-=BORDER_Y;

  }
#else
  if(psDiag->fYMin < GC_SEA_LEVEL_TOLERANCE)
	Rectangle(hdc,rc.left,rc.bottom,rc.right,rc.bottom-BORDER_Y);
#endif
   y0 = iBottom;
  SelectObject(hdc, hpHorizonGround);
  SelectObject(hdc, hbHorizonGround);



  for (j=1; j< AIRSPACE_SCANSIZE_X; j++) { // scan range
    ground[0].x = iround((j-1)*dx)+x0;
    ground[1].x = ground[0].x;
    ground[2].x = iround(j*dx)+x0;
    ground[3].x = ground[2].x;
    ground[0].y = iBottom;

    itemp = iround((d_h[j-1]-hmin)/(hmax-hmin)*(rc.top-rc.bottom+BORDER_Y))+y0;
    if (itemp>y0) itemp = y0;
    ground[1].y = itemp;
    itemp = iround((d_h[j]-hmin)/(hmax-hmin)*(rc.top-rc.bottom+BORDER_Y))+y0;
    if (itemp>y0) itemp = y0;
    ground[2].y = itemp;
    ground[3].y = iBottom;// y0;
    if ((ground[1].y == y0) && (ground[2].y == y0)) continue;
    Polygon(hdc, ground, 4);
  }




  SetTextColor(hdc, Sideview_TextColor); // RGB_MENUTITLEFG

  SelectObject(hdc, (HPEN)oldpen);
  DeleteObject(mpen);
  DeleteObject(hpHorizonGround);
  DeleteObject(hbHorizonGround);


}


