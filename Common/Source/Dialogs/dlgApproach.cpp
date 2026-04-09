/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   Approach dialog: runway choice (and direct leg length). Mode is always direct until circuit exists.
   Selected button = inverted (dark) style. Timer keeps runway selection UI in sync if state changes.
*/

#include "externs.h"
#include "dlgTools.h"
#include <memory>
#include "Dialogs.h"
#include "WindowControls.h"
#include "Event/Event.h"
#include "resource.h"
#include "MapWindow.h"
#include "NavFunctions.h"
#include "Waypointparser.h"
#include "Logger.h"
#include "Units.h"
#include "LKLanguage.h"

extern void ResetTaskWaypoint(int j);

static unsigned dlgSize = 0;
static int runway_heading_1 = 90;
static int runway_heading_2 = 270;

/// Request map redraw when in Approach pan mode.
static void RefreshMapApproach() {
  if (MapWindow::mode.Is(MapWindow::Mode::MODE_APPROACH_PAN)) {
    MapWindow::RefreshMap();
  }
}

/// Re-apply selected (highlighted) state for runway buttons.
static void RefreshApproachButtonStyles(WndForm* form) {
  if (!form) return;

  WndButton* btnRunway1 = form->FindByName<WndButton>(TEXT("btnRunway1"));
  WndButton* btnRunway2 = form->FindByName<WndButton>(TEXT("btnRunway2"));

  const bool rw1 = (MapApproachRunwayDir == runway_heading_1);
  const bool rw2 = (MapApproachRunwayDir == runway_heading_2);
  if (btnRunway1) btnRunway1->SetSelected(rw1);
  if (btnRunway2) btnRunway2->SetSelected(rw2);
}

/// Timer: keep runway highlight in sync if shared state changes while the dialog is open.
static bool OnApproachTimerNotify(WndForm* pWnd) {
  RefreshApproachButtonStyles(pWnd);
  return true;
}

