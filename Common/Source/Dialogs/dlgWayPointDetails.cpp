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
#include "utils/printf.h"


namespace {

void OnPaintWaypointPicto(WndOwnerDrawFrame * Sender, LKSurface& Surface) {
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

void SetVisible(WndForm* pForm, const TCHAR* szName, bool bVisible) {
  auto pWnd = pForm->FindByName(szName);
  if (pWnd) {
    pWnd->SetVisible(bVisible);

    auto plistFrame = dynamic_cast<WndListFrame*>(pWnd);
    if (plistFrame) {
      plistFrame->ResetList();
      plistFrame->Redraw();
    }
  }
}

void UpdateVisiblePage(WndForm* pForm, int page) {
  SetVisible(pForm, _T("frmInfos"), (page == 0));
  SetVisible(pForm, _T("frmDetails"), (page == 1));
  SetVisible(pForm, _T("frmCommands"), (page == 2));
  SetVisible(pForm, _T("frmSpecial"), (page == 3));
}

template<int step>
void NextPage(WndForm* pForm, int& page) {
  page += step;
  for (bool page_ok = false; !page_ok;) {
    if (page < 0) {
      page = 3;
    }
    if (page > 3) {
      page = 0;
    }
    if (page == 1 && !WayPointList[SelectedWaypoint].Details) {
      page += step;
    }
    else {
      page_ok = true;
    }
  }
  UpdateVisiblePage(pForm, page);
}

void OnPaintDetailsListItem(WndOwnerDrawFrame* Sender,
                                   LKSurface& Surface,
                                   const TextWrapArray& aDetailTextLine) {

  auto pWndList = dynamic_cast<WndListFrame*>(Sender->GetParent());
  if (!pWndList) {
    return;
  }
  size_t DrawListIndex = pWndList->GetDrawIndex();

  if (DrawListIndex < aDetailTextLine.size()) {
    const TCHAR* szText = aDetailTextLine[DrawListIndex];
    Surface.SetTextColor(RGB_BLACK);
    Surface.DrawText(DLGSCALE(2), DLGSCALE(2), szText);
  }
}

void OnDetailsListInfo(WndListFrame* Sender,
                              WndListFrame::ListInfo_t* ListInfo,
                              const TextWrapArray& aDetailTextLine) {
  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = aDetailTextLine.size();
  }
}

void OnPaintWpCommentListItem(WndOwnerDrawFrame* Sender,
                                     LKSurface& Surface,
                                     const TextWrapArray& aCommentTextLine) {
  auto pWndList = dynamic_cast<WndListFrame*>(Sender->GetParent());
  if (!pWndList) {
    return;
  }

  size_t DrawListIndex = pWndList->GetDrawIndex();
  if (DrawListIndex < aCommentTextLine.size()) {
    const TCHAR* szText = aCommentTextLine[DrawListIndex];
    Surface.SetTextColor(RGB_BLACK);
    Surface.DrawText(DLGSCALE(2), DLGSCALE(2), szText);

    size_t pos, len;
    unsigned khz = ExtractFrequency(szText, &pos, &len);

    if ((khz > 0) && ((pos + len) < 255)) {
      TCHAR sTmp[255];
      // copy text until end of substring
      TCHAR* end =
          std::copy_n(aCommentTextLine[DrawListIndex], pos + len, sTmp);
      *end = _T('\0');
      // size of the text is end of underline
      const int subend = Surface.GetTextWidth(sTmp) + DLGSCALE(2);
      // truncate string to start of substring
      sTmp[pos] = 0;
      // size of the text is start of underline
      const int substart = Surface.GetTextWidth(sTmp) + DLGSCALE(2);

      if (substart < subend) {
        int h = Sender->GetHeight() - IBLSCALE(1);
        const auto hOldPen = Surface.SelectObject(LKPen_Black_N1);
        Surface.DrawLine(substart, h, subend, h);
        Surface.SelectObject(hOldPen);
      }
    }
  }
}

void OnWpCommentListInfo(WndListFrame* Sender,
                                WndListFrame::ListInfo_t* ListInfo,
                                const TextWrapArray& aCommentTextLine) {
  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = aCommentTextLine.size();
  }
}

void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

