/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgSelectAirspaceList.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
 */

#include "externs.h"
#include "Dialogs.h"
#include "dlgMultiSelectList.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "Multimap.h"
#include "resource.h"
#include "NavFunctions.h"
#include "Draw/ScreenProjection.h"
#include "InputEvents.h"
#include "Sound/Sound.h"
#include "LKStyle.h"

#include "Dialogs.h"
#include "Asset.hpp"
#include "Library/Utm.h"
#include "utils/printf.h"
#include "FlarmIdFile.h"

#define MAX_LEN 200
#define MAX_COMMENT 80

namespace DlgMultiSelect {
namespace {

struct ListElement {
  im_element elmt;
  double Dist;

  bool operator<(const ListElement& rhs) const {
    return elmt.index() == rhs.elmt.index() && Dist <= rhs.Dist;
  }
};

std::vector<ListElement> Elements;

size_t ItemIndex = -1;
size_t DrawListIndex = 0;

size_t CountAirfields = 0;
size_t CountOutlands = 0;
size_t CountWaypoints = 0;
size_t CountTaskPoints = 0;
size_t CountAirspaces = 0;
size_t CountFlarm = 0;
size_t CountWeatherSt = 0;

constexpr size_t MaxAirfields = 3;
constexpr size_t MaxOutland = 3;
constexpr size_t MaxWaypoints = 10;
constexpr size_t MaxTask = 3;
constexpr size_t MaxAirspaces = 10;
constexpr size_t MaxFlarms = 5;
constexpr size_t MaxWeatherSt = 5;

void UpdateList(WndListFrame* pList) {
  if (pList) {
    pList->ResetList();
    pList->Redraw();
  }
}

bool OnTimer(WndForm* pWnd) {
  // Keep the dialog list updated every second
  auto pList = pWnd->FindByName<WndListFrame>(_T("frmMultiSelectListList"));
  if (pList) {
    pList->Redraw();
  }
  return true;
}

void Update(WndForm* pForm) {
  auto pList = pForm->FindByName<WndListFrame>(_T("frmMultiSelectListList"));
  if (pList) {
    pList->SetItemIndexPos(ItemIndex);
    pList->Redraw();

    auto pListEntry = pForm->FindByName<WndOwnerDrawFrame>(_T("frmMultiSelectListListEntry"));
    if (pListEntry) {
      pListEntry->SetFocus();
    }
  }
}

void OnUpClicked(WndButton* pWnd) {
  if (ItemIndex > 0) {
    ItemIndex--;
  }
  else {
    assert(!Elements.empty());
    ItemIndex = Elements.size() - 1;
  }
  LKSound(TEXT("LK_TICK.WAV"));
  Update(pWnd->GetParentWndForm());
}

void OnDownClicked(WndButton* pWnd) {
  if ((ItemIndex + 1) < Elements.size()) {
    ItemIndex++;
  }
  else {
    ItemIndex = 0;
  }
  LKSound(TEXT("LK_TOCK.WAV"));
  Update(pWnd->GetParentWndForm());
}

int LastTaskPointIndex() {
  int iLastTaskPoint = 0;
  while (ValidTaskPoint(iLastTaskPoint)) {
    iLastTaskPoint++;
  }
  return iLastTaskPoint;
}

void dlgTaskShowModal(WndForm* pForm, int idx) {

  RealActiveWaypoint = -1;
  if (idx == 0) {
    dlgTaskWaypointShowModal(idx, 0, false, true);
  }
  else {
    int iLastTaskPoint = LastTaskPointIndex();
    if (idx == iLastTaskPoint) {
      dlgTaskWaypointShowModal(idx, 2, false, true);
    }
    else if ((UseAATTarget()) && (CALCULATED_INFO.Flying) && (!IsMultiMapNoMain())) {
      pForm->SetModalResult(mrOK);
      pForm->SetVisible(false);
      dlgTarget(idx);
    }
    else {
      dlgTaskWaypointShowModal(idx, 1, false, true);
    }
  }
}

struct OnDetailsDialogVisitor {
  OnDetailsDialogVisitor() = delete;
  OnDetailsDialogVisitor(WndForm* pForm) : _pForm(pForm) {}

