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
#include "LKObjects.h"
#include "Multimap.h"
#include "Topology.h"
#include "Dialogs.h"

extern	 int Sideview_iNoHandeldSpaces;
extern	 AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS];

double fSplitFact = 0.30;
double fOffset = 0.0;
using std::min;
using std::max;
int k;
double fZOOMScale[3] = {1.0,1.0,1.0};
double fDelta = MIN_OFFSET;
extern int XstartScreen, YstartScreen;
extern COLORREF  Sideview_TextColor;


#define TBSIZE 80
#define ADDITIONAL_INFO_THRESHOLD 0.5

void MapWindow::RenderAirspace(HDC hdc, const RECT rci) {
  zoom.SetLimitMapScale(false);
  /****************************************************************/
  if (GetSideviewPage() == IM_NEAR_AS)
	return  RenderNearAirspace( hdc,   rci);
  /****************************************************************/


static bool bHeightScale = false;
RECT rc  = rci; /* rectangle for sideview */
bool bInvCol = true; //INVERTCOLORS;
static double fHeigtScaleFact = MIN_OFFSET;
double fDist = 55.0*1000; // kmbottom
double aclat, aclon, ach, acb, speed, calc_average30s;
double GPSbrg=0;
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
bool bSideView = false;
SIZE tsize;
TCHAR text[TBSIZE+1];
TCHAR buffer[TBSIZE+1];
BOOL bDrawRightSide =false;
COLORREF GREEN_COL     = RGB_GREEN;
COLORREF RED_COL       = RGB_LIGHTORANGE;
COLORREF BLUE_COL      = RGB_BLUE;
COLORREF LIGHTBLUE_COL = RGB_LIGHTBLUE;
COLORREF col           =  RGB_BLACK;
double zoomfactor=1;

int *iSplit = &Multimap_SizeY[Get_Current_Multimap_Type()];
unsigned short getsideviewpage=GetSideviewPage();
LKASSERT(getsideviewpage<3);

  if ( Current_Multimap_SizeY<SIZE4 )
	zoomfactor=ZOOMFACTOR;
  else
	zoomfactor=2.0;

#if 0
StartupStore(_T("...Type=%d  CURRENT=%d  Multimap_size=%d = isplit=%d\n"),
	Get_Current_Multimap_Type(), Current_Multimap_SizeY, Multimap_SizeY[Get_Current_Multimap_Type()], *iSplit);
#endif

  if(bInvCol)
	col =  RGB_WHITE;

  HPEN hpHorizon   = (HPEN)  CreatePen(PS_SOLID, IBLSCALE(1), col);
  HBRUSH hbHorizon = (HBRUSH)CreateSolidBrush(col);
  HPEN OldPen      = (HPEN)   SelectObject(hdc, hpHorizon);
  HBRUSH OldBrush  = (HBRUSH) SelectObject(hdc, hbHorizon);


  //bool bFound = false;
  SelectObject(hdc, OldPen);
  SelectObject(hdc, OldBrush);
  DeleteObject(hpHorizon);
  DeleteObject(hbHorizon);

  RECT rct = rc; /* rectangle for topview */
  rc.top     = (long)((double)(rci.bottom-rci.top  )*fSplitFact);
  rct.bottom = rc.top ;
  // Expose the topview rect size in use..
  Current_Multimap_TopRect=rct;


  if(bInvCol)
    Sideview_TextColor = INV_GROUND_TEXT_COLOUR;
  else
    Sideview_TextColor = RGB_WHITE;

  SetTextColor(hdc, Sideview_TextColor);

  /****************************************************************/
	  switch(LKevent) {
		case LKEVENT_NEWRUN:
			// CALLED ON ENTRY: when we select this page coming from another mapspace
			///fZOOMScale = 1.0;
			bHeightScale = false;
			if (IsMultimapTopology()) ForceVisibilityScan=true;
		//	fHeigtScaleFact = 1000;
		break;
		case LKEVENT_UP:
			// click on upper part of screen, excluding center
			if(bHeightScale)
			  fHeigtScaleFact -=  fDelta;
			else
			  fZOOMScale[getsideviewpage] /= zoomfactor;

			if (IsMultimapTopology()) ForceVisibilityScan=true;
			break;

		case LKEVENT_DOWN:
			// click on lower part of screen,  excluding center
			if(bHeightScale)
			  fHeigtScaleFact += fDelta;
			else
		  	  fZOOMScale[getsideviewpage] *= zoomfactor;

			if (IsMultimapTopology()) ForceVisibilityScan=true;
			break;

		case LKEVENT_LONGCLICK:
	//		ToggleMMNorthUp(getsideviewpage);
		     for (k=0 ; k <= Sideview_iNoHandeldSpaces; k++)
	             {
			   if( Sideview_pHandeled[k].psAS != NULL)
			   {
				 if (PtInRect(XstartScreen,YstartScreen,Sideview_pHandeled[k].rc ))
				 {
				   if (EnableSoundModes)PlayResource(TEXT("IDR_WAV_BTONE4"));
				   dlgAirspaceDetails(Sideview_pHandeled[k].psAS);       // dlgA
				   //bFound = true;
				   LKevent=LKEVENT_NONE; 
				   // return; // NO. We must finish painting the background instead.
				   // otherwise upon exiting we have no map underneath.
				 }
			   }
		     }
#if 0
		     // This is not working correctly because InteriorAirspaceDetails is finding
		     // only the first airspace in the list, only one.
		     if (LKevent!=LKEVENT_NONE) {
			double xlat, ylon;
			SideviewScreen2LatLon(XstartScreen,YstartScreen,xlat,ylon);
			if (Event_InteriorAirspaceDetails(xlat, ylon))
			{
			        LKevent=LKEVENT_NONE; 
			}
	             }			
#endif

		     if (LKevent!=LKEVENT_NONE) {
			 if (PtInRect(XstartScreen, YstartScreen,rc ))
			   bHeightScale = !bHeightScale;
			 if (PtInRect(XstartScreen, YstartScreen,rct ))
			   bHeightScale = false;
		     }
	     break;

		case LKEVENT_PAGEUP:
#ifdef OFFSET_SETP
			if(bHeightScale)
			  fOffset -= OFFSET_SETP;
			else
#endif
			{
			  if(*iSplit == SIZE1) *iSplit = SIZE0;
			  if(*iSplit == SIZE2) *iSplit = SIZE1;
			  if(*iSplit == SIZE3) *iSplit = SIZE2;
			  if(*iSplit == SIZE4) *iSplit = SIZE3;
			}
		break;
		case LKEVENT_PAGEDOWN:
#ifdef OFFSET_SETP
			if(bHeightScale)
			  fOffset += OFFSET_SETP;
			else
#endif
			{
			  if(*iSplit == SIZE3) *iSplit = SIZE4;
			  if(*iSplit == SIZE2) *iSplit = SIZE3;
			  if(*iSplit == SIZE1) *iSplit = SIZE2;
			  if(*iSplit == SIZE0) *iSplit = SIZE1;
			}

		break;

	  }
          if ( (fSplitFact*100)<SIZE4 )
           bSideView = true;
	  //LKevent=LKEVENT_NONE;

	  // Current_Multimap_SizeY is global, and must be used by all multimaps!
	  // It is defined in Multimap.cpp and as an external in Multimap.h
	  // It is important that it is updated, because we should resize terrain
	  // only if needed! Resizing terrain takes some cpu and some time.
	  // So we need to know when this is not necessary, having the same size of previous
	  // multimap, if we are switching.
	  // The current implementation is terribly wrong by managing resizing of sideview in
	  // each multimap: it should be done by a common layer.
	  // CAREFUL:
	  // If for any reason DrawTerrain() is called after resizing to 0 (full sideview)
	  // LK WILL CRASH with no hope to recover.
	  if(Current_Multimap_SizeY != *iSplit)
	  {
		Current_Multimap_SizeY=*iSplit;
		SetSplitScreenSize(*iSplit);
		rc.top     = (long)((double)(rci.bottom-rci.top  )*fSplitFact);
		rct.bottom = rc.top ;
	  }

	  double hmin =  max(0.0, DerivedDrawInfo.NavAltitude-0);
	  double hmax = max(MAXALTTODAY, DerivedDrawInfo.NavAltitude+0);
#ifdef OFFSET_SETP
	  if((hmax + fOffset) > MAX_ALTITUDE)
		fOffset -= OFFSET_SETP;
	  if((hmin + fOffset) < 0.0)
		fOffset += OFFSET_SETP;

	  hmin +=  fOffset;
	  hmax +=  fOffset;
#endif

    if(  ( fHeigtScaleFact) < MIN_ALTITUDE)
      fHeigtScaleFact = MIN_ALTITUDE;
//	if(  (DerivedDrawInfo.NavAltitude +  fHeigtScaleFact) > MAX_ALTITUDE)
//	  fHeigtScaleFact -= fDelta;

	hmax =  DerivedDrawInfo.NavAltitude +  fHeigtScaleFact;
	hmin = DerivedDrawInfo.NavAltitude -  2*fHeigtScaleFact;
	if( hmin <0 )
	 hmin = 0;





  if(bInvCol)
  {
    GREEN_COL     = ChangeBrightness(GREEN_COL     , 0.6);
    RED_COL       = ChangeBrightness(RGB_RED       , 0.6);;
    BLUE_COL      = ChangeBrightness(BLUE_COL      , 0.6);;
    LIGHTBLUE_COL = ChangeBrightness(LIGHTBLUE_COL , 0.4);;
  }
//  LockFlightData();
  {
    fMC0 = GlidePolar::SafetyMacCready;
    aclat = DrawInfo.Latitude;
    aclon = DrawInfo.Longitude;
    ach   = DrawInfo.Altitude;
    acb    = DrawInfo.TrackBearing;
    GPSbrg = acb;

    acb    = DrawInfo.TrackBearing;
    GPSbrg = DrawInfo.TrackBearing;
    speed = DrawInfo.Speed;

    calc_average30s = DerivedDrawInfo.Average30s;
    calc_terrainalt = DerivedDrawInfo.TerrainAlt;
    calc_altitudeagl = DerivedDrawInfo.AltitudeAGL;
  }
//  UnlockFlightData();

  HFONT hfOld = (HFONT)SelectObject(hdc, LK8PanelUnitFont);
  overindex = GetOvertargetIndex();
  wpt_brg = AngleLimit360( acb );
  wpt_dist         = 0.0;
  wpt_altarriv     = 0.0;
  wpt_altarriv_mc0 = 0.0;
  wpt_altitude     = 0.0;
  fMC0 = 0.0;
  fLD  = 0.0;

  if (getsideviewpage==IM_NEXT_WP )
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



      //LockFlightData();
      wpt_altarriv_mc0 =   DerivedDrawInfo.NavAltitude -
        GlidePolar::MacCreadyAltitude( fMC0,
                                       wpt_dist,
                                       acb,
                                       DerivedDrawInfo.WindSpeed,
                                       DerivedDrawInfo.WindBearing,
                                       0, 0, true,
                                       0)  - WayPointList[overindex].Altitude;
      if (IsSafetyAltitudeInUse(overindex)) wpt_altarriv_mc0 -= (SAFETYALTITUDEARRIVAL/10);

      wpt_altarriv =   DerivedDrawInfo.NavAltitude -
        GlidePolar::MacCreadyAltitude( MACCREADY,
                                       wpt_dist,
                                       acb,
                                       DerivedDrawInfo.WindSpeed,
                                       DerivedDrawInfo.WindBearing,
                                       0, 0, true,
                                       0)  - WayPointList[overindex].Altitude;

     
      if ( (DerivedDrawInfo.NavAltitude-wpt_altarriv+wpt_altitude)!=0)
      	fLD = (int) wpt_dist / (DerivedDrawInfo.NavAltitude-wpt_altarriv+wpt_altitude);
      else
	fLD=999;

      if (IsSafetyAltitudeInUse(overindex)) wpt_altarriv -= (SAFETYALTITUDEARRIVAL/10);


      //UnlockFlightData();

    } // valid overindex
    else {
	// Not valid overindex, we paint like HEADING
	wpt_brg=90.0;
    }
  } // NEXTWP
  
  // Else in MAPWPT with no overtarget we paint a track heading

  if(fZOOMScale[getsideviewpage] != 1.0)
  {
    if( (fDist *fZOOMScale[getsideviewpage]) > 750000)
	  fZOOMScale[getsideviewpage] /= zoomfactor;

    if((fDist *fZOOMScale[getsideviewpage]) < 500)
	  fZOOMScale[getsideviewpage] *= zoomfactor;
  }
  fDist *=fZOOMScale[getsideviewpage];

  DiagrammStruct sDia;
  sDia.fXMin =-9500.0f;

  #if 0
  if( sDia.fXMin > (-0.1f * fDist))
	sDia.fXMin = -0.1f * fDist;
  if( -sDia.fXMin > (fDist))
    sDia.fXMin = -fDist;
  #else
  if (IsMultimapOverlaysText()|| IsMultimapOverlaysGauges()) {
	sDia.fXMin = -0.35f * fDist;
  } else {
	sDia.fXMin = -0.2f * fDist;
  }
  #endif

  sDia.fXMax = fDist;
  sDia.fYMin = hmin;
  sDia.fYMax = hmax;


  #if 1
  // TO CHECK, MOVED FROM OTHER FORWARD POSITION
  // What is this text color used for.. apparently there is no difference to take it off.
  SetTextColor(hdc, GROUND_TEXT_COLOUR);
  if(bInvCol)
    if(sDia.fYMin > GC_SEA_LEVEL_TOLERANCE)
	  SetTextColor(hdc, INV_GROUND_TEXT_COLOUR);

  if(fSplitFact > 0.0)
  {
  	sDia.rc = rct;
	sDia.rc.bottom-=1;
    if (getsideviewpage == IM_HEADING)
  	  MapWindow::AirspaceTopView(hdc, &sDia, GPSbrg, 90.0, bSideView);

    if (getsideviewpage == IM_NEXT_WP)
  	  MapWindow::AirspaceTopView(hdc, &sDia, acb, wpt_brg, false );

    //sDia.rc = rcc;

  }
  #endif


  RECT rcc =  rc;
  if( (Current_Multimap_SizeY<SIZE4) && (sDia.fYMin < GC_SEA_LEVEL_TOLERANCE))
    rcc.bottom -= SV_BORDER_Y; /* scale witout sea  */
  sDia.rc = rcc;


  if ((Current_Multimap_SizeY<SIZE4) || ( GetMMNorthUp(getsideviewpage)  != NORTHUP))
    RenderAirspaceTerrain( hdc, aclat, aclon,  acb, ( DiagrammStruct*) &sDia );



  int x0 = CalcDistanceCoordinat( 0,  &sDia);
  int y0 = CalcHeightCoordinat  ( 0,  &sDia);

  double xtick = 0.5;
  double fRange = fabs (sDia.fXMax-sDia.fXMin);
  if (fRange>2.0*1000.0) xtick = 1.0;
  if (fRange>8.0*1000.0) xtick = 2.0;
  if (fRange>15*1000.0) xtick = 5.0;
  if (fRange>50.0*1000.0) xtick = 10.0;
  if (fRange>100.0*1000.0) xtick = 20.0;
  if (fRange>200.0*1000.0) xtick = 25.0;
  if (fRange>250.0*1000.0) xtick = 50.0;
  if (fRange>500.0*1000.0) xtick = 100.0;
  if (fRange>1000.0*1000.0) xtick = 500.0;

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

