/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   Approach dialog: type (Direct/Circuit), runway, circuit side (left/right).
   Selected button = inverted (dark) style. Draw only when all choices are set.
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

extern void ResetTaskWaypoint(int j);

static WndForm* wf = nullptr;
static unsigned dlgSize = 0;
static int runway_heading_1 = 90;
static int runway_heading_2 = 270;

static void RefreshMapApproach() {
  if (MapWindow::mode.Is(MapWindow::Mode::MODE_APPROACH_PAN)) {
    MapWindow::RefreshMap();
  }
}

// Re-apply selected (highlighted) state: button stays "pushed" to show active choice
static void RefreshApproachButtonStyles() {
  if (!wf) return;

  WndButton* btnDirect = wf->FindByName<WndButton>(TEXT("btnDirect"));
  // Circuit not implemented yet
  // WndButton* btnCircuit = wf->FindByName<WndButton>(TEXT("btnCircuit"));
  WndButton* btnRunway1 = wf->FindByName<WndButton>(TEXT("btnRunway1"));
  WndButton* btnRunway2 = wf->FindByName<WndButton>(TEXT("btnRunway2"));
  // WndButton* btnCircuitLeft = wf->FindByName<WndButton>(TEXT("btnCircuitLeft"));
  // WndButton* btnCircuitRight = wf->FindByName<WndButton>(TEXT("btnCircuitRight"));

  if (btnDirect) btnDirect->SetSelected(MapApproachMode == 0);
  // if (btnCircuit) btnCircuit->SetSelected(MapApproachMode == 1);

  const bool rw1 = (MapApproachRunwayDir == runway_heading_1);
  const bool rw2 = (MapApproachRunwayDir == runway_heading_2);
  if (btnRunway1) btnRunway1->SetSelected(rw1);
  if (btnRunway2) btnRunway2->SetSelected(rw2);

  // Circuit not implemented yet
  // if (btnCircuitLeft) {
  //   btnCircuitLeft->SetVisible(MapApproachMode == 1);
  //   btnCircuitLeft->SetSelected(MapApproachCircuitSide == 0);
  // }
  // if (btnCircuitRight) {
  //   btnCircuitRight->SetVisible(MapApproachMode == 1);
  //   btnCircuitRight->SetSelected(MapApproachCircuitSide == 1);
  // }
}

static bool OnApproachTimerNotify(WndForm* pWnd) {
  RefreshApproachButtonStyles();
  return true;  // keep timer active
}