  void operator()(const im_airspace& elmt) const {
    CAirspaceManager::Instance().PopupAirspaceDetail(elmt.pAirspace);
  }

  void operator()(const im_waypoint& elmt) const {
    SelectedWaypoint = elmt.idx;
    PopupWaypointDetails();
  }
  
  void operator()(const im_flarm& elmt) const {
    InputEvents::processPopupDetails(InputEvents::PopupTraffic, elmt.idx);
  }
  
  void operator()(const im_task_pt& elmt) const {
    dlgTaskShowModal(_pForm, elmt.idx);
  }
  
  void operator()(const im_oracle& elmt) const {
    InputEvents::processPopupDetails(InputEvents::PopupOracle, elmt.idx);
  }

  void operator()(const im_weatherst& elmt) const {
    InputEvents::processPopupDetails(InputEvents::PopupWeatherSt, elmt.idx);
  }
  
  void operator()(const im_own_pos& elmt) const {
    InputEvents::processPopupDetails(InputEvents::PopupBasic, 0);
  }
  
  void operator()(const im_team& elmt) const {
    InputEvents::processPopupDetails(InputEvents::PopupTeam, 0);
  }
  
private:
  WndForm* _pForm;
};

void OnDetailsDialog(WndForm* pForm, size_t Index) {
  if (Index < Elements.size()) {
    pForm->SetTimerNotify(0, nullptr);
    try {
      std::visit(OnDetailsDialogVisitor(pForm), Elements[Index].elmt);
    } catch (std::exception& e) {
      StartupStore(_T("Exception in dlgMultiSelect::<unamed>::OnDetailsDialog: %s"), to_tstring(e.what()).c_str());
    }
  }  // if Index..
}

void BuildFLARMText(FLARM_TRAFFIC* pFlarm, TCHAR (&text1)[MAX_LEN],TCHAR (&text2)[MAX_LEN]) {
  if(!pFlarm) {
    return;
  }

  TCHAR Comment[MAX_COMMENT] = _T("");

  double Distance, Bear;
  DistanceBearing( GPS_INFO.Latitude,GPS_INFO.Longitude, pFlarm->Latitude,  pFlarm->Longitude, &Distance, &Bear);
  if(_tcscmp(pFlarm->Name,_T("?")) ==0)
    lk::snprintf(text1, _T("%X"), pFlarm->RadioId);
  else
    lk::snprintf(text1, _T("[%s] %X"),pFlarm->Name, pFlarm->RadioId);

  const FlarmId* flarmId = LookupFlarmId(pFlarm->RadioId);
  if(flarmId) {
    if(flarmId->freq[3] != ' ')
      lk::snprintf(Comment, _T("%s  %s %s"), flarmId->type     // FLARMID_SIZE_TYPE   22
                                            , flarmId->freq     // FLARMID_SIZE_FREQ   8    r
                                            , flarmId->name);   // FLARMID_SIZE_NAME   22 => max 52 char
    else
      lk::snprintf(Comment, _T("%s %s"), flarmId->type            // FLARMID_SIZE_TYPE   22
                                     , flarmId->name);          // FLARMID_SIZE_NAME   22 => max 52 char

    lk::snprintf(text1, _T("%s [%s] %s "), 
                                    pFlarm->Cn,  // 4
                                    pFlarm->Name // 31
                                    , Comment); // 80 => Total 121
  }

  lk::snprintf(text2, TEXT("%3.1f%s  (%i%s  %3.1f%s  %i°) %s")
            , Units::ToDistance(Distance)                   //        6
            , Units::GetDistanceName()                      // 2+3=   5
            , (int)Units::ToAltitude(pFlarm->Altitude)      //        5
            , Units::GetAltitudeName()                      // 3+2=   5
            , Units::ToVerticalSpeed(pFlarm->Average30s)    //        5
            , Units::GetVerticalSpeedName()                 // 3+2=   5
            , (int) Bear
            , (flarmId ? flarmId->airfield : _T("")) );     //FLARMID_SIZE_AIRFIELD           22  =>  53 char
}

void BuildWEATHERText(FANET_WEATHER* pWeather, TCHAR (&text1)[MAX_LEN],TCHAR (&text2)[MAX_LEN], const TCHAR* name) {
  if(!pWeather) {
    return;
  }

  TCHAR Comment[MAX_COMMENT] = _T("");;
  double Distance, Bear;
  DistanceBearing( GPS_INFO.Latitude,GPS_INFO.Longitude, pWeather->Latitude,  pWeather->Longitude, &Distance, &Bear);
  float press = pWeather->pressure;
  lk::snprintf(Comment, _T("%d° %d|%d %3.3f%s"), 
            (int)round(pWeather->windDir),
            (int)round(Units::ToWindSpeed(pWeather->windSpeed)),
            (int)round(Units::ToWindSpeed(pWeather->windGust)),
            Units::ToPressure(press), Units::GetPressureName());

  if(_tcslen(name) == 0) {
    lk::snprintf(text1, _T("%X %s"),pWeather->ID,Comment);
  }
  else {
    lk::snprintf(text1, _T("%s %s"),name,Comment);
  }

  lk::snprintf(Comment, _T("%d°C %d%% %d%%"), (int)round(pWeather->temp)
                                            , (int)round(pWeather->hum)
                                            , (int)round(pWeather->Battery));
  lk::snprintf(text2, _T("%3.1f%s (%i°) %s"),
            Units::ToDistance(Distance),
            Units::GetDistanceName(),
            (int) Bear , Comment);
}

void BuildTaskPointText(int iTaskIdx, TCHAR (&text1)[MAX_LEN], TCHAR (&text2)[MAX_LEN]) {
  if (!ValidTaskPointFast(iTaskIdx)) {
    return;
  }
  int idx = Task[iTaskIdx].Index;
  int iLastTaskPoint = LastTaskPointIndex();

  if (iTaskIdx == 0) {
     // _@M2301_  "S"    # S = Start Task point
    lk::snprintf(text1, _T("%s: (%s)"), MsgToken<2301>(), WayPointList[idx].Name);
    lk::snprintf(text2, _T("Radius %3.1f%s (%i%s)"),
               Units::ToDistance(StartRadius), Units::GetDistanceName(),
               (int) Units::ToAltitude(WayPointList[idx].Altitude),
               Units::GetAltitudeName());
  } 
  else if (iTaskIdx == iLastTaskPoint) {
    //  _@M2303_  "F"                 // max 30         30 => max 60 char
    lk::snprintf(text1, _T("%s: (%s) "), MsgToken<2303>(),
              WayPointList[idx].Name);
    lk::snprintf(text2, _T("Radius %3.1f%s (%i%s)"),
              Units::ToDistance(FinishRadius), Units::GetDistanceName(),
              (int) Units::ToAltitude(WayPointList[idx].Altitude),
              Units::GetAltitudeName());
  }
  else {
    //   _@M2302_  "T"    # F = Finish point            // max 30         30 => max 60 char
    lk::snprintf(text1, _T("%s%i: (%s) "), MsgToken<2302>(), iTaskIdx,
              WayPointList[idx].Name);
    double SecRadius = SectorRadius;
    if (UseAATTarget()) {
      if (Task[iTaskIdx].AATType == sector_type_t::SECTOR)
        SecRadius = Task[iTaskIdx].AATSectorRadius;
      else
        SecRadius = Task[iTaskIdx].AATCircleRadius;
    }

    lk::snprintf(text2, _T("Radius %3.1f%s (%i%s)"),
              Units::ToDistance(SecRadius), Units::GetDistanceName(),
              (int) Units::ToAltitude(WayPointList[idx].Altitude),
              Units::GetAltitudeName());
  }
}

void ShowTextEntries(LKSurface& Surface, const RECT& rc, TCHAR (&text1)[MAX_LEN], TCHAR (&text2)[MAX_LEN]) {
  /********************
   * show text
   ********************/
  Surface.SetBackgroundTransparent();
  Surface.SetTextColor(RGB_BLACK);
  Surface.DrawText(rc.right + DLGSCALE(2), DLGSCALE(2), text1);
  int ytext2 = Surface.GetTextHeight(text1);
  Surface.SetTextColor(RGB_DARKBLUE);
  Surface.DrawText(rc.right + DLGSCALE(2), ytext2, text2);
}

void BuildLandableText(size_t idx, double Distance, TCHAR (&text1)[MAX_LEN], TCHAR (&text2)[MAX_LEN]) {
  TCHAR Comment[MAX_COMMENT] = _T("");

  if (idx >= WayPointList.size()) {
    return;
  }
  const auto& wp = WayPointList[idx];

  if (wp.Comment) {
    lk::strcpy(Comment, wp.Comment);
  }
  else {
    lk::strcpy(Comment, TEXT(""));
  }

  if (_tcslen(wp.Freq) > 0) {
    if (_tcslen(wp.Freq) > 2)
      lk::snprintf(text1, _T("%s %s"), wp.Name, wp.Freq);
    else
      lk::snprintf(text1, _T("%s"), wp.Name);
  }
  else {
    if (wp.Style == STYLE_THERMAL)
      lk::snprintf(text1, _T("%s: %s"), MsgToken<905>(), wp.Name);
    else if (wp.Comment)
      lk::snprintf(text1, _T("%s %s"), wp.Name, Comment);
    else
      lk::snprintf(text1, _T("%s"), wp.Name);
  }

  if ((wp.RunwayLen >= 10) || (wp.RunwayDir > 0)) {
    lk::snprintf(text2, _T("%3.1f%s (%i%s  %02i/%02i  %i%s)"),
              Units::ToDistance(Distance), Units::GetDistanceName(),
              (int) Units::ToAltitude(wp.Altitude),
              Units::GetAltitudeName(),
              (int) (wp.RunwayDir / 10.0 + 0.5),
              (int) (AngleLimit360(wp.RunwayDir + 180.0) /
                      10.0 + 0.5),
              (int) Units::ToAltitude(wp.RunwayLen),
              Units::GetAltitudeName());
  }
  else {
    lk::snprintf(text2, _T("%3.1f%s (%i%s) "),
              Units::ToDistance(Distance),
              Units::GetDistanceName(),
              (int) Units::ToAltitude(wp.Altitude),
              Units::GetAltitudeName());
  }
}

void UTF8Pictorial(LKSurface& Surface, const RECT& rc,const TCHAR *Pict ,const LKColor& Color) {
  if (!Pict) {
    return;
  }
  Surface.SetBackgroundTransparent();
  const auto OldFont =  Surface.SelectObject(LK8PanelBigFont);

  Surface.SetTextColor(Color);
  int xtext = Surface.GetTextWidth(Pict);
  Surface.DrawText(rc.left +(rc.right-rc.left-xtext)/2 , DLGSCALE(2), Pict);
  Surface.SelectObject(OldFont);
}

struct PaintListItemVisitor {
  PaintListItemVisitor() = delete;
  PaintListItemVisitor(WndOwnerDrawFrame* Sender, LKSurface& Surface) : _Sender(Sender), _Surface(Surface) {}

