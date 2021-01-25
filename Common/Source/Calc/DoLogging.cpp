/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: DoLogging.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "Logger.h"
#include "DoInits.h"
#include "NavFunctions.h"

extern void AddSnailPoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

#define LOGINTERVAL  1  // seconds between IGC log lines
#define MAXERRDOLOG  30

//
// Logger activity, and also add snailpoints
//
void DoLogging(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  static double SnailLastTime=0;
  static double LogLastTime=0;
  static double StatsLastTime=0;
  static short maxerrlog=MAXERRDOLOG;

  // prevent bad fixes from being logged or added to OLC store
  static double Time_last=0;
  static double Longitude_last = 10;
  static double Latitude_last = 10;
  static double Longitude_snailed = 10;
  static double Latitude_snailed = 10;

  double dtSnail = 2.0;
  double dtStats = 60.0;

  if (DoInit[MDI_CALCLOGGING]) {
	SnailLastTime=0;
	LogLastTime=0;
	StatsLastTime=0;
	maxerrlog=MAXERRDOLOG;
	Time_last=0;
	Longitude_last = 10;
	Latitude_last = 10;
	Longitude_snailed = 10;
	Latitude_snailed = 10;
	DoInit[MDI_CALCLOGGING]=false;
  }

  if (Basic->NAVWarning) return;

  // We do not want of course any duplicate timestamps, but in case something has gone wrong from gps,
  // we have to reset LogLastTime and start back accepting fixes. We declare that a difference back in
  // time of 10 minutes is enough to declare forfait
  if((Basic->Time - LogLastTime) < -600) {
    StartupStore(_T(". DoLogging: BasicTime=%f LogLastTime=%f : time is in the past. Reset.%s"),Basic->Time,LogLastTime,NEWLINE);
    LogLastTime = Basic->Time;
  }
  if(Basic->Time <= SnailLastTime)  {
    SnailLastTime = Basic->Time;
  }
  if(Basic->Time <= StatsLastTime) {
    StatsLastTime = Basic->Time;
  }

  // draw snail points more often in circling mode
  if (Calculated->Circling) {
    dtSnail = 1.0;
  } else {
    dtSnail = 5.0;
  }

  double distance;

  DistanceBearing(Basic->Latitude, Basic->Longitude,
		  Latitude_last, Longitude_last,
		  &distance, NULL);

  // Do not log or add a snail point if in a single second we made more than 300m. (1000kmh)
  // This should allow loggin and snail logging also while using LK on a liner for fun.
  // This filter is necessary for managing wrong position fixes by the gps
  // Until 3.1f it was set to 200m
  double timepassed=Basic->Time - Time_last;
  if (Time_last==0) timepassed=0; // skip check

  // Now we can save values, because we want to compare fix after fix. If we really jumped away,
  // we shall accept this fact after the second fix far away.
  Latitude_last = Basic->Latitude;
  Longitude_last = Basic->Longitude;
  Time_last=Basic->Time;

  if (timepassed>0 && ((distance/timepassed)>300.0) ) {
	if (maxerrlog>0) {
                if (SIMMODE && maxerrlog<MAXERRDOLOG)
		   StartupStore(_T("... DoLogging: at %s distance jumped too much, %f in %fs!\n"),WhatTimeIsIt(),distance,timepassed);
		maxerrlog--;
	}
	return;
  }

  Latitude_last = Basic->Latitude;
  Longitude_last = Basic->Longitude;
  Time_last=Basic->Time;

  //
  // If time has advanced enough, add point to IGC
  // Reminder: Basic Time is counted in seconds and accounts for crossing UTC 00:00 so it goes over 86400
  //
  if (Basic->Time - LogLastTime >= LOGINTERVAL) {
    LogLastTime = Basic->Time;
    double balt = -1;
    if (Basic->BaroAltitudeAvailable) {
      balt = QNHAltitudeToQNEAltitude(Basic->BaroAltitude);
      // check for balt validity are NOT performed in logpoint functions anymore
      if (balt<-1000||balt>15000) balt=Basic->BaroAltitude;
    } else {
      balt = 0;
    }

    // 110723 this is not a solution, only a workaround.
    // Problem is that different threads are using indirectly IGCWrite in Logger.cpp
    // That function as an internal sort-of locking, which probably may be much better
    // to remove, resulting currently in data loss inside IGC.
    // Since at takeoff calculation and main thread are using IGCWrite, we want to be sure
    // that the initial declaration is completed before proceeding with F and B records here!
    if (IGCWriteLock) {
	unsigned short loop=0;
	while (++loop<50) {
		Poco::Thread::sleep(10); //  500 ms delay max
		if (!IGCWriteLock) break;
	}
	if (IGCWriteLock) {
		if (maxerrlog>0) StartupStore(_T("..... LogPoint failed, IGCWriteLock %s!%s"),WhatTimeIsIt(),NEWLINE);
	} else {
		if (maxerrlog>0) StartupStore(_T("..... LogPoint delayed by IGCWriteLock, ok.%s"),NEWLINE);
		LogPoint(Basic->Latitude , Basic->Longitude , Basic->Altitude, balt, Basic->Hour, Basic->Minute, Basic->Second);
	}
	maxerrlog--;
    } else
	LogPoint(Basic->Latitude , Basic->Longitude , Basic->Altitude, balt, Basic->Hour, Basic->Minute, Basic->Second);

    // Remarks: LogPoint is also checking that there is a valid fix to proceed

  } // time has advanced enough: >= LOGINTERVAL

  // 120812 For car/trekking mode, log snailpoint only if at least 5m were made in the dtSnail time
  if (ISCAR) {

	// if 5 seconds have passed.. (no circling mode in car mode)
	if ( (Basic->Time - SnailLastTime) >= dtSnail) {
		DistanceBearing(Basic->Latitude, Basic->Longitude, Latitude_snailed, Longitude_snailed, &distance, NULL);
		// and distance made is at least 5m (moving average 3.6kmh)
		if (distance>5) {
			AddSnailPoint(Basic, Calculated);
			SnailLastTime += dtSnail;
			if (SnailLastTime< Basic->Time-dtSnail) {
				SnailLastTime = Basic->Time-dtSnail;
			}
			Latitude_snailed = Basic->Latitude;
			Longitude_snailed = Basic->Longitude;
		}
	}
	// else do not log, and do not update snailtime, so we shall be here every second until at least 10m are made
	goto _afteriscar;
  }

  if (Basic->Time - SnailLastTime >= dtSnail) {
    AddSnailPoint(Basic, Calculated);
    SnailLastTime += dtSnail;
    if (SnailLastTime< Basic->Time-dtSnail) {
      SnailLastTime = Basic->Time-dtSnail;
    }
  }

_afteriscar:

  if (Calculated->Flying) {
    if (Basic->Time - StatsLastTime >= dtStats) {

      flightstats.Altitude_Terrain.
        least_squares_update(max(0.0,
                                 Basic->Time-Calculated->TakeOffTime)/3600.0,
                             Calculated->TerrainAlt);

      flightstats.Altitude.
        least_squares_update(max(0.0,
                                 Basic->Time-Calculated->TakeOffTime)/3600.0,
                             Calculated->NavAltitude);
      StatsLastTime += dtStats;
      if (StatsLastTime< Basic->Time-dtStats) {
        StatsLastTime = Basic->Time-dtStats;
      }
    }

    if(UseContestEngine() && Calculated->FreeFlying)
      CContestMgr::Instance().Add(static_cast<unsigned>(Basic->Time),
                                  Basic->Latitude, Basic->Longitude,
                                  static_cast<unsigned>(Basic->Altitude));
  }
}
