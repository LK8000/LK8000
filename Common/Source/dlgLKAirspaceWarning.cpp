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

extern HWND   hWndMainWindow;
extern HWND   hWndMapWindow;
CAirspace *airspace;
CAirspace airspace_copy;

WndForm *dlg=NULL;


static void OnAckClicked(WindowControl * Sender)
{
  (void)Sender;
  if (airspace==NULL) return;
  if (dlg==NULL) return;
  CAirspaceManager::Instance().AirspaceWarnListAckWarn(*airspace);
  dlg->SetModalResult(mrOK);
}

static void OnCloseClicked(WindowControl * Sender)
{
  (void)Sender;
  if (dlg==NULL) return;
  dlg->SetModalResult(mrOK);
}


static int OnKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam)
{
  (void)lParam;
	switch(wParam){
    case VK_RETURN:
      OnAckClicked(Sender);
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
  DeclareCallBackEntry(OnAckClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


void dlgLKAirspaceFill(AirspaceWarningMessageType msgtype)
{
	//Fill up dialog data
	WndProperty* wp;	
	
	wp = (WndProperty*)dlg->FindByName(TEXT("prpReason"));
	if (wp) {
	  //wp->SetFont(TempMapWindowFont);
	  switch (msgtype) {
		default:
		  wp->SetCaption(TEXT("Unknown"));
		  break;
		case awmNone:
		  wp->SetCaption(TEXT("None"));
		  break;
		case awmYellow:
		  wp->SetCaption(TEXT("Yellow"));
		  break;
		case awmYellowRepeated:
		  wp->SetCaption(TEXT("Yellow repeat"));
		  break;
		case awmRed:
		  wp->SetCaption(TEXT("Red"));
		  break;
		case awmRedRepeated:
		  wp->SetCaption(TEXT("Red repeat"));
		  break;
	  }//sw
	  wp->RefreshDisplay();

	  
	  wp = (WndProperty*)dlg->FindByName(TEXT("prpName"));
	  if (wp) {
		wp->SetText(airspace_copy.Name());
		wp->RefreshDisplay();
	  }	
	}
  
}

// Called periodically to show airspace warning messages to user
void ShowAirspaceWarningsToUser()
{
  if (airspace != NULL) return;		// Dialog already open

  AirspaceWarningMessage msg;
  bool there_is_msg_to_show;
  there_is_msg_to_show = CAirspaceManager::Instance().PopWarningMessage(&msg);
  if (!there_is_msg_to_show) return;		// no message to display

  airspace = msg.originator;
  airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(airspace);

  bool ackdialog_required = false;
  TCHAR msgbuf[128];

  // which message we need to show?
  switch (msg.msgtype) {
	default:
	  DoStatusMessage(TEXT("Unknown airspace warning message"));
	  break;	//Unknown msg type
	  
	case awmYellow:
	case awmYellowRepeated:
	case awmRed:
	case awmRedRepeated:
	  ackdialog_required = true;
	  break;
	  
	case awmEnteringFly:
	  wsprintf(msgbuf, TEXT("Entering FLY %s ZONE %s"), getAirspaceTypeText(airspace_copy.Type()), airspace_copy.Name());
	  DoStatusMessage(msgbuf);
	  break;

	case awmLeavingNonFly:
	  wsprintf(msgbuf, TEXT("Leaving NON-FLY %s ZONE %s"), getAirspaceTypeText(airspace_copy.Type()), airspace_copy.Name());
	  DoStatusMessage(msgbuf);
	  break;
	  
	case awmEnteringAckedNonFly:
	  wsprintf(msgbuf, TEXT("Entering acked NON-FLY %s ZONE %s"), getAirspaceTypeText(airspace_copy.Type()), airspace_copy.Name());
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


	dlgLKAirspaceFill(msg.msgtype);

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