  PixelRect GetRect() const {
    constexpr int PICTO_WIDTH = 50;
    return {
      0, 0,
      DLGSCALE(PICTO_WIDTH),
      static_cast<PixelScalar>(_Sender->GetHeight())
    };
  }

  void operator()(const im_airspace& elmt) const {
    CAirspace* pAS = elmt.pAirspace;
    if (pAS) {

      TCHAR text1[MAX_LEN] = {TEXT("empty")};
      TCHAR text2[MAX_LEN] = {TEXT("empty")};
      TCHAR Comment[MAX_COMMENT] = {TEXT("")};
      TCHAR Comment1[MAX_COMMENT] = {TEXT("")};

      auto& pAspMgr = CAirspaceManager::Instance();
      /***********************************************************************
       * here we use a local copy of the airspace, only common property exists
       ***********************************************************************/
      CAirspaceBase airspace_copy = pAspMgr.GetAirspaceCopy(pAS);

      // airspace type already in name?
      if (_tcsnicmp(airspace_copy.Name(), airspace_copy.TypeName(), _tcslen(airspace_copy.TypeName())) == 0) {
        lk::strcpy(text1, airspace_copy.Name());  // yes, take name only
      }
      else {
        // fixed strings max. 20 NAME_SIZE 30 => max. 30 char
        lk::snprintf(text1, _T("%s %s"), airspace_copy.TypeName(), airspace_copy.Name());
      }

      pAspMgr.GetSimpleAirspaceAltText(Comment, std::size(Comment), airspace_copy.Top());
      pAspMgr.GetSimpleAirspaceAltText(Comment1, std::size(Comment1), airspace_copy.Base());

      int HorDist, Bearing, VertDist;
      pAspMgr.AirspaceCalculateDistance(pAS, &HorDist, &Bearing, &VertDist);

      lk::snprintf(text2, _T("%3.1f%s (%s - %s)"),
                  Units::ToDistance(HorDist),
                  Units::GetDistanceName(),
                  Comment1, Comment);  // 8 + 8+3   21

      /****************************************************************
       * for drawing the airspace pictorial, we need the original data.
       * copy contain only base class property, not geo data,
       * original data are shared ressources !
       * for that we need to grant all called methods are thread safe
       ****************************************************************/
      const PixelRect rc= GetRect();
      ShowTextEntries(_Surface, rc, text1, text2);
      pAS->DrawPicto(_Surface, rc);
    }
  }

