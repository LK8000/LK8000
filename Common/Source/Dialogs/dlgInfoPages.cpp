/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgInfoPages.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Terrain.h"
#include "LKMapWindow.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "resource.h"

static bool changed = false;
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

  wp = wf->FindByName<WndProperty>(TEXT("prpIP11"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP11);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP12"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP12);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP13"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP13);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP14"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP14);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP15"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP15);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP16"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP16);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP17"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP17);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP21"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP21);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP22"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP22);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP23"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP23);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP24"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP24);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP31"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP31);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP32"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP32);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIP33"));
  if (wp) {
    DataField* dfb = wp->GetDataField();
    dfb->Set(ConfIP33);
    wp->RefreshDisplay();
  }

}


static CallBackTableEntry_t CallBackTable[]={
  CallbackEntry(OnCloseClicked),
  EndCallbackEntry()
};


void dlgInfoPagesShowModal(void){

  WndProperty *wp;
  wf = dlgLoadFromXML(CallBackTable, IDR_XML_INFOPAGES);

  if (!wf) return;

  setVariables();

  changed = false;

  wf->ShowModal();

  wp = wf->FindByName<WndProperty>(TEXT("prpIP11"));
  if (wp) {
	if (ConfIP11 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP11 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP12"));
  if (wp) {
	if (ConfIP12 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP12 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP13"));
  if (wp) {
	if (ConfIP13 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP13 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP14"));
  if (wp) {
	if (ConfIP14 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP14 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP15"));
  if (wp) {
	if (ConfIP15 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP15 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP16"));
  if (wp) {
	if (ConfIP16 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP16 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP17"));
  if (wp) {
	if (ConfIP17 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP17 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP21"));
  if (wp) {
	if (ConfIP21 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP21 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP22"));
  if (wp) {
	if (ConfIP22 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP22 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP23"));
  if (wp) {
	if (ConfIP23 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP23 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP24"));
  if (wp) {
	if (ConfIP24 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP24 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP31"));
  if (wp) {
	if (ConfIP31 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP31 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP32"));
  if (wp) {
	if (ConfIP32 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP32 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIP33"));
  if (wp) {
	if (ConfIP33 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP33 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  if (changed) {
    UpdateConfIP();
    MessageBoxX (MsgToken<1608>(), // infopages config saved
                 TEXT(""), mbOk);
  }


  delete wf;
  wf = NULL;

}
