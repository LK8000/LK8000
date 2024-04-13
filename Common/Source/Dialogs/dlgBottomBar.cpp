/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgBottomBar.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "Terrain.h"
#include "LKMapWindow.h"
#include "LKInterface.h"
#include "resource.h"

static bool changed = false;

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void setVariables(WndForm* wf) {
  WndProperty *wp;

  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB0"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfBB0);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB1"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfBB1);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB2"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfBB2);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB3"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfBB3);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB4"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfBB4);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB5"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfBB5);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB6"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfBB6);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB7"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfBB7);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB8"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfBB8);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB9"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfBB9);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB0Auto"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken<2252>());  // MANUAL
    dfe->addEnumText(MsgToken<2253>()); //  AUTO THERMALLING
    dfe->addEnumText(MsgToken<2254>()); //  FULL AUTO
    dfe->Set(ConfBB0Auto);
    wp->RefreshDisplay();
  }

}


static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};


void dlgBottomBarShowModal(void){

  WndProperty *wp;

  WndForm* wf = dlgLoadFromXML(CallBackTable, IDR_XML_BOTTOMBAR);

  if (!wf) return;

  setVariables(wf);

  changed = false;

  wf->ShowModal();

  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB0"));
  if (wp) {
	if (ConfBB0 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB0 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB1"));
  if (wp) {
	if (ConfBB1 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB1 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB2"));
  if (wp) {
	if (ConfBB2 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB2 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB3"));
  if (wp) {
	if (ConfBB3 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB3 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB4"));
  if (wp) {
	if (ConfBB4 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB4 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB5"));
  if (wp) {
	if (ConfBB5 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB5 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB6"));
  if (wp) {
	if (ConfBB6 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB6 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB7"));
  if (wp) {
	if (ConfBB7 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB7 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB8"));
  if (wp) {
	if (ConfBB8 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB8 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB9"));
  if (wp) {
	if (ConfBB9 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB9 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpConfBB0Auto"));
  if (wp) {
    if (ConfBB0Auto != wp->GetDataField()->GetAsInteger() )
    {
      ConfBB0Auto = wp->GetDataField()->GetAsInteger();
    }
  }

  if (changed) {

    if (!(ConfBB1 || ConfBB2 || ConfBB3 || ConfBB4 || ConfBB5 ||
        ConfBB6 || ConfBB7 || ConfBB8 || ConfBB9)) {
      MessageBoxX(
                   MsgToken<16>(), // can't disable all non-TRM0
                   TEXT(""), mbOk);        // bottom bar stripes
      // Automatically enable NAV1 bottom bar
      ConfBB1 = true;
    }

    UpdateConfBB();
    MessageBoxX (MsgToken<1607>(), // bottom bar config saved
                 TEXT(""), mbOk);

    // If the user just disabled the currently-shown BB stripe, then
    // automatically advance to the next enabled stripe.
    if (!ConfBB[BottomMode]) BottomBarChange(true);
  }


  delete wf;
}