  void operator()(const im_waypoint& elmt) const {
    auto idx = elmt.idx;
    if (!ValidWayPointFast(idx)) {
      throw std::runtime_error("Invalid waypoint index");
    }

    TCHAR text1[MAX_LEN] = {TEXT("empty")};
    TCHAR text2[MAX_LEN] = {TEXT("empty")};
    TCHAR Comment[MAX_COMMENT] = {TEXT("")};
    double Distance;

    if (WayPointList[idx].Comment) {
      lk::strcpy(Comment, WayPointList[idx].Comment);
    } else {
      lk::strcpy(Comment, TEXT(""));
    }
    DistanceBearing(GPS_INFO.Latitude, GPS_INFO.Longitude, WayPointList[idx].Latitude,
                    WayPointList[idx].Longitude, &Distance, NULL);
    BuildLandableText(idx, Distance, text1, text2);
    const PixelRect rc= GetRect();
    ShowTextEntries(_Surface, rc, text1, text2);
    if (WayPointCalc[idx].IsLandable) {
      MapWindow::DrawRunway(_Surface, &WayPointList[idx], rc, nullptr, 1.5, true);
    }  // waypoint isLandable
    else {
      MapWindow::DrawWaypointPicto(_Surface, rc, &WayPointList[idx]);
    }
  }

