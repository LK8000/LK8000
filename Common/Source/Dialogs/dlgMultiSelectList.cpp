/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceSelectList.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include <aygshell.h>
#include "dlgTools.h"
#include "InfoBoxLayout.h"
#include "MapWindow.h"
#include "Dialogs.h"
#include "FlarmIdFile.h"
#include "InputEvents.h"
#include "Units.h"
#include "Multimap.h"
#define LINE_HEIGT 50

extern  int FindNearestWayPoint(double X, double Y, double MaxRange,               bool exhaustive);
extern void dlgOracleShowModal(void);
extern void LK_wsplitpath(const WCHAR* path, WCHAR* drv, WCHAR* dir, WCHAR* name, WCHAR* ext);
extern void LatLonToUtmWGS84 (int& utmXZone, char& utmYZone, double& easting, double& northing, double lat, double lon);
extern void dlgTeamCodeShowModal(void);
extern FlarmIdFile *file;
#define  LEAVE_AFTER_ENTER
#define MAX_LIST_ITEMS 50
ListElement* pResult= NULL;

ListElement Elements[MAX_LIST_ITEMS];

int iNO_ELEMENTS=0;
static WndForm *wf=NULL;
static WndListFrame *wMultiSelectListList=NULL;
static WndOwnerDrawFrame *wMultiSelectListListEntry = NULL;

static int ItemIndex = -1;

static int NoAirfields =0;
static int NoOutlands  =0;
static int NoWaypoints =0;
static int NoTaskPoints=0;
static int NoFarm      =0;
static int NoAirspace  =0;
/*
#define MAX_AIRFILEDS 4
#define MAX_OUTLAND   4
#define MAX_WAYPOINTS 3
#define MAX_FLARM     8
#define MAX_TASK      3
#define MAX_AIRSPACES 10
*/


#define MAX_AIRFILEDS 3
#define MAX_OUTLAND   2
#define MAX_WAYPOINTS 3
#define MAX_FLARM     5
#define MAX_TASK      3
#define MAX_AIRSPACES 10


static void UpdateList(void){
	wMultiSelectListList->ResetList();
	wMultiSelectListList->Redraw();

	wMultiSelectListListEntry->SetFocused(true,NULL);
}

static int DrawListIndex=0;


#ifdef AUTOUPDATE_MS
static int OnTimer(WindowControl * Sender){
  (void)Sender;

  // Timer events comes at 500ms, we need every second
  static bool timer_divider = false;
  timer_divider = !timer_divider;
  if (timer_divider) return 0;

	wMultiSelectListList->Redraw();
  return 0;
}
#endif

static void OnUpClicked(WindowControl * Sender)
{
  if(ItemIndex > 0)
  {
	ItemIndex--;
  }
  else
  {
	ItemIndex = (iNO_ELEMENTS-1);
  }
  wMultiSelectListList->SetItemIndexPos(ItemIndex);
  wMultiSelectListList->Redraw();
  wMultiSelectListListEntry->SetFocused(true,NULL);
}

static void OnDownClicked(WindowControl * Sender)
{
  (void)Sender;

  if(ItemIndex < (iNO_ELEMENTS-1))
  {
	ItemIndex++;
  }
  else
  {
	ItemIndex =0;
  }
  wMultiSelectListList->SetItemIndexPos(ItemIndex);
  wMultiSelectListList->Redraw();
  wMultiSelectListListEntry->SetFocused(true,NULL);
}

