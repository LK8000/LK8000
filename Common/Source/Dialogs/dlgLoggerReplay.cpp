/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgLoggerReplay.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "Logger.h"
#include "resource.h"

static WndForm *wf=NULL;

static void OnStopClicked(WndButton* pWnd) {
  ReplayLogger::Stop();
}

static void OnStartClicked(WndButton* pWnd) {
  WndProperty* wp;
  wp = wf->FindByName<WndProperty>(TEXT("prpIGCFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    ReplayLogger::SetFilename(dfe->GetPathFile());
  }
  ReplayLogger::Start();
}

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void OnRateData(DataField *Sender, DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
      Sender->Set(ReplayLogger::TimeScale);
    break;
    case DataField::daPut:
    case DataField::daChange:
      ReplayLogger::TimeScale = Sender->GetAsFloat();
    break;
  case DataField::daInc:
  case DataField::daDec:
  case DataField::daSpecial:
    break;
  }

}


static CallBackTableEntry_t CallBackTable[]={
  CallbackEntry(OnStopClicked),
  CallbackEntry(OnStartClicked),
  CallbackEntry(OnRateData),
  CallbackEntry(OnCloseClicked),
  EndCallbackEntry()
};


void dlgLoggerReplayShowModal(void){

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_LOGGERREPLAY);

  WndProperty* wp;

  if (wf) {

    wp = wf->FindByName<WndProperty>(TEXT("prpRate"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(ReplayLogger::TimeScale);
      wp->RefreshDisplay();
    }

    wp = wf->FindByName<WndProperty>(TEXT("prpIGCFile"));
    if (wp) {
      DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
      if(dfe) {
        dfe->ScanDirectoryTop(_T(LKD_LOGS), _T(LKS_IGC));
        dfe->Lookup(ReplayLogger::GetFilename());
      }
      wp->RefreshDisplay();
    }

    wf->ShowModal();
    delete wf;
  }
  wf = NULL;
}