  void operator()(const im_flarm& elmt) const {
    FLARM_TRAFFIC Target = WithLock(CritSec_FlightData, [&] {
      return GPS_INFO.FLARM_Traffic[elmt.idx];
    });

    TCHAR text1[MAX_LEN] = {TEXT("empty")};
    TCHAR text2[MAX_LEN] = {TEXT("empty")};

    BuildFLARMText(&Target, text1, text2);
    const PixelRect rc= GetRect();

    ShowTextEntries(_Surface, rc, text1, text2);
    MapWindow::DrawFlarmPicto(_Surface, rc, &Target);  // draw MAP icons
  }

  void operator()(const im_task_pt& elmt) const {
    TCHAR text1[MAX_LEN] = {TEXT("empty")};
    TCHAR text2[MAX_LEN] = {TEXT("empty")};
    const PixelRect rc= GetRect();
    WithLock(CritSec_TaskData, [&] {
      auto idx = elmt.idx;
      if (ValidTaskPointFast(idx)) {
        BuildTaskPointText(idx, text1, text2);
        ShowTextEntries(_Surface, rc, text1, text2);
        MapWindow::DrawTaskPicto(_Surface, idx, rc);
      }
    });
  }

  void operator()(const im_oracle& elmt) const {
    int idx = elmt.idx;
    TCHAR text1[MAX_LEN] = {TEXT("empty")};
    TCHAR text2[MAX_LEN] = {TEXT("empty")};
    const PixelRect rc= GetRect();
    lk::snprintf(text1, _T("%s"), MsgToken<2058>());  //_@M2058_ "Oracle"
    if (idx >= 0) {
      lk::snprintf(text2, _T("%s: %s"), MsgToken<456>(), WayPointList[idx].Name);  // _@M456_ "Near"
    }
    else {
      lk::snprintf(text2, _T("%s"), MsgToken<1690>());  //_@M1690_ "THE LK8000 ORACLE"
    }
    ShowTextEntries(_Surface, rc, text1, text2);

    auto szPicto = Appearance.UTF8Pictorials ? MsgToken<2381>() : _T("?");  // _@M2381_ "♕"
    UTF8Pictorial(_Surface, rc, szPicto, RGB_BLUE);
  }