static void OnApproachOKClicked(WndButton* pWnd) {
  if (pWnd) {
    WndForm* pForm = pWnd->GetParentWndForm();
    if (pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnDirectClicked(WndButton* pWnd) {
  MapApproachMode = 0;
  RefreshApproachButtonStyles();
  RefreshMapApproach();
}

// Circuit not implemented yet
// static void OnCircuitClicked(WndButton* pWnd) {
//   MapApproachMode = 1;
//   RefreshApproachButtonStyles();
//   RefreshMapApproach();
// }

static void OnRunway1Clicked(WndButton* pWnd) {
  MapApproachRunwayDir = runway_heading_1;
  RefreshApproachButtonStyles();
  RefreshMapApproach();
}

static void OnRunway2Clicked(WndButton* pWnd) {
  MapApproachRunwayDir = runway_heading_2;
  RefreshApproachButtonStyles();
  RefreshMapApproach();
}

// Circuit not implemented yet
// static void OnCircuitLeftClicked(WndButton* pWnd) {
//   MapApproachCircuitSide = 0;
//   RefreshApproachButtonStyles();
//   RefreshMapApproach();
// }
// static void OnCircuitRightClicked(WndButton* pWnd) {
//   MapApproachCircuitSide = 1;
//   RefreshApproachButtonStyles();
//   RefreshMapApproach();
// }

// Approve popup: confirm or ignore creating the approach task
static void OnApproachApproveConfirmClicked(WndButton* pWnd) {
  if (pWnd) {
    WndForm* pForm = pWnd->GetParentWndForm();
    if (pForm) pForm->SetModalResult(mrOK);
  }
}

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

// Returns true if user clicked Approve, false if IGNORE!!
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
    prpMessage->SetText(_T("LK8000 creates approach task ignoring traffic and terrain. Pilot situational awareness must be at maximum."));
  }

  WndButton* btnIgnore = pf->FindByName<WndButton>(TEXT("btnIgnore"));
  if (btnIgnore) btnIgnore->SetSelected(true);  // highlighted

  const int res = pf->ShowModal();
  return (res == mrOK);
}

// Distance from runway centre to "punto di intersezione" (far end of direct approach line), metres
static constexpr double APPROACH_IF_DISTANCE_M = 5000.0;

static void OnApproveClicked(WndButton* pWnd) {
  if (!ShowApproachApproveDialog()) return;  // user chose IGNORE!!
  if (!CheckDeclaration()) return;

  const int approach_wp = MapApproachWaypoint;
  if (approach_wp < 0 || !ValidWayPointFast(approach_wp)) return;

  const WAYPOINT& centre = WayPointList[approach_wp];
  int rw_dir = MapApproachRunwayDir >= 0 ? MapApproachRunwayDir : centre.RunwayDir;
  if (rw_dir < 0) rw_dir = 0;
  const double rw_recip = AngleLimit360(static_cast<double>(rw_dir) + 180.0);

  double if_lat, if_lon;
  FindLatitudeLongitude(centre.Latitude, centre.Longitude, rw_recip,
                        APPROACH_IF_DISTANCE_M, &if_lat, &if_lon);

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
  CallbackEntry(OnDirectClicked),
  // CallbackEntry(OnCircuitClicked),
  CallbackEntry(OnRunway1Clicked),
  CallbackEntry(OnRunway2Clicked),
  // CallbackEntry(OnCircuitLeftClicked),
  // CallbackEntry(OnCircuitRightClicked),
  CallbackEntry(OnApproveClicked),
  EndCallbackEntry()
};

void dlgApproach(int waypoint_index) {
  if (waypoint_index < 0 || !ValidWayPointFast(waypoint_index)) {
    return;
  }

  MapApproachEnabled = true;
  MapApproachWaypoint = waypoint_index;

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

  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_APPROACH_L : IDR_XML_APPROACH_P);
  if (!wf) return;

  const unsigned form_height = main_window->GetHeight();
  wf->SetHeight(form_height);
  dlgSize = ScreenLandscape ? wf->GetWidth() : form_height;
  if (ScreenLandscape) {
    wf->SetLeft(main_window->GetRight() - dlgSize);
  }

  // Position Approve button with margin from bottom (form height is now the window height)
  constexpr int APPROVE_BUTTON_HEIGHT = 28;
  constexpr int APPROVE_MARGIN_BOTTOM = 61;  // same visual margin as runway button from left edge
  WndButton* btnApprove = wf->FindByName<WndButton>(TEXT("btnApprove"));
  if (btnApprove) {
    btnApprove->SetTop(static_cast<int>(form_height) - APPROVE_BUTTON_HEIGHT - APPROVE_MARGIN_BOTTOM - 10);
  }

  TCHAR cap1[8], cap2[8];
  lk::snprintf(cap1, _T("%02d"), runway_heading_1 / 10);
  lk::snprintf(cap2, _T("%02d"), runway_heading_2 / 10);

  WndButton* btn1 = wf->FindByName<WndButton>(TEXT("btnRunway1"));
  if (btn1) btn1->SetCaption(cap1);
  WndButton* btn2 = wf->FindByName<WndButton>(TEXT("btnRunway2"));
  if (btn2) btn2->SetCaption(cap2);

  RefreshApproachButtonStyles();

  MapWindow::SetApproachPan(true, waypoint_index, dlgSize);

  wf->SetTimerNotify(400, OnApproachTimerNotify);
  wf->ShowModal();
  wf->SetTimerNotify(0, nullptr);

  MapWindow::SetApproachPan(false, 0);
  MapApproachEnabled = false;
  MapApproachWaypoint = -1;
  MapApproachRunwayDir = -1;

  delete wf;
  wf = nullptr;
}
