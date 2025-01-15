/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgSelectObject.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 15 January 2021
 */

#include "externs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "resource.h"
#include "Util/TruncateString.hpp"
#include "Dialogs.h"
#include "dlgSelectObject.h"
#include "utils/stringext.h"
#include "Event/Key.h"
#include "dlgSelectObject.h"
#include <functional>

namespace {

constexpr double DistanceFilter[] = {0.0, 25.0, 50.0, 75.0, 100.0, 150.0, 250.0, 500.0, 1000.0};

#define DirNoFilter 0
#define DirHDG -1
#define DirBRG  -2
constexpr int DirectionFilter[] = {DirNoFilter, DirHDG, DirBRG, 360, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330};


void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrCancel);
    }
  }
}


void ResetFilter(WndForm* pForm, const TCHAR* Name) {
  auto pWnd = pForm->FindByName<WndProperty>(Name);
  if (pWnd) {
    DataField* dfe = pWnd->GetDataField();
    if(dfe) {
      dfe->Set(TEXT("*"));
    }
    pWnd->RefreshDisplay();
  }
}


void SetNameCaption(WndButton* pWnd, const TCHAR* tFilter) {
  if (!pWnd) {
    return;
  }

  TCHAR NameFilter[50] = _T("");
  if (tFilter[0] == '\0' || _tcscmp(tFilter, _T("*")) == 0) {
    lk::strcpy(NameFilter, _T("*"));
  } else {
    if (_tcslen(tFilter) < 6) {
      _stprintf(NameFilter, _T("*%s*"), tFilter);
    } else {
      lk::strcpy(NameFilter, _T("*"));
      CopyTruncateString(NameFilter + 1, (50 - 1), tFilter, 5);
      _tcscat(NameFilter, _T("..*"));
    }
  }

  pWnd->SetCaption(NameFilter);
}

} // namespace

void dlgSelectObject::OnSelectClicked(WndButton* pWnd) {
  WndForm* pForm = pWnd->GetParentWndForm();
  if(pForm) {
    auto wList = pForm->FindByName<WndListFrame>(TEXT("frmList"));
    if (wList) {
      size_t idx = wList->GetItemIndex();
      if ( idx < GetVisibleCount()) {
        array_info[idx].Select();
        pForm->SetModalResult(mrOK);
        return;
      }
    }
    pForm->SetModalResult(mrCancel);
  }
}


void dlgSelectObject::OnListEnter(WindowControl* pWnd,  WndListFrame::ListInfo_t *ListInfo) {

  size_t idx = (ListInfo->ScrollIndex + ListInfo->ItemIndex);
  if (idx < GetVisibleCount()) {
    if(array_info[idx].Toggle()) {
        WndForm* pForm = pWnd->GetParentWndForm();
        if (pForm) {
          pForm->SetModalResult(mrOK);
        }
    }
  }
}


void dlgSelectObject::ResetFilter(WndForm* pForm) {
  SetDistanceFilterIdx(0);
  ::ResetFilter(pForm, _T("prpFltDistance"));

  SetDirectionFilterIdx(0);
  ::ResetFilter(pForm, _T("prpFltDirection"));

  SetTypeFilterIdx(0);
  ::ResetFilter(pForm, _T("prpFltType"));
}


struct key_filter : public key_filter_interface {

  key_filter(dlgSelectObject& _dlg) : dlg(_dlg) {}

  constexpr static size_t npos = ObjectSelectInfo_t::npos;

  const TCHAR* GetLabel() const override {
    return dlg.GetFilterLabel();
  }

  bool isHiddenKey(TCHAR c) const override {
    return !(all_keys || (key_list.find(to_lower(c)) != key_list.end()));
  }

