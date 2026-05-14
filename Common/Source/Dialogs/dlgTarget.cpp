/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTarget.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "dlgTools.h"
#include "Calculations2.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "Event/Event.h"
#include "resource.h"
#include "NavFunctions.h"
#include "CalcTask.h"

#include <algorithm>

static WndForm *wf=NULL;
static WindowControl *btnMove = NULL;
static int ActiveWayPointOnEntry = 0;


static double Range = 0;
static double Radial = 0;
static int target_point = 0;
static bool TargetMoveMode = false;

static unsigned dlgSize = 0;

bool TargetDialogOpen = false;

static void OnOKClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void MoveTarget(double adjust_angle) {
  if (gTaskType != task_type_t::AAT) return;
  if (target_point==0) return;
  if (!ValidTaskPoint(target_point)) return;
  if (!ValidTaskPoint(target_point+1)) return;
  if (target_point < ActiveTaskPoint) return;

  LockTaskData();

  double target_latitude, target_longitude;
  double bearing, distance;
  distance = 500;
  if(Task[target_point].AATType == sector_type_t::SECTOR) {
    distance = max(Task[target_point].AATSectorRadius/20.0,distance);
  } else {
    distance = max(Task[target_point].AATCircleRadius/20.0,distance);
  }

  bearing = AngleLimit360(MapWindow::GetDisplayAngle() + adjust_angle);
  FindLatitudeLongitude (Task[target_point].AATTargetLat,
                         Task[target_point].AATTargetLon,
                         bearing,
                         distance,
                         &target_latitude,
                         &target_longitude);

  if (InTurnSector({{target_latitude, target_longitude}, 0}, target_point)) {
    if (CALCULATED_INFO.IsInSector && (target_point == ActiveTaskPoint)) {
      // set range/radial for inside sector
      double course_bearing, target_bearing;
      DistanceBearing(Task[target_point-1].AATTargetLat,
                      Task[target_point-1].AATTargetLon,
                      GPS_INFO.Latitude,
                      GPS_INFO.Longitude,
                      NULL, &course_bearing);

      DistanceBearing(GPS_INFO.Latitude,
                      GPS_INFO.Longitude,
                      target_latitude,
                      target_longitude,
                      &distance, &target_bearing);
      bearing = AngleLimit180(target_bearing-course_bearing);

      if (fabs(bearing)<90.0) {
        Task[target_point].AATTargetLat = target_latitude;
        Task[target_point].AATTargetLon = target_longitude;
        Radial = bearing;
        Task[target_point].AATTargetOffsetRadial = Radial;
        Range =
          FindInsideAATSectorRange(GPS_INFO.Latitude,
                                   GPS_INFO.Longitude,
                                   target_point,
                                   target_bearing,
                                   distance);
        Task[target_point].AATTargetOffsetRadius = Range;
        TaskModified = true;
        TargetModified = true;
      }
    } else {
      // OK to change it..
      Task[target_point].AATTargetLat = target_latitude;
      Task[target_point].AATTargetLon = target_longitude;

      // set range/radial for outside sector
      DistanceBearing(WayPointList[Task[target_point].Index].Latitude,
                      WayPointList[Task[target_point].Index].Longitude,
                      Task[target_point].AATTargetLat,
                      Task[target_point].AATTargetLon,
                      &distance, &bearing);
      bearing = AngleLimit180(bearing-Task[target_point].Bisector);
      if(Task[target_point].AATType == sector_type_t::SECTOR) {
        Range = (fabs(distance)/Task[target_point].AATSectorRadius)*2-1;
      } else {
        if (fabs(bearing)>90.0) {
          distance = -distance;
          bearing = AngleLimit180(bearing+180);
        }
        Range = distance/Task[target_point].AATCircleRadius;
      }
      Task[target_point].AATTargetOffsetRadius = Range;
      Task[target_point].AATTargetOffsetRadial = bearing;
      Radial = bearing;
      TaskModified = true;
      TargetModified = true;
    }
  }
  UnlockTaskData();
}


