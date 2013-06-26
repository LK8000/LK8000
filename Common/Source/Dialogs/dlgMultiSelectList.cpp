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

extern FlarmIdFile *file;
#define  LEAVE_AFTER_ENTER
#define MAX_LIST_ITEMS 30
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
static int NoFarm      =0;
static int NoAirspace  =0;

#define MAX_AIRFILEDS 50
#define MAX_OUTLAND   50
#define MAX_WAYPOINTS 50
#define MAX_FLARM     50
#define MAX_AIRSPACES 50




static void UpdateList(void){
	wMultiSelectListList->ResetList();
	wMultiSelectListList->Redraw();

	wMultiSelectListListEntry->SetFocused(true,NULL);
}

static int DrawListIndex=0;


static void OnUpClicked(WindowControl * Sender)
{
  if(ItemIndex > 0)
  {
	ItemIndex--;
	wMultiSelectListList->SetItemIndex(ItemIndex);
  }
}

static void OnDownClicked(WindowControl * Sender)
{
  (void)Sender;

  if(ItemIndex < (iNO_ELEMENTS-1))
  {
	ItemIndex++;
    wMultiSelectListList->SetItemIndex(ItemIndex);
  }
}

void dlgAddMultiSelectListDetailsDialog(int Index)
{
  if((Index >= 0) && (Index < iNO_ELEMENTS))
  {
    switch(Elements[Index].type)
    {
      case IM_AIRSPACE:
    	LKASSERT(Elements[Index].ptr);
	    dlgAirspaceDetails((CAirspace*)Elements[Index].ptr);
      break;
      case IM_WAYPOINT:
	    LKASSERT(Elements[Index].iIdx < (int)NumberOfWayPoints);
      SelectedWaypoint	=Elements[Index].iIdx;
	    PopupWaypointDetails();
	  break;
      case IM_FLARM:
	    LKASSERT(Elements[Index].iIdx<FLARM_MAX_TRAFFIC);
	    dlgLKTrafficDetails(Elements[Index].iIdx);
      break;
    }
  }
}

void dlgAddMultiSelectListItem(long* pNew ,int Idx, char type, double Distance){

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
	if(type == IM_AIRSPACE )
	  Distance = 999999.0;


    bool skip = false;
	if(iNO_ELEMENTS < MAX_LIST_ITEMS-1)
	{
		switch(type)
		{
		  case IM_AIRSPACE:	if(NoAirspace < MAX_AIRSPACES)	NoAirspace++;	else  skip=true;  break;
		  case IM_FLARM:	if(NoFarm     < MAX_FLARM    )	NoFarm++;       else  skip=true;  break;
		  case IM_WAYPOINT:
			  if (WayPointCalc[Idx].IsLandable ){
				if( WayPointCalc[Idx].IsAirport) {
				  if(NoAirfields     < MAX_AIRFILEDS    )	NoAirfields++;       else  skip=true;
				} else {
				  if(NoOutlands     < MAX_OUTLAND       )  NoOutlands++;       else  skip=true;
				}
			  } else {
				if(NoWaypoints     < MAX_WAYPOINTS    )	NoWaypoints++;       else  skip=true;
			  }
		  break;
		}

		if(!skip)  // insert in list sorted by distance
		{
          int Pos = 0;
		  while ((Elements[Pos].Dist < Distance) && (Pos < iNO_ELEMENTS))
			Pos++;

		  for (int i =  iNO_ELEMENTS; i > Pos; i--)
		  {
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
#define PICTO_WIDTH 60

	if ((DrawListIndex < iNO_ELEMENTS) &&(DrawListIndex>=0))
	{
	  int j;
	  static CAirspace airspace_copy;
	  int i = DrawListIndex;
	  LKASSERT(i < MAX_LIST_ITEMS);
	  RECT rc = {2*ScreenScale,  2*ScreenScale, PICTO_WIDTH*ScreenScale,   34*ScreenScale};

	  CAirspace* pAS = NULL;
	  FLARM_TRAFFIC* pFlarm=NULL;
	  int HorDist,Bearing, VertDist;
	  double Distance;
	  unsigned int idx=0;
	  TCHAR text1[180] ={TEXT("empty")};
	  TCHAR text2[180] ={TEXT("empty")};
	  TCHAR Comment[80]={TEXT("")};
	  TCHAR Comment1[80]={TEXT("")};
	  SetBkColor  (hDC, RGB(0xFF, 0xFF, 0xFF));
	  LKASSERT(i < MAX_LIST_ITEMS);
	  switch(Elements[i].type )
	  {
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
		case IM_WAYPOINT:
		  idx= Elements[i].iIdx;
		  LKASSERT(idx < NumberOfWayPoints);

		  if(WayPointList[idx].Comment != NULL)  {
		    LK_tcsncpy(Comment,WayPointList[idx].Comment,30);
		  } else {
			_stprintf(Comment,TEXT(""));
		  }
          DistanceBearing( GPS_INFO.Latitude,GPS_INFO.Longitude, WayPointList[idx].Latitude,  WayPointList[idx].Longitude, &Distance, NULL);

		  if (WayPointCalc[idx].IsLandable )
		  {
			MapWindow::DrawRunway(hDC,&WayPointList[idx],  rc, 3000, true);

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

			_stprintf(text2,TEXT("%3.1f%s (%i%s  %02i/%02i  %i%s)"), Distance*DISTANCEMODIFY
					                                               , Units::GetDistanceName()
				                                                   , (int)(WayPointList[idx].Altitude * ALTITUDEMODIFY)
														           , Units::GetAltitudeName()
														           , (int)(WayPointList[idx].RunwayDir/10.0)
														           , (int)(AngleLimit360(((WayPointList[idx].RunwayDir+180)/10.0)))
														           , (int)((double)WayPointList[idx].RunwayLen* ALTITUDEMODIFY)
														           , Units::GetAltitudeName());
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


  if((ItemIndex >= 0) && (ItemIndex < iNO_ELEMENTS))
  {
	dlgAddMultiSelectListDetailsDialog( ItemIndex);
  }
  /*******************************************************
   * define LEAVE_AFTER_ENTER for immediate leave
   * of the MultiSelct dialog after seltecting an item
   * undefining it will return to the MultiSelct
   *******************************************************/
#ifdef LEAVE_AFTER_ENTER
  ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex>=iNO_ELEMENTS) {
    ItemIndex = iNO_ELEMENTS-1;
  }
  if (ItemIndex>=0) {
    wf->SetModalResult(mrOK);
  }
#endif
}


static void OnEnterClicked(WindowControl * Sender)
{
  (void)Sender;

  if((ItemIndex >= 0) && (ItemIndex < iNO_ELEMENTS))
  {
	dlgAddMultiSelectListDetailsDialog( ItemIndex);
  }
  /*******************************************************
   * define LEAVE_AFTER_ENTER for immediate leave
   * of the MultiSelct dialog after seltecting an item
   * undefining it will return to the MultiSelct
   *******************************************************/
#ifdef LEAVE_AFTER_ENTER

  if (ItemIndex>=iNO_ELEMENTS) {
    ItemIndex = iNO_ELEMENTS-1;
  }
  if (ItemIndex>=0) {
    wf->SetModalResult(mrOK);
  }
#endif
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


return pResult;
}


