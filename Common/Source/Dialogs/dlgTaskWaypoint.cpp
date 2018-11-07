/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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

static int twItemIndex= 0;
static WndForm *wf=NULL;
static int twType = 0; // start, turnpoint, finish


static WndFrame *wMove=NULL;
static WndFrame *wStart=NULL;
static WndFrame *wTurnpoint=NULL;
static WndFrame *wAATTurnpoint=NULL;
static WndFrame *wFinish=NULL;


//frmTaskPointPicto
static void UpdateCaption(void) {
  TCHAR sTmp[128];
  TCHAR title[128];
  if (ValidTaskPoint(twItemIndex)) {
    switch (twType) {
    case 0:
	// LKTOKEN  _@M657_ = "Start" 
      _tcscpy(title, MsgToken(657));
      break;
    case 1:
	// LKTOKEN  _@M749_ = "Turnpoint" 
      _tcscpy(title, MsgToken(749));
      break;
    case 2:
	// LKTOKEN  _@M299_ = "Finish" 
      _tcscpy(title, MsgToken(299));
      break;
    };

    TCHAR landableStr[5] = TEXT(" [X]");
    // LKTOKEN _@M1238_ "L"
    landableStr[2] = MsgToken(1238)[0];
    
    _stprintf(sTmp, TEXT("%s: %s%s"), title,
              WayPointList[Task[twItemIndex].Index].Name,
              (WayPointList[Task[twItemIndex].Index].Flags & LANDPOINT) ? landableStr : TEXT(""));
    wf->SetCaption(sTmp);
  } else {
	// LKTOKEN  _@M9_ = "(invalid)" 
    wf->SetCaption(MsgToken(9));
  }
}

