/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights
*/

#include "externs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "resource.h"
#include "NavFunctions.h"
#include "CalcTask.h"
#include "Library/TimeFunctions.h"
#include "utils/printf.h"
#include "GADirectTo.h"

static int countdown_seconds = 0;
static TCHAR countdown_wp_name[NAME_SIZE] = {};
static TCHAR countdown_sec_str[8] = {};
static TCHAR countdown_info1[64] = {};
static TCHAR countdown_info2[64] = {};

static void UpdateCountdownFrames(WndForm* pWnd) {
  WndFrame* frmWpName    = pWnd->FindByName<WndFrame>(TEXT("frmWpName"));
  WndFrame* frmInfo1     = pWnd->FindByName<WndFrame>(TEXT("frmInfo1"));
  WndFrame* frmInfo2     = pWnd->FindByName<WndFrame>(TEXT("frmInfo2"));
  WndFrame* frmCountdown = pWnd->FindByName<WndFrame>(TEXT("frmCountdown"));
  if (frmWpName)    frmWpName->SetCaption(countdown_wp_name);
  if (frmInfo1)     frmInfo1->SetCaption(countdown_info1);
  if (frmInfo2)     frmInfo2->SetCaption(countdown_info2);
  if (frmCountdown) {
    lk::snprintf(countdown_sec_str, TEXT("%d"), countdown_seconds);
    frmCountdown->SetCaption(countdown_sec_str);
  }
}

static bool OnDirectToCountdownTimer(WndForm* pWnd) {
  countdown_seconds--;
  UpdateCountdownFrames(pWnd);
  if (countdown_seconds <= 0) {
    pWnd->SetModalResult(mrOK);
  }
  return true;
}

static void OnDirectToCountdownCancelClicked(WndButton* pWnd) {
  if (pWnd) {
    WndForm* pForm = pWnd->GetParentWndForm();
    if (pForm) pForm->SetModalResult(mrCancel);
  }
}

static CallBackTableEntry_t CountdownCallBackTable[] = {
  CallbackEntry(OnDirectToCountdownCancelClicked),
  EndCallbackEntry()
};

// Compute BRG/DIST and AvETE/AvETA for the countdown info rows.
static void ComputeCountdownInfo(int wp_index) {
  countdown_info1[0] = 0;
  countdown_info2[0] = 0;

  if (!ValidWayPointFast(wp_index)) return;

  double wp_lat, wp_lon;
  {
    const std::lock_guard lock(CritSec_TaskData);
    wp_lat = WayPointList[wp_index].Latitude;
    wp_lon = WayPointList[wp_index].Longitude;
  }

  double dist_m = 0., bearing = 0.;
  DistanceBearing(GPS_INFO.Latitude, GPS_INFO.Longitude,
                  wp_lat, wp_lon, &dist_m, &bearing);

  DoAlternates(&GPS_INFO, &CALCULATED_INFO, wp_index);
  double ete_s = WayPointCalc[wp_index].NextAvrETE;

  TCHAR dist_buf[20];
  lk::snprintf(dist_buf, _T("%.1f %s"),
               Units::ToDistance(dist_m), Units::GetDistanceName());
  lk::snprintf(countdown_info1, _T("BRG %.0f°   DIST %s"), bearing, dist_buf);

  TCHAR ete_buf[12];
  TCHAR eta_buf[12];
  lk::strcpy(ete_buf, _T("--:--"));
  lk::strcpy(eta_buf, _T("--:--"));
  if (ete_s > 0 && ete_s < ERROR_TIME) {
    Units::TimeToTextDown(ete_buf, (int)ete_s);
    Units::TimeToText(eta_buf, (int)(LocalTime(GPS_INFO.Time) + ete_s));
  }
  lk::snprintf(countdown_info2, _T("ETE %s  ETA %s"), ete_buf, eta_buf);
}

