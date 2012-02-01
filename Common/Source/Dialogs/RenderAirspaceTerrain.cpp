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


extern AirSpaceSideViewSTRUCT Sideview_pHandeled[GC_MAX_NO];
extern COLORREF Sideview_TextColor;
extern int Sideview_iNoHandeldSpaces;



void RenderAirspaceTerrain(HDC hdc, const RECT rc,double PosLat, double PosLon,  double brg,  DiagrammStruct* psDiag )
{

  double range =psDiag->fXMax - psDiag->fXMin; // km
  double fi, fj;
  double hmin = psDiag->fYMin;
  double hmax = psDiag->fYMax;
  double lat, lon;
  RECT rcd;
  int i,j,k;
   AirSpaceSideViewSTRUCT Sideview_asDrawn[GC_MAX_NO];
#if (WINDOWSPC>0)
  if(INVERTCOLORS)
    RenderSky( hdc, rc, SKY_HORIZON_COL , SKY_SPACE_COL , GC_NO_COLOR_STEPS);
#endif

  FindLatitudeLongitude(PosLat, PosLon, brg  , psDiag->fXMin , &lat, &lon);

  double d_lat[AIRSPACE_SCANSIZE_X];
  double d_lon[AIRSPACE_SCANSIZE_X];
  double d_alt[AIRSPACE_SCANSIZE_X];
  double d_h[AIRSPACE_SCANSIZE_H];
  AirSpaceSideViewSTRUCT d_airspace[AIRSPACE_SCANSIZE_H][AIRSPACE_SCANSIZE_X];


  BOOL bFound = false;
  for ( k=0 ; k < GC_MAX_NO; k++) {
    wsprintf(   Sideview_asDrawn[k].szAS_Name, TEXT(""));
    Sideview_asDrawn[k].aiLable = ID_NO_LABLE;
    Sideview_asDrawn[k].iType   = -1;
    Sideview_asDrawn[k].iIdx    = -1;
    Sideview_pHandeled[k].bEnabled=false;
    Sideview_asDrawn[k].psAS    = NULL;
    wsprintf( Sideview_pHandeled[k].szAS_Name, TEXT(""));
    Sideview_pHandeled[k].aiLable = ID_NO_LABLE;
    Sideview_pHandeled[k].iType   = -1;
    Sideview_pHandeled[k].iIdx    = -1;
    Sideview_pHandeled[k].bEnabled=false;
    Sideview_pHandeled[k].psAS    = NULL;
  }




#define FRACT 0.75
  RasterTerrain::Lock();
  // want most accurate rounding here
  RasterTerrain::SetTerrainRounding(0,0);

  for (j=0; j< AIRSPACE_SCANSIZE_X; j++) { // scan range
    fj = j*1.0/(AIRSPACE_SCANSIZE_X-1);
    FindLatitudeLongitude(lat, lon, brg, range*fj,
                          &d_lat[j], &d_lon[j]);
    d_alt[j] = RasterTerrain::GetTerrainHeight(d_lat[j], d_lon[j]);
    if (d_alt[j] == TERRAIN_INVALID) d_alt[j]=0; //@ 101027 BUGFIX
    hmax = max(hmax, d_alt[j]);
  }
  RasterTerrain::Unlock();


  double dfi = 1.0/(AIRSPACE_SCANSIZE_H-1);
  double dfj = 1.0/(AIRSPACE_SCANSIZE_X-1);

  for (i=0; i< AIRSPACE_SCANSIZE_H; i++) { // scan height
    d_h[i] = (hmax-hmin)*i*dfi+hmin;
  }
  for (i=0; i< AIRSPACE_SCANSIZE_H; i++) { // scan height
    for (j=0; j< AIRSPACE_SCANSIZE_X; j++) { // scan range
      d_airspace[i][j].iType =  -1; // no airspace
    }
  }
  CAirspaceManager::Instance().ScanAirspaceLine(d_lat, d_lon, d_h, d_alt, d_airspace);
  int type;

  double dx = dfj*(rc.right-rc.left-BORDER_X);
  double dy = dfi*(rc.top-rc.bottom+BORDER_Y);
  int x0 = rc.left+BORDER_X;
  int y0 = rc.bottom-BORDER_Y;


  for ( k=0 ; k < GC_MAX_NO; k++)
  {
	wsprintf(   Sideview_asDrawn[k].szAS_Name, TEXT(""));
	Sideview_asDrawn[k].aiLable = ID_NO_LABLE;
	Sideview_asDrawn[k].iType   = -1;
	Sideview_asDrawn[k].iIdx    = -1;
	Sideview_asDrawn[k].psAS    = NULL;
    wsprintf( Sideview_pHandeled[k].szAS_Name, TEXT(""));
    Sideview_pHandeled[k].aiLable = ID_NO_LABLE;
    Sideview_pHandeled[k].iType   = -1;
    Sideview_pHandeled[k].iIdx    = -1;
    Sideview_pHandeled[k].psAS    = NULL;
  }

  HPEN mpen = (HPEN)CreatePen(PS_NULL, 0, RGB(0xf0,0xf0,0xb0));
  HPEN oldpen = (HPEN)SelectObject(hdc, (HPEN)mpen);

  Sideview_iNoHandeldSpaces=0;
  for (i=0; i< AIRSPACE_SCANSIZE_H; i++)
  { // scan height
    fi = i*dfi;
    for (j=0; j< AIRSPACE_SCANSIZE_X; j++)
    { // scan range
      fj = j*dfj;
      type = d_airspace[i][j].iType;
      {
        if (type>=0)
        {
	      rcd.left = iround((j-FRACT)*dx)+x0;
	      rcd.right = iround((j+FRACT)*dx)+x0;
	      rcd.bottom = iround(((i)+FRACT)*dy)+y0;
	      rcd.top = iround(((i)-FRACT)*dy)+y0;
		  bFound = false;
          if(rcd.top > y0)
        	rcd.top = y0;
          if(d_airspace[i][j].bEnabled)
		    if(d_airspace[i][j].bRectAllowed == false)
		    {
		  	  SelectObject(hdc, MapWindow::GetAirspaceBrushByClass(type));
			  SetTextColor(hdc, MapWindow::GetAirspaceColourByClass(type));
			  Rectangle(hdc,rcd.left,rcd.top,rcd.right,rcd.bottom);
		    }

          for (k=0 ; k < Sideview_iNoHandeldSpaces; k++)
          {
            if(k < GC_MAX_NO)
            {
			  if(  Sideview_pHandeled[k].iIdx  ==  d_airspace[i][j].iIdx )
			  {
			    bFound = true;
			    if( rcd.left   < Sideview_pHandeled[k].rc.left  )  Sideview_pHandeled[k].rc.left    = rcd.left;
			    if( rcd.right  > Sideview_pHandeled[k].rc.right )  Sideview_pHandeled[k].rc.right   = rcd.right;
			    if( rcd.bottom < Sideview_pHandeled[k].rc.bottom)  Sideview_pHandeled[k].rc.bottom  = rcd.bottom;
			    if( rcd.top    > Sideview_pHandeled[k].rc.top   )  Sideview_pHandeled[k].rc.top     = rcd.top;
			  }
            }
          }
	      if(!bFound)
	      {
	        if(Sideview_iNoHandeldSpaces < GC_MAX_NO)
	        {
		      Sideview_pHandeled[Sideview_iNoHandeldSpaces].iType =  d_airspace[i][j].iType;
		      Sideview_pHandeled[Sideview_iNoHandeldSpaces].iIdx  =  d_airspace[i][j].iIdx;
		      Sideview_pHandeled[Sideview_iNoHandeldSpaces].bRectAllowed =  d_airspace[i][j].bRectAllowed;
		      Sideview_pHandeled[Sideview_iNoHandeldSpaces].psAS     = d_airspace[i][j].psAS;
		      Sideview_pHandeled[Sideview_iNoHandeldSpaces].rc       = rcd;
		      Sideview_pHandeled[Sideview_iNoHandeldSpaces].bEnabled =  d_airspace[i][j].bEnabled;

			  _tcsncpy(Sideview_pHandeled[Sideview_iNoHandeldSpaces].szAS_Name, d_airspace[i][j].szAS_Name, NAME_SIZE-1);
			  Sideview_iNoHandeldSpaces++;
	        }
	      }
        }
      }
    }
  }


  SelectObject(hdc, (HPEN)mpen);
  for (k=0 ; k < Sideview_iNoHandeldSpaces; k++)
  {
	if( Sideview_pHandeled[k].iIdx != -1)
    {
	  int  type = Sideview_pHandeled[k].iType;
	  RECT rcd =Sideview_pHandeled[k].rc;
	  if(Sideview_pHandeled[k].bEnabled)
	  {
		if(Sideview_pHandeled[k].bRectAllowed == true)
		{
		  SelectObject(hdc, MapWindow::GetAirspaceBrushByClass(type));
		  SetTextColor(hdc, MapWindow::GetAirspaceColourByClass(type));
		  Rectangle(hdc,rcd.left,rcd.top,rcd.right,rcd.bottom);
		}
	  }
	  else
	  {
		//	NULL_BRUSH
		SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		long lDisabledColor = ChangeBrightness( MapWindow::GetAirspaceColourByClass(type), 0.4f);
	//	HPEN Newpen = (HPEN)CreatePen(PS_DOT, 3, lDisabledColor);
		HPEN Newpen = (HPEN)CreatePen(PS_SOLID, 3, lDisabledColor);
		HPEN Oldpen = (HPEN)SelectObject(hdc, Newpen);
		Rectangle(hdc,rcd.left,rcd.top,rcd.right,rcd.bottom);
		SelectObject(hdc, Oldpen);
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
	RenderSky( hdc,   sea,  ChangeBrightness(RGB_BLUE,1.4) , RGB_BLUE , 7);
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

    itemp = iround((d_alt[j-1]-hmin)/(hmax-hmin)*(rc.top-rc.bottom+BORDER_Y))+y0;
    if (itemp>y0) itemp = y0;
    ground[1].y = itemp;
    itemp = iround((d_alt[j]-hmin)/(hmax-hmin)*(rc.top-rc.bottom+BORDER_Y))+y0;
    if (itemp>y0) itemp = y0;
    ground[2].y = itemp;
    ground[3].y = iBottom;// y0;
    if ((ground[1].y == y0) && (ground[2].y == y0)) continue;
    Polygon(hdc, ground, 4);
  }


BOOL bDrawn ;
int iNoOfDrawnNames=0;
_TCHAR text [80];
SIZE tsize;
  HFONT hfOld = (HFONT)SelectObject(hdc, LK8PanelUnitFont); // LK8MapFont
    for (k=0 ; k < Sideview_iNoHandeldSpaces ; k++)
    {
      if( Sideview_pHandeled[k].iIdx != -1)
      {
        bDrawn = false;
        for (i=0; i < iNoOfDrawnNames; i++)
        {
          if(Sideview_pHandeled[k].bEnabled)
            if( bDrawn == false)
            {
              if(_tcsncmp((TCHAR*)Sideview_asDrawn[i].szAS_Name, (TCHAR*)Sideview_pHandeled[k].szAS_Name,NAME_SIZE) == 0)
              //if(  Sideview_pHandeled[k].iIdx  ==  Sideview_asDrawn[i].iIdx )
              if(Sideview_pHandeled[k].iType == Sideview_asDrawn[i].iType) bDrawn = true;
            }
        }

        if(bDrawn == false)
        {
          int  type = Sideview_pHandeled[k].iType;
          SelectObject(hdc, MapWindow::GetAirspaceBrushByClass(type));
          if(Sideview_pHandeled[k].bEnabled)
            SetTextColor(hdc, RGB_BLUE /*ChangeBrightness( Sideview_TextColor, 1.4)*/); // RGB_MENUTITLEFG
          else
            SetTextColor(hdc, RGB_GGREY);
          RECT rcd =Sideview_pHandeled[k].rc;

          int x = rcd.left + (rcd.right - rcd.left)/2;
          int y = rcd.top  - (rcd.top   - rcd.bottom)/2;

          _tcsncpy(text, Sideview_pHandeled[k].szAS_Name,NAME_SIZE-1/* sizeof(text)/sizeof(text[0])*/);
          GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
          x -= tsize.cx/2; // - NIBLSCALE(5);
          y -= tsize.cy;   // - NIBLSCALE(5);
          if ( (Sideview_asDrawn[iNoOfDrawnNames].aiLable < ID_FULL_LABLE) && /* already drawn ? */
               (tsize.cx < (rcd.right-rcd.left)) &&
               ((y)  < rcd.top) &&
               ((y + tsize.cy)  > rcd.bottom)
             )
          {
            ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

            _tcsncpy((wchar_t*)  Sideview_asDrawn[iNoOfDrawnNames].szAS_Name, (wchar_t*) Sideview_pHandeled[k].szAS_Name, NAME_SIZE);
            Sideview_asDrawn[iNoOfDrawnNames].aiLable = ID_FULL_LABLE;
            Sideview_asDrawn[iNoOfDrawnNames].iType = type;
            Sideview_asDrawn[iNoOfDrawnNames].iIdx  = Sideview_pHandeled[k].iIdx;
            iNoOfDrawnNames++;
            y = rcd.top  - (rcd.top   - rcd.bottom)/2;
          }

          _tcsncpy((wchar_t*)text, (wchar_t*) CAirspaceManager::Instance().GetAirspaceTypeShortText( Sideview_pHandeled[k].iType), NAME_SIZE);
          GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
          x = rcd.left + (rcd.right - rcd.left)/2;
          if ( (Sideview_asDrawn[iNoOfDrawnNames].aiLable == ID_NO_LABLE)  && /* already drawn ? */
               (tsize.cx < (rcd.right-rcd.left)) &&
               ((y)  < rcd.top) &&
               ((y + tsize.cy)  > rcd.bottom)
             )
          {
            x -= tsize.cx/2; // - NIBLSCALE(5);
            ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
             _tcsncpy((wchar_t*)  Sideview_asDrawn[iNoOfDrawnNames].szAS_Name, (wchar_t*) Sideview_pHandeled[k].szAS_Name, NAME_SIZE);
             Sideview_asDrawn[iNoOfDrawnNames].aiLable = ID_SHORT_LABLE;
             Sideview_asDrawn[iNoOfDrawnNames].iType = type;
             Sideview_asDrawn[iNoOfDrawnNames].iIdx  = Sideview_pHandeled[k].iIdx;
            iNoOfDrawnNames++;
          }
        } //if bDrawn==false
      } //if iIdx!=-1
    } //for k
  SelectObject(hdc, hfOld);
  SetTextColor(hdc, Sideview_TextColor); // RGB_MENUTITLEFG

  SelectObject(hdc, (HPEN)oldpen);
  DeleteObject(mpen);
  DeleteObject(hpHorizonGround);
  DeleteObject(hbHorizonGround);


}

