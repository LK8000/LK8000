/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Task.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "Task.h"
#include "Logger.h"
#include "AATDistance.h"
#include "Utils.h"
#include "Calculations.h"
#include "Waypointparser.h"
#include "McReady.h"
#include "LKMapWindow.h"

using std::min;
using std::max;

bool TaskModified = false;
bool TargetModified = false;

#define TASKINDEX    Task[ActiveWayPoint].Index

TCHAR LastTaskFileName[MAX_PATH]= TEXT("\0");
extern bool TargetDialogOpen;

extern AATDistance aatdistance;

void ResetTaskWaypoint(int j) {
  Task[j].Index = -1;
  if (DoOptimizeRoute())
  	Task[j].AATTargetOffsetRadius = -100.0;
  else
  	Task[j].AATTargetOffsetRadius = 0.0;
  Task[j].AATTargetOffsetRadial = 0.0;
  Task[j].AATTargetLocked = false;
  Task[j].AATSectorRadius = SectorRadius;
  Task[j].AATCircleRadius = SectorRadius;
  Task[j].AATStartRadial = 0;
  Task[j].AATFinishRadial = 360;
}


void FlyDirectTo(int index) {
  if (!CheckDeclaration())
    return;

  LockTaskData();

  TaskModified = true;
  TargetModified = true;
  ActiveWayPoint = -1; 

  AATEnabled = FALSE;

  /*  JMW disabled this so task info is preserved
  for(int j=0;j<MAXTASKPOINTS;j++)
  {
    ResetTaskWaypoint(j);
  }
  */

  InsertRecentList(index);

  Task[0].Index = index;
  for (int i=1; i<=MAXTASKPOINTS; i++) {
    Task[i].Index = -1;
  }
  ActiveWayPoint = 0;
  RefreshTask();
  UnlockTaskData();
}


// Swaps waypoint at current index with next one.
void SwapWaypoint(int index) {
  if (!CheckDeclaration())
    return;

  LockTaskData();
  TaskModified = true;
  TargetModified = true;
  if (index<0) {
    return;
  }
  if (index+1>= MAXTASKPOINTS-1) {
    return;
  }
  if ((Task[index].Index != -1)&&(Task[index+1].Index != -1)) {
    TASK_POINT tmpPoint;
    tmpPoint = Task[index];
    Task[index] = Task[index+1];
    Task[index+1] = tmpPoint;
  }
  RefreshTask();
  UnlockTaskData();
}


// Inserts a waypoint into the task, in the
// position of the ActiveWaypoint.  If append=true, insert at end of the
// task.
void InsertWaypoint(int index, bool append) {
  if (!CheckDeclaration())
    return;

  int i;
  
  LockTaskData();
  TaskModified = true;
  TargetModified = true;

  if ((ActiveWayPoint<0) || !ValidTaskPoint(0)) {
    ActiveWayPoint = 0;
    ResetTaskWaypoint(ActiveWayPoint);
    Task[ActiveWayPoint].Index = index;

    UnlockTaskData();
    return;
  }
  
  if (ValidTaskPoint(MAXTASKPOINTS-1)) {
    // No room for any more task points!
    MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M727_ = "Too many waypoints in task!" 
      gettext(TEXT("_@M727_")),
	// LKTOKEN  _@M357_ = "Insert Waypoint" 
      gettext(TEXT("_@M357_")),
      MB_OK|MB_ICONEXCLAMATION);
    
    UnlockTaskData();
    return;
  }

  int indexInsert = max(ActiveWayPoint,0);
  if (append) {
	for (i=indexInsert; i<MAXTASKPOINTS-2; i++) {
		#if 100526
		if (Task[i+2].Index<0 && Task[i+1].Index>=0) {
			// i+1 is the last one, so we insert before the last one: shift i+1 to i+2, insert in i+1
			Task[i+2] = Task[i+1];
			ResetTaskWaypoint(i+1);
			Task[i+1].Index = index;
			break;
		}
		if (Task[i+1].Index<0) {
			// i+1 is empty, so the activewaypoint is the last one: we append after finish, because
			// pilot can use insert waypoint to make it BEFORE finish
			ResetTaskWaypoint(i+1);
			Task[i+1].Index = index;
			break;
		}
		#else
		if (Task[i+1].Index<0) {
			ResetTaskWaypoint(i+1);
			Task[i+1].Index = index;
			break;
		}
		#endif
	}  
  } else {
    // Shuffle ActiveWaypoint and all later task points
    // to the right by one position
    for (i=MAXTASKPOINTS-1; i>indexInsert; i--) {
      Task[i] = Task[i-1];
    }  
    // Insert new point and update task details
    ResetTaskWaypoint(indexInsert);
    Task[indexInsert].Index = index;
  }
  
  RefreshTask();
  UnlockTaskData();
  
}

// Create a default task to home at startup if no task is present
void DefaultTask(void) {
  LockTaskData();
  TaskModified = true;
  TargetModified = true;
  if ((Task[0].Index == -1)||(ActiveWayPoint==-1)) {

	if (HomeWaypoint >= 0) {
		if ( HomeWaypoint == 0 )  // 091213
			StartupStore(_T("... DefaultTask assigning TAKEOFF as default destination%s"),NEWLINE);
		else
		StartupStore(_T(". DefaultTask assigning Home (wp=%d) as default destination%s"),HomeWaypoint,NEWLINE); // 091213
		Task[0].Index = HomeWaypoint;
		ActiveWayPoint = 0;
	} else
		StartupStore(_T(". DefaultTask: no task, no active waypoint and no Home%s"),NEWLINE); // 091112
  }
  RefreshTask();
  UnlockTaskData();
}


// RemoveTaskpoint removes a single waypoint
// from the current task.  index specifies an entry
// in the Task[] array - NOT a waypoint index.
//
// If you call this function, you MUST deal with
// correctly setting ActiveWayPoint yourself!
void RemoveTaskPoint(int index) {
  if (!CheckDeclaration())
    return;
  
  int i;
  
  if (index < 0 || index >= MAXTASKPOINTS) {
    return; // index out of bounds
  }
  
  LockTaskData();
  TaskModified = true;
  TargetModified = true;

  if (Task[index].Index == -1) {
    UnlockTaskData();
    return; // There's no WP at this location
  }
  
  // Shuffle all later taskpoints to the left to
  // fill the gap
  for (i=index; i<MAXTASKPOINTS-1; ++i) {
    Task[i] = Task[i+1];
  }
  Task[MAXTASKPOINTS-1].Index = -1;
  if (DoOptimizeRoute())
  	Task[MAXTASKPOINTS-1].AATTargetOffsetRadius= -100.0;
  else
  	Task[MAXTASKPOINTS-1].AATTargetOffsetRadius= 0.0;

  RefreshTask();
  UnlockTaskData();
  
}