static void MoveTarget(double target_longitude, double target_latitude) {
  if (gTaskType != task_type_t::AAT) return;
  if (target_point==0) return;
  if (!ValidTaskPoint(target_point)) return;
  if (!ValidTaskPoint(target_point+1)) return;
  if (target_point < ActiveTaskPoint) return;

  LockTaskData();

  double distance, bearing;

  if (InTurnSector({{target_latitude, target_longitude}, 0.}, target_point)) {
    if (CALCULATED_INFO.IsInSector && (target_point == ActiveTaskPoint)) {
      // set range/radial for inside sector
      double course_bearing, target_bearing;
      DistanceBearing(Task[target_point-1].AATTargetLat,
                      Task[target_point-1].AATTargetLon,
                      GPS_INFO.Latitude,
                      GPS_INFO.Longitude,
                      NULL, &course_bearing);

      DistanceBearing(GPS_INFO.Latitude,
                      GPS_INFO.Longitude,
                      target_latitude,
                      target_longitude,
                      &distance, &target_bearing);
      bearing = AngleLimit180(target_bearing-course_bearing);

      if (fabs(bearing)<90.0) {
        Task[target_point].AATTargetLat = target_latitude;
        Task[target_point].AATTargetLon = target_longitude;
        Radial = bearing;
        Task[target_point].AATTargetOffsetRadial = Radial;
        Range =
          FindInsideAATSectorRange(GPS_INFO.Latitude,
                                   GPS_INFO.Longitude,
                                   target_point,
                                   target_bearing,
                                   distance);
        Task[target_point].AATTargetOffsetRadius = Range;
        TaskModified = true;
        TargetModified = true;
      }
    } else {
      // OK to change it..
      Task[target_point].AATTargetLat = target_latitude;
      Task[target_point].AATTargetLon = target_longitude;

      // set range/radial for outside sector
      DistanceBearing(WayPointList[Task[target_point].Index].Latitude,
                      WayPointList[Task[target_point].Index].Longitude,
                      Task[target_point].AATTargetLat,
                      Task[target_point].AATTargetLon,
                      &distance, &bearing);
      bearing = AngleLimit180(bearing-Task[target_point].Bisector);
      if(Task[target_point].AATType == sector_type_t::SECTOR) {
        Range = (fabs(distance)/Task[target_point].AATSectorRadius)*2-1;
      } else {
        if (fabs(bearing)>90.0) {
          distance = -distance;
          bearing = AngleLimit180(bearing+180);
        }
        Range = distance/Task[target_point].AATCircleRadius;
      }
      Task[target_point].AATTargetOffsetRadius = Range;
      Task[target_point].AATTargetOffsetRadial = bearing;
      Radial = bearing;
      TaskModified = true;
      TargetModified = true;
    }
  }
  UnlockTaskData();
}

//
// This is working only with real keypresses, not with touch screen
//
static bool FormKeyDown(WndForm* pWnd, unsigned KeyCode) {
    switch (KeyCode & 0xffff) {
        case '2':
            MoveTarget(0);
            return true;
        case '3':
            MoveTarget(180);
            return true;
        case '6':
            MoveTarget(270);
            return true;
        case '7':
            MoveTarget(90);
            return true;
    }
    if (TargetMoveMode) {
        switch (KeyCode & 0xffff) {
            case KEY_UP:
                MoveTarget(0);
                return true;
            case KEY_DOWN:
                MoveTarget(180);
                return true;
            case KEY_LEFT:
                MoveTarget(270);
                return true;
            case KEY_RIGHT:
                MoveTarget(90);
                return true;
        }
    }
    return false;
}



