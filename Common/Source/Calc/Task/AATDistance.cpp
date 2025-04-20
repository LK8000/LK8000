/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: AATDistance.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "AATDistance.h"
#include "CalcTask.h"
#include "NavFunctions.h"
#include "Waypointparser.h"

constexpr size_t MAXNUM_AATDISTANCE = 50;

AATDistance aatdistance;

void AATDistance::Reset() {
  (*this) = {};
}

double AATDistance::LegDistanceAchieved(int taskwaypoint) const {
  return aat_datas[taskwaypoint].legdistance_achieved;
}

void AATDistance::ResetEnterTrigger(int taskwaypoint) {
  aat_datas[taskwaypoint].has_entered = false;
}

void AATDistance::AddPoint(const GeoPoint& position, int taskwaypoint) {
  if (taskwaypoint < 0) {
    return;
  }

  ScopeLock lock(CritSec_TaskData);
  if (gTaskType != task_type_t::AAT) {
    return;  // nothing else to do for non-AAT tasks
  }

  aat_data& data = aat_datas[taskwaypoint];
  bool was_entered = std::exchange(data.has_entered, true);

  if (data.points.size() >= MAXNUM_AATDISTANCE) {
    ThinData(taskwaypoint);
  }

  bool add_point = data.points.empty();
  if (!add_point && data.points.size() > 1) {
    const GeoPoint& p = std::next(data.points.rbegin(), 1)->position;
    double d = p.Distance(position);
    add_point = (d >= data.distancethreshold);
  }

  double distance = 0.;
  if (taskwaypoint > 0) {
    aat_data& prev_data = aat_datas[taskwaypoint - 1];
    if (taskwaypoint == 1 && prev_data.points.empty()) {
      prev_data.points.push_back({{Task[0].AATTargetLat, Task[0].AATTargetLon}, 0.});
    }

    // find the longest distance
    for (auto& p : prev_data.points) {
      double d = p.distance + p.position.Distance(position);
      if (d > distance) {
        distance = d;
      }
    }
  }

  if (add_point || data.points.empty()) {
    data.points.push_back({position, distance});
  } else {
    data.points.back() = {position, distance};
  }

  if (data.points[data.best].distance < distance) {
    data.best = data.points.size() - 1;
  }

  if (data.points.size() == 1) {
    if ((!was_entered) && (taskwaypoint > 0) && !Task[taskwaypoint].AATTargetLocked) {
      Task[taskwaypoint].AATTargetOffsetRadial = 0.0;
    }
  }

  // update max search for future waypoints
  if (taskwaypoint == ActiveTaskPoint) {
    DistanceCovered_internal(position, true);
  }
}

void AATDistance::ShiftTargetFromInFront(const GeoPoint& position, int taskwaypoint) const {
  double course_bearing;

  // this point is in sector and is improved

  // JMW, now moves target to in line with previous target whenever
  // you are in AAT sector and improving on the target distance

  //JMWAAT  Task[taskwaypoint].AATTargetOffsetRadial = -1.0;

  if (Task[taskwaypoint].AATTargetLocked) {
    // have improved on the locked value, so unlock it in case user
    // wants to move it.
    Task[taskwaypoint].AATTargetOffsetRadius = -1.0;
    Task[taskwaypoint].AATTargetOffsetRadial = 0;
    Task[taskwaypoint].AATTargetLocked = false;
  }

  DistanceBearing(Task[taskwaypoint-1].AATTargetLat,
                  Task[taskwaypoint-1].AATTargetLon,
                  position.latitude,
                  position.longitude,
                  NULL, &course_bearing);

  course_bearing = AngleLimit360(course_bearing+
                                 Task[taskwaypoint].AATTargetOffsetRadial);

  FindLatitudeLongitude(position.latitude, position.longitude,
                        course_bearing, AATCloseDistance(),
                        &Task[taskwaypoint].AATTargetLat,
                        &Task[taskwaypoint].AATTargetLon);
  // JMW, distance here was 100m, now changed to speed * 2

  UpdateTargetAltitude(Task[taskwaypoint]);

  TargetModified = true;
  CalculateAATIsoLines();
}

