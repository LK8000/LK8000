/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTimeGates.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKProcess.h"
#include "LKProfiles.h"
#include "Calculations2.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "resource.h"
#include "Calc/Task/TimeGates.h"

static bool changed = false;
static WndForm *wf=NULL;


static void OnTGActiveData(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
  case DataField::daGet:
    break;
  case DataField::daPut:
  case DataField::daChange:
    break;
  case DataField::daInc:
  case DataField::daDec:
  case DataField::daSpecial:
    break;
  }
}

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static CallBackTableEntry_t CallBackTable[]={
  DataAccessCallbackEntry(OnTGActiveData),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};



static void setVariables(void) {
  WndProperty *wp;

  wp = wf->FindByName<WndProperty>(TEXT("prpPGOptimizeRoute"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->Set(TskOptimizeRoute);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGNumberOfGates"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGNumberOfGates);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGOpenTimeH"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGOpenTimeH);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGOpenTimeM"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGOpenTimeM);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGCloseTimeH"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGCloseTimeH);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGCloseTimeM"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGCloseTimeM);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGGateIntervalTime"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGGateIntervalTime);
    wp->RefreshDisplay();
  }
}


void dlgTimeGatesShowModal(void){

  WndProperty *wp;
  wf = dlgLoadFromXML(CallBackTable, IDR_XML_TIMEGATES);

  if (!wf) return;

  setVariables();

  changed = false;

  wf->ShowModal();
  // TODO enhancement: implement a cancel button that skips all this below after exit.


  wp = wf->FindByName<WndProperty>(TEXT("prpPGOptimizeRoute"));
  if (wp) {
    if (TskOptimizeRoute != (wp->GetDataField()->GetAsBoolean())) {
      TskOptimizeRoute = (wp->GetDataField()->GetAsBoolean());
      changed = true;

      if (gTaskType == task_type_t::GP) {
        ClearOptimizedTargetPos();
      }
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGNumberOfGates"));
  if (wp) {
    if ( PGNumberOfGates != wp->GetDataField()->GetAsInteger()) {
      PGNumberOfGates = wp->GetDataField()->GetAsInteger();
      changed = true;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGOpenTimeH"));
  if (wp) {
    if ( PGOpenTimeH != wp->GetDataField()->GetAsInteger()) {
      PGOpenTimeH = wp->GetDataField()->GetAsInteger();
      changed = true;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGOpenTimeM"));
  if (wp) {
    if ( PGOpenTimeM != wp->GetDataField()->GetAsInteger()) {
      PGOpenTimeM = wp->GetDataField()->GetAsInteger();
      changed = true;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGCloseTimeH"));
  if (wp) {
    if ( PGCloseTimeH != wp->GetDataField()->GetAsInteger()) {
      PGCloseTimeH = wp->GetDataField()->GetAsInteger();
      changed = true;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGCloseTimeM"));
  if (wp) {
    if ( PGCloseTimeM != wp->GetDataField()->GetAsInteger()) {
      PGCloseTimeM = wp->GetDataField()->GetAsInteger();
      changed = true;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGGateIntervalTime"));
  if (wp) {
    if ( PGGateIntervalTime != wp->GetDataField()->GetAsInteger()) {
      PGGateIntervalTime = wp->GetDataField()->GetAsInteger();
      changed = true;
    }
  }

  if (changed) {
      InitActiveGate();
  }


  delete wf;
  wf = NULL;

}