  void Update(const TCHAR* filter) override {
    const auto& array = dlg.GetArrayInfo();

    all_keys = (!filter || !filter[0]);
    match_count = array.size();
    best_match_idx = npos;
    key_list.clear();

    if (!all_keys) {
      match_count = 0;
      size_t min_match_pos = npos;
      for (size_t i = 0; i < array.size(); ++i) {
        const ObjectSelectInfo_t& object = array[i];
        size_t match_pos = object.MatchUpdate(filter, key_list);
        if(match_pos < min_match_pos) {
          min_match_pos = match_pos;
          best_match_idx = i;
        }
        if(match_pos < array.size()) {
          ++match_count;
        }
      }
    }
  }

  unsigned GetMatchCount() const override {
    return match_count;
  }

  const TCHAR* GetMatchText() const override {
    const auto& array = dlg.GetArrayInfo();
    if (best_match_idx < npos) {
      return array[best_match_idx].Name();
    }
    return _T("");
  }

  size_t best_match_idx = npos;
  size_t match_count = 0;

  std::set<TCHAR> key_list;
  bool all_keys = true;

  dlgSelectObject& dlg;
};


void dlgSelectObject::OnFilterName(WndButton* pWnd) {

  TCHAR newNameFilter[dlgSelectObject::NAMEFILTERLEN];

  lk::strcpy(newNameFilter, GetNameFilter());

  key_filter filter(*this);
  dlgTextEntryShowModal(newNameFilter, dlgSelectObject::NAMEFILTERLEN, &filter); // todo : implement keyFilter

  SetNameCaption(pWnd, newNameFilter);
  SetNameFilter(newNameFilter);

  WndForm* pForm = pWnd->GetParentWndForm();

  ResetFilter(pForm);

  auto wList = pForm->FindByName<WndListFrame>(TEXT("frmList"));
  if (wList) {
    UpdateList();

    auto wListEntry = pForm->FindByName<WndOwnerDrawFrame>(TEXT("frmListEntry"));
    if (wListEntry) {
      wListEntry->SetFocus();
    }
    wList->Redraw();
  }
}


void dlgSelectObject::SetDirectionData(DataField *Sender) {

  TCHAR sTmp[30];

  int direction = DirectionFilter[GetDirectionFilterIdx()];

  switch (direction) {
    case DirNoFilter: 
      _stprintf(sTmp, TEXT("%c"), '*');
      break;
    case DirHDG:
      _stprintf(sTmp, TEXT("%s(%d%s)"), 
                  MsgToken<1229>(), // _@1229 HDG
                  iround(AngleLimit360(CALCULATED_INFO.Heading)),
                  MsgToken<2179>()); // _@M2179 °
      break;
    case DirBRG:
      _stprintf(sTmp, TEXT("%s(%d%s)"),
                  MsgToken<154>(), // _@M154 Brg
                  iround(GetTaskBearing()),
                  MsgToken<2179>()); // _@M2179 °
      break;
    default:
      _stprintf(sTmp, TEXT("%d%s"), direction, MsgToken<2179>()); // _@M2179 °
      break;
  }

  Sender->Set(sTmp);
}

