/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTimeGates.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include <aygshell.h>

#include "dlgTools.h"

#include "Process.h"
#include "LKProfiles.h"


static bool changed = false;
static WndForm *wf=NULL;


static void OnTGActiveData(DataField *Sender, DataField::DataAccessKind_t Mode){
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

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnTGActiveData),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};



static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpPGNumberOfGates"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGNumberOfGates);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOpenTimeH"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGOpenTimeH);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOpenTimeM"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGOpenTimeM);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGGateIntervalTime"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGGateIntervalTime);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGStartOut"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M343_ = "IN (Exit)" 
    dfe->addEnumText(gettext(TEXT("_@M343_")));
	// LKTOKEN  _@M498_ = "OUT (Enter)" 
    dfe->addEnumText(gettext(TEXT("_@M498_")));
    dfe->Set(PGStartOut);
    wp->RefreshDisplay();
  }
}


void dlgTimeGatesShowModal(void){

  WndProperty *wp;
  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgTimeGates.xml"));
  wf = dlgLoadFromXML(CallBackTable,                        
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_TIMEGATES"));

  if (!wf) return;
  
  setVariables();

  changed = false;

  wf->ShowModal();
  // TODO enhancement: implement a cancel button that skips all this below after exit.


  wp = (WndProperty*)wf->FindByName(TEXT("prpPGNumberOfGates"));
  if (wp) {
    if ( PGNumberOfGates != wp->GetDataField()->GetAsInteger()) {
      PGNumberOfGates = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGNumberOfGates, PGNumberOfGates);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOpenTimeH"));
  if (wp) {
    if ( PGOpenTimeH != wp->GetDataField()->GetAsInteger()) {
      PGOpenTimeH = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGOpenTimeH, PGOpenTimeH);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOpenTimeM"));
  if (wp) {
    if ( PGOpenTimeM != wp->GetDataField()->GetAsInteger()) {
      PGOpenTimeM = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGOpenTimeM, PGOpenTimeM);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGGateIntervalTime"));
  if (wp) {
    if ( PGGateIntervalTime != wp->GetDataField()->GetAsInteger()) {
      PGGateIntervalTime = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGGateIntervalTime, PGGateIntervalTime);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGStartOut"));
  if (wp) {
    if ( PGStartOut != wp->GetDataField()->GetAsInteger()) {
      PGStartOut = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGStartOut, PGStartOut);
      changed = true;
    }
  }

  if (changed) {

     PGOpenTime=((PGOpenTimeH*60)+PGOpenTimeM)*60;
     PGCloseTime=PGOpenTime+(PGGateIntervalTime*PGNumberOfGates*60);
     if (PGCloseTime>86399) PGCloseTime=86399; // 23:59:59
     ActiveGate=-1;

    #if OLDPROFILES
    StoreRegistry();
    MessageBoxX (hWndMainWindow, 
	// LKTOKEN  _@M168_ = "Changes to configuration saved." 
		 gettext(TEXT("_@M168_")), 
		 TEXT(""), MB_OK);
    #endif
  }


  delete wf;
  wf = NULL;

}

