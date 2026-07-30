/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   GA Direct To logic for the Target dialog.
   All functions are no-ops (or return safe defaults) when !ISGAAIRCRAFT.
*/

#include "externs.h"
#include "WindowControls.h"
#include "NavFunctions.h"
#include "GADirectTo.h"
#include "Library/TimeFunctions.h"

static WndForm* s_wf = nullptr;
static int*     s_tp  = nullptr;

static bool ga_offtask_browsing = false;
static bool show_direct_button  = false;

// ---------------------------------------------------------------------------

void GA_InitTargetDialog(WndForm* wf, int* target_point_ptr) {
  s_wf = wf;
  s_tp = target_point_ptr;
  ga_offtask_browsing = false;
  show_direct_button  = false;
}

void GA_ResetTargetDialog() {
  s_wf = nullptr;
  s_tp = nullptr;
  ga_offtask_browsing = false;
  show_direct_button  = false;
}

bool GA_IsOfftaskBrowsing() {
  return ga_offtask_browsing;
}

bool GA_CanOpenWithoutTask(int& TaskPoint) {
  if (!ISGAAIRCRAFT) return false;
  if (!DirectToActive || !ValidWayPointFast(DirectToWaypointIndex)) return false;
  TaskPoint = -1;
  return true;
}

void GA_OnTargetPointSelected() {
  show_direct_button = true;
}

// ---------------------------------------------------------------------------

void GA_UpdateNavButtons() {
  if (!s_wf || !s_tp) return;

  WndButton* btnPrev   = s_wf->FindByName<WndButton>(TEXT("btnPrev"));
  WndButton* btnNext   = s_wf->FindByName<WndButton>(TEXT("btnNext"));
  WndButton* btnDirect = s_wf->FindByName<WndButton>(TEXT("btnDirectTo"));

  if (!ISGAAIRCRAFT) {
    if (btnPrev)   btnPrev->SetVisible(false);
    if (btnNext)   btnNext->SetVisible(false);
    if (btnDirect) btnDirect->SetVisible(false);
    return;
  }

  const int tp = *s_tp;
  bool in_task     = false;
  bool has_prev    = false;
  bool has_next    = false;
  bool has_fwd_wp  = false;
  {
    const std::lock_guard lock(CritSec_TaskData);
    in_task  = (ActiveTaskPoint >= 0) && ValidTaskPoint(tp);
    has_prev = in_task && (tp > 0) && ValidTaskPoint(tp - 1);
    has_next = in_task && ValidTaskPoint(tp + 1);
    if (DirectToActive && ValidWayPointFast(DirectToWaypointIndex) && !ga_offtask_browsing) {
      has_fwd_wp = ValidTaskPoint(
          GA_FindNextForwardTaskWP(GPS_INFO.Latitude, GPS_INFO.Longitude));
    }
  }

  const bool ga_offtask = DirectToActive && ValidWayPointFast(DirectToWaypointIndex)
                          && !ga_offtask_browsing;

  if (btnPrev)
    btnPrev->SetVisible(!ga_offtask && in_task && has_prev);
  if (btnNext)
    btnNext->SetVisible((!ga_offtask && in_task && has_next) || (ga_offtask && has_fwd_wp));
  if (btnDirect)
    btnDirect->SetVisible(!ga_offtask && show_direct_button && in_task);
}

// ---------------------------------------------------------------------------

