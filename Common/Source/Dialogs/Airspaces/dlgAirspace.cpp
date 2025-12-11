/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspace.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "LKObjects.h"
#include "resource.h"

namespace {

void UpdateList(WndListFrame* pWnd) {
  if (pWnd) {
    pWnd->ResetList();
    pWnd->Redraw();
  }
}

template <bool colormode>
void OnAirspacePaintListItem(WndOwnerDrawFrame* Sender, LKSurface& Surface) {
  auto pWndList = dynamic_cast<WndListFrame*>(Sender->GetParent());
  if (!pWndList) {
    return;
  }

  const int DrawIndex = pWndList->GetDrawIndex();
  if (DrawIndex < 0 || DrawIndex >= AIRSPACECLASSCOUNT) {
    return;
  }

  const TCHAR* label = CAirspaceManager::GetAirspaceTypeText(DrawIndex);

  const PixelRect rcClient(Sender->GetClientRect());
  const int w0 = rcClient.GetSize().cx;
  // LKTOKEN  _@M789_ = "Warn"
  const int w1 = Surface.GetTextWidth(MsgToken<789>()) + DLGSCALE(10);
  // LKTOKEN  _@M241_ = "Display"
  const int w2 = Surface.GetTextWidth(MsgToken<241>()) + DLGSCALE(2);

  const int x0 = w0 - w1 - w2;

  Surface.SetTextColor(RGB_BLACK);
  Surface.DrawTextClip(DLGSCALE(2), DLGSCALE(2), label, x0 - DLGSCALE(10));

  if constexpr (colormode) {
    PixelRect rcColor = {x0, rcClient.top + DLGSCALE(2),
                         rcClient.right - DLGSCALE(2),
                         rcClient.bottom - DLGSCALE(2)};

    Surface.SelectObject(LK_WHITE_PEN);
    Surface.SelectObject(LKBrush_White);
    Surface.Rectangle(rcColor.left, rcColor.top, rcColor.right, rcColor.bottom);

    Surface.SetTextColor(MapWindow::GetAirspaceColourByClass(DrawIndex));
    Surface.SetBkColor(RGB_WHITE);
    Surface.SelectObject(MapWindow::GetAirspaceBrushByClass(DrawIndex));
    Surface.Rectangle(rcColor.left, rcColor.top, rcColor.right, rcColor.bottom);
  }
  else {
    if (MapWindow::aAirspaceMode[DrawIndex].warning()) {
      // LKTOKEN  _@M789_ = "Warn"
      Surface.DrawText(x0, DLGSCALE(2), MsgToken<789>());
    }
    if (MapWindow::aAirspaceMode[DrawIndex].display()) {
      // LKTOKEN  _@M241_ = "Display"
      Surface.DrawText(w0 - w2, DLGSCALE(2), MsgToken<241>());
    }
  }
}

template <bool colormode>
void OnAirspaceListEnter(WindowControl* Sender,
                         WndListFrame::ListInfo_t* ListInfo) {
  int ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex >= AIRSPACECLASSCOUNT) {
    ItemIndex = AIRSPACECLASSCOUNT - 1;
  }
  if (ItemIndex >= 0) {
    if constexpr (colormode) {
      int c = dlgAirspaceColoursShowModal();
      if (c >= 0) {
        MapWindow::iAirspaceColour[ItemIndex] = c;
      }
#ifdef HAVE_HATCHED_BRUSH
      int p = dlgAirspacePatternsShowModal();
      if (p >= 0) {
        MapWindow::iAirspaceBrush[ItemIndex] = p;
      }
#endif
    }
    else {
      MapWindow::aAirspaceMode[ItemIndex].rotate_set();
    }
  }
}

void OnAirspaceListInfo(WndListFrame* Sender,
                        WndListFrame::ListInfo_t* ListInfo) {
  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = AIRSPACECLASSCOUNT;
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

void OnLookupClicked(WndButton* pWnd) {
  dlgSelectAirspace();
}

template <typename T, std::size_t N>
std::array<T, N> snapshot(const T (&raw)[N]) {
  std::array<T, N> out{};
  std::copy(std::begin(raw), std::end(raw), out.begin());
  return out;
}

template <typename T, std::size_t N>
bool equal_snaphot(const T (&raw)[N], const std::array<T, N>& arr) {
  return std::equal(std::begin(raw), std::end(raw), arr.begin());
}

template <typename T, std::size_t N>
bool changed_snaphot(const T (&raw)[N], const std::array<T, N>& arr) {
  return !equal_snaphot(raw, arr);
}

template <bool coloredit>
bool dlgAirspaceShowModal() {
  CallBackTableEntry_t CallBackTable[] = {
      callback_entry("OnAirspacePaintListItem",
                     OnAirspacePaintListItem<coloredit>),
      CallbackEntry(OnAirspaceListInfo),
      CallbackEntry(OnCloseClicked),
      CallbackEntry(OnLookupClicked),
      EndCallbackEntry()
    };

  std::unique_ptr<WndForm> pForm(
      dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_AIRSPACE_L
                                                    : IDR_XML_AIRSPACE_P));
  if (!pForm) {
    return false;
  }

  auto wAirspaceList = pForm->FindByName<WndListFrame>(TEXT("frmAirspaceList"));
  if (wAirspaceList) {
    wAirspaceList->SetBorderKind(BORDERLEFT);
    wAirspaceList->SetEnterCallback(OnAirspaceListEnter<coloredit>);
  }
  auto wAirspaceListEntry =
      pForm->FindByName<WndOwnerDrawFrame>(TEXT("frmAirspaceListEntry"));
  if (wAirspaceListEntry) {
    wAirspaceListEntry->SetCanFocus(true);
  }

  if (wAirspaceList) {
    UpdateList(wAirspaceList);
  }

  const auto oldMode = MapWindow::aAirspaceMode;
  const auto oldColor = snapshot(MapWindow::iAirspaceColour);
#ifdef HAVE_HATCHED_BRUSH
  const auto oldBrush = snapshot(MapWindow::iAirspaceBrush);
#endif

  pForm->ShowModal();

  if (oldMode != MapWindow::aAirspaceMode) {
    return true;
  }
  if (changed_snaphot(MapWindow::iAirspaceColour, oldColor)) {
    return true;
  }
#ifdef HAVE_HATCHED_BRUSH
  if (changed_snaphot(MapWindow::iAirspaceBrush, oldBrush)) {
    return true;
  }
#endif

  return false;
}

}  // namespace

bool dlgAirspaceColor() {
  return dlgAirspaceShowModal<true>();
}

bool dlgAirspaceMode() {
  return dlgAirspaceShowModal<false>();
}
