/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include <iterator>
#include "NavFunctions.h"
#include "Util/ScopeExit.hxx"
#include "utils/lookup_table.h"
#include "CalcTask.h"

bool TaskModified = false;
bool TargetModified = false;
TCHAR LastTaskFileName[MAX_PATH]= TEXT("\0");

namespace {

constexpr sector_type_t default_start_sectors[] = {
  sector_type_t::CIRCLE,
  sector_type_t::LINE,
  sector_type_t::SECTOR,
};

constexpr sector_type_t aat_start_sectors[] = {
  sector_type_t::CIRCLE,
  sector_type_t::LINE,
};

constexpr sector_type_t default_finish_sectors[] = {
  sector_type_t::CIRCLE,
  sector_type_t::LINE,
  sector_type_t::SECTOR,
};

constexpr sector_type_t aat_finish_sectors[] = {
  sector_type_t::CIRCLE,
  sector_type_t::LINE,
};

constexpr sector_type_t default_task_sectors[] = {
  sector_type_t::CIRCLE,
  sector_type_t::SECTOR,
  sector_type_t::DAe,
  sector_type_t::LINE,
};

constexpr sector_type_t aat_task_sectors[] = {
  sector_type_t::CIRCLE,
  sector_type_t::SECTOR,
};

constexpr sector_type_t gp_task_sectors[] = {
  sector_type_t::CIRCLE,
  sector_type_t::SECTOR,
  sector_type_t::ESS_CIRCLE,
  sector_type_t::DAe,
};


constexpr auto sector_labels_table = lookup_table<sector_type_t, const TCHAR*(*)()>({
  { sector_type_t::CIRCLE , [] { return MsgToken<210>(); } }, // _@M210_ = "Cylinder"
  { sector_type_t::SECTOR , [] { return MsgToken<590>(); } }, // _@M590_ = "Sector"
  { sector_type_t::SECTOR , [] { return MsgToken<274>(); } }, // _@M274_ = "FAI Sector"
  { sector_type_t::DAe, [] { return LKGetText(_T("DAe 0.5/10")); } },
  { sector_type_t::LINE, [] { return MsgToken<393>(); } }, // _@M393_ = "Line"
  { sector_type_t::ESS_CIRCLE , [] { return MsgToken<2189>(); } }, // _@M2189_ = "Circle ESS"
});

template<size_t size>
class task_sectors_adaptor final : public task_sectors {
  using type_array_t = sector_type_t[size];
public:
  explicit task_sectors_adaptor(const type_array_t& array) : type_array(array) {}

  const sector_type_t* begin() const override {
    return std::begin(type_array);
  }
  const sector_type_t* end() const override {
    return std::end(type_array);
  }

  unsigned index(sector_type_t type) const override {
    auto it = std::find(begin(), end(), type);
    if (it != end()) {
      return std::distance(begin(), it);
    }
    return 0;
  }

  sector_type_t type(unsigned idx) const override {
    auto item = std::next(begin(), idx < std::size(type_array) ? idx : 0);
    return *item;
  }

private:
  const type_array_t& type_array;
};

template<size_t size>
std::unique_ptr<task_sectors> make_task_sectors_adaptor(const sector_type_t (&array)[size]) {
  return std::make_unique<task_sectors_adaptor<size>>(array);
}

} // namespace

const TCHAR* get_sectors_label(sector_type_t type) {
  auto label = sector_labels_table.get(type, []() {
    return _T("");
  });
  return label();
}

std::unique_ptr<task_sectors> get_start_sectors(int type) {
  switch (type) {  
  default:
  case TSK_DEFAULT:
  case TSK_GP:
    return make_task_sectors_adaptor(default_start_sectors);
  case TSK_AAT:
    return make_task_sectors_adaptor(aat_start_sectors);
  }
}

std::unique_ptr<task_sectors> get_finish_sectors(int type) {
  switch (type) {  
  default:
  case TSK_DEFAULT:
  case TSK_GP:
    return make_task_sectors_adaptor(default_finish_sectors);
  case TSK_AAT:
    return make_task_sectors_adaptor(aat_finish_sectors);
  }
}

std::unique_ptr<task_sectors> get_task_sectors(int type) {
  switch (type) {  
  default:
  case TSK_DEFAULT:
    return make_task_sectors_adaptor(default_task_sectors);
  case TSK_AAT:
    return make_task_sectors_adaptor(aat_task_sectors);
  case TSK_GP:
    return make_task_sectors_adaptor(gp_task_sectors);
  }
}

