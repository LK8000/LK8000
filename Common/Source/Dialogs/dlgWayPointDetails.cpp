/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWayPointDetails.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include <aygshell.h>

#include "McReady.h"
#include "InfoBoxLayout.h"
#include "LKProfiles.h"
#include "Dialogs.h"

extern void DrawJPG(HDC hdc, RECT rc);

static int page=0;
static WndForm *wf=NULL;
static WndListFrame *wDetails=NULL;
static WndOwnerDrawFrame *wDetailsEntry = NULL;
static WndFrame *wInfo=NULL;
static WndFrame *wCommand=NULL;
static WndFrame *wSpecial=NULL;

#define MAXLINES 100
static int LineOffsets[MAXLINES];
static int DrawListIndex=0;
static int nTextLines=0;

#define WPLSEL WayPointList[SelectedWaypoint]

static void OnPaintWaypointPicto(WindowControl * Sender, HDC hDC){
#ifdef PICTORIALS
	  (void)Sender;
	  WndFrame  *wPicto = ((WndFrame *)wf->FindByName(TEXT("frmWaypointPicto")));
	  LKASSERT(wPicto!=NULL);

RECT *prc;
prc = wPicto->GetBoundRect();


//  StartupStore(_T("..Entered OnPaintWaypointPicto \n"));

  SetBkColor  (hDC, RGB_LIGHTGREY);

  if (WayPointCalc[SelectedWaypoint].IsLandable )
  {
	MapWindow::DrawRunway(hDC,&WayPointList[SelectedWaypoint],  *prc, 7000 , true);
  }
  else
  {
	MapWindow::DrawWaypointPictoBg(hDC,  *prc);
	MapWindow::DrawWaypointPicto(hDC,  *prc, &WayPointList[SelectedWaypoint]);
  }
#endif
}


static void NextPage(int Step){
  bool page_ok=false;
  page += Step;
  do {
    if (page<0) {
      page = 5;
    }
    if (page>5) {
      page = 0;
    }
    switch(page) {
    case 0:
      page_ok = true;
      break;
    case 1:
      if (!WayPointList[SelectedWaypoint].Details) {
        page += Step;
      } else {
        page_ok = true;
      }
      break;
    case 2:
      page_ok = true;
      break;
    case 3: // VENTA3
      page_ok = true;
      break;
    case 4:
        page += Step;
      break;
    case 5:
        page += Step;
      break;
    default:
      page_ok = true;
      page = 0;
      break;
      // error!
    }
  } while (!page_ok);

  wInfo->SetVisible(page == 0);
  wDetails->SetVisible(page == 1);
  wCommand->SetVisible(page == 2);
  wSpecial->SetVisible(page == 3);

  if (page==1) {
    wDetails->ResetList();
    wDetails->Redraw();
  }

}


static void OnPaintDetailsListItem(WindowControl * Sender, HDC hDC){
  (void)Sender;
  if (DrawListIndex < nTextLines){
    TCHAR* text = WayPointList[SelectedWaypoint].Details;
    int nstart = LineOffsets[DrawListIndex];
    int nlen;
    if (DrawListIndex<nTextLines-1) {
      nlen = LineOffsets[DrawListIndex+1]-LineOffsets[DrawListIndex]-1;
      nlen--;
    } else {
      nlen = _tcslen(text+nstart);
    }
    while (_tcscmp(text+nstart+nlen-1,TEXT("\r"))==0) {
      nlen--;
    }
    while (_tcscmp(text+nstart+nlen-1,TEXT("\n"))==0) {
      nlen--;
    }
    if (nlen>0) {
      ExtTextOut(hDC, 2*ScreenScale, 2*ScreenScale,
		 ETO_OPAQUE, NULL,
		 text+nstart,
		 nlen, 
		 NULL);
    }
  }
}


static void OnDetailsListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
	(void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = nTextLines-1;
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
  }
}



static void OnNextClicked(WindowControl * Sender){
	(void)Sender;
  NextPage(+1);
}

