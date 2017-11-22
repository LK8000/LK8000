/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTaskRules.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "dlgTools.h"
#include "LKProcess.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "resource.h"


static bool changed = false;
static WndForm *wf=NULL;


static void OnRulesActiveData(DataField *Sender, DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      // TODO enhancement: hide/show fields as appropriate
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
  DataAccessCallbackEntry(OnRulesActiveData),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};



static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFAIFinishHeight"));
  if (wp) {
    wp->GetDataField()->Set(EnableFAIFinishHeight);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartHeightRef"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(LKGetText(TEXT("AGL")));
    dfe->addEnumText(LKGetText(TEXT("MSL")));
    dfe->Set(StartHeightRef);
    wp->RefreshDisplay();
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpFinishMinHeight"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToUserAltitude(FinishMinHeight / 1000.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxHeight"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToUserAltitude(StartMaxHeight / 1000.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxHeightMargin"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToUserAltitude(StartMaxHeightMargin / 1000.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxSpeed"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToUserHorizontalSpeed(StartMaxSpeed / 1000.0)));
    wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxSpeedMargin"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToUserHorizontalSpeed(StartMaxSpeedMargin / 1000)));
    wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFAI28_45Threshold"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(LKGetText(TEXT("500km (DMSt/OLC)")));
    dfe->addEnumText(LKGetText(TEXT("750km (FAI)")));
    if(FAI28_45Threshold > FAI_BIG_THRESHOLD)
      dfe->Set(1);
    else
      dfe->Set(0);
    wp->RefreshDisplay();
  }
}


bool dlgTaskRules(void){

  WndProperty *wp;

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_TASKRULES);

  if (!wf) return false;

  setVariables();

  changed = false;

  wf->ShowModal();

  // TODO enhancement: implement a cancel button that skips all this below after exit.

  int ival;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFAIFinishHeight"));
  if (wp) {
    if (EnableFAIFinishHeight != wp->GetDataField()->GetAsBoolean()) {
      EnableFAIFinishHeight = wp->GetDataField()->GetAsBoolean();
      changed = true;
    }
  }


  int Tmp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFAI28_45Threshold"));
  if (wp) {
     Tmp= wp->GetDataField()->GetAsInteger();

     if(Tmp==0)
     {
       if(FAI28_45Threshold >FAI_BIG_THRESHOLD)
       {
	  FAI28_45Threshold =FAI_BIG_THRESHOLD;
          changed = true;
       }
     }
     else
     {
       if(FAI28_45Threshold <750000)
       {
         FAI28_45Threshold =750000;
         changed = true;
       }
     }
   }



  wp = (WndProperty*)wf->FindByName(TEXT("prpStartHeightRef"));
  if (wp) {
    if (StartHeightRef != wp->GetDataField()->GetAsInteger()) {
      StartHeightRef = wp->GetDataField()->GetAsInteger();
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFinishMinHeight"));
  if (wp) {
    ival = iround(Units::ToSysAltitude(wp->GetDataField()->GetAsInteger())*1000);
    if ((int)FinishMinHeight != ival) {
      FinishMinHeight = ival;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxHeight"));
  if (wp) {
    ival = iround(Units::ToSysAltitude(wp->GetDataField()->GetAsInteger())*1000);
    if ((int)StartMaxHeight != ival) {
      StartMaxHeight = ival;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxHeightMargin"));
  if (wp) {
    ival = iround(Units::ToSysAltitude(wp->GetDataField()->GetAsInteger())*1000); // 100315
    if ((int)StartMaxHeightMargin != ival) {
      StartMaxHeightMargin = ival;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxSpeed"));
  if (wp) {
    ival = iround(Units::ToSysHorizontalSpeed(wp->GetDataField()->GetAsInteger())*1000);
    if ((int)StartMaxSpeed != ival) {
      StartMaxSpeed = ival;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxSpeedMargin"));
  if (wp) {
    ival = iround(Units::ToSysHorizontalSpeed(wp->GetDataField()->GetAsInteger())*1000);
    if ((int)StartMaxSpeedMargin != ival) {
      StartMaxSpeedMargin = ival;
      changed = true;
    }
  }


  delete wf;
  wf = NULL;
  return changed;
}
