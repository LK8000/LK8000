/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgConfiguration.cpp,v 8.12 2010/12/13 12:50:04 root Exp root $
*/


#include "StdAfx.h"
#include <aygshell.h>

#include "XCSoar.h"
#include "Utils2.h"
#include "MapWindow.h"
#include "Terrain.h"
#include "GaugeFLARM.h"

#include "WindowControls.h"
#include "Statistics.h"
#include "externs.h"
#include "McReady.h"
#include "dlgTools.h"
#include "device.h"

#include "compatibility.h"
#ifdef OLDPPC
#include "XCSoarProcess.h"
#else
#include "Process.h"
#endif

#include "McReady.h"
#include "Utils.h"
#include "InfoBoxLayout.h"
#include "Waypointparser.h"
#include "LKMapWindow.h"

static HFONT TempInfoWindowFont;
static HFONT TempTitleWindowFont;
static HFONT TempMapWindowFont;
static HFONT TempTitleSmallWindowFont;
static HFONT TempMapWindowBoldFont;
static HFONT TempCDIWindowFont; // New
static HFONT TempMapLabelFont;
static HFONT TempStatisticsFont;
static HFONT TempUseCustomFontsFont;

extern LOGFONT autoInfoWindowLogFont;
extern LOGFONT autoTitleWindowLogFont;
extern LOGFONT autoMapWindowLogFont;
extern LOGFONT autoTitleSmallWindowLogFont;
extern LOGFONT autoMapWindowBoldLogFont;
extern LOGFONT autoCDIWindowLogFont; // New
extern LOGFONT autoMapLabelLogFont;
extern LOGFONT autoStatisticsLogFont;

// #define ISPARAGLIDER (AircraftCategory == (AircraftCategory_t)umParaglider)

extern void InitializeOneFont (HFONT * theFont, 
                               const TCHAR FontRegKey[] , 
                               LOGFONT autoLogFont, 
                               LOGFONT * LogFontUsed);

extern bool dlgFontEditShowModal(const TCHAR * FontDescription, 
                          const TCHAR * FontRegKey, 
                          LOGFONT autoLogFont);
// default user level is now expert
int UserLevel = 1; 

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
static WndButton *buttonLoggerID=NULL;
static WndButton *buttonCopy=NULL;
static WndButton *buttonPaste=NULL;

#define NUMPAGES 25 		// ADDPAGE FIX HERE  27 as of 101126