/// Portrait: stack visible rows under Approach (or OK/Task) so hidden AAT fields do not leave white bands.
static void CompactTargetPortraitLayout(void) {
  if (!wf || ScreenLandscape) return;

  WndButton* const btnApproach = wf->FindByName<WndButton>(TEXT("btnApproach"));
  if (!btnApproach) return;

  WindowControl* const btnOK = wf->FindByName<WindowControl>(TEXT("btnOK"));
  WndProperty* const wTask = wf->FindByName<WndProperty>(TEXT("prpTaskPoint"));
  WndProperty* const pRange = wf->FindByName<WndProperty>(TEXT("prpRange"));
  WndProperty* const pRadial = wf->FindByName<WndProperty>(TEXT("prpRadial"));
  WndProperty* const pEst = wf->FindByName<WndProperty>(TEXT("prpAATEst"));
  WndProperty* const pDelta = wf->FindByName<WndProperty>(TEXT("prpAATDelta"));
  WndProperty* const pRem = wf->FindByName<WndProperty>(TEXT("prpSpeedRemaining"));
  WndProperty* const pAch = wf->FindByName<WndProperty>(TEXT("prpSpeedAchieved"));
  WndButton* const btnMov = wf->FindByName<WndButton>(TEXT("btnMove"));
  WndProperty* const pLock = wf->FindByName<WndProperty>(TEXT("prpAATTargetLocked"));

  const int gap = NIBLSCALE(2);

  int y;
  if (btnApproach->IsVisible()) {
    y = (int)btnApproach->GetTop() + (int)btnApproach->GetHeight() + gap;
  } else {
    int yref = 0;
    if (wTask && wTask->IsVisible()) {
      yref = (int)wTask->GetTop() + (int)wTask->GetHeight();
    } else if (btnOK && btnOK->IsVisible()) {
      yref = (int)btnOK->GetTop() + (int)btnOK->GetHeight();
    }
    y = yref + gap;
  }

  auto rowPairHeight = [](WndProperty* a, WndProperty* b) -> int {
    int h = 0;
    if (a && a->IsVisible()) h = std::max(h, (int)a->GetHeight());
    if (b && b->IsVisible()) h = std::max(h, (int)b->GetHeight());
    return h;
  };

  if (pRange && pRange->IsVisible()) {
    pRange->SetTop(y);
    if (pRadial) pRadial->SetTop(y);
    y += rowPairHeight(pRange, pRadial) + gap;
  }

  const int hEstDelta = rowPairHeight(pEst, pDelta);
  if (pEst && pEst->IsVisible()) pEst->SetTop(y);
  if (pDelta && pDelta->IsVisible()) pDelta->SetTop(y);
  if (hEstDelta > 0) y += hEstDelta + gap;

  const int hSpd = rowPairHeight(pRem, pAch);
  if (pRem && pRem->IsVisible()) pRem->SetTop(y);
  if (pAch && pAch->IsVisible()) pAch->SetTop(y);
  if (hSpd > 0) y += hSpd + gap;

  if (btnMov && btnMov->IsVisible()) {
    btnMov->SetTop(y);
    if (pLock && pLock->IsVisible()) pLock->SetTop(y);
    y += (int)btnMov->GetHeight() + gap;
  } else if (pLock && pLock->IsVisible()) {
    pLock->SetTop(y);
    y += (int)pLock->GetHeight() + gap;
  }

  int maxBottom = 0;
  const auto acc = [&maxBottom](const WindowControl* w) {
    if (w && w->IsVisible()) {
      const int b = (int)w->GetTop() + (int)w->GetHeight();
      maxBottom = std::max(maxBottom, b);
    }
  };
  acc(btnOK);
  acc(wTask);
  acc(btnApproach);
  acc(pRange);
  acc(pRadial);
  acc(pEst);
  acc(pDelta);
  acc(pRem);
  acc(pAch);
  acc(btnMov);
  acc(pLock);

  /* maxBottom is in client (content) coordinates; outer WndForm height must include title bar
     and borders. Growing only when newH < GetHeight() never expanded when scaled widgets
     exceeded the XML client area — clipped ETE / V rem rows. Resize by client-area delta. */
  if (maxBottom > 0) {
    const unsigned needClient = (unsigned)maxBottom + (unsigned)NIBLSCALE(10);
    WindowControl* const client = wf->GetClientArea();
    const unsigned curClient = client->GetHeight();
    const int delta = (int)needClient - (int)curClient;
    if (delta != 0) {
      wf->SetHeight((unsigned)((int)wf->GetHeight() + delta));
    }
  }
}

/// True if Approach can open for task point tp (landable); optional waypoint index out.
static bool CanOpenApproachForTargetPoint(int tp, int* wp_index_out) {
  if (!ValidTaskPoint(tp)) return false;
  const int wp_index = Task[tp].Index;
  if (!ValidWayPointFast(wp_index) || !WayPointCalc[wp_index].IsLandable) return false;
  if (wp_index_out) *wp_index_out = wp_index;
  return true;
}

