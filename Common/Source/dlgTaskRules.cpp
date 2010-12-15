/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTaskRules.cpp,v 8.2 2010/12/13 14:30:56 root Exp root $
*/

#include "StdAfx.h"
#include <aygshell.h>

#include "XCSoar.h"

#include "externs.h"
#include "dlgTools.h"

#include "compatibility.h"
#ifdef OLDPPC
#include "XCSoarProcess.h"
#else
#include "Process.h"
#endif

#include "Utils.h"


static bool changed = false;
static WndForm *wf=NULL;


static void OnRulesActiveData(DataField *Sender, 
			      DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      // TODO enhancement: hide/show fields as appropriate
    break;
  }
}


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnRulesActiveData),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};



static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpOLCEnabled"));
  if (wp) {
    wp->GetDataField()->Set(EnableOLC);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFAIFinishHeight"));
  if (wp) {
    wp->GetDataField()->Set(EnableFAIFinishHeight);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartHeightRef"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("AGL")));
    dfe->addEnumText(gettext(TEXT("MSL")));
    dfe->Set(StartHeightRef);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOLCRules"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M633_ = "Sprint" 
    dfe->addEnumText(gettext(TEXT("_@M633_")));
	// LKTOKEN  _@M742_ = "Triangle" 
    dfe->addEnumText(gettext(TEXT("_@M742_")));
	// LKTOKEN  _@M176_ = "Classic" 
    dfe->addEnumText(gettext(TEXT("_@M176_")));
    dfe->Set(OLCRules);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFinishMinHeight"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(FinishMinHeight*ALTITUDEMODIFY/1000));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxHeight"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(StartMaxHeight*ALTITUDEMODIFY));// 100315
    wp->GetDataField()->SetAsFloat(iround(StartMaxHeight*ALTITUDEMODIFY/1000));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxSpeed"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(StartMaxSpeed*SPEEDMODIFY/1000));
    wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
    wp->RefreshDisplay();
  }
}


bool dlgTaskRules(void){

  WndProperty *wp;

  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgTaskRules.xml"));
  wf = dlgLoadFromXML(CallBackTable,                        
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_TASKRULES"));

  if (!wf) return false;
  
  setVariables();

  changed = false;

  wf->ShowModal();

  // TODO enhancement: implement a cancel button that skips all this below after exit.

  int ival;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFAIFinishHeight"));
  if (wp) {
    if (EnableFAIFinishHeight != (wp->GetDataField()->GetAsInteger()>0)) {
      EnableFAIFinishHeight = (wp->GetDataField()->GetAsInteger()>0);
      SetToRegistry(szRegistryFAIFinishHeight, EnableFAIFinishHeight);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartHeightRef"));
  if (wp) {
    if (StartHeightRef != wp->GetDataField()->GetAsInteger()) {
      StartHeightRef = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryStartHeightRef, StartHeightRef);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOLCRules"));
  if (wp) {
    if (OLCRules != wp->GetDataField()->GetAsInteger()) {
      OLCRules = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryOLCRules, OLCRules);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOLCEnabled"));
  if (wp) {
    if (EnableOLC != (wp->GetDataField()->GetAsInteger()==1)) {
      EnableOLC = (wp->GetDataField()->GetAsInteger()==1);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFinishMinHeight"));
  if (wp) {
    ival = iround((wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY)*1000);
    if ((int)FinishMinHeight != ival) {
      FinishMinHeight = ival;
      SetToRegistry(szRegistryFinishMinHeight,FinishMinHeight);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxHeight"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY)*1000);
    if ((int)StartMaxHeight != ival) {
      StartMaxHeight = ival;
      SetToRegistry(szRegistryStartMaxHeight,StartMaxHeight);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxSpeed"));
  if (wp) {
    ival = iround((wp->GetDataField()->GetAsInteger()/SPEEDMODIFY)*1000);
    if ((int)StartMaxSpeed != ival) {
      StartMaxSpeed = ival;
      SetToRegistry(szRegistryStartMaxSpeed,StartMaxSpeed);
      changed = true;
    }
  }


  if (changed) {
    StoreRegistry();

    MessageBoxX (hWndMainWindow, 
	// LKTOKEN  _@M168_ = "Changes to configuration saved." 
		 gettext(TEXT("_@M168_")), 
		 TEXT(""), MB_OK);
  }

  delete wf;
  wf = NULL;
  return changed;
}

