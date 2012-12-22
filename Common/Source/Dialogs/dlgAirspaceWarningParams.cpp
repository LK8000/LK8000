/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceWarningParams.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKProfiles.h"
#include <aygshell.h>
#include "Dialogs.h"


static bool changed = false;
static WndForm *wf=NULL;


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}

static void SetReadOnlyItems()
{
  WndProperty* wp;
  bool aspw=false;
  bool aspmaplabels=false;
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceWarnings"));
  if (wp) aspw=(wp->GetDataField()->GetAsBoolean());
  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningMapLabels"));
  if (wp) aspmaplabels=(wp->GetDataField()->GetAsBoolean());
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningTime"));
  if (wp) wp->SetReadOnly(!aspw && !aspmaplabels);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAcknowledgementTime"));
  if (wp) wp->SetReadOnly(!aspw);
  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningDlgTimeout"));
  if (wp) wp->SetReadOnly(!aspw);
  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningVerticalMargin"));
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(WarningTime);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAcknowledgementTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AcknowledgementTime/60);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningVerticalMargin"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(AirspaceWarningVerticalMargin*ALTITUDEMODIFY/10));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningDlgTimeout"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AirspaceWarningDlgTimeout);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningMapLabels"));
  if (wp) {
    wp->GetDataField()->Set(AirspaceWarningMapLabels);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceWarnings"));
  if (wp) {
    bool aw = AIRSPACEWARNINGS != 0;
    wp->GetDataField()->Set(aw);
    wp->RefreshDisplay();
  }

  SetReadOnlyItems();
}



static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnWarningsClicked),
  DeclareCallBackEntry(NULL)
};


void dlgAirspaceWarningParamsShowModal(void){

  WndProperty *wp;
  char filename[MAX_PATH];
  int ival;
  
  LocalPathS(filename, TEXT("dlgAirspaceWarningParams.xml"));
  wf = dlgLoadFromXML(CallBackTable,                        
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_AIRSPACEWARNINGPARAMS"));

  if (!wf) return;

  setVariables();

  changed = false;

  wf->ShowModal();

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningTime"));
  if (wp) {
    if (WarningTime != wp->GetDataField()->GetAsInteger()) {
      WarningTime = wp->GetDataField()->GetAsInteger();
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAcknowledgementTime"));
  if (wp) {
    if (AcknowledgementTime != wp->GetDataField()->GetAsInteger()*60) {
      AcknowledgementTime = wp->GetDataField()->GetAsInteger()*60;
      changed = true;
    }
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningVerticalMargin"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY*10);
    if (AirspaceWarningVerticalMargin != ival) {
      AirspaceWarningVerticalMargin = ival;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningDlgTimeout"));
  if (wp) {
    if (AirspaceWarningDlgTimeout != wp->GetDataField()->GetAsInteger()) {
      AirspaceWarningDlgTimeout = wp->GetDataField()->GetAsInteger();
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningMapLabels"));
  if (wp) {
    if (AirspaceWarningMapLabels != wp->GetDataField()->GetAsInteger()) {
      AirspaceWarningMapLabels = wp->GetDataField()->GetAsInteger();
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceWarnings"));
  if (wp) {
    if (AIRSPACEWARNINGS != wp->GetDataField()->GetAsInteger()) {
      AIRSPACEWARNINGS = wp->GetDataField()->GetAsInteger();
      changed = true;
    }
  }


  delete wf;
  wf = NULL;

}

