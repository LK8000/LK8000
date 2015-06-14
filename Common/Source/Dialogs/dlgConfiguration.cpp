/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgConfiguration.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include "Terrain.h"
#include "LKProcess.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "McReady.h"

#include "Modeltype.h"

#include "McReady.h"
#include "Waypointparser.h"
#include "LKMapWindow.h"
#include "LKProfiles.h"
#include "Calculations2.h"
#include "DoInits.h"
#include "Multimap.h"
#include "Dialogs.h"

#include "utils/stl_utils.h"
#include <iterator>
#include "BtHandler.h"
#include <functional>
#include "Sound/Sound.h"

using namespace std::placeholders;

extern void UpdateAircraftConfig(void);
extern void dlgCustomMenuShowModal(void);
void UpdateComPortList(WndProperty* wp, LPCTSTR szPort);
void UpdateComPortSetting(size_t idx, const TCHAR* szPortName);
void ShowWindowControl(WndForm* pOwner, const TCHAR* WndName, bool bShow);


static bool taskchanged = false;
static bool requirerestart = false;
static bool utcchanged = false;
static bool waypointneedsave = false;
static bool fontschanged= false;


short configMode=0;	// current configuration mode, 0=system 1=pilot 2=aircraft 3=device
short config_page[4]={0,0,0,0}; // remember last page we were using, for each profile

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

static WndButton *buttonPilotName=NULL;
static WndButton *buttonLiveTrackersrv=NULL;
static WndButton *buttonLiveTrackerport=NULL;
static WndButton *buttonLiveTrackerusr=NULL;
static WndButton *buttonLiveTrackerpwd=NULL;
static WndButton *buttonAircraftType=NULL;
static WndButton *buttonAircraftRego=NULL;
static WndButton *buttonCompetitionClass=NULL;
static WndButton *buttonCompetitionID=NULL;
static WndButton *buttonCopy=NULL;
static WndButton *buttonPaste=NULL;

short numPages=0;

void reset_wConfig(void) {
 wConfig1=NULL;
 wConfig2=NULL;
 wConfig3=NULL;
 wConfig4=NULL;
 wConfig5=NULL;
 wConfig6=NULL;
 wConfig7=NULL;
 wConfig8=NULL;
 wConfig9=NULL;
 wConfig10=NULL;
 wConfig11=NULL;
 wConfig12=NULL;
 wConfig13=NULL;
 wConfig14=NULL;
 wConfig15=NULL;
 wConfig16=NULL;
 wConfig17=NULL;
 wConfig18=NULL;
 wConfig19=NULL;
 wConfig20=NULL;
 wConfig21=NULL;
 wConfig22=NULL; 
 wConfig23=NULL; 
 wConfig24=NULL; 
 wConfig25=NULL; 
}

int GlobalToBoxType(int i) {
	int iTmp;
	switch (i) {
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
		case MODELTYPE_PNA_MINIMAP:
				iTmp=(InfoBoxModelAppearance_t)apImPnaMinimap;
				break;
		case MODELTYPE_PNA_GENERIC_BTKA:
				iTmp=(InfoBoxModelAppearance_t)apImPnaGenericBTKA;
				break;
		case MODELTYPE_PNA_GENERIC_BTKB:
				iTmp=(InfoBoxModelAppearance_t)apImPnaGenericBTKB;
				break;
		case MODELTYPE_PNA_GENERIC_BTKC:
				iTmp=(InfoBoxModelAppearance_t)apImPnaGenericBTKC;
				break;
		case MODELTYPE_PNA_GENERIC_BTK1:
				iTmp=(InfoBoxModelAppearance_t)apImPnaGenericBTK1;
				break;
		case MODELTYPE_PNA_GENERIC_BTK2:
				iTmp=(InfoBoxModelAppearance_t)apImPnaGenericBTK2;
				break;
		case MODELTYPE_PNA_GENERIC_BTK3:
				iTmp=(InfoBoxModelAppearance_t)apImPnaGenericBTK3;
				break;

		default:
				iTmp=(InfoBoxModelAppearance_t)apImPnaGeneric;
	}
	return iTmp;
}

static void UpdateButtons(void) {
  TCHAR text[120];
  TCHAR val[100];

  if (buttonPilotName) {
    _tcscpy(val,PilotName_Config);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M524_ = "Pilot name" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M524_")), val);
    buttonPilotName->SetCaption(text);
  }
  if (buttonAircraftType) {
    _tcscpy(val,AircraftType_Config);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M59_ = "Aircraft type" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M59_")), val);
    buttonAircraftType->SetCaption(text);
  }
  if (buttonAircraftRego) {
    _tcscpy(val,AircraftRego_Config);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M57_ = "Aircraft Reg" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M57_")), val);
    buttonAircraftRego->SetCaption(text);
  }
  if (buttonCompetitionClass) {
    _tcscpy(val,CompetitionClass_Config);
    if (_tcslen(val)<=0) {
      // LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M936_ = "Competition Class" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M936_")), val);
    buttonCompetitionClass->SetCaption(text);
  }
  if (buttonCompetitionID) {
    _tcscpy(val,CompetitionID_Config);
    if (_tcslen(val)<=0) {
      // LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M938_ = "Competition ID" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M938_")), val);
    buttonCompetitionID->SetCaption(text);
  }

  WndButton* wCmdBth = ((WndButton *)wf->FindByName(TEXT("cmdBth")));
  if(wCmdBth) {
#ifdef NO_BLUETOOTH
      wCmdBth->SetVisible(false);
#else
      CBtHandler* pHandler = CBtHandler::Get();
      wCmdBth->SetVisible(pHandler && pHandler->IsOk());
#endif
  }
}



static void NextPage(int Step){
  LKASSERT((unsigned int)configMode<sizeof(config_page));
  config_page[configMode] += Step;

  if (configMode==0) {
	if (!EngineeringMenu) { 
		if (config_page[configMode]>=(numPages-2)) { config_page[configMode]=0; }
		if (config_page[configMode]<0) { config_page[configMode]=numPages-3; } 
	} else {
		if (config_page[configMode]>=numPages) { config_page[configMode]=0; }
		if (config_page[configMode]<0) { config_page[configMode]=numPages-1; }
	}
	// temporary skip page 7 and 8, old aircraft/polar/comm setup
	if (config_page[configMode]==6 || config_page[configMode]==7) {
		if (Step>0) 
			config_page[configMode]=8;
		else
			config_page[configMode]=5;
	}
        #if 0
        // Keep menu 12 ready for new font editor
	if (config_page[configMode]==11) {
		if (Step>0) 
			config_page[configMode]=12;
		else
			config_page[configMode]=10;
	}
        #endif


  } else {
	config_page[configMode]=0;
  }
  
  switch(config_page[configMode]) {
  case 0:
    if (configMode==0) {
	wf->SetCaption(MsgToken(10)); // LKTOKEN  _@M10_ = "1 Site" 
    }
    if (configMode==1) {
	wf->SetCaption(MsgToken(1785)); // pilot configuration
    }
    if (configMode==2) {
	wf->SetCaption(MsgToken(1786)); // aircraft configuration
    }
    if (configMode==3) {
	wf->SetCaption(MsgToken(1820)); // device configuration
    }
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
    wf->SetCaption(gettext(TEXT("_@M36_")));	// UNUSED
    break;
  case 7:
	// LKTOKEN  _@M37_ = "8 Devices" 
    wf->SetCaption(gettext(TEXT("_@M37_")));	// UNUSED
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
    wf->SetCaption(gettext(TEXT("_@M13_"))); // TODO in V6
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
  } 

	  if ((config_page[configMode]>=15) && (config_page[configMode]<=18)) {
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


  wConfig1->SetVisible(config_page[configMode] == 0);
  if (wConfig2)
  wConfig2->SetVisible(config_page[configMode] == 1); 
  if (wConfig3)
  wConfig3->SetVisible(config_page[configMode] == 2); 
  if (wConfig4)
  wConfig4->SetVisible(config_page[configMode] == 3); 
  if (wConfig5)
  wConfig5->SetVisible(config_page[configMode] == 4); 
  if (wConfig6)
  wConfig6->SetVisible(config_page[configMode] == 5); 
  if (wConfig7)
  wConfig7->SetVisible(config_page[configMode] == 6); 
  if (wConfig8)
  wConfig8->SetVisible(config_page[configMode] == 7); 
  if (wConfig9)
  wConfig9->SetVisible(config_page[configMode] == 8); 
  if (wConfig10)
  wConfig10->SetVisible(config_page[configMode] == 9); 
  if (wConfig11)
  wConfig11->SetVisible(config_page[configMode] == 10); 
  if (wConfig12)
  wConfig12->SetVisible(config_page[configMode] == 11); 
  if (wConfig13)
  wConfig13->SetVisible(config_page[configMode] == 12); 
  if (wConfig14)
  wConfig14->SetVisible(config_page[configMode] == 13); 
  if (wConfig15)
  wConfig15->SetVisible(config_page[configMode] == 14); 
  if (wConfig16)
  wConfig16->SetVisible(config_page[configMode] == 15); 
  if (wConfig17)
  wConfig17->SetVisible(config_page[configMode] == 16); 
  if (wConfig18)
  wConfig18->SetVisible(config_page[configMode] == 17); 
  if (wConfig19)
  wConfig19->SetVisible(config_page[configMode] == 18); 
  if (wConfig20)
  wConfig20->SetVisible(config_page[configMode] == 19); 
  if (wConfig21)
  wConfig21->SetVisible(config_page[configMode] == 20); 
  if (wConfig22)
  wConfig22->SetVisible(config_page[configMode] == 21);  
  if (wConfig23)
  wConfig23->SetVisible(config_page[configMode] == 22);  
  if (wConfig24)
  wConfig24->SetVisible(config_page[configMode] == 23);  
  if (wConfig25)
  wConfig25->SetVisible(config_page[configMode] == 24);  

} // NextPage

static void OnSetupDeviceAClicked(WndButton* pWnd) {

#if (ToDo)
    devA()->DoSetup();
    wf->FocusNext(NULL);
#endif

    // this is a hack, devices dont jet support device dependant setup dialogs
    if (!SIMMODE) {
        if ((devA() == NULL) || (_tcscmp(devA()->Name, TEXT("Vega")) != 0)) {
            return;
        }
    }

    // this is a hack to get the dialog to retain focus because
    // the progress dialog in the vario configuration somehow causes
    // focus problems
    wf->FocusNext(NULL);
}

static void OnSetupDeviceBClicked(WndButton* pWnd) {

#if (ToDo)
    devB()->DoSetup();
    wf->FocusNext(NULL);
#endif

    // this is a hack, devices dont jet support device dependant setup dialogs
    if (!SIMMODE) {
        if ((devB() == NULL) || (_tcscmp(devB()->Name, TEXT("Vega")) != 0)) {
            return;
        }
    }

    // this is a hack to get the dialog to retain focus because
    // the progress dialog in the vario configuration somehow causes
    // focus problems
    wf->FocusNext(NULL);
}

static void UpdateDeviceSetupButton(size_t idx, TCHAR *Name) {
    const TCHAR * DevicePropName[] = {_T("prpComPort1"), _T("prpComPort2")};
    const TCHAR * SetupButtonName[] = {_T("cmdSetupDeviceA"), _T("cmdSetupDeviceB")};

    // check if all array have same size ( compil time check );
    static_assert(array_size(DeviceList) == array_size(DevicePropName), "DevicePropName array size need to be same of DeviceList array size");
    static_assert(array_size(DeviceList) == array_size(SetupButtonName), "SetupButtonName array size need to be same of DeviceList array size");

    if (std::begin(DeviceList) + idx < std::end(DeviceList)) {
        bool bHidePort = DeviceList[idx].Disabled = (_tcslen(Name) == 0) || (_tcscmp(Name, _T(DEV_DISABLED_NAME)) == 0);
        _tcscpy(DeviceList[idx].Name, Name);
        bHidePort |= (_tcscmp(Name, _T("Internal")) == 0);
        
        ShowWindowControl(wf, DevicePropName[idx], !bHidePort);
        ShowWindowControl(wf, SetupButtonName[idx], !bHidePort && false/*DeviceList[idx].DoSetup*/);  
        
        WndProperty* wp = (WndProperty*) wf->FindByName(DevicePropName[idx]);
        if (wp) {
            UpdateComPortSetting(idx, wp->GetDataField()->GetAsString());
        }
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

 
static void OnAspPermModified(DataField *Sender, DataField::DataAccessKind_t Mode){
  WndProperty* wp;


  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      wp = (WndProperty*)wf->FindByName(TEXT("prpAspPermDisable"));
      if (wp)
    	  AspPermanentChanged=(wp->GetDataField()->GetAsInteger());
		StartupStore(_T(".......AspPermanentChanged %i %s"),AspPermanentChanged,NEWLINE);

    break;
	default:
		StartupStore(_T("........... DBG-908%s"),NEWLINE);
		break;
  }

}

static void OnGearWarningModeChange(DataField *Sender, DataField::DataAccessKind_t Mode){
WndProperty* wp;

int ival;
    switch(Mode){
      case DataField::daGet:
	  break;
      case DataField::daPut:
      case DataField::daChange:
    	wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmGearWarning"));
    	if (wp) {
    	  ival = iround( wp->GetDataField()->GetAsInteger() );
    	  if ((int)GearWarningMode != ival) {
    	  	GearWarningMode = ival;
    	  }
    	}
        wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmGearAltitude"));
        if(wp)
        {
          if(GearWarningMode == 0)
          {
            wp->SetVisible(false);
            wp->SetReadOnly(true);
            wp->RefreshDisplay();
          }
          else
          {
            wp->GetDataField()->SetAsFloat(iround(GearWarningAltitude*ALTITUDEMODIFY/1000));
            wp->GetDataField()->SetUnits(Units::GetAltitudeName());
            wp->SetVisible(true);
            wp->SetReadOnly(false);
            wp->RefreshDisplay();
          }
        }
      break;
      default:
//     	StartupStore(_T("........... DBG-908%s"),NEWLINE);
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
			Look8000 = lxcStandard+(Look8000_t) (wp->GetDataField()->GetAsInteger());
		}
	}

	wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayClock"));
	if (wp) {
		if (Look8000==lxcStandard)  {
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


static void OnAircraftRegoClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonAircraftRego) {
        _tcscpy(Temp, AircraftRego_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(AircraftRego_Config, Temp);
    }
    UpdateButtons();
}

static void OnAircraftTypeClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonAircraftType) {
        _tcscpy(Temp, AircraftType_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(AircraftType_Config, Temp);
    }
    UpdateButtons();
}

static void OnPilotNameClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonPilotName) {
        _tcscpy(Temp, PilotName_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(PilotName_Config, Temp);
    }
    UpdateButtons();
}

static void OnLiveTrackersrvClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackersrv) {
        _tcscpy(Temp, LiveTrackersrv_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(LiveTrackersrv_Config, Temp);
    }
    UpdateButtons();
}

static void OnLiveTrackerportClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackerport) {
        _stprintf(Temp, _T("%d"), LiveTrackerport_Config);
        dlgNumEntryShowModal(Temp, 100, false);
        TCHAR *sz = NULL;
        LiveTrackerport_Config = _tcstol(Temp, &sz, 10);
    }
    UpdateButtons();
}