// Compute "15 km SE LINATE" description for an arbitrary map position.
// Priority with distance caps: airport <=80 km, outlanding <=30 km,
// turnpoint <=15 km, else nearest anything.
static void ComputePanDescription(double pan_lat, double pan_lon,
                                  TCHAR* buf, size_t bufsz) {
  constexpr double CAP_AIRPORT    = 80000.;
  constexpr double CAP_OUTLANDING = 30000.;
  constexpr double CAP_TURNPOINT  = 15000.;

  struct Candidate { int idx=-1; double dist=1e20, lat=0., lon=0.; TCHAR name[NAME_SIZE]={}; };
  Candidate airport, outlanding, turnpoint, any_wp;

  {
    const std::lock_guard lock(CritSec_TaskData);
    for (int i = NUMRESWP; i < (int)WayPointList.size(); i++) {
      if (WayPointList[i].Latitude == RESWP_INVALIDNUMBER) continue;
      double d = 0., b = 0.;
      DistanceBearing(pan_lat, pan_lon,
                      WayPointList[i].Latitude, WayPointList[i].Longitude,
                      &d, &b);
      auto fill = [&](Candidate& c) {
        c.idx = i; c.dist = d;
        c.lat = WayPointList[i].Latitude;
        c.lon = WayPointList[i].Longitude;
        lk::strcpy(c.name, WayPointList[i].Name);
      };
      switch (WayPointCalc[i].WpType) {
        case WPT_AIRPORT:    if (d < airport.dist)    fill(airport);    break;
        case WPT_OUTLANDING: if (d < outlanding.dist) fill(outlanding); break;
        case WPT_TURNPOINT:  if (d < turnpoint.dist)  fill(turnpoint);  break;
        default: break;
      }
      if (d < any_wp.dist) fill(any_wp);
    }
  }

  Candidate* ref = nullptr;
  if      (airport.idx   >= 0 && airport.dist   <= CAP_AIRPORT)    ref = &airport;
  else if (outlanding.idx >= 0 && outlanding.dist <= CAP_OUTLANDING) ref = &outlanding;
  else if (turnpoint.idx  >= 0 && turnpoint.dist  <= CAP_TURNPOINT)  ref = &turnpoint;
  else if (any_wp.idx >= 0) ref = &any_wp;

  if (!ref) {
    TCHAR lat_hemi = pan_lat >= 0 ? _T('N') : _T('S');
    TCHAR lon_hemi = pan_lon >= 0 ? _T('E') : _T('W');
    lk::snprintf(buf, bufsz, _T("%.4f %c %.4f %c"),
                 std::fabs(pan_lat), lat_hemi, std::fabs(pan_lon), lon_hemi);
    return;
  }

  double dist = 0., bearing = 0.;
  DistanceBearing(ref->lat, ref->lon, pan_lat, pan_lon, &dist, &bearing);
  static const TCHAR* dirs[] = {
    _T("N"), _T("NE"), _T("E"), _T("SE"),
    _T("S"), _T("SW"), _T("W"), _T("NW")
  };
  lk::snprintf(buf, bufsz, _T("%.0f km %s %s"),
               dist / 1000.0,
               dirs[((int)((bearing + 22.5) / 45.0)) % 8],
               ref->name);
}

