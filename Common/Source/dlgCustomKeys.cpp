/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgCustomKeys.cpp,v 1.1 2010/12/13 13:32:44 root Exp root $
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
#include "LKMapWindow.h"

#include "Utils.h"

void AddConfList( DataFieldEnum* dfe);

static bool changed = false;
static WndForm *wf=NULL;


static void OnCustomKeysActiveData(DataField *Sender, DataField::DataAccessKind_t Mode){
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

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static void setVariables(void) {
  WndProperty *wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyTime"));
  if (wp) {
	wp->GetDataField()->SetAsFloat(CustomKeyTime);
	wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeLeftUpCorner"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddConfList(dfe);
	//#include "LKinclude_confcuskey.cpp" REMOVE
	dfe->Set(CustomKeyModeLeftUpCorner);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeRightUpCorner"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddConfList(dfe);
	//#include "LKinclude_confcuskey.cpp" REMOVE
	dfe->Set(CustomKeyModeRightUpCorner);
	dfe->Set(CustomKeyModeRightUpCorner);
	if (ISPARAGLIDER) wp->SetReadOnly(true);
	wp->RefreshDisplay();
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeCenter"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddConfList(dfe);
	//#include "LKinclude_confcuskey.cpp" REMOVE
	dfe->Set(CustomKeyModeCenter);
	wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeLeft"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddConfList(dfe);
	//#include "LKinclude_confcuskey.cpp" REMOVE
	dfe->Set(CustomKeyModeLeft);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeRight"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	AddConfList(dfe);
	//#include "LKinclude_confcuskey.cpp" // REMOVE
	dfe->Set(CustomKeyModeRight);
	wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeAircraftIcon"));
  if (wp) {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	//#include "LKinclude_confcuskey.cpp" // REMOVE
	AddConfList(dfe);
	dfe->addEnumText(TEXT("Toggle IBOX"));
	dfe->Set(CustomKeyModeAircraftIcon);
	wp->RefreshDisplay();
  }

}



static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCustomKeysActiveData),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


void dlgCustomKeysShowModal(void){

  WndProperty *wp;
  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgCustomKeys.xml"));
  wf = dlgLoadFromXML(CallBackTable,                        
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_CUSTOMKEYS"));

  if (!wf) return;

  setVariables();

  changed = false;

  wf->ShowModal();

  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeLeftUpCorner"));
  if (wp) {
	if (CustomKeyModeLeftUpCorner != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeLeftUpCorner = (wp->GetDataField()->GetAsInteger());
		SetToRegistry(szRegistryCustomKeyModeLeftUpCorner, (DWORD)(CustomKeyModeLeftUpCorner));
		changed=true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeRightUpCorner"));
  if (wp) {
	if (CustomKeyModeRightUpCorner != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeRightUpCorner = (wp->GetDataField()->GetAsInteger());
		SetToRegistry(szRegistryCustomKeyModeRightUpCorner, (DWORD)(CustomKeyModeRightUpCorner));
		changed=true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeCenter"));
  if (wp) {
	if (CustomKeyModeCenter != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeCenter = (wp->GetDataField()->GetAsInteger());
		SetToRegistry(szRegistryCustomKeyModeCenter, (DWORD)(CustomKeyModeCenter));
		changed=true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyTime"));
  if (wp) {
	if (CustomKeyTime != wp->GetDataField()->GetAsInteger()) {
		CustomKeyTime = wp->GetDataField()->GetAsInteger();
		SetToRegistry(szRegistryCustomKeyTime,CustomKeyTime);
		changed = true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeLeft"));
  if (wp) {
	if (CustomKeyModeLeft != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeLeft = (wp->GetDataField()->GetAsInteger());
		SetToRegistry(szRegistryCustomKeyModeLeft, (DWORD)(CustomKeyModeLeft));
		changed=true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeRight"));
  if (wp) {
	if (CustomKeyModeRight != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeRight = (wp->GetDataField()->GetAsInteger());
		SetToRegistry(szRegistryCustomKeyModeRight, (DWORD)(CustomKeyModeRight));
		changed=true;
	}
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCustomKeyModeAircraftIcon"));
  if (wp) {
	if (CustomKeyModeAircraftIcon != (wp->GetDataField()->GetAsInteger())) {
		CustomKeyModeAircraftIcon = (wp->GetDataField()->GetAsInteger());
		SetToRegistry(szRegistryCustomKeyModeAircraftIcon, (DWORD)(CustomKeyModeAircraftIcon));
		changed=true;
	}
  }

  if (changed) {
    StoreRegistry();
    MessageBoxX (hWndMainWindow, 
	// LKTOKEN  _@M207_ = "Custom keys config saved" 
		 gettext(TEXT("_@M207_")), 
		 TEXT(""), MB_OK);
  }


  delete wf;
  wf = NULL;

}


void AddConfList( DataFieldEnum* dfe) {

	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M435_ = "Menu" 
    dfe->addEnumText(gettext(TEXT("_@M435_")));
	// LKTOKEN  _@M517_ = "Page Back" 
    dfe->addEnumText(gettext(TEXT("_@M517_")));
	// LKTOKEN  _@M725_ = "Toggle Map<>current page" 
    dfe->addEnumText(gettext(TEXT("_@M725_")));
	// LKTOKEN  _@M723_ = "Toggle Map<>Landables" 
    dfe->addEnumText(gettext(TEXT("_@M723_")));
	// LKTOKEN  _@M385_ = "Landables" 
    dfe->addEnumText(gettext(TEXT("_@M385_")));
	// LKTOKEN  _@M722_ = "Toggle Map<>Commons" 
    dfe->addEnumText(gettext(TEXT("_@M722_")));
	// LKTOKEN  _@M192_ = "Commons" 
    dfe->addEnumText(gettext(TEXT("_@M192_")));
	// LKTOKEN  _@M724_ = "Toggle Map<>Traffic" 
    dfe->addEnumText(gettext(TEXT("_@M724_")));
	// LKTOKEN  _@M738_ = "Traffic" 
    dfe->addEnumText(gettext(TEXT("_@M738_")));
	// LKTOKEN  _@M363_ = "Invert colors" 
    dfe->addEnumText(gettext(TEXT("_@M363_")));
    dfe->addEnumText(TEXT("TrueWind"));
	// LKTOKEN  _@M726_ = "Toggle overlays" 
    dfe->addEnumText(gettext(TEXT("_@M726_")));
    dfe->addEnumText(TEXT("AutoZoom On/Off"));
    dfe->addEnumText(TEXT("ActiveMap On/Off"));
	// LKTOKEN  _@M426_ = "Mark Location" 
    dfe->addEnumText(gettext(TEXT("_@M426_")));
    dfe->addEnumText(TEXT("PG/Delta Time Gates"));
    dfe->addEnumText(TEXT("Thermal Booster"));
	// LKTOKEN  _@M329_ = "Goto Home" 
    dfe->addEnumText(gettext(TEXT("_@M329_")));
	// LKTOKEN  _@M519_ = "Panorama trigger" 
    dfe->addEnumText(gettext(TEXT("_@M519_")));
	// LKTOKEN  _@M448_ = "Multitarget rotate" 
    dfe->addEnumText(gettext(TEXT("_@M448_")));
	// LKTOKEN  _@M447_ = "Multitarget menu" 
    dfe->addEnumText(gettext(TEXT("_@M447_")));
	// LKTOKEN  _@M700_ = "Team code" 
    dfe->addEnumText(gettext(TEXT("_@M700_")));
	// LKTOKEN  _@M767_ = "Use HBar on/off" 
    dfe->addEnumText(gettext(TEXT("_@M767_")));
	// LKTOKEN  _@M130_ = "Basic Setup menu" 
    dfe->addEnumText(gettext(TEXT("_@M130_")));
    dfe->addEnumText(TEXT("SIMulation menu"));

}