static void SetValues(bool first=false) {
  WndProperty* wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishLine"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if (first) {
	// LKTOKEN  _@M210_ = "Cylinder" 
      dfe->addEnumText(MsgToken(210));
	// LKTOKEN  _@M393_ = "Line" 
      dfe->addEnumText(MsgToken(393));
	// LKTOKEN  _@M274_ = "FAI Sector" 
      dfe->addEnumText(MsgToken(274));
    }
    dfe->Set(FinishLine);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(FinishRadius*DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartLine"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if (first) {
	// LKTOKEN  _@M210_ = "Cylinder" 
      dfe->addEnumText(MsgToken(210));
	// LKTOKEN  _@M393_ = "Line" 
      dfe->addEnumText(MsgToken(393));
	// LKTOKEN  _@M274_ = "FAI Sector" 
      dfe->addEnumText(MsgToken(274));
    }
    dfe->SetDetachGUI(true); // disable call to OnAATEnabled
    dfe->Set(StartLine);
    dfe->SetDetachGUI(false);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(StartRadius*DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFAISector"));
  if (wp) {
    // 110223 CAN ANYONE PLEASE CHECK WHAT THE HACK IS A BOOL FOR BILL GATES? BECAUSE IF FALSE IS -1 THEN
    // WE HAVE MANY PROBLEMS! I THINK IT IS TIME TO GO BACK TO bool AND GET RID OF MS BOOLS!!
  //  wp->SetVisible((AATEnabled==0) || (twItemIndex >0) );
    DataField* dfe = wp->GetDataField();
    if (first) {
	// LKTOKEN  _@M210_ = "Cylinder" 
      dfe->addEnumText(MsgToken(210));
	// LKTOKEN  _@M274_ = "FAI Sector" 
      dfe->addEnumText(MsgToken(274));
      dfe->addEnumText(LKGetText(TEXT("DAe 0.5/10")));
      	// LKTOKEN  _@M393_ = "Line" 
      dfe->addEnumText(MsgToken(393));
    }
    dfe->SetDetachGUI(true); // disable call to OnAATEnabled
    dfe->Set(SectorType);
    dfe->SetDetachGUI(false);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskSectorRadius"));
  if (wp) {
    wp->SetVisible(AATEnabled==0);
    wp->GetDataField()->SetAsFloat(round(SectorRadius*DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoAdvance"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if (first) {
	// LKTOKEN  _@M418_ = "Manual" 
      dfe->addEnumText(MsgToken(418));
	// LKTOKEN _@M897_ "Auto"
      dfe->addEnumText(MsgToken(897));
	// LKTOKEN  _@M97_ = "Arm" 
      dfe->addEnumText(MsgToken(97));
	// LKTOKEN  _@M96_ = "Arm start" 
      dfe->addEnumText(MsgToken(96));
	// LKTOKEN  _@M1798_ = "Arm TPs" 
      dfe->addEnumText(MsgToken(1798));
    }
    dfe->Set(AutoAdvance);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMinTime"));
  if (wp) {
    wp->SetVisible(AATEnabled>0 && (!PGOptimizeRoute || !ISPARAGLIDER));
    wp->GetDataField()->SetAsFloat(AATTaskLength);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableMultipleStartPoints"));
  if (wp) {
    wp->SetVisible(!ISPARAGLIDER);
    wp->GetDataField()->Set(EnableMultipleStartPoints);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAATEnabled"));
  if (wp) {
	if (ISPARAGLIDER && PGOptimizeRoute) {
		wp->SetVisible(false);
		AATEnabled=true;
		wp->RefreshDisplay(); 
	} else {
		bool aw = (AATEnabled != 0);
		wp->GetDataField()->Set(aw);
		wp->RefreshDisplay(); 
	}
  }

  WndButton* wb;
  wb = (WndButton *)wf->FindByName(TEXT("EditStartPoints"));
  if (wb) {
    wb->SetVisible(EnableMultipleStartPoints!=0 && !ISPARAGLIDER);
  }

}

#define CHECK_CHANGED(a,b) if (a != b) { changed = true; a = b; }

static void GetWaypointValues(void) {
  WndProperty* wp;
  bool changed = false;

  if (!AATEnabled) {
    return;
  }

  if ((twItemIndex<MAXTASKPOINTS)&&(twItemIndex>=0)) {
    LockTaskData();
    wp = (WndProperty*)wf->FindByName(TEXT("prpAATType"));
    if (wp) {
      CHECK_CHANGED(Task[twItemIndex].AATType, 
                    wp->GetDataField()->GetAsInteger());
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpAATCircleRadius"));
    if (wp) {
      CHECK_CHANGED(Task[twItemIndex].AATCircleRadius,
                    iround(wp->GetDataField()->GetAsFloat()/DISTANCEMODIFY));
    }
    
    wp = (WndProperty*)wf->FindByName(TEXT("prpAATSectorRadius"));
    if (wp) {
      CHECK_CHANGED(Task[twItemIndex].AATSectorRadius,
                    iround(wp->GetDataField()->GetAsFloat()/DISTANCEMODIFY));
    }
    
    wp = (WndProperty*)wf->FindByName(TEXT("prpAATStartRadial"));
    if (wp) {
      CHECK_CHANGED(Task[twItemIndex].AATStartRadial,
                    wp->GetDataField()->GetAsInteger());
    }
    
    wp = (WndProperty*)wf->FindByName(TEXT("prpAATFinishRadial"));
    if (wp) {
      CHECK_CHANGED(Task[twItemIndex].AATFinishRadial, 
                    wp->GetDataField()->GetAsInteger());
    }  
	
	wp = (WndProperty*)wf->FindByName(TEXT("prpOutCircle"));
	if (wp) {
		CHECK_CHANGED(Task[twItemIndex].OutCircle,
			wp->GetDataField()->GetAsInteger());
	}

   	wp = (WndProperty*)wf->FindByName(TEXT("prpConeSlope"));
	if (wp) {
		CHECK_CHANGED(Task[twItemIndex].PGConeSlope,
			wp->GetDataField()->GetAsFloat());
	}

  	wp = (WndProperty*)wf->FindByName(TEXT("prpConeBase"));
	if (wp) {
		CHECK_CHANGED(Task[twItemIndex].PGConeBase,
			wp->GetDataField()->GetAsFloat()/ALTITUDEMODIFY);
	}
  	
    wp = (WndProperty*)wf->FindByName(TEXT("prpConeRadius"));
	if (wp) {
		CHECK_CHANGED(Task[twItemIndex].PGConeBaseRadius,
			wp->GetDataField()->GetAsFloat()/DISTANCEMODIFY);
	}


    if (changed) {
      TaskModified = true;
    }
    UnlockTaskData();

  }
}


static void SetWaypointValues(bool first=false) {
  WndProperty* wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpAATType"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if (first) {
	// LKTOKEN  _@M210_ = "Cylinder" 
      dfe->addEnumText(MsgToken(210));
	// LKTOKEN  _@M590_ = "Sector" 
      dfe->addEnumText(MsgToken(590));
      if(DoOptimizeRoute()) {
        // Conical ESS
        dfe->addEnumText(MsgToken(2175));
        // Circle ESS
        dfe->addEnumText(MsgToken(2189));
      }
    }
    dfe->SetDetachGUI(true); // disable call to OnAATEnabled
    dfe->Set(Task[twItemIndex].AATType);
    dfe->SetDetachGUI(false);
    wp->RefreshDisplay();
  }

  WindowControl* pFrm = wf->FindByName(_T("frmCircle"));
  if(pFrm) {
    pFrm->SetVisible((Task[twItemIndex].AATType==0) || (Task[twItemIndex].AATType==3));
  }
  pFrm = wf->FindByName(_T("frmSector"));
  if(pFrm) {
    pFrm->SetVisible(Task[twItemIndex].AATType==1);
  }
  pFrm = wf->FindByName(_T("frmCone"));
  if(pFrm) {
    pFrm->SetVisible(Task[twItemIndex].AATType==2);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAATCircleRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(Task[twItemIndex].AATCircleRadius
                                          *DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAATSectorRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(Task[twItemIndex].AATSectorRadius
                                          *DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();    
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAATStartRadial"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Task[twItemIndex].AATStartRadial);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAATFinishRadial"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Task[twItemIndex].AATFinishRadial);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOutCircle"));
  if (wp) {
	  DataField* dfe = wp->GetDataField();
	  if (dfe) {
		  if (first) {
			  // LKTOKEN  _@M2226_ = "Enter" 
			  dfe->addEnumText(MsgToken(2145));
			  // LKTOKEN  _@M2227_ = "Exit" 
			  dfe->addEnumText(MsgToken(2146));
		  }
		  dfe->Set(Task[twItemIndex].OutCircle);
	  }
	  wp->SetVisible(DoOptimizeRoute());
	  wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpConeSlope"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Task[twItemIndex].PGConeSlope);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpConeBase"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Task[twItemIndex].PGConeBase*ALTITUDEMODIFY);
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpConeRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(Task[twItemIndex].PGConeBaseRadius
                                          *DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }
  
}


static void ReadValues(void) {
  WndProperty* wp;
  bool changed = false;

  LockTaskData();
  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableMultipleStartPoints"));
  if (wp) {
    CHECK_CHANGED(EnableMultipleStartPoints,
                  wp->GetDataField()->GetAsBoolean());
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAATEnabled"));
  if (wp) {
    CHECK_CHANGED(AATEnabled,
                  wp->GetDataField()->GetAsBoolean());
	if (DoOptimizeRoute()) AATEnabled=true; // force it on
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishLine"));
  if (wp) {
    CHECK_CHANGED(FinishLine,
                  wp->GetDataField()->GetAsInteger());
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishRadius"));
  if (wp) {
    CHECK_CHANGED(FinishRadius, wp->GetDataField()->GetAsFloat() / DISTANCEMODIFY);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartLine"));
  if (wp) {
    CHECK_CHANGED(StartLine, 
                  wp->GetDataField()->GetAsInteger());
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartRadius"));
  if (wp) {
    CHECK_CHANGED(StartRadius, wp->GetDataField()->GetAsFloat() / DISTANCEMODIFY);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFAISector"));
  if (wp) {
    CHECK_CHANGED(SectorType,
                  wp->GetDataField()->GetAsInteger());
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskSectorRadius"));
  if (wp) {
    CHECK_CHANGED(SectorRadius,
                  (unsigned)iround(wp->GetDataField()->GetAsFloat()
				/DISTANCEMODIFY));
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoAdvance"));
  if (wp) {
    CHECK_CHANGED(AutoAdvance, 
                  wp->GetDataField()->GetAsInteger());
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMinTime"));
  if (wp) {
    CHECK_CHANGED(AATTaskLength, 
                  wp->GetDataField()->GetAsInteger());
	if (changed) CALCULATED_INFO.AATTimeToGo=AATTaskLength*60; 
  }
  if (changed) {
    TaskModified = true;
  }

  UnlockTaskData();

}

static void OnAATEnabled(DataField *Sender, DataField::DataAccessKind_t Mode) {
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



static void OnSelectClicked(WndButton* pWnd){
  int res;
  res = dlgWayPointSelect();
  if (res != -1){
    SelectedWaypoint = res;    
    LKASSERT(twItemIndex>=0);
    if (Task[twItemIndex].Index != res) {
      if (CheckDeclaration()) {
        LockTaskData();
        ResetTaskWaypoint(twItemIndex);
        Task[twItemIndex].Index = res;
        Task[twItemIndex].PGConeBase = WayPointList[res].Altitude;
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

static void OnTaskPointPicto(WindowControl * Sender, LKSurface& Surface) {
    (void) Sender;

    WndFrame *wPicto = ((WndFrame *) wf->FindByName(TEXT("frmTaskPointPicto")));
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
        MapWindow::DrawTaskPicto(Surface, twItemIndex, rc, 2000);
    }
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
  DataAccessCallbackEntry(OnAATEnabled),
  ClickNotifyCallbackEntry(OnTaskRulesClicked),
  OnPaintCallbackEntry(OnTaskPointPicto),
  EndCallBackEntry()
};


void dlgTaskWaypointShowModal(int itemindex, int tasktype, bool addonly, bool Moveallowed){

    wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_TASKWAYPOINT_L : IDR_XML_TASKWAYPOINT_P);

  if (ISPARAGLIDER) {
    if(DoOptimizeRoute()) 
		AATEnabled=TRUE;
	EnableMultipleStartPoints=false;
  }

  twItemIndex = itemindex;
  twType = tasktype;

  if (!wf) return;

  //ASSERT(wf!=NULL);
  //  wf->SetKeyDownNotify(FormKeyDown);
  if(!Moveallowed)
  {
	wMove    = ((WndFrame *)wf->FindByName(TEXT("frmMoveTurnpoint")));
        LKASSERT(wMove!=NULL);
    wMove->SetVisible(FALSE);
  }
  wStart     = ((WndFrame *)wf->FindByName(TEXT("frmStart")));
  wTurnpoint = ((WndFrame *)wf->FindByName(TEXT("frmTurnpoint")));
  wAATTurnpoint = ((WndFrame *)wf->FindByName(TEXT("frmAATTurnpoint")));
  wFinish    = ((WndFrame *)wf->FindByName(TEXT("frmFinish")));

  LKASSERT(wStart!=NULL);
  LKASSERT(wTurnpoint!=NULL);
  LKASSERT(wAATTurnpoint!=NULL);
  LKASSERT(wFinish!=NULL);

  WndButton* wb;
  if (addonly) {
    wb = (WndButton *)wf->FindByName(TEXT("butSelect"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = (WndButton *)wf->FindByName(TEXT("butRemove"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = (WndButton *)wf->FindByName(TEXT("butDetails"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = (WndButton *)wf->FindByName(TEXT("butDown"));
    if (wb) {
      wb->SetVisible(false);
    }
    wb = (WndButton *)wf->FindByName(TEXT("butUp"));
    if (wb) {
      wb->SetVisible(false);
    }
  } else {
    if (!ValidTaskPoint(twItemIndex-1)) {
      wb = (WndButton *)wf->FindByName(TEXT("butUp"));
      if (wb) {
        wb->SetVisible(false);
      }
    }
    if (!ValidTaskPoint(twItemIndex+1)) {
      wb = (WndButton *)wf->FindByName(TEXT("butDown"));
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
      if (AATEnabled) {
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