void dlgAddMultiSelectListDetailsDialog(int Index)
{
int iLastTaskPoint=0;
while( ValidTaskPoint(iLastTaskPoint))
  iLastTaskPoint++;
iLastTaskPoint--;
  if((Index >= 0) && (Index < iNO_ELEMENTS))
  {
    switch(Elements[Index].type)
    {

#ifdef BUTTONS_MS
      case  IM_TEAM:
        dlgTeamCodeShowModal();
      break;
      case  IM_ORACLE:
    	dlgOracleShowModal();
      break;
      case IM_OWN_POS:
    	dlgBasicSettingsShowModal();
      break;
#endif // BUTTONS_MS

      case IM_AIRSPACE:
    	LKASSERT(Elements[Index].ptr);
	    dlgAirspaceDetails((CAirspace*)Elements[Index].ptr);
      break;
      case IM_WAYPOINT:
	    LKASSERT(Elements[Index].iIdx < (int)NumberOfWayPoints);
      SelectedWaypoint	=Elements[Index].iIdx;
	    PopupWaypointDetails();
	  break;
      case IM_TASK_PT:
	    LKASSERT(Elements[Index].iIdx<=MAXTASKPOINTS);
	    RealActiveWaypoint =-1;
	    if(Elements[Index].iIdx == 0)
	      dlgTaskWaypointShowModal(Elements[Index].iIdx,0, false, true);
	    else
	    {
	      if(Elements[Index].iIdx == iLastTaskPoint)
		     dlgTaskWaypointShowModal(Elements[Index].iIdx,2, false, true);
	      else
//#ifdef MS_TARGET_DIALOG
	    	if ((AATEnabled ) && (CALCULATED_INFO.Flying ) && (!IsMultiMapNoMain()))
	    	{
	    		wf->SetModalResult(mrOK);
	    		wf->Close();
	    		dlgTarget(Elements[Index].iIdx);
	    	}
	    	else
//#endif
	    	{
			  dlgTaskWaypointShowModal(Elements[Index].iIdx,1, false, true);
	    	}
	      
	    }
      break;
      case IM_FLARM:
	    LKASSERT(Elements[Index].iIdx<FLARM_MAX_TRAFFIC);
	    dlgLKTrafficDetails(Elements[Index].iIdx);
      break;
    }
  }
}
int dlgGetNoElements(void)
{
  return iNO_ELEMENTS;
}

void dlgAddMultiSelectListItem(long* pNew ,int Idx, char type, double Distance){

#ifdef BUTTONS_MS
	if(type == IM_TEAM )
	{
#ifndef TEAM_CODE_MS
	  return;
#endif
	  if (TeamCodeRefWaypoint  < -1)
		 return;

	  if(  _tcslen(CALCULATED_INFO.OwnTeamCode) < 3)
		return;
	}

	if(type == IM_OWN_POS )
	{
#ifndef OWN_POS_MS
	  return;
#endif
	  Idx = FindNearestWayPoint(GPS_INFO.Longitude,  GPS_INFO.Latitude,  100000.0, true); // big range limit
	}
	if(type == IM_ORACLE )
	{
#ifndef ORACLE_MS
	  return;
#endif
	  Idx = FindNearestWayPoint(GPS_INFO.Longitude,  GPS_INFO.Latitude,  100000.0, true); // big range limit
	}
#endif	// BUTTONS_MS

	if(type == IM_TASK_PT )
	{
#ifndef GOTO_AS_SIMPLETASK
	  int iLastTaskPoint=0;
	  while( ValidTaskPoint(iLastTaskPoint))
	    iLastTaskPoint++;
	  if (iLastTaskPoint < 2)
		return;
#endif
	  if(ValidTaskPoint(PanTaskEdit))
	    return;
	  if( Task[Idx].Index	== RESWP_PANPOS)
	    return;
	}

	if(type == IM_WAYPOINT )
	  if(Idx == RESWP_PANPOS)
	    return;

	for (int i=0 ; i < iNO_ELEMENTS; i++)
	{
	  LKASSERT(i < MAX_LIST_ITEMS);
	  if(pNew != NULL)
	    if( Elements[i].ptr  == pNew)
		 return;

	  if(type != IM_AIRSPACE )
	      if(type == Elements[i].type )
		    if(Idx == Elements[i].iIdx )
		      return;

	}

    bool full = false;
	if(iNO_ELEMENTS < MAX_LIST_ITEMS-1)
	{
		switch(type)
		{
		  case IM_AIRSPACE:	if(NoAirspace   < MAX_AIRSPACES) NoAirspace++; 	 else  full=true; break;
		  case IM_FLARM:	if(NoFarm       < MAX_FLARM    ) NoFarm++;       else  full=true; break;
		  case IM_TASK_PT:  if(NoTaskPoints < MAX_TASK     ) NoTaskPoints++; else  full=true; break;
		  case IM_WAYPOINT:
			  if (WayPointCalc[Idx].IsLandable ){
				if( WayPointCalc[Idx].IsAirport) {
				  if(NoAirfields     < MAX_AIRFILEDS    )	NoAirfields++;       else  full=true;
				} else {
				  if(NoOutlands     < MAX_OUTLAND       )  NoOutlands++;       else  full=true;
				}
			  } else {
				if(NoWaypoints     < MAX_WAYPOINTS    )	NoWaypoints++;       else  full=true;
			  }
		  break;
		}

		if(full)
		{
		  for(int i=0; i < iNO_ELEMENTS; i++)
		  {
			if(Elements[i].type == type)
			  if(Distance < Elements[i].Dist )
			  {
				Elements[i].ptr  = pNew;
				Elements[i].type = type;
				Elements[i].iIdx = Idx;
				Elements[i].Dist = Distance;
			    return;
			  }
		  }
		}
		else
		{
          int Pos = 0;
		  while ((Elements[Pos].Dist <= Distance) && (Pos < iNO_ELEMENTS))
			Pos++;
		  LKASSERT(Pos          < MAX_LIST_ITEMS);
          LKASSERT(iNO_ELEMENTS < MAX_LIST_ITEMS);
		  for (int i =  iNO_ELEMENTS; i > Pos; i--)
		  {
			LKASSERT(i >=0);
			Elements[i].ptr  = Elements[i-1].ptr;
			Elements[i].type = Elements[i-1].type;
			Elements[i].iIdx = Elements[i-1].iIdx;
			Elements[i].Dist = Elements[i-1].Dist;
		  }
		  Elements[Pos].ptr  = pNew;
		  Elements[Pos].type = type;
		  Elements[Pos].iIdx = Idx;
		  Elements[Pos].Dist = Distance;
		  iNO_ELEMENTS++;
		}
	}
}


