/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   Approach dialog: type (Diretto/Circuito), runway, circuit side (left/right).
   Selected button = inverted (dark) style. Draw only when all choices are set.
*/

#include "externs.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "Event/Event.h"
#include "resource.h"
#include "MapWindow.h"

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
  WndButton* btnCircuit = wf->FindByName<WndButton>(TEXT("btnCircuit"));
  WndButton* btnRunway1 = wf->FindByName<WndButton>(TEXT("btnRunway1"));
  WndButton* btnRunway2 = wf->FindByName<WndButton>(TEXT("btnRunway2"));
  WndButton* btnCircuitLeft = wf->FindByName<WndButton>(TEXT("btnCircuitLeft"));
  WndButton* btnCircuitRight = wf->FindByName<WndButton>(TEXT("btnCircuitRight"));

  if (btnDirect) btnDirect->SetSelected(MapApproachMode == 0);
  if (btnCircuit) btnCircuit->SetSelected(MapApproachMode == 1);

  const bool rw1 = (MapApproachRunwayDir == runway_heading_1);
  const bool rw2 = (MapApproachRunwayDir == runway_heading_2);
  if (btnRunway1) btnRunway1->SetSelected(rw1);
  if (btnRunway2) btnRunway2->SetSelected(rw2);

  if (btnCircuitLeft) {
    btnCircuitLeft->SetVisible(MapApproachMode == 1);
    btnCircuitLeft->SetSelected(MapApproachCircuitSide == 0);
  }
  if (btnCircuitRight) {
    btnCircuitRight->SetVisible(MapApproachMode == 1);
    btnCircuitRight->SetSelected(MapApproachCircuitSide == 1);
  }
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

static void OnCircuitClicked(WndButton* pWnd) {
  MapApproachMode = 1;
  RefreshApproachButtonStyles();
  RefreshMapApproach();
}

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

static void OnCircuitLeftClicked(WndButton* pWnd) {
  MapApproachCircuitSide = 0;
  RefreshApproachButtonStyles();
  RefreshMapApproach();
}

static void OnCircuitRightClicked(WndButton* pWnd) {
  MapApproachCircuitSide = 1;
  RefreshApproachButtonStyles();
  RefreshMapApproach();
}

static CallBackTableEntry_t CallBackTable[] = {
  CallbackEntry(OnApproachOKClicked),
  CallbackEntry(OnDirectClicked),
  CallbackEntry(OnCircuitClicked),
  CallbackEntry(OnRunway1Clicked),
  CallbackEntry(OnRunway2Clicked),
  CallbackEntry(OnCircuitLeftClicked),
  CallbackEntry(OnCircuitRightClicked),
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

  if (ScreenLandscape) {
    dlgSize = wf->GetWidth();
    wf->SetLeft(main_window->GetRight() - dlgSize);
  } else {
    dlgSize = wf->GetHeight();
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
