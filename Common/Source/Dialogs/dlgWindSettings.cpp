/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWindSettings.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "resource.h"

static WndForm *wf=NULL;

static void OnCancelClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void UpdateWind(bool set) {
  WndProperty *wp;
  double ws = 0.0, wb = 0.0;
  wp = (WndProperty*)wf->FindByName(TEXT("prpSpeed"));
  if (wp) {
    ws = wp->GetDataField()->GetAsFloat()/SPEEDMODIFY;
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpDirection"));
  if (wp) {
    wb = wp->GetDataField()->GetAsFloat();
  }
  if ((ws != CALCULATED_INFO.WindSpeed)
      ||(wb != CALCULATED_INFO.WindBearing)) {
    if (set) {
      SetWindEstimate(ws, wb);
    }
    CALCULATED_INFO.WindSpeed = ws;
    CALCULATED_INFO.WindBearing = wb;
  }
}


static void OnCloseClicked(WndButton* pWnd){
  UpdateWind(true);
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void OnWindSpeedData(DataField *Sender, DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
      Sender->SetMax(SPEEDMODIFY*(200.0/TOKPH));
      Sender->Set(SPEEDMODIFY*CALCULATED_INFO.WindSpeed);
    break;
    case DataField::daPut:
      UpdateWind(false);
    break;
    case DataField::daChange:
    default:
      // calc alt...
    break;
  }
}

static void OnWindDirectionData(DataField *Sender, DataField::DataAccessKind_t Mode){

  double lastWind;

  switch(Mode){
	case DataField::daGet:
		lastWind = CALCULATED_INFO.WindBearing;
		if (lastWind>=359)
			lastWind=0;
		Sender->Set(lastWind);
		break;
	case DataField::daPut:
		UpdateWind(false);
		break;
	case DataField::daChange:
		lastWind = Sender->GetAsFloat();
		if (lastWind > 359)
			Sender->Set(0.0);
		break;
	default:
		break;
	}
}

static CallBackTableEntry_t CallBackTable[]={
  DataAccessCallbackEntry(OnWindSpeedData),
  DataAccessCallbackEntry(OnWindDirectionData),
  ClickNotifyCallbackEntry(OnCancelClicked),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};

void dlgWindSettingsShowModal(void){

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_WINDSETTINGS);

  if (wf) {
    WndProperty* wp;

    wp = (WndProperty*)wf->FindByName(TEXT("prpSpeed"));
    if (wp) {
      wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
      wp->RefreshDisplay();
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpAutoWind"));
    if (wp) {
      DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M418_ = "Manual"
      dfe->addEnumText(MsgToken(418));
	// LKTOKEN  _@M175_ = "Circling"
      dfe->addEnumText(MsgToken(175));
      dfe->addEnumText(LKGetText(TEXT("ZigZag")));
	// LKTOKEN  _@M149_ = "Both"
      dfe->addEnumText(MsgToken(149));
      dfe->addEnumText(MsgToken(1793)); // External

      wp->GetDataField()->Set(AutoWindMode);
      wp->RefreshDisplay();

      wp = (WndProperty*)wf->FindByName(TEXT("prpTrailDrift"));
      if (wp) {
        wp->GetDataField()->Set(MapWindow::EnableTrailDrift);
        wp->RefreshDisplay();
      }
    }

    wf->ShowModal();

    wp = (WndProperty*)wf->FindByName(TEXT("prpAutoWind"));
    if (wp) {
      if (AutoWindMode != wp->GetDataField()->GetAsInteger()) {
	AutoWindMode = wp->GetDataField()->GetAsInteger();
      }
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpTrailDrift"));
    if (wp) {
      if (MapWindow::EnableTrailDrift != wp->GetDataField()->GetAsBoolean()) {
        MapWindow::EnableTrailDrift = wp->GetDataField()->GetAsBoolean();
      }
    }

    delete wf;
  }
  wf = NULL;

}