// Painting elements after init
void dlgSelectObject::OnPaintListItem(WindowControl * Sender, LKSurface& Surface) {
  if (!Sender) {
    return;
  }

  TCHAR sTmp[50];

  Surface.SetTextColor(RGB_BLACK);

  const int width = Sender->GetWidth(); // total width
  const int LineHeight = Sender->GetHeight();
  const int TextHeight = Surface.GetTextHeight(_T("dp"));

  const int TextPos = (LineHeight - TextHeight) / 2; // offset for text vertical center


  const int w0 = LineHeight; // Picto Width

  const int w2 = Surface.GetTextWidth(TEXT("0000km")); // distance Width
  _stprintf(sTmp, _T(" 000%s"), MsgToken<2179>());
  const int w3 = Surface.GetTextWidth(sTmp); // bearing width
  
  const int w4 = GetTypeWidth(Surface);

  const int w1 = width - w0 - w2 - w3 - w4; // Max Name width


  if (DrawListIndex < GetVisibleCount()) {

    const ObjectSelectInfo_t& info = array_info[DrawListIndex];

    // Draw Picto
    const RECT PictoRect = {0, 0, w0, LineHeight};
    info.DrawPicto(Surface, PictoRect);

    // Draw Name
    Surface.SelectObject(Sender->GetFont());
    Surface.DrawTextClip(w0, TextPos, info.Name() , w1); 

    const TCHAR* filter = GetNameFilter();

    const TCHAR* start = ci_search_substr(info.Name(), filter);
    if (start) {
      // copy text before substring found
      TCHAR* end = std::copy(info.Name(), start, sTmp);
      *end = _T('\0');
      // size of this text is start of underline
      const int substart = std::max(w0, w0 + Surface.GetTextWidth(sTmp));
      // add found text
      end = std::copy_n(start, _tcslen(filter), end);
      *end = _T('\0');
      // size of result string is end of underline
      const int subend = std::min(w0 + w1, w0 + Surface.GetTextWidth(sTmp));

      if(substart < subend) {
        int h =  LineHeight - IBLSCALE(4);
        const auto hOldPen = Surface.SelectObject(LKPen_Black_N1);
        Surface.DrawLine(substart, h, subend, h);
        Surface.SelectObject(hOldPen);
      }
    }

    if (w4 > 0) {
      const TCHAR* szType = info.Type();
      const int x = std::max(w0 + w1, (w0 + w1 + w4) - Surface.GetTextWidth(szType));
      Surface.DrawTextClip(x , TextPos, info.Type(), w4);
    }

    // Draw Distance : right justified after waypoint Name
    _stprintf(sTmp, TEXT("%.0f%s"), info.Distance, Units::GetDistanceName());
    const int x2 = width - w3 - Surface.GetTextWidth(sTmp);
    Surface.DrawText(x2, TextPos, sTmp);

    // Draw Bearing right justified after distance
    _stprintf(sTmp, TEXT("%d%s"), iround(info.Direction), MsgToken<2179>());
    const int x3 = width - Surface.GetTextWidth(sTmp);
    Surface.DrawText(x3, TextPos, sTmp);
  } else {
    if (DrawListIndex == 0) {
      // LKTOKEN  _@M466_ = "No Match!"
      Surface.DrawText(IBLSCALE(2), TextPos, MsgToken<466>());
    }
  }
}


void dlgSelectObject::OnWpListInfo(WindowControl * Sender, 
                  WndListFrame::ListInfo_t *ListInfo) {
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = GetVisibleCount();
  } else {
    DrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
  }
}


bool dlgSelectObject::OnTimerNotify(WndForm* pForm) {

  int a = -1;
  switch (DirectionFilter[GetDirectionFilterIdx()]) {
    case DirHDG:
      a =  iround(AngleLimit360(CALCULATED_INFO.Heading)); // TODO : LockFlightData
      break;
    case DirBRG:
      a = iround(GetTaskBearing());
      break;
  }

  if (a >= 0) {
    if (abs(a - lastHeading) > 10) {
      lastHeading = a;

      auto wList = pForm->FindByName<WndListFrame>(TEXT("frmList"));
      if(wList) {
        auto wpDirection = pForm->FindByName<WndProperty>(TEXT("prpFltDirection"));
        if (wpDirection) {
          SetDirectionData(wpDirection->GetDataField());
          wpDirection->RefreshDisplay();
        }
        UpdateList();
        wList->Redraw();
      }
    }
  }
  return true;
}


bool dlgSelectObject::FormKeyDown(WndForm* pForm, unsigned KeyCode) {
  unsigned idx = GetTypeFilterIdx();

  switch(KeyCode) {
    case KEY_F1:
      idx = 0;
    break;
    case KEY_F2:
      idx = 2;
    break;
    case KEY_F3:
      idx = 3;
    break;
  }

  if (GetTypeFilterIdx() != idx) {
    SetTypeFilterIdx(idx);

    auto wp = pForm->FindByName<WndProperty>(TEXT("prpFltType"));
    if (wp) {
      DataField* dfe = wp->GetDataField();
      if (dfe) {
        dfe->Set(CAirspaceManager::GetAirspaceTypeText(idx));
      }
      wp->RefreshDisplay();
    }
    return true;
  }
  return false;
}


