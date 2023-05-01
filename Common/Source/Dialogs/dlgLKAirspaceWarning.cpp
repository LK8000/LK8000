/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgLKAirspaceWarning.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "InputEvents.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "LKInterface.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "RGB.h"
#include "Event/Event.h"
#include "Sound/Sound.h"
#include "resource.h"

CAirspaceBase airspace_copy;
AirspaceWarningMessage msg;
int timer_counter;

static void dlgLKAirspaceFill(WndForm* dlg);

static void OnPaintAirspacePicto(WndOwnerDrawFrame * Sender, LKSurface& Surface) {
    if (Sender) {
        Surface.SetBkColor(RGB_LIGHTGREY);
        /****************************************************************
         * for drawing the airspace pictorial, we need the original data.
         * copy contain only base class property, not geo data,
         * original data are shared ressources !
         * for that we need to grant all called methods are thread safe
         ****************************************************************/
        msg.originator->DrawPicto(Surface, Sender->GetClientRect());
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

static void OnAckForTimeClicked(WndButton* pWnd) {
  if (msg.originator == NULL) return;
  CAirspaceManager::Instance().AirspaceSetAckLevel(*msg.originator, msg.warnlevel);
  OnCloseClicked(pWnd);
}

static bool OnTimer(WndForm* pWnd){
  // Auto close dialog after some time
  if (!(--timer_counter)) {
    if(pWnd) {
      pWnd->SetModalResult(mrOK);
    }
    return true;
  }

  //Get a new copy with current values from airspacemanager
  if (msg.originator) {
	airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(msg.originator);
	dlgLKAirspaceFill(pWnd);
  }
  return true;
}

static bool OnKeyDown(WndForm* pWnd, unsigned KeyCode) {
    switch (KeyCode) {
        case KEY_RETURN:
            OnAckForTimeClicked(nullptr);
            return true;
        case KEY_ESCAPE:
            OnCloseClicked(nullptr);
            return true;
    }

    return false;
}


static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnAckForTimeClicked),
  ClickNotifyCallbackEntry(OnCloseClicked),
  OnPaintCallbackEntry(OnPaintAirspacePicto),
  EndCallBackEntry()
};


static void dlgLKAirspaceFill(WndForm* dlg)
{
  if(!dlg) {
    return;
  }

  if (msg.warnlevel != airspace_copy.WarningLevel()) {
    // we can automatically close the dialog when the warning level changes, probably new msg waiting in the queue
    dlg->SetModalResult(mrOK);
  }

    //Fill up dialog data
    WndProperty* wp;
    WndButton* wb;

    wp = (WndProperty*)dlg->FindByName(TEXT("prpReason"));
    if (wp) {
      switch (msg.event) {
        default:
          // Normally not show
          // LKTOKEN _@M765_ "Unknown"
          wp->SetText(MsgToken(765));
          break;

        case aweNone:
          // LKTOKEN _@M479_ "None"
            wp->SetText(MsgToken(479));
          break;

        case aweMovingInsideFly:
            // LKTOKEN _@M1242_ "Flying inside FLY zone"
            wp->SetText(MsgToken(1242));
          break;

        case awePredictedLeavingFly:
            // LKTOKEN _@M1243_ "Predicted leaving FLY zone"
            wp->SetText(MsgToken(1243));
          break;

        case aweNearOutsideFly:
            // LKTOKEN _@M1244_ "Near leaving FLY zone"
            wp->SetText(MsgToken(1244));
          break;

        case aweLeavingFly:
            // LKTOKEN _@M1245_ "Leaving FLY zone"
            wp->SetText(MsgToken(1245));
          break;

        case awePredictedEnteringFly:
            // LKTOKEN _@M1246_ "Predicted entering FLY zone"
            wp->SetText(MsgToken(1246));
          break;

        case aweEnteringFly:
            // LKTOKEN _@M1247_ "Entering FLY zone"
            wp->SetText(MsgToken(1247));
          break;

        case aweMovingOutsideFly:
            // LKTOKEN _@M1248_ "Flying outside FLY zone"
            wp->SetText(MsgToken(1248));
          break;


        // Events for NON-FLY zones
        case aweMovingOutsideNonfly:
            // LKTOKEN _@M1249_ "Flying outside NOFLY zone"
            wp->SetText(MsgToken(1249));
          break;

        case awePredictedEnteringNonfly:
            // LKTOKEN _@M1250_ "Predicted entering NOFLY zone"
            wp->SetText(MsgToken(1250));
          break;

        case aweNearInsideNonfly:
            // LKTOKEN _@M1251_ "Near entering NOFLY zone"
            wp->SetText(MsgToken(1251));
          break;

        case aweEnteringNonfly:
            // LKTOKEN _@M1252_ "Entering NOFLY zone"
            wp->SetText(MsgToken(1252));
          break;

        case aweMovingInsideNonfly:
            // LKTOKEN _@M1253_ "Flying inside NOFLY zone"
            wp->SetText(MsgToken(1253));
          break;

        case aweLeavingNonFly:
            // LKTOKEN _@M1254_ "Leaving NOFLY zone"
            wp->SetText(MsgToken(1254));
          break;

      }//sw
      switch (airspace_copy.WarningLevel()) {
        case awYellow:
            wp->SetBackColor(RGB_YELLOW);
            wp->SetForeColor(RGB_BLACK);
          break;
        case awRed:
            wp->SetBackColor(RGB_RED);
          break;
	default:
	  break;
     }
      wp->RefreshDisplay();
    }

#if 0 // unused but available for sometime
    wp = (WndProperty*)dlg->FindByName(TEXT("prpState"));
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
            wp->SetBackColor(RGB_YELLOW);
            wp->SetForeColor(RGB_BLACK);
          break;

        case awRed:
            // LKTOKEN _@M1256_ "RED WARNING"
            wp->SetText(MsgToken(1256));
            wp->SetBackColor(RGB_RED);
          break;
      }//sw
      wp->RefreshDisplay();
    }

    wp = (WndProperty*)dlg->FindByName(TEXT("prpName"));
    if (wp) {
      wp->SetText(airspace_copy.Name());
      wp->RefreshDisplay();
    }
