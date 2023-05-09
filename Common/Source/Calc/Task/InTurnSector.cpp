/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "NavFunctions.h"
#include "Calc/Task/PGTask/PGConeTaskPt.h"

namespace {

GeoPoint from_task(int tp) {
  return {
    WayPointList[Task[tp].Index].Latitude,
    WayPointList[Task[tp].Index].Longitude
  };
}

/**
 *  struct to store data required to check if current position is inside Turnpoint.
 */

struct sector_data {
  GeoPoint center;
  double start_radial;
  double end_radial;
  double max_radius;
};

struct circle_data {
  GeoPoint center;
  double radius;
};

struct dae_data {
  GeoPoint center;
  double bisector;
};

struct line_data {
  GeoPoint center;
  double bisector;
  double inbound;
  // TODO : Line as no radius ?
};

struct conical_data {
  GeoPoint center;
  double slope;
  double base_altitude;
  double base_radius;
};

/**
 *  helper to get sector data struct from Task definition.
 */

template <sector_type_t type, int task_type>
struct sector;

template <>
struct sector<sector_type_t::SECTOR, TSK_DEFAULT> {
  static sector_data get(int tp_index) {
    return {
      from_task(tp_index),
      Task[tp_index].Bisector - 45.,
      Task[tp_index].Bisector + 45.,
      SectorRadius
    };
  }
};

template <>
struct sector<sector_type_t::CIRCLE, TSK_DEFAULT> {
  static circle_data get(int tp_index) {
    return {
      from_task(tp_index),
      SectorRadius
    };
  }
};

template <int task_type>
struct sector<sector_type_t::DAe, task_type> {
  static dae_data get(int tp_index) {
    return {
      from_task(tp_index),
      Task[tp_index].Bisector
    };
  }
};

template <>
struct sector<sector_type_t::LINE, TSK_DEFAULT> {
  static line_data get(int tp_index) {
    return {
      from_task(tp_index),
      Task[tp_index].Bisector,
      Task[tp_index].InBound
    };
  }
};

template <>
struct sector<sector_type_t::SECTOR, TSK_AAT> {
  static sector_data get(int tp_index) {
    return {
        from_task(tp_index), 
        Task[tp_index].AATStartRadial,
        Task[tp_index].AATFinishRadial,
        Task[tp_index].AATSectorRadius
        // TODO : implement AAT min radius
    };
  }
};

template <>
struct sector<sector_type_t::CIRCLE, TSK_AAT> {
  static circle_data get(int tp_index) {
    return {
      from_task(tp_index),
      Task[tp_index].AATCircleRadius
    };
  }
};

template <>
struct sector<sector_type_t::CONE, TSK_GP> {
  static conical_data get(int tp_index) {
    return {
      from_task(tp_index),
      Task[tp_index].PGConeSlope,
      Task[tp_index].PGConeBase,
      Task[tp_index].PGConeBaseRadius
    };
  }
};

/*
 * function to check if current position is inside Turnpoint. 
 */

bool InSector(const AGeoPoint& position, const sector_data&& data) {
  double distance;
  double bearing;

  data.center.Reverse(position, bearing, distance);

  if (distance < data.max_radius) {
    return AngleInRange(data.start_radial, data.end_radial, AngleLimit360(bearing), true);
  }
  return false;
}

bool InSector(const AGeoPoint& position, const circle_data& data) {
  return (position.Distance(data.center) < data.radius);
}

bool InSector(const AGeoPoint& position, const dae_data& data) {
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

bool InSector(const AGeoPoint& position, const line_data& data) {
  double bearing = position.Bearing(data.center);

  // TODO : check for radius ?

  // check if we passed the bisector
  if (AngleLimit360(data.inbound - data.bisector) < 180) {
    return AngleInRange(Reciprocal(data.bisector), data.bisector, bearing, true);
  } else {
    return AngleInRange(data.bisector, Reciprocal(data.bisector), bearing, true);
  }
}

bool InSector(const AGeoPoint& position, const conical_data& data) {
  double radius = PGConeTaskPt::ConeRadius(position.altitude, data.base_altitude, data.slope, data.base_radius);
  return (position.Distance(data.center) < radius);
}


template <sector_type_t type, int task_type>
bool InSector(const AGeoPoint& position, int tp_index) {
  return InSector(position, sector<type, task_type>::get(tp_index));
}

} // namespace

bool InTurnSector(const AGeoPoint& position, int tp_index) {
  ScopeLock lock(CritSec_TaskData);
  if (!ValidTaskPointFast(tp_index)) {
    return false;
  }

  sector_type_t type = (UseAATTarget() ? Task[tp_index].AATType : SectorType);
  switch (type) {
    case sector_type_t::ESS_CIRCLE:
    case sector_type_t::CIRCLE:
      if (UseAATTarget()) {
        return InSector<sector_type_t::CIRCLE, TSK_AAT>(position, tp_index);
      } else {
        return InSector<sector_type_t::CIRCLE, TSK_DEFAULT>(position, tp_index);
      }
    case sector_type_t::DAe:
      return InSector<sector_type_t::DAe, TSK_DEFAULT>(position, tp_index);
    case sector_type_t::SECTOR:
      if (UseAATTarget()) {
        return InSector<sector_type_t::SECTOR, TSK_AAT>(position, tp_index);
      } else {
        return InSector<sector_type_t::SECTOR, TSK_DEFAULT>(position, tp_index);
      }
    case sector_type_t::LINE:
      return InSector<sector_type_t::LINE, TSK_DEFAULT>(position, tp_index);
    case sector_type_t::CONE:
      return InSector<sector_type_t::CONE, TSK_GP>(position, tp_index);
    default:
      assert(false);
      break;
  }
  return false;
}
