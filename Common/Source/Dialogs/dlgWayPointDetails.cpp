/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWayPointDetails.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include "McReady.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "TeamCodeCalculation.h"
#include "NavFunctions.h"
#include "Event/Event.h"
#include "utils/TextWrapArray.h"
#include "resource.h"
#include "LKStyle.h"
#include "Sound/Sound.h"
#include "Radio.h"
#include "Waypoints/SetHome.h"

static int page=0;
static WndForm *wf=NULL;
static WndListFrame *wDetails=NULL;
static WndOwnerDrawFrame *wDetailsEntry = NULL;
static WndFrame *wInfo=NULL;
static WndFrame *wCommand=NULL;
static WndFrame *wSpecial=NULL;
static WndListFrame *wComment=NULL;
static WndOwnerDrawFrame *wCommentEntry = NULL;

#define WPLSEL WayPointList[SelectedWaypoint]

static int DetailDrawListIndex=0;
static TextWrapArray aDetailTextLine;

static int CommentDrawListIndex=0;
static TextWrapArray aCommentTextLine;

static void OnPaintWaypointPicto(WndOwnerDrawFrame * Sender, LKSurface& Surface) {
    if (Sender) {
        const RECT rc = Sender->GetClientRect();

        LockTaskData();
        LKASSERT(ValidWayPointFast(SelectedWaypoint));
        if (WayPointCalc[SelectedWaypoint].IsLandable) {
            MapWindow::DrawRunway(Surface, &WayPointList[SelectedWaypoint], rc, nullptr, 1.0 , true);
        } else {
            MapWindow::DrawWaypointPictoBg(Surface, rc);
            MapWindow::DrawWaypointPicto(Surface, rc, &WayPointList[SelectedWaypoint]);
        }
        UnlockTaskData();
    }
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
      LKASSERT(SelectedWaypoint>=0);
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


static void OnPaintDetailsListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface){

  if (DetailDrawListIndex < (int)aDetailTextLine.size()){
      LKASSERT(DetailDrawListIndex>=0);
      const TCHAR* szText = aDetailTextLine[DetailDrawListIndex];
      Surface.SetTextColor(RGB_BLACK);
      Surface.DrawText(DLGSCALE(2), DLGSCALE(2), szText);
  }
}


static void OnDetailsListInfo(WndListFrame * Sender, WndListFrame::ListInfo_t *ListInfo){

  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = aDetailTextLine.size();
  } else {
    DetailDrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
  }
}




static void OnPaintWpCommentListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface){

  if (CommentDrawListIndex < (int)aCommentTextLine.size()){
    LKASSERT(CommentDrawListIndex>=0);
    const TCHAR* szText = aCommentTextLine[CommentDrawListIndex];
    Surface.SetTextColor(RGB_BLACK);
    Surface.DrawText(DLGSCALE(2), DLGSCALE(2), szText);

    size_t pos, len;
    unsigned khz = ExtractFrequency(szText, &pos, &len); 

    if ((khz > 0) && ((pos + len) < 255)) {
      TCHAR sTmp[255];
      // copy text until end of substring
      TCHAR* end = std::copy_n(aCommentTextLine[CommentDrawListIndex], pos + len, sTmp);
      *end = _T('\0');
      // size of the text is end of underline
      const int subend = Surface.GetTextWidth(sTmp) + DLGSCALE(2);
      // truncate string to start of substring
      sTmp[pos] = 0;
      // size of the text is start of underline
      const int substart = Surface.GetTextWidth(sTmp) + DLGSCALE(2);

      if(substart < subend) {
        int h =  Sender->GetHeight() - IBLSCALE(1);
        const auto hOldPen = Surface.SelectObject(LKPen_Black_N1);
        Surface.DrawLine(substart, h, subend, h);
        Surface.SelectObject(hOldPen);
      }
    }
  }
}

static void OnWpCommentListInfo(WndListFrame * Sender, WndListFrame::ListInfo_t *ListInfo){

  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = aCommentTextLine.size();
    //AlphaLima
  } else {
    CommentDrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
  }
}



static void OnNextClicked(WndButton* pWnd){
  NextPage(+1);
}

static void OnPrevClicked(WndButton* pWnd){
  NextPage(-1);
}

