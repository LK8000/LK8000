/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"
#include <aygshell.h>

#include "XCSoar.h"

#include "externs.h"
#include "dlgTools.h"
#include "Utils.h"

#ifdef LKAIRSPACE

static bool changed = false;
static WndForm *wf=NULL;


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(WarningTime);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAcknowledgementTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AcknowledgementTime);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningMessageRepeatTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AirspaceWarningRepeatTime/60);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningVerticalMargin"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(AirspaceWarningVerticalMargin*ALTITUDEMODIFY));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningDlgTimeout"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AirspaceWarningDlgTimeout);
    wp->RefreshDisplay();
  }

  #if 0 // REMOVE
  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningMapLabels"));
  if (wp) {
    wp->GetDataField()->Set(AirspaceWarningMapLabels);
    wp->RefreshDisplay();
  }
  #endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceWarnings"));
  if (wp) {
    bool aw = AIRSPACEWARNINGS != 0;
    wp->GetDataField()->Set(aw);
    wp->RefreshDisplay();
  }


}



static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


void dlgAirspaceWarningParamsShowModal(void){

  WndProperty *wp;
  char filename[MAX_PATH];
  int ival;
  
  LocalPathS(filename, TEXT("dlgAirspaceWarningParams.xml"));
  wf = dlgLoadFromXML(CallBackTable,                        
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_AIRSPACEWARNINGPARAMS"));

  if (!wf) return;

  setVariables();

  changed = false;

  wf->ShowModal();

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningTime"));
  if (wp) {
    if (WarningTime != wp->GetDataField()->GetAsInteger()) {
      WarningTime = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryWarningTime,(DWORD)WarningTime);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAcknowledgementTime"));
  if (wp) {
    if (AcknowledgementTime != wp->GetDataField()->GetAsInteger()) {
      AcknowledgementTime = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAcknowledgementTime,
		    (DWORD)AcknowledgementTime);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningMessageRepeatTime"));
  if (wp) {
    if (AirspaceWarningRepeatTime != (wp->GetDataField()->GetAsInteger()*60)) {
      AirspaceWarningRepeatTime = wp->GetDataField()->GetAsInteger()*60;
      SetToRegistry(szRegistryAirspaceWarningRepeatTime, (DWORD)AirspaceWarningRepeatTime);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningVerticalMargin"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY);
    if (AirspaceWarningVerticalMargin != ival) {
      AirspaceWarningVerticalMargin = ival;
      SetToRegistry(szRegistryAirspaceWarningVerticalMargin,AirspaceWarningVerticalMargin);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningDlgTimeout"));
  if (wp) {
    if (AirspaceWarningDlgTimeout != wp->GetDataField()->GetAsInteger()) {
      AirspaceWarningDlgTimeout = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAirspaceWarningDlgTimeout,
		    (DWORD)AirspaceWarningDlgTimeout);
      changed = true;
    }
  }

  #if 0 // REMOVE
  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningMapLabels"));
  if (wp) {
    if (AirspaceWarningMapLabels != wp->GetDataField()->GetAsInteger()) {
      AirspaceWarningMapLabels = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAirspaceWarningMapLabels,
            (DWORD)AirspaceWarningMapLabels);
      changed = true;
    }
  }
  #endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceWarnings"));
  if (wp) {
    if (AIRSPACEWARNINGS != wp->GetDataField()->GetAsInteger()) {
      AIRSPACEWARNINGS = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAirspaceWarning,(DWORD)AIRSPACEWARNINGS);
      changed = true;
    }
  }


  if (changed) {
    StoreRegistry();
    MessageBoxX (hWndMainWindow, 
		  // LKTOKEN  _@M1276_ "Airspace warning parameters saved."
		 gettext(TEXT("_@M1276_")), 
		 TEXT(""), MB_OK);
  }


  delete wf;
  wf = NULL;

}

#endif //LKAIRSPACE 
