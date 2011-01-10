/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWayPointDetails.cpp,v 8.3 2010/12/13 16:42:57 root Exp root $
*/


#include "StdAfx.h"
#include <aygshell.h>

#include "XCSoar.h"

#include "Statistics.h"
#include "externs.h"
#include "McReady.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"
#include "Units.h"

extern void DrawJPG(HDC hdc, RECT rc);

#ifndef CECORE
#ifndef GNAV
#include "VOIMAGE.h"
#endif
#endif

static int page=0;
static WndForm *wf=NULL;
static WndListFrame *wDetails=NULL;
static WndOwnerDrawFrame *wDetailsEntry = NULL;
static WndFrame *wInfo=NULL;
static WndFrame *wCommand=NULL;
static WndFrame *wSpecial=NULL; // VENTA3
static WndOwnerDrawFrame *wImage=NULL;
static BOOL hasimage1 = false;
static BOOL hasimage2 = false;

#ifndef CECORE
#ifndef GNAV
static CVOImage jpgimage1;
static CVOImage jpgimage2;
#endif
#endif

static TCHAR path_modis[MAX_PATH];
static TCHAR path_google[MAX_PATH];
static TCHAR szWaypointFile[MAX_PATH] = TEXT("\0");
static TCHAR Directory[MAX_PATH];

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
      if (!hasimage1) {
        page += Step;
      } else {
        page_ok = true;
      }
      break;
    case 5:
      if (!hasimage2) {
        page += Step;
      } else {
        page_ok = true;
      }
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
  wImage->SetVisible(page > 4);

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
      ExtTextOut(hDC, 2*InfoBoxLayout::scale, 2*InfoBoxLayout::scale,
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


static void OnGotoClicked(WindowControl * Sender){
	(void)Sender;
#if 100125
  #if 100717
  GotoWaypoint(SelectedWaypoint);
  #else
  // If we are running a real task
  if (ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) {
	if (MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M159_ = "CONFIRM goto, ABORTING task?" 
	gettext(TEXT("_@M159_")),
	// LKTOKEN  _@M40_ = "A task is running!" 
	gettext(TEXT("_@M40_")),
	MB_YESNO|MB_ICONQUESTION) == IDYES) {
		LockTaskData();
		FlyDirectTo(SelectedWaypoint);
		UnlockTaskData();
	} 
  } else {
	LockTaskData();
	FlyDirectTo(SelectedWaypoint);
	UnlockTaskData();
  }
  #endif
#else
  LockTaskData();
  FlyDirectTo(SelectedWaypoint);
  UnlockTaskData();
#endif
  wf->SetModalResult(mrOK);
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
  #if NOSIM
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
  #else
  #ifdef _SIM_
  GPS_INFO.Latitude = WayPointList[HomeWaypoint].Latitude;	// 100213
  GPS_INFO.Longitude = WayPointList[HomeWaypoint].Longitude;
  GPS_INFO.Altitude = WayPointList[HomeWaypoint].Altitude;
  #else
  if ( GPS_INFO.NAVWarning ) {
	GPS_INFO.Latitude = WayPointList[HomeWaypoint].Latitude;	// 100213
	GPS_INFO.Longitude = WayPointList[HomeWaypoint].Longitude;
	GPS_INFO.Altitude = WayPointList[HomeWaypoint].Altitude;
  }
  #endif
  #endif
  // Update HomeWaypoint
  WpHome_Lat=WayPointList[HomeWaypoint].Latitude; // 100213
  WpHome_Lon=WayPointList[HomeWaypoint].Longitude;
  _tcscpy(WpHome_Name,WayPointList[HomeWaypoint].Name);

  SetToRegistry(szRegistryHomeWaypoint, HomeWaypoint);
  RefreshTask();
  UnlockTaskData();
  wf->SetModalResult(mrOK);
}