static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static bool FormKeyDown(WndForm* pWnd, unsigned KeyCode) {
    Window * pBtn = NULL;

    switch (KeyCode & 0xffff) {
        case KEY_LEFT:
        case '6':
            pBtn = pWnd->FindByName(TEXT("cmdPrev"));
            NextPage(-1);
            break;
        case KEY_RIGHT:
        case '7':
            pBtn = pWnd->FindByName(TEXT("cmdNext"));
            NextPage(+1);
            break;;
    }
    if (pBtn) {
        pBtn->SetFocus();
        return true;
    }

    return false;
}


static void OnReplaceClicked(WndButton* pWnd){
  LockTaskData();

  ReplaceWaypoint(SelectedWaypoint);
  RealActiveWaypoint = SelectedWaypoint;

  RefreshTask();
  UnlockTaskData();
  MapWindow::RefreshMap();
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnNewHomeClicked(WndButton* pWnd){
  SetNewHome(SelectedWaypoint);

  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnTeamCodeClicked(WndButton* pWnd){
  TeamCodeRefWaypoint = SelectedWaypoint;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void OnInserInTaskClicked(WndButton* pWnd){
  LockTaskData();
  InsertWaypoint(SelectedWaypoint);
  RefreshTask();
  UnlockTaskData();
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnAppendInTask1Clicked(WndButton* pWnd){
  LockTaskData();
  InsertWaypoint(SelectedWaypoint, 1); // append before finish
  RefreshTask();
  UnlockTaskData();
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnAppendInTask2Clicked(WndButton* pWnd){
  LockTaskData();
  InsertWaypoint(SelectedWaypoint, 2); // append after finish
  RefreshTask();
  UnlockTaskData();
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void OnRemoveFromTaskClicked(WndButton* pWnd){
  LockTaskData();
  RemoveWaypoint(SelectedWaypoint);
  RefreshTask();
  UnlockTaskData();
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnNextClicked),
  ClickNotifyCallbackEntry(OnPrevClicked),
  OnPaintCallbackEntry(OnPaintDetailsListItem),
  OnListCallbackEntry(OnDetailsListInfo),
  OnPaintCallbackEntry(OnPaintWaypointPicto),
  OnPaintCallbackEntry(OnPaintWpCommentListItem),
  OnListCallbackEntry(OnWpCommentListInfo),
  EndCallBackEntry()
};


static void OnCommentEnter(WindowControl * Sender,
                                       WndListFrame::ListInfo_t *ListInfo) {
  int  ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;

  if(ItemIndex >=0) {
    if(RadioPara.Enabled) {
      unsigned khz = ExtractFrequency(aCommentTextLine[ItemIndex]);
      if (ValidFrequency(khz)) {
        LKSound(TEXT("LK_TICK.WAV"));
        dlgRadioPriSecSelShowModal(aCommentTextLine[ItemIndex], khz);
      }
    }
  }
}

void dlgWayPointDetailsShowModal(short mypage){

  TCHAR sTmp[128];
  WndProperty *wp;

  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_WAYPOINTDETAILS_L : IDR_XML_WAYPOINTDETAILS_P);


  if (!wf) return;

  wInfo    = (wf->FindByName<WndFrame>(TEXT("frmInfos")));
  wCommand = (wf->FindByName<WndFrame>(TEXT("frmCommands")));
  wSpecial = (wf->FindByName<WndFrame>(TEXT("frmSpecial")));
  wDetails = wf->FindByName<WndListFrame>(TEXT("frmDetails"));
  wComment = wf->FindByName<WndListFrame>(TEXT("frmWpComment"));
  wCommentEntry = wf->FindByName<WndOwnerDrawFrame>(TEXT("frmWpCommentEntry"));

  LKASSERT(wInfo!=NULL);
  LKASSERT(wCommand!=NULL);
  LKASSERT(wSpecial!=NULL);
  LKASSERT(wDetails!=NULL);
  LKASSERT(wComment!=NULL);
  LKASSERT(wCommentEntry!=NULL);

  wComment->SetEnterCallback(OnCommentEnter);

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
  LKASSERT(SelectedWaypoint>=0);

  // if SeeYou waypoint and it is landable
  if ((WPLSEL.Format == LKW_CUP  || WPLSEL.Format == LKW_OPENAIP) &&  WPLSEL.Style >= STYLE_AIRFIELDGRASS && WPLSEL.Style <= STYLE_AIRFIELDSOLID) {
     TCHAR ttmp[50];
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
			_stprintf(ttmp,_T("%.0f%s"),Units::ToAltitude((double)WPLSEL.RunwayLen), Units::GetAltitudeName());
			_tcscat(sTmp, ttmp);
		}
  } else {
     TCHAR code[20];
     double wpdistance = 0;
     double wpbearing = 0;

     if (TeamCodeRefWaypoint >= 0) {
        DistanceBearing(WayPointList[TeamCodeRefWaypoint].Latitude,
                  WayPointList[TeamCodeRefWaypoint].Longitude,
                  WayPointList[SelectedWaypoint].Latitude,
                  WayPointList[SelectedWaypoint].Longitude,
                  &wpdistance, &wpbearing);

        GetTeamCode(code,wpbearing, wpdistance);
	_stprintf(sTmp, TEXT("%s: %s  (%s)"), wf->GetCaption(), WayPointList[SelectedWaypoint].Name, code);
     } else {
	_stprintf(sTmp, TEXT("%s: %s"), wf->GetCaption(), WayPointList[SelectedWaypoint].Name);
     }
  }
  wf->SetCaption(sTmp);

  CommentDrawListIndex=0;
  aCommentTextLine.clear();

  wComment->SetBorderKind(BORDERLEFT);

  LKASSERT(wCommentEntry);
  wCommentEntry->SetCanFocus(true);

  {
    LKWindowSurface Surface(*wCommentEntry);
    Surface.SelectObject(wCommentEntry->GetFont());
    aCommentTextLine.update(Surface, wCommentEntry->GetWidth(), WayPointList[SelectedWaypoint].Comment );
  }


  //
  // Lat and Lon
  //
  Units::CoordinateToString(WayPointList[SelectedWaypoint].Longitude, WayPointList[SelectedWaypoint].Latitude, sTmp);

  wp = (wf->FindByName<WndProperty>(TEXT("prpCoordinate")));
  LKASSERT(wp);
  wp->SetText(sTmp);

  //
  // Waypoint Altitude
  //
  Units::FormatAltitude(WayPointList[SelectedWaypoint].Altitude, sTmp, sizeof(sTmp)-1);
  wp = (wf->FindByName<WndProperty>(TEXT("prpAltitude")));
  LKASSERT(wp);
  wp->SetText(sTmp);

  //
  // SUNSET at waypoint
  //
  const unsigned sunset_time = DoSunEphemeris(WayPointList[SelectedWaypoint].Longitude,
                                              WayPointList[SelectedWaypoint].Latitude);
  Units::TimeToText(sTmp, sunset_time);
  (wf->FindByName<WndProperty>(TEXT("prpSunset")))->SetText(sTmp);


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
  if (ScreenLandscape) {
      Units::FormatDistance(distance, DistanceText, 10);

      if ( Units::GetDistanceUnit() == unNauticalMiles ||
           Units::GetDistanceUnit() == unStatuteMiles ) {

          _stprintf(sTmp,_T("  (%.1fkm)"), Units::To(unKiloMeter, distance));
      } else {
	  _stprintf(sTmp,_T("  (%.1fnm)"), Units::To(unNauticalMiles, distance));
      }
      _tcscat(DistanceText,sTmp);
  } else {
      Units::FormatDistance(distance, DistanceText, 10);
  }
  (wf->FindByName<WndProperty>(TEXT("prpDistance")))->SetText(DistanceText);

  if (ScreenLandscape) {
      _stprintf(sTmp, TEXT("%d%s  (R:%d%s)"),iround(bearing), MsgToken<2179>(),
         iround(AngleLimit360(bearing+180)), MsgToken<2179>());
  } else {
      _stprintf(sTmp, TEXT("%d%s"), iround(bearing),MsgToken<2179>());
  }
  (wf->FindByName<WndProperty>(TEXT("prpBearing")))->SetText(sTmp);


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

  _stprintf(sTmp, TEXT("%.0f %s"),
            Units::ToAltitude(alt),
            Units::GetAltitudeName());

  wp = (wf->FindByName<WndProperty>(TEXT("prpMc0")));
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

  _stprintf(sTmp, TEXT("%.0f %s"),
            Units::ToAltitude(alt),
            Units::GetAltitudeName());

  wp = (wf->FindByName<WndProperty>(TEXT("prpMc2")));
  if (wp) {
	wp->SetText(sTmp);
  }


  wf->SetKeyDownNotify(FormKeyDown);

  (wf->FindByName<WndButton>(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  // We DONT use PREV  anymore
  (wf->FindByName<WndButton>(TEXT("cmdPrev")))->SetVisible(false);


  //
  // Details (WAYNOTES) page
  //
  DetailDrawListIndex=0;
  aDetailTextLine.clear();

  wDetailsEntry = wf->FindByName<WndOwnerDrawFrame>(TEXT("frmDetailsEntry"));
  LKASSERT(wDetailsEntry!=NULL);
  wDetailsEntry->SetCanFocus(true);

  {
    LKWindowSurface Surface(*wDetailsEntry);
    Surface.SelectObject(wDetailsEntry->GetFont());
    aDetailTextLine.update(Surface, wDetailsEntry->GetWidth(), WayPointList[SelectedWaypoint].Details );
  }

  WndButton *wb;

  TCHAR captmp[200];

  // Resize also buttons
  wb = (wf->FindByName<WndButton>(TEXT("cmdInserInTask")));
  if (wb) {
    wb->SetOnClickNotify(OnInserInTaskClicked);
    wb->SetWidth(wCommand->GetWidth()-wb->GetLeft()*2);

    if ((ActiveTaskPoint<0) || !ValidTaskPoint(0)) {
	// this is going to be the first tp (ActiveTaskPoint 0)
	_stprintf(captmp,_T("%s"),MsgToken<1824>()); // insert as START
    } else {
	LKASSERT(ActiveTaskPoint>=0 && ValidTaskPoint(0));
	int indexInsert = max(ActiveTaskPoint,0); // safe check
	if (indexInsert==0) {
		_stprintf(captmp,_T("%s"),MsgToken<1824>()); // insert as START
	} else {
		LKASSERT(ValidWayPoint(Task[indexInsert].Index));
		_stprintf(captmp,_T("%s <%s>"),MsgToken<1825>(),WayPointList[ Task[indexInsert].Index ].Name); // insert before xx
	}
    }
    wb->SetCaption(captmp);
  }

  wb = (wf->FindByName<WndButton>(TEXT("cmdAppendInTask1")));
  if (wb) {
    wb->SetOnClickNotify(OnAppendInTask1Clicked);
    wb->SetWidth(wCommand->GetWidth()-wb->GetLeft()*2);
  }

  wb = (wf->FindByName<WndButton>(TEXT("cmdAppendInTask2")));
  if (wb) {
    wb->SetOnClickNotify(OnAppendInTask2Clicked);
    wb->SetWidth(wCommand->GetWidth()-wb->GetLeft()*2);
  }

  wb = (wf->FindByName<WndButton>(TEXT("cmdRemoveFromTask")));
  if (wb) {
    wb->SetOnClickNotify(OnRemoveFromTaskClicked);
    wb->SetWidth(wCommand->GetWidth()-wb->GetLeft()*2);
  }

  wb = (wf->FindByName<WndButton>(TEXT("cmdReplace")));
  if (wb) {
    wb->SetWidth(wCommand->GetWidth()-wb->GetLeft()*2);

    int tmpIdx =  -1;
    if (ValidTaskPoint(ActiveTaskPoint))
      tmpIdx = Task[ActiveTaskPoint].Index;
    if(  ValidTaskPoint(PanTaskEdit))
     tmpIdx = RealActiveWaypoint;
    if(tmpIdx != -1)
    {
	wb->SetOnClickNotify(OnReplaceClicked);
	_stprintf(captmp,_T("%s <%s>"),MsgToken<1826>(),WayPointList[tmpIdx ].Name); // replace  xx
    } else {
	_stprintf(captmp,_T("( %s )"),MsgToken<555>());
    }
    wb->SetCaption(captmp);
  }


  wb = (wf->FindByName<WndButton>(TEXT("cmdNewHome")));
  if (wb)  {
    wb->SetOnClickNotify(OnNewHomeClicked);
    wb->SetWidth(wSpecial->GetWidth()-wb->GetLeft()*2);
  }

  wb = (wf->FindByName<WndButton>(TEXT("cmdTeamCode")));
  if (wb) {
    wb->SetOnClickNotify(OnTeamCodeClicked);
    wb->SetWidth(wSpecial->GetWidth()-wb->GetLeft()*2);
  }

  if(WayPointList[SelectedWaypoint].Format == LKW_VIRTUAL)
  {
    WindowControl*pWnd = wf->FindByName(TEXT("cmdNext"));
    if(pWnd) {    pWnd->SetVisible(false);}
    pWnd = wf->FindByName(TEXT("cmdPrev"));
    if(pWnd) {    pWnd->SetVisible(false);}
  }
  page = mypage;

  NextPage(0);
  wComment->ResetList();
  wComment->Redraw();
  wf->ShowModal();

  delete wf;

  aCommentTextLine.clear();

  wf = NULL;

}
