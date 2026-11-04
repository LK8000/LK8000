/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "ZoomTopology.h"
#include "Terrain.h"
#include "RGB.h"

extern Topology* TopoStore[MAXTOPOLOGY];

namespace {

// clang-format off

// coast_areas and water_area zoom are special cases :
//  - Polygon are always drawn, regardless of zoom level.
//  - Zoom level (shared with "water_line") is used for determining when to
//    display labels.
constexpr const TopologyZoomLevel topology_props[] = {
    { topology_category::coast_areas,     LKTopoZoomCat20,  10, _T("")          },
    { topology_category::water_area,      LKTopoZoomCat20,  10, _T("")          },
    { topology_category::water_line,      LKTopoZoomCat20,  10, _T("prpCat20")  },
    { topology_category::big_road,        LKTopoZoomCat30,  25, _T("prpCat30")  },
    { topology_category::medium_road,     LKTopoZoomCat40,   6, _T("prpCat40")  },
    { topology_category::small_road,      LKTopoZoomCat50,   3, _T("prpCat50")  },
    { topology_category::railroad,        LKTopoZoomCat60,   8, _T("prpCat60")  },
    { topology_category::big_city,        LKTopoZoomCat70,  15, _T("prpCat70")  },
    { topology_category::medium_city,     LKTopoZoomCat80,  10, _T("prpCat80")  },
    { topology_category::small_city,      LKTopoZoomCat90,   6, _T("prpCat90")  },
    { topology_category::very_small_city, LKTopoZoomCat100,  3, _T("prpCat100") },
    { topology_category::city_area,       LKTopoZoomCat110, 15, _T("prpCat110") },
};
// clang-format on

}  // namespace

const TopologyZoomLevel* TopologyZoomLevel::list::begin() {
  return std::begin(topology_props);
}

const TopologyZoomLevel* TopologyZoomLevel::list::end() {
  return std::end(topology_props);
}

const TopologyZoomLevel* TopologyZoomLevel::get(topology_category category) {
  auto it = std::ranges::find(topology_props, category, &TopologyZoomLevel::category_);
  if (it != std::end(topology_props)) {
    return it;
  }
  return nullptr;
}

void TopologyZoomLevel::set(topology_category category, double value) {
  using std::ranges::find;
  auto it = find(topology_props, category, &TopologyZoomLevel::category_);
  if (it != std::end(topology_props)) {
    if (it->ref_zoom_ != value) {
      it->ref_zoom_ = value;
    }
  }
}

double TopologyZoomLevel::DefaultZoom(topology_category category) {
  auto prop = TopologyZoomLevel::get(category);
  if (prop) {
    return prop->default_zoom_;
  }
  return 100;  // fallback default value
}

void TopologyZoomLevel::list::reset() {
  for (auto& prop : topology_props) {
    prop.ref_zoom_ = prop.default_zoom_;
  }
}
