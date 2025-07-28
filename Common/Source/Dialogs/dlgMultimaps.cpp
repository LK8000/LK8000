/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "resource.h"
#include "Multimap.h"

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

  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap1"));
  if (wp) {
	DataField* dfe = wp->GetDataField();

	dfe->addEnumText(MsgToken<959>()); //	_@M959_ "OFF"
	dfe->addEnumText(MsgToken<958>()); //	_@M958_ "ON"


	dfe->Set(Multimap1);
	wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap2"));
  if (wp) {
	DataField* dfe = wp->GetDataField();

	dfe->addEnumText(MsgToken<959>()); //	_@M959_ "OFF"
	dfe->addEnumText(MsgToken<958>()); //	_@M958_ "ON"


	dfe->Set(Multimap2);
	wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap3"));
  if (wp) {
	DataField* dfe = wp->GetDataField();

	dfe->addEnumText(MsgToken<959>()); //	_@M959_ "OFF"
	dfe->addEnumText(MsgToken<958>()); //	_@M958_ "ON"


	dfe->Set(Multimap3);
	wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap4"));
  if (wp) {
	DataField* dfe = wp->GetDataField();

	dfe->addEnumText(MsgToken<959>()); //	_@M959_ "OFF"
	dfe->addEnumText(MsgToken<958>()); //	_@M958_ "ON"

	dfe->Set(Multimap4);
	wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap5"));
  if (wp) {
	DataField* dfe = wp->GetDataField();

	dfe->addEnumText(MsgToken<959>()); //	_@M959_ "OFF"
	dfe->addEnumText(MsgToken<958>()); //	_@M958_ "ON"


	dfe->Set(Multimap5);
	wp->RefreshDisplay();
  }
}


//
// We cannot use setVariables from here, otherwise we would be adding more
// enum text to the selection list, each time we reset.
// We simply set the value, because the enum text list is already set.
//
static void OnResetClicked(WndButton* pWnd){

  WndProperty *wp;
  Reset_Multimap_Mode();

  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap1"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(Multimap1);
	wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap2"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(Multimap2);
	wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap3"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(Multimap3);
	wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap4"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(Multimap4);
	wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap5"));
  if (wp) {
	DataField* dfe = wp->GetDataField();
	dfe->Set(Multimap5);
	wp->RefreshDisplay();
  }
}

static CallBackTableEntry_t CallBackTable[]={
  CallbackEntry(OnCloseClicked),
  CallbackEntry(OnResetClicked),
  EndCallbackEntry()
};



void dlgMultimapsShowModal(void){
  WndProperty *wp;

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_MULTIMAPS);

  if (!wf) return;

  setVariables();

  wf->ShowModal();

  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap1"));
  if (wp) Multimap1 = (wp->GetDataField()->GetAsInteger());

  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap2"));
  if (wp) Multimap2 = (wp->GetDataField()->GetAsInteger());

  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap3"));
  if (wp) Multimap3 = (wp->GetDataField()->GetAsInteger());

  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap4"));
  if (wp) Multimap4 = (wp->GetDataField()->GetAsInteger());

  wp = wf->FindByName<WndProperty>(TEXT("prpMultimap5"));
  if (wp) Multimap5 = (wp->GetDataField()->GetAsInteger());

  UpdateMultimapOrient();

  delete wf;
  wf = NULL;

}