  void operator()(const im_weatherst& elmt) const {
    int idx = elmt.idx;
    FANET_WEATHER Station = {};
    TCHAR StationName[MAXFANETNAME + 1];
    StationName[0] = 0;
    WithLock(CritSec_FlightData, [&] {
      memcpy(&Station, &GPS_INFO.FANET_Weather[idx], sizeof(FANET_WEATHER));
      GetFanetName(Station.ID, GPS_INFO, StationName);
    });
    TCHAR text1[MAX_LEN] = {TEXT("empty")};
    TCHAR text2[MAX_LEN] = {TEXT("empty")};
    BuildWEATHERText(&Station, text1, text2, StationName);
    const PixelRect rc= GetRect();
    ShowTextEntries(_Surface, rc, text1, text2);
    MapWindow::DrawWeatherStPicto(_Surface, rc, &Station);  // draw MAP icons
  }

  void operator()(const im_own_pos& elmt) const {
    TCHAR text1[MAX_LEN] = {TEXT("empty")};
    TCHAR text2[MAX_LEN] = {TEXT("empty")};
    TCHAR Comment[MAX_COMMENT] = {TEXT("")};
    lk::snprintf(text1, _T("%s [%s]"), AircraftRego_Config, AircraftType_Config);
    if (ISPARAGLIDER || ISCAR) {
      int utmzone;
      char utmchar;
      double easting, northing;
      LatLonToUtmWGS84(utmzone, utmchar, easting, northing, GPS_INFO.Latitude, GPS_INFO.Longitude);
      lk::snprintf(text2, _T("UTM %d%c %.0f %.0f"), utmzone, utmchar, easting, northing);
    } else {
      WithLock(CritSec_FlightData, [&] {
        Units::CoordinateToString(GPS_INFO.Longitude, GPS_INFO.Latitude, Comment, sizeof(Comment) - 1);
        lk::snprintf(text2, _T("%s %6.0f%s"), Comment,
                  Units::To(unFeet, GPS_INFO.Altitude),
                  Units::GetName(unFeet));
      });
    }
    const PixelRect rc = GetRect();
    ShowTextEntries(_Surface, rc, text1, text2);
    MapWindow::DrawAircraft(_Surface, (POINT){(rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2});
  }

  void operator()(const im_team& elmt) const {
    TCHAR text1[MAX_LEN] = {TEXT("empty")};
    TCHAR text2[MAX_LEN] = {TEXT("empty")};
    lk::snprintf(text1, _T("%s:"), MsgToken<700>());  //_@M700_ "Team code"
    WithLock(CritSec_FlightData, [&] {
      lk::snprintf(text2, _T("%s"), CALCULATED_INFO.OwnTeamCode);
    });
    const PixelRect rc = GetRect();
    ShowTextEntries(_Surface, rc, text1, text2);
    auto szPicto = Appearance.UTF8Pictorials ? MsgToken<2380>() : _T("@");  // _@M2380_ "⚑"
    UTF8Pictorial(_Surface, rc, szPicto, RGB_VDARKRED);
  }

private:
  WndOwnerDrawFrame* _Sender;
  LKSurface& _Surface;  
};

void OnMultiSelectListPaintListItem(WndOwnerDrawFrame* Sender, LKSurface& Surface) {
  try {
    if (DrawListIndex < Elements.size()) {
      const ListElement& el = Elements[DrawListIndex];

      Surface.SetTextColor(RGB_BLACK);
      Surface.SetBkColor(RGB_WHITE);

      std::visit(PaintListItemVisitor(Sender, Surface), el.elmt);
    }
  } catch (std::exception& e) {
    StartupStore(_T("Exception in `DlgMultiSelect::PaintListItem`: %s"), to_tstring(e.what()).c_str());
  }
}

void OnListEnter(WindowControl* Sender, WndListFrame::ListInfo_t* ListInfo) {
  ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  if (ItemIndex >= Elements.size()) {
    ItemIndex = Elements.size() - 1;
  }

  if (Sender && ItemIndex >= 0) {
    WndForm* pForm = Sender->GetParentWndForm();
    if (pForm) {
      pForm->SetModalResult(mrOK);
      OnDetailsDialog(pForm, ItemIndex);
    }
  }
}

void OnEnterClicked(WndButton* pWnd) {
  if (ItemIndex >= Elements.size()) {
    ItemIndex = Elements.size() - 1;
  }
  if (pWnd && ItemIndex >= 0) {
    WndForm* pForm = pWnd->GetParentWndForm();
    if (pForm) {
      pForm->SetModalResult(mrOK);
    }
    OnDetailsDialog(pForm, ItemIndex);
  }
}

void OnMultiSelectListListInfo(WndListFrame* Sender, WndListFrame::ListInfo_t* ListInfo) {
  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = Elements.size();
  }
  else {
    DrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
    ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  }
}

void OnCloseClicked(WndButton* pWnd) {
  ItemIndex = -1;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrCancel);
    }
  }
}

