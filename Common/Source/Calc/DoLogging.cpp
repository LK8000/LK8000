/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"


using std::min;
using std::max;
extern void AddSnailPoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

int FastLogNum = 0; // number of points to log at high rate

//
// Logger activity, and also add snailpoints
//
void DoLogging(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  static double SnailLastTime=0;
  static double LogLastTime=0;
  static double StatsLastTime=0;
  double dtLog = 5.0;
  double dtSnail = 2.0;
  double dtStats = 60.0;
  double dtFRecord = 270; // 4.5 minutes (required minimum every 5)

  if(Basic->Time <= LogLastTime) {
    LogLastTime = Basic->Time;
  }
  if(Basic->Time <= SnailLastTime)  {
    SnailLastTime = Basic->Time;
  }
  if(Basic->Time <= StatsLastTime) {
    StatsLastTime = Basic->Time;
  }
  if(Basic->Time <= GetFRecordLastTime()) {
    SetFRecordLastTime(Basic->Time);
  }

  // draw snail points more often in circling mode
  if (Calculated->Circling) {
    dtLog = LoggerTimeStepCircling;
    dtSnail = 1.0;
  } else {
    dtLog = LoggerTimeStepCruise;
    dtSnail = 5.0;
  }
  if (FastLogNum) {
    dtLog = 1.0;
  }

  // prevent bad fixes from being logged or added to OLC store
  static double Longitude_last = 10;
  static double Latitude_last = 10;
  double distance;

  DistanceBearing(Basic->Latitude, Basic->Longitude, 
		  Latitude_last, Longitude_last,
		  &distance, NULL);
  Latitude_last = Basic->Latitude;
  Longitude_last = Basic->Longitude;

  if (distance>200.0) {
    return;
  }

  if (Basic->Time - LogLastTime >= dtLog) {
    double balt = -1;
    if (Basic->BaroAltitudeAvailable) {
      balt = Basic->BaroAltitude;
    } else {
      balt = Basic->Altitude;
    }

    // 110723 this is not a solution, only a workaround.
    // Problem is that different threads are using indirectly IGCWrite in Logger.cpp
    // That function as an internal sort-of locking, which probably may be much better
    // to remove, resulting currently in data loss inside IGC.
    // Since at takeoff calculation and main thread are using IGCWrite, we want to be sure
    // that the initial declaration is completed before proceeding with F and B records here!
    static bool dowarn=true;
    if (IGCWriteLock) {
	unsigned short loop=0;
	while (++loop<50) {
		Sleep(10); //  500 ms delay max
		if (!IGCWriteLock) break;
	}
	if (IGCWriteLock) {
		if (dowarn) StartupStore(_T("..... LogPoint failed, IGCWriteLock %s!%s"),WhatTimeIsIt(),NEWLINE);
	} else {
		if (dowarn) StartupStore(_T("..... LogPoint delayed by IGCWriteLock, ok.%s"),NEWLINE);
		LogPoint(Basic->Latitude , Basic->Longitude , Basic->Altitude, balt);
	}
	dowarn=false;
    } else
	LogPoint(Basic->Latitude , Basic->Longitude , Basic->Altitude, balt);

    LogLastTime += dtLog;
    if (LogLastTime< Basic->Time-dtLog) {
      LogLastTime = Basic->Time-dtLog;
    }
    if (FastLogNum) FastLogNum--;
  }

  if (Basic->Time - GetFRecordLastTime() >= dtFRecord) 
  { 
    if (LogFRecord(Basic->SatelliteIDs,false))
    {  // need F record every 5 minutes
       // so if write fails or constellation is invalid, don't update timer and try again next cycle
      SetFRecordLastTime(GetFRecordLastTime() + dtFRecord);  
      // the FRecordLastTime is reset when the logger restarts so it is always at the start of the file
      if (GetFRecordLastTime() < Basic->Time-dtFRecord)
        SetFRecordLastTime(Basic->Time-dtFRecord);
    }
  }

  if (Basic->Time - SnailLastTime >= dtSnail) {
    AddSnailPoint(Basic, Calculated);
    SnailLastTime += dtSnail;
    if (SnailLastTime< Basic->Time-dtSnail) {
      SnailLastTime = Basic->Time-dtSnail;
    }
  }

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
      CContestMgr::Instance().Add(new CPointGPS(static_cast<unsigned>(Basic->Time),
                                                Basic->Latitude, Basic->Longitude,
                                                static_cast<unsigned>(Basic->Altitude)));
  }
}

