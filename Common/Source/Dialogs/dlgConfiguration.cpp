/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
#include "ContestMgr.h"
#include "Tracking/http_session.h"
#include "Tracking/Tracking.h"
#include "Devices/DeviceRegister.h"
#include "Library/TimeFunctions.h"

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
      /*20*/  { _T("frmSpecials2"),           _T("_@M94_"), false },  // "23 Map Scale"
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

static_assert(std::size(config_page) == std::size(ConfigPageNames), "invalid array size");
static_assert(std::size(wConfig) == std::size(ConfigPageNames[0]), "invalid array size");
static_assert(std::size(wConfig) == std::size(ConfigPageNames[1]), "invalid array size");
static_assert(std::size(wConfig) == std::size(ConfigPageNames[2]), "invalid array size");
static_assert(std::size(wConfig) == std::size(ConfigPageNames[2]), "invalid array size");


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
  if (dfe) {
    for (int i = -MAXFONTRESIZE; i <= MAXFONTRESIZE; ++i) {
      dfe->addEnumText(to_tstring(i).c_str());
    }
  }
}

static void UpdateButtons(WndForm *pOwner) {
  TCHAR text[120];
  TCHAR val[100];
if(!pOwner) return;
  if (buttonPilotName) {
    _tcscpy(val,PilotName_Config);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, MsgToken<7>());
    }
	// LKTOKEN  _@M524_ = "Pilot name" 
    _stprintf(text,TEXT("%s: %s"), MsgToken<524>(), val);
    buttonPilotName->SetCaption(text);
  }
  if (buttonAircraftType) {
    _tcscpy(val,AircraftType_Config);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, MsgToken<7>());
    }
	// LKTOKEN  _@M59_ = "Aircraft type" 
    _stprintf(text,TEXT("%s: %s"), MsgToken<59>(), val);
    buttonAircraftType->SetCaption(text);
  }
  if (buttonAircraftRego) {
    _tcscpy(val,AircraftRego_Config);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, MsgToken<7>());
    }
	// LKTOKEN  _@M57_ = "Aircraft Reg" 
    _stprintf(text,TEXT("%s: %s"), MsgToken<57>(), val);
    buttonAircraftRego->SetCaption(text);
  }
  if (buttonCompetitionClass) {
    _tcscpy(val,CompetitionClass_Config);
    if (_tcslen(val)<=0) {
      // LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, MsgToken<7>());
    }
	// LKTOKEN  _@M936_ = "Competition Class" 
    _stprintf(text,TEXT("%s: %s"), MsgToken<936>(), val);
    buttonCompetitionClass->SetCaption(text);
  }
  if (buttonCompetitionID) {
    _tcscpy(val,CompetitionID_Config);
    if (_tcslen(val)<=0) {
      // LKTOKEN  _@M7_ = "(blank)" 
      _tcscpy(val, MsgToken<7>());
    }
	// LKTOKEN  _@M938_ = "Competition ID" 
    _stprintf(text,TEXT("%s: %s"), MsgToken<938>(), val);
    buttonCompetitionID->SetCaption(text);
  }

  WindowControl* pFfvlKey = pOwner->FindByName(_T("prp_ffvl_key"));
  if (pFfvlKey) {
    pFfvlKey->SetVisible(http_session::ssl_available());
  }

  WndButton* wCmdBth = (pOwner->FindByName<WndButton>(TEXT("cmdBth")));
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
    LKASSERT((size_t)configMode<std::size(config_page));
    config_page[configMode] += Step;

    if (configMode==CONFIGMODE_SYSTEM && !EngineeringMenu) { 
        if (config_page[configMode]>=(numPages-NUMENGPAGES)) { config_page[configMode]=0; }
        if (config_page[configMode]<0) { config_page[configMode]=numPages-(NUMENGPAGES+1); } 
    } else {
        if (config_page[configMode]>=numPages) { config_page[configMode]=0; }
        if (config_page[configMode]<0) { config_page[configMode]=numPages-1; }
    }

    LKASSERT((size_t)configMode < std::size(ConfigPageNames));
    LKASSERT((size_t)config_page[configMode] < std::size(ConfigPageNames[0]));
    
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

    for (short i = 0; i < (short)std::size(wConfig); ++i) {
        if (wConfig[i]) {
            wConfig[i]->SetVisible(config_page[configMode] == i);
        }
    }
} // NextPage


static void UpdateDeviceSetupButton(WndForm* pOwner,size_t idx /*, const TCHAR *Name*/) {

  // const TCHAR * DevicePropName[] = {_T("prpComPort1")};
  // check if all array have same size ( compil time check );
  // static_assert(std::size(DeviceList) == std::size(DevicePropName), "DevicePropName array size need to be same of DeviceList array size");

  if(!pOwner)
    return;
  WndProperty* wp;

  auto& Port = PortConfig[SelectedDevice];


  wp = pOwner->FindByName<WndProperty>(TEXT("prpComPort1"));
  if (wp) {
      if (_tcscmp(Port.GetPort(), wp->GetDataField()->GetAsString()) != 0)
      {
          Port.SetPort(wp->GetDataField()->GetAsString());
          COMPORTCHANGED = true;
      }
  }

  wp = pOwner->FindByName<WndProperty>(TEXT("prpExtSound1"));
  if (wp) {
        if (Port.UseExtSound != (wp->GetDataField()->GetAsBoolean())) {
                Port.UseExtSound = (wp->GetDataField()->GetAsBoolean());
        }
  }

  wp = pOwner->FindByName<WndProperty>(TEXT("prpComSpeed1"));
  if (wp) {
    if ((int)Port.dwSpeedIndex != wp->GetDataField()->GetAsInteger()) {
      Port.dwSpeedIndex = wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
    }
  }

  wp = pOwner->FindByName<WndProperty>(TEXT("prpComBit1"));
  if (wp) {
    if ((int)Port.dwBitIndex != wp->GetDataField()->GetAsInteger()) {
      Port.dwBitIndex = static_cast<BitIndex_t>(wp->GetDataField()->GetAsInteger());
      COMPORTCHANGED = true;
    }
  }

  wp = pOwner->FindByName<WndProperty>(TEXT("prpComIpAddr1"));
  if (wp) {
    if (_tcscmp(Port.szIpAddress, wp->GetDataField()->GetAsString()) != 0) {
      _tcsncpy(Port.szIpAddress, wp->GetDataField()->GetAsString(), std::size(Port.szIpAddress));
      Port.szIpAddress[std::size(Port.szIpAddress)-1] = _T('\0');
      COMPORTCHANGED = true;
    }
  }

  wp = pOwner->FindByName<WndProperty>(TEXT("prpComIpPort1"));
  if (wp) {
    if ((int)Port.dwIpPort != wp->GetDataField()->GetAsInteger()) {
      Port.dwIpPort = wp->GetDataField()->GetAsInteger();
      COMPORTCHANGED = true;
    }
  }

  wp = pOwner->FindByName<WndProperty>(TEXT("prpComDevice1"));
  if (wp) {

    DataField* df = wp->GetDataField();
    const TCHAR* Name = df->GetAsString();
    if (tstring_view(Name) != Port.szDeviceName) {
      COMPORTCHANGED = true;
      _tcscpy(Port.szDeviceName, Name);
    }
  }

  /************************************************************************/

    UpdateComPortSetting(pOwner, idx, Port.GetPort());
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
      wp = wf->FindByName<WndProperty>(TEXT("prpAirspaceOpacity"));
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
      wp = wf->FindByName<WndProperty>(TEXT("prpAirspaceDisplay"));
      if (wp) altmode=(wp->GetDataField()->GetAsInteger());
      // Warning, this is duplicated later on
      wp = wf->FindByName<WndProperty>(TEXT("prpClipAltitude"));
      if (wp) wp->SetVisible(altmode==CLIP);
      wp = wf->FindByName<WndProperty>(TEXT("prpAltWarningMargin"));
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
      wp = wf->FindByName<WndProperty>(TEXT("prpAspPermDisable"));
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
        wp = wf->FindByName<WndProperty>(TEXT("prpAutoContrast"));
        if (wp) {
           if (AutoContrast != wp->GetDataField()->GetAsBoolean()) {
              AutoContrast = wp->GetDataField()->GetAsBoolean();
           }
        }
        wp = wf->FindByName<WndProperty>(TEXT("prpTerrainContrast"));
        if(wp) {
           if(AutoContrast) {
              wp->SetReadOnly(true);
           } else {
              wp->SetReadOnly(false);
           }
           wp->RefreshDisplay();
        }
        wp = wf->FindByName<WndProperty>(TEXT("prpTerrainBrightness"));
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
    	wp = wf->FindByName<WndProperty>(TEXT("prpAlarmGearWarning"));
    	if (wp) {
    	  ival = iround( wp->GetDataField()->GetAsInteger() );
    	  if ((int)GearWarningMode != ival) {
    	  	GearWarningMode = ival;
    	  }
    	}
        wp = wf->FindByName<WndProperty>(TEXT("prpAlarmGearAltitude"));
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
            wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(GearWarningAltitude / 1000.0)));
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
    dlgTerminal(SelectedDevice);
    UpdateDeviceEntries(pWnd->GetParentWndForm(), SelectedDevice);
}

extern bool SysOpMode;
extern bool Sysop(TCHAR *command);

static void OnPilotNameClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonPilotName) {
        _tcscpy(Temp, PilotName_Config);
        dlgTextEntryShowModal(Temp, 100);

        //
        // ACCESS TO SYSOP MODE 
        //
        if (!SysOpMode) {
           if (!_tcscmp(Temp,_T("OPSYS"))) {
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
  if (Sender) {

    if(Sender->getCount() == 0) {
      Sender->addEnumList({
          MsgToken<2334>(),	// _@M2334_ "In flight only (default)"
          MsgToken<2335>() 	// _@M2335_ "permanent (test purpose)"
        });
    }

    switch (Mode) {
    case DataField::daGet:
      Sender->Set(tracking::always_config);
      break;
    case DataField::daPut:
    case DataField::daChange:
      tracking::always_config = Sender->GetAsBoolean();
      break;
    case DataField::daInc:
    case DataField::daDec:
    case DataField::daSpecial:
    default:
      break;
    }
  }
}

static void OnLiveTrackersrvClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackersrv) {
        _tcscpy(Temp, tracking::server_config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(tracking::server_config, Temp);
    }
    UpdateButtons(pWnd->GetParentWndForm());
}

static void OnLiveTrackerportClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackerport) {
        _stprintf(Temp, _T("%d"), tracking::port_config);
        dlgNumEntryShowModal(Temp, 100);
        tracking::port_config = _tcstol(Temp, nullptr, 10);
    }
    UpdateButtons(pWnd->GetParentWndForm());
}

static void OnLiveTrackerusrClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackerusr) {
        _tcscpy(Temp, tracking::usr_config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(tracking::usr_config, Temp);
    }
    UpdateButtons(pWnd->GetParentWndForm());
}