#endif



    int hdist;
    int vdist;
    int bearing;
    bool inside;
    TCHAR stmp[21];



    TCHAR buffer[80];
    wp = (WndProperty*)dlg->FindByName(TEXT("prpType"));
    if (wp) {
	if (airspace_copy.Flyzone()) {
	  _stprintf(buffer,TEXT("%s %s"), TEXT("FLY"), airspace_copy.TypeName());
	} else {
	  _stprintf(buffer,TEXT("%s %s"), TEXT("NOFLY"), airspace_copy.TypeName());
	}

	  wp->SetText( buffer );
   //   wp->SetBackColor( airspace_copy.TypeColor());
   //  wp->SetForeColor( ContrastTextColor(airspace_copy.TypeColor()));
      wp->RefreshDisplay();
    }

    // Unfortunatelly virtual methods don't work on copied instances
    // we have to ask airspacemanager to perform the required calculations
    //inside = airspace_copy.CalculateDistance(&hdist, &bearing, &vdist);
    //inside = CAirspaceManager::Instance().AirspaceCalculateDistance(msg.originator, &hdist, &bearing, &vdist);
    bool distances_ready = airspace_copy.GetDistanceInfo(inside, hdist, bearing, vdist);

    wp = (WndProperty*)dlg->FindByName(TEXT("prpHDist"));
    if (wp) {
      TCHAR stmp2[40];
      if (distances_ready) {
        Units::FormatUserDistance((double)abs(hdist),stmp, 10);
        if (hdist<0) {
          // LKTOKEN _@M1257_ "to leave"
          _stprintf(stmp2, TEXT("%s %s"), stmp, MsgToken(1257));
        } else {
          // LKTOKEN _@M1258_ "to enter"
          _stprintf(stmp2,TEXT("%s %s"), stmp, MsgToken(1258));
        }
      } else {
        // no distance info calculated
        // LKTOKEN _@M1259_ "Too far, not calculated"
        _tcscpy(stmp2,MsgToken(1259));
      }
      wp->SetText(stmp2);
      wp->RefreshDisplay();
    }

    wp = (WndProperty*)dlg->FindByName(TEXT("prpVDist"));
    if (wp) {
        TCHAR stmp2[40];
      if (distances_ready) {
        Units::FormatUserAltitude((double)abs(vdist),stmp, 10);
        if (vdist<0) {
          // LKTOKEN _@M1260_ "below"
          _stprintf(stmp2,TEXT("%s %s"), stmp, MsgToken(1260));
        } else {
          // LKTOKEN _@M1261_ "above"
          _stprintf(stmp2,TEXT("%s %s"), stmp, MsgToken(1261));
        }
      } else {
        // no distance info calculated
        // LKTOKEN _@M1259_ "Too far, not calculated"
        _tcscpy(stmp2,MsgToken(1259));
      }
      wp->SetText(stmp2);
      wp->RefreshDisplay();
    }

    wp = (WndProperty*)dlg->FindByName(TEXT("prpTopAlt"));
    if (wp) {
      TCHAR stmp2[40];
      CAirspaceManager::Instance().GetAirspaceAltText(stmp2, 40, airspace_copy.Top());
      wp->SetText(stmp2);
      wp->RefreshDisplay();
    }

    wp = (WndProperty*)dlg->FindByName(TEXT("prpBaseAlt"));
    if (wp) {
      TCHAR stmp2[40];
      CAirspaceManager::Instance().GetAirspaceAltText(stmp2, 40, airspace_copy.Base());
      wp->SetText(stmp2);
      wp->RefreshDisplay();
    }

    wb = (WndButton*)dlg->FindByName(TEXT("cmdClose"));
    if (wb) {
      TCHAR stmp2[40];
      _stprintf(stmp2,TEXT("%s (%d)"), MsgToken(186), timer_counter);
      wb->SetCaption(stmp2);
    }

}

