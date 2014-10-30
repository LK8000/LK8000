/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Dialogs.h"

extern void AddCustomKeyList( DataFieldEnum* dfe);

static WndForm *wf=NULL;


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu1"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddCustomKeyList(dfe); dfe->Set(CustomMenu1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu2"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddCustomKeyList(dfe); dfe->Set(CustomMenu2);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu3"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddCustomKeyList(dfe); dfe->Set(CustomMenu3);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu4"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddCustomKeyList(dfe); dfe->Set(CustomMenu4);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu5"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddCustomKeyList(dfe); dfe->Set(CustomMenu5);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu6"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddCustomKeyList(dfe); dfe->Set(CustomMenu6);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu7"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddCustomKeyList(dfe); dfe->Set(CustomMenu7);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu8"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddCustomKeyList(dfe); dfe->Set(CustomMenu8);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu9"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddCustomKeyList(dfe); dfe->Set(CustomMenu9);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu10"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddCustomKeyList(dfe); dfe->Set(CustomMenu10);
	wp->RefreshDisplay();
  }
}


static void OnResetClicked(WindowControl * Sender){

  WndProperty *wp;
  extern void Reset_CustomMenu(void);
  Reset_CustomMenu();

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu1"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(CustomMenu1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu2"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(CustomMenu2);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu3"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(CustomMenu3);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu4"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(CustomMenu4);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu5"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(CustomMenu5);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu6"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(CustomMenu6);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu7"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(CustomMenu7);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu8"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(CustomMenu8);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu9"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(CustomMenu9);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu10"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(CustomMenu10);
	wp->RefreshDisplay();
  }

}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnResetClicked),
  DeclareCallBackEntry(NULL)
};



void dlgCustomMenuShowModal(void){

  WndProperty *wp;
  TCHAR filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgCustomMenu.xml"));
  wf = dlgLoadFromXML(CallBackTable,                        
		      filename, 
		      TEXT("IDR_XML_CUSTOMMENU"));

  if (!wf) return;

  setVariables();

  wf->ShowModal();

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu1"));
  if (wp) CustomMenu1 = (wp->GetDataField()->GetAsInteger());
 
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu2"));
  if (wp) CustomMenu2 = (wp->GetDataField()->GetAsInteger());
 
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu3"));
  if (wp) CustomMenu3 = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu4"));
  if (wp) CustomMenu4 = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu5"));
  if (wp) CustomMenu5 = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu6"));
  if (wp) CustomMenu6 = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu7"));
  if (wp) CustomMenu7 = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu8"));
  if (wp) CustomMenu8 = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu9"));
  if (wp) CustomMenu9 = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomMenu10"));
  if (wp) CustomMenu10 = (wp->GetDataField()->GetAsInteger());

  delete wf;
  wf = NULL;

}