// Index specifies a waypoint in the WP list
// It won't necessarily be a waypoint that's
// in the task
void RemoveWaypoint(int index) {
  int i;

  if (!CheckDeclaration())
    return;
  
  if (ActiveWayPoint<0) {
    return; // No waypoint to remove
  }
  
  // Check to see whether selected WP is actually
  // in the task list.
  // If not, we'll ask the user if they want to remove
  // the currently active task point.
  // If the WP is in the task multiple times then we'll
  // remove the first instance after (or including) the
  // active WP.
  // If they're all before the active WP then just remove
  // the nearest to the active WP
  
  LockTaskData();
  TaskModified = true;
  TargetModified = true;

  // Search forward first
  i = ActiveWayPoint;
  while ((i < MAXTASKPOINTS) && (Task[i].Index != index)) {
    ++i;
  }
  
  if (i < MAXTASKPOINTS) {
    // Found WP, so remove it
    RemoveTaskPoint(i);
    
    if (Task[ActiveWayPoint].Index == -1) {
      // We've just removed the last task point and it was
      // active at the time
      ActiveWayPoint--;
    }
    
  } else {
    // Didn't find WP, so search backwards
    
    i = ActiveWayPoint;
    do {
      --i;
    } while (i >= 0 && Task[i].Index != index);
    
    if (i >= 0) {
      // Found WP, so remove it
      RemoveTaskPoint(i);
      ActiveWayPoint--;
      
    } else {
      // WP not found, so ask user if they want to
      // remove the active WP
      UnlockTaskData();

	TCHAR tlkbuf[100];
	_stprintf(tlkbuf,_T("%s\n%s"),
	// LKTOKEN  _@M169_ = "Chosen Waypoint not in current task." 
        gettext(TEXT("_@M169_")),
	// LKTOKEN  _@M551_ = "Remove active Waypoint?" 
	gettext(TEXT("_@M551_")));

      int ret = MessageBoxX(hWndMapWindow,
        tlkbuf,
	// LKTOKEN  _@M550_ = "Remove Waypoint" 
        gettext(TEXT("_@M550_")),
        MB_YESNO|MB_ICONQUESTION);
      LockTaskData();
      
      if (ret == IDYES) {
        RemoveTaskPoint(ActiveWayPoint);
        if (Task[ActiveWayPoint].Index == -1) {
          // Active WayPoint was last in the list so is currently
          // invalid.
          ActiveWayPoint--;
        }
      }
    }
  }
  RefreshTask();
  UnlockTaskData();

}


void ReplaceWaypoint(int index) {
  if (!CheckDeclaration())
    return;

  LockTaskData();
  TaskModified = true;
  TargetModified = true;

  // ARH 26/06/05 Fixed array out-of-bounds bug
  if (ActiveWayPoint>=0) {	
    ResetTaskWaypoint(ActiveWayPoint);
    Task[ActiveWayPoint].Index = index;
  } else {
    
    // Insert a new waypoint since there's
    // nothing to replace
    ActiveWayPoint=0;
    ResetTaskWaypoint(ActiveWayPoint);
    Task[ActiveWayPoint].Index = index;
  }
  RefreshTask();
  UnlockTaskData();
}


void RefreshTask(void) {
  double lengthtotal = 0.0;
  int i;

  LockTaskData();
  if ((ActiveWayPoint<0)&&(Task[0].Index>=0)) {
    ActiveWayPoint=0;
  }

  // Only need to refresh info where the removal happened
  // as the order of other taskpoints hasn't changed
  for (i=0; i<MAXTASKPOINTS; i++) {
    if (!ValidTaskPoint(i)) {
      Task[i].Index = -1;
    } else {
      RefreshTaskWaypoint(i);
      lengthtotal += Task[i].Leg;
    }
  }
  if (lengthtotal>0) {
    for (i=0; i<MAXTASKPOINTS; i++) {
      if (ValidTaskPoint(i)) {
	RefreshTaskWaypoint(i);
	TaskStats[i].LengthPercent = Task[i].Leg/lengthtotal;
	if (!ValidTaskPoint(i+1)) {
          // this is the finish waypoint
	  if (DoOptimizeRoute())
	  	Task[i].AATTargetOffsetRadius = -100.0;
	  else
	  	Task[i].AATTargetOffsetRadius = 0.0;
	  Task[i].AATTargetOffsetRadial = 0.0;
	  Task[i].AATTargetLat = WayPointList[Task[i].Index].Latitude;
	  Task[i].AATTargetLon = WayPointList[Task[i].Index].Longitude;
	}
      }
    }
  }

  // Determine if a waypoint is in the task
  if (WayPointList) {
    for (i=NUMRESWP; i< (int)NumberOfWayPoints; i++) { // maybe paragliders takeoff is set as home
      WayPointList[i].InTask = false;
      if ((WayPointList[i].Flags & HOME) == HOME) {
        WayPointList[i].InTask = true;
      }
    }
    if (HomeWaypoint>=0) {
      WayPointList[HomeWaypoint].InTask = true;
    }
    for (i=0; i<MAXTASKPOINTS; i++) {
      if (ValidTaskPoint(i)) {
        WayPointList[Task[i].Index].InTask = true;
      }
    }
    if (EnableMultipleStartPoints) {
      for (i=0; i<MAXSTARTPOINTS; i++) {
        if (ValidWayPoint(StartPoints[i].Index) && StartPoints[i].Active) {
          WayPointList[StartPoints[i].Index].InTask = true;
        }
      }
    }
  }

  CalculateTaskSectors();
  CalculateAATTaskSectors();
  UnlockTaskData();
}


void RotateStartPoints(void) {
  if (ActiveWayPoint>0) return;
  if (!EnableMultipleStartPoints) return;

  LockTaskData();
  
  int found = -1;
  int imax = 0;
  for (int i=0; i<MAXSTARTPOINTS; i++) {
    if (StartPoints[i].Active && ValidWayPoint(StartPoints[i].Index)) {
      if (Task[0].Index == StartPoints[i].Index) {
        found = i;
      }
      imax = i;
    }
  }
  found++;
  if (found>imax) {
    found = 0;
  }
  if (ValidWayPoint(StartPoints[found].Index)) {
    Task[0].Index = StartPoints[found].Index;
  }

  RefreshTask();
  UnlockTaskData();
}


