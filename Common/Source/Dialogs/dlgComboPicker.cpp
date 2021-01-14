/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgComboPicker.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "InputEvents.h"
#include "WindowControls.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "resource.h"


static WndForm *wf=NULL;

static void OnComboPopupListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo);
static void OnPaintComboPopupListItem(WindowControl * Sender, LKSurface& Surface);

WndProperty * wComboPopupWndProperty;
WindowControl * wComboPopupListEntry;
WndListFrame *wComboPopupListFrame;
DataField * ComboPopupDataField = NULL;
ComboList * ComboListPopup=NULL;

static TCHAR sSavedInitialValue[ComboPopupITEMMAX];
static int iSavedInitialDataIndex=-1;

static void OnPaintComboPopupListItem(WindowControl * Sender, LKSurface& Surface) {

    if (Sender) {

        if (ComboListPopup->ComboPopupDrawListIndex >= 0 &&
                ComboListPopup->ComboPopupDrawListIndex < ComboListPopup->ComboPopupItemCount) {

            // Fill Background with Highlight color if Selected Item
            if (!Sender->HasFocus() && ComboListPopup->ComboPopupItemIndex == ComboListPopup->ComboPopupDrawListIndex) {
                RECT rc = Sender->GetClientRect();
                Surface.FillRect(&rc, LKBrush_Higlighted);
            }

            const int w = Sender->GetWidth();
            const int h = Sender->GetHeight();

            const TCHAR* szText = ComboListPopup->ComboPopupItemList[ComboListPopup->ComboPopupDrawListIndex]->StringValueFormatted;

            Surface.SetBackgroundTransparent();
            Surface.SetTextColor(RGB_BLACK);
            const int xText = DLGSCALE(2);
            const int yText = (h - Surface.GetTextHeight(szText)) / 2;
            Surface.DrawTextClip(xText, yText, szText, w - DLGSCALE(2));
        }
    }
}

static void OnComboPopupListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo)
{ // callback function for the ComboPopup
  (void)Sender;
  if (ListInfo->DrawIndex == -1){ // initialize

    ListInfo->ItemCount = ComboListPopup->ComboPopupItemCount;
    ListInfo->ScrollIndex = 0;
    ListInfo->ItemIndex = ComboListPopup->PropertyDataFieldIndexSaved;

  }
  else {
    ComboListPopup->ComboPopupDrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
    ComboListPopup->ComboPopupItemIndex=ListInfo->ItemIndex + ListInfo->ScrollIndex;
  }
}


static void OnHelpClicked(WindowControl * Sender){
  (void)Sender;
  if (ComboListPopup->ComboPopupItemIndex >=0) {

    int iDataIndex = ComboListPopup->ComboPopupItemList[ComboListPopup->ComboPopupItemIndex]->DataFieldIndex;
    ComboPopupDataField->SetFromCombo(iDataIndex,
      ComboListPopup->ComboPopupItemList[ComboListPopup->ComboPopupItemIndex]->StringValue);
  }

  wComboPopupWndProperty->OnHelp();
}

static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnComboPopupListEnter(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo)
{ // double-click on item -- NOT in callback table because added manually
  if(Sender) {
    WndForm * pForm = Sender->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnCancelClicked(WndButton* pWnd){
  ComboListPopup->ComboPopupItemIndex= -1;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrCancel);
    }
  }
}


static CallBackTableEntry_t CallBackTable[]={
  OnListCallbackEntry(OnComboPopupListInfo),
  OnPaintCallbackEntry(OnPaintComboPopupListItem),
  ClickNotifyCallbackEntry(OnHelpClicked),
  ClickNotifyCallbackEntry(OnCloseClicked),
  ClickNotifyCallbackEntry(OnCancelClicked),
  EndCallBackEntry()
};







