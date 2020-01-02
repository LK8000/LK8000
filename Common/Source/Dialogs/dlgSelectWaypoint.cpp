/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgSelectWaypoint.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "dlgSelectObject.h"
#include "utils/stringext.h"

namespace {

size_t SelectedIndex = std::numeric_limits<size_t>::max();

struct WaypointInfo_t : public ObjectAdaptor_t {
  WaypointInfo_t() = delete;

  explicit WaypointInfo_t(size_t i, WAYPOINT& Tp) {
    index = i;
    fileIdx = Tp.FileNum;
    name = Tp.Name;
    if(Tp.Code[0] && !ci_search_substr(Tp.Name, Tp.Code)) {
      // add CODE if not already included in Name.
      name += _T(" (");
      name += Tp.Code;
      name += _T(")");
    }

    type = Tp.Flags;
  }

  void Select() const override {
    SelectedIndex = index;
  }

  bool Toggle() const override {
    SelectedIndex = index;
    return true;
  }

  void DrawPicto(LKSurface& Surface, const RECT &rc) const override {
    if (WayPointCalc[index].IsLandable) {
      MapWindow::DrawRunway(Surface, &WayPointList[index], rc, nullptr, 1 , true);
    } else {
      MapWindow::DrawWaypointPicto(Surface, rc, &WayPointList[index]);
    }     
  }

  const TCHAR* Name() const override {
    return name.c_str();
  }

  const TCHAR* Type() const override {
    return _T("");
  }
  
  bool FilterType(unsigned type) const override {
    if (type == 1) {
      return (this->type & AIRPORT);
    }
    if (type == 2) {
      return (this->type &  (AIRPORT | LANDPOINT));
    }
    if (type == 3) {
      return (this->type &  (TURNPOINT));
    }
    if (type >= 4 && type < (4 + NO_WP_FILES)) {
      return (fileIdx == static_cast<int>(type) - 4);
    }
    return true; // no filter
  }

  size_t index;
  tstring name;
  int fileIdx;
  int type;
};

class dlgSelectWaypoint_t final : public dlgSelectObject  {
public:

  dlgSelectWaypoint_t(int type, int FilterNear) {
    //If you add more items don't forget to change TYPEFILTERSNUM and UpdateList() also
    _stprintf(TypeFilter[0], TEXT("*"));	
    _stprintf(TypeFilter[1], TEXT("%s"), MsgToken<1224>()); // LKTOKEN _@M1224_ "Airport"
    _stprintf(TypeFilter[2], TEXT("%s"), MsgToken<1225>()); // LKTOKEN _@M1225_ "Landable"
    _stprintf(TypeFilter[3], TEXT("%s"), MsgToken<1226>()); // LKTOKEN _@M1226_ "Turnpoint"
    for (unsigned i = 0 ; i < NO_WP_FILES; i++) {
      // TODO : do not add Empty File
      _stprintf(TypeFilter[4+i], TEXT("%s %i"), MsgToken<2342>(), i+1 ); // LKTOKEN _@M2342_ "File"
    }

    if (type > -1) {
      SetTypeFilterIdx(type);
    }
    if (FilterNear) {
      SetDistanceFilterIdx(FilterNear);
    }
  }

  unsigned GetTypeCount() const override {
    return 4 + NO_WP_FILES;
  }

  const TCHAR* GetTypeLabel(unsigned type) const override {
    return TypeFilter[type];
  }

  int GetTypeWidth(LKSurface& Surface) override {
    return 0; // don't draw 'type' in list.
  }

  const TCHAR* GetFilterLabel() const override {
    return MsgToken<1226>(); // "_@M1226_": "Turnpoint"
  };

protected:

  const TCHAR* GetCaption() const override {
    return MsgToken<592>();
  };

  array_info_t PrepareData(const GeoPoint& position) override {
    array_info_t data;
    try {
      data.reserve(WayPointList.size());

      for (size_t i = 0; i < WayPointList.size(); ++i) {
        WAYPOINT& Tp = WayPointList[i];
        
        if(Tp.Latitude!=RESWP_INVALIDNUMBER) {
          double direction = 0;
          double distance = 0;
          position.Reverse({Tp.Latitude, Tp.Longitude}, direction, distance);
         
          data.push_back({DISTANCEMODIFY *distance, direction, std::make_unique<WaypointInfo_t>(i, Tp)});
        }
      }

    } catch (std::bad_alloc&) {
      OutOfMemory(_T(__FILE__),__LINE__);
    }
    return data;
  }

  TCHAR TypeFilter[4 + NO_WP_FILES][50];

};

}

int dlgSelectWaypoint(int type, int FilterNear) {
  // SelectedIndex is global, reset it's value !!
  SelectedIndex = std::numeric_limits<size_t>::max();

  dlgSelectWaypoint_t dlg(type, FilterNear);

  if (dlg.DoModal() != mrOK) {
    SelectedIndex = std::numeric_limits<size_t>::max();
  }

  return SelectedIndex;
}