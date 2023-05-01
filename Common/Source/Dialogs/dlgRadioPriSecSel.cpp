/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWindSettings.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "resource.h"
#include "Util/TruncateString.hpp"
#include "Radio.h"


#define SHORT_DEVICE_NAME_LEN 12

unsigned frequency_khz = 0.0;
TCHAR  StationName[SHORT_DEVICE_NAME_LEN] = _T("");


static void OnCancelClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}



static void OnSelActiveButton(WndButton* pWnd){

  if(!ValidFrequency(frequency_khz)) {
    MessageBoxX(MsgToken(2490), MsgToken(2491), mbOk); //   "_@M002490_": "Invalid radio frequency/channel input!",
  }
  else {
    devPutFreqActive(frequency_khz, StationName);
    OnCancelClicked(pWnd);
  }
}


static void OnSelPassiveButton(WndButton* pWnd){

  if(!ValidFrequency(frequency_khz)) {
    MessageBoxX(MsgToken(2490), MsgToken(2491), mbOk); //    "_@M002490_": "Invalid radio frequency/channel input!",
  }
  else {
    devPutFreqStandby(frequency_khz, StationName);
    OnCancelClicked(pWnd);
  }
}


static CallBackTableEntry_t CallBackTable[]={

  ClickNotifyCallbackEntry(OnSelActiveButton),
  ClickNotifyCallbackEntry(OnSelPassiveButton),
  ClickNotifyCallbackEntry(OnCancelClicked),


  EndCallBackEntry()
};


void dlgRadioPriSecSelShowModal(const TCHAR*  pName, unsigned khz){
  TCHAR Name[SHORT_DEVICE_NAME_LEN+30];
  WndForm *wf=NULL;

  frequency_khz = khz;
  CopyTruncateString(StationName, SHORT_DEVICE_NAME_LEN, pName);		

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_RADIOPRISECSEL );
  if (!wf) return;


  WndOwnerDrawFrame* frm  = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmStationText"));
  LKASSERT(  frm  !=NULL)
  if (frm) {
    frm->SetCaptionStyle(DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    frm->SetCaption(StationName);	
  }

  WndOwnerDrawFrame* frmFreq  = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmStationFreq"));
  _stprintf(Name,_T("%7.3f"),  frequency_khz / 1000.);
  if (frmFreq) {
    frmFreq->SetCaptionStyle(DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    frmFreq->SetCaption(Name);
  }

  WndButton *wpnewActive = nullptr;
  WndButton *wpnewPassive = nullptr; 
  wpnewActive  = (WndButton*)wf->FindByName(TEXT("cmdSelActive"));
  LKASSERT(   wpnewActive   !=NULL)
  if(wpnewActive) 
    wpnewActive->SetCaption(GetActiveStationSymbol(Appearance.UTF8Pictorials));

  wpnewPassive  = (WndButton*)wf->FindByName(TEXT("cmdSelPassive"));
  LKASSERT(   wpnewPassive   !=NULL)
  if(wpnewPassive) 
    wpnewPassive->SetCaption(GetStandyStationSymbol(Appearance.UTF8Pictorials));

  // center dialog
  PixelRect main_rect(main_window->GetClientRect());
  RasterPoint main_center = main_rect.GetCenter();
  wf->SetTop(main_center.y - wf->GetHeight() / 2);
  wf->SetLeft(main_center.x - wf->GetWidth() / 2);

  wf->ShowModal();
  delete wf;
}
