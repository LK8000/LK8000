/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTarget.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "dlgTools.h"
#include "Calculations2.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "Event/Event.h"

static WndForm *wf=NULL;
static WindowControl *btnMove = NULL;
static int ActiveWayPointOnEntry = 0;


static double Range = 0;
static double Radial = 0;
static int target_point = 0;
static bool TargetMoveMode = false;

static DWORD dlgSize = 0;

bool TargetDialogOpen = false;

static void OnOKClicked(WndButton* pWnd) {
  wf->SetModalResult(mrOK);
}

static void MoveTarget(double adjust_angle) {
  if (!AATEnabled) return;
  if (target_point==0) return;
  if (!ValidTaskPoint(target_point)) return;
  if (!ValidTaskPoint(target_point+1)) return;
  if (target_point < ActiveWayPoint) return;

  LockTaskData();

  double target_latitude, target_longitude;
  double bearing, distance;
  distance = 500;
  if(Task[target_point].AATType == SECTOR) {
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

  if (InAATTurnSector(target_longitude, target_latitude, target_point, 0)) {
    if (CALCULATED_INFO.IsInSector && (target_point == ActiveWayPoint)) {
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
      if(Task[target_point].AATType == SECTOR) {
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
  if (!AATEnabled) return;
  if (target_point==0) return;
  if (!ValidTaskPoint(target_point)) return;
  if (!ValidTaskPoint(target_point+1)) return;
  if (target_point < ActiveWayPoint) return;

  LockTaskData();

  double distance, bearing;

  if (InAATTurnSector(target_longitude, target_latitude, target_point, 0)) {
    if (CALCULATED_INFO.IsInSector && (target_point == ActiveWayPoint)) {
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
      if(Task[target_point].AATType == SECTOR) {
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

static bool FormKeyDown(Window* pWnd, unsigned KeyCode) {
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
        StartupStore(TEXT("... moving%s"), NEWLINE);
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



static void RefreshCalculator(void) {
  WndProperty* wp;

  RefreshTask();
  RefreshTaskStatistics();
  target_point = max(target_point,ActiveWayPoint);

  bool nodisplay = !AATEnabled 
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskPoint"));
  if (wp) {
    if (TargetMoveMode) {
      wp->SetVisible(false);
    } else {
      wp->SetVisible(true);
    }
  }

  WindowControl* wc = (WindowControl*)wf->FindByName(TEXT("btnOK"));
  if (wc) {
    if (TargetMoveMode) {
      wc->SetVisible(false);
    } else {
      wc->SetVisible(true);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAATTargetLocked"));
  if (wp) {
    wp->GetDataField()->Set(Task[target_point].AATTargetLocked);
    wp->RefreshDisplay();
    if (nodisplay) {
      wp->SetVisible(false);
    } else {
      wp->SetVisible(true);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRange"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Range*100.0);
    wp->RefreshDisplay();
    if (nodisplay) {
      wp->SetVisible(false);
    } else {
      wp->SetVisible(true);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRadial"));
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
  wp = (WndProperty*)wf->FindByName(TEXT("prpAATEst"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(dd);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAATDelta"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(dd-AATTaskLength);
    if (AATEnabled) {
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpSpeedRemaining"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(v1*TASKSPEEDMODIFY);
    wp->GetDataField()->SetUnits(Units::GetTaskSpeedName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSpeedAchieved"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(CALCULATED_INFO.TaskSpeed*TASKSPEEDMODIFY);
    wp->GetDataField()->SetUnits(Units::GetTaskSpeedName());
    wp->RefreshDisplay();
  }

}

static bool OnTimerNotify() {
    double lon, lat;

    if (MapWindow::TargetMoved(lon, lat)) {
        MoveTarget(lon, lat);
    }
    if (TargetModified) {
        RefreshCalculator();
        TargetModified = false;
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
      if (target_point>=ActiveWayPoint) {
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
      if (target_point>=ActiveWayPoint) {
        if (!CALCULATED_INFO.IsInSector || (target_point != ActiveWayPoint)) {
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
  target_point = max(target_point, ActiveWayPoint);
  if (ValidTaskPoint(target_point)) {
    MapWindow::SetTargetPan(true, target_point, dlgSize);
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
      target_point = max(target_point,ActiveWayPoint);
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
  DataAccessCallbackEntry(OnTaskPointData),
  DataAccessCallbackEntry(OnRangeData),
  DataAccessCallbackEntry(OnRadialData),
  DataAccessCallbackEntry(OnLockedData),
  ClickNotifyCallbackEntry(OnOKClicked),
  ClickNotifyCallbackEntry(OnMoveClicked),
  EndCallBackEntry()
};


void dlgTarget(int TaskPoint) {

  if(TaskPoint == -1)
	  TaskPoint =  ActiveWayPoint;
  if (!ValidTaskPoint(TaskPoint)) {
    return;
  }
  target_point = TaskPoint;

  if (!ScreenLandscape) {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgTarget_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        TEXT("IDR_XML_TARGET_L"));
  } else {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgTarget.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        TEXT("IDR_XML_TARGET"));
  }

  if (!wf) return;

  TargetDialogOpen = true;
  TargetMoveMode = false;

  WndFrame *wf2 = (WndFrame*)wf->FindByName(TEXT("frmTarget"));
  if (wf2) {
    if (ScreenLandscape) 
    {// make flush right in landscape mode (at top in portrait mode)
      dlgSize = wf2->GetWidth();
      wf->SetLeft(MainWindow.GetRight() - dlgSize);
    }
    else {
      dlgSize = wf2->GetHeight();
    }
  }

  btnMove = (WindowControl*)wf->FindByName(TEXT("btnMove"));

  wf->SetKeyDownNotify(FormKeyDown);

  WndProperty *wp;
  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskPoint"));
  DataFieldEnum* dfe;
  dfe = (DataFieldEnum*)wp->GetDataField();
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
      _stprintf(tp_label, TEXT("%d %s"), i, tp_short);
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

  wf->SetTimerNotify(500, OnTimerNotify);

  wf->ShowModal();

  MapWindow::SetTargetPan(false, 0);

  TargetDialogOpen = false;

  delete wf;
  wf = NULL;
}