static void OnLiveTrackerusrClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackerusr) {
        _tcscpy(Temp, LiveTrackerusr_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(LiveTrackerusr_Config, Temp);
    }
    UpdateButtons();
}

static void OnLiveTrackerpwdClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackerpwd) {
        _tcscpy(Temp, LiveTrackerpwd_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(LiveTrackerpwd_Config, Temp);
    }
    UpdateButtons();
}

static void OnCompetitionClassClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonCompetitionClass) {
        _tcscpy(Temp, CompetitionClass_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(CompetitionClass_Config, Temp);
    }
    UpdateButtons();
}

static void OnCompetitionIDClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonCompetitionID) {
        _tcscpy(Temp, CompetitionID_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(CompetitionID_Config, Temp);
    }
    UpdateButtons();
}

static void OnAirspaceColoursClicked(WndButton* pWnd) {
    bool retval;
    retval = dlgAirspaceShowModal(true);
    if (retval) {
        requirerestart = true;
    }
}

static void OnSetTopologyClicked(WndButton* pWnd) {
    dlgTopologyShowModal();
}

static void OnMultimapsClicked(WndButton* pWnd) {
    dlgMultimapsShowModal();
}

static void OnSetCustomKeysClicked(WndButton* pWnd) {
    dlgCustomKeysShowModal();
}

static void OnSetCustomMenuClicked(WndButton* pWnd) {
    dlgCustomMenuShowModal();
}

static void OnSetBottomBarClicked(WndButton* pWnd) {
    dlgBottomBarShowModal();
}

static void OnSetInfoPagesClicked(WndButton* pWnd) {
    dlgInfoPagesShowModal();
}

static void OnTaskRulesClicked(WndButton* pWnd) {
    dlgTaskRules();
}

static void OnAirspaceWarningParamsClicked(WndButton* pWnd) {
    dlgAirspaceWarningParamsShowModal();
}

static void OnAirspaceModeClicked(WndButton* pWnd) {
    if (dlgAirspaceShowModal(false)) {
        requirerestart = true;
    }
}

static void OnNextClicked(WndButton* pWnd) {
    NextPage(+1);
}

static void OnPrevClicked(WndButton* pWnd) {
    NextPage(-1);
}

static void OnCloseClicked(WndButton* pWnd) {
    wf->SetModalResult(mrOK);
}

static int cpyInfoBox[10];

