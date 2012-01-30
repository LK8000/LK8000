/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgConfiguration.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include <aygshell.h>

#include "MapWindow.h"
#include "Terrain.h"
#include "Process.h"

#include "WindowControls.h"
#include "McReady.h"
#include "dlgTools.h"

#include "Modeltype.h"

#include "McReady.h"
#include "Utils.h"
#include "InfoBoxLayout.h"
#include "Waypointparser.h"
#include "LKMapWindow.h"
#include "LKProfiles.h"
#include "Calculations2.h"
#include "DoInits.h"

extern void UpdateAircraftConfig(void);
// extern void UpdatePilotConfig(void); REMOVE

static HFONT TempMapWindowFont;
static HFONT TempMapLabelFont;
static HFONT TempUseCustomFontsFont;

extern void LKAircraftSave(const TCHAR *szFile);
extern void LKPilotSave(const TCHAR *szFile);

extern void InitializeOneFont (HFONT * theFont, 
                               const TCHAR FontRegKey[] , 
                               LOGFONT autoLogFont, 
                               LOGFONT * LogFontUsed);

extern bool dlgFontEditShowModal(const TCHAR * FontDescription, 
                          const TCHAR * FontRegKey, 
                          LOGFONT autoLogFont);

static bool changed = false;
static bool taskchanged = false;
static bool requirerestart = false;
static bool utcchanged = false;
static bool waypointneedsave = false;
static bool FontRegistryChanged=false;
int config_page=0;
static WndForm *wf=NULL;
static WndFrame *wConfig1=NULL;
static WndFrame *wConfig2=NULL;
static WndFrame *wConfig3=NULL;
static WndFrame *wConfig4=NULL;
static WndFrame *wConfig5=NULL;
static WndFrame *wConfig6=NULL;
static WndFrame *wConfig7=NULL;
static WndFrame *wConfig8=NULL;
static WndFrame *wConfig9=NULL;
static WndFrame *wConfig10=NULL;
static WndFrame *wConfig11=NULL;
static WndFrame *wConfig12=NULL;
static WndFrame *wConfig13=NULL;
static WndFrame *wConfig14=NULL;
static WndFrame *wConfig15=NULL;
static WndFrame *wConfig16=NULL;
static WndFrame *wConfig17=NULL;
static WndFrame *wConfig18=NULL;
static WndFrame *wConfig19=NULL;
static WndFrame *wConfig20=NULL;
static WndFrame *wConfig21=NULL;
static WndFrame *wConfig22=NULL; 
static WndFrame *wConfig23=NULL; 
static WndFrame *wConfig24=NULL; 
static WndFrame *wConfig25=NULL; 
//static WndFrame *wConfig26=NULL; 
//static WndFrame *wConfig27=NULL; 
// ADDPAGE  HERE
static WndButton *buttonPilotName=NULL;
static WndButton *buttonAircraftType=NULL;
static WndButton *buttonAircraftRego=NULL;
static WndButton *buttonCompetitionClass=NULL;
static WndButton *buttonCompetitionID=NULL;
static WndButton *buttonCopy=NULL;
static WndButton *buttonPaste=NULL;

#define NUMPAGES 25 		// ADDPAGE FIX HERE  27 as of 101126


static void UpdateButtons(void) {
  TCHAR text[120];
  TCHAR val[100];
  if (buttonPilotName) {
    #if OLDPROFILES
    GetRegistryString(szRegistryPilotName, val, 100);
    #else
    _tcscpy(val,PilotName_Config);
    #endif
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _stprintf(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M524_ = "Pilot name" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M524_")), val);
    buttonPilotName->SetCaption(text);
  }
  if (buttonAircraftType) {
    #if OLDPROFILES
    GetRegistryString(szRegistryAircraftType, val, 100);
    #else
    _tcscpy(val,AircraftType_Config);
    #endif
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _stprintf(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M59_ = "Aircraft type" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M59_")), val);
    buttonAircraftType->SetCaption(text);
  }
  if (buttonAircraftRego) {
    #if OLDPROFILES
    GetRegistryString(szRegistryAircraftRego, val, 100);
    #else
    _tcscpy(val,AircraftRego_Config);
    #endif
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _stprintf(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M57_ = "Aircraft Reg" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M57_")), val);
    buttonAircraftRego->SetCaption(text);
  }
  if (buttonCompetitionClass) {
    #if OLDPROFILES
    GetRegistryString(szRegistryCompetitionClass, val, 100);
    #else
    _tcscpy(val,CompetitionClass_Config);
    #endif
    if (_tcslen(val)<=0) {
      // LKTOKEN  _@M7_ = "(blank)" 
      _stprintf(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M936_ = "Competition Class" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M936_")), val);
    buttonCompetitionClass->SetCaption(text);
  }
  if (buttonCompetitionID) {
    #if OLDPROFILES
    GetRegistryString(szRegistryCompetitionID, val, 100);
    #else
    _tcscpy(val,CompetitionID_Config);
    #endif
    if (_tcslen(val)<=0) {
      // LKTOKEN  _@M7_ = "(blank)" 
      _stprintf(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M938_ = "Competition ID" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M938_")), val);
    buttonCompetitionID->SetCaption(text);
  }
}


static void NextPage(int Step){
  config_page += Step;
  if (!EngineeringMenu) { 
	  if (config_page>=(NUMPAGES-2)) { config_page=0; } // ADDPAGE FIX HERE
	  if (config_page<0) { config_page=NUMPAGES-3; } // ADDPAGE FIX HERE
  } else {
	  if (config_page>=NUMPAGES) { config_page=0; }
	  if (config_page<0) { config_page=NUMPAGES-1; }
  }
  switch(config_page) {
  case 0:
	// LKTOKEN  _@M10_ = "1 Site" 
    wf->SetCaption(gettext(TEXT("_@M10_")));
    break;
  case 1:
	// LKTOKEN  _@M22_ = "2 Airspace" 
    wf->SetCaption(gettext(TEXT("_@M22_")));
    break;
  case 2:
	// LKTOKEN  _@M28_ = "3 Map Display" 
    wf->SetCaption(gettext(TEXT("_@M28_")));
    break;
  case 3:
	// LKTOKEN  _@M32_ = "4 Terrain Display" 
    wf->SetCaption(gettext(TEXT("_@M32_")));
    break;
  case 4:
	// LKTOKEN  _@M33_ = "5 Glide Computer" 
    wf->SetCaption(gettext(TEXT("_@M33_")));
    break;
  case 5:
	// LKTOKEN  _@M34_ = "6 Safety factors" 
    wf->SetCaption(gettext(TEXT("_@M34_")));
    break;
  case 6:
	// LKTOKEN  _@M36_ = "7 Aircraft" 
    wf->SetCaption(gettext(TEXT("_@M36_")));
    break;
  case 7:
	// LKTOKEN  _@M37_ = "8 Devices" 
    wf->SetCaption(gettext(TEXT("_@M37_")));
    break;
  case 8:
	// LKTOKEN  _@M38_ = "9 Units" 
    wf->SetCaption(gettext(TEXT("_@M38_")));
    break;
  case 9:
	// LKTOKEN  _@M11_ = "10 Interface" 
    wf->SetCaption(gettext(TEXT("_@M11_")));
    break;
  case 10:
	// LKTOKEN  _@M12_ = "11 Appearance" 
    wf->SetCaption(gettext(TEXT("_@M12_")));
    break;
  case 11:
	// LKTOKEN  _@M13_ = "12 Fonts" 
    wf->SetCaption(gettext(TEXT("_@M13_")));
    break;
  case 12:
	// LKTOKEN  _@M14_ = "13 Map Overlays " 
    wf->SetCaption(gettext(TEXT("_@M14_")));
    break;
  case 13:
	// LKTOKEN  _@M15_ = "14 Task" 
    wf->SetCaption(gettext(TEXT("_@M15_")));
    break;
  case 14:
    wf->SetCaption(gettext(TEXT("_@M1646_"))); // 15 Alarms
    break;
  case 15:
	// LKTOKEN  _@M18_ = "16 InfoBox Cruise" 
    wf->SetCaption(gettext(TEXT("_@M18_")));
    break;
  case 16:
	// LKTOKEN  _@M19_ = "17 InfoBox Thermal" 
    wf->SetCaption(gettext(TEXT("_@M19_")));
    break;
  case 17:
	// LKTOKEN  _@M20_ = "18 InfoBox Final Glide" 
    wf->SetCaption(gettext(TEXT("_@M20_")));
    break;
  case 18:
	// LKTOKEN  _@M21_ = "19 InfoBox Auxiliary" 
    wf->SetCaption(gettext(TEXT("_@M21_")));
    break;
  case 19:
	// LKTOKEN  _@M24_ = "20 Logger" 
    wf->SetCaption(gettext(TEXT("_@M24_")));
    break;
  case 20:
	// LKTOKEN  _@M25_ = "21 Waypoint Edit" 
    wf->SetCaption(gettext(TEXT("_@M25_")));
    break;
  case 21:
	// LKTOKEN  _@M26_ = "22 System" 
    wf->SetCaption(gettext(TEXT("_@M26_")));
    break;
  case 22:
	// LKTOKEN  _@M27_ = "23 Paragliders/Delta specials" 
    wf->SetCaption(gettext(TEXT("_@M27_")));
    break;
  case 23:
    wf->SetCaption(TEXT("24 Engineering Menu 1"));
    break;
  case 24:
    wf->SetCaption(TEXT("25 Engineering Menu 2"));
    break;
  case 25:
    wf->SetCaption(TEXT("26 Engineering Menu 3"));
    break;
  case 26:
    wf->SetCaption(TEXT("27 Engineering Menu 4: old Vario"));
    break;
  } // ADDPAGE FIX HERE
  if ((config_page>=15) && (config_page<=18)) {
    if (buttonCopy) {
      buttonCopy->SetVisible(true);
    }
    if (buttonPaste) {
      buttonPaste->SetVisible(true);
    }
  } else {
    if (buttonCopy) {
      buttonCopy->SetVisible(false);
    }
    if (buttonPaste) {
      buttonPaste->SetVisible(false);
    }
  }
  wConfig1->SetVisible(config_page == 0);
  wConfig2->SetVisible(config_page == 1); 
  wConfig3->SetVisible(config_page == 2); 
  wConfig4->SetVisible(config_page == 3); 
  wConfig5->SetVisible(config_page == 4); 
  wConfig6->SetVisible(config_page == 5); 
  wConfig7->SetVisible(config_page == 6); 
  wConfig8->SetVisible(config_page == 7); 
  wConfig9->SetVisible(config_page == 8); 
  wConfig10->SetVisible(config_page == 9); 
  wConfig11->SetVisible(config_page == 10); 
  wConfig12->SetVisible(config_page == 11); 
  wConfig13->SetVisible(config_page == 12); 
  wConfig14->SetVisible(config_page == 13); 
  wConfig15->SetVisible(config_page == 14); 
  wConfig16->SetVisible(config_page == 15); 
  wConfig17->SetVisible(config_page == 16); 
  wConfig18->SetVisible(config_page == 17); 
  wConfig19->SetVisible(config_page == 18); 
  wConfig20->SetVisible(config_page == 19); 
  wConfig21->SetVisible(config_page == 20); 
  wConfig22->SetVisible(config_page == 21);  
  wConfig23->SetVisible(config_page == 22);  
  wConfig24->SetVisible(config_page == 23);  
  wConfig25->SetVisible(config_page == 24);  
 // wConfig26->SetVisible(config_page == 25);  
 //  wConfig27->SetVisible(config_page == 26);  
} // ADDPAGE FIX HERE



static void OnSetupDeviceAClicked(WindowControl * Sender){
  (void)Sender;

  #if (ToDo)
    devA()->DoSetup();
    wf->FocusNext(NULL);
  #endif

// this is a hack, devices dont jet support device dependant setup dialogs

  if (!SIMMODE) {
	if ((devA() == NULL) || (_tcscmp(devA()->Name,TEXT("Vega")) != 0)) {
		return;
	}
  }

    // this is a hack to get the dialog to retain focus because
    // the progress dialog in the vario configuration somehow causes
    // focus problems
    wf->FocusNext(NULL);
}


static void OnSetupDeviceBClicked(WindowControl * Sender){
  (void)Sender;

  #if (ToDo)
    devB()->DoSetup();
    wf->FocusNext(NULL);
  #endif

// this is a hack, devices dont jet support device dependant setup dialogs
  if (!SIMMODE) {
	if ((devB() == NULL) || (_tcscmp(devB()->Name,TEXT("Vega")) != 0)) {
		return;
	}
  }

    // this is a hack to get the dialog to retain focus because
    // the progress dialog in the vario configuration somehow causes
    // focus problems
    wf->FocusNext(NULL);
}

static void UpdateDeviceSetupButton(int DeviceIdx, TCHAR *Name){ 

  WndButton *wb;
  WndProperty *wp;
  if (DeviceIdx<0||DeviceIdx>1) return;

  #ifdef DEBUG_DEVSETTING
  StartupStore(_T("...... dev=%d, name=<%s> disabled=%d\n"),DeviceIdx, Name, DeviceList[DeviceIdx].Disabled);
  #endif
  if (_tcslen(Name)>0) {
    if (_tcscmp(Name,_T(DEV_DISABLED_NAME))==0) { // Do NOT use tokens here!
	DeviceList[DeviceIdx].Disabled=true;
    } else {
	DeviceList[DeviceIdx].Disabled=false;
    }
  }

  if (DeviceIdx == 0){
    wp = (WndProperty*)wf->FindByName(TEXT("prpComPort1"));
    if (wp != NULL) wp->SetReadOnly(DeviceList[0].Disabled);
    wp = (WndProperty*)wf->FindByName(TEXT("prpComSpeed1"));
    if (wp != NULL) wp->SetReadOnly(DeviceList[0].Disabled);
    wp = (WndProperty*)wf->FindByName(TEXT("prpComBit1"));
    if (wp != NULL) wp->SetReadOnly(DeviceList[0].Disabled);

    wb = ((WndButton *)wf->FindByName(TEXT("cmdSetupDeviceA")));
    if (wb != NULL) wb->SetVisible(false);

  }


  if (DeviceIdx == 1){
    wp = (WndProperty*)wf->FindByName(TEXT("prpComPort2"));
    if (wp != NULL) wp->SetReadOnly(DeviceList[1].Disabled);
    wp = (WndProperty*)wf->FindByName(TEXT("prpComSpeed2"));
    if (wp != NULL) wp->SetReadOnly(DeviceList[1].Disabled);
    wp = (WndProperty*)wf->FindByName(TEXT("prpComBit2"));
    if (wp != NULL) wp->SetReadOnly(DeviceList[1].Disabled);

    wb = ((WndButton *)wf->FindByName(TEXT("cmdSetupDeviceB")));
    if (wb != NULL) wb->SetVisible(false);

  }

}


static void OnDeviceAData(DataField *Sender, DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      UpdateDeviceSetupButton(0, Sender->GetAsString());
    break;
	default: 
		StartupStore(_T("........... DBG-902%s"),NEWLINE); // 091105
		break;
  }

}

static void OnDeviceBData(DataField *Sender, DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      UpdateDeviceSetupButton(1, Sender->GetAsString());
    break;
	default: 
		StartupStore(_T("........... DBG-903%s"),NEWLINE); // 091105
		break;
  }

}
  
static void OnAirspaceFillType(DataField *Sender, DataField::DataAccessKind_t Mode){
  WndProperty* wp;

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceOpacity"));
      if (wp)
        wp->SetVisible( (Sender->GetAsInteger() == (int)MapWindow::asp_fill_ablend_full) || (Sender->GetAsInteger() == (int)MapWindow::asp_fill_ablend_borders) );
    break;
	default: 
		StartupStore(_T("........... DBG-908%s"),NEWLINE); 
		break;
  }
}
 
static void OnAirspaceDisplay(DataField *Sender, DataField::DataAccessKind_t Mode){
  WndProperty* wp;
  int altmode=0;
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceDisplay"));
      if (wp) altmode=(wp->GetDataField()->GetAsInteger());
      // Warning, this is duplicated later on
      wp = (WndProperty*)wf->FindByName(TEXT("prpClipAltitude"));
      if (wp) wp->SetVisible(altmode==CLIP);
      wp = (WndProperty*)wf->FindByName(TEXT("prpAltWarningMargin"));
      if (wp) wp->SetVisible(altmode==AUTO || altmode==ALLBELOW);
    break;
	default: 
		StartupStore(_T("........... DBG-908%s"),NEWLINE); 
		break;
  }
}
 
static void OnLk8000ModeChange(DataField *Sender, DataField::DataAccessKind_t Mode){
  WndProperty* wp;

  switch(Mode){
    case DataField::daGet:
	break;
    case DataField::daPut:
    case DataField::daChange:
	wp = (WndProperty*)wf->FindByName(TEXT("prpLook8000"));
	if (wp) {
		if (Look8000 != (Look8000_t) (wp->GetDataField()->GetAsInteger())) {
			Look8000 = (Look8000_t) (wp->GetDataField()->GetAsInteger());
			if (Look8000!=0) { // 091115 do not allow Reserved mode , do not disable LK8000 !
				#if OLDPROFILES
				SetToRegistry(szRegistryLook8000, (DWORD)(Look8000));
				#endif
				changed = true;
			} else Look8000=1;
		}
	}

	wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayClock"));
	if (wp) {
		if (Look8000==lxcStandard || !ScreenLandscape)  {
			wp->SetReadOnly(true);
			OverlayClock=0;
		} else {
			wp->SetReadOnly(false);
		}
		// Update the OverlayClock selection, without changing of course the content of enumerated list
		DataFieldEnum* dfe;
		dfe = (DataFieldEnum*)wp->GetDataField();
		dfe->Set(OverlayClock);
    		wp->RefreshDisplay();
	}

	break;
    default: 
	StartupStore(_T("........... DBG-908%s"),NEWLINE); 
	break;
  }
}

static void ResetFonts(bool bUseCustom) {
// resest fonts when UseCustomFonts is turned off

  int UseCustomFontsold = UseCustomFonts;
  UseCustomFonts=bUseCustom;



  InitializeOneFont (&TempUseCustomFontsFont, 
                        TEXT("THIS FONT IS NOT CUSTOMIZABLE"), 
                        autoMapWindowLogFont,
                        NULL);

  InitializeOneFont (&TempMapWindowFont, 
                        szRegistryFontMapWindowFont, 
                        autoMapWindowLogFont,
                        NULL);

  InitializeOneFont (&TempMapLabelFont, 
                        szRegistryFontMapLabelFont, 
                        autoMapLabelLogFont,
                        NULL);

  UseCustomFonts=UseCustomFontsold;
}

static void ShowFontEditButtons(bool bVisible) {
  WndProperty * wp;
  wp = (WndProperty*)wf->FindByName(TEXT("cmdMapWindowFont"));
  if (wp) {
    wp->SetVisible(bVisible);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("cmdMapLabelFont"));
  if (wp) {
    wp->SetVisible(bVisible);
  }
}


static void RefreshFonts(void) {

  WndProperty * wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpUseCustomFonts"));
  if (wp) {
    bool bUseCustomFonts= ((DataFieldBoolean*)(wp->GetDataField()))->GetAsBoolean();
    ResetFonts(bUseCustomFonts);
    wp->SetFont(TempUseCustomFontsFont); // this font is never customized
    wp->SetVisible(false);
    wp->SetVisible(true);
    ShowFontEditButtons(bUseCustomFonts);

  }

// now set SampleTexts on the Fonts frame
  wp = (WndProperty*)wf->FindByName(TEXT("prpMapWindowFont"));
  if (wp) {
    wp->SetFont(TempMapWindowFont);
    wp->SetVisible(false);
    wp->SetVisible(true);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMapLabelFont"));
  if (wp) {
    wp->SetFont(TempMapLabelFont);
    wp->SetVisible(false);
    wp->SetVisible(true);
  }

}



static void OnUseCustomFontData(DataField *Sender, DataField::DataAccessKind_t Mode) {

  switch(Mode){
    case DataField::daGet:
    break;

    case DataField::daPut: 
    break;

    case DataField::daChange:
      RefreshFonts();

    break;
	default: 
		StartupStore(_T("........... DBG-904%s"),NEWLINE); // 091105
		break;
  }
}

static void GetFontDescription(TCHAR Description[], TCHAR * prpName, int iMaxLen)
{
  WndProperty * wp;
  wp = (WndProperty*)wf->FindByName(prpName);
  if (wp) {
    _tcsncpy(Description, wp->GetCaption(), iMaxLen-1);
  }
}

static void OnEditMapWindowFontClicked(WindowControl *Sender) {
  TCHAR fontDesc[MAX_EDITFONT_DESC_LEN];
  GetFontDescription(fontDesc, TEXT("prpMapWindowFont"), MAX_EDITFONT_DESC_LEN);
  if (dlgFontEditShowModal(fontDesc,
                            szRegistryFontMapWindowFont, 
                            autoMapWindowLogFont)) {
    FontRegistryChanged=true;
    RefreshFonts(); 
  }
}
static void OnEditMapLabelFontClicked(WindowControl *Sender) {
  TCHAR fontDesc[MAX_EDITFONT_DESC_LEN];
  GetFontDescription(fontDesc, TEXT("prpMapLabelFont"), MAX_EDITFONT_DESC_LEN);
  if (dlgFontEditShowModal(fontDesc,
                            szRegistryFontMapLabelFont, 
                            autoMapLabelLogFont)) {
    FontRegistryChanged=true;
    RefreshFonts(); 
  }
}

static void OnAircraftRegoClicked(WindowControl *Sender) {
	(void)Sender;
  TCHAR Temp[100];
  if (buttonAircraftRego) {
    #if OLDPROFILES
    GetRegistryString(szRegistryAircraftRego,Temp,100);
    #else
    _tcscpy(Temp,AircraftRego_Config);
    #endif
    dlgTextEntryShowModal(Temp,100);
    #if OLDPROFILES
    SetRegistryString(szRegistryAircraftRego,Temp);
    #else
    _tcscpy(AircraftRego_Config,Temp);
    #endif
    changed = true;
  }
  UpdateButtons();
}


static void OnAircraftTypeClicked(WindowControl *Sender) {
	(void)Sender;
  TCHAR Temp[100];
  if (buttonAircraftType) {
    #if OLDPROFILES
    GetRegistryString(szRegistryAircraftType,Temp,100);
    #else
    _tcscpy(Temp,AircraftType_Config);
    #endif
    dlgTextEntryShowModal(Temp,100);
    #if OLDPROFILES
    SetRegistryString(szRegistryAircraftType,Temp);
    #else
    _tcscpy(AircraftType_Config,Temp);
    #endif
    changed = true;
  }
  UpdateButtons();
}


static void OnPilotNameClicked(WindowControl *Sender) {
	(void)Sender;
  TCHAR Temp[100];
  if (buttonPilotName) {
    #if OLDPROFILES
    GetRegistryString(szRegistryPilotName,Temp,100);
    #else
    _tcscpy(Temp,PilotName_Config);
    #endif
    dlgTextEntryShowModal(Temp,100);
    #if OLDPROFILES
    SetRegistryString(szRegistryPilotName,Temp);
    #else
    _tcscpy(PilotName_Config,Temp);
    #endif
    changed = true;
  }
  UpdateButtons();
}


static void OnCompetitionClassClicked(WindowControl *Sender)
{
  TCHAR Temp[100];
  if (buttonCompetitionClass) {
    #if OLDPROFILES
    GetRegistryString(szRegistryCompetitionClass,Temp,100);
    #else
    _tcscpy(Temp,CompetitionClass_Config);
    #endif
    dlgTextEntryShowModal(Temp,100);
    #if OLDPROFILES
    SetRegistryString(szRegistryCompetitionClass,Temp);
    #else
    _tcscpy(CompetitionClass_Config,Temp);
    #endif
    changed = true;
  }
  UpdateButtons();
}


static void OnCompetitionIDClicked(WindowControl *Sender)
{
  TCHAR Temp[100];
  if (buttonCompetitionID) {
    #if OLDPROFILES
    GetRegistryString(szRegistryCompetitionID,Temp,100);
    #else
    _tcscpy(Temp,CompetitionID_Config);
    #endif
    dlgTextEntryShowModal(Temp,100);
    #if OLDPROFILES
    SetRegistryString(szRegistryCompetitionID,Temp);
    #else
    _tcscpy(CompetitionID_Config,Temp);
    #endif
    changed = true;
  }
  UpdateButtons();
}


static void OnAirspaceColoursClicked(WindowControl * Sender){
	(void)Sender;
	bool retval;
	retval = dlgAirspaceShowModal(true);
	if (retval) {
		requirerestart = true;
		changed = true;
	}
}

static void OnSetTopologyClicked(WindowControl * Sender){
	(void)Sender;
  dlgTopologyShowModal();
}
/*
static void OnResetTopologyClicked(WindowControl * Sender){
	(void)Sender;
  dlgTopologyShowModal();
}
*/
static void OnSetCustomKeysClicked(WindowControl * Sender){
	(void)Sender;
  dlgCustomKeysShowModal();
}
static void OnSetBottomBarClicked(WindowControl * Sender){
	(void)Sender;
  dlgBottomBarShowModal();
}
static void OnSetInfoPagesClicked(WindowControl * Sender){
	(void)Sender;
  dlgInfoPagesShowModal();
}
static void OnTaskRulesClicked(WindowControl * Sender){
	(void)Sender;
  dlgTaskRules();
}

static void OnAirspaceWarningParamsClicked(WindowControl * Sender){
	(void)Sender;
  dlgAirspaceWarningParamsShowModal();
}

static void OnAirspaceModeClicked(WindowControl * Sender){
	(void)Sender;
	bool retval;
	retval = dlgAirspaceShowModal(false);
	if (retval) {
		requirerestart = true;
		changed = true;
	}
}

static void OnNextClicked(WindowControl * Sender){
	(void)Sender;
  NextPage(+1);
}

static void OnPrevClicked(WindowControl * Sender){
	(void)Sender;
  NextPage(-1);
}

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}

static int cpyInfoBox[10];

static int page2mode(void) {
  return config_page-15;  // RLD upped by 1
}


static void InfoBoxPropName(TCHAR *name, int item, int mode) {
  _tcscpy(name,TEXT("prpInfoBox"));
  switch (mode) {
  case 0:
    _tcscat(name,TEXT("Circling"));
    break;
  case 1:
    _tcscat(name,TEXT("Cruise"));
    break;
  case 2:
    _tcscat(name,TEXT("FinalGlide"));
    break;
  case 3:
    _tcscat(name,TEXT("Aux"));
    break;
  }
  TCHAR buf[3];
  _stprintf(buf,TEXT("%1d"), item);
  _tcscat(name,buf);
}

static void OnCopy(WindowControl *Sender) {
  (void)Sender;
  int mode = page2mode();
  TCHAR name[80];
  if ((mode<0)||(mode>3)) {
    return;
  }

  for (int item=0; item<8; item++) {
    InfoBoxPropName(name, item, mode);
    WndProperty *wp;
    wp = (WndProperty*)wf->FindByName(name);
    if (wp) {
      cpyInfoBox[item] = wp->GetDataField()->GetAsInteger();
    }
  }
}

static void OnPaste(WindowControl *Sender) {
  (void)Sender;
  int mode = page2mode();
  TCHAR name[80];
  if ((mode<0)||(mode>3)||(cpyInfoBox[0]<0)) {
    return;
  }

  if(MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M510_ = "Overwrite?" 
		 gettext(TEXT("_@M510_")),
	// LKTOKEN  _@M354_ = "InfoBox paste" 
		 gettext(TEXT("_@M354_")),
		 MB_YESNO|MB_ICONQUESTION) == IDYES) {

    for (int item=0; item<8; item++) {
      InfoBoxPropName(name, item, mode);
      WndProperty *wp;
      wp = (WndProperty*)wf->FindByName(name);
      if (wp && (cpyInfoBox[item]>=0)&&(cpyInfoBox[item]<NumDataOptions)) {
	wp->GetDataField()->Set(cpyInfoBox[item]);
	wp->RefreshDisplay();
      }
    }
  }
}

static int FormKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){
	(void)lParam;
	(void)Sender;
  switch(wParam & 0xffff){
    case '6':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdPrev")))->GetHandle());
      NextPage(-1);
      //((WndButton *)wf->FindByName(TEXT("cmdPrev")))->SetFocused(true, NULL);
    return(0);
    case '7':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdNext")))->GetHandle());
      NextPage(+1);
      //((WndButton *)wf->FindByName(TEXT("cmdNext")))->SetFocused(true, NULL);
    return(0);
  }
  return(1);
}

static void SetLocalTime(void) {
  WndProperty* wp;
  TCHAR temp[20];
  Units::TimeToText(temp, 
		    (int)TimeLocal((int)(GPS_INFO.Time)));
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpLocalTime"));
  if (wp) {
    wp->SetText(temp);
    wp->RefreshDisplay();
  }
}

static void OnUTCData(DataField *Sender, DataField::DataAccessKind_t Mode){
//  WndProperty* wp;
  int ival;

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut: 
    case DataField::daChange:
      ival = iround(Sender->GetAsFloat()*3600.0);
      if (UTCOffset != ival) {
	UTCOffset = ival;
	utcchanged = true;
      }
      SetLocalTime();
    break;
	default: 
		StartupStore(_T("........... DBG-905%s"),NEWLINE); // 091105
		break;
  }

}

