/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgSelectItem.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 31 April 2022
 */
#include "dlgSelectItem.h"
#include "externs.h"
#include "dlgTools.h"
#include "resource.h"

#include <memory>
#include <functional>

namespace {

void OnListEnter(WndListFrame* Sender, WndListFrame::ListInfo_t* ListInfo) {
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

void OnCloseClicked(WndButton* pWnd) {
  if (pWnd) {
    WndForm* pForm = pWnd->GetParentWndForm();
    if (pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

}  // namespace

int dlgSelectItem::DoModal() {
  using std::placeholders::_1;
  using std::placeholders::_2;

  CallBackTableEntry_t CallBackTable[] = {
      callback_entry("OnPaintListItem", std::bind(&dlgSelectItem::OnPaintListItem, this, _1, _2)),
      callback_entry("OnListInfo", std::bind(&dlgSelectItem::OnListInfo, this, _1, _2)),
      ClickNotifyCallbackEntry(OnCloseClicked),
      EndCallBackEntry()
  };

  std::unique_ptr<WndForm> pForm(
      dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_SELECTITEM_L : IDR_XML_SELECTITEM_P));

  if (!pForm) {
    return -1;
  }

  pForm->SetCaption(GetTitle());

  auto wListEntry = dynamic_cast<WndOwnerDrawFrame*>(pForm->FindByName(TEXT("frmListEntry")));
  if (wListEntry) {
    wListEntry->SetCanFocus(true);
  }

  auto pWndList = dynamic_cast<WndListFrame*>(pForm->FindByName(TEXT("frmList")));
  if (pWndList) {
    pWndList->SetEnterCallback(OnListEnter);
    pWndList->ResetList();
    pWndList->Redraw();
  }

  pForm->ShowModal();

  if (pWndList && pForm->GetModalResult() == mrOK) {
    return pWndList->GetItemIndex();
  }
  return -1;
}

void dlgSelectItem::OnPaintListItem(WndOwnerDrawFrame* Sender, LKSurface& Surface) {
  auto pWndList = dynamic_cast<WndListFrame*>(Sender->GetParent());
  if (pWndList) {
    int DrawListIndex = pWndList->GetDrawIndex();

    if ((DrawListIndex < GetItemCount()) && (DrawListIndex >= 0)) {
      PixelRect rcClient(Sender->GetClientRect());
      rcClient.Grow(DLGSCALE(-2));

      DrawItem(Surface, rcClient, DrawListIndex);
    }
  }
}

void dlgSelectItem::OnListInfo(WndListFrame* Sender, WndListFrame::ListInfo_t* ListInfo) {
  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = GetItemCount();
  }
}
