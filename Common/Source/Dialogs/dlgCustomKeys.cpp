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
#include "LKInterface.h"

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void setVariables(WndForm* pForm) {
  auto wp = pForm->FindByName<WndProperty>(TEXT("prpCustomKeyTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(CustomKeyTime);
    wp->RefreshDisplay();
  }
  AddCustomKeyList(pForm, _T("prpCustomKeyModeLeftUpCorner"), CustomKeyModeLeftUpCorner);
  AddCustomKeyList(pForm, _T("prpCustomKeyModeRightUpCorner"), CustomKeyModeRightUpCorner);
  AddCustomKeyList(pForm, _T("prpCustomKeyModeCenter"), CustomKeyModeCenter);
  AddCustomKeyList(pForm, _T("prpCustomKeyModeLeft"), CustomKeyModeLeft);
  AddCustomKeyList(pForm, _T("prpCustomKeyModeRight"), CustomKeyModeRight);
  AddCustomKeyList(pForm, _T("prpCustomKeyModeAircraftIcon"), CustomKeyModeAircraftIcon);
}

static void getVariables(WndForm* pForm) {
  GetCustomKey(pForm, _T("prpCustomKeyModeLeftUpCorner"), CustomKeyModeLeftUpCorner);
  GetCustomKey(pForm, _T("prpCustomKeyModeRightUpCorner"), CustomKeyModeRightUpCorner);
  GetCustomKey(pForm, _T("prpCustomKeyModeCenter"), CustomKeyModeCenter);
  GetCustomKey(pForm, _T("prpCustomKeyTime"), CustomKeyTime);
  GetCustomKey(pForm, _T("prpCustomKeyModeLeft"), CustomKeyModeLeft);
  GetCustomKey(pForm, _T("prpCustomKeyModeRight"), CustomKeyModeRight);
  GetCustomKey(pForm, _T("prpCustomKeyModeAircraftIcon"), CustomKeyModeAircraftIcon);
}


static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};


void dlgCustomKeysShowModal(void){

  std::unique_ptr<WndForm> pForm(dlgLoadFromXML(CallBackTable, IDR_XML_CUSTOMKEYS));

  if (!pForm) {
    return;
  }

  setVariables(pForm.get());

  pForm->ShowModal();

  getVariables(pForm.get());
}