void CalculateTaskSectors(void)
{
  int i;
  double SectorAngle, SectorSize, SectorBearing;

  LockTaskData();

  if (EnableMultipleStartPoints) {
    for(i=0;i<MAXSTARTPOINTS-1;i++) {
      if (StartPoints[i].Active && ValidWayPoint(StartPoints[i].Index)) {
	if (StartLine==2) {
          SectorAngle = 45+90;
        } else {
          SectorAngle = 90;
        }
        SectorSize = StartRadius;
        SectorBearing = StartPoints[i].OutBound;

        FindLatitudeLongitude(WayPointList[StartPoints[i].Index].Latitude,
                              WayPointList[StartPoints[i].Index].Longitude, 
                              SectorBearing + SectorAngle, SectorSize,
                              &StartPoints[i].SectorStartLat,
                              &StartPoints[i].SectorStartLon);

        FindLatitudeLongitude(WayPointList[StartPoints[i].Index].Latitude,
                              WayPointList[StartPoints[i].Index].Longitude,
                              SectorBearing - SectorAngle, SectorSize,
                              &StartPoints[i].SectorEndLat,
                              &StartPoints[i].SectorEndLon);
      }
    }
  }

  for(i=0;i<=MAXTASKPOINTS-1;i++)
    {
      if((Task[i].Index >=0))
	{
	  if ((Task[i+1].Index >=0)||(i==MAXTASKPOINTS-1)) {

	    if(i == 0)
	      {
		// start line
		if (StartLine==2) {
		  SectorAngle = 45+90;
		} else {
		  SectorAngle = 90;
		}
		SectorSize = StartRadius;
		SectorBearing = Task[i].OutBound;
	      }
	    else
	      {
		// normal turnpoint sector
		SectorAngle = 45;
		if (SectorType == 2) {
		  SectorSize = 10000; // German DAe 0.5/10
		} else {
		  SectorSize = SectorRadius;  // FAI sector
		}
		SectorBearing = Task[i].Bisector;
	      }
	  } else {
	    // finish line
	    if (FinishLine==2) {
	      SectorAngle = 45;
	    } else {
	      SectorAngle = 90;
	    }
	    SectorSize = FinishRadius;
	    SectorBearing = Task[i].InBound;

            // no clearing of this, so default can happen with ClearTask
            // Task[i].AATCircleRadius = 0;
            // Task[i].AATSectorRadius = 0;

	  }

          FindLatitudeLongitude(WayPointList[Task[i].Index].Latitude,
                                WayPointList[Task[i].Index].Longitude, 
                                SectorBearing + SectorAngle, SectorSize,
                                &Task[i].SectorStartLat,
                                &Task[i].SectorStartLon);

          FindLatitudeLongitude(WayPointList[Task[i].Index].Latitude,
                                WayPointList[Task[i].Index].Longitude,	
                                SectorBearing - SectorAngle, SectorSize,
                                &Task[i].SectorEndLat,
                                &Task[i].SectorEndLon);

          if (!AATEnabled) {
            Task[i].AATStartRadial  = 
              AngleLimit360(SectorBearing - SectorAngle);
            Task[i].AATFinishRadial = 
              AngleLimit360(SectorBearing + SectorAngle);
          }

	}
    }
  UnlockTaskData();
}


double AdjustAATTargets(double desired) {
  int i, istart, inum;
  double av=0;
  istart = max(1,ActiveWayPoint);
  inum=0;

  LockTaskData();
  for(i=istart;i<MAXTASKPOINTS-1;i++)
    {
      if(ValidTaskPoint(i)&&ValidTaskPoint(i+1) && !Task[i].AATTargetLocked)
	{
          Task[i].AATTargetOffsetRadius = max(-1.0,min(1.0,
                                          Task[i].AATTargetOffsetRadius));
	  av += Task[i].AATTargetOffsetRadius;
	  inum++;
	}
    }
  if (inum>0) {
    av/= inum;
  }
  if (fabs(desired)>1.0) {
    // don't adjust, just retrieve.
    goto OnExit;
  }

  // TODO accuracy: Check here for true minimum distance between
  // successive points (especially second last to final point)

  // Do this with intersection tests

  desired = (desired+1.0)/2.0; // scale to 0,1
  av = (av+1.0)/2.0; // scale to 0,1

  for(i=istart;i<MAXTASKPOINTS-1;i++)
    {
      if((Task[i].Index >=0)&&(Task[i+1].Index >=0) && !Task[i].AATTargetLocked)
	{
	  double d = (Task[i].AATTargetOffsetRadius+1.0)/2.0;
          // scale to 0,1

          if (av>0.01) {
            d = desired; 
	    // 20080615 JMW
	    // was (desired/av)*d;
	    // now, we don't want it to be proportional 
          } else {
            d = desired;
          } 
          d = min(1.0, max(d, 0.0))*2.0-1.0;
          Task[i].AATTargetOffsetRadius = d;
	}
    }
 OnExit:
  UnlockTaskData();
  return av;
}


extern NMEA_INFO GPS_INFO;
extern DERIVED_INFO CALCULATED_INFO;


