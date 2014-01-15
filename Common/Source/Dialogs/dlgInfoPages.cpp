/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgInfoPages.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include <aygshell.h>

#include "Terrain.h"
#include "LKMapWindow.h"
#include "LKProfiles.h"
#include "Dialogs.h"


static bool changed = false;
static WndForm *wf=NULL;


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP11"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP11);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP12"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP12);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP13"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP13);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP14"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP14);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP15"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP15);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP16"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP16);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP17"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP17);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP21"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP21);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP22"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP22);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP23"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP23);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP24"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP24);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP31"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP31);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP32"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP32);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpIP33"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfIP33);
    wp->RefreshDisplay();
  }

}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


void dlgInfoPagesShowModal(void){

  WndProperty *wp;
  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgInfoPages.xml"));
  wf = dlgLoadFromXML(CallBackTable,                        
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_INFOPAGES"));

  if (!wf) return;

  setVariables();

  changed = false;

  wf->ShowModal();

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP11"));
  if (wp) {
	if (ConfIP11 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP11 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP12"));
  if (wp) {
	if (ConfIP12 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP12 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP13"));
  if (wp) {
	if (ConfIP13 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP13 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP14"));
  if (wp) {
	if (ConfIP14 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP14 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP15"));
  if (wp) {
	if (ConfIP15 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP15 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP16"));
  if (wp) {
	if (ConfIP16 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP16 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP21"));
  if (wp) {
	if (ConfIP21 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP21 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP22"));
  if (wp) {
	if (ConfIP22 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP22 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP23"));
  if (wp) {
	if (ConfIP23 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP23 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP24"));
  if (wp) {
	if (ConfIP24 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP24 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP31"));
  if (wp) {
	if (ConfIP31 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP31 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP32"));
  if (wp) {
	if (ConfIP32 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP32 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIP33"));
  if (wp) {
	if (ConfIP33 != (wp->GetDataField()->GetAsBoolean())) {
		ConfIP33 = (wp->GetDataField()->GetAsBoolean());
		changed=true;
	}
  }

  if (changed) {
    UpdateConfIP();
    MessageBoxX (hWndMainWindow,
                 gettext(TEXT("_@M1608_")), // infopages config saved
                 TEXT(""), MB_OK);
  }


  delete wf;
  wf = NULL;

}


