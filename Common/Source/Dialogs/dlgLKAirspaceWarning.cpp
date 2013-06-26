/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgLKAirspaceWarning.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include <aygshell.h>

#include "InfoBoxLayout.h"
#include "InputEvents.h"
#include "Dialogs.h"
#include "LKInterface.h"
#include "TraceThread.h"

#include "RGB.h"

extern HWND   hWndMainWindow;
extern HWND   hWndMapWindow;
CAirspace airspace_copy;
AirspaceWarningMessage msg;
int timer_counter;

WndForm *dlg=NULL;

void dlgLKAirspaceFill();

static void OnPaintAirspacePicto(WindowControl * Sender, HDC hDC){
	  (void)Sender;
	  RECT *prc;

	  WndFrame  *wPicto = ((WndFrame *)dlg->FindByName(TEXT("frmAirspacePicto")));
	  prc = wPicto->GetBoundRect();


	  SetBkColor  (hDC, RGB_LIGHTGREY);
	//  airspace_copy.DrawPicto(hDC, *prc, true);
      /*************************************************************
       * @Paolo
       * for drawing the airspace pictorial, we need the original data.
       * seems that the copy does not contain geo data
       * Do we really need to work with the copy?
       * works fine with the origin airspace
       ************************************************************/
	  msg.originator->DrawPicto(hDC, *prc, true);


}

COLORREF ContrastTextColor(COLORREF Col)
{
//  human eye brightness color factors
//	Y=0.30 R + 0.59 G + 0.11 B.
	double  Brightness = 0.30 *(double)((Col & 0xFF0000) >> 16);
	Brightness   +=      0.59 *(double)((Col & 0x00FF00) >> 8 );
	Brightness   +=      0.11 *(double)((Col & 0x0000FF)      );
	if(  Brightness > 127.0)
	  return( RGB_BLACK);
	else
	  return( RGB_WHITE);
}

static void OnAckForTimeClicked(WindowControl * Sender)
{
  (void)Sender;
  if (dlg == NULL) return;
  if (msg.originator == NULL) return;
  CAirspaceManager::Instance().AirspaceSetAckLevel(*msg.originator, msg.warnlevel);
  dlg->SetModalResult(mrOK);
}

static void OnCloseClicked(WindowControl * Sender)
{
  (void)Sender;
  if (dlg==NULL) return;
  dlg->SetModalResult(mrOK);
}

static int OnTimer(WindowControl * Sender){
  (void)Sender;
  
  // Timer events comes at 500ms, we need every second
  static bool timer_divider = false;
  timer_divider = !timer_divider;
  if (timer_divider) return 0;
  
  // Auto close dialog after some time
  if (!(--timer_counter)) {
    dlg->SetModalResult(mrOK);
    return 0;
  }
  
  //Get a new copy with current values from airspacemanager
  if (msg.originator == NULL) return 0;
  airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(msg.originator);
  dlgLKAirspaceFill();
  return(0);
}

static int OnKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam)
{
  (void)lParam;
    switch(wParam){
    case VK_RETURN:
      OnAckForTimeClicked(Sender);
      return(0);
    case VK_ESCAPE:
      OnCloseClicked(Sender);
      return(0);
  }

  return(1);
  
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnAckForTimeClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnPaintAirspacePicto),
  DeclareCallBackEntry(NULL)
};