bool FormKeyDown(WndForm* pWnd, unsigned KeyCode, int& page) {
    Window * pBtn = nullptr;

    switch (KeyCode & 0xffff) {
        case KEY_LEFT:
        case '6':
            pBtn = pWnd->FindByName(TEXT("cmdPrev"));
            NextPage<-1>(pWnd, page);
            break;
        case KEY_RIGHT:
        case '7':
            pBtn = pWnd->FindByName(TEXT("cmdNext"));
            NextPage<1>(pWnd, page);
            break;
    }
    if (pBtn) {
        pBtn->SetFocus();
        return true;
    }

    return false;
}

void OnReplaceClicked(WndButton* pWnd){
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

void OnNewHomeClicked(WndButton* pWnd){
  SetNewHome(SelectedWaypoint);

  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

void OnTeamCodeClicked(WndButton* pWnd){
  TeamCodeRefWaypoint = SelectedWaypoint;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

void OnInserInTaskClicked(WndButton* pWnd){
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

void OnAppendInTask1Clicked(WndButton* pWnd){
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

void OnAppendInTask2Clicked(WndButton* pWnd){
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

void OnRemoveFromTaskClicked(WndButton* pWnd){
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

void OnCommentEnter(WindowControl* Sender,
                           WndListFrame::ListInfo_t* ListInfo,
                           const TextWrapArray& aCommentTextLine) {
  int ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;

  if (ItemIndex >= 0) {
    if (RadioPara.Enabled) {
      unsigned khz = ExtractFrequency(aCommentTextLine[ItemIndex]);
      if (ValidFrequency(khz)) {
        LKSound(TEXT("LK_TICK.WAV"));
        dlgRadioPriSecSelShowModal(aCommentTextLine[ItemIndex], khz);
      }
    }
  }
}

}  // namespace

void dlgWayPointDetailsShowModal(int page) {
  if (!ValidWayPoint(SelectedWaypoint)) {
    return;
  }
  const WAYPOINT& WPLSEL = WayPointList[SelectedWaypoint];

  TextWrapArray aDetailTextLine;
  TextWrapArray aCommentTextLine;

  const CallBackTableEntry_t CallBackTable[] = {
      callback_entry("OnNextClicked",
                     [&](WndButton* pWnd) {
                       NextPage<1>(pWnd->GetParentWndForm(), page);
                     }),
      callback_entry("OnPrevClicked",
                     [&](WndButton* pWnd) {
                       NextPage<-1>(pWnd->GetParentWndForm(), page);
                     }),
      callback_entry("OnPaintDetailsListItem",
                     [&](WndOwnerDrawFrame* Sender, LKSurface& Surface) {
                       OnPaintDetailsListItem(Sender, Surface, aDetailTextLine);
                     }),
      callback_entry(
          "OnDetailsListInfo",
          [&](WndListFrame* Sender, WndListFrame::ListInfo_t* ListInfo) {
            OnDetailsListInfo(Sender, ListInfo, aDetailTextLine);
          }),
      callback_entry("OnPaintWpCommentListItem",
                     [&](WndOwnerDrawFrame* Sender, LKSurface& Surface) {
                       OnPaintWpCommentListItem(Sender, Surface,
                                                aCommentTextLine);
                     }),
      callback_entry(
          "OnWpCommentListInfo",
          [&](WndListFrame* Sender, WndListFrame::ListInfo_t* ListInfo) {
            OnWpCommentListInfo(Sender, ListInfo, aCommentTextLine);
          }),
      CallbackEntry(OnPaintWaypointPicto),
      EndCallbackEntry()};

  TCHAR sTmp[128];
  WndProperty *wp;

  std::unique_ptr<WndForm> wf(dlgLoadFromXML(
      CallBackTable,
      ScreenLandscape ? IDR_XML_WAYPOINTDETAILS_L : IDR_XML_WAYPOINTDETAILS_P));

  if (!wf) {
    return;
  }

  auto wInfo = wf->FindByName<WndFrame>(_T("frmInfos"));
  if (wInfo) {
    // Resize Frames up to real screen size on the right.
    wInfo->SetBorderKind(BORDERLEFT);
  }
  auto wCommand = wf->FindByName<WndFrame>(_T("frmCommands"));
  if (wCommand) {
    wCommand->SetBorderKind(BORDERLEFT);
    wCommand->SetVisible(false);
  }
  auto wSpecial = wf->FindByName<WndFrame>(_T("frmSpecial"));
  if (wSpecial) {
    wSpecial->SetBorderKind(BORDERLEFT);
    wSpecial->SetVisible(false);
  }

  auto wDetails = wf->FindByName<WndListFrame>(_T("frmDetails"));
  if (wDetails) {
    wDetails->SetBorderKind(BORDERLEFT);
  }
  auto wComment = wf->FindByName<WndListFrame>(_T("frmWpComment"));
  if (wComment) {
    wComment->SetBorderKind(BORDERLEFT);
    wComment->SetEnterCallback(
        [&](WindowControl* Sender, WndListFrame::ListInfo_t* ListInfo) {
          OnCommentEnter(Sender, ListInfo, aCommentTextLine);
        });
  }
  
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
                       WPLSEL.Latitude, WPLSEL.Longitude, &wpdistance,
                       &wpbearing);

       GetTeamCode(code, wpbearing, wpdistance);
       _stprintf(sTmp, TEXT("%s: %s  (%s)"), wf->GetCaption(),
                 WPLSEL.Name, code);
     } else {
	_stprintf(sTmp, TEXT("%s: %s"), wf->GetCaption(), WPLSEL.Name);
     }
  }
  wf->SetCaption(sTmp);

  aCommentTextLine.clear();

  auto wCommentEntry =
      wf->FindByName<WndOwnerDrawFrame>(_T("frmWpCommentEntry"));
  if (wCommentEntry) {
    wCommentEntry->SetCanFocus(true);

    LKWindowSurface Surface(*wCommentEntry);
    Surface.SelectObject(wCommentEntry->GetFont());
    aCommentTextLine.update(Surface, wCommentEntry->GetWidth(), WPLSEL.Comment);
  }

  //
  // Lat and Lon
  //
  Units::CoordinateToString(WPLSEL.Longitude, WPLSEL.Latitude, sTmp);
  wp = (wf->FindByName<WndProperty>(TEXT("prpCoordinate")));
  if (wp) {
    wp->SetText(sTmp);
  }
  //
  // Waypoint Altitude
  //
  Units::FormatAltitude(WPLSEL.Altitude, sTmp, std::size(sTmp));
  wp = (wf->FindByName<WndProperty>(TEXT("prpAltitude")));
  if (wp) {
    wp->SetText(sTmp);
  }
  //
  // SUNSET at waypoint
  //
  const unsigned sunset_time =
      DoSunEphemeris(WPLSEL.Longitude, WPLSEL.Latitude);
  Units::TimeToText(sTmp, sunset_time);
  wp = wf->FindByName<WndProperty>(TEXT("prpSunset"));
  if (wp) {
    wp->SetText(sTmp);
  }

  //
  // Distance and bearing
  //
  double distance, bearing;
  DistanceBearing(GPS_INFO.Latitude, GPS_INFO.Longitude, WPLSEL.Latitude,
                  WPLSEL.Longitude, &distance, &bearing);

  TCHAR DistanceText[MAX_PATH];
  if (ScreenLandscape) {
    Units::FormatDistance(distance, DistanceText, 10);

    if (Units::GetDistanceUnit() == unNauticalMiles ||
        Units::GetDistanceUnit() == unStatuteMiles) {
      lk::snprintf(sTmp, _T("  (%.1fkm)"), Units::To(unKiloMeter, distance));
    } else {
      lk::snprintf(sTmp, _T("  (%.1fnm)"),
                   Units::To(unNauticalMiles, distance));
      }
      _tcscat(DistanceText, sTmp);
  }
  else {
    Units::FormatDistance(distance, DistanceText, 10);
  }
  (wf->FindByName<WndProperty>(TEXT("prpDistance")))->SetText(DistanceText);

  if (ScreenLandscape) {
    lk::snprintf(sTmp, _T("%d%s  (R:%d%s)"), iround(bearing), MsgToken<2179>(),
                 iround(AngleLimit360(bearing + 180)), MsgToken<2179>());
  }
  else {
    lk::snprintf(sTmp, TEXT("%d%s"), iround(bearing), MsgToken<2179>());
  }
  (wf->FindByName<WndProperty>(TEXT("prpBearing")))->SetText(sTmp);

  //
  // Altitude reqd at mc 0
  //
  double alt = CALCULATED_INFO.NavAltitude -
               GlidePolar::MacCreadyAltitude(
                   0.0, distance, bearing, CALCULATED_INFO.WindSpeed,
                   CALCULATED_INFO.WindBearing, 0, 0, true, 0);

  alt -= WPLSEL.Altitude;

  if (SafetyAltitudeMode == 1 || WayPointCalc[SelectedWaypoint].IsLandable) {
    alt -= (SAFETYALTITUDEARRIVAL / 10);
  }

  Units::FormatAltitude(alt, sTmp, std::size(sTmp));

  wp = (wf->FindByName<WndProperty>(TEXT("prpMc0")));
  if (wp) {
    wp->SetText(sTmp);
  }

  // alt reqd at current mc
  alt = CALCULATED_INFO.NavAltitude -
        GlidePolar::MacCreadyAltitude(
            MACCREADY, distance, bearing, CALCULATED_INFO.WindSpeed,
            CALCULATED_INFO.WindBearing, 0, 0, true, 0);

  alt -= WPLSEL.Altitude;

  if (SafetyAltitudeMode == 1 || WayPointCalc[SelectedWaypoint].IsLandable) {
    alt -= (SAFETYALTITUDEARRIVAL / 10);
  }

  Units::FormatAltitude(alt, sTmp, std::size(sTmp));

  wp = (wf->FindByName<WndProperty>(TEXT("prpMc2")));
  if (wp) {
    wp->SetText(sTmp);
  }

  wf->SetKeyDownNotify([&](WndForm* pForm, unsigned KeyCode) {
    return FormKeyDown(pForm, KeyCode, page);
  });

  (wf->FindByName<WndButton>(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  // We DONT use PREV  anymore
  (wf->FindByName<WndButton>(TEXT("cmdPrev")))->SetVisible(false);


  //
  // Details (WAYNOTES) page
  //
  aDetailTextLine.clear();

  auto wDetailsEntry = wf->FindByName<WndOwnerDrawFrame>(TEXT("frmDetailsEntry"));
  LKASSERT(wDetailsEntry!=NULL);
  wDetailsEntry->SetCanFocus(true);

  {
    LKWindowSurface Surface(*wDetailsEntry);
    Surface.SelectObject(wDetailsEntry->GetFont());
    aDetailTextLine.update(Surface, wDetailsEntry->GetWidth(), WPLSEL.Details );
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
    wb->SetWidth(wCommand->GetWidth() - wb->GetLeft() * 2);
  }

  wb = (wf->FindByName<WndButton>(TEXT("cmdAppendInTask2")));
  if (wb) {
    wb->SetOnClickNotify(OnAppendInTask2Clicked);
    wb->SetWidth(wCommand->GetWidth() - wb->GetLeft() * 2);
  }

  wb = (wf->FindByName<WndButton>(TEXT("cmdRemoveFromTask")));
  if (wb) {
    wb->SetOnClickNotify(OnRemoveFromTaskClicked);
    wb->SetWidth(wCommand->GetWidth() - wb->GetLeft() * 2);
  }

  wb = (wf->FindByName<WndButton>(TEXT("cmdReplace")));
  if (wb) {
    wb->SetWidth(wCommand->GetWidth() - wb->GetLeft() * 2);

    int tmpIdx =  -1;
    if (ValidTaskPoint(ActiveTaskPoint))
      tmpIdx = Task[ActiveTaskPoint].Index;
    if(  ValidTaskPoint(PanTaskEdit))
     tmpIdx = RealActiveWaypoint;
    if (tmpIdx != -1) {
      wb->SetOnClickNotify(OnReplaceClicked);
      lk::snprintf(captmp, _T("%s <%s>"), MsgToken<1826>(),
                   WayPointList[tmpIdx].Name);  // replace  xx
    }
    else {
      lk::snprintf(captmp, _T("( %s )"), MsgToken<555>());
    }
    wb->SetCaption(captmp);
  }

  wb = (wf->FindByName<WndButton>(TEXT("cmdNewHome")));
  if (wb)  {
    wb->SetOnClickNotify(OnNewHomeClicked);
    wb->SetWidth(wSpecial->GetWidth() - wb->GetLeft() * 2);
  }

  wb = (wf->FindByName<WndButton>(TEXT("cmdTeamCode")));
  if (wb) {
    wb->SetOnClickNotify(OnTeamCodeClicked);
    wb->SetWidth(wSpecial->GetWidth() - wb->GetLeft() * 2);
  }

  if (WPLSEL.Format == LKW_VIRTUAL) {
    WindowControl* pWnd = wf->FindByName(TEXT("cmdNext"));
    if (pWnd) {
      pWnd->SetVisible(false);
    }
    pWnd = wf->FindByName(TEXT("cmdPrev"));
    if (pWnd) {
      pWnd->SetVisible(false);
    }
  }

  if (page == 1 && !WPLSEL.Details) {
    page = 0;
  }
  UpdateVisiblePage(wf.get(), page);

  wf->ShowModal();
}
