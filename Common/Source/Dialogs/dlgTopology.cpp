/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTopology.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "LKProcess.h"
#include "Terrain.h"
#include "LKProfiles.h"
#include "resource.h"

static bool changed = false;
static WndForm *wf=NULL;

// Correction factor for topology zoom levels, which are in Km.
// Using non-metric distances, these values should be changed accordingly.
// new=old*DISTANCEMODIFY*1000
#define RZC	(DISTANCEMODIFY*1000)


static void OnTopoActiveData(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
  case DataField::daGet:
    break;
  case DataField::daPut:
  case DataField::daChange:
    break;
  case DataField::daInc:
  case DataField::daDec:
  case DataField::daSpecial:
    break;
  }
}

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

  wp = (WndProperty*)wf->FindByName(TEXT("prpCat10"));
  if (wp) {
	if (HaveZoomTopology(10)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(10)*RZC);
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat20"));
  if (wp) {
	if (HaveZoomTopology(20)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(20)*RZC);
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat30"));
  if (wp) {
	if (HaveZoomTopology(30)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(30)*RZC);
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat40"));
  if (wp) {
	if (HaveZoomTopology(40)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(40)*RZC);
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat50"));
  if (wp) {
	if (HaveZoomTopology(50)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(50)*RZC);
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat60"));
  if (wp) {
	if (HaveZoomTopology(60)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(60)*RZC);
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat70"));
  if (wp) {
	if (HaveZoomTopology(70)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(70)*RZC);
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat80"));
  if (wp) {
	if (HaveZoomTopology(80)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(80)*RZC);
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat90"));
  if (wp) {
	if (HaveZoomTopology(90)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(90)*RZC);
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat100"));
  if (wp) {
	if (HaveZoomTopology(100)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(100)*RZC);
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);
	}
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat110"));
  if (wp) {
	if (HaveZoomTopology(110)) {
		wp->GetDataField()->SetAsFloat( ReadZoomTopology(110)*RZC);
	} else {
		wp->GetDataField()->SetAsFloat( 0 );
		wp->SetReadOnly(true);
	}
	wp->RefreshDisplay();
  }

}


static void OnResetTopologyClicked(WndButton* pWnd){

  ChangeZoomTopology(1,1,4);
  setVariables();

  //LKTOKEN _@M1223_ "Topology reset to default values"
  MessageBoxX (
		 MsgToken(1223),
		 TEXT(""), mbOk);
}




static CallBackTableEntry_t CallBackTable[]={
  DataAccessCallbackEntry(OnTopoActiveData),
  ClickNotifyCallbackEntry(OnResetTopologyClicked),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};


void dlgTopologyShowModal(void){

  if (LKTopo<1) {
	MessageBoxX (
	// LKTOKEN  _@M502_ = "Only LKMaps can be configured, sorry!"
		MsgToken(502),
		TEXT(""), mbOk);
	return;
  }

  WndProperty *wp;
  wf = dlgLoadFromXML(CallBackTable, IDR_XML_TOPOLOGY);

  if (!wf) return;

  setVariables();

  changed = false;

  wf->ShowModal();


  wp = (WndProperty*)wf->FindByName(TEXT("prpCat10"));
  if (wp) {
	if (HaveZoomTopology(5))
	if ( LKTopoZoomCat05 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat05 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(5,LKTopoZoomCat05,0);
		changed = true;
	}
  }
  if (wp) {
	if (HaveZoomTopology(10))
	if ( LKTopoZoomCat10 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat10 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(10,LKTopoZoomCat10,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat20"));
  if (wp) {
	if (HaveZoomTopology(20))
	if ( LKTopoZoomCat20 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat20 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(20,LKTopoZoomCat20,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat30"));
  if (wp) {
	if (HaveZoomTopology(30))
	if ( LKTopoZoomCat30 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat30 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(30,LKTopoZoomCat30,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat40"));
  if (wp) {
	if (HaveZoomTopology(40))
	if ( LKTopoZoomCat40 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat40 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(40,LKTopoZoomCat40,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat50"));
  if (wp) {
	if (HaveZoomTopology(50))
	if ( LKTopoZoomCat50 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat50 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(50,LKTopoZoomCat50,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat60"));
  if (wp) {
	if (HaveZoomTopology(60))
	if ( LKTopoZoomCat60 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat60 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(60,LKTopoZoomCat60,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat70"));
  if (wp) {
	if (HaveZoomTopology(70))
	if ( LKTopoZoomCat70 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat70 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(70,LKTopoZoomCat70,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat80"));
  if (wp) {
	if (HaveZoomTopology(80))
	if ( LKTopoZoomCat80 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat80 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(80,LKTopoZoomCat80,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat90"));
  if (wp) {
	if (HaveZoomTopology(90))
	if ( LKTopoZoomCat90 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat90 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(90,LKTopoZoomCat90,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat100"));
  if (wp) {
	if (HaveZoomTopology(100))
	if ( LKTopoZoomCat100 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat100 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(100,LKTopoZoomCat100,0);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCat110"));
  if (wp) {
	if (HaveZoomTopology(110))
	if ( LKTopoZoomCat110 != wp->GetDataField()->GetAsFloat()) {
		LKTopoZoomCat110 = wp->GetDataField()->GetAsFloat()/RZC;
		ChangeZoomTopology(110,LKTopoZoomCat110,0);
		changed = true;
	}
  }


  delete wf;
  wf = NULL;

}
