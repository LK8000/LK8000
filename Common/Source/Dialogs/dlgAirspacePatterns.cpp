/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspacePatterns.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "LKObjects.h"
#include "resource.h"

#ifdef HAVE_HATCHED_BRUSH

static int ItemIndex = -1;


static void UpdateList(WndListFrame* pWnd){
  if(pWnd) {
    pWnd->ResetList();
    pWnd->Redraw();
  }
}

static int DrawListIndex=0;

static void OnAirspacePatternsPaintListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface) {

    if ((DrawListIndex < NUMAIRSPACEBRUSHES) &&(DrawListIndex >= 0)) {
        Surface.SelectObject(LKBrush_White);
        Surface.SelectObject(LK_BLACK_PEN);
        Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));

        Surface.SelectObject(MapWindow::GetAirspaceBrush(DrawListIndex));
        Surface.SetTextColor(LKColor(0x00, 0x00, 0x00));
        const PixelRect rcClient(Sender->GetClientRect());
        Surface.Rectangle(rcClient.left + DLGSCALE(2), 
                          rcClient.top + DLGSCALE(2), 
                          rcClient.right - DLGSCALE(2), 
                          rcClient.bottom - DLGSCALE(2));
    }
}

static void OnAirspacePatternsListEnter(WindowControl * Sender,
				WndListFrame::ListInfo_t *ListInfo) {
  (void)Sender;
  ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex>=NUMAIRSPACEBRUSHES) {
    ItemIndex = NUMAIRSPACEBRUSHES-1;
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


static void OnAirspacePatternsListInfo(WindowControl * Sender,
			       WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = NUMAIRSPACEBRUSHES;
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
#ifdef HAVE_HATCHED_BRUSH
  OnPaintCallbackEntry(OnAirspacePatternsPaintListItem),
#endif
  OnListCallbackEntry(OnAirspacePatternsListInfo),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};


int dlgAirspacePatternsShowModal(void){

  ItemIndex = -1;

  WndForm* wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_AIRSPACEPATTERNS_L : IDR_XML_AIRSPACEPATTERNS_P);

  if (!wf) return -1;

  //ASSERT(wf!=NULL);

  WndListFrame* wAirspacePatternsList = (WndListFrame*)wf->FindByName(TEXT("frmAirspacePatternsList"));
  if(wAirspacePatternsList) {
    wAirspacePatternsList->SetBorderKind(BORDERLEFT);
    wAirspacePatternsList->SetEnterCallback(OnAirspacePatternsListEnter);
    UpdateList(wAirspacePatternsList);
  }

  WndOwnerDrawFrame* wAirspacePatternsListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmAirspacePatternsListEntry"));
  if(wAirspacePatternsListEntry) {
    wAirspacePatternsListEntry->SetCanFocus(true);
  }
  wf->ShowModal();

  // now retrieve back the properties...

  delete wf;
  return ItemIndex;
}

#endif