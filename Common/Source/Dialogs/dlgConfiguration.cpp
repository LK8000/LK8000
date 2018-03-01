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
#include "resource.h"
#include "LKStyle.h"

#ifdef ANDROID
#include <jni.h>
#include "Android/Main.hpp"
#include "Android/NativeView.hpp"
#include "Android/Context.hpp"
#include "Android/BluetoothHelper.hpp"
#include "Android/BluetoothLeScan.h"
#include "Util/ScopeExit.hxx"
#include <sstream>
#endif
using namespace std::placeholders;

extern void UpdateAircraftConfig(void);
extern void dlgCustomMenuShowModal(void);
void UpdateComPortList(WndProperty* wp, LPCTSTR szPort);
void UpdateComPortSetting(WndForm* pOwner, size_t idx, const TCHAR* szPortName);
void ShowWindowControl(WndForm* pOwner, const TCHAR* WndName, bool bShow);
void UpdateDeviceEntries(WndForm *pOwner, int DeviceIdx);
static void UpdateButtons(WndForm *pOwner) ;
static bool taskchanged = false;
static bool requirerestart = false;
static bool utcchanged = false;
static bool waypointneedsave = false;
static bool fontschanged= false;
static bool snailchanged= false;



static  int dwDeviceIndex[NUMDEV] = {0,0,0,0,0,0};

#define CONFIGMODE_SYSTEM   0
#define CONFIGMODE_PILOT    1
#define CONFIGMODE_AIRCRAFT 2
#define CONFIGMODE_DEVICE   3

static short configMode=0;  // current configuration mode, see above
static short config_page[4]={0,0,0,0}; // remember last page we were using, for each profile

static WndForm *wf=NULL;

#define NUMOFCONFIGPAGES 22 // total number of config pages including engineering
#define NUMENGPAGES 1       // number of engineering hidden pages, part of NUMOFCONFIGPAGES
#define FIRST_INFOBOX_PAGE 13 
#define MAXNUMDEVICES 6     // A B C D E F

static WndFrame *wConfig[NUMOFCONFIGPAGES]={};

typedef struct {
    const TCHAR* szName;
    const TCHAR* szCpation;
    bool CopyPaste;
} ConfigPageNames_t;

/* 
 * carefull : if order change, check all "config_page" array use...
 *    like "OnInfoBoxHelp", "page2mode" and "InfoBoxPropName" function
 */
const ConfigPageNames_t ConfigPageNames[4][NUMOFCONFIGPAGES] = {
    { // config system
      /*0 */  { _T("frmSite"),                _T("_@M10_"), false },  // "1 Site"
      /*1 */  { _T("frmAirspace"),            _T("_@M22_"), false },  // "2 Airspace"
      /*2 */  { _T("frmDisplay"),             _T("_@M28_"), false },  // "3 Map Display" 
      /*3 */  { _T("frmTerrain"),             _T("_@M32_"), false },  // "4 Terrain Display" 
      /*4 */  { _T("frmFinalGlide"),          _T("_@M33_"), false },  // "5 Glide Computer" 
      /*5 */  { _T("frmSafety"),              _T("_@M34_"), false },  // "6 Safety factors" 
      /*6 */  { _T("frmUnits"),               _T("_@M38_"), false },  // "9 Units" 
      /*7 */  { _T("frmInterface"),           _T("_@M11_"), false },  // "10 Interface" 
      /*8 */  { _T("frmAppearance"),          _T("_@M12_"), false },  // "11 Appearance" 
      /*9 */  { _T("frmFonts"),               _T("_@M13_"), false },  // "12 Fonts" 
      /*10*/  { _T("frmVarioAppearance"),     _T("_@M14_"), false },  // "13 Map Overlays " 
      /*11*/  { _T("frmTask"),                _T("_@M15_"), false },  // "14 Task" 
      /*12*/  { _T("frmAlarms"),              _T("_@M1646_"), false },// "15 Alarms"
      /*13*/  { _T("frmInfoBoxCruise"),       _T("_@M18_"), true },   // "16 InfoBox Cruise" 
      /*14*/  { _T("frmInfoBoxCircling"),     _T("_@M19_"), true },   // "17 InfoBox Thermal" 
      /*15*/  { _T("frmInfoBoxFinalGlide"),   _T("_@M20_"), true },	  // "18 InfoBox Final Glide" 
      /*16*/  { _T("frmInfoBoxAuxiliary"),    _T("_@M21_"), true },	  // "19 InfoBox Auxiliary" 
      /*17*/  { _T("frmLogger"),              _T("_@M24_"), false },  // "20 Logger" 
      /*18*/  { _T("frmWaypointEdit"),        _T("_@M25_"), false },  // "21 Waypoint Edit" 
      /*19*/  { _T("frmSpecials1"),           _T("_@M26_"), false },  // "22 System" 
      /*20*/  { _T("frmSpecials2"),           _T("_@M27_"), false },  // "23 Paragliders/Delta specials" 
      /*21*/  { _T("frmEngineering1"),        _T("24 Engineering Menu"), false },
    }, 
    { // config Pilot
      /*0 */  { _T("frmPilot"),                _T("_@M1785_"), false }, // pilot configuration
              {},
    }, 
    { // config aircraft
      /*0 */  { _T("frmPolar"),                _T("_@M1786_"), false }, // aircraft configuration
              {},
    },
    { // config device
      /*0 */  { _T("frmComm"),                _T("_@M1820_"), false }, // // device configuration
              {},
    },
};

static_assert(array_size(config_page) == array_size(ConfigPageNames), "invalid array size");
static_assert(array_size(wConfig) == array_size(ConfigPageNames[0]), "invalid array size");
static_assert(array_size(wConfig) == array_size(ConfigPageNames[1]), "invalid array size");
static_assert(array_size(wConfig) == array_size(ConfigPageNames[2]), "invalid array size");
static_assert(array_size(wConfig) == array_size(ConfigPageNames[2]), "invalid array size");


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
    numPages = 0;
    std::fill(std::begin(wConfig), std::end(wConfig), nullptr);
}

void FontSetEnums( DataField* dfe) {

    dfe->addEnumText(TEXT("-10"));
    dfe->addEnumText(TEXT("-9"));
    dfe->addEnumText(TEXT("-8"));
    dfe->addEnumText(TEXT("-7"));
    dfe->addEnumText(TEXT("-6"));
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
    dfe->addEnumText(TEXT("+6"));
    dfe->addEnumText(TEXT("+7"));
    dfe->addEnumText(TEXT("+8"));
    dfe->addEnumText(TEXT("+9"));
    dfe->addEnumText(TEXT("+10"));
}
/*
    dfe = wp->GetDataField();
    dfe->addEnumText(TEXT("-5"));
*/

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

static void UpdateButtons(WndForm *pOwner) {
  TCHAR text[120];
  TCHAR val[100];
if(!pOwner) return;
  if (buttonPilotName) {
    _tcscpy(val,PilotName_Config);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, MsgToken(7));
    }
	// LKTOKEN  _@M524_ = "Pilot name" 
    _stprintf(text,TEXT("%s: %s"), MsgToken(524), val);
    buttonPilotName->SetCaption(text);
  }
  if (buttonAircraftType) {
    _tcscpy(val,AircraftType_Config);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, MsgToken(7));
    }
	// LKTOKEN  _@M59_ = "Aircraft type" 
    _stprintf(text,TEXT("%s: %s"), MsgToken(59), val);
    buttonAircraftType->SetCaption(text);
  }
  if (buttonAircraftRego) {
    _tcscpy(val,AircraftRego_Config);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, MsgToken(7));
    }
	// LKTOKEN  _@M57_ = "Aircraft Reg" 
    _stprintf(text,TEXT("%s: %s"), MsgToken(57), val);
    buttonAircraftRego->SetCaption(text);
  }
  if (buttonCompetitionClass) {
    _tcscpy(val,CompetitionClass_Config);
    if (_tcslen(val)<=0) {
      // LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, MsgToken(7));
    }
	// LKTOKEN  _@M936_ = "Competition Class" 
    _stprintf(text,TEXT("%s: %s"), MsgToken(936), val);
    buttonCompetitionClass->SetCaption(text);
  }
  if (buttonCompetitionID) {
    _tcscpy(val,CompetitionID_Config);
    if (_tcslen(val)<=0) {
      // LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, MsgToken(7));
    }
	// LKTOKEN  _@M938_ = "Competition ID" 
    _stprintf(text,TEXT("%s: %s"), MsgToken(938), val);
    buttonCompetitionID->SetCaption(text);
  }

  WndButton* wCmdBth = ((WndButton *)pOwner->FindByName(TEXT("cmdBth")));
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
    LKASSERT((size_t)configMode<array_size(config_page));
    config_page[configMode] += Step;

    if (configMode==CONFIGMODE_SYSTEM && !EngineeringMenu) { 
        if (config_page[configMode]>=(numPages-NUMENGPAGES)) { config_page[configMode]=0; }
        if (config_page[configMode]<0) { config_page[configMode]=numPages-(NUMENGPAGES+1); } 
    } else {
        if (config_page[configMode]>=numPages) { config_page[configMode]=0; }
        if (config_page[configMode]<0) { config_page[configMode]=numPages-1; }
    }

    LKASSERT((size_t)configMode < array_size(ConfigPageNames));
    LKASSERT((size_t)config_page[configMode] < array_size(ConfigPageNames[0]));
    
    const ConfigPageNames_t* current = ConfigPageNames[configMode];
    const TCHAR* szCaption = LKGetText(current[config_page[configMode]].szCpation);
    if (!szCaption || (_tcslen(szCaption) <= 0)) {
        szCaption = current[config_page[configMode]].szCpation;
    }
    wf->SetCaption(szCaption);
    if (buttonCopy) {
        buttonCopy->SetVisible(current[config_page[configMode]].CopyPaste);
    }
    if (buttonPaste) {
        buttonPaste->SetVisible(current[config_page[configMode]].CopyPaste);
    }

    for (short i = 0; i < (short)array_size(wConfig); ++i) {
        if (wConfig[i]) {
            wConfig[i]->SetVisible(config_page[configMode] == i);
        }
    }
} // NextPage