void dlgSelectObject::OnFilterDistance(DataField *Sender, 
                                       DataField::DataAccessKind_t Mode) {

  TCHAR sTmp[12];

  unsigned idx = GetDistanceFilterIdx();

  switch(Mode){

    case DataField::daInc:
      ++idx;
      if (idx >= std::size(DistanceFilter)) {
        idx = 0;
      }
      break;

    case DataField::daDec:
      if (idx == 0) {
        idx = std::size(DistanceFilter);
      }
      --idx;
      break;

    default:
      break;
  }

  if (idx == 0) {
    _stprintf(sTmp, TEXT("%c"), '*');
  } else {
    _stprintf(sTmp, TEXT("%.0f%s"), 
              DistanceFilter[idx],
              Units::GetDistanceName());
  }
  SetDistanceFilterIdx(idx);
  Sender->Set(sTmp);
}


void dlgSelectObject::OnFilterDirection(DataField *Sender, 
                                        DataField::DataAccessKind_t Mode) {

  unsigned idx = GetDirectionFilterIdx();

  switch(Mode){

    case DataField::daInc:
      ++idx;
      if (idx >= std::size(DirectionFilter)) {
        idx = 0;
      }
      break;

    case DataField::daDec:
      if (idx == 0) {
        idx = std::size(DirectionFilter);
      }
      --idx;
      break;

    default:
      break;
  }

  SetDirectionFilterIdx(idx);
  SetDirectionData(Sender);
}


void dlgSelectObject::OnFilterType(DataField *Sender, 
                                   DataField::DataAccessKind_t Mode) {

  unsigned idx = GetTypeFilterIdx();

  switch(Mode){

    case DataField::daInc:
      ++idx;
      if (idx >= GetTypeCount()) {
        idx = 0;
      }
      break;

    case DataField::daDec:
      if (idx == 0) {
        idx = GetTypeCount();
      }
      --idx;
      break;

    default:
      break;
  }

  SetTypeFilterIdx(idx);
  Sender->Set(GetTypeLabel(idx));
}


void dlgSelectObject::SetDistanceFilterIdx(int idx) {
  unsigned new_idx = std::clamp<unsigned>(idx, 0, std::size(DistanceFilter));
  if(DistanceFilterIdx != new_idx) {
    DistanceFilterIdx = new_idx;
    UpdateList();
  }
}


void dlgSelectObject::SetDirectionFilterIdx(int idx) {
  unsigned new_idx = std::clamp<unsigned>(idx, 0, std::size(DirectionFilter));
  if(DirectionFilterIdx != new_idx) {
    DirectionFilterIdx = new_idx;
    UpdateList();
  }
}


void dlgSelectObject::SetTypeFilterIdx(int idx) {
  unsigned new_idx = std::clamp<unsigned>(idx, 0, GetTypeCount());
  if(TypeFilterIdx != new_idx) {
    TypeFilterIdx = new_idx;
    UpdateList();
  }
}


void dlgSelectObject::SetNameFilter(const TCHAR (&filter)[dlgSelectObject::NAMEFILTERLEN]) {
  lk::strcpy(sNameFilter, filter);
}