#if 0 // REMOVE
static void OnPolarFileData(DataField *Sender, DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
    break;
	default: 
		StartupStore(_T("........... DBG-907%s"),NEWLINE); // 091105
		break;
  }

}

// Probably we can remove this stuff entirely!
static void OnPilotFileData(DataField *Sender, DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
    break;
	default: 
		StartupStore(_T("........... DBG-907%s"),NEWLINE); // 091105
		break;
  }
#endif



// mode 0: Aircraft
// mode 1: Pilot
static void OnProfileSaveAs(WindowControl * Sender, short mode) {
  (void)Sender;

  int file_index; 
  TCHAR file_name[MAX_PATH];
  WndProperty* wp;
  DataFieldFileReader *dfe;

  if ( CheckClubVersion() ) {
	ClubForbiddenMsg();
	return;
  }

  if (mode==0)
	wp = (WndProperty*)wf->FindByName(TEXT("prpAircraftFile"));
  else
	wp = (WndProperty*)wf->FindByName(TEXT("prpPilotFile"));

  if (!wp) return;

  HWND hwnd = wp->GetHandle();
  SendMessage(hwnd,WM_LBUTTONDOWN,0,0);
  dfe = (DataFieldFileReader*) wp->GetDataField();

  file_index = dfe->GetAsInteger();

  if (file_index>0) {
	_tcscpy(file_name,dfe->GetAsString());
	if(MessageBoxX(hWndMapWindow, file_name, 
	// LKTOKEN  _@M509_ = "Overwrite profile?" 
		gettext(TEXT("_@M509_")), 
		MB_YESNO|MB_ICONQUESTION) == IDYES) {
		if (mode==0) {
			UpdateAircraftConfig();
			LKAircraftSave(dfe->GetPathFile());
		} else {
			// UpdatePilotConfig(); REMOVE
			LKPilotSave(dfe->GetPathFile());
		}
	// LKTOKEN  _@M535_ = "Profile saved!" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M535_")),_T(""), MB_OK|MB_ICONEXCLAMATION);
		return;
	}
  	dfe->Set(0);
  } 
}


static void OnAircraftSaveAsClicked(WindowControl * Sender) {
  (void)Sender;
	OnProfileSaveAs(Sender,0 );
}
static void OnPilotSaveAsClicked(WindowControl * Sender) {
  (void)Sender;
	OnProfileSaveAs(Sender,1 );
}

// mode 0: Aircraft
// mode 1: Pilot
static void OnProfileSaveNew(WindowControl * Sender, short mode) {
  (void)Sender;

  int file_index; 
  TCHAR file_name[MAX_PATH];
  TCHAR profile_name[MAX_PATH];
  TCHAR tmptext[MAX_PATH];
  WndProperty* wp;
  DataFieldFileReader *dfe;

  if (mode==0)
	wp = (WndProperty*)wf->FindByName(TEXT("prpAircraftFile"));
  else
	wp = (WndProperty*)wf->FindByName(TEXT("prpPilotFile"));

  if (!wp) return;
  dfe = (DataFieldFileReader*) wp->GetDataField();

  _tcscpy(profile_name,_T(""));
  dlgTextEntryShowModal(profile_name, 13); // max length including termination 0

  if (_tcslen(profile_name)<=0) return;

  if (mode==0)
	_tcscat(profile_name, TEXT(LKS_AIRCRAFT));
  else
	_tcscat(profile_name, TEXT(LKS_PILOT));

  LocalPath(file_name,TEXT(LKD_CONF));
  _tcscat(file_name,TEXT("\\"));
  _tcscat(file_name,profile_name);

  dfe->Lookup(file_name);
  file_index = dfe->GetAsInteger();

  if (file_index==0) {
	_stprintf(tmptext, TEXT("%s: %s"), 
	// LKTOKEN  _@M458_ = "New profile" 
		gettext(TEXT("_@M458_")), 
		profile_name);

	if(MessageBoxX(hWndMapWindow, tmptext, 
	// LKTOKEN  _@M579_ = "Save ?" 
		gettext(TEXT("_@M579_")), 
		MB_YESNO|MB_ICONQUESTION) == IDYES) {
		if (mode==0) {
			UpdateAircraftConfig();
			LKAircraftSave(file_name);
		} else {
			// UpdatePilotConfig(); // REMOVE
			LKPilotSave(file_name);
		}
		dfe->addFile(profile_name, file_name);

		MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M535_ = "Profile saved!" 
		gettext(TEXT("_@M535_")), 
		_T(""), MB_OK|MB_ICONEXCLAMATION);

  		dfe->Set(0);
		return;
	}
  }

  if (file_index>0) {
	_stprintf(tmptext, TEXT("%s: %s"), 
	// LKTOKEN  _@M533_ = "Profile already exists" 
		gettext(TEXT("_@M533_")), 	
		profile_name);

	if (CheckClubVersion() ) {
		MessageBoxX(hWndMapWindow, tmptext,
	// LKTOKEN  _@M162_ = "Cannot overwrite!" 
		gettext(TEXT("_@M162_")),
		MB_OK|MB_ICONEXCLAMATION);
	} else {
		if(MessageBoxX(hWndMapWindow, tmptext, 
	// LKTOKEN  _@M510_ = "Overwrite?" 
		gettext(TEXT("_@M510_")), 
		MB_YESNO|MB_ICONQUESTION) == IDYES) {

			if (mode==0) {
				LKAircraftSave(file_name);
			} else {
				LKPilotSave(file_name);
			}
			MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M535_ = "Profile saved!" 
			gettext(TEXT("_@M535_")),
			_T(""), MB_OK|MB_ICONEXCLAMATION);
			return;
		}
	}
  	dfe->Set(0);
  }

} // Save new


static void OnAircraftSaveNewClicked(WindowControl * Sender) {
	OnProfileSaveNew(Sender,0 );
}
static void OnPilotSaveNewClicked(WindowControl * Sender) {
	OnProfileSaveNew(Sender,1 );
}


extern void OnInfoBoxHelp(WindowControl * Sender);