// VENTA3
static void OnSetAlternate1Clicked(WindowControl * Sender){
	(void)Sender;
  LockTaskData();
  Alternate1 = SelectedWaypoint;
  SetToRegistry(szRegistryAlternate1, Alternate1);
  RefreshTask();
  UnlockTaskData();
  wf->SetModalResult(mrOK);
}

static void OnSetAlternate2Clicked(WindowControl * Sender){
	(void)Sender;
  LockTaskData();
  Alternate2 = SelectedWaypoint;
  SetToRegistry(szRegistryAlternate2, Alternate2);
  RefreshTask();
  UnlockTaskData();
  wf->SetModalResult(mrOK);
}

static void OnClearAlternatesClicked(WindowControl * Sender){
	(void)Sender;
  LockTaskData();
  Alternate1 = -1; OnAlternate1=false;
  Alternate2 = -1; OnAlternate2=false;
  SetToRegistry(szRegistryAlternate1, Alternate1);
  SetToRegistry(szRegistryAlternate2, Alternate2);
  RefreshTask();
  UnlockTaskData();
  wf->SetModalResult(mrOK);
}


static void OnTeamCodeClicked(WindowControl * Sender){
	(void)Sender;
  TeamCodeRefWaypoint = SelectedWaypoint;
  SetToRegistry(szRegistryTeamcodeRefWaypoint, TeamCodeRefWaypoint);
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

static void OnImagePaint(WindowControl * Sender, HDC hDC){
  (void)Sender;

#ifndef CECORE
#ifndef GNAV
  if (page == 3)
    jpgimage1.Draw(hDC, 0, 0, -1, -1);

  if (page == 4)
    jpgimage2.Draw(hDC, 0, 0, -1, -1);

#endif
#endif
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnNextClicked),
  DeclareCallBackEntry(OnPrevClicked),
  DeclareCallBackEntry(OnPaintDetailsListItem),
  DeclareCallBackEntry(OnDetailsListInfo),
  DeclareCallBackEntry(NULL)
};



