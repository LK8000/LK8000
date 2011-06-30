/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceDetails.cpp,v 8.3 2010/12/13 12:23:03 root Exp root $
*/

#include "StdAfx.h"
#include <aygshell.h>
#include "lk8000.h"
#include "Statistics.h"
#include "externs.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"
#include "InputEvents.h"
#include "Airspace.h"
#include "AirspaceWarning.h"
#include "LKAirspace.h"

#include "utils/heapcheck.h"

static CAirspace *airspace = NULL;
static CAirspace airspace_copy;
static WndForm *wf=NULL;

static void SetValues(void);

static void OnFlyClicked(WindowControl * Sender){
  (void)Sender;

  if (airspace == NULL) return;
  if (wf == NULL) return;

  #if 0 // We dont ask for confirmation, but we might change our mind!
  UINT answer;
  if (airspace_copy.Flyzone()) {
	// LKTOKEN _@M1273_ "Set as NOFLY zone?"
	answer = MessageBoxX(hWndMapWindow, airspace_copy.Name(), gettext(TEXT("_@M1273_")), MB_YESNO|MB_ICONQUESTION);
  } else {
	// LKTOKEN _@M1272_ "Set as FLY zone?"
	answer = MessageBoxX(hWndMapWindow, airspace_copy.Name(), gettext(TEXT("_@M1272_")), MB_YESNO|MB_ICONQUESTION);
  }
  if (answer == IDYES) {
	CAirspaceManager::Instance().AirspaceFlyzoneToggle(*airspace);
	SetValues();
	// wf->SetModalResult(mrOK);
  }
  #endif

  CAirspaceManager::Instance().AirspaceFlyzoneToggle(*airspace);
  SetValues();
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));

}

static void OnSelectClicked(WindowControl * Sender){
  (void)Sender;

  if (airspace == NULL) return;

  CAirspaceManager::Instance().AirspaceSetSelect(*airspace);
  SetValues();
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));

}

static void OnAnalysisClicked(WindowControl * Sender){
  (void)Sender;

  InputEvents::eventSetup(_T("AspAnalysis"));

  if (wf == NULL) return;
  wf->SetModalResult(mrOK);

}