extern bool TargetDialogOpen;

void AATDistance::ShiftTargetFromBehind(const GeoPoint& position, int taskwaypoint) const {
  // JMWAAT if being externally updated e.g. from task dialog, don't move it
  if (TargetDialogOpen) return;
  if (taskwaypoint==0) return;

  // best is decreasing or first entry in sector, so project
  // target in direction of improvement or first entry into sector

  double course_bearing;
  double course_bearing_orig;
  double d_total_orig;
  double d_total_this;

  d_total_this = DoubleLegDistance(taskwaypoint,
                                   position.longitude,
                                   position.latitude);

  d_total_orig = DoubleLegDistance(taskwaypoint,
                                   Task[taskwaypoint].AATTargetLon,
                                   Task[taskwaypoint].AATTargetLat);

  if (d_total_this>d_total_orig-2.0*AATCloseDistance()) {
    // this is better than the previous best! (or very close)
    ShiftTargetFromInFront(position, taskwaypoint);
    return;
  }

  // JMWAAT if locked, don't move it
  if (Task[taskwaypoint].AATTargetLocked) {
    return;
  }

  DistanceBearing(Task[taskwaypoint-1].AATTargetLat,
                  Task[taskwaypoint-1].AATTargetLon,
                  position.latitude,
                  position.longitude,
                  NULL, &course_bearing);
  course_bearing = AngleLimit360(course_bearing+
                                 Task[taskwaypoint].AATTargetOffsetRadial);

  DistanceBearing(position.latitude,
                  position.longitude,
                  Task[taskwaypoint].AATTargetLat,
                  Task[taskwaypoint].AATTargetLon,
                  NULL, &course_bearing_orig);

  if (fabs(AngleLimit180(course_bearing-course_bearing_orig))<5.0) {
    // don't update it if course deviation is less than 5 degrees,
    // otherwise we end up wasting a lot of CPU in recalculating, and also
    // the target ends up drifting.
    return;
  }

  double max_distance =
    FindInsideAATSectorDistance(position.latitude,
                                position.longitude,
                                taskwaypoint,
                                course_bearing,
                                0);

  // total distance of legs from previous through this to next target
  double delta = max_distance/2;

  // move target in line with previous target along track
  // at an offset to improve on max distance

  double t_distance_lower = 0;
  double t_distance = delta*2;

  int steps = 0;

  do {
    // find target position along projected line but
    // make sure it is in sector, and set at a distance
    // to preserve total task distance
    // we are aiming to make d_total_this = d_total_orig

    GeoPoint t_position = position.Direct(course_bearing, t_distance);

    if (InTurnSector({t_position, 0}, taskwaypoint)) {
      d_total_this = DoubleLegDistance(taskwaypoint, t_position.longitude, t_position.latitude);
      if (d_total_orig - d_total_this > 0.0) {
        t_distance_lower = t_distance;
        // ok, can go further
        t_distance += delta;
      } else {
        t_distance -= delta;
      }
    } else {
      t_distance -= delta;
    }
    delta /= 2.0;
  } while ((delta > 5.0) && (steps++ < 20));

  // now scan to edge of sector to find approximate range %
  if (t_distance_lower>5.0) {
    FindLatitudeLongitude(position.latitude, position.longitude,
                          course_bearing, t_distance_lower,
                          &Task[taskwaypoint].AATTargetLat,
                          &Task[taskwaypoint].AATTargetLon);

    UpdateTargetAltitude(Task[taskwaypoint]);

    Task[taskwaypoint].AATTargetOffsetRadius =
      FindInsideAATSectorRange(position.latitude,
                               position.longitude,
                               taskwaypoint, course_bearing,
                               t_distance_lower);
    TargetModified = true;
    CalculateAATIsoLines();

  }
}

double AATDistance::DistanceCovered_internal(const GeoPoint& position, bool insector) {
  if (!ValidTaskPointFast(ActiveTaskPoint) || (ActiveTaskPoint==0)) {
    //   max_achieved_distance = 0;
    return 0.0;
  }

  if (insector) {
    return DistanceCovered_inside(position);
  } else {
    return DistanceCovered_outside(position);
  }
}