/// After compact or field visibility changes, sync map pan strip size (width in landscape).
static void ApplyTargetPanIfNeeded(void) {
  if (!wf || !TargetDialogOpen || !ValidTaskPoint(target_point)) return;
  dlgSize = ScreenLandscape ? wf->GetWidth() : wf->GetHeight();
  MapWindow::SetTargetPan(true, target_point, dlgSize);
}

/// Refresh Target dialog fields and show/hide Approach button for landable waypoints.
static void RefreshCalculator(void) {
  WndProperty* wp;

  RefreshTask();
  RefreshTaskStatistics();
  target_point = max(target_point,ActiveTaskPoint);

  bool nodisplay = gTaskType != task_type_t::AAT 
    || (target_point==0)
    || !ValidTaskPoint(target_point+1);

  if (btnMove) {
    if (nodisplay) {
      btnMove->SetVisible(false);
      TargetMoveMode = false;
    } else {
      btnMove->SetVisible(true);
    }
  }

  nodisplay = nodisplay || TargetMoveMode;

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskPoint"));
  if (wp) {
    if (TargetMoveMode) {
      wp->SetVisible(false);
    } else {
      wp->SetVisible(true);
    }
  }

  WindowControl* wc = wf->FindByName<WindowControl>(TEXT("btnOK"));
  if (wc) {
    if (TargetMoveMode) {
      wc->SetVisible(false);
    } else {
      wc->SetVisible(true);
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAATTargetLocked"));
  if (wp) {
    wp->GetDataField()->Set(Task[target_point].AATTargetLocked);
    wp->RefreshDisplay();
    if (nodisplay) {
      wp->SetVisible(false);
    } else {
      wp->SetVisible(true);
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpRange"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Range*100.0);
    wp->RefreshDisplay();
    if (nodisplay) {
      wp->SetVisible(false);
    } else {
      wp->SetVisible(true);
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpRadial"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Radial);
    wp->RefreshDisplay();
    if (nodisplay) {
      wp->SetVisible(false);
    } else {
      wp->SetVisible(true);
    }
  }

  // update outputs
  double dd = CALCULATED_INFO.TaskTimeToGo;
  if ((CALCULATED_INFO.TaskStartTime>0.0)&&(CALCULATED_INFO.Flying)) {
    dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
  }
  dd= min(24.0*60.0,dd/60.0);
  wp = wf->FindByName<WndProperty>(TEXT("prpAATEst"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(dd);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpAATDelta"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(dd-AATTaskLength);
    if (gTaskType == task_type_t::AAT) {
      wp->SetVisible(true);
    } else {
      wp->SetVisible(false);
    }
    wp->RefreshDisplay();
  }

  double v1;
  if (CALCULATED_INFO.TaskTimeToGo>0) {
    v1 = CALCULATED_INFO.TaskDistanceToGo/
      CALCULATED_INFO.TaskTimeToGo;
  } else {
    v1 = 0;
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSpeedRemaining"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Units::ToTaskSpeed(v1));
    wp->GetDataField()->SetUnits(Units::GetTaskSpeedName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSpeedAchieved"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Units::ToTaskSpeed(CALCULATED_INFO.TaskSpeed));
    wp->GetDataField()->SetUnits(Units::GetTaskSpeedName());
    wp->RefreshDisplay();
  }

  WndButton* btnApproach = wf->FindByName<WndButton>(TEXT("btnApproach"));
  if (btnApproach) {
    const bool landable = ValidTaskPoint(target_point) &&
        ValidWayPointFast(Task[target_point].Index) &&
        WayPointCalc[Task[target_point].Index].IsLandable;
    btnApproach->SetVisible(landable);
  }

  if (!ScreenLandscape && wf) {
    CompactTargetPortraitLayout();
    dlgSize = wf->GetHeight();
  } else if (ScreenLandscape && wf) {
    dlgSize = wf->GetWidth();
  }
}

static bool OnTimerNotify(WndForm* pWnd) {
    double lon, lat;

    if (MapWindow::TargetMoved(lon, lat)) {
        MoveTarget(lon, lat);
    }
    if (TargetModified) {
        RefreshCalculator();
        TargetModified = false;
        ApplyTargetPanIfNeeded();
    }
    return true;
}

static void OnMoveClicked(WndButton* pWnd) {
  TargetMoveMode = !TargetMoveMode;
  if (TargetMoveMode) {
    btnMove->SetCaption(TEXT("Cursor"));
  } else {
    btnMove->SetCaption(TEXT("Move"));
  }
  RefreshCalculator();
  ApplyTargetPanIfNeeded();
}

/// Open Approach dialog for current target waypoint if it is landable.
/// Closes Target only if a task was created (Approve clicked); on Ignore the user returns to Target.
static void OnTargetApproachClicked(WndButton* pWnd) {
  int wp_index = -1;
  if (!CanOpenApproachForTargetPoint(target_point, &wp_index)) return;
  const bool task_created = dlgApproach(wp_index);
  if (task_created) {
    WndForm* targetForm = pWnd ? pWnd->GetParentWndForm() : nullptr;
    if (targetForm) targetForm->SetModalResult(mrOK);
  }
}

static void OnRangeData(DataField *Sender, DataField::DataAccessKind_t Mode) {
  double RangeNew;
  bool updated = false;
  switch(Mode){
    case DataField::daGet:
      //      Sender->Set(Range*100.0);
    break;
    case DataField::daPut:
    case DataField::daChange:
      LockTaskData();
      if (target_point>=ActiveTaskPoint) {
        RangeNew = Sender->GetAsFloat()/100.0;
        if (RangeNew != Range) {
          Task[target_point].AATTargetOffsetRadius = RangeNew;
          Range = RangeNew;
          updated = true;
        }
      }
      UnlockTaskData();
      if (updated) {
        TaskModified = true;
        TargetModified = true;
        // done by timer now        RefreshCalculator();
      }
    break;
  case DataField::daInc:
  case DataField::daDec:
  case DataField::daSpecial:
    break;
  }
}


static void OnRadialData(DataField *Sender, DataField::DataAccessKind_t Mode) {
  double RadialNew;
  bool updated = false;
  bool dowrap = false;
  switch(Mode){
    case DataField::daGet:
      //      Sender->Set(Range*100.0);
    break;
    case DataField::daPut:
    case DataField::daChange:
      LockTaskData();
      if (target_point>=ActiveTaskPoint) {
        if (!CALCULATED_INFO.IsInSector || (target_point != ActiveTaskPoint)) {
          dowrap = true;
        }
        RadialNew = Sender->GetAsFloat();
        if (fabs(RadialNew)>90) {
          if (dowrap) {
            RadialNew = AngleLimit180(RadialNew+180);
            // flip!
            Range = -Range;
            Task[target_point].AATTargetOffsetRadius =
              -Task[target_point].AATTargetOffsetRadius;
            updated = true;
          } else {
            RadialNew = max(-90.0,min(90.0,RadialNew));
            updated = true;
          }
        }
        if (RadialNew != Radial) {
          Task[target_point].AATTargetOffsetRadial = RadialNew;
          Radial = RadialNew;
          updated = true;
        }
      }
      UnlockTaskData();
      if (updated) {
        TaskModified = true;
        TargetModified = true;
        // done by timer now        RefreshCalculator();
      }
    break;
  case DataField::daInc:
  case DataField::daDec:
  case DataField::daSpecial:
    break;
  }
}


static void RefreshTargetPoint(void) {
  LockTaskData();
  target_point = max(target_point, ActiveTaskPoint);
  if (ValidTaskPoint(target_point)) {
    Range = Task[target_point].AATTargetOffsetRadius;
    Radial = Task[target_point].AATTargetOffsetRadial;
  } else {
    Range = 0;
    Radial = 0;
  }
  UnlockTaskData();
  RefreshCalculator();
}


static void OnLockedData(DataField *Sender, DataField::DataAccessKind_t Mode) {
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      {
      bool lockedthis = Sender->GetAsBoolean();
      if (ValidTaskPoint(target_point)) {
        if (Task[target_point].AATTargetLocked !=
            lockedthis) {
          TaskModified = true;
          TargetModified = true;
          Task[target_point].AATTargetLocked = lockedthis;
        }
      }
      }
    break;
  case DataField::daInc:
  case DataField::daDec:
  case DataField::daSpecial:
    break;
  }
}


static void OnTaskPointData(DataField *Sender, DataField::DataAccessKind_t Mode) {
  int old_target_point = target_point;
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      target_point = Sender->GetAsInteger() + ActiveWayPointOnEntry;
      target_point = max(target_point,ActiveTaskPoint);
      if (target_point != old_target_point) {
        RefreshTargetPoint();
      }
    break;
  case DataField::daInc:
  case DataField::daDec:
  case DataField::daSpecial:
    break;
  }
}


static CallBackTableEntry_t CallBackTable[]={
  CallbackEntry(OnTaskPointData),
  CallbackEntry(OnRangeData),
  CallbackEntry(OnRadialData),
  CallbackEntry(OnLockedData),
  CallbackEntry(OnOKClicked),
  CallbackEntry(OnMoveClicked),
  CallbackEntry(OnTargetApproachClicked),
  EndCallbackEntry()
};


void dlgTarget(int TaskPoint) {

  if(TaskPoint == -1)
	  TaskPoint =  ActiveTaskPoint;
  if (!ValidTaskPoint(TaskPoint)) {
    return;
  }
  target_point = TaskPoint;

  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_TARGET_L : IDR_XML_TARGET_P);

  if (!wf) return;

  TargetDialogOpen = true;
  TargetMoveMode = false;

  if (ScreenLandscape) {
    /* Full-height strip aligned to main map client (same idea as Approach). Do not use SetTop(0):
       on Kobo the map client is often offset — that caused the gap above the overlay. Portrait
       layout is handled separately; do not shrink strip height to content here (regression). */
    dlgSize = wf->GetWidth();
    const PixelRect rc(main_window->GetClientRect());
    const int client_h = rc.GetSize().cy;
    /* Full client height so the strip reaches the bottom (Approve etc. stay visible). Width unchanged. */
    const unsigned max_h = (unsigned)max(1, client_h);
    wf->SetHeight(max_h);
    wf->SetTop(rc.top);
    wf->SetLeft(rc.left + rc.GetSize().cx - (int)dlgSize);
    wf->SetCaption(wf->GetWndText());
  }
  else {
    wf->SetLeft(0);
  }

  btnMove = wf->FindByName<WindowControl>(TEXT("btnMove"));

  wf->SetKeyDownNotify(FormKeyDown);

  WndProperty *wp = wf->FindByName<WndProperty>(TEXT("prpTaskPoint"));
  DataField* dfe = wp->GetDataField();
  TCHAR tp_label[80];
  TCHAR tp_short[21];
  LockTaskData();
  if (!ValidTaskPoint(target_point)) {
    target_point = ActiveWayPointOnEntry;
  } else {
    target_point = max(target_point, ActiveWayPointOnEntry);
  }
  for (int i=ActiveWayPointOnEntry; i<MAXTASKPOINTS; i++) {
    if (ValidTaskPoint(i)) {
      LK_tcsncpy(tp_short, WayPointList[Task[i].Index].Name, 20);
      lk::snprintf(tp_label, TEXT("%d %s"), i, tp_short);
      dfe->addEnumText(tp_label);
    } else {
      if (target_point>= i) {
        target_point= ActiveWayPointOnEntry;
      }
    }
  }
  dfe->Set(max(0,target_point-ActiveWayPointOnEntry));
  UnlockTaskData();
  wp->RefreshDisplay();

  RefreshTargetPoint();

  if (!ScreenLandscape) {
    dlgApplyPortraitOverlayGeometry(wf);
    dlgSize = wf->GetHeight();
  }

  ApplyTargetPanIfNeeded();
  /* Map refresh: also done inside SetTargetPan(do_pan=true); keep for edge cases. */
  MapWindow::RefreshMap();

  wf->SetTimerNotify(500, OnTimerNotify);

  wf->ShowModal();

  MapWindow::SetTargetPan(false, 0);

  TargetDialogOpen = false;

  delete wf;
  wf = NULL;
}
