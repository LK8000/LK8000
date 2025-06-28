/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "../task_zone.h"
#include "Draw/Task/TaskRendererMgr.h"
#include "Geographic/GeoPoint.h"

namespace {

void SetSectorRenderer(int tp_index, const task::circle_data& data) {
  gTaskSectorRenderer.SetCircle(tp_index, data.center, data.radius);
}

void SetSectorRenderer(int tp_index, const task::sector_data& data) {
  if (tp_index == 0) {
    gTaskSectorRenderer.SetStartSector(tp_index, data.center, data.max_radius, data.start_radial, data.end_radial);
  } else {
    gTaskSectorRenderer.SetSector(tp_index, data.center, data.max_radius, data.start_radial, data.end_radial);
  }
}

void SetSectorRenderer(int tp_index, const task::dae_data& data) {
  gTaskSectorRenderer.SetDae(tp_index, data.center, data.bisector - 45, data.bisector + 45);
}

void SetSectorRenderer(int tp_index, const task::line_data& data) {
  if (tp_index == 0) {
    gTaskSectorRenderer.SetLine(tp_index, data.center, data.radius, data.outbound + 180);
  } else if (ValidTaskPointFast(tp_index + 1)) {
    gTaskSectorRenderer.SetLine(tp_index, data.center, data.radius, data.bisector + 90);
  } else {
    gTaskSectorRenderer.SetLine(tp_index, data.center, data.radius, data.inbound);
  }
}

void SetSectorRenderer(int tp_index, const task::sgp_start_data& data) {
  assert(tp_index == 0);
  if (tp_index == 0) {
    gTaskSectorRenderer.SetGPStart(tp_index, data.center, data.radius, data.outbound + 180);
  }
}

struct SetSectorRenderer_t {
  using result_type = void;

  template <sector_type_t type, task_type_t task_type>
  static void invoke(int tp_index) {
    SetSectorRenderer(tp_index, task::zone_data<type, task_type>::get(tp_index));
  }
};


}  // namespace

void CalculateTaskSectors(int tp_index) {
  task::invoke_for_task_point<SetSectorRenderer_t>(tp_index);
}

void CalculateTaskSectors() {

    LockTaskData();

    if (EnableMultipleStartPoints) {
        for (int i = 0; i < MAXSTARTPOINTS - 1; i++) {
            const START_POINT &StartPt = StartPoints[i];
            if (StartPt.Active && ValidWayPoint(StartPt.Index)) {
                const WAYPOINT &StartWpt = WayPointList[StartPt.Index];
                const GeoPoint center(StartWpt.Latitude, StartWpt.Longitude);
                switch (StartLine) {
                case sector_type_t::SECTOR:
                    gStartSectorRenderer.SetStartSector(i, center, StartRadius,
                                                        StartPt.OutBound + 45,
                                                        StartPt.OutBound - 45);
                    break;
                case sector_type_t::LINE:
                    gStartSectorRenderer.SetLine(i, center, StartRadius, StartPt.OutBound);
                    break;
                case sector_type_t::SGP_START:
                case sector_type_t::CIRCLE:
                case sector_type_t::DAe:
                case sector_type_t::ESS_CIRCLE:
                    assert(false);
                    break;
                }
            }
        }
    }

    for (int i = 0; i < MAXTASKPOINTS; i++) {
        CalculateTaskSectors(i);
    }
    UnlockTaskData();
}
