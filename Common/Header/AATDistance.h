/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: AATDistance.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef AATDISTANCE_H
#define AATDISTANCE_H

class AATDistance final {
public:
  AATDistance() = default;

  void Reset();

  void AddPoint(const GeoPoint& position, int taskwaypoint);
  double DistanceCovered(const GeoPoint& position);
  double LegDistanceAchieved(int taskwaypoint) const;
  bool HasEntered(int taskwaypoint) const;
  void ResetEnterTrigger(int taskwaypoint);

private:
  double DistanceCovered_internal(const GeoPoint& position, bool insector);
  double DistanceCovered_inside(const GeoPoint& position);
  double DistanceCovered_outside(const GeoPoint& position);
  double distance_achieved(int taskwaypoint, int jbest, const GeoPoint& position);

  void ThinData(int taskwaypoint);

  struct aat_distance {
    GeoPoint position = {0., 0.};
    double distance = 0.;
  };

  constexpr static double DISTANCETHRESHOLD = 500;

  struct aat_data {
    double distancethreshold = DISTANCETHRESHOLD;
    double legdistance_achieved = 0.;
    bool has_entered = false;

    std::vector<aat_distance> points = {};
    size_t best;
  };

  std::array<aat_data, MAXTASKPOINTS> aat_datas;

  void ShiftTargetFromBehind(const GeoPoint& position, int taskwaypoint) const;
  void ShiftTargetFromInFront(const GeoPoint& position, int taskwaypoint) const;
};

extern AATDistance aatdistance;

double AATCloseDistance();

#endif
