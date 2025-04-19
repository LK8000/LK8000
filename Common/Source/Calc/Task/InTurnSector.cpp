/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "task_zone.h"
#include "NavFunctions.h"

namespace {

/*
 * function to check if current position is inside Turnpoint. 
 */

bool InSector(const AGeoPoint& position, const task::sector_data&& data) {
  double distance;
  double bearing;

  data.center.Reverse(position, bearing, distance);

  if (distance < data.max_radius) {
    return AngleInRange(data.start_radial, data.end_radial, AngleLimit360(bearing), true);
  }
  return false;
}

bool InSector(const AGeoPoint& position, const task::circle_data& data) {
  return (position.Distance(data.center) < data.radius);
}

bool InSector(const AGeoPoint& position, const task::dae_data& data) {
  double distance;
  double bearing;

  data.center.Reverse(position, bearing, distance);
  if (distance < 500) {
    return true;
  }

  if (distance < 10000) {
    bearing = AngleLimit180(bearing - data.bisector);
    if (bearing >= -45 && bearing <= 45) {
      return true;
    }
  }
  return false;
}

bool InSector(const AGeoPoint& position, const task::line_data& data) {
  double bearing = position.Bearing(data.center);

  // TODO : check for radius ?

  // check if we passed the bisector
  if (AngleLimit360(data.inbound - data.bisector) < 180) {
    return AngleInRange(Reciprocal(data.bisector), data.bisector, bearing, true);
  } else {
    return AngleInRange(data.bisector, Reciprocal(data.bisector), bearing, true);
  }
}

template <sector_type_t type, int task_type>
bool InSector(int tp_index, const AGeoPoint& position) {
  return InSector(position, task::zone_data<type, task_type>::get(tp_index));
}

struct InSector_t {
  using result_type = bool;

  template <sector_type_t type, int task_type>
  static bool invoke(int tp_index, const AGeoPoint& position) {
    return InSector<type, task_type>(tp_index, position);
  }
};

} // namespace

bool InTurnSector(const AGeoPoint& position, int tp_index) {
  return task::invoke_for_task_point<InSector_t, const AGeoPoint&>(tp_index, position);
}
