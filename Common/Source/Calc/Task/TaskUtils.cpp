/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include <iterator>
#include "NavFunctions.h"

bool TaskModified = false;
bool TargetModified = false;
TCHAR LastTaskFileName[MAX_PATH]= TEXT("\0");



int GetTaskSectorParameter(int TskIdx, int *SecType, double *SecRadius)
{
*SecType = LINE;
  if(TskIdx ==0 )
  {
	*SecType = StartLine;
	if(StartLine ==0)
	  *SecType = CIRCLE;
	if(StartLine ==1)
	  *SecType = LINE;
	if(StartLine ==2)
	  *SecType = SECTOR;

    if(SecRadius)
      *SecRadius = (double)StartRadius;
  }
  else
  {
    if(!ValidTaskPoint(TskIdx+1) )
    {
      *SecType = FinishLine;
      if(FinishLine ==0)
	*SecType = CIRCLE;
	  if(FinishLine ==1)
		*SecType = LINE;
	  if(FinishLine ==2)
	*SecType = SECTOR;

      if(SecRadius)
	    *SecRadius  = (double)FinishRadius;
    }
    else
    {
      if(UseAATTarget())
      {
        LKASSERT(ValidTaskPoint(TskIdx)); // could be -1
	*SecType = Task[TskIdx].AATType;

        if(SecRadius)
	  *SecRadius  = Task[TskIdx].AATCircleRadius;

        switch(Task[TskIdx].AATType) {
            case 0:
                *SecType = CIRCLE;
                break;
            case 1:
                *SecType = SECTOR;
                if(SecRadius)
                  *SecRadius  = Task[TskIdx].AATSectorRadius;
                break;
            case 2:
                *SecType = CONE;
                break;
            case 3:
                *SecType = ESS_CIRCLE;
                break;
        }
      }
      else
      {
	*SecType = SectorType;
/*
	if(SectorType ==0)
	  *SecType = CIRCLE;
	if(SectorType ==1)
	  *SecType = SECTOR;
	if(SectorType ==2)
	  *SecType = DAe;
	if(SectorType ==3)
	  *SecType = LINE;
*/
        if(SecRadius)
	  *SecRadius = SectorRadius;
      }
    }
  }
  return 0;
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
    TaskWpt.OutCircle = false;
    TaskWpt.PGConeBase = 0;
    TaskWpt.PGConeSlope = 2.5;
    TaskWpt.PGConeBaseRadius = 0.;
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

void ResetStartPoint(START_POINT& StartPt) {
    StartPt.Index = -1;
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
                            NULL,
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
                                NULL,
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
                              NULL, &StartPoints[j].OutBound);
            }
          }
        }
      }
    }
}



double FindInsideAATSectorDistance_old(double latitude,
                                       double longitude,
                                       int taskwaypoint,
                                       double course_bearing,
                                       double p_found) {
  bool t_in_sector;
  double delta;
  double max_distance;
  if(Task[taskwaypoint].AATType == SECTOR) {
    max_distance = Task[taskwaypoint].AATSectorRadius*2;
  } else {
    max_distance = Task[taskwaypoint].AATCircleRadius*2;
  }
  delta = max(250.0, max_distance/40.0);

  double t_distance = p_found;
  double t_distance_inside;

  do {
    double t_lat, t_lon;
    t_distance_inside = t_distance;
    t_distance += delta;

    FindLatitudeLongitude(latitude, longitude,
                          course_bearing, t_distance,
                          &t_lat,
                          &t_lon);

    t_in_sector = InAATTurnSector(t_lon,
                                  t_lat,
                                  taskwaypoint, 0);

  } while (t_in_sector);

  return t_distance_inside;
}


double FindInsideAATSectorDistance(double latitude,
                                   double longitude,
                                   int taskwaypoint,
                                   double course_bearing,
                                   double p_found) {

  double max_distance;
  if(Task[taskwaypoint].AATType == SECTOR) {
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

    if (InAATTurnSector(t_lon, t_lat, taskwaypoint, 0)) {
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
		    &d1, NULL);
    return d1;
  }

}


const WAYPOINT* TaskWayPoint(size_t idx) {
    if (ValidTaskPoint(idx)) {
        return &WayPointList[Task[idx].Index];
    }
    return NULL;
}

void ReverseTask() {
	int lower=0;
	int upper = getFinalWaypoint();
	while(lower<upper) { //Swap in pairs starting from the sides of task array
		std::swap(Task[lower++],Task[upper--]);
	}
	ResetTask(false); // Reset the task without showing the message about task reset
	RefreshTask(); //Recalculate the task
	DoStatusMessage(MsgToken(1853)); // LKTOKEN  _@M1853_ "TASK REVERSED"
}