// Called periodically to show new airspace warning messages to user
// return 1 only for requesting run analysis
// This is called by WINMAIN thread, every second (1hz)
short ShowAirspaceWarningsToUser()
{

  if (msg.originator != NULL) return 0; // Warning in progress

  bool there_is_message = CAirspaceManager::Instance().PopWarningMessage(&msg);

  if (!there_is_message) return 0;        // no message to display

  airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(msg.originator);

  bool ackdialog_required = false;
  TCHAR msgbuf[128];

  // which message we need to show?
  switch (msg.event) {
    default:
      // normally not show
      DoStatusMessage(TEXT("Unknown airspace warning message"));
      break;    //Unknown msg type

    case aweNone:
    case aweMovingInsideFly:            // normal, no msg, normally this msg type shouldn't get here
    case awePredictedEnteringFly:       // normal, no msg, normally this msg type shouldn't get here
    case aweMovingOutsideNonfly:        // normal, no msg, normally this msg type shouldn't get here
      break;

    case awePredictedLeavingFly:
    case aweNearOutsideFly:
    case aweLeavingFly:
    case awePredictedEnteringNonfly:
    case aweNearInsideNonfly:
    case aweEnteringNonfly:
    case aweMovingInsideNonfly:             // repeated messages
    case aweMovingOutsideFly:               // repeated messages
      ackdialog_required = true;
      break;

    case aweEnteringFly:
      // LKTOKEN _@M1240_ "Entering"
	  if( _tcsnicmp(  airspace_copy.Name(),   airspace_copy.TypeName() ,_tcslen(airspace_copy.TypeName())) == 0)
		_stprintf(msgbuf,TEXT("%s %s"),MsgToken(1240),airspace_copy.Name());
	  else
		_stprintf(msgbuf,TEXT("%s %s %s"),MsgToken(1240),airspace_copy.TypeName(),airspace_copy.Name());
      DoStatusMessage(msgbuf);
      break;

    case aweLeavingNonFly:
      // LKTOKEN _@M1241_ "Leaving"
      if(!airspace_copy.Acknowledged() )  // don't warn on leaving acknolaged airspaces
      {
        if( _tcsnicmp(  airspace_copy.Name(),   airspace_copy.TypeName() ,_tcslen(airspace_copy.TypeName())) == 0)
          _stprintf(msgbuf,TEXT("%s %s"),MsgToken(1241),airspace_copy.Name());
        else
          _stprintf(msgbuf,TEXT("%s %s %s"),MsgToken(1241),airspace_copy.TypeName(),airspace_copy.Name());
        DoStatusMessage(msgbuf);
      }
      break;

  }


  // show dialog to user if needed
  if (ackdialog_required && (airspace_copy.WarningLevel() == msg.warnlevel)) {
    WndForm * dlg = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_LKAIRSPACEWARNING_L : IDR_XML_LKAIRSPACEWARNING_P);
    if (dlg==NULL) {
      StartupStore(_T("------ LKAirspaceWarning setup FAILED!%s"),NEWLINE); //@ 101027
      return 0;
    }

    dlg->SetKeyDownNotify(OnKeyDown);
    dlg->SetTimerNotify(1000, OnTimer);
    timer_counter = AirspaceWarningDlgTimeout;                    // Auto closing dialog in x secs

    WndButton *wb = (WndButton*)dlg->FindByName(TEXT("cmdAckForTime"));
    if (wb) {
      TCHAR stmp2[40];
      _stprintf(stmp2,TEXT("%s (%dmin)"), MsgToken(46), AcknowledgementTime/60);
      wb->SetCaption(stmp2);
    }

    dlgLKAirspaceFill(dlg);

    LKSound(_T("LK_AIRSPACE.WAV")); // 100819

    if( _tcsnicmp( airspace_copy.Name(),   airspace_copy.TypeName() ,_tcslen(airspace_copy.TypeName())) == 0) {
			_stprintf(msgbuf,TEXT("%s"),airspace_copy.Name());
    } else {
		    _stprintf(msgbuf,TEXT("%s %s"),airspace_copy.TypeName(),airspace_copy.Name());
    }
    dlg->SetCaption(msgbuf);
    dlg->ShowModal();

    delete dlg;
    dlg = NULL;
  }

  msg.originator = NULL;

  // If we clicked on Analysis button, we shall return 1 and the calling function will
  // detect and take care of it.
  // 120128 unused, we call directly eventSetup
  return 1;
}
