/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"
#include <aygshell.h>

#include "InfoBoxLayout.h"

#include "externs.h"
#include "Units.h"
#include "MapWindow.h"

#include "dlgTools.h"
#include "criticalsection.h"
#include "LKAirspace.h"

extern HWND   hWndMainWindow;
extern HWND   hWndMapWindow;
CAirspace *airspace;
CAirspace airspace_copy;
AirspaceWarningEvent msg_reason;

WndForm *dlg=NULL;

void dlgLKAirspaceFill();

static void OnAckForTimeClicked(WindowControl * Sender)
{
  (void)Sender;
  if (airspace==NULL) return;
  if (dlg==NULL) return;
  CAirspaceManager::Instance().AirspaceWarnListAckForTime(*airspace);
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
  //Get a new copy from airspacemanager
  if (airspace == NULL) return 0;
  airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(airspace);
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
#ifdef GNAV
    case VK_APP1:
    case '6':
      OnAckClicked(Sender);
      return(0);
#endif
  }

  return(1);
  
}


TCHAR* getAirspaceTypeText(int Type)
{
  switch (Type)
    {
    case RESTRICT:
      return(TEXT("LxR"));
    case PROHIBITED:
      return(TEXT("LxP"));
    case DANGER:
      return(TEXT("LxD"));
    case CLASSA:
      return(TEXT("A"));
    case CLASSB:
      return(TEXT("B"));
    case CLASSC:
      return(TEXT("C"));
    case CLASSD:
      return(TEXT("D"));
    case CLASSE:
      return(TEXT("E"));
    case CLASSF:
      return(TEXT("F"));
    case CLASSG:
      return(TEXT("G"));
    case NOGLIDER:
      return(TEXT("NoGld"));
    case CTR:
      return(TEXT("CTR"));
    case WAVE:
      return(TEXT("Wav"));
    default:
      return(TEXT("?"));
    }
}




static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnAckForTimeClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


void FillStateDisplay(WndProperty* wp, AirspaceWarningEvent ev)
{
	if (wp) {
	  //wp->SetFont(TempMapWindowFont);
	  switch (ev) {
		default:
		  wp->SetText(TEXT("Unknown"));
		  break;
		  
		case aweNone:
  		  wp->SetText(TEXT("None"));
		  // we can automatically close the dialog when the warning msg gone
		  // dlg->SetModalResult(mrOK);
		  // this is not too good, because the dialog can disappear before the user can touch the screen,
		  // or worst he presses a button on another airspace warning dialog appearing after this
		  break;

		case aweMovingInsideFly:
			wp->SetText(TEXT("aweMovingInsideFly"));
		  break;
		
		case awePredictedLeavingFly:
			wp->SetText(TEXT("awePredictedLeavingFly"));
		  break;
		  
		case aweLeavingFly:
			wp->SetText(TEXT("aweLeavingFly"));
		  break;

		case awePredictedEnteringFly:
			wp->SetText(TEXT("awePredictedEnteringFly"));
		  break;
		  
		case aweEnteringFly:
			wp->SetText(TEXT("aweEnteringFly"));
		  break;
		  
		case aweMovingOutsideFly:
			wp->SetText(TEXT("aweMovingOutsideFly"));
		  break;
		  
				
		// Events for NON-FLY zones
		case aweMovingOutsideNonfly:
			wp->SetText(TEXT("aweMovingOutsideNonfly"));
		  break;
		  
		case awePredictedEnteringNonfly:
			wp->SetText(TEXT("awePredictedEnteringNonfly"));
		  break;
		  
		case aweEnteringNonfly:
			wp->SetText(TEXT("aweEnteringNonfly"));
		  break;

		case aweMovingInsideNonfly:
			wp->SetText(TEXT("aweMovingInsideNonfly"));
		  break;

		case aweLeavingNonFly:
			wp->SetText(TEXT("aweLeavingNonFly"));
		  break;

	  }//sw
	  wp->RefreshDisplay();
	}
}

