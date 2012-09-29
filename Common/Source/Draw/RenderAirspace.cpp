/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InfoBoxLayout.h"
#include "McReady.h"
#include "dlgTools.h"
#include "Atmosphere.h"
#include "RasterTerrain.h"
#include "LKInterface.h"
#include "LKAirspace.h"
#include "RGB.h"
#include "Sideview.h"


double fSplitFact = 0.30;

using std::min;
using std::max;

double fZOOMScale = 1.0;
extern int Sideview_asp_heading_task;
extern AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS];

extern COLORREF  Sideview_TextColor;



void MapWindow::RenderAirspace(HDC hdc, const RECT rci) {

	RECT rc  = rci; /* rectangle for sideview */
//	rc.bottom += 4;
bool bInvCol = true; //INVERTCOLORS;


//	rc.bottom = rc.bottom/2;
  double fDist = 50.0*1000; // kmbottom
  double aclat, aclon, ach, acb, speed, calc_average30s;

  double wpt_brg;
  double wpt_dist;
  double wpt_altarriv;
  double wpt_altitude;
  double wpt_altarriv_mc0;
  double calc_terrainalt;
  double calc_altitudeagl;
  double fMC0 = 0.0f;
  int overindex=-1;
  bool show_mc0= true;
  double fLD;
  SIZE tsize;
  TCHAR text[80];
  TCHAR buffer[80];
  BOOL bDrawRightSide =false;
  COLORREF GREEN_COL     = RGB_GREEN;
  COLORREF RED_COL       = RGB_LIGHTORANGE;
  COLORREF BLUE_COL      = RGB_BLUE;
  COLORREF LIGHTBLUE_COL = RGB_LIGHTBLUE;
  zoom.SetLimitMapScale(false);
  /****************************************************************/

//  SelectObject(hdc, LK8InfoNormalFont);
  COLORREF col =  RGB_BLACK;
  if(bInvCol)
	col =  RGB_WHITE;

  HPEN hpHorizon   = (HPEN)  CreatePen(PS_SOLID, IBLSCALE(1), col);
  HBRUSH hbHorizon = (HBRUSH)CreateSolidBrush(col);
  HPEN OldPen      = (HPEN)   SelectObject(hdc, hpHorizon);
  HBRUSH OldBrush  = (HBRUSH) SelectObject(hdc, hbHorizon);


  Rectangle(hdc,rci.left,rci.top,rci.right,rci.bottom);

  SelectObject(hdc, OldPen);
  SelectObject(hdc, OldBrush);
  DeleteObject(hpHorizon);
  DeleteObject(hbHorizon);

  if(bInvCol)
  {
  //  SetBackColor(SKY_HORIZON_COL);
    Sideview_TextColor = INV_GROUND_TEXT_COLOUR;
  }
  else
    Sideview_TextColor = RGB_WHITE;

  SetTextColor(hdc, Sideview_TextColor);

  /****************************************************************/
  double GPSbrg=0;
  if (Sideview_asp_heading_task == 2)
	return  RenderNearAirspace( hdc,   rc);


	RECT rct = rc; /* rectangle for topview */
	rc.top     = (long)((double)(rc.bottom-rc.top  )*fSplitFact);
	rct.bottom = rc.top ;



//  if (Sideview_asp_heading_task == 3)
//	return MapWindow::LKDrawFlarmRadar( hdc,   rc);
  if(bInvCol)
  {
    GREEN_COL     = ChangeBrightness(GREEN_COL     , 0.6);
    RED_COL       = ChangeBrightness(RGB_RED       , 0.6);;
    BLUE_COL      = ChangeBrightness(BLUE_COL      , 0.6);;
    LIGHTBLUE_COL = ChangeBrightness(LIGHTBLUE_COL , 0.4);;
  }
  LockFlightData();
  {
    fMC0 = GlidePolar::SafetyMacCready;
    aclat = GPS_INFO.Latitude;
    aclon = GPS_INFO.Longitude;
    ach   = GPS_INFO.Altitude;

    acb    = GPS_INFO.TrackBearing;

//    LL_to_BearRange(aclat, aclon,  WayPointList[GetOvertargetIndex()].Latitude,   WayPointList[GetOvertargetIndex()].Longitude  ,&acb, &wpt_dist);


    GPSbrg = acb;

    acb    = GPS_INFO.TrackBearing;
    GPSbrg = GPS_INFO.TrackBearing;
//    DrawInfo
    speed = GPS_INFO.Speed;

    calc_average30s = CALCULATED_INFO.Average30s;

// TODO FIX CHECK  use NavAltitude instead, no need to use alt
    if (GPS_INFO.BaroAltitudeAvailable && EnableNavBaroAltitude) {
      CALCULATED_INFO.NavAltitude = GPS_INFO.BaroAltitude;
    } else {
      CALCULATED_INFO.NavAltitude = GPS_INFO.Altitude;
    }
    calc_terrainalt = CALCULATED_INFO.TerrainAlt;
    calc_altitudeagl = CALCULATED_INFO.AltitudeAGL;
  }
  UnlockFlightData();

  HFONT hfOld = (HFONT)SelectObject(hdc, LK8PanelUnitFont);
  overindex = GetOvertargetIndex();
  wpt_brg = AngleLimit360( acb );
  wpt_dist         = 0.0;
  wpt_altarriv     = 0.0;
  wpt_altarriv_mc0 = 0.0;
  wpt_altitude     = 0.0;
  fMC0 = 0.0;
  fLD  = 0.0;


  if (Sideview_asp_heading_task)
  {
    // Show towards target
    if (overindex>=0)
    {
      double wptlon = WayPointList[overindex].Longitude;
      double wptlat = WayPointList[overindex].Latitude;
      DistanceBearing(aclat, aclon, wptlat, wptlon, &wpt_dist, &acb);

      wpt_brg = AngleLimit360(wpt_brg - acb +90.0);
      fDist = max(5.0*1000.0, wpt_dist*1.15);   // 20% more distance to show, minimum 5km
      wpt_altarriv     = WayPointCalc[overindex].AltArriv[ALTA_MC ];
      wpt_altarriv_mc0 = WayPointCalc[overindex].AltArriv[ALTA_MC0];
      wpt_altitude     = WayPointList[overindex].Altitude;
       // calculate the MC=0 arrival altitude



      LockFlightData();
      wpt_altarriv_mc0 =   CALCULATED_INFO.NavAltitude -
        GlidePolar::MacCreadyAltitude( fMC0,
                                       wpt_dist,
                                       acb,
                                       CALCULATED_INFO.WindSpeed,
                                       CALCULATED_INFO.WindBearing,
                                       0, 0, true,
                                       0)  - WayPointList[overindex].Altitude;
      if (IsSafetyAltitudeInUse(overindex)) wpt_altarriv_mc0 -= SAFETYALTITUDEARRIVAL;

      wpt_altarriv =   CALCULATED_INFO.NavAltitude -
        GlidePolar::MacCreadyAltitude( MACCREADY,
                                       wpt_dist,
                                       acb,
                                       CALCULATED_INFO.WindSpeed,
                                       CALCULATED_INFO.WindBearing,
                                       0, 0, true,
                                       0)  - WayPointList[overindex].Altitude;

     
      if ( (CALCULATED_INFO.NavAltitude-wpt_altarriv+wpt_altitude)!=0)
      	fLD = (int) wpt_dist / (CALCULATED_INFO.NavAltitude-wpt_altarriv+wpt_altitude);
      else
	fLD=999;

      if (IsSafetyAltitudeInUse(overindex)) wpt_altarriv -= SAFETYALTITUDEARRIVAL;


      UnlockFlightData();

    }
    else
    {
      // no selected target
      Statistics::DrawNoData(hdc, rc);
      return;
    }
  }
  

  double hmin = max(0.0, CALCULATED_INFO.NavAltitude-2300);
  double hmax = max(MAXALTTODAY, CALCULATED_INFO.NavAltitude+1000);

  if(fZOOMScale != 1.0)
  {
    if( (fDist *fZOOMScale) > 750000)
	  fZOOMScale /= ZOOMFACTOR;

    if((fDist *fZOOMScale) < 5000)
	  fZOOMScale *= ZOOMFACTOR;
  }
  fDist *=fZOOMScale;
  DiagrammStruct sDia;
  sDia.fXMin =-5000.0f;
  if( sDia.fXMin > (-0.1f * fDist))
	sDia.fXMin = -0.1f * fDist;
  sDia.fXMax = fDist;
  sDia.fYMin = hmin;
  sDia.fYMax = hmax;
  sDia.rc = rc;
  RenderAirspaceTerrain( hdc, aclat, aclon,  acb, ( DiagrammStruct*) &sDia );


/*
  zoom.RequestedScale((sDia.fXMax -sDia.fXMin)* (DISTANCEMODIFY/10.0f));
double fOldScale  =  zoom.Scale();
*/
  Statistics::ResetScale();
  Statistics::ScaleXFromValue(rc, sDia.fXMin);
  Statistics::ScaleXFromValue(rc, sDia.fXMax);
  Statistics::ScaleYFromValue(rc, sDia.fYMin);
  Statistics::ScaleYFromValue(rc, sDia.fYMax);

  int x0 = CalcDistanceCoordinat( 0,  &sDia);
  int y0 = CalcHeightCoordinat  ( 0,  &sDia);

  double xtick = 1.0;
  if (fDist>10.0*1000.0) xtick = 5.0;
  if (fDist>50.0*1000.0) xtick = 10.0;
  if (fDist>100.0*1000.0) xtick = 20.0;
  if (fDist>200.0*1000.0) xtick = 25.0;
  if (fDist>250.0*1000.0) xtick = 50.0;
  if (fDist>500.0*1000.0) xtick = 100.0;

  if(bInvCol)
  {
    SelectObject(hdc, GetStockObject(BLACK_PEN));
    SelectObject(hdc, GetStockObject(BLACK_BRUSH));
  }
  else
  {
    SelectObject(hdc, GetStockObject(WHITE_PEN));
    SelectObject(hdc, GetStockObject(WHITE_BRUSH));
  }

  SetTextColor(hdc, GROUND_TEXT_COLOUR);
  if(bInvCol)
    if(sDia.fYMin > GC_SEA_LEVEL_TOLERANCE)
	  SetTextColor(hdc, INV_GROUND_TEXT_COLOUR);

  if(fSplitFact > 0.0)
  {
  	sDia.rc = rct;
    if (Sideview_asp_heading_task == 0)
  	  MapWindow::AirspaceTopView(hdc, &sDia, GPSbrg, 90.0 );

    if (Sideview_asp_heading_task == 1)
  	  MapWindow::AirspaceTopView(hdc, &sDia, acb, wpt_brg );
  	sDia.rc = rc;
  }

  Statistics::DrawXGrid(hdc, rci, xtick/DISTANCEMODIFY, 0,  STYLE_THINDASHPAPER, xtick, true);
  SetTextColor(hdc, Sideview_TextColor);
  if(Units::GetUserInvAltitudeUnit() == unFeet)
	Statistics::DrawYGrid(hdc, rc, 500.0/ALTITUDEMODIFY, 0, STYLE_THINDASHPAPER, 500.0, true);
  else
	Statistics::DrawYGrid(hdc, rc, 1000.0/ALTITUDEMODIFY, 0, STYLE_THINDASHPAPER, 1000.0, true);

  POINT line[4];

  // draw target symbolic line
  int iWpPos =  CalcDistanceCoordinat( wpt_dist,  &sDia);
  if (Sideview_asp_heading_task > 0)
  {
    if(WayPointCalc[overindex].IsLandable == 0)
    {
      // Not landable - Mark wpt with a vertical marker line
      line[0].x = CalcDistanceCoordinat( wpt_dist,  &sDia);
      line[0].y = y0;
      line[1].x = line[0].x;
      line[1].y = rc.top;
      Statistics::StyleLine(hdc, line[0], line[1], STYLE_WHITETHICK, rc);
    }
    else
    {
      // Landable
      line[0].x = iWpPos;
      line[0].y = CalcHeightCoordinat( wpt_altitude, &sDia);
      line[1].x = line[0].x;
      line[1].y = CalcHeightCoordinat( SAFETYALTITUDEARRIVAL+wpt_altitude,  &sDia );
      Statistics::StyleLine(hdc, line[0], line[1], STYLE_ORANGETHICK, rc);

      float fArrHight = 0.0f;
      if(wpt_altarriv > 0.0f)
      {
        fArrHight = wpt_altarriv;
        line[0].x = iWpPos;
        line[0].y = CalcHeightCoordinat( SAFETYALTITUDEARRIVAL+wpt_altitude,  &sDia );
        line[1].x = line[0].x;
        line[1].y = CalcHeightCoordinat( SAFETYALTITUDEARRIVAL+wpt_altitude+fArrHight, &sDia );
        Statistics::StyleLine(hdc, line[0], line[1], STYLE_GREENTHICK, rc);
      }
      // Mark wpt with a vertical marker line
      line[0].x = iWpPos;
      line[0].y = CalcHeightCoordinat( SAFETYALTITUDEARRIVAL+wpt_altitude+fArrHight,  &sDia );
      line[1].x = line[0].x;
      line[1].y = rc.top;
      Statistics::StyleLine(hdc, line[0], line[1], STYLE_WHITETHICK, rc);
    }
  }

  // Draw estimated gliding line (blue)
 // if (speed>10.0)
  {
    if (Sideview_asp_heading_task > 0) {
      double altarriv;
      // Draw estimated gliding line MC=0 (green)
      if( show_mc0 )
      {
        altarriv = wpt_altarriv_mc0 + wpt_altitude;
        if (IsSafetyAltitudeInUse(overindex)) altarriv += SAFETYALTITUDEARRIVAL;
        line[0].x = CalcDistanceCoordinat( 0, &sDia);
        line[0].y = CalcHeightCoordinatOutbound  ( CALCULATED_INFO.NavAltitude, &sDia );
        line[1].x = CalcDistanceCoordinat( wpt_dist, &sDia);
        line[1].y = CalcHeightCoordinatOutbound( altarriv ,  &sDia );
        Statistics::StyleLine(hdc, line[0], line[1], STYLE_BLUETHIN, rc);
      }
      altarriv = wpt_altarriv + wpt_altitude;
      if (IsSafetyAltitudeInUse(overindex)) altarriv += SAFETYALTITUDEARRIVAL;
      line[0].x = CalcDistanceCoordinat( 0, &sDia);
      line[0].y = CalcHeightCoordinatOutbound( CALCULATED_INFO.NavAltitude, &sDia );
      line[1].x = CalcDistanceCoordinat( wpt_dist, &sDia);
      line[1].y = CalcHeightCoordinatOutbound( altarriv ,  &sDia );
      Statistics::StyleLine(hdc, line[0], line[1], STYLE_BLUETHIN, rc);
    } else {
      double t = fDist/(speed!=0?speed:1);

      line[0].x = CalcDistanceCoordinat( 0, &sDia);
      line[0].y = CalcHeightCoordinat  ( CALCULATED_INFO.NavAltitude, &sDia);
      line[1].x = rc.right;
      line[1].y = CalcHeightCoordinat  ( CALCULATED_INFO.NavAltitude+calc_average30s*t, &sDia);
      Statistics::StyleLine(hdc, line[0], line[1], STYLE_BLUETHIN, rc);
    }
  }


  SelectObject(hdc, GetStockObject(Sideview_TextColor));
  SelectObject(hdc, GetStockObject(WHITE_BRUSH));


  //Draw wpt info texts
  if (Sideview_asp_heading_task > 0) {
//HFONT hfOld = (HFONT)SelectObject(hdc, LK8MapFont);
    line[0].x = CalcDistanceCoordinat( wpt_dist,  &sDia);
    // Print wpt name next to marker line
    LK_tcsncpy(text, WayPointList[overindex].Name, sizeof(text)/sizeof(text[0]) - 1);
    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
    int x = line[0].x - tsize.cx - NIBLSCALE(5);

    if (x<x0) bDrawRightSide = true;
    if (bDrawRightSide) x = line[0].x + NIBLSCALE(5);
    int y = rc.top + 3*tsize.cy;

    SetTextColor(hdc, Sideview_TextColor);

//	HFONT oldfont=NULL;
//	oldfont=(HFONT)SelectObject(hdc,LK8TargetFont);
	TextInBoxMode_t TextInBoxMode = {0};
	TextInBoxMode.Color = RGB_WHITE;
	TextInBoxMode.NoSetFont=1;
	TextInBoxMode.AlligneCenter = 0;
	TextInBoxMode.AlligneRight = 1;
	TextInBoxMode.WhiteBorder = 1;
	TextInBoxMode.Border = 1;

	// same position for gps warnings: if navwarning, then no alarms. So no overlapping.
    //    TextInBox(hdc, text ,line[0].x, y, 0, &TextInBoxMode);

//	SelectObject(hdc,oldfont);

    ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

    // Print wpt distance
    Units::FormatUserDistance(wpt_dist, text, 7);
    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
    x = line[0].x - tsize.cx - NIBLSCALE(5);
    if (bDrawRightSide) x = line[0].x + NIBLSCALE(5);
    y += tsize.cy;
    ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    double altarriv = wpt_altarriv_mc0; // + wpt_altitude;
    if (IsSafetyAltitudeInUse(overindex)) altarriv += SAFETYALTITUDEARRIVAL;



    SetTextColor(hdc, Sideview_TextColor);
    if (wpt_altarriv_mc0 > ALTDIFFLIMIT)
    {
      _stprintf(text, TEXT("Mc %3.1f: "), (LIFTMODIFY*fMC0));
      Units::FormatUserArrival(wpt_altarriv_mc0, buffer, 7);
      _tcscat(text,buffer);
    } else {
      LK_tcsncpy(text, TEXT("---"), sizeof(text)/sizeof(text[0]) - 1);
    }
    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
    x = line[0].x - tsize.cx - NIBLSCALE(5);
    if (bDrawRightSide) x = line[0].x + NIBLSCALE(5);   // Show on right side if left not possible
    y += tsize.cy;

    if((wpt_altarriv_mc0 > 0) && (  WayPointList[overindex].Reachable)) {
  	  SetTextColor(hdc, GREEN_COL);
    } else {
  	  SetTextColor(hdc, RED_COL);
    }
    ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    
    // Print arrival altitude
    if (wpt_altarriv > ALTDIFFLIMIT) {
      _stprintf(text, TEXT("Mc %3.1f: "), (LIFTMODIFY*MACCREADY));
//      iround(LIFTMODIFY*MACCREADY*10)/10.0
      Units::FormatUserArrival(wpt_altarriv, buffer, 7);
      _tcscat(text,buffer);
    } else {
      LK_tcsncpy(text, TEXT("---"), sizeof(text)/sizeof(text[0]) - 1);
    }

    if(  WayPointList[overindex].Reachable) {
  	  SetTextColor(hdc, GREEN_COL);
    } else {
  	  SetTextColor(hdc, RED_COL);
    }
    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
    x = line[0].x - tsize.cx - NIBLSCALE(5);
    if (bDrawRightSide) x = line[0].x + NIBLSCALE(5);   // Show on right side if left not possible
    y += tsize.cy;
    ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

    // Print arrival AGL
    altarriv = wpt_altarriv;
    if (IsSafetyAltitudeInUse(overindex)) altarriv += SAFETYALTITUDEARRIVAL;
    if(altarriv  > 0)
    {
  	  Units::FormatUserAltitude(altarriv, buffer, 7);
      LK_tcsncpy(text, gettext(TEXT("_@M1742_")), sizeof(text)/sizeof(text[0]) - 1);
      _tcscat(text,buffer);
      GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
   //   x = CalcDistanceCoordinat(wpt_dist,  rc) - tsize.cx - NIBLSCALE(5);;
      x = line[0].x - tsize.cx - NIBLSCALE(5);
      if (bDrawRightSide) x = line[0].x + NIBLSCALE(5);
      y = CalcHeightCoordinat(  altarriv + wpt_altitude , &sDia );
      if(  WayPointList[overindex].Reachable) {
        SetTextColor(hdc, GREEN_COL);
      } else {
        SetTextColor(hdc, RED_COL);
      }
      ExtTextOut(hdc, x, y-tsize.cy/2, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    }
    // Print current Elevation
    SetTextColor(hdc, RGB_BLACK);
    if((calc_terrainalt- hmin) > 0)
    {
  	  Units::FormatUserAltitude(calc_terrainalt, buffer, 7);
      LK_tcsncpy(text, gettext(TEXT("_@M1743_")), sizeof(text)/sizeof(text[0]) - 1);   // ELV:
      _tcscat(text,buffer);
      GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
      x = CalcDistanceCoordinat(0, &sDia)- tsize.cx/2;
      y = CalcHeightCoordinat(  (calc_terrainalt), &sDia );
      if ((ELV_FACT*tsize.cy) < abs(rc.bottom - y))
      {
        ExtTextOut(hdc, x, rc.bottom -(int)(ELV_FACT * tsize.cy) /* rc.top-tsize.cy*/, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
      }
    }


    // Print arrival Elevation
    SetTextColor(hdc, RGB_BLACK);
    if((wpt_altitude- hmin) > 0)
    {
  	  Units::FormatUserAltitude(wpt_altitude, buffer, 7);
      LK_tcsncpy(text, gettext(TEXT("_@M1743_")), sizeof(text)/sizeof(text[0]) - 1);   // ELV:
      _tcscat(text,buffer);
      GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
      x0 = CalcDistanceCoordinat(wpt_dist, &sDia)- tsize.cx/2;
      if(abs(x - x0)> tsize.cx )
      {
        y = CalcHeightCoordinat(  (wpt_altitude), &sDia );
          if ((ELV_FACT*tsize.cy) < abs(rc.bottom - y))
          {
            ExtTextOut(hdc, x0, rc.bottom -(int)(ELV_FACT * tsize.cy) /* rc.top-tsize.cy*/, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
          }
      }
    }


    if(altarriv  > 0)
    {
    // Print L/D
      _stprintf(text, TEXT("1/%i"), (int)fLD);
      GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
      SetTextColor(hdc, BLUE_COL);
      x = CalcDistanceCoordinat(wpt_dist/2, &sDia)- tsize.cx/2;
      y = CalcHeightCoordinat( (CALCULATED_INFO.NavAltitude + altarriv)/2 + wpt_altitude , &sDia ) + tsize.cy;
      ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    }

    // Print current AGL
    if(calc_altitudeagl - hmin > 0)
    {
      SetTextColor(hdc, LIGHTBLUE_COL);
      Units::FormatUserAltitude(calc_altitudeagl, buffer, 7);
      LK_tcsncpy(text, gettext(TEXT("_@M1742_")), sizeof(text)/sizeof(text[0]) - 1);
      _tcscat(text,buffer);
      GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
      x = CalcDistanceCoordinat( 0, &sDia) - tsize.cx/2;
      y = CalcHeightCoordinat(  (calc_terrainalt +  calc_altitudeagl)*0.8,   &sDia );
    //    if(x0 > tsize.cx)
          if((tsize.cy) < ( CalcHeightCoordinat(  calc_terrainalt, &sDia )-y)) {
            ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
          }
    }
    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, hfOld);
//SelectObject(hdc, hfOld);
  } // if Sideview_asp_heading_task

;
  

  if (!Sideview_asp_heading_task)
    wpt_brg =90.0;
  RenderPlaneSideview( hdc,  0.0f, CALCULATED_INFO.NavAltitude,wpt_brg, &sDia );
  HFONT hfOld2 = (HFONT)SelectObject(hdc, LK8InfoNormalFont);
  SetTextColor(hdc, Sideview_TextColor);
  SetBkMode(hdc, OPAQUE);

//  SetTextColor(hdc, RGB_BLACK);
  DrawTelescope      ( hdc, acb-90.0, rc.right - NIBLSCALE(13),  rc.top   + NIBLSCALE(58));
  SelectObject(hdc, hfOld2);
  SelectObject(hdc, hfOld);

  SetTextColor(hdc, GROUND_TEXT_COLOUR);
  if(bInvCol)
    if(sDia.fYMin > GC_SEA_LEVEL_TOLERANCE)
	  SetTextColor(hdc, INV_GROUND_TEXT_COLOUR);

  Statistics::DrawXLabel(hdc, rc, TEXT("D"));
  SetTextColor(hdc, Sideview_TextColor);
  Statistics::DrawYLabel(hdc, rc, TEXT("h"));

  SelectObject(hdc,hfOld/* Sender->GetFont()*/);

  if(fSplitFact > 0.0)
  	sDia.rc = rct;

  hfOld = (HFONT)SelectObject(hdc,LK8InfoNormalFont/* Sender->GetFont()*/);

  DrawNorthArrow     ( hdc, GPSbrg          , rct.right - NIBLSCALE(13),  rct.top   + NIBLSCALE(13));
//  RenderBearingDiff( hdc, wpt_brg,  &sDia );


  SelectObject(hdc,hfOld/* Sender->GetFont()*/);
  zoom.SetLimitMapScale(true);
}