// Shared countdown popup.
// new_tp >= 0: task point; wp_index: off-task waypoint index (GA only).
// name_preset: caller has already filled countdown_wp_name.
static bool RunDirectToCountdown(int new_tp, int wp_index,
                                 bool name_preset = false) {
  struct DirectToSnapshot {
    bool active = false;
    int  wp_index = -1;
    double origin_lat = 0., origin_lon = 0.;
  };

  int info_wp = -1;
  DirectToSnapshot saved_state;
  {
    const std::lock_guard lock(CritSec_TaskData);
    if (new_tp >= 0) {
      if (!ValidTaskPoint(new_tp) || !ValidWayPointFast(Task[new_tp].Index))
        return false;
      if (!name_preset)
        lk::strcpy(countdown_wp_name, WayPointList[Task[new_tp].Index].Name);
      info_wp = Task[new_tp].Index;
    } else {
      if (!ValidWayPointFast(wp_index))
        return false;
      if (!name_preset)
        lk::strcpy(countdown_wp_name, WayPointList[wp_index].Name);
      info_wp = wp_index;
      // Save full DirectTo state for cancel restore.
      saved_state = { DirectToActive, DirectToWaypointIndex,
                      DirectToOriginLat, DirectToOriginLon };
      // Detach autopilot from the previous DirectTo target for the duration of
      // the countdown.  Without this, writing new coordinates into RESWP_PANPOS
      // while DirectToWaypointIndex already points there causes the autopilot
      // to start commanding a turn immediately, before the pilot confirms.
      DirectToWaypointIndex = -1;
    }
  }

  ComputeCountdownInfo(info_wp);

  std::unique_ptr<WndForm> pf(dlgLoadFromXML(CountdownCallBackTable,
      ScreenLandscape ? IDR_XML_DIRECTTO_COUNTDOWN_L : IDR_XML_DIRECTTO_COUNTDOWN_P));
  if (!pf) {
    if (new_tp < 0) {
      const std::lock_guard lock(CritSec_TaskData);
      DirectToActive        = saved_state.active;
      DirectToWaypointIndex = saved_state.wp_index;
      DirectToOriginLat     = saved_state.origin_lat;
      DirectToOriginLon     = saved_state.origin_lon;
    }
    return false;
  }

  const PixelRect rc(main_window->GetClientRect());
  pf->SetLeft((rc.left + rc.GetSize().cx - (int)pf->GetWidth()) / 2);
  pf->SetTop((rc.top + rc.GetSize().cy - (int)pf->GetHeight()) / 2);

  countdown_seconds = 10;

  const UINT centered = DT_CENTER | DT_VCENTER | DT_NOCLIP;
  WndFrame* frmLabel = pf->FindByName<WndFrame>(TEXT("frmLabel"));
  if (frmLabel) { frmLabel->SetCaption(TEXT("Direct to:")); frmLabel->SetCaptionStyle(centered); }
  WndFrame* frmWpName = pf->FindByName<WndFrame>(TEXT("frmWpName"));
  if (frmWpName) frmWpName->SetCaptionStyle(centered);
  WndFrame* frmInfo1 = pf->FindByName<WndFrame>(TEXT("frmInfo1"));
  if (frmInfo1) frmInfo1->SetCaptionStyle(centered);
  WndFrame* frmInfo2 = pf->FindByName<WndFrame>(TEXT("frmInfo2"));
  if (frmInfo2) frmInfo2->SetCaptionStyle(centered);
  WndFrame* frmCountdown = pf->FindByName<WndFrame>(TEXT("frmCountdown"));
  if (frmCountdown) frmCountdown->SetCaptionStyle(centered);

  UpdateCountdownFrames(pf.get());
  pf->SetTimerNotify(1000, OnDirectToCountdownTimer);

  if (pf->ShowModal() != mrOK) {
    const std::lock_guard lock(CritSec_TaskData);
    if (new_tp < 0) {
      DirectToActive        = saved_state.active;
      DirectToWaypointIndex = saved_state.wp_index;
      DirectToOriginLat     = saved_state.origin_lat;
      DirectToOriginLon     = saved_state.origin_lon;
    }
    return false;
  }

  const std::lock_guard lock(CritSec_TaskData);
  DirectToActive = true;
  DirectToOriginLat = GPS_INFO.Latitude;
  DirectToOriginLon = GPS_INFO.Longitude;
  if (new_tp >= 0 && ValidTaskPoint(new_tp)) {
    ActiveTaskPoint = new_tp;
    DirectToWaypointIndex = -1;
  } else if (ValidWayPointFast(wp_index)) {
    DirectToWaypointIndex = wp_index;
  } else {
    DirectToActive = false;
    return false;
  }
  return true;
}

bool ShowDirectToCountdownDialog(int new_tp) {
  return RunDirectToCountdown(new_tp, -1);
}

bool ShowDirectToOffTaskDialog(int wp_index) {
  return RunDirectToCountdown(-1, wp_index);
}

bool ShowDirectToFromPanDialog(int wp_index, double pan_lat, double pan_lon) {
  // Snap to a real landable waypoint if the pan cursor is within ~4% of the visible
  // screen range (~30 px on a typical screen), independent of zoom level.
  const double SNAP_M = MapWindow::GetApproxScreenRange() * 0.04;
  {
    int best_idx = -1;
    double best_dist = SNAP_M;
    const std::lock_guard lock(CritSec_TaskData);
    for (int i = NUMRESWP; i < (int)WayPointList.size(); i++) {
      if (WayPointList[i].Latitude == RESWP_INVALIDNUMBER) continue;
      if (!WayPointCalc[i].IsLandable) continue;
      double d = 0.;
      DistanceBearing(pan_lat, pan_lon,
                      WayPointList[i].Latitude, WayPointList[i].Longitude,
                      &d, nullptr);
      if (d < best_dist) { best_dist = d; best_idx = i; }
    }
    if (best_idx >= 0) {
      lk::strcpy(countdown_wp_name, WayPointList[best_idx].Name);
      wp_index = best_idx;
    }
  }
  if (wp_index == RESWP_PANPOS)
    ComputePanDescription(pan_lat, pan_lon, countdown_wp_name, NAME_SIZE - 1);
  return RunDirectToCountdown(-1, wp_index, /*name_preset=*/true);
}