#if 0
  // THIS HAS BEEN MOVED UP, BEFORE PAINTING SIDEVIEW
  SetTextColor(hdc, GROUND_TEXT_COLOUR);
  if(bInvCol)
    if(sDia.fYMin > GC_SEA_LEVEL_TOLERANCE)
	  SetTextColor(hdc, INV_GROUND_TEXT_COLOUR);

  if(fSplitFact > 0.0)
  {
  	sDia.rc = rct;
    if (getsideviewpage == IM_HEADING)
  	  MapWindow::AirspaceTopView(hdc, &sDia, GPSbrg, 90.0 );

    if (getsideviewpage == IM_NEXT_WP)
  	  MapWindow::AirspaceTopView(hdc, &sDia, acb, wpt_brg );
  	sDia.rc = rcc;
  }
#endif

  SelectObject(hdc, LK8PanelUnitFont);
  COLORREF txtCol = GROUND_TEXT_COLOUR;
  if(bInvCol)
    if(sDia.fYMin > GC_SEA_LEVEL_TOLERANCE)
    	txtCol = INV_GROUND_TEXT_COLOUR;
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, txtCol);

  _stprintf(text, TEXT("%s"),Units::GetUnitName(Units::GetUserDistanceUnit()));

 if (Current_Multimap_SizeY<SIZE4)
  switch(GetMMNorthUp(getsideviewpage))
  {
	 case NORTHUP:
	 default:
       DrawXGrid(hdc, rc, xtick/DISTANCEMODIFY, xtick, 0,TEXT_ABOVE_LEFT, Sideview_TextColor,  &sDia,text);
     break;

	 case TRACKUP:
       DrawXGrid(hdc, rci, xtick/DISTANCEMODIFY, xtick, 0,TEXT_ABOVE_LEFT, Sideview_TextColor,  &sDia,text);
     break;
  }
  SetTextColor(hdc, Sideview_TextColor);

  double fHeight = sDia.fYMax - sDia.fYMin;
  double  ytick = 100.0;
  if (fHeight >500.0) ytick = 200.0;
  if (fHeight >1000.0) ytick = 500.0;
  if (fHeight >2000.0) ytick = 1000.0;
  if (fHeight >4000.0) ytick = 2000.0;
  if (fHeight >8000.0) ytick = 4000.0;

  if(Units::GetUserAltitudeUnit() == unFeet)
	 ytick = ytick * FEET_FACTOR;

  _stprintf(text, TEXT("%s"),Units::GetUnitName(Units::GetUserAltitudeUnit()));

  // Draw only if topview is not fullscreen
  if (Current_Multimap_SizeY<SIZE4)
	DrawYGrid(hdc, rcc, ytick/ALTITUDEMODIFY,ytick, 0,TEXT_UNDER_RIGHT ,Sideview_TextColor,  &sDia, text);

  POINT line[4];

  // draw target symbolic line

 if (overindex>=0) {
  int iWpPos =  CalcDistanceCoordinat( wpt_dist,  &sDia);
  if ( (getsideviewpage == IM_NEXT_WP) && (Current_Multimap_SizeY<SIZE4))
  {
    if(WayPointCalc[overindex].IsLandable == 0)
    {
      // Not landable - Mark wpt with a vertical marker line
      line[0].x = CalcDistanceCoordinat( wpt_dist,  &sDia);
      line[0].y = y0;
      line[1].x = line[0].x;
      line[1].y = rc.top;
      DrawDashLine(hdc,4, line[0], line[1],  RGB_DARKGREY, rc);
    }
    else
    {
      // Landable
      line[0].x = iWpPos;
      line[0].y = CalcHeightCoordinat( wpt_altitude, &sDia);
      line[1].x = line[0].x;
      line[1].y = CalcHeightCoordinat( (SAFETYALTITUDEARRIVAL/10)+wpt_altitude,  &sDia );
      _DrawLine   (hdc, PS_SOLID, 5, line[0], line[1], RGB_ORANGE, rc);


      float fArrHight = 0.0f;
      if(wpt_altarriv > 0.0f)
      {
        fArrHight = wpt_altarriv;
        line[0].x = iWpPos;
        line[0].y = CalcHeightCoordinat( (SAFETYALTITUDEARRIVAL/10)+wpt_altitude,  &sDia );
        line[1].x = line[0].x;
        line[1].y = CalcHeightCoordinat( (SAFETYALTITUDEARRIVAL/10)+wpt_altitude+fArrHight, &sDia );
        _DrawLine   (hdc, PS_SOLID, 4, line[0], line[1], RGB_GREEN, rc);
      }
      // Mark wpt with a vertical marker line
      line[0].x = iWpPos;
      line[0].y = CalcHeightCoordinat( (SAFETYALTITUDEARRIVAL/10)+wpt_altitude+fArrHight,  &sDia );
      line[1].x = line[0].x;
      line[1].y = rc.top;
      DrawDashLine(hdc,4, line[0], line[1],  RGB_DARKGREY, rc);
    }
  }
 } // overindex is valid

  // Draw estimated gliding line (blue)
 // if (speed>10.0)
  if ( Current_Multimap_SizeY<SIZE4 ) 
  {
    //
    // WE FORCE CLIPPING, BEING IN MULTIMAPS
    //
    ForcedClipping=true;

    if ( (getsideviewpage == IM_NEXT_WP) && (overindex>=0)) {
      double altarriv;

      // Draw estimated gliding line MC=0 (green)
      if( !ISCAR && show_mc0 )
      {
        altarriv = wpt_altarriv_mc0 + wpt_altitude;
        if (IsSafetyAltitudeInUse(overindex)) altarriv += (SAFETYALTITUDEARRIVAL/10);
        line[0].x = CalcDistanceCoordinat( 0, &sDia);
        line[0].y = CalcHeightCoordinat ( DerivedDrawInfo.NavAltitude, &sDia );
        line[1].x = CalcDistanceCoordinat( wpt_dist, &sDia);
        line[1].y = CalcHeightCoordinatOutbound( altarriv ,  &sDia );
        DrawDashLine(hdc,3, line[0], line[1],  RGB_BLUE, rc);
      }
      altarriv = wpt_altarriv + wpt_altitude;
      if (IsSafetyAltitudeInUse(overindex)) altarriv += (SAFETYALTITUDEARRIVAL/10);
      line[0].x = CalcDistanceCoordinat( 0, &sDia);
      line[0].y = CalcHeightCoordinat( DerivedDrawInfo.NavAltitude, &sDia );

	if (ISCAR) {
		line[1].x = CalcDistanceCoordinat( wpt_dist, &sDia);
		line[1].y = CalcHeightCoordinatOutbound( wpt_altitude ,  &sDia );
	} else {
		line[1].x = CalcDistanceCoordinat( wpt_dist, &sDia);
		line[1].y = CalcHeightCoordinatOutbound( altarriv ,  &sDia );
	}
	DrawDashLine(hdc,3, line[0], line[1],  RGB_BLUE, rc);

    } else {
    //  double t = fDist/(speed!=0?speed:1);
      if (SIMMODE && !DerivedDrawInfo.Flying) {
	      calc_average30s= -GlidePolar::bestld;
	      if (calc_average30s<1) calc_average30s=1;
	      double t = fabs(DerivedDrawInfo.NavAltitude / calc_average30s);
	      line[0].x = CalcDistanceCoordinat( 0, &sDia);
	      line[0].y = CalcHeightCoordinat  ( DerivedDrawInfo.NavAltitude, &sDia);
	      line[1].x = CalcDistanceCoordinat( GlidePolar::Vbestld * t, &sDia);
	      line[1].y = CalcHeightCoordinat  ( 0, &sDia);
      }  else {
	      if (calc_average30s<1) calc_average30s=1;
	      double t = fabs(DerivedDrawInfo.NavAltitude / calc_average30s);
	      line[0].x = CalcDistanceCoordinat( 0, &sDia);
	      line[0].y = CalcHeightCoordinat  ( DerivedDrawInfo.NavAltitude, &sDia);
	      line[1].x = CalcDistanceCoordinat( speed * t, &sDia);
	      line[1].y = CalcHeightCoordinat  ( 0, &sDia);
      }
      // Limit climb rate to flat, for free flyers
      if (ISGLIDER || ISPARAGLIDER)
	    if ( line[1].y  < line[0].y )  line[1].y  = line[0].y;

      DrawDashLine(hdc,3, line[0], line[1],  RGB_BLUE, rc);
    }
    ForcedClipping=false;
  }


  SelectObject(hdc, GetStockObject(Sideview_TextColor));
  SelectObject(hdc, GetStockObject(WHITE_BRUSH));


  //Draw wpt info texts
  if ( (getsideviewpage == IM_NEXT_WP) && (Current_Multimap_SizeY<SIZE4) && (overindex>=0)) {
//HFONT hfOld = (HFONT)SelectObject(hdc, LK8MapFont);
    line[0].x = CalcDistanceCoordinat( wpt_dist,  &sDia);
    // Print wpt name next to marker line

    GetOvertargetName(text);

    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
    int x = line[0].x - tsize.cx - NIBLSCALE(5);

    if (x<x0) bDrawRightSide = true;
    if (bDrawRightSide) x = line[0].x + NIBLSCALE(5);
    int y = rc.top + 3*tsize.cy;

    SetTextColor(hdc, Sideview_TextColor);

    /**************************************************
     * draw waypoint target
     **************************************************/
	if (INVERTCOLORS)
	  SelectObject(hdc,LKBrush_Petrol);
	else
	  SelectObject(hdc,LKBrush_LightCyan);
	MapWindow::LKWriteBoxedText(hdc,&MapRect,text,  line[0].x, y-3, 0, WTALIGN_CENTER, RGB_WHITE, RGB_BLACK);


	// Print wpt distance
    Units::FormatUserDistance(wpt_dist, text, 7);
    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
    x = line[0].x - tsize.cx - NIBLSCALE(5);
    if (bDrawRightSide) x = line[0].x + NIBLSCALE(5);
    y += tsize.cy;
    if(fSplitFact < ADDITIONAL_INFO_THRESHOLD)
      ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    double altarriv = wpt_altarriv_mc0; // + wpt_altitude;
    if (IsSafetyAltitudeInUse(overindex)) altarriv += (SAFETYALTITUDEARRIVAL/10);



    SetTextColor(hdc, Sideview_TextColor);
    if (wpt_altarriv_mc0 > ALTDIFFLIMIT)
    {
      _stprintf(text, TEXT("Mc %3.1f: "), (LIFTMODIFY*fMC0));
      Units::FormatUserArrival(wpt_altarriv_mc0, buffer, 7);
      _tcscat(text,buffer);
    } else {
      _tcscpy(text, TEXT("---"));
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
    if(fSplitFact < ADDITIONAL_INFO_THRESHOLD)
      ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    
    // Print arrival altitude
    if (wpt_altarriv > ALTDIFFLIMIT) {
      _stprintf(text, TEXT("Mc %3.1f: "), (LIFTMODIFY*MACCREADY));
//      iround(LIFTMODIFY*MACCREADY*10)/10.0
      Units::FormatUserArrival(wpt_altarriv, buffer, 7);
      _tcscat(text,buffer);
    } else {
      if(fSplitFact < ADDITIONAL_INFO_THRESHOLD)
        _tcscpy(text, TEXT("---"));
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
    if(fSplitFact < ADDITIONAL_INFO_THRESHOLD)
      ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

    // Print arrival AGL
    altarriv = wpt_altarriv;
    if (IsSafetyAltitudeInUse(overindex)) altarriv += (SAFETYALTITUDEARRIVAL/10);
    if(altarriv  > 0)
    {
  	  Units::FormatUserAltitude(altarriv, buffer, 7);
      LK_tcsncpy(text, MsgToken(1742), TBSIZE-_tcslen(buffer));
      _tcscat(text,buffer);
      GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
   //   x = CalcDistanceCoordinat(wpt_dist,  rc) - tsize.cx - NIBLSCALE(5);;
      x = line[0].x - tsize.cx - NIBLSCALE(5);
      if (bDrawRightSide) x = line[0].x + NIBLSCALE(5);
      y = CalcHeightCoordinat(  altarriv + wpt_altitude , &sDia );
      if(  WayPointList[overindex].Reachable) { // CAREFUL, overindex<0 was making sw crash here.
        SetTextColor(hdc, GREEN_COL);
      } else {
        SetTextColor(hdc, RED_COL);
      }
      if(fSplitFact < ADDITIONAL_INFO_THRESHOLD)
        ExtTextOut(hdc, x, y-tsize.cy/2, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    }

    // Print current Elevation
    SetTextColor(hdc, RGB_BLACK);
    if((calc_terrainalt- hmin) > 0)
    {
  	  Units::FormatUserAltitude(calc_terrainalt, buffer, 7);
      LK_tcsncpy(text, MsgToken(1743), TBSIZE - _tcslen(buffer));
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
      LK_tcsncpy(text, MsgToken(1743), TBSIZE - _tcslen(buffer));
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
      SetTextColor(hdc, RGB_BLACK);
      x = CalcDistanceCoordinat(wpt_dist/2, &sDia)- tsize.cx/2;
      y = CalcHeightCoordinat( (DerivedDrawInfo.NavAltitude + altarriv)/2 + wpt_altitude , &sDia ) + tsize.cy;
      ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    }

    // Print current AGL
    if(calc_altitudeagl - hmin > 0)
    {
      SetTextColor(hdc, LIGHTBLUE_COL);
      Units::FormatUserAltitude(calc_altitudeagl, buffer, 7);
      LK_tcsncpy(text, MsgToken(1742), TBSIZE-_tcslen(buffer));
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
  } // IM_NEXT_WP with valid overindex


  if (getsideviewpage== IM_HEADING)
    wpt_brg =90.0;

  if ( Current_Multimap_SizeY<SIZE4)
  RenderPlaneSideview( hdc,  0.0f, DerivedDrawInfo.NavAltitude,wpt_brg, &sDia );

  HFONT hfOld2 = (HFONT)SelectObject(hdc, LK8InfoNormalFont);
  SetTextColor(hdc, Sideview_TextColor);

  SelectObject(hdc, hfOld2);
  SelectObject(hdc, hfOld);

  SetTextColor(hdc, GROUND_TEXT_COLOUR);

  if(bInvCol)
    if(sDia.fYMin > GC_SEA_LEVEL_TOLERANCE)
	  SetTextColor(hdc, INV_GROUND_TEXT_COLOUR);

  SelectObject(hdc,hfOld/* Sender->GetFont()*/);

  if(fSplitFact > 0.0)
  	sDia.rc = rct;

  hfOld = (HFONT)SelectObject(hdc,LK8InfoNormalFont/* Sender->GetFont()*/);

/*
  switch(GetMMNorthUp(getsideviewpage))
  {
	 case NORTHUP:
	 default:
		 DrawCompass( hdc,  rct, 0);
     break;

	 case TRACKUP:
		 DrawCompass( hdc,  rct, acb-90.0);
     break;
  }
*/

  DrawMultimap_SideTopSeparator(hdc,rct);

  /****************************************************************************************************
   * draw selection frame
   ****************************************************************************************************/
	if(bHeightScale)
	  DrawSelectionFrame(hdc,  rc);
#ifdef TOP_SELECTION_FRAME
	else
	  DrawSelectionFrame(hdc,  rci);
#endif


  SelectObject(hdc,hfOld/* Sender->GetFont()*/);
  zoom.SetLimitMapScale(true);
}






