/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgOracle.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "Sound/Sound.h"
#include "Topology.h"
#include "resource.h"
#include "Asset.hpp"

static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static CallBackTableEntry_t CallBackTable[]={
  CallbackEntry(OnCloseClicked),
  EndCallbackEntry()
};

namespace {
  WhereAmI _WhereAmI;
}
// Remember that this function is called at 10hz
static bool OnTimerNotify(WndForm* pWnd) {

  if(!_WhereAmI.IsDone()) {
    return false;
  }

  pWnd->SetTimerNotify(0, NULL);

  // Bell, and print results
  LKSound(TEXT("LK_GREEN.WAV"));
  pWnd->SetBackColor(RGB_WINBACKGROUND);

  WindowControl* pWndClose = pWnd->FindByName(_T("cmdClose"));
  if(pWndClose) {
    pWndClose->SetVisible(true);
    pWndClose->SetFocus();
  }
  WndFrame* pWndText = pWnd->FindByName<WndFrame>(_T("WndText"));
  if (pWndText) {
    pWndText->SetBackColor(RGB_WINBACKGROUND);
    pWndText->SetCaption(_WhereAmI.getText());
  }

  return true;
}


void dlgOracleShowModal(void){

  WndForm* wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_ORACLE_L : IDR_XML_ORACLE_P);
  if (!wf) {
    return;
  }
  MapWindow::SuspendDrawingThread();
  _WhereAmI.Start();

  WindowControl* pWndClose = wf->FindByName(_T("cmdClose"));
  if (pWndClose) {
    pWndClose->SetVisible(false);
  }
  WndFrame* pWndText = wf->FindByName<WndFrame>(_T("WndText"));
  if (pWndText) {
    pWndText->SetFont(MapWindowBoldFont);
    pWndText->SetCaptionStyle(DT_EXPANDTABS | DT_CENTER | DT_NOCLIP | DT_WORDBREAK);
    TCHAR szText[200] = {};
    _stprintf(szText, _T("\n\n%s\n\n%s"), MsgToken<1691>(), MsgToken<1692>());
    pWndText->SetCaption(szText);
  }

  // We must wait for data ready, so we shall do it  with timer notify.
  wf->SetTimerNotify(100, OnTimerNotify);
  wf->ShowModal();
  MapWindow::ResumeDrawingThread();

  delete wf;
}
