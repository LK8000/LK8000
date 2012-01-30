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

#define GC_HORIZONTAL_TOLERANCE      100
#define GC_HORIZONTAL_THRESHOLD     2500
#define GC_VERTICAL_THRESHOLD        250
#define GC_VERTICAL_TOLERANCE        200
#define GC_HORIZONTAL_DELAY_FACT   250.0f
#define GC_VERTICAL_DELAY_FACT      25.0f

//#define  SHOW_ALL_AS_DISTANCES
using std::min;
using std::max;

bool   bNearAirspace_CheckAllAirspaces =false;


extern AirSpaceSonarLevelStruct sSonarLevel[10];
extern AirSpaceSideViewSTRUCT Sideview_pHandeled[GC_MAX_NO];

extern int Sideview_iNoHandeldSpaces;
extern COLORREF Sideview_TextColor;

extern long iSonarLevel;
extern bool Sonar_IsEnabled;
extern TCHAR Sideview_szNearAS[];


int CalcSonarDelay (int iNoAs, AirSpaceSideViewSTRUCT asAirspaces[]);

void Statistics::RenderNearAirspace(HDC hdc, const RECT rc)
{
  double range = 50.0*1000; // km
  double GPSlat, GPSlon, GPSalt, GPSbrg, GPSspeed, calc_average30s;
  bool GPSValid;
  double calc_terrainalt;
  double calc_altitudeagl;
 // double alt;
  int calc_circling;
  TCHAR text[80];
  TCHAR buffer[80];

  CAirspace near_airspace;
  CAirspace *found = NULL;

  DiagrammStruct sDia;
  bool bAS_Inside;
  int iAS_Bearing;
  int iAS_HorDistance;
  int iABS_AS_HorDistance=0;
  int iAS_VertDistance;
  bool   bValid;
  long wpt_brg = 0;
  POINT line[2];
  POINT TxYPt;
  POINT TxXPt;
  SIZE tsize;
  COLORREF GREEN_COL     = RGB_GREEN;
  COLORREF RED_COL       = RGB_LIGHTORANGE;
  COLORREF BLUE_COL      = RGB_BLUE;
  COLORREF LIGHTBLUE_COL = RGB_LIGHTBLUE;

  if(INVERTCOLORS)
  {
    GREEN_COL     = ChangeBrightness(GREEN_COL     , 0.6);
    RED_COL       = ChangeBrightness(RGB_RED       , 0.6);;
    BLUE_COL      = ChangeBrightness(BLUE_COL      , 0.6);;
    LIGHTBLUE_COL = ChangeBrightness(LIGHTBLUE_COL , 0.4);;
  }
  LockFlightData();
  {
    GPSlat = GPS_INFO.Latitude;
    GPSlon = GPS_INFO.Longitude;
    GPSalt = GPS_INFO.Altitude;
    GPSbrg = GPS_INFO.TrackBearing;
    GPSspeed = GPS_INFO.Speed;
    GPSValid = !GPS_INFO.NAVWarning;
    calc_circling    = CALCULATED_INFO.Circling;
    calc_terrainalt  = CALCULATED_INFO.TerrainAlt;
    calc_altitudeagl = CALCULATED_INFO.AltitudeAGL;
    calc_average30s  = CALCULATED_INFO.Average30s;
  }
  UnlockFlightData();
calc_circling = false;
  bValid = false;
  found = CAirspaceManager::Instance().GetNearestAirspaceForSideview();
  if(found != NULL) {
    near_airspace = CAirspaceManager::Instance().GetAirspaceCopy(found);
    bValid = near_airspace.GetDistanceInfo(bAS_Inside, iAS_HorDistance, iAS_Bearing, iAS_VertDistance);
  }
//if(bValid)
//  near_airspace.CalculateDistance(&iAS_sHorDistance,&iAS_VertDistance, &iAS_Bearing);
 // if(bLeft)
 // fAS_HorDistance = fabs(fAS_HorDistance);
   iABS_AS_HorDistance = abs( iAS_HorDistance);
  wpt_brg = (long)AngleLimit360(GPSbrg - iAS_Bearing + 90.0);


//  bool CAirspace::GetWarningPoint(double &longitude, double &latitude, AirspaceWarningDrawStyle_t &hdrawstyle, int &vDistance, AirspaceWarningDrawStyle_t &vdrawstyle) const
//  if(near_airspace.IsAltitudeInside(alt,calc_altitudeagl,0) && near_airspace.IsAltitudeInside(GPSlon,GPSlat))
//    bAS_Inside = true;
/*
  int iHor,iVer,  iBear;
   near_airspace.CalculateDistance(&iHor,&iVer, &iBear);

  fAS_HorDistance = (double) iHor;
  fAS_Bearing     = (double) iBear;
  iAS_VertDistance= iVer;
*/
  // Variables from ASP system here contain the following informations:
  // fAS_HorDistance - always contains horizontal distance from the asp, negative if horizontally inside (This does not mean that we're inside vertically as well!)
  // fAS_Bearing - always contains bearing to the nearest horizontal point
  // bValid - true if bAS_Inside, iAS_HorDistance, iAS_Bearing, iAS_VertDistance contains valid informations
  //          this will be true if the asp border is close enough to be tracked by the warning system
  // bAS_Inside - current position is inside in the asp, calculated by the warning system
  // iAS_HorDistance - horizontal distance to the nearest horizontal border, negative if horizontally inside, calculated by the warning system
  // iAS_Bearing - bearing to the nearest horizontal border, calculated by the warning system
  // iAS_VertDistance - vertical distance to the nearest asp border, negative if the border is above us, positive if the border below us, calculated by the warning system
  // near_airspace.WarningLevel():
  //           awNone - no warning condition exists
  //           awYellow - current position is near to a warning position
  //           awRed - current posisiton is forbidden by asp system, we are in a warning position

  /*********************************************************************
   * calc sonar delay
   *********************************************************************/

  iSonarLevel = -1;
  if(bValid)
    if(Sonar_IsEnabled)
      if(GPSValid) {
	    #if TESTBENCH
	    if(1)
	    #else
	    if(CALCULATED_INFO.FreeFlying)
	    #endif
	    {
	      if(bNearAirspace_CheckAllAirspaces)
	        iSonarLevel = CalcSonarDelay(Sideview_iNoHandeldSpaces, Sideview_pHandeled);
	      else
	      {
	    	AirSpaceSideViewSTRUCT Tmp;
	    	Tmp.psAS =  &near_airspace;
    	    iSonarLevel = CalcSonarDelay( 1, &Tmp);
	      }
	    }
      }
  /*********************************************************************
   * calc the horizontal zoom
   *********************************************************************/
  sDia.fXMin = -15000.0;
  sDia.fXMax =  15000.0;
  /* even when invalid the horizontal distance is calculated correctly */

  if(bValid)
  {
	double fDist;
	if(  calc_circling  > 0)
	  fDist = (double)(iABS_AS_HorDistance/1000+1) * 1500.0f;   // zoom fix
	else
	  fDist = (double)(iABS_AS_HorDistance) * 1.5;

	sDia.fXMin = min(-2500.0 , iABS_AS_HorDistance * 1.5 );
	sDia.fXMax = max( 2500.0 , iABS_AS_HorDistance * 1.5 );

	#ifdef NEAR_AS_ZOOM_1000M
	if(((iABS_AS_HorDistance) < 900) && (bValid)) // 1km zoom
	{

	  sDia.fXMin = min(-900.0, iABS_AS_HorDistance * 1.5 );
	  sDia.fXMax = max( 900.0, iABS_AS_HorDistance * 1.5 );

	}
	#endif
	#ifdef NEAR_AS_ZOOM_1000FT
	  if((abs(iABS_AS_HorDistance) < 333)) // 1000ft zoom
	  {
		sDia.fXMin = min(-333.0, iABS_AS_HorDistance * 1.5 );
		sDia.fXMax = max( 333.0, iABS_AS_HorDistance * 1.5 );
	  }
	#endif
  }

  /*********************************************************************
   * calc the vertical zoom
   *********************************************************************/
sDia.fYMin = max(0.0, GPSalt-2300);
sDia.fYMax = max(MAXALTTODAY, GPSalt+1000);

if(bValid)
{
 // double fTop    = near_airspace.Top()->Altitude;
  double fBottom = near_airspace.Base()->Altitude;
  sDia.fYMin = min(fBottom*0.8, sDia.fYMin );
  sDia.fYMin = max(0.0, sDia.fYMin );
  if(sDia.fYMin < 300) sDia.fYMin =0;
  sDia.fYMax = max((fBottom*1.2f), sDia.fYMax );

  if(abs(iAS_VertDistance) < 250)
  {
  //  if(ExternalTriggerCircling)
    sDia.fYMax =  ((int)((GPSalt+abs(iAS_VertDistance))/400) + 2) *400 ;
    sDia.fYMin =  ((int)((GPSalt-abs(iAS_VertDistance))/400) - 1) *400 ;
    if(sDia.fYMin-200 < 0)
      sDia.fYMin = 0;
  }

  if(abs(iAS_VertDistance) < 50)
  {
    sDia.fYMax =  ((int)((GPSalt+abs(iAS_VertDistance))/100) + 2) *100 ;
    sDia.fYMin =  ((int)((GPSalt-abs(iAS_VertDistance))/100) - 1) *100 ;
    if(sDia.fYMin-200 < 0)
      sDia.fYMin = 0;
  }
  sDia.fYMin = max((double)0.0f,(double) sDia.fYMin);
}



  range =sDia.fXMax - sDia.fXMin ;
  sDia.rc = rc;

  ResetScale();
  ScaleXFromValue(rc, sDia.fXMin);
  ScaleXFromValue(rc, sDia.fXMax);
  ScaleYFromValue(rc, sDia.fYMin);
  ScaleYFromValue(rc, sDia.fYMax);


  RenderAirspaceTerrain( hdc,  rc,  GPSlat, GPSlon, iAS_Bearing, &sDia );
#ifdef  SHOW_ALL_AS_DISTANCES
int iDiaBearing = iAS_Bearing;
#endif
  HFONT hfOld = (HFONT)SelectObject(hdc, LK8InfoNormalFont);
  if(bValid)
    _stprintf(Sideview_szNearAS,TEXT("%s"),  near_airspace.Name() );
  else
  {
	_stprintf(text,TEXT("%s"), gettext(TEXT("_@M1259_"))); 	 // LKTOKEN _@M1259_ "Too far, not calculated"
	GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
	TxYPt.x = (rc.right-rc.left-tsize.cx)/2;
	TxYPt.y = (rc.bottom-rc.top)/2;
	SetBkMode(hdc, TRANSPARENT);
	ExtTextOut(hdc,TxYPt.x, TxYPt.y-20, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
	_stprintf(Sideview_szNearAS,TEXT("%s"), text);

  }
  SelectObject(hdc, hfOld);

#ifdef SHOW_YELLO_RED_WARNING
  if (bValid)
  {
    if (near_airspace.WarningLevel() != awNone )
    {
      if (near_airspace.WarningLevel() == awYellow )
      {
        SetTextColor(hdc, RGB_YELLOW);
        _stprintf(text,TEXT("!! %s !!"), gettext(TEXT("_@M1255_"))); // _@M1255_ "YELLOW WARNING"
      }
      else
      {
        SetTextColor(hdc, RGB_RED);
        _stprintf(text,TEXT("!! %s !!"), gettext(TEXT("_@M1256_"))); 	// _@M1293_ "RED WARNING"
      }
      static int callcnt = 0;
      if(callcnt++%2 == 0)
        SetTextColor(hdc, RGB_BLACK);
      GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
      TxYPt.x = (rc.right-rc.left-tsize.cx)/2;
      ExtTextOut(hdc,  TxYPt.x , NIBLSCALE(25), ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    }
  }
#endif
  double xtick = 1.0;
  if (range>0.01*1000.0) xtick = 0.01;
  if (range>0.1*1000.0) xtick = 0.1;
  if (range>1.0*1000.0) xtick = 1.0;
  if (range>10.0*1000.0) xtick = 5.0;
  if (range>50.0*1000.0) xtick = 10.0;
  if (range>100.0*1000.0) xtick = 20.0;
  if (range>200.0*1000.0) xtick = 25.0;
  if (range>250.0*1000.0) xtick = 50.0;
  if (range>500.0*1000.0) xtick = 100.0;

  if(INVERTCOLORS)
  {
    SelectObject(hdc, GetStockObject(BLACK_PEN));
    SelectObject(hdc, GetStockObject(BLACK_BRUSH));
  }
  else
  {
    SelectObject(hdc, GetStockObject(WHITE_PEN));
    SelectObject(hdc, GetStockObject(WHITE_BRUSH));
  }

  if(sDia.fYMin == 0)
    SetTextColor(hdc, GROUND_TEXT_COLOUR);
  else
	SetTextColor(hdc, INV_GROUND_TEXT_COLOUR);
  DrawXGrid(hdc, rc, xtick/DISTANCEMODIFY, 0, STYLE_THINDASHPAPER, xtick, true);
  SetTextColor(hdc, Sideview_TextColor);
  double fScale = 1000;
  if((sDia.fYMax-sDia.fYMin) <= 1000)
	fScale = 200.0f;
  else
	fScale = 1000.0f;
  if (Units::GetUserInvAltitudeUnit() == unFeet)
	  fScale /= 2;

  DrawYGrid(hdc, rc, fScale/ALTITUDEMODIFY, 0, STYLE_THINDASHPAPER, fScale, true);

  if(!INVERTCOLORS)
    SetBkMode(hdc, OPAQUE);


  if(calc_altitudeagl - sDia.fYMin  > 500)
  {
    SetTextColor(hdc, LIGHTBLUE_COL);
    Units::FormatUserAltitude(calc_altitudeagl, buffer, 7);
    _tcsncpy(text, gettext(TEXT("_@M1742_")), sizeof(text)/sizeof(text[0])); // AGL:
    _tcscat(text,buffer);
    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
    TxYPt.x = CalcDistanceCoordinat(0,  rc, &sDia)- tsize.cx/2;
    TxYPt.y  = CalcHeightCoordinat(  (calc_terrainalt + calc_altitudeagl )*0.5,   rc, &sDia );
    if((tsize.cy) < ( CalcHeightCoordinat(  calc_terrainalt, rc, &sDia )- TxYPt.y )) {
      ExtTextOut(hdc,  TxYPt.x+IBLSCALE(1),  TxYPt.y , ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    }
  }

  SetBkMode(hdc, TRANSPARENT);

  // Print current Elevation
  SetTextColor(hdc, RGB_BLACK);
  int x,y;
  if((calc_terrainalt-  sDia.fYMin)  > 0)
  {
	Units::FormatUserAltitude(calc_terrainalt, buffer, 7);
    _tcsncpy(text, gettext(TEXT("_@M1743_")), sizeof(text)/sizeof(text[0]));   // ELV:
    _tcscat(text,buffer);
    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
    x = CalcDistanceCoordinat(0,  rc, &sDia) - tsize.cx/2;
    y = CalcHeightCoordinat( calc_terrainalt, rc, &sDia  );
    if ((ELV_FACT*tsize.cy) < abs(rc.bottom - y)) {
      ExtTextOut(hdc, x, rc.bottom -(int)(ELV_FACT * tsize.cy) , ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    }
  }



#ifdef SHOW_ALL_AS_DISTANCES
int i;
for( i =  0 ; i < Sideview_iNoHandeldSpaces ; i++)
{
	bValid = false;
  found =  Sideview_pHandeled[i].psAS;
  if(found != NULL)
  {
    near_airspace = CAirspaceManager::Instance().GetAirspaceCopy(found);
    bValid = near_airspace.GetDistanceInfo(bAS_Inside, iAS_HorDistance, iAS_Bearing, iAS_VertDistance);
    iABS_AS_HorDistance =  abs(iAS_HorDistance);
    if(AngleLimit360(iAS_Bearing - iDiaBearing) < -90)
    	iABS_AS_HorDistance = -iABS_AS_HorDistance;
    if(AngleLimit360(iAS_Bearing - iDiaBearing) > 90)
    	iABS_AS_HorDistance = -iABS_AS_HorDistance;
  }
#endif
  if (bValid)
  {
	SetTextColor(hdc, Sideview_TextColor);
//	  SetTextColor(hdc, MapWindow::GetAirspaceColourByClass(Sideview_pHandeled[i].iType));


	if(!INVERTCOLORS)
	  SetBkMode(hdc, OPAQUE);
	HFONT hfOldU = (HFONT)SelectObject(hdc, LK8InfoNormalFont);
    // horizontal distance
    line[0].x = CalcDistanceCoordinat(0,  rc, &sDia);
    line[0].y = CalcHeightCoordinat(  GPSalt,   rc, &sDia );
    line[1].x = CalcDistanceCoordinat(iABS_AS_HorDistance,  rc, &sDia);
    line[1].y = line[0].y;
    StyleLine(hdc, line[0], line[1], STYLE_WHITETHICK, rc);

    bool bLeft = false;
    if( line[0].x < line[1].x)
      bLeft = false;
    else
      bLeft = true;

    Units::FormatUserDistance(iABS_AS_HorDistance, buffer, 7);
    _tcsncpy(text, TEXT(" "), sizeof(text)/sizeof(text[0]));
    _tcscat(text,buffer);
    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);

    if((GPSalt- sDia.fYMin /*-calc_terrainalt */) < 300)
      TxXPt.y = CalcHeightCoordinat(  GPSalt,   rc, &sDia ) -  tsize.cy;
    else
      TxXPt.y = CalcHeightCoordinat(  GPSalt,   rc, &sDia ) +  NIBLSCALE(3);


    if(tsize.cx > (line[1].x - line[0].x) )
      TxXPt.x = CalcDistanceCoordinat( iABS_AS_HorDistance , rc, &sDia) -tsize.cx-  NIBLSCALE(3);
    else
      TxXPt.x = CalcDistanceCoordinat( iABS_AS_HorDistance / 2.0, rc, &sDia) -tsize.cx/2;
    ExtTextOut(hdc,  TxXPt.x,  TxXPt.y , ETO_OPAQUE, NULL, text, _tcslen(text), NULL);


    // vertical distance
    line[0].x = CalcDistanceCoordinat(iABS_AS_HorDistance ,  rc, &sDia);
    line[0].y = CalcHeightCoordinat( GPSalt, rc, &sDia );
    line[1].x = line[0].x;
    line[1].y = CalcHeightCoordinat( GPSalt - (double)iAS_VertDistance, rc, &sDia );
    StyleLine(hdc, line[0], line[1], STYLE_WHITETHICK, rc);

    Units::FormatUserAltitude( (double)abs(iAS_VertDistance), buffer, 7);
    _tcsncpy(text, TEXT(" "), sizeof(text)/sizeof(text[0]));
    _tcscat(text,buffer);
    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);

    if ( bLeft )
      TxYPt.x = CalcDistanceCoordinat(iABS_AS_HorDistance,  rc, &sDia)- tsize.cx - NIBLSCALE(3);
    else
      TxYPt.x = CalcDistanceCoordinat(iABS_AS_HorDistance,  rc, &sDia)+ NIBLSCALE(5);
    if( abs( line[0].y -  line[1].y) > tsize.cy)
      TxYPt.y = CalcHeightCoordinat( GPSalt - (double)iAS_VertDistance/2.0, rc, &sDia) -tsize.cy/2 ;
    else
      TxYPt.y = min( line[0].y ,  line[1].y) - tsize.cy ;
    ExtTextOut(hdc,  TxYPt.x,  TxYPt.y , ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
	SelectObject(hdc, hfOldU);
  }
#ifdef SHOW_ALL_AS_DISTANCES
}
#endif

  RenderPlaneSideview( hdc, rc,0 , GPSalt,wpt_brg, &sDia );

  SetTextColor(hdc, Sideview_TextColor);
  if(!INVERTCOLORS)
    SetBkMode(hdc, OPAQUE);
  HFONT hfOld2 = (HFONT)SelectObject(hdc, LK8InfoNormalFont);
  DrawNorthArrow     ( hdc, GPSbrg          , rc.right - NIBLSCALE(13),  rc.top   + NIBLSCALE(13));
  DrawTelescope      ( hdc, iAS_Bearing-90.0, rc.right - NIBLSCALE(13),  rc.top   + NIBLSCALE(38));
  SelectObject(hdc, hfOld2);
  SetBkMode(hdc, TRANSPARENT);



  SelectObject(hdc, hfOld);
  if(sDia.fYMin == 0)
    SetTextColor(hdc, GROUND_TEXT_COLOUR);
  else
	SetTextColor(hdc, INV_GROUND_TEXT_COLOUR);
  DrawXLabel(hdc, rc, TEXT("D"));
  SetTextColor(hdc, Sideview_TextColor);
  DrawYLabel(hdc, rc, TEXT("h"));

  RenderBearingDiff  ( hdc, rc   , wpt_brg,  &sDia );
}




/*********************************************************************
 * calc the sonar delay time
 *********************************************************************/
int CalcSonarDelay (int iNoAs, AirSpaceSideViewSTRUCT asAirspaces[])
{
int iAS_HorDist;
int iAS_VertDist;
int iAS_Bearing;
int iAltitude;
int iAltitudeAGL;
int i;
bool bAS_Inside = false;
bool bOK = false;
int iTreadLevel;
CAirspace SelectedAS;
CAirspace *Sel_AS_Ptr = NULL;

int	iH_Level = 1000;
int	iV_Level = 1000;
int	divider=1;

LockFlightData();
{
  iAltitudeAGL = (int)CALCULATED_INFO.AltitudeAGL;
  iAltitude = (int)CALCULATED_INFO.NavAltitude;
}
UnlockFlightData();

	if (ISPARAGLIDER) divider=2;

	for( i =  0 ; i < iNoAs ; i++)
	{

	  Sel_AS_Ptr =	asAirspaces[i].psAS;
	  if(Sel_AS_Ptr != NULL)
	  {
		SelectedAS = CAirspaceManager::Instance().GetAirspaceCopy( Sel_AS_Ptr );
		bOK = SelectedAS.GetDistanceInfo(bAS_Inside, iAS_HorDist, iAS_Bearing, iAS_VertDist);

		if(bOK)
		{
		  int iTmpV_Level = -1;
		  if((iAS_HorDist) < GC_HORIZONTAL_TOLERANCE)                          /* horizontal near or inside */
		  {
			int iTmp =	abs(iAS_VertDist);

			if(iTmp < sSonarLevel[9].iDistantrance/divider)
			  iTmpV_Level = 9;
			if(iTmp < sSonarLevel[8].iDistantrance/divider)
			  iTmpV_Level = 8;
			if(iTmp < sSonarLevel[7].iDistantrance/divider)
			  iTmpV_Level = 7;
			if(iTmp < sSonarLevel[6].iDistantrance/divider)
			  iTmpV_Level = 6;
			if(iTmp < sSonarLevel[5].iDistantrance/divider)
			  iTmpV_Level = 5;
		  }
		  if(iTmpV_Level != -1)
            if(iTmpV_Level < iV_Level )
        	  iV_Level = iTmpV_Level;


		  int iTmpH_Level = -1;
		  if(SelectedAS.IsAltitudeInside(iAltitude,iAltitudeAGL,GC_VERTICAL_TOLERANCE))  /* vertically near or inside ? */
		  {
			int iTmp =	abs(iAS_HorDist);
            if(iTmp < sSonarLevel[4].iDistantrance/divider)
              iTmpH_Level = 4;
            if(iTmp < sSonarLevel[3].iDistantrance/divider)
              iTmpH_Level = 3;
            if(iTmp < sSonarLevel[2].iDistantrance/divider)
              iTmpH_Level = 2;
            if(iTmp < sSonarLevel[1].iDistantrance/divider)
              iTmpH_Level = 1;
            if(iTmp < sSonarLevel[0].iDistantrance/divider)
              iTmpH_Level = 0;
		  }
		  if(iTmpH_Level != -1)
            if(iTmpH_Level < iH_Level )
        	  iH_Level = iTmpH_Level;

		  if(SelectedAS.IsAltitudeInside(iAltitude,iAltitudeAGL,0))  /* vertically inside ? */
			if(iAS_HorDist < 0)                                             /* complete inside, do no beep! */
			{
			  iV_Level = -1;   /* red allert */
			  iH_Level = -1;   /* red allert */
			}
		  }
	  }
	}

    iTreadLevel = iV_Level;
	if(iH_Level > -1)
	  if((iH_Level+5) <= iV_Level)
	    iTreadLevel = iH_Level;

    // StartupStore(_T("... HDist=%d Vdist=%d SonarLevel=%d \n"), iAS_HorDist, iAS_VertDist,iTreadLevel);
return iTreadLevel;
}
