/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"
#include "externs.h"
#include "Units.h"
#include "InputEvents.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"

#include "utils/heapcheck.h"

extern HWND   hWndMainWindow;
static WndForm *wf=NULL;


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


void dlgHelpShowModal(const TCHAR* Caption, const TCHAR* HelpText) {
  if (!Caption || !HelpText) {
    return;
  }

  if (!ScreenLandscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgHelp_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename,
                        hWndMainWindow,
                        TEXT("IDR_XML_HELP_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgHelp.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_HELP"));
  }
  WndProperty* wp;

  if (wf) {

    TCHAR fullcaption[100];
    _stprintf(fullcaption,TEXT("%s: %s"), gettext(TEXT("_@M336_")), Caption); // Help

    wf->SetCaption(fullcaption);

    wp = (WndProperty*)wf->FindByName(TEXT("prpHelpText"));
    if (wp) {
      wp->SetText(LKgethelptext(HelpText));
      wp->RefreshDisplay();
    }
    wf->ShowModal();
    delete wf;
  }
  wf = NULL;

}