static void UpdateDeviceSetupButton(WndForm* pOwner,size_t idx /*, const TCHAR *Name*/) {

  // const TCHAR * DevicePropName[] = {_T("prpComPort1")};
  // check if all array have same size ( compil time check );
  // static_assert(array_size(DeviceList) == array_size(DevicePropName), "DevicePropName array size need to be same of DeviceList array size");

  if(!pOwner)
    return;
  WndProperty* wp;

  wp = (WndProperty*)pOwner->FindByName(TEXT("prpComPort1"));
  if (wp) {
      if (_tcscmp(szPort[SelectedDevice], wp->GetDataField()->GetAsString()) != 0)
      {
          _tcscpy(szPort[SelectedDevice], wp->GetDataField()->GetAsString());
          COMPORTCHANGED = true;
      }
  }

  wp = (WndProperty*)pOwner->FindByName(TEXT("prpExtSound1"));
  if (wp) {
        if (UseExtSound[SelectedDevice] != (wp->GetDataField()->GetAsBoolean())) {
                UseExtSound[SelectedDevice] = (wp->GetDataField()->GetAsBoolean());
        }
  }

  wp = (WndProperty*)pOwner->FindByName(TEXT("prpComSpeed1"));
  if (wp) {
    if ((int)dwSpeedIndex[SelectedDevice] != wp->GetDataField()->GetAsInteger()) {
      dwSpeedIndex[SelectedDevice] = wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)pOwner->FindByName(TEXT("prpComBit1"));
  if (wp) {
    if ((int)dwBitIndex[SelectedDevice] != wp->GetDataField()->GetAsInteger()) {
      dwBitIndex[SelectedDevice] = wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)pOwner->FindByName(TEXT("prpComIpAddr1"));
  if (wp) {
    if (_tcscmp(szIpAddress[SelectedDevice], wp->GetDataField()->GetAsString()) != 0) {
      _tcsncpy(szIpAddress[SelectedDevice], wp->GetDataField()->GetAsString(), array_size(szIpAddress[SelectedDevice]));
      szIpAddress[SelectedDevice][array_size(szIpAddress[SelectedDevice])-1] = _T('\0');
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)pOwner->FindByName(TEXT("prpComIpPort1"));
  if (wp) {
    if ((int)dwIpPort[SelectedDevice] != wp->GetDataField()->GetAsInteger()) {
      dwIpPort[SelectedDevice] = wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
    }
  }

  wp = (WndProperty*)pOwner->FindByName(TEXT("prpComDevice1"));
  if (wp) {

    if (dwDeviceIndex[SelectedDevice] != wp->GetDataField()->GetAsInteger()) {
      dwDeviceIndex[SelectedDevice]= wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
      WriteDeviceSettings(SelectedDevice, devRegisterGetName(dwDeviceIndex[SelectedDevice]));
    }
  }

  /************************************************************************/

    UpdateComPortSetting(pOwner, idx, szPort[SelectedDevice]);
}

static void OnDeviceAData(DataField *Sender, DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      #ifdef TESTBENCH
      StartupStore(_T("........... OnDeviceAData Device %i %s %s"),SelectedDevice, Sender->GetAsString(),NEWLINE); // 091105
      #endif
      UpdateDeviceSetupButton(wf, SelectedDevice);
    break;
	default: 
		StartupStore(_T("........... DBG-902%s"),NEWLINE); // 091105
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
                #ifdef TESTBENCH
		StartupStore(_T("........... DBG-908%s"),NEWLINE); 
                #endif
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
                #ifdef TESTBENCH
		StartupStore(_T("........... DBG-908%s"),NEWLINE); 
                #endif
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
      if (wp) {
    	  AspPermanentChanged=(wp->GetDataField()->GetAsInteger());
      }
#ifdef TESTBENCH 
      StartupStore(_T(".......AspPermanentChanged %i %s"),AspPermanentChanged,NEWLINE);
#endif
      break;
    default:
                #ifdef TESTBENCH
		StartupStore(_T("........... DBG-908%s"),NEWLINE);
                #endif
		break;
  }

}

static void OnAutoContrastChange(DataField *Sender, DataField::DataAccessKind_t Mode){
  WndProperty* wp;
  switch(Mode){
     case DataField::daGet:
        break;
     case DataField::daPut:
     case DataField::daChange:
        wp = (WndProperty*)wf->FindByName(TEXT("prpAutoContrast"));
        if (wp) {
           if (AutoContrast != wp->GetDataField()->GetAsBoolean()) {
              AutoContrast = wp->GetDataField()->GetAsBoolean();
           }
        }
        wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainContrast"));
        if(wp) {
           if(AutoContrast) {
              wp->SetReadOnly(true);
           } else {
              wp->SetReadOnly(false);
           }
           wp->RefreshDisplay();
        }
        wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainBrightness"));
        if(wp) {
           if(AutoContrast) {
              wp->SetReadOnly(true);
           } else {
              wp->SetReadOnly(false);
           }
           wp->RefreshDisplay();
        }
        break;
     default:
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
    UpdateButtons(pWnd->GetParentWndForm());
}

static void OnAircraftTypeClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonAircraftType) {
        _tcscpy(Temp, AircraftType_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(AircraftType_Config, Temp);
    }
    UpdateButtons(pWnd->GetParentWndForm());
}

static void OnTerminalClicked(WndButton* pWnd) {
    extern void dlgTerminal(int portnum);
    dlgTerminal(SelectedDevice);
    UpdateDeviceEntries(pWnd->GetParentWndForm(), SelectedDevice);
}

static void OnPilotNameClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonPilotName) {
        _tcscpy(Temp, PilotName_Config);
        dlgTextEntryShowModal(Temp, 100);

        //
        // ACCESS TO SYSOP MODE 
        //
        extern bool SysOpMode;
        extern bool Sysop(TCHAR *command);
        #define SYSOPW "OPSYS"

        if (!SysOpMode) {
           if (!_tcscmp(Temp,_T(SYSOPW))) {
              _tcscpy(Temp,_T("SYSOP"));
              Sysop(Temp); // activate sysop mode and exit dialog
              if(pWnd) {
                 WndForm * pForm = pWnd->GetParentWndForm();
                 if(pForm) pForm->SetModalResult(mrOK);
              }
              return;
           }
        } 
        
        if (SysOpMode && _tcslen(Temp)>=2) {
           if (Sysop(Temp)) {
              // if requested, close immediately the parent dialog and back to normal menu!
              // This is needed for example when commanding resolution chang because
              // dialogs are not changed until closed, and we risk not seeing the Close button anymore.
              if(pWnd) {
                 WndForm * pForm = pWnd->GetParentWndForm();
                 if(pForm) pForm->SetModalResult(mrOK);
              }
           }
           return; // in SysOp mode no change of pilot name
        } 

        if (!SysOpMode) _tcscpy(PilotName_Config, Temp);
    }
    UpdateButtons(pWnd->GetParentWndForm());
}



static void OnLiveTrackerStartConfig(DataField *Sender, DataField::DataAccessKind_t Mode){
WndProperty* wp;
if(wf)
{
  wp = (WndProperty*)wf->FindByName(TEXT("prpLiveTrackerStart_config"));
  if (wp)
  {
    if (LiveTrackerStart_config != wp->GetDataField()->GetAsInteger())
    {
      LiveTrackerStart_config = (int)wp->GetDataField()->GetAsInteger();
      requirerestart = true;

    }
  }
}
}

static void OnLiveTrackersrvClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackersrv) {
        _tcscpy(Temp, LiveTrackersrv_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(LiveTrackersrv_Config, Temp);
    }
    UpdateButtons(pWnd->GetParentWndForm());
}

static void OnLiveTrackerportClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackerport) {
        _stprintf(Temp, _T("%d"), LiveTrackerport_Config);
        dlgNumEntryShowModal(Temp, 100, false);
        TCHAR *sz = NULL;
        LiveTrackerport_Config = _tcstol(Temp, &sz, 10);
    }
    UpdateButtons(pWnd->GetParentWndForm());
}

static void OnLiveTrackerusrClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackerusr) {
        _tcscpy(Temp, LiveTrackerusr_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(LiveTrackerusr_Config, Temp);
    }
    UpdateButtons(pWnd->GetParentWndForm());
}

static void OnLiveTrackerpwdClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackerpwd) {
        _tcscpy(Temp, LiveTrackerpwd_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(LiveTrackerpwd_Config, Temp);
    }
    UpdateButtons(pWnd->GetParentWndForm());
}

static void OnCompetitionClassClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonCompetitionClass) {
        _tcscpy(Temp, CompetitionClass_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(CompetitionClass_Config, Temp);
    }
    UpdateButtons(pWnd->GetParentWndForm());
}

static void OnCompetitionIDClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonCompetitionID) {
        _tcscpy(Temp, CompetitionID_Config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(CompetitionID_Config, Temp);
    }
    UpdateButtons(pWnd->GetParentWndForm());
}

static void OnAirspaceColoursClicked(WndButton* pWnd) {
    bool retval;
    retval = dlgAirspaceShowModal(true);
    if (retval) {
        requirerestart = true;
    }
}


