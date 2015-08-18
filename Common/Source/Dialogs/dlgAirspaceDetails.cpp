/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceDetails.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "InputEvents.h"
#include "Airspace.h"
#include "AirspaceWarning.h"
#include "Dialogs.h"
#include "TraceThread.h"
#include "LKObjects.h"
#include "Sound/Sound.h"
#include "resource.h"

static CAirspaceBase airspace_copy;
static WndForm *wf=NULL;

static void SetValues(void);

static void OnPaintAirspacePicto(WindowControl * Sender, LKSurface& Surface){
	  (void)Sender;

	  WndFrame  *wPicto = ((WndFrame *)wf->FindByName(TEXT("frmAirspacePicto")));
	  LKASSERT(wPicto!=NULL);
	  const RECT rc = wPicto->GetClientRect();
	  const auto oldPen = Surface.SelectObject(LKPen_Petrol_C2);
	  const auto oldBrush = Surface.SelectObject(LKBrush_Petrol);

	  Surface.Rectangle(rc.left,rc.top,rc.right,rc.bottom);


	  Surface.SetBkColor(RGB_LIGHTGREY);
      /****************************************************************
       * for drawing the airspace pictorial, we need the original data.
       * copy contain only base class property, not geo data, 
       * original data are shared ressources ! 
       * for that we need to grant all called methods are thread safe
       ****************************************************************/
   {
      CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
      CAirspace* airspace = CAirspaceManager::Instance().GetAirspacesForDetails();
      if(airspace) {
        airspace->DrawPicto(Surface, rc);
      }
   }
    Surface.SelectObject(oldPen);
    Surface.SelectObject(oldBrush);

}

static void OnFlyClicked(WndButton* pWnd) {
    (void) pWnd;

    if (wf == NULL) return;
    
   {
      CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
      CAirspace* airspace = CAirspaceManager::Instance().GetAirspacesForDetails();
      if(airspace) {
        CAirspaceManager::Instance().AirspaceFlyzoneToggle(*airspace);
      }
   }
    SetValues();
    PlayResource(TEXT("IDR_WAV_CLICK"));
}

static void OnSelectClicked(WndButton* pWnd) {
    (void) pWnd;

   {
      CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
      CAirspace* airspace = CAirspaceManager::Instance().GetAirspacesForDetails();
      if(airspace) {
        CAirspaceManager::Instance().AirspaceSetSelect(*airspace);
      }
   }
    SetValues();
    PlayResource(TEXT("IDR_WAV_CLICK"));
}


static void OnAcknowledgeClicked(WndButton* pWnd){
  (void)pWnd;

  if (wf == NULL) return;

  {
    CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
    CAirspace* airspace = CAirspaceManager::Instance().GetAirspacesForDetails();
    if(airspace) {
      if (airspace_copy.Enabled()) {
        CAirspaceManager::Instance().AirspaceDisable(*airspace);
      } else {
        CAirspaceManager::Instance().AirspaceEnable(*airspace);
      }
    }
  }


  WndFrame  *wPicto = ((WndFrame *)wf->FindByName(TEXT("frmAirspacePicto")));
  if(wPicto) {
    wPicto->Redraw();
  }
  SetValues();
  PlayResource(TEXT("IDR_WAV_CLICK"));
}

static void OnCloseClicked(WndButton* pWnd){
	(void)pWnd;
  wf->SetModalResult(mrOK);
}

static bool OnTimer(){
  SetValues();
  return true;
}


static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnAcknowledgeClicked),
  ClickNotifyCallbackEntry(OnFlyClicked),
  ClickNotifyCallbackEntry(OnCloseClicked),
  ClickNotifyCallbackEntry(OnSelectClicked),
  OnPaintCallbackEntry(OnPaintAirspacePicto),
  EndCallBackEntry()
};

static void SetValues(void) {

  
  WndProperty* wp;
  WndButton *wb;
  TCHAR buffer[80];
  TCHAR buffer2[160]; // must contain buffer

  int bearing;
  int hdist;
  int vdist;

  
  bool inside = false;
  {
    CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
    CAirspace* airspace = CAirspaceManager::Instance().GetAirspacesForDetails();
    if(airspace) {
        // Get an object instance copy with actual values
        airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(airspace);
        inside = CAirspaceManager::Instance().AirspaceCalculateDistance( airspace, &hdist, &bearing, &vdist);
    } else {
        // error : CAirspaceManager are closed ?
        return;
    }
  }
  
  if (wf!=NULL) {
	TCHAR capbuffer[250];
	_stprintf(capbuffer,_T("%s ("),airspace_copy.Name());
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
	  _stprintf(buffer,TEXT("%s %s"), CAirspaceManager::Instance().GetAirspaceTypeText(airspace_copy.Type()), gettext(TEXT("FLY")));
/*
	  if( _tcsnicmp(  airspace_copy.Name(),   airspace_copy.TypeName() ,_tcslen(airspace_copy.TypeName())) == 0)
		_stprintf(buffer,TEXT("%s"),airspace_copy.Name());
	  else
	    _stprintf(buffer,TEXT("%s %s"),airspace_copy.TypeName()   // fixed strings max. 20
			                          ,airspace_copy.Name());     // NAME_SIZE          30   => max. 30 char
*/
	} else {
	  _stprintf(buffer,TEXT("%s %s"), gettext(TEXT("NOFLY")), CAirspaceManager::Instance().GetAirspaceTypeText(airspace_copy.Type()));
	}

	wp->SetText( buffer );
//    wp->SetBackColor( airspace_copy.TypeColor());
//	wp->SetForeColor( ContrastTextColor(airspace_copy.TypeColor()));
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
	  _stprintf(buffer2, TEXT("%s %d%s %s"), buffer, iround(bearing), gettext(_T("_@M2179_")), gettext(TEXT("_@M1257_")));
    } else {
	  // LKTOKEN _@M1258_ "to enter"
	  _stprintf(buffer2, TEXT("%s %d%s %s"), buffer, iround(bearing), gettext(_T("_@M2179_")), gettext(TEXT("_@M1258_")));
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

/*
 * only called by #CAirspaceManager::ProcessAirspaceDetailQueue()
 * for display AirspaceDetails, use #PopupAirspaceDetail
 */
void dlgAirspaceDetails() {

  wf = dlgLoadFromXML(CallBackTable,
		      TEXT("dlgAirspaceDetails.xml"), 
		      IDR_XML_AIRSPACEDETAILS);

  if (!wf) return;
  wf->SetTimerNotify(1000, OnTimer);
  
  SetValues();

  wf->ShowModal();

  delete wf;
  wf = NULL;

  return;
}