static int page2mode(void) {
  return config_page[configMode]-15;
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

static void OnCopy(WndButton* pWnd) {
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

static void OnPaste(WndButton* pWnd) {
  int mode = page2mode();
  TCHAR name[80];
  if ((mode<0)||(mode>3)||(cpyInfoBox[0]<0)) {
    return;
  }

  if(MessageBoxX(
	// LKTOKEN  _@M510_ = "Overwrite?" 
		 gettext(TEXT("_@M510_")),
	// LKTOKEN  _@M354_ = "InfoBox paste" 
		 gettext(TEXT("_@M354_")),
		 mbYesNo) == IdYes) {

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

static bool FormKeyDown(Window* pWnd, unsigned KeyCode) {
    Window * pBtn = NULL;

    switch (KeyCode & 0xffff) {
        case '6':
            pBtn = wf->FindByName(TEXT("cmdPrev"));
            NextPage(-1);
            break;
        case '7':
            pBtn = wf->FindByName(TEXT("cmdNext"));
            NextPage(+1);
            break;;
    }
    if (pBtn) {
        pBtn->SetFocus();
        return true;
    }

    return false;
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

extern void OnInfoBoxHelp(WindowControl * Sender);

static void OnWaypointNewClicked(WndButton* pWnd){

  // Cannot save waypoint if no file
  if ( WayPointList.size()<=NUMRESWP) {
	MessageBoxX(
	// LKTOKEN  _@M478_ = "No waypoint file selected, cannot save." 
	gettext(TEXT("_@M478_")),
	// LKTOKEN  _@M457_ = "New Waypoint" 
               gettext(TEXT("_@M457_")),
               mbOk);

	return; 
  }
 
  WAYPOINT edit_waypoint;
  edit_waypoint.Latitude = GPS_INFO.Latitude;
  edit_waypoint.Longitude = GPS_INFO.Longitude;

  WaypointAltitudeFromTerrain(&edit_waypoint);
  if (!SIMMODE) {
	// If we have a real fix and a real altiude, adopt it if terrain is lower.
	// Since we dont create waypoints in flight, we are on ground there. We assume this.
	if (GPS_INFO.Altitude >edit_waypoint.Altitude)
		edit_waypoint.Altitude=GPS_INFO.Altitude;
  }
  edit_waypoint.FileNum = 0; // default, put into primary waypoint file
  edit_waypoint.Flags = 0;
  edit_waypoint.Comment=(TCHAR*)malloc((COMMENT_SIZE+1)*sizeof(TCHAR));

  extern void MSG_NotEnoughMemory(void);
  if (edit_waypoint.Comment == (TCHAR *)NULL) {
	MSG_NotEnoughMemory();
	return;
  }
  _tcscpy(edit_waypoint.Comment,_T(""));

  edit_waypoint.Name[0] = 0;
  edit_waypoint.Details = 0;
  edit_waypoint.Number = WayPointList.size();
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
    if(AddWaypoint(std::move(edit_waypoint))) {
      waypointneedsave = true;
    } 
    UnlockTaskData();
  }
}


static void OnWaypointEditClicked(WndButton* pWnd){

    int res;
  if (CheckClubVersion()) {
	ClubForbiddenMsg();
	return;
  }
  res = dlgWayPointSelect();
  if (res != -1){
	#if 0 // 101214 READ ONLY FILES
	if ( WayPointList[res].Format == LKW_COMPE) {      // 100212
		MessageBoxX(
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
		gettext(TEXT("_@M716_")),
	// LKTOKEN  _@M194_ = "CompeGPS Waypoint" 
                gettext(TEXT("_@M194_")),
                mbOk);

		return;
	}
	#endif

	if ( WayPointList[res].Format == LKW_VIRTUAL) {      // 100212
		MessageBoxX(
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
		gettext(TEXT("_@M716_")),
	// LKTOKEN  _@M775_ = "VIRTUAL Waypoint" 
                gettext(TEXT("_@M775_")),
                mbOk);

		return;
	}


    dlgWaypointEditShowModal(&WayPointList[res]);
    waypointneedsave = true;
  }
}

static void AskWaypointSave(void) {
  if (WaypointsOutOfRange==2) {

    if(MessageBoxX(
	// LKTOKEN  _@M810_ = "Waypoints excluded, save anyway?" 
                   gettext(TEXT("_@M810_")),
	// LKTOKEN  _@M811_ = "Waypoints outside terrain" 
                   gettext(TEXT("_@M811_")),
                   mbYesNo) == IdYes) {
      
      WaypointWriteFiles();
      
///      WAYPOINTFILECHANGED= true;
      
    }
  } else {
    
    WaypointWriteFiles();
    
///    WAYPOINTFILECHANGED= true;
  }
  waypointneedsave = false;
}

static void OnWaypointSaveClicked(WndButton* pWnd) {
    AskWaypointSave();
}

static void OnWaypointDeleteClicked(WndButton* pWnd){
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
		MessageBoxX(gettext(TEXT("_@M716_")),
	// LKTOKEN  _@M194_ = "CompeGPS Waypoint" 
			gettext(TEXT("_@M194_")),
			mbOk);
		return;
	} else 
	#endif
		if ( WayPointList[res].Format == LKW_VIRTUAL ) { // 100212
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
			MessageBoxX(gettext(TEXT("_@M716_")),
	// LKTOKEN  _@M775_ = "VIRTUAL Waypoint" 
				gettext(TEXT("_@M775_")),
				mbOk);
			return;
		} else 
	// LKTOKEN  _@M229_ = "Delete Waypoint?" 
	if(MessageBoxX(WayPointList[res].Name, gettext(TEXT("_@M229_")), 
	mbYesNo) == IdYes) {
		LockTaskData();
		WayPointList[res].FileNum = -1;
		UnlockTaskData();
		waypointneedsave = true;
		// Only for DELETE we shall reload the entire waypoints files
		// because we still do not have a "valid" flag for wpts.
		// Mostly because a text file is not a database!
		WAYPOINTFILECHANGED= true;
	}
  }
}
#ifndef NO_BLUETOOTH
static void OnBthDevice(WndButton* pWnd) {
    DlgBluetooth::Show();
    
    TCHAR szPort[MAX_PATH];
    ReadPort1Settings(szPort, NULL, NULL);
    UpdateComPortList((WndProperty*) wf->FindByName(TEXT("prpComPort1")), szPort);

    ReadPort2Settings(szPort, NULL, NULL);
    UpdateComPortList((WndProperty*) wf->FindByName(TEXT("prpComPort2")), szPort);
}
#endif

void ShowWindowControl(WndForm* pOwner, const TCHAR* WndName, bool bShow) {
    WindowControl* pWnd = wf->FindByName(WndName);
    if(pWnd) {
        pWnd->SetVisible(bShow);
    }
}

void UpdateComPortSetting(size_t idx, const TCHAR* szPortName) {
    const TCHAR* PortPropName[][2] = { 
        { _T("prpComSpeed1"), _T("prpComBit1") }, 
        { _T("prpComSpeed2"), _T("prpComBit2") }
    };
    const TCHAR * prpExtSound[2] = {
            _T("prpExtSound1"),
            _T("prpExtSound2")
        };

    LKASSERT(szPortName);
    // check if all array have same size ( compil time check );
    static_assert(array_size(DeviceList) == array_size(PortPropName), "PortPropName array size need to be same of DeviceList array size");
    static_assert(array_size(DeviceList) == array_size(prpExtSound), "prpExtSound array size need to be same of DeviceList array size");

#ifdef DISABLEEXTAUDIO    
    bool bManageExtAudio = false;
#else
    bool bManageExtAudio = true;
#endif
    bManageExtAudio &= IsSoundInit();
    bool bHide = (DeviceList[idx].Disabled || (_tcscmp(DeviceList[idx].Name, _T("Internal")) == 0));
    bool bBt = ((_tcslen(szPortName) > 3) && (_tcsncmp(szPortName, _T("BT:"), 3) == 0));

    // For com port properties, hide them for disable, internal or Bluetooth, show otherwise
    if (std::begin(PortPropName) + idx < std::end(PortPropName)) {
        std::for_each(
                std::begin(PortPropName[idx]),
                std::end(PortPropName[idx]),
                std::bind(ShowWindowControl, wf, _1, !(bHide || bBt))
                );
    }
    // Manage external sounds only if necessary
    if (bManageExtAudio) {
        ShowWindowControl(wf, prpExtSound[idx], !bHide);
    }
}

static void OnComPort1Data(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
        UpdateComPortSetting(0, Sender->GetAsString());
    break;
	default: 
		break;
  }
}

static void OnComPort2Data(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
        UpdateComPortSetting(1, Sender->GetAsString());
    break;
	default: 
		break;
  }
}


static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnAirspaceColoursClicked),
  ClickNotifyCallbackEntry(OnAirspaceModeClicked),
  ClickNotifyCallbackEntry(OnAirspaceWarningParamsClicked),
  DataAccessCallbackEntry(OnUTCData),
  ClickNotifyCallbackEntry(OnNextClicked),
  ClickNotifyCallbackEntry(OnPrevClicked),
  ClickNotifyCallbackEntry(OnSetupDeviceAClicked),
  ClickNotifyCallbackEntry(OnSetupDeviceBClicked),
  OnHelpCallbackEntry(OnInfoBoxHelp),
  ClickNotifyCallbackEntry(OnWaypointNewClicked),
  ClickNotifyCallbackEntry(OnWaypointDeleteClicked),
  ClickNotifyCallbackEntry(OnWaypointEditClicked),
  ClickNotifyCallbackEntry(OnWaypointSaveClicked),

  DataAccessCallbackEntry(OnDeviceAData),
  DataAccessCallbackEntry(OnDeviceBData),
  
  DataAccessCallbackEntry(OnComPort1Data),
  DataAccessCallbackEntry(OnComPort2Data),


  ClickNotifyCallbackEntry(OnSetTopologyClicked),
  ClickNotifyCallbackEntry(OnSetCustomKeysClicked),
  ClickNotifyCallbackEntry(OnMultimapsClicked),
  ClickNotifyCallbackEntry(OnSetCustomMenuClicked),
  ClickNotifyCallbackEntry(OnSetBottomBarClicked),
  ClickNotifyCallbackEntry(OnSetInfoPagesClicked),
  ClickNotifyCallbackEntry(OnTaskRulesClicked),
  
  DataAccessCallbackEntry(OnAirspaceFillType),
  DataAccessCallbackEntry(OnAirspaceDisplay),
  DataAccessCallbackEntry(OnAspPermModified),
  DataAccessCallbackEntry(OnLk8000ModeChange),
  DataAccessCallbackEntry(OnGearWarningModeChange),
#ifndef NO_BLUETOOTH
  ClickNotifyCallbackEntry(OnBthDevice),
#endif
  EndCallBackEntry()
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
    }
  }
}

static  int dwDeviceIndex1=0;
static  int dwDeviceIndex2=0;
static  TCHAR DeviceName[DEVNAMESIZE+1];
static  TCHAR temptext[MAX_PATH];

void UpdateComPortList(WndProperty* wp, LPCTSTR szPort) {
    if(COMMPort.empty()) {
        RefreshComPortList();
    }
    if (wp) {
        DataFieldEnum* dfe = (DataFieldEnum*) wp->GetDataField();
        if(dfe) {
            dfe->Clear();
            std::for_each(
            	COMMPort.begin(),
            	COMMPort.end(),
            	std::bind(&DataFieldEnum::addEnumText, dfe, std::bind(&COMMPortItem_t::GetLabel, _1))
            );
            COMMPort_t::iterator It = std::find_if(
                COMMPort.begin(), 
                COMMPort.end(), 
                std::bind(&COMMPortItem_t::IsSamePort, _1, szPort)
            );
            if(It != COMMPort.end()) {
                dfe->Set(std::distance(COMMPort.begin(), It));
            } else {
                dfe->Set(0);
            }
        }
        wp->RefreshDisplay();
    }
}

