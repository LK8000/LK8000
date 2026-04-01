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

#include <array>
#include <functional>
#include <vector>

namespace {

constexpr bool default_filter_used_only = true;

template <bool coloredit>
class dlgAirspace final {
 public:
  bool DoModal() {
    using std::placeholders::_1;
    using std::placeholders::_2;

    _filter_used_only = default_filter_used_only;
    RebuildVisibleAirspaceTypes();

    CallBackTableEntry_t CallBackTable[] = {
        callback_entry(
            "OnAirspacePaintListItem",
            std::bind(&dlgAirspace::OnAirspacePaintListItem, this, _1, _2)),
        callback_entry("OnFilterUsedOnly",
                       std::bind(&dlgAirspace::OnFilterUsedOnly, this, _1, _2)),
        callback_entry(
            "OnAirspaceListInfo",
            std::bind(&dlgAirspace::OnAirspaceListInfo, this, _1, _2)),
        callback_entry("OnCloseClicked", &dlgAirspace::OnCloseClicked),
        callback_entry("OnLookupClicked", &dlgAirspace::OnLookupClicked),
        EndCallbackEntry()};

    std::unique_ptr<WndForm> pForm(
        dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_AIRSPACE_L
                                                      : IDR_XML_AIRSPACE_P));
    if (!pForm) {
      return false;
    }

    _airspace_list = pForm->FindByName<WndListFrame>(TEXT("frmAirspaceList"));
    if (_airspace_list) {
      _airspace_list->SetBorderKind(BORDERLEFT);
      _airspace_list->SetEnterCallback(
          std::bind(&dlgAirspace::OnAirspaceListEnter, this, _1, _2));
    }

    auto wAirspaceListEntry =
        pForm->FindByName<WndOwnerDrawFrame>(TEXT("frmAirspaceListEntry"));
    if (wAirspaceListEntry) {
      wAirspaceListEntry->SetCanFocus(true);
    }

    if (_airspace_list) {
      UpdateList(_airspace_list);
    }

    const auto oldMode = MapWindow::AirspaceMode();

    pForm->ShowModal();

    bool changed = oldMode != MapWindow::AirspaceMode();

    return changed;
  }

 private:
  static void UpdateList(WndListFrame* pWnd) {
    if (pWnd) {
      pWnd->ResetList();
      pWnd->Redraw();
    }
  }

  void RebuildVisibleAirspaceTypes() {
    _visible_airspace_types.clear();
    if (_filter_used_only) {
      _visible_airspace_types =
          CAirspaceManager::Instance().GetUsedAirspaceTypes();
    }
  }

  int GetVisibleAirspaceCount() const {
    if (_filter_used_only) {
      return static_cast<int>(_visible_airspace_types.size());
    }
    return AIRSPACECLASSCOUNT;
  }

  bool ResolveAirspaceType(int index, Airspace::Type& type) const {
    if (index < 0) {
      return false;
    }

    if (_filter_used_only) {
      if (index >= static_cast<int>(_visible_airspace_types.size())) {
        return false;
      }

      type = _visible_airspace_types[index];
      return true;
    }

    if (index >= AIRSPACECLASSCOUNT) {
      return false;
    }

    type = static_cast<Airspace::Type>(index);
    return true;
  }

  static void OnCloseClicked(WndButton* pWnd) {
    if (pWnd) {
      WndForm* pForm = pWnd->GetParentWndForm();
      if (pForm) {
        pForm->SetModalResult(mrOK);
      }
    }
  }

  static void OnLookupClicked(WndButton* pWnd) {
    dlgSelectAirspace();
  }

