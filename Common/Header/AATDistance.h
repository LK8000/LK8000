/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: AATDistance.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef AATDISTANCE_H
#define AATDISTANCE_H


#define MAXNUM_AATDISTANCE 50

class AATDistance {
public:
  AATDistance();
  void Reset();

  void AddPoint(double longitude, double latitude, int taskwaypoint);
  double DistanceCovered(double longitude, double latitude, int taskwaypoint);
  double LegDistanceAchieved(int taskwaypoint);
  bool HasEntered(int taskwaypoint);
  void ResetEnterTrigger(int taskwaypoint);

private:

  double DistanceCovered_internal(double longitude, double latitude,
                                  bool insector);
  double DistanceCovered_inside(double longitude, double latitude);
  double DistanceCovered_outside(double longitude, double latitude);
  double distance_achieved(int taskwaypoint, int jbest,
                           double longitude, double latitude);

  void UpdateSearch(int taskwaypoint);
  void ThinData(int taskwaypoint);

  double max_achieved_distance;

  bool has_entered[MAXTASKPOINTS];
  double distancethreshold[MAXTASKPOINTS];
  double legdistance_achieved[MAXTASKPOINTS];
  // index to max distance sample to task point n
  int imax[MAXTASKPOINTS][MAXNUM_AATDISTANCE];

  double Dmax[MAXTASKPOINTS][MAXNUM_AATDISTANCE];

  double lat_points[MAXTASKPOINTS][MAXNUM_AATDISTANCE];
  double lon_points[MAXTASKPOINTS][MAXNUM_AATDISTANCE];
  int best[MAXTASKPOINTS];
  int num_points[MAXTASKPOINTS];

  void ShiftTargetFromBehind(double longitude, double latitude,
                             int taskwaypoint);
  void ShiftTargetFromInFront(double longitude, double latitude,
                              int taskwaypoint);
  void ShiftTargetOutside(double longitude, double latitude,
                          int taskwaypoint);
};

double AATCloseDistance(void);

#endif
