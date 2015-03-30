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


#define MAXLINES 100 // max number of lines for an help text
static WndForm *wf=NULL;
static WndListFrame *wHelp=NULL;
static WndOwnerDrawFrame *wHelpEntry = NULL;
static TCHAR thisHelp[1000];
static int DrawListIndex=0;
static int LineOffsets[MAXLINES];
static int nTextLines=0;

static void InitHelp(void) {
  wf=(WndForm *)NULL;
  wHelp=(WndListFrame *)NULL;
  wHelpEntry = (WndOwnerDrawFrame *)NULL;
  DrawListIndex=0;
  nTextLines=0;
  _tcscpy(thisHelp,_T(""));
  for (int i=0; i<MAXLINES; i++) {
        LineOffsets[i]=0;
  }
}



static void OnCloseClicked(WndButton* pWnd) {
    wf->SetModalResult(mrOK);
}


static void OnPaintDetailsListItem(WindowControl * Sender, LKSurface& Surface){
  (void)Sender;
  if (DrawListIndex < nTextLines){
    TCHAR* text = thisHelp; 
    int nstart = LineOffsets[DrawListIndex];
    int nlen;
    if (DrawListIndex<nTextLines-1) {
      nlen = LineOffsets[DrawListIndex+1]-LineOffsets[DrawListIndex]-1;
      nlen--;
    } else {
      nlen = _tcslen(text+nstart);
    }
    while (_tcscmp(text+nstart+nlen-1,TEXT("\r"))==0) {
      nlen--;
    }
    while (_tcscmp(text+nstart+nlen-1,TEXT("\n"))==0) {
      nlen--;
    }
    if (nlen>0) {
      Surface.SetTextColor(RGB_BLACK);
      Surface.DrawText(2*ScreenScale, 2*ScreenScale, text+nstart, nlen);
    }
  }
}


static void OnDetailsListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
        (void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = nTextLines-1;
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
  nTextLines=0;
  LK_tcsncpy(thisHelp,LKgethelptext(HelpText),sizeof(thisHelp)-1);

  LKASSERT(wHelp!=NULL);
  if (!wHelp) goto _getout;

  wHelp->SetBorderKind(BORDERLEFT);
  wHelp->SetWidth(wf->GetWidth() - wHelp->GetLeft()-2);

  wHelpEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmDetailsEntry"));
  LKASSERT(wHelpEntry);
  if (!wHelpEntry) goto _getout;
  wHelpEntry->SetCanFocus(true);

  nTextLines = TextToLineOffsets(thisHelp, LineOffsets, MAXLINES);

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


  wHelp->ResetList();
  wHelp->Redraw();
  wf->ShowModal();
  delete wf;
  

_getout:
  wf = NULL;

}