static void OnMultiSelectListPaintListItem(WindowControl * Sender, HDC hDC){
  (void)Sender;
#ifdef MULTISEL_PICTORIALS
  #define PICTO_WIDTH 50
#else
  #define PICTO_WIDTH 0
#endif
	if ((DrawListIndex < iNO_ELEMENTS) &&(DrawListIndex>=0))
	{
	  int j;
	  static CAirspace airspace_copy;
	  int i = DrawListIndex;
	  LKASSERT(i < MAX_LIST_ITEMS);
	  RECT rc = {0*ScreenScale,  0*ScreenScale, PICTO_WIDTH*ScreenScale,   34*ScreenScale};

	  CAirspace* pAS = NULL;
	  FLARM_TRAFFIC* pFlarm=NULL;
	  int HorDist,Bearing, VertDist;
	  double Distance;
	  unsigned int idx=0;
#ifdef BUTTONS_MS
      SIZE tsize;
      HFONT oldFont;
      int nearest_waypoint = 0;
#endif
	  TCHAR text1[180] ={TEXT("empty")};
	  TCHAR text2[180] ={TEXT("empty")};
	  TCHAR Comment[80]={TEXT("")};
	  TCHAR Comment1[80]={TEXT("")};
	  SetBkColor  (hDC, RGB(0xFF, 0xFF, 0xFF));
	  LKASSERT(i < MAX_LIST_ITEMS);


	  switch(Elements[i].type )
	  {

#ifdef BUTTONS_MS
	    case IM_TEAM:
	    	_stprintf(text1,_T("%s:"),gettext(_T("_@M700_"))); //_@M700_ "Team code"
		    _stprintf(text2,_T("%s"), CALCULATED_INFO.OwnTeamCode );
	    //	MapWindow::DrawAircraft(hDC, (POINT) { (rc.right-rc.left)/2,(rc.bottom-rc.top)/2} );

	        SetBkMode(hDC,TRANSPARENT);
	        _stprintf(Comment,_T("T"));
	        oldFont = (HFONT)SelectObject(hDC, LK8PanelBigFont);
	        SetTextColor(hDC, RGB_DARKGREEN);
	        GetTextExtentPoint(hDC, Comment, _tcslen(Comment), &tsize);
	  	    ExtTextOut(hDC,  (rc.right-rc.left-tsize.cx)/2,(rc.bottom-rc.top-tsize.cy)/2, ETO_OPAQUE, NULL, Comment, 1, NULL);
	  	    SelectObject(hDC, oldFont);
		break;
	    /************************************************************************************************
	     * IM_ORACLE
	     ************************************************************************************************/
	    case IM_ORACLE:
	    	_stprintf(text1,_T("%s"),gettext(_T("_@M2058_"))); //_@M2058_ "Oracle"
	    	if(nearest_waypoint >= 0)
	    	  _stprintf(text2,_T("%s: %s"),    gettext(_T("_@M456_")), WayPointList[Elements[i].iIdx].Name);// _@M456_ "Near"
	    	else
		       _stprintf(text2,_T("%s"),   gettext(_T("_@M1690_"))); //_@M1690_ "THE LK8000 ORACLE"
	    //	MapWindow::DrawAircraft(hDC, (POINT) { (rc.right-rc.left)/2,(rc.bottom-rc.top)/2} );

	        SetBkMode(hDC,TRANSPARENT);
	        _stprintf(Comment,_T("?"));
	        oldFont = (HFONT)SelectObject(hDC, LK8PanelBigFont);
	        SetTextColor(hDC, RGB_DARKBLUE);
	        GetTextExtentPoint(hDC, Comment, _tcslen(Comment), &tsize);
	  	    ExtTextOut(hDC,  (rc.right-rc.left-tsize.cx)/2,(rc.bottom-rc.top-tsize.cy)/2, ETO_OPAQUE, NULL, Comment, 1, NULL);
	  	    SelectObject(hDC, oldFont);
		break;
	    /************************************************************************************************
	     * IM_OWN_POS
	     ************************************************************************************************/
	    case IM_OWN_POS:

			LK_wsplitpath(szPolarFile, (WCHAR*) NULL, (WCHAR*) NULL, Comment, (WCHAR*) NULL);
			_stprintf(text1,_T("%s [%s]"), AircraftRego_Config,Comment);
		   if (ISPARAGLIDER || ISCAR)
		   {
			 int utmzone; char utmchar;
			 double easting, northing;
			 LatLonToUtmWGS84 ( utmzone, utmchar, easting, northing, GPS_INFO.Latitude, GPS_INFO.Longitude );
			 _stprintf(text2,_T("UTM %d%c  %.0f  %.0f"), utmzone, utmchar, easting, northing);
		   }
		   else
		   {
			   /*
			 _stprintf(text2,_T("%s:%2.1f %s:%3.1f%% %s:%3.1f"),gettext(_T("_@M1022_")),GPS_INFO.MacReady, // _@M1022_ "MCready"
					                                            gettext(_T("_@M125_")) ,GPS_INFO.Ballast,  // _@M125_ "Ballast "
					                                            gettext(_T("_@M253_")) ,GPS_INFO.Bugs);    // _@M253_ "Efficiency "

			 _stprintf(text2,_T("%s:%4.1f%s %4.1f%s"),gettext(_T("Alt")),GPS_INFO.Altitude*ALTITUDEMODIFY, // _@M1022_ "MCready"
				                                                  GPS_INFO.Altitude,  // _@M125_ "Ballast "
					                                            gettext(_T("_@M253_")) ,GPS_INFO.Bugs);    // _@M253_ "Efficiency "*/

			 LockFlightData();
			 Units::CoordinateToString( GPS_INFO.Latitude,GPS_INFO.Longitude, Comment, sizeof(text2)-1);
			 UnlockFlightData();
			 _stprintf(text2,TEXT("%s %6.0f%s"),Comment, GPS_INFO.Altitude*TOFEET,Units::GetUnitName(unFeet));
		   }

		   MapWindow::DrawAircraft(hDC, (POINT) { (rc.right-rc.left)/2,(rc.bottom-rc.top)/2} );
	    break;
#endif // BUTTONS_MS enabled

	    /************************************************************************************************
	     * IM_AIRSPACE
	     ************************************************************************************************/
		case IM_AIRSPACE:
	      /*******************************
	       * @Paolo
	       * here I use a local copy of the airspace
	        */
		  airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy((CAirspace*)Elements[i].ptr);
		  pAS = &airspace_copy;
		//  pAS =  (CAirspace*) Elements[i].ptr;
		  LKASSERT(pAS);

		  if( _tcsnicmp(  pAS->Name(), pAS->TypeName() ,_tcslen(pAS->TypeName())) == 0)  // airspace type already in name?
			_stprintf(text1,TEXT("%s"),pAS->Name());                                     // yes, take name only
		  else
		    _stprintf(text1,TEXT("%s %s"),pAS->TypeName()   // fixed strings max. 20
				                         ,pAS->Name());     // NAME_SIZE          30   => max. 30 char

		  CAirspaceManager::Instance().GetSimpleAirspaceAltText(Comment, sizeof(Comment)/sizeof(Comment[0]), airspace_copy.Top());
		  CAirspaceManager::Instance().GetSimpleAirspaceAltText(Comment1, sizeof(Comment1)/sizeof(Comment1[0]), airspace_copy.Base());

		  ((CAirspace*) Elements[i].ptr)->CalculateDistance(&HorDist, &Bearing, &VertDist);
		  _stprintf(text2,TEXT("%3.1f%s (%s - %s)"), (double)HorDist*DISTANCEMODIFY ,Units::GetDistanceName(), Comment1, Comment);  //8 + 8+3   21

          /*************************************************************
           * @Paolo
           * for drawing the airspace pictorial, we need the original data.
           * seems that the copy does not contain geo data
           * Do we really need to work with the copy?
           * works fine with the origin airspace
           ************************************************************/
		  pAS = (CAirspace*) Elements[i].ptr;
		  LKASSERT(pAS);
		  pAS->DrawPicto(hDC, rc, true);
		break;
	    /************************************************************************************************
	     * IM_WAYPOINT
	     ************************************************************************************************/
		case IM_TASK_PT:
		case IM_WAYPOINT:
		  idx= Elements[i].iIdx;
		  if(Elements[i].type ==  IM_TASK_PT)
		  {
			LKASSERT(Elements[i].iIdx <= MAXTASKPOINTS);
			idx= Task[Elements[i].iIdx].Index;
		  }
		  if(idx >= NumberOfWayPoints) idx = NumberOfWayPoints;
		  LKASSERT(idx <= NumberOfWayPoints);

		  if(WayPointList[idx].Comment != NULL)  {
		    LK_tcsncpy(Comment,WayPointList[idx].Comment,30);
		  } else {
			_stprintf(Comment,TEXT(""));
		  }
          DistanceBearing( GPS_INFO.Latitude,GPS_INFO.Longitude, WayPointList[idx].Latitude,  WayPointList[idx].Longitude, &Distance, NULL);
          if( Elements[i].type != IM_TASK_PT)
          {
		    if (WayPointCalc[idx].IsLandable )
		    {

			MapWindow::DrawRunway(hDC,&WayPointList[idx],  rc, 4000 , true);

			if( WayPointCalc[idx].IsAirport)
			{
			  for(j=1; j < (CUPSIZE_FREQ); j++)                              // remove spaces from frequency
				if(WayPointList[idx].Freq[CUPSIZE_FREQ-j] == ' ')
				  WayPointList[idx].Freq[CUPSIZE_FREQ-j] = '\0';

			  if(_tcslen(WayPointList[idx].Freq) > 2)
			    _stprintf(text1,TEXT("%s %s MHz"), WayPointList[idx].Name    // NAME_SIZE        30
					                               , WayPointList[idx].Freq);// CUPSIZE_FREQ     15  => max 45 char
			  else
				_stprintf(text1,TEXT("%s"), WayPointList[idx].Name);         // NAME_SIZE        30
			}
			else
			{
			  if(WayPointList[idx].Comment != NULL)
				_stprintf(text1,TEXT("%s %s"), WayPointList[idx].Name        // NAME_SIZE        30
						                     , Comment);                     // max .30          15  => max 45 char
			  else
				_stprintf(text1,TEXT("%s"),WayPointList[idx].Name );         // NAME_SIZE        30
			}
            if((WayPointList[idx].RunwayLen >= 10) || (WayPointList[idx].RunwayDir > 0))
            {
			_stprintf(text2,TEXT("%3.1f%s (%i%s  %02i/%02i  %i%s)"), Distance*DISTANCEMODIFY
					                                               , Units::GetDistanceName()
				                                                   , (int)(WayPointList[idx].Altitude * ALTITUDEMODIFY)
														           , Units::GetAltitudeName()
														           , (int)(WayPointList[idx].RunwayDir/10.0+0.5)
														           , (int)(AngleLimit360(WayPointList[idx].RunwayDir+180.0)/10.0+0.5)
														           , (int)((double)WayPointList[idx].RunwayLen* ALTITUDEMODIFY)
														           , Units::GetAltitudeName());
            }
            else
            {
    			_stprintf(text2,TEXT("%3.1f%s (%i%s) "), Distance*DISTANCEMODIFY  ,Units::GetDistanceName()
                                                       , (int)(WayPointList[idx].Altitude * ALTITUDEMODIFY)
		                                               , Units::GetAltitudeName());
            }
		    }
		  else
		  {
			    MapWindow::DrawWaypointPicto(hDC,  rc, &WayPointList[idx]);
			  _stprintf(text1,TEXT("%s %s"), WayPointList[idx].Name             // NAME_SIZE      30
										   , Comment  );                        // max 30         30 => max 60 char

			  _stprintf(text2,TEXT("%3.1f%s (%i%s)"), Distance*DISTANCEMODIFY
                                                    , Units::GetDistanceName()
				                                    , (int)(WayPointList[idx].Altitude * ALTITUDEMODIFY)
					                                , Units::GetAltitudeName());
		  }
          }
		  else
          {
			LockTaskData(); // protect from external task changes
        	int iTaskIdx = Elements[i].iIdx;
  			MapWindow::DrawTaskPicto(hDC, iTaskIdx,  rc, 3000);
        	int iLastTaskPoint=0;

        	while( ValidTaskPoint(iLastTaskPoint))
        	  iLastTaskPoint++;
        	iLastTaskPoint--;

            if(iTaskIdx ==0)
            {
			  _stprintf(text1,TEXT("%s: (%s)"),gettext(_T("_@M2301_")), WayPointList[idx].Name   );  // _@M2301_  "S"    # S = Start Task point
			  _stprintf(text2,TEXT("Radius %3.1f%s (%i%s)"), StartRadius*DISTANCEMODIFY
			                                               , Units::GetDistanceName()
							                               , (int)(WayPointList[idx].Altitude * ALTITUDEMODIFY)
								                           , Units::GetAltitudeName());
            }
            else
              if(iTaskIdx == iLastTaskPoint)
              {
			     _stprintf(text1,TEXT("%s: (%s) "),gettext(_T("_@M2303_")), WayPointList[idx].Name   );       //	_@M2303_  "F"                 // max 30         30 => max 60 char
				 _stprintf(text2,TEXT("Radius %3.1f%s (%i%s)"), FinishRadius*DISTANCEMODIFY
			                                                  , Units::GetDistanceName()
							                                  , (int)(WayPointList[idx].Altitude * ALTITUDEMODIFY)
								                              , Units::GetAltitudeName());
              }
              else
              {
		        _stprintf(text1,TEXT("%s%i: (%s) "),gettext(_T("_@M2302_")),iTaskIdx  , WayPointList[idx].Name   );    //   _@M2302_  "T"    # F = Finish point            // max 30         30 => max 60 char
	        	double SecRadius =0;

	        	SecRadius =  SectorRadius;
			    if (AATEnabled)
			    {
			      if(Task[iTaskIdx].AATType	== SECTOR)
	        	    SecRadius = Task[iTaskIdx].AATSectorRadius;
			      else
			        SecRadius = Task[iTaskIdx].AATCircleRadius;
			    }
				_stprintf(text2,TEXT("Radius %3.1f%s (%i%s)"), SecRadius*DISTANCEMODIFY
		                                              , Units::GetDistanceName()
						                              , (int)(WayPointList[idx].Altitude * ALTITUDEMODIFY)
							                          , Units::GetAltitudeName());
              }


			UnlockTaskData(); // protect from external task changes
		  }
		break;
	    /************************************************************************************************
	     * IM_FLARM
	     ************************************************************************************************/
		case IM_FLARM:
		  LKASSERT(Elements[i].ptr);
		  pFlarm= ( FLARM_TRAFFIC* )Elements[i].ptr;
		  MapWindow::DrawFlarmPicto(hDC, rc, pFlarm);

          if(_tcscmp(pFlarm->Name,_T("?")) ==0)
			_stprintf(text1,TEXT("%X"),pFlarm->ID);
		  else
		    _stprintf(text1,TEXT("[%s] %X"),pFlarm->Name, pFlarm->ID);

		  FlarmId* flarmId = file->GetFlarmIdItem(pFlarm->ID);
		  if(flarmId != NULL)
		  {

			for(j=1; j < (FLARMID_SIZE_NAME-1); j++)
			  if(flarmId->type[FLARMID_SIZE_NAME-j] == ' ')
				flarmId->type[FLARMID_SIZE_NAME-j] = '\0';
			if(flarmId->freq[3] != ' ')
			  _stprintf(Comment,TEXT("%s  %sMHz %s"), flarmId->type     // FLARMID_SIZE_TYPE   22
				  	                                , flarmId->freq     // FLARMID_SIZE_FREQ   8    r
                                                    , flarmId->name);   // FLARMID_SIZE_NAME   22 => max 52 char
            else
			  _stprintf(Comment,TEXT("%s %s"), flarmId->type            // FLARMID_SIZE_TYPE   22
                                             , flarmId->name);          // FLARMID_SIZE_NAME   22 => max 52 char

			_stprintf(text1,TEXT("%s %s "), pFlarm->Cn            // MAXFLARMCN          3
					                      , Comment);             //                     52  => 70
          }
		  if(flarmId != NULL)
			_stprintf(Comment,TEXT("%s"), flarmId->airfield);
		  _stprintf(text2,TEXT("(%3.1f%s  %i%s  %3.1f%s) %s"), pFlarm->Average30s *  LIFTMODIFY                  //        5
					                                         , Units::GetVerticalSpeedName()                     // 3+2=   5
			                                                 , (int)(pFlarm->RelativeAltitude * ALTITUDEMODIFY)  //        5
                                                             , Units::GetAltitudeName()                          // 3+2=   5
			                                                 , pFlarm->Distance*DISTANCEMODIFY                   //        6
			                                                 , Units::GetDistanceName()                          // 2+3=   5
			                                                 , Comment );         //FLARMID_SIZE_AIRFIELD                  22  =>  53 char


		break;
	  }

	  /********************
	   * show text
	   ********************/
	  SetBkMode(hDC, TRANSPARENT);
	  SetTextColor(hDC,  RGB_BLACK);
	  int iLen = _tcslen(text1);
	  if( iLen > 100)
		  iLen = 100;
	  ExtTextOut(hDC, (int)(PICTO_WIDTH*1.1)*ScreenScale,  2 *ScreenScale, ETO_OPAQUE, NULL, text1, iLen, NULL);
	  SetTextColor(hDC,  RGB_DARKBLUE);
	  iLen = _tcslen(text2);
	  if( iLen > 100)
		iLen = 100;
	  ExtTextOut(hDC, (int)(PICTO_WIDTH*1.1)*ScreenScale,  15*ScreenScale, ETO_OPAQUE, NULL, text2, iLen, NULL);

	}
}


