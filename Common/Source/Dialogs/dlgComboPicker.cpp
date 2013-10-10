/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgComboPicker.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "InputEvents.h"
#include "WindowControls.h"
#include "InfoBoxLayout.h"
#include "Dialogs.h"


extern HWND   hWndMainWindow;
static WndForm *wf=NULL;

static void OnComboPopupListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo);
static void OnPaintComboPopupListItem(WindowControl * Sender, HDC hDC);

WndProperty * wComboPopupWndProperty;
WindowControl * wComboPopupListEntry;
WndListFrame *wComboPopupListFrame; 
DataField * ComboPopupDataField = NULL;
ComboList * ComboListPopup=NULL;

static TCHAR sSavedInitialValue[ComboPopupITEMMAX];
static int iSavedInitialDataIndex=-1;

static void OnPaintComboPopupListItem(WindowControl * Sender, HDC hDC){

  (void)Sender;

  if ( ComboListPopup->ComboPopupDrawListIndex >= 0 && 
        ComboListPopup->ComboPopupDrawListIndex < ComboListPopup->ComboPopupItemCount ) {

	int w;

	w=Sender->GetWidth();

	ExtTextOutClip(hDC, 2*ScreenScale,
		2*ScreenScale,
		ComboListPopup->ComboPopupItemList[ComboListPopup->ComboPopupDrawListIndex]->StringValueFormatted,
		w-ScreenScale*5);
  }
}

static void OnComboPopupListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo)
{ // callback function for the ComboPopup
  (void)Sender;
  if (ListInfo->DrawIndex == -1){ // initialize

    ListInfo->ItemCount = ComboListPopup->ComboPopupItemCount;
    ListInfo->ScrollIndex = 0;
    ListInfo->ItemIndex = ComboListPopup->ComboPopupItemSavedIndex;

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

static void OnCloseClicked(WindowControl * Sender){
  (void)Sender;
  wf->SetModalResult(mrOK);
}

static void OnComboPopupListEnter(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo)
{ // double-click on item -- NOT in callback table because added manually
  (void)Sender; 
  OnCloseClicked(Sender);
}

static void OnCancelClicked(WindowControl * Sender){
	(void)Sender;
  ComboListPopup->ComboPopupItemIndex= -1;
  wf->SetModalResult(mrCancle);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnComboPopupListInfo),
  DeclareCallBackEntry(OnPaintComboPopupListItem),
  DeclareCallBackEntry(OnHelpClicked),
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnCancelClicked),
  DeclareCallBackEntry(NULL)
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

    if (!ScreenLandscape) {
      char filename[MAX_PATH]; 
      LocalPathS(filename, TEXT("dlgComboPicker_L.xml"));
      wf = dlgLoadFromXML(CallBackTable, 
                          filename, 
                          hWndMainWindow,
                          TEXT("IDR_XML_COMBOPICKER_L"));
    } else {
      char filename[MAX_PATH];
      LocalPathS(filename, TEXT("dlgComboPicker.xml"));
      wf = dlgLoadFromXML(CallBackTable, 
                          filename, 
                          hWndMainWindow,
                          TEXT("IDR_XML_COMBOPICKER"));
    }

    if (!wf) return -1;

    LKASSERT(wf->GetWidth() <=ScreenSizeX);  // sometimes we have a bogus window, setfocus goes nuts

    wf->SetCaption(theProperty->GetCaption());

    wComboPopupListFrame = (WndListFrame*)wf->FindByName(TEXT("frmComboPopupList"));
    LKASSERT(wComboPopupListFrame!=NULL);
    wComboPopupListFrame->SetBorderKind(BORDERLEFT | BORDERTOP | BORDERRIGHT|BORDERBOTTOM);
    wComboPopupListFrame->SetEnterCallback(OnComboPopupListEnter);

    // allow item to be focused / hightlighted
    wComboPopupListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmComboPopupListEntry"));
    LKASSERT(wComboPopupListEntry!=NULL);
    wComboPopupListEntry->SetCanFocus(true);
    wComboPopupListEntry->SetFocused(true, wComboPopupWndProperty->GetHandle());

    // ScrollbarWidth is initialised from DrawScrollBar in WindowControls, so it might not be ready here
    if ( wComboPopupListFrame->ScrollbarWidth == -1) {
      #if defined (PNA)
      #define SHRINKSBFACTOR 1.0 // shrink width factor.  Range .1 to 1 where 1 is very "fat"
      #else
      #define SHRINKSBFACTOR 0.75  // shrink width factor.  Range .1 to 1 where 1 is very "fat"
      #endif
      wComboPopupListFrame->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);
    }
    wComboPopupListEntry->SetWidth(wComboPopupListFrame->GetWidth() - wComboPopupListFrame->ScrollbarWidth - 5);


    ComboPopupDataField = wComboPopupWndProperty->GetDataField();
    ComboListPopup = ComboPopupDataField->GetCombo();
    LKASSERT(ComboPopupDataField!=NULL);

    ComboPopupDataField->CreateComboList();
    wComboPopupListFrame->ResetList();
    wComboPopupListFrame->SetItemIndex(ComboListPopup->ComboPopupItemSavedIndex);
    if (bInitialPage) { // save values for "Cancel" from first page only
      bInitialPage=false;
      iSavedInitialDataIndex=ComboListPopup->ComboPopupItemList[ComboListPopup->ComboPopupItemSavedIndex]->DataFieldIndex;
      ComboPopupDataField->CopyString(sSavedInitialValue,false);
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