void dlgLKAirspaceFill()
{
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
          wp->SetText(gettext(TEXT("_@M765_")));
          break;
          
        case aweNone:
          // LKTOKEN _@M479_ "None"
            wp->SetText(gettext(TEXT("_@M479_")));
          break;

        case aweMovingInsideFly:
            // LKTOKEN _@M1242_ "Flying inside FLY zone"
            wp->SetText(gettext(TEXT("_@M1242_")));
          break;
        
        case awePredictedLeavingFly:
            // LKTOKEN _@M1243_ "Predicted leaving FLY zone"
            wp->SetText(gettext(TEXT("_@M1243_")));
          break;
        
        case aweNearOutsideFly:
            // LKTOKEN _@M1244_ "Near leaving FLY zone"
            wp->SetText(gettext(TEXT("_@M1244_")));
          break;
          
        case aweLeavingFly:
            // LKTOKEN _@M1245_ "Leaving FLY zone"
            wp->SetText(gettext(TEXT("_@M1245_")));
          break;

        case awePredictedEnteringFly:
            // LKTOKEN _@M1246_ "Predicted entering FLY zone"
            wp->SetText(gettext(TEXT("_@M1246_")));
          break;
          
        case aweEnteringFly:
            // LKTOKEN _@M1247_ "Entering FLY zone"
            wp->SetText(gettext(TEXT("_@M1247_")));
          break;
          
        case aweMovingOutsideFly:
            // LKTOKEN _@M1248_ "Flying outside FLY zone"
            wp->SetText(gettext(TEXT("_@M1248_")));
          break;
          
                
        // Events for NON-FLY zones
        case aweMovingOutsideNonfly:
            // LKTOKEN _@M1249_ "Flying outside NOFLY zone"
            wp->SetText(gettext(TEXT("_@M1249_")));
          break;
          
        case awePredictedEnteringNonfly:
            // LKTOKEN _@M1250_ "Predicted entering NOFLY zone"
            wp->SetText(gettext(TEXT("_@M1250_")));
          break;

        case aweNearInsideNonfly:
            // LKTOKEN _@M1251_ "Near entering NOFLY zone"
            wp->SetText(gettext(TEXT("_@M1251_")));
          break;

        case aweEnteringNonfly:
            // LKTOKEN _@M1252_ "Entering NOFLY zone"
            wp->SetText(gettext(TEXT("_@M1252_")));
          break;

        case aweMovingInsideNonfly:
            // LKTOKEN _@M1253_ "Flying inside NOFLY zone"
            wp->SetText(gettext(TEXT("_@M1253_")));
          break;

        case aweLeavingNonFly:
            // LKTOKEN _@M1254_ "Leaving NOFLY zone"
            wp->SetText(gettext(TEXT("_@M1254_")));
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
          wp->SetText(gettext(TEXT("_@M765_")));
          break;
          
        case awNone:
          // LKTOKEN _@M479_ "None"
            wp->SetText(gettext(TEXT("_@M479_")));
          break;

        case awYellow:
            // LKTOKEN _@M1255_ "YELLOW WARNING"
            wp->SetText(gettext(TEXT("_@M1255_")));
            wp->SetBackColor(RGB_YELLOW);
            wp->SetForeColor(RGB_BLACK);
          break;
        
        case awRed:
            // LKTOKEN _@M1256_ "RED WARNING"
            wp->SetText(gettext(TEXT("_@M1256_")));
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
  	  wsprintf(buffer,TEXT("%s %s"), gettext(TEXT("FLY")), airspace_copy.TypeName());
  	} else {
  	  wsprintf(buffer,TEXT("%s %s"), gettext(TEXT("NOFLY")), airspace_copy.TypeName());
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
          wsprintf(stmp2, TEXT("%s %s"), stmp, gettext(TEXT("_@M1257_")));
        } else {
          // LKTOKEN _@M1258_ "to enter"
          wsprintf(stmp2,TEXT("%s %s"), stmp, gettext(TEXT("_@M1258_")));
        }
      } else {
        // no distance info calculated
        // LKTOKEN _@M1259_ "Too far, not calculated"
        wsprintf(stmp2,gettext(TEXT("_@M1259_")));
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
          wsprintf(stmp2,TEXT("%s %s"), stmp, gettext(TEXT("_@M1260_")));
        } else {
          // LKTOKEN _@M1261_ "above"
          wsprintf(stmp2,TEXT("%s %s"), stmp, gettext(TEXT("_@M1261_")));
        }
      } else {
        // no distance info calculated
        // LKTOKEN _@M1259_ "Too far, not calculated"
        wsprintf(stmp2,gettext(TEXT("_@M1259_")));
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
      wsprintf(stmp2,TEXT("%s (%d)"), gettext(TEXT("_@M186_")), timer_counter);
      wb->SetCaption(stmp2);
    }    

}

