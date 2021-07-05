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
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpOvTitle"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
        dfe->addEnumText(MsgToken(491)); // OFF
        dfe->addEnumText(MsgToken(894)); // ON
	dfe->Set(Overlay_Title);
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
    if ( Overlay_LeftTop < 2 )
	  dfe->Set(Overlay_LeftTop);
    else
      dfe->Set(Overlay_LeftTop - 1000 +  2 ); // Custom value
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
    if ( Overlay_LeftMid < 2 )
      dfe->Set(Overlay_LeftMid);
    else
      dfe->Set(Overlay_LeftMid - 1000 +  2 ); // Custom value
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
    if ( Overlay_LeftBottom < 2 )
      dfe->Set(Overlay_LeftBottom);
    else
      dfe->Set(Overlay_LeftBottom - 1000 +  2 ); // Custom value
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
    if ( Overlay_LeftDown < 2 )
      dfe->Set(Overlay_LeftDown);
    else
      dfe->Set(Overlay_LeftDown - 1000 +  2 ); // Custom value
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
    if ( Overlay_RightTop < 2 )
      dfe->Set(Overlay_RightTop);
    else
      dfe->Set(Overlay_RightTop - 1000 +  2 ); // Custom value
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
    if ( Overlay_RightMid < 2 )
      dfe->Set(Overlay_RightMid);
    else
      dfe->Set(Overlay_RightMid - 1000 +  2 ); // Custom value
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
    if ( Overlay_RightBottom < 2 )
      dfe->Set(Overlay_RightBottom);
    else
      dfe->Set(Overlay_RightBottom - 1000 +  2 ); // Custom value
	wp->RefreshDisplay();
  }
}


static void OnResetClicked(WndButton* pWnd){

  WndProperty *wp;
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpOvTitle"));
  if (wp) Overlay_Title =  (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpTopMid"));
  if (wp) Overlay_TopMid = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpTopRight"));
  if (wp) Overlay_TopRight = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpTopDown"));
  if (wp) Overlay_TopDown = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftTop"));
  if (wp) {
    int dataField = (wp->GetDataField()->GetAsInteger());
    if ( dataField < 2 )
      Overlay_LeftTop = dataField;
    else
      Overlay_LeftTop = dataField+1000-2; // Custom overlay have a code LKValue+1000
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftMid"));
  if (wp) {
    int dataField = (wp->GetDataField()->GetAsInteger());
    if ( dataField < 2 )
      Overlay_LeftMid = dataField;
    else
      Overlay_LeftMid = dataField+1000-2; // Custom overlay have a code LKValue+1000
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftBottom"));
  if (wp) {
    int dataField = (wp->GetDataField()->GetAsInteger());
    if ( dataField < 2 )
      Overlay_LeftBottom = dataField;
    else
      Overlay_LeftBottom = dataField+1000-2; // Custom overlay have a code LKValue+1000
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLeftDown"));
  if (wp) {
    int dataField = (wp->GetDataField()->GetAsInteger());
    if ( dataField < 2 )
      Overlay_LeftDown = dataField;
    else
      Overlay_LeftDown = dataField+1000-2; // Custom overlay have a code LKValue+1000
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRightTop"));
  if (wp) {
    int dataField = (wp->GetDataField()->GetAsInteger());
    if ( dataField < 2 )
      Overlay_RightTop = dataField;
    else
      Overlay_RightTop = dataField+1000-2; // Custom overlay have a code LKValue+1000
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRightMid"));
  if (wp) {
    int dataField = (wp->GetDataField()->GetAsInteger());
    if ( dataField < 2 )
      Overlay_RightMid = dataField;
    else
      Overlay_RightMid = dataField+1000-2; // Custom overlay have a code LKValue+1000
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRightBottom"));
  if (wp) {
    int dataField = (wp->GetDataField()->GetAsInteger());
    if ( dataField < 2 )
      Overlay_RightBottom = dataField;
    else
      Overlay_RightBottom = dataField+1000-2; // Custom overlay have a code LKValue+1000
  }
  


  delete wf;
  wf = NULL;

}
