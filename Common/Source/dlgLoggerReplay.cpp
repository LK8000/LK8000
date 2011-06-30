/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"

#include "Statistics.h"

#include "externs.h"
#include "Units.h"
#include "Logger.h"
#include "dlgTools.h"

#include "utils/heapcheck.h"

extern HWND   hWndMainWindow;
static WndForm *wf=NULL;


static void OnStopClicked(WindowControl * Sender){
	(void)Sender;
  ReplayLogger::Stop();
}

static void OnStartClicked(WindowControl * Sender){
	(void)Sender;
  WndProperty* wp;
  wp = (WndProperty*)wf->FindByName(TEXT("prpIGCFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    ReplayLogger::SetFilename(dfe->GetPathFile());
  }
  ReplayLogger::Start();
}


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
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
  DeclareCallBackEntry(OnStopClicked),
  DeclareCallBackEntry(OnStartClicked),
  DeclareCallBackEntry(OnRateData), 
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


void dlgLoggerReplayShowModal(void){

  TCHAR tsuf[10];
  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgLoggerReplay.xml"));
  wf = dlgLoadFromXML(CallBackTable, 
                      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_LOGGERREPLAY"));

  WndProperty* wp;

  if (wf) {

    wp = (WndProperty*)wf->FindByName(TEXT("prpRate"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(ReplayLogger::TimeScale);
      wp->RefreshDisplay();
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpIGCFile"));
    if (wp) {
      DataFieldFileReader* dfe;
      dfe = (DataFieldFileReader*)wp->GetDataField();
      // dfe->ScanDirectoryTop(_T(""),TEXT("*.igc")); 
      _stprintf(tsuf,_T("*%S"),LKS_IGC);
      dfe->ScanDirectoryTop(_T(LKD_LOGS),tsuf);
      dfe->Lookup(ReplayLogger::GetFilename());
      wp->RefreshDisplay();
    }

    wf->ShowModal();
    delete wf;
  }
  wf = NULL;
}