static void OnMultiSelectListListEnter(WindowControl * Sender,
				WndListFrame::ListInfo_t *ListInfo) {
  (void)Sender;

  ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex>=iNO_ELEMENTS) {
    ItemIndex = iNO_ELEMENTS-1;
  }
  if (ItemIndex>=0) {
    wf->SetModalResult(mrOK);
  }


  if((ItemIndex >= 0) && (ItemIndex < iNO_ELEMENTS))
  {
	dlgAddMultiSelectListDetailsDialog( ItemIndex);
  }

}


static void OnEnterClicked(WindowControl * Sender)
{
  (void)Sender;

  if (ItemIndex>=iNO_ELEMENTS) {
    ItemIndex = iNO_ELEMENTS-1;
  }
  if (ItemIndex>=0) {
    wf->SetModalResult(mrOK);
  }

  if((ItemIndex >= 0) && (ItemIndex < iNO_ELEMENTS))
  {
	dlgAddMultiSelectListDetailsDialog( ItemIndex);
  }

}


static void OnMultiSelectListListInfo(WindowControl * Sender,
			       WndListFrame::ListInfo_t *ListInfo){

//StartupStore(_T("OnMultiSelectListListInfo ListInfo %i\n"),(long) ListInfo);


  (void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = iNO_ELEMENTS;
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
    ItemIndex     = ListInfo->ItemIndex+ListInfo->ScrollIndex;
  }
/*
  StartupStore(_T(". =============================\n"));
  StartupStore(_T(".                 ItemIndex: %i\n"),ItemIndex);
  StartupStore(_T(".             DrawListIndex: %i\n"),DrawListIndex);
  StartupStore(_T(". =============================\n"));
 // StartupStore(_T(".                    iValue: %i\n"),iValue);
  StartupStore(_T(".       mListInfo.ItemIndex: %i\n"), ListInfo->ItemIndex);
  StartupStore(_T(".     mListInfo.ScrollIndex: %i\n"), ListInfo->ScrollIndex );
  StartupStore(_T(".       mListInfo.DrawIndex: %i\n"), ListInfo->DrawIndex );
  StartupStore(_T(".        mListInfo.TopIndex: %i\n"), ListInfo->TopIndex );
  StartupStore(_T(".     mListInfo.BottomIndex: %i\n"), ListInfo->BottomIndex );
  StartupStore(_T(". mListInfo.ItemInViewCount: %i\n"), ListInfo->ItemInViewCount );
  StartupStore(_T(". mListInfo.ItemInPageCount: %i\n"), ListInfo->ItemInPageCount );
  StartupStore(_T(".       mListInfo.ItemCount: %i\n"), ListInfo->ItemCount );
*/
}