static void OnWaypointNewClicked(WindowControl * Sender){
  (void)Sender;

  // Cannot save waypoint if no file
  if ( NumberOfWayPoints<=NUMRESWP) { 
	MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M478_ = "No waypoint file selected, cannot save." 
	gettext(TEXT("_@M478_")),
	// LKTOKEN  _@M457_ = "New Waypoint" 
               gettext(TEXT("_@M457_")),
               MB_OK|MB_ICONEXCLAMATION);

	return; 
  }
 
  WAYPOINT edit_waypoint;
  edit_waypoint.Latitude = GPS_INFO.Latitude;
  edit_waypoint.Longitude = GPS_INFO.Longitude;
  edit_waypoint.FileNum = 0; // default, put into primary waypoint file
  edit_waypoint.Flags = 0;
  edit_waypoint.Comment=(TCHAR*)malloc(100*sizeof(TCHAR));

  extern void MSG_NotEnoughMemory(void);
  if (edit_waypoint.Comment == (TCHAR *)NULL) {
	MSG_NotEnoughMemory();
	return;
  }
  _tcscpy(edit_waypoint.Comment,_T(""));

  edit_waypoint.Name[0] = 0;
  edit_waypoint.Details = 0;
  edit_waypoint.Number = NumberOfWayPoints;
  edit_waypoint.Format = LKW_NEW;	// 100208
  edit_waypoint.RunwayLen = 0;
  edit_waypoint.RunwayDir = -1;
  edit_waypoint.Style = 1; // normal turnpoint
  edit_waypoint.Code[0]=0;
  edit_waypoint.Freq[0]=0;
  edit_waypoint.Country[0]=0;
  dlgWaypointEditShowModal(&edit_waypoint);

  // SeeYou style not correct when new waypoint created
  // This setting will override Flags when reloading the file after changes!
  if ( (edit_waypoint.Flags & AIRPORT) == AIRPORT) {
	edit_waypoint.Style=5;  // airport
  } else {
	if ( (edit_waypoint.Flags & LANDPOINT) == LANDPOINT) {
		edit_waypoint.Style=3;  // outlanding
	}
  }
  // else it is already a turnpoint


  if (_tcslen(edit_waypoint.Name)>0) {
    LockTaskData();
    WAYPOINT *new_waypoint = GrowWaypointList();
    if (new_waypoint) {
      memcpy(new_waypoint,&edit_waypoint,sizeof(WAYPOINT));
      new_waypoint->Details= 0;
      waypointneedsave = true;
    } 
    UnlockTaskData();
  }
}


static void OnWaypointEditClicked(WindowControl * Sender){
	(void)Sender;
  int res;
  if (CheckClubVersion()) {
	ClubForbiddenMsg();
	return;
  }
  res = dlgWayPointSelect();
  if (res != -1){
	#if 0 // 101214 READ ONLY FILES
	if ( WayPointList[res].Format == LKW_COMPE) {      // 100212
		MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
		gettext(TEXT("_@M716_")),
	// LKTOKEN  _@M194_ = "CompeGPS Waypoint" 
                gettext(TEXT("_@M194_")),
                MB_OK|MB_ICONEXCLAMATION);

		return;
	}
	#endif

	if ( WayPointList[res].Format == LKW_VIRTUAL) {      // 100212
		MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
		gettext(TEXT("_@M716_")),
	// LKTOKEN  _@M775_ = "VIRTUAL Waypoint" 
                gettext(TEXT("_@M775_")),
                MB_OK|MB_ICONEXCLAMATION);

		return;
	}


    dlgWaypointEditShowModal(&WayPointList[res]);
    waypointneedsave = true;
  }
}

static void AskWaypointSave(void) {
  if (WaypointsOutOfRange==2) {

    if(MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M810_ = "Waypoints excluded, save anyway?" 
                   gettext(TEXT("_@M810_")),
	// LKTOKEN  _@M811_ = "Waypoints outside terrain" 
                   gettext(TEXT("_@M811_")),
                   MB_YESNO|MB_ICONQUESTION) == IDYES) {
      
      WaypointWriteFiles();
      
      WAYPOINTFILECHANGED= true;
      changed = true;
      
    }
  } else {
    
    WaypointWriteFiles();
    
    WAYPOINTFILECHANGED= true;
    changed = true;
  }
  waypointneedsave = false;
}


static void OnWaypointSaveClicked(WindowControl * Sender){
  (void)Sender;

  AskWaypointSave();
}

static void OnWaypointDeleteClicked(WindowControl * Sender){
	(void)Sender;
  int res;
  if (CheckClubVersion()) {
	ClubForbiddenMsg();
	return;
  }
  res = dlgWayPointSelect();
  if (res > RESWP_END) { // 100212
	#if 0 // 101214 READ ONLY FILES
	if ( WayPointList[res].Format == LKW_COMPE ) { // 100212
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M716_")),
	// LKTOKEN  _@M194_ = "CompeGPS Waypoint" 
			gettext(TEXT("_@M194_")),
			MB_OK|MB_ICONEXCLAMATION);
		return;
	} else 
	#endif
		if ( WayPointList[res].Format == LKW_VIRTUAL ) { // 100212
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
			MessageBoxX(hWndMapWindow, gettext(TEXT("_@M716_")),
	// LKTOKEN  _@M775_ = "VIRTUAL Waypoint" 
				gettext(TEXT("_@M775_")),
				MB_OK|MB_ICONEXCLAMATION);
			return;
		} else 
	// LKTOKEN  _@M229_ = "Delete Waypoint?" 
	if(MessageBoxX(hWndMapWindow, WayPointList[res].Name, gettext(TEXT("_@M229_")), 
	MB_YESNO|MB_ICONQUESTION) == IDYES) {
		LockTaskData();
		WayPointList[res].FileNum = -1;
		UnlockTaskData();
		waypointneedsave = true;
	}
  }
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnAirspaceColoursClicked),
  DeclareCallBackEntry(OnAirspaceModeClicked),
  DeclareCallBackEntry(OnAirspaceWarningParamsClicked),
  DeclareCallBackEntry(OnUTCData),
  DeclareCallBackEntry(OnNextClicked),
  DeclareCallBackEntry(OnPrevClicked),
  DeclareCallBackEntry(OnSetupDeviceAClicked),
  DeclareCallBackEntry(OnSetupDeviceBClicked),
  DeclareCallBackEntry(OnInfoBoxHelp),
  DeclareCallBackEntry(OnWaypointNewClicked),
  DeclareCallBackEntry(OnWaypointDeleteClicked),
  DeclareCallBackEntry(OnWaypointEditClicked),
  DeclareCallBackEntry(OnWaypointSaveClicked),

//  DeclareCallBackEntry(OnPolarFileData), REMOVE

  DeclareCallBackEntry(OnDeviceAData),
  DeclareCallBackEntry(OnDeviceBData),

  DeclareCallBackEntry(OnPilotSaveAsClicked),
  DeclareCallBackEntry(OnPilotSaveNewClicked),

  DeclareCallBackEntry(OnAircraftSaveAsClicked),
  DeclareCallBackEntry(OnAircraftSaveNewClicked),

  DeclareCallBackEntry(OnUseCustomFontData),
  DeclareCallBackEntry(OnEditMapWindowFontClicked),
  DeclareCallBackEntry(OnEditMapLabelFontClicked),

  DeclareCallBackEntry(OnSetTopologyClicked),
  DeclareCallBackEntry(OnSetCustomKeysClicked),
  DeclareCallBackEntry(OnSetBottomBarClicked),
  DeclareCallBackEntry(OnSetInfoPagesClicked),
  DeclareCallBackEntry(OnTaskRulesClicked),
  
  DeclareCallBackEntry(OnAirspaceFillType),
  DeclareCallBackEntry(OnAirspaceDisplay),
  DeclareCallBackEntry(OnLk8000ModeChange),
  DeclareCallBackEntry(NULL)
};



static void SetInfoBoxSelector(int item, int mode)
{
  TCHAR name[80];
  InfoBoxPropName(name, item, mode);

  WndProperty *wp;
  wp = (WndProperty*)wf->FindByName(name);
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    for (int i=0; i<NumDataOptions; i++) {
      dfe->addEnumText(gettext(Data_Options[i].Description));
    }
    dfe->Sort(0);

    int it=0;
    
    switch(mode) {
    case 1: // climb
      it = (InfoType[item])& 0xff;
      break;
    case 0: // cruise
      it = (InfoType[item]>>8)& 0xff;
      break;
    case 2: // final glide
      it = (InfoType[item]>>16)& 0xff;
      break;
    case 3: // aux
      it = (InfoType[item]>>24)& 0xff;
      break;
    };
    dfe->Set(it);
    wp->RefreshDisplay();
  }
}


static void GetInfoBoxSelector(int item, int mode)
{
  TCHAR name[80];
  InfoBoxPropName(name, item, mode);
  WndProperty *wp;
  wp = (WndProperty*)wf->FindByName(name);
  if (wp) {
    int itnew = wp->GetDataField()->GetAsInteger();
    int it=0;
    
    switch(mode) {
    case 1: // climb
      it = (InfoType[item])& 0xff;
      break;
    case 0: // cruise
      it = (InfoType[item]>>8)& 0xff;
      break;
    case 2: // final glide
      it = (InfoType[item]>>16)& 0xff;
      break;
    case 3: // aux
      it = (InfoType[item]>>24)& 0xff;
      break;
    };

    if (it != itnew) {

      changed = true;

      switch(mode) {
      case 0: // cruise
	InfoType[item] &= 0xffff00ff;
	InfoType[item] += (itnew<<8);
	break;
      case 1: // climb
	InfoType[item] &= 0xffffff00;
	InfoType[item] += itnew;
	break;
      case 2: // final glide
	InfoType[item] &= 0xff00ffff;
	InfoType[item] += (itnew<<16);
	break;
      case 3: // aux
	InfoType[item] &= 0x00ffffff;
	InfoType[item] += (itnew<<24);
	break;
      };
#if OLDPROFILES
      StoreType(item,InfoType[item]);
#endif
    }
  }
}

static  int dwDeviceIndex1=0;
static  int dwDeviceIndex2=0;
static  TCHAR DeviceName[DEVNAMESIZE+1];
static  TCHAR temptext[MAX_PATH];



