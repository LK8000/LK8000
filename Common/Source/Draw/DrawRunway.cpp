/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKStyle.h"
#include "DoInits.h"
#include "LKObjects.h"


#if (WINDOWSPC>0)
#include <wingdi.h>
#endif

#include "utils/heapcheck.h"
#include <string.h>



void DrawRunway(HDC hdc,WAYPOINT* wp, RECT rc, double fScaleFact)
{
int solid= false;
HPEN    oldPen  ;
HBRUSH  oldBrush ;
bool bGlider = false;
bool bOutland = false;
bool bRunway = false;
static double rwl = 8;
static double rwb = 1;
static double cir = 6;
int l,p,b;
fScaleFact /=1600;
if (DoInit[MDI_MAPWPVECTORS]) // DoInit does not work correctly here
{
   switch(ScreenSize)
   {
     case ss240x320: rwl = 8.0; rwb = 2.0;cir = 4.0; break;
     case ss240x400: rwl = 8.0; rwb = 2.0;cir = 4.0; break;
     case ss272x480: rwl = 8.0; rwb = 2.5;cir = 5.0; break;
     case ss480x640: rwl = 6.0; rwb = 2.0;cir = 5.0; break;
     case ss480x800: rwl = 6.0; rwb = 1.2;cir = 5.0; break;
     case sslandscape: rwl = 6.0; rwb = 1.0;cir = 5.0; break;
     case ss320x240: rwl = 8.0; rwb = 2.0;cir = 4.0; break;
     case ss400x240: rwl = 7.0; rwb = 1.0;cir = 4.0; break;
     case ss480x234: rwl = 7.0; rwb = 1.0;cir = 4.0; break;
     case ss480x272: rwl = 8.0; rwb = 2.5;cir = 4.0; break;
     case ss640x480: rwl = 6.0; rwb = 2.5;cir = 5.0; break;
     case ss720x408: rwl = 6.0; rwb = 2.5;cir = 5.0; break;
     case ss800x480: rwl = 6.0; rwb = 2.5;cir = 5.0; break;
     case ss896x672: rwl = 6.0; rwb = 2.5;cir = 5.0; break;
   }
   DoInit[MDI_MAPWPVECTORS]=false;
}

//wp->RunwayLen =0;
if( wp->RunwayLen > 100) /* square if no runway defined */
{
  l = (int) (rwl * (1.0+ ((double)wp->RunwayLen/800.0-1.0)/4.0));
  b = (int) (rwb/1.5 );
}
else
{
  l = (int)( rwl*0.5);
  b = l ;
}

l = (int)(l * fScaleFact); if(l==0) l=1;
b = (int)(b * fScaleFact); if(b==0) b=1;
p = (int)(cir * 2.0 * fScaleFact); if(p==0) p=1;
int iScale = (int)(fScaleFact*2.0);if(iScale==0) iScale=1;







  switch(wp->Style) {
	case STYLE_AIRFIELDSOLID: solid = true;  bRunway  = true;  bOutland = false;  bGlider  = false;	break;
	case STYLE_AIRFIELDGRASS: solid = false; bRunway  = true;  bOutland = false;  bGlider  = false;	break;
	case STYLE_OUTLANDING	: solid = false; bRunway  = true;  bOutland = true;   bGlider  = false; b*=2; break;
	case STYLE_GLIDERSITE	: solid = false; bRunway  = true;  bOutland = false;  bGlider  = true;	break;
	default: return; break;
  }

  int Brg =0;
  if (  DisplayOrientation == TRACKUP )
	Brg = (int)GPS_INFO.TrackBearing;

    oldPen   = (HPEN) SelectObject(hdc, GetStockObject(BLACK_PEN));
    oldBrush = (HBRUSH)SelectObject(hdc, LKBrush_Red);

	if( wp->Reachable == TRUE)
	 oldBrush = (HBRUSH)SelectObject(hdc, LKBrush_Green);


	if(!bOutland)
	{
	  Circle( hdc, wp->Screen.x, wp->Screen.y, p,  rc,true, true);
	}

	if(bRunway)
	{
	  POINT Runway[5] = {
		  { b, l },  // 1
		  {-b, l },  // 2
		  {-b,-l },  // 3
		  { b,-l },  // 4
		  { b,l  }   // 5
	  };
	  if(!bOutland)
	  {
	    if(solid)
	  	  SelectObject(hdc, LKBrush_DarkGrey );
		else
		  SelectObject(hdc, LKBrush_White);
	  }
	  PolygonRotateShift(Runway, 5,  wp->Screen.x, wp->Screen.y,  wp->RunwayDir-Brg);
	  Polygon(hdc,Runway ,5 );
	}


	if(fScaleFact >= 0.9)
	  if(bGlider)
	  {
	    int iScale = (int)(fScaleFact*2.0);
	    if(iScale==0) iScale=1;
	    POINT WhiteWing [15]  = {
		  { 0 * iScale, 0 * iScale },   // 1
		  { 1 * iScale,-1 * iScale },   // 2
		  { 2 * iScale,-1 * iScale },   // 3
		  { 3 * iScale, 0 * iScale },   // 4
		  { 3 * iScale, 1 * iScale },   // 5
		  { 2 * iScale, 0 * iScale },   // 6
		  { 1 * iScale, 0 * iScale },   // 7
		  { 0 * iScale, 1 * iScale },   // 8
		  {-1 * iScale, 0 * iScale },   // 9
	 	  {-2 * iScale, 0 * iScale },   // 10
		  {-3 * iScale, 1 * iScale },   // 11
		  {-3 * iScale, 0 * iScale },   // 12
		  {-2 * iScale,-1 * iScale },   // 13
		  {-1 * iScale,-1 * iScale },   // 14
		  { 0 * iScale, 0 * iScale }    // 15
	    };
	    PolygonRotateShift(WhiteWing, 15,  wp->Screen.x, wp->Screen.y,  0/*+ wp->RunwayDir-Brg*/);
	    Polygon(hdc,WhiteWing ,15 );
	  }

	SelectObject(hdc, oldPen);
	SelectObject(hdc, oldBrush);
#define	PRINT_FREQUENCY
#ifdef PRINT_FREQUENCY
  if(fScaleFact >= 2)
	if (MapWindow::zoom.RealScale()<5.4)
	{
	  SIZE tsize;
      SetTextColor(hdc, RGB_BLACK);
	  HFONT hfOld = (HFONT)SelectObject(hdc, LK8PanelSmallFont);
	  GetTextExtentPoint(hdc, wp->Freq, _tcslen(wp->Freq), &tsize);
      ExtTextOut(hdc,wp->Screen.x-tsize.cx/2 ,wp->Screen.y-tsize.cy/2 , ETO_OPAQUE, NULL, wp->Freq, _tcslen( wp->Freq), NULL);
      SelectObject(hdc, hfOld);
	}
#endif
}