static void OnCloseClicked(WindowControl * Sender){
  (void)Sender;
  ItemIndex = -1;

  wf->SetModalResult(mrCancle);

}




static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnMultiSelectListPaintListItem),
  DeclareCallBackEntry(OnMultiSelectListListInfo),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnUpClicked),
  DeclareCallBackEntry(OnEnterClicked),
  DeclareCallBackEntry(OnDownClicked),
  DeclareCallBackEntry(NULL)
};



ListElement* dlgMultiSelectListShowModal(void){

  ItemIndex = -1;

  if(iNO_ELEMENTS == 0)
  {
    if (EnableSoundModes)
	  PlayResource(TEXT("IDR_WAV_MM1"));
	return NULL;
  }
  if (EnableSoundModes)
  	PlayResource(TEXT("IDR_WAV_MM4"));
/*
  if(iNO_ELEMENTS == 1)
  {
	dlgAddMultiSelectListDetailsDialog(0);
	NoAirfields =0;
	NoOutlands  =0;
	NoWaypoints =0;
	NoFarm      =0;
	NoAirspace  =0;
	iNO_ELEMENTS =0;
	return &Elements[0];
  }
*/
  if (!ScreenLandscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgMultiSelectList_L.xml"));
    wf = dlgLoadFromXML(CallBackTable,
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_MULTISELECTLIST_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgMultiSelectList.xml"));
    wf = dlgLoadFromXML(CallBackTable,
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_MULTISELECTLIST"));

  }

  if (!wf) return NULL;

#ifdef AUTOUPDATE_MS
  wf->SetTimerNotify(OnTimer);
#endif
  wMultiSelectListList = (WndListFrame*)wf->FindByName(TEXT("frmMultiSelectListList"));
  LKASSERT(wMultiSelectListList!=NULL);
  wMultiSelectListList->SetBorderKind(BORDERLEFT);
  wMultiSelectListList->SetEnterCallback(OnMultiSelectListListEnter);
  wMultiSelectListList->SetWidth(wf->GetWidth() - wMultiSelectListList->GetLeft()-2);

  wMultiSelectListListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmMultiSelectListListEntry"));
  LKASSERT(wMultiSelectListListEntry!=NULL);
  wMultiSelectListListEntry->SetCanFocus(true);

   // ScrollbarWidth is initialised from DrawScrollBar in WindowControls, so it might not be ready here
  if ( wMultiSelectListList->ScrollbarWidth == -1) {
   #if defined (PNA)
   #define SHRINKSBFACTOR 1.0 // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #else
   #define SHRINKSBFACTOR 0.75  // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #endif
   wMultiSelectListList->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);

  }
  wMultiSelectListListEntry->SetWidth(wMultiSelectListList->GetWidth() - wMultiSelectListList->ScrollbarWidth - 5);

  UpdateList();

  wf->ShowModal();

  delete wf;

  wf = NULL;

  iNO_ELEMENTS = 0;

  NoAirfields =0;
  NoOutlands  =0;
  NoWaypoints =0;
  NoFarm      =0;
  NoAirspace  =0;
  NoTaskPoints=0;

return pResult;
}


