/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"
#include "externs.h"
#include "Units.h"
#include "device.h"
#include "InputEvents.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"

#include "utils/heapcheck.h"

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

	ExtTextOutClip(hDC, 2*InfoBoxLayout::scale, 
		2*InfoBoxLayout::scale,
		ComboListPopup->ComboPopupItemList[ComboListPopup->ComboPopupDrawListIndex]->StringValueFormatted,
		w-InfoBoxLayout::scale*5);
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
    ASSERT(theProperty!=NULL);
    wComboPopupWndProperty = theProperty;


    if (!InfoBoxLayout::landscape) {
      char filename[MAX_PATH]; 
      LocalPathS(filename, TEXT("dlgComboPicker_L.xml"));
      wf = dlgLoadFromXML(CallBackTable, 
                          filename, 
                          hWndMainWindow,
                          TEXT("IDR_XML_COMBOPICKER_L"));
    } else {
      char filename[MAX_PATH];
      LocalPathS(filename, TEXT("dlgWayComboPicker.xml"));
      wf = dlgLoadFromXML(CallBackTable, 
                          filename, 
                          hWndMainWindow,
                          TEXT("IDR_XML_COMBOPICKER"));
    }

    if (!wf) return -1;

    ASSERT(wf!=NULL);
    //ASSERT(wf->GetWidth() <1200);  // sometimes we have a bogus window, setfocus goes nuts

    wf->SetCaption(theProperty->GetCaption());

    wComboPopupListFrame = (WndListFrame*)wf->FindByName(TEXT("frmComboPopupList"));
    ASSERT(wComboPopupListFrame!=NULL);
    wComboPopupListFrame->SetBorderKind(BORDERLEFT | BORDERTOP | BORDERRIGHT|BORDERBOTTOM);
    wComboPopupListFrame->SetEnterCallback(OnComboPopupListEnter);

    // allow item to be focused / hightlighted
    wComboPopupListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmComboPopupListEntry"));
    ASSERT(wComboPopupListEntry!=NULL);
    wComboPopupListEntry->SetCanFocus(true);
    wComboPopupListEntry->SetFocused(true, wComboPopupWndProperty->GetHandle());


    ComboPopupDataField = wComboPopupWndProperty->GetDataField();
    ComboListPopup = ComboPopupDataField->GetCombo();
    ASSERT(ComboPopupDataField!=NULL);

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
#if 100124
      ComboPopupDataField->GetCombo()->LastModalResult=1; // OK Hit Used then calling via SendMessage()
#endif
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
#if 100124
	ComboPopupDataField->GetCombo()->LastModalResult=0; // Cancel Hit.  Used then calling via SendMessage()
#endif
#endif
      ASSERT(iSavedInitialDataIndex >=0);
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
