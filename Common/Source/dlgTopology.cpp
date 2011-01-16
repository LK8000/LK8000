/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTopology.cpp,v 1.1 2010/12/13 16:40:01 root Exp root $
*/

#include "StdAfx.h"
#include <aygshell.h>

#include "XCSoar.h"

#include "externs.h"
#include "dlgTools.h"

#include "compatibility.h"
#ifdef OLDPPC
#include "XCSoarProcess.h"
#else
#include "Process.h"
#endif

#include "Terrain.h"

#include "Utils.h"


static bool changed = false;
static WndForm *wf=NULL;


static void OnTopoActiveData(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
    break;
  }
}

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}






static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpCat10"));
  if (wp) {
	if (HaveZoomTopology(10)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(10));
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);    
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat20"));
  if (wp) {
	if (HaveZoomTopology(20)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(20));
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);    
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat30"));
  if (wp) {
	if (HaveZoomTopology(30)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(30));
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);    
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat40"));
  if (wp) {
	if (HaveZoomTopology(40)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(40));
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);    
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat50"));
  if (wp) {
	if (HaveZoomTopology(50)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(50));
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);    
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat60"));
  if (wp) {
	if (HaveZoomTopology(60)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(60));
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);    
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat70"));
  if (wp) {
	if (HaveZoomTopology(70)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(70));
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);    
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat80"));
  if (wp) {
	if (HaveZoomTopology(80)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(80));
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);    
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat90"));
  if (wp) {
	if (HaveZoomTopology(90)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(90));
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);    
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat100"));
  if (wp) {
	if (HaveZoomTopology(100)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(100));
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);    
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat110"));
  if (wp) {
	if (HaveZoomTopology(110)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(110));
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);    
	}
	wp->RefreshDisplay();
  }

}


static void OnResetTopologyClicked(WindowControl * Sender){
	(void)Sender;

  ChangeZoomTopology(1,1,4);
  setVariables();

  MessageBoxX (hWndMainWindow, 
		 TEXT("Topology reset to default values"), 
		 TEXT(""), MB_OK);
}




static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnTopoActiveData),
  DeclareCallBackEntry(OnResetTopologyClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


void dlgTopologyShowModal(void){

  if (LKTopo<1) { 
	MessageBoxX (hWndMainWindow, 
	// LKTOKEN  _@M502_ = "Only LKMaps can be configured, sorry!" 
		gettext(TEXT("_@M502_")), 
		TEXT(""), MB_OK);
  	return;
  }

  WndProperty *wp;
  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgTopology.xml"));
  wf = dlgLoadFromXML(CallBackTable,                        
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_TOPOLOGY"));

  if (!wf) return;

  setVariables();

  changed = false;

  wf->ShowModal();


  wp = (WndProperty*)wf->FindByName(TEXT("prpCat10"));
  if (wp) {
	if (HaveZoomTopology(5)) 
	if ( LKTopoZoomCat05 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat05 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat05, (DWORD)LKTopoZoomCat05);
		ChangeZoomTopology(5,LKTopoZoomCat05,0);
		changed = true;
	}
  }
  if (wp) {
	if (HaveZoomTopology(10)) 
	if ( LKTopoZoomCat10 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat10 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat10, (DWORD)LKTopoZoomCat10);
		ChangeZoomTopology(10,LKTopoZoomCat10,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat20"));
  if (wp) {
	if (HaveZoomTopology(20)) 
	if ( LKTopoZoomCat20 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat20 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat20, (DWORD)LKTopoZoomCat20);
		ChangeZoomTopology(20,LKTopoZoomCat20,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat30"));
  if (wp) {
	if (HaveZoomTopology(30)) 
	if ( LKTopoZoomCat30 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat30 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat30, (DWORD)LKTopoZoomCat30);
		ChangeZoomTopology(30,LKTopoZoomCat30,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat40"));
  if (wp) {
	if (HaveZoomTopology(40)) 
	if ( LKTopoZoomCat40 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat40 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat40, (DWORD)LKTopoZoomCat40);
		ChangeZoomTopology(40,LKTopoZoomCat40,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat50"));
  if (wp) {
	if (HaveZoomTopology(50)) 
	if ( LKTopoZoomCat50 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat50 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat50, (DWORD)LKTopoZoomCat50);
		ChangeZoomTopology(50,LKTopoZoomCat50,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat60"));
  if (wp) {
	if (HaveZoomTopology(60)) 
	if ( LKTopoZoomCat60 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat60 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat60, (DWORD)LKTopoZoomCat60);
		ChangeZoomTopology(60,LKTopoZoomCat60,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat70"));
  if (wp) {
	if (HaveZoomTopology(70)) 
	if ( LKTopoZoomCat70 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat70 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat70, (DWORD)LKTopoZoomCat70);
		ChangeZoomTopology(70,LKTopoZoomCat70,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat80"));
  if (wp) {
	if (HaveZoomTopology(80)) 
	if ( LKTopoZoomCat80 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat80 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat80, (DWORD)LKTopoZoomCat80);
		ChangeZoomTopology(80,LKTopoZoomCat80,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat90"));
  if (wp) {
	if (HaveZoomTopology(90)) 
	if ( LKTopoZoomCat90 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat90 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat90, (DWORD)LKTopoZoomCat90);
		ChangeZoomTopology(90,LKTopoZoomCat90,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat100"));
  if (wp) {
	if (HaveZoomTopology(100)) 
	if ( LKTopoZoomCat100 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat100 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat100, (DWORD)LKTopoZoomCat100);
		ChangeZoomTopology(100,LKTopoZoomCat100,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat110"));
  if (wp) {
	if (HaveZoomTopology(110)) 
	if ( LKTopoZoomCat110 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat110 = wp->GetDataField()->GetAsFloat();
		SetToRegistry(szRegistryLKTopoZoomCat110, (DWORD)LKTopoZoomCat110);
		ChangeZoomTopology(110,LKTopoZoomCat110,0);
		changed = true;
	}
  }


  if (changed) {
    StoreRegistry();
    MessageBoxX (hWndMainWindow, 
	// LKTOKEN  _@M732_ = "Topology configuration saved." 
		 gettext(TEXT("_@M732_")), 
		 TEXT(""), MB_OK);
  }


  delete wf;
  wf = NULL;

}