struct CheckCountVisitor {
  template <size_t max>
  static bool CheckCount(size_t& count) {
    if (count < max) {
      count++;
      return false;
    }
    return true;
  }

  bool operator()(const im_airspace& elmt) const {
    return CheckCount<MaxAirspaces>(CountAirspaces);
  }

  bool operator()(const im_waypoint& elmt) const {
    if (WayPointCalc[elmt.idx].IsLandable) {
      if (WayPointCalc[elmt.idx].IsAirport) {
        return CheckCount<MaxAirfields>(CountAirfields);
      }
      else {
        return CheckCount<MaxOutland>(CountOutlands);
      }
    }
    return CheckCount<MaxWaypoints>(CountWaypoints);
  }

  bool operator()(const im_flarm& elmt) const {
    return CheckCount<MaxFlarms>(CountFlarm);
  }

  bool operator()(const im_task_pt& elmt) const {
    return CheckCount<MaxTask>(CountTaskPoints);
  }

  bool operator()(const im_oracle& elmt) const {
    // TODO: check if we need to limit the number of oracle
    // should be only one, but it have `idx` parameter,
    // so, not technically limited by the "exist" check
    return false;
  }

  bool operator()(const im_weatherst& elmt) const {
    return CheckCount<MaxWeatherSt>(CountWeatherSt);
  }

  bool operator()(const im_own_pos& elmt) const {
    return false;
  }

  bool operator()(const im_team& elmt) const {
    return false;
  }
};

} // namespace

int GetItemCount() {
  return Elements.size();
}