static void OnAcknowledgeClicked(WindowControl * Sender){
  (void)Sender;

  if (airspace == NULL) return;
  if (wf == NULL) return;

  #if 0  // We dont ask anymore to the user for enable/disable confirmation
  UINT answer;
  if (!airspace_copy.Enabled()) {
  
    // LKTOKEN  _@M1280_ "Enable this airspace?"
    answer = MessageBoxX(hWndMapWindow, airspace_copy.Name(), gettext(TEXT("_@M1280_")),  MB_YESNO|MB_ICONQUESTION);
    if (answer == IDYES) {
      // this will cancel a daily ack
      CAirspaceManager::Instance().AirspaceEnable(*airspace);
      wf->SetModalResult(mrOK);
    }
  } else {
    // LKTOKEN  _@M1284_ "Disable this airspace?" 
    answer = MessageBoxX(hWndMapWindow, airspace_copy.Name(), gettext(TEXT("_@M1284_")),	MB_YESNO|MB_ICONQUESTION);
    if (answer == IDYES) {
      CAirspaceManager::Instance().AirspaceDisable(*airspace);
      wf->SetModalResult(mrOK);
    }
  }
  #endif

  if (airspace_copy.Enabled()) 
      CAirspaceManager::Instance().AirspaceDisable(*airspace);
  else 
      CAirspaceManager::Instance().AirspaceEnable(*airspace);

  SetValues();
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));


}

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnAcknowledgeClicked),
  DeclareCallBackEntry(OnFlyClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnSelectClicked),
  DeclareCallBackEntry(OnAnalysisClicked),
  DeclareCallBackEntry(NULL)
};
static void SetValues(void) {

  if (airspace==NULL) return;

   // Get an object instance copy with actual values
  airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(airspace);

  WndProperty* wp;
  WndButton *wb;
  TCHAR buffer[80];
  TCHAR buffer2[80];

  int bearing;
  int hdist;
  int vdist;
  bool inside = CAirspaceManager::Instance().AirspaceCalculateDistance( airspace, &hdist, &bearing, &vdist);

  if (wf!=NULL) {
	TCHAR capbuffer[250];
	wsprintf(capbuffer,_T("%s ("),airspace_copy.Name());
        if (airspace_copy.Enabled()) {
        	_tcscat(capbuffer,gettext(TEXT("_@M1643_"))); // ENABLED
        } else {
        	_tcscat(capbuffer,gettext(TEXT("_@M1600_"))); // DISABLED
        }
        _tcscat(capbuffer,_T(")")); // DISABLED
	wf->SetCaption(capbuffer);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpType"));
  if (wp) {
	if (airspace_copy.Flyzone()) {
	  wsprintf(buffer,TEXT("%s %s"), CAirspaceManager::Instance().GetAirspaceTypeText(airspace_copy.Type()), gettext(TEXT("FLY")));
	} else {
	  wsprintf(buffer,TEXT("%s %s"), CAirspaceManager::Instance().GetAirspaceTypeText(airspace_copy.Type()), gettext(TEXT("NOFLY")));
	}
	wp->SetText( buffer );
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpTop"));
  if (wp) {
	CAirspaceManager::Instance().GetAirspaceAltText(buffer, sizeof(buffer)/sizeof(buffer[0]), airspace_copy.Top());
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBase"));
  if (wp) {
	CAirspaceManager::Instance().GetAirspaceAltText(buffer, sizeof(buffer)/sizeof(buffer[0]), airspace_copy.Base());
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRange"));
  if (wp) {
    Units::FormatUserDistance(abs(hdist), buffer, 20);
    if (inside) {
	  // LKTOKEN  _@M359_ = "Inside" 
      wp->SetCaption(gettext(TEXT("_@M359_")));
    }
    if (hdist < 0) {
	  // LKTOKEN _@M1257_ "to leave"
	  wsprintf(buffer2, TEXT("%s %d")TEXT(DEG)TEXT(" %s"), buffer, iround(bearing), gettext(TEXT("_@M1257_")));
    } else {
	  // LKTOKEN _@M1258_ "to enter"
	  wsprintf(buffer2, TEXT("%s %d")TEXT(DEG)TEXT(" %s"), buffer, iround(bearing), gettext(TEXT("_@M1258_")));
	}
    wp->SetText(buffer2);
    wp->RefreshDisplay();
  }
  
  // ONLY for DIAGNOSTICS- ENABLE ALSO XML
  #if 0
  wp = (WndProperty*)wf->FindByName(TEXT("prpWarnLevel"));
  if (wp) {
	  switch (airspace_copy.WarningLevel()) {
		default:
		  // LKTOKEN _@M765_ "Unknown"
		  wp->SetText(gettext(TEXT("_@M765_")));
		  break;
		  
		case awNone:
		  // LKTOKEN _@M479_ "None"
  		  wp->SetText(gettext(TEXT("_@M479_")));
		  break;

		case awYellow:
			// LKTOKEN _@M1255_ "YELLOW WARNING"
			wp->SetText(gettext(TEXT("_@M1255_")));
		  break;
		
		case awRed:
			// LKTOKEN _@M1256_ "RED WARNING"
			wp->SetText(gettext(TEXT("_@M1256_")));
		  break;
	  }//sw
	  wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAckLevel"));
  if (wp) {
      if (airspace_copy.Enabled()) {
        switch (airspace_copy.WarningAckLevel()) {
          default:
            // LKTOKEN _@M765_ "Unknown"
            wp->SetText(gettext(TEXT("_@M765_")));
            break;
            
          case awNone:
            // LKTOKEN _@M479_ "None"
            wp->SetText(gettext(TEXT("_@M479_")));
            break;

          case awYellow:
              // LKTOKEN _@M1267_ "Yellow acknowledged"
              wp->SetText(gettext(TEXT("_@M1267_")));
            break;
          
          case awRed:
              // LKTOKEN _@M1268_ "Red acknowledged"
              wp->SetText(gettext(TEXT("_@M1268_")));
            break;

        }//sw
      } else {
          // LKTOKEN _@M1269_ "Disabled"
          wp->SetText(gettext(TEXT("_@M1269_")));
      }
	  wp->RefreshDisplay();
  }
  #endif

  wb = (WndButton*)wf->FindByName(TEXT("cmdFly"));
  if (wb) {
	if (airspace_copy.Flyzone()) {
	  // LKTOKEN _@M1271_ "NOFLY"
	  wb->SetCaption(gettext(TEXT("_@M1271_")));
	} else {
	  // LKTOKEN _@M1270_ "FLY"
	  wb->SetCaption(gettext(TEXT("_@M1270_")));
	}
	wb->Redraw();
  }

  wb = (WndButton*)wf->FindByName(TEXT("cmdSelect"));
  if (wb) {
	if (airspace_copy.Selected()) {
	  wb->SetCaption(gettext(TEXT("_@M1656_"))); // SELECTED!
	} else {
	  wb->SetCaption(gettext(TEXT("_@M1654_"))); // SELECT
	}
	wb->Redraw();
  }

  wb = (WndButton*)wf->FindByName(TEXT("cmdAcknowledge"));
  if (wb) {
    if (airspace_copy.Enabled()) {
      // LKTOKEN _@M1283_ "Disable"
      wb->SetCaption(gettext(TEXT("_@M1283_")));
    } else {
      // LKTOKEN _@M1282_ "Enable"
      wb->SetCaption(gettext(TEXT("_@M1282_")));
    }
    wb->Redraw();
  }

}


void dlgAirspaceDetails(CAirspace *airspace_to_show) {
  if (airspace != NULL) {
    return;
  }

  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgAirspaceDetails.xml"));
  wf = dlgLoadFromXML(CallBackTable,
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_AIRSPACEDETAILS"));

  if (!wf) return;
  airspace = airspace_to_show;
  SetValues();

  wf->ShowModal();
  airspace = NULL;
  delete wf;
  wf = NULL;
  return;
}

