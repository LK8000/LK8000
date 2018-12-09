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

static void SetValues(WndForm* wf);

static void OnPaintAirspacePicto(WindowControl * Sender, LKSurface& Surface){

	  const RECT rc = Sender->GetClientRect();

	  Surface.SetBkColor(RGB_LIGHTGREY);
      /****************************************************************
       * for drawing the airspace pictorial, we need the original data.
       * copy contain only base class property, not geo data, 
       * original data are shared ressources ! 
       * for that we need to grant all called methods are thread safe
       ****************************************************************/
   {
      ScopeLock guard(CAirspaceManager::Instance().MutexRef());
      CAirspace* airspace = CAirspaceManager::Instance().GetAirspacesForDetails();
      if(airspace) {
        airspace->DrawPicto(Surface, rc);
      }
   }
}

static void OnFlyClicked(WndButton* pWnd) {
    (void) pWnd;
   {
      ScopeLock guard(CAirspaceManager::Instance().MutexRef());
      CAirspace* airspace = CAirspaceManager::Instance().GetAirspacesForDetails();
      if(airspace) {
        CAirspaceManager::Instance().AirspaceFlyzoneToggle(*airspace);
      }
   }
    SetValues(pWnd->GetParentWndForm());
    PlayResource(TEXT("IDR_WAV_CLICK"));
}

static void OnSelectClicked(WndButton* pWnd) {
    (void) pWnd;

   {
      ScopeLock guard(CAirspaceManager::Instance().MutexRef());
      CAirspace* airspace = CAirspaceManager::Instance().GetAirspacesForDetails();
      if(airspace) {
        CAirspaceManager::Instance().AirspaceSetSelect(*airspace);
      }
   }
    SetValues(pWnd->GetParentWndForm());
    PlayResource(TEXT("IDR_WAV_CLICK"));
}


static void OnAcknowledgeClicked(WndButton* pWnd){
  (void)pWnd;
  {
    ScopeLock guard(CAirspaceManager::Instance().MutexRef());
    CAirspace* airspace = CAirspaceManager::Instance().GetAirspacesForDetails();
    if(airspace) {
      if (airspace_copy.Enabled()) {
        CAirspaceManager::Instance().AirspaceDisable(*airspace);
      } else {
        CAirspaceManager::Instance().AirspaceEnable(*airspace);
      }
    }
  }

  WndForm* wf = pWnd->GetParentWndForm();
  if(wf) {
    WndFrame  *wPicto = ((WndFrame *)wf->FindByName(TEXT("frmAirspacePicto")));
    if(wPicto) {
      wPicto->Redraw();
    }
    SetValues(wf);
  }
  PlayResource(TEXT("IDR_WAV_CLICK"));
}

static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static bool OnTimer(WndForm* pWnd){
  SetValues(pWnd);
  return true;
}

double  ExtractFrequency(TCHAR *text)
{
    if(text == NULL)
        return 0.0;
 double fFreq = 0.0;
 int iTxtlen = (int)_tcslen(text);
 int i,Mhz=0,kHz=0;
 for (i=0; i < iTxtlen; i++)
 {
   if (text[i] == '1')
   {
     Mhz  = _tcstol(&text[i], nullptr, 10);
	 if(Mhz >= 118)
	   if(Mhz <= 138)
	         if((i+3) < iTxtlen)
	         {
		       if((text[i+3] == '.') || (text[i+3] == ','))
		       {
		    	 kHz =0;
		    	 if((i+4) < iTxtlen)
			       if((text[i+4] >= '0') && (text[i+4] <= '9'))
			    	   kHz += (text[i+4]-'0') * 100;

		    	 if((i+5) < iTxtlen)
			       if((text[i+5] >= '0') && (text[i+5] <= '9'))
			    	   kHz += (text[i+5]-'0') * 10;

		    	 if((i+6) < iTxtlen)
			       if((text[i+6] >= '0') && (text[i+6] <= '9'))
			    	   kHz += (text[i+6]-'0');

		       }
		       fFreq = (double) Mhz+ (double)kHz/1000.0f;
		       return fFreq;
	         }
	   }
   }

 return fFreq;
}

