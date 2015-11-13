/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "resource.h"

extern void AddCustomKeyList( DataFieldEnum* dfe);

static WndForm *wf=NULL;

static void OnCloseClicked(WndButton* pWnd) {
    wf->SetModalResult(mrOK);
}


static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpTopLeft"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
        dfe->addEnumText(gettext(TEXT("_@M491_"))); // OFF
        dfe->addEnumText(gettext(TEXT("_@M894_"))); // ON

	dfe->Set(Overlay_TopLeft);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopMid"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
        dfe->addEnumText(gettext(TEXT("_@M491_"))); // OFF
        dfe->addEnumText(gettext(TEXT("_@M894_"))); // ON
	dfe->Set(Overlay_TopMid);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopRight"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
        dfe->addEnumText(gettext(TEXT("_@M491_"))); // OFF
        dfe->addEnumText(gettext(TEXT("_@M894_"))); // ON
	dfe->Set(Overlay_TopRight);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopDown"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
        dfe->addEnumText(gettext(TEXT("_@M491_"))); // OFF
        dfe->addEnumText(gettext(TEXT("_@M894_"))); // ON
	dfe->Set(Overlay_TopDown);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftTop"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
        dfe->addEnumText(gettext(TEXT("_@M491_"))); // OFF
        dfe->addEnumText(gettext(TEXT("_@M894_"))); // ON
        dfe->addEnumText(_T("AUX 4"));
	dfe->Set(Overlay_LeftTop);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftMid"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
        dfe->addEnumText(gettext(TEXT("_@M491_"))); // OFF
        dfe->addEnumText(gettext(TEXT("_@M894_"))); // ON
        dfe->addEnumText(_T("AUX 5"));
	dfe->Set(Overlay_LeftMid);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftBottom"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
        dfe->addEnumText(gettext(TEXT("_@M491_"))); // OFF
        dfe->addEnumText(gettext(TEXT("_@M894_"))); // ON
        dfe->addEnumText(_T("AUX 6"));
	dfe->Set(Overlay_LeftBottom);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftDown"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
        dfe->addEnumText(gettext(TEXT("_@M491_"))); // OFF
        dfe->addEnumText(gettext(TEXT("_@M894_"))); // ON
        dfe->addEnumText(_T("AUX 7"));
	dfe->Set(Overlay_LeftDown);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightTop"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
        dfe->addEnumText(gettext(TEXT("_@M491_"))); // OFF
        dfe->addEnumText(gettext(TEXT("_@M894_"))); // ON
        dfe->addEnumText(_T("AUX 1"));
	dfe->Set(Overlay_RightTop);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightMid"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
        dfe->addEnumText(gettext(TEXT("_@M491_"))); // OFF
        dfe->addEnumText(gettext(TEXT("_@M894_"))); // ON
        dfe->addEnumText(_T("AUX 2"));
	dfe->Set(Overlay_RightMid);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightBottom"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
        dfe->addEnumText(gettext(TEXT("_@M491_"))); // OFF
        dfe->addEnumText(gettext(TEXT("_@M894_"))); // ON
        dfe->addEnumText(_T("AUX 3"));
	dfe->Set(Overlay_RightBottom);
	wp->RefreshDisplay();
  }
}


static void OnResetClicked(WndButton* pWnd){

  WndProperty *wp;
  extern void Reset_CustomMenu(void);
  Reset_CustomMenu();

  wp = (WndProperty*)wf->FindByName(TEXT("prpTopLeft"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopMid"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopRight"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopDown"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftTop"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftMid"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftBottom"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftDown"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightTop"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightMid"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightBottom"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }

}

static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnCloseClicked),
  ClickNotifyCallbackEntry(OnResetClicked),
  EndCallBackEntry()
};



void dlgOverlaysShowModal(void){

  WndProperty *wp;
  wf = dlgLoadFromXML(CallBackTable,                        
		      TEXT("dlgOverlays.xml"), 
		      IDR_XML_OVERLAYS);

  if (!wf) return;

  setVariables();

  wf->ShowModal();

  wp = (WndProperty*)wf->FindByName(TEXT("prpTopLeft"));
  if (wp) Overlay_TopLeft = (wp->GetDataField()->GetAsInteger());
 
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopMid"));
  if (wp) Overlay_TopMid = (wp->GetDataField()->GetAsInteger());
 
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopRight"));
  if (wp) Overlay_TopRight = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpTopDown"));
  if (wp) Overlay_TopDown = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftTop"));
  if (wp) Overlay_LeftTop = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftMid"));
  if (wp) Overlay_LeftMid = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftBottom"));
  if (wp) Overlay_LeftBottom = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftDown"));
  if (wp) Overlay_LeftDown = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpRightTop"));
  if (wp) Overlay_RightTop = (wp->GetDataField()->GetAsInteger());
 
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightMid"));
  if (wp) Overlay_RightMid = (wp->GetDataField()->GetAsInteger());
 
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightBottom"));
  if (wp) Overlay_RightBottom = (wp->GetDataField()->GetAsInteger());

  delete wf;
  wf = NULL;

}