void CalculateAATTaskSectors()
{
  int i;
  int awp = ActiveWayPoint;

  if(AATEnabled == FALSE || DoOptimizeRoute())
    return;

  double latitude = GPS_INFO.Latitude;
  double longitude = GPS_INFO.Longitude;

  LockTaskData();

  Task[0].AATTargetOffsetRadius = 0.0;
  Task[0].AATTargetOffsetRadial = 0.0;
  if (Task[0].Index>=0) {
    Task[0].AATTargetLat = WayPointList[Task[0].Index].Latitude;
    Task[0].AATTargetLon = WayPointList[Task[0].Index].Longitude;
  }

  for(i=1;i<MAXTASKPOINTS;i++) {
    if(ValidTaskPoint(i)) {
      if (!ValidTaskPoint(i+1)) {
        // This must be the final waypoint, so it's not an AAT OZ
        Task[i].AATTargetLat = WayPointList[Task[i].Index].Latitude;
        Task[i].AATTargetLon = WayPointList[Task[i].Index].Longitude;
        continue;
      }

      if(Task[i].AATType == SECTOR) {
        FindLatitudeLongitude (WayPointList[Task[i].Index].Latitude,
                                 WayPointList[Task[i].Index].Longitude, 
                               Task[i].AATStartRadial , 
                               Task[i].AATSectorRadius ,
                               &Task[i].AATStartLat,
                               &Task[i].AATStartLon);
        
        FindLatitudeLongitude (WayPointList[Task[i].Index].Latitude,
                               WayPointList[Task[i].Index].Longitude,
                               Task[i].AATFinishRadial , 
                               Task[i].AATSectorRadius,
                               &Task[i].AATFinishLat,
                               &Task[i].AATFinishLon);
      }

      // JMWAAT: if locked, don't move it
      if (i<awp) {
        // only update targets for current/later waypoints 
        continue;
      }

      Task[i].AATTargetOffsetRadius = 
        min(1.0, max(Task[i].AATTargetOffsetRadius,-1.0));

      Task[i].AATTargetOffsetRadial = 
        min(90.0, max(-90.0, Task[i].AATTargetOffsetRadial));

      double targetbearing;
      double targetrange;
      
      targetbearing = AngleLimit360(Task[i].Bisector+Task[i].AATTargetOffsetRadial);
      
      if(Task[i].AATType == SECTOR) {

        //AATStartRadial
        //AATFinishRadial

        targetrange = ((Task[i].AATTargetOffsetRadius+1.0)/2.0);

        double aatbisector = HalfAngle(Task[i].AATStartRadial, 
                                       Task[i].AATFinishRadial);

        if (fabs(AngleLimit180(aatbisector-targetbearing))>90) {
          // bisector is going away from sector 
          targetbearing = Reciprocal(targetbearing);
          targetrange = 1.0-targetrange;
        }
        if (!AngleInRange(Task[i].AATStartRadial,
                          Task[i].AATFinishRadial,
                          targetbearing,true)) {

          // Bisector is not within AAT sector, so
          // choose the closest radial as the target line

          if (fabs(AngleLimit180(Task[i].AATStartRadial-targetbearing))
              <fabs(AngleLimit180(Task[i].AATFinishRadial-targetbearing))) {
            targetbearing = Task[i].AATStartRadial;
          } else {
            targetbearing = Task[i].AATFinishRadial;
          }
        }

        targetrange*= Task[i].AATSectorRadius;

      } else {
        targetrange = Task[i].AATTargetOffsetRadius
          *Task[i].AATCircleRadius;
      }
      
      // TODO accuracy: if i=awp and in sector, range parameter needs to
      // go from current aircraft position to projection of target
      // out to the edge of the sector
      
      if (InAATTurnSector(longitude, latitude, i) && (awp==i) &&
          !Task[i].AATTargetLocked) {

        // special case, currently in AAT sector/cylinder
        
        double dist;
        double qdist;
        double bearing;
        
        // find bearing from last target through current aircraft position with offset
        DistanceBearing(Task[i-1].AATTargetLat,
                        Task[i-1].AATTargetLon,
                        latitude,
                        longitude,
                        &qdist, &bearing);

        bearing = AngleLimit360(bearing+Task[i].AATTargetOffsetRadial);

        dist = ((Task[i].AATTargetOffsetRadius+1)/2.0)*
          FindInsideAATSectorDistance(latitude, longitude, i, bearing);
        
        // if (dist+qdist>aatdistance.LegDistanceAchieved(awp)) {
        // JMW: don't prevent target from being closer to the aircraft
        // than the best achieved, so can properly plan arrival time        

        FindLatitudeLongitude (latitude,
                               longitude, 
                               bearing, 
                               dist,
                               &Task[i].AATTargetLat,
                               &Task[i].AATTargetLon);

        TargetModified = true;

        // }
        
      } else {
        
        FindLatitudeLongitude (WayPointList[Task[i].Index].Latitude,
                               WayPointList[Task[i].Index].Longitude, 
                               targetbearing, 
                               targetrange,
                               &Task[i].AATTargetLat,
                               &Task[i].AATTargetLon);
        TargetModified = true;
        
      }
    }
  }

  CalculateAATIsoLines();
  if (!TargetDialogOpen) {
    TargetModified = false;
    // allow target dialog to detect externally changed targets
  }

  UnlockTaskData();
}


#include "Logger.h"


void guiStartLogger(bool noAsk) {
  int i;

  if (!LoggerActive)
  {
	if (ReplayLogger::IsEnabled()) { 
		if (LoggerActive) 
			guiStopLogger(true); 
		return;
	}
	TCHAR TaskMessage[1024];
	_tcscpy(TaskMessage,gettext(TEXT("_@M876_"))); // Start Logger With Declaration\r\n")); 
	_tcscat(TaskMessage,_T("\r\n"));
	for(i=0;i<MAXTASKPOINTS;i++)
	{
		if(Task[i].Index == -1)
		{
			if(i==0) _tcscat(TaskMessage,gettext(TEXT("_@M479_"))); // None
			Debounce(); 
			break;
		}
		_tcscat(TaskMessage,WayPointList[ Task[i].Index ].Name);
		_tcscat(TaskMessage,TEXT(" - "));
		if (i==1) {
			_tcscat(TaskMessage,TEXT("\r\n ..."));
			break;
		}
	}

	// LKTOKEN  _@M637_ = "Start Logger" 
	if(noAsk || (MessageBoxX(hWndMapWindow,TaskMessage,gettext(TEXT("_@M637_")), MB_YESNO|MB_ICONQUESTION) == IDYES))
	{
		IGCWriteLock=true; // Lock ASAP
		if (LoggerClearFreeSpace()) {
	  
			StartLogger();
#if OLDLOGGER
			// ResumeSession never did work, always false
			if (ResumeSession) {

				LoggerActive=true;
				// ResetFRecord ??  TODO 
			} else {
#endif
				LoggerHeader();
				// THIS IS HAPPENING TOO EARLY, and we still have concurrency with F record!
				// LoggerActive = true; // start logger after Header is completed.  Concurrency
	  
				int ntp=0;
				for(i=0;i<MAXTASKPOINTS;i++)
				{
					if(Task[i].Index == -1)
					{
						break;
					}
					ntp++;
				}
				StartDeclaration(ntp);
				for(i=0;i<MAXTASKPOINTS;i++)
				{
					if(Task[i].Index == -1) {
						Debounce(); 
						break;
					}
					AddDeclaration(WayPointList[Task[i].Index].Latitude, WayPointList[Task[i].Index].Longitude, 
					WayPointList[Task[i].Index].Name );
				}
				EndDeclaration();
				LoggerActive = true; // start logger now
				ResetFRecord(); // reset timer & lastRecord string so if logger is restarted, FRec appears at top of file
#if OLDLOGGER
			} 
#endif
		} else {

	// LKTOKEN  _@M408_ = "Logger inactive, insufficient storage!" 
			MessageBoxX(hWndMapWindow, gettext(TEXT("_@M408_")),
	// LKTOKEN  _@M404_ = "Logger Error" 
			gettext(TEXT("_@M404_")), MB_OK| MB_ICONERROR);
			StartupStore(TEXT("------ Logger not started: Insufficient Storage%s"),NEWLINE);
		}
		IGCWriteLock=false; 
	}
	FullScreen();
  }
}