/// Close Approach dialog with OK.
static void OnApproachOKClicked(WndButton* pWnd) {
  if (pWnd) {
    WndForm* pForm = pWnd->GetParentWndForm();
    if (pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

/// Set selected runway to first heading (e.g. 16).
static void OnRunway1Clicked(WndButton* pWnd) {
  MapApproachRunwayDir = runway_heading_1;
  if (pWnd) RefreshApproachButtonStyles(pWnd->GetParentWndForm());
  RefreshMapApproach();
}

/// Set selected runway to second heading (e.g. 34).
static void OnRunway2Clicked(WndButton* pWnd) {
  MapApproachRunwayDir = runway_heading_2;
  if (pWnd) RefreshApproachButtonStyles(pWnd->GetParentWndForm());
  RefreshMapApproach();
}

static void ResetApproachDirectDistanceToDefault() {
  if (Units::GetDistanceUnit() == unNauticalMiles) {
    MapApproachDirectDistance_m = Units::FromDistance(3.0);
  } else {
    MapApproachDirectDistance_m = 5000.0;
  }
}

static void ClampApproachDirectDistanceFromUserUnits() {
  double u = Units::ToDistance(MapApproachDirectDistance_m);
  if (Units::GetDistanceUnit() == unNauticalMiles) {
    u = max(0.5, min(50.0, u));
  } else {
    u = max(1.0, min(50.0, u));
  }
  MapApproachDirectDistance_m = Units::FromDistance(u);
}

/// Distance of direct leg outer point (same control as waypoint list: inc/dec arrows, no drag).
static void OnApproachDistanceData(DataField* Sender, DataField::DataAccessKind_t Mode) {
  const Units_t du = Units::GetDistanceUnit();
  const double step_u = (du == unNauticalMiles) ? 0.5 : 1.0;

  switch (Mode) {
    case DataField::daGet:
      Sender->SetAsFloat(Units::ToDistance(MapApproachDirectDistance_m));
      break;
    case DataField::daPut:
    case DataField::daChange: {
      MapApproachDirectDistance_m = Units::FromDistance(Sender->GetAsFloat());
      ClampApproachDirectDistanceFromUserUnits();
      MapWindow::SyncApproachZoomFromDirectLeg();
      Sender->SetAsFloat(Units::ToDistance(MapApproachDirectDistance_m));
      RefreshMapApproach();
      break;
    }
    case DataField::daInc: {
      double u = Units::ToDistance(MapApproachDirectDistance_m) + step_u;
      MapApproachDirectDistance_m = Units::FromDistance(u);
      ClampApproachDirectDistanceFromUserUnits();
      MapWindow::SyncApproachZoomFromDirectLeg();
      Sender->SetAsFloat(Units::ToDistance(MapApproachDirectDistance_m));
      RefreshMapApproach();
      break;
    }
    case DataField::daDec: {
      double u = Units::ToDistance(MapApproachDirectDistance_m) - step_u;
      MapApproachDirectDistance_m = Units::FromDistance(u);
      ClampApproachDirectDistanceFromUserUnits();
      MapWindow::SyncApproachZoomFromDirectLeg();
      Sender->SetAsFloat(Units::ToDistance(MapApproachDirectDistance_m));
      RefreshMapApproach();
      break;
    }
    default:
      break;
  }
}

/// Approve popup: confirm creating the approach task and close with OK.
static void OnApproachApproveConfirmClicked(WndButton* pWnd) {
  if (pWnd) {
    WndForm* pForm = pWnd->GetParentWndForm();
    if (pForm) pForm->SetModalResult(mrOK);
  }
}

/// Approve popup: cancel (ignore) and close without creating task.
static void OnApproachApproveIgnoreClicked(WndButton* pWnd) {
  if (pWnd) {
    WndForm* pForm = pWnd->GetParentWndForm();
    if (pForm) pForm->SetModalResult(mrCancel);
  }
}

static CallBackTableEntry_t ApproveCallBackTable[] = {
  CallbackEntry(OnApproachApproveConfirmClicked),
  CallbackEntry(OnApproachApproveIgnoreClicked),
  EndCallbackEntry()
};

/// Show approval warning popup; returns true if user clicked Approve, false if IGNORE!!
static bool ShowApproachApproveDialog() {
  std::unique_ptr<WndForm> pf(dlgLoadFromXML(ApproveCallBackTable,
      ScreenLandscape ? IDR_XML_APPROACH_APPROVE_L : IDR_XML_APPROACH_APPROVE_P));
  if (!pf) return false;

  const int fw = pf->GetWidth();
  const int fh = pf->GetHeight();
  pf->SetLeft((ScreenSizeX - fw) / 2);
  pf->SetTop((ScreenSizeY - fh) / 2);

  WndProperty* prpMessage = pf->FindByName<WndProperty>(TEXT("prpMessage"));
  if (prpMessage) {
    prpMessage->SetText(MsgToken<2520>());
  }

  WndButton* btnIgnore = pf->FindByName<WndButton>(TEXT("btnIgnore"));
  if (btnIgnore) btnIgnore->SetSelected(true);

  const int res = pf->ShowModal();
  return (res == mrOK);
}

/// Create two-point approach task (DIRECT nn + airfield) after user confirms in popup.
static void OnApproveClicked(WndButton* pWnd) {
  if (!ShowApproachApproveDialog()) return;
  if (!CheckDeclaration()) return;

  const int approach_wp = MapApproachWaypoint;
  WAYPOINT centre = {};
  {
    LockTaskData();
    if (approach_wp < 0 || !ValidWayPointFast(approach_wp)) {
      UnlockTaskData();
      return;
    }
    centre = WayPointList[approach_wp];
    UnlockTaskData();
  }

  int rw_dir = MapApproachRunwayDir >= 0 ? MapApproachRunwayDir : centre.RunwayDir;
  if (rw_dir < 0) rw_dir = 0;
  const double rw_recip = AngleLimit360(static_cast<double>(rw_dir) + 180.0);

  double if_lat, if_lon;
  const double leg_m = max(100.0, MapApproachDirectDistance_m);
  FindLatitudeLongitude(centre.Latitude, centre.Longitude, rw_recip, leg_m, &if_lat, &if_lon);

  WAYPOINT if_wp = {};
  if_wp.Latitude = if_lat;
  if_wp.Longitude = if_lon;
  if_wp.Altitude = centre.Altitude;
  if_wp.FileNum = -1;
  lk::snprintf(if_wp.Name, NAME_SIZE, _T("DIRECT %02d"), rw_dir / 10);

  if (!AddWaypoint(if_wp)) return;
  const int if_index = (int)WayPointList.size() - 1;

  LockTaskData();
  gTaskType = task_type_t::DEFAULT;
  ClearTask();
  ResetTaskWaypoint(0);
  Task[0].Index = if_index;
  ResetTaskWaypoint(1);
  Task[1].Index = approach_wp;
  ActiveTaskPoint = 0;
  RefreshTask();
  UnlockTaskData();

  if (pWnd) {
    WndForm* pForm = pWnd->GetParentWndForm();
    if (pForm) pForm->SetModalResult(mrOK);
  }
}

static CallBackTableEntry_t CallBackTable[] = {
  CallbackEntry(OnApproachOKClicked),
  CallbackEntry(OnRunway1Clicked),
  CallbackEntry(OnRunway2Clicked),
  CallbackEntry(OnApproachDistanceData),
  CallbackEntry(OnApproveClicked),
  EndCallbackEntry()
};

/// Open Approach dialog for the given waypoint; enables map overlay and runway/task setup.
void dlgApproach(int waypoint_index) {
  if (waypoint_index < 0 || !ValidWayPointFast(waypoint_index)) {
    return;
  }

  const WAYPOINT& wp = WayPointList[waypoint_index];
  if (wp.RunwayDir >= 0) {
    int h1 = std::lrint(wp.RunwayDir / 10.0);
    if (h1 <= 0) h1 = 36;
    int h2 = (h1 > 18) ? h1 - 18 : h1 + 18;
    runway_heading_1 = h1 * 10;
    runway_heading_2 = h2 * 10;
    if (MapApproachRunwayDir < 0) {
      MapApproachRunwayDir = runway_heading_1;
    }
  } else {
    runway_heading_1 = 90;
    runway_heading_2 = 270;
    MapApproachRunwayDir = runway_heading_1;
  }

  std::unique_ptr<WndForm> wf(dlgLoadFromXML(CallBackTable,
      ScreenLandscape ? IDR_XML_APPROACH_L : IDR_XML_APPROACH_P));
  if (!wf) {
    return;
  }

  MapApproachMode = 0;
  MapApproachEnabled = true;
  MapApproachWaypoint = waypoint_index;
  ResetApproachDirectDistanceToDefault();
  MapWindow::SyncApproachZoomFromDirectLeg();
  WndProperty* prpDist = wf->FindByName<WndProperty>(TEXT("prpApproachDistance"));
  if (prpDist && prpDist->GetDataField()) {
    prpDist->GetDataField()->SetUnits(Units::GetDistanceName());
    prpDist->RefreshDisplay();
  }

  if (ScreenLandscape) {
    const PixelRect rc(main_window->GetClientRect());
    const int client_h = rc.GetSize().cy;
    const unsigned max_h = (unsigned)max(1, client_h);
    wf->SetHeight(max_h);
    dlgSize = wf->GetWidth();
    wf->SetTop(rc.top);
#ifdef KOBO
    wf->SetLeft(rc.left + rc.GetSize().cx - (int)dlgSize - NIBLSCALE(8));
#else
    wf->SetLeft(rc.left + rc.GetSize().cx - (int)dlgSize);
#endif
    wf->SetCaption(wf->GetWndText());
  } else {
    WndButton* btnApprove = wf->FindByName<WndButton>(TEXT("btnApprove"));
    int maxBottom = 0;
    if (btnApprove) {
      maxBottom = (int)btnApprove->GetTop() + (int)btnApprove->GetHeight();
    }
    if (maxBottom > 0) {
      const unsigned needClient = (unsigned)maxBottom + (unsigned)NIBLSCALE(10);
      WindowControl* const client = wf->GetClientArea();
      const int delta = (int)needClient - (int)client->GetHeight();
      if (delta > 0) {
        wf->SetHeight(wf->GetHeight() + (unsigned)delta);
      }
    }
    dlgSize = wf->GetHeight();
#if defined(__linux__) && !defined(ANDROID)
    dlgApplyPortraitOverlayGeometry(wf.get());
    dlgSize = wf->GetHeight();
#else
    wf->SetLeft(0);
    wf->SetTop(0);
#endif
  }

  TCHAR cap1[8], cap2[8];
  lk::snprintf(cap1, _T("%02d"), runway_heading_1 / 10);
  lk::snprintf(cap2, _T("%02d"), runway_heading_2 / 10);

  WndButton* btn1 = wf->FindByName<WndButton>(TEXT("btnRunway1"));
  if (btn1) btn1->SetCaption(cap1);
  WndButton* btn2 = wf->FindByName<WndButton>(TEXT("btnRunway2"));
  if (btn2) btn2->SetCaption(cap2);

  RefreshApproachButtonStyles(wf.get());

  MapWindow::SetApproachPan(true, waypoint_index, dlgSize);
  MapWindow::RefreshMap();

  wf->SetTimerNotify(400, OnApproachTimerNotify);
  wf->ShowModal();
  wf->SetTimerNotify(0, nullptr);

  MapWindow::SetApproachPan(false, 0);
  MapApproachEnabled = false;
  MapApproachWaypoint = -1;
  MapApproachRunwayDir = -1;
}
