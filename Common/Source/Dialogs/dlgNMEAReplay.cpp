/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

 
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "resource.h"

static WndForm *wf=NULL;

static void OnStopClicked(WndButton* pWnd) {
    
  auto& Port = PortConfig[SelectedDevice];
  Port.ReplaySpeed = 0;

	if(wf) {
	  WndProperty* wp = (WndProperty*)wf->FindByName(TEXT("prpRate"));
	  if(wp) {
	    wp->GetDataField()->Set(Port.ReplaySpeed);
	    wp->RefreshDisplay();
	  }
	}
}

static void OnStartClicked(WndButton* pWnd) {

  auto& Port = PortConfig[SelectedDevice];

  WndProperty* wp = (WndProperty*)wf->FindByName(TEXT("prpIGCFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(Port.Replay_FileName,dfe->GetPathFile());

  }
  if( Port.ReplaySpeed ==0)
  {
    Port.ReplaySpeed = 1;
    if(wf)
    {
      WndProperty* wp = (WndProperty*)wf->FindByName(TEXT("prpRate"));
      if(wp)
      {
        wp->GetDataField()->Set(Port.ReplaySpeed);
        wp->RefreshDisplay();
      }
    }
  }
  RestartCommPorts();
}

static void OnCloseClicked(WndButton* pWnd) {
  auto& Port = PortConfig[SelectedDevice];

  if(pWnd) {
    WndProperty* wp;
    wp = (WndProperty*)wf->FindByName(TEXT("prpIGCFile"));
    if (wp) {
      DataFieldFileReader* dfe;
      dfe = (DataFieldFileReader*)wp->GetDataField();

      _tcscpy(Port.Replay_FileName,dfe->GetPathFile());

    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpRate"));
    if (wp) {
      DataFieldFileReader* dfe;
      dfe = (DataFieldFileReader*)wp->GetDataField();

      Port.ReplaySpeed = dfe->GetAsFloat();

    }
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void OnRateData(DataField *Sender, DataField::DataAccessKind_t Mode){
  auto& Port = PortConfig[SelectedDevice];

  switch(Mode){
    case DataField::daGet:
      Sender->Set(Port.ReplaySpeed);
    break;
    case DataField::daPut:
    case DataField::daChange:
    	Port.ReplaySpeed = (int) Sender->GetAsFloat();
    break;
  case DataField::daInc:
  case DataField::daDec:
  case DataField::daSpecial:
    break;
  }

}

static void OnSyncNMEAModified(DataField *Sender, DataField::DataAccessKind_t Mode) {
  if (Sender) {

    if(Sender->getCount() == 0) {
      Sender->addEnumList({
          MsgToken<2482>(),	// _@M2334_ "In flight only (default)"
          _T("$GPRMC"),
          _T("$GPGGA")
        });
    }

    auto& Port = PortConfig[SelectedDevice];

    switch (Mode) {
    case DataField::daGet:
      Sender->Set(Port.ReplaySync);
      break;
    case DataField::daPut:
    case DataField::daChange:
      Port.ReplaySync = Sender->GetAsInteger();
      break;
    case DataField::daInc:
    case DataField::daDec:
    case DataField::daSpecial:
    default:
      break;
    }
  }
}



static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnStopClicked),
  ClickNotifyCallbackEntry(OnStartClicked),
  DataAccessCallbackEntry(OnRateData),
  ClickNotifyCallbackEntry(OnCloseClicked),
  DataAccessCallbackEntry(OnSyncNMEAModified),
  EndCallBackEntry()
};


void dlgNMEAReplayShowModal(){
  auto& Port = PortConfig[SelectedDevice];

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_NMEAREPLAY);

  WndProperty* wp;

  if (wf) {

    wp = (WndProperty*)wf->FindByName(TEXT("prpRate"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(Port.ReplaySpeed);
      wp->RefreshDisplay();
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpIGCFile"));
    if (wp) {
      DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
      if(dfe) {
        dfe->ScanDirectoryTop(_T(LKD_LOGS),TEXT(LKS_TXT));
        dfe->Lookup(Port.Replay_FileName);
      }
      wp->RefreshDisplay();
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpRaw"));
    if (wp) {      
      wp->GetDataField()->Set(  Port.RawByteData );
      wp->RefreshDisplay();
    }      
      
    wf->ShowModal();

    
    wp = (WndProperty*)wf->FindByName(TEXT("prpRaw"));
    if (wp) {
      if (Port.RawByteData != wp->GetDataField()->GetAsBoolean()) {
        Port.RawByteData = (int) wp->GetDataField()->GetAsBoolean();
      }
    }
    
    delete wf;
  }
  wf = NULL;
}