double AATDistance::DistanceCovered_inside(const GeoPoint& position) {
  const aat_data& data = aat_datas[ActiveTaskPoint];

  if (data.points.empty()) {
    // not actually in this sector?
    return 0.0;
  } else {
    if (ValidTaskPointFast(ActiveTaskPoint + 1)) {
      ShiftTargetFromBehind(position, ActiveTaskPoint);
    }
    return distance_achieved(ActiveTaskPoint, data.best, position);
  }
}

double AATDistance::distance_achieved(int taskwaypoint, int jbest, const GeoPoint& position) {
  aat_data& data = aat_datas[taskwaypoint];

  data.legdistance_achieved = 0;

  double achieved = data.points[jbest].distance;
  const GeoPoint& pt = data.points[jbest].position;
  double d0a = pt.Distance(position);
  if (d0a > 0) {
    // Calculates projected distance from P3 along line P1-P2
    data.legdistance_achieved =
        ProjectedDistance(pt.longitude, pt.latitude, Task[taskwaypoint + 1].AATTargetLon,
                          Task[taskwaypoint + 1].AATTargetLat, position.longitude, position.latitude, nullptr, nullptr);

    achieved += data.legdistance_achieved;
  }
  return achieved;
}

double AATDistance::DistanceCovered_outside(const GeoPoint& position) {
  if (ActiveTaskPoint <= 0) {
    return 0.0;
  }

  size_t nstart = 0;

  aat_data& prev_data = aat_datas[ActiveTaskPoint - 1];
  if (ActiveTaskPoint == 1) {
    if (prev_data.points.empty()) {
      prev_data.points.push_back({{ Task[0].AATTargetLat, Task[0].AATTargetLon }, 0.});
    }
    nstart = prev_data.points.size() - 1;
  }

  if (prev_data.points.empty()) {
    // no points in previous sector!
    return 0.0;
  }

  double best_doubleleg_distance = 0;
  int jbest = -1;

  for (size_t j = nstart; j < prev_data.points.size(); j++) {

    double d0t = prev_data.points[j].position.Distance({Task[ActiveTaskPoint].AATTargetLat, Task[ActiveTaskPoint].AATTargetLon});
    double doubleleg_distance = prev_data.points[j].distance + d0t;

    if (doubleleg_distance > best_doubleleg_distance) {
      best_doubleleg_distance = doubleleg_distance;
      jbest = j;
    }
  }

  if (jbest >= 0) {
    // set previous target for display purposes
    prev_data.best = jbest;
    
    Task[ActiveTaskPoint - 1].AATTargetLat = prev_data.points[jbest].position.latitude;
    Task[ActiveTaskPoint - 1].AATTargetLon = prev_data.points[jbest].position.longitude;
    
    return distance_achieved(ActiveTaskPoint - 1, jbest, position);
  }

  return 0.;
}

/*
JMW
  Notes: distancecovered can decrease if the glider flys backwards along
  the task --- since points for outlanding are determined not on
  greatest distance along task but landing location, this is reasonable.

*/


double AATDistance::DistanceCovered(const GeoPoint& position) {
  ScopeLock lock(CritSec_TaskData); 
  return DistanceCovered_internal(position, false);
}

bool AATDistance::HasEntered(int taskwaypoint) const {
  ScopeLock lock(CritSec_TaskData);
  return aat_datas[taskwaypoint].has_entered;
}

void AATDistance::ThinData(int taskwaypoint) {
  constexpr double contractfactor = 0.8;

  aat_data& data = aat_datas[taskwaypoint];

  while (data.points.size() > (MAXNUM_AATDISTANCE * contractfactor)) {
    data.distancethreshold /= contractfactor;

    for (size_t i = data.points.size() - 1; i > 0; i--) {
      double d = data.points[i].position.Distance(data.points[i - 1].position);
      if ((d < data.distancethreshold) && (data.best != i)) {
        data.points.erase(std::next(data.points.begin(), i));
      }
    }
  }
}

double AATCloseDistance() {
  return max(100.0, GPS_INFO.Speed * 1.5);
}
