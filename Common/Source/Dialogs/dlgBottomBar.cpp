/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKProfiles.h"
#include <aygshell.h>

#include "dlgTools.h"

#include "Terrain.h"
#include "LKMapWindow.h"

#include "Utils.h"


static bool changed = false;
static WndForm *wf=NULL;


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB1"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfBB1);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB2"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfBB2);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB3"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfBB3);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB4"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfBB4);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB5"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfBB5);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB6"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfBB6);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB7"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfBB7);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB8"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfBB8);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB9"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(ConfBB9);
    wp->RefreshDisplay();
  }

}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


void dlgBottomBarShowModal(void){

  WndProperty *wp;
  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgBottomBar.xml"));
  wf = dlgLoadFromXML(CallBackTable,                        
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_BOTTOMBAR"));

  if (!wf) return;

  setVariables();

  changed = false;

  wf->ShowModal();

  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB1"));
  if (wp) {
	if (ConfBB1 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB1 = (wp->GetDataField()->GetAsBoolean());
		SetToRegistry(szRegistryConfBB1, (DWORD)(ConfBB1));
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB2"));
  if (wp) {
	if (ConfBB2 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB2 = (wp->GetDataField()->GetAsBoolean());
		SetToRegistry(szRegistryConfBB2, (DWORD)(ConfBB2));
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB3"));
  if (wp) {
	if (ConfBB3 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB3 = (wp->GetDataField()->GetAsBoolean());
		SetToRegistry(szRegistryConfBB3, (DWORD)(ConfBB3));
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB4"));
  if (wp) {
	if (ConfBB4 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB4 = (wp->GetDataField()->GetAsBoolean());
		SetToRegistry(szRegistryConfBB4, (DWORD)(ConfBB4));
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB5"));
  if (wp) {
	if (ConfBB5 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB5 = (wp->GetDataField()->GetAsBoolean());
		SetToRegistry(szRegistryConfBB5, (DWORD)(ConfBB5));
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB6"));
  if (wp) {
	if (ConfBB6 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB6 = (wp->GetDataField()->GetAsBoolean());
		SetToRegistry(szRegistryConfBB6, (DWORD)(ConfBB6));
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB7"));
  if (wp) {
	if (ConfBB7 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB7 = (wp->GetDataField()->GetAsBoolean());
		SetToRegistry(szRegistryConfBB7, (DWORD)(ConfBB7));
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB8"));
  if (wp) {
	if (ConfBB8 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB8 = (wp->GetDataField()->GetAsBoolean());
		SetToRegistry(szRegistryConfBB8, (DWORD)(ConfBB8));
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpConfBB9"));
  if (wp) {
	if (ConfBB9 != (wp->GetDataField()->GetAsBoolean())) {
		ConfBB9 = (wp->GetDataField()->GetAsBoolean());
		SetToRegistry(szRegistryConfBB9, (DWORD)(ConfBB9));
		changed=true;
	}
  }

  if (changed) {
    #if OLDPROFILES
    StoreRegistry();
    #endif
    UpdateConfBB();
    MessageBoxX (hWndMainWindow,
                 gettext(TEXT("_@M1607_")), // bottom bar config saved
                 TEXT(""), MB_OK);
  }


  delete wf;
  wf = NULL;

}