void dlgSelectObject::UpdateList() {

  auto begin = array_info.begin();
  auto end =  array_info.end();

  if (TypeFilterIdx > 0) {
    end = std::partition(begin, end, [&](const auto& info) {
      return (info.FilterType(TypeFilterIdx));
    });
  }

  bool sort_by_distance = false;
  if (DistanceFilterIdx > 0 && DistanceFilterIdx < std::size(DistanceFilter)) {
    sort_by_distance = true;
    const double distance = DistanceFilter[DistanceFilterIdx];
    end = std::partition(begin, end, [distance](const auto& info) {
      // only object close to 'distance'
      return info.Distance < distance;
    });
  }

  if (DirectionFilterIdx > 0 && DirectionFilterIdx < std::size(DirectionFilter)) {

    switch (DirectionFilter[DirectionFilterIdx]) {
      case DirHDG:
        lastHeading = iround(AngleLimit360(CALCULATED_INFO.Heading));
        break;
      case DirBRG:
        lastHeading = iround(GetTaskBearing());
        break;
      default:
        lastHeading = DirectionFilter[DirectionFilterIdx];
        break;
    }

    end = std::partition(begin, end, [&](const auto& info) {
      // only object in front of heading... 
      return std::abs(AngleLimit180(info.Direction - lastHeading)) < 18;
    });
  }

  if (sNameFilter[0]) { // filter not empty
    end = std::partition(begin, end, [&](const auto& info) {
      return info.FilterName(sNameFilter);
    });
  }


  std::sort(begin, end, [&](const auto &a, const auto &b) {
    if (!sort_by_distance || a.Distance == b.Distance) {
      return _tcsicmp(a.Name(), b.Name()) < 0;
    }
    return a.Distance < b.Distance;
  });


  VisibleCount = std::distance(begin, end);

  if (pWndList) {
    pWndList->ResetList();
    pWndList->Redraw();
  }
}


int dlgSelectObject::DoModal() {

  GeoPoint position = WithLock(CritSec_FlightData, GetCurrentPosition, GPS_INFO);
  array_info = PrepareData(position);

  if (array_info.empty()) {
    return mrCancel;
  }

  AtScopeExit(&) {
      pWndList = nullptr; // to be sure this pointing will be null after pForm delete;
  };
  
  using std::placeholders::_1;
  using std::placeholders::_2;

  CallBackTableEntry_t CallBackTable[] = {
    callback_entry("OnFilterDistance", std::bind(&dlgSelectObject::OnFilterDistance, this, _1, _2)),
    callback_entry("OnFilterDirection", std::bind(&dlgSelectObject::OnFilterDirection, this, _1, _2)),
    callback_entry("OnFilterType", std::bind(&dlgSelectObject::OnFilterType, this, _1, _2)),

    callback_entry("OnPaintListItem", std::bind(&dlgSelectObject::OnPaintListItem, this, _1, _2)),

    callback_entry("OnWpListInfo", std::bind(&dlgSelectObject::OnWpListInfo, this, _1, _2)),

    callback_entry("OnFilterName", std::bind(&dlgSelectObject::OnFilterName, this, _1)),
    callback_entry("OnSelectClicked", std::bind(&dlgSelectObject::OnSelectClicked, this, _1)),
    callback_entry("OnCloseClicked", OnCloseClicked),

    EndCallBackEntry()
  };

  std::unique_ptr<WndForm> pForm(dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_SELECTOBJECT_L : IDR_XML_SELECTOBJECT_P));
  if (!pForm) {
    return mrCancel;
  }


  pWndList = pForm->FindByName<WndListFrame>(TEXT("frmList"));
  if (!pWndList) {
      return mrCancel;
  }

  pForm->SetCaption(GetCaption());

  pWndList->SetBorderKind(BORDERLEFT);


  auto wListEntry = pForm->FindByName<WndOwnerDrawFrame>(TEXT("frmListEntry"));
  if (wListEntry) {
    wListEntry->SetCanFocus(true);
  }

  UpdateList();

  pWndList->SetEnterCallback(std::bind(&dlgSelectObject::OnListEnter, this, _1, _2));

  pForm->SetKeyDownNotify(std::bind(&dlgSelectObject::FormKeyDown, this, _1, _2));
  pForm->SetTimerNotify(500, std::bind(&dlgSelectObject::OnTimerNotify, this, _1));

  return pForm->ShowModal();
}
