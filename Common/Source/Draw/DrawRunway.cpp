/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKStyle.h"
#include "DoInits.h"
#include "LKObjects.h"
#include <string.h>
#include "ScreenGeometry.h"
#include "Draw/ScreenProjection.h"
#include "Asset.hpp"

/**
 * @brief Draw Runway on map
 *
 * @remarks THIS FUNCTION is threadsafe only if called by dialogs using picto bool true
 *
 * @param Surface : Surface to Draw
 * @param wp : Landable Waypoint
 * @param rc : Clipping Rect
 * @param fScaleFact : Scaling factor for runway symbol radius and runway length, 
                       use #MapWindow::zoom value for Moving Map or constant for waypoint Picto
 * @param picto true for drawing Waypoint Picto ( don't draw radio info )
 *                     Pictos are painted in Dialogs. We need full scale choice here.
 */

void MapWindow::DrawRunway(LKSurface& Surface, const WAYPOINT* wp, const RECT& rc, const ScreenProjection* _Proj, double fScaleFact, BOOL picto)
{
  if(!picto && !_Proj) {
     // if we don't draw picto ScreenProjection parameter are mandatory.
     LKASSERT(false);
     return;
  }

  int solid= false;
  bool bOutland = false;

  static double dinitrw_len = 1;       // runway len
  static double dinitrw_thick = 1;     // runway thickness
  static double dinitrw_radius = 1;    // runway circle radius
  static double dinitrw_len_out = 1;   // outlanding runway len 
  static double dinitrw_thick_out = 1; // outlanding runway thickness

  static double scale_drawradio=0;
  static double scale_bigfont=0;
  static double scale_fullinfos=0;


  RasterPoint Center;
  if(picto) {
     Center.x = rc.left+ (rc.right- rc.left)/2;
     Center.y = rc.bottom +(rc.top-rc.bottom)/2;
  } else {
     Center =  _Proj->ToRasterPoint(wp->Latitude, wp->Longitude);
  }

  //
  // TOP AND BOTTOM ZOOM LEVELS for rescaling vectors.
  // RealScale is an absolute value, depending on resolution and distance unit in use.
  // The smaller RealScale, the bigger airfield becomes, within these limits.
  // Up to V6, we had values equivalent to (approx) 0.30 - 3.10
  // Notice: for very low resolutions, values are already preset and cannot be changed.
  //
  #define MIN_REALSCALE 0.30 // reduce to enlarge vectors at high zoom (ex.300m)
  #define MAX_REALSCALE 3.10 // increase to reduce vectors at low zoom (ex.7.5km)

  static double  dmin_realscale=0;
  static double  dmax_realscale=0;

  // 
  // TODO cache many values if fScaleFact does not change.
  // So we can use floating point arithmetic for accuracy.


  if (DoInit[MDI_MAPWPVECTORS])
  {

    //
    // Everything rescales properly on all resolutions.
    //
    dinitrw_radius = 6.5;           // the most important
    dinitrw_len = dinitrw_radius;     // half of len (+- from the center, so it doubles)
    dinitrw_thick = dinitrw_len/4.3;   // half of thickness
    //
    dinitrw_len_out   = dinitrw_len * 0.8;    // outlandings runway sizes
    dinitrw_thick_out = dinitrw_thick * 1.3;  // thickness

    //
    // These are the (absolute) scale thresholds for painting informations
    // For example, paint just radio starting from 5.0km (3.6abs) etc.
    // On very low resolutions, we cant split bits and pixels, and we must go custom.
    // No big deal, we are inside a doinit.
    //
    switch(ScreenSize)
    {
	case ss320x240:
	case ss240x320:
		scale_drawradio=2.6;
		scale_bigfont=1.1;
		scale_fullinfos=0.8;
                dmin_realscale=0.30;
                dmax_realscale=1.85;
		break;

	case ss480x272:
	case ss272x480:
		scale_drawradio=2.6;
		scale_bigfont=1.1;
		scale_fullinfos=0.8;
                dmin_realscale=0.30;
                dmax_realscale=2.5;
		break;
	case ss800x480:
	case ss480x800:
		scale_drawradio=2.6;
		scale_bigfont=1.5;
		scale_fullinfos=1.5;
                dmin_realscale=MIN_REALSCALE;
                dmax_realscale=MAX_REALSCALE;
		break;
	default:
                scale_drawradio=3.6;
                scale_bigfont=1.5;
                scale_fullinfos=1.5;
                dmin_realscale=MIN_REALSCALE;
                dmax_realscale=MAX_REALSCALE;
                break;
    }
    DoInit[MDI_MAPWPVECTORS]=false;
  }
  // END OF DOINIT


  // StartupStore(_T("IN fScale=%f  "),fScaleFact);

  //
  // Threshold boundaries, not for pictos
  //
  if (!picto) {
     if (fScaleFact < dmin_realscale) fScaleFact=dmin_realscale;
     if (fScaleFact > dmax_realscale) fScaleFact=dmax_realscale;
     // StartupStore(_T("NORM fScale=%f  "),fScaleFact);
     fScaleFact= MAX_REALSCALE / fScaleFact;
     // StartupStore(_T("OUT fScale=%f\n"),fScaleFact);
  }





  double drw_len,drw_thick;

  //
  // Ordered values for compiler jump table following LKStyle.h enum
  // Adjust here desired difference of sizes for each type.
  // By default, all similar except outlandings
  //
  switch(wp->Style) {
     case STYLE_AIRFIELDGRASS: 
        solid = false; bOutland = false;  
        drw_len = dinitrw_len;
        drw_thick = dinitrw_thick;
        break;

     case STYLE_OUTLANDING: 
        solid = false;  bOutland = true;   
        drw_thick=dinitrw_thick_out;
        drw_len=dinitrw_len_out;
        break;

     case STYLE_GLIDERSITE: 
        solid = true;   bOutland = false; 
        drw_len = dinitrw_len;
        drw_thick = dinitrw_thick;

        break;

     case STYLE_AIRFIELDSOLID: 
        solid = true;   bOutland = false;
        drw_len = dinitrw_len;
        drw_thick = dinitrw_thick;
        break;

     default: return; 
        break;
  }

  // 
  // If we have no information on runway len, we shall draw it as a square.
  // For airfields, the square is drawn inside circle.
  // For outlandings, as a standalone square
  //
  if( wp->RunwayLen == 0) {  
     drw_len = dinitrw_len *0.6;
     drw_thick = drw_len ;
  }

  //
  // Adjust sizes for current zoom level, and finally go integer.
  // We need to use float multiplier for radius, len, thick, or we loose any accuracy
  // when multiplying by scale factor. This is why we go integer only here.
  //

  int irw_radius = (int)(dinitrw_radius * fScaleFact); if(irw_radius==0) irw_radius=1;
  int irw_len    = (int)(drw_len * fScaleFact);   if(irw_len==0)    irw_len=1;
  int irw_thick  = (int)(drw_thick * fScaleFact); if(irw_thick==0)  irw_thick=1;

  // 
  // Rescale radius only, since len and thick are rescaled by polygon rotation 
  // (which is bad idea in any case- it should at least be called RescaledPolygon..)
  //
  irw_radius=IBLSCALE(irw_radius);


  const auto oldPen = Surface.SelectObject(LK_BLACK_PEN);
  const auto oldBrush = Surface.SelectObject(LKBrush_Red);

  if( wp->Reachable == TRUE)
    Surface.SelectObject(LKBrush_Green);


  if(!bOutland)
  {
	if (picto)
		Surface.DrawCircle(Center.x, Center.y, irw_radius, true);
	else
		Surface.DrawCircle(Center.x, Center.y, irw_radius,  rc, true);
  }

  POINT Runway[5] = {
     { irw_thick, irw_len },  // 1
     {-irw_thick, irw_len },  // 2
     {-irw_thick,-irw_len },  // 3
     { irw_thick,-irw_len },  // 4
     { irw_thick,irw_len  }   // 5
  };

  if(!bOutland) {
    if (!IsDithered()) {
      if (solid)
        Surface.SelectObject(LKBrush_DarkGrey);
      else
        Surface.SelectObject(LKBrush_White);
    } else {
      if (solid)
        Surface.SelectObject(LKBrush_Black);
      else
        Surface.SelectObject(LKBrush_White);
    }
  }

  if(picto) {
     threadsafePolygonRotateShift(Runway, 5,  Center.x, Center.y,  wp->RunwayDir);
  } else {
     PolygonRotateShift(Runway, 5,  Center.x, Center.y,  wp->RunwayDir- (int)MapWindow::GetDisplayAngle());
  }
  Surface.Polygon(Runway ,5 );


  //
  // Print waypoint information on screen, not for Pictos
  // 
  if( !picto && (MapWindow::zoom.RealScale() <= scale_drawradio)) {

     const auto hfOld = Surface.SelectObject(MapWindow::zoom.RealScale() <= scale_bigfont
                                             ? LK8PanelUnitFont : LK8GenericVar02Font);
    if (!IsDithered()) {
      if (INVERTCOLORS)
        Surface.SelectObject(LKBrush_Petrol);
      else
        Surface.SelectObject(LKBrush_LightCyan);
    } else {
      if (INVERTCOLORS)
        Surface.SelectObject(LKBrush_Black);
      else
        Surface.SelectObject(LKBrush_White);
    }

     unsigned int offset = irw_radius + NIBLSCALE(1);

     if ( _tcslen(wp->Freq)>0 ) {
        MapWindow::LKWriteBoxedText(Surface,rc,wp->Freq, Center.x- offset, Center.y -offset, 
                                    WTALIGN_RIGHT, RGB_WHITE, RGB_BLACK);
     }

     //
     // Full infos! 1.5km scale
     //
     if (MapWindow::zoom.RealScale() <=scale_fullinfos) {
        if ( _tcslen(wp->Code)==4 ) {
           MapWindow::LKWriteBoxedText(Surface,rc,wp->Code,Center.x + offset, Center.y - offset, 
                                       WTALIGN_LEFT, RGB_WHITE,RGB_BLACK);
        }

        if (wp->Altitude >0) {
           TCHAR tAlt[20];
           _stprintf(tAlt,_T("%.0f %s"),Units::ToUserAltitude(wp->Altitude),Units::GetAltitudeName());
           MapWindow::LKWriteBoxedText(Surface,rc,tAlt, Center.x + offset, Center.y + offset, 
                                       WTALIGN_LEFT, RGB_WHITE, RGB_BLACK);
        }
     }
     Surface.SelectObject(hfOld);
  }

  Surface.SelectObject(oldPen);
  Surface.SelectObject(oldBrush);

}