GeoPoint GetWayPointPosition(const WAYPOINT& p) {
	return { p.Latitude, p.Longitude };
}

GeoPoint GetWaypointPosition(size_t idx) {
    return GetWayPointPosition(WayPointList[idx]);
}

GeoPoint GetTurnpointPosition(size_t idx) {
    return GetWaypointPosition(Task[idx].Index);
}

GeoPoint GetTurnpointTarget(size_t idx) {
  if (UseAATTarget()) {
    return {
      Task[idx].AATTargetLat,
      Task[idx].AATTargetLon
    };
  }
  return GetTurnpointPosition(idx);
}

void GetTaskSectorParameter(int TskIdx, sector_type_t* SecType, double* SecRadius) {
  if (TskIdx == 0) {
    if (SecType) {
      *SecType = StartLine;
    }
    if (SecRadius) {
      *SecRadius = StartRadius;
    }
  } else if (!ValidTaskPoint(TskIdx + 1)) {
    if (SecType) {
      *SecType = FinishLine;
    }
    if (SecRadius) {
      *SecRadius = FinishRadius;
    }
  } else if (UseAATTarget()) {
    LKASSERT(ValidTaskPoint(TskIdx));  // could be -1
    if (SecType) {
      *SecType = Task[TskIdx].AATType;
    }
    if (SecRadius) {
      if (Task[TskIdx].AATType == sector_type_t::SECTOR) {
        *SecRadius = Task[TskIdx].AATSectorRadius;
      }
      else {
        *SecRadius = Task[TskIdx].AATCircleRadius;
      }
    }
  } else {
    if (SecType) {
      *SecType = SectorType;
    }
    if (SecRadius) {
      *SecRadius = SectorRadius;
    }
  }
}


void ResetTaskWpt(TASK_POINT& TaskWpt) {
    TaskWpt.Index = -1;
    TaskWpt.AATTargetOffsetRadius = 0.0;
    TaskWpt.AATTargetOffsetRadial = 0.0;
    TaskWpt.AATTargetLocked = false;
    TaskWpt.AATType = SectorType;
    TaskWpt.AATSectorRadius = SectorRadius;
    TaskWpt.AATCircleRadius = SectorRadius;
    TaskWpt.AATStartRadial = 0;
    TaskWpt.AATFinishRadial = 360;
}

void ResetTaskStat(TASKSTATS_POINT& StatPt) {
    std::fill(std::begin(StatPt.IsoLine_valid), std::end(StatPt.IsoLine_valid), false);
}

void ResetTaskWaypoint(int j) {
    if(j>=0 && j<MAXTASKPOINTS) {
        ResetTaskWpt(Task[j]);
        ResetTaskStat(TaskStats[j]);
    } else {
        LKASSERT(false);
    }
}

void RefreshTaskWaypoint(int i) {
  if(i==0)
    {
      Task[i].Leg = 0;
      Task[i].InBound = 0;
    }
  else
    {
      if (Task[i-1].Index == Task[i].Index) {
        // Leg is Always 0 !
        Task[i].Leg = 0;

        // InBound need calculated with previous not same as current.
        int j = i-1;
        while(j>=0 && Task[j].Index == Task[i].Index) {
            --j;
        }
        if(j>=0) {
            DistanceBearing(WayPointList[Task[i].Index].Latitude,
                            WayPointList[Task[i].Index].Longitude,
                            WayPointList[Task[j].Index].Latitude,
                            WayPointList[Task[j].Index].Longitude,
                            nullptr,
                            &Task[i].InBound);
        } else {
            j = i+1;
            while(j>=0 && ValidWayPoint(Task[j].Index) && Task[j].Index == Task[i].Index) {
                j++;
            }
            if(ValidWayPoint(Task[j].Index)) {
                DistanceBearing(WayPointList[Task[j].Index].Latitude,
                                WayPointList[Task[j].Index].Longitude,
                                WayPointList[Task[i].Index].Latitude,
                                WayPointList[Task[i].Index].Longitude,
                                nullptr,
                                &Task[i].InBound);
            }
        }
      } else {
            DistanceBearing(WayPointList[Task[i].Index].Latitude,
                      WayPointList[Task[i].Index].Longitude,
                      WayPointList[Task[i-1].Index].Latitude,
                      WayPointList[Task[i-1].Index].Longitude,
                      &Task[i].Leg,
                      &Task[i].InBound);
      }

      Task[i].InBound += 180;
      if (Task[i].InBound >= 360)
        Task[i].InBound -= 360;

      Task[i-1].OutBound = Task[i].InBound;
      Task[i-1].Bisector = BiSector(Task[i-1].InBound,Task[i-1].OutBound);
      if (i==1) {
        if (EnableMultipleStartPoints) {
          for (int j=0; j<MAXSTARTPOINTS; j++) {
            if ((StartPoints[j].Index != -1)&&(StartPoints[j].Active)) {
              DistanceBearing(WayPointList[StartPoints[j].Index].Latitude,
                              WayPointList[StartPoints[j].Index].Longitude,
                              WayPointList[Task[i].Index].Latitude,
                              WayPointList[Task[i].Index].Longitude,
                              nullptr, &StartPoints[j].OutBound);
            }
          }
        }
      }
    }
}


