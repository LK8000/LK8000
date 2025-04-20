/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTaskCalculator.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/
#include "externs.h"
#include "McReady.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "Calculations2.h"
#include "resource.h"


static WndForm *wf=NULL;

extern double CRUISE_EFFICIENCY;

static double emc= 0.0;
static double cruise_efficiency= 1.0;

static void OnCancelClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrCancel);
    }
  }
}

static void OnOKClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static double Range = 0;


static void GetCruiseEfficiency(void) {
  if ((CALCULATED_INFO.Flying) && (CALCULATED_INFO.TaskStartTime>0) &&
      !(CALCULATED_INFO.FinalGlide &&
	(CALCULATED_INFO.TaskAltitudeDifference>0))) {

    cruise_efficiency = EffectiveCruiseEfficiency(&GPS_INFO, &CALCULATED_INFO);
  } else {
    cruise_efficiency = CRUISE_EFFICIENCY;
  }
}

static void RefreshCalculator(void) {
  WndProperty* wp;

  RefreshTask();
  RefreshTaskStatistics();

  // update outputs
  wp = wf->FindByName<WndProperty>(TEXT("prpAATEst"));
  if (wp) {
    double dd = CALCULATED_INFO.TaskTimeToGo;
    if ((CALCULATED_INFO.TaskStartTime>0.0)&&(CALCULATED_INFO.Flying)) {
      dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
    }
    dd= min(24.0*60.0,dd/60.0);
    wp->GetDataField()->SetAsFloat(dd);
    wp->RefreshDisplay();
  }

  // update outputs
  wp = wf->FindByName<WndProperty>(TEXT("prpAATTime"));
  if (wp) {
    if (!(gTaskType == task_type_t::AAT)) {
      wp->SetVisible(false);
    } else {
      wp->GetDataField()->SetAsFloat(AATTaskLength);
    }
      wp->RefreshDisplay();
  }

  double d1 = (CALCULATED_INFO.TaskDistanceToGo
	       +CALCULATED_INFO.TaskDistanceCovered);
  if (gTaskType == task_type_t::AAT && (d1==0.0)) {
    d1 = CALCULATED_INFO.AATTargetDistance;
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpDistance"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Units::ToDistance(d1));
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMacCready"));
  if (wp) {
    wp->GetDataField()->SetUnits(Units::GetVerticalSpeedName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpEffectiveMacCready"));
  if (wp) {
    wp->GetDataField()->SetUnits(Units::GetVerticalSpeedName());
    wp->GetDataField()->SetAsFloat(Units::ToVerticalSpeed(emc));
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpRange"));
  if (wp) {
    wp->RefreshDisplay();
    if (gTaskType != task_type_t::AAT || !ValidTaskPoint(ActiveTaskPoint+1)) {
      wp->SetVisible(false);
    } else {
      wp->SetVisible(true);
    }
    wp->GetDataField()->SetAsFloat(Range*100.0);
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

  wp = wf->FindByName<WndProperty>(TEXT("prpCruiseEfficiency"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(cruise_efficiency*100.0);
    wp->RefreshDisplay();
  }
}


extern bool TargetDialogOpen;

static void DoOptimise(void) {
  double myrange= Range;
  double RangeLast= Range;
  double deltaTlast = 0;
  int steps = 0;
  if (gTaskType != task_type_t::AAT) return;

  LockFlightData();
  LockTaskData();
  TargetDialogOpen = true;
  do {
    myrange = Range;
    AdjustAATTargets(Range);
    RefreshTask();
    RefreshTaskStatistics();
    double deltaT = CALCULATED_INFO.TaskTimeToGo;
    if ((CALCULATED_INFO.TaskStartTime>0.0)&&(CALCULATED_INFO.Flying)) {
      deltaT += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
    }
    deltaT= min(24.0*60.0,deltaT/60.0)-AATTaskLength-5;

    double dRdT = 0.001;
    if (steps>0) {
      if (fabs(deltaT-deltaTlast)>0.01) {
        dRdT = min(0.5,(Range-RangeLast)/(deltaT-deltaTlast));
        if (dRdT<=0.0) {
          // error, time decreases with increasing range!
          // or, no effect on time possible
          break;
        }
      } else {
        // minimal change for whatever reason
        // or, no effect on time possible, e.g. targets locked
        break;
      }
    }
    RangeLast = Range;
    deltaTlast = deltaT;

    if (fabs(deltaT)>0.25) {
      // more than 15 seconds error
      Range -= dRdT*deltaT;
      Range = max(-1.0, min(Range,1.0));
    } else {
      break;
    }

  } while (steps++<25);

  Range = myrange;
  AdjustAATTargets(Range);
  RefreshCalculator();

  TargetDialogOpen = false;
  UnlockTaskData();
  UnlockFlightData();
}


static void OnTargetClicked(WndButton* pWnd){
  wf->SetVisible(false);
  dlgTarget();
  // find start value for range (it may have changed)
  Range = AdjustAATTargets(2.0);
  RefreshCalculator();
  wf->SetVisible(true);
}


static void OnMacCreadyData(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
  case DataField::daSpecial:
    if (CALCULATED_INFO.timeCircling>0) {
      CheckSetMACCREADY(CALCULATED_INFO.TotalHeightClimb /CALCULATED_INFO.timeCircling, nullptr);
      Sender->Set(Units::ToVerticalSpeed(MACCREADY));
      RefreshCalculator();
    }
    break;
  case DataField::daGet:
    Sender->Set(Units::ToVerticalSpeed(MACCREADY));
    break;
  case DataField::daPut:
  case DataField::daChange:
    CheckSetMACCREADY(Units::FromVerticalSpeed(Sender->GetAsFloat()), nullptr);
    RefreshCalculator();
    break;
  case DataField::daInc:
  case DataField::daDec:
    break;
  }
}


static void OnRangeData(DataField *Sender, DataField::DataAccessKind_t Mode){
  double rthis;
  switch(Mode){
  case DataField::daSpecial:
    DoOptimise();
    break;
  case DataField::daGet:
    //      Sender->Set(Range*100.0);
    break;
  case DataField::daPut:
  case DataField::daChange:
    rthis = Sender->GetAsFloat()/100.0;
    if (fabs(Range-rthis)>0.01) {
      Range = rthis;
      AdjustAATTargets(Range);
      RefreshCalculator();
    }
    break;
  case DataField::daInc:
  case DataField::daDec:
    break;
  }
}


static void OnCruiseEfficiencyData(DataField *Sender, DataField::DataAccessKind_t Mode) {
  double clast = CRUISE_EFFICIENCY;
  switch(Mode){
  case DataField::daGet:
    break;
  case DataField::daSpecial:
    GetCruiseEfficiency();
    CRUISE_EFFICIENCY = cruise_efficiency;
    if (fabs(cruise_efficiency-clast)>0.01) {
      RefreshCalculator();
    }
    break;
  case DataField::daPut:
  case DataField::daChange:
    cruise_efficiency = Sender->GetAsFloat()/100.0;
    CRUISE_EFFICIENCY = cruise_efficiency;
    if (fabs(cruise_efficiency-clast)>0.01) {
      RefreshCalculator();
    }
    break;
  case DataField::daInc:
  case DataField::daDec:
    break;
  }
}

static void OnOptimiseClicked(WndButton* pWnd) {
    DoOptimise();
}

static CallBackTableEntry_t CallBackTable[]={
  DataAccessCallbackEntry(OnMacCreadyData),
  DataAccessCallbackEntry(OnRangeData),
  ClickNotifyCallbackEntry(OnOKClicked),
  ClickNotifyCallbackEntry(OnCancelClicked),
  ClickNotifyCallbackEntry(OnOptimiseClicked),
  ClickNotifyCallbackEntry(OnTargetClicked),
  DataAccessCallbackEntry(OnCruiseEfficiencyData),
  EndCallBackEntry()
};


void dlgTaskCalculatorShowModal(void){

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_TASKCALCULATOR);
  if (!wf) return;

  double MACCREADY_enter = MACCREADY;
  double CRUISE_EFFICIENCY_enter = CRUISE_EFFICIENCY;

  emc = EffectiveMacCready(&GPS_INFO, &CALCULATED_INFO);

  cruise_efficiency = CRUISE_EFFICIENCY;

  // find start value for range
  Range = AdjustAATTargets(2.0);

  RefreshCalculator();

  if ((gTaskType != task_type_t::AAT) || !ValidTaskPoint(ActiveTaskPoint+1)) {
    (wf->FindByName<WndButton>(TEXT("Optimise")))->SetVisible(false);
  }
  if (!ValidTaskPoint(ActiveTaskPoint)) {
    (wf->FindByName<WndButton>(TEXT("Target")))->SetVisible(false);
  }

  if (wf->ShowModal() == mrCancel) {
    // todo: restore task settings.
    CheckSetMACCREADY(MACCREADY_enter, nullptr);
    CRUISE_EFFICIENCY = CRUISE_EFFICIENCY_enter;
  }
  delete wf;
  wf = NULL;
}