void dlgWayPointDetailsShowModal(void){

  TCHAR sTmp[128];
  double sunsettime;
  int sunsethours;
  int sunsetmins;
  WndProperty *wp;

  if (!InfoBoxLayout::landscape) {
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

  GetRegistryString(szRegistryWayPointFile, szWaypointFile, MAX_PATH);
  ExpandLocalPath(szWaypointFile);
  ExtractDirectory(Directory, szWaypointFile);

  _stprintf(path_modis,TEXT("%s\\modis-%03d.jpg"),
           Directory,
           SelectedWaypoint+1);
  _stprintf(path_google,TEXT("%s\\google-%03d.jpg"),
           Directory,
           SelectedWaypoint+1);

  #ifdef CUPSUP
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
  #else
  _stprintf(sTmp, TEXT("%s: "), wf->GetCaption());
  _tcscat(sTmp, WayPointList[SelectedWaypoint].Name);
  wf->SetCaption(sTmp);
  #endif

  wp = ((WndProperty *)wf->FindByName(TEXT("prpWpComment")));
  #if CUPCOM	// 101112
  if (WayPointList[SelectedWaypoint].Comment==NULL)
	wp->SetText(_T(""));
  else
	wp->SetText(WayPointList[SelectedWaypoint].Comment);
  #else
  wp->SetText(WayPointList[SelectedWaypoint].Comment);
  #endif
  wp->SetButtonSize(16);

  Units::LongitudeToString(WayPointList[SelectedWaypoint].Longitude, sTmp, sizeof(sTmp)-1);
  ((WndProperty *)wf->FindByName(TEXT("prpLongitude")))
    ->SetText(sTmp);

  Units::LatitudeToString(WayPointList[SelectedWaypoint].Latitude, sTmp, sizeof(sTmp)-1);
  ((WndProperty *)wf->FindByName(TEXT("prpLatitude")))
    ->SetText(sTmp);

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

  // alt reqd at safety mc

  // this is unused in fact
  alt = CALCULATED_INFO.NavAltitude - 
    GlidePolar::MacCreadyAltitude(GlidePolar::AbortSafetyMacCready(),
				  distance,
				  bearing, 
				  CALCULATED_INFO.WindSpeed, 
				  CALCULATED_INFO.WindBearing, 
				  0, 0, true,
				  0)-SAFETYALTITUDEARRIVAL-
    WayPointList[SelectedWaypoint].Altitude;

  wp = ((WndProperty *)wf->FindByName(TEXT("prpMc1")));
  if (wp) wp->SetText(sTmp);
  //


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
  wSpecial = ((WndFrame *)wf->FindByName(TEXT("frmSpecial"))); // VENTA3
  wImage   = ((WndOwnerDrawFrame *)wf->FindByName(TEXT("frmImage")));
  wDetails = (WndListFrame*)wf->FindByName(TEXT("frmDetails"));

  ASSERT(wInfo!=NULL);
  ASSERT(wCommand!=NULL);
  ASSERT(wSpecial!=NULL); // VENTA3
  ASSERT(wImage!=NULL);
  ASSERT(wDetails!=NULL);

  wDetailsEntry = 
    (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmDetailsEntry"));
  ASSERT(wDetailsEntry!=NULL);
  wDetailsEntry->SetCanFocus(true);

  nTextLines = TextToLineOffsets(WayPointList[SelectedWaypoint].Details,
				 LineOffsets,
				 MAXLINES);
  /* TODO enhancement: wpdetails
  wp = ((WndProperty *)wf->FindByName(TEXT("prpWpDetails")));
  wp->SetText(WayPointList[SelectedWaypoint].Details);
  */

  wInfo->SetBorderKind(BORDERLEFT);
  wCommand->SetBorderKind(BORDERLEFT);
  wSpecial->SetBorderKind(BORDERLEFT);
  wImage->SetBorderKind(BORDERLEFT | BORDERTOP | BORDERBOTTOM | BORDERRIGHT);
  wDetails->SetBorderKind(BORDERLEFT);

  wCommand->SetVisible(false);
  wSpecial->SetVisible(false);
	// LKTOKEN  _@M145_ = "Blank!" 
  wImage->SetCaption(gettext(TEXT("_@M145_")));
  wImage->SetOnPaintNotify(OnImagePaint);

  WndButton *wb;

  wb = ((WndButton *)wf->FindByName(TEXT("cmdGoto")));
  if (wb) 
    wb->SetOnClickNotify(OnGotoClicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdGoto2"))); // VNT FIX TODO duplicate cmds are not allowed in XML
  if (wb) 						// this is a workaround
    wb->SetOnClickNotify(OnGotoClicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdReplace")));
  if (wb)
    wb->SetOnClickNotify(OnReplaceClicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdNewHome")));
  if (wb) 
    wb->SetOnClickNotify(OnNewHomeClicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdSetAlternate1")));
  if (wb) 
    wb->SetOnClickNotify(OnSetAlternate1Clicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdSetAlternate1b")));
  if (wb) 
    wb->SetOnClickNotify(OnSetAlternate1Clicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdSetAlternate2")));
  if (wb) 
    wb->SetOnClickNotify(OnSetAlternate2Clicked);
  wb = ((WndButton *)wf->FindByName(TEXT("cmdSetAlternate2b")));
  if (wb) 
    wb->SetOnClickNotify(OnSetAlternate2Clicked);

  wb = ((WndButton *)wf->FindByName(TEXT("cmdClearAlternates")));
  if (wb) 
    wb->SetOnClickNotify(OnClearAlternatesClicked);

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

#ifndef CECORE
#ifndef GNAV
  hasimage1 = jpgimage1.Load(wImage->GetDeviceContext() ,path_modis );
  hasimage2 = jpgimage2.Load(wImage->GetDeviceContext() ,path_google );
#endif
#endif

  page = 0;

  NextPage(0); // JMW just to turn proper pages on/off

  wf->ShowModal();

  delete wf;

  wf = NULL;

}