  void OnAirspacePaintListItem(WndOwnerDrawFrame* Sender, LKSurface& Surface) {
    auto pWndList = dynamic_cast<WndListFrame*>(Sender->GetParent());
    if (!pWndList) {
      return;
    }

    const int DrawIndex = pWndList->GetDrawIndex();
    Airspace::Type asp_type = Airspace::Type::NONE;
    if (!ResolveAirspaceType(DrawIndex, asp_type)) {
      return;
    }

    const TCHAR* label = CAirspaceManager::GetAirspaceTypeText(asp_type);

    const PixelRect rcClient(Sender->GetClientRect());
    const int w0 = rcClient.GetSize().cx;
    // LKTOKEN  _@M789_ = "Warn"
    const int w1 = Surface.GetTextWidth(MsgToken<789>()) + DLGSCALE(10);
    // LKTOKEN  _@M241_ = "Display"
    const int w2 = Surface.GetTextWidth(MsgToken<241>()) + DLGSCALE(2);

    const int x0 = w0 - w1 - w2;

    Surface.SetTextColor(RGB_BLACK);
    Surface.DrawTextClip(DLGSCALE(2), DLGSCALE(2), label, x0 - DLGSCALE(10));

    if constexpr (coloredit) {
      PixelRect rcColor = {x0, rcClient.top + DLGSCALE(2),
                           rcClient.right - DLGSCALE(2),
                           rcClient.bottom - DLGSCALE(2)};

      Surface.SelectObject(LK_WHITE_PEN);

      auto color = MapWindow::AirspaceModeColor(asp_type);
      if (color) {
        // draw Color Rectangle
        Surface.SetTextColor(*color);
        Surface.SetBkColor(RGB_WHITE);
#ifdef HAVE_HATCHED_BRUSH
        auto pattern = MapWindow::AirspaceModePattern(asp_type);
        auto brush = MapWindow::AirspaceBrush(pattern.value_or(0));
#else
        auto brush = MapWindow::AirspaceBrush(*color);
#endif
        Surface.SelectObject(brush);
        Surface.Rectangle(rcColor.left, rcColor.top, rcColor.right,
                          rcColor.bottom);
      }
      else {
        Surface.SelectObject(LKBrush_White);
        Surface.Rectangle(rcColor.left, rcColor.top, rcColor.right,
                          rcColor.bottom);
        Surface.SetTextColor(RGB_BLACK);
        Surface.SetBackgroundTransparent();

        // white rectangle with centered "Class Color"
        RECT rc = rcColor;
        Surface.DrawText(MsgToken<1921>(), &rc,
                         DT_SINGLELINE | DT_VCENTER | DT_CENTER);
      }
    }
    else {
      if (MapWindow::AirspaceModeWarning(asp_type)) {
        // LKTOKEN  _@M789_ = "Warn"
        Surface.DrawText(x0, DLGSCALE(2), MsgToken<789>());
      }
      if (MapWindow::AirspaceModeDisplay(asp_type)) {
        // LKTOKEN  _@M241_ = "Display"
        Surface.DrawText(w0 - w2, DLGSCALE(2), MsgToken<241>());
      }
    }
  }

  void OnAirspaceListEnter(WndListFrame* Sender,
                           WndListFrame::ListInfo_t* ListInfo) {
    int ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
    Airspace::Type asp_type = Airspace::Type::NONE;
    if (ResolveAirspaceType(ItemIndex, asp_type)) {
      if constexpr (coloredit) {
        int c = dlgAirspaceColoursShowModal();
        if (c > 0) {
          MapWindow::SetAirspaceColor(asp_type, {MapWindow::Colours[c - 1]});
        }
        else if (c == 0) {
          MapWindow::SetAirspaceColor(asp_type, {});
        }
#ifdef HAVE_HATCHED_BRUSH
        int p = dlgAirspacePatternsShowModal();
        if (p > 0) {
          MapWindow::SetAirspaceModePattern(asp_type, {p - 1});
        }
        else if (p == 0) {
          MapWindow::SetAirspaceModePattern(asp_type, std::nullopt);
        }
#endif
      }
      else {
        MapWindow::AirspaceModeRotateSet(asp_type);
      }
    }
  }

  void OnAirspaceListInfo(WndListFrame* Sender,
                          WndListFrame::ListInfo_t* ListInfo) {
    if (ListInfo->DrawIndex == -1) {
      ListInfo->ItemCount = GetVisibleAirspaceCount();
    }
  }

  void OnFilterUsedOnly(DataField* Sender, DataField::DataAccessKind_t Mode) {
    switch (Mode) {
      case DataField::daGet:
        Sender->Set(_filter_used_only);
        break;

      case DataField::daPut:
      case DataField::daChange: {
        const bool new_value = Sender->GetAsBoolean();
        if (_filter_used_only != new_value) {
          _filter_used_only = new_value;
          RebuildVisibleAirspaceTypes();
          UpdateList(_airspace_list);
        }
        break;
      }

      case DataField::daInc:
      case DataField::daDec:
      case DataField::daSpecial:
        break;
    }
  }

  bool _filter_used_only = default_filter_used_only;
  std::vector<Airspace::Type> _visible_airspace_types;
  WndListFrame* _airspace_list = nullptr;
};

}  // namespace

bool dlgAirspaceColor() {
  return dlgAirspace<true>().DoModal();
}

bool dlgAirspaceMode() {
  return dlgAirspace<false>().DoModal();
}