void AddItem(im_element&& elmt, double distance) {

  try {
    if (std::holds_alternative<im_task_pt>(elmt)) {
      if (Task[std::get<im_task_pt>(elmt).idx].Index == RESWP_PANPOS) {
        // PanPos is not a task point
        return;
      }
      if (ValidTaskPoint(PanTaskEdit)) {
        return;
      }
      if (LastTaskPointIndex() < 2) {
        return;
      }
    }

    if (std::holds_alternative<im_waypoint>(elmt)) {
      if (std::get<im_waypoint>(elmt).idx == RESWP_PANPOS) {
        // PanPos is not a waypoint
        return;
      }
    }

    bool exist = std::any_of(Elements.begin(), Elements.end(), [&elmt](const ListElement& el) {
      return el.elmt == elmt;
    });
    if (exist) {
      return; // already in list
    }

    // check if we are full
    bool full = std::visit(CheckCountVisitor(), elmt);
    if (full) {
      // replace the farthest element of the same type
      auto it = std::max_element(Elements.begin(), Elements.end());
      if (it != Elements.end()) {
        if (it->Dist < distance) {
          // farthest element is closer than the new one...
          return;
        }
        Elements.erase(it);
      }
    }

    // find new element position
    auto it = std::upper_bound(Elements.begin(), Elements.end(), distance, [](double distance, const ListElement& a) {
      return a.Dist > distance;
    });
    Elements.insert(it, { elmt, distance });

    assert(std::is_sorted(Elements.begin(), Elements.end(), [](const auto& a, const auto& b) {
      return a.Dist < b.Dist;
    }));

  } catch (std::exception& e) {
    StartupStore(_T("Exception in DlgMultiSelect::AddItem: %s"), to_tstring(e.what()).c_str());
  }
}

void ShowModal() {
  if (Elements.empty()) {
    return;
  }

  CallBackTableEntry_t CallBackTable[] = {
    OnPaintCallbackEntry(OnMultiSelectListPaintListItem),
    OnListCallbackEntry(OnMultiSelectListListInfo),
    ClickNotifyCallbackEntry(OnCloseClicked),
    ClickNotifyCallbackEntry(OnUpClicked),
    ClickNotifyCallbackEntry(OnEnterClicked),
    ClickNotifyCallbackEntry(OnDownClicked),
    EndCallBackEntry()
  };

  ItemIndex = -1;

  std::unique_ptr<WndForm> pForm(
      dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_MULTISELECTLIST_L : IDR_XML_MULTISELECTLIST_P));

  if (!pForm) {
    return;
  }
  pForm->SetTimerNotify(1000, OnTimer);

  auto pList = pForm->FindByName<WndListFrame>(_T("frmMultiSelectListList"));
  if (pList) {
    pList->SetBorderKind(BORDERLEFT);
    pList->SetEnterCallback(OnListEnter);

    auto pListEntry = pForm->FindByName<WndOwnerDrawFrame>(_T("frmMultiSelectListListEntry"));
    if (pListEntry) {
      /*
       * control height must contains 2 text Line
       * Check and update Height if necessary
       */
      LKWindowSurface windowSurface(*main_window);
      LKBitmapSurface tmpSurface(windowSurface, 1, 1);

      const auto oldFont = tmpSurface.SelectObject(pListEntry->GetFont());
      const int minHeight = 2 * tmpSurface.GetTextHeight(_T("dp")) + 2 * DLGSCALE(2);
      tmpSurface.SelectObject(oldFont);
      const int wHeight = pListEntry->GetHeight();
      if (minHeight > wHeight) {
        pListEntry->SetHeight(minHeight);
      }
      pListEntry->SetCanFocus(true);
    }
    UpdateList(pList);
  }

  pForm->ShowModal();

  Elements.clear();

  CountAirfields = 0;
  CountOutlands = 0;
  CountWaypoints = 0;
  CountAirspaces = 0;
  CountTaskPoints = 0;
  CountFlarm = 0;
  CountWeatherSt = 0;
}

} // namespace DlgMultiSelect
