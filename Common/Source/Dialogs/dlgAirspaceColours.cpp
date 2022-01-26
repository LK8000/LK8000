/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceColours.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "LKObjects.h"
#include "resource.h"

static int ItemIndex = -1;


static void UpdateList(WndListFrame* pWnd){
  if(pWnd) {
    pWnd->ResetList();
    pWnd->Redraw();
  }
}

static int DrawListIndex=0;

static void OnAirspaceColoursPaintListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface){

  if ((DrawListIndex < NUMAIRSPACECOLORS) &&(DrawListIndex>=0)) {
    Surface.SelectObject(LKBrush_White);
    Surface.SelectObject(LK_BLACK_PEN);
    Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));
    Surface.SelectObject(MapWindow::GetAirspaceSldBrush(DrawListIndex)); // this is the solid brush
    Surface.SetTextColor(MapWindow::GetAirspaceColour(DrawListIndex));
    
    const PixelRect rcClient(Sender->GetClientRect());
    Surface.Rectangle(rcClient.left + DLGSCALE(2), 
                      rcClient.top + DLGSCALE(2), 
                      rcClient.right - DLGSCALE(2), 
                      rcClient.bottom - DLGSCALE(2));
  }
}


static void OnAirspaceColoursListEnter(WindowControl * Sender,
				WndListFrame::ListInfo_t *ListInfo) {

  ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex>=NUMAIRSPACECOLORS) {
    ItemIndex = NUMAIRSPACECOLORS-1;
  }
  if (ItemIndex>=0) {
    if(Sender) {
      WndForm * pForm = Sender->GetParentWndForm();
      if(pForm) {
        pForm->SetModalResult(mrOK);
      }
    }
  }
}


static void OnAirspaceColoursListInfo(WindowControl * Sender,
			       WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = NUMAIRSPACECOLORS;
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
    ItemIndex = ListInfo->ItemIndex+ListInfo->ScrollIndex;
  }
}

static void OnCloseClicked(WndButton* pWnd){
  ItemIndex = -1;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static CallBackTableEntry_t CallBackTable[]={
  OnPaintCallbackEntry(OnAirspaceColoursPaintListItem),
  OnListCallbackEntry(OnAirspaceColoursListInfo),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};


int dlgAirspaceColoursShowModal(void){

  ItemIndex = -1;

  WndForm*  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_AIRSPACECOLOURS_L : IDR_XML_AIRSPACECOLOURS_P);

  if (!wf) return -1;

  WndListFrame *wAirspaceColoursList = (WndListFrame*)wf->FindByName(TEXT("frmAirspaceColoursList"));
  if(wAirspaceColoursList) {
    wAirspaceColoursList->SetEnterCallback(OnAirspaceColoursListEnter);
  }

  WndOwnerDrawFrame *wAirspaceColoursListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmAirspaceColoursListEntry"));
  if(wAirspaceColoursListEntry) {
    wAirspaceColoursListEntry->SetCanFocus(true);
  }

  UpdateList(wAirspaceColoursList);

  wf->ShowModal();

  delete wf;

  return ItemIndex;
}