void guiStopLogger(bool noAsk) {
  if (LoggerActive) {
    if(noAsk || 
	// LKTOKEN  _@M669_ = "Stop Logger" 
       (MessageBoxX(hWndMapWindow,gettext(TEXT("_@M669_")),
	// LKTOKEN  _@M669_ = "Stop Logger" 
		    gettext(TEXT("_@M669_")),
		    MB_YESNO|MB_ICONQUESTION) == IDYES)) {
      StopLogger();
      if (!noAsk) {
	// force landing for paragliders..
	if ( (ISPARAGLIDER) && CALCULATED_INFO.Flying && 
		((GPS_INFO.Speed <= TakeOffSpeedThreshold) || GPS_INFO.NAVWarning) ) {
			// force landing event from TakeoffLanding
			time_in_flight=0;
			StartupStore(_T(". Logger stopped manually, landing is forced%s"),NEWLINE);
	}
		
        FullScreen();
      }
    }
  }
}


void guiToggleLogger(bool noAsk) {
  if (LoggerActive) {
    guiStopLogger(noAsk);
  } else {
    guiStartLogger(noAsk);
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
      DistanceBearing(WayPointList[Task[i-1].Index].Latitude, 
                      WayPointList[Task[i-1].Index].Longitude,
                      WayPointList[Task[i].Index].Latitude,   
                      WayPointList[Task[i].Index].Longitude,
                      &Task[i].Leg,
                      &Task[i].InBound);

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


// this is called only from Task LoadTaskWaypoints
static int FindOrAddWaypoint(WAYPOINT *read_waypoint) {
  // this is an invalid pointer!
  read_waypoint->Details = 0;
  read_waypoint->Comment = 0;
  read_waypoint->Name[NAME_SIZE-1] = 0; // prevent overrun if data is bogus
 
  int waypoint_index = FindMatchingWaypoint(read_waypoint);
  if (waypoint_index == -1) {
	// waypoint not found, so add it!
    
	// TODO bug: Set WAYPOINTFILECHANGED so waypoints get saved?
	// NO, we dont save task waypoints inside WP files! 
	WAYPOINT* new_waypoint = GrowWaypointList();
	if (!new_waypoint) {
		// error, can't allocate!
		return false;
	}
	memcpy(new_waypoint, read_waypoint, sizeof(WAYPOINT));
	// 100229 set no-save flag on
	new_waypoint->FileNum=-1;
	waypoint_index = NumberOfWayPoints-1;
  }
  return waypoint_index;
}


static bool LoadTaskWaypoints(HANDLE hFile) {
  WAYPOINT read_waypoint;
  DWORD dwBytesRead;

  int i;
  for(i=0;i<MAXTASKPOINTS;i++) {
    if(!ReadFile(hFile,&read_waypoint,sizeof(read_waypoint),&dwBytesRead, (OVERLAPPED *)NULL)
       || (dwBytesRead<sizeof(read_waypoint))) {
      return false;
    }
    if (Task[i].Index != -1) { //  091213 CHECK do not load reserved WP
      Task[i].Index = FindOrAddWaypoint(&read_waypoint);
    }
  }
  for(i=0;i<MAXSTARTPOINTS;i++) {
    if(!ReadFile(hFile,&read_waypoint,sizeof(read_waypoint),&dwBytesRead, (OVERLAPPED *)NULL)
       || (dwBytesRead<sizeof(read_waypoint))) {
      return false;
    }
    if (StartPoints[i].Index != -1) {
      StartPoints[i].Index = FindOrAddWaypoint(&read_waypoint);
    }
  }
  // managed to load everything
  return true;
}


// loads a new task from scratch.
void LoadNewTask(TCHAR *szFileName)
{
  HANDLE hFile;
  TASK_POINT Temp;
  START_POINT STemp;
  DWORD dwBytesRead;
  int i;
  bool TaskInvalid = false;
  bool WaypointInvalid = false;
  bool TaskLoaded = false;
  char taskinfo[LKPREAMBOLSIZE+1]; // 100207
  bool oldversion=false; // 100207

  LockTaskData();

  ActiveWayPoint = -1;
  for(i=0;i<MAXTASKPOINTS;i++) {
	Task[i].Index = -1;
  }
  
 StartupStore(_T(". LoadNewTask <%s>%s"),szFileName,NEWLINE);
  hFile = CreateFile(szFileName,GENERIC_READ,0,
                     (LPSECURITY_ATTRIBUTES)NULL,OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,NULL);
  
  if(hFile!= INVALID_HANDLE_VALUE )
    {

      // Defaults
      int   old_StartLine    = StartLine;
      int   old_SectorType   = SectorType;
      DWORD old_SectorRadius = SectorRadius;
      DWORD old_StartRadius  = StartRadius;
      int   old_AutoAdvance  = AutoAdvance;
      double old_AATTaskLength = AATTaskLength;
      BOOL   old_AATEnabled  = AATEnabled;
      DWORD  old_FinishRadius = FinishRadius;
      int    old_FinishLine = FinishLine;
      bool   old_EnableMultipleStartPoints = EnableMultipleStartPoints;

      TaskLoaded = true;

	if(!ReadFile(hFile,&taskinfo,LKPREAMBOLSIZE,&dwBytesRead, (OVERLAPPED *)NULL)) {
		TaskInvalid = true;
		goto goEnd;
	}

	// task version check
	if ( (taskinfo[0]!= 'L') || (taskinfo[1]!= 'K') || (taskinfo[2]!=LKTASKVERSION) ) { 
		TaskInvalid = true;
		oldversion = true;
		goto goEnd;
	}

      for(i=0;i<MAXTASKPOINTS;i++)
        {
          if(!ReadFile(hFile,&Temp,sizeof(TASK_POINT),&dwBytesRead, (OVERLAPPED *)NULL))
            {
              TaskInvalid = true;
              break;
            }
	  memcpy(&Task[i],&Temp, sizeof(TASK_POINT));

          if( !ValidNotResWayPoint(Temp.Index) && (Temp.Index != -1) ) { // 091213
            // Task is only invalid here if the index is out of range
            // of the waypoints and not equal to -1.
            // (Because -1 indicates a null task item)
	        WaypointInvalid = true; 
	  }
        }

      if (!TaskInvalid) {

	if (!ReadFile(hFile,&AATEnabled,sizeof(BOOL),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&AATTaskLength,sizeof(double),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	
	// ToDo review by JW
	
	// 20060521:sgi added additional task parameters
	if (!ReadFile(hFile,&FinishRadius,sizeof(FinishRadius),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&FinishLine,sizeof(FinishLine),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&StartRadius,sizeof(StartRadius),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&StartLine,sizeof(StartLine),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&SectorType,sizeof(SectorType),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&SectorRadius,sizeof(SectorRadius),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }
	if (!ReadFile(hFile,&AutoAdvance,sizeof(AutoAdvance),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }

        if (!ReadFile(hFile,&EnableMultipleStartPoints,sizeof(bool),&dwBytesRead,(OVERLAPPED*)NULL)) {
          TaskInvalid = true;
        }

        for(i=0;i<MAXSTARTPOINTS;i++)
        {
          if(!ReadFile(hFile,&STemp,sizeof(START_POINT),&dwBytesRead, (OVERLAPPED *)NULL)) {
            TaskInvalid = true;
            break;
          }
	  
          if( ValidNotResWayPoint(STemp.Index) || (STemp.Index==-1) ) { // 091213
            memcpy(&StartPoints[i],&STemp, sizeof(START_POINT));
          } else {
	    WaypointInvalid = true;
		StartupStore(_T("--- LoadNewTask: invalid waypoint=%d found%s"),STemp.Index,NEWLINE); // 091213
	  }
        }

        // search for waypoints...
        if (!TaskInvalid) {
          if (!LoadTaskWaypoints(hFile) && WaypointInvalid) {
            // couldn't lookup the waypoints in the file and we know there are invalid waypoints
            TaskInvalid = true;
            StartupStore(_T(". LoadTaskNew: cant locate waypoint in file, and invalid wp in task file%s"),NEWLINE);
          }
        }

      }

goEnd:

      CloseHandle(hFile);

      if (TaskInvalid) {
	if (oldversion)
		StartupStore(_T("------ Task is invalid: old task format%s"),NEWLINE);
	else
		StartupStore(_T("------ Task is invalid%s"),NEWLINE);

        StartLine = old_StartLine;
        SectorType = old_SectorType;
        SectorRadius = old_SectorRadius;
        StartRadius = old_StartRadius;
        AutoAdvance = old_AutoAdvance;
        AATTaskLength = old_AATTaskLength;
        AATEnabled = old_AATEnabled;
        FinishRadius = old_FinishRadius;
        FinishLine = old_FinishLine;
        EnableMultipleStartPoints = old_EnableMultipleStartPoints;
      }

  } else {
    StartupStore(_T("... LoadNewTask: file <%s> not found%s"),szFileName,NEWLINE); // 091213
    TaskInvalid = true;
  }
  
  if (TaskInvalid) {
    ClearTask();
  } 

  RefreshTask();
  
  if (!ValidTaskPoint(0)) {
    ActiveWayPoint = 0;
  }

  UnlockTaskData();

  if (TaskInvalid && TaskLoaded) {
	if (oldversion) {
	// LKTOKEN  _@M360_ = "Invalid old task format!" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M360_")), 
	// LKTOKEN  _@M396_ = "Load task" 
			gettext(TEXT("_@M396_")), MB_OK|MB_ICONEXCLAMATION);
	} else {
	// LKTOKEN  _@M264_ = "Error in task file!" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M264_")), 
	// LKTOKEN  _@M396_ = "Load task" 
			gettext(TEXT("_@M396_")), MB_OK|MB_ICONEXCLAMATION);
	}
  } else {
	TaskModified = false; 
	TargetModified = false;
	_tcscpy(LastTaskFileName, szFileName);
  }

}


void ClearTask(void) {

  LockTaskData();
  TaskModified = true; 
  TargetModified = true;
  LastTaskFileName[0] = _T('\0');
  ActiveWayPoint = -1;
  int i;
  for(i=0;i<MAXTASKPOINTS;i++) {
    Task[i].Index = -1;
    Task[i].AATSectorRadius = SectorRadius; // JMW added default
    Task[i].AATCircleRadius = SectorRadius; // JMW added default
    Task[i].AATTargetOffsetRadial = 0;
    if (DoOptimizeRoute())
      Task[i].AATTargetOffsetRadius = -100;
    else
      Task[i].AATTargetOffsetRadius = 0;
    Task[i].AATTargetLocked = false;
    for (int j=0; j<MAXISOLINES; j++) {
      TaskStats[i].IsoLine_valid[j] = false;
    }
  }
  for (i=0; i<MAXSTARTPOINTS; i++) {
    StartPoints[i].Index = -1;
  }
  UnlockTaskData();
}


bool ValidWayPoint(int i) {
  bool retval = true;
  LockTaskData();
  if ((!WayPointList)||(i<0)||(i>=(int)NumberOfWayPoints)) {
    retval = false;
  }
  UnlockTaskData();
  return retval;
}

// A waypoint is valid here only if a list exists, it is within range and it is not reserved
bool ValidNotResWayPoint(int i) { // 091213
  bool retval = true;
  LockTaskData();
  if ((!WayPointList)||(i<=RESWP_END)||(i>=(int)NumberOfWayPoints)) {
    retval = false;
  }
  UnlockTaskData();
  return retval;
}

// 100929 A waypoint is valid here only if it is virtual, and with a valid content
bool ValidResWayPoint(int i) { // 091213
  bool retval = true;
  LockTaskData();
  if ( (i<0) || (i>RESWP_END) )
	retval = false;
  else {
	if (WayPointList[i].Latitude == RESWP_INVALIDNUMBER) retval=false;
  }
  UnlockTaskData();
  return retval;
}

bool ValidTaskPoint(int i) {
  bool retval = true;
  LockTaskData();
  if ((i<0) || (i>= MAXTASKPOINTS)) 
    retval = false;
  else if (!ValidWayPoint(Task[i].Index)) 
    retval = false;
  UnlockTaskData();
  return retval;
}

void SaveTask(TCHAR *szFileName)
{
  HANDLE hFile;
  DWORD dwBytesWritten;
  char taskinfo[LKPREAMBOLSIZE+1];

  if (!WayPointList) return; // this should never happen, but just to be safe...

  LockTaskData();
  StartupStore(_T(". SaveTask: saving <%s>%s"),szFileName,NEWLINE); // 091112
        
  hFile = CreateFile(szFileName,GENERIC_WRITE,0, (LPSECURITY_ATTRIBUTES)NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
        
  if(hFile!=INVALID_HANDLE_VALUE ) {

	// add a version string inside task files
	sprintf(taskinfo,"LK%c%02d%02d___x_________x_________x_________x_________",LKTASKVERSION,MAXTASKPOINTS,MAXSTARTPOINTS);
	WriteFile(hFile,&taskinfo,LKPREAMBOLSIZE,&dwBytesWritten,(OVERLAPPED *)NULL);

	WriteFile(hFile,&Task[0],sizeof(TASK_POINT)*MAXTASKPOINTS,&dwBytesWritten,(OVERLAPPED *)NULL);
	WriteFile(hFile,&AATEnabled,sizeof(BOOL),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&AATTaskLength,sizeof(double),&dwBytesWritten,(OVERLAPPED*)NULL);
    
	// 20060521:sgi added additional task parameters
	WriteFile(hFile,&FinishRadius,sizeof(FinishRadius),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&FinishLine,sizeof(FinishLine),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&StartRadius,sizeof(StartRadius),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&StartLine,sizeof(StartLine),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&SectorType,sizeof(SectorType),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&SectorRadius,sizeof(SectorRadius),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&AutoAdvance,sizeof(AutoAdvance),&dwBytesWritten,(OVERLAPPED*)NULL);
	    
	WriteFile(hFile,&EnableMultipleStartPoints,sizeof(bool),&dwBytesWritten,(OVERLAPPED*)NULL);
	WriteFile(hFile,&StartPoints[0],sizeof(START_POINT)*MAXSTARTPOINTS,&dwBytesWritten,(OVERLAPPED*)NULL);
    
	// JMW added writing of waypoint data, in case it's missing
	int i;

	for(i=0;i<MAXTASKPOINTS;i++) {
		if (ValidNotResWayPoint(Task[i].Index)) {
			WriteFile(hFile,&WayPointList[Task[i].Index],
			sizeof(WAYPOINT), &dwBytesWritten, (OVERLAPPED*)NULL);
		} else {
			// dummy data..
			WriteFile(hFile,&WayPointList[NUMRESWP],
			sizeof(WAYPOINT), &dwBytesWritten, (OVERLAPPED*)NULL);
		}
	}

	for(i=0;i<MAXSTARTPOINTS;i++) {
		if (ValidNotResWayPoint(StartPoints[i].Index)) { // 091223
			WriteFile(hFile,&WayPointList[StartPoints[i].Index], sizeof(WAYPOINT), &dwBytesWritten, (OVERLAPPED*)NULL);
		} else {
			// dummy data..
			WriteFile(hFile,&WayPointList[NUMRESWP], sizeof(WAYPOINT), &dwBytesWritten, (OVERLAPPED*)NULL);
		}
	}
    
	CloseHandle(hFile);
	TaskModified = false; // task successfully saved
	TargetModified = false;
	_tcscpy(LastTaskFileName, szFileName);
	StartupStore(_T(". SaveTask: Ok%s"),NEWLINE);

  } else {
    
	MessageBoxX(hWndMapWindow,
	// LKTOKEN  _@M263_ = "Error in saving task!" 
	gettext(TEXT("_@M263_")), gettext(TEXT("Save task")), MB_OK|MB_ICONEXCLAMATION);
	StartupStore(_T("++++++ SaveTask: ERROR saving task!%s"),NEWLINE);
	FailStore(_T("SaveTask: ERROR saving task!%s"),NEWLINE);
  }
  UnlockTaskData();
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
                                  taskwaypoint);

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

    if (InAATTurnSector(t_lon, t_lat, taskwaypoint)) {
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


void CalculateAATIsoLines(void) {
  int i;
  int awp = ActiveWayPoint;
  double stepsize = 25.0;

  if(AATEnabled == FALSE)
    return;

  LockTaskData();

  for(i=1;i<MAXTASKPOINTS;i++) {

    if(ValidTaskPoint(i)) {
      if (!ValidTaskPoint(i+1)) {
        // This must be the final waypoint, so it's not an AAT OZ
        continue;
      }
      // JMWAAT: if locked, don't move it
      if (i<awp) {
        // only update targets for current/later waypoints 
        continue;
      }

      int j;
      for (j=0; j<MAXISOLINES; j++) {
        TaskStats[i].IsoLine_valid[j] = false;
      }

      double latitude = Task[i].AATTargetLat;
      double longitude = Task[i].AATTargetLon;
      double dist_0, dist_north, dist_east;
      bool in_sector = true;

      double max_distance, delta;
      if(Task[i].AATType == SECTOR) {
        max_distance = Task[i].AATSectorRadius;
      } else {
        max_distance = Task[i].AATCircleRadius;
      }
      delta = max_distance*2.4 / (MAXISOLINES);
      bool left = false;

      /*
      double distance_glider=0;
      if ((i==ActiveWayPoint) && (CALCULATED_INFO.IsInSector)) {
        distance_glider = DoubleLegDistance(i, GPS_INFO.Longitude, GPS_INFO.Latitude);
      }
      */

      // fill
      j=0;
      // insert start point
      TaskStats[i].IsoLine_Latitude[j] = latitude;
      TaskStats[i].IsoLine_Longitude[j] = longitude;
      TaskStats[i].IsoLine_valid[j] = true;
      j++;

      do {
        dist_0 = DoubleLegDistance(i, longitude, latitude);

        double latitude_north, longitude_north;
        FindLatitudeLongitude(latitude, longitude, 
                              0, stepsize,
                              &latitude_north,
                              &longitude_north);
        dist_north = DoubleLegDistance(i, longitude_north, latitude_north);
        
        double latitude_east, longitude_east;
        FindLatitudeLongitude(latitude, longitude, 
                              90, stepsize,
                              &latitude_east,
                              &longitude_east);
        dist_east = DoubleLegDistance(i, longitude_east, latitude_east);
        
        double angle = AngleLimit360(RAD_TO_DEG*atan2(dist_east-dist_0, dist_north-dist_0)+90);
        if (left) {
          angle += 180;
        }
        
        FindLatitudeLongitude(latitude, longitude, 
                              angle, delta,
                              &latitude,
                              &longitude);
        
        in_sector = InAATTurnSector(longitude, latitude, i);
        /*
        if (dist_0 < distance_glider) {
          in_sector = false;
        }
        */
        if (in_sector) {
          TaskStats[i].IsoLine_Latitude[j] = latitude;
          TaskStats[i].IsoLine_Longitude[j] = longitude;
          TaskStats[i].IsoLine_valid[j] = true;
          j++;
        } else {
          j++;
          if (!left && (j<MAXISOLINES-2))  {
            left = true;
            latitude = Task[i].AATTargetLat;
            longitude = Task[i].AATTargetLon;
            in_sector = true; // cheat to prevent early exit

            // insert start point (again)
            TaskStats[i].IsoLine_Latitude[j] = latitude;
            TaskStats[i].IsoLine_Longitude[j] = longitude;
            TaskStats[i].IsoLine_valid[j] = true;
            j++;
          }
        }
      } while (in_sector && (j<MAXISOLINES));

    }
  }
  UnlockTaskData();
}


void SaveDefaultTask(void) {
  LockTaskData();
    TCHAR buffer[MAX_PATH];
//#if (!defined(WINDOWSPC) || (WINDOWSPC <=0) )
#if 1
  LocalPath(buffer,TEXT(LKD_TASKS));
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,_T(LKF_DEFAULTASK)); // 091101
#else // REMOVE 
  SHGetSpecialFolderPath(hWndMainWindow, buffer, CSIDL_PERSONAL, false); // REMOVABLE
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,TEXT(LKDATADIR));
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,TEXT(LKD_TASKS)); // 091101
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,_T(LKF_DEFAULTASK)); // 091101
#endif
    SaveTask(buffer);
  UnlockTaskData();
}

#include "NavFunctions.h"


void CalculateOptimizedTargetPos(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

	if (!DoOptimizeRoute()) 
		return;

	int curwp; // cur for current pos
	int stdwp; // std for current standard wp
	int nxtwp; // nxt for next after current wp

	double curlat, curlon;
	double stdlat, stdlon;
	double stddst, stdbrg;
	double nxtlat, nxtlon;
	double nxtbrg;

	curwp  = ActiveWayPoint;
	curlat = Basic->Latitude; 
	curlon = Basic->Longitude; 

	bool bCalcPrev = false;

	while(ValidWayPoint(nxtwp=Task[curwp+1].Index)) {
		
		stdwp=Task[curwp].Index;

		stdlat = WayPointList[stdwp].Latitude;
		stdlon = WayPointList[stdwp].Longitude;

		nxtlat = WayPointList[nxtwp].Latitude;
		nxtlon = WayPointList[nxtwp].Longitude;

		// if Same Wpt Calc Next before if Exist
		if(stdwp == nxtwp && ValidWayPoint(Task[curwp+2].Index)){  
			bCalcPrev = true;
			Task[curwp].AATTargetLat= stdlat;
			Task[curwp].AATTargetLon= stdlon;
			Task[curwp].AATTargetLocked=true;
			++curwp;
			continue;
		}

		// From Current Position To Current Wpt
		DistanceBearing(curlat, curlon, stdlat, stdlon, &stddst, &stdbrg);

		// From Current Wpt To Next Wpt
		DistanceBearing(stdlat, stdlon, nxtlat, nxtlon, NULL, &nxtbrg);

		double obrg_f = BiSector(nxtbrg, stdbrg);

		double radius= (curwp>0)?(Task[curwp].AATCircleRadius):StartRadius;
		if( radius < stddst) {
			obrg_f = nxtbrg;
		}

		// Why ?? Check if Point is Loged beffore advance to Next WP ...
//		dist_ui-=30; // 30m margin 

		double optlat, optlon;
		FindLatitudeLongitude(stdlat,stdlon, obrg_f, radius, &optlat, &optlon);

		double errbrg, optbrg; // beraing from current to next
		DistanceBearing(curlat, curlon, nxtlat, nxtlon, NULL, &errbrg);
		DistanceBearing(curlat, curlon, optlat, optlon, NULL, &optbrg);

		double dBrg = fabs((stdbrg - errbrg) * DEG_TO_RAD);
		if( (dBrg < PI/2) && (sin(dBrg) < radius/stddst ) ) {

			if(radius > stddst * sin(dBrg)) {

				if( (dBrg < PI/2) && (radius < stddst)) {
					dBrg = - dBrg + asin((stddst * sin(dBrg)) / radius);
				}
				else{
					dBrg = PI - dBrg - asin((stddst * sin(dBrg)) / radius);
				}
				dBrg *= RAD_TO_DEG;
				obrg_f = AngleLimit360(dBrg + 180 + stdbrg);

				FindLatitudeLongitude(stdlat,stdlon, obrg_f, radius, &optlat, &optlon);
			}
		}

		Task[curwp].AATTargetLat= optlat;
		Task[curwp].AATTargetLon= optlon;
		Task[curwp].AATTargetLocked=true;

		if(bCalcPrev) {
			double errbrg; // beraing from current to optNext
			DistanceBearing(curlat, curlon, optlat, optlon, NULL, &errbrg);
			radius= ((curwp-1)>0)?(Task[curwp-1].AATCircleRadius):StartRadius;

			if( radius < stddst) {
				double dBrg = (stdbrg - errbrg) * DEG_TO_RAD;
				if(dBrg>PI) dBrg -= 2*PI;
				if( (dBrg > (PI/360)) && (sin(fabs(dBrg))) < radius/stddst && radius > stddst * sin(dBrg)) {
					if( (dBrg < PI/2) && (radius < stddst)) {
						dBrg = - dBrg + asin((stddst * sin(dBrg)) / radius);
					}
					else{
						dBrg = PI - dBrg - asin((stddst * sin(dBrg)) / radius);
					}
					dBrg *= RAD_TO_DEG;
					obrg_f = AngleLimit360(dBrg + 180 + stdbrg);
				}
				else {
					obrg_f = AngleLimit360(stdbrg + 180);
				}
			}
			FindLatitudeLongitude(stdlat,stdlon, obrg_f, radius, &optlat, &optlon);


			Task[curwp-1].AATTargetLat= optlat;
			Task[curwp-1].AATTargetLon= optlon;
			Task[curwp-1].AATTargetLocked=true;

			bCalcPrev = false;
		}

		curlat = Task[curwp].AATTargetLat;
		curlon = Task[curwp].AATTargetLon;
		Task[curwp].AATTargetLocked=true;
		++curwp;
	}

	// Last radius
	stdwp=Task[curwp].Index;
	stdlat = WayPointList[stdwp].Latitude;
	stdlon = WayPointList[stdwp].Longitude;

	DistanceBearing(stdlat, stdlon, curlat, curlon, NULL, &stdbrg);
	double dist_ui= (curwp>0)?(Task[curwp].AATCircleRadius):StartRadius;
	FindLatitudeLongitude(stdlat,stdlon, stdbrg, dist_ui, &(Task[curwp].AATTargetLat), &(Task[curwp].AATTargetLon));
	Task[curwp].AATTargetLocked=true;



	stdwp=Task[ActiveWayPoint].Index;

	WayPointList[RESWP_OPTIMIZED].Latitude = Task[ActiveWayPoint].AATTargetLat;
	WayPointList[RESWP_OPTIMIZED].Longitude = Task[ActiveWayPoint].AATTargetLon;
	WayPointList[RESWP_OPTIMIZED].Altitude = WayPointList[stdwp].Altitude;
	wsprintf(WayPointList[RESWP_OPTIMIZED].Name, _T("!%s"),WayPointList[stdwp].Name);
}

// Clear PG 
void ClearOptimizedTargetPos() {

	WayPointList[RESWP_OPTIMIZED].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Altitude=RESWP_INVALIDNUMBER;
	// name will be assigned by function dynamically
	_tcscpy(WayPointList[RESWP_OPTIMIZED].Name, _T("OPTIMIZED") );

	for(int i = 0; ValidWayPoint(Task[i].Index); ++i) {
		Task[i].AATTargetLat = WayPointList[Task[i].Index].Latitude;
		Task[i].AATTargetLon = WayPointList[Task[i].Index].Longitude;
		Task[i].AATTargetLocked = false;
	}
}

