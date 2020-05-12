/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgLoggerReplay.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "resource.h"

static WndForm *wf=NULL;

static void OnStopClicked(WndButton* pWnd) {
    ReplaySpeed[SelectedDevice] = 0;
	if(wf)
	{
		WndProperty* wp = (WndProperty*)wf->FindByName(TEXT("prpRate"));
		if(wp)
		{
	    wp->GetDataField()->Set(ReplaySpeed[SelectedDevice]);
	    wp->RefreshDisplay();
		}
	}

}

static void OnStartClicked(WndButton* pWnd) {
  WndProperty* wp;
  wp = (WndProperty*)wf->FindByName(TEXT("prpIGCFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(Replay_FileName[SelectedDevice],dfe->GetPathFile());

  }
  if( ReplaySpeed[SelectedDevice] ==0)
  {
    ReplaySpeed[SelectedDevice] = 1;
    if(wf)
    {
      WndProperty* wp = (WndProperty*)wf->FindByName(TEXT("prpRate"));
      if(wp)
      {
        wp->GetDataField()->Set(ReplaySpeed[SelectedDevice]);
        wp->RefreshDisplay();
      }
    }
  }
  RestartCommPorts();
}

static void OnCloseClicked(WndButton* pWnd) {

  if(pWnd) {
    WndProperty* wp;
    wp = (WndProperty*)wf->FindByName(TEXT("prpIGCFile"));
    if (wp) {
      DataFieldFileReader* dfe;
      dfe = (DataFieldFileReader*)wp->GetDataField();

      _tcscpy(Replay_FileName[SelectedDevice],dfe->GetPathFile());

    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpRate"));
    if (wp) {
      DataFieldFileReader* dfe;
      dfe = (DataFieldFileReader*)wp->GetDataField();

      ReplaySpeed[SelectedDevice] = dfe->GetAsFloat();

    }
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void OnRateData(DataField *Sender, DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
      Sender->Set(ReplaySpeed[SelectedDevice]);
    break;
    case DataField::daPut:
    case DataField::daChange:
    	ReplaySpeed[SelectedDevice] = (int) Sender->GetAsFloat();
    break;
  case DataField::daInc:
  case DataField::daDec:
  case DataField::daSpecial:
    break;
  }

}


static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnStopClicked),
  ClickNotifyCallbackEntry(OnStartClicked),
  DataAccessCallbackEntry(OnRateData),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};


void dlgNMEAReplayShowModal(){

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_LOGGERREPLAY);

  WndProperty* wp;

  if (wf) {

    wp = (WndProperty*)wf->FindByName(TEXT("prpRate"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(ReplaySpeed[SelectedDevice]);
      wp->RefreshDisplay();
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpIGCFile"));
    if (wp) {
      DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
      if(dfe) {
        dfe->ScanDirectoryTop(_T(LKD_LOGS), _T("*" LKS_TXT));
        dfe->Lookup(Replay_FileName[SelectedDevice]);
      }
      wp->RefreshDisplay();
    }

    wf->ShowModal();

    delete wf;
  }
  wf = NULL;
}