void dlgLKAirspaceFill()
{
	//Fill up dialog data
	WndProperty* wp;	
	
	wp = (WndProperty*)dlg->FindByName(TEXT("prpReason"));
	FillStateDisplay(wp, msg_reason);

	wp = (WndProperty*)dlg->FindByName(TEXT("prpState"));
	FillStateDisplay(wp, airspace_copy.WarningMsg());

	  
	wp = (WndProperty*)dlg->FindByName(TEXT("prpName"));
	if (wp) {
	  wp->SetText(airspace_copy.Name());
	  wp->RefreshDisplay();
	}	

	int hdist;
	int vdist;
	int bearing;
	bool inside;
	TCHAR stmp[21];
	
	inside = airspace_copy.CalculateDistance(&hdist, &bearing, &vdist);
	
	wp = (WndProperty*)dlg->FindByName(TEXT("prpHDist"));
	if (wp) {
/*	  if (inside) {
		_tcsncpy(stmp,gettext(TEXT("_@M359_")),20);
	  } else {
	  }*/
	  Units::FormatUserDistance(fabs(hdist),stmp, 10);
	  wp->SetText(stmp);
	  wp->RefreshDisplay();
	}	

	wp = (WndProperty*)dlg->FindByName(TEXT("prpVDist"));
	if (wp) {
	  Units::FormatUserAltitude(fabs(vdist),stmp, 10);
	  TCHAR stmp2[40];
	  if (vdist<0) {
		wsprintf(stmp2,TEXT("below %s"), stmp);
	  } else {
		wsprintf(stmp2,TEXT("above %s"), stmp);
	  }
	  wp->SetText(stmp2);
	  wp->RefreshDisplay();
	}	

}

// Called periodically to show new airspace warning messages to user
void ShowAirspaceWarningsToUser()
{
  if (airspace != NULL) return;		// Dialog already open

  airspace = CAirspaceManager::Instance().PopWarningMessagedAirspace();
  if (airspace == NULL) return;		// no message to display

  airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(airspace);
  msg_reason = airspace_copy.WarningMsg();

  bool ackdialog_required = false;
  TCHAR msgbuf[128];

  // which message we need to show?
  switch (msg_reason) {
	default:
	  DoStatusMessage(TEXT("Unknown airspace warning message"));
	  break;	//Unknown msg type

	case aweNone:
	  break;
	  
	case awePredictedLeavingFly:
	case aweLeavingFly:
	case awePredictedEnteringNonfly:
	case aweEnteringNonfly:
	case aweMovingInsideNonfly:
	  ackdialog_required = true;
	  break;
	  
	case aweEnteringFly:
	  wsprintf(msgbuf, TEXT("Entering FLY %s ZONE %s"), getAirspaceTypeText(airspace_copy.Type()), airspace_copy.Name());
	  DoStatusMessage(msgbuf);
	  break;

	case aweLeavingNonFly:
	  wsprintf(msgbuf, TEXT("Leaving NON-FLY %s ZONE %s"), getAirspaceTypeText(airspace_copy.Type()), airspace_copy.Name());
	  DoStatusMessage(msgbuf);
	  break;
	  
  }

  if (ackdialog_required) {
	if (!InfoBoxLayout::landscape)
	  dlg = dlgLoadFromXML(CallBackTable, NULL, hWndMainWindow, TEXT("IDR_XML_LKAIRSPACEWARNING_L"));
	else
	  dlg = dlgLoadFromXML(CallBackTable, NULL, hWndMainWindow, TEXT("IDR_XML_LKAIRSPACEWARNING"));

	if (dlg==NULL) {
	  StartupStore(_T("------ LKAirspaceWarning setup FAILED!%s"),NEWLINE); //@ 101027
	  return;
	}
	
	dlg->SetKeyDownNotify(OnKeyDown);
    dlg->SetTimerNotify(OnTimer);


	dlgLKAirspaceFill();

	#ifndef DISABLEAUDIO
	if (EnableSoundModes) LKSound(_T("LK_AIRSPACE.WAV")); // 100819
	#endif

	dlg->ShowModal();

	delete dlg;
	dlg = NULL;
  }
  
  airspace = NULL;
  return;
}



