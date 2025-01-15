/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTaskWaypoint.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Logger.h"
#include "LKMapWindow.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "InputEvents.h"
#include "resource.h"

extern void ResetTaskWaypoint(int j);
static void SetValues(bool first = false) ;
static int twItemIndex= 0;
static WndForm *wf=NULL;
static int twType = 0; // start, turnpoint, finish


static WndFrame *wStart=NULL;
static WndFrame *wTurnpoint=NULL;
static WndFrame *wAATTurnpoint=NULL;
static WndFrame *wFinish=NULL;


//frmTaskPointPicto


BOOL bMoveallowed = false;
BOOL bAddonly = false;

static void SetWaypointValues(bool first=false) {
  WndProperty* wp;

  wp = wf->FindByName<WndProperty>(TEXT("prpAATType"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    auto sectors = get_task_sectors(gTaskType);
    if (first) {
      dfe->Clear();
      for (auto type : *sectors) {
        dfe->addEnumText(get_sectors_label(type));
      }
    }
    dfe->SetDetachGUI(true); // disable call to OnTaskType
    dfe->Set(sectors->index(Task[twItemIndex].AATType));
    dfe->SetDetachGUI(false);
    wp->RefreshDisplay();
  }

  WindowControl* pFrm = wf->FindByName(_T("frmCircle"));
  if(pFrm) {
    pFrm->SetVisible((Task[twItemIndex].AATType ==  sector_type_t::CIRCLE ) || (Task[twItemIndex].AATType == sector_type_t::ESS_CIRCLE));
  }
  pFrm = wf->FindByName(_T("frmSector"));
  if(pFrm) {
    pFrm->SetVisible(Task[twItemIndex].AATType == sector_type_t::SECTOR);
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAATCircleRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(Units::ToDistance(Task[twItemIndex].AATCircleRadius)*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAATSectorRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(Units::ToDistance(Task[twItemIndex].AATSectorRadius)*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAATStartRadial"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Task[twItemIndex].AATStartRadial);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAATFinishRadial"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Task[twItemIndex].AATFinishRadial);
    wp->RefreshDisplay();
  }
}

static void UpdateCaption(void) {

  if (!bMoveallowed) {
	  auto wMove = (wf->FindByName<WndButton>(TEXT("btMoveTurnpoint")));
    if (wMove) {
      wMove->SetVisible(FALSE);
    }
  }
  wStart     = (wf->FindByName<WndFrame>(TEXT("frmStart")));
  wTurnpoint = (wf->FindByName<WndFrame>(TEXT("frmTurnpoint")));
  wAATTurnpoint = (wf->FindByName<WndFrame>(TEXT("frmAATTurnpoint")));
  wFinish    = (wf->FindByName<WndFrame>(TEXT("frmFinish")));

  LKASSERT(wStart!=NULL);
  LKASSERT(wTurnpoint!=NULL);
  LKASSERT(wAATTurnpoint!=NULL);
  LKASSERT(wFinish!=NULL);

  WndButton* wb;
  if (bAddonly) {
    wb = wf->FindByName<WndButton>(TEXT("butSelect"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = wf->FindByName<WndButton>(TEXT("butRemove"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = wf->FindByName<WndButton>(TEXT("butDetails"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = wf->FindByName<WndButton>(TEXT("butDown"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = wf->FindByName<WndButton>(TEXT("butUp"));
    if (wb) {
      wb->SetVisible(false);
    }
  } else {
      wb = wf->FindByName<WndButton>(TEXT("butUp"));
      if (wb) {
        wb->SetVisible(ValidTaskPoint(twItemIndex-1));
      }

      wb = wf->FindByName<WndButton>(TEXT("butDown"));
      if (wb) {
        wb->SetVisible(ValidTaskPoint(twItemIndex+1));
      }

      wb = wf->FindByName<WndButton>(TEXT("butPrev"));
      if (wb) {
        wb->SetVisible(ValidTaskPoint(twItemIndex-1));
      }

      wb = wf->FindByName<WndButton>(TEXT("butNext"));
      if (wb) {
        wb->SetVisible(ValidTaskPoint(twItemIndex+1));
      }
  }

  SetWaypointValues(true);

  switch (twType) {
    case 0:
      wStart->SetVisible(1);
      wTurnpoint->SetVisible(0);
      wAATTurnpoint->SetVisible(0);
      wFinish->SetVisible(0);
      break;
    case 1:
      wStart->SetVisible(0);
      wTurnpoint->SetVisible(!UseAATTarget());
      wAATTurnpoint->SetVisible(UseAATTarget());
      wTurnpoint->SetVisible(1);
      wFinish->SetVisible(0);
    break;
    case 2:
      wStart->SetVisible(0);
      wTurnpoint->SetVisible(0);
      wAATTurnpoint->SetVisible(0);
      wFinish->SetVisible(1);
    break;
  }
  // set properties...

  SetValues(true);
//******************************************************/
  TCHAR sTmp[256];
  TCHAR title[128];
  if (ValidTaskPoint(twItemIndex)) {
    switch (twType) {
    case 0:
	// LKTOKEN  _@M657_ = "Start" 
      lk::strcpy(title, MsgToken<657>());
      break;
    case 1:
	// LKTOKEN  _@M749_ = "Turnpoint"
      _stprintf(title, TEXT("%s %i"),  MsgToken<749>(),twItemIndex);
      break;
    case 2:
	// LKTOKEN  _@M299_ = "Finish" 
      lk::strcpy(title, MsgToken<299>());
      break;
    };

    TCHAR landableStr[5] = TEXT(" [X]");
    // LKTOKEN _@M1238_ "L"
    landableStr[2] = MsgToken<1238>()[0];
    
    _stprintf(sTmp, TEXT("%s: %s%s"), title,
              WayPointList[Task[twItemIndex].Index].Name,
              (WayPointList[Task[twItemIndex].Index].Flags & LANDPOINT) ? landableStr : TEXT(""));
    wf->SetCaption(sTmp);
  } else {
	// LKTOKEN  _@M9_ = "(invalid)" 
    wf->SetCaption(MsgToken<9>());
  }
}

static void SetValues(bool first) {
  WndProperty* wp;

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFinishLine"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    auto sectors = get_finish_sectors(gTaskType);
    if (dfe && sectors) {
      dfe->Clear();
      for (auto type : *sectors) {
        dfe->addEnumText(get_sectors_label(type));
      }
      dfe->Set(sectors->index(FinishLine));
    }
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFinishRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(Units::ToDistance(FinishRadius)*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskStartLine"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    auto sectors = get_start_sectors(gTaskType);
    if (dfe && sectors) {
      dfe->Clear();
      for (auto type : *sectors) {
        dfe->addEnumText(get_sectors_label(type));
      }
      dfe->SetDetachGUI(true); // disable call to OnTaskType
      dfe->Set(sectors->index(StartLine));
      dfe->SetDetachGUI(false);
    }
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskStartRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(Units::ToDistance(StartRadius)*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFAISector"));
  if (wp) {
    // 110223 CAN ANYONE PLEASE CHECK WHAT THE HACK IS A BOOL FOR BILL GATES? BECAUSE IF FALSE IS -1 THEN
    // WE HAVE MANY PROBLEMS! I THINK IT IS TIME TO GO BACK TO bool AND GET RID OF MS BOOLS!!
    wp->SetVisible(gTaskType==TSK_DEFAULT);
    DataField* dfe = wp->GetDataField();
    auto sectors = get_task_sectors(TSK_DEFAULT);
    if (first) {
      dfe->Clear();
      for (auto type : *sectors) {
        dfe->addEnumText(get_sectors_label(type));
      }
    }
    dfe->SetDetachGUI(true); // disable call to OnTaskType
    dfe->Set(sectors->index(SectorType));
    dfe->SetDetachGUI(false);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskSectorRadius"));
  if (wp) {
    wp->SetVisible(gTaskType==TSK_DEFAULT);
    wp->GetDataField()->SetAsFloat(round(Units::ToDistance(SectorRadius)*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoAdvance"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if (first) {
      dfe->Clear();
      // LKTOKEN  _@M418_ = "Manual" 
      dfe->addEnumText(MsgToken<418>());
      // LKTOKEN _@M897_ "Auto"
      dfe->addEnumText(MsgToken<897>());
      // LKTOKEN  _@M97_ = "Arm" 
      dfe->addEnumText(MsgToken<97>());
      // LKTOKEN  _@M96_ = "Arm start" 
      dfe->addEnumText(MsgToken<96>());
      // LKTOKEN  _@M1798_ = "Arm TPs" 
      dfe->addEnumText(MsgToken<1798>());
    }
    dfe->Set(AutoAdvance);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMinTime"));
  if (wp) {
    wp->SetVisible(gTaskType==TSK_AAT);
    wp->GetDataField()->SetAsFloat(AATTaskLength);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpEnableMultipleStartPoints"));
  if (wp) {
    wp->SetVisible(gTaskType!=TSK_GP);
    wp->GetDataField()->Set(EnableMultipleStartPoints);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskType"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if (first) {
        dfe->Clear();
        // LKTOKEN  _@M1916_ "Default" 
        dfe->addEnumText(MsgToken<1916>());
        // LKTOKEN _@M1902_ "AAT"
        dfe->addEnumText(MsgToken<1918>());
        if(ISPARAGLIDER) {
            // LKTOKEN  _@M1904_ "Race To Goal / Min Time"
            dfe->addEnumText(MsgToken<1915>());
        } else {
            // LKTOKEN  _@M1903_ "Grand Prix"
            dfe->addEnumText(MsgToken<1914>());
        }
    }
    dfe->Set(gTaskType);
    wp->RefreshDisplay();      
  }

  WndButton* wb;
  wb = wf->FindByName<WndButton>(TEXT("EditStartPoints"));
  if (wb) {
    wb->SetVisible(EnableMultipleStartPoints!=0 && gTaskType!=TSK_GP);
  }

}

template<typename T>
bool CHECK_CHANGED(T& a, const T& b) {
  if (a != b) { 
    a = b; 
    return true;
  }
  return false;
}

static void GetWaypointValues(void) {
  WndProperty* wp;
  bool changed = false;

  if (!UseAATTarget()) {
    return;
  }

  if ((twItemIndex<MAXTASKPOINTS)&&(twItemIndex>=0)) {
    LockTaskData();
    wp = wf->FindByName<WndProperty>(TEXT("prpAATType"));
    if (wp) {
      auto dfe = wp->GetDataField();
      auto sectors = get_task_sectors(gTaskType);
      changed = CHECK_CHANGED(Task[twItemIndex].AATType, sectors->type(dfe->GetAsInteger()));
    }

    wp = wf->FindByName<WndProperty>(TEXT("prpAATCircleRadius"));
    if (wp) {
      changed = CHECK_CHANGED(Task[twItemIndex].AATCircleRadius,
                    round(Units::FromDistance(wp->GetDataField()->GetAsFloat())));
    }
    
    wp = wf->FindByName<WndProperty>(TEXT("prpAATSectorRadius"));
    if (wp) {
      changed = CHECK_CHANGED(Task[twItemIndex].AATSectorRadius,
                    round(Units::FromDistance(wp->GetDataField()->GetAsFloat())));
    }
    
    wp = wf->FindByName<WndProperty>(TEXT("prpAATStartRadial"));
    if (wp) {
      changed = CHECK_CHANGED(Task[twItemIndex].AATStartRadial,
                    round(wp->GetDataField()->GetAsFloat()));
    }
    
    wp = wf->FindByName<WndProperty>(TEXT("prpAATFinishRadial"));
    if (wp) {
      changed = CHECK_CHANGED(Task[twItemIndex].AATFinishRadial, 
                    round(wp->GetDataField()->GetAsFloat()));
    }

    if (changed) {
      TaskModified = true;
    }
    UnlockTaskData();

  }
}

static void ReadValues(void) {
  WndProperty* wp;
  bool changed = false;

  LockTaskData();
  wp = wf->FindByName<WndProperty>(TEXT("prpEnableMultipleStartPoints"));
  if (wp) {
    changed = CHECK_CHANGED(EnableMultipleStartPoints,
                  wp->GetDataField()->GetAsBoolean());
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskType"));
  if (wp) {
    changed = CHECK_CHANGED(gTaskType,
                  wp->GetDataField()->GetAsInteger());
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFinishLine"));
  if (wp) {
    auto dfe = wp->GetDataField();
    auto sectors = get_finish_sectors(gTaskType);
    changed = CHECK_CHANGED(FinishLine, sectors->type(dfe->GetAsInteger()));
  }
  
  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFinishRadius"));
  if (wp) {
    changed = CHECK_CHANGED(FinishRadius,
                  Units::FromDistance(wp->GetDataField()->GetAsFloat()));
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskStartLine"));
  if (wp) {
    auto dfe = wp->GetDataField();
    auto sectors = get_start_sectors(gTaskType);
    changed = CHECK_CHANGED(StartLine, sectors->type(dfe->GetAsInteger()));
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskStartRadius"));
  if (wp) {
    changed = CHECK_CHANGED(StartRadius,
                  Units::FromDistance(wp->GetDataField()->GetAsFloat()));
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFAISector"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    auto sectors = get_task_sectors(TSK_DEFAULT);
    changed = CHECK_CHANGED(SectorType, sectors->type(dfe->GetAsInteger()));
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskSectorRadius"));
  if (wp) {
    changed = CHECK_CHANGED(SectorRadius,
                  round(Units::FromDistance(wp->GetDataField()->GetAsFloat())));
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoAdvance"));
  if (wp) {
    changed = CHECK_CHANGED(AutoAdvance, 
                  wp->GetDataField()->GetAsInteger());
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMinTime"));
  if (wp) {
    changed = CHECK_CHANGED(AATTaskLength, 
                  wp->GetDataField()->GetAsFloat());
    if (changed) {
      CALCULATED_INFO.AATTimeToGo=AATTaskLength*60;
    }
  }
  if (changed) {
    TaskModified = true;
  }

  UnlockTaskData();

}

static void OnTaskType(DataField *Sender, DataField::DataAccessKind_t Mode) {
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      ReadValues();
      SetValues();
      GetWaypointValues();
      SetWaypointValues();
      RefreshTask();
    break;
  case DataField::daInc:
  case DataField::daDec:
  case DataField::daSpecial:
    break;
  }
}



static void OnSelectClicked(WndButton* pWnd) {
  int res = dlgSelectWaypoint();
  if (res != -1){
    SelectedWaypoint = res;    
    LKASSERT(twItemIndex>=0);
    if (Task[twItemIndex].Index != res) {
      if (CheckDeclaration()) {
        LockTaskData();
        ResetTaskWaypoint(twItemIndex);
        Task[twItemIndex].Index = res;
        TaskModified = true;
        UnlockTaskData();
      }
    }
    UpdateCaption();
  };
}

static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnMoveClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
  MapWindow::SetPanTaskEdit(twItemIndex);
}

static void OnStartPointClicked(WndButton* pWnd){
    dlgStartPointShowModal();
}


static void OnMoveAfterClicked(WndButton* pWnd){
  LockTaskData();
  SwapWaypoint(twItemIndex);
  SetWaypointValues();
  UnlockTaskData();
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnMoveBeforeClicked(WndButton* pWnd){
  LockTaskData();
  SwapWaypoint(twItemIndex-1);
  SetWaypointValues();
  UnlockTaskData();
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnDetailsClicked(WndButton* pWnd){
  LKASSERT(twItemIndex>=0);
  SelectedWaypoint = Task[twItemIndex].Index;
  dlgWayPointDetailsShowModal(0);
}

static void OnRemoveClicked(WndButton* pWnd) {
  LockTaskData();
  RemoveTaskPoint(twItemIndex);
  SetWaypointValues();
  if (ActiveTaskPoint>=twItemIndex) {
    ActiveTaskPoint--;
  }
  if (ActiveTaskPoint<0) {
    ActiveTaskPoint= -1;
  }
  UnlockTaskData();
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void OnTaskRulesClicked(WndButton* pWnd){
  wf->SetVisible(false);
  if (dlgTaskRules()) {
    TaskModified = true;
  }
  wf->SetVisible(true);
}

static void OnTaskPointPicto(WndOwnerDrawFrame * Sender, LKSurface& Surface) {

    WndFrame *wPicto = (wf->FindByName<WndFrame>(TEXT("frmTaskPointPicto")));
    if(wPicto) {
        /********************/
        ReadValues();
        GetWaypointValues();
        //  CalculateTaskSectors();
        //  if (AATEnabled)
        //    CalculateAATTaskSectors();
        RefreshTask();
        /*******************/
        const RECT rc = wPicto->GetClientRect();

        MapWindow::DrawWaypointPictoBg(Surface, rc);
        MapWindow::DrawTaskPicto(Surface, twItemIndex, rc);
    }
}




static void OnPrevClicked(WndButton* pWnd){

 LockTaskData();

  if (ValidTaskPoint(twItemIndex-1))
  {
    twItemIndex--;
  }
  if (twItemIndex<0) {
      twItemIndex= 0;
  }

  twType =1;
  if (!ValidTaskPoint(twItemIndex+1))
    twType = 2;
  if(twItemIndex == 0)
    twType =0;


  UpdateCaption();
  UnlockTaskData();
}


static void OnNextClicked(WndButton* pWnd){
  LockTaskData();

  if (ValidTaskPoint(twItemIndex+1))
  {
    twItemIndex++;
  }

  twType =1;
  if (!ValidTaskPoint(twItemIndex+1))
    twType = 2;
  if(twItemIndex == 0)
    twType =0;

  UpdateCaption();
  UnlockTaskData();
}

static CallBackTableEntry_t CallBackTable[]={

  ClickNotifyCallbackEntry(OnMoveClicked),
  ClickNotifyCallbackEntry(OnSelectClicked),
  ClickNotifyCallbackEntry(OnDetailsClicked),
  ClickNotifyCallbackEntry(OnRemoveClicked),
  ClickNotifyCallbackEntry(OnCloseClicked),
  ClickNotifyCallbackEntry(OnStartPointClicked),
  ClickNotifyCallbackEntry(OnMoveAfterClicked),
  ClickNotifyCallbackEntry(OnMoveBeforeClicked),


  ClickNotifyCallbackEntry(OnPrevClicked),
  ClickNotifyCallbackEntry(OnNextClicked),

  DataAccessCallbackEntry(OnTaskType),
  ClickNotifyCallbackEntry(OnTaskRulesClicked),
  OnPaintCallbackEntry(OnTaskPointPicto),
  EndCallBackEntry()
};


void dlgTaskWaypointShowModal(int itemindex, int tasktype, bool addonly, bool Moveallowed){

    wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_TASKWAYPOINT_L : IDR_XML_TASKWAYPOINT_P);

  bMoveallowed = Moveallowed;
  bAddonly = addonly;

  if (gTaskType!=TSK_GP) {
	EnableMultipleStartPoints=false;
  }

  twItemIndex = itemindex;
  twType = tasktype;

  if (!wf) return;

  //ASSERT(wf!=NULL);
  //  wf->SetKeyDownNotify(FormKeyDown);
  wStart     = (wf->FindByName<WndFrame>(TEXT("frmStart")));
  wTurnpoint = (wf->FindByName<WndFrame>(TEXT("frmTurnpoint")));
  wAATTurnpoint = (wf->FindByName<WndFrame>(TEXT("frmAATTurnpoint")));
  wFinish    = (wf->FindByName<WndFrame>(TEXT("frmFinish")));

  LKASSERT(wStart!=NULL);
  LKASSERT(wTurnpoint!=NULL);
  LKASSERT(wAATTurnpoint!=NULL);
  LKASSERT(wFinish!=NULL);

  WndButton* wb;
  if (addonly) {
    wb = wf->FindByName<WndButton>(TEXT("butSelect"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = wf->FindByName<WndButton>(TEXT("butRemove"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = wf->FindByName<WndButton>(TEXT("butDetails"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = wf->FindByName<WndButton>(TEXT("butDown"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = wf->FindByName<WndButton>(TEXT("butUp"));
    if (wb) {
      wb->SetVisible(false);
    }
  } else {
    if (!ValidTaskPoint(twItemIndex-1)) {
      wb = wf->FindByName<WndButton>(TEXT("butUp"));
      if (wb) {
        wb->SetVisible(false);
      }
    }
    if (!ValidTaskPoint(twItemIndex+1)) {
      wb = wf->FindByName<WndButton>(TEXT("butDown"));
      if (wb) {
        wb->SetVisible(false);
      }
    }
  }

  SetWaypointValues(true);

  switch (twType) {
    case 0:
      wStart->SetVisible(1);
      wTurnpoint->SetVisible(0);
      wAATTurnpoint->SetVisible(0);
      wFinish->SetVisible(0);
      break;
    case 1:
      wStart->SetVisible(0);
      if (UseAATTarget()) {
	wTurnpoint->SetVisible(0);
	wAATTurnpoint->SetVisible(1);
      } else {
	wTurnpoint->SetVisible(1);
	wAATTurnpoint->SetVisible(0);
      }
      wTurnpoint->SetVisible(1);
      wFinish->SetVisible(0);
    break;
    case 2:
      wStart->SetVisible(0);
      wTurnpoint->SetVisible(0);
      wAATTurnpoint->SetVisible(0);
      wFinish->SetVisible(1);
    break;
  }
  // set properties...

  SetValues(true);

  UpdateCaption();

  wf->ShowModal();

  // now retrieve changes

  GetWaypointValues();

  ReadValues();

  delete wf;

  wf = NULL;

}