static void UpdateButtons(void) {
  TCHAR text[120];
  TCHAR val[100];
  if (buttonPilotName) {
    GetRegistryString(szRegistryPilotName, val, 100);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _stprintf(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M524_ = "Pilot name" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M524_")), val);
    buttonPilotName->SetCaption(text);
  }
  if (buttonAircraftType) {
    GetRegistryString(szRegistryAircraftType, val, 100);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _stprintf(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M59_ = "Aircraft type" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M59_")), val);
    buttonAircraftType->SetCaption(text);
  }
  if (buttonAircraftRego) {
    GetRegistryString(szRegistryAircraftRego, val, 100);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _stprintf(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M57_ = "Aircraft Reg" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M57_")), val);
    buttonAircraftRego->SetCaption(text);
  }
  if (buttonCompetitionClass) {
    GetRegistryString(szRegistryCompetitionClass, val, 100);
    if (_tcslen(val)<=0) {
      // LKTOKEN  _@M7_ = "(blank)" 
      _stprintf(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M936_ = "Competition Class" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M936_")), val);
    buttonCompetitionClass->SetCaption(text);
  }
  if (buttonCompetitionID) {
    GetRegistryString(szRegistryCompetitionID, val, 100);
    if (_tcslen(val)<=0) {
      // LKTOKEN  _@M7_ = "(blank)" 
      _stprintf(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M938_ = "Competition ID" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M938_")), val);
    buttonCompetitionID->SetCaption(text);
  }
  if (buttonLoggerID) {
    GetRegistryString(szRegistryLoggerID, val, 100);
    if (_tcslen(val)<=0) {
	// LKTOKEN  _@M7_ = "(blank)" 
      _stprintf(val, gettext(TEXT("_@M7_")));
    }
	// LKTOKEN  _@M405_ = "Logger ID" 
    _stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M405_")), val);
    buttonLoggerID->SetCaption(text);
  }
}

extern bool EnableAnimation;


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
	// LKTOKEN  _@M16_ = "15 Task rules" 
    wf->SetCaption(gettext(TEXT("_@M16_")));
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

#if NOSIM
  if (!SIMMODE) {
	if ((devA() == NULL) || (_tcscmp(devA()->Name,TEXT("Vega")) != 0)) {
		return;
	}
  }
#else
#ifndef _SIM_
    if ((devA() == NULL) ||
	(_tcscmp(devA()->Name,TEXT("Vega")) != 0)) {
      return;
    }
#endif
#endif

#ifdef VEGAVOICE
    changed = dlgConfigurationVarioShowModal();
#endif
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
#if NOSIM
  if (!SIMMODE) {
	if ((devB() == NULL) || (_tcscmp(devB()->Name,TEXT("Vega")) != 0)) {
		return;
	}
  }
#else
#ifndef _SIM_
    if ((devB() == NULL) ||
	(_tcscmp(devB()->Name,TEXT("Vega")) != 0)) {
      return;
    }
#endif
#endif
#ifdef VEGAVOICE
    changed = dlgConfigurationVarioShowModal();
#endif

    // this is a hack to get the dialog to retain focus because
    // the progress dialog in the vario configuration somehow causes
    // focus problems
    wf->FocusNext(NULL);
}

static void UpdateDeviceSetupButton(int DeviceIdx, TCHAR *Name){

  WndButton *wb;

  if (DeviceIdx == 0){

    wb = ((WndButton *)wf->FindByName(TEXT("cmdSetupDeviceA")));
    if (wb != NULL) {
      if (_tcscmp(Name, TEXT("Vega")) == 0)
        wb->SetVisible(true);
      else 
        wb->SetVisible(false);
    }

  }

  if (DeviceIdx == 1){

    wb = ((WndButton *)wf->FindByName(TEXT("cmdSetupDeviceB")));
    if (wb != NULL) {
      if (_tcscmp(Name, TEXT("Vega")) == 0)
        wb->SetVisible(true);
      else 
        wb->SetVisible(false);
    }

  }

}


static void OnUserLevel(DataField *Sender, DataField::DataAccessKind_t Mode){
  WndProperty* wp;

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      wp = (WndProperty*)wf->FindByName(TEXT("prpUserLevel"));
      if (wp) {
        if (wp->GetDataField()->GetAsInteger() != UserLevel) {
          UserLevel = wp->GetDataField()->GetAsInteger();
          changed = true;
          SetToRegistry(szRegistryUserLevel,UserLevel);
          wf->FilterAdvanced(UserLevel>0);
        }
      }
    break;
	default: 
		StartupStore(_T("........... DBG-901%s"),NEWLINE); // 091105
		break;
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


static void ResetFonts(bool bUseCustom) {
// resest fonts when UseCustomFonts is turned off

  int UseCustomFontsold = UseCustomFonts;
  UseCustomFonts=bUseCustom;



  InitializeOneFont (&TempUseCustomFontsFont, 
                        TEXT("THIS FONT IS NOT CUSTOMIZABLE"), 
                        autoMapWindowLogFont,
                        NULL);



  InitializeOneFont (&TempInfoWindowFont, 
                        szRegistryFontInfoWindowFont, 
                        autoInfoWindowLogFont,
                        NULL);

  InitializeOneFont (&TempTitleWindowFont, 
                        szRegistryFontTitleWindowFont, 
                        autoTitleWindowLogFont,
                        NULL);

  InitializeOneFont (&TempMapWindowFont, 
                        szRegistryFontMapWindowFont, 
                        autoMapWindowLogFont,
                        NULL);

  InitializeOneFont (&TempTitleSmallWindowFont, 
                        szRegistryFontTitleSmallWindowFont, 
                        autoTitleSmallWindowLogFont,
                        NULL);

  InitializeOneFont (&TempMapWindowBoldFont, 
                        szRegistryFontMapWindowBoldFont, 
                        autoMapWindowBoldLogFont,
                        NULL);

  InitializeOneFont (&TempCDIWindowFont, 
                        szRegistryFontCDIWindowFont, 
                        autoCDIWindowLogFont,
                        NULL);

  InitializeOneFont (&TempMapLabelFont, 
                        szRegistryFontMapLabelFont, 
                        autoMapLabelLogFont,
                        NULL);

  InitializeOneFont (&TempStatisticsFont, 
                        szRegistryFontStatisticsFont, 
                        autoStatisticsLogFont,
                        NULL);

  UseCustomFonts=UseCustomFontsold;
}

static void ShowFontEditButtons(bool bVisible) {
  WndProperty * wp;
  wp = (WndProperty*)wf->FindByName(TEXT("cmdInfoWindowFont"));
  if (wp) {
    wp->SetVisible(bVisible);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("cmdTitleWindowFont"));
  if (wp) {
    wp->SetVisible(bVisible);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("cmdMapWindowFont"));
  if (wp) {
    wp->SetVisible(bVisible);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("cmdTitleSmallWindowFont"));
  if (wp) {
    wp->SetVisible(bVisible);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("cmdMapWindowBoldFont"));
  if (wp) {
    wp->SetVisible(bVisible);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("cmdCDIWindowFont"));
  if (wp) {
    wp->SetVisible(bVisible);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("cmdMapLabelFont"));
  if (wp) {
    wp->SetVisible(bVisible);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("cmdStatisticsFont"));
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
  wp = (WndProperty*)wf->FindByName(TEXT("prpInfoWindowFont"));
  if (wp) {
    wp->SetFont(TempInfoWindowFont);
    wp->SetVisible(false);
    wp->SetVisible(true);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTitleWindowFont"));
  if (wp) {
    wp->SetFont(TempTitleWindowFont);
    wp->SetVisible(false);
    wp->SetVisible(true);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpMapWindowFont"));
  if (wp) {
    wp->SetFont(TempMapWindowFont);
    wp->SetVisible(false);
    wp->SetVisible(true);
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpTitleSmallWindowFont"));
  if (wp) {
    wp->SetFont(TempTitleSmallWindowFont);
    wp->SetVisible(false);
    wp->SetVisible(true);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpMapWindowBoldFont"));
  if (wp) {
    wp->SetFont(TempMapWindowBoldFont);
    wp->SetVisible(false);
    wp->SetVisible(true);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpCDIWindowFont"));
  if (wp) {
    wp->SetFont(TempCDIWindowFont);
    wp->SetVisible(false);
    wp->SetVisible(true);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpMapLabelFont"));
  if (wp) {
    wp->SetFont(TempMapLabelFont);
    wp->SetVisible(false);
    wp->SetVisible(true);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpStatisticsFont"));
  if (wp) {
    wp->SetFont(TempStatisticsFont);
    wp->SetVisible(false);
    wp->SetVisible(true);
  }

  // now fix the rest of the dlgConfiguration fonts:
  wf->SetFont(TempMapWindowBoldFont);
  wf->SetTitleFont(TempMapWindowBoldFont);

  /*
  wp = (WndProperty*)wf->FindByName(TEXT("cmdNext"));
  if (wp) {
    wp->SetFont(TempCDIWindowFont);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("cmdPrev"));
  if (wp) {
    wp->SetFont(TempCDIWindowFont);
  }
  */

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

static void OnEditInfoWindowFontClicked(WindowControl *Sender) {
  // updates registry for font info and updates LogFont values
#define MAX_EDITFONT_DESC_LEN 100
  TCHAR FontDesc[MAX_EDITFONT_DESC_LEN];
  GetFontDescription(FontDesc, TEXT("prpInfoWindowFont"), MAX_EDITFONT_DESC_LEN);
  if (dlgFontEditShowModal(FontDesc,
                            szRegistryFontInfoWindowFont, 
                            autoInfoWindowLogFont)) {
    FontRegistryChanged=true;
    RefreshFonts(); 
  }
}

static void OnEditTitleWindowFontClicked(WindowControl *Sender) {
  TCHAR FontDesc[MAX_EDITFONT_DESC_LEN];
  GetFontDescription(FontDesc, TEXT("prpTitleWindowFont"), MAX_EDITFONT_DESC_LEN);
  if (dlgFontEditShowModal(FontDesc,
                            szRegistryFontTitleWindowFont, 
                            autoTitleWindowLogFont)) {
    FontRegistryChanged=true;
    RefreshFonts(); 
  }
}
static void OnEditMapWindowFontClicked(WindowControl *Sender) {
  TCHAR FontDesc[MAX_EDITFONT_DESC_LEN];
  GetFontDescription(FontDesc, TEXT("prpMapWindowFont"), MAX_EDITFONT_DESC_LEN);
  if (dlgFontEditShowModal(FontDesc,
                            szRegistryFontMapWindowFont, 
                            autoMapWindowLogFont)) {
    FontRegistryChanged=true;
    RefreshFonts(); 
  }
}
static void OnEditTitleSmallWindowFontClicked(WindowControl *Sender) {
  TCHAR FontDesc[MAX_EDITFONT_DESC_LEN];
  GetFontDescription(FontDesc, TEXT("prpTitleSmallWindowFont"), MAX_EDITFONT_DESC_LEN);
  if (dlgFontEditShowModal(FontDesc,
                            szRegistryFontTitleSmallWindowFont, 
                            autoTitleSmallWindowLogFont)) {
    FontRegistryChanged=true;
    RefreshFonts(); 
  }
}
static void OnEditMapWindowBoldFontClicked(WindowControl *Sender) {
  TCHAR FontDesc[MAX_EDITFONT_DESC_LEN];
  GetFontDescription(FontDesc, TEXT("prpMapWindowBoldFont"), MAX_EDITFONT_DESC_LEN);
  if (dlgFontEditShowModal(FontDesc,
                            szRegistryFontMapWindowBoldFont, 
                            autoMapWindowBoldLogFont)) {
    FontRegistryChanged=true;
    RefreshFonts(); 
  }
}
static void OnEditCDIWindowFontClicked(WindowControl *Sender) {
  TCHAR FontDesc[MAX_EDITFONT_DESC_LEN];
  GetFontDescription(FontDesc, TEXT("prpCDIWindowFont"), MAX_EDITFONT_DESC_LEN);
  if (dlgFontEditShowModal(FontDesc,
                            szRegistryFontCDIWindowFont, 
                            autoCDIWindowLogFont)) {
    FontRegistryChanged=true;
    RefreshFonts(); 
  }
}
static void OnEditMapLabelFontClicked(WindowControl *Sender) {
  TCHAR FontDesc[MAX_EDITFONT_DESC_LEN];
  GetFontDescription(FontDesc, TEXT("prpMapLabelFont"), MAX_EDITFONT_DESC_LEN);
  if (dlgFontEditShowModal(FontDesc,
                            szRegistryFontMapLabelFont, 
                            autoMapLabelLogFont)) {
    FontRegistryChanged=true;
    RefreshFonts(); 
  }
}
static void OnEditStatisticsFontClicked(WindowControl *Sender) {
  TCHAR FontDesc[MAX_EDITFONT_DESC_LEN];
  GetFontDescription(FontDesc, TEXT("prpStatisticsFont"), MAX_EDITFONT_DESC_LEN);
  if (dlgFontEditShowModal(FontDesc,
                            szRegistryFontStatisticsFont, 
                            autoStatisticsLogFont)) {
    FontRegistryChanged=true;
    RefreshFonts(); 
  }
}


static void OnAircraftRegoClicked(WindowControl *Sender) {
	(void)Sender;
  TCHAR Temp[100];
  if (buttonAircraftRego) {
    GetRegistryString(szRegistryAircraftRego,Temp,100);
    dlgTextEntryShowModal(Temp,100);
    SetRegistryString(szRegistryAircraftRego,Temp);
    changed = true;
  }
  UpdateButtons();
}


static void OnAircraftTypeClicked(WindowControl *Sender) {
	(void)Sender;
  TCHAR Temp[100];
  if (buttonAircraftType) {
    GetRegistryString(szRegistryAircraftType,Temp,100);
    dlgTextEntryShowModal(Temp,100);
    SetRegistryString(szRegistryAircraftType,Temp);
    changed = true;
  }
  UpdateButtons();
}


static void OnPilotNameClicked(WindowControl *Sender) {
	(void)Sender;
  TCHAR Temp[100];
  if (buttonPilotName) {
    GetRegistryString(szRegistryPilotName,Temp,100);
    dlgTextEntryShowModal(Temp,100);
    SetRegistryString(szRegistryPilotName,Temp);
    changed = true;
  }
  UpdateButtons();
}


static void OnCompetitionClassClicked(WindowControl *Sender)
{
  TCHAR Temp[100];
  if (buttonCompetitionClass) {
    GetRegistryString(szRegistryCompetitionClass,Temp,100);
    dlgTextEntryShowModal(Temp,100);
    SetRegistryString(szRegistryCompetitionClass,Temp);
    changed = true;
  }
  UpdateButtons();
}


static void OnCompetitionIDClicked(WindowControl *Sender)
{
  TCHAR Temp[100];
  if (buttonCompetitionID) {
    GetRegistryString(szRegistryCompetitionID,Temp,100);
    dlgTextEntryShowModal(Temp,100);
    SetRegistryString(szRegistryCompetitionID,Temp);
    changed = true;
  }
  UpdateButtons();
}


static void OnLoggerIDClicked(WindowControl *Sender) {
	(void)Sender;
  TCHAR Temp[100];
  if (buttonLoggerID) {
    GetRegistryString(szRegistryLoggerID,Temp,100);
    dlgTextEntryShowModal(Temp,100);
    SetRegistryString(szRegistryLoggerID,Temp);
    changed = true;
    ReadAssetNumber();
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

#if LKTOPO
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
#endif
static void OnSetCustomKeysClicked(WindowControl * Sender){
	(void)Sender;
  dlgCustomKeysShowModal();
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

  for (int item=0; item<9; item++) {
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

    for (int item=0; item<9; item++) {
      InfoBoxPropName(name, item, mode);
      WndProperty *wp;
      wp = (WndProperty*)wf->FindByName(name);
      if (wp && (cpyInfoBox[item]>=0)&&(cpyInfoBox[item]<NUMSELECTSTRINGS)) {
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

static int lastSelectedPolarFile = -1;

static void OnPolarFileData(DataField *Sender, DataField::DataAccessKind_t Mode){
  WndProperty* wp;

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      if (Sender->GetAsString() != NULL && _tcscmp(Sender->GetAsString(), TEXT("")) != 0){
        // then ... set Polar Tape to Winpilot

        wp = (WndProperty *)wf->FindByName(TEXT("prpPolarType"));

        if (wp != NULL){
          wp->GetDataField()->SetAsInteger(POLARUSEWINPILOTFILE);
          wp->RefreshDisplay();
        }

      }
    break;
	default: 
		StartupStore(_T("........... DBG-907%s"),NEWLINE); // 091105
		break;
  }

}


static void OnPolarTypeData(DataField *Sender, DataField::DataAccessKind_t Mode){
  WndProperty* wp;

  switch(Mode){
    case DataField::daGet:
    break;
    case DataField::daPut:
    case DataField::daChange:
      wp = (WndProperty *)wf->FindByName(TEXT("prpPolarFile"));

      if (Sender->GetAsInteger() != POLARUSEWINPILOTFILE){
        // then ... clear Winpilot File if Polar Type is not WinpilotFile

        if (wp != NULL && wp->GetDataField()->GetAsInteger() > 0){
          lastSelectedPolarFile = wp->GetDataField()->GetAsInteger();
          wp->GetDataField()->SetAsInteger(-1);
          wp->RefreshDisplay();
        }

      } else {
        if (wp != NULL && wp->GetDataField()->GetAsInteger() <= 0 && lastSelectedPolarFile > 0){
          wp->GetDataField()->SetAsInteger(lastSelectedPolarFile);
          wp->RefreshDisplay();
        }

      }
    break;
	default: 
		StartupStore(_T("........... DBG-906%s"),NEWLINE); // 091105
		break;
  }

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
  #if CUPCOM
  edit_waypoint.Comment=(TCHAR*)malloc(100*sizeof(TCHAR)); //@ bugfix 101110
  _tcscpy(edit_waypoint.Comment,_T(""));
  #else
  edit_waypoint.Comment[0] = 0;
  #endif
  edit_waypoint.Name[0] = 0;
  edit_waypoint.Details = 0;
  edit_waypoint.Number = NumberOfWayPoints;
#ifdef CUPSUP
  edit_waypoint.Format = LKW_NEW;	// 100208
  edit_waypoint.RunwayLen = 0;
  edit_waypoint.RunwayDir = -1;
  edit_waypoint.Style = 1; // normal turnpoint
  edit_waypoint.Code[0]=0;
  edit_waypoint.Freq[0]=0;
  edit_waypoint.Country[0]=0;
#endif 
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
	#ifdef CUPSUP
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

	#endif

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
	#ifdef CUPSUP
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
	#endif
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

  DeclareCallBackEntry(OnPolarFileData),
  DeclareCallBackEntry(OnPolarTypeData),

  DeclareCallBackEntry(OnDeviceAData),
  DeclareCallBackEntry(OnDeviceBData),

  DeclareCallBackEntry(OnUseCustomFontData),
  DeclareCallBackEntry(OnEditInfoWindowFontClicked),
  DeclareCallBackEntry(OnEditTitleWindowFontClicked),
  DeclareCallBackEntry(OnEditMapWindowFontClicked),
  DeclareCallBackEntry(OnEditTitleSmallWindowFontClicked),
  DeclareCallBackEntry(OnEditMapWindowBoldFontClicked),
  DeclareCallBackEntry(OnEditCDIWindowFontClicked),
  DeclareCallBackEntry(OnEditMapLabelFontClicked),
  DeclareCallBackEntry(OnEditStatisticsFontClicked),

  DeclareCallBackEntry(OnUserLevel),
  #if LKTOPO
  DeclareCallBackEntry(OnSetTopologyClicked),
  #endif
  DeclareCallBackEntry(OnSetCustomKeysClicked),
  DeclareCallBackEntry(NULL)
};


extern SCREEN_INFO Data_Options[];
extern int InfoType[];



static void SetInfoBoxSelector(int item, int mode)
{
  TCHAR name[80];
  InfoBoxPropName(name, item, mode);

  WndProperty *wp;
  wp = (WndProperty*)wf->FindByName(name);
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    for (int i=0; i<NUMSELECTSTRINGS; i++) {
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
      StoreType(item,InfoType[item]);
    }
  }
}

extern const TCHAR *PolarLabels[];

static  TCHAR szPolarFile[MAX_PATH] = TEXT("\0");
static  TCHAR szAirspaceFile[MAX_PATH] = TEXT("\0");
static  TCHAR szAdditionalAirspaceFile[MAX_PATH] = TEXT("\0");
static  TCHAR szWaypointFile[MAX_PATH] = TEXT("\0");
static  TCHAR szAdditionalWaypointFile[MAX_PATH] = TEXT("\0");
static  TCHAR szTerrainFile[MAX_PATH] = TEXT("\0");
static  TCHAR szTopologyFile[MAX_PATH] = TEXT("\0");
static  TCHAR szAirfieldFile[MAX_PATH] = TEXT("\0");
static  TCHAR szLanguageFile[MAX_PATH] = TEXT("\0");
static  TCHAR szStatusFile[MAX_PATH] = TEXT("\0");
static  TCHAR szInputFile[MAX_PATH] = TEXT("\0");
static  TCHAR szMapFile[MAX_PATH] = TEXT("\0");
static  DWORD dwPortIndex1 = 0;
static  DWORD dwSpeedIndex1 = 2;
static  DWORD dwBit1Index = (BitIndex_t)bit8N1;
static  DWORD dwPortIndex2 = 0;
static  DWORD dwSpeedIndex2 = 2;
static  DWORD dwBit2Index = (BitIndex_t)bit8N1;
static  int dwDeviceIndex1=0;
static  int dwDeviceIndex2=0;
static  TCHAR DeviceName[DEVNAMESIZE+1];
static  DWORD Speed = 2; // default is kmh 100219
static  DWORD TaskSpeed = 2; // default is kph
static  DWORD Distance = 2; // default is km
static  DWORD Lift = 1; // default m/s
static  DWORD Altitude = 1; //default m
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
  buttonLoggerID = ((WndButton *)wf->FindByName(TEXT("cmdLoggerID")));
  if (buttonLoggerID) {
    buttonLoggerID->SetOnClickNotify(OnLoggerIDClicked);
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


  wp = (WndProperty*)wf->FindByName(TEXT("prpUserLevel"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M131_ = "Basic" 
    dfe->addEnumText(gettext(TEXT("_@M131_")));
	// LKTOKEN  _@M271_ = "Expert" 
    dfe->addEnumText(gettext(TEXT("_@M271_")));
    dfe->Set(UserLevel);
    wp->RefreshDisplay();
  }

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
    dfe->Sort(1);
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
    dfe->Sort(1);
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
    dfe->Set(AltitudeMode);
    wp->RefreshDisplay();
  }
  // 
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
    wp->GetDataField()->Set(MapWindow::zoom.AutoZoom());
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
	// LKTOKEN  _@M491_ = "OFF" 
    dfe->addEnumText(gettext(TEXT("_@M491_")));
	// LKTOKEN  _@M496_ = "ON/Fixed" 
    dfe->addEnumText(gettext(TEXT("_@M496_")));
	// LKTOKEN  _@M497_ = "ON/Scaled" 
    dfe->addEnumText(gettext(TEXT("_@M497_")));
    dfe->Set(EnableFLARMMap);
    wp->RefreshDisplay();
  }

  #if 0 
  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableFLARMGauge"));
  if (wp) {
    wp->GetDataField()->Set(EnableFLARMGauge);
    wp->RefreshDisplay();
  }
  #endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceWarnings"));
  if (wp) {
    bool aw = AIRSPACEWARNINGS != 0;
    wp->GetDataField()->Set(aw);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(WarningTime);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAcknowledgementTime"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(AcknowledgementTime);
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
    wp->GetDataField()->Set(EnableTerrain);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableTopology"));
  if (wp) {
    wp->GetDataField()->Set(EnableTopology);
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
    #if AUTORIENT
    dfe->Set(OldDisplayOrientation);
    #else
    dfe->Set(DisplayOrientation);
    #endif
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMenuTimeout"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(MenuTimeoutMax/2);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeArrival"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(ALTITUDEMODIFY*SAFETYALTITUDEARRIVAL));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeBreakoff"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(ALTITUDEMODIFY*SAFETYALTITUDEBREAKOFF));
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
	// LKTOKEN  _@M491_ = "OFF" 
    dfe->addEnumText(gettext(TEXT("_@M491_")));
	// LKTOKEN  _@M393_ = "Line" 
    dfe->addEnumText(gettext(TEXT("_@M393_")));
	// LKTOKEN  _@M609_ = "Shade" 
    dfe->addEnumText(gettext(TEXT("_@M609_")));
    dfe->Set(FinalGlideTerrain);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableNavBaroAltitude"));
  if (wp) {
    wp->GetDataField()->Set(EnableNavBaroAltitude);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpOrbiter"));
  if (wp) {
    wp->GetDataField()->Set(Orbiter);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpShading"));
  if (wp) {
    wp->GetDataField()->Set(Shading);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWindArrowStyle"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M99_ = "Arrow head" 
    dfe->addEnumText(gettext(TEXT("_@M99_")));
	// LKTOKEN  _@M310_ = "Full arrow" 
    dfe->addEnumText(gettext(TEXT("_@M310_")));
    wp->GetDataField()->Set(MapWindow::WindArrowStyle);
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
    wp->GetDataField()->Set(AutoWindMode);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoMcMode"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M290_ = "Final glide" 
    dfe->addEnumText(gettext(TEXT("_@M290_")));
	// LKTOKEN  _@M118_ = "Average climb" 
    dfe->addEnumText(gettext(TEXT("_@M118_")));
	// LKTOKEN  _@M148_ = "Both (Fin+Ave)" 
    dfe->addEnumText(gettext(TEXT("_@M148_")));
    #if EQMC
	// LKTOKEN  _@M262_ = "Equivalent MC" 
    dfe->addEnumText(gettext(TEXT("_@M262_")));
    #endif
    wp->GetDataField()->Set(AutoMcMode);
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpBlockSTF"));
  if (wp) {
    wp->GetDataField()->Set(EnableBlockSTF);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFAIFinishHeight"));
  if (wp) {
    wp->GetDataField()->Set(EnableFAIFinishHeight);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOLCRules"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M633_ = "Sprint" 
    dfe->addEnumText(gettext(TEXT("_@M633_")));
	// LKTOKEN  _@M742_ = "Triangle" 
    dfe->addEnumText(gettext(TEXT("_@M742_")));
	// LKTOKEN  _@M176_ = "Classic" 
    dfe->addEnumText(gettext(TEXT("_@M176_")));
    dfe->Set(OLCRules);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpHandicap"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(Handicap);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPGClimbZoom"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M438_ = "More" 
    dfe->addEnumText(gettext(TEXT("_@M438_")));
	// LKTOKEN  _@M634_ = "Standard" 
    dfe->addEnumText(gettext(TEXT("_@M634_")));
	// LKTOKEN  _@M389_ = "Less" 
    dfe->addEnumText(gettext(TEXT("_@M389_")));
	// LKTOKEN  _@M415_ = "Lower" 
    dfe->addEnumText(gettext(TEXT("_@M415_")));
	// LKTOKEN  _@M342_ = "Higher" 
    dfe->addEnumText(gettext(TEXT("_@M342_")));
    dfe->Set(PGClimbZoom);
    // if (!ISPARAGLIDER) wp->SetVisible(false); 
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGCruiseZoom"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(PGCruiseZoom);
    // if (!ISPARAGLIDER) wp->SetVisible(false); 
    wp->RefreshDisplay();
  }
  #if AUTORIENT
  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoOrientScale"));
  if (wp) {
    wp->GetDataField()->SetAsInteger(AutoOrientScale);
    wp->RefreshDisplay();
  }
  #endif

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
  

  if(GetFromRegistry(szRegistrySpeedUnitsValue,&Speed)!=ERROR_SUCCESS) {
    SetToRegistry(szRegistrySpeedUnitsValue, Speed);
    changed = true;
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
    dfe->Set(Speed);
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

  if(GetFromRegistry(szRegistryTaskSpeedUnitsValue,&TaskSpeed)!=ERROR_SUCCESS) {
    SetToRegistry(szRegistryTaskSpeedUnitsValue, TaskSpeed);
    changed = true;
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
    dfe->Set(TaskSpeed);
    wp->RefreshDisplay();
  }

  if(GetFromRegistry(szRegistryDistanceUnitsValue,&Distance)!=ERROR_SUCCESS) {
    SetToRegistry(szRegistryDistanceUnitsValue, Distance);
    changed = true;
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
    dfe->Set(Distance);
    wp->RefreshDisplay();
  }

  if(GetFromRegistry(szRegistryAltitudeUnitsValue,&Altitude)!=ERROR_SUCCESS) {
    SetToRegistry(szRegistryAltitudeUnitsValue, Altitude);
    changed = true;
  } 
  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsAltitude"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("feet")));
    dfe->addEnumText(gettext(TEXT("meters")));
    dfe->Set(Altitude);
    wp->RefreshDisplay();
  }

  if(GetFromRegistry(szRegistryLiftUnitsValue,&Lift)!=ERROR_SUCCESS) {
    SetToRegistry(szRegistryLiftUnitsValue, Lift);
    changed = true;
  } 
  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLift"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("knots")));
    dfe->addEnumText(gettext(TEXT("m/s")));
    dfe->Set(Lift);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTrailDrift"));
  if (wp) {
    wp->GetDataField()->Set(MapWindow::EnableTrailDrift);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpThermalLocator"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M491_ = "OFF" 
    dfe->addEnumText(gettext(TEXT("_@M491_")));
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


  wp = (WndProperty*)wf->FindByName(TEXT("prpAbortSafetyUseCurrent"));
  if (wp) {
    wp->GetDataField()->Set(GlidePolar::AbortSafetyUseCurrent);
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpRiskGamma"));
  if (wp) {
    wp->GetDataField()->Set(GlidePolar::RiskGamma);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAnimation"));
  if (wp) {
    wp->GetDataField()->Set(EnableAnimation);
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
    dfe->Set(TrailActive);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKMaxLabels"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(LKMaxLabels);
    wp->RefreshDisplay();
  }

  // VENTA3 VisualGlide
  wp = (WndProperty*)wf->FindByName(TEXT("prpVGlide"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M499_ = "Off" 
    dfe->addEnumText(gettext(TEXT("_@M499_")));
	// LKTOKEN  _@M668_ = "Steady" 
    dfe->addEnumText(gettext(TEXT("_@M668_")));
	// LKTOKEN  _@M445_ = "Moving" 
    dfe->addEnumText(gettext(TEXT("_@M445_")));
    dfe->Set(VisualGlide);
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
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPolarType"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    for (i=0; i<NUMPOLARS; i++) {
      dfe->addEnumText(PolarLabels[i]);
    }
    i=0;
    bool ok = true;
    while (ok) {
      TCHAR *name;
      name = GetWinPilotPolarInternalName(i);
      if (!name) {
	ok=false;
      } else {
	dfe->addEnumText(name);
      }
      i++;
    }
    dfe->Sort();
    dfe->Set(POLARID);
    wp->RefreshDisplay();
  }

  GetRegistryString(szRegistryPolarFile, szPolarFile, MAX_PATH);
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

  GetRegistryString(szRegistryAirspaceFile, szAirspaceFile, MAX_PATH);
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

  GetRegistryString(szRegistryAdditionalAirspaceFile, 
		    szAdditionalAirspaceFile, MAX_PATH);
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

  GetRegistryString(szRegistryWayPointFile, szWaypointFile, MAX_PATH);
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
#ifdef CUPSUP
    _stprintf(tsuf,_T("*%S"),LKS_WP_CUP);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%S"),LKS_WP_COMPE);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
#endif
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  GetRegistryString(szRegistryAdditionalWayPointFile, 
		    szAdditionalWaypointFile, MAX_PATH);
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
#ifdef CUPSUP
    _stprintf(tsuf,_T("*%S"),LKS_WP_CUP);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
    _stprintf(tsuf,_T("*%S"),LKS_WP_COMPE);
    dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS),tsuf);
#endif
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  GetRegistryString(szRegistryMapFile, szMapFile, MAX_PATH);
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

  GetRegistryString(szRegistryTerrainFile, szTerrainFile, MAX_PATH);
  _tcscpy(temptext,szTerrainFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpTerrainFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_TERRAINDEM);
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
    _stprintf(tsuf,_T("*%S"),LKS_TERRAINDAT);
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
    _stprintf(tsuf,_T("*%S"),LKS_TERRAINJP2);
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  GetRegistryString(szRegistryTopologyFile, szTopologyFile, MAX_PATH);
  _tcscpy(temptext,szTopologyFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpTopologyFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_TOPOLOGY);
    dfe->ScanDirectoryTop(_T(LKD_MAPS),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  GetRegistryString(szRegistryAirfieldFile, szAirfieldFile, MAX_PATH);
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

  GetRegistryString(szRegistryLanguageFile, szLanguageFile, MAX_PATH);
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

  GetRegistryString(szRegistryStatusFile, szStatusFile, MAX_PATH);
  _tcscpy(temptext,szStatusFile);
  ExpandLocalPath(temptext);
  wp = (WndProperty*)wf->FindByName(TEXT("prpStatusFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(tsuf,_T("*%S"),LKS_STATUS);
    dfe->ScanDirectoryTop(_T(LKD_CONF),tsuf);
    dfe->Lookup(temptext);
    wp->RefreshDisplay();
  }

  GetRegistryString(szRegistryInputFile, szInputFile, MAX_PATH);
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppStatusMessageAlignment"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M167_ = "Center" 
    dfe->addEnumText(gettext(TEXT("_@M167_")));
	// LKTOKEN  _@M730_ = "Topleft" 
    dfe->addEnumText(gettext(TEXT("_@M730_")));
    dfe->Set(Appearance.StateMessageAlligne);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTextInput"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M341_ = "HighScore Style" 
    dfe->addEnumText(gettext(TEXT("_@M341_")));
	// LKTOKEN  _@M370_ = "Keyboard" 
    dfe->addEnumText(gettext(TEXT("_@M370_")));
	dfe->Set(Appearance.TextInputStyle);
    wp->RefreshDisplay();
  }



  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxBorder"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M151_ = "Box" 
    dfe->addEnumText(gettext(TEXT("_@M151_")));
	// LKTOKEN  _@M679_ = "Tab" 
    dfe->addEnumText(gettext(TEXT("_@M679_")));
    dfe->Set(Appearance.InfoBoxBorder);
    wp->RefreshDisplay();
  }

#if defined(PNA) || defined(FIVV)
// VENTA-ADDON Geometry change config menu 11
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxGeom"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();

    if (InfoBoxLayout::landscape) {

	
	dfe->addEnumText(TEXT("vario+9box")); // 0
	dfe->addEnumText(TEXT("(empty) A"));  // 1 
	dfe->addEnumText(TEXT("(empty) B"));  // 2 
	dfe->addEnumText(TEXT("(empty) C"));  // 3
	dfe->addEnumText(TEXT("8box left"));  // 4
	dfe->addEnumText(TEXT("8box right")); // 5
	dfe->addEnumText(TEXT("(empty) D"));  // 6
	dfe->addEnumText(TEXT("5box right")); // 7
	dfe->Set(Appearance.InfoBoxGeom);
	wp->RefreshDisplay();

    } else { 
	dfe->addEnumText(TEXT("top+bottom")); // 0
	dfe->addEnumText(TEXT("bottom"));     // 1
	dfe->addEnumText(TEXT("top"));        // 2 
	dfe->addEnumText(TEXT("3 free")); // 3
	dfe->addEnumText(TEXT("4 free"));       // 4
	dfe->addEnumText(TEXT("5 free"));      // 5
	dfe->addEnumText(TEXT("6 free"));      // 6
	dfe->addEnumText(TEXT("7"));          // 7
	dfe->Set(Appearance.InfoBoxGeom);
	wp->RefreshDisplay();

    }
  }
//
#endif

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
		default:
				iTmp=(InfoBoxModelAppearance_t)apImPnaGeneric;
				break;
	}

    dfe->Set(iTmp);
    wp->RefreshDisplay();
  }
#elif defined FIVV
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxModel"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
// VENTA2- PC model
#if (WINDOWSPC>0)
	// LKTOKEN  _@M511_ = "PC/normal" 
    dfe->addEnumText(gettext(TEXT("_@M511_")));
    #ifdef FIVV
   	 wp->SetVisible(true); // no more gaps in menus
    #else
   	 wp->SetVisible(false); // currently no need to display default
    #endif
#else
	// LKTOKEN  _@M512_ = "PDA/normal" 
    dfe->addEnumText(gettext(TEXT("_@M512_")));
    #ifdef FIVV
    wp->SetVisible(true);
    #else
    wp->SetVisible(false);
    #endif
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpExtendedVisualGlide"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M480_ = "Normal" 
    dfe->addEnumText(gettext(TEXT("_@M480_")));
	// LKTOKEN  _@M273_ = "Extended" 
    dfe->addEnumText(gettext(TEXT("_@M273_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(ExtendedVisualGlide);
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpNewMap"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(NewMap);
    wp->RefreshDisplay();
  }
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
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(LKVarioBar);
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpVirtualKeys"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(VirtualKeys);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUseMapLock"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M239_ = "Disabled" 
    dfe->addEnumText(gettext(TEXT("_@M239_")));
	// LKTOKEN  _@M259_ = "Enabled" 
    dfe->addEnumText(gettext(TEXT("_@M259_")));
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->Set(UseMapLock);
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
    dfe->Set(ActiveMap);
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
  wp = (WndProperty*)wf->FindByName(TEXT("prpOverlayClock"));
  if (wp) {
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
	// LKTOKEN  _@M17_ = "15 seconds" 
    dfe->addEnumText(gettext(TEXT("_@M17_")));
	// LKTOKEN  _@M30_ = "30 seconds" 
    dfe->addEnumText(gettext(TEXT("_@M30_")));
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
    dfe->Set(BgMapColor);
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppCompassAppearance"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M480_ = "Normal" 
    dfe->addEnumText(gettext(TEXT("_@M480_")));
	// LKTOKEN  _@M815_ = "White outline" 
    dfe->addEnumText(gettext(TEXT("_@M815_")));
    dfe->Set(Appearance.CompassAppearance);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppIndFinalGlide"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M227_ = "Default" 
    dfe->addEnumText(gettext(TEXT("_@M227_")));
	// LKTOKEN  _@M87_ = "Alternate" 
    dfe->addEnumText(gettext(TEXT("_@M87_")));
    dfe->Set(Appearance.IndFinalGlide);
    wp->RefreshDisplay();
  }

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

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableExternalTriggerCruise"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M491_ = "OFF" 
    dfe->addEnumText(gettext(TEXT("_@M491_")));
    dfe->addEnumText(gettext(TEXT("Flap")));
    dfe->addEnumText(gettext(TEXT("SC")));
    dfe->Set(EnableExternalTriggerCruise);
    wp->RefreshDisplay();
  }

  #if 100922
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInverseInfoBox"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M894_ = "ON" 
    dfe->addEnumText(gettext(TEXT("_@M894_")));
	// LKTOKEN  _@M491_ = "OFF" 
    dfe->addEnumText(gettext(TEXT("_@M491_")));
    dfe->Set(Appearance.InverseInfoBox);
    wp->RefreshDisplay();
  }
  #else
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInverseInfoBox"));
  if (wp) {
    wp->GetDataField()->Set(Appearance.InverseInfoBox);
    wp->RefreshDisplay();
  }
  #endif

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
    dfe->Set(TerrainRamp);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxColors"));
  if (wp) {
    wp->GetDataField()->Set(Appearance.InfoBoxColors);
    wp->RefreshDisplay();
  }

  #if !110101
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppAveNeedle"));
  if (wp) {
    wp->GetDataField()->Set(Appearance.GaugeVarioAveNeedle);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioSpeedToFly"));
  if (wp) {
    wp->GetDataField()->Set(Appearance.GaugeVarioSpeedToFly);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioAvgText"));
  if (wp) {
    wp->GetDataField()->Set(Appearance.GaugeVarioAvgText);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioGross"));
  if (wp) {
    wp->GetDataField()->Set(Appearance.GaugeVarioGross);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioMc"));
  if (wp) {
    wp->GetDataField()->Set(Appearance.GaugeVarioMc);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioBugs"));
  if (wp) {
    wp->GetDataField()->Set(Appearance.GaugeVarioBugs);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioBallast"));
  if (wp) {
    wp->GetDataField()->Set(Appearance.GaugeVarioBallast);
    wp->RefreshDisplay();
  }
  #endif // REMOVABLE GAUGEVARIO

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoBlank"));
  if (wp) {
#ifdef GNAV
    wp->SetVisible(false);
#endif
#if (WINDOWSPC>0)
    wp->SetVisible(false);
#endif
    wp->GetDataField()->Set(EnableAutoBlank);
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
    dfe->Set(AutoAdvance);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFinishMinHeight"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(FinishMinHeight*ALTITUDEMODIFY/1000)); // BUGFIX XCSOAR 100315
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxHeight"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(StartMaxHeight*ALTITUDEMODIFY/1000)); // BUGFIX XCSOAR 100315
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxHeightMargin"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(StartMaxHeightMargin*ALTITUDEMODIFY/1000));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartHeightRef"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(gettext(TEXT("AGL")));
    dfe->addEnumText(gettext(TEXT("MSL")));
    dfe->Set(StartHeightRef);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxSpeed"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(StartMaxSpeed*SPEEDMODIFY/1000)); 
    wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxSpeedMargin"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(iround(StartMaxSpeedMargin*SPEEDMODIFY/1000)); 
    wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
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

  #if 0
  wp = (WndProperty*)wf->FindByName(TEXT("prpShortcutIbox"));
  if (wp) {
    wp->GetDataField()->Set(ShortcutIbox);
    wp->RefreshDisplay();
  }
  #endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpEngineeringMenu")); 
  if (wp) {
    wp->GetDataField()->Set(EngineeringMenu);
    wp->RefreshDisplay();
  }

  for (i=0; i<4; i++) {
    for (int j=0; j<9; j++) {
      SetInfoBoxSelector(j, i);
    }
  }
}




void dlgConfigurationShowModal(void){

  WndProperty *wp;

  StartHourglassCursor(); 

  if (!InfoBoxLayout::landscape) {
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
  wConfig15    = ((WndFrame *)wf->FindByName(TEXT("frmTaskRules")));
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
  // ADDPAGE HERE
  ASSERT(wConfig1!=NULL);
  ASSERT(wConfig2!=NULL);
  ASSERT(wConfig3!=NULL);
  ASSERT(wConfig4!=NULL);
  ASSERT(wConfig5!=NULL);
  ASSERT(wConfig6!=NULL);
  ASSERT(wConfig7!=NULL);
  ASSERT(wConfig8!=NULL);
  ASSERT(wConfig9!=NULL);
  ASSERT(wConfig10!=NULL);
  ASSERT(wConfig11!=NULL);
  ASSERT(wConfig12!=NULL);
  ASSERT(wConfig13!=NULL);
  ASSERT(wConfig14!=NULL);
  ASSERT(wConfig15!=NULL);
  ASSERT(wConfig16!=NULL);
  ASSERT(wConfig17!=NULL);
  ASSERT(wConfig18!=NULL);
  ASSERT(wConfig19!=NULL);
  ASSERT(wConfig20!=NULL);
  ASSERT(wConfig21!=NULL);
  ASSERT(wConfig22!=NULL);
  ASSERT(wConfig23!=NULL);
  ASSERT(wConfig24!=NULL);
  ASSERT(wConfig25!=NULL);
  //ASSERT(wConfig26!=NULL);
  //ASSERT(wConfig27!=NULL);
  // ADDPAGE HERE

  wf->FilterAdvanced(UserLevel>0);


#if !defined(PNA) && !defined(FIVV)
  // JMW we don't want these for non-PDA platforms yet
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxGeom"));
  if (wp) {
    wp->SetVisible(false);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxModel"));
  if (wp) {
    wp->SetVisible(false);
  }
#endif

  for (int item=0; item<10; item++) {
    cpyInfoBox[item] = -1;
  }

  setVariables();

  UpdateDeviceSetupButton(0, devA()->Name);
  UpdateDeviceSetupButton(1, devB()->Name);

  NextPage(0); // just to turn proper pages on/off

  changed = false;
  taskchanged = false;
  requirerestart = false;
  utcchanged = false;
  waypointneedsave = false;

  StopHourglassCursor();
  wf->ShowModal();

  // TODO enhancement: implement a cancel button that skips all this
  // below after exit.

  wp = (WndProperty*)wf->FindByName(TEXT("prpAbortSafetyUseCurrent"));
  if (wp) {
    if (GlidePolar::AbortSafetyUseCurrent
	!= wp->GetDataField()->GetAsBoolean()) {
      GlidePolar::AbortSafetyUseCurrent = 
	wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryAbortSafetyUseCurrent, 
		    GlidePolar::AbortSafetyUseCurrent);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDisableAutoLogger"));
  if (wp) {
    if (!DisableAutoLogger
	!= wp->GetDataField()->GetAsBoolean()) {
      DisableAutoLogger = 
	!(wp->GetDataField()->GetAsBoolean());
      SetToRegistry(szRegistryDisableAutoLogger, 
		    DisableAutoLogger);
      changed = true;
    }
  }

  double val;

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyMacCready"));
  if (wp) {
    val = wp->GetDataField()->GetAsFloat()/LIFTMODIFY;
    if (GlidePolar::SafetyMacCready != val) {
      GlidePolar::SafetyMacCready = val;
      SetToRegistry(szRegistrySafetyMacCready, 
		    iround(GlidePolar::SafetyMacCready*10));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRiskGamma"));
  if (wp) {
    val = wp->GetDataField()->GetAsFloat();
    if (GlidePolar::RiskGamma != val) {
      GlidePolar::RiskGamma = val;
      SetToRegistry(szRegistryRiskGamma, 
		    iround(GlidePolar::RiskGamma*10));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSetSystemTimeFromGPS"));
  if (wp) {
    if (SetSystemTimeFromGPS != wp->GetDataField()->GetAsBoolean()) {
      SetSystemTimeFromGPS = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistrySetSystemTimeFromGPS, SetSystemTimeFromGPS);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAnimation"));
  if (wp) {
    if (EnableAnimation != wp->GetDataField()->GetAsBoolean()) {
      EnableAnimation = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryAnimation, EnableAnimation);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTrailDrift"));
  if (wp) {
    if (MapWindow::EnableTrailDrift != wp->GetDataField()->GetAsBoolean()) {
      MapWindow::EnableTrailDrift = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryTrailDrift, MapWindow::EnableTrailDrift);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpThermalLocator"));
  if (wp) {
    if (EnableThermalLocator != wp->GetDataField()->GetAsInteger()) {
      EnableThermalLocator = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryThermalLocator, EnableThermalLocator);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTrail"));
  if (wp) {
    if (TrailActive != wp->GetDataField()->GetAsInteger()) {
      TrailActive = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistrySnailTrail, TrailActive);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLKMaxLabels"));
  if (wp) {
    if (LKMaxLabels != wp->GetDataField()->GetAsInteger()) {
      LKMaxLabels = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryLKMaxLabels, LKMaxLabels);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPGCruiseZoom"));
  if (wp) {
    if ( PGCruiseZoom != wp->GetDataField()->GetAsInteger()) {
      PGCruiseZoom = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGCruiseZoom, PGCruiseZoom);
      changed = true;
      MapWindow::zoom.Reset();
      requirerestart=true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGClimbZoom"));
  if (wp) {
    if ( PGClimbZoom != wp->GetDataField()->GetAsInteger()) {
      PGClimbZoom = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGClimbZoom, PGClimbZoom);
      changed = true;
      MapWindow::zoom.Reset();
      requirerestart=true; 
    }
  }

  #if AUTORIENT
  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoOrientScale"));
  if (wp) {
    if ( AutoOrientScale != wp->GetDataField()->GetAsInteger()) {
      AutoOrientScale = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAutoOrientScale, AutoOrientScale);
      changed = true;
    }
  }
  #endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpPGNumberOfGates"));
  if (wp) {
    if ( PGNumberOfGates != wp->GetDataField()->GetAsInteger()) {
      PGNumberOfGates = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGNumberOfGates, PGNumberOfGates);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOpenTimeH"));
  if (wp) {
    if ( PGOpenTimeH != wp->GetDataField()->GetAsInteger()) {
      PGOpenTimeH = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGOpenTimeH, PGOpenTimeH);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGOpenTimeM"));
  if (wp) {
    if ( PGOpenTimeM != wp->GetDataField()->GetAsInteger()) {
      PGOpenTimeM = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGOpenTimeM, PGOpenTimeM);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGGateIntervalTime"));
  if (wp) {
    if ( PGGateIntervalTime != wp->GetDataField()->GetAsInteger()) {
      PGGateIntervalTime = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGGateIntervalTime, PGGateIntervalTime);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpPGStartOut"));
  if (wp) {
    if ( PGStartOut != wp->GetDataField()->GetAsInteger()) {
      PGStartOut = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPGStartOut, PGStartOut);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPolarType"));
  if (wp) {
    if (POLARID != wp->GetDataField()->GetAsInteger()) {
      POLARID = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryPolarID, POLARID);
      GlidePolar::SetBallast();
      POLARFILECHANGED = true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceDisplay"));
  if (wp) {
    if (AltitudeMode != wp->GetDataField()->GetAsInteger()) {
      AltitudeMode = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAltMode, AltitudeMode);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeMode"));
  if (wp) {
    if (SafetyAltitudeMode != wp->GetDataField()->GetAsInteger()) {
      SafetyAltitudeMode = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistrySafetyAltitudeMode, SafetyAltitudeMode);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLockSettingsInFlight"));
  if (wp) {
    if (LockSettingsInFlight != 
	wp->GetDataField()->GetAsBoolean()) {
      LockSettingsInFlight = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryLockSettingsInFlight,
		    LockSettingsInFlight);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLoggerShortName"));
  if (wp) {
    if (LoggerShortName != 
	wp->GetDataField()->GetAsBoolean()) {
      LoggerShortName = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryLoggerShort,
		    LoggerShortName);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableFLARMMap"));
  if (wp) {
    if ((int)EnableFLARMMap != 
	wp->GetDataField()->GetAsInteger()) {
      EnableFLARMMap = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryEnableFLARMMap,
		    EnableFLARMMap);
      changed = true;
    }
  }

  #if 0
  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableFLARMGauge"));
  if (wp) {
    if (EnableFLARMGauge != 
	wp->GetDataField()->GetAsBoolean()) {
      EnableFLARMGauge = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryEnableFLARMGauge,
		    EnableFLARMGauge);
      changed = true;
    }
  }
  #endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpDebounceTimeout"));
  if (wp) {
    if (debounceTimeout != wp->GetDataField()->GetAsInteger()) {
      debounceTimeout = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryDebounceTimeout, debounceTimeout);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceOutline"));
  if (wp) {
    if (MapWindow::bAirspaceBlackOutline != 
	wp->GetDataField()->GetAsBoolean()) {
      MapWindow::bAirspaceBlackOutline = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryAirspaceBlackOutline,
		    MapWindow::bAirspaceBlackOutline);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoZoom"));
  if (wp) {
    if (MapWindow::zoom.AutoZoom() != 
	wp->GetDataField()->GetAsBoolean()) {
      MapWindow::zoom.AutoZoom(wp->GetDataField()->GetAsBoolean());
      SetToRegistry(szRegistryAutoZoom,
		    MapWindow::zoom.AutoZoom());
      changed = true;
    }
  }

  int ival;

  wp = (WndProperty*)wf->FindByName(TEXT("prpUTCOffset"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsFloat()*3600.0);
    if ((UTCOffset != ival)||(utcchanged)) {
      UTCOffset = ival;

      // have to do this because registry variables can't be negative!
      int lival = UTCOffset;
      if (lival<0) { lival+= 24*3600; }
      SetToRegistry(szRegistryUTCOffset, lival);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpClipAltitude"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY);
    if (ClipAltitude != ival) {
      ClipAltitude = ival;
      SetToRegistry(szRegistryClipAlt,ClipAltitude);  // fixed 20060430/sgi was szRegistryAltMode
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAltWarningMargin"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY);
    if (AltWarningMargin != ival) {
      AltWarningMargin = ival;
      SetToRegistry(szRegistryAltMargin,AltWarningMargin);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAirspaceWarnings"));
  if (wp) {
    if (AIRSPACEWARNINGS != wp->GetDataField()->GetAsInteger()) {
      AIRSPACEWARNINGS = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAirspaceWarning,(DWORD)AIRSPACEWARNINGS);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWarningTime"));
  if (wp) {
    if (WarningTime != wp->GetDataField()->GetAsInteger()) {
      WarningTime = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryWarningTime,(DWORD)WarningTime);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAcknowledgementTime"));
  if (wp) {
    if (AcknowledgementTime != wp->GetDataField()->GetAsInteger()) {
      AcknowledgementTime = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAcknowledgementTime,
		    (DWORD)AcknowledgementTime);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointLabels"));
  if (wp) {
    if (DisplayTextType != wp->GetDataField()->GetAsInteger()) {
      DisplayTextType = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryDisplayText,DisplayTextType);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableTerrain"));
  if (wp) {
    if (EnableTerrain != wp->GetDataField()->GetAsBoolean()) {
      EnableTerrain = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryDrawTerrain, EnableTerrain);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableTopology"));
  if (wp) {
    if (EnableTopology != wp->GetDataField()->GetAsBoolean()) {
      EnableTopology = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryDrawTopology, EnableTopology);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpCirclingZoom"));
  if (wp) {
    if (MapWindow::zoom.CircleZoom() != wp->GetDataField()->GetAsBoolean()) {
      MapWindow::zoom.CircleZoom(wp->GetDataField()->GetAsBoolean());
      SetToRegistry(szRegistryCircleZoom, MapWindow::zoom.CircleZoom());
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOrientation"));
  if (wp) {
#if AUTORIENT
    if (OldDisplayOrientation != wp->GetDataField()->GetAsInteger()) {
      OldDisplayOrientation = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryDisplayUpValue,OldDisplayOrientation);
      DisplayOrientation=OldDisplayOrientation;
      changed = true;
    }
#else
    if (DisplayOrientation != wp->GetDataField()->GetAsInteger()) {
      DisplayOrientation = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryDisplayUpValue,DisplayOrientation);
      changed = true;
    }
#endif
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMenuTimeout"));
  if (wp) {
    if (MenuTimeoutMax != wp->GetDataField()->GetAsInteger()*2) {
      MenuTimeoutMax = wp->GetDataField()->GetAsInteger()*2;
      SetToRegistry(szRegistryMenuTimeout,MenuTimeoutMax);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeArrival"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY);
    if (SAFETYALTITUDEARRIVAL != ival) {
      SAFETYALTITUDEARRIVAL = ival;
      SetToRegistry(szRegistrySafetyAltitudeArrival,
		    (DWORD)SAFETYALTITUDEARRIVAL);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeBreakoff"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY);
    if (SAFETYALTITUDEBREAKOFF != ival) {
      SAFETYALTITUDEBREAKOFF = ival;
      SetToRegistry(szRegistrySafetyAltitudeBreakOff,
		    (DWORD)SAFETYALTITUDEBREAKOFF);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSafetyAltitudeTerrain"));
  if (wp) {
    ival = iround(wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY);
    if (SAFETYALTITUDETERRAIN != ival) {
      SAFETYALTITUDETERRAIN = ival;
      SetToRegistry(szRegistrySafetyAltitudeTerrain,
		    (DWORD)SAFETYALTITUDETERRAIN);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoWind"));
  if (wp) {
    if (AutoWindMode != wp->GetDataField()->GetAsInteger()) {
      AutoWindMode = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAutoWind, AutoWindMode);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWindArrowStyle"));
  if (wp) {
    if (MapWindow::WindArrowStyle != wp->GetDataField()->GetAsInteger()) {
      MapWindow::WindArrowStyle = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryWindArrowStyle, MapWindow::WindArrowStyle);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoMcMode"));
  if (wp) {
    if (AutoMcMode != wp->GetDataField()->GetAsInteger()) {
      AutoMcMode = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAutoMcMode, AutoMcMode);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointsOutOfRange"));
  if (wp) {
    if (WaypointsOutOfRange != wp->GetDataField()->GetAsInteger()) {
      WaypointsOutOfRange = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryWaypointsOutOfRange, WaypointsOutOfRange);
      WAYPOINTFILECHANGED= true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoForceFinalGlide"));
  if (wp) {
    if (AutoForceFinalGlide != wp->GetDataField()->GetAsBoolean()) {
      AutoForceFinalGlide = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryAutoForceFinalGlide, AutoForceFinalGlide);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableNavBaroAltitude"));
  if (wp) {
    if (EnableNavBaroAltitude != wp->GetDataField()->GetAsBoolean()) {
      EnableNavBaroAltitude = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryEnableNavBaroAltitude, EnableNavBaroAltitude);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOrbiter"));
  if (wp) {
    if (Orbiter != wp->GetDataField()->GetAsBoolean()) {
      Orbiter = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryOrbiter, Orbiter);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpShading"));
  if (wp) {
    if (Shading != wp->GetDataField()->GetAsBoolean()) {
      Shading = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryShading, Shading);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFinalGlideTerrain"));
  if (wp) {
    if (FinalGlideTerrain != wp->GetDataField()->GetAsInteger()) {
      FinalGlideTerrain = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryFinalGlideTerrain, FinalGlideTerrain);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBlockSTF"));
  if (wp) {
    if (EnableBlockSTF != wp->GetDataField()->GetAsBoolean()) {
      EnableBlockSTF = wp->GetDataField()->GetAsBoolean();
      SetToRegistry(szRegistryBlockSTF, EnableBlockSTF);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsSpeed"));
  if (wp) {
    if ((int)Speed != wp->GetDataField()->GetAsInteger()) {
      Speed = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistrySpeedUnitsValue, Speed);
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
      SetToRegistry(szRegistryLatLonUnits, Units::CoordinateFormat);
      Units::NotifyUnitChanged();
      requirerestart = true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsTaskSpeed"));
  if (wp) {
    if ((int)TaskSpeed != wp->GetDataField()->GetAsInteger()) {
      TaskSpeed = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryTaskSpeedUnitsValue, TaskSpeed);
      Units::NotifyUnitChanged();
      requirerestart = true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsDistance"));
  if (wp) {
    if ((int)Distance != wp->GetDataField()->GetAsInteger()) {
      Distance = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryDistanceUnitsValue, Distance);
      Units::NotifyUnitChanged();
      requirerestart = true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsLift"));
  if (wp) {
    if ((int)Lift != wp->GetDataField()->GetAsInteger()) {
      Lift = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryLiftUnitsValue, Lift);
      Units::NotifyUnitChanged();
      requirerestart = true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUnitsAltitude"));
  if (wp) {
    if ((int)Altitude != wp->GetDataField()->GetAsInteger()) {
      Altitude = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAltitudeUnitsValue, Altitude);
      Units::NotifyUnitChanged();
      changed = true;
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFAIFinishHeight"));
  if (wp) {
    if (EnableFAIFinishHeight != (wp->GetDataField()->GetAsInteger()>0)) {
      EnableFAIFinishHeight = (wp->GetDataField()->GetAsInteger()>0);
      SetToRegistry(szRegistryFAIFinishHeight, EnableFAIFinishHeight);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpOLCRules"));
  if (wp) {
    if (OLCRules != wp->GetDataField()->GetAsInteger()) {
      OLCRules = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryOLCRules, OLCRules);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpHandicap"));
  if (wp) {
    ival  = wp->GetDataField()->GetAsInteger();
    if (Handicap != ival) {
      Handicap = ival;
      SetToRegistry(szRegistryHandicap, Handicap);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPolarFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szPolarFile)) {
      SetRegistryString(szRegistryPolarFile, temptext);
      POLARFILECHANGED = true;
      GlidePolar::SetBallast();
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWaypointFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szWaypointFile)) {
      SetRegistryString(szRegistryWayPointFile, temptext);
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
      SetRegistryString(szRegistryAdditionalWayPointFile, temptext);
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
      SetRegistryString(szRegistryAirspaceFile, temptext);
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
      SetRegistryString(szRegistryAdditionalAirspaceFile, temptext);
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
      SetRegistryString(szRegistryMapFile, temptext);
      MAPFILECHANGED= true;
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
      SetRegistryString(szRegistryTerrainFile, temptext);
      TERRAINFILECHANGED= true;
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTopologyFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szTopologyFile)) {
      SetRegistryString(szRegistryTopologyFile, temptext);
      TOPOLOGYFILECHANGED= true;
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
      SetRegistryString(szRegistryAirfieldFile, temptext);
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
      SetRegistryString(szRegistryLanguageFile, temptext);
      requirerestart = true; // restart needed for XCI reload
      LKReadLanguageFile();
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStatusFile"));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _tcscpy(temptext, dfe->GetPathFile());
    ContractLocalPath(temptext);
    if (_tcscmp(temptext,szStatusFile)) {
      SetRegistryString(szRegistryStatusFile, temptext);
      requirerestart = true;
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
      SetRegistryString(szRegistryInputFile, temptext);
      requirerestart = true;
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpWindCalcTime")); // 100113
  if (wp) {
	ival = wp->GetDataField()->GetAsInteger();
	if (WindCalcTime != ival) {
		WindCalcTime = ival;
		SetToRegistry(szRegistryWindCalcTime,(DWORD)WindCalcTime);
		changed = true;
	}
  }

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

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppIndFinalGlide"));
  if (wp) {
    if (Appearance.IndFinalGlide != (IndFinalGlide_t)(wp->GetDataField()->GetAsInteger())) {
      Appearance.IndFinalGlide = (IndFinalGlide_t)(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryAppIndFinalGlide,(DWORD)(Appearance.IndFinalGlide));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppCompassAppearance"));
  if (wp) {
    if (Appearance.CompassAppearance != (CompassAppearance_t)
	(wp->GetDataField()->GetAsInteger())) {
      Appearance.CompassAppearance = (CompassAppearance_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryAppCompassAppearance,
		    (DWORD)(Appearance.CompassAppearance));
      changed = true;
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxBorder"));
  if (wp) {
    if (Appearance.InfoBoxBorder != (InfoBoxBorderAppearance_t)
	(wp->GetDataField()->GetAsInteger())) {
      Appearance.InfoBoxBorder = (InfoBoxBorderAppearance_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryAppInfoBoxBorder,
		    (DWORD)(Appearance.InfoBoxBorder));
      changed = true;
      requirerestart = true;
    }
  }

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
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpExtendedVisualGlide")); // VENTA4
  if (wp) {
    if (ExtendedVisualGlide != (ExtendedVisualGlide_t)
	(wp->GetDataField()->GetAsInteger())) {
      ExtendedVisualGlide = (ExtendedVisualGlide_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryExtendedVisualGlide,
		    (DWORD)(ExtendedVisualGlide));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLook8000"));
  if (wp) {
    if (Look8000 != (Look8000_t)
	(wp->GetDataField()->GetAsInteger())) {
      Look8000 = (Look8000_t)
	(wp->GetDataField()->GetAsInteger());
	if (Look8000!=0) { // 091115 do not allow Reserved mode , do not disable LK8000 !
		SetToRegistry(szRegistryLook8000, (DWORD)(Look8000));
		changed = true;
	} else Look8000=1;
    }
  }

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

  wp = (WndProperty*)wf->FindByName(TEXT("prpNewMap")); 
  if (wp) {
    if (NewMap != (NewMap_t)
	(wp->GetDataField()->GetAsInteger())) {
      NewMap = (NewMap_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryNewMap,
		    (DWORD)(NewMap));
      changed = true;
    }
  }
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
  wp = (WndProperty*)wf->FindByName(TEXT("prpVirtualKeys")); // VENTA6
  if (wp) {
    if (VirtualKeys != (VirtualKeys_t)
	(wp->GetDataField()->GetAsInteger())) {
      VirtualKeys = (VirtualKeys_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryVirtualKeys,
		    (DWORD)(VirtualKeys));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpUseMapLock")); // VENTA9
  if (wp) {
    if (UseMapLock != (UseMapLock_t) (wp->GetDataField()->GetAsInteger())) {
      UseMapLock = (UseMapLock_t) (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryUseMapLock, (DWORD)(UseMapLock));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpActiveMap")); // 100318
  if (wp) {
    if (ActiveMap != (ActiveMap_t) (wp->GetDataField()->GetAsInteger())) {
      ActiveMap = (ActiveMap_t) (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryActiveMap, (DWORD)(ActiveMap));
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
      LastRangeLandableTime=0;
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
      requirerestart = true;
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

  #if 0
  wp = (WndProperty*)wf->FindByName(TEXT("prpShortcutIbox"));
  if (wp) {
	if (ShortcutIbox != (wp->GetDataField()->GetAsInteger())) {
		ShortcutIbox = (wp->GetDataField()->GetAsInteger());
		SetToRegistry(szRegistryShortcutIbox, (DWORD)(ShortcutIbox));
		changed=true;
	}
  }
  #endif

  wp = (WndProperty*)wf->FindByName(TEXT("prpAverEffTime")); // VENTA6
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
    if (BgMapColor != 
	(wp->GetDataField()->GetAsInteger())) {
      BgMapColor = 
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryBgMapColor, (DWORD)(BgMapColor));
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

#if defined(PNA) || defined(FIVV)
// VENTA-ADDON GEOM
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxGeom"));
  if (wp) {
    if (Appearance.InfoBoxGeom != (InfoBoxGeomAppearance_t)
        (wp->GetDataField()->GetAsInteger())) {
      Appearance.InfoBoxGeom = (InfoBoxGeomAppearance_t)
        (wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryAppInfoBoxGeom,
                    (DWORD)(Appearance.InfoBoxGeom));
      changed = true;
      requirerestart = true;
    }
  }
//
#endif

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
      default:
	GlobalModelType = MODELTYPE_UNKNOWN; // Can't happen, troubles ..
	break;
	
      }
      SetToRegistry(szRegistryAppInfoBoxModel,
                    GlobalModelType);
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

  DeleteObject (TempInfoWindowFont);
  DeleteObject (TempTitleWindowFont);
  DeleteObject (TempMapWindowFont);
  DeleteObject (TempTitleSmallWindowFont);
  DeleteObject (TempMapWindowBoldFont);
  DeleteObject (TempCDIWindowFont); 
  DeleteObject (TempMapLabelFont);
  DeleteObject (TempStatisticsFont);



  wp = (WndProperty*)wf->FindByName(TEXT("prpAppStatusMessageAlignment"));
  if (wp) {
    if (Appearance.StateMessageAlligne != (StateMessageAlligne_t)
	(wp->GetDataField()->GetAsInteger())) {
      Appearance.StateMessageAlligne = (StateMessageAlligne_t)
	(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryAppStatusMessageAlignment,
		    (DWORD)(Appearance.StateMessageAlligne));
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTextInput"));
  if (wp) 
  {
    if (Appearance.TextInputStyle != (TextInputStyle_t)(wp->GetDataField()->GetAsInteger())) 
      {
	Appearance.TextInputStyle = (TextInputStyle_t)(wp->GetDataField()->GetAsInteger());
	SetToRegistry(szRegistryAppTextInputStyle, (DWORD)(Appearance.TextInputStyle));
	changed = true;
      }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppIndLandable"));
  if (wp) {
    if (Appearance.IndLandable != (IndLandable_t)(wp->GetDataField()->GetAsInteger())) {
      Appearance.IndLandable = (IndLandable_t)(wp->GetDataField()->GetAsInteger());
      SetToRegistry(szRegistryAppIndLandable,(DWORD)(Appearance.IndLandable));
      changed = true;
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpEnableExternalTriggerCruise"));
  if (wp) {
    if ((int)(EnableExternalTriggerCruise) != 
	wp->GetDataField()->GetAsInteger()) {
      EnableExternalTriggerCruise = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryEnableExternalTriggerCruise,
		    EnableExternalTriggerCruise);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInverseInfoBox"));
  if (wp) {
    if ((int)(Appearance.InverseInfoBox) != wp->GetDataField()->GetAsInteger()) {
      Appearance.InverseInfoBox = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAppInverseInfoBox,Appearance.InverseInfoBox);
      requirerestart = true;
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
#ifdef NEWMOVEICON
	MapWindow::GliderScreenPositionY=MapWindow::GliderScreenPosition;
#endif
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

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppInfoBoxColors"));
  if (wp) {
    if ((int)(Appearance.InfoBoxColors) != 
	wp->GetDataField()->GetAsInteger()) {
      Appearance.InfoBoxColors = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAppInfoBoxColors,Appearance.InfoBoxColors);
      requirerestart = true;
      changed = true;
    }
  }

  #if !110101
  wp = (WndProperty*)wf->FindByName(TEXT("prpAppAveNeedle"));
  if (wp) {
    if ((int)(Appearance.GaugeVarioAveNeedle) != 
	wp->GetDataField()->GetAsInteger()) {
      Appearance.GaugeVarioAveNeedle = 
        (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAppAveNeedle,Appearance.GaugeVarioAveNeedle);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioSpeedToFly"));
  if (wp) {
    if ((int)(Appearance.GaugeVarioSpeedToFly) != wp->GetDataField()->GetAsInteger()) {
      Appearance.GaugeVarioSpeedToFly = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAppGaugeVarioSpeedToFly,Appearance.GaugeVarioSpeedToFly);
      changed = true;
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioAvgText"));
  if (wp) {
    if ((int)Appearance.GaugeVarioAvgText != wp->GetDataField()->GetAsInteger()) {
      Appearance.GaugeVarioAvgText = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAppGaugeVarioAvgText,Appearance.GaugeVarioAvgText);
      changed = true;
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioGross"));
  if (wp) {
    if ((int)Appearance.GaugeVarioGross != wp->GetDataField()->GetAsInteger()) {
      Appearance.GaugeVarioGross = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAppGaugeVarioGross,Appearance.GaugeVarioGross);
      changed = true;
      requirerestart = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioMc"));
  if (wp) {
    if ((int)Appearance.GaugeVarioMc != wp->GetDataField()->GetAsInteger()) {
      Appearance.GaugeVarioMc = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAppGaugeVarioMc,Appearance.GaugeVarioMc);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioBugs"));
  if (wp) {
    if ((int)Appearance.GaugeVarioBugs != wp->GetDataField()->GetAsInteger()) {
      Appearance.GaugeVarioBugs = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAppGaugeVarioBugs,Appearance.GaugeVarioBugs);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAppGaugeVarioBallast"));
  if (wp) {
    if ((int)Appearance.GaugeVarioBallast != wp->GetDataField()->GetAsInteger()) {
      Appearance.GaugeVarioBallast = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAppGaugeVarioBallast,Appearance.GaugeVarioBallast);
      changed = true;
    }
  }
  #endif // REMOVABLE

  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoBlank"));
  if (wp) {
    if (EnableAutoBlank != (wp->GetDataField()->GetAsInteger()!=0)) {
      EnableAutoBlank = (wp->GetDataField()->GetAsInteger() != 0);
      SetToRegistry(szRegistryAutoBlank, EnableAutoBlank);
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
    if (TerrainRamp != wp->GetDataField()->GetAsInteger()) {
      TerrainRamp = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryTerrainRamp, TerrainRamp);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFinishMinHeight"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY)*1000.0); // 100315 BUGFIX XCSOAr
    if ((int)FinishMinHeight != ival) {
      FinishMinHeight = ival;
      SetToRegistry(szRegistryFinishMinHeight,FinishMinHeight);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxHeight"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY) *1000.0); // 100315 BUGFIX XCSOAR
    if ((int)StartMaxHeight != ival) {
      StartMaxHeight = ival;
      SetToRegistry(szRegistryStartMaxHeight,StartMaxHeight);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxHeightMargin"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/ALTITUDEMODIFY) *1000.0 ); // 100315
    if ((int)StartMaxHeightMargin != ival) {
      StartMaxHeightMargin = ival;
      SetToRegistry(szRegistryStartMaxHeightMargin,StartMaxHeightMargin);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpStartHeightRef"));
  if (wp) {
    if (StartHeightRef != wp->GetDataField()->GetAsInteger()) {
      StartHeightRef = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryStartHeightRef, StartHeightRef);
      changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxSpeed"));
  if (wp) {
    ival = iround( (wp->GetDataField()->GetAsInteger()/SPEEDMODIFY)*1000.0);  
    if (((int)StartMaxSpeed) != ival) {
	StartMaxSpeed = ival;
	SetToRegistry(szRegistryStartMaxSpeed,StartMaxSpeed);
	changed = true;
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartMaxSpeedMargin"));
  if (wp) {
    ival = iround((wp->GetDataField()->GetAsInteger()/SPEEDMODIFY)*1000.0); 
    if ((int)StartMaxSpeedMargin != ival) {
      StartMaxSpeedMargin = ival;
      SetToRegistry(szRegistryStartMaxSpeedMargin,StartMaxSpeedMargin);
      changed = true;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpAutoAdvance"));
  if (wp) {
    if (AutoAdvance != wp->GetDataField()->GetAsInteger()) {
      AutoAdvance = wp->GetDataField()->GetAsInteger();
      SetToRegistry(szRegistryAutoAdvance,
		    AutoAdvance);
      changed = true;
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

  if (COMPORTCHANGED) {
    WritePort1Settings(dwPortIndex1,dwSpeedIndex1, dwBit1Index);
    WritePort2Settings(dwPortIndex2,dwSpeedIndex2, dwBit2Index);
  }

  int i,j;
  for (i=0; i<4; i++) {
    for (j=0; j<9; j++) {
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


    StoreRegistry();

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