static void OnSetFrequency(WndButton* pWnd){
(void)pWnd;
#ifdef RADIO_ACTIVE
TCHAR Tmp[255];
 if(RadioPara.Enabled)
 {
   double ASFrequency = ExtractFrequency((TCHAR*)airspace_copy.Name());
    if((ASFrequency >= 118) && (ASFrequency <= 138))
    {
      _stprintf(Tmp,_T("%7.3fMHz"),ASFrequency);
      devPutFreqActive(ASFrequency, (TCHAR*)airspace_copy.Name());
        DoStatusMessage(_T(""), Tmp );
    }
  }
#endif  // RADIO_ACTIVE        
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
} 

static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnAcknowledgeClicked),
  ClickNotifyCallbackEntry(OnFlyClicked),
  ClickNotifyCallbackEntry(OnCloseClicked),
  ClickNotifyCallbackEntry(OnSelectClicked),
  ClickNotifyCallbackEntry(OnSetFrequency),
  OnPaintCallbackEntry(OnPaintAirspacePicto),
  EndCallBackEntry()
};

static void SetValues(WndForm* wf) {

  
  WndProperty* wp;
  WndButton *wb;
  TCHAR buffer[80];
  TCHAR buffer2[160]; // must contain buffer

  int bearing;
  int hdist;
  int vdist;

  
  bool inside = false;
  {
    ScopeLock guard(CAirspaceManager::Instance().MutexRef());
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
        	_tcscat(capbuffer,MsgToken(1643)); // ENABLED
        } else {
        	_tcscat(capbuffer,MsgToken(1600)); // DISABLED
        }
        _tcscat(capbuffer,_T(")")); // DISABLED
	wf->SetCaption(capbuffer);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpType"));
  if (wp) {
	if (airspace_copy.Flyzone()) {
	  _stprintf(buffer,TEXT("%s %s"), CAirspaceManager::GetAirspaceTypeText(airspace_copy.Type()), TEXT("FLY"));
/*
	  if( _tcsnicmp(  airspace_copy.Name(),   airspace_copy.TypeName() ,_tcslen(airspace_copy.TypeName())) == 0)
		_stprintf(buffer,TEXT("%s"),airspace_copy.Name());
	  else
	    _stprintf(buffer,TEXT("%s %s"),airspace_copy.TypeName()   // fixed strings max. 20
			                          ,airspace_copy.Name());     // NAME_SIZE          30   => max. 30 char
*/
	} else {
	  _stprintf(buffer,TEXT("%s %s"), TEXT("NOFLY"), CAirspaceManager::GetAirspaceTypeText(airspace_copy.Type()));
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
      wp->SetCaption(MsgToken(359));
    }
    if (hdist < 0) {
	  // LKTOKEN _@M1257_ "to leave"
	  _stprintf(buffer2, TEXT("%s %d%s %s"), buffer, iround(bearing), MsgToken(2179), MsgToken(1257));
    } else {
	  // LKTOKEN _@M1258_ "to enter"
	  _stprintf(buffer2, TEXT("%s %d%s %s"), buffer, iround(bearing), MsgToken(2179), MsgToken(1258));
	}
    wp->SetText(buffer2);
    wp->RefreshDisplay();
  }
#ifdef  RADIO_ACTIVE

  WindowControl* wFreq = wf->FindByName(TEXT("cmdSFrequency"));
  if (wFreq) {
    bool bRadio = false;

    if(RadioPara.Enabled) {

      double fASFrequency = ExtractFrequency((TCHAR*)airspace_copy.Name());
      if(fASFrequency >0) {

        WindowControl* wClose = wf->FindByName(TEXT("cmdClose"));
        if(wClose) {
          wClose->SetLeft(IBLSCALE(115));
          wClose->SetWidth(IBLSCALE(120));
        }

        wFreq->SetLeft(IBLSCALE(3));
        wFreq->SetWidth(IBLSCALE(110));

        _stprintf(buffer2,_T("%7.3fMHz"),fASFrequency);
        wFreq->SetCaption(buffer2);
        wFreq->Redraw();
        bRadio = true;
      }
    }
    wFreq->SetVisible(bRadio);
  }

#else
  WndProperty* wFreq = wf->FindByName(TEXT("cmdSFrequency"));
  if (wFreq) {
    wFreq->Hide();
  }
#endif  // RADIO_ACTIVE        
  // ONLY for DIAGNOSTICS- ENABLE ALSO XML
  #if 0
  wp = (WndProperty*)wf->FindByName(TEXT("prpWarnLevel"));
  if (wp) {
	  switch (airspace_copy.WarningLevel()) {
		default:
		  // LKTOKEN _@M765_ "Unknown"
		  wp->SetText(MsgToken(765));
		  break;
		  
		case awNone:
		  // LKTOKEN _@M479_ "None"
  		  wp->SetText(MsgToken(479));
		  break;

		case awYellow:
			// LKTOKEN _@M1255_ "YELLOW WARNING"
			wp->SetText(MsgToken(1255));
		  break;
		
		case awRed:
			// LKTOKEN _@M1256_ "RED WARNING"
			wp->SetText(MsgToken(1256));
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
            wp->SetText(MsgToken(765));
            break;
            
          case awNone:
            // LKTOKEN _@M479_ "None"
            wp->SetText(MsgToken(479));
            break;

          case awYellow:
              // LKTOKEN _@M1267_ "Yellow acknowledged"
              wp->SetText(MsgToken(1267));
            break;
          
          case awRed:
              // LKTOKEN _@M1268_ "Red acknowledged"
              wp->SetText(MsgToken(1268));
            break;

        }//sw
      } else {
          // LKTOKEN _@M1269_ "Disabled"
          wp->SetText(MsgToken(1269));
      }
	  wp->RefreshDisplay();
  }
  #endif

  wb = (WndButton*)wf->FindByName(TEXT("cmdFly"));
  if (wb) {
	if (airspace_copy.Flyzone()) {
	  // LKTOKEN _@M1271_ "NOFLY"
	  wb->SetCaption(MsgToken(1271));
	} else {
	  // LKTOKEN _@M1270_ "FLY"
	  wb->SetCaption(MsgToken(1270));
	}
	wb->Redraw();
  }

  wb = (WndButton*)wf->FindByName(TEXT("cmdSelect"));
  if (wb) {
	if (airspace_copy.Selected()) {
	  wb->SetCaption(MsgToken(1656)); // SELECTED!
	} else {
	  wb->SetCaption(MsgToken(1654)); // SELECT
	}
	wb->Redraw();
  }

  wb = (WndButton*)wf->FindByName(TEXT("cmdAcknowledge"));
  if (wb) {
    if (airspace_copy.Enabled()) {
      // LKTOKEN _@M1283_ "Disable"
      wb->SetCaption(MsgToken(1283));
    } else {
      // LKTOKEN _@M1282_ "Enable"
      wb->SetCaption(MsgToken(1282));
    }
    wb->Redraw();
  }

}

/*
 * only called by #CAirspaceManager::ProcessAirspaceDetailQueue()
 * for display AirspaceDetails, use #PopupAirspaceDetail
 */
void dlgAirspaceDetails() {

  WndForm* wf = dlgLoadFromXML(CallBackTable, IDR_XML_AIRSPACEDETAILS);

  if (!wf) return;
  wf->SetTimerNotify(1000, OnTimer);
  
  SetValues(wf);

  wf->ShowModal();

  delete wf;
  wf = NULL;

  return;
}

