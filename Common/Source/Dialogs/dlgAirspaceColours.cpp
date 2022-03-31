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

namespace {

void OnAirspaceColoursPaintListItem(WndOwnerDrawFrame* Sender, LKSurface& Surface) {
  auto pWndList = dynamic_cast<WndListFrame*>(Sender->GetParent());
  if (pWndList) {
    int DrawListIndex = pWndList->GetDrawIndex();

    if ((DrawListIndex < NUMAIRSPACECOLORS) && (DrawListIndex >= 0)) {
      Surface.SelectObject(LK_BLACK_PEN);
      Surface.SelectObject(MapWindow::GetAirspaceSldBrush(DrawListIndex));  // this is the solid brush

      Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));
      Surface.SetTextColor(MapWindow::GetAirspaceColour(DrawListIndex));

      PixelRect rcClient(Sender->GetClientRect());
      rcClient.Grow(DLGSCALE(-2));

      Surface.Rectangle(rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
    }
  }
}

void OnAirspaceColoursListEnter(WndListFrame* Sender, WndListFrame::ListInfo_t* ListInfo) {
  int ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex >= ListInfo->ItemCount) {
    ItemIndex = ListInfo->ItemCount - 1;
  }

  if (ItemIndex >= 0) {
    if (Sender) {
      WndForm* pForm = Sender->GetParentWndForm();
      if (pForm) {
        pForm->SetModalResult(mrOK);
      }
    }
  }
}

void OnAirspaceColoursListInfo(WndListFrame* Sender, WndListFrame::ListInfo_t* ListInfo) {
  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = NUMAIRSPACECOLORS;
  }
}

void OnCloseClicked(WndButton* pWnd) {
  if (pWnd) {
    WndForm* pForm = pWnd->GetParentWndForm();
    if (pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

CallBackTableEntry_t CallBackTable[] = {
  OnPaintCallbackEntry(OnAirspaceColoursPaintListItem),
  OnListCallbackEntry(OnAirspaceColoursListInfo),
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};

}  // namespace

int dlgAirspaceColoursShowModal() {
  std::unique_ptr<WndForm> pForm(
      dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_AIRSPACECOLOURS_L : IDR_XML_AIRSPACECOLOURS_P));
  if (!pForm) {
    return -1;
  }

  auto wListEntry = dynamic_cast<WndOwnerDrawFrame*>(pForm->FindByName(TEXT("frmAirspaceColoursListEntry")));
  if (wListEntry) {
    wListEntry->SetCanFocus(true);
  }

  auto pWndList = dynamic_cast<WndListFrame*>(pForm->FindByName(TEXT("frmAirspaceColoursList")));
  if (pWndList) {
    pWndList->SetEnterCallback(OnAirspaceColoursListEnter);
    pWndList->ResetList();
    pWndList->Redraw();
  }

  pForm->ShowModal();

  if (pWndList && pForm->GetModalResult() == mrOK) {
    return pWndList->GetItemIndex();
  }
  return -1;
}