double FindInsideAATSectorDistance(double latitude,
                                   double longitude,
                                   int taskwaypoint,
                                   double course_bearing,
                                   double p_found) {

  double max_distance;
  if(Task[taskwaypoint].AATType == sector_type_t::SECTOR) {
    max_distance = Task[taskwaypoint].AATSectorRadius;
  } else {
    max_distance = Task[taskwaypoint].AATCircleRadius;
  }

  // Do binary bounds search for longest distance within sector

  double delta = max_distance;
  double t_distance_lower = p_found;
  double t_distance = p_found+delta*2;
  int steps = 0;
  do {

    double t_lat, t_lon;
    FindLatitudeLongitude(latitude, longitude,
                          course_bearing, t_distance,
                          &t_lat, &t_lon);

    if (InTurnSector({{t_lat, t_lon}, 0 }, taskwaypoint)) {
      t_distance_lower = t_distance;
      // ok, can go further
      t_distance += delta;
    } else {
      t_distance -= delta;
    }
    delta /= 2.0;
  } while ((delta>5.0)&&(steps++<20));

  return t_distance_lower;
}


double FindInsideAATSectorRange(double latitude,
                                double longitude,
                                int taskwaypoint,
                                double course_bearing,
                                double p_found) {

  double t_distance = FindInsideAATSectorDistance(latitude, longitude, taskwaypoint,
                                                  course_bearing, p_found);
  return (p_found /
          max(1.0,t_distance))*2-1;
}


double DoubleLegDistance(int taskwaypoint,
                         double longitude,
                         double latitude) {

  if (taskwaypoint>0) {
    return DoubleDistance(Task[taskwaypoint-1].AATTargetLat,
			  Task[taskwaypoint-1].AATTargetLon,
			  latitude,
			  longitude,
			  Task[taskwaypoint+1].AATTargetLat,
			  Task[taskwaypoint+1].AATTargetLon);
  } else {
    double d1;
    DistanceBearing(latitude,
		    longitude,
		    Task[taskwaypoint+1].AATTargetLat,
		    Task[taskwaypoint+1].AATTargetLon,
		    &d1, nullptr);
    return d1;
  }

}


const WAYPOINT* TaskWayPoint(size_t idx) {
    if (ValidTaskPoint(idx)) {
        return &WayPointList[Task[idx].Index];
    }
    return nullptr;
}

void ReverseTask() {
	int lower=0;
	int upper = getFinalWaypoint();
	while(lower<upper) { //Swap in pairs starting from the sides of task array
		std::swap(Task[lower++],Task[upper--]);
	}
	ResetTask(false); // Reset the task without showing the message about task reset
	RefreshTask(); //Recalculate the task
	DoStatusMessage(MsgToken<1853>()); // LKTOKEN  _@M1853_ "TASK REVERSED"
}

int GetTaskBearing() {
  LockTaskData();
  AtScopeExit() {
    UnlockTaskData();
  };

  if (ValidTaskPointFast(ActiveTaskPoint)) {
    int index = Task[ActiveTaskPoint].Index;
    if (index>=0) {

      double value = UseAATTarget()
              ? CALCULATED_INFO.WaypointBearing
              : WayPointCalc[index].Bearing;

      return AngleLimit360(value);
    }
  }

  return 0;
}