int dlgComboPicker(WndProperty* theProperty){

  static bool bInComboPicker=false;
  bool bInitialPage=true;
  bool bOpenCombo=true; // used to exit loop (optionally reruns combo with
                        //lower/higher index of items for int/float

  if (bInComboPicker) // prevents multiple instances
    return 0;
  else
    bInComboPicker=true;

  while (bOpenCombo)
  {
    LKASSERT(theProperty!=NULL);
    wComboPopupWndProperty = theProperty;

    wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_COMBOPICKER_L : IDR_XML_COMBOPICKER_P);

    if (!wf) return -1;

    wf->SetCaption(theProperty->GetCaption());

    wComboPopupListFrame = (WndListFrame*)wf->FindByName(TEXT("frmComboPopupList"));
    LKASSERT(wComboPopupListFrame!=NULL);
    wComboPopupListFrame->SetBorderKind(BORDERLEFT | BORDERTOP | BORDERRIGHT|BORDERBOTTOM);
    wComboPopupListFrame->SetEnterCallback(OnComboPopupListEnter);

    // allow item to be focused / hightlighted
    wComboPopupListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmComboPopupListEntry"));
    LKASSERT(wComboPopupListEntry!=NULL);
    wComboPopupListEntry->SetCanFocus(true);

    ComboPopupDataField = wComboPopupWndProperty->GetDataField();
    ComboListPopup = ComboPopupDataField->GetCombo();
    LKASSERT(ComboPopupDataField!=NULL);

    ComboPopupDataField->CreateComboList();
    wComboPopupListFrame->ResetList();
    wComboPopupListFrame->SetItemIndex(ComboListPopup->PropertyDataFieldIndexSaved);
    if (bInitialPage) { // save values for "Cancel" from first page only
      bInitialPage=false;
      iSavedInitialDataIndex=ComboListPopup->ComboPopupItemList[ComboListPopup->PropertyDataFieldIndexSaved]->DataFieldIndex;
      ComboPopupDataField->CopyString(sSavedInitialValue,false);
    }

    WindowControl* pBtHelp = wf->FindByName(TEXT("cmdHelp"));
    if(pBtHelp) {
       pBtHelp->SetVisible(wComboPopupWndProperty->HasHelpText());
    }

    wf->ShowModal();

    bOpenCombo=false;  //tell  combo to exit loop after close

    if (ComboListPopup->ComboPopupItemIndex >=0) // OK/Select
    {
      #if 0
      ComboPopupDataField->GetCombo()->LastModalResult=1; // OK Hit Used then calling via SendMessage()
      #endif

      if (ComboListPopup->ComboPopupItemList[ComboListPopup->ComboPopupItemIndex]->DataFieldIndex
                          ==ComboPopupReopenMOREDataIndex)
      { // we're last in list and the want more past end of list so select last real list item and reopen
        ComboPopupDataField->SetDetachGUI(true);  // we'll reopen, so don't call xcsoar data changed routine yet
        ComboListPopup->ComboPopupItemIndex--;
        bOpenCombo=true; // reopen combo with new selected index at center
      }
      else if (ComboListPopup->ComboPopupItemList[ComboListPopup->ComboPopupItemIndex]->DataFieldIndex
                          ==ComboPopupReopenLESSDataIndex) // same as above but lower items needed
      {
        ComboPopupDataField->SetDetachGUI(true);
        ComboListPopup->ComboPopupItemIndex++;
        bOpenCombo=true;
      }
      int iDataIndex = ComboListPopup->ComboPopupItemList[ComboListPopup->ComboPopupItemIndex]->DataFieldIndex;
      ComboPopupDataField->SetFromCombo(iDataIndex,
        ComboListPopup->ComboPopupItemList[ComboListPopup->ComboPopupItemIndex]->StringValue);
    }
    else // Cancel
    { // if we've detached the GUI during the load, then there is nothing to do here
      #if 0
      ComboPopupDataField->GetCombo()->LastModalResult=0; // Cancel Hit.  Used then calling via SendMessage()
      #endif
      // NOTE 130206 : we are missing currently the Cancel return status .
      // The list selection does not return the Cancel button status, because so far we have been setting an empty
      // value on entry, and we check on exit if it is still empty. In such case, a no/action is performed, either
      // because the user did not select anything with Select (click on empty field at the top) or because he really
      // clicked on Cancel and we returned again the initial empty value.
      // BUT, in some cases, like on TaskOverview, we set the initial item of the list to Default.task,
      // and in this case a Cancel will return correctly Default.tsk!
      // This is why we get the confirmation message for loading default task, instead of a quiet return.
      // Solution: either use a WindowControl variable, or a more simple global for ComboCancel.
      // This would be an hack, but quick and dirty solution with no disde effects.
      // Set ComboCancel true if we are here, otherwise false, and check that after
      // dfe = (DataFieldFileReader*) wp->GetDataField()
      // If ever we want to manage this Cancel button correctly, we should use one of these approaches.

      LKASSERT(iSavedInitialDataIndex >=0);
      if (iSavedInitialDataIndex >=0) {
        ComboPopupDataField->SetFromCombo(iSavedInitialDataIndex, sSavedInitialValue);
      }
    }


    wComboPopupWndProperty->RefreshDisplay();
    ComboListPopup->FreeComboPopupItemList();

    delete wf;

    wf = NULL;

  } // loop reopen combo if <<More>> << LESS>> picked

  bInComboPicker=false;
  return 1;

}
