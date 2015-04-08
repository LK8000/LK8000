/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgHelp.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "InputEvents.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "utils/TextWrapArray.h"


static WndForm *wf=NULL;
static WndListFrame *wHelp=NULL;
static WndOwnerDrawFrame *wHelpEntry = NULL;

static int DrawListIndex=0;

static TextWrapArray aTextLine;

static void InitHelp(void) {
  wf=(WndForm *)NULL;
  wHelp=(WndListFrame *)NULL;
  wHelpEntry = (WndOwnerDrawFrame *)NULL;
  DrawListIndex=0;
  
  aTextLine.clear();
}



static void OnCloseClicked(WndButton* pWnd) {
    wf->SetModalResult(mrOK);
}


static void OnPaintDetailsListItem(WindowControl * Sender, LKSurface& Surface){
  (void)Sender;
  if (DrawListIndex < (int)aTextLine.size()){
      LKASSERT(DrawListIndex>=0);
      const TCHAR* szText = aTextLine[DrawListIndex];
      Surface.SetTextColor(RGB_BLACK);
      Surface.DrawText(2*ScreenScale, 2*ScreenScale, szText, _tcslen(szText));
  }
}


static void OnDetailsListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
        (void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = aTextLine.size();
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
  }
}



static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnCloseClicked),
  OnPaintCallbackEntry(OnPaintDetailsListItem),
  OnListCallbackEntry(OnDetailsListInfo),
  EndCallBackEntry()
};

void dlgHelpShowModal(const TCHAR* Caption, const TCHAR* HelpText) {
  if (!Caption || !HelpText) {
    return;
  }
  InitHelp();
  if (!ScreenLandscape) {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgHelp_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename,
                        TEXT("IDR_XML_HELP_L"));
  } else {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgHelp.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        TEXT("IDR_XML_HELP"));
  }
  LKASSERT(wf);
  if (!wf) goto _getout;

  TCHAR fullcaption[100];
  _stprintf(fullcaption,TEXT("%s: %s"), gettext(TEXT("_@M336_")), Caption); // Help
  wf->SetCaption(fullcaption);

  wHelp = (WndListFrame*)wf->FindByName(TEXT("frmDetails"));
  wHelpEntry = (WndOwnerDrawFrame *)NULL;
  DrawListIndex=0;

  LKASSERT(wHelp!=NULL);
  if (!wHelp) goto _getout;

  wHelp->SetBorderKind(BORDERLEFT);
  wHelp->SetWidth(wf->GetWidth() - wHelp->GetLeft()-2);

  wHelpEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmDetailsEntry"));
  LKASSERT(wHelpEntry);
  if (!wHelpEntry) goto _getout;
  wHelpEntry->SetCanFocus(true);

  // ScrollbarWidth is initialised from DrawScrollBar in WindowControls, so it might not be ready here
  if ( wHelp->ScrollbarWidth == -1) {
    #if defined (PNA)
    #define SHRINKSBFACTOR 1.0 // shrink width factor.  Range .1 to 1 where 1 is very "fat"
    #else
    #define SHRINKSBFACTOR 0.75  // shrink width factor.  Range .1 to 1 where 1 is very "fat"
    #endif
    wHelp->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);
  }
  wHelpEntry->SetWidth(wHelp->GetWidth() - wHelp->ScrollbarWidth - 5);
  {
    LKWindowSurface Surface(*wHelpEntry);
    Surface.SelectObject(wHelpEntry->GetFont());
    aTextLine.update(Surface, wHelpEntry->GetWidth(), LKgethelptext(HelpText));
  }

  wHelp->ResetList();
  wHelp->Redraw();
  wf->ShowModal();
  delete wf;
  
  aTextLine.clear();
  

_getout:
  wf = NULL;

}


