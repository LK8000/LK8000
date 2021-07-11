/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgCustomKeys.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Terrain.h"
#include "LKMapWindow.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "resource.h"

extern void AddCustomKeyList( DataField* dfe);

static bool changed = false;
static WndForm *wf=NULL;


static void OnCustomKeysActiveData(DataField *Sender, DataField::DataAccessKind_t Mode){
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


static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyTime"));
  if (wp) {
	wp->GetDataField()->SetAsFloat(CustomKeyTime);
	wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeLeftUpCorner"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	AddCustomKeyList(dfe);
	dfe->Set(CustomKeyModeLeftUpCorner);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeRightUpCorner"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	AddCustomKeyList(dfe);
	dfe->Set(CustomKeyModeRightUpCorner);
	dfe->Set(CustomKeyModeRightUpCorner);
	// if (ISPARAGLIDER) wp->SetReadOnly(true); 2.3q also PGs can use it
	wp->RefreshDisplay();
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeCenter"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	AddCustomKeyList(dfe);
	dfe->Set(CustomKeyModeCenter);
	wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeCenterScreen"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	AddCustomKeyList(dfe);
	dfe->Set(CustomKeyModeCenterScreen);
	wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeLeft"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	AddCustomKeyList(dfe);
	dfe->Set(CustomKeyModeLeft);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeRight"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	AddCustomKeyList(dfe);
	dfe->Set(CustomKeyModeRight);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeAircraftIcon"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	AddCustomKeyList(dfe);
	dfe->Set(CustomKeyModeAircraftIcon);
	wp->RefreshDisplay();
  }

}



static CallBackTableEntry_t CallBackTable[]={
  DataAccessCallbackEntry(OnCustomKeysActiveData),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};


void dlgCustomKeysShowModal(void){

  WndProperty *wp;
  wf = dlgLoadFromXML(CallBackTable, IDR_XML_CUSTOMKEYS);

  if (!wf) return;

  setVariables();

  changed = false;

  wf->ShowModal();

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeLeftUpCorner"));
  if (wp) {
	if (CustomKeyModeLeftUpCorner != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeLeftUpCorner = (wp->GetDataField()->GetAsInteger());
		changed=true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeRightUpCorner"));
  if (wp) {
	if (CustomKeyModeRightUpCorner != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeRightUpCorner = (wp->GetDataField()->GetAsInteger());
		changed=true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeCenter"));
  if (wp) {
	if (CustomKeyModeCenter != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeCenter = (wp->GetDataField()->GetAsInteger());
		changed=true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeCenterScreen"));
  if (wp) {
	if (CustomKeyModeCenterScreen != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeCenterScreen = (wp->GetDataField()->GetAsInteger());
		changed=true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyTime"));
  if (wp) {
	if (CustomKeyTime != wp->GetDataField()->GetAsInteger()) {
		CustomKeyTime = wp->GetDataField()->GetAsInteger();
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeLeft"));
  if (wp) {
	if (CustomKeyModeLeft != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeLeft = (wp->GetDataField()->GetAsInteger());
		changed=true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeRight"));
  if (wp) {
	if (CustomKeyModeRight != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeRight = (wp->GetDataField()->GetAsInteger());
		changed=true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeAircraftIcon"));
  if (wp) {
	if (CustomKeyModeAircraftIcon != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeAircraftIcon = (wp->GetDataField()->GetAsInteger());
		changed=true;
	}
  }


  delete wf;
  wf = NULL;

}
