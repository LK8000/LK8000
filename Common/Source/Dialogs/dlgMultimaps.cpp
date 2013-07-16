/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Dialogs.h"


static WndForm *wf=NULL;


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap1"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();

	dfe->addEnumText(MsgToken(239)); // Disabled
	dfe->addEnumText(MsgToken(259)); // Enabled


	dfe->Set(Multimap1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap2"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();

	dfe->addEnumText(MsgToken(239)); // Disabled
	dfe->addEnumText(MsgToken(259)); // Enabled


	dfe->Set(Multimap2);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap3"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();

	dfe->addEnumText(MsgToken(239)); // Disabled
	dfe->addEnumText(MsgToken(259)); // Enabled


	dfe->Set(Multimap3);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap4"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();

	dfe->addEnumText(MsgToken(239)); // Disabled
	dfe->addEnumText(MsgToken(259)); // Enabled


	dfe->Set(Multimap4);
	wp->RefreshDisplay();
  }
}


//
// We cannot use setVariables from here, otherwise we would be adding more
// enum text to the selection list, each time we reset.
// We simply set the value, because the enum text list is already set.
//
static void OnResetClicked(WindowControl * Sender){

  WndProperty *wp;
  extern void Reset_Multimap_Mode(void);
  Reset_Multimap_Mode();

  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap1"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(Multimap1);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap2"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(Multimap2);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap3"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(Multimap3);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap4"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->Set(Multimap4);
	wp->RefreshDisplay();
  }

}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnResetClicked),
  DeclareCallBackEntry(NULL)
};



void dlgMultimapsShowModal(void){
  WndProperty *wp;
  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgMultimaps.xml"));
  wf = dlgLoadFromXML(CallBackTable,                        
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_MULTIMAPS"));

  if (!wf) return;

  setVariables();

  wf->ShowModal();

  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap1"));
  if (wp) Multimap1 = (wp->GetDataField()->GetAsInteger());
 
  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap2"));
  if (wp) Multimap2 = (wp->GetDataField()->GetAsInteger());
 
  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap3"));
  if (wp) Multimap3 = (wp->GetDataField()->GetAsInteger());

  wp = (WndProperty*)wf->FindByName(TEXT("prpMultimap4"));
  if (wp) Multimap4 = (wp->GetDataField()->GetAsInteger());

  UpdateMultimapOrient();

  delete wf;
  wf = NULL;

}