// Called periodically to show new airspace warning messages to user
// return 1 only for requesting run analysis
// This is called by WINMAIN thread, every second (1hz)
short ShowAirspaceWarningsToUser()
{

  if (msg.originator != NULL) return 0;        // Dialog already open


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
		_stprintf(msgbuf,TEXT("%s %s"),gettext(TEXT("_@M1240_")),airspace_copy.Name());
	  else
		_stprintf(msgbuf,TEXT("%s %s %s"),gettext(TEXT("_@M1240_")),airspace_copy.TypeName(),airspace_copy.Name());
//    wsprintf(msgbuf, TEXT("%s %s %s "), gettext(TEXT("_@M1240_")),airspace_copy.TypeName(), airspace_copy.Name());
      DoStatusMessage(msgbuf);
      break;

    case aweLeavingNonFly:
      // LKTOKEN _@M1241_ "Leaving"
  	  if( _tcsnicmp(  airspace_copy.Name(),   airspace_copy.TypeName() ,_tcslen(airspace_copy.TypeName())) == 0)
  		_stprintf(msgbuf,TEXT("%s %s"),gettext(TEXT("_@M1241_")),airspace_copy.Name());
  	  else
  		_stprintf(msgbuf,TEXT("%s %s %s"),gettext(TEXT("_@M1241_")),airspace_copy.TypeName(),airspace_copy.Name());
//      wsprintf(msgbuf, TEXT("%s %s %s"), gettext(TEXT("_@M1241_")),airspace_copy.TypeName(), airspace_copy.Name());
      DoStatusMessage(msgbuf);
      break;
      
  }


  // show dialog to user if needed
  if (ackdialog_required && (airspace_copy.WarningLevel() == msg.warnlevel)) {
    if (!ScreenLandscape)
      dlg = dlgLoadFromXML(CallBackTable, NULL, hWndMainWindow, TEXT("IDR_XML_LKAIRSPACEWARNING_L"));
    else
      dlg = dlgLoadFromXML(CallBackTable, NULL, hWndMainWindow, TEXT("IDR_XML_LKAIRSPACEWARNING"));

    if (dlg==NULL) {
      StartupStore(_T("------ LKAirspaceWarning setup FAILED!%s"),NEWLINE); //@ 101027
      return 0;
    }
    
    dlg->SetKeyDownNotify(OnKeyDown);
    dlg->SetTimerNotify(OnTimer);
    timer_counter = AirspaceWarningDlgTimeout;                    // Auto closing dialog in x secs

    WndButton *wb = (WndButton*)dlg->FindByName(TEXT("cmdAckForTime"));
    if (wb) {
      TCHAR stmp2[40];
      wsprintf(stmp2,TEXT("%s (%dmin)"), gettext(TEXT("_@M46_")), AcknowledgementTime/60);
      wb->SetCaption(stmp2);
    }    

    dlgLKAirspaceFill();

    #ifndef DISABLEAUDIO
    if (EnableSoundModes) LKSound(_T("LK_AIRSPACE.WAV")); // 100819
    #endif
		  if( _tcsnicmp( airspace_copy.Name(),   airspace_copy.TypeName() ,_tcslen(airspace_copy.TypeName())) == 0)
			_stprintf(msgbuf,TEXT("%s"),airspace_copy.Name());
		  else
		    _stprintf(msgbuf,TEXT("%s %s"),airspace_copy.TypeName(),airspace_copy.Name());
//    _stprintf(msgbuf,_T("%s: %s %s"),gettext(_T("_@M68_")),airspace_copy.TypeName(),airspace_copy.Name());
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