void GA_RefreshTargetFields() {
  if (!s_wf) return;

  WndProperty* pDist = s_wf->FindByName<WndProperty>(TEXT("prpGADist"));
  WndProperty* pETE  = s_wf->FindByName<WndProperty>(TEXT("prpGAETE"));
  WndProperty* pETA  = s_wf->FindByName<WndProperty>(TEXT("prpGAETA"));

  const bool show = ISGAAIRCRAFT;
  if (pDist) pDist->SetVisible(show);
  if (pETE)  pETE->SetVisible(show);
  if (pETA)  pETA->SetVisible(show);

  if (!show || !s_tp) return;

  const bool ga_offtask_active = DirectToActive && ValidWayPointFast(DirectToWaypointIndex)
                                  && !ga_offtask_browsing;

  if (ga_offtask_active) {
    TCHAR dt_title[NAME_SIZE + 12];
    lk::snprintf(dt_title, _T("%s: %s"),
                 MsgToken<2521>(), WayPointList[DirectToWaypointIndex].Name);
    s_wf->SetCaption(dt_title);
  }

  const int tp = *s_tp;
  const int ga_calc_wp = ga_offtask_active
                         ? DirectToWaypointIndex
                         : (ValidTaskPoint(tp) ? Task[tp].Index : -1);

  if (!ValidWayPointFast(ga_calc_wp)) return;

  double dist_m = 0., bearing = 0.;
  DistanceBearing(GPS_INFO.Latitude, GPS_INFO.Longitude,
                  WayPointList[ga_calc_wp].Latitude,
                  WayPointList[ga_calc_wp].Longitude,
                  &dist_m, &bearing);

  const double ete_s = (CALCULATED_INFO.AverageGS > 0)
                       ? dist_m / CALCULATED_INFO.AverageGS
                       : -1.;

  if (pDist) {
    TCHAR buf[32];
    lk::snprintf(buf, TEXT("%.1f %s"),
                 Units::ToDistance(dist_m), Units::GetDistanceName());
    pDist->SetText(buf);
  }
  if (pETE) {
    TCHAR buf[32];
    if (ete_s > 0) Units::TimeToTextDown(buf, (int)ete_s);
    else lk::strcpy(buf, TEXT("--:--"));
    pETE->SetText(buf);
  }
  if (pETA) {
    TCHAR buf[32];
    if (ete_s > 0)
      Units::TimeToText(buf, (int)(LocalTime(GPS_INFO.Time) + ete_s));
    else
      lk::strcpy(buf, TEXT("--:--"));
    pETA->SetText(buf);
  }
}

// ---------------------------------------------------------------------------

bool GA_ApplyTargetPanOverride(unsigned& dlgSize) {
  if (!ISGAAIRCRAFT) return false;
  if (!DirectToActive || !ValidWayPointFast(DirectToWaypointIndex)) return false;
  if (ga_offtask_browsing) return false;
  MapWindow::SetTargetPanWaypoint(DirectToWaypointIndex, dlgSize);
  GA_SetTargetBrowseWP(-1, -1);
  return true;
}

int GA_GetApproachWPForTarget() {
  if (!ISGAAIRCRAFT) return -1;
  if (!DirectToActive || !ValidWayPointFast(DirectToWaypointIndex)) return -1;
  if (!WayPointCalc[DirectToWaypointIndex].IsLandable) return -1;
  if (ga_offtask_browsing) return -1;
  return DirectToWaypointIndex;
}

// ---------------------------------------------------------------------------

bool GA_OnTargetPrev() {
  if (!ISGAAIRCRAFT || !s_tp) return false;
  bool valid = false;
  {
    const std::lock_guard lock(CritSec_TaskData);
    valid = (*s_tp > 0) && ValidTaskPoint(*s_tp - 1);
  }
  if (!valid) return false;
  (*s_tp)--;
  show_direct_button = true;
  return true;
}

bool GA_OnTargetNext() {
  if (!ISGAAIRCRAFT || !s_tp) return false;

  // First Next during off-task DirectTo: jump to first forward task WP.
  if (DirectToActive && ValidWayPointFast(DirectToWaypointIndex) && !ga_offtask_browsing) {
    int fwd = -1;
    {
      const std::lock_guard lock(CritSec_TaskData);
      fwd = GA_FindNextForwardTaskWP(GPS_INFO.Latitude, GPS_INFO.Longitude);
    }
    if (!ValidTaskPoint(fwd)) return false;
    *s_tp = fwd;
    ga_offtask_browsing = true;
    show_direct_button  = true;
    return true;
  }

  bool valid = false;
  {
    const std::lock_guard lock(CritSec_TaskData);
    valid = ValidTaskPoint(*s_tp + 1);
  }
  if (!valid) return false;
  (*s_tp)++;
  show_direct_button = true;
  return true;
}

void GA_OnTargetDirectTo(WndButton* pWnd) {
  if (!ISGAAIRCRAFT || !s_tp) return;
  const bool activated = ShowDirectToCountdownDialog(*s_tp);
  if (activated) {
    WndForm* targetForm = pWnd ? pWnd->GetParentWndForm() : nullptr;
    if (targetForm) targetForm->SetModalResult(mrOK);
  }
}
