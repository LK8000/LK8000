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

extern void AddCustomKeyList( DataField* dfe);

static WndForm *wf=NULL;

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpTopLeft"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
        dfe->addEnumText(MsgToken(491)); // OFF
        dfe->addEnumText(MsgToken(894)); // ON
	dfe->Set(Overlay_TopLeft);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopMid"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
        dfe->addEnumText(MsgToken(491)); // OFF
        dfe->addEnumText(MsgToken(894)); // ON
	dfe->Set(Overlay_TopMid);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopRight"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
        dfe->addEnumText(MsgToken(491)); // OFF
        dfe->addEnumText(MsgToken(894)); // ON
	dfe->Set(Overlay_TopRight);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopDown"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
        dfe->addEnumText(MsgToken(491)); // OFF
        dfe->addEnumText(MsgToken(894)); // ON
	dfe->Set(Overlay_TopDown);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftTop"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(491)); // OFF
    dfe->addEnumText(MsgToken(2090)); // Default
    for (int i=0; i<NumDataOptions; i++) {
      dfe->addEnumText(LKGetText(Data_Options[i].Description));
    }
    dfe->Sort(2);
	dfe->Set(Overlay_LeftTop+2);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftMid"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(491)); // OFF
    dfe->addEnumText(MsgToken(2090)); // Default
    for (int i=0; i<NumDataOptions; i++) {
      dfe->addEnumText(LKGetText(Data_Options[i].Description));
    }
    dfe->Sort(2);
    dfe->Set(Overlay_LeftMid+2);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftBottom"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(491)); // OFF
    dfe->addEnumText(MsgToken(2090)); // Default
    for (int i=0; i<NumDataOptions; i++) {
      dfe->addEnumText(LKGetText(Data_Options[i].Description));
    }
    dfe->Sort(2);
    dfe->Set(Overlay_LeftBottom+2);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftDown"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(491)); // OFF
    dfe->addEnumText(MsgToken(2090)); // Default
    for (int i=0; i<NumDataOptions; i++) {
      dfe->addEnumText(LKGetText(Data_Options[i].Description));
    }
    dfe->Sort(2);
    dfe->Set(Overlay_LeftDown+2);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightTop"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(491)); // OFF
    dfe->addEnumText(MsgToken(2090)); // Default
    for (int i=0; i<NumDataOptions; i++) {
      dfe->addEnumText(LKGetText(Data_Options[i].Description));
    }
    dfe->Sort(2);
    dfe->Set(Overlay_RightTop+2);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightMid"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(491)); // OFF
    dfe->addEnumText(MsgToken(2090)); // Default
    for (int i=0; i<NumDataOptions; i++) {
      dfe->addEnumText(LKGetText(Data_Options[i].Description));
    }
    dfe->Sort(2);
    dfe->Set(Overlay_RightMid+2);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightBottom"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(491)); // OFF
    dfe->addEnumText(MsgToken(2090)); // Default
    for (int i=0; i<NumDataOptions; i++) {
      dfe->addEnumText(LKGetText(Data_Options[i].Description));
    }
    dfe->Sort(2);
    dfe->Set(Overlay_RightBottom+2);
	wp->RefreshDisplay();
  }
}


static void OnResetClicked(WndButton* pWnd){

  WndProperty *wp;
  extern void Reset_CustomMenu(void);
  Reset_CustomMenu();

  wp = (WndProperty*)wf->FindByName(TEXT("prpTopLeft"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopMid"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopRight"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopDown"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftTop"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftMid"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftBottom"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftDown"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightTop"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightMid"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpRightBottom"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
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
  wf = dlgLoadFromXML(CallBackTable, IDR_XML_OVERLAYS);

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
  if (wp) Overlay_LeftTop = (wp->GetDataField()->GetAsInteger())-2; // OFF=-2 ; Default=-1; >0 = LKValue

  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftMid"));
  if (wp) Overlay_LeftMid = (wp->GetDataField()->GetAsInteger())-2; // OFF=-2 ; Default=-1; >0 = LKValue

  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftBottom"));
  if (wp) Overlay_LeftBottom = (wp->GetDataField()->GetAsInteger())-2;; // OFF=-2 ; Default=-1; >0 = LKValue

  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftDown"));
  if (wp) Overlay_LeftDown = (wp->GetDataField()->GetAsInteger())-2; // OFF=-2 ; Default=-1; >0 = LKValue

  wp = (WndProperty*)wf->FindByName(TEXT("prpRightTop"));
  if (wp) Overlay_RightTop = (wp->GetDataField()->GetAsInteger())-2; // OFF=-2 ; Default=-1; >0 = LKValue;

  wp = (WndProperty*)wf->FindByName(TEXT("prpRightMid"));
  if (wp) Overlay_RightMid = (wp->GetDataField()->GetAsInteger())-2; // OFF=-2 ; Default=-1; >0 = LKValue;

  wp = (WndProperty*)wf->FindByName(TEXT("prpRightBottom"));
  if (wp) Overlay_RightBottom = (wp->GetDataField()->GetAsInteger())-2; // OFF=-2 ; Default=-1; >0 = LKValue;

  delete wf;
  wf = NULL;

}