static void OnLKMapOpenClicked(WndButton* pWnd) {
#ifdef ANDROID
  jclass cls = Java::GetEnv()->FindClass("org/LK8000/LKMaps");
  if(cls == nullptr) return;
  jmethodID mid = Java::GetEnv()->GetStaticMethodID(cls, "openLKMaps","(Landroid/content/Context;)V");
  if(mid == nullptr) return;
  Java::GetEnv()->CallStaticVoidMethod(cls, mid,context->Get());
#endif
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

static void OnSetOverlaysClicked(WndButton* pWnd) {
    extern void dlgOverlaysShowModal(void);
    dlgOverlaysShowModal();
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
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static int cpyInfoBox[10];

int page2mode(void) {
  return config_page[configMode]-FIRST_INFOBOX_PAGE;
}


static void InfoBoxPropName(TCHAR *name, int item, int mode) {
  _tcscpy(name,TEXT("prpInfoBox"));
  switch (mode) {
  case 0:
    _tcscat(name,TEXT("Cruise"));
    break;
  case 1:
    _tcscat(name,TEXT("Circling"));
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
    wp = (WndProperty*)pWnd->GetParentWndForm()->FindByName(name);
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
		 MsgToken(510),
	// LKTOKEN  _@M354_ = "InfoBox paste" 
		 MsgToken(354),
		 mbYesNo) == IdYes) {

    for (int item=0; item<8; item++) {
      InfoBoxPropName(name, item, mode);
      WndProperty *wp;
      wp = (WndProperty*)pWnd->GetParentWndForm()->FindByName(name);
      if (wp && (cpyInfoBox[item]>=0)&&(cpyInfoBox[item]<NumDataOptions)) {
	wp->GetDataField()->Set(cpyInfoBox[item]);
	wp->RefreshDisplay();
      }
    }
  }
}

static bool FormKeyDown(WndForm* pWnd, unsigned KeyCode) {
    Window * pBtn = NULL;

    switch (KeyCode & 0xffff) {
        case '6':
            pBtn = pWnd->FindByName(TEXT("cmdPrev"));
            NextPage(-1);
            break;
        case '7':
            pBtn = pWnd->FindByName(TEXT("cmdNext"));
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
	MsgToken(478),
	// LKTOKEN  _@M457_ = "New Waypoint" 
               MsgToken(457),
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
  edit_waypoint.Style = STYLE_NORMAL; // normal turnpoint
  edit_waypoint.Code[0]=0;
  edit_waypoint.Freq[0]=0;
  edit_waypoint.Country[0]=0;
  dlgWaypointEditShowModal(&edit_waypoint);

  // SeeYou style not correct when new waypoint created
  // This setting will override Flags when reloading the file after changes!
  if ( (edit_waypoint.Flags & AIRPORT) == AIRPORT) {
	edit_waypoint.Style=STYLE_AIRFIELDSOLID;  // airport
  } else {
	if ( (edit_waypoint.Flags & LANDPOINT) == LANDPOINT) {
		edit_waypoint.Style=STYLE_OUTLANDING;  // outlanding
	}
  }
  // else it is already a turnpoint


  if (_tcslen(edit_waypoint.Name)>0) {
    LockTaskData();
    if(AddWaypoint(edit_waypoint)) {
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
		MsgToken(716),
	// LKTOKEN  _@M194_ = "CompeGPS Waypoint" 
                MsgToken(194),
                mbOk);

		return;
	}
	#endif

	if ( WayPointList[res].Format == LKW_VIRTUAL) {      // 100212
		MessageBoxX(
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
		MsgToken(716),
	// LKTOKEN  _@M775_ = "VIRTUAL Waypoint" 
                MsgToken(775),
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
                   MsgToken(810),
	// LKTOKEN  _@M811_ = "Waypoints outside terrain" 
                   MsgToken(811),
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
		MessageBoxX(MsgToken(716),
	// LKTOKEN  _@M194_ = "CompeGPS Waypoint" 
			MsgToken(194),
			mbOk);
		return;
	} else 
	#endif
		if ( WayPointList[res].Format == LKW_VIRTUAL ) { // 100212
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
			MessageBoxX(MsgToken(716),
	// LKTOKEN  _@M775_ = "VIRTUAL Waypoint" 
				MsgToken(775),
				mbOk);
			return;
		} else 
	// LKTOKEN  _@M229_ = "Delete Waypoint?" 
	if(MessageBoxX(WayPointList[res].Name, MsgToken(229), 
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
    ReadPortSettings(SelectedDevice,szPort, NULL, NULL);
    UpdateComPortList((WndProperty*) wf->FindByName(TEXT("prpComPort1")), szPort);

    // ReadPort2Settings(szPort, NULL, NULL);
    // UpdateComPortList((WndProperty*) wf->FindByName(TEXT("prpComPort2")), szPort);
}
#endif



static void OnNextDevice(WndButton* pWnd) {
WndForm* pOwner = pWnd->GetParentWndForm();
  UpdateDeviceSetupButton(pOwner,SelectedDevice );

  SelectedDevice++;
  if(SelectedDevice >=NUMDEV)
    SelectedDevice = 0;

  UpdateDeviceEntries(pOwner, SelectedDevice);

}


static void OnA(WndButton* pWnd) {
WndForm* pOwner = pWnd->GetParentWndForm();
  UpdateDeviceSetupButton(pOwner,SelectedDevice );
  SelectedDevice =0;
  UpdateDeviceEntries(pOwner, SelectedDevice);
}

static void OnB(WndButton* pWnd) {
WndForm* pOwner = pWnd->GetParentWndForm();
  UpdateDeviceSetupButton(pOwner,SelectedDevice );
  SelectedDevice =1;
  UpdateDeviceEntries(pOwner, SelectedDevice);
}


static void OnC(WndButton* pWnd) {
WndForm* pOwner = pWnd->GetParentWndForm();
  UpdateDeviceSetupButton(pOwner,SelectedDevice );
  SelectedDevice =2;
  UpdateDeviceEntries(pOwner, SelectedDevice);
}


static void OnD(WndButton* pWnd) {
WndForm* pOwner = pWnd->GetParentWndForm();
  UpdateDeviceSetupButton(pOwner,SelectedDevice );
  SelectedDevice =3;
  UpdateDeviceEntries(pOwner, SelectedDevice);
}

static void OnE(WndButton* pWnd) {
WndForm* pOwner = pWnd->GetParentWndForm();
  UpdateDeviceSetupButton(pOwner,SelectedDevice );
  SelectedDevice =4;
  UpdateDeviceEntries(pOwner, SelectedDevice);
}


static void OnF(WndButton* pWnd) {
WndForm* pOwner = pWnd->GetParentWndForm();
  UpdateDeviceSetupButton(pOwner,SelectedDevice );
  SelectedDevice =5;
  UpdateDeviceEntries(pOwner, SelectedDevice);
}

void ShowWindowControl(WndForm* pOwner, const TCHAR* WndName, bool bShow) {
    WindowControl* pWnd = pOwner->FindByName(WndName);
    if(pWnd) {
        pWnd->SetVisible(bShow);
    }
}

void UpdateComPortSetting(WndForm* pOwner,  size_t idx, const TCHAR* szPortName) {

    LKASSERT(szPortName);
    // check if all array have same size ( compil time check );
    /*
    static_assert(array_size(DeviceList) == array_size(PortPropName[0]), "PortPropName array size need to be same of DeviceList array size");
    static_assert(array_size(DeviceList) == array_size(prpExtSound), "prpExtSound array size need to be same of DeviceList array size");
    static_assert(array_size(DeviceList) == array_size(prpIpAddr[0]), "prpIpAddr array size need to be same of DeviceList array size");
    static_assert(array_size(DeviceList) == array_size(prpIpPort[0]), "prpIpPort array size need to be same of DeviceList array size");
*/
#ifdef DISABLEEXTAUDIO    
    bool bManageExtAudio = false;
#else
    bool bManageExtAudio = true;
#endif

    WndProperty* wp;



    wp = (WndProperty*)pOwner->FindByName(TEXT("prpComDevice1"));


    TCHAR newname[25];

    if(pOwner) {
        _stprintf(newname,  TEXT("%s"), MsgToken(232));
        newname[_tcslen(newname)-1] = (TCHAR)('A'+SelectedDevice);
      WndFrame  *wDev = ((WndFrame *)pOwner->FindByName(TEXT("frmCommName")));
      if(wDev) {
          wDev->SetCaption(newname);
      }
    }
    bool bHide = false;

    if (wp)
    {
      if(wp->GetDataField()->GetAsString())
      {
        {
          if(_tcscmp(wp->GetDataField()->GetAsString(), _T(DEV_DISABLED_NAME)) == 0)
          {
            DeviceList[idx].Disabled = true;
            bHide = true;
          }
          else
            DeviceList[idx].Disabled = false;

          if (_tcscmp(wp->GetDataField()->GetAsString(), _T("Internal")) == 0)
          {
            bHide = true;
          }
        }
      }
    }

    if ((WndButton *)pOwner->FindByName(TEXT("cmdA")))
       ((WndButton *)pOwner->FindByName(TEXT("cmdA")))->LedSetOnOff(!DeviceList[0].Disabled);
    if ((WndButton *)pOwner->FindByName(TEXT("cmdB")))
       ((WndButton *)pOwner->FindByName(TEXT("cmdB")))->LedSetOnOff(!DeviceList[1].Disabled);
    if ((WndButton *)pOwner->FindByName(TEXT("cmdC")))
       ((WndButton *)pOwner->FindByName(TEXT("cmdC")))->LedSetOnOff(!DeviceList[2].Disabled);
    if ((WndButton *)pOwner->FindByName(TEXT("cmdD")))
       ((WndButton *)pOwner->FindByName(TEXT("cmdD")))->LedSetOnOff(!DeviceList[3].Disabled);
    if ((WndButton *)pOwner->FindByName(TEXT("cmdE")))
       ((WndButton *)pOwner->FindByName(TEXT("cmdE")))->LedSetOnOff(!DeviceList[4].Disabled);
    if ((WndButton *)pOwner->FindByName(TEXT("cmdF")))
       ((WndButton *)pOwner->FindByName(TEXT("cmdF")))->LedSetOnOff(!DeviceList[5].Disabled);

    if(bHide)
    {
        ShowWindowControl(wf, TEXT("prpComPort1"), !bHide);
        ShowWindowControl(wf, TEXT("prpComSpeed1"),!bHide);
        ShowWindowControl(wf, TEXT("prpComBit1"),  !bHide );
        ShowWindowControl(wf, TEXT("prpComIpAddr1"),!bHide);
        ShowWindowControl(wf, TEXT("prpComIpPort1"),!bHide);
        ShowWindowControl(wf, TEXT("prpExtSound1"), !bHide);
    }
    else
    {
    bManageExtAudio &= IsSoundInit();

    bool bBt = ((_tcslen(szPortName) > 3) && ((_tcsncmp(szPortName, _T("BT:"), 3) == 0) || (_tcsncmp(szPortName, _T("Bluetooth Server"), 3) == 0)));
    bool bTCPClient = (_tcscmp(szPortName, _T("TCPClient")) == 0);
    bool bTCPServer = (_tcscmp(szPortName, _T("TCPServer")) == 0);
    bool bUDPServer = (_tcscmp(szPortName, _T("UDPServer")) == 0);
    bool bCOM = !(bBt || bTCPClient || bTCPServer || bUDPServer || ( DeviceList[SelectedDevice].iSharedPort>=0 ));
    if(bCOM)
    {
      ShowWindowControl(wf, TEXT("prpComPort1"), true);
      ShowWindowControl(wf, TEXT("prpComSpeed1"),true);
      ShowWindowControl(wf, TEXT("prpComBit1"),  true );
      ShowWindowControl(wf, TEXT("prpComIpAddr1"),false);
      ShowWindowControl(wf, TEXT("prpComIpPort1"),false);
      ShowWindowControl(wf, TEXT("prpExtSound1"), false);
    }
    else
    {
      ShowWindowControl(wf, TEXT("prpComPort1"), true);
      ShowWindowControl(wf, TEXT("prpComSpeed1"),false);
      ShowWindowControl(wf, TEXT("prpComBit1"),  false);
      ShowWindowControl(wf, TEXT("prpComIpAddr1"), bTCPClient);
      ShowWindowControl(wf, TEXT("prpComIpPort1"),  bTCPClient || bTCPServer || bUDPServer);
    }

    // Manage external sounds only if necessary
    if (bManageExtAudio) {
        ShowWindowControl(wf,  TEXT("prpExtSound1"), !bHide);
    }
    }
    TCHAR StateText[255];
    _tcscpy(StateText,_T(""));
    if( DeviceList[SelectedDevice].nmeaParser.gpsValid) _tcscat(StateText,TEXT("GPSFix "));
    if( DeviceList[SelectedDevice].nmeaParser.isFlarm) _tcscat(StateText,TEXT("Flarm "));
    if( DeviceList[SelectedDevice].nmeaParser.IsValidBaroSource()) _tcscat(StateText,TEXT("Baro "));
    if( DeviceList[SelectedDevice].iSharedPort>=0 ) _tcscat(StateText,TEXT("Shared "));

    if(DeviceList[idx].Disabled)
      ShowWindowControl(wf, TEXT("prpStatus"),false);
    else
    {
      ShowWindowControl(wf, TEXT("prpStatus"),true);
      wp = (WndProperty*)pOwner->FindByName(TEXT("prpStatus"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        dfe->Set(StateText);
        wp->RefreshDisplay();
      }
    }


}

static void OnComPort1Data(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
        UpdateComPortSetting(wf, SelectedDevice, Sender->GetAsString());
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
  OnHelpCallbackEntry(OnInfoBoxHelp),
  ClickNotifyCallbackEntry(OnWaypointNewClicked),
  ClickNotifyCallbackEntry(OnWaypointDeleteClicked),
  ClickNotifyCallbackEntry(OnWaypointEditClicked),
  ClickNotifyCallbackEntry(OnWaypointSaveClicked),
  ClickNotifyCallbackEntry(OnLKMapOpenClicked),

  DataAccessCallbackEntry(OnDeviceAData),
 // DataAccessCallbackEntry(OnDeviceBData),
  
  DataAccessCallbackEntry(OnComPort1Data),
  //DataAccessCallbackEntry(OnComPort2Data),


  ClickNotifyCallbackEntry(OnSetTopologyClicked),
  ClickNotifyCallbackEntry(OnSetCustomKeysClicked),
  ClickNotifyCallbackEntry(OnMultimapsClicked),
  ClickNotifyCallbackEntry(OnSetCustomMenuClicked),
  ClickNotifyCallbackEntry(OnSetBottomBarClicked),
  ClickNotifyCallbackEntry(OnSetInfoPagesClicked),
  ClickNotifyCallbackEntry(OnTaskRulesClicked),
  ClickNotifyCallbackEntry(OnSetOverlaysClicked),
  
  DataAccessCallbackEntry(OnAirspaceFillType),
  DataAccessCallbackEntry(OnAirspaceDisplay),
  DataAccessCallbackEntry(OnAspPermModified),
  DataAccessCallbackEntry(OnGearWarningModeChange),
  DataAccessCallbackEntry(OnLiveTrackerStartConfig),
  DataAccessCallbackEntry(OnAutoContrastChange),
#ifndef NO_BLUETOOTH
  ClickNotifyCallbackEntry(OnBthDevice),
#endif
  ClickNotifyCallbackEntry(OnNextDevice),
  ClickNotifyCallbackEntry(OnA),
  ClickNotifyCallbackEntry(OnB),
  ClickNotifyCallbackEntry(OnC),
  ClickNotifyCallbackEntry(OnD),
  ClickNotifyCallbackEntry(OnE),
  ClickNotifyCallbackEntry(OnF),
  ClickNotifyCallbackEntry(OnNextDevice),
  ClickNotifyCallbackEntry(OnTerminalClicked),
  EndCallBackEntry()
};



static void SetInfoBoxSelector(int item, int mode)
{
  TCHAR name[80];
  InfoBoxPropName(name, item, mode);

  WndProperty *wp;
  wp = (WndProperty*)wf->FindByName(name);
  if (wp) {
    DataField* dfe = wp->GetDataField();
    for (int i=0; i<NumDataOptions; i++) {
      dfe->addEnumText(LKGetText(Data_Options[i].Description));
    }
    dfe->Sort(0);

    int it=0;
    
    switch(mode) {
    case 0: // cruise
      it = (InfoType[item]>>8)& 0xff;
      break;
    case 1: // climb
      it = (InfoType[item])& 0xff;
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
    case 0: // cruise
      it = (InfoType[item]>>8)& 0xff;
      break;
    case 1: // climb
      it = (InfoType[item])& 0xff;
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


//static  int dwDeviceIndex2=0;
static  TCHAR temptext[MAX_PATH];

void UpdateComPortList(WndProperty* wp, LPCTSTR szPort) {
#ifdef ANDROID
    ScopeLock lock(COMMPort_mutex);
#endif

    if (wp) {
        DataField* dfe =  wp->GetDataField();
        if(dfe) {
            dfe->Clear();
            std::for_each(
                    COMMPort.begin(), COMMPort.end(),
                    [dfe](const COMMPortItem_t &item) {
                        dfe->addEnumText(item.GetName(), item.GetLabel());
                    });

            COMMPort_t::iterator It = std::find_if(
                COMMPort.begin(), 
                COMMPort.end(), 
                std::bind(&COMMPortItem_t::IsSamePort, _1, szPort)
            );
            if(It != COMMPort.end()) {
                dfe->Set((unsigned)std::distance(COMMPort.begin(), It));
            } else {
                dfe->addEnumText(szPort);
                dfe->Set((unsigned)COMMPort.size());
            }
        }
        wp->RefreshDisplay();
    }
}


void UpdateDeviceEntries(WndForm *wf, int DeviceIdx)
{
  WndProperty *wp;

  LKASSERT(wf);


TCHAR szPort[MAX_PATH];

ReadPortSettings(DeviceIdx,szPort,NULL, NULL);
UpdateComPortSetting(wf,DeviceIdx,szPort);
UpdateComPortList((WndProperty*)wf->FindByName(TEXT("prpComPort1")), szPort);


wp = (WndProperty*)wf->FindByName(TEXT("prpComSpeed1"));
if (wp) {
  DataField* dfe = wp->GetDataField();
  dfe->Set(dwSpeedIndex[DeviceIdx]);
  wp->SetReadOnly(false);
  wp->RefreshDisplay();
}
wp = (WndProperty*)wf->FindByName(TEXT("prpComBit1"));
if (wp) {
  DataField* dfe = wp->GetDataField();
  dfe->Set(dwBitIndex[DeviceIdx]);
  wp->RefreshDisplay();
}
wp = (WndProperty*)wf->FindByName(TEXT("prpExtSound1"));
if (wp) {
  wp->GetDataField()->Set(UseExtSound[DeviceIdx]);
  wp->RefreshDisplay();
}

wp = (WndProperty*)wf->FindByName(TEXT("prpComIpAddr1"));
if (wp) {
  wp->GetDataField()->Set(szIpAddress[DeviceIdx]);
  wp->RefreshDisplay();
}

wp = (WndProperty*)wf->FindByName(TEXT("prpComIpPort1"));
if (wp) {
  wp->GetDataField()->Set((int)dwIpPort[DeviceIdx]);
  wp->RefreshDisplay();
}

TCHAR deviceName1[MAX_PATH];
//  TCHAR deviceName2[MAX_PATH];
ReadDeviceSettings(DeviceIdx, deviceName1);


  wp = (WndProperty*)wf->FindByName(TEXT("prpComDevice1"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->Set(dwDeviceIndex[DeviceIdx]);
    wp->RefreshDisplay();
    UpdateButtons(wf);
  }

UpdateComPortSetting(wf, DeviceIdx,szPort);
UpdateComPortList((WndProperty*)wf->FindByName(TEXT("prpComPort1")), szPort);
  
  UpdateComPortSetting(wf, DeviceIdx, szPort);
}

static void setVariables( WndForm *pOwner) {
  WndProperty *wp;

  LKASSERT(pOwner);

  buttonPilotName = ((WndButton *)pOwner->FindByName(TEXT("cmdPilotName")));
  if (buttonPilotName) {
    buttonPilotName->SetOnClickNotify(OnPilotNameClicked);
  }
  buttonLiveTrackersrv = ((WndButton *)pOwner->FindByName(TEXT("cmdLiveTrackersrv")));
  if (buttonLiveTrackersrv) {
    buttonLiveTrackersrv->SetOnClickNotify(OnLiveTrackersrvClicked);
  }
  buttonLiveTrackerport = ((WndButton *)pOwner->FindByName(TEXT("cmdLiveTrackerport")));
  if (buttonLiveTrackerport) {
    buttonLiveTrackerport->SetOnClickNotify(OnLiveTrackerportClicked);
  }
  buttonLiveTrackerusr = ((WndButton *)pOwner->FindByName(TEXT("cmdLiveTrackerusr")));
  if (buttonLiveTrackerusr) {
    buttonLiveTrackerusr->SetOnClickNotify(OnLiveTrackerusrClicked);
  }
  buttonLiveTrackerpwd = ((WndButton *)pOwner->FindByName(TEXT("cmdLiveTrackerpwd")));
  if (buttonLiveTrackerpwd) {
    buttonLiveTrackerpwd->SetOnClickNotify(OnLiveTrackerpwdClicked);
  }
  buttonAircraftType = ((WndButton *)pOwner->FindByName(TEXT("cmdAircraftType")));
  if (buttonAircraftType) {
    buttonAircraftType->SetOnClickNotify(OnAircraftTypeClicked);
  }
  buttonAircraftRego = ((WndButton *)pOwner->FindByName(TEXT("cmdAircraftRego")));
  if (buttonAircraftRego) {
    buttonAircraftRego->SetOnClickNotify(OnAircraftRegoClicked);
  }
  buttonCompetitionClass = ((WndButton *)pOwner->FindByName(TEXT("cmdCompetitionClass")));
  if (buttonCompetitionClass) {
    buttonCompetitionClass->SetOnClickNotify(OnCompetitionClassClicked);
  }
  buttonCompetitionID = ((WndButton *)pOwner->FindByName(TEXT("cmdCompetitionID")));
  if (buttonCompetitionID) {
    buttonCompetitionID->SetOnClickNotify(OnCompetitionIDClicked);
  }
  buttonCopy = ((WndButton *)pOwner->FindByName(TEXT("cmdCopy")));
  if (buttonCopy) {
    buttonCopy->SetOnClickNotify(OnCopy);
  }
  buttonPaste = ((WndButton *)pOwner->FindByName(TEXT("cmdPaste")));
  if (buttonPaste) {
    buttonPaste->SetOnClickNotify(OnPaste);
  }

  UpdateButtons(pOwner);


  const TCHAR *tSpeed[] = {TEXT("1200"),TEXT("2400"),TEXT("4800"),TEXT("9600"),
			     TEXT("19200"),TEXT("38400"),TEXT("57600"),TEXT("115200")};

  TCHAR szPort[MAX_PATH];
  
  ReadPortSettings(SelectedDevice,szPort,NULL, NULL);
  UpdateComPortList((WndProperty*)pOwner->FindByName(TEXT("prpComPort1")), szPort);

  wp = (WndProperty*)pOwner->FindByName(TEXT("prpComSpeed1"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    std::for_each(std::begin(tSpeed), std::end(tSpeed), std::bind(&DataField::addEnumText, dfe, _1, nullptr));
    
    dfe->Set(dwSpeedIndex[SelectedDevice]);
    wp->SetReadOnly(false);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)pOwner->FindByName(TEXT("prpComBit1"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(TEXT("8bit"));
    dfe->addEnumText(TEXT("7bit"));
    dfe->Set(dwBitIndex[SelectedDevice]);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpExtSound1"));
  if (wp) {
    wp->GetDataField()->Set(UseExtSound[SelectedDevice]);
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpComIpAddr1"));
  if (wp) {
    wp->GetDataField()->Set(szIpAddress[SelectedDevice]);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpComIpPort1"));
  if (wp) {
    wp->GetDataField()->Set((int)dwIpPort[SelectedDevice]);
    wp->RefreshDisplay();
  }

  TCHAR deviceName1[MAX_PATH];
  ReadDeviceSettings(SelectedDevice, deviceName1);
  wp = (WndProperty*)wf->FindByName(TEXT("prpComDevice1"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    for (int i=0; i<DeviceRegisterCount; i++) {
      LPCTSTR DeviceName = devRegisterGetName(i);
      dfe->addEnumText(DeviceName);

      static_assert(array_size(dwDeviceIndex) ==  array_size(dwDeviceName), "Invalid array size");
      for (unsigned j=0; j< array_size(dwDeviceName); j++) {
        LPCTSTR DeviceName = devRegisterGetName(i);
        if (_tcscmp(DeviceName, dwDeviceName[j]) == 0) {
          dwDeviceIndex[j] = i;
          break;
        }
      }  
    }
    dfe->Sort(3);
    dfe->Set(dwDeviceIndex[SelectedDevice]);
    wp->RefreshDisplay();
  }

#ifdef TESTBENCH
  
  if (configMode==CONFIGMODE_DEVICE) for(int devIdx=0; devIdx < NUMDEV; devIdx++)
  {
      ReadDeviceSettings(devIdx, deviceName1);
      {
        for (int i=0; i<DeviceRegisterCount; i++)
        {
          if (_tcscmp(devRegisterGetName(i), deviceName1) == 0)
          {
            StartupStore(_T("==================================================== %s"),NEWLINE); // 091105
            dwDeviceIndex[devIdx] = i;
            StartupStore(_T("........... SetVariable Device #%i id:%i %s %s"),devIdx,  dwDeviceIndex[devIdx], deviceName1,NEWLINE); // 091105
            ReadPortSettings(devIdx,szPort,NULL, NULL);
            StartupStore(_T("........... SetVariable Port   #%i %s %s"),devIdx,  szPort,NEWLINE); // 091105

            StartupStore(_T("........... SetVariable SppedIdx   #%i %i %s"),devIdx,  dwSpeedIndex[devIdx],NEWLINE); // 091105
            StartupStore(_T("........... SetVariable Bit        #%i %i %s"),devIdx,  dwBitIndex[devIdx],NEWLINE); // 091105
            StartupStore(_T("........... SetVariable IP-Adr     #%i %s %s"),devIdx,  szIpAddress[devIdx],NEWLINE); // 091105
            StartupStore(_T("........... SetVariable Port       #%i %i %s"),devIdx,  dwIpPort[devIdx],NEWLINE); // 091105
            StartupStore(_T("........... SetVariable Sound      #%i %i %s"),devIdx,  UseExtSound[devIdx],NEWLINE); // 091105

          }
        }
      }
  }
#endif
  UpdateButtons(wf);



  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceDisplay"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M79_ = "All on" 
    dfe->addEnumText(MsgToken(79));
	// LKTOKEN  _@M184_ = "Clip" 
    dfe->addEnumText(MsgToken(184));
    dfe->addEnumText(TEXT("Auto"));
	// LKTOKEN  _@M77_ = "All below" 
    dfe->addEnumText(MsgToken(77));
    dfe->Set(AltitudeMode_Config);
    wp->RefreshDisplay();
      wp = (WndProperty*)wf->FindByName(TEXT("prpClipAltitude"));
      if (wp) wp->SetVisible(AltitudeMode_Config==CLIP);
      wp = (WndProperty*)wf->FindByName(TEXT("prpAltWarningMargin"));
      if (wp) wp->SetVisible(AltitudeMode_Config==AUTO || AltitudeMode_Config==ALLBELOW);
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceFillType"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if(dfe) {
        dfe->addEnumText(MsgToken(941));
#ifdef HAVE_HATCHED_BRUSH
        dfe->addEnumText(MsgToken(942));
        dfe->addEnumText(MsgToken(945));
#endif
        if (LKSurface::AlphaBlendSupported()) {
            dfe->addEnumText(MsgToken(943));
            dfe->addEnumText(MsgToken(946));
        }
    }
    dfe->Set((int)MapWindow::GetAirSpaceFillType());
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceOpacity"));
  if (wp) {
    wp->SetVisible(true);
    DataField* dfe = wp->GetDataField();
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
DataField* dfe = wp->GetDataField();
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
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontMapWaypoint);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontMapTopology"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontMapTopology);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontInfopage1L"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontInfopage1L);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontBottomBar"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontBottomBar);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSnailScale"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(SnailScale);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontVisualGlide"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontVisualGlide);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayTarget"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontOverlayMedium);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayValues"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
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
    DataField* dfe = wp->GetDataField();

    dfe->addEnumText(MsgToken(955)); // Clear Type Compatible
    dfe->addEnumText(MsgToken(956)); // Anti Aliasing
    dfe->addEnumText(MsgToken(480)); // Normal
    dfe->addEnumText(MsgToken(479)); // None
    dfe->Set(FontRenderer);
    #ifdef __linux__
    wp->SetVisible(false);
    #endif
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpAspPermDisable"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(968)); // _@M968_ "for this time only"
    dfe->addEnumText(MsgToken(969)); // _@M969_ "permanent"
    dfe->Set(AspPermanentChanged);
    wp->RefreshDisplay();
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeMode"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M382_ = "Landables only" 
    dfe->addEnumText(MsgToken(382));
	// LKTOKEN  _@M380_ = "Landables and Turnpoints" 
    dfe->addEnumText(MsgToken(380));
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
    wp->GetDataField()->SetAsInteger(debounceTimeout);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableFLARMMap"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(239)); // Disabled
    dfe->addEnumText(MsgToken(259)); // Enabled
    // dfe->addEnumText(MsgToken(959)); // OFF
    // dfe->addEnumText(MsgToken(496)); // ON fixed
    // dfe->addEnumText(MsgToken(497)); // ON scaled
    dfe->Set((int)EnableFLARMMap);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointLabels"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M454_ = "Names" 
    dfe->addEnumText(MsgToken(454));

	// LKTOKEN  _@M488_ = "Numbers" 
    dfe->addEnumText(MsgToken(488));

	// LKTOKEN  _@M453_ = "Names in task" 
    dfe->addEnumText(MsgToken(453));

	// LKTOKEN  _@M301_ = "First 3" 
    dfe->addEnumText(MsgToken(301));

	// LKTOKEN  _@M302_ = "First 5" 
    dfe->addEnumText(MsgToken(302));

	// LKTOKEN  _@M838_ = "First 8" 
    dfe->addEnumText(MsgToken(838));
	// LKTOKEN  _@M839_ = "First 10" 
    dfe->addEnumText(MsgToken(839));
	// LKTOKEN  _@M840_ = "First 12" 
    dfe->addEnumText(MsgToken(840));

	// LKTOKEN  _@M479_ = "None" 
    dfe->addEnumText(MsgToken(479));
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
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M737_ = "Track up" 
    dfe->addEnumText(MsgToken(737));
	// LKTOKEN  _@M483_ = "North up" 
    dfe->addEnumText(MsgToken(483));
	// LKTOKEN  _@M482_ = "North circling" 
    dfe->addEnumText(MsgToken(482));
	// LKTOKEN  _@M682_ = "Target circling" 
    dfe->addEnumText(MsgToken(682));
	// LKTOKEN  _@M484_ = "North/track" 
    dfe->addEnumText(MsgToken(484));
	// LKTOKEN  _@M481_ = "North Smart" 
    dfe->addEnumText(MsgToken(481)); // 100417
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
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M959_ = "OFF" 
    dfe->addEnumText(MsgToken(959));
	// LKTOKEN  _@M393_ = "Line" 
    dfe->addEnumText(MsgToken(393));
	// LKTOKEN  _@M609_ = "Shade" 
    dfe->addEnumText(MsgToken(609));
        // "Line+NextWP"
    dfe->addEnumText(MsgToken(1805));
        // "Shade+NextWP"
    dfe->addEnumText(MsgToken(1806));
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
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M418_ = "Manual" 
    dfe->addEnumText(MsgToken(418));
	// LKTOKEN  _@M175_ = "Circling" 
    dfe->addEnumText(MsgToken(175));
    dfe->addEnumText(TEXT("ZigZag"));
	// LKTOKEN  _@M149_ = "Both" 
    dfe->addEnumText(MsgToken(149));
    dfe->addEnumText(MsgToken(1793)); // External

    wp->GetDataField()->Set(AutoWindMode_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoMcMode"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(290));  // Final Glide
    dfe->addEnumText(MsgToken(1684)); // Average thermal
    dfe->addEnumText(MsgToken(1685)); // Final + Average
    dfe->addEnumText(MsgToken(262));  // Equivalent
    wp->GetDataField()->Set(AutoMcMode_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointsOutOfRange"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M100_ = "Ask" 
    dfe->addEnumText(MsgToken(100));
	// LKTOKEN  _@M350_ = "Include" 
    dfe->addEnumText(MsgToken(350));
	// LKTOKEN  _@M269_ = "Exclude" 
    dfe->addEnumText(MsgToken(269));
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
    DataField* dfe = wp->GetDataField();
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
    DataField* dfe = wp->GetDataField();
    for (int i=0; i<=14; ++i) {
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
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M667_ = "Statute" 
    dfe->addEnumText(MsgToken(667));
	// LKTOKEN  _@M455_ = "Nautical" 
    dfe->addEnumText(MsgToken(455));
	// LKTOKEN  _@M436_ = "Metric" 
    dfe->addEnumText(MsgToken(436));
    dfe->Set((int)SpeedUnit_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLatLon"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
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
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M667_ = "Statute" 
    dfe->addEnumText(MsgToken(667));
	// LKTOKEN  _@M455_ = "Nautical" 
    dfe->addEnumText(MsgToken(455));
	// LKTOKEN  _@M436_ = "Metric" 
    dfe->addEnumText(MsgToken(436));
    dfe->Set(TaskSpeedUnit_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsDistance"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M667_ = "Statute" 
    dfe->addEnumText(MsgToken(667));
	// LKTOKEN  _@M455_ = "Nautical" 
    dfe->addEnumText(MsgToken(455));
	// LKTOKEN  _@M436_ = "Metric" 
    dfe->addEnumText(MsgToken(436));
    dfe->Set(DistanceUnit_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsAltitude"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(TEXT("feet"));
    dfe->addEnumText(TEXT("meters"));
    dfe->Set(AltitudeUnit_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLift"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(TEXT("knots"));
    dfe->addEnumText(TEXT("m/s"));
    dfe->addEnumText(TEXT("ft/min"));
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
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M959_ = "OFF" 
    dfe->addEnumText(MsgToken(959));
	// LKTOKEN  _@M427_ = "Mark center" 
    dfe->addEnumText(MsgToken(427));
	// LKTOKEN  _@M518_ = "Pan to center" 
    dfe->addEnumText(MsgToken(518));
    dfe->Set(EnableThermalLocator);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSetSystemTimeFromGPS"));
  if (wp) {
#if defined(PPC2003) || defined(PNA)
      wp->SetVisible(true);
      wp->GetDataField()->Set(SetSystemTimeFromGPS);
      wp->RefreshDisplay();
#else
      wp->SetVisible(false);
#endif
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
    wp->GetDataField()->SetAsInteger(LiveTrackerInterval);
    wp->GetDataField()->SetUnits(TEXT("sec"));
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpLiveTrackerRadar_config"));
  if (wp) {
    wp->GetDataField()->Set(LiveTrackerRadar_config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLiveTrackerStart_config"));
  if (wp)
  {
	DataField* dfe = wp->GetDataField();
	dfe->addEnumText(MsgToken(2334));	// LKTOKEN   _@M2334_ "In flight only (default)"
	dfe->addEnumText(MsgToken(2335));	// LKTOKEN  _@M2335_ "permanent (test purpose)"
    dfe->Set(LiveTrackerStart_config);
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
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M499_ = "Off" 
    dfe->addEnumText(MsgToken(499));
	// LKTOKEN  _@M410_ = "Long" 
    dfe->addEnumText(MsgToken(410));
	// LKTOKEN  _@M612_ = "Short" 
    dfe->addEnumText(MsgToken(612));
	// LKTOKEN  _@M312_ = "Full" 
    dfe->addEnumText(MsgToken(312));
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
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_CONF),_T("*" LKS_PILOT));
      dfe->Set(0);
    }
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAircraftFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_CONF), _T("*" LKS_AIRCRAFT));
      dfe->Set(0);
    }
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpDeviceFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_CONF), _T("*" LKS_DEVICE));
      dfe->Set(0);
    }
    wp->RefreshDisplay();
  }

  if (_tcscmp(szPolarFile,_T(""))==0) 
    _tcscpy(temptext,_T(LKD_DEFAULT_POLAR));
  else
    _tcscpy(temptext,szPolarFile);

  wp = (WndProperty*)wf->FindByName(TEXT("prpPolarFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_POLARS), _T("*" LKS_POLARS)); //091101
#ifdef LKD_SYS_POLAR
      /**
       * Add entry from system directory not exist in data directory.
       */
#ifdef ANDROID
      dfe->ScanZipDirectory(_T(LKD_SYS_POLAR), _T("*" LKS_POLARS));
#else
#warning "not implemented"
#endif
#endif
      dfe->Sort();
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szAirspaceFile);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_AIRSPACES), _T("*" LKS_AIRSPACES));
      dfe->ScanDirectoryTop(_T(LKD_AIRSPACES), _T("*" LKS_OPENAIP));
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szAdditionalAirspaceFile);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_AIRSPACES), _T("*" LKS_AIRSPACES));
      dfe->ScanDirectoryTop(_T(LKD_AIRSPACES), _T("*" LKS_OPENAIP));
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szWaypointFile);
  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_WINPILOT));
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_XCSOAR));
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_CUP));
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_COMPE));
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_OPENAIP));
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szAdditionalWaypointFile);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_WINPILOT));
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_XCSOAR));
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_CUP));
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_COMPE));
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_OPENAIP));
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  WndButton *cmdLKMapOpen = ((WndButton *) wf->FindByName(TEXT("cmdLKMapOpen")));
  if (cmdLKMapOpen) {
#ifdef ANDROID
    cmdLKMapOpen->SetVisible(true);
    jclass cls = Java::GetEnv()->FindClass("org/LK8000/LKMaps");
    if (cls != nullptr) {
      jmethodID mid = Java::GetEnv()->GetStaticMethodID(cls, "isPackageInstalled",
                                                        "(Landroid/content/Context;)Z");
      if (mid != nullptr) {
        jboolean isPackageInstalled = Java::GetEnv()->CallStaticBooleanMethod(cls, mid, context->Get());
        if ( isPackageInstalled )
          cmdLKMapOpen->SetWndText(MsgToken(2329));
        else
          cmdLKMapOpen->SetWndText(MsgToken(2328));
      }
    }
#else
    cmdLKMapOpen->SetVisible(false);
#endif
  }

  _tcscpy(temptext,szMapFile);
  wp = (WndProperty*)wf->FindByName(TEXT("prpMapFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_MAPS), _T("*" LKS_MAPS));
      dfe->ScanDirectoryTop(_T(LKD_MAPS), _T("*" XCS_MAPS));
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szTerrainFile);
  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_MAPS), _T("*" LKS_TERRAINDEM));
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szAirfieldFile);
  wp = (WndProperty*)wf->FindByName(TEXT("prpAirfieldFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_AIRFIELDS));
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szLanguageFile);
  if (_tcslen(temptext)==0) {
	_tcscpy(temptext,_T(LKD_DEFAULT_LANGUAGE));
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLanguageFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_LANGUAGE), _T("*" LKS_LANGUAGE));
#ifdef LKD_SYS_LANGUAGE
      /**
       * Add entry from system directory not exist in data directory.
       */