static void setVariables(void) {
  WndProperty *wp;

  TCHAR tsuf[10]; // 091101
  buttonPilotName = ((WndButton *)wf->FindByName(TEXT("cmdPilotName")));
  if (buttonPilotName) {
    buttonPilotName->SetOnClickNotify(OnPilotNameClicked);
  }
  buttonAircraftType = ((WndButton *)wf->FindByName(TEXT("cmdAircraftType")));
  if (buttonAircraftType) {
    buttonAircraftType->SetOnClickNotify(OnAircraftTypeClicked);
  }
  buttonAircraftRego = ((WndButton *)wf->FindByName(TEXT("cmdAircraftRego")));
  if (buttonAircraftRego) {
    buttonAircraftRego->SetOnClickNotify(OnAircraftRegoClicked);
  }
  buttonCompetitionClass = ((WndButton *)wf->FindByName(TEXT("cmdCompetitionClass")));
  if (buttonCompetitionClass) {
    buttonCompetitionClass->SetOnClickNotify(OnCompetitionClassClicked);
  }
  buttonCompetitionID = ((WndButton *)wf->FindByName(TEXT("cmdCompetitionID")));
  if (buttonCompetitionID) {
    buttonCompetitionID->SetOnClickNotify(OnCompetitionIDClicked);
  }
  buttonCopy = ((WndButton *)wf->FindByName(TEXT("cmdCopy")));
  if (buttonCopy) {
    buttonCopy->SetOnClickNotify(OnCopy);
  }
  buttonPaste = ((WndButton *)wf->FindByName(TEXT("cmdPaste")));
  if (buttonPaste) {
    buttonPaste->SetOnClickNotify(OnPaste);
  }

  UpdateButtons();


// extended COM ports for PC
// Changing items requires also changing the i<13 loop later on for port1 and port2
#if (WINDOWSPC>0)
    const TCHAR *COMMPort[] = {TEXT("COM1"),TEXT("COM2"),TEXT("COM3"),TEXT("COM4"),
			       TEXT("COM5"),TEXT("COM6"),TEXT("COM7"),TEXT("COM8"),
			       TEXT("COM9"),TEXT("COM10"),TEXT("COM11"),TEXT("COM12"),TEXT("COM13"),TEXT("COM14"),
			       TEXT("COM15"),TEXT("COM16"),TEXT("COM17"),TEXT("COM18"),TEXT("COM19"),TEXT("COM20"),
			       TEXT("COM21"),TEXT("COM22"),TEXT("COM23"),TEXT("COM24"),TEXT("COM25"),TEXT("COM26"),
			       TEXT("COM27"),TEXT("COM28"),TEXT("COM29"),TEXT("COM30"),TEXT("COM31"),TEXT("COM32"),
				TEXT("COM0")};
#else
    const TCHAR *COMMPort[] = {TEXT("COM1"),TEXT("COM2"),TEXT("COM3"),TEXT("COM4"),
			       TEXT("COM5"),TEXT("COM6"),TEXT("COM7"),TEXT("COM8"),
			       TEXT("COM9"),TEXT("COM10"),TEXT("COM0"),TEXT("VSP0"),TEXT("VSP1")};
#endif
    const TCHAR *tSpeed[] = {TEXT("1200"),TEXT("2400"),TEXT("4800"),TEXT("9600"),
			     TEXT("19200"),TEXT("38400"),TEXT("57600"),TEXT("115200")};
//  DWORD dwSpeed[] = {1200,2400,4800,9600,19200,38400,57600,115200};

  int i;

  ReadPort1Settings(&dwPortIndex1,&dwSpeedIndex1, &dwBit1Index);

  wp = (WndProperty*)wf->FindByName(TEXT("prpComPort1"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
#if (WINDOWSPC>0)
    for (i=0; i<33; i++) { // 091117
#else
    for (i=0; i<13; i++) {
#endif
      dfe->addEnumText((COMMPort[i]));
    }
    dfe->Set(dwPortIndex1);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComSpeed1"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    for (i=0; i<8; i++) {
      dfe->addEnumText((tSpeed[i]));
    }
    dfe->Set(dwSpeedIndex1);
    wp->SetReadOnly(false);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpComBit1"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("8bit"));
    dfe->addEnumText(TEXT("7bit"));
    dfe->Set(dwBit1Index);
    wp->RefreshDisplay();
  }

  TCHAR deviceName1[MAX_PATH];
  TCHAR deviceName2[MAX_PATH];
  ReadDeviceSettings(0, deviceName1);
  ReadDeviceSettings(1, deviceName2);
  #ifdef DEBUG_DEVSETTING
  StartupStore(_T("...... Config ReadDeviceSet 0=<%s> 1=<%s>\n"),deviceName1, deviceName2);
  #endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpComDevice1"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    for (i=0; i<DeviceRegisterCount; i++) {
      devRegisterGetName(i, DeviceName);
      dfe->addEnumText((DeviceName));

      if (_tcscmp(DeviceName, deviceName1) == 0)
        dwDeviceIndex1 = i;

    }
    dfe->Sort(2);
    dfe->Set(dwDeviceIndex1);
    wp->RefreshDisplay();
  }

  ReadPort2Settings(&dwPortIndex2,&dwSpeedIndex2, &dwBit2Index);

  wp = (WndProperty*)wf->FindByName(TEXT("prpComPort2"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
#if (WINDOWSPC>0)
    for (i=0; i<33; i++) { // 091117
#else
    for (i=0; i<13; i++) {
#endif
      dfe->addEnumText((COMMPort[i]));
    }
    dfe->Set(dwPortIndex2);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComSpeed2"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    for (i=0; i<8; i++) {
      dfe->addEnumText((tSpeed[i]));
    }
    dfe->Set(dwSpeedIndex2);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComBit2"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("8bit"));
    dfe->addEnumText(TEXT("7bit"));
    dfe->Set(dwBit2Index);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComDevice2"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    for (i=0; i<DeviceRegisterCount; i++) {
      devRegisterGetName(i, DeviceName);
      dfe->addEnumText((DeviceName));
      if (_tcscmp(DeviceName, deviceName2) == 0)
        dwDeviceIndex2 = i;
    }
    dfe->Sort(2);
    dfe->Set(dwDeviceIndex2);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceDisplay"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M79_ = "All on" 
    dfe->addEnumText(gettext(TEXT("_@M79_")));
	// LKTOKEN  _@M184_ = "Clip" 
    dfe->addEnumText(gettext(TEXT("_@M184_")));
    dfe->addEnumText(gettext(TEXT("Auto")));
	// LKTOKEN  _@M77_ = "All below" 
    dfe->addEnumText(gettext(TEXT("_@M77_")));
    dfe->Set(AltitudeMode_Config);
    wp->RefreshDisplay();
      wp = (WndProperty*)wf->FindByName(TEXT("prpClipAltitude"));
      if (wp) wp->SetVisible(AltitudeMode_Config==CLIP);
      wp = (WndProperty*)wf->FindByName(TEXT("prpAltWarningMargin"));
      if (wp) wp->SetVisible(AltitudeMode_Config==AUTO || AltitudeMode_Config==ALLBELOW);
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceFillType"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// TODOasp: transl 
    dfe->addEnumText(gettext(TEXT("_@M941_")));
    dfe->addEnumText(gettext(TEXT("_@M942_")));
    dfe->addEnumText(gettext(TEXT("_@M945_")));
    if (MapWindow::AlphaBlendSupported()) {
      dfe->addEnumText(gettext(TEXT("_@M943_")));
      dfe->addEnumText(gettext(TEXT("_@M946_")));
    }
    dfe->Set((int)MapWindow::GetAirSpaceFillType());
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceOpacity"));
  if (wp) {
    wp->SetVisible(true);
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("0 %"));
    dfe->addEnumText(TEXT("10 %"));
    dfe->addEnumText(TEXT("20 %"));
    dfe->addEnumText(TEXT("30 %"));
    dfe->addEnumText(TEXT("40 %"));
    dfe->addEnumText(TEXT("50 %"));
    dfe->addEnumText(TEXT("60 %"));
    dfe->addEnumText(TEXT("70 %"));
    dfe->addEnumText(TEXT("80 %"));
    dfe->addEnumText(TEXT("90 %"));
    dfe->addEnumText(TEXT("100 %"));
    dfe->Set(MapWindow::GetAirSpaceOpacity() / 10);
    
    wp->SetVisible( (MapWindow::GetAirSpaceFillType() == (int)MapWindow::asp_fill_ablend_full) || (MapWindow::GetAirSpaceFillType() == (int)MapWindow::asp_fill_ablend_borders) );
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBarOpacity"));
  if (wp) {
    wp->SetVisible(true);
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("0 %"));
    dfe->addEnumText(TEXT("5 %"));
    dfe->addEnumText(TEXT("10 %"));
    dfe->addEnumText(TEXT("15 %"));
    dfe->addEnumText(TEXT("20 %"));
    dfe->addEnumText(TEXT("25 %"));
    dfe->addEnumText(TEXT("30 %"));
    dfe->addEnumText(TEXT("35 %"));
    dfe->addEnumText(TEXT("40 %"));
    dfe->addEnumText(TEXT("45 %"));
    dfe->addEnumText(TEXT("50 %"));
    dfe->addEnumText(TEXT("55 %"));
    dfe->addEnumText(TEXT("60 %"));
    dfe->addEnumText(TEXT("65 %"));
    dfe->addEnumText(TEXT("70 %"));
    dfe->addEnumText(TEXT("75 %"));
    dfe->addEnumText(TEXT("80 %"));
    dfe->addEnumText(TEXT("85 %"));
    dfe->addEnumText(TEXT("90 %"));
    dfe->addEnumText(TEXT("95 %"));
    dfe->addEnumText(TEXT("100 %"));
    if (!MapWindow::AlphaBlendSupported()) {
	dfe->Set(100 / 5);
	wp->SetReadOnly(!MapWindow::AlphaBlendSupported());
    } else {
	dfe->Set(BarOpacity / 5);
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontRenderer"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();

    dfe->addEnumText(gettext(TEXT("_@M955_"))); // Clear Type Compatible
    dfe->addEnumText(gettext(TEXT("_@M956_"))); // Anti Aliasing
    dfe->addEnumText(gettext(TEXT("_@M480_"))); // Normal
    dfe->addEnumText(gettext(TEXT("_@M479_"))); // None
    dfe->Set(FontRenderer);
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeMode"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M382_ = "Landables only" 
    dfe->addEnumText(gettext(TEXT("_@M382_")));
	// LKTOKEN  _@M380_ = "Landables and Turnpoints" 
    dfe->addEnumText(gettext(TEXT("_@M380_")));
    dfe->Set(SafetyAltitudeMode);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUTCOffset"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(UTCOffset/1800.0)/2.0);
    wp->RefreshDisplay();
  }
  SetLocalTime();

  wp = (WndProperty*)wf->FindByName(TEXT("prpClipAltitude"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(ClipAltitude*ALTITUDEMODIFY));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }
    
  wp = (WndProperty*)wf->FindByName(TEXT("prpAltWarningMargin"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(AltWarningMargin*ALTITUDEMODIFY));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoZoom"));
  if (wp) {
    wp->GetDataField()->Set(AutoZoom_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningMessageRepeatTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AirspaceWarningRepeatTime/60);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceOutline"));
  if (wp) {
    wp->GetDataField()->Set(MapWindow::bAirspaceBlackOutline);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLockSettingsInFlight"));
  if (wp) {
    wp->GetDataField()->Set(LockSettingsInFlight);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLoggerShortName"));
  if (wp) {
    wp->GetDataField()->Set(LoggerShortName);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDebounceTimeout"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(debounceTimeout);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableFLARMMap"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("_@M239_"))); // Disabled
    dfe->addEnumText(gettext(TEXT("_@M259_"))); // Enabled
    // dfe->addEnumText(gettext(TEXT("_@M959_"))); // OFF
    // dfe->addEnumText(gettext(TEXT("_@M496_"))); // ON fixed
    // dfe->addEnumText(gettext(TEXT("_@M497_"))); // ON scaled
    dfe->Set(EnableFLARMMap);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointLabels"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M454_ = "Names" 
    dfe->addEnumText(gettext(TEXT("_@M454_")));

	// LKTOKEN  _@M488_ = "Numbers" 
    dfe->addEnumText(gettext(TEXT("_@M488_")));

	// LKTOKEN  _@M453_ = "Names in task" 
    dfe->addEnumText(gettext(TEXT("_@M453_")));

	// LKTOKEN  _@M301_ = "First 3" 
    dfe->addEnumText(gettext(TEXT("_@M301_")));

	// LKTOKEN  _@M302_ = "First 5" 
    dfe->addEnumText(gettext(TEXT("_@M302_")));

	// LKTOKEN  _@M838_ = "First 8" 
    dfe->addEnumText(gettext(TEXT("_@M838_")));
	// LKTOKEN  _@M839_ = "First 10" 
    dfe->addEnumText(gettext(TEXT("_@M839_")));
	// LKTOKEN  _@M840_ = "First 12" 
    dfe->addEnumText(gettext(TEXT("_@M840_")));

	// LKTOKEN  _@M479_ = "None" 
    dfe->addEnumText(gettext(TEXT("_@M479_")));
    dfe->Set(DisplayTextType);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableTerrain"));
  if (wp) {
    wp->GetDataField()->Set(EnableTerrain_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableTopology"));
  if (wp) {
    wp->GetDataField()->Set(EnableTopology_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpCirclingZoom"));
  if (wp) {
    wp->GetDataField()->Set(MapWindow::zoom.CircleZoom());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOrientation"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M737_ = "Track up" 
    dfe->addEnumText(gettext(TEXT("_@M737_")));
	// LKTOKEN  _@M483_ = "North up" 
    dfe->addEnumText(gettext(TEXT("_@M483_")));
	// LKTOKEN  _@M482_ = "North circling" 
    dfe->addEnumText(gettext(TEXT("_@M482_")));
	// LKTOKEN  _@M682_ = "Target circling" 
    dfe->addEnumText(gettext(TEXT("_@M682_")));
	// LKTOKEN  _@M484_ = "North/track" 
    dfe->addEnumText(gettext(TEXT("_@M484_")));
	// LKTOKEN  _@M481_ = "North Smart" 
    dfe->addEnumText(gettext(TEXT("_@M481_"))); // 100417
    dfe->Set(DisplayOrientation_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMenuTimeout"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(MenuTimeout_Config/2);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeArrival"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(ALTITUDEMODIFY*SAFETYALTITUDEARRIVAL));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeTerrain"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(ALTITUDEMODIFY*SAFETYALTITUDETERRAIN));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFinalGlideTerrain"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M959_ = "OFF" 
    dfe->addEnumText(gettext(TEXT("_@M959_")));
	// LKTOKEN  _@M393_ = "Line" 
    dfe->addEnumText(gettext(TEXT("_@M393_")));
	// LKTOKEN  _@M609_ = "Shade" 
    dfe->addEnumText(gettext(TEXT("_@M609_")));
    dfe->Set(FinalGlideTerrain);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableNavBaroAltitude"));
  if (wp) {
    wp->GetDataField()->Set(EnableNavBaroAltitude_Config);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpOrbiter"));
  if (wp) {
    wp->GetDataField()->Set(Orbiter_Config);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoMcStatus"));
  if (wp) {
    wp->GetDataField()->Set(AutoMacCready_Config);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpShading"));
  if (wp) {
    wp->GetDataField()->Set(Shading_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoWind"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M418_ = "Manual" 
    dfe->addEnumText(gettext(TEXT("_@M418_")));
	// LKTOKEN  _@M175_ = "Circling" 
    dfe->addEnumText(gettext(TEXT("_@M175_")));
    dfe->addEnumText(gettext(TEXT("ZigZag")));
	// LKTOKEN  _@M149_ = "Both" 
    dfe->addEnumText(gettext(TEXT("_@M149_")));
    wp->GetDataField()->Set(AutoWindMode_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoMcMode"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("_@M290_")));  // Final Glide
    dfe->addEnumText(gettext(TEXT("_@M1684_"))); // Average thermal
    dfe->addEnumText(gettext(TEXT("_@M1685_"))); // Final + Average
    dfe->addEnumText(gettext(TEXT("_@M262_")));  // Equivalent
    wp->GetDataField()->Set(AutoMcMode_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointsOutOfRange"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M100_ = "Ask" 
    dfe->addEnumText(gettext(TEXT("_@M100_")));
	// LKTOKEN  _@M350_ = "Include" 
    dfe->addEnumText(gettext(TEXT("_@M350_")));
	// LKTOKEN  _@M269_ = "Exclude" 
    dfe->addEnumText(gettext(TEXT("_@M269_")));
    wp->GetDataField()->Set(WaypointsOutOfRange);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoForceFinalGlide"));
  if (wp) {
    wp->GetDataField()->Set(AutoForceFinalGlide);
    wp->RefreshDisplay();
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpHandicap"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(Handicap);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPGClimbZoom"));
  if (wp) {
    TCHAR buf1[32];
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    for (int i=0; i<=5; ++i) {
      MapWindow::zoom.GetPgClimbInitMapScaleText(i, buf1, sizeof(buf1)/sizeof(buf1[0]));
      dfe->addEnumText(buf1);
    }
    dfe->Set(PGClimbZoom);
    // if (!ISPARAGLIDER) wp->SetVisible(false); 
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGCruiseZoom"));
  if (wp) {
    TCHAR buf1[32];
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    for (int i=0; i<=9; ++i) {
      if (MapWindow::zoom.GetPgCruiseInitMapScaleText(i, buf1, sizeof(buf1)/sizeof(buf1[0]))) {
	dfe->addEnumText(buf1);
      } else {
	_stprintf(buf1,TEXT("%d"),i);
	dfe->addEnumText(buf1);
      }
    }
    dfe->Set(PGCruiseZoom);
    // if (!ISPARAGLIDER) wp->SetVisible(false); 
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGAutoZoomThreshold"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(DISTANCEMODIFY*PGAutoZoomThreshold));
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    // if (!ISPARAGLIDER) wp->SetVisible(false); 
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoOrientScale"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(AutoOrientScale);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPGNumberOfGates"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGNumberOfGates);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOpenTimeH"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGOpenTimeH);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOpenTimeM"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGOpenTimeM);
    // if (PGNumberOfGates==0) wp->SetReadOnly(true);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGGateIntervalTime"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGGateIntervalTime);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGStartOut"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M343_ = "IN (Exit)" 
    dfe->addEnumText(gettext(TEXT("_@M343_")));
	// LKTOKEN  _@M498_ = "OUT (Enter)" 
    dfe->addEnumText(gettext(TEXT("_@M498_")));
    dfe->Set(PGStartOut);
    wp->RefreshDisplay();
  }
  
  #if OLDPROFILES
  if(GetFromRegistry(szRegistrySpeedUnitsValue,&SpeedUnit_Config)!=ERROR_SUCCESS) {
    SetToRegistry(szRegistrySpeedUnitsValue, SpeedUnit_Config);
    changed = true;
  } 
  #endif
  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsSpeed"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M667_ = "Statute" 
    dfe->addEnumText(gettext(TEXT("_@M667_")));
	// LKTOKEN  _@M455_ = "Nautical" 
    dfe->addEnumText(gettext(TEXT("_@M455_")));
	// LKTOKEN  _@M436_ = "Metric" 
    dfe->addEnumText(gettext(TEXT("_@M436_")));
    dfe->Set(SpeedUnit_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLatLon"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("DDMMSS"));
    dfe->addEnumText(TEXT("DDMMSS.ss"));
    dfe->addEnumText(TEXT("DDMM.mmm"));
    dfe->addEnumText(TEXT("DD.dddd"));
#ifdef NEWUTM
    dfe->addEnumText(TEXT("UTM"));
#endif
    dfe->Set(Units::CoordinateFormat);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  if(GetFromRegistry(szRegistryTaskSpeedUnitsValue,&TaskSpeedUnit_Config)!=ERROR_SUCCESS) {
    SetToRegistry(szRegistryTaskSpeedUnitsValue, TaskSpeedUnit_Config);
    changed = true;
  } 
  #endif
  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsTaskSpeed"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M667_ = "Statute" 
    dfe->addEnumText(gettext(TEXT("_@M667_")));
	// LKTOKEN  _@M455_ = "Nautical" 
    dfe->addEnumText(gettext(TEXT("_@M455_")));
	// LKTOKEN  _@M436_ = "Metric" 
    dfe->addEnumText(gettext(TEXT("_@M436_")));
    dfe->Set(TaskSpeedUnit_Config);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  if(GetFromRegistry(szRegistryDistanceUnitsValue,&DistanceUnit_Config)!=ERROR_SUCCESS) {
    SetToRegistry(szRegistryDistanceUnitsValue, DistanceUnit_Config);
    changed = true;
  } 
  #endif
  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsDistance"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M667_ = "Statute" 
    dfe->addEnumText(gettext(TEXT("_@M667_")));
	// LKTOKEN  _@M455_ = "Nautical" 
    dfe->addEnumText(gettext(TEXT("_@M455_")));
	// LKTOKEN  _@M436_ = "Metric" 
    dfe->addEnumText(gettext(TEXT("_@M436_")));
    dfe->Set(DistanceUnit_Config);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  if(GetFromRegistry(szRegistryAltitudeUnitsValue,&AltitudeUnit_Config)!=ERROR_SUCCESS) {
    SetToRegistry(szRegistryAltitudeUnitsValue, AltitudeUnit_Config);
    changed = true;
  } 
  #endif
  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsAltitude"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("feet")));
    dfe->addEnumText(gettext(TEXT("meters")));
    dfe->Set(AltitudeUnit_Config);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  if(GetFromRegistry(szRegistryLiftUnitsValue,&LiftUnit_Config)!=ERROR_SUCCESS) {
    SetToRegistry(szRegistryLiftUnitsValue, LiftUnit_Config);
    changed = true;
  } 
  #endif
  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLift"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("knots")));
    dfe->addEnumText(gettext(TEXT("m/s")));
    dfe->Set(LiftUnit_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTrailDrift"));
  if (wp) {
    wp->GetDataField()->Set(EnableTrailDrift_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpThermalLocator"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M959_ = "OFF" 
    dfe->addEnumText(gettext(TEXT("_@M959_")));
	// LKTOKEN  _@M427_ = "Mark center" 
    dfe->addEnumText(gettext(TEXT("_@M427_")));
	// LKTOKEN  _@M518_ = "Pan to center" 
    dfe->addEnumText(gettext(TEXT("_@M518_")));
    dfe->Set(EnableThermalLocator);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSetSystemTimeFromGPS"));
  if (wp) {
    wp->GetDataField()->Set(SetSystemTimeFromGPS);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDisableAutoLogger"));
  if (wp) {
    wp->GetDataField()->Set(!DisableAutoLogger);
    wp->RefreshDisplay();
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyMacCready"));
  if (wp) {
    wp->GetDataField()->Set(GlidePolar::SafetyMacCready*LIFTMODIFY);
    wp->GetDataField()->SetUnits(Units::GetVerticalSpeedName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTrail"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M499_ = "Off" 
    dfe->addEnumText(gettext(TEXT("_@M499_")));
	// LKTOKEN  _@M410_ = "Long" 
    dfe->addEnumText(gettext(TEXT("_@M410_")));
	// LKTOKEN  _@M612_ = "Short" 
    dfe->addEnumText(gettext(TEXT("_@M612_")));
	// LKTOKEN  _@M312_ = "Full" 
    dfe->addEnumText(gettext(TEXT("_@M312_")));
    dfe->Set(TrailActive_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKMaxLabels"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(LKMaxLabels);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBugs"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(BUGS_Config*100); // we show the value correctly
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMaxManoeuveringSpeed"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(SPEEDMODIFY*SAFTEYSPEED));
    wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWindCalcSpeed")); // 100112
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(SPEEDMODIFY*WindCalcSpeed));
    wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWindCalcTime")); // 100113
  if (wp) {
    wp->GetDataField()->Set(WindCalcTime);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBallastSecsToEmpty"));
  if (wp) {
    wp->GetDataField()->Set(BallastSecsToEmpty);
#if 0
    // this wont update visibility or readonly after changing polar, so we dont use it
    // because this is also saved in the aircraft file and be better to be visible all the times
    if (WEIGHTS[2]==0)
	wp->SetVisible(false);
    else
	wp->SetVisible(true);
#endif
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPilotFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_PILOT);
    dfe->ScanDirectoryTop(_T(LKD_CONF),tsuf);
    dfe->Set(0);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAircraftFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_AIRCRAFT);
    dfe->ScanDirectoryTop(_T(LKD_CONF),tsuf);
    dfe->Set(0);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  GetRegistryString(szRegistryPolarFile, szPolarFile, MAX_PATH);
  #endif
  if (_tcscmp(szPolarFile,_T(""))==0) 
    _tcscpy(temptext,_T("%LOCAL_PATH%\\\\_Polars\\Default.plr"));
  else
    _tcscpy(temptext,szPolarFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpPolarFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_POLARS);
    dfe->ScanDirectoryTop(_T(LKD_POLARS),tsuf); //091101
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  GetRegistryString(szRegistryAirspaceFile, szAirspaceFile, MAX_PATH);
  #endif
  _tcscpy(temptext,szAirspaceFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_AIRSPACES);
    dfe->ScanDirectoryTop(_T(LKD_AIRSPACES),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  GetRegistryString(szRegistryAdditionalAirspaceFile, szAdditionalAirspaceFile, MAX_PATH);
  #endif
  _tcscpy(temptext,szAdditionalAirspaceFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_AIRSPACES);
    dfe->ScanDirectoryTop(_T(LKD_AIRSPACES),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  GetRegistryString(szRegistryWayPointFile, szWaypointFile, MAX_PATH);
  #endif
  _tcscpy(temptext,szWaypointFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_WP_WINPILOT);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%S"),LKS_WP_XCSOAR);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%S"),LKS_WP_CUP);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%S"),LKS_WP_COMPE);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  GetRegistryString(szRegistryAdditionalWayPointFile, szAdditionalWaypointFile, MAX_PATH);
  #endif
  _tcscpy(temptext,szAdditionalWaypointFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_WP_WINPILOT);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%S"),LKS_WP_XCSOAR);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%S"),LKS_WP_CUP);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%S"),LKS_WP_COMPE);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  GetRegistryString(szRegistryMapFile, szMapFile, MAX_PATH);
  #endif
  _tcscpy(temptext,szMapFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpMapFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_MAPS);
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
    _stprintf(tsuf,_T("*%S"),XCS_MAPS);
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  GetRegistryString(szRegistryTerrainFile, szTerrainFile, MAX_PATH);
  #endif
  _tcscpy(temptext,szTerrainFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_TERRAINDEM);
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
#if LKMTERRAIN
    _stprintf(tsuf,_T("*%S"),LKS_TERRAINDAT);
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
#endif
#if JP2000
    _stprintf(tsuf,_T("*%S"),LKS_TERRAINJP2);
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
#endif
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  GetRegistryString(szRegistryAirfieldFile, szAirfieldFile, MAX_PATH);
  #endif
  _tcscpy(temptext,szAirfieldFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAirfieldFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_AIRFIELDS);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  GetRegistryString(szRegistryLanguageFile, szLanguageFile, MAX_PATH);
  #endif
  _tcscpy(temptext,szLanguageFile);
  if (_tcslen(temptext)==0) {
	_tcscpy(temptext,_T("%LOCAL_PATH%\\\\_Language\\ENGLISH.LNG"));
  }
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpLanguageFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_LANGUAGE);
    dfe->ScanDirectoryTop(_T(LKD_LANGUAGE),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  #if OLDPROFILES
  GetRegistryString(szRegistryInputFile, szInputFile, MAX_PATH);
  #endif
  _tcscpy(temptext,szInputFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpInputFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_INPUT);
    dfe->ScanDirectoryTop(_T(LKD_CONF),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

#ifdef PNA
// VENTA-ADDON Model change config menu 11
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxModel"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("Generic"));
    dfe->addEnumText(TEXT("HP31x"));
    dfe->addEnumText(TEXT("MedionP5"));
    dfe->addEnumText(TEXT("MIO"));
    dfe->addEnumText(TEXT("Nokia500")); // VENTA3
    dfe->addEnumText(TEXT("PN6000"));
    dfe->addEnumText(TEXT("Navigon"));
    dfe->addEnumText(TEXT("Holux FunTrek GM-130"));
    dfe->addEnumText(TEXT("Medion S3747 / Royaltek BV-3200"));
	
	int iTmp;
	switch (GlobalModelType) {
		case MODELTYPE_PNA_PNA:
				iTmp=(InfoBoxModelAppearance_t)apImPnaGeneric;
				break;
		case MODELTYPE_PNA_HP31X:
				iTmp=(InfoBoxModelAppearance_t)apImPnaHp31x;
				break;
		case MODELTYPE_PNA_PN6000:
				iTmp=(InfoBoxModelAppearance_t)apImPnaPn6000;
				break;
		case MODELTYPE_PNA_MIO:
				iTmp=(InfoBoxModelAppearance_t)apImPnaMio;
				break;
		case MODELTYPE_PNA_NOKIA_500:
				iTmp=(InfoBoxModelAppearance_t)apImPnaNokia500;
				break;
		case MODELTYPE_PNA_MEDION_P5:
				iTmp=(InfoBoxModelAppearance_t)apImPnaMedionP5;
				break;
		case MODELTYPE_PNA_NAVIGON:
				iTmp=(InfoBoxModelAppearance_t)apImPnaNavigon;
				break;
		case MODELTYPE_PNA_FUNTREK:
				iTmp=(InfoBoxModelAppearance_t)apImPnaFuntrek;
				break;
		case MODELTYPE_PNA_ROYALTEK3200:
				iTmp=(InfoBoxModelAppearance_t)apImPnaRoyaltek3200;
				break;
		default:
				iTmp=(InfoBoxModelAppearance_t)apImPnaGeneric;
				break;
	}

    dfe->Set(iTmp);
    wp->RefreshDisplay();
  }
#else
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxModel"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
// VENTA2- PC model
#if (WINDOWSPC>0)
	// LKTOKEN  _@M511_ = "PC/normal" 
    dfe->addEnumText(gettext(TEXT("_@M511_")));
    wp->SetVisible(true); // no more gaps in menus
#else
	// LKTOKEN  _@M512_ = "PDA/normal" 
    dfe->addEnumText(gettext(TEXT("_@M512_")));
    wp->SetVisible(true);
#endif
        dfe->Set(0);
    wp->RefreshDisplay();
  }
#endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpAircraftCategory"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M328_ = "Glider" 
    dfe->addEnumText(gettext(TEXT("_@M328_")));
	// LKTOKEN  _@M520_ = "Paraglider/Delta" 
    dfe->addEnumText(gettext(TEXT("_@M520_")));
	// LKTOKEN  _@M163_ = "Car" 
    dfe->addEnumText(gettext(TEXT("_@M163_")));
	// LKTOKEN  _@M313_ = "GA Aircraft" 
    dfe->addEnumText(gettext(TEXT("_@M313_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(AircraftCategory);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLook8000"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField(); 
     dfe->addEnumText(TEXT("(Reserved)"));
	// LKTOKEN  _@M475_ = "No overlay" 
     dfe->addEnumText(gettext(TEXT("_@M475_"))); // 091115
	// LKTOKEN  _@M333_ = "Half overlay" 
     dfe->addEnumText(gettext(TEXT("_@M333_")));
	// LKTOKEN  _@M311_ = "Full overlay" 
    	dfe->addEnumText(gettext(TEXT("_@M311_")));
     dfe = (DataFieldEnum*)wp->GetDataField(); // see above
     dfe->Set(Look8000);
    wp->RefreshDisplay();
  }

#if (0)
  wp = (WndProperty*)wf->FindByName(TEXT("prpAltArrivMode"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M768_ = "Use MacCready" 
     dfe->addEnumText(gettext(TEXT("_@M768_")));
	// LKTOKEN  _@M90_ = "Always use MC=0" 
     dfe->addEnumText(gettext(TEXT("_@M90_")));
	// LKTOKEN  _@M91_ = "Always use safety MC" 
     dfe->addEnumText(gettext(TEXT("_@M91_")));
	// LKTOKEN  _@M769_ = "Use aver.efficiency" 
     dfe->addEnumText(gettext(TEXT("_@M769_")));
     dfe = (DataFieldEnum*)wp->GetDataField();
     dfe->Set(AltArrivMode);
    wp->RefreshDisplay();
  }
#endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpCheckSum"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(CheckSum);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIphoneGestures"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M364_ = "Inverted" 
    dfe->addEnumText(gettext(TEXT("_@M364_")));
	// LKTOKEN  _@M480_ = "Normal" 
    dfe->addEnumText(gettext(TEXT("_@M480_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(IphoneGestures);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPollingMode"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M480_ = "Normal" 
    dfe->addEnumText(gettext(TEXT("_@M480_")));
	// LKTOKEN  _@M529_ = "Polling" 
    dfe->addEnumText(gettext(TEXT("_@M529_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(PollingMode);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKVarioBar"));
  if (wp) {
    DataFieldEnum* dfe;
    TCHAR newtoken[150];
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M782_ = "Vario rainbow" 
    dfe->addEnumText(gettext(TEXT("_@M782_")));
	// LKTOKEN  _@M780_ = "Vario black" 
    dfe->addEnumText(gettext(TEXT("_@M780_")));
	// LKTOKEN  _@M783_ = "Vario red+blue" 
    dfe->addEnumText(gettext(TEXT("_@M783_")));
	// LKTOKEN  _@M781_ = "Vario green+red" 
    dfe->addEnumText(gettext(TEXT("_@M781_")));

    _stprintf(newtoken,_T("%s %s"),gettext(TEXT("_@M953_")),gettext(TEXT("_@M782_")) );
    dfe->addEnumText(newtoken);
    _stprintf(newtoken,_T("%s %s"),gettext(TEXT("_@M953_")),gettext(TEXT("_@M780_")) );
    dfe->addEnumText(newtoken);
    _stprintf(newtoken,_T("%s %s"),gettext(TEXT("_@M953_")),gettext(TEXT("_@M783_")) );
    dfe->addEnumText(newtoken);
    _stprintf(newtoken,_T("%s %s"),gettext(TEXT("_@M953_")),gettext(TEXT("_@M781_")) );
    dfe->addEnumText(newtoken);

    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(LKVarioBar);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKVarioVal"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("_@M1425_"))); // vario in thermal and cruise
    dfe->addEnumText(gettext(TEXT("_@M1426_")));  // vario in thermal, netto in cruise
    dfe->addEnumText(gettext(TEXT("_@M1427_")));  // vario in thermal, sollfahr in cruise
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(LKVarioVal);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpHideUnits"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(HideUnits);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDeclutterMode"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M414_ = "Low" 
    dfe->addEnumText(gettext(TEXT("_@M414_")));
	// LKTOKEN  _@M433_ = "Medium" 
    dfe->addEnumText(gettext(TEXT("_@M433_")));
	// LKTOKEN  _@M339_ = "High" 
    dfe->addEnumText(gettext(TEXT("_@M339_")));
	// LKTOKEN  _@M786_ = "Very High" 
    dfe->addEnumText(gettext(TEXT("_@M786_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(DeclutterMode);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpActiveMap")); // 100318
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(ActiveMap_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBestWarning")); // 091122
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(BestWarning);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpUseTotalEnergy"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("_@M239_"))); // disabled
    dfe->addEnumText(gettext(TEXT("_@M259_"))); // enabled
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(UseTotalEnergy_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpThermalBar")); // 091122
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(ThermalBar);
    wp->RefreshDisplay();
  }

  // This is updated also from DoLook8000ModeChange function
  // These are only the initial startup values
  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayClock"));
  if (wp) {
    if (Look8000==lxcStandard || !ScreenLandscape) {
	OverlayClock=0;	// Disable clock
    	wp->SetReadOnly(true);
    } 
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    // LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
    // LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(OverlayClock);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMcOverlay")); // 091122
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(McOverlay);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOptimizeRoute"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(PGOptimizeRoute);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTrackBar")); // 091122
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(TrackBar);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOutlinedTp"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M78_ = "All black" 
    dfe->addEnumText(gettext(TEXT("_@M78_")));
	// LKTOKEN  _@M778_ = "Values white" 
    dfe->addEnumText(gettext(TEXT("_@M778_")));
	// LKTOKEN  _@M81_ = "All white" 
    dfe->addEnumText(gettext(TEXT("_@M81_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(OutlinedTp);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTpFilter"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M473_ = "No landables" 
    dfe->addEnumText(gettext(TEXT("_@M473_")));
	// LKTOKEN  _@M80_ = "All waypoints" 
    dfe->addEnumText(gettext(TEXT("_@M80_")));
	// LKTOKEN  _@M211_ = "DAT Turnpoints" 
    dfe->addEnumText(gettext(TEXT("_@M211_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(TpFilter);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverColor"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M816_ = "White" 
    dfe->addEnumText(gettext(TEXT("_@M816_")));
	// LKTOKEN  _@M144_ = "Black" 
    dfe->addEnumText(gettext(TEXT("_@M144_")));
	// LKTOKEN  _@M218_ = "DarkBlue" 
    dfe->addEnumText(gettext(TEXT("_@M218_")));
	// LKTOKEN  _@M825_ = "Yellow" 
    dfe->addEnumText(gettext(TEXT("_@M825_")));
	// LKTOKEN  _@M331_ = "Green" 
    dfe->addEnumText(gettext(TEXT("_@M331_")));
	// LKTOKEN  _@M505_ = "Orange" 
    dfe->addEnumText(gettext(TEXT("_@M505_")));
	// LKTOKEN  _@M209_ = "Cyan" 
    dfe->addEnumText(gettext(TEXT("_@M209_")));
	// LKTOKEN  _@M417_ = "Magenta" 
    dfe->addEnumText(gettext(TEXT("_@M417_")));
	// LKTOKEN  _@M332_ = "Grey" 
    dfe->addEnumText(gettext(TEXT("_@M332_")));
	// LKTOKEN  _@M215_ = "Dark Grey" 
    dfe->addEnumText(gettext(TEXT("_@M215_")));
	// LKTOKEN  _@M216_ = "Dark White" 
    dfe->addEnumText(gettext(TEXT("_@M216_")));
	// LKTOKEN  _@M92_ = "Amber" 
    dfe->addEnumText(gettext(TEXT("_@M92_")));
	// LKTOKEN  _@M391_ = "Light Green" 
    dfe->addEnumText(gettext(TEXT("_@M391_")));
	// LKTOKEN  _@M522_ = "Petrol" 
    dfe->addEnumText(gettext(TEXT("_@M522_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(OverColor);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMapBox"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M763_ = "Unboxed, no units" 
    dfe->addEnumText(gettext(TEXT("_@M763_")));
	// LKTOKEN  _@M764_ = "Unboxed, with units" 
    dfe->addEnumText(gettext(TEXT("_@M764_")));
	// LKTOKEN  _@M152_ = "Boxed, no units" 
    dfe->addEnumText(gettext(TEXT("_@M152_")));
	// LKTOKEN  _@M153_ = "Boxed, with units" 
    dfe->addEnumText(gettext(TEXT("_@M153_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(MapBox);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlaySize"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M842_ = "Big font " 
    dfe->addEnumText(gettext(TEXT("_@M842_")));
	// LKTOKEN  _@M843_ = "Small font " 
    dfe->addEnumText(gettext(TEXT("_@M843_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(OverlaySize);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGlideBarMode"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M461_ = "Next turnpoint" 
    dfe->addEnumText(gettext(TEXT("_@M461_")));
	// LKTOKEN  _@M299_ = "Finish" 
    dfe->addEnumText(gettext(TEXT("_@M299_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(GlideBarMode);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpArrivalValue"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M98_ = "ArrivalAltitude" 
    dfe->addEnumText(gettext(TEXT("_@M98_")));
	// LKTOKEN  _@M254_ = "EfficiencyReq" 
    dfe->addEnumText(gettext(TEXT("_@M254_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(ArrivalValue);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpNewMapDeclutter"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M414_ = "Low" 
    dfe->addEnumText(gettext(TEXT("_@M414_")));
	// LKTOKEN  _@M339_ = "High" 
    dfe->addEnumText(gettext(TEXT("_@M339_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(NewMapDeclutter);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAverEffTime"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(MsgToken(1775)); // 3 sec
    dfe->addEnumText(MsgToken(1776)); // 5 sec
    dfe->addEnumText(MsgToken(1777)); // 10 sec
	// LKTOKEN  _@M17_ = "15 seconds" 
    dfe->addEnumText(gettext(TEXT("_@M17_")));
	// LKTOKEN  _@M30_ = "30 seconds" 
    dfe->addEnumText(gettext(TEXT("_@M30_")));
    dfe->addEnumText(MsgToken(1778)); // 45 sec
	// LKTOKEN  _@M35_ = "60 seconds" 
    dfe->addEnumText(gettext(TEXT("_@M35_")));
	// LKTOKEN  _@M39_ = "90 seconds" 
    dfe->addEnumText(gettext(TEXT("_@M39_")));
	// LKTOKEN  _@M23_ = "2 minutes" 
    dfe->addEnumText(gettext(TEXT("_@M23_")));
	// LKTOKEN  _@M29_ = "3 minutes" 
    dfe->addEnumText(gettext(TEXT("_@M29_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(AverEffTime);
    wp->RefreshDisplay();
 }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBgMapcolor"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M816_ = "White" 
    dfe->addEnumText(gettext(TEXT("_@M816_")));
	// LKTOKEN  _@M392_ = "Light grey" 
    dfe->addEnumText(gettext(TEXT("_@M392_")));
	// LKTOKEN  _@M374_ = "LCD green" 
    dfe->addEnumText(gettext(TEXT("_@M374_")));
	// LKTOKEN  _@M373_ = "LCD darkgreen" 
    dfe->addEnumText(gettext(TEXT("_@M373_")));
	// LKTOKEN  _@M332_ = "Grey" 
    dfe->addEnumText(gettext(TEXT("_@M332_")));
	// LKTOKEN  _@M147_ = "Blue lake" 
    dfe->addEnumText(gettext(TEXT("_@M147_")));
	// LKTOKEN  _@M256_ = "Emerald green" 
    dfe->addEnumText(gettext(TEXT("_@M256_")));
	// LKTOKEN  _@M217_ = "Dark grey" 
    dfe->addEnumText(gettext(TEXT("_@M217_")));
	// LKTOKEN  _@M567_ = "Rifle grey" 
    dfe->addEnumText(gettext(TEXT("_@M567_")));
	// LKTOKEN  _@M144_ = "Black" 
    dfe->addEnumText(gettext(TEXT("_@M144_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(BgMapColor_Config);
    wp->RefreshDisplay();
 }

// Fonts
  wp = (WndProperty*)wf->FindByName(TEXT("prpUseCustomFonts"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    dfb->Set(UseCustomFonts);
    ShowFontEditButtons(dfb->GetAsBoolean());
    wp->RefreshDisplay();
    RefreshFonts();
  }
  FontRegistryChanged=false;


// end fonts


  wp = (WndProperty*)wf->FindByName(TEXT("prpAppIndLandable"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M822_ = "Winpilot" 
    dfe->addEnumText(gettext(TEXT("_@M822_")));
	// LKTOKEN  _@M87_ = "Alternate" 
    dfe->addEnumText(gettext(TEXT("_@M87_")));
    dfe->Set(Appearance.IndLandable);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInverseInfoBox"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M958_ = "ON" 
    dfe->addEnumText(gettext(TEXT("_@M958_")));
	// LKTOKEN  _@M959_ = "OFF" 
    dfe->addEnumText(gettext(TEXT("_@M959_")));
    dfe->Set(InverseInfoBox_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppDefaultMapWidth"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Appearance.DefaultMapWidth);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGliderScreenPosition"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(MapWindow::GliderScreenPosition);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainContrast"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(TerrainContrast*100/255));
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainBrightness"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(TerrainBrightness*100/255));
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainRamp"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M413_ = "Low lands" 
    dfe->addEnumText(gettext(TEXT("_@M413_")));
	// LKTOKEN  _@M439_ = "Mountainous" 
    dfe->addEnumText(gettext(TEXT("_@M439_")));
    dfe->addEnumText(TEXT("Imhof 7"));
    dfe->addEnumText(TEXT("Imhof 4"));
    dfe->addEnumText(TEXT("Imhof 12"));
    dfe->addEnumText(TEXT("Imhof Atlas"));
    dfe->addEnumText(TEXT("ICAO")); 
	// LKTOKEN  _@M377_ = "LKoogle lowlands" 
    dfe->addEnumText(gettext(TEXT("_@M377_"))); 
	// LKTOKEN  _@M378_ = "LKoogle mountains" 
    dfe->addEnumText(gettext(TEXT("_@M378_")));
	// LKTOKEN  _@M412_ = "Low Alps" 
    dfe->addEnumText(gettext(TEXT("_@M412_")));
	// LKTOKEN  _@M338_ = "High Alps" 
    dfe->addEnumText(gettext(TEXT("_@M338_")));
    dfe->addEnumText(TEXT("YouSee"));
	// LKTOKEN  _@M340_ = "HighContrast" 
    dfe->addEnumText(gettext(TEXT("_@M340_")));
    dfe->addEnumText(TEXT("GA Relative"));
    dfe->Set(TerrainRamp_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoBacklight")); // VENTA4
  if (wp) {
    wp->SetVisible(true); // 091115 changed to true
#ifdef PNA
    if (GlobalModelType == MODELTYPE_PNA_HP31X )
    	wp->SetVisible(true);
#endif
    wp->GetDataField()->Set(EnableAutoBacklight);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoSoundVolume")); // VENTA4
  if (wp) {
    wp->SetVisible(true); // 091115 changed to true
#if ( !defined(WINDOWSPC) || WINDOWSPC==0 )
    	wp->SetVisible(true);
#endif
    wp->GetDataField()->Set(EnableAutoSoundVolume);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishLine"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M210_ = "Cylinder" 
    dfe->addEnumText(gettext(TEXT("_@M210_")));
	// LKTOKEN  _@M393_ = "Line" 
    dfe->addEnumText(gettext(TEXT("_@M393_")));
	// LKTOKEN  _@M274_ = "FAI Sector" 
    dfe->addEnumText(gettext(TEXT("_@M274_")));
    dfe->Set(FinishLine);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(lround(FinishRadius*DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartLine"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M210_ = "Cylinder" 
    dfe->addEnumText(gettext(TEXT("_@M210_")));
	// LKTOKEN  _@M393_ = "Line" 
    dfe->addEnumText(gettext(TEXT("_@M393_")));
	// LKTOKEN  _@M274_ = "FAI Sector" 
    dfe->addEnumText(gettext(TEXT("_@M274_")));
    dfe->Set(StartLine);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(lround(StartRadius*DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFAISector"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M210_ = "Cylinder" 
    dfe->addEnumText(gettext(TEXT("_@M210_")));
	// LKTOKEN  _@M274_ = "FAI Sector" 
    dfe->addEnumText(gettext(TEXT("_@M274_")));
    dfe->addEnumText(gettext(TEXT("DAe 0.5/10")));
    dfe->Set(SectorType);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskSectorRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(lround(SectorRadius*DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoAdvance"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M418_ = "Manual" 
    dfe->addEnumText(gettext(TEXT("_@M418_")));
    dfe->addEnumText(gettext(TEXT("Auto")));
	// LKTOKEN  _@M97_ = "Arm" 
    dfe->addEnumText(gettext(TEXT("_@M97_")));
	// LKTOKEN  _@M96_ = "Arm start" 
    dfe->addEnumText(gettext(TEXT("_@M96_")));
    dfe->Set(AutoAdvance_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLoggerTimeStepCruise"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(LoggerTimeStepCruise);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLoggerTimeStepCircling"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(LoggerTimeStepCircling);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSnailWidthScale"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(MapWindow::SnailWidthScale);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGPSAltitudeOffset"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(GPSAltitudeOffset*ALTITUDEMODIFY/1000)); // 100429
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmMaxAltitude1"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(AlarmMaxAltitude1*ALTITUDEMODIFY/1000));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmMaxAltitude2"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(AlarmMaxAltitude2*ALTITUDEMODIFY/1000));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmMaxAltitude3"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(AlarmMaxAltitude3*ALTITUDEMODIFY/1000));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }



  wp = (WndProperty*)wf->FindByName(TEXT("prpUseGeoidSeparation"));
  if (wp) {
    wp->GetDataField()->Set(UseGeoidSeparation);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPressureHg"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("hPa"));
    dfe->addEnumText(TEXT("inHg"));
    dfe->Set(PressureHg);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEngineeringMenu")); 
  if (wp) {
    wp->GetDataField()->Set(EngineeringMenu);
    wp->RefreshDisplay();
  }

  for (i=0; i<4; i++) {
    for (int j=0; j<8; j++) {
      SetInfoBoxSelector(j, i);
    }
  }
}




void dlgConfigurationShowModal(void){

  WndProperty *wp;

  StartHourglassCursor(); 

  if (!ScreenLandscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgConfiguration_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_CONFIGURATION_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgConfiguration.xml"));
    wf = dlgLoadFromXML(CallBackTable,                        
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_CONFIGURATION"));
  }

  if (!wf) return;
  
  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  wConfig1    = ((WndFrame *)wf->FindByName(TEXT("frmSite")));
  wConfig2    = ((WndFrame *)wf->FindByName(TEXT("frmAirspace")));
  wConfig3    = ((WndFrame *)wf->FindByName(TEXT("frmDisplay")));
  wConfig4    = ((WndFrame *)wf->FindByName(TEXT("frmTerrain")));
  wConfig5    = ((WndFrame *)wf->FindByName(TEXT("frmFinalGlide")));
  wConfig6    = ((WndFrame *)wf->FindByName(TEXT("frmSafety")));
  wConfig7    = ((WndFrame *)wf->FindByName(TEXT("frmPolar")));
  wConfig8    = ((WndFrame *)wf->FindByName(TEXT("frmComm")));
  wConfig9    = ((WndFrame *)wf->FindByName(TEXT("frmUnits")));
  wConfig10    = ((WndFrame *)wf->FindByName(TEXT("frmInterface")));
  wConfig11    = ((WndFrame *)wf->FindByName(TEXT("frmAppearance")));
  wConfig12    = ((WndFrame *)wf->FindByName(TEXT("frmFonts")));
  wConfig13    = ((WndFrame *)wf->FindByName(TEXT("frmVarioAppearance")));
  wConfig14    = ((WndFrame *)wf->FindByName(TEXT("frmTask")));
  wConfig15    = ((WndFrame *)wf->FindByName(TEXT("frmAlarms")));
  wConfig16    = ((WndFrame *)wf->FindByName(TEXT("frmInfoBoxCircling")));
  wConfig17    = ((WndFrame *)wf->FindByName(TEXT("frmInfoBoxCruise")));
  wConfig18    = ((WndFrame *)wf->FindByName(TEXT("frmInfoBoxFinalGlide")));
  wConfig19    = ((WndFrame *)wf->FindByName(TEXT("frmInfoBoxAuxiliary")));
  wConfig20    = ((WndFrame *)wf->FindByName(TEXT("frmLogger")));
  wConfig21    = ((WndFrame *)wf->FindByName(TEXT("frmWaypointEdit")));
  wConfig22    = ((WndFrame *)wf->FindByName(TEXT("frmSpecials1")));
  wConfig23    = ((WndFrame *)wf->FindByName(TEXT("frmSpecials2")));
  wConfig24    = ((WndFrame *)wf->FindByName(TEXT("frmEngineering1")));
  wConfig25    = ((WndFrame *)wf->FindByName(TEXT("frmEngineering2")));
  //wConfig26    = ((WndFrame *)wf->FindByName(TEXT("frmEngineering3")));
  //wConfig27    = ((WndFrame *)wf->FindByName(TEXT("frmEngineering4")));

  wf->FilterAdvanced(1);

  for (int item=0; item<10; item++) {
    cpyInfoBox[item] = -1;
  }

  setVariables();

  TCHAR deviceName1[MAX_PATH];
  TCHAR deviceName2[MAX_PATH];
  ReadDeviceSettings(0, deviceName1);
  ReadDeviceSettings(1, deviceName2);
  UpdateDeviceSetupButton(0, deviceName1);
  UpdateDeviceSetupButton(1, deviceName2);

  NextPage(0); // just to turn proper pages on/off

  changed = false;
  taskchanged = false;
  requirerestart = false;
  utcchanged = false;
  waypointneedsave = false;

  StopHourglassCursor();
  wf->ShowModal();

  wp = (WndProperty*)wf->FindByName(TEXT("prpDisableAutoLogger"));
  if (wp) {
    if (!DisableAutoLogger
	!= wp->GetDataField()->GetAsBoolean()) {
      DisableAutoLogger = 
	!(wp->GetDataField()->GetAsBoolean());
#if OLDPROFILES
      SetToRegistry(szRegistryDisableAutoLogger, DisableAutoLogger);
#endif
      changed = true;
    }
  }

  double val;

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyMacCready"));
  if (wp) {
    val = wp->GetDataField()->GetAsFloat()/LIFTMODIFY;
    if (GlidePolar::SafetyMacCready != val) {
      GlidePolar::SafetyMacCready = val;
#if OLDPROFILES
      // saved *10 by LKProfileSave
      SetToRegistry(szRegistrySafetyMacCready, iround(GlidePolar::SafetyMacCready*10));
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSetSystemTimeFromGPS"));
  if (wp) {
    if (SetSystemTimeFromGPS != wp->GetDataField()->GetAsBoolean()) {
      SetSystemTimeFromGPS = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistrySetSystemTimeFromGPS, SetSystemTimeFromGPS);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTrailDrift"));
  if (wp) {
    if (EnableTrailDrift_Config != wp->GetDataField()->GetAsBoolean()) {
      EnableTrailDrift_Config = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistryTrailDrift, MapWindow::EnableTrailDrift);
#endif
      changed = true;
      MapWindow::EnableTrailDrift = EnableTrailDrift_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpThermalLocator"));
  if (wp) {
    if (EnableThermalLocator != wp->GetDataField()->GetAsInteger()) {
      EnableThermalLocator = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryThermalLocator, EnableThermalLocator);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTrail"));
  if (wp) {
    if (TrailActive_Config != wp->GetDataField()->GetAsInteger()) {
      TrailActive_Config = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistrySnailTrail, TrailActive_Config);
#endif
      changed = true;
      TrailActive = TrailActive_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKMaxLabels"));
  if (wp) {
    if (LKMaxLabels != wp->GetDataField()->GetAsInteger()) {
      LKMaxLabels = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryLKMaxLabels, LKMaxLabels);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBugs"));
  if (wp) {
    if (BUGS_Config != wp->GetDataField()->GetAsFloat()/100.0) {
      BUGS_Config = wp->GetDataField()->GetAsFloat()/100.0;
      changed = true;
      BUGS=BUGS_Config;
      requirerestart=true;
    }
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpPGCruiseZoom"));
  if (wp) {
    if ( PGCruiseZoom != wp->GetDataField()->GetAsInteger()) {
      PGCruiseZoom = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryPGCruiseZoom, PGCruiseZoom);
#endif
      changed = true;
      MapWindow::zoom.Reset();
        requirerestart=true;
    }
  }

  int ival;

  wp = (WndProperty*)wf->FindByName(TEXT("prpPGAutoZoomThreshold"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsFloat()/DISTANCEMODIFY);
    if ((int)PGAutoZoomThreshold != ival) {
      PGAutoZoomThreshold = ival;
#if OLDPROFILES
      SetToRegistry(szRegistryPGAutoZoomThreshold, PGAutoZoomThreshold);
#endif
      changed = true;
    }
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGClimbZoom"));
  if (wp) {
    if ( PGClimbZoom != wp->GetDataField()->GetAsInteger()) {
      PGClimbZoom = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryPGClimbZoom, PGClimbZoom);
#endif
      changed = true;
      MapWindow::zoom.Reset();
        requirerestart=true; 
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoOrientScale"));
  if (wp) {
    if ( AutoOrientScale != wp->GetDataField()->GetAsInteger()) {
      AutoOrientScale = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryAutoOrientScale, AutoOrientScale);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPGNumberOfGates"));
  if (wp) {
    if ( PGNumberOfGates != wp->GetDataField()->GetAsInteger()) {
      PGNumberOfGates = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryPGNumberOfGates, PGNumberOfGates);
#endif
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOpenTimeH"));
  if (wp) {
    if ( PGOpenTimeH != wp->GetDataField()->GetAsInteger()) {
      PGOpenTimeH = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryPGOpenTimeH, PGOpenTimeH);
#endif
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOpenTimeM"));
  if (wp) {
    if ( PGOpenTimeM != wp->GetDataField()->GetAsInteger()) {
      PGOpenTimeM = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryPGOpenTimeM, PGOpenTimeM);
#endif
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGGateIntervalTime"));
  if (wp) {
    if ( PGGateIntervalTime != wp->GetDataField()->GetAsInteger()) {
      PGGateIntervalTime = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryPGGateIntervalTime, PGGateIntervalTime);
#endif
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGStartOut"));
  if (wp) {
    if ( PGStartOut != wp->GetDataField()->GetAsInteger()) {
      PGStartOut = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryPGStartOut, PGStartOut);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceDisplay"));
  if (wp) {
    if (AltitudeMode_Config != wp->GetDataField()->GetAsInteger()) {
      AltitudeMode_Config = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryAltMode, AltitudeMode);
#endif
      AltitudeMode = AltitudeMode_Config;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeMode"));
  if (wp) {
    if (SafetyAltitudeMode != wp->GetDataField()->GetAsInteger()) {
      SafetyAltitudeMode = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistrySafetyAltitudeMode, SafetyAltitudeMode);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLockSettingsInFlight"));
  if (wp) {
    if (LockSettingsInFlight != 
	wp->GetDataField()->GetAsBoolean()) {
      LockSettingsInFlight = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistryLockSettingsInFlight, LockSettingsInFlight);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLoggerShortName"));
  if (wp) {
    if (LoggerShortName != 
	wp->GetDataField()->GetAsBoolean()) {
      LoggerShortName = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistryLoggerShort, LoggerShortName);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableFLARMMap"));
  if (wp) {
    if ((int)EnableFLARMMap != 
	wp->GetDataField()->GetAsInteger()) {
      EnableFLARMMap = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryEnableFLARMMap, EnableFLARMMap);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDebounceTimeout"));
  if (wp) {
    if (debounceTimeout != wp->GetDataField()->GetAsInteger()) {
      debounceTimeout = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryDebounceTimeout, debounceTimeout);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceFillType"));
  if (wp) {
    if (MapWindow::GetAirSpaceFillType() != wp->GetDataField()->GetAsInteger()) {
      MapWindow::SetAirSpaceFillType((MapWindow::EAirspaceFillType)wp->GetDataField()->GetAsInteger());
#if OLDPROFILES
      SetToRegistry(szRegistryAirspaceFillType, MapWindow::GetAirSpaceFillType());
#endif
      changed = true;
    }
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceOpacity"));
  if (wp) {
    if (MapWindow::GetAirSpaceOpacity() != wp->GetDataField()->GetAsInteger()*10) {
      MapWindow::SetAirSpaceOpacity(wp->GetDataField()->GetAsInteger() * 10);
#if OLDPROFILES
      SetToRegistry(szRegistryAirspaceOpacity, MapWindow::GetAirSpaceOpacity());
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBarOpacity"));
  if (wp) {
    if (BarOpacity != wp->GetDataField()->GetAsInteger()*5 ) {
	BarOpacity= wp->GetDataField()->GetAsInteger() * 5;
#if OLDPROFILES
      SetToRegistry(szRegistryBarOpacity, BarOpacity);
#endif
      changed = true;
    }
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpFontRenderer"));
  if (wp) {
    if (FontRenderer != wp->GetDataField()->GetAsInteger() ) 
    {
	    FontRenderer = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryFontRenderer, FontRenderer);
#endif
      requirerestart = true;
      changed = true;
    }
  }
  
 wp = (WndProperty*)wf->FindByName(TEXT("prpWarningMessageRepeatTime"));
  if (wp) {
    if (AirspaceWarningRepeatTime != (wp->GetDataField()->GetAsInteger()*60)) {
      AirspaceWarningRepeatTime = wp->GetDataField()->GetAsInteger()*60;
#if OLDPROFILES
      SetToRegistry(szRegistryAirspaceWarningRepeatTime, (DWORD)AirspaceWarningRepeatTime);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceOutline"));
  if (wp) {
    if (MapWindow::bAirspaceBlackOutline != wp->GetDataField()->GetAsBoolean()) {
      MapWindow::bAirspaceBlackOutline = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistryAirspaceBlackOutline,
		    MapWindow::bAirspaceBlackOutline);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoZoom"));
  if (wp) {
    if (AutoZoom_Config != 
	wp->GetDataField()->GetAsBoolean()) {
      AutoZoom_Config = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistryAutoZoom,
		    MapWindow::zoom.AutoZoom());
#endif
      changed = true;
      MapWindow::zoom.AutoZoom(AutoZoom_Config);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUTCOffset"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsFloat()*3600.0);
    if ((UTCOffset != ival)||(utcchanged)) {
      UTCOffset = ival;

#if OLDPROFILES
      // have to do this because registry variables can't be negative!
      int lival = UTCOffset;
      if (lival<0) { lival+= 24*3600; }
      SetToRegistry(szRegistryUTCOffset, lival);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpClipAltitude"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY);
    if (ClipAltitude != ival) {
      ClipAltitude = ival;
#if OLDPROFILES
      SetToRegistry(szRegistryClipAlt,ClipAltitude);  // fixed 20060430/sgi was szRegistryAltMode
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAltWarningMargin"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY);
    if (AltWarningMargin != ival) {
      AltWarningMargin = ival;
#if OLDPROFILES
      SetToRegistry(szRegistryAltMargin,AltWarningMargin);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningTime"));
  if (wp) {
    if (WarningTime != wp->GetDataField()->GetAsInteger()) {
      WarningTime = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryWarningTime,(DWORD)WarningTime);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAcknowledgementTime"));
  if (wp) {
    if (AcknowledgementTime != wp->GetDataField()->GetAsInteger()) {
      AcknowledgementTime = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryAcknowledgementTime,
		    (DWORD)AcknowledgementTime);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointLabels"));
  if (wp) {
    if (DisplayTextType != wp->GetDataField()->GetAsInteger()) {
      DisplayTextType = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryDisplayText,DisplayTextType);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableTerrain"));
  if (wp) {
    if (EnableTerrain_Config != wp->GetDataField()->GetAsBoolean()) {
      EnableTerrain_Config = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistryDrawTerrain, EnableTerrain_Config);
#endif
      changed = true;
      EnableTerrain=EnableTerrain_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableTopology"));
  if (wp) {
    if (EnableTopology_Config != wp->GetDataField()->GetAsBoolean()) {
      EnableTopology_Config = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistryDrawTopology, EnableTopology_Config);
#endif
      changed = true;
      EnableTopology=EnableTopology_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpCirclingZoom"));
  if (wp) {
    if (MapWindow::zoom.CircleZoom() != wp->GetDataField()->GetAsBoolean()) {
      MapWindow::zoom.CircleZoom(wp->GetDataField()->GetAsBoolean());
#if OLDPROFILES
      SetToRegistry(szRegistryCircleZoom, MapWindow::zoom.CircleZoom());
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOrientation"));
  if (wp) {
    if (DisplayOrientation_Config != wp->GetDataField()->GetAsInteger()) {
      DisplayOrientation_Config = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryDisplayUpValue,DisplayOrientation_Config);
#endif
      DisplayOrientation=DisplayOrientation_Config;
      MapWindow::SetAutoOrientation(true); // reset
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMenuTimeout"));
  if (wp) {
    if (MenuTimeout_Config != wp->GetDataField()->GetAsInteger()*2) {
      MenuTimeout_Config = wp->GetDataField()->GetAsInteger()*2;
#if OLDPROFILES
      SetToRegistry(szRegistryMenuTimeout,MenuTimeout_Config);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeArrival"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY);
    if (SAFETYALTITUDEARRIVAL != ival) {
      SAFETYALTITUDEARRIVAL = ival;
#if OLDPROFILES
      SetToRegistry(szRegistrySafetyAltitudeArrival,
		    (DWORD)SAFETYALTITUDEARRIVAL);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeTerrain"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY);
    if (SAFETYALTITUDETERRAIN != ival) {
      SAFETYALTITUDETERRAIN = ival;
#if OLDPROFILES
      SetToRegistry(szRegistrySafetyAltitudeTerrain,
		    (DWORD)SAFETYALTITUDETERRAIN);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoWind"));
  if (wp) {
    if (AutoWindMode_Config != wp->GetDataField()->GetAsInteger()) {
      AutoWindMode_Config = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryAutoWind, AutoWindMode);
#endif
      AutoWindMode = AutoWindMode_Config;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoMcMode"));
  if (wp) {
    if (AutoMcMode_Config != wp->GetDataField()->GetAsInteger()) {
      AutoMcMode_Config = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryAutoMcMode, AutoMcMode_Config);
#endif
	AutoMcMode=AutoMcMode_Config;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointsOutOfRange"));
  if (wp) {
    if (WaypointsOutOfRange != wp->GetDataField()->GetAsInteger()) {
      WaypointsOutOfRange = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryWaypointsOutOfRange, WaypointsOutOfRange);
#endif
      WAYPOINTFILECHANGED= true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoForceFinalGlide"));
  if (wp) {
    if (AutoForceFinalGlide != wp->GetDataField()->GetAsBoolean()) {
      AutoForceFinalGlide = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistryAutoForceFinalGlide, AutoForceFinalGlide);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableNavBaroAltitude"));
  if (wp) {
    if (EnableNavBaroAltitude_Config != wp->GetDataField()->GetAsBoolean()) {
      EnableNavBaroAltitude_Config = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistryEnableNavBaroAltitude, EnableNavBaroAltitude_Config);
#endif
      changed = true;
      EnableNavBaroAltitude=EnableNavBaroAltitude_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOrbiter"));
  if (wp) {
    if (Orbiter_Config != wp->GetDataField()->GetAsBoolean()) {
      Orbiter_Config = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistryOrbiter, Orbiter_Config);
#endif
      changed = true;
      Orbiter=Orbiter_Config;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoMcStatus"));
  if (wp) {
    if (AutoMacCready_Config != wp->GetDataField()->GetAsInteger()) {
      AutoMacCready_Config = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryAutoMcStatus, AutoMacCready_Config);
#endif
      CALCULATED_INFO.AutoMacCready=AutoMacCready_Config;
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpShading"));
  if (wp) {
    if (Shading_Config != wp->GetDataField()->GetAsBoolean()) {
      Shading_Config = wp->GetDataField()->GetAsBoolean();
#if OLDPROFILES
      SetToRegistry(szRegistryShading, Shading_Config);
#endif
      changed = true;
      Shading=Shading_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFinalGlideTerrain"));
  if (wp) {
    if (FinalGlideTerrain != wp->GetDataField()->GetAsInteger()) {
      FinalGlideTerrain = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryFinalGlideTerrain, FinalGlideTerrain);
#endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsSpeed"));
  if (wp) {
    if ((int)SpeedUnit_Config != wp->GetDataField()->GetAsInteger()) {
      SpeedUnit_Config = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistrySpeedUnitsValue, SpeedUnit_Config);
#endif
      Units::NotifyUnitChanged();
      requirerestart = true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLatLon"));
  if (wp) {
    if ((int)Units::CoordinateFormat != wp->GetDataField()->GetAsInteger()) {
      Units::CoordinateFormat = (CoordinateFormats_t)
        wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryLatLonUnits, Units::CoordinateFormat);
#endif
      Units::NotifyUnitChanged();
      requirerestart = true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsTaskSpeed"));
  if (wp) {
    if ((int)TaskSpeedUnit_Config != wp->GetDataField()->GetAsInteger()) {
      TaskSpeedUnit_Config = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryTaskSpeedUnitsValue, TaskSpeedUnit_Config);
#endif
      Units::NotifyUnitChanged();
      requirerestart = true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsDistance"));
  if (wp) {
    if ((int)DistanceUnit_Config != wp->GetDataField()->GetAsInteger()) {
      DistanceUnit_Config = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryDistanceUnitsValue, DistanceUnit_Config);
#endif
      Units::NotifyUnitChanged();
      requirerestart = true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLift"));
  if (wp) {
    if ((int)LiftUnit_Config != wp->GetDataField()->GetAsInteger()) {
      LiftUnit_Config = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryLiftUnitsValue, LiftUnit_Config);
#endif
      Units::NotifyUnitChanged();
      requirerestart = true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsAltitude"));
  if (wp) {
    if ((int)AltitudeUnit_Config != wp->GetDataField()->GetAsInteger()) {
      AltitudeUnit_Config = wp->GetDataField()->GetAsInteger();
#if OLDPROFILES
      SetToRegistry(szRegistryAltitudeUnitsValue, AltitudeUnit_Config);
#endif
      Units::NotifyUnitChanged();
      changed = true;
      requirerestart = true;
    }
  }

/* REMOVE
  wp = (WndProperty*)wf->FindByName(TEXT("prpHandicap"));
  if (wp) {
    ival  = wp->GetDataField()->GetAsInteger();
    if (Handicap != ival) {
      Handicap = ival;
#if OLDPROFILES
      SetToRegistry(szRegistryHandicap, Handicap);
#endif
      changed = true;
    }
  }
*/

/* REMOVE
  wp = (WndProperty*)wf->FindByName(TEXT("prpPolarFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,_T(""))==0) {
	_tcscpy(temptext,_T("%LOCAL_PATH%\\\\_Polars\\Default.plr"));
    } else
      ContractLocalPath(temptext);

    if (_tcscmp(temptext,szPolarFile)) {
      #if OLDPROFILES
      SetRegistryString(szRegistryPolarFile, temptext);
      #else
      _tcscpy(szPolarFile,temptext);
      #endif
      POLARFILECHANGED = true;
      GlidePolar::SetBallast();
      changed = true;
    }
  }
*/

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szWaypointFile)) {
      #if OLDPROFILES
      SetRegistryString(szRegistryWayPointFile, temptext);
      #else
      _tcscpy(szWaypointFile,temptext);
      #endif
      WAYPOINTFILECHANGED= true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szAdditionalWaypointFile)) {
      #if OLDPROFILES
      SetRegistryString(szRegistryAdditionalWayPointFile, temptext);
      #else
      _tcscpy(szAdditionalWaypointFile,temptext);
      #endif
      WAYPOINTFILECHANGED= true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szAirspaceFile)) {
      #if OLDPROFILES
      SetRegistryString(szRegistryAirspaceFile, temptext);
      #else
      _tcscpy(szAirspaceFile,temptext);
      #endif
      AIRSPACEFILECHANGED= true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szAdditionalAirspaceFile)) {
      #if OLDPROFILES
      SetRegistryString(szRegistryAdditionalAirspaceFile, temptext);
      #else
      _tcscpy(szAdditionalAirspaceFile,temptext);
      #endif
      AIRSPACEFILECHANGED= true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMapFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szMapFile)) {
      #if OLDPROFILES
      SetRegistryString(szRegistryMapFile, temptext);
      #else
      _tcscpy(szMapFile,temptext);
      #endif
      MAPFILECHANGED= true;
      #if LKMTERRAIN
      TERRAINFILECHANGED= true; //for .xcm
      #endif
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szTerrainFile)) {
      #if OLDPROFILES
      SetRegistryString(szRegistryTerrainFile, temptext);
      #else
      _tcscpy(szTerrainFile,temptext);
      #endif
      TERRAINFILECHANGED= true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirfieldFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szAirfieldFile)) {
      #if OLDPROFILES
      SetRegistryString(szRegistryAirfieldFile, temptext);
      #else
      _tcscpy(szAirfieldFile,temptext);
      #endif
      AIRFIELDFILECHANGED= true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLanguageFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szLanguageFile)) {
      #if OLDPROFILES
      SetRegistryString(szRegistryLanguageFile, temptext);
      #else
      _tcscpy(szLanguageFile,temptext);
      #endif
      requirerestart = true; // restart needed for language load
      // LKReadLanguageFile(); // NO GOOD. MEMORY LEAKS PENDING
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpInputFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szInputFile)) {
      #if OLDPROFILES
      SetRegistryString(szRegistryInputFile, temptext);
      #else
      _tcscpy(szInputFile,temptext);
      #endif
      requirerestart = true;
      changed = true;
    }
  }

/* REMOVE
  wp = (WndProperty*)wf->FindByName(TEXT("prpBallastSecsToEmpty"));
  if (wp) {
    ival = wp->GetDataField()->GetAsInteger();
    if (BallastSecsToEmpty != ival) {
      BallastSecsToEmpty = ival;
      SetToRegistry(szRegistryBallastSecsToEmpty,(DWORD)BallastSecsToEmpty);
      changed = true;
    }
  }
*/

  wp = (WndProperty*)wf->FindByName(TEXT("prpWindCalcTime")); // 100113
  if (wp) {
	ival = wp->GetDataField()->GetAsInteger();
	if (WindCalcTime != ival) {
		WindCalcTime = ival;
		SetToRegistry(szRegistryWindCalcTime,(DWORD)WindCalcTime);
		changed = true;
	}
  }

/* REMOVE
  wp = (WndProperty*)wf->FindByName(TEXT("prpMaxManoeuveringSpeed"));
  if (wp) {
    ival = iround((wp->GetDataField()->GetAsInteger()/SPEEDMODIFY)*1000.0);
    if ((int)SAFTEYSPEED != (int)iround(ival/1000)) {
      SAFTEYSPEED = ival;
      SetToRegistry(szRegistrySafteySpeed,(DWORD)SAFTEYSPEED);
	SAFTEYSPEED=ival/1000.0;
      GlidePolar::SetBallast();
      changed = true;
    }
  }
*/

  wp = (WndProperty*)wf->FindByName(TEXT("prpWindCalcSpeed")); // 100112
  if (wp) {
	ival = iround((wp->GetDataField()->GetAsInteger()/SPEEDMODIFY)*1000.0);
	if ((int)WindCalcSpeed != (int)iround(ival/1000)) {
		WindCalcSpeed = ival;
		SetToRegistry(szRegistryWindCalcSpeed,(DWORD)WindCalcSpeed);
		WindCalcSpeed=ival/1000.0;
		changed = true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishLine"));
  if (wp) {
    if (FinishLine != wp->GetDataField()->GetAsInteger()) {
      FinishLine = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryFinishLine,FinishLine);
      changed = true;
      taskchanged = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishRadius"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsFloat()/DISTANCEMODIFY);
    if ((int)FinishRadius != ival) {
      FinishRadius = ival;
      SetToRegistry(szRegistryFinishRadius,FinishRadius);
      changed = true;
      taskchanged = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartLine"));
  if (wp) {
    if (StartLine != wp->GetDataField()->GetAsInteger()) {
      StartLine = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryStartLine,StartLine);
      changed = true;
      taskchanged = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartRadius"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsFloat()/DISTANCEMODIFY);
    if ((int)StartRadius != ival) {
      StartRadius = ival;
      SetToRegistry(szRegistryStartRadius,StartRadius);
      changed = true;
      taskchanged = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFAISector"));
  if (wp) {
    if ((int)SectorType != wp->GetDataField()->GetAsInteger()) {
      SectorType = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryFAISector,SectorType);
      changed = true;
      taskchanged = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskSectorRadius"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsFloat()/DISTANCEMODIFY);
    if ((int)SectorRadius != ival) {
      SectorRadius = ival;
      SetToRegistry(szRegistrySectorRadius,SectorRadius);
      changed = true;
      taskchanged = true;
    }
  }

/* REMOVE
  wp = (WndProperty*)wf->FindByName(TEXT("prpAircraftCategory")); // VENTA4
  if (wp) {
    if (AircraftCategory != (AircraftCategory_t)
	(wp->GetDataField()->GetAsInteger())) {
      AircraftCategory = (AircraftCategory_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryAircraftCategory,
		    (DWORD)(AircraftCategory));
      changed = true;
      requirerestart = true;
	if (ISPARAGLIDER) AATEnabled=TRUE;
    }
  }
*/

  #if (0)
  wp = (WndProperty*)wf->FindByName(TEXT("prpAltArrivMode"));
  if (wp) {
    if (AltArrivMode != (AltArrivMode_t)
	(wp->GetDataField()->GetAsInteger())) {
      AltArrivMode = (AltArrivMode_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryAltArrivMode,
		    (DWORD)(AltArrivMode));
      changed = true;
    }
  }
  #endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpCheckSum")); 
  if (wp) {
    if (CheckSum != (CheckSum_t)
	(wp->GetDataField()->GetAsInteger())) {
      CheckSum = (CheckSum_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryCheckSum,
		    (DWORD)(CheckSum));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIphoneGestures")); 
  if (wp) {
    if (IphoneGestures != (IphoneGestures_t) (wp->GetDataField()->GetAsInteger())) {
      IphoneGestures = (IphoneGestures_t) (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryIphoneGestures, (DWORD)(IphoneGestures));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPollingMode")); 
  if (wp) {
    if (PollingMode != (PollingMode_t) (wp->GetDataField()->GetAsInteger())) {
      PollingMode = (PollingMode_t) (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryPollingMode, (DWORD)(PollingMode));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKVarioBar")); 
  if (wp) {
    if (LKVarioBar != (LKVarioBar_t)
	(wp->GetDataField()->GetAsInteger())) {
      LKVarioBar = (LKVarioBar_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryLKVarioBar,
		    (DWORD)(LKVarioBar));
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLKVarioVal")); 
  if (wp) {
    if (LKVarioVal != (LKVarioVal_t)
	(wp->GetDataField()->GetAsInteger())) {
      LKVarioVal = (LKVarioVal_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryLKVarioVal,
		    (DWORD)(LKVarioVal));
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpHideUnits")); // VENTA6
  if (wp) {
    if (HideUnits != (HideUnits_t)
	(wp->GetDataField()->GetAsInteger())) {
      HideUnits = (HideUnits_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryHideUnits,
		    (DWORD)(HideUnits));
      changed = true;
      requirerestart = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpDeclutterMode")); // VENTA10
  if (wp) {
    if (DeclutterMode != (DeclutterMode_t)
	(wp->GetDataField()->GetAsInteger())) {
      DeclutterMode = (DeclutterMode_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryDeclutterMode,
		    (DWORD)(DeclutterMode));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpActiveMap")); // 100318
  if (wp) {
    if (ActiveMap_Config != (ActiveMap_t) (wp->GetDataField()->GetAsInteger())) {
      ActiveMap_Config = (ActiveMap_t) (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryActiveMap, (DWORD)(ActiveMap_Config));
      ActiveMap=ActiveMap_Config;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBestWarning"));
  if (wp) {
    if (BestWarning != (wp->GetDataField()->GetAsInteger())) {
      BestWarning = (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryBestWarning, (DWORD)(BestWarning));
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpUseTotalEnergy"));
  if (wp) {
    if (UseTotalEnergy_Config != (wp->GetDataField()->GetAsInteger())) {
      UseTotalEnergy_Config = (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryUseTotalEnergy, (DWORD)(UseTotalEnergy_Config));
      UseTotalEnergy=UseTotalEnergy_Config;
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpThermalBar"));
  if (wp) {
    if (ThermalBar != (wp->GetDataField()->GetAsInteger())) {
      ThermalBar = (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryThermalBar, (DWORD)(ThermalBar));
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayClock"));
  if (wp) {
    if (OverlayClock != (wp->GetDataField()->GetAsInteger())) {
      OverlayClock = (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryOverlayClock, (DWORD)(OverlayClock));
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpMcOverlay"));
  if (wp) {
    if (McOverlay != (wp->GetDataField()->GetAsInteger())) {
      McOverlay = (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryMcOverlay, (DWORD)(McOverlay));
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTrackBar"));
  if (wp) {
    if (TrackBar != (wp->GetDataField()->GetAsInteger())) {
      TrackBar = (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryTrackBar, (DWORD)(TrackBar));
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOptimizeRoute"));
  if (wp) {
    if (PGOptimizeRoute != (wp->GetDataField()->GetAsInteger())) {
      PGOptimizeRoute = (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryPGOptimizeRoute, (DWORD)(PGOptimizeRoute));
      changed = true;

      if (ISPARAGLIDER) {
	    if(PGOptimizeRoute) {
		  changed = !AATEnabled;
		  AATEnabled = true;
	    }
	    else{
	      ClearOptimizedTargetPos();
	    }
	  }
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOutlinedTp")); // VENTA6
  if (wp) {
    if (OutlinedTp != (OutlinedTp_t)
	(wp->GetDataField()->GetAsInteger())) {
      OutlinedTp = (OutlinedTp_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryOutlinedTp,
		    (DWORD)(OutlinedTp));
      changed = true;
      requirerestart = true; // 100819
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTpFilter"));
  if (wp) {
    if (TpFilter != (TpFilter_t)
	(wp->GetDataField()->GetAsInteger())) {
      TpFilter = (TpFilter_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryTpFilter,
		    (DWORD)(TpFilter));
      LastDoRangeWaypointListTime=0;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverColor"));
  if (wp) {
    if (OverColor != (OverColor_t)
	(wp->GetDataField()->GetAsInteger())) {
      OverColor = (OverColor_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryOverColor,
		    (DWORD)(OverColor));
      SetOverColorRef();
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMapBox")); // VENTA6
  if (wp) {
    if (MapBox != (MapBox_t)
	(wp->GetDataField()->GetAsInteger())) {
      MapBox = (MapBox_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryMapBox,
		    (DWORD)(MapBox));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlaySize"));
  if (wp) {
    if (OverlaySize != wp->GetDataField()->GetAsInteger() ) {
      OverlaySize = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryOverlaySize,
		    (DWORD)(OverlaySize));
      changed = true;
      Reset_Single_DoInits(MDI_DRAWLOOK8000);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGlideBarMode")); // VENTA6
  if (wp) {
    if (GlideBarMode != (GlideBarMode_t)
	(wp->GetDataField()->GetAsInteger())) {
      GlideBarMode = (GlideBarMode_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryGlideBarMode,
		    (DWORD)(GlideBarMode));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpNewMapDeclutter")); // VENTA6
  if (wp) {
    if (NewMapDeclutter != 
	(wp->GetDataField()->GetAsInteger())) {
      NewMapDeclutter = 
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryNewMapDeclutter,
		    (DWORD)(NewMapDeclutter));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGPSAltitudeOffset")); 
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY)*1000.0);  
    if ( ((int)GPSAltitudeOffset) != ival ) {
	GPSAltitudeOffset = ival;
	int sword;
	if (GPSAltitudeOffset<0) {
		sword=(int) ( (GPSAltitudeOffset*(-1)) +9999999);
	} else {
		sword=(int) GPSAltitudeOffset;
	}
	SetToRegistry(szRegistryGpsAltitudeOffset, (DWORD)(sword));
	changed=true;
    }

  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUseGeoidSeparation"));
  if (wp) {
	if (UseGeoidSeparation != (wp->GetDataField()->GetAsInteger())) {
		UseGeoidSeparation = (wp->GetDataField()->GetAsInteger());
		SetToRegistry(szRegistryUseGeoidSeparation, (DWORD)(UseGeoidSeparation));
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPressureHg"));
  if (wp) {
	if (PressureHg != (wp->GetDataField()->GetAsInteger())) {
		PressureHg = (wp->GetDataField()->GetAsInteger());
		SetToRegistry(szRegistryPressureHg, (DWORD)(PressureHg));
		changed=true;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAverEffTime"));
  if (wp) {
    if (AverEffTime != 
	(wp->GetDataField()->GetAsInteger())) {
      AverEffTime = 
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryAverEffTime,
		    (DWORD)(AverEffTime));
      changed = true;
      InitLDRotary(&rotaryLD);
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpBgMapcolor"));
  if (wp) {
    if (BgMapColor_Config != (wp->GetDataField()->GetAsInteger())) {
      BgMapColor_Config = (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryBgMapColor, (DWORD)(BgMapColor_Config));
      BgMapColor = BgMapColor_Config;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpArrivalValue")); // VENTA6
  if (wp) {
    if (ArrivalValue != (ArrivalValue_t)
	(wp->GetDataField()->GetAsInteger())) {
      ArrivalValue = (ArrivalValue_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryArrivalValue,
		    (DWORD)(ArrivalValue));
      changed = true;
    }
  }

#if defined(PNA) 
// VENTA-ADDON MODEL CHANGE
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxModel"));
  if (wp) {
    if (Appearance.InfoBoxModel != (InfoBoxModelAppearance_t)
        (wp->GetDataField()->GetAsInteger())) {
      Appearance.InfoBoxModel = (InfoBoxModelAppearance_t)
        (wp->GetDataField()->GetAsInteger());

      switch (Appearance.InfoBoxModel) {
      case apImPnaGeneric: 
	GlobalModelType = MODELTYPE_PNA_PNA;
	break;
      case apImPnaHp31x:
	GlobalModelType = MODELTYPE_PNA_HP31X;
	break;
      case apImPnaPn6000:
	GlobalModelType = MODELTYPE_PNA_PN6000;
	break;
      case apImPnaMio:
	GlobalModelType = MODELTYPE_PNA_MIO;
	break;
      case apImPnaNokia500:
	GlobalModelType = MODELTYPE_PNA_NOKIA_500;
	break;
      case apImPnaMedionP5:
	GlobalModelType = MODELTYPE_PNA_MEDION_P5;
	break;
      case apImPnaNavigon:
	GlobalModelType = MODELTYPE_PNA_NAVIGON;
	break;
      case apImPnaFuntrek:
	GlobalModelType = MODELTYPE_PNA_FUNTREK;
	break;
      case apImPnaRoyaltek3200:
	GlobalModelType = MODELTYPE_PNA_ROYALTEK3200;
	break;
      default:
	GlobalModelType = MODELTYPE_UNKNOWN; // Can't happen, troubles ..
	break;
	
      }
      #if OLDPROFILES
      SetToRegistry(szRegistryAppInfoBoxModel, GlobalModelType);
      #else
      Appearance.InfoBoxModel = (InfoBoxModelAppearance_t)GlobalModelType;
      #endif
      changed = true;
      requirerestart = true;
    }
  }
//
#endif

  //Fonts
  int UseCustomFontsold = UseCustomFonts;
  wp = (WndProperty*)wf->FindByName(TEXT("prpUseCustomFonts"));
  if (wp) {
    DataFieldBoolean * dfb = (DataFieldBoolean*) wp->GetDataField();
    if (dfb) {
      SetToRegistry(szRegistryUseCustomFonts, dfb->GetAsInteger());  
      UseCustomFonts = dfb->GetAsInteger(); // global var
    }
  }
  if ( (UseCustomFontsold != UseCustomFonts) ||
    (UseCustomFonts && FontRegistryChanged) ) {
      changed = true;
      requirerestart = true;
  }
  DeleteObject(TempUseCustomFontsFont);

  DeleteObject (TempMapWindowFont);
  DeleteObject (TempMapLabelFont);

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppIndLandable"));
  if (wp) {
    if (Appearance.IndLandable != (IndLandable_t)(wp->GetDataField()->GetAsInteger())) {
      Appearance.IndLandable = (IndLandable_t)(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryAppIndLandable,(DWORD)(Appearance.IndLandable));
      changed = true;
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInverseInfoBox"));
  if (wp) {
    if ((int)(InverseInfoBox_Config) != wp->GetDataField()->GetAsInteger()) {
      InverseInfoBox_Config = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAppInverseInfoBox,InverseInfoBox_Config);
      requirerestart = true;
      Appearance.InverseInfoBox=InverseInfoBox_Config;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGliderScreenPosition"));
  if (wp) {
    if (MapWindow::GliderScreenPosition != 
	wp->GetDataField()->GetAsInteger()) {
      MapWindow::GliderScreenPosition = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryGliderScreenPosition,
		    MapWindow::GliderScreenPosition);
	MapWindow::GliderScreenPositionY=MapWindow::GliderScreenPosition;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppDefaultMapWidth"));
  if (wp) {
    if ((int)(Appearance.DefaultMapWidth) != 
	wp->GetDataField()->GetAsInteger()) {
      Appearance.DefaultMapWidth = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAppDefaultMapWidth,Appearance.DefaultMapWidth);
      requirerestart = true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoBacklight")); // VENTA4
  if (wp) {
    if (EnableAutoBacklight != (wp->GetDataField()->GetAsInteger()!=0)) {
      EnableAutoBacklight = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAutoBacklight, EnableAutoBacklight);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoSoundVolume")); // VENTA4
  if (wp) {
    if (EnableAutoSoundVolume != (wp->GetDataField()->GetAsInteger()!=0)) {
      EnableAutoSoundVolume = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAutoSoundVolume, EnableAutoSoundVolume);
      changed = true;
    }
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainContrast"));
  if (wp) {
    if (iround(TerrainContrast*100/255) != 
	wp->GetDataField()->GetAsInteger()) {
      TerrainContrast = (short)iround(wp->GetDataField()->GetAsInteger()*255.0/100);
      SetToRegistry(szRegistryTerrainContrast,TerrainContrast);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainBrightness"));
  if (wp) {
    if (iround(TerrainBrightness*100/255) != 
	wp->GetDataField()->GetAsInteger()) {
      TerrainBrightness = (short)iround(wp->GetDataField()->GetAsInteger()*255.0/100);
      SetToRegistry(szRegistryTerrainBrightness,TerrainBrightness);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainRamp"));
  if (wp) {
    if (TerrainRamp_Config != wp->GetDataField()->GetAsInteger()) {
      TerrainRamp_Config = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryTerrainRamp, TerrainRamp_Config);
      TerrainRamp=TerrainRamp_Config;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmMaxAltitude1"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY) *1000.0);
    if ((int)AlarmMaxAltitude1 != ival) {
      AlarmMaxAltitude1 = ival;
      LKalarms[0].triggervalue=(int)AlarmMaxAltitude1/1000;
      LKalarms[0].triggerscount=0;
      SetToRegistry(szRegistryAlarmMaxAltitude1,AlarmMaxAltitude1);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmMaxAltitude2"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY) *1000.0);
    if ((int)AlarmMaxAltitude2 != ival) {
      AlarmMaxAltitude2 = ival;
      LKalarms[1].triggervalue=(int)AlarmMaxAltitude2/1000;
      LKalarms[1].triggerscount=0;
      SetToRegistry(szRegistryAlarmMaxAltitude2,AlarmMaxAltitude2);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmMaxAltitude3"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY) *1000.0);
    if ((int)AlarmMaxAltitude3 != ival) {
      AlarmMaxAltitude3 = ival;
      LKalarms[2].triggervalue=(int)AlarmMaxAltitude3/1000;
      LKalarms[2].triggerscount=0;
      SetToRegistry(szRegistryAlarmMaxAltitude3,AlarmMaxAltitude3);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoAdvance"));
  if (wp) {
    if (AutoAdvance_Config != wp->GetDataField()->GetAsInteger()) {
      AutoAdvance_Config = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAutoAdvance, AutoAdvance_Config);
      changed = true;
      AutoAdvance=AutoAdvance_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLoggerTimeStepCruise"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger());
    if (LoggerTimeStepCruise != ival) {
      LoggerTimeStepCruise = ival;
      SetToRegistry(szRegistryLoggerTimeStepCruise,LoggerTimeStepCruise);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLoggerTimeStepCircling"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger());
    if (LoggerTimeStepCircling != ival) {
      LoggerTimeStepCircling = ival;
      SetToRegistry(szRegistryLoggerTimeStepCircling,LoggerTimeStepCircling);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComPort1"));
  if (wp) {
    if ((int)dwPortIndex1 != wp->GetDataField()->GetAsInteger()) {
      dwPortIndex1 = wp->GetDataField()->GetAsInteger();
      changed = true;
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComSpeed1"));
  if (wp) {
    if ((int)dwSpeedIndex1 != wp->GetDataField()->GetAsInteger()) {
      dwSpeedIndex1 = wp->GetDataField()->GetAsInteger();
      changed = true;
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComBit1"));
  if (wp) {
    if ((int)dwBit1Index != wp->GetDataField()->GetAsInteger()) {
      dwBit1Index = wp->GetDataField()->GetAsInteger();
      changed = true;
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComDevice1"));
  if (wp) {
    if (dwDeviceIndex1 != wp->GetDataField()->GetAsInteger()) {
      dwDeviceIndex1 = wp->GetDataField()->GetAsInteger();
      changed = true;
      COMPORTCHANGED = true;
      devRegisterGetName(dwDeviceIndex1, DeviceName);
      WriteDeviceSettings(0, DeviceName);  
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComPort2"));
  if (wp) {
    if ((int)dwPortIndex2 != wp->GetDataField()->GetAsInteger()) {
      dwPortIndex2 = wp->GetDataField()->GetAsInteger();
      changed = true;
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComSpeed2"));
  if (wp) {
    if ((int)dwSpeedIndex2 != wp->GetDataField()->GetAsInteger()) {
      dwSpeedIndex2 = wp->GetDataField()->GetAsInteger();
      changed = true;
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComBit1"));
  if (wp) {
    if ((int)dwBit1Index != wp->GetDataField()->GetAsInteger()) {
      dwBit1Index = wp->GetDataField()->GetAsInteger();
      changed = true;
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComDevice2"));
  if (wp) {
    if (dwDeviceIndex2 != wp->GetDataField()->GetAsInteger()) {
      dwDeviceIndex2 = wp->GetDataField()->GetAsInteger();
      changed = true;
      COMPORTCHANGED = true;
      devRegisterGetName(dwDeviceIndex2, DeviceName);
      WriteDeviceSettings(1, DeviceName);  
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSnailWidthScale"));
  if (wp) {
    if (MapWindow::SnailWidthScale != wp->GetDataField()->GetAsInteger()) {
      MapWindow::SnailWidthScale = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistrySnailWidthScale,MapWindow::SnailWidthScale);
      changed = true;
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEngineeringMenu")); // VENTA9
  if (wp) EngineeringMenu = wp->GetDataField()->GetAsInteger();

  #ifndef NEWPROFILES
  if (COMPORTCHANGED) {
    WritePort1Settings(dwPortIndex1,dwSpeedIndex1, dwBit1Index);
    WritePort2Settings(dwPortIndex2,dwSpeedIndex2, dwBit2Index);
  }
  #endif


 UpdateAircraftConfig();

  int i,j;
  for (i=0; i<4; i++) {
    for (j=0; j<8; j++) {
      GetInfoBoxSelector(j, i);
    }
  }

  if (waypointneedsave) {
    if(MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M581_ = "Save changes to waypoint file?" 
                   gettext(TEXT("_@M581_")),
	// LKTOKEN  _@M809_ = "Waypoints edited" 
                   gettext(TEXT("_@M809_")),
                   MB_YESNO|MB_ICONQUESTION) == IDYES) {
    
      AskWaypointSave();

    }
  }

  if (taskchanged) {
    RefreshTask();
  }

#if (WINDOWSPC>0)
  if (COMPORTCHANGED) {
    requirerestart = true;
  }
#endif

  if (changed) {

	PGOpenTime=((PGOpenTimeH*60)+PGOpenTimeM)*60;
	PGCloseTime=PGOpenTime+(PGGateIntervalTime*PGNumberOfGates*60);
	if (PGCloseTime>86399) PGCloseTime=86399; // 23:59:59
	ActiveGate=-1;

    #if OLDPROFILES
    StoreRegistry();
    #endif

    if (!requirerestart) {
      MessageBoxX (hWndMainWindow, 
	// LKTOKEN  _@M168_ = "Changes to configuration saved." 
		   gettext(TEXT("_@M168_")), 
		   TEXT("Configuration"), MB_OK);
    } else {

      MessageBoxX (hWndMainWindow, 
	// LKTOKEN  _@M561_ = "Restart LK8000 to apply changes." 
		   gettext(TEXT("_@M561_")), 
		   TEXT("Configuration"), MB_OK);
    }
  }

  delete wf;

  wf = NULL;

}

//
// We must call this update also before saving profiles during config showmodal, otherwise we 
// wont be updating the current set values! This is why we keep separated function for polar.
//
void UpdateAircraftConfig(void){

 WndProperty *wp;
 int ival;

 wp = (WndProperty*)wf->FindByName(TEXT("prpAircraftCategory"));
  if (wp) {
    if (AircraftCategory != (AircraftCategory_t)
        (wp->GetDataField()->GetAsInteger())) {
      AircraftCategory = (AircraftCategory_t)
        (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryAircraftCategory, (DWORD)(AircraftCategory));
      changed = true;
      requirerestart = true;
        if (ISPARAGLIDER) AATEnabled=TRUE; // NOT SURE THIS IS NEEDED ANYMORE. 
    }
  }

 wp = (WndProperty*)wf->FindByName(TEXT("prpPolarFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,_T(""))==0) {
        _tcscpy(temptext,_T("%LOCAL_PATH%\\\\_Polars\\Default.plr"));
    } else
      ContractLocalPath(temptext);

    if (_tcscmp(temptext,szPolarFile)) {
      #if OLDPROFILES
      SetRegistryString(szRegistryPolarFile, temptext);
      #else
      _tcscpy(szPolarFile,temptext);
      #endif
      POLARFILECHANGED = true;
      GlidePolar::SetBallast();
      changed = true;
    }
  }

 wp = (WndProperty*)wf->FindByName(TEXT("prpMaxManoeuveringSpeed"));
  if (wp) {
    ival = iround((wp->GetDataField()->GetAsInteger()/SPEEDMODIFY)*1000.0);
    if ((int)SAFTEYSPEED != (int)iround(ival/1000)) {
        SAFTEYSPEED=ival/1000.0;
      GlidePolar::SetBallast();
      changed = true;
    }
  }

 wp = (WndProperty*)wf->FindByName(TEXT("prpHandicap"));
  if (wp) {
    ival  = wp->GetDataField()->GetAsInteger();
    if (Handicap != ival) {
      Handicap = ival;
#if OLDPROFILES
      SetToRegistry(szRegistryHandicap, Handicap);
#endif
      changed = true;
    }
  }

 wp = (WndProperty*)wf->FindByName(TEXT("prpBallastSecsToEmpty"));
  if (wp) {
    ival = wp->GetDataField()->GetAsInteger();
    if (BallastSecsToEmpty != ival) {
      BallastSecsToEmpty = ival;
      SetToRegistry(szRegistryBallastSecsToEmpty,(DWORD)BallastSecsToEmpty);
      changed = true;
    }
  }
}


#if 0 // REMOVE
void UpdatePilotConfig(void){

 WndProperty *wp;
 int ival;

#if 0
static void OnPilotNameClicked(WindowControl *Sender) {
	(void)Sender;
  TCHAR Temp[100];
  if (buttonPilotName) {
    #if OLDPROFILES
    GetRegistryString(szRegistryPilotName,Temp,100);
    #else
    _tcscpy(Temp,PilotName_Config);
    #endif
    dlgTextEntryShowModal(Temp,100);
    #if OLDPROFILES
    SetRegistryString(szRegistryPilotName,Temp);
    #else
    _tcscpy(PilotName_Config,Temp);
    #endif
    changed = true;
  }
  UpdateButtons();
}
#endif
}
#endif // REMOVE