static void OnPrevClicked(WindowControl * Sender){
	(void)Sender;
  NextPage(-1);
}

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}

static int FormKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){
  (void)lParam; (void)Sender;
  switch(wParam & 0xffff){
    case VK_LEFT:
    case '6':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdPrev")))->GetHandle());
      NextPage(-1);
      //((WndButton *)wf->FindByName(TEXT("cmdPrev")))->SetFocused(true, NULL);
    return(0);
    case VK_RIGHT:
    case '7':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdNext")))->GetHandle());
      NextPage(+1);
      //((WndButton *)wf->FindByName(TEXT("cmdNext")))->SetFocused(true, NULL);
    return(0);
  }
  return(1);
}


static void OnReplaceClicked(WindowControl * Sender){
	(void)Sender;
  LockTaskData();

  ReplaceWaypoint(SelectedWaypoint);
  RealActiveWaypoint = SelectedWaypoint;

  RefreshTask();
  UnlockTaskData();
  MapWindow::RefreshMap();
  wf->SetModalResult(mrOK);
}

static void OnNewHomeClicked(WindowControl * Sender){
	(void)Sender;
  LockTaskData();
  HomeWaypoint = SelectedWaypoint;
  if (SIMMODE) {
	GPS_INFO.Latitude = WayPointList[HomeWaypoint].Latitude;
	GPS_INFO.Longitude = WayPointList[HomeWaypoint].Longitude;
	GPS_INFO.Altitude = WayPointList[HomeWaypoint].Altitude;
  } else {
	if ( GPS_INFO.NAVWarning ) {
		GPS_INFO.Latitude = WayPointList[HomeWaypoint].Latitude;
		GPS_INFO.Longitude = WayPointList[HomeWaypoint].Longitude;
		GPS_INFO.Altitude = WayPointList[HomeWaypoint].Altitude;
	}
  }
  // Update HomeWaypoint
  WpHome_Lat=WayPointList[HomeWaypoint].Latitude; // 100213
  WpHome_Lon=WayPointList[HomeWaypoint].Longitude;
  _tcscpy(WpHome_Name,WayPointList[HomeWaypoint].Name);

  #if TESTBENCH
  StartupStore(_T("... Home set to wp.%d by dlgWayPointDetails\n"),HomeWaypoint);
  #endif
  RefreshTask();
  UnlockTaskData();
  wf->SetModalResult(mrOK);
}

static void OnTeamCodeClicked(WindowControl * Sender){
	(void)Sender;
  TeamCodeRefWaypoint = SelectedWaypoint;
  wf->SetModalResult(mrOK);
}


static void OnInserInTaskClicked(WindowControl * Sender){
	(void)Sender;
  LockTaskData();
  InsertWaypoint(SelectedWaypoint);
  RefreshTask();
  UnlockTaskData();
  wf->SetModalResult(mrOK);
}

static void OnAppendInTask1Clicked(WindowControl * Sender){
	(void)Sender;
  LockTaskData();
  InsertWaypoint(SelectedWaypoint, 1); // append before finish
  RefreshTask();
  UnlockTaskData();
  wf->SetModalResult(mrOK);
}

static void OnAppendInTask2Clicked(WindowControl * Sender){
	(void)Sender;
  LockTaskData();
  InsertWaypoint(SelectedWaypoint, 2); // append after finish
  RefreshTask();
  UnlockTaskData();
  wf->SetModalResult(mrOK);
}


static void OnRemoveFromTaskClicked(WindowControl * Sender){
	(void)Sender;
  LockTaskData();
  RemoveWaypoint(SelectedWaypoint);
  RefreshTask();
  UnlockTaskData();
  wf->SetModalResult(mrOK);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnNextClicked),
  DeclareCallBackEntry(OnPrevClicked),
  DeclareCallBackEntry(OnPaintDetailsListItem),
  DeclareCallBackEntry(OnDetailsListInfo),
  DeclareCallBackEntry(OnPaintWaypointPicto),
  DeclareCallBackEntry(NULL)
};



void dlgWayPointDetailsShowModal(short mypage){

  TCHAR sTmp[128];
  double sunsettime;
  int sunsethours;
  int sunsetmins;
  WndProperty *wp;

  if (!ScreenLandscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgWayPointDetails_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_WAYPOINTDETAILS_L"));

  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgWayPointDetails.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_WAYPOINTDETAILS"));
  }
  nTextLines = 0;

  if (!wf) return;

  wInfo    = ((WndFrame *)wf->FindByName(TEXT("frmInfos")));
  wCommand = ((WndFrame *)wf->FindByName(TEXT("frmCommands")));
  wSpecial = ((WndFrame *)wf->FindByName(TEXT("frmSpecial")));
  wDetails = (WndListFrame*)wf->FindByName(TEXT("frmDetails"));

  LKASSERT(wInfo!=NULL);
  LKASSERT(wCommand!=NULL);
  LKASSERT(wSpecial!=NULL);
  LKASSERT(wDetails!=NULL);

  // Resize Frames up to real screen size on the right.
  wInfo->SetBorderKind(BORDERLEFT);

  wCommand->SetBorderKind(BORDERLEFT);
  wSpecial->SetBorderKind(BORDERLEFT);
  wDetails->SetBorderKind(BORDERLEFT);

  wCommand->SetVisible(false);
  wSpecial->SetVisible(false);


  //
  // CAPTION: top line in black
  //

  // if SeeYou waypoint
  if (WPLSEL.Format == LKW_CUP) { 
	TCHAR ttmp[50];
	// and it is landable
	if ((WPLSEL.Style>1) && (WPLSEL.Style<6) ) {

		_stprintf(sTmp, TEXT("%s "), WPLSEL.Name);
		// ICAO name probably, let's print it
		if ( _tcslen(WPLSEL.Code)==4 ) {
			_stprintf(ttmp,_T("(%s) "),WPLSEL.Code);
			_tcscat(sTmp, ttmp);
		}

		if ( _tcslen(WPLSEL.Freq)>0 )  {
			_stprintf(ttmp,_T("%s "),WPLSEL.Freq);
			_tcscat(sTmp, ttmp);
		}

		if ( WPLSEL.RunwayDir>=0 )  {
			_stprintf(ttmp,_T("RW %d "),WPLSEL.RunwayDir);
			_tcscat(sTmp, ttmp);
		}
		if ( WPLSEL.RunwayLen>0 )  {
			// we use Altitude instead of distance, to keep meters and feet
			_stprintf(ttmp,_T("%.0f%s"),Units::ToUserAltitude((double)WPLSEL.RunwayLen), Units::GetAltitudeName());
			_tcscat(sTmp, ttmp);
		}

		wf->SetCaption(sTmp);
	} else {
		_stprintf(sTmp, TEXT("%s: "), wf->GetCaption());
		_tcscat(sTmp, WayPointList[SelectedWaypoint].Name);
		wf->SetCaption(sTmp);
	}
  } else {
	_stprintf(sTmp, TEXT("%s: "), wf->GetCaption());
	_tcscat(sTmp, WayPointList[SelectedWaypoint].Name);
	wf->SetCaption(sTmp);
  }

  //
  // Waypoint Comment
  //
  wp = ((WndProperty *)wf->FindByName(TEXT("prpWpComment")));
  LKASSERT(wp);
  if (WayPointList[SelectedWaypoint].Comment==NULL)
	wp->SetText(_T(""));
  else
	wp->SetText(WayPointList[SelectedWaypoint].Comment);

  wp->SetButtonSize(16);

  //
  // Lat and Lon
  //
  Units::CoordinateToString(
		  WayPointList[SelectedWaypoint].Longitude,
		  WayPointList[SelectedWaypoint].Latitude,
		  sTmp, sizeof(sTmp)-1);

  wp = ((WndProperty *)wf->FindByName(TEXT("prpCoordinate")));
  LKASSERT(wp);
  wp->SetText(sTmp);
	
  //
  // Waypoint Altitude 
  //
  Units::FormatUserAltitude(WayPointList[SelectedWaypoint].Altitude, sTmp, sizeof(sTmp)-1);
  wp = ((WndProperty *)wf->FindByName(TEXT("prpAltitude")));
  LKASSERT(wp);
  wp->SetText(sTmp);

  //
  // SUNSET at waypoint
  //
  sunsettime = DoSunEphemeris(WayPointList[SelectedWaypoint].Longitude,
                              WayPointList[SelectedWaypoint].Latitude);
  sunsethours = (int)sunsettime;
  sunsetmins = (int)((sunsettime-sunsethours)*60);

  _stprintf(sTmp, TEXT("%02d:%02d"), sunsethours, sunsetmins);
  ((WndProperty *)wf->FindByName(TEXT("prpSunset")))->SetText(sTmp);


  // 
  // Distance and bearing
  //
  double distance, bearing;
  DistanceBearing(GPS_INFO.Latitude,
                  GPS_INFO.Longitude,
                  WayPointList[SelectedWaypoint].Latitude,
                  WayPointList[SelectedWaypoint].Longitude, 
                  &distance, 
                  &bearing);

  TCHAR DistanceText[MAX_PATH];
  Units::FormatUserDistance(distance, DistanceText, 10);
  ((WndProperty *)wf->FindByName(TEXT("prpDistance")))->SetText(DistanceText);

  _stprintf(sTmp, TEXT("%d")TEXT(DEG), iround(bearing));
  ((WndProperty *)wf->FindByName(TEXT("prpBearing")))->SetText(sTmp);


  //
  // Altitude reqd at mc 0
  // 
  double alt=0;
  alt = CALCULATED_INFO.NavAltitude - 
    GlidePolar::MacCreadyAltitude(0.0,
				  distance,
				  bearing, 
				  CALCULATED_INFO.WindSpeed, 
				  CALCULATED_INFO.WindBearing, 
				  0, 0, true,
				  0)- WayPointList[SelectedWaypoint].Altitude;

  if (SafetyAltitudeMode==1 || WayPointCalc[SelectedWaypoint].IsLandable) 
	alt-=(SAFETYALTITUDEARRIVAL/10);

  _stprintf(sTmp, TEXT("%.0f %s"), alt*ALTITUDEMODIFY,
	    Units::GetAltitudeName());

  wp = ((WndProperty *)wf->FindByName(TEXT("prpMc0")));
  if (wp) wp->SetText(sTmp);



  // alt reqd at current mc
  alt = CALCULATED_INFO.NavAltitude - 
    GlidePolar::MacCreadyAltitude(MACCREADY,
				  distance,
				  bearing, 
				  CALCULATED_INFO.WindSpeed, 
				  CALCULATED_INFO.WindBearing, 
				  0, 0, true,
				  0)-
    WayPointList[SelectedWaypoint].Altitude;

  if (SafetyAltitudeMode==1 || WayPointCalc[SelectedWaypoint].IsLandable) 
	alt-=(SAFETYALTITUDEARRIVAL/10);

  _stprintf(sTmp, TEXT("%.0f %s"), alt*ALTITUDEMODIFY,
	    Units::GetAltitudeName());

  wp = ((WndProperty *)wf->FindByName(TEXT("prpMc2")));
  if (wp) {
	wp->SetText(sTmp);
  }


  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  // We DONT use PREV  anymore
  ((WndButton *)wf->FindByName(TEXT("cmdPrev")))->SetVisible(false);


  //
  // Details (WAYNOTES) page
  //
  wDetailsEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmDetailsEntry"));
  LKASSERT(wDetailsEntry!=NULL);
  wDetailsEntry->SetCanFocus(true);

  nTextLines = TextToLineOffsets(WayPointList[SelectedWaypoint].Details,
				 LineOffsets,
				 MAXLINES);

  // ScrollbarWidth is initialised from DrawScrollBar in WindowControls, so it might not be ready here
  if ( wDetails->ScrollbarWidth == -1) {
   #if defined (PNA)
   #define SHRINKSBFACTOR 1.0 // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #else
   #define SHRINKSBFACTOR 0.75  // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #endif
   wDetails->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);

  }
  wDetailsEntry->SetWidth(wDetails->GetWidth() - wDetails->ScrollbarWidth - 5);

  WndButton *wb;

  TCHAR captmp[200];

  // Resize also buttons
  wb = ((WndButton *)wf->FindByName(TEXT("cmdInserInTask")));
  if (wb) {
    wb->SetOnClickNotify(OnInserInTaskClicked);
    wb->SetWidth(wCommand->GetWidth()-wb->GetLeft()*2);

    if ((ActiveWayPoint<0) || !ValidTaskPoint(0)) {
	// this is going to be the first tp (ActiveWayPoint 0)
	_stprintf(captmp,_T("%s"),MsgToken(1824)); // insert as START
    } else {
	LKASSERT(ActiveWayPoint>=0 && ValidTaskPoint(0));
	int indexInsert = max(ActiveWayPoint,0); // safe check
	if (indexInsert==0) {
		_stprintf(captmp,_T("%s"),MsgToken(1824)); // insert as START
	} else {
		LKASSERT(ValidWayPoint(Task[indexInsert].Index));
		_stprintf(captmp,_T("%s <%s>"),MsgToken(1825),WayPointList[ Task[indexInsert].Index ].Name); // insert before xx
	}
    }
    wb->SetCaption(captmp);
  }

  wb = ((WndButton *)wf->FindByName(TEXT("cmdAppendInTask1")));
  if (wb) {
    wb->SetOnClickNotify(OnAppendInTask1Clicked);
    wb->SetWidth(wCommand->GetWidth()-wb->GetLeft()*2);
  }

  wb = ((WndButton *)wf->FindByName(TEXT("cmdAppendInTask2")));
  if (wb) {
    wb->SetOnClickNotify(OnAppendInTask2Clicked);
    wb->SetWidth(wCommand->GetWidth()-wb->GetLeft()*2);
  }

  wb = ((WndButton *)wf->FindByName(TEXT("cmdRemoveFromTask")));
  if (wb) {
    wb->SetOnClickNotify(OnRemoveFromTaskClicked);
    wb->SetWidth(wCommand->GetWidth()-wb->GetLeft()*2);
  }

  wb = ((WndButton *)wf->FindByName(TEXT("cmdReplace")));
  if (wb) {
    wb->SetWidth(wCommand->GetWidth()-wb->GetLeft()*2);

    int tmpIdx =  -1;
    if (ValidTaskPoint(ActiveWayPoint))
      tmpIdx = Task[ActiveWayPoint].Index;
    if(  ValidTaskPoint(PanTaskEdit))
     tmpIdx = RealActiveWaypoint;
    if(tmpIdx != -1)
    {
    	wb->SetOnClickNotify(OnReplaceClicked);
	_stprintf(captmp,_T("%s <%s>"),MsgToken(1826),WayPointList[tmpIdx ].Name); // replace  xx
    } else {
	_stprintf(captmp,_T("( %s )"),MsgToken(555));
    }
    wb->SetCaption(captmp);
  }


  wb = ((WndButton *)wf->FindByName(TEXT("cmdNewHome")));
  if (wb)  {
    wb->SetOnClickNotify(OnNewHomeClicked);
    wb->SetWidth(wSpecial->GetWidth()-wb->GetLeft()*2);
  }

  wb = ((WndButton *)wf->FindByName(TEXT("cmdTeamCode")));
  if (wb) {
    wb->SetOnClickNotify(OnTeamCodeClicked);
    wb->SetWidth(wSpecial->GetWidth()-wb->GetLeft()*2);
  }

  page = mypage;

  NextPage(0);

  wf->ShowModal();

  delete wf;

  wf = NULL;

}


