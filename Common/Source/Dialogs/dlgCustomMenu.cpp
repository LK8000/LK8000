/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
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
  if (!pForm) {
    return;
  }

  AddCustomKeyList(pForm, _T("prpCustomMenu1"), CustomMenu1);
  AddCustomKeyList(pForm, _T("prpCustomMenu2"), CustomMenu2);
  AddCustomKeyList(pForm, _T("prpCustomMenu3"), CustomMenu3);
  AddCustomKeyList(pForm, _T("prpCustomMenu4"), CustomMenu4);
  AddCustomKeyList(pForm, _T("prpCustomMenu5"), CustomMenu5);
  AddCustomKeyList(pForm, _T("prpCustomMenu6"), CustomMenu6);
  AddCustomKeyList(pForm, _T("prpCustomMenu7"), CustomMenu7);
  AddCustomKeyList(pForm, _T("prpCustomMenu8"), CustomMenu8);
  AddCustomKeyList(pForm, _T("prpCustomMenu9"), CustomMenu9);
  AddCustomKeyList(pForm, _T("prpCustomMenu10"), CustomMenu10);
}

static void getVariables(WndForm* pForm) {
  if (!pForm) {
    return;
  }

  GetCustomKey(pForm, _T("prpCustomMenu1"), CustomMenu1);
  GetCustomKey(pForm, _T("prpCustomMenu2"), CustomMenu2);
  GetCustomKey(pForm, _T("prpCustomMenu3"), CustomMenu3);
  GetCustomKey(pForm, _T("prpCustomMenu4"), CustomMenu4);
  GetCustomKey(pForm, _T("prpCustomMenu5"), CustomMenu5);
  GetCustomKey(pForm, _T("prpCustomMenu6"), CustomMenu6);
  GetCustomKey(pForm, _T("prpCustomMenu7"), CustomMenu7);
  GetCustomKey(pForm, _T("prpCustomMenu8"), CustomMenu8);
  GetCustomKey(pForm, _T("prpCustomMenu9"), CustomMenu9);
  GetCustomKey(pForm, _T("prpCustomMenu10"), CustomMenu10);
}

static void OnResetClicked(WndButton* pWnd) {
  Reset_CustomMenu();
  setVariables(pWnd->GetParentWndForm());
}

static CallBackTableEntry_t CallBackTable[]={
  CallbackEntry(OnCloseClicked),
  CallbackEntry(OnResetClicked),
  EndCallbackEntry()
};

void dlgCustomMenuShowModal() {

  std::unique_ptr<WndForm> pForm(dlgLoadFromXML(CallBackTable, IDR_XML_CUSTOMMENU));

  if (!pForm) return;

  setVariables(pForm.get());

  pForm->ShowModal();

  getVariables(pForm.get());
}
