/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgHelp.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "InputEvents.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "utils/TextWrapArray.h"
#include "resource.h"

static int DrawListIndex=0;

static TextWrapArray aTextLine;

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static void OnPaintDetailsListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface){

  if (DrawListIndex < (int)aTextLine.size()){
      LKASSERT(DrawListIndex>=0);
      const TCHAR* szText = aTextLine[DrawListIndex];
      Surface.SetTextColor(RGB_BLACK);
      Surface.DrawText(DLGSCALE(2), DLGSCALE(2), szText);
  }
}


static void OnDetailsListInfo(WndListFrame * Sender, WndListFrame::ListInfo_t *ListInfo){

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

void dlgHelpShowModal(const TCHAR* Caption, const TCHAR* HelpText, bool bHelpCapt ) {
  if (!Caption || !HelpText) {
    return;
  }

  std::unique_ptr<WndForm> wf(dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_HELP_L : IDR_XML_HELP_P));
  if(!wf) {
    return;
  }
  WndListFrame* wHelp = static_cast<WndListFrame*>(wf->FindByName(TEXT("frmDetails")));
  if(!wHelp) {
    return;
  }
  wHelp->SetBorderKind(BORDERLEFT);

  WndOwnerDrawFrame* wHelpEntry = static_cast<WndOwnerDrawFrame*>(wf->FindByName(TEXT("frmDetailsEntry")));
  if (!wHelpEntry) {
    return;
  };
  wHelpEntry->SetCanFocus(true);

  DrawListIndex=0;


  if( bHelpCapt)
  {
    TCHAR fullcaption[100];
    _stprintf(fullcaption,TEXT("%s: %s"), MsgToken<336>(), Caption); // Help
    wf->SetCaption(fullcaption);
  }
  else
    wf->SetCaption(Caption);

  aTextLine.clear();

  {
    LKWindowSurface Surface(*wHelpEntry);

    const auto oldFont = Surface.SelectObject(wHelpEntry->GetFont());
    const int minHeight = Surface.GetTextHeight(_T("dp")) + 2 * DLGSCALE(2);
    const int wHeight = wHelpEntry->GetHeight();
    if(minHeight != wHeight) {
      wHelpEntry->SetHeight(minHeight);
    }
    if( bHelpCapt)
      aTextLine.update(Surface, wHelpEntry->GetWidth(), LKgethelptext(HelpText).c_str());
    else
      aTextLine.update(Surface, wHelpEntry->GetWidth(), HelpText);
    Surface.SelectObject(oldFont);
  }

  wHelp->ResetList();
  wHelp->Redraw();
  wf->ShowModal();

  aTextLine.clear();
}