static void OnLiveTrackerpwdClicked(WndButton* pWnd) {
    TCHAR Temp[100];
    if (buttonLiveTrackerpwd) {
        _tcscpy(Temp, tracking::pwd_config);
        dlgTextEntryShowModal(Temp, 100);
        _tcscpy(tracking::pwd_config, Temp);
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
    dlgOverlaysShowModal();
}

static void OnTaskRulesClicked(WndButton* pWnd) {
    dlgTaskRules();
}

static void OnAirspaceWarningParamsClicked(WndButton* pWnd) {
    dlgAirspaceWarningParamsShowModal();
}

static void OnAirspaceFilesClicked(WndButton* pWnd) {
  dlgAirspaceFilesShowModal();
}

static void OnWaypointFilesClicked(WndButton* pWnd) {
  dlgWaypointFilesShowModal();
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
  TCHAR buf[12];
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
    wp = pWnd->GetParentWndForm()->FindByName<WndProperty>(name);
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
		 MsgToken<510>(),
	// LKTOKEN  _@M354_ = "InfoBox paste" 
		 MsgToken<354>(),
		 mbYesNo) == IdYes) {

    for (int item=0; item<8; item++) {
      InfoBoxPropName(name, item, mode);
      WndProperty *wp;
      wp = pWnd->GetParentWndForm()->FindByName<WndProperty>(name);
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
  WndProperty* wp = wf->FindByName<WndProperty>(TEXT("prpLocalTime"));
  if (wp) {
    TCHAR temp[20];
    Units::TimeToText(temp, LocalTime());
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

static void OnWaypointNewClicked(WndButton* pWnd){

  // Cannot save waypoint if no file
  if ( WayPointList.size()<=NUMRESWP) {
	MessageBoxX(
	// LKTOKEN  _@M478_ = "No waypoint file selected, cannot save." 
	MsgToken<478>(),
	// LKTOKEN  _@M457_ = "New Waypoint" 
               MsgToken<457>(),
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

  if (edit_waypoint.Comment == (TCHAR *)NULL) {
    OutOfMemory(_T(__FILE__), __LINE__);
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


static void OnWaypointEditClicked(WndButton* pWnd) {
  if (CheckClubVersion()) {
	ClubForbiddenMsg();
	return;
  }
  int res = dlgSelectWaypoint();
  if (res != -1){
	#if 0 // 101214 READ ONLY FILES
	if ( WayPointList[res].Format == LKW_COMPE) {      // 100212
		MessageBoxX(
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
		MsgToken<716>(),
	// LKTOKEN  _@M194_ = "CompeGPS Waypoint" 
                MsgToken<194>(),
                mbOk);

		return;
	}
	#endif

	if ( WayPointList[res].Format == LKW_VIRTUAL) {      // 100212
		MessageBoxX(
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
		MsgToken<716>(),
	// LKTOKEN  _@M775_ = "VIRTUAL Waypoint" 
                MsgToken<775>(),
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
                   MsgToken<810>(),
	// LKTOKEN  _@M811_ = "Waypoints outside terrain" 
                   MsgToken<811>(),
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

static void OnWaypointDeleteClicked(WndButton* pWnd) {
  if (CheckClubVersion()) {
	ClubForbiddenMsg();
	return;
  }
  int res = dlgSelectWaypoint();
  if (res > RESWP_END) { // 100212
	#if 0 // 101214 READ ONLY FILES
	if ( WayPointList[res].Format == LKW_COMPE ) { // 100212
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
		MessageBoxX(MsgToken<716>(),
	// LKTOKEN  _@M194_ = "CompeGPS Waypoint" 
			MsgToken<194>(),
			mbOk);
		return;
	} else 
	#endif
		if ( WayPointList[res].Format == LKW_VIRTUAL ) { // 100212
	// LKTOKEN  _@M716_ = "This waypoint is read-only" 
			MessageBoxX(MsgToken<716>(),
	// LKTOKEN  _@M775_ = "VIRTUAL Waypoint" 
				MsgToken<775>(),
				mbOk);
			return;
		} else 
	// LKTOKEN  _@M229_ = "Delete Waypoint?" 
	if(MessageBoxX(WayPointList[res].Name, MsgToken<229>(), 
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

static void OnBthDevice(WndButton* pWnd) {
#ifndef NO_BLUETOOTH
    DlgBluetooth::Show();
    
    const auto& Port = PortConfig[SelectedDevice];
    UpdateComPortList(wf->FindByName<WndProperty>(TEXT("prpComPort1")), Port.GetPort());
#endif
}



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

static void LedSetOnOff(WndForm* pOwner, const TCHAR* name, bool Enable) {
    WndButton* pWnd = pOwner->FindByName<WndButton>(name);
    if (pWnd) {
       pWnd->LedSetOnOff(Enable);
    }
}

void UpdateComPortSetting(WndForm* pOwner,  size_t idx, const TCHAR* szPortName) {

    LKASSERT(szPortName);
    // check if all array have same size ( compil time check );
    /*
    static_assert(std::size(DeviceList) == std::size(PortPropName[0]), "PortPropName array size need to be same of DeviceList array size");
    static_assert(std::size(DeviceList) == std::size(prpExtSound), "prpExtSound array size need to be same of DeviceList array size");
    static_assert(std::size(DeviceList) == std::size(prpIpAddr[0]), "prpIpAddr array size need to be same of DeviceList array size");
    static_assert(std::size(DeviceList) == std::size(prpIpPort[0]), "prpIpPort array size need to be same of DeviceList array size");
*/
#ifdef DISABLEEXTAUDIO    
    bool bManageExtAudio = false;
#else
    bool bManageExtAudio = true;
#endif

    WndProperty* wp;



    wp = pOwner->FindByName<WndProperty>(TEXT("prpComDevice1"));


    TCHAR newname[25];

    if(pOwner) {
        _stprintf(newname,  TEXT("%s"), MsgToken<232>());
        newname[_tcslen(newname)-1] = (TCHAR)('A'+SelectedDevice);
      WndFrame  *wDev = (pOwner->FindByName<WndFrame>(TEXT("frmCommName")));
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
          if(_tcscmp(wp->GetDataField()->GetAsString(), DEV_DISABLED_NAME) == 0)
          {
            DeviceList[idx].Disabled = true;
            bHide = true;
          }
          else
            DeviceList[idx].Disabled = false;

          if (_tcscmp(wp->GetDataField()->GetAsString(), DEV_INTERNAL_NAME) == 0)
          {
            bHide = true;
          }
        }
      }
    }


    ShowWindowControl(wf, TEXT("cmdConfigDev"), DeviceList[SelectedDevice].Config);

    LedSetOnOff(pOwner, _T("cmdA"), !DeviceList[0].Disabled);
    LedSetOnOff(pOwner, _T("cmdB"), !DeviceList[1].Disabled);
    LedSetOnOff(pOwner, _T("cmdC"), !DeviceList[2].Disabled);
    LedSetOnOff(pOwner, _T("cmdD"), !DeviceList[3].Disabled);
    LedSetOnOff(pOwner, _T("cmdE"), !DeviceList[4].Disabled);
    LedSetOnOff(pOwner, _T("cmdF"), !DeviceList[5].Disabled);

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

    bool bBt = ((_tcslen(szPortName) > 3)
            && ((_tcsncmp(szPortName, _T("BT_SPP:"), 7) == 0)
                || (_tcsncmp(szPortName, _T("BT_HM10:"), 8) == 0)
                || (_tcsncmp(szPortName, _T("Bluetooth Server"), 16) == 0)));

    bool bTCPClient = (_tcscmp(szPortName, _T("TCPClient")) == 0);
    bool bTCPServer = (_tcscmp(szPortName, _T("TCPServer")) == 0);
    bool bFileReplay = (_tcscmp(szPortName, NMEA_REPLAY)    == 0);
    bool bUDPServer = (_tcscmp(szPortName, _T("UDPServer")) == 0);
    bool bCOM = !(bBt || bTCPClient || bTCPServer || bUDPServer || bFileReplay || ( DeviceList[SelectedDevice].iSharedPort>=0 ));
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

    ShowWindowControl(wf, TEXT("cmdReplay"), bFileReplay);

    // Manage external sounds only if necessary
    if (bManageExtAudio) {
        ShowWindowControl(wf,  TEXT("prpExtSound1"), !bHide);
    }
    }
    TCHAR StateText[255];
    _tcscpy(StateText,_T(""));
    if( DeviceList[SelectedDevice].nmeaParser.gpsValid) _tcscat(StateText,TEXT("GPSFix "));
    if( DeviceList[SelectedDevice].nmeaParser.isFlarm) _tcscat(StateText,TEXT("Flarm "));
    if( devIsBaroSource(DeviceList[SelectedDevice])) _tcscat(StateText,TEXT("Baro "));
    if( DeviceList[SelectedDevice].iSharedPort>=0 ) _tcscat(StateText,TEXT("Shared "));

    if(DeviceList[idx].Disabled)
      ShowWindowControl(wf, TEXT("prpStatus"),false);
    else
    {
      ShowWindowControl(wf, TEXT("prpStatus"),true);
      wp = pOwner->FindByName<WndProperty>(TEXT("prpStatus"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        dfe->Set(StateText);
        wp->RefreshDisplay();
      }
    }


}


  static void OnConfigDevClicked(WndButton* pWnd){
     if(DeviceList[SelectedDevice].Config)
       DeviceList[SelectedDevice].Config(&DeviceList[SelectedDevice]);
}


extern void dlgNMEAReplayShowModal(void);
static void OnConfigDevReplayClicked(WndButton* pWnd){
	dlgNMEAReplayShowModal();
}

static void OnFfvlKey(DataField *Sender, DataField::DataAccessKind_t Mode) {
  switch (Mode) {
    case DataField::daGet:
      Sender->SetAsString(utf8_to_tstring(tracking::ffvl_user_key).c_str());
      break;
    case DataField::daPut:
    case DataField::daChange:
      tracking::ffvl_user_key = to_utf8(Sender->GetAsString());
      break;
    case DataField::daInc:
    case DataField::daDec:
    case DataField::daSpecial:
      break;
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

void OnInfoBoxHelp(WndProperty * Sender);

static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnAirspaceColoursClicked),
  ClickNotifyCallbackEntry(OnAirspaceFilesClicked),
  ClickNotifyCallbackEntry(OnWaypointFilesClicked),
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

  ClickNotifyCallbackEntry(OnBthDevice),

  ClickNotifyCallbackEntry(OnNextDevice),
  ClickNotifyCallbackEntry(OnA),
  ClickNotifyCallbackEntry(OnB),
  ClickNotifyCallbackEntry(OnC),
  ClickNotifyCallbackEntry(OnD),
  ClickNotifyCallbackEntry(OnE),
  ClickNotifyCallbackEntry(OnF),
  ClickNotifyCallbackEntry(OnConfigDevClicked),
  ClickNotifyCallbackEntry(OnConfigDevReplayClicked),
  ClickNotifyCallbackEntry(OnNextDevice),
  ClickNotifyCallbackEntry(OnTerminalClicked),

  DataAccessCallbackEntry(OnFfvlKey),

  EndCallBackEntry()
};



static void SetInfoBoxSelector(int item, int mode)
{
  TCHAR name[80];
  InfoBoxPropName(name, item, mode);

  WndProperty *wp;
  wp = wf->FindByName<WndProperty>(name);
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FillDataOptionDescription(dfe);
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
  wp = wf->FindByName<WndProperty>(name);
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

            auto It = FindCOMMPort(szPort);
            if(It != COMMPort.end()) {
                dfe->Set((unsigned)std::distance(COMMPort.begin(), It));
            } else {
                auto label = [&]() -> tstring {
#ifdef ANDROID
                    std::string address;
                    std::string prefix = szPort;
                    if (prefix.find("BT_HM10:") == 0) {
                        address = prefix.substr(8);
                        prefix = prefix.substr(0, 8);
                    }
                    else if (prefix.find("BT_SPP:") == 0) {
                        address = prefix.substr(7);
                        prefix = prefix.substr(0, 7);
                    }
                    if (!address.empty()) {
                        const char* label = BluetoothHelper::GetNameFromAddress(Java::GetEnv(), address.c_str());
                        if (label) {
                            return prefix + label;
                        }
                    }
#endif
                    return _T("");
                };
                dfe->addEnumText(szPort, label().c_str());
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

  const auto& Port = PortConfig[DeviceIdx];

  UpdateComPortSetting(wf,DeviceIdx, Port.GetPort());
  UpdateComPortList(wf->FindByName<WndProperty>(TEXT("prpComPort1")), Port.GetPort());

wp = wf->FindByName<WndProperty>(TEXT("prpComSpeed1"));
if (wp) {
  DataField* dfe = wp->GetDataField();
  dfe->Set(Port.dwSpeedIndex);
  wp->SetReadOnly(false);
  wp->RefreshDisplay();
}
wp = wf->FindByName<WndProperty>(TEXT("prpComBit1"));
if (wp) {
  DataField* dfe = wp->GetDataField();
  dfe->Set(Port.dwBitIndex);
  wp->RefreshDisplay();
}
wp = wf->FindByName<WndProperty>(TEXT("prpExtSound1"));
if (wp) {
  wp->GetDataField()->Set(Port.UseExtSound);
  wp->RefreshDisplay();
}

wp = wf->FindByName<WndProperty>(TEXT("prpComIpAddr1"));
if (wp) {
  wp->GetDataField()->Set(Port.szIpAddress);
  wp->RefreshDisplay();
}

wp = wf->FindByName<WndProperty>(TEXT("prpComIpPort1"));
if (wp) {
  wp->GetDataField()->Set((int)Port.dwIpPort);
  wp->RefreshDisplay();
}

  wp = wf->FindByName<WndProperty>(TEXT("prpComDevice1"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->Set(Port.szDeviceName);
    wp->RefreshDisplay();
    UpdateButtons(wf);
  }

  UpdateComPortSetting(wf, DeviceIdx, Port.GetPort());
  UpdateComPortList(wf->FindByName<WndProperty>(TEXT("prpComPort1")), Port.GetPort());
}

static void setVariables( WndForm *pOwner) {
  WndProperty *wp;

  LKASSERT(pOwner);

  buttonPilotName = (pOwner->FindByName<WndButton>(TEXT("cmdPilotName")));
  if (buttonPilotName) {
    buttonPilotName->SetOnClickNotify(OnPilotNameClicked);
  }
  buttonLiveTrackersrv = (pOwner->FindByName<WndButton>(TEXT("cmdLiveTrackersrv")));
  if (buttonLiveTrackersrv) {
    buttonLiveTrackersrv->SetOnClickNotify(OnLiveTrackersrvClicked);
  }
  buttonLiveTrackerport = (pOwner->FindByName<WndButton>(TEXT("cmdLiveTrackerport")));
  if (buttonLiveTrackerport) {
    buttonLiveTrackerport->SetOnClickNotify(OnLiveTrackerportClicked);
  }
  buttonLiveTrackerusr = (pOwner->FindByName<WndButton>(TEXT("cmdLiveTrackerusr")));
  if (buttonLiveTrackerusr) {
    buttonLiveTrackerusr->SetOnClickNotify(OnLiveTrackerusrClicked);
  }
  buttonLiveTrackerpwd = (pOwner->FindByName<WndButton>(TEXT("cmdLiveTrackerpwd")));
  if (buttonLiveTrackerpwd) {
    buttonLiveTrackerpwd->SetOnClickNotify(OnLiveTrackerpwdClicked);
  }
  buttonAircraftType = (pOwner->FindByName<WndButton>(TEXT("cmdAircraftType")));
  if (buttonAircraftType) {
    buttonAircraftType->SetOnClickNotify(OnAircraftTypeClicked);
  }
  buttonAircraftRego = (pOwner->FindByName<WndButton>(TEXT("cmdAircraftRego")));
  if (buttonAircraftRego) {
    buttonAircraftRego->SetOnClickNotify(OnAircraftRegoClicked);
  }
  buttonCompetitionClass = (pOwner->FindByName<WndButton>(TEXT("cmdCompetitionClass")));
  if (buttonCompetitionClass) {
    buttonCompetitionClass->SetOnClickNotify(OnCompetitionClassClicked);
  }
  buttonCompetitionID = (pOwner->FindByName<WndButton>(TEXT("cmdCompetitionID")));
  if (buttonCompetitionID) {
    buttonCompetitionID->SetOnClickNotify(OnCompetitionIDClicked);
  }
  buttonCopy = (pOwner->FindByName<WndButton>(TEXT("cmdCopy")));
  if (buttonCopy) {
    buttonCopy->SetOnClickNotify(OnCopy);
  }
  buttonPaste = (pOwner->FindByName<WndButton>(TEXT("cmdPaste")));
  if (buttonPaste) {
    buttonPaste->SetOnClickNotify(OnPaste);
  }

  UpdateButtons(pOwner);

  const auto& Port = PortConfig[SelectedDevice];

  UpdateComPortList(pOwner->FindByName<WndProperty>(TEXT("prpComPort1")), Port.GetPort());


  wp = pOwner->FindByName<WndProperty>(TEXT("prpComSpeed1"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    std::for_each(std::begin(baudrate_string), 
                  std::end(baudrate_string), 
                  std::bind(&DataField::addEnumText, dfe, _1, nullptr));
    
    dfe->Set(Port.dwSpeedIndex);
    wp->SetReadOnly(false);
    wp->RefreshDisplay();
  }
  wp = pOwner->FindByName<WndProperty>(TEXT("prpComBit1"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(TEXT("8bit"));
    dfe->addEnumText(TEXT("7bit"));
    dfe->Set(Port.dwBitIndex);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpExtSound1"));
  if (wp) {
    wp->GetDataField()->Set(Port.UseExtSound);
    wp->RefreshDisplay();
  }
  
  wp = wf->FindByName<WndProperty>(TEXT("prpComIpAddr1"));
  if (wp) {
    wp->GetDataField()->Set(Port.szIpAddress);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpComIpPort1"));
  if (wp) {
    wp->GetDataField()->Set((int)Port.dwIpPort);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpComDevice1"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    
    for (auto& dev : devRegisterIterator()) {
      dfe->addEnumText(dev.Name);
    }

    dfe->Sort(3);
    dfe->Set(Port.szDeviceName);
    wp->RefreshDisplay();
  }

  UpdateButtons(wf);



  wp = wf->FindByName<WndProperty>(TEXT("prpAirspaceDisplay"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M79_ = "All on" 
    dfe->addEnumText(MsgToken<79>());
	// LKTOKEN  _@M184_ = "Clip" 
    dfe->addEnumText(MsgToken<184>());
    dfe->addEnumText(TEXT("Auto"));
	// LKTOKEN  _@M77_ = "All below" 
    dfe->addEnumText(MsgToken<77>());
    dfe->Set(AltitudeMode_Config);
    wp->RefreshDisplay();
      wp = wf->FindByName<WndProperty>(TEXT("prpClipAltitude"));
      if (wp) wp->SetVisible(AltitudeMode_Config==CLIP);
      wp = wf->FindByName<WndProperty>(TEXT("prpAltWarningMargin"));
      if (wp) wp->SetVisible(AltitudeMode_Config==AUTO || AltitudeMode_Config==ALLBELOW);
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpAirspaceFillType"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if(dfe) {
        dfe->addEnumText(MsgToken<941>());
#ifdef HAVE_HATCHED_BRUSH
        dfe->addEnumText(MsgToken<942>());
        dfe->addEnumText(MsgToken<945>());
#endif
        if (LKSurface::AlphaBlendSupported()) {
            dfe->addEnumText(MsgToken<943>());
            dfe->addEnumText(MsgToken<946>());
        }
        dfe->Set((int)MapWindow::GetAirSpaceFillType());
    }
    wp->RefreshDisplay();
  }
  
  wp = wf->FindByName<WndProperty>(TEXT("prpAirspaceOpacity"));
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

  wp = wf->FindByName<WndProperty>(TEXT("prpBarOpacity"));
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
  wp = wf->FindByName<WndProperty>(TEXT("prpFontMapWaypoint"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontMapWaypoint);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpFontSymbols"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken<2327>()); // LKTOKEN  _@M2327_ "Bitmap"
    dfe->addEnumText(MsgToken<2386>()); // LKTOKEN  _@M2386_ "UTF8"

    dfe->Set(Appearance.UTF8Pictorials);

    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpFontMapTopology"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontMapTopology);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpFontInfopage1L"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontInfopage1L);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpFontBottomBar"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontBottomBar);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSnailScale"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(SnailScale);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpFontVisualGlide"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontVisualGlide);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOverlayTarget"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontOverlayMedium);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOverlayValues"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    FontSetEnums(dfe);
    dfe->Set(FontOverlayBig);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUseTwoLines"));
  if (wp) {
    wp->GetDataField()->Set(UseTwoLines);
    if (ScreenLandscape) wp->SetReadOnly(true);
    wp->RefreshDisplay();
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpFontRenderer"));
  if (wp) {
    DataField* dfe = wp->GetDataField();

    dfe->addEnumText(MsgToken<955>()); // Clear Type Compatible
    dfe->addEnumText(MsgToken<956>()); // Anti Aliasing
    dfe->addEnumText(MsgToken<480>()); // Normal
    dfe->addEnumText(MsgToken<479>()); // None
    dfe->Set(FontRenderer);
    #ifdef __linux__
    wp->SetVisible(false);
    #endif
    wp->RefreshDisplay();
  }
  
  wp = wf->FindByName<WndProperty>(TEXT("prpAspPermDisable"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken<968>()); // _@M968_ "for this time only"
    dfe->addEnumText(MsgToken<969>()); // _@M969_ "permanent"
    dfe->Set(AspPermanentChanged);
    wp->RefreshDisplay();
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpSafetyAltitudeMode"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M382_ = "Landables only" 
    dfe->addEnumText(MsgToken<382>());
	// LKTOKEN  _@M380_ = "Landables and Turnpoints" 
    dfe->addEnumText(MsgToken<380>());
    dfe->Set(SafetyAltitudeMode);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUTCOffset"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(UTCOffset/900.0)/4.0);
    wp->RefreshDisplay();
  }
  SetLocalTime();

  wp = wf->FindByName<WndProperty>(TEXT("prpClipAltitude"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(ClipAltitude / 10.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }
    
  wp = wf->FindByName<WndProperty>(TEXT("prpAltWarningMargin"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(AltWarningMargin / 10.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoZoom"));
  if (wp) {
    wp->GetDataField()->Set(AutoZoom_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpDrawFAI"));
  if (wp) {
    wp->GetDataField()->Set(Flags_DrawFAI_config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpDrawXC"));
  if (wp) {
    wp->GetDataField()->Set(Flags_DrawXC_config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLockSettingsInFlight"));
  if (wp) {
    wp->GetDataField()->Set(LockSettingsInFlight);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLoggerShortName"));
  if (wp) {
    wp->GetDataField()->Set(LoggerShortName);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpDebounceTimeout"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(debounceTimeout);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpEnableFLARMMap"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken<239>()); // Disabled
    dfe->addEnumText(MsgToken<259>()); // Enabled
    // dfe->addEnumText(MsgToken<959>()); // OFF
    // dfe->addEnumText(MsgToken<496>()); // ON fixed
    // dfe->addEnumText(MsgToken<497>()); // ON scaled
    dfe->Set((int)EnableFLARMMap);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWaypointLabels"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M454_ = "Names" 
    dfe->addEnumText(MsgToken<454>());

	// LKTOKEN  _@M488_ = "Numbers" 
    dfe->addEnumText(MsgToken<488>());

	// LKTOKEN  _@M453_ = "Names in task" 
    dfe->addEnumText(MsgToken<453>());

	// LKTOKEN  _@M301_ = "First 3" 
    dfe->addEnumText(MsgToken<301>());

	// LKTOKEN  _@M302_ = "First 5" 
    dfe->addEnumText(MsgToken<302>());

	// LKTOKEN  _@M838_ = "First 8" 
    dfe->addEnumText(MsgToken<838>());
	// LKTOKEN  _@M839_ = "First 10" 
    dfe->addEnumText(MsgToken<839>());
	// LKTOKEN  _@M840_ = "First 12" 
    dfe->addEnumText(MsgToken<840>());

	// LKTOKEN  _@M479_ = "None" 
    dfe->addEnumText(MsgToken<479>());

    dfe->addEnumText(MsgToken<2336>());
    dfe->Set(DisplayTextType);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpCirclingZoom"));
  if (wp) {
    wp->GetDataField()->Set(MapWindow::zoom.CircleZoom());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOrientation"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M737_ = "Track up" 
    dfe->addEnumText(MsgToken<737>());
	// LKTOKEN  _@M483_ = "North up" 
    dfe->addEnumText(MsgToken<483>());
	// LKTOKEN  _@M482_ = "North circling" 
    dfe->addEnumText(MsgToken<482>());
	// LKTOKEN  _@M682_ = "Target circling" 
    dfe->addEnumText(MsgToken<682>());
	// LKTOKEN  _@M484_ = "North/track" 
    dfe->addEnumText(MsgToken<484>());
	// LKTOKEN  _@M481_ = "North Smart" 
    dfe->addEnumText(MsgToken<481>()); // 100417
    // LKTOKEN  _@M2349_"Target up"
    dfe->addEnumText(MsgToken<2349>());

    dfe->Set(DisplayOrientation_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMenuTimeout"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(MenuTimeout_Config/2);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSafetyAltitudeArrival"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(SAFETYALTITUDEARRIVAL / 10.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSafetyAltitudeTerrain"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(SAFETYALTITUDETERRAIN / 10.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpFinalGlideTerrain"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M959_ = "OFF" 
    dfe->addEnumText(MsgToken<959>());
	// LKTOKEN  _@M393_ = "Line" 
    dfe->addEnumText(MsgToken<393>());
	// LKTOKEN  _@M609_ = "Shade" 
    dfe->addEnumText(MsgToken<609>());
        // "Line+NextWP"
    dfe->addEnumText(MsgToken<1805>());
        // "Shade+NextWP"
    dfe->addEnumText(MsgToken<1806>());
    dfe->Set(FinalGlideTerrain);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpEnableNavBaroAltitude"));
  if (wp) {
    wp->GetDataField()->Set(EnableNavBaroAltitude_Config);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpOrbiter"));
  if (wp) {
    wp->GetDataField()->Set(Orbiter_Config);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpAutoMcStatus"));
  if (wp) {
    wp->GetDataField()->Set(AutoMacCready_Config);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpShading"));
  if (wp) {
    wp->GetDataField()->Set(Shading_Config);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIsoLine"));
  if (wp) {
    wp->GetDataField()->Set(IsoLine_Config);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpSonarWarning"));
  if (wp) {
    wp->GetDataField()->Set(SonarWarning_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoWind"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M418_ = "Manual" 
    dfe->addEnumText(MsgToken<418>());
	// LKTOKEN  _@M175_ = "Circling" 
    dfe->addEnumText(MsgToken<175>());
    dfe->addEnumText(TEXT("ZigZag"));
	// LKTOKEN  _@M149_ = "Both" 
    dfe->addEnumText(MsgToken<149>());
    dfe->addEnumText(MsgToken<1793>()); // External

    wp->GetDataField()->Set(AutoWindMode_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoMcMode"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken<290>());  // Final Glide
    dfe->addEnumText(MsgToken<1684>()); // Average thermal
    dfe->addEnumText(MsgToken<1685>()); // Final + Average
    dfe->addEnumText(MsgToken<262>());  // Equivalent
    wp->GetDataField()->Set(AutoMcMode_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWaypointsOutOfRange"));
  if (wp) {

  // ToDo: Remove Ask ?
    DataField* dfe = wp->GetDataField();
//#define  ASK_WAYPOINTS
#ifdef ASK_WAYPOINTS
	// LKTOKEN  _@M100_ = "Ask" 
    dfe->addEnumText(MsgToken<100>());
#endif
	// LKTOKEN  _@M350_ = "Include" _@M2343_ "Include Data" 
    dfe->addEnumText(MsgToken<2343>());
	// LKTOKEN  _@M269_ = "Exclude" _@M2344_ "Exclude Data" 
    dfe->addEnumText(MsgToken<2344>());
#ifdef ASK_WAYPOINTS
    wp->GetDataField()->Set(WaypointsOutOfRange);
#else
    wp->GetDataField()->Set(WaypointsOutOfRange-1);
#endif
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoForceFinalGlide"));
  if (wp) {
    wp->GetDataField()->Set(AutoForceFinalGlide);
    wp->RefreshDisplay();
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpHandicap"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(Handicap);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpClimbZoom"));
  if (wp) {
    TCHAR buf1[32];
    DataField* dfe = wp->GetDataField();
    for (int i=0; i<MapWindow::GetScaleListCount(); ++i) {
      MapWindow::zoom.GetInitMapScaleText(i, buf1);
      dfe->addEnumText(buf1);
    }
    dfe->Set(ClimbZoom);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpCruiseZoom"));
  if (wp) {
    TCHAR buf1[32];
    DataField* dfe = wp->GetDataField();
    for (int i=0; i<MapWindow::GetScaleListCount(); ++i) {
      MapWindow::zoom.GetInitMapScaleText(i, buf1);
	    dfe->addEnumText(buf1);
    }
    dfe->Set(CruiseZoom);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpAutoZoomThreshold"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(Units::ToDistance(AutoZoomThreshold));
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMaxAutoZoom"));
  if (wp) {
    TCHAR buf1[32];
    DataField* dfe = wp->GetDataField();
    for (int i=0; i<MapWindow::GetScaleListCount(); ++i) {
      MapWindow::zoom.GetInitMapScaleText(i, buf1);
      dfe->addEnumText(buf1);
    }
    dfe->Set(MaxAutoZoom);
    wp->RefreshDisplay();
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpAutoOrientScale"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AutoOrientScale);
    wp->RefreshDisplay();
  }
  
  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsSpeed"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M667_ = "Statute" 
    dfe->addEnumText(MsgToken<667>());
	// LKTOKEN  _@M455_ = "Nautical" 
    dfe->addEnumText(MsgToken<455>());
	// LKTOKEN  _@M436_ = "Metric" 
    dfe->addEnumText(MsgToken<436>());
    dfe->Set(Units::SpeedUnit_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsLatLon"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(TEXT("DDMMSS"));
    dfe->addEnumText(TEXT("DDMMSS.ss"));
    dfe->addEnumText(TEXT("DDMM.mmm"));
    dfe->addEnumText(TEXT("DD.dddd"));
    dfe->addEnumText(TEXT("UTM"));
    dfe->Set(Units::LatLonUnits_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsTaskSpeed"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M667_ = "Statute" 
    dfe->addEnumText(MsgToken<667>());
	// LKTOKEN  _@M455_ = "Nautical" 
    dfe->addEnumText(MsgToken<455>());
	// LKTOKEN  _@M436_ = "Metric" 
    dfe->addEnumText(MsgToken<436>());
    dfe->Set(Units::TaskSpeedUnit_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsDistance"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M667_ = "Statute" 
    dfe->addEnumText(MsgToken<667>());
	// LKTOKEN  _@M455_ = "Nautical" 
    dfe->addEnumText(MsgToken<455>());
	// LKTOKEN  _@M436_ = "Metric" 
    dfe->addEnumText(MsgToken<436>());
    dfe->Set(Units::DistanceUnit_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsAltitude"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(TEXT("feet"));
    dfe->addEnumText(TEXT("meters"));
    dfe->Set(Units::AltitudeUnit_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsLift"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(TEXT("knots"));
    dfe->addEnumText(TEXT("m/s"));
    dfe->addEnumText(TEXT("ft/min"));
    dfe->Set(Units::VerticalSpeedUnit_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTrailDrift"));
  if (wp) {
    wp->GetDataField()->Set(EnableTrailDrift_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpThermalLocator"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M959_ = "OFF" 
    dfe->addEnumText(MsgToken<959>());
	// LKTOKEN  _@M427_ = "Mark center" 
    dfe->addEnumText(MsgToken<427>());
	// LKTOKEN  _@M518_ = "Pan to center" 
    dfe->addEnumText(MsgToken<518>());
    dfe->Set(EnableThermalLocator);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSetSystemTimeFromGPS"));
  if (wp) {
#if defined(PPC2003) || defined(PNA)
      wp->SetVisible(true);
      wp->GetDataField()->Set(SetSystemTimeFromGPS);
      wp->RefreshDisplay();
#else
      wp->SetVisible(false);
#endif
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSaveRuntime"));
  if (wp) {
    wp->GetDataField()->Set(SaveRuntime);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpDisableAutoLogger"));
  if (wp) {
    wp->GetDataField()->Set(!DisableAutoLogger);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLiveTrackerInterval"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(tracking::interval);
    wp->GetDataField()->SetUnits(TEXT("sec"));
    wp->RefreshDisplay();
  }
  
  wp = wf->FindByName<WndProperty>(TEXT("prpLiveTrackerRadar_config"));
  if (wp) {
    wp->GetDataField()->Set(tracking::radar_config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSafetyMacCready"));
  if (wp) {
    wp->GetDataField()->Set(Units::ToVerticalSpeed(GlidePolar::SafetyMacCready));
    wp->GetDataField()->SetUnits(Units::GetVerticalSpeedName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTrail"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M499_ = "Off" 
    dfe->addEnumText(MsgToken<499>());
	// LKTOKEN  _@M410_ = "Long" 
    dfe->addEnumText(MsgToken<410>());
	// LKTOKEN  _@M612_ = "Short" 
    dfe->addEnumText(MsgToken<612>());
	// LKTOKEN  _@M312_ = "Full" 
    dfe->addEnumText(MsgToken<312>());
    dfe->Set(TrailActive_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLKMaxLabels"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(LKMaxLabels);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBugs"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(BUGS_Config*100); // we show the value correctly
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMaxManoeuveringSpeed"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToHorizontalSpeed(SAFTEYSPEED)));
    wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWindCalcSpeed")); // 100112
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToHorizontalSpeed(WindCalcSpeed)));
    wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWindCalcTime")); // 100113
  if (wp) {
    wp->GetDataField()->Set(WindCalcTime);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBallastSecsToEmpty"));
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

  wp = wf->FindByName<WndProperty>(TEXT("prpPilotFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_CONF),_T(LKS_PILOT));
      dfe->Set(0);
    }
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpAircraftFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_CONF), _T(LKS_AIRCRAFT));
      dfe->Set(0);
    }
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpDeviceFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_CONF), _T(LKS_DEVICE));
      dfe->Set(0);
    }
    wp->RefreshDisplay();
  }

  if (_tcscmp(szPolarFile,_T(""))==0) 
    _tcscpy(temptext,_T(LKD_DEFAULT_POLAR));
  else
    _tcscpy(temptext,szPolarFile);

  wp = wf->FindByName<WndProperty>(TEXT("prpPolarFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_POLARS), _T(LKS_POLARS)); //091101
#ifdef LKD_SYS_POLAR
      /**
       * Add entry from system directory not exist in data directory.
       */
      dfe->ScanSystemDirectoryTop(_T(LKD_SYS_POLAR), _T(LKS_POLARS));
#endif
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  WndButton *cmdLKMapOpen = (wf->FindByName<WndButton>(TEXT("cmdLKMapOpen")));
  if (cmdLKMapOpen) {
#ifdef ANDROID
    cmdLKMapOpen->SetVisible(true);
#else
    cmdLKMapOpen->SetVisible(false);
#endif
  }

  _tcscpy(temptext,szMapFile);
  wp = wf->FindByName<WndProperty>(TEXT("prpMapFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      const TCHAR* suffix_filters[] = {
        _T(LKS_MAPS),
        _T(XCS_MAPS)
      };
      dfe->ScanDirectoryTop(_T(LKD_MAPS), suffix_filters);
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szTerrainFile);
  wp = wf->FindByName<WndProperty>(TEXT("prpTerrainFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_MAPS), _T(LKS_TERRAINDEM));
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  _tcscpy(temptext,szAirfieldFile);
  wp = wf->FindByName<WndProperty>(TEXT("prpAirfieldFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T(LKS_AIRFIELDS));
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLanguageCode"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if(dfe) {
      const auto list = LoadLanguageList();
      for(auto& lang : list) {
        dfe->addEnumText(lang.first.c_str(), lang.second.c_str());
      }
      dfe->Sort();
      dfe->Set(dfe->Find(szLanguageCode));
    }
    wp->RefreshDisplay();
  }


  _tcscpy(temptext,szInputFile);
  wp = wf->FindByName<WndProperty>(TEXT("prpInputFile"));
  if (wp) {
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
      dfe->ScanDirectoryTop(_T(LKD_CONF), _T(LKS_INPUT));
      dfe->Lookup(temptext);
    }
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAppInfoBoxModel"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    for (auto& item : ModelType::list()) {
      dfe->addEnumText(std::get<1>(item));
    }
    dfe->Set(ModelType::get_index(ModelType::Get()));
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAircraftCategory"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M328_ = "Glider" 
    dfe->addEnumText(MsgToken<328>());
	// LKTOKEN  _@M520_ = "Paraglider/Delta" 
    dfe->addEnumText(MsgToken<520>());
	// LKTOKEN  _@M163_ = "Car" 
    dfe->addEnumText(MsgToken<2148>());
	// LKTOKEN  _@M313_ = "GA Aircraft" 
    dfe->addEnumText(MsgToken<313>());
    dfe->Set(AircraftCategory);
    wp->RefreshDisplay();
  }


#if (0)
  wp = wf->FindByName<WndProperty>(TEXT("prpAltArrivMode"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = wp->GetDataField();
	// LKTOKEN  _@M768_ = "Use MacCready" 
     dfe->addEnumText(MsgToken<768>());
	// LKTOKEN  _@M90_ = "Always use MC=0" 
     dfe->addEnumText(MsgToken<90>());
	// LKTOKEN  _@M91_ = "Always use safety MC" 
     dfe->addEnumText(MsgToken<91>());
	// LKTOKEN  _@M769_ = "Use aver.efficiency" 
     dfe->addEnumText(MsgToken<769>());
     dfe->Set(AltArrivMode);
    wp->RefreshDisplay();
  }
#endif

  wp = wf->FindByName<WndProperty>(TEXT("prpCheckSum"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken<239>());
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(MsgToken<259>());
    dfe->Set(CheckSum);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIphoneGestures"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M364_ = "Inverted" 
    dfe->addEnumText(MsgToken<364>());
	// LKTOKEN  _@M480_ = "Normal" 
    dfe->addEnumText(MsgToken<480>());
    dfe->Set(IphoneGestures);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAndroidScreenOrientation"));
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

  wp = wf->FindByName<WndProperty>(TEXT("prpPollingMode"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M480_ = "Normal" 
    dfe->addEnumText(MsgToken<480>());
	// LKTOKEN  _@M529_ = "Polling" 
    dfe->addEnumText(MsgToken<529>());
    dfe->Set(PollingMode);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLKVarioBar"));
  if (wp) {
    TCHAR newtoken[150];
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken<239>());
	// LKTOKEN  _@M782_ = "Vario rainbow" 
    dfe->addEnumText(MsgToken<782>());
	// LKTOKEN  _@M780_ = "Vario black" 
    dfe->addEnumText(MsgToken<780>());
	// LKTOKEN  _@M783_ = "Vario red+blue" 
    dfe->addEnumText(MsgToken<783>());
	// LKTOKEN  _@M781_ = "Vario green+red" 
    dfe->addEnumText(MsgToken<781>());

    _stprintf(newtoken,_T("%s %s"),MsgToken<953>(),MsgToken<782>() );
    dfe->addEnumText(newtoken);
    _stprintf(newtoken,_T("%s %s"),MsgToken<953>(),MsgToken<780>() );
    dfe->addEnumText(newtoken);
    _stprintf(newtoken,_T("%s %s"),MsgToken<953>(),MsgToken<783>() );
    dfe->addEnumText(newtoken);
    _stprintf(newtoken,_T("%s %s"),MsgToken<953>(),MsgToken<781>() );
    dfe->addEnumText(newtoken);

    dfe->Set(LKVarioBar);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLKVarioVal"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken<1425>()); // vario in thermal and cruise
    dfe->addEnumText(MsgToken<1426>());  // vario in thermal, netto in cruise
    dfe->addEnumText(MsgToken<1427>());  // vario in thermal, sollfahr in cruise
    dfe->Set(LKVarioVal);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpHideUnits"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if(dfe) {
        dfe->Set(HideUnits);
    }
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpDeclutterMode"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken<239>());
	// LKTOKEN  _@M414_ = "Low" 
    dfe->addEnumText(MsgToken<414>());
	// LKTOKEN  _@M433_ = "Medium" 
    dfe->addEnumText(MsgToken<433>());
	// LKTOKEN  _@M339_ = "High" 
    dfe->addEnumText(MsgToken<339>());
	// LKTOKEN  _@M786_ = "Very High" 
    dfe->addEnumText(MsgToken<786>());
    dfe->Set(DeclutterMode);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBestWarning")); // 091122
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if(dfe) {
        dfe->Set(BestWarning);
    }
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpUseTotalEnergy"));
  if (wp) {
    DataField * dfe = wp->GetDataField();
    if(dfe) {
        dfe->Set(UseTotalEnergy_Config);
    }
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpUseUngestures"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    LKASSERT(dfe);
    if(dfe) {
        dfe->Set(UseUngestures);
    }
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpThermalBar")); // 091122
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken<239>());
	// LKTOKEN  +_@M2149_ = "in thermal"
    dfe->addEnumText(MsgToken<2149>());
	// LKTOKEN  +_@M2150_ = "in thermal and cruise"
    dfe->addEnumText(MsgToken<2150>());
    dfe->addEnumText(MsgToken<1833>()); // Always
    dfe->Set(ThermalBar);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOverlayClock"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    // LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken<239>());
    // LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(MsgToken<259>());
    dfe->Set(OverlayClock);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpPGOptimizeRoute"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if(dfe) {
        dfe->Set(TskOptimizeRoute_Config);
    }
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpGliderSymbol"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken<227>());  //  Default
    dfe->addEnumText(MsgToken<2317>()); // _@M2317_ "Triangle"
    dfe->addEnumText(MsgToken<2318>()); // _@M2318_ "Paraglider"
    dfe->addEnumText(MsgToken<2319>()); // _@M2319_ "Hangglider"
    dfe->addEnumText(MsgToken<2320>()); // _@M2320_ "Big Glider"
    dfe->addEnumText(MsgToken<2321>()); // _@M2322_ "Aircraft"
    dfe->addEnumText(MsgToken<2325>()); //  "Small Glider"
    dfe->addEnumText(MsgToken<2326>()); //  "Gem"
    dfe->addEnumText(MsgToken<2331>()); //  "Glider Black"
    dfe->addEnumText(MsgToken<2332>()); //  "Big Glider Black"
    dfe->Set(GliderSymbol);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTrackBar")); // 091122
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if(dfe) {
        dfe->Set(TrackBar);
    }
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOutlinedTp"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M78_ = "All black" 
    dfe->addEnumText(MsgToken<78>());
	// LKTOKEN  _@M778_ = "Values white" 
    dfe->addEnumText(MsgToken<778>());
	// LKTOKEN  _@M81_ = "All white" 
    dfe->addEnumText(MsgToken<81>());
    dfe->Set(OutlinedTp_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTpFilter"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M473_ = "No landables" 
    dfe->addEnumText(MsgToken<473>());
	// LKTOKEN  _@M80_ = "All waypoints" 
    dfe->addEnumText(MsgToken<80>());
	// LKTOKEN  _@M211_ = "DAT Turnpoints" 
    dfe->addEnumText(MsgToken<211>());
    dfe->Set(TpFilter);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOverColor"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M816_ = "White" 
    dfe->addEnumText(MsgToken<816>());
	// LKTOKEN  _@M144_ = "Black" 
    dfe->addEnumText(MsgToken<144>());
	// LKTOKEN  _@M218_ = "DarkBlue" 
    dfe->addEnumText(MsgToken<218>());
	// LKTOKEN  _@M825_ = "Yellow" 
    dfe->addEnumText(MsgToken<825>());
	// LKTOKEN  _@M331_ = "Green" 
    dfe->addEnumText(MsgToken<331>());
	// LKTOKEN  _@M505_ = "Orange" 
    dfe->addEnumText(MsgToken<505>());
	// LKTOKEN  _@M209_ = "Cyan" 
    dfe->addEnumText(MsgToken<209>());
	// LKTOKEN  _@M417_ = "Magenta" 
    dfe->addEnumText(MsgToken<417>());
	// LKTOKEN  _@M332_ = "Grey" 
    dfe->addEnumText(MsgToken<332>());
	// LKTOKEN  _@M215_ = "Dark Grey" 
    dfe->addEnumText(MsgToken<215>());
	// LKTOKEN  _@M216_ = "Dark White" 
    dfe->addEnumText(MsgToken<216>());
	// LKTOKEN  _@M92_ = "Amber" 
    dfe->addEnumText(MsgToken<92>());
	// LKTOKEN  _@M391_ = "Light Green" 
    dfe->addEnumText(MsgToken<391>());
	// LKTOKEN  _@M522_ = "Petrol" 
    dfe->addEnumText(MsgToken<522>());
    dfe->Set(OverColor);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMapBox"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M763_ = "Unboxed, no units" 
    dfe->addEnumText(MsgToken<763>());
	// LKTOKEN  _@M764_ = "Unboxed, with units" 
    dfe->addEnumText(MsgToken<764>());
	// LKTOKEN  _@M152_ = "Boxed, no units" 
    dfe->addEnumText(MsgToken<152>());
	// LKTOKEN  _@M153_ = "Boxed, with units" 
    dfe->addEnumText(MsgToken<153>());
    dfe->Set(MapBox);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOverlaySize"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M842_ = "Big font " 
    dfe->addEnumText(MsgToken<842>());
	// LKTOKEN  _@M843_ = "Small font " 
    dfe->addEnumText(MsgToken<843>());
    dfe = wp->GetDataField();
    dfe->Set(OverlaySize);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpGlideBarMode"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken<239>());
	// LKTOKEN  _@M461_ = "Next turnpoint" 
    dfe->addEnumText(MsgToken<461>());
	// LKTOKEN  _@M299_ = "Finish" 
    dfe->addEnumText(MsgToken<299>());
    dfe->Set(GlideBarMode);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpArrivalValue"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M98_ = "ArrivalAltitude" 
    dfe->addEnumText(MsgToken<98>());
	// LKTOKEN  _@M254_ = "EfficiencyReq" 
    dfe->addEnumText(MsgToken<254>());
    // LKTOKEN _@M2471_ = "Required GR & Arrival altitude"
	dfe->addEnumText(MsgToken<2471>());
	// LKTOKEN  _@M491_ = "Off"
    dfe->addEnumText(MsgToken<491>());
    dfe->Set(ArrivalValue);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpNewMapDeclutter"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(MsgToken<239>());
	// LKTOKEN  _@M414_ = "Low" 
    dfe->addEnumText(MsgToken<414>());
	// LKTOKEN  _@M339_ = "High" 
    dfe->addEnumText(MsgToken<339>());
    dfe->Set(NewMapDeclutter);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAverEffTime"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken<1775>()); // 3 sec
    dfe->addEnumText(MsgToken<1776>()); // 5 sec
    dfe->addEnumText(MsgToken<1777>()); // 10 sec
	// LKTOKEN  _@M17_ = "15 seconds" 
    dfe->addEnumText(MsgToken<17>());
	// LKTOKEN  _@M30_ = "30 seconds" 
    dfe->addEnumText(MsgToken<30>());
    dfe->addEnumText(MsgToken<1778>()); // 45 sec
	// LKTOKEN  _@M35_ = "60 seconds" 
    dfe->addEnumText(MsgToken<35>());
	// LKTOKEN  _@M39_ = "90 seconds" 
    dfe->addEnumText(MsgToken<39>());
	// LKTOKEN  _@M23_ = "2 minutes" 
    dfe->addEnumText(MsgToken<23>());
	// LKTOKEN  _@M29_ = "3 minutes" 
    dfe->addEnumText(MsgToken<29>());
    dfe->Set(AverEffTime);
    wp->RefreshDisplay();
 }

  wp = wf->FindByName<WndProperty>(TEXT("prpBgMapcolor"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M816_ = "White" 
    dfe->addEnumText(MsgToken<816>());
	// LKTOKEN  _@M392_ = "Light grey" 
    dfe->addEnumText(MsgToken<392>());
	// LKTOKEN  _@M374_ = "LCD green" 
    dfe->addEnumText(MsgToken<374>());
	// LKTOKEN  _@M373_ = "LCD darkgreen" 
    dfe->addEnumText(MsgToken<373>());
	// LKTOKEN  _@M332_ = "Grey" 
    dfe->addEnumText(MsgToken<332>());
	// LKTOKEN  _@M147_ = "Blue lake" 
    dfe->addEnumText(MsgToken<147>());
	// LKTOKEN  _@M256_ = "Emerald green" 
    dfe->addEnumText(MsgToken<256>());
	// LKTOKEN  _@M217_ = "Dark grey" 
    dfe->addEnumText(MsgToken<217>());
	// LKTOKEN  _@M567_ = "Rifle grey" 
    dfe->addEnumText(MsgToken<567>());
	// LKTOKEN  _@M144_ = "Black" 
    dfe->addEnumText(MsgToken<144>());
    dfe->Set(BgMapColor_Config);
    wp->RefreshDisplay();
 }


  wp = wf->FindByName<WndProperty>(TEXT("prpAppIndLandable"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken<1797>()); // Vector
	// LKTOKEN  _@M2327_ "Bitmap"
    dfe->addEnumText(MsgToken<2327>());
    dfe->Set(Appearance.IndLandable);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAppInverseInfoBox"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M958_ = "ON" 
    dfe->addEnumText(MsgToken<958>());
	// LKTOKEN  _@M959_ = "OFF" 
    dfe->addEnumText(MsgToken<959>());
    dfe->Set(InverseInfoBox_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpGliderScreenPosition"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(MapWindow::GliderScreenPosition);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoContrast"));
  if (wp) {
     wp->GetDataField()->Set(AutoContrast);
     wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTerrainContrast"));
  if (wp) {

    wp->GetDataField()->SetAsFloat(iround((TerrainContrast*100.0)/255.0));
    if (AutoContrast) wp->SetReadOnly(true); // needed on dlg startup
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTerrainBrightness"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround((TerrainBrightness*100.0)/255.0));
    if (AutoContrast) wp->SetReadOnly(true); // needed on dlg startup
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTerrainRamp"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
	// LKTOKEN  _@M413_ = "Low lands" 
    dfe->addEnumText(MsgToken<413>());
	// LKTOKEN  _@M439_ = "Mountainous" 
    dfe->addEnumText(MsgToken<439>());
    dfe->addEnumText(TEXT("Imhof 7"));
    dfe->addEnumText(TEXT("Imhof 4"));
    dfe->addEnumText(TEXT("Imhof 12"));
    dfe->addEnumText(TEXT("Imhof Atlas"));
    dfe->addEnumText(TEXT("ICAO")); 
	// LKTOKEN  _@M377_ = "LKoogle lowlands" 
    dfe->addEnumText(MsgToken<377>()); 
	// LKTOKEN  _@M378_ = "LKoogle mountains" 
    dfe->addEnumText(MsgToken<378>());
	// LKTOKEN  _@M412_ = "Low Alps" 
    dfe->addEnumText(MsgToken<412>());
	// LKTOKEN  _@M338_ = "High Alps" 
    dfe->addEnumText(MsgToken<338>());
    dfe->addEnumText(TEXT("YouSee"));
	// LKTOKEN  _@M340_ = "HighContrast" 
    dfe->addEnumText(MsgToken<340>());
    dfe->addEnumText(TEXT("GA Relative"));
    dfe->addEnumText(TEXT("LiteAlps"));
    dfe->addEnumText(TEXT("Low Hills"));
    dfe->addEnumText(TEXT("Low Alps color e-ink"));
    dfe->addEnumText(TEXT("Low Alps gray e-ink"));
    dfe->Set(TerrainRamp_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoBacklight")); // VENTA4
  if (wp) {
#ifndef PPC2003   // PNA is also PPC2003
    wp->SetVisible(false);
#else
    wp->SetVisible(true);
#endif
    wp->GetDataField()->Set(EnableAutoBacklight);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoSoundVolume")); // VENTA4
  if (wp) {
#ifndef PPC2003   // PNA is also PPC2003
    	wp->SetVisible(false);
#else
        wp->SetVisible(true);
#endif
    wp->GetDataField()->Set(EnableAutoSoundVolume);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFinishLine"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    auto sectors = get_finish_sectors(TSK_DEFAULT);
    if (dfe && sectors) {
      for (auto type : *sectors) {
        dfe->addEnumText(get_sectors_label(type));
      }
      dfe->Set(sectors->index(FinishLine));
    }
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFinishRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(Units::ToDistance(FinishRadius)*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskStartLine"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    auto sectors = get_start_sectors(gTaskType);
    if (dfe && sectors) {
      for (auto type : *sectors) {
        dfe->addEnumText(get_sectors_label(type));
      }
      dfe->Set(sectors->index(StartLine));
    }
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskStartRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(Units::ToDistance(StartRadius)*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFAISector"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    auto sectors = get_task_sectors(TSK_DEFAULT);
    if (dfe && sectors) {
      for (auto type : *sectors) {
        dfe->addEnumText(get_sectors_label(type));
      }
      dfe->Set(sectors->index(SectorType));
    }
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskSectorRadius"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(round(Units::ToDistance(SectorRadius)*DISTANCE_ROUNDING)/DISTANCE_ROUNDING);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoAdvance"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(MsgToken<418>()); // Manual
    dfe->addEnumText(MsgToken<897>()); // Auto
    dfe->addEnumText(MsgToken<97>()); // Arm
    dfe->addEnumText(MsgToken<96>()); // Arm start
    dfe->Set(AutoAdvance_Config);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpGPSAltitudeOffset"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(GPSAltitudeOffset / 1000.0))); // 100429
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmMaxAltitude1"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(AlarmMaxAltitude1 / 1000.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmMaxAltitude2"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(AlarmMaxAltitude2 / 1000.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmMaxAltitude3"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(AlarmMaxAltitude3 / 1000.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmTakeoffSafety"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(AlarmTakeoffSafety / 1000.0)));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmGearWarning"));
  if (wp)
  {
	DataField* dfe = wp->GetDataField();
	dfe->addEnumText(MsgToken<1831>());	// LKTOKEN  _@M1831_ "Off"
	dfe->addEnumText(MsgToken<1832>());	// LKTOKEN  _@M1832_ "Near landables"
	dfe->addEnumText(MsgToken<1833>());	// LKTOKEN   _@M1833_ "Allways"
    dfe->Set(GearWarningMode);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAdditionalContestRule"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if (dfe) {
      for (int i = 0; i < static_cast<int>(CContestMgr::ContestRule::NUM_OF_XC_RULES); i++) {
        dfe->addEnumText(CContestMgr::XCRuleToString(static_cast<CContestMgr::ContestRule>(i)));
      }
      dfe->Set(static_cast<int>(AdditionalContestRule));
    }
    wp->RefreshDisplay();
  }


#if 0  // no gear warning, just in case
{
GearWarningMode =0;
GearWarningAltitude=0;
wp = wf->FindByName<WndProperty>(TEXT("prpAlarmGearWarning"));
wp->SetVisible(false);
wp = wf->FindByName<WndProperty>(TEXT("prpAlarmGearAltitude"));
wp->SetVisible(false);
wp->RefreshDisplay();
}
#else
  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmGearAltitude"));

  if (wp) {
	if(GearWarningMode == 0)
	{
	  wp->SetVisible(false);
	}
	else
	{
      wp->GetDataField()->SetAsFloat(iround(Units::ToAltitude(GearWarningAltitude / 1000.0)));
      wp->GetDataField()->SetUnits(Units::GetAltitudeName());
	  wp->SetVisible(true);
      wp->RefreshDisplay();
    }
    wp->RefreshDisplay();
  }
#endif

  wp = wf->FindByName<WndProperty>(TEXT("prpUseGeoidSeparation"));
  if (wp) {
    wp->GetDataField()->Set(UseGeoidSeparation);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpPressureHg"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->addEnumText(TEXT("hPa"));
    dfe->addEnumText(TEXT("inHg"));
    dfe->Set(PressureHg);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpEngineeringMenu")); 
  if (wp) {
    wp->GetDataField()->Set(EngineeringMenu);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAndroidAudioVario"));
  if (wp) {
#ifndef ANDROID
    wp->SetVisible(false);
#endif
    wp->GetDataField()->Set(EnableAudioVario);
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

static void OnLeScan(WndForm* pWndForm, const char *address, const char *name, const char *type) {
  ScopeLock lock(COMMPort_mutex);

  using std::string_view_literals::operator""sv;

  auto prefix = [&]() {
    if ("HM10"sv == type) {
      return "BT_HM10:";
    }
    else {
      return "BLE:";
    }
  };

  std::stringstream prefixed_address_stream;
  prefixed_address_stream << prefix() << address;
  std::string prefixed_address = prefixed_address_stream.str();

  std::stringstream prefixed_name_stream;
  prefixed_name_stream << prefix() << ((strlen(name)>0)? name : address);

  auto it = FindCOMMPort(prefixed_address.c_str());
  if (it == COMMPort.end()) {
    COMMPort.emplace_back(std::move(prefixed_address), prefixed_name_stream.str());
  }
  else {
    (*it) = { std::move(prefixed_address), prefixed_name_stream.str() };
  }

  if(pWndForm) {
    pWndForm->SendUser(UPDATE_COM_PORT);
  }
}

static bool OnUser(WndForm * pWndForm, unsigned id) {
  switch(id) {
    case UPDATE_COM_PORT: {
      auto pWnd = pWndForm->FindByName<WndProperty>(_T("prpComPort1"));
      if (pWnd) {
        DataField * dataField = pWnd->GetDataField();
        if(dataField) {

          ScopeLock lock(COMMPort_mutex);

          for( const auto& item : COMMPort ) {
            int idx = dataField->Find(item.GetName());
            if(idx == -1) {
              dataField->addEnumText(item.GetName(), item.GetLabel());
            } else {
              dataField->setEnumLabel(idx, item.GetLabel());
            }
          }
        }
        pWnd->RefreshDisplay();
      }
      return true;
    }
    default:
      break;
  }
  return false;
}

#endif



//
// Setup device dialogs fine tuning
//
static void InitDlgDevice(WndForm *pWndForm) {

  if(!pWndForm) {
    LKASSERT(0);
    return;
  }

  // spacing between buttons and left&right
  const unsigned int SPACEBORDER = DLGSCALE(2);
  const unsigned int w = (pWndForm->GetWidth() - (SPACEBORDER * (MAXNUMDEVICES + 1))) / MAXNUMDEVICES;
  unsigned int lx = SPACEBORDER; // count from 0

  static_assert(MAXNUMDEVICES == std::size(DeviceList), "wrong array size");

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
  
  static_assert(std::size(dlgTemplate_L) == std::size(dlgTemplate_P), "check array size");
  
  StartHourglassCursor();

  if (configMode==CONFIGMODE_DEVICE) {
    RefreshComPortList();
  }

  if(configMode >= 0 && configMode < (int)std::size(dlgTemplate_L)) {
    
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
    BluetoothLeScanPtr = std::make_unique<BluetoothLeScan>(wf, OnLeScan);
  }
#endif

  wf->SetKeyDownNotify(FormKeyDown);

  LKASSERT(wf->FindByName<WndButton>(TEXT("cmdClose")));
  (wf->FindByName<WndButton>(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  reset_wConfig();

  for(size_t i = 0; i < std::size(ConfigPageNames[configMode]); i++) {
      wConfig[i]    = wf->FindByName<WndFrame>(ConfigPageNames[configMode][i].szName);
      numPages += wConfig[i]?1:0;
  }
  std::fill(std::begin(cpyInfoBox), std::end(cpyInfoBox), -1);

  setVariables(wf);

  if (configMode==CONFIGMODE_DEVICE) {
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
     InitDlgDevice(wf);


  } // end configmode_device

  wp = wf->FindByName<WndProperty>(_T("prpEarthModel"));
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
  
  bool notify_units_change = false;
  bool notify_reset_zoom = false;

#ifdef ANDROID
    // stop LE Scanner first dialog close
    BluetoothLeScanPtr = nullptr;
#endif

#ifdef _WGS84
  wp = wf->FindByName<WndProperty>(_T("prpEarthModel"));
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

  wp = wf->FindByName<WndProperty>(TEXT("prpDisableAutoLogger"));
  if (wp) {
    if (!DisableAutoLogger
	!= wp->GetDataField()->GetAsBoolean()) {
      DisableAutoLogger = 
	!(wp->GetDataField()->GetAsBoolean());
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLiveTrackerInterval"));
  if (wp) {
    if (tracking::interval != wp->GetDataField()->GetAsInteger()) {
        tracking::interval = (wp->GetDataField()->GetAsInteger());
      requirerestart = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLiveTrackerRadar_config"));
  if (wp) {
    if (tracking::radar_config != wp->GetDataField()->GetAsBoolean()) {
        tracking::radar_config = wp->GetDataField()->GetAsBoolean();
      requirerestart = true;
    }
  }
  
  double val;

  wp = wf->FindByName<WndProperty>(TEXT("prpSafetyMacCready"));
  if (wp) {
    val = Units::FromVerticalSpeed(wp->GetDataField()->GetAsFloat());
    if (GlidePolar::SafetyMacCready != val) {
      GlidePolar::SafetyMacCready = val;
    }
  }
#if defined(PPC2003) || defined(PNA)
  wp = wf->FindByName<WndProperty>(TEXT("prpSetSystemTimeFromGPS"));
  if (wp) {
    if (SetSystemTimeFromGPS != wp->GetDataField()->GetAsBoolean()) {
      SetSystemTimeFromGPS = wp->GetDataField()->GetAsBoolean();
    }
  }
#endif

  wp = wf->FindByName<WndProperty>(TEXT("prpSaveRuntime"));
  if (wp) {
    if (SaveRuntime != wp->GetDataField()->GetAsBoolean()) {
      SaveRuntime = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTrailDrift"));
  if (wp) {
    if (EnableTrailDrift_Config != wp->GetDataField()->GetAsBoolean()) {
      EnableTrailDrift_Config = wp->GetDataField()->GetAsBoolean();
      MapWindow::EnableTrailDrift = EnableTrailDrift_Config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpThermalLocator"));
  if (wp) {
    if (EnableThermalLocator != wp->GetDataField()->GetAsInteger()) {
      EnableThermalLocator = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTrail"));
  if (wp) {
    if (TrailActive_Config != wp->GetDataField()->GetAsInteger()) {
      TrailActive_Config = wp->GetDataField()->GetAsInteger();
      TrailActive = TrailActive_Config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLKMaxLabels"));
  if (wp) {
    if (LKMaxLabels != wp->GetDataField()->GetAsInteger()) {
      LKMaxLabels = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBugs"));
  if (wp) {
    if (BUGS_Config != wp->GetDataField()->GetAsFloat()/100.0) {
      BUGS_Config = wp->GetDataField()->GetAsFloat()/100.0;
      CheckSetBugs(BUGS_Config, nullptr);
      requirerestart=true;
    }
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpCruiseZoom"));
  if (wp) {
    if ( CruiseZoom != wp->GetDataField()->GetAsInteger()) {
      CruiseZoom = wp->GetDataField()->GetAsInteger();
      MapWindow::zoom.Reset();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpClimbZoom"));
  if (wp) {
    if ( ClimbZoom != wp->GetDataField()->GetAsInteger()) {
      ClimbZoom = wp->GetDataField()->GetAsInteger();
      MapWindow::zoom.Reset();
    }
  }


  double dval;
  wp = wf->FindByName<WndProperty>(TEXT("prpAutoZoomThreshold"));
  if (wp) {
    dval = Units::FromDistance(wp->GetDataField()->GetAsFloat());
    if (AutoZoomThreshold != dval) {
      AutoZoomThreshold = dval;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMaxAutoZoom"));
  if (wp) {
    if ( MaxAutoZoom != wp->GetDataField()->GetAsInteger()) {
      MaxAutoZoom = wp->GetDataField()->GetAsInteger();
      MapWindow::zoom.Reset();
    }
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpAutoOrientScale"));
  if (wp) {
    if ( AutoOrientScale != wp->GetDataField()->GetAsFloat()) {
      AutoOrientScale = wp->GetDataField()->GetAsFloat();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAirspaceDisplay"));
  if (wp) {
    if (AltitudeMode_Config != wp->GetDataField()->GetAsInteger()) {
      AltitudeMode_Config = wp->GetDataField()->GetAsInteger();
      AltitudeMode = AltitudeMode_Config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSafetyAltitudeMode"));
  if (wp) {
    if (SafetyAltitudeMode != wp->GetDataField()->GetAsInteger()) {
      SafetyAltitudeMode = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLockSettingsInFlight"));
  if (wp) {
    if (LockSettingsInFlight != 
	wp->GetDataField()->GetAsBoolean()) {
      LockSettingsInFlight = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLoggerShortName"));
  if (wp) {
    if (LoggerShortName != 
	wp->GetDataField()->GetAsBoolean()) {
      LoggerShortName = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpEnableFLARMMap"));
  if (wp) {
    if ((int)EnableFLARMMap != 
	wp->GetDataField()->GetAsInteger()) {
      EnableFLARMMap = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpDebounceTimeout"));
  if (wp) {
    if (debounceTimeout != (unsigned)wp->GetDataField()->GetAsInteger()) {
      debounceTimeout = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAirspaceFillType"));
  if (wp) {
    if (MapWindow::GetAirSpaceFillType() != wp->GetDataField()->GetAsInteger()) {
      MapWindow::SetAirSpaceFillType((MapWindow::EAirspaceFillType)wp->GetDataField()->GetAsInteger());
    }
  }
  
  wp = wf->FindByName<WndProperty>(TEXT("prpAirspaceOpacity"));
  if (wp) {
    if (MapWindow::GetAirSpaceOpacity() != wp->GetDataField()->GetAsInteger()*10) {
      MapWindow::SetAirSpaceOpacity(wp->GetDataField()->GetAsInteger() * 10);
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBarOpacity"));
  if (wp) {
    if (BarOpacity != wp->GetDataField()->GetAsInteger()*5 ) {
	BarOpacity= wp->GetDataField()->GetAsInteger() * 5;
    }
  }

  
  wp = wf->FindByName<WndProperty>(TEXT("prpFontMapWaypoint"));
  if (wp) {
      if (FontMapWaypoint != wp->GetDataField()->GetAsInteger() ) {
          FontMapWaypoint = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }



  wp = wf->FindByName<WndProperty>(TEXT("prpFontSymbols"));
  if (wp) {
    if (Appearance.UTF8Pictorials != (IndLandable_t)(wp->GetDataField()->GetAsInteger())) {
      Appearance.UTF8Pictorials = (IndLandable_t)(wp->GetDataField()->GetAsInteger());
      requirerestart = true;
    }
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpFontMapTopology"));
  if (wp) {
      if (FontMapTopology != wp->GetDataField()->GetAsInteger() ) {
          FontMapTopology = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpFontInfopage1L"));
  if (wp) {
      if (FontInfopage1L != wp->GetDataField()->GetAsInteger() ) {
          FontInfopage1L = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpFontBottomBar"));
  if (wp) {
      if (FontBottomBar != wp->GetDataField()->GetAsInteger() ) {
          FontBottomBar = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSnailScale"));
  if (wp) {
      if (SnailScale != wp->GetDataField()->GetAsInteger() ) {
          SnailScale = wp->GetDataField()->GetAsInteger();
          snailchanged=true;
      }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpFontVisualGlide"));
  if (wp) {
      if (FontVisualGlide != wp->GetDataField()->GetAsInteger() ) {
          FontVisualGlide = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOverlayTarget"));
  if (wp) {
      if (FontOverlayMedium != wp->GetDataField()->GetAsInteger() ) {
          FontOverlayMedium = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOverlayValues"));
  if (wp) {
      if (FontOverlayBig != wp->GetDataField()->GetAsInteger() ) {
          FontOverlayBig = wp->GetDataField()->GetAsInteger();
          fontschanged=true;
      }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUseTwoLines"));
  if (wp) {
      if (UseTwoLines != wp->GetDataField()->GetAsBoolean()) {
          UseTwoLines = wp->GetDataField()->GetAsBoolean();
          Reset_Single_DoInits(MDI_DRAWNEAREST);
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpFontRenderer"));
  if (wp) {
    if (FontRenderer != wp->GetDataField()->GetAsInteger() ) 
    {
	    FontRenderer = wp->GetDataField()->GetAsInteger();
      requirerestart = true;
    }
  }
  
  wp = wf->FindByName<WndProperty>(TEXT("prpAutoZoom"));
  if (wp) {
    if (AutoZoom_Config != 
	wp->GetDataField()->GetAsBoolean()) {
      AutoZoom_Config = wp->GetDataField()->GetAsBoolean();
      MapWindow::zoom.AutoZoom(AutoZoom_Config);
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpDrawFAI"));
  if (wp) {
    if (Flags_DrawFAI_config != wp->GetDataField()->GetAsBoolean()) {
      Flags_DrawFAI_config = wp->GetDataField()->GetAsBoolean();
      Flags_DrawFAI = Flags_DrawFAI_config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpDrawXC"));
  if (wp) {
    if (Flags_DrawXC_config !=  wp->GetDataField()->GetAsBoolean()) {
      Flags_DrawXC_config = wp->GetDataField()->GetAsBoolean();
      Flags_DrawXC = Flags_DrawXC_config;
    }
  }


int ival; 

  wp = wf->FindByName<WndProperty>(TEXT("prpUTCOffset"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsFloat()*3600.0);
    if ((UTCOffset != ival)||(utcchanged)) {
      UTCOffset = ival;

    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpClipAltitude"));
  if (wp) {
    ival = iround(Units::FromAltitude(wp->GetDataField()->GetAsInteger()) * 10);
    if (ClipAltitude != ival) {
      ClipAltitude = ival;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAltWarningMargin"));
  if (wp) {
    ival = iround(Units::FromAltitude(wp->GetDataField()->GetAsInteger())*10);
    if (AltWarningMargin != ival) {
      AltWarningMargin = ival;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWaypointLabels"));
  if (wp) {
    if (DisplayTextType != wp->GetDataField()->GetAsInteger()) {
      DisplayTextType = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpCirclingZoom"));
  if (wp) {
    if (MapWindow::zoom.CircleZoom() != wp->GetDataField()->GetAsBoolean()) {
      MapWindow::zoom.CircleZoom(wp->GetDataField()->GetAsBoolean());
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOrientation"));
  if (wp) {
    if (DisplayOrientation_Config != wp->GetDataField()->GetAsInteger()) {
      DisplayOrientation_Config = wp->GetDataField()->GetAsInteger();
      DisplayOrientation=DisplayOrientation_Config;
      MapWindow::SetAutoOrientation(); // reset
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMenuTimeout"));
  if (wp) {
    if (MenuTimeout_Config != wp->GetDataField()->GetAsInteger()*2) {
      MenuTimeout_Config = wp->GetDataField()->GetAsInteger()*2;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSafetyAltitudeArrival"));
  if (wp) {
    ival = iround(Units::FromAltitude(wp->GetDataField()->GetAsInteger())*10);
    if (SAFETYALTITUDEARRIVAL != ival) {
      SAFETYALTITUDEARRIVAL = ival;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSafetyAltitudeTerrain"));
  if (wp) {
    ival = iround(Units::FromAltitude(wp->GetDataField()->GetAsInteger())*10);
    if (SAFETYALTITUDETERRAIN != ival) {
      SAFETYALTITUDETERRAIN = ival;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoWind"));
  if (wp) {
    if (AutoWindMode_Config != wp->GetDataField()->GetAsInteger()) {
      AutoWindMode_Config = wp->GetDataField()->GetAsInteger();
      AutoWindMode = AutoWindMode_Config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoMcMode"));
  if (wp) {
    if (AutoMcMode_Config != wp->GetDataField()->GetAsInteger()) {
      AutoMcMode_Config = wp->GetDataField()->GetAsInteger();
	AutoMcMode=AutoMcMode_Config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWaypointsOutOfRange"));
  if (wp) {
#ifdef ASK_WAYPOINTS
	  if (WaypointsOutOfRange != wp->GetDataField()->GetAsInteger()) {
		WaypointsOutOfRange = wp->GetDataField()->GetAsInteger();
		WAYPOINTFILECHANGED= true;
		AIRSPACEFILECHANGED = true;
	  }
#else  // option ASK  is removed but numbers  for WaypointsOutOfRange stay the same
	  if (WaypointsOutOfRange != (wp->GetDataField()->GetAsInteger()+1)) {
		WaypointsOutOfRange = wp->GetDataField()->GetAsInteger()+1;
		WAYPOINTFILECHANGED= true;
		AIRSPACEFILECHANGED = true;
	  }
#endif
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoForceFinalGlide"));
  if (wp) {
    if (AutoForceFinalGlide != wp->GetDataField()->GetAsBoolean()) {
      AutoForceFinalGlide = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpEnableNavBaroAltitude"));
  if (wp) {
    if (EnableNavBaroAltitude_Config != wp->GetDataField()->GetAsBoolean()) {
      EnableNavBaroAltitude_Config = wp->GetDataField()->GetAsBoolean();
      EnableNavBaroAltitude=EnableNavBaroAltitude_Config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOrbiter"));
  if (wp) {
    if (Orbiter_Config != wp->GetDataField()->GetAsBoolean()) {
      Orbiter_Config = wp->GetDataField()->GetAsBoolean();
      Orbiter=Orbiter_Config;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpAutoMcStatus"));
  if (wp) {
    if (AutoMacCready_Config != wp->GetDataField()->GetAsBoolean()) {
      AutoMacCready_Config = wp->GetDataField()->GetAsBoolean();
      CALCULATED_INFO.AutoMacCready=AutoMacCready_Config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpShading"));
  if (wp) {
    if (Shading_Config != wp->GetDataField()->GetAsBoolean()) {
      Shading_Config = wp->GetDataField()->GetAsBoolean();
      Shading=Shading_Config;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpIsoLine"));
  if (wp) {
    if (IsoLine_Config != wp->GetDataField()->GetAsBoolean()) {
      IsoLine_Config = wp->GetDataField()->GetAsBoolean();
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpSonarWarning"));
  if (wp) {
    if (SonarWarning_Config != wp->GetDataField()->GetAsBoolean()) {
      SonarWarning_Config = wp->GetDataField()->GetAsBoolean();
      SonarWarning=SonarWarning_Config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpFinalGlideTerrain"));
  if (wp) {
    if (FinalGlideTerrain != wp->GetDataField()->GetAsInteger()) {
      FinalGlideTerrain = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsSpeed"));
  if (wp) {
    if (Units::SpeedUnit_Config != wp->GetDataField()->GetAsInteger()) {
      Units::SpeedUnit_Config = wp->GetDataField()->GetAsInteger();
      notify_units_change = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsLatLon"));
  if (wp) {
    if (Units::LatLonUnits_Config != wp->GetDataField()->GetAsInteger()) {
      Units::LatLonUnits_Config = wp->GetDataField()->GetAsInteger();
      notify_units_change = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsTaskSpeed"));
  if (wp) {
    if (Units::TaskSpeedUnit_Config != wp->GetDataField()->GetAsInteger()) {
      Units::TaskSpeedUnit_Config = wp->GetDataField()->GetAsInteger();
      notify_units_change = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsDistance"));
  if (wp) {
    if (Units::DistanceUnit_Config != wp->GetDataField()->GetAsInteger()) {
      Units::DistanceUnit_Config = wp->GetDataField()->GetAsInteger();
      notify_units_change = true;
      notify_reset_zoom = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsLift"));
  if (wp) {
    if (Units::VerticalSpeedUnit_Config != wp->GetDataField()->GetAsInteger()) {
      Units::VerticalSpeedUnit_Config = wp->GetDataField()->GetAsInteger();
      notify_units_change = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUnitsAltitude"));
  if (wp) {
    if (Units::AltitudeUnit_Config != wp->GetDataField()->GetAsInteger()) {
      Units::AltitudeUnit_Config = wp->GetDataField()->GetAsInteger();
      notify_units_change = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMapFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szMapFile)) {
      _tcscpy(szMapFile,temptext);
      MAPFILECHANGED= true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTerrainFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szTerrainFile)) {
      _tcscpy(szTerrainFile,temptext);
      TERRAINFILECHANGED= true;
      if(WaypointsOutOfRange > 1)
      {
	    WAYPOINTFILECHANGED  = true;
	    AIRSPACEFILECHANGED  = true;
      }
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAirfieldFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szAirfieldFile)) {
      _tcscpy(szAirfieldFile,temptext);
      AIRFIELDFILECHANGED= true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLanguageCode"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    _tcscpy(temptext, dfe->GetAsString());
    if (_tcscmp(temptext,szLanguageCode)) {
      _tcscpy(szLanguageCode,temptext);
      requirerestart = true; // restart needed for language load
      // LKLoadLanguageFile(); // NO GOOD. MEMORY LEAKS PENDING
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpInputFile"));
  if (wp) {
    DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    if (_tcscmp(temptext,szInputFile)) {
      _tcscpy(szInputFile,temptext);
      requirerestart = true;
    }
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpWindCalcTime")); // 100113
  if (wp) {
	ival = wp->GetDataField()->GetAsInteger();
	if (WindCalcTime != ival) {
		WindCalcTime = ival;
	}
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpWindCalcSpeed"));
  if (wp) {
    WindCalcSpeed = iround(Units::FromHorizontalSpeed(wp->GetDataField()->GetAsInteger())*1000.0) / 1000;
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFinishLine"));
  if (wp) {
    auto sectors = get_finish_sectors(TSK_DEFAULT);
    sector_type_t new_type = sectors->type(wp->GetDataField()->GetAsInteger());
    if (FinishLine != new_type) {
      FinishLine = new_type;
      taskchanged = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFinishRadius"));
  if (wp) {
    ival = iround(Units::FromDistance(wp->GetDataField()->GetAsFloat()));
    if ((int)FinishRadius != ival) {
      FinishRadius = ival;
      taskchanged = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskStartLine"));
  if (wp) {
    auto sectors = get_start_sectors(gTaskType);
    sector_type_t new_type = sectors->type(wp->GetDataField()->GetAsInteger());
    if (StartLine != new_type) {
      StartLine = new_type;
      taskchanged = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskStartRadius"));
  if (wp) {
    ival = iround(Units::FromDistance(wp->GetDataField()->GetAsFloat()));
    if ((int)StartRadius != ival) {
      StartRadius = ival;
      taskchanged = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskFAISector"));
  if (wp) {
    auto sectors = get_task_sectors(TSK_DEFAULT);
    sector_type_t new_type = sectors->type(wp->GetDataField()->GetAsInteger());
    if (SectorType != new_type) {
      SectorType = new_type;
      taskchanged = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTaskSectorRadius"));
  if (wp) {
    ival = iround(Units::FromDistance(wp->GetDataField()->GetAsFloat()));
    if ((int)SectorRadius != ival) {
      SectorRadius = ival;
      taskchanged = true;
    }
  }


  #if (0)
  wp = wf->FindByName<WndProperty>(TEXT("prpAltArrivMode"));
  if (wp) {
    if (AltArrivMode != (AltArrivMode_t)
	(wp->GetDataField()->GetAsInteger())) {
      AltArrivMode = (AltArrivMode_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }
  #endif

  wp = wf->FindByName<WndProperty>(TEXT("prpCheckSum")); 
  if (wp) {
    if (CheckSum != (CheckSum_t)
	(wp->GetDataField()->GetAsInteger())) {
      CheckSum = (CheckSum_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpIphoneGestures")); 
  if (wp) {
    if (IphoneGestures != (IphoneGestures_t) (wp->GetDataField()->GetAsInteger())) {
      IphoneGestures = (IphoneGestures_t) (wp->GetDataField()->GetAsInteger());
    }
  }

#ifdef ANDROID
  wp = wf->FindByName<WndProperty>(TEXT("prpAndroidScreenOrientation"));
  if (wp) {
    int ret = wp->GetDataField()->GetAsInteger();
    if (ret != native_view->getScreenOrientation()) {
      native_view->setScreenOrientation(ret);
    }
  }
#endif

  wp = wf->FindByName<WndProperty>(TEXT("prpPollingMode")); 
  if (wp) {
    if (PollingMode != (PollingMode_t) (wp->GetDataField()->GetAsInteger())) {
      PollingMode = (PollingMode_t) (wp->GetDataField()->GetAsInteger());
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpLKVarioBar")); 
  if (wp) {
    const LKVarioBar_t tmpValue = static_cast<LKVarioBar_t>(wp->GetDataField()->GetAsInteger());
    if (LKVarioBar != tmpValue) {
      LKVarioBar = tmpValue;
      Reset_Single_DoInits(MDI_DRAWVARIO);
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpLKVarioVal")); 
  if (wp) {
    if (LKVarioVal != (LKVarioVal_t)
	(wp->GetDataField()->GetAsInteger())) {
      LKVarioVal = (LKVarioVal_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpHideUnits"));
  if (wp) {
      if (HideUnits != (HideUnits_t) (wp->GetDataField()->GetAsBoolean())) {
          HideUnits = (HideUnits_t) (wp->GetDataField()->GetAsBoolean());
          Reset_Single_DoInits(MDI_DRAWBOTTOMBAR);
          Reset_Single_DoInits(MDI_DRAWFLIGHTMODE);
      }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpDeclutterMode")); // VENTA10
  if (wp) {
    if (DeclutterMode != (DeclutterMode_t)
	(wp->GetDataField()->GetAsInteger())) {
      DeclutterMode = (DeclutterMode_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBestWarning"));
  if (wp) {
    if (BestWarning != (wp->GetDataField()->GetAsBoolean())) {
      BestWarning = (wp->GetDataField()->GetAsBoolean());
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpUseTotalEnergy"));
  if (wp) {
    if (UseTotalEnergy_Config != (wp->GetDataField()->GetAsBoolean())) {
      UseTotalEnergy_Config = (wp->GetDataField()->GetAsBoolean());
      UseTotalEnergy=UseTotalEnergy_Config;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpUseUngestures"));
  if (wp) {
    if (UseUngestures != (wp->GetDataField()->GetAsBoolean())) {
      UseUngestures = (wp->GetDataField()->GetAsBoolean());
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpThermalBar"));
  if (wp) {
    if (ThermalBar != (unsigned)(wp->GetDataField()->GetAsInteger())) {
      ThermalBar = (wp->GetDataField()->GetAsInteger());
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpOverlayClock"));
  if (wp) {
    if (OverlayClock != (wp->GetDataField()->GetAsInteger())) {
      OverlayClock = (wp->GetDataField()->GetAsInteger());
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpTrackBar"));
  if (wp) {
    if (TrackBar != (wp->GetDataField()->GetAsBoolean())) {
      TrackBar = (wp->GetDataField()->GetAsBoolean());
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpPGOptimizeRoute"));
  if (wp) {
    if (TskOptimizeRoute != (wp->GetDataField()->GetAsBoolean())) {
      TskOptimizeRoute = (wp->GetDataField()->GetAsBoolean());

      if (gTaskType==TSK_GP) {
        ClearOptimizedTargetPos();
	  }
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpGliderSymbol"));
  if (wp) {
    if (GliderSymbol != wp->GetDataField()->GetAsInteger() )
    {
    	GliderSymbol = wp->GetDataField()->GetAsInteger();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOutlinedTp"));
  if (wp) {
	if (OutlinedTp_Config != (OutlinedTp_t) (wp->GetDataField()->GetAsInteger()))  {
		OutlinedTp_Config = (OutlinedTp_t) (wp->GetDataField()->GetAsInteger());
		OutlinedTp=OutlinedTp_Config;
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTpFilter"));
  if (wp) {
    if (TpFilter != (TpFilter_t)
	(wp->GetDataField()->GetAsInteger())) {
      TpFilter = (TpFilter_t)
	(wp->GetDataField()->GetAsInteger());
      LastDoRangeWaypointListTime=0;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOverColor"));
  if (wp) {
    if (OverColor != (OverColor_t)
	(wp->GetDataField()->GetAsInteger())) {
      OverColor = (OverColor_t)
	(wp->GetDataField()->GetAsInteger());
      SetOverColorRef();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpMapBox")); // VENTA6
  if (wp) {
    if (MapBox != (MapBox_t)
	(wp->GetDataField()->GetAsInteger())) {
      MapBox = (MapBox_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpOverlaySize"));
  if (wp) {
    if (OverlaySize != wp->GetDataField()->GetAsInteger() ) {
      OverlaySize = wp->GetDataField()->GetAsInteger();
      Reset_Single_DoInits(MDI_DRAWLOOK8000);
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpGlideBarMode")); // VENTA6
  if (wp) {
    if (GlideBarMode != (GlideBarMode_t)
	(wp->GetDataField()->GetAsInteger())) {
      GlideBarMode = (GlideBarMode_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpNewMapDeclutter")); // VENTA6
  if (wp) {
    if (NewMapDeclutter != 
	(wp->GetDataField()->GetAsInteger())) {
      NewMapDeclutter = 
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpGPSAltitudeOffset")); 
  if (wp) {
    ival = iround( Units::FromAltitude(wp->GetDataField()->GetAsInteger())*1000.0);
    if(((int) GPSAltitudeOffset) != ival) {
      GPSAltitudeOffset = ival;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpUseGeoidSeparation"));
  if (wp) {
	if (UseGeoidSeparation != (wp->GetDataField()->GetAsBoolean())) {
		UseGeoidSeparation = (wp->GetDataField()->GetAsBoolean());
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpPressureHg"));
  if (wp) {
	if (PressureHg != (wp->GetDataField()->GetAsInteger())) {
		PressureHg = (wp->GetDataField()->GetAsInteger());
	}
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAverEffTime"));
  if (wp) {
    if (AverEffTime != 
	(wp->GetDataField()->GetAsInteger())) {
      AverEffTime = 
	(wp->GetDataField()->GetAsInteger());
        LKSW_ResetLDRotary=true;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpBgMapcolor"));
  if (wp) {
    if (BgMapColor_Config != (wp->GetDataField()->GetAsInteger())) {
      BgMapColor_Config = (wp->GetDataField()->GetAsInteger());
      BgMapColor = BgMapColor_Config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpArrivalValue")); // VENTA6
  if (wp) {
    if (ArrivalValue != (ArrivalValue_t)
	(wp->GetDataField()->GetAsInteger())) {
      ArrivalValue = (ArrivalValue_t)
	(wp->GetDataField()->GetAsInteger());
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAppInfoBoxModel"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    ModelType::Type_t selected = ModelType::get_id(dfe->GetAsInteger());
    if (ModelType::Get() != selected) {
      ModelType::Set(selected);
      requirerestart = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAppIndLandable"));
  if (wp) {
    if (Appearance.IndLandable != (IndLandable_t)(wp->GetDataField()->GetAsInteger())) {
      Appearance.IndLandable = (IndLandable_t)(wp->GetDataField()->GetAsInteger());
      requirerestart = true;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAppInverseInfoBox"));
  if (wp) {
    if ((int)(InverseInfoBox_Config) != wp->GetDataField()->GetAsInteger()) {
      InverseInfoBox_Config = (wp->GetDataField()->GetAsInteger() != 0);
      requirerestart = true;
      Appearance.InverseInfoBox=InverseInfoBox_Config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpGliderScreenPosition"));
  if (wp) {
    if (MapWindow::GliderScreenPosition != 
	wp->GetDataField()->GetAsInteger()) {
      MapWindow::GliderScreenPosition = wp->GetDataField()->GetAsInteger();
	MapWindow::GliderScreenPositionY=MapWindow::GliderScreenPosition;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoBacklight")); // VENTA4
  if (wp) {
    if (EnableAutoBacklight != wp->GetDataField()->GetAsBoolean()) {
      EnableAutoBacklight = wp->GetDataField()->GetAsBoolean();
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoSoundVolume")); // VENTA4
  if (wp) {
    if (EnableAutoSoundVolume != wp->GetDataField()->GetAsBoolean()) {
      EnableAutoSoundVolume = wp->GetDataField()->GetAsBoolean();
    }
  }


  wp = wf->FindByName<WndProperty>(TEXT("prpTerrainContrast"));
  if (wp) {
    if (iround((TerrainContrast*100)/255) != 
	wp->GetDataField()->GetAsInteger()) {
      TerrainContrast = (short)iround(wp->GetDataField()->GetAsInteger()*255.0/100);
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTerrainBrightness"));
  if (wp) {
    if (iround((TerrainBrightness*100)/255) != 
	wp->GetDataField()->GetAsInteger()) {
      TerrainBrightness = (short)iround(wp->GetDataField()->GetAsInteger()*255.0/100);
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTerrainRamp"));
  if (wp) {
    if (TerrainRamp_Config != wp->GetDataField()->GetAsInteger()) {
      TerrainRamp_Config = wp->GetDataField()->GetAsInteger();
      TerrainRamp=TerrainRamp_Config;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmMaxAltitude1"));
  if (wp) {
    ival = iround( Units::FromAltitude(wp->GetDataField()->GetAsInteger()) *1000.0);
    if ((int)AlarmMaxAltitude1 != ival) {
      AlarmMaxAltitude1 = ival;
      LKalarms[0].triggervalue=(int)AlarmMaxAltitude1/1000;
      LKalarms[0].triggerscount=0;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmMaxAltitude2"));
  if (wp) {
    ival = iround(Units::FromAltitude(wp->GetDataField()->GetAsInteger()) *1000.0);
    if ((int)AlarmMaxAltitude2 != ival) {
      AlarmMaxAltitude2 = ival;
      LKalarms[1].triggervalue=(int)AlarmMaxAltitude2/1000;
      LKalarms[1].triggerscount=0;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmMaxAltitude3"));
  if (wp) {
    ival = iround(Units::FromAltitude(wp->GetDataField()->GetAsInteger()) *1000.0);
    if ((int)AlarmMaxAltitude3 != ival) {
      AlarmMaxAltitude3 = ival;
      LKalarms[2].triggervalue=(int)AlarmMaxAltitude3/1000;
      LKalarms[2].triggerscount=0;
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmTakeoffSafety"));
  if (wp) {
    ival = iround(Units::FromAltitude(wp->GetDataField()->GetAsInteger()) *1000.0);
    if ((int)AlarmTakeoffSafety != ival) {
      AlarmTakeoffSafety = ival;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmGearWarning"));
  if (wp) {
    ival = iround( wp->GetDataField()->GetAsInteger() );
    if ((int)GearWarningMode != ival) {
    	GearWarningMode = ival;
    }

  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAdditionalContestRule"));
  if (wp) {
      AdditionalContestRule = static_cast<CContestMgr::ContestRule>(wp->GetDataField()->GetAsInteger());
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAlarmGearAltitude"));
  if (wp) {
	  unsigned tmp = iround(Units::FromAltitude(wp->GetDataField()->GetAsInteger()) *1000.0);
    if (GearWarningAltitude != tmp)
    {
    	GearWarningAltitude = tmp;
    }
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoAdvance"));
  if (wp) {
    if (AutoAdvance_Config != wp->GetDataField()->GetAsInteger()) {
      AutoAdvance_Config = wp->GetDataField()->GetAsInteger();
      AutoAdvance=AutoAdvance_Config;
    }
  }

  UpdateDeviceSetupButton(wf, SelectedDevice);

  wp = wf->FindByName<WndProperty>(TEXT("prpEngineeringMenu")); // VENTA9
  if (wp) EngineeringMenu = wp->GetDataField()->GetAsBoolean();

  wp = wf->FindByName<WndProperty>(TEXT("prpAndroidAudioVario"));
  if (wp) {
    if (std::exchange(EnableAudioVario, wp->GetDataField()->GetAsBoolean()) != EnableAudioVario) {
      // TODO : notify thread waiting for `EnableAudioVario`.
      // currently, only Android `Internal` wait for this.
      // must be refactored if another one is added.
      for (auto& d : DeviceList) {
        ScopeLock lock(CritSec_Comm);
        if (d.Com) {
          d.Com->CancelWaitEvent();
        }
      }
    }
  }

  UpdateAircraftConfig();

  int i,j;
  for (i=0; i<4; i++) {
    for (j=0; j<8; j++) {
      GetInfoBoxSelector(j, i);
    }
  }
  
  
  if(notify_units_change) {
    Units::NotifyUnitChanged();
  }
  
  // ! important ! need to be done after #Units::NotifyUnitChanged()
  if(notify_reset_zoom) {
      MapWindow::zoom.Reset();
      MapWindow::zoom.ModifyMapScale();
      MapWindow::FillScaleListForEngineeringUnits();
      MapWindow::RefreshMap();
  }

  if (waypointneedsave) {
    if(MessageBoxX(
    // LKTOKEN  _@M581_ = "Save changes to waypoint file?" 
                   MsgToken<581>(),
	// LKTOKEN  _@M809_ = "Waypoints edited" 
                   MsgToken<809>(),
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

  InitActiveGate();

    if (requirerestart) {
      MessageBoxX (
	// LKTOKEN  _@M561_ = "Restart LK8000 to apply changes." 
		   MsgToken<561>(), 
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

 wp = wf->FindByName<WndProperty>(TEXT("prpAircraftCategory"));
  if (wp) {
    if (AircraftCategory != (AircraftCategory_t)
        (wp->GetDataField()->GetAsInteger())) {
      AircraftCategory = (AircraftCategory_t)
        (wp->GetDataField()->GetAsInteger());
      requirerestart = true;
      AIRCRAFTTYPECHANGED=true;
    }
  }

 wp = wf->FindByName<WndProperty>(TEXT("prpPolarFile"));
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

 wp = wf->FindByName<WndProperty>(TEXT("prpMaxManoeuveringSpeed"));
  if (wp) {
    ival = iround(Units::FromHorizontalSpeed(wp->GetDataField()->GetAsFloat())*1000.0);
    if ((int)(SAFTEYSPEED*1000) != (int)iround(ival)) {
        SAFTEYSPEED=ival/1000.0;
      GlidePolar::SetBallast();
    }
  }

 wp = wf->FindByName<WndProperty>(TEXT("prpHandicap"));
  if (wp) {
    ival  = wp->GetDataField()->GetAsInteger();
    if (Handicap != ival) {
      Handicap = ival;
    }
  }

 wp = wf->FindByName<WndProperty>(TEXT("prpBallastSecsToEmpty"));
  if (wp) {
    ival = wp->GetDataField()->GetAsInteger();
    if (BallastSecsToEmpty != ival) {
      BallastSecsToEmpty = ival;
    }
  }
}

