/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgSelectAirspace.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "dlgSelectObject.h"
#include "Sound/Sound.h"
#include "utils/stringext.h"

namespace {

struct AirspaceInfo_t : public ObjectAdaptor_t {
  AirspaceInfo_t() = delete;

  explicit AirspaceInfo_t(CAirspace * pAsp) : airspace(pAsp) {};

  void Select() const override {
    LKSound(TEXT("LK_TICK.WAV"));
    CAirspaceManager::Instance().PopupAirspaceDetail(airspace);
  }

  bool Toggle() const override {
    if (airspace->Enabled()) {
      LKSound(TEXT("LK_BEEP0.WAV"));
      CAirspaceManager::Instance().AirspaceDisable(*airspace);
    } else {
      LKSound(TEXT("LK_BEEP1.WAV"));
      CAirspaceManager::Instance().AirspaceEnable(*airspace);
    }
    return false;
  }

  void DrawPicto(LKSurface& Surface, const RECT &rc) const override {
    airspace->DrawPicto(Surface, rc);
  }

  const TCHAR* Name() const override {
    return airspace->Name(); 
  }

  const TCHAR* Type() const override {
    return CAirspaceManager::GetAirspaceTypeShortText(airspace->Type());
  }

  bool FilterType(unsigned type) const override {
    if (type == AIRSPACECLASSCOUNT + 1) {
      // only enabled airspaces
      return !airspace->Enabled();
    } else {
      // only selected class
      return airspace->Type() == static_cast<int>(type) - 1;  
    }
  }

  CAirspace *airspace;
};

class dlgSelectAirspace_t final : public dlgSelectObject  {
public:
  unsigned GetTypeCount() const override {
    //Need to count + 2 because 0 is no-filter and (count + 1) is Disabled
    return AIRSPACECLASSCOUNT + 2;
  }

  const TCHAR* GetTypeLabel(unsigned type) const override {
    if( type == AIRSPACECLASSCOUNT + 1) {
      return MsgToken<239>(); // "_@M00239_": "Disabled"
    } else if (type > 0) {
      return CAirspaceManager::GetAirspaceTypeText(type-1);
    } 
    return _T("*");
  }

  int GetTypeWidth(LKSurface& Surface) override {
    if(TypeWidth < 0) {
      TypeWidth = 0;
      for (unsigned i = 0 ; i < AIRSPACECLASSCOUNT; ++i) {
        const TCHAR* szShortLabel = CAirspaceManager::GetAirspaceTypeShortText(i);
        TypeWidth = std::max(TypeWidth, Surface.GetTextWidth(szShortLabel));
      }
    }
    return TypeWidth;
  }  

  const TCHAR* GetFilterLabel() const override {
    return MsgToken<68>(); // "_@M68_": "Airspace"
  };

protected:

  const TCHAR* GetCaption() const override {
    return MsgToken<591>();
  };

  array_info_t PrepareData(const GeoPoint& position) override {
    array_info_t data;
    try {
      CAirspaceList Airspaces = CAirspaceManager::Instance().GetAllAirspaces(); // this do a copy of CAirspaceList (list of pointer)

      data.reserve(Airspaces.size());

      auto ToSelectInfo = [&](CAirspace* pAsp) -> ObjectSelectInfo_t {
        double direction = 0;
        double distance = std::max(0., DISTANCEMODIFY * pAsp->Range(position, direction));
        return { distance, direction, std::make_unique<AirspaceInfo_t>(pAsp) };
      };

      std::transform(std::begin(Airspaces), std::end(Airspaces), std::back_inserter(data), ToSelectInfo);

    } catch (std::bad_alloc&) {
      OutOfMemory(_T(__FILE__),__LINE__);
    }
    return data;
  }

  int TypeWidth = -1; // cahed type column width.
};

} // namespace

void dlgSelectAirspace() {
  dlgSelectAirspace_t dlg;
  dlg.DoModal();
}