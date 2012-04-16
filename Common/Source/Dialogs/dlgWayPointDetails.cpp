/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWayPointDetails.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include <aygshell.h>

#include "McReady.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"
#include "LKProfiles.h"

#include "Units.h"


extern void DrawJPG(HDC hdc, RECT rc);

static int page=0;
static WndForm *wf=NULL;
static WndListFrame *wDetails=NULL;
static WndOwnerDrawFrame *wDetailsEntry = NULL;
static WndFrame *wInfo=NULL;
static WndFrame *wCommand=NULL;
static WndFrame *wSpecial=NULL; // VENTA3

#define MAXLINES 100
static int LineOffsets[MAXLINES];
static int DrawListIndex=0;
static int nTextLines=0;

#define WPLSEL WayPointList[SelectedWaypoint]

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
  RefreshTask();
  UnlockTaskData();
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

static void OnAppendInTaskClicked(WindowControl * Sender){
	(void)Sender;
  LockTaskData();
  InsertWaypoint(SelectedWaypoint, true);
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

  wp = ((WndProperty *)wf->FindByName(TEXT("prpWpComment")));
  if (WayPointList[SelectedWaypoint].Comment==NULL)
	wp->SetText(_T(""));
  else
	wp->SetText(WayPointList[SelectedWaypoint].Comment);
  wp->SetButtonSize(16);

  Units::CoordinateToString(
		  WayPointList[SelectedWaypoint].Longitude,
		  WayPointList[SelectedWaypoint].Latitude,
		  sTmp, sizeof(sTmp)-1);

  ((WndProperty *)wf->FindByName(TEXT("prpCoordinate")))->SetText(sTmp);

  Units::FormatUserAltitude(WayPointList[SelectedWaypoint].Altitude, sTmp, sizeof(sTmp)-1);
  ((WndProperty *)wf->FindByName(TEXT("prpAltitude")))
    ->SetText(sTmp);

  sunsettime = DoSunEphemeris(WayPointList[SelectedWaypoint].Longitude,
                              WayPointList[SelectedWaypoint].Latitude);
  sunsethours = (int)sunsettime;
  sunsetmins = (int)((sunsettime-sunsethours)*60);

  _stprintf(sTmp, TEXT("%02d:%02d"), sunsethours, sunsetmins);
  ((WndProperty *)wf->FindByName(TEXT("prpSunset")))
    ->SetText(sTmp);

  double distance, bearing;
  DistanceBearing(GPS_INFO.Latitude,
                  GPS_INFO.Longitude,
                  WayPointList[SelectedWaypoint].Latitude,
                  WayPointList[SelectedWaypoint].Longitude, 
                  &distance, 
                  &bearing);

  TCHAR DistanceText[MAX_PATH];
  Units::FormatUserDistance(distance, DistanceText, 10);
  ((WndProperty *)wf->FindByName(TEXT("prpDistance")))
    ->SetText(DistanceText);

  _stprintf(sTmp, TEXT("%d")TEXT(DEG), iround(bearing));
  ((WndProperty *)wf->FindByName(TEXT("prpBearing")))
    ->SetText(sTmp);

  double alt=0;

  // alt reqd at mc 0
  
  alt = CALCULATED_INFO.NavAltitude - 
    GlidePolar::MacCreadyAltitude(0.0,
				  distance,
				  bearing, 
				  CALCULATED_INFO.WindSpeed, 
				  CALCULATED_INFO.WindBearing, 
				  0, 0, true,
				  0)- WayPointList[SelectedWaypoint].Altitude;

  if (SafetyAltitudeMode==1 || WayPointCalc[SelectedWaypoint].IsLandable) 
	alt-=SAFETYALTITUDEARRIVAL;

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
	alt-=SAFETYALTITUDEARRIVAL;

  _stprintf(sTmp, TEXT("%.0f %s"), alt*ALTITUDEMODIFY,
	    Units::GetAltitudeName());

  wp = ((WndProperty *)wf->FindByName(TEXT("prpMc2")));
  if (wp) wp->SetText(sTmp);

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  wInfo    = ((WndFrame *)wf->FindByName(TEXT("frmInfos")));
  wCommand = ((WndFrame *)wf->FindByName(TEXT("frmCommands")));
  wSpecial = ((WndFrame *)wf->FindByName(TEXT("frmSpecial")));
  wDetails = (WndListFrame*)wf->FindByName(TEXT("frmDetails"));

  LKASSERT(wInfo!=NULL);
  LKASSERT(wCommand!=NULL);
  LKASSERT(wSpecial!=NULL);
  LKASSERT(wDetails!=NULL);

  wDetailsEntry = 
    (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmDetailsEntry"));
  LKASSERT(wDetailsEntry!=NULL);
  wDetailsEntry->SetCanFocus(true);

  nTextLines = TextToLineOffsets(WayPointList[SelectedWaypoint].Details,
				 LineOffsets,
				 MAXLINES);

  wInfo->SetBorderKind(BORDERLEFT);
  wCommand->SetBorderKind(BORDERLEFT);
  wSpecial->SetBorderKind(BORDERLEFT);
  wDetails->SetBorderKind(BORDERLEFT);

  wCommand->SetVisible(false);
  wSpecial->SetVisible(false);

  WndButton *wb;

  wb = ((WndButton *)wf->FindByName(TEXT("cmdReplace")));
  if (wb)
    wb->SetOnClickNotify(OnReplaceClicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdNewHome")));
  if (wb) 
    wb->SetOnClickNotify(OnNewHomeClicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdTeamCode")));
  if (wb) 
    wb->SetOnClickNotify(OnTeamCodeClicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdInserInTask")));
  if (wb) 
    wb->SetOnClickNotify(OnInserInTaskClicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdAppendInTask")));
  if (wb) 
    wb->SetOnClickNotify(OnAppendInTaskClicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdRemoveFromTask")));
  if (wb) 
    wb->SetOnClickNotify(OnRemoveFromTaskClicked);

  page = mypage;

  NextPage(0);

  wf->ShowModal();

  delete wf;

  wf = NULL;

}
