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
WndForm *dlg=NULL;


static void OnAckClicked(WindowControl * Sender)
{
  (void)Sender;
  if (airspace==NULL) return;
  CAirspaceManager::Instance().AirspaceWarnListAckWarn(*airspace);
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


static void getAirspaceType(TCHAR *buf, int Type){
  switch (Type)
    {
    case RESTRICT:
      _tcscpy(buf, TEXT("LxR"));
      return;
    case PROHIBITED:
      _tcscpy(buf, TEXT("LxP"));
      return;
    case DANGER:
      _tcscpy(buf, TEXT("LxD"));
      return;
    case CLASSA:
      _tcscpy(buf, TEXT("A"));
      return;
    case CLASSB:
      _tcscpy(buf, TEXT("B"));
      return;
    case CLASSC:
      _tcscpy(buf, TEXT("C"));
      return;
    case CLASSD:
      _tcscpy(buf, TEXT("D"));
      return;
    case CLASSE:
      _tcscpy(buf, TEXT("E"));
      return;
    case CLASSF:
      _tcscpy(buf, TEXT("F"));
      return;
    case CLASSG:
      _tcscpy(buf, TEXT("G"));
      return;
    case NOGLIDER:
      _tcscpy(buf, TEXT("NoGld"));
      return;
    case CTR:
      _tcscpy(buf, TEXT("CTR"));
      return;
    case WAVE:
      _tcscpy(buf, TEXT("Wav"));
      return;
    default:
      _tcscpy(buf, TEXT("?"));
      return;
    }
}




static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnAckClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};

  

void CAirspaceManager::ShowWarningsToUser()
{
 
  if (airspace != NULL) return;		// Dialog already open
  
  if (1) {
	CCriticalSection::CGuard guard(_csuser_warning_queue);
	if (_user_warning_queue.size()==0) return;
  }
  
  if (InfoBoxLayout::landscape)
	dlg = dlgLoadFromXML(CallBackTable, NULL, hWndMainWindow, TEXT("IDR_XML_LKAIRSPACEWARNING_L"));
  else
	dlg = dlgLoadFromXML(CallBackTable, NULL, hWndMainWindow, TEXT("IDR_XML_LKAIRSPACEWARNING"));

  if (dlg==NULL) {
	StartupStore(_T("------ LKAirspaceWarning setup FAILED!%s"),NEWLINE); //@ 101027
	return;
  }
  
  dlg->SetKeyDownNotify(OnKeyDown);
 
	if (1) {
	  CCriticalSection::CGuard guard(_csuser_warning_queue);
	  airspace = _user_warning_queue.front();
	  _user_warning_queue.pop_front();
	  //Fill up dialog data
	  WndProperty* wp;	
	  wp = (WndProperty*)dlg->FindByName(TEXT("prpName"));
	  if (wp) {
		wp->SetText(airspace->Name());
		wp->RefreshDisplay();
	  }	
	}

    #ifndef DISABLEAUDIO
    if (EnableSoundModes) LKSound(_T("LK_AIRSPACE.WAV")); // 100819
    #endif
    dlg->ShowModal();
	
  delete dlg;
  dlg = NULL;
  airspace = NULL;

  return;
}



