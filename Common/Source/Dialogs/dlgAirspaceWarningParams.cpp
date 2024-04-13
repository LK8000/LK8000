/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceWarningParams.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "resource.h"


static WndForm *wf=NULL;


static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void SetReadOnlyItems()
{
  WndProperty* wp;
  bool aspw=false;
  bool aspmaplabels=false;

  wp = wf->FindByName<WndProperty>(TEXT("prpAirspaceWarnings"));
  if (wp) aspw=(wp->GetDataField()->GetAsBoolean());
  wp = wf->FindByName<WndProperty>(TEXT("prpWarningMapLabels"));
  if (wp) aspmaplabels=(wp->GetDataField()->GetAsBoolean());

  wp = wf->FindByName<WndProperty>(TEXT("prpWarningTime"));
  if (wp) wp->SetReadOnly(!aspw && !aspmaplabels);
  wp = wf->FindByName<WndProperty>(TEXT("prpAcknowledgementTime"));
  if (wp) wp->SetReadOnly(!aspw);
  wp = wf->FindByName<WndProperty>(TEXT("prpWarningMessageRepeatTime"));
  if (wp) wp->SetReadOnly(!aspw);
  wp = wf->FindByName<WndProperty>(TEXT("prpAckAllSame"));
  if (wp) wp->SetReadOnly(!aspw);
  wp = wf->FindByName<WndProperty>(TEXT("prpWarningDlgTimeout"));
  if (wp) wp->SetReadOnly(!aspw);
  wp = wf->FindByName<WndProperty>(TEXT("prpWarningVerticalMargin"));
  if (wp) wp->SetReadOnly(!aspw && !aspmaplabels);

}

static void OnWarningsClicked(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
       SetReadOnlyItems();
    break;
        default:
                StartupStore(_T("........... DBG-908%s"),NEWLINE);
                break;
  }
}

static void setVariables(void) {
  WndProperty *wp;

  wp = wf->FindByName<WndProperty>(TEXT("prpWarningTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(WarningTime);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAcknowledgementTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AcknowledgementTime/60);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAckAllSame"));
  if (wp) {
    wp->GetDataField()->Set(AirspaceAckAllSame);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWarningVerticalMargin"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(AirspaceWarningVerticalMargin / 10.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWarningDlgTimeout"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AirspaceWarningDlgTimeout);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWarningMapLabels"));
  if (wp) {
    wp->GetDataField()->Set(AirspaceWarningMapLabels);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAirspaceWarnings"));
  if (wp) {
    bool aw = AIRSPACEWARNINGS != 0;
    wp->GetDataField()->Set(aw);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWarningMessageRepeatTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AirspaceWarningRepeatTime/60);
    wp->RefreshDisplay();
  }


  SetReadOnlyItems();
}



static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnCloseClicked),
  DataAccessCallbackEntry(OnWarningsClicked),
  EndCallBackEntry()
};


void dlgAirspaceWarningParamsShowModal(void){

  WndProperty *wp;
  int ival;

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_AIRSPACEWARNINGPARAMS);

  if (!wf) return;

  setVariables();

  wf->ShowModal();

  wp = wf->FindByName<WndProperty>(TEXT("prpWarningTime"));
  if (wp) {
    if (WarningTime != wp->GetDataField()->GetAsInteger()) {
      WarningTime = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAcknowledgementTime"));
  if (wp) {
    if (AcknowledgementTime != wp->GetDataField()->GetAsInteger()*60) {
      AcknowledgementTime = wp->GetDataField()->GetAsInteger()*60;
    }
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpWarningVerticalMargin"));
  if (wp) {
    ival = iround(Units::FromAltitude(wp->GetDataField()->GetAsInteger())*10);
    if (AirspaceWarningVerticalMargin != ival) {
      AirspaceWarningVerticalMargin = ival;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWarningDlgTimeout"));
  if (wp) {
    if (AirspaceWarningDlgTimeout != wp->GetDataField()->GetAsInteger()) {
      AirspaceWarningDlgTimeout = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWarningMapLabels"));
  if (wp) {
    if (AirspaceWarningMapLabels != wp->GetDataField()->GetAsBoolean()) {
      AirspaceWarningMapLabels = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAckAllSame"));
  if (wp) {
    if (AirspaceAckAllSame != wp->GetDataField()->GetAsBoolean()) {
      AirspaceAckAllSame = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAirspaceWarnings"));
  if (wp) {
    if (AIRSPACEWARNINGS != wp->GetDataField()->GetAsBoolean()) {
      AIRSPACEWARNINGS = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWarningMessageRepeatTime"));
  if (wp) {
    if (AirspaceWarningRepeatTime != (wp->GetDataField()->GetAsInteger()*60)) {
      AirspaceWarningRepeatTime = wp->GetDataField()->GetAsInteger()*60;
    }
  }



  delete wf;
  wf = NULL;

}
