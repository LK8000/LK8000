/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWindSettings.cpp,v 8.2 2010/12/13 17:31:19 root Exp root $
*/

#include "StdAfx.h"

#include "Statistics.h"

#include "externs.h"
#include "Units.h"
#include "dlgTools.h"

extern HWND   hWndMainWindow;
static WndForm *wf=NULL;

static void OnCancelClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
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


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
        UpdateWind(true);
  #ifndef NOWINDREGISTRY
  SaveWindToRegistry();
  #endif
  wf->SetModalResult(mrOK);
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
		#if 101117
		if (lastWind>=360.0)
			lastWind=0;
		#else
		if (lastWind < 0.5)
			lastWind = 360.0;
		#endif
		Sender->Set(lastWind);
		break;
	case DataField::daPut:
		UpdateWind(false);
		break;
	case DataField::daChange:
		lastWind = Sender->GetAsFloat();
		if (lastWind < 0.5)
			Sender->Set(360.0);
		if (lastWind > 360.5)
			Sender->Set(1.0);
		break;
	default:
		break;
	}
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnWindSpeedData),
  DeclareCallBackEntry(OnWindDirectionData),
  DeclareCallBackEntry(OnCancelClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};

void dlgWindSettingsShowModal(void){

  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgWindSettings.xml"));
  wf = dlgLoadFromXML(CallBackTable, 
		      
                      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_WINDSETTINGS"));

  if (wf) {
    WndProperty* wp;

    wp = (WndProperty*)wf->FindByName(TEXT("prpSpeed"));
    if (wp) {
      wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
      wp->RefreshDisplay();
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpAutoWind"));
    if (wp) {
      DataFieldEnum* dfe;
      dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M418_ = "Manual" 
      dfe->addEnumText(gettext(TEXT("_@M418_")));
	// LKTOKEN  _@M175_ = "Circling" 
      dfe->addEnumText(gettext(TEXT("_@M175_")));
      dfe->addEnumText(gettext(TEXT("ZigZag")));
	// LKTOKEN  _@M149_ = "Both" 
      dfe->addEnumText(gettext(TEXT("_@M149_")));
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
	SetToRegistry(szRegistryAutoWind, AutoWindMode);
      }
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpTrailDrift"));
    if (wp) {
      if (MapWindow::EnableTrailDrift != wp->GetDataField()->GetAsBoolean()) {
        MapWindow::EnableTrailDrift = wp->GetDataField()->GetAsBoolean();
        // SetToRegistry(szRegistryTrailDrift, MapWindow::EnableTrailDrift);
      }
    }
    
    delete wf;
  }
  wf = NULL;

}

