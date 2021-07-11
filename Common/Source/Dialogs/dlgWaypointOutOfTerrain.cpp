/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWaypointOutOfTerrain.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "Waypointparser.h"
#include "dlgTools.h"
#include "resource.h"


static void OnYesClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(wpTerrainBoundsYes);
    }
  }
}

static void OnYesAllClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(wpTerrainBoundsYesAll);
    }
  }
}

static void OnNoClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(wpTerrainBoundsNo);
    }
  }
}

static void OnNoAllClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(wpTerrainBoundsNoAll);
    }
  }
}


static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnYesClicked),
  ClickNotifyCallbackEntry(OnYesAllClicked),
  ClickNotifyCallbackEntry(OnNoClicked),
  ClickNotifyCallbackEntry(OnNoAllClicked),
  EndCallBackEntry()
};

int dlgWaypointOutOfTerrain(TCHAR *Message){

  WndFrame* wfrm;
  int res = 0;

    WndForm* wf = dlgLoadFromXML(CallBackTable, IDR_XML_WAYPOINTTERRAIN);
    if (wf) {


      wfrm = (WndFrame*)wf->FindByName(TEXT("frmWaypointOutOfTerrainText"));

      wfrm->SetCaption(Message);
      wfrm->SetCaptionStyle(
          DT_EXPANDTABS
        | DT_CENTER
        | DT_NOCLIP
        | DT_WORDBREAK);


      res = wf->ShowModal();
      delete wf;

    }

    wf = NULL;


  return(res);

}