#ifdef ANDROID      
      dfe->ScanZipDirectory(_T(LKD_SYS_LANGUAGE), _T("*" LKS_LANGUAGE));
#else
#warning "not implemented"
#endif
      dfe->Sort();
#endif
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szInputFile);
  wp = (WndProperty*)wf->FindByName(TEXT("prpInputFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_CONF), _T("*" LKS_INPUT));
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxModel"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
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
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M328_ = "Glider" 
    dfe->addEnumText(MsgToken(328));
	// LKTOKEN  _@M520_ = "Paraglider/Delta" 
    dfe->addEnumText(MsgToken(520));
	// LKTOKEN  _@M163_ = "Car" 
    dfe->addEnumText(MsgToken(2148));
	// LKTOKEN  _@M313_ = "GA Aircraft" 
    dfe->addEnumText(MsgToken(313));
    dfe->Set(AircraftCategory);
    wp->RefreshDisplay();
  }


#if (0)
  wp = (WndProperty*)wf->FindByName(TEXT("prpAltArrivMode"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = wp->GetDataField();
	// LKTOKEN  _@M768_ = "Use MacCready" 
     dfe->addEnumText(MsgToken(768));
	// LKTOKEN  _@M90_ = "Always use MC=0" 
     dfe->addEnumText(MsgToken(90));
	// LKTOKEN  _@M91_ = "Always use safety MC" 
     dfe->addEnumText(MsgToken(91));
	// LKTOKEN  _@M769_ = "Use aver.efficiency" 
     dfe->addEnumText(MsgToken(769));
     dfe->Set(AltArrivMode);
    wp->RefreshDisplay();
  }
#endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpCheckSum"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken(239));
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(MsgToken(259));
    dfe->Set(CheckSum);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpIphoneGestures"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M364_ = "Inverted" 
    dfe->addEnumText(MsgToken(364));
	// LKTOKEN  _@M480_ = "Normal" 
    dfe->addEnumText(MsgToken(480));
    dfe->Set(IphoneGestures);
    wp->RefreshDisplay();
  }

  wp = (WndProperty *) wf->FindByName(TEXT("prpAndroidScreenOrientation"));
  if (wp) {
#ifdef ANDROID
    wp->SetVisible(true);
    DataField *dfe = wp->GetDataField();
    dfe->addEnumText(TEXT("Auto"));
    dfe->addEnumText(TEXT("Portrait"));
    dfe->addEnumText(TEXT("Landscape"));
    dfe->addEnumText(TEXT("Reverse Portrait"));
    dfe->addEnumText(TEXT("Reverse Landscape"));
    dfe->Set(native_view->getScreenOrientation());
    wp->RefreshDisplay();
#else
    wp->SetVisible(false);
#endif
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPollingMode"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M480_ = "Normal" 
    dfe->addEnumText(MsgToken(480));
	// LKTOKEN  _@M529_ = "Polling" 
    dfe->addEnumText(MsgToken(529));
    dfe->Set(PollingMode);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKVarioBar"));
  if (wp) {
    TCHAR newtoken[150];
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken(239));
	// LKTOKEN  _@M782_ = "Vario rainbow" 
    dfe->addEnumText(MsgToken(782));
	// LKTOKEN  _@M780_ = "Vario black" 
    dfe->addEnumText(MsgToken(780));
	// LKTOKEN  _@M783_ = "Vario red+blue" 
    dfe->addEnumText(MsgToken(783));
	// LKTOKEN  _@M781_ = "Vario green+red" 
    dfe->addEnumText(MsgToken(781));

    _stprintf(newtoken,_T("%s %s"),MsgToken(953),MsgToken(782) );
    dfe->addEnumText(newtoken);
    _stprintf(newtoken,_T("%s %s"),MsgToken(953),MsgToken(780) );
    dfe->addEnumText(newtoken);
    _stprintf(newtoken,_T("%s %s"),MsgToken(953),MsgToken(783) );
    dfe->addEnumText(newtoken);
    _stprintf(newtoken,_T("%s %s"),MsgToken(953),MsgToken(781) );
    dfe->addEnumText(newtoken);

    dfe->Set(LKVarioBar);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKVarioVal"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(1425)); // vario in thermal and cruise
    dfe->addEnumText(MsgToken(1426));  // vario in thermal, netto in cruise
    dfe->addEnumText(MsgToken(1427));  // vario in thermal, sollfahr in cruise
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
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken(239));
	// LKTOKEN  _@M414_ = "Low" 
    dfe->addEnumText(MsgToken(414));
	// LKTOKEN  _@M433_ = "Medium" 
    dfe->addEnumText(MsgToken(433));
	// LKTOKEN  _@M339_ = "High" 
    dfe->addEnumText(MsgToken(339));
	// LKTOKEN  _@M786_ = "Very High" 
    dfe->addEnumText(MsgToken(786));
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
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken(239));
	// LKTOKEN  +_@M2149_ = "in thermal"
    dfe->addEnumText(MsgToken(2149));
	// LKTOKEN  +_@M2150_ = "in thermal and cruise"
    dfe->addEnumText(MsgToken(2150));
    dfe->addEnumText(MsgToken(1833)); // Always
    dfe->Set(ThermalBar);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayClock"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    // LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken(239));
    // LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(MsgToken(259));
    dfe->Set(OverlayClock);
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpGliderSymbol"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(227));  //  Default
    dfe->addEnumText(MsgToken(2317)); // _@M2317_ "Triangle"
    dfe->addEnumText(MsgToken(2318)); // _@M2318_ "Paraglider"
    dfe->addEnumText(MsgToken(2319)); // _@M2319_ "Hangglider"
    dfe->addEnumText(MsgToken(2320)); // _@M2320_ "Big Glider"
    dfe->addEnumText(MsgToken(2321)); // _@M2322_ "Aircraft"
    dfe->addEnumText(MsgToken(2325)); //  "Small Glider"
    dfe->addEnumText(MsgToken(2326)); //  "Gem"
    dfe->addEnumText(MsgToken(2331)); //  "Glider Black"
    dfe->addEnumText(MsgToken(2332)); //  "Big Glider Black"
    dfe->Set(GliderSymbol);
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
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M78_ = "All black" 
    dfe->addEnumText(MsgToken(78));
	// LKTOKEN  _@M778_ = "Values white" 
    dfe->addEnumText(MsgToken(778));
	// LKTOKEN  _@M81_ = "All white" 
    dfe->addEnumText(MsgToken(81));
    dfe->Set(OutlinedTp_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTpFilter"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M473_ = "No landables" 
    dfe->addEnumText(MsgToken(473));
	// LKTOKEN  _@M80_ = "All waypoints" 
    dfe->addEnumText(MsgToken(80));
	// LKTOKEN  _@M211_ = "DAT Turnpoints" 
    dfe->addEnumText(MsgToken(211));
    dfe->Set(TpFilter);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverColor"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M816_ = "White" 
    dfe->addEnumText(MsgToken(816));
	// LKTOKEN  _@M144_ = "Black" 
    dfe->addEnumText(MsgToken(144));
	// LKTOKEN  _@M218_ = "DarkBlue" 
    dfe->addEnumText(MsgToken(218));
	// LKTOKEN  _@M825_ = "Yellow" 
    dfe->addEnumText(MsgToken(825));
	// LKTOKEN  _@M331_ = "Green" 
    dfe->addEnumText(MsgToken(331));
	// LKTOKEN  _@M505_ = "Orange" 
    dfe->addEnumText(MsgToken(505));
	// LKTOKEN  _@M209_ = "Cyan" 
    dfe->addEnumText(MsgToken(209));
	// LKTOKEN  _@M417_ = "Magenta" 
    dfe->addEnumText(MsgToken(417));
	// LKTOKEN  _@M332_ = "Grey" 
    dfe->addEnumText(MsgToken(332));
	// LKTOKEN  _@M215_ = "Dark Grey" 
    dfe->addEnumText(MsgToken(215));
	// LKTOKEN  _@M216_ = "Dark White" 
    dfe->addEnumText(MsgToken(216));
	// LKTOKEN  _@M92_ = "Amber" 
    dfe->addEnumText(MsgToken(92));
	// LKTOKEN  _@M391_ = "Light Green" 
    dfe->addEnumText(MsgToken(391));
	// LKTOKEN  _@M522_ = "Petrol" 
    dfe->addEnumText(MsgToken(522));
    dfe->Set(OverColor);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMapBox"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M763_ = "Unboxed, no units" 
    dfe->addEnumText(MsgToken(763));
	// LKTOKEN  _@M764_ = "Unboxed, with units" 
    dfe->addEnumText(MsgToken(764));
	// LKTOKEN  _@M152_ = "Boxed, no units" 
    dfe->addEnumText(MsgToken(152));
	// LKTOKEN  _@M153_ = "Boxed, with units" 
    dfe->addEnumText(MsgToken(153));
    dfe->Set(MapBox);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlaySize"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M842_ = "Big font " 
    dfe->addEnumText(MsgToken(842));
	// LKTOKEN  _@M843_ = "Small font " 
    dfe->addEnumText(MsgToken(843));
    dfe = wp->GetDataField();
    dfe->Set(OverlaySize);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGlideBarMode"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken(239));
	// LKTOKEN  _@M461_ = "Next turnpoint" 
    dfe->addEnumText(MsgToken(461));
	// LKTOKEN  _@M299_ = "Finish" 
    dfe->addEnumText(MsgToken(299));
    dfe->Set(GlideBarMode);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpArrivalValue"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M98_ = "ArrivalAltitude" 
    dfe->addEnumText(MsgToken(98));
	// LKTOKEN  _@M254_ = "EfficiencyReq" 
    dfe->addEnumText(MsgToken(254));
    dfe->Set(ArrivalValue);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpNewMapDeclutter"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken(239));
	// LKTOKEN  _@M414_ = "Low" 
    dfe->addEnumText(MsgToken(414));
	// LKTOKEN  _@M339_ = "High" 
    dfe->addEnumText(MsgToken(339));
    dfe->Set(NewMapDeclutter);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAverEffTime"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(1775)); // 3 sec
    dfe->addEnumText(MsgToken(1776)); // 5 sec
    dfe->addEnumText(MsgToken(1777)); // 10 sec
	// LKTOKEN  _@M17_ = "15 seconds" 
    dfe->addEnumText(MsgToken(17));
	// LKTOKEN  _@M30_ = "30 seconds" 
    dfe->addEnumText(MsgToken(30));
    dfe->addEnumText(MsgToken(1778)); // 45 sec
	// LKTOKEN  _@M35_ = "60 seconds" 
    dfe->addEnumText(MsgToken(35));
	// LKTOKEN  _@M39_ = "90 seconds" 
    dfe->addEnumText(MsgToken(39));
	// LKTOKEN  _@M23_ = "2 minutes" 
    dfe->addEnumText(MsgToken(23));
	// LKTOKEN  _@M29_ = "3 minutes" 
    dfe->addEnumText(MsgToken(29));
    dfe->Set(AverEffTime);
    wp->RefreshDisplay();
 }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBgMapcolor"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M816_ = "White" 
    dfe->addEnumText(MsgToken(816));
	// LKTOKEN  _@M392_ = "Light grey" 
    dfe->addEnumText(MsgToken(392));
	// LKTOKEN  _@M374_ = "LCD green" 
    dfe->addEnumText(MsgToken(374));
	// LKTOKEN  _@M373_ = "LCD darkgreen" 
    dfe->addEnumText(MsgToken(373));
	// LKTOKEN  _@M332_ = "Grey" 
    dfe->addEnumText(MsgToken(332));
	// LKTOKEN  _@M147_ = "Blue lake" 
    dfe->addEnumText(MsgToken(147));
	// LKTOKEN  _@M256_ = "Emerald green" 
    dfe->addEnumText(MsgToken(256));
	// LKTOKEN  _@M217_ = "Dark grey" 
    dfe->addEnumText(MsgToken(217));
	// LKTOKEN  _@M567_ = "Rifle grey" 
    dfe->addEnumText(MsgToken(567));
	// LKTOKEN  _@M144_ = "Black" 
    dfe->addEnumText(MsgToken(144));
    dfe->Set(BgMapColor_Config);
    wp->RefreshDisplay();
 }


  wp = (WndProperty*)wf->FindByName(TEXT("prpAppIndLandable"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(1797)); // Vector
	// LKTOKEN  _@M2327_ "Bitmap"
    dfe->addEnumText(MsgToken(2327));
    dfe->Set(Appearance.IndLandable);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInverseInfoBox"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M958_ = "ON" 
    dfe->addEnumText(MsgToken(958));
	// LKTOKEN  _@M959_ = "OFF" 
    dfe->addEnumText(MsgToken(959));
    dfe->Set(InverseInfoBox_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGliderScreenPosition"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(MapWindow::GliderScreenPosition);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoContrast"));
  if (wp) {
     wp->GetDataField()->Set(AutoContrast);
     wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainContrast"));
  if (wp) {

    wp->GetDataField()->SetAsFloat(iround((TerrainContrast*100.0)/255.0));
    if (AutoContrast) wp->SetReadOnly(true); // needed on dlg startup
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainBrightness"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround((TerrainBrightness*100.0)/255.0));
    if (AutoContrast) wp->SetReadOnly(true); // needed on dlg startup
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainRamp"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M413_ = "Low lands" 
    dfe->addEnumText(MsgToken(413));
	// LKTOKEN  _@M439_ = "Mountainous" 
    dfe->addEnumText(MsgToken(439));
    dfe->addEnumText(TEXT("Imhof 7"));
    dfe->addEnumText(TEXT("Imhof 4"));
    dfe->addEnumText(TEXT("Imhof 12"));
    dfe->addEnumText(TEXT("Imhof Atlas"));
    dfe->addEnumText(TEXT("ICAO")); 
	// LKTOKEN  _@M377_ = "LKoogle lowlands" 
    dfe->addEnumText(MsgToken(377)); 
	// LKTOKEN  _@M378_ = "LKoogle mountains" 
    dfe->addEnumText(MsgToken(378));
	// LKTOKEN  _@M412_ = "Low Alps" 
    dfe->addEnumText(MsgToken(412));
	// LKTOKEN  _@M338_ = "High Alps" 
    dfe->addEnumText(MsgToken(338));
    dfe->addEnumText(TEXT("YouSee"));
	// LKTOKEN  _@M340_ = "HighContrast" 
    dfe->addEnumText(MsgToken(340));
    dfe->addEnumText(TEXT("GA Relative"));
    dfe->addEnumText(TEXT("LiteAlps"));
    dfe->addEnumText(TEXT("Low Hills"));
    dfe->Set(TerrainRamp_Config);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoBacklight")); // VENTA4
  if (wp) {
#ifndef PPC2003   // PNA is also PPC2003
    wp->SetVisible(false);
#else
    wp->SetVisible(true);
#endif
    wp->GetDataField()->Set(EnableAutoBacklight);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoSoundVolume")); // VENTA4
  if (wp) {
#ifndef PPC2003   // PNA is also PPC2003
    	wp->SetVisible(false);
#else
        wp->SetVisible(true);
#endif
    wp->GetDataField()->Set(EnableAutoSoundVolume);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishLine"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M210_ = "Cylinder" 
    dfe->addEnumText(MsgToken(210));
	// LKTOKEN  _@M393_ = "Line" 
    dfe->addEnumText(MsgToken(393));
	// LKTOKEN  _@M274_ = "FAI Sector" 
    dfe->addEnumText(MsgToken(274));
    dfe->Set(FinishLine);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFinishRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(FinishRadius*DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartLine"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M210_ = "Cylinder" 
    dfe->addEnumText(MsgToken(210));
	// LKTOKEN  _@M393_ = "Line" 
    dfe->addEnumText(MsgToken(393));
	// LKTOKEN  _@M274_ = "FAI Sector" 
    dfe->addEnumText(MsgToken(274));
    dfe->Set(StartLine);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskStartRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(StartRadius*DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskFAISector"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M210_ = "Cylinder" 
    dfe->addEnumText(MsgToken(210));
	// LKTOKEN  _@M274_ = "FAI Sector" 
    dfe->addEnumText(MsgToken(274));
    dfe->addEnumText(gettext(TEXT("DAe 0.5/10")));
    dfe->addEnumText(MsgToken(393));
    
    dfe->Set(SectorType);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskSectorRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(SectorRadius*DISTANCEMODIFY*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoAdvance"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken(418)); // Manual
    dfe->addEnumText(MsgToken(897)); // Auto
    dfe->addEnumText(MsgToken(97)); // Arm
    dfe->addEnumText(MsgToken(96)); // Arm start
    dfe->Set(AutoAdvance_Config);
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
	DataField* dfe = wp->GetDataField();
	dfe->addEnumText(MsgToken(1831));	// LKTOKEN  _@M1831_ "Off"
	dfe->addEnumText(MsgToken(1832));	// LKTOKEN  _@M1832_ "Near landables"
	dfe->addEnumText(MsgToken(1833));	// LKTOKEN   _@M1833_ "Allways"
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
    DataField* dfe = wp->GetDataField();
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

#ifdef ANDROID

#define UPDATE_COM_PORT 1

static void OnLeScan(WndForm* pWndForm, const char *address, const char *name) {
#ifdef ANDROID
  ScopeLock lock(COMMPort_mutex);
#endif

  std::stringstream prefixed_address_stream;
  prefixed_address_stream << "BT:" << address;
  const std::string prefixed_address(prefixed_address_stream.str());

  COMMPort_t::const_iterator It = std::find_if(COMMPort.begin(),
                                               COMMPort.end(),
                                               std::bind(&COMMPortItem_t::IsSamePort, _1,
                                                         prefixed_address.c_str()));

  if (It == COMMPort.end()) {
    std::stringstream prefixed_name_stream;
    prefixed_name_stream << "BT:" << ((strlen(name)>0)? name : address);
    const std::string prefixed_name(prefixed_name_stream.str());

    COMMPort.push_back(COMMPortItem_t(std::move(prefixed_address), std::move(prefixed_name)));
  }
  if(pWndForm) {
    pWndForm->SendUser(UPDATE_COM_PORT);
  }
}

static bool OnUser(WndForm * pWndForm, unsigned id) {
  switch(id) {
    case UPDATE_COM_PORT: {
      WndProperty *pWnd = static_cast<WndProperty *>(pWndForm->FindByName(TEXT("prpComPort1")));
      if (pWnd) {
        DataField * dataField = pWnd->GetDataField();
        if(dataField) {

#ifdef ANDROID
          ScopeLock lock(COMMPort_mutex);
#endif
          for( const auto& item : COMMPort ) {
            if(dataField->Find(item.GetName()) == -1) {
              dataField->addEnumText(item.GetName(), item.GetLabel());
            }
          }
        }
      }
      return true;
    }
    default:
      break;
  }
  return false;
}

#endif

void dlgConfigurationShowModal(short mode){

  WndProperty *wp;

  configMode=mode;

  typedef struct {
      const unsigned resid;
  }  dlgTemplate_t;
  
  static const dlgTemplate_t dlgTemplate_L [] = { 
      { IDR_XML_CONFIGURATION_L },
      { IDR_XML_CONFIGPILOT_L },
      { IDR_XML_CONFIGAIRCRAFT_L },
      { IDR_XML_CONFIGDEVICE_L }
  };
  
  static const dlgTemplate_t dlgTemplate_P [] = { 
      { IDR_XML_CONFIGURATION_P },
      { IDR_XML_CONFIGPILOT_P },
      { IDR_XML_CONFIGAIRCRAFT_P },
      { IDR_XML_CONFIGDEVICE_P }
  };
  
  static_assert(array_size(dlgTemplate_L) == array_size(dlgTemplate_P), "check array size");
  
  StartHourglassCursor();

  if (configMode==CONFIGMODE_DEVICE) {
    RefreshComPortList();
  }

  if(configMode >= 0 && configMode < (int)array_size(dlgTemplate_L)) {
    
    auto dlgTemplate = (ScreenLandscape ? dlgTemplate_L[configMode] : dlgTemplate_P[configMode]);
  
    wf = dlgLoadFromXML(CallBackTable, dlgTemplate.resid);
    
  } else {
      wf = nullptr;
  }
  
  if (!wf) {
	return;
  }

#ifdef ANDROID
  wf->SetOnUser(OnUser);

  std::unique_ptr<BluetoothLeScan> BluetoothLeScanPtr;
  if(configMode==CONFIGMODE_DEVICE) {
    // Start Bluetooth LE device scan before Open Dialog
    BluetoothLeScanPtr.reset(new BluetoothLeScan(wf, OnLeScan));
  }
#endif

  wf->SetKeyDownNotify(FormKeyDown);

  LKASSERT((WndButton *)wf->FindByName(TEXT("cmdClose")));
  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  reset_wConfig();

  for(size_t i = 0; i < array_size(ConfigPageNames[configMode]); i++) {
      wConfig[i]    = (WndFrame *)wf->FindByName(ConfigPageNames[configMode][i].szName);
      numPages += wConfig[i]?1:0;
  }
  std::fill(std::begin(cpyInfoBox), std::end(cpyInfoBox), -1);

  wf->FilterAdvanced(1); // useless, we dont use advanced options anymore TODO remove

  setVariables(wf);

  if (configMode==CONFIGMODE_DEVICE) {
	TCHAR deviceName1[MAX_PATH];
//      TCHAR deviceName2[MAX_PATH];
	ReadDeviceSettings(SelectedDevice, deviceName1);
//      ReadDeviceSettings(1, deviceName2);
	UpdateDeviceSetupButton(wf, SelectedDevice);
//      UpdateDeviceSetupButton(1, deviceName2);
// Don't show external sound config if not compiled for this device or not used
#ifdef DISABLEEXTAUDIO
        ShowWindowControl(wf, _T("prpExtSound1"), false);
//      ShowWindowControl(wf, _T("prpExtSound2"), false);
#else
        if (!IsSoundInit()) {
           ShowWindowControl(wf, _T("prpExtSound1"), false);
//         ShowWindowControl(wf, _T("prpExtSound2"), false);
        }
#endif
     extern void InitDlgDevice(WndForm *wf);
     InitDlgDevice(wf);


  } // end configmode_device

  wp = static_cast<WndProperty*>(wf->FindByName(_T("prpEarthModel")));
  if(wp) {
#ifdef _WGS84
    DataField* pDataField = wp->GetDataField();
    if(pDataField) {
      pDataField->addEnumText(_T("FAI Sphere"));
      pDataField->addEnumText(_T("WGS84 Ellipsoid"));
      pDataField->Set(earth_model_wgs84);
    }
    wp->RefreshDisplay();
#else
    wp->SetVisible(false);
#endif
  }


  NextPage(0);

  taskchanged = false;
  requirerestart = false;
  utcchanged = false;
  waypointneedsave = false;

  StopHourglassCursor();
  wf->ShowModal();

#ifdef ANDROID
    // stop LE Scaner first aftar dialog close
    BluetoothLeScanPtr.reset();
#endif

#ifdef _WGS84
  wp = static_cast<WndProperty*>(wf->FindByName(_T("prpEarthModel")));
  if(wp) {
    DataField* pDataField = wp->GetDataField();
    if(pDataField) {
      if(pDataField->GetAsBoolean() != earth_model_wgs84) {
        earth_model_wgs84 = pDataField->GetAsBoolean();
        requirerestart = true;
      }
    }
  }
#endif

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
    if (LiveTrackerInterval != wp->GetDataField()->GetAsInteger()) {
      LiveTrackerInterval = (wp->GetDataField()->GetAsInteger());
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLiveTrackerRadar_config"));
  if (wp) {
    if (LiveTrackerRadar_config != wp->GetDataField()->GetAsBoolean()) {
      LiveTrackerRadar_config = wp->GetDataField()->GetAsBoolean();
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
#if defined(PPC2003) || defined(PNA)
  wp = (WndProperty*)wf->FindByName(TEXT("prpSetSystemTimeFromGPS"));
  if (wp) {
    if (SetSystemTimeFromGPS != wp->GetDataField()->GetAsBoolean()) {
      SetSystemTimeFromGPS = wp->GetDataField()->GetAsBoolean();
    }
  }
#endif

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
    if (debounceTimeout != (unsigned)wp->GetDataField()->GetAsInteger()) {
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpSnailScale"));
  if (wp) {
      if (SnailScale != wp->GetDataField()->GetAsInteger() ) {
          SnailScale = wp->GetDataField()->GetAsInteger();
          snailchanged=true;
      }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontVisualGlide"));
  if (wp) {
      if (FontVisualGlide != wp->GetDataField()->GetAsInteger() ) {
          FontVisualGlide = wp->GetDataField()->GetAsInteger();
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
    if (AutoMacCready_Config != wp->GetDataField()->GetAsBoolean()) {
      AutoMacCready_Config = wp->GetDataField()->GetAsBoolean();
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
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLatLon"));
  if (wp) {
    if ((int)Units::CoordinateFormat != wp->GetDataField()->GetAsInteger()) {
      Units::CoordinateFormat = (CoordinateFormats_t)wp->GetDataField()->GetAsInteger();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsTaskSpeed"));
  if (wp) {
    if ((int)TaskSpeedUnit_Config != wp->GetDataField()->GetAsInteger()) {
      TaskSpeedUnit_Config = wp->GetDataField()->GetAsInteger();
      Units::NotifyUnitChanged();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsDistance"));
  if (wp) {
    if ((int)DistanceUnit_Config != wp->GetDataField()->GetAsInteger()) {
      DistanceUnit_Config = wp->GetDataField()->GetAsInteger();
      Units::NotifyUnitChanged();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLift"));
  if (wp) {
    if ((int)LiftUnit_Config != wp->GetDataField()->GetAsInteger()) {
      LiftUnit_Config = wp->GetDataField()->GetAsInteger();
      Units::NotifyUnitChanged();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsAltitude"));
  if (wp) {
    if ((int)AltitudeUnit_Config != wp->GetDataField()->GetAsInteger()) {
      AltitudeUnit_Config = wp->GetDataField()->GetAsInteger();
      Units::NotifyUnitChanged();
    }
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szWaypointFile)) {
      _tcscpy(szWaypointFile,temptext);
      WAYPOINTFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szAdditionalWaypointFile)) {
      _tcscpy(szAdditionalWaypointFile,temptext);
      WAYPOINTFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szAirspaceFile)) {
      _tcscpy(szAirspaceFile,temptext);
      AIRSPACEFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAdditionalAirspaceFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szAdditionalAirspaceFile)) {
      _tcscpy(szAdditionalAirspaceFile,temptext);
      AIRSPACEFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMapFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szMapFile)) {
      _tcscpy(szMapFile,temptext);
      MAPFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szTerrainFile)) {
      _tcscpy(szTerrainFile,temptext);
      TERRAINFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirfieldFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szAirfieldFile)) {
      _tcscpy(szAirfieldFile,temptext);
      AIRFIELDFILECHANGED= true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLanguageFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szLanguageFile)) {
      _tcscpy(szLanguageFile,temptext);
      requirerestart = true; // restart needed for language load
      // LKReadLanguageFile(); // NO GOOD. MEMORY LEAKS PENDING
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpInputFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
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

#ifdef ANDROID
  wp = (WndProperty *) wf->FindByName(TEXT("prpAndroidScreenOrientation"));
  if (wp) {
    int ret = wp->GetDataField()->GetAsInteger();
    if (ret != native_view->getScreenOrientation()) {
      native_view->setScreenOrientation(ret);
    }
  }
#endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpPollingMode")); 
  if (wp) {
    if (PollingMode != (PollingMode_t) (wp->GetDataField()->GetAsInteger())) {
      PollingMode = (PollingMode_t) (wp->GetDataField()->GetAsInteger());
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKVarioBar")); 
  if (wp) {
    const LKVarioBar_t tmpValue = static_cast<LKVarioBar_t>(wp->GetDataField()->GetAsInteger());
    if (LKVarioBar != tmpValue) {
      LKVarioBar = tmpValue;
      Reset_Single_DoInits(MDI_DRAWVARIO);
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
      if (HideUnits != (HideUnits_t) (wp->GetDataField()->GetAsBoolean())) {
          HideUnits = (HideUnits_t) (wp->GetDataField()->GetAsBoolean());
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
    if (BestWarning != (wp->GetDataField()->GetAsBoolean())) {
      BestWarning = (wp->GetDataField()->GetAsBoolean());
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpUseTotalEnergy"));
  if (wp) {
    if (UseTotalEnergy_Config != (wp->GetDataField()->GetAsBoolean())) {
      UseTotalEnergy_Config = (wp->GetDataField()->GetAsBoolean());
      UseTotalEnergy=UseTotalEnergy_Config;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpUseUngestures"));
  if (wp) {
    if (UseUngestures != (wp->GetDataField()->GetAsBoolean())) {
      UseUngestures = (wp->GetDataField()->GetAsBoolean());
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
  wp = (WndProperty*)wf->FindByName(TEXT("prpTrackBar"));
  if (wp) {
    if (TrackBar != (wp->GetDataField()->GetAsBoolean())) {
      TrackBar = (wp->GetDataField()->GetAsBoolean());
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOptimizeRoute"));
  if (wp) {
    if (PGOptimizeRoute_Config != (wp->GetDataField()->GetAsBoolean())) {
      PGOptimizeRoute_Config = (wp->GetDataField()->GetAsBoolean());
      PGOptimizeRoute = PGOptimizeRoute_Config;

      if (ISPARAGLIDER) {
	    if(PGOptimizeRoute) {
		  AATEnabled = true;
	    }
        ClearOptimizedTargetPos();
	  }
	}
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGliderSymbol"));
  if (wp) {
    if (GliderSymbol != wp->GetDataField()->GetAsInteger() )
    {
    	GliderSymbol = wp->GetDataField()->GetAsInteger();
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
	if (UseGeoidSeparation != (wp->GetDataField()->GetAsBoolean())) {
		UseGeoidSeparation = (wp->GetDataField()->GetAsBoolean());
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoBacklight")); // VENTA4
  if (wp) {
    if (EnableAutoBacklight != wp->GetDataField()->GetAsBoolean()) {
      EnableAutoBacklight = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoSoundVolume")); // VENTA4
  if (wp) {
    if (EnableAutoSoundVolume != wp->GetDataField()->GetAsBoolean()) {
      EnableAutoSoundVolume = wp->GetDataField()->GetAsBoolean();
    }
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainContrast"));
  if (wp) {
    if (iround((TerrainContrast*100)/255) != 
	wp->GetDataField()->GetAsInteger()) {
      TerrainContrast = (short)iround(wp->GetDataField()->GetAsInteger()*255.0/100);
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainBrightness"));
  if (wp) {
    if (iround((TerrainBrightness*100)/255) != 
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
  wp = (WndProperty*)wf->FindByName(TEXT("prpAlarmGearAltitude"));
  if (wp) {
	  unsigned tmp = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY) *1000.0);
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

  UpdateDeviceSetupButton(wf, SelectedDevice);

  wp = (WndProperty*)wf->FindByName(TEXT("prpEngineeringMenu")); // VENTA9
  if (wp) EngineeringMenu = wp->GetDataField()->GetAsBoolean();


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
                   MsgToken(581),
	// LKTOKEN  _@M809_ = "Waypoints edited" 
                   MsgToken(809),
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
      fontschanged=false;
  }

  if (snailchanged) {
      #if TESTBENCH
      StartupStore(_T("..... dlgConfiguration: snailchanged requested\n"));
      #endif
      SNAILCHANGED=true;
      snailchanged=false;
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
		   MsgToken(561), 
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
      AIRCRAFTTYPECHANGED=true;

        if (ISPARAGLIDER) AATEnabled=TRUE; // NOT SURE THIS IS NEEDED ANYMORE. 
    }
  }

 wp = (WndProperty*)wf->FindByName(TEXT("prpPolarFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,_T(""))==0) {
        _tcscpy(temptext,_T(LKD_DEFAULT_POLAR));
    }

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


//
// Setup device dialogs fine tuning
//
void InitDlgDevice(WndForm *pWndForm) {

  if(!pWndForm) {
    LKASSERT(0);
    return;
  }

  // spacing between buttons and left&right
  const unsigned int SPACEBORDER = DLGSCALE(2);
  const unsigned int w = (pWndForm->GetWidth() - (SPACEBORDER * (MAXNUMDEVICES + 1))) / MAXNUMDEVICES;
  unsigned int lx = SPACEBORDER; // count from 0

  static_assert(MAXNUMDEVICES == array_size(DeviceList), "wrong array size");

  for(unsigned i = 0; i < MAXNUMDEVICES; ++i) {
    TCHAR szWndName[5];
    _stprintf(szWndName, _T("cmd%c"), _T('A')+i);
    WindowControl * pWnd = pWndForm->FindByName(szWndName);
    if(pWnd) {
      pWnd->SetWidth(w);
      pWnd->SetLeft(lx);
      ((WndButton*)pWnd)->LedSetMode(LEDMODE_OFFGREEN);
      ((WndButton*)pWnd)->LedSetOnOff(!DeviceList[i].Disabled);
      if(i==0) OnA((WndButton*)pWnd);
      lx += w + SPACEBORDER;
    }
  }
}