static void setVariables(void) {
  WndProperty *wp;

  LKASSERT(wf);

  TCHAR tsuf[10]; // 091101
  buttonPilotName = ((WndButton *)wf->FindByName(TEXT("cmdPilotName")));
  if (buttonPilotName) {
    buttonPilotName->SetOnClickNotify(OnPilotNameClicked);
  }
  buttonLiveTrackersrv = ((WndButton *)wf->FindByName(TEXT("cmdLiveTrackersrv")));
  if (buttonLiveTrackersrv) {
    buttonLiveTrackersrv->SetOnClickNotify(OnLiveTrackersrvClicked);
  }
  buttonLiveTrackerport = ((WndButton *)wf->FindByName(TEXT("cmdLiveTrackerport")));
  if (buttonLiveTrackerport) {
    buttonLiveTrackerport->SetOnClickNotify(OnLiveTrackerportClicked);
  }
  buttonLiveTrackerusr = ((WndButton *)wf->FindByName(TEXT("cmdLiveTrackerusr")));
  if (buttonLiveTrackerusr) {
    buttonLiveTrackerusr->SetOnClickNotify(OnLiveTrackerusrClicked);
  }
  buttonLiveTrackerpwd = ((WndButton *)wf->FindByName(TEXT("cmdLiveTrackerpwd")));
  if (buttonLiveTrackerpwd) {
    buttonLiveTrackerpwd->SetOnClickNotify(OnLiveTrackerpwdClicked);
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


  const TCHAR *tSpeed[] = {TEXT("1200"),TEXT("2400"),TEXT("4800"),TEXT("9600"),
			     TEXT("19200"),TEXT("38400"),TEXT("57600"),TEXT("115200")};

  TCHAR szPort[MAX_PATH];
  
  ReadPort1Settings(szPort,NULL, NULL);
  UpdateComPortList((WndProperty*)wf->FindByName(TEXT("prpComPort1")), szPort);

  wp = (WndProperty*)wf->FindByName(TEXT("prpComSpeed1"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    std::for_each(std::begin(tSpeed), std::end(tSpeed), std::bind(&DataFieldEnum::addEnumText, dfe, _1));
    
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
  wp = (WndProperty*)wf->FindByName(TEXT("prpExtSound1"));
  if (wp) {
    wp->GetDataField()->Set(UseExtSound1);
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
    for (int i=0; i<DeviceRegisterCount; i++) {
      devRegisterGetName(i, DeviceName);
      dfe->addEnumText((DeviceName));

      if (_tcscmp(DeviceName, deviceName1) == 0)
        dwDeviceIndex1 = i;

    }
    dfe->Sort(3);
    dfe->Set(dwDeviceIndex1);
    wp->RefreshDisplay();
  }


  ReadPort2Settings(szPort,NULL, NULL);

  UpdateComPortList((WndProperty*)wf->FindByName(TEXT("prpComPort2")), szPort);

  wp = (WndProperty*)wf->FindByName(TEXT("prpComSpeed2"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    std::for_each(std::begin(tSpeed), std::end(tSpeed), std::bind(&DataFieldEnum::addEnumText, dfe, _1));

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

  wp = (WndProperty*)wf->FindByName(TEXT("prpExtSound2"));
  if (wp) {
    wp->GetDataField()->Set(UseExtSound2);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComDevice2"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    for (int i=0; i<DeviceRegisterCount; i++) {
      devRegisterGetName(i, DeviceName);
      dfe->addEnumText((DeviceName));
      if (_tcscmp(DeviceName, deviceName2) == 0)
        dwDeviceIndex2 = i;
    }
    dfe->Sort(3);
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
    DataFieldEnum* dfe = (DataFieldEnum*)wp->GetDataField();
    if(dfe) {
        dfe->addEnumText(gettext(TEXT("_@M941_")));
#ifdef HAVE_HATCHED_BRUSH
        dfe->addEnumText(gettext(TEXT("_@M942_")));
        dfe->addEnumText(gettext(TEXT("_@M945_")));
#endif
        if (LKSurface::AlphaBlendSupported()) {
            dfe->addEnumText(gettext(TEXT("_@M943_")));
            dfe->addEnumText(gettext(TEXT("_@M946_")));
        }
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
    if (!LKSurface::AlphaBlendSupported()) {
        dfe->Set(100 / 5);
        wp->SetReadOnly(true);
    } else {
        dfe->Set(BarOpacity / 5);
    }
    wp->RefreshDisplay();
  }

  //
  // Font manager
  //
  wp = (WndProperty*)wf->FindByName(TEXT("prpFontMapWaypoint"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("-5"));
    dfe->addEnumText(TEXT("-4"));
    dfe->addEnumText(TEXT("-3"));
    dfe->addEnumText(TEXT("-2"));
    dfe->addEnumText(TEXT("-1"));
    dfe->addEnumText(TEXT("0"));
    dfe->addEnumText(TEXT("+1"));
    dfe->addEnumText(TEXT("+2"));
    dfe->addEnumText(TEXT("+3"));
    dfe->addEnumText(TEXT("+4"));
    dfe->addEnumText(TEXT("+5"));
    dfe->Set(FontMapWaypoint);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontMapTopology"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("-5"));
    dfe->addEnumText(TEXT("-4"));
    dfe->addEnumText(TEXT("-3"));
    dfe->addEnumText(TEXT("-2"));
    dfe->addEnumText(TEXT("-1"));
    dfe->addEnumText(TEXT("0"));
    dfe->addEnumText(TEXT("+1"));
    dfe->addEnumText(TEXT("+2"));
    dfe->addEnumText(TEXT("+3"));
    dfe->addEnumText(TEXT("+4"));
    dfe->addEnumText(TEXT("+5"));
    dfe->Set(FontMapTopology);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontInfopage1L"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("-5"));
    dfe->addEnumText(TEXT("-4"));
    dfe->addEnumText(TEXT("-3"));
    dfe->addEnumText(TEXT("-2"));
    dfe->addEnumText(TEXT("-1"));
    dfe->addEnumText(TEXT("0"));
    dfe->addEnumText(TEXT("+1"));
    dfe->addEnumText(TEXT("+2"));
    dfe->addEnumText(TEXT("+3"));
    dfe->addEnumText(TEXT("+4"));
    dfe->addEnumText(TEXT("+5"));
    dfe->Set(FontInfopage1L);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontBottomBar"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("-5"));
    dfe->addEnumText(TEXT("-4"));
    dfe->addEnumText(TEXT("-3"));
    dfe->addEnumText(TEXT("-2"));
    dfe->addEnumText(TEXT("-1"));
    dfe->addEnumText(TEXT("0"));
    dfe->addEnumText(TEXT("+1"));
    dfe->addEnumText(TEXT("+2"));
    dfe->addEnumText(TEXT("+3"));
    dfe->addEnumText(TEXT("+4"));
    dfe->addEnumText(TEXT("+5"));
    dfe->Set(FontBottomBar);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayTarget"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("-5"));
    dfe->addEnumText(TEXT("-4"));
    dfe->addEnumText(TEXT("-3"));
    dfe->addEnumText(TEXT("-2"));
    dfe->addEnumText(TEXT("-1"));
    dfe->addEnumText(TEXT("0"));
    dfe->addEnumText(TEXT("+1"));
    dfe->addEnumText(TEXT("+2"));
    dfe->addEnumText(TEXT("+3"));
    dfe->addEnumText(TEXT("+4"));
    dfe->addEnumText(TEXT("+5"));
    dfe->Set(FontOverlayMedium);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayValues"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("-5"));
    dfe->addEnumText(TEXT("-4"));
    dfe->addEnumText(TEXT("-3"));
    dfe->addEnumText(TEXT("-2"));
    dfe->addEnumText(TEXT("-1"));
    dfe->addEnumText(TEXT("0"));
    dfe->addEnumText(TEXT("+1"));
    dfe->addEnumText(TEXT("+2"));
    dfe->addEnumText(TEXT("+3"));
    dfe->addEnumText(TEXT("+4"));
    dfe->addEnumText(TEXT("+5"));
    dfe->Set(FontOverlayBig);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUseTwoLines"));
  if (wp) {
    wp->GetDataField()->Set(UseTwoLines);
    if (ScreenLandscape) wp->SetReadOnly(true);
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
    #ifdef __linux__
    wp->SetVisible(false);
    #endif
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpAspPermDisable"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("_@M968_"))); // _@M968_ "for this time only"
    dfe->addEnumText(gettext(TEXT("_@M969_"))); // _@M969_ "permanent"
    dfe->Set(AspPermanentChanged);
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
    wp->GetDataField()->SetAsFloat(iround(UTCOffset/900.0)/4.0);
    wp->RefreshDisplay();
  }
  SetLocalTime();

  wp = (WndProperty*)wf->FindByName(TEXT("prpClipAltitude"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(ClipAltitude*ALTITUDEMODIFY/10));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }
    
  wp = (WndProperty*)wf->FindByName(TEXT("prpAltWarningMargin"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(AltWarningMargin*ALTITUDEMODIFY/10));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoZoom"));
  if (wp) {
    wp->GetDataField()->Set(AutoZoom_Config);
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
    wp->GetDataField()->SetAsFloat(debounceTimeout.totalMilliseconds());
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
    wp->GetDataField()->SetAsFloat(iround(ALTITUDEMODIFY*SAFETYALTITUDEARRIVAL/10));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeTerrain"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(ALTITUDEMODIFY*SAFETYALTITUDETERRAIN/10));
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
    #ifdef GTL2
        // "Line+NextWP"
    dfe->addEnumText(gettext(TEXT("_@M1805_")));
        // "Shade+NextWP"
    dfe->addEnumText(gettext(TEXT("_@M1806_")));
    #endif
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
  wp = (WndProperty*)wf->FindByName(TEXT("prpSonarWarning"));
  if (wp) {
    wp->GetDataField()->Set(SonarWarning_Config);
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
    dfe->addEnumText(MsgToken(1793)); // External

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
    wp->GetDataField()->SetAsFloat(DISTANCEMODIFY*PGAutoZoomThreshold);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    // if (!ISPARAGLIDER) wp->SetVisible(false); 
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoOrientScale"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AutoOrientScale);
    wp->RefreshDisplay();
  }
  
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
    dfe->addEnumText(TEXT("UTM"));
    dfe->Set(Units::CoordinateFormat);
    wp->RefreshDisplay();
  }

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

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsAltitude"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("feet")));
    dfe->addEnumText(gettext(TEXT("meters")));
    dfe->Set(AltitudeUnit_Config);
    wp->RefreshDisplay();
  }

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

  wp = (WndProperty*)wf->FindByName(TEXT("prpSaveRuntime"));
  if (wp) {
    wp->GetDataField()->Set(SaveRuntime);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDisableAutoLogger"));
  if (wp) {
    wp->GetDataField()->Set(!DisableAutoLogger);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLiveTrackerInterval"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(LiveTrackerInterval));
    wp->GetDataField()->SetUnits(TEXT("sec"));
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
    _stprintf(tsuf,_T("*%s"),_T(LKS_PILOT));
    dfe->ScanDirectoryTop(_T(LKD_CONF),tsuf);
    dfe->Set(0);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAircraftFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),_T(LKS_AIRCRAFT));
    dfe->ScanDirectoryTop(_T(LKD_CONF),tsuf);
    dfe->Set(0);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpDeviceFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),_T(LKS_DEVICE));
    dfe->ScanDirectoryTop(_T(LKD_CONF),tsuf);
    dfe->Set(0);
    wp->RefreshDisplay();
  }

  if (_tcscmp(szPolarFile,_T(""))==0) 
    _tcscpy(temptext,_T("%LOCAL_PATH%\\\\_Polars\\Default.plr"));
  else
    _tcscpy(temptext,szPolarFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpPolarFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),_T(LKS_POLARS));
    dfe->ScanDirectoryTop(_T(LKD_POLARS),tsuf); //091101
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szAirspaceFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),_T(LKS_AIRSPACES));
    dfe->ScanDirectoryTop(_T(LKD_AIRSPACES),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szAdditionalAirspaceFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),_T(LKS_AIRSPACES));
    dfe->ScanDirectoryTop(_T(LKD_AIRSPACES),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szWaypointFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),_T(LKS_WP_WINPILOT));
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%s"),_T(LKS_WP_XCSOAR));
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%s"),_T(LKS_WP_CUP));
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%s"),_T(LKS_WP_COMPE));
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szAdditionalWaypointFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),(LKS_WP_WINPILOT));
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%s"),(LKS_WP_XCSOAR));
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%s"),_T(LKS_WP_CUP));
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%s"),_T(LKS_WP_COMPE));
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szMapFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpMapFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),_T(LKS_MAPS));
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
    _stprintf(tsuf,_T("*%s"),_T(XCS_MAPS));
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szTerrainFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),_T(LKS_TERRAINDEM));
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
#if LKMTERRAIN
    _stprintf(tsuf,_T("*%s"),_T(LKS_TERRAINDAT));
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
#endif
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szAirfieldFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAirfieldFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),_T(LKS_AIRFIELDS));
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szLanguageFile);
  if (_tcslen(temptext)==0) {
	_tcscpy(temptext,_T("%LOCAL_PATH%\\\\_Language\\ENGLISH.LNG"));
  }
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpLanguageFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),_T(LKS_LANGUAGE));
    dfe->ScanDirectoryTop(_T(LKD_LANGUAGE),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szInputFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpInputFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%s"),_T(LKS_INPUT));
    dfe->ScanDirectoryTop(_T(LKD_CONF),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

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
    dfe->addEnumText(TEXT("Holux FunTrek GM-130 / GM-132"));
    dfe->addEnumText(TEXT("Medion S3747 / Royaltek BV-3200"));
    dfe->addEnumText(TEXT("LX MiniMap"));
    dfe->addEnumText(TEXT("Keyboard A"));
    dfe->addEnumText(TEXT("KeyBoard B"));
    dfe->addEnumText(TEXT("KeyBoard C"));
    dfe->addEnumText(TEXT("KeyBoard 1"));
    dfe->addEnumText(TEXT("KeyBoard 2"));
    dfe->addEnumText(TEXT("KeyBoard 3"));

    dfe->Set(GlobalToBoxType(GlobalModelType));
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAircraftCategory"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M328_ = "Glider" 
    dfe->addEnumText(gettext(TEXT("_@M328_")));
	// LKTOKEN  _@M520_ = "Paraglider/Delta" 
    dfe->addEnumText(gettext(TEXT("_@M520_")));
	// LKTOKEN  _@M163_ = "Car" 
    dfe->addEnumText(gettext(TEXT("_@M2148_")));
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
	// LKTOKEN  _@M333_ = "Half overlay" 
     dfe->addEnumText(gettext(TEXT("_@M333_")));
	// LKTOKEN  _@M311_ = "Full overlay" 
    	dfe->addEnumText(gettext(TEXT("_@M311_")));
     dfe = (DataFieldEnum*)wp->GetDataField(); // see above
     dfe->Set(Look8000-lxcStandard);
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
    DataField* dfe = wp->GetDataField();
    if(dfe) {
        dfe->Set(HideUnits);
    }
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpBestWarning")); // 091122
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if(dfe) {
        dfe->Set(BestWarning);
    }
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpUseTotalEnergy"));
  if (wp) {
    DataField * dfe = wp->GetDataField();
    if(dfe) {
        dfe->Set(UseTotalEnergy_Config);
    }
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpUseUngestures"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    LKASSERT(dfe);
    if(dfe) {
        dfe->Set(UseUngestures);
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpThermalBar")); // 091122
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  +_@M2149_ = "in thermal"
    dfe->addEnumText(gettext(TEXT("_@M2149_")));
	// LKTOKEN  +_@M2150_ = "in thermal and cruise"
    dfe->addEnumText(gettext(TEXT("_@M2150_")));
    dfe->addEnumText(gettext(TEXT("_@M1833_"))); // Always
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(ThermalBar);
    wp->RefreshDisplay();
  }

  // This is updated also from DoLook8000ModeChange function
  // These are only the initial startup values
  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayClock"));
  if (wp) {
    if (Look8000==lxcStandard) {
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
    DataField* dfe = wp->GetDataField();
    if(dfe) {
        dfe->Set(PGOptimizeRoute_Config);
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTrackBar")); // 091122
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if(dfe) {
        dfe->Set(TrackBar);
    }
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
    dfe->Set(OutlinedTp_Config);
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


  wp = (WndProperty*)wf->FindByName(TEXT("prpAppIndLandable"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(MsgToken(1797)); // Vector
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
    dfe->addEnumText(TEXT("LiteAlps"));
    dfe->Set(TerrainRamp_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoBacklight")); // VENTA4
  if (wp) {
    wp->SetVisible(true); // 091115 changed to true
    wp->GetDataField()->Set(EnableAutoBacklight);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoSoundVolume")); // VENTA4
  if (wp) {
    wp->SetVisible(true); // 091115 changed to true
#if ( WINDOWSPC==0 )
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
    dfe->addEnumText(gettext(TEXT("_@M393_")));
    
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
    dfe->addEnumText(MsgToken(418)); // Manual
    dfe->addEnumText(MsgToken(897)); // Auto
    dfe->addEnumText(MsgToken(97)); // Arm
    dfe->addEnumText(MsgToken(96)); // Arm start
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
  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmTakeoffSafety"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(AlarmTakeoffSafety*ALTITUDEMODIFY/1000));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmGearWarning"));
  if (wp)
  {
	DataFieldEnum* dfe;
	dfe = (DataFieldEnum*)wp->GetDataField();
	dfe->addEnumText(gettext(TEXT("_@M1831_")));	// LKTOKEN  _@M1831_ "Off"
	dfe->addEnumText(gettext(TEXT("_@M1832_")));	// LKTOKEN  _@M1832_ "Near landables"
	dfe->addEnumText(gettext(TEXT("_@M1833_")));	// LKTOKEN   _@M1833_ "Allways"
    dfe->Set(GearWarningMode);
    wp->RefreshDisplay();
  }

#if 0  // no gear warning, just in case
{
GearWarningMode =0;
GearWarningAltitude=0;
wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmGearWarning"));
wp->SetVisible(false);
wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmGearAltitude"));
wp->SetVisible(false);
wp->RefreshDisplay();
}
#else
  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmGearAltitude"));

  if (wp) {
	if(GearWarningMode == 0)
	{
	  wp->SetVisible(false);
	}
	else
	{
      wp->GetDataField()->SetAsFloat(iround(GearWarningAltitude*ALTITUDEMODIFY/1000));
      wp->GetDataField()->SetUnits(Units::GetAltitudeName());
	  wp->SetVisible(true);
      wp->RefreshDisplay();
    }
    wp->RefreshDisplay();
  }
#endif

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

  for (int i=0; i<4; i++) {
    for (int j=0; j<8; j++) {
      SetInfoBoxSelector(j, i);
    }
  }
}




void dlgConfigurationShowModal(short mode){

  WndProperty *wp;

  configMode=mode;

  static const TCHAR* dlgTemplate_L [][2] = { 
      { TEXT("dlgConfiguration_L.xml"),  TEXT("IDR_XML_CONFIGURATION_L") },
      { TEXT("dlgConfigPilot_L.xml"), TEXT("IDR_XML_CONFIGPILOT_L") },
      { TEXT("dlgConfigAircraft_L.xml"), TEXT("IDR_XML_CONFIGAIRCRAFT_L") },
      { TEXT("dlgConfigDevice_L.xml"), TEXT("IDR_XML_CONFIGDEVICE_L") }
  };
  static const TCHAR* dlgTemplate_P [][2] = { 
      { TEXT("dlgConfiguration_P.xml"),  TEXT("IDR_XML_CONFIGURATION_P") },
      { TEXT("dlgConfigPilot_P.xml"), TEXT("IDR_XML_CONFIGPILOT_P") },
      { TEXT("dlgConfigAircraft_P.xml"), TEXT("IDR_XML_CONFIGAIRCRAFT_P") },
      { TEXT("dlgConfigDevice_P.xml"), TEXT("IDR_XML_CONFIGDEVICE_P") }
  };
  static_assert(array_size(dlgTemplate_L) == array_size(dlgTemplate_P), "check array size");
  
  StartHourglassCursor(); 

  if(configMode >= 0 && configMode < (int)array_size(dlgTemplate_L)) {
    
      auto dlgTemplate = ScreenLandscape ? dlgTemplate_L[configMode] : dlgTemplate_P[configMode];
    TCHAR filename[MAX_PATH];
  
    LocalPathS(filename,  dlgTemplate[0]);
    wf = dlgLoadFromXML(CallBackTable, filename, dlgTemplate[1]);
    
  } else {
      wf = nullptr;
  }
  
  if (!wf) {
	return;
  }
  
  wf->SetKeyDownNotify(FormKeyDown);

  LKASSERT((WndButton *)wf->FindByName(TEXT("cmdClose")));
  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  reset_wConfig();

  if (configMode==0) {

	wConfig1    = ((WndFrame *)wf->FindByName(TEXT("frmSite")));
	wConfig2    = ((WndFrame *)wf->FindByName(TEXT("frmAirspace")));
	wConfig3    = ((WndFrame *)wf->FindByName(TEXT("frmDisplay")));
	wConfig4    = ((WndFrame *)wf->FindByName(TEXT("frmTerrain")));
	wConfig5    = ((WndFrame *)wf->FindByName(TEXT("frmFinalGlide")));
	wConfig6    = ((WndFrame *)wf->FindByName(TEXT("frmSafety")));
	wConfig7    = ((WndFrame *)wf->FindByName(TEXT("frmEmpty")));
	wConfig8    = ((WndFrame *)wf->FindByName(TEXT("frmEmpty")));
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

	LKASSERT(wConfig1);
	LKASSERT(wConfig2);
	LKASSERT(wConfig3);
	LKASSERT(wConfig4);
	LKASSERT(wConfig5);
	LKASSERT(wConfig6);
	LKASSERT(wConfig7);
	LKASSERT(wConfig8);
	LKASSERT(wConfig9);
	LKASSERT(wConfig10);
	LKASSERT(wConfig11);
	LKASSERT(wConfig12);
	LKASSERT(wConfig13);
	LKASSERT(wConfig14);
	LKASSERT(wConfig15);
	LKASSERT(wConfig16);
	LKASSERT(wConfig17);
	LKASSERT(wConfig18);
	LKASSERT(wConfig19);
	LKASSERT(wConfig20);
	LKASSERT(wConfig21);
	LKASSERT(wConfig22);
	LKASSERT(wConfig23);
	LKASSERT(wConfig24);
	LKASSERT(wConfig25);

	numPages=25;

	for (int item=0; item<10; item++) {
		cpyInfoBox[item] = -1;
	}
  }
  if (configMode==1) {
	wConfig1 = ((WndFrame *)wf->FindByName(TEXT("frmPilot")));
	numPages=1;
	LKASSERT(wConfig1);
  }
  if (configMode==2) {
	wConfig1 = ((WndFrame *)wf->FindByName(TEXT("frmPolar")));
	numPages=1;
	LKASSERT(wConfig1);
  }
  if (configMode==3) {
	wConfig1 = ((WndFrame *)wf->FindByName(TEXT("frmComm")));
	numPages=1;
	LKASSERT(wConfig1);
  }

  wf->FilterAdvanced(1); // useless, we dont use advanced options anymore TODO remove

  setVariables();

  if (mode==3) {
	TCHAR deviceName1[MAX_PATH];
	TCHAR deviceName2[MAX_PATH];
	ReadDeviceSettings(0, deviceName1);
	ReadDeviceSettings(1, deviceName2);
	UpdateDeviceSetupButton(0, deviceName1);
	UpdateDeviceSetupButton(1, deviceName2);
// Don't show external sound config if not compiled for this device or not used
#ifdef DISABLEEXTAUDIO
        ShowWindowControl(wf, _T("prpExtSound1"), false);
        ShowWindowControl(wf, _T("prpExtSound2"), false);
#else
        if (!IsSoundInit()) {
            ShowWindowControl(wf, _T("prpExtSound1"), false);
            ShowWindowControl(wf, _T("prpExtSound2"), false);
        }
#endif
  }

  NextPage(0);

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
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLiveTrackerInterval"));
  if (wp) {
    if (LiveTrackerInterval != (int)wp->GetDataField()->GetAsFloat()) {
      LiveTrackerInterval = (int)(wp->GetDataField()->GetAsFloat());
      requirerestart = true;
    }
  }

  double val;

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyMacCready"));
  if (wp) {
    val = wp->GetDataField()->GetAsFloat()/LIFTMODIFY;
    if (GlidePolar::SafetyMacCready != val) {
      GlidePolar::SafetyMacCready = val;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSetSystemTimeFromGPS"));
  if (wp) {
    if (SetSystemTimeFromGPS != wp->GetDataField()->GetAsBoolean()) {
      SetSystemTimeFromGPS = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSaveRuntime"));
  if (wp) {
    if (SaveRuntime != wp->GetDataField()->GetAsBoolean()) {
      SaveRuntime = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTrailDrift"));
  if (wp) {
    if (EnableTrailDrift_Config != wp->GetDataField()->GetAsBoolean()) {
      EnableTrailDrift_Config = wp->GetDataField()->GetAsBoolean();
      MapWindow::EnableTrailDrift = EnableTrailDrift_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpThermalLocator"));
  if (wp) {
    if (EnableThermalLocator != wp->GetDataField()->GetAsInteger()) {
      EnableThermalLocator = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTrail"));
  if (wp) {
    if (TrailActive_Config != wp->GetDataField()->GetAsInteger()) {
      TrailActive_Config = wp->GetDataField()->GetAsInteger();
      TrailActive = TrailActive_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKMaxLabels"));
  if (wp) {
    if (LKMaxLabels != wp->GetDataField()->GetAsInteger()) {
      LKMaxLabels = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBugs"));
  if (wp) {
    if (BUGS_Config != wp->GetDataField()->GetAsFloat()/100.0) {
      BUGS_Config = wp->GetDataField()->GetAsFloat()/100.0;
      CheckSetBugs(BUGS_Config);
      requirerestart=true;
    }
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpPGCruiseZoom"));
  if (wp) {
    if ( PGCruiseZoom != wp->GetDataField()->GetAsInteger()) {
      PGCruiseZoom = wp->GetDataField()->GetAsInteger();
      MapWindow::zoom.Reset();
        requirerestart=true;
    }
  }

double dval;

  wp = (WndProperty*)wf->FindByName(TEXT("prpPGAutoZoomThreshold"));
  if (wp) {
    dval = wp->GetDataField()->GetAsFloat()/DISTANCEMODIFY;
    if (PGAutoZoomThreshold != dval) {
      PGAutoZoomThreshold = dval;
    }
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGClimbZoom"));
  if (wp) {
    if ( PGClimbZoom != wp->GetDataField()->GetAsInteger()) {
      PGClimbZoom = wp->GetDataField()->GetAsInteger();
      MapWindow::zoom.Reset();
        requirerestart=true; 
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoOrientScale"));
  if (wp) {
    if ( AutoOrientScale != wp->GetDataField()->GetAsFloat()) {
      AutoOrientScale = wp->GetDataField()->GetAsFloat();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceDisplay"));
  if (wp) {
    if (AltitudeMode_Config != wp->GetDataField()->GetAsInteger()) {
      AltitudeMode_Config = wp->GetDataField()->GetAsInteger();
      AltitudeMode = AltitudeMode_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeMode"));
  if (wp) {
    if (SafetyAltitudeMode != wp->GetDataField()->GetAsInteger()) {
      SafetyAltitudeMode = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLockSettingsInFlight"));
  if (wp) {
    if (LockSettingsInFlight != 
	wp->GetDataField()->GetAsBoolean()) {
      LockSettingsInFlight = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLoggerShortName"));
  if (wp) {
    if (LoggerShortName != 
	wp->GetDataField()->GetAsBoolean()) {
      LoggerShortName = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableFLARMMap"));
  if (wp) {
    if ((int)EnableFLARMMap != 
	wp->GetDataField()->GetAsInteger()) {
      EnableFLARMMap = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDebounceTimeout"));
  if (wp) {
    if (debounceTimeout != wp->GetDataField()->GetAsInteger()) {
      debounceTimeout = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceFillType"));
  if (wp) {
    if (MapWindow::GetAirSpaceFillType() != wp->GetDataField()->GetAsInteger()) {
      MapWindow::SetAirSpaceFillType((MapWindow::EAirspaceFillType)wp->GetDataField()->GetAsInteger());
    }
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceOpacity"));
  if (wp) {
    if (MapWindow::GetAirSpaceOpacity() != wp->GetDataField()->GetAsInteger()*10) {
      MapWindow::SetAirSpaceOpacity(wp->GetDataField()->GetAsInteger() * 10);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBarOpacity"));
  if (wp) {
    if (BarOpacity != wp->GetDataField()->GetAsInteger()*5 ) {
	BarOpacity= wp->GetDataField()->GetAsInteger() * 5;
    }
  }

  
  wp = (WndProperty*)wf->FindByName(TEXT("prpFontMapWaypoint"));
  if (wp) {
      if (FontMapWaypoint != wp->GetDataField()->GetAsInteger() ) {
          FontMapWaypoint = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpFontMapTopology"));
  if (wp) {
      if (FontMapTopology != wp->GetDataField()->GetAsInteger() ) {
          FontMapTopology = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpFontInfopage1L"));
  if (wp) {
      if (FontInfopage1L != wp->GetDataField()->GetAsInteger() ) {
          FontInfopage1L = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontBottomBar"));
  if (wp) {
      if (FontBottomBar != wp->GetDataField()->GetAsInteger() ) {
          FontBottomBar = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayTarget"));
  if (wp) {
      if (FontOverlayMedium != wp->GetDataField()->GetAsInteger() ) {
          FontOverlayMedium = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayValues"));
  if (wp) {
      if (FontOverlayBig != wp->GetDataField()->GetAsInteger() ) {
          FontOverlayBig = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUseTwoLines"));
  if (wp) {
      if (UseTwoLines != wp->GetDataField()->GetAsBoolean()) {
          UseTwoLines = wp->GetDataField()->GetAsBoolean();
          Reset_Single_DoInits(MDI_DRAWNEAREST);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontRenderer"));
  if (wp) {
    if (FontRenderer != wp->GetDataField()->GetAsInteger() ) 
    {
	    FontRenderer = wp->GetDataField()->GetAsInteger();
      requirerestart = true;
    }
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoZoom"));
  if (wp) {
    if (AutoZoom_Config != 
	wp->GetDataField()->GetAsBoolean()) {
      AutoZoom_Config = wp->GetDataField()->GetAsBoolean();
      MapWindow::zoom.AutoZoom(AutoZoom_Config);
    }
  }

int ival; 

  wp = (WndProperty*)wf->FindByName(TEXT("prpUTCOffset"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsFloat()*3600.0);
    if ((UTCOffset != ival)||(utcchanged)) {
      UTCOffset = ival;

    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpClipAltitude"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY*10);
    if (ClipAltitude != ival) {
      ClipAltitude = ival;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAltWarningMargin"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY*10);
    if (AltWarningMargin != ival) {
      AltWarningMargin = ival;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointLabels"));
  if (wp) {
    if (DisplayTextType != wp->GetDataField()->GetAsInteger()) {
      DisplayTextType = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpCirclingZoom"));
  if (wp) {
    if (MapWindow::zoom.CircleZoom() != wp->GetDataField()->GetAsBoolean()) {
      MapWindow::zoom.CircleZoom(wp->GetDataField()->GetAsBoolean());
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOrientation"));
  if (wp) {
    if (DisplayOrientation_Config != wp->GetDataField()->GetAsInteger()) {
      DisplayOrientation_Config = wp->GetDataField()->GetAsInteger();
      DisplayOrientation=DisplayOrientation_Config;
      MapWindow::SetAutoOrientation(true); // reset
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMenuTimeout"));
  if (wp) {
    if (MenuTimeout_Config != wp->GetDataField()->GetAsInteger()*2) {
      MenuTimeout_Config = wp->GetDataField()->GetAsInteger()*2;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeArrival"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY*10);
    if (SAFETYALTITUDEARRIVAL != ival) {
      SAFETYALTITUDEARRIVAL = ival;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeTerrain"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY*10);
    if (SAFETYALTITUDETERRAIN != ival) {
      SAFETYALTITUDETERRAIN = ival;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoWind"));
  if (wp) {
    if (AutoWindMode_Config != wp->GetDataField()->GetAsInteger()) {
      AutoWindMode_Config = wp->GetDataField()->GetAsInteger();
      AutoWindMode = AutoWindMode_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoMcMode"));
  if (wp) {
    if (AutoMcMode_Config != wp->GetDataField()->GetAsInteger()) {
      AutoMcMode_Config = wp->GetDataField()->GetAsInteger();
	AutoMcMode=AutoMcMode_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointsOutOfRange"));
  if (wp) {
    if (WaypointsOutOfRange != wp->GetDataField()->GetAsInteger()) {
      WaypointsOutOfRange = wp->GetDataField()->GetAsInteger();
      WAYPOINTFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoForceFinalGlide"));
  if (wp) {
    if (AutoForceFinalGlide != wp->GetDataField()->GetAsBoolean()) {
      AutoForceFinalGlide = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableNavBaroAltitude"));
  if (wp) {
    if (EnableNavBaroAltitude_Config != wp->GetDataField()->GetAsBoolean()) {
      EnableNavBaroAltitude_Config = wp->GetDataField()->GetAsBoolean();
      EnableNavBaroAltitude=EnableNavBaroAltitude_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOrbiter"));
  if (wp) {
    if (Orbiter_Config != wp->GetDataField()->GetAsBoolean()) {
      Orbiter_Config = wp->GetDataField()->GetAsBoolean();
      Orbiter=Orbiter_Config;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoMcStatus"));
  if (wp) {
    if (AutoMacCready_Config != wp->GetDataField()->GetAsInteger()) {
      AutoMacCready_Config = wp->GetDataField()->GetAsInteger();
      CALCULATED_INFO.AutoMacCready=AutoMacCready_Config;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpShading"));
  if (wp) {
    if (Shading_Config != wp->GetDataField()->GetAsBoolean()) {
      Shading_Config = wp->GetDataField()->GetAsBoolean();
      Shading=Shading_Config;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpSonarWarning"));
  if (wp) {
    if (SonarWarning_Config != wp->GetDataField()->GetAsBoolean()) {
      SonarWarning_Config = wp->GetDataField()->GetAsBoolean();
      SonarWarning=SonarWarning_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFinalGlideTerrain"));
  if (wp) {
    if (FinalGlideTerrain != wp->GetDataField()->GetAsInteger()) {
      FinalGlideTerrain = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsSpeed"));
  if (wp) {
    if ((int)SpeedUnit_Config != wp->GetDataField()->GetAsInteger()) {
      SpeedUnit_Config = wp->GetDataField()->GetAsInteger();
      Units::NotifyUnitChanged();
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLatLon"));
  if (wp) {
    if ((int)Units::CoordinateFormat != wp->GetDataField()->GetAsInteger()) {
      Units::CoordinateFormat = (CoordinateFormats_t)
        wp->GetDataField()->GetAsInteger();
      Units::NotifyUnitChanged();
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsTaskSpeed"));
  if (wp) {
    if ((int)TaskSpeedUnit_Config != wp->GetDataField()->GetAsInteger()) {
      TaskSpeedUnit_Config = wp->GetDataField()->GetAsInteger();
      Units::NotifyUnitChanged();
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsDistance"));
  if (wp) {
    if ((int)DistanceUnit_Config != wp->GetDataField()->GetAsInteger()) {
      DistanceUnit_Config = wp->GetDataField()->GetAsInteger();
      Units::NotifyUnitChanged();
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLift"));
  if (wp) {
    if ((int)LiftUnit_Config != wp->GetDataField()->GetAsInteger()) {
      LiftUnit_Config = wp->GetDataField()->GetAsInteger();
      Units::NotifyUnitChanged();
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsAltitude"));
  if (wp) {
    if ((int)AltitudeUnit_Config != wp->GetDataField()->GetAsInteger()) {
      AltitudeUnit_Config = wp->GetDataField()->GetAsInteger();
      Units::NotifyUnitChanged();
      requirerestart = true;
    }
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szWaypointFile)) {
      _tcscpy(szWaypointFile,temptext);
      WAYPOINTFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szAdditionalWaypointFile)) {
      _tcscpy(szAdditionalWaypointFile,temptext);
      WAYPOINTFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szAirspaceFile)) {
      _tcscpy(szAirspaceFile,temptext);
      AIRSPACEFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szAdditionalAirspaceFile)) {
      _tcscpy(szAdditionalAirspaceFile,temptext);
      AIRSPACEFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMapFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szMapFile)) {
      _tcscpy(szMapFile,temptext);
      MAPFILECHANGED= true;
      #if LKMTERRAIN
      TERRAINFILECHANGED= true; //for .xcm
      #endif
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szTerrainFile)) {
      _tcscpy(szTerrainFile,temptext);
      TERRAINFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirfieldFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szAirfieldFile)) {
      _tcscpy(szAirfieldFile,temptext);
      AIRFIELDFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLanguageFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szLanguageFile)) {
      _tcscpy(szLanguageFile,temptext);
      requirerestart = true; // restart needed for language load
      // LKReadLanguageFile(); // NO GOOD. MEMORY LEAKS PENDING
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpInputFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szInputFile)) {
      _tcscpy(szInputFile,temptext);
      requirerestart = true;
    }
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpWindCalcTime")); // 100113
  if (wp) {
	ival = wp->GetDataField()->GetAsInteger();
	if (WindCalcTime != ival) {
		WindCalcTime = ival;
	}
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpWindCalcSpeed"));
  if (wp) {
	ival = iround((wp->GetDataField()->GetAsInteger()/SPEEDMODIFY)*1000.0);
	WindCalcSpeed = ival;
	WindCalcSpeed=ival/1000.0;
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishLine"));
  if (wp) {
    if (FinishLine != wp->GetDataField()->GetAsInteger()) {
      FinishLine = wp->GetDataField()->GetAsInteger();
      taskchanged = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishRadius"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsFloat()/DISTANCEMODIFY);
    if ((int)FinishRadius != ival) {
      FinishRadius = ival;
      taskchanged = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartLine"));
  if (wp) {
    if (StartLine != wp->GetDataField()->GetAsInteger()) {
      StartLine = wp->GetDataField()->GetAsInteger();
      taskchanged = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartRadius"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsFloat()/DISTANCEMODIFY);
    if ((int)StartRadius != ival) {
      StartRadius = ival;
      taskchanged = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFAISector"));
  if (wp) {
    if ((int)SectorType != wp->GetDataField()->GetAsInteger()) {
      SectorType = wp->GetDataField()->GetAsInteger();
      taskchanged = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskSectorRadius"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsFloat()/DISTANCEMODIFY);
    if ((int)SectorRadius != ival) {
      SectorRadius = ival;
      taskchanged = true;
    }
  }


  #if (0)
  wp = (WndProperty*)wf->FindByName(TEXT("prpAltArrivMode"));
  if (wp) {
    if (AltArrivMode != (AltArrivMode_t)
	(wp->GetDataField()->GetAsInteger())) {
      AltArrivMode = (AltArrivMode_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }
  #endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpCheckSum")); 
  if (wp) {
    if (CheckSum != (CheckSum_t)
	(wp->GetDataField()->GetAsInteger())) {
      CheckSum = (CheckSum_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIphoneGestures")); 
  if (wp) {
    if (IphoneGestures != (IphoneGestures_t) (wp->GetDataField()->GetAsInteger())) {
      IphoneGestures = (IphoneGestures_t) (wp->GetDataField()->GetAsInteger());
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPollingMode")); 
  if (wp) {
    if (PollingMode != (PollingMode_t) (wp->GetDataField()->GetAsInteger())) {
      PollingMode = (PollingMode_t) (wp->GetDataField()->GetAsInteger());
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKVarioBar")); 
  if (wp) {
    if (LKVarioBar != (LKVarioBar_t)
	(wp->GetDataField()->GetAsInteger())) {
      LKVarioBar = (LKVarioBar_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLKVarioVal")); 
  if (wp) {
    if (LKVarioVal != (LKVarioVal_t)
	(wp->GetDataField()->GetAsInteger())) {
      LKVarioVal = (LKVarioVal_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpHideUnits"));
  if (wp) {
      if (HideUnits != (HideUnits_t) (wp->GetDataField()->GetAsInteger())) {
          HideUnits = (HideUnits_t) (wp->GetDataField()->GetAsInteger());
          Reset_Single_DoInits(MDI_DRAWBOTTOMBAR);
          Reset_Single_DoInits(MDI_DRAWFLIGHTMODE);
      }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpDeclutterMode")); // VENTA10
  if (wp) {
    if (DeclutterMode != (DeclutterMode_t)
	(wp->GetDataField()->GetAsInteger())) {
      DeclutterMode = (DeclutterMode_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBestWarning"));
  if (wp) {
    if (BestWarning != (wp->GetDataField()->GetAsInteger())) {
      BestWarning = (wp->GetDataField()->GetAsInteger());
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpUseTotalEnergy"));
  if (wp) {
    if (UseTotalEnergy_Config != (wp->GetDataField()->GetAsInteger())) {
      UseTotalEnergy_Config = (wp->GetDataField()->GetAsInteger());
      UseTotalEnergy=UseTotalEnergy_Config;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpUseUngestures"));
  if (wp) {
    if (UseUngestures != (wp->GetDataField()->GetAsInteger())) {
      UseUngestures = (wp->GetDataField()->GetAsInteger());
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpThermalBar"));
  if (wp) {
    if (ThermalBar != (unsigned)(wp->GetDataField()->GetAsInteger())) {
      ThermalBar = (wp->GetDataField()->GetAsInteger());
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayClock"));
  if (wp) {
    if (OverlayClock != (wp->GetDataField()->GetAsInteger())) {
      OverlayClock = (wp->GetDataField()->GetAsInteger());
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpMcOverlay"));
  if (wp) {
    if (McOverlay != (wp->GetDataField()->GetAsInteger())) {
      McOverlay = (wp->GetDataField()->GetAsInteger());
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTrackBar"));
  if (wp) {
    if (TrackBar != (wp->GetDataField()->GetAsInteger())) {
      TrackBar = (wp->GetDataField()->GetAsInteger());
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOptimizeRoute"));
  if (wp) {
    if (PGOptimizeRoute_Config != (wp->GetDataField()->GetAsInteger())) {
      PGOptimizeRoute_Config = (wp->GetDataField()->GetAsInteger());
      PGOptimizeRoute = PGOptimizeRoute_Config;

      if (ISPARAGLIDER) {
	    if(PGOptimizeRoute) {
		  AATEnabled = true;
	    }
        ClearOptimizedTargetPos();
	  }
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOutlinedTp"));
  if (wp) {
	if (OutlinedTp_Config != (OutlinedTp_t) (wp->GetDataField()->GetAsInteger()))  {
		OutlinedTp_Config = (OutlinedTp_t) (wp->GetDataField()->GetAsInteger());
		OutlinedTp=OutlinedTp_Config;
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTpFilter"));
  if (wp) {
    if (TpFilter != (TpFilter_t)
	(wp->GetDataField()->GetAsInteger())) {
      TpFilter = (TpFilter_t)
	(wp->GetDataField()->GetAsInteger());
      LastDoRangeWaypointListTime=0;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverColor"));
  if (wp) {
    if (OverColor != (OverColor_t)
	(wp->GetDataField()->GetAsInteger())) {
      OverColor = (OverColor_t)
	(wp->GetDataField()->GetAsInteger());
      SetOverColorRef();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMapBox")); // VENTA6
  if (wp) {
    if (MapBox != (MapBox_t)
	(wp->GetDataField()->GetAsInteger())) {
      MapBox = (MapBox_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlaySize"));
  if (wp) {
    if (OverlaySize != wp->GetDataField()->GetAsInteger() ) {
      OverlaySize = wp->GetDataField()->GetAsInteger();
      Reset_Single_DoInits(MDI_DRAWLOOK8000);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGlideBarMode")); // VENTA6
  if (wp) {
    if (GlideBarMode != (GlideBarMode_t)
	(wp->GetDataField()->GetAsInteger())) {
      GlideBarMode = (GlideBarMode_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpNewMapDeclutter")); // VENTA6
  if (wp) {
    if (NewMapDeclutter != 
	(wp->GetDataField()->GetAsInteger())) {
      NewMapDeclutter = 
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGPSAltitudeOffset")); 
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY)*1000.0);
    if(((int) GPSAltitudeOffset) != ival) {
      GPSAltitudeOffset = ival;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUseGeoidSeparation"));
  if (wp) {
	if (UseGeoidSeparation != (wp->GetDataField()->GetAsInteger())) {
		UseGeoidSeparation = (wp->GetDataField()->GetAsInteger());
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPressureHg"));
  if (wp) {
	if (PressureHg != (wp->GetDataField()->GetAsInteger())) {
		PressureHg = (wp->GetDataField()->GetAsInteger());
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAverEffTime"));
  if (wp) {
    if (AverEffTime != 
	(wp->GetDataField()->GetAsInteger())) {
      AverEffTime = 
	(wp->GetDataField()->GetAsInteger());
        LKSW_ResetLDRotary=true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpBgMapcolor"));
  if (wp) {
    if (BgMapColor_Config != (wp->GetDataField()->GetAsInteger())) {
      BgMapColor_Config = (wp->GetDataField()->GetAsInteger());
      BgMapColor = BgMapColor_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpArrivalValue")); // VENTA6
  if (wp) {
    if (ArrivalValue != (ArrivalValue_t)
	(wp->GetDataField()->GetAsInteger())) {
      ArrivalValue = (ArrivalValue_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxModel"));
  if (wp) {
    if (GlobalToBoxType(Appearance.InfoBoxModel) != (InfoBoxModelAppearance_t)
        (wp->GetDataField()->GetAsInteger())) {

      // We temporarily set it to the index 
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
      case apImPnaMinimap:
	GlobalModelType = MODELTYPE_PNA_MINIMAP;
	break;
      case apImPnaGenericBTKA:
	GlobalModelType = MODELTYPE_PNA_GENERIC_BTKA;
	break;
      case apImPnaGenericBTKB:
	GlobalModelType = MODELTYPE_PNA_GENERIC_BTKB;
	break;
      case apImPnaGenericBTKC:
	GlobalModelType = MODELTYPE_PNA_GENERIC_BTKC;
	break;
      case apImPnaGenericBTK1:
	GlobalModelType = MODELTYPE_PNA_GENERIC_BTK1;
	break;
      case apImPnaGenericBTK2:
	GlobalModelType = MODELTYPE_PNA_GENERIC_BTK2;
	break;
      case apImPnaGenericBTK3:
	GlobalModelType = MODELTYPE_PNA_GENERIC_BTK3;
	break;
      default:
	GlobalModelType = MODELTYPE_UNKNOWN; // Can't happen, troubles ..
	break;
	
      }
      // we set it correctly yo global value , ex. 10001
      Appearance.InfoBoxModel = (InfoBoxModelAppearance_t)GlobalModelType;
      requirerestart = true;
    }
  }
//


  wp = (WndProperty*)wf->FindByName(TEXT("prpAppIndLandable"));
  if (wp) {
    if (Appearance.IndLandable != (IndLandable_t)(wp->GetDataField()->GetAsInteger())) {
      Appearance.IndLandable = (IndLandable_t)(wp->GetDataField()->GetAsInteger());
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInverseInfoBox"));
  if (wp) {
    if ((int)(InverseInfoBox_Config) != wp->GetDataField()->GetAsInteger()) {
      InverseInfoBox_Config = (wp->GetDataField()->GetAsInteger() != 0);
      requirerestart = true;
      Appearance.InverseInfoBox=InverseInfoBox_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGliderScreenPosition"));
  if (wp) {
    if (MapWindow::GliderScreenPosition != 
	wp->GetDataField()->GetAsInteger()) {
      MapWindow::GliderScreenPosition = wp->GetDataField()->GetAsInteger();
	MapWindow::GliderScreenPositionY=MapWindow::GliderScreenPosition;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppDefaultMapWidth"));
  if (wp) {
    if ((int)(Appearance.DefaultMapWidth) != 
	wp->GetDataField()->GetAsInteger()) {
      Appearance.DefaultMapWidth = wp->GetDataField()->GetAsInteger();
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoBacklight")); // VENTA4
  if (wp) {
    if (EnableAutoBacklight != (wp->GetDataField()->GetAsInteger()!=0)) {
      EnableAutoBacklight = (wp->GetDataField()->GetAsInteger() != 0);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoSoundVolume")); // VENTA4
  if (wp) {
    if (EnableAutoSoundVolume != (wp->GetDataField()->GetAsInteger()!=0)) {
      EnableAutoSoundVolume = (wp->GetDataField()->GetAsInteger() != 0);
    }
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainContrast"));
  if (wp) {
    if (iround(TerrainContrast*100/255) != 
	wp->GetDataField()->GetAsInteger()) {
      TerrainContrast = (short)iround(wp->GetDataField()->GetAsInteger()*255.0/100);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainBrightness"));
  if (wp) {
    if (iround(TerrainBrightness*100/255) != 
	wp->GetDataField()->GetAsInteger()) {
      TerrainBrightness = (short)iround(wp->GetDataField()->GetAsInteger()*255.0/100);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainRamp"));
  if (wp) {
    if (TerrainRamp_Config != wp->GetDataField()->GetAsInteger()) {
      TerrainRamp_Config = wp->GetDataField()->GetAsInteger();
      TerrainRamp=TerrainRamp_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmMaxAltitude1"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY) *1000.0);
    if ((int)AlarmMaxAltitude1 != ival) {
      AlarmMaxAltitude1 = ival;
      LKalarms[0].triggervalue=(int)AlarmMaxAltitude1/1000;
      LKalarms[0].triggerscount=0;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmMaxAltitude2"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY) *1000.0);
    if ((int)AlarmMaxAltitude2 != ival) {
      AlarmMaxAltitude2 = ival;
      LKalarms[1].triggervalue=(int)AlarmMaxAltitude2/1000;
      LKalarms[1].triggerscount=0;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmMaxAltitude3"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY) *1000.0);
    if ((int)AlarmMaxAltitude3 != ival) {
      AlarmMaxAltitude3 = ival;
      LKalarms[2].triggervalue=(int)AlarmMaxAltitude3/1000;
      LKalarms[2].triggerscount=0;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmTakeoffSafety"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY) *1000.0);
    if ((int)AlarmTakeoffSafety != ival) {
      AlarmTakeoffSafety = ival;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmGearWarning"));
  if (wp) {
    ival = iround( wp->GetDataField()->GetAsInteger() );
    if ((int)GearWarningMode != ival) {
    	GearWarningMode = ival;
    }

  }
  DWORD tmp;
  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmGearAltitude"));
  if (wp) {
	  tmp = lround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY) *1000.0);
    if (GearWarningAltitude != tmp)
    {
    	GearWarningAltitude = tmp;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoAdvance"));
  if (wp) {
    if (AutoAdvance_Config != wp->GetDataField()->GetAsInteger()) {
      AutoAdvance_Config = wp->GetDataField()->GetAsInteger();
      AutoAdvance=AutoAdvance_Config;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLoggerTimeStepCruise"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger());
    if (LoggerTimeStepCruise != ival) {
      LoggerTimeStepCruise = ival;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLoggerTimeStepCircling"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger());
    if (LoggerTimeStepCircling != ival) {
      LoggerTimeStepCircling = ival;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComPort1"));
  if (wp) {
      COMMPort_t::const_iterator It = COMMPort.begin();
      std::advance(It, wp->GetDataField()->GetAsInteger());
      if(It != COMMPort.end() && !It->IsSamePort(szPort1)) {
          _tcscpy(szPort1, It->GetName());
          COMPORTCHANGED = true;
      }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpExtSound1"));
  if (wp) {
	if (UseExtSound1 != (wp->GetDataField()->GetAsInteger())) {
		UseExtSound1 = (wp->GetDataField()->GetAsInteger());
	}
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpComSpeed1"));
  if (wp) {
    if ((int)dwSpeedIndex1 != wp->GetDataField()->GetAsInteger()) {
      dwSpeedIndex1 = wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComBit1"));
  if (wp) {
    if ((int)dwBit1Index != wp->GetDataField()->GetAsInteger()) {
      dwBit1Index = wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComDevice1"));
  if (wp) {
    if (dwDeviceIndex1 != wp->GetDataField()->GetAsInteger()) {
      dwDeviceIndex1 = wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
      devRegisterGetName(dwDeviceIndex1, DeviceName);
      WriteDeviceSettings(0, DeviceName);  
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComPort2"));
  if (wp) {
      COMMPort_t::const_iterator It = COMMPort.begin();
      std::advance(It, wp->GetDataField()->GetAsInteger());
      if(It != COMMPort.end() && !It->IsSamePort(szPort2)) {
          _tcscpy(szPort2, It->GetName());
          COMPORTCHANGED = true;
      }
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpExtSound2"));
  if (wp) {
	if (UseExtSound2 != (wp->GetDataField()->GetAsInteger())) {
		UseExtSound2 = (wp->GetDataField()->GetAsInteger());
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComSpeed2"));
  if (wp) {
    if ((int)dwSpeedIndex2 != wp->GetDataField()->GetAsInteger()) {
      dwSpeedIndex2 = wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComBit1"));
  if (wp) {
    if ((int)dwBit1Index != wp->GetDataField()->GetAsInteger()) {
      dwBit1Index = wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComDevice2"));
  if (wp) {
    if (dwDeviceIndex2 != wp->GetDataField()->GetAsInteger()) {
      dwDeviceIndex2 = wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
      devRegisterGetName(dwDeviceIndex2, DeviceName);
      WriteDeviceSettings(1, DeviceName);  
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSnailWidthScale"));
  if (wp) {
    if (MapWindow::SnailWidthScale != wp->GetDataField()->GetAsInteger()) {
      MapWindow::SnailWidthScale = wp->GetDataField()->GetAsInteger();
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEngineeringMenu")); // VENTA9
  if (wp) EngineeringMenu = wp->GetDataField()->GetAsInteger();


 UpdateAircraftConfig();

  int i,j;
  for (i=0; i<4; i++) {
    for (j=0; j<8; j++) {
      GetInfoBoxSelector(j, i);
    }
  }

  if (waypointneedsave) {
    if(MessageBoxX(
    // LKTOKEN  _@M581_ = "Save changes to waypoint file?" 
                   gettext(TEXT("_@M581_")),
	// LKTOKEN  _@M809_ = "Waypoints edited" 
                   gettext(TEXT("_@M809_")),
                   mbYesNo) == IdYes) {
    
      AskWaypointSave();

    }
  }

  if (taskchanged) {
    RefreshTask();
  }

  if (fontschanged) {
      #if TESTBENCH
      StartupStore(_T("..... dlgConfiguration: fontschanged requested\n"));
      #endif
      FONTSCHANGED=true;
  }

#if (WINDOWSPC>0)
  if (COMPORTCHANGED) {
    requirerestart = true;
  }
#endif

  InitActiveGate();

    if (requirerestart) {
      MessageBoxX (
	// LKTOKEN  _@M561_ = "Restart LK8000 to apply changes." 
		   gettext(TEXT("_@M561_")), 
		   TEXT("Configuration"), mbOk);
    }

  delete wf;

  wf = NULL;
  

}

//
// We must call this update also before saving profiles during config showmodal, otherwise we 
// wont be updating the current set values! This is why we keep separated function for polar.
// IN 3.1 we Call it only on exit, because saving is done externally, always.
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
      requirerestart = true;
	LKSW_ReloadProfileBitmaps=true;

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
      _tcscpy(szPolarFile,temptext);
      POLARFILECHANGED = true;
      GlidePolar::SetBallast();
    }
  }

 wp = (WndProperty*)wf->FindByName(TEXT("prpMaxManoeuveringSpeed"));
  if (wp) {
    ival = iround((wp->GetDataField()->GetAsFloat()/SPEEDMODIFY)*1000.0);
    if ((int)(SAFTEYSPEED*1000) != (int)iround(ival)) {
        SAFTEYSPEED=ival/1000.0;
      GlidePolar::SetBallast();
    }
  }

 wp = (WndProperty*)wf->FindByName(TEXT("prpHandicap"));
  if (wp) {
    ival  = wp->GetDataField()->GetAsInteger();
    if (Handicap != ival) {
      Handicap = ival;
    }
  }

 wp = (WndProperty*)wf->FindByName(TEXT("prpBallastSecsToEmpty"));
  if (wp) {
    ival = wp->GetDataField()->GetAsInteger();
    if (BallastSecsToEmpty != ival) {
      BallastSecsToEmpty = ival;
    }
  }
}


