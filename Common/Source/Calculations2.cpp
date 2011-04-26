/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Calculations2.cpp,v 8.5 2010/12/10 23:45:23 root Exp root $
*/

#include "StdAfx.h"
#include "Calculations.h"
#include "Dialogs.h"
#include "Parser.h"
#include "Utils.h"
#include "externs.h"
#include "McReady.h"
#include "Airspace.h"
#include "Logger.h"
#include <windows.h>
#include <math.h>
#include "InputEvents.h"
#include "Message.h"
#include "RasterTerrain.h"
#include "TeamCodeCalculation.h"
#include "Process.h"

#include <tchar.h>

#include "ThermalLocator.h"
#include "windanalyser.h"
#include "Atmosphere.h"

#ifdef NEW_OLC
#include "ContestMgr.h"
#else
#include "OnLineContest.h"
#endif /* NEW_OLC */
#include "AATDistance.h"

#include "NavFunctions.h" // used for team code

#include "utils/heapcheck.h"

#ifdef LKAIRSPACE
using std::min;
using std::max;
#endif

#ifdef NEW_OLC
using std::min;
using std::max;
#else
extern OLCOptimizer olc;
#endif /* NEW_OLC */

int FastLogNum = 0; // number of points to log at high rate

void AddSnailPoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if (!Calculated->Flying) return;

  SnailTrail[SnailNext].Latitude = (float)(Basic->Latitude);
  SnailTrail[SnailNext].Longitude = (float)(Basic->Longitude);
  SnailTrail[SnailNext].Time = Basic->Time;
  SnailTrail[SnailNext].FarVisible = true; // hasn't been filtered out yet.
  if (Calculated->TerrainValid) {
	double hr = max(0.0, Calculated->AltitudeAGL)/100.0;
	SnailTrail[SnailNext].DriftFactor = 2.0/(1.0+exp(-hr))-1.0;
  } else {
	SnailTrail[SnailNext].DriftFactor = 1.0;
  }

#if 0
  if (Calculated->Circling) {
	SnailTrail[SnailNext].Vario = (float)(Calculated->NettoVario) ;
  } else {
	SnailTrail[SnailNext].Vario = (float)(Calculated->NettoVario) ;
  }
#else
  // paragliders use Vario and not NettoVario
  if ( AircraftCategory != (AircraftCategory_t)umParaglider ) 
	SnailTrail[SnailNext].Vario = (float)(Calculated->NettoVario) ;
  else
	SnailTrail[SnailNext].Vario = (float)(Calculated->Vario) ;
#endif

  SnailTrail[SnailNext].Colour = -1; // need to have colour calculated
  SnailTrail[SnailNext].Circling = Calculated->Circling;

  SnailNext ++;
  SnailNext %= TRAILSIZE;

}


int LoggerTimeStepCruise=1;	//@ 101008 changed to 1 second 
int LoggerTimeStepCircling=1;

void DoLogging(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  static double SnailLastTime=0;
  static double LogLastTime=0;
  static double StatsLastTime=0;
#ifndef NEW_OLC
  static double OLCLastTime = 0;
#endif /* NEW_OLC */
  double dtLog = 5.0;
  double dtSnail = 2.0;
  double dtStats = 60.0;
#ifndef NEW_OLC
  double dtOLC = 5.0;
#endif /* NEW_OLC */
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
#ifndef NEW_OLC
  if(Basic->Time <= OLCLastTime) {
    OLCLastTime = Basic->Time;
  }
#endif /* NEW_OLC */
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
    LogPoint(Basic->Latitude , Basic->Longitude , Basic->Altitude,
             balt);
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

#ifdef NEW_OLC
    if(UseContestEngine() && Calculated->FreeFlying)
      CContestMgr::Instance().Add(new CPointGPS(static_cast<unsigned>(Basic->Time),
                                                Basic->Latitude, Basic->Longitude,
                                                static_cast<unsigned>(Basic->Altitude)));
#else
    if (Calculated->Flying && (Basic->Time - OLCLastTime >= dtOLC)) {
      bool restart;      
      restart = olc.addPoint(Basic->Longitude, 
			     Basic->Latitude, 
			     Calculated->NavAltitude,
			     Calculated->WaypointBearing,
			     Basic->Time-Calculated->TakeOffTime);
      
      if (restart && EnableOLC) {
	Calculated->ValidFinish = false;
	StartTask(Basic, Calculated, false, false);
	Calculated->ValidStart = true;
      }
      OLCLastTime += dtOLC;
    }
#endif /* NEW_OLC */
  }
}


// Final glide through terrain and footprint calculations


double FinalGlideThroughTerrain(const double this_bearing, 
				NMEA_INFO *Basic, 
                                DERIVED_INFO *Calculated,
                                double *retlat, double *retlon,
                                const double max_range,
				bool *out_of_range,
				double *TerrainBase) 
{
  double irange = GlidePolar::MacCreadyAltitude(MACCREADY, 
						1.0, this_bearing, 
						Calculated->WindSpeed, 
						Calculated->WindBearing, 
						0, 0, true, 0);
  const double start_lat = Basic->Latitude;
  const double start_lon = Basic->Longitude;
  double safetyterrain;

  if (retlat && retlon) {
    *retlat = start_lat;
    *retlon = start_lon;
  }
  *out_of_range = false;

  if ((irange<=0.0)||(Calculated->NavAltitude<=0)) {
    // can't make progress in this direction at the current windspeed/mc
    return 0;
  }

  const double glide_max_range = Calculated->NavAltitude/irange;

  safetyterrain=SAFETYALTITUDETERRAIN <= 0 ? 1 : SAFETYALTITUDETERRAIN;

  // returns distance one would arrive at altitude in straight glide
  // first estimate max range at this altitude
  double lat, lon;
  double last_lat, last_lon;
  double h=0.0, dh=0.0;
//  int imax=0;
  double last_dh=0;
  double altitude;

  // Warning: leave this part locked, RasterTerrain need no changes on the map position while working
  RasterTerrain::Lock();  //@ 101031 WE DO NEED IT!   BUG 101027:  no need, locking individually
  double retval = 0;
  int i=0;
  bool start_under = false;

  // calculate terrain rounding factor

  FindLatitudeLongitude(start_lat, start_lon, 0, 
                        glide_max_range/NUMFINALGLIDETERRAIN, &lat, &lon);

  double Xrounding = fabs(lon-start_lon)/2;
  double Yrounding = fabs(lat-start_lat)/2;
  RasterTerrain::SetTerrainRounding(Xrounding, Yrounding);

  lat = last_lat = start_lat;
  lon = last_lon = start_lon;

  altitude = Calculated->NavAltitude;
  h =  max((short)0, RasterTerrain::GetTerrainHeight(lat, lon)); 
  if (h==TERRAIN_INVALID) h=0; // @ 101027 FIX
  dh = altitude - h - safetyterrain; 
  last_dh = dh;
  if (dh<0) {
    start_under = true;
    // already below safety terrain height
    //    retval = 0;
    //    goto OnExit;
  }

  // find grid
  double dlat, dlon;

  FindLatitudeLongitude(lat, lon, this_bearing, glide_max_range, &dlat, &dlon);
  dlat -= start_lat;
  dlon -= start_lon;

  double f_scale = 1.0/NUMFINALGLIDETERRAIN;
  if ((max_range>0) && (max_range<glide_max_range)) {
    f_scale *= max_range/glide_max_range;
  }

  double delta_alt = -f_scale*Calculated->NavAltitude;

  dlat *= f_scale;
  dlon *= f_scale;

  for (i=1; i<=NUMFINALGLIDETERRAIN; i++) {
    double f;
    bool solution_found = false;
    double fi = i*f_scale;
    // fraction of glide_max_range

    if ((max_range>0)&&(fi>=1.0)) {
      // early exit
      *out_of_range = true;
#if 100125
      if (max_range<glide_max_range) 
      	retval = max_range;
      else
      	retval = glide_max_range;
#else
      retval = max_range;
#endif
      goto OnExit;
    }

    if (start_under) {
      altitude += 2.0*delta_alt;
    } else {
      altitude += delta_alt;
    }

    // find lat, lon of point of interest

    lat += dlat;
    lon += dlon;

    // find height over terrain
   
    h =  max((short)0, RasterTerrain::GetTerrainHeight(lat, lon)); 
    if (h==TERRAIN_INVALID) h=0; //@ 101027 FIX
    

    dh = altitude - h - safetyterrain;

    if (TerrainBase && (dh>0) && (h>0)) {
      *TerrainBase = min(*TerrainBase, h);
    }

    if (start_under) {
      if (dh>last_dh) {
	// better solution found, ok to continue...
	if (dh>0) {
	  // we've now found a terrain point above safety altitude,
	  // so consider rest of track to search for safety altitude
	  start_under = false;
	}
      } else {
	f= 0.0;
	solution_found = true;
      }
    } else if (dh<=0) {
      if ((dh<last_dh) && (last_dh>0)) {
	if (dh-last_dh==0) {
		f = 0.0;
	} else
        f = max(0.0, min(1.0, (-last_dh)/(dh-last_dh)));
      } else {
	f = 0.0;
      }
      solution_found = true;
    }
    if (solution_found) {
      double distance;
      lat = last_lat*(1.0-f)+lat*f;
      lon = last_lon*(1.0-f)+lon*f;
      if (retlat && retlon) {
        *retlat = lat;
        *retlon = lon;
      }
      DistanceBearing(start_lat, start_lon, lat, lon, &distance, NULL);
      retval = distance;
      goto OnExit;
    }
    last_dh = dh;
    last_lat = lat;
    last_lon = lon;
  }

  *out_of_range = true;
  retval = glide_max_range;

 OnExit:
  RasterTerrain::Unlock();
  return retval;
}


double PirkerAnalysis(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                      const double this_bearing,
                      const double GlideSlope) {


//  bool maxfound = false;
//  bool first = true;
  double pirker_mc = 0.0;
  double h_target = GlideSlope;
  double h;
  double dh= 1.0;
  double last_pirker_mc = 5.0;
  double last_dh = -1.0;
  double pirker_mc_zero = 0.0;
 
  (void)Basic;

  while (pirker_mc<10.0) {

    h = GlidePolar::MacCreadyAltitude(pirker_mc, 
                                      1.0, // unit distance
				      this_bearing, 
                                      Calculated->WindSpeed, 
                                      Calculated->WindBearing, 
                                      0, 0, true, 0);

    dh = (h_target-h); 
    // height difference, how much we have compared to 
    // how much we need at that speed.
    //   dh>0, we can afford to speed up

    if (dh==last_dh) {
      // same height, must have hit max speed.
      if (dh>0) {
        return last_pirker_mc;
      } else {
        return 0.0;
      }
    }

    if ((dh<=0)&&(last_dh>0)) {
      if (dh-last_dh < 0) {
	double f = (-last_dh)/(dh-last_dh);
	pirker_mc_zero = last_pirker_mc*(1.0-f)+f*pirker_mc;	
      } else {
	pirker_mc_zero = pirker_mc;
      }
      return pirker_mc_zero;
    }
    last_dh = dh;
    last_pirker_mc = pirker_mc;

    pirker_mc += 0.5;
  }
  if (dh>=0) {
    return pirker_mc;
  }
  return -1.0; // no solution found, unreachable without further climb
}


double MacCreadyTimeLimit(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
			  const double this_bearing,
			  const double time_remaining,
			  const double h_final) {

  // find highest Mc to achieve greatest distance in remaining time and height
  (void)Basic;

  double time_to_go;
  double mc;
  double mc_best = 0.0;
  double d_best = 0.0;
  const double windspeed =   Calculated->WindSpeed;
  const double windbearing = Calculated->WindBearing;
  const double navaltitude = Calculated->NavAltitude;
  
  for (mc=0; mc<10.0; mc+= 0.1) {

    double h_unit = GlidePolar::MacCreadyAltitude(mc, 
						 1.0, // unit distance
						 this_bearing, 
						 windspeed, 
						 windbearing,
						 NULL,
						 NULL,
						 1, // final glide
						 &time_to_go);
    if (time_to_go>0) {
      double p = time_remaining/time_to_go;    
      double h_spent = h_unit*p;    
      double dh = navaltitude-h_spent-h_final;    
      double d = 1.0*p;
      
      if ((d>d_best) && (dh>=0)) {
	mc_best = mc;
      }
    }
  }
  return mc_best;
}


DWORD lastTeamCodeUpdateTime = GetTickCount();

void CalculateOwnTeamCode(NMEA_INFO *Basic, DERIVED_INFO *Calculated) 
{
  if (!WayPointList) return;
  if (TeamCodeRefWaypoint < 0) return;
  if (lastTeamCodeUpdateTime + 10000 > GetTickCount()) return;

	
  double distance = 0;
  double bearing = 0;
  TCHAR code[10];
	
  lastTeamCodeUpdateTime = GetTickCount();

  /*
  distance =  Distance(WayPointList[TeamCodeRefWaypoint].Latitude, 
  	WayPointList[TeamCodeRefWaypoint].Longitude,
  	Basic->Latitude, 
  	Basic->Longitude);
  	
  bearing = Bearing(WayPointList[TeamCodeRefWaypoint].Latitude, 
  	WayPointList[TeamCodeRefWaypoint].Longitude,
  	Basic->Latitude, 
  	Basic->Longitude);
  */
	
  LL_to_BearRange(WayPointList[TeamCodeRefWaypoint].Latitude, 
                  WayPointList[TeamCodeRefWaypoint].Longitude,
                  Basic->Latitude, 
                  Basic->Longitude,
                  &bearing, &distance);

  GetTeamCode(code, bearing, distance);

  Calculated->TeammateBearing = bearing;
  Calculated->TeammateRange = distance;	

  //double teammateBearing = GetTeammateBearingFromRef(TeammateCode);
  //double teammateRange = GetTeammateRangeFromRef(TeammateCode);

  //Calculated->TeammateLongitude = FindLongitude(

  wcsncpy(Calculated->OwnTeamCode, code, 5);
  Calculated->OwnTeamCode[5]='\0'; // BUGFIX 100331
}


void CalculateTeammateBearingRange(NMEA_INFO *Basic, DERIVED_INFO *Calculated) 
{
  static bool InTeamSector = false;

  if (!WayPointList) return;
  if (TeamCodeRefWaypoint < 0) return;

  double ownDistance = 0;
  double ownBearing = 0;
  double mateDistance = 0;
  double mateBearing = 0;
	
  //ownBearing = Bearing(Basic->Latitude, Basic->Longitude,
  //	WayPointList[TeamCodeRefWaypoint].Latitude, 
  //	WayPointList[TeamCodeRefWaypoint].Longitude);	
  //
  //ownDistance =  Distance(Basic->Latitude, Basic->Longitude,
  //	WayPointList[TeamCodeRefWaypoint].Latitude, 
  //	WayPointList[TeamCodeRefWaypoint].Longitude);
		
  /*
  ownBearing = Bearing(WayPointList[TeamCodeRefWaypoint].Latitude, 
                       WayPointList[TeamCodeRefWaypoint].Longitude,
                       Basic->Latitude, 
                       Basic->Longitude
                       );	
  //
  ownDistance =  Distance(WayPointList[TeamCodeRefWaypoint].Latitude, 
                          WayPointList[TeamCodeRefWaypoint].Longitude,
                          Basic->Latitude, 
                          Basic->Longitude
                          );	
  */

  LL_to_BearRange(WayPointList[TeamCodeRefWaypoint].Latitude, 
                  WayPointList[TeamCodeRefWaypoint].Longitude,
                  Basic->Latitude, 
                  Basic->Longitude,
                  &ownBearing, &ownDistance);

  if (TeammateCodeValid)
    {
	
      //mateBearing = Bearing(Basic->Latitude, Basic->Longitude, TeammateLatitude, TeammateLongitude);
      //mateDistance = Distance(Basic->Latitude, Basic->Longitude, TeammateLatitude, TeammateLongitude);

      CalcTeammateBearingRange(ownBearing, ownDistance, 
                               TeammateCode, 
                               &mateBearing, &mateDistance);

      // TODO code ....change the result of CalcTeammateBearingRange to do this !
      if (mateBearing > 180)
        {
          mateBearing -= 180;
        }
      else
        {
          mateBearing += 180;
        }


      Calculated->TeammateBearing = mateBearing;
      Calculated->TeammateRange = mateDistance;

      FindLatitudeLongitude(Basic->Latitude, 
                            Basic->Longitude,
                            mateBearing,
                            mateDistance, 
                            &TeammateLatitude,
                            &TeammateLongitude);

      WayPointList[RESWP_TEAMMATE].Latitude   = TeammateLatitude;
      WayPointList[RESWP_TEAMMATE].Longitude  = TeammateLongitude;
      WayPointList[RESWP_TEAMMATE].Altitude   = Calculated->NavAltitude;

      if (mateDistance < 100 && InTeamSector==false)
        {
          InTeamSector=true;
          InputEvents::processGlideComputer(GCE_TEAM_POS_REACHED);
        }
      else if (mateDistance > 300)
        {
          InTeamSector = false;
        }
    }
  else
    {
      Calculated->TeammateBearing = 0;
      Calculated->TeammateRange = 0;
      WayPointList[RESWP_TEAMMATE].Latitude   = RESWP_INVALIDNUMBER;
      WayPointList[RESWP_TEAMMATE].Longitude  = RESWP_INVALIDNUMBER;
      WayPointList[RESWP_TEAMMATE].Altitude   = RESWP_INVALIDNUMBER;
    }

}

extern double CRUISE_EFFICIENCY;

#if 0 // NOT USED
static TCHAR szCalculationsPersistFileName[MAX_PATH]= TEXT("\0");
static TCHAR szCalculationsPersistDirectory[MAX_PATH]= TEXT("\0");

void DeleteCalculationsPersist(void) {
  DeleteFile(szCalculationsPersistFileName);
}

void LoadCalculationsPersist(DERIVED_INFO *Calculated) {
  if (szCalculationsPersistFileName[0]==0) {
    LocalPath(szCalculationsPersistFileName, TEXT(LKF_PERSIST));    
    LocalPath(szCalculationsPersistDirectory);
  }

  StartupStore(TEXT("LoadCalculationsPersist%s"),NEWLINE);

  HANDLE hFile;
  DWORD dwBytesWritten;
  DWORD size, sizein;
  hFile = CreateFile(szCalculationsPersistFileName,
                     GENERIC_READ,0,(LPSECURITY_ATTRIBUTES)NULL,
                     OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if(hFile!=INVALID_HANDLE_VALUE ) {
    size = sizeof(DERIVED_INFO);
    ReadFile(hFile,&sizein,sizeof(DWORD),&dwBytesWritten,(OVERLAPPED*)NULL);
    if (sizein != size) { CloseHandle(hFile); return; }
    ReadFile(hFile,Calculated,size,&dwBytesWritten,(OVERLAPPED*)NULL);

    size = sizeof(Statistics);
    ReadFile(hFile,&sizein,sizeof(DWORD),&dwBytesWritten,(OVERLAPPED*)NULL);
    if (sizein != size) { flightstats.Reset(); CloseHandle(hFile); return; }
    ReadFile(hFile,&flightstats,size,&dwBytesWritten,(OVERLAPPED*)NULL);

#ifndef NEW_OLC
    size = sizeof(OLCData);
    ReadFile(hFile,&sizein,sizeof(DWORD),&dwBytesWritten,(OVERLAPPED*)NULL);
    if (sizein != size) { olc.ResetFlight(); CloseHandle(hFile); return; }
    ReadFile(hFile,&olc.data,size,&dwBytesWritten,(OVERLAPPED*)NULL);   
#endif /* NEW_OLC */

    size = sizeof(double);
    ReadFile(hFile,&sizein,sizeof(DWORD),&dwBytesWritten,(OVERLAPPED*)NULL);
    if (sizein != size*5) { CloseHandle(hFile); return; }
    ReadFile(hFile,&MACCREADY,size,&dwBytesWritten,(OVERLAPPED*)NULL);   
    ReadFile(hFile,&QNH,size,&dwBytesWritten,(OVERLAPPED*)NULL);   
    ReadFile(hFile,&BUGS,size,&dwBytesWritten,(OVERLAPPED*)NULL);   
    ReadFile(hFile,&BALLAST,size,&dwBytesWritten,(OVERLAPPED*)NULL);   
    ReadFile(hFile,&CuSonde::maxGroundTemperature,
             size,&dwBytesWritten,(OVERLAPPED*)NULL);   
    //    ReadFile(hFile,&CRUISE_EFFICIENCY,
    //             size,&dwBytesWritten,(OVERLAPPED*)NULL);   

    MACCREADY = min(10.0,max(MACCREADY,0));
    QNH = min(1113.2, max(QNH,913.2));
    BUGS = min(1.0, max(BUGS,0.0));
    BALLAST = min(1.0, max(BALLAST,0.0));
    //   CRUISE_EFFICIENCY = min(1.5, max(CRUISE_EFFICIENCY,0.75));

    StartupStore(TEXT("LoadCalculationsPersist OK%s"),NEWLINE);

    CloseHandle(hFile);
  } else {
    StartupStore(TEXT("LoadCalculationsPersist file not found%s"),NEWLINE);
  }
}


void SaveCalculationsPersist(DERIVED_INFO *Calculated) {
  HANDLE hFile;
  DWORD dwBytesWritten;
  DWORD size;

  LoggerClearFreeSpace();

  if (FindFreeSpace(szCalculationsPersistDirectory)<MINFREESTORAGE) {
    if (!LoggerClearFreeSpace()) {
      StartupStore(TEXT("SaveCalculationsPersist insufficient storage%s"),NEWLINE);
      return;
    } else {
      StartupStore(TEXT("SaveCalculationsPersist cleared logs to free storage%s"),NEWLINE);
    }
  }

  StartupStore(TEXT("SaveCalculationsPersist%s"),NEWLINE);

  hFile = CreateFile(szCalculationsPersistFileName,
                     GENERIC_WRITE,0,(LPSECURITY_ATTRIBUTES)NULL,
                     CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if (hFile!=INVALID_HANDLE_VALUE ) {
    size = sizeof(DERIVED_INFO);
    WriteFile(hFile,&size,sizeof(DWORD),&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile,Calculated,size,&dwBytesWritten,(OVERLAPPED*)NULL);
    size = sizeof(Statistics);
    WriteFile(hFile,&size,sizeof(DWORD),&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile,&flightstats,size,&dwBytesWritten,(OVERLAPPED*)NULL);
#ifndef NEW_OLC
    size = sizeof(OLCData);
    WriteFile(hFile,&size,sizeof(DWORD),&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile,&olc.data,size,&dwBytesWritten,(OVERLAPPED*)NULL);
#endif /* NEW_OLC */
    size = sizeof(double)*5;
    WriteFile(hFile,&size,sizeof(DWORD),&dwBytesWritten,(OVERLAPPED*)NULL);
    size = sizeof(double);
    WriteFile(hFile,&MACCREADY,size,&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile,&QNH,size,&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile,&BUGS,size,&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile,&BALLAST,size,&dwBytesWritten,(OVERLAPPED*)NULL);
    WriteFile(hFile,&CuSonde::maxGroundTemperature,
              size,&dwBytesWritten,(OVERLAPPED*)NULL);
    //    WriteFile(hFile,&CRUISE_EFFICIENCY,
    //              size,&dwBytesWritten,(OVERLAPPED*)NULL);

    StartupStore(TEXT("SaveCalculationsPersist ok%s"),NEWLINE);

    CloseHandle(hFile);
  } else {
    StartupStore(TEXT("SaveCalculationsPersist can't create file%s"),NEWLINE);
  }

}

#endif // NO USE 

#define NUM_CAL_SPEED 25
#define NUM_CAL_VARIO 101
#define NUM_CAL_VSPEED 50

#if 0 // NO CALIBRATION FOR LK

static double calibration_tevario_val[NUM_CAL_SPEED][NUM_CAL_VARIO];
static unsigned int calibration_tevario_num[NUM_CAL_SPEED][NUM_CAL_VARIO];
static double calibration_speed_val[NUM_CAL_VSPEED];
static unsigned int calibration_speed_num[NUM_CAL_VSPEED];


void CalibrationInit(void) {
  int i, j;
  for (i=0; i< NUM_CAL_SPEED; i++) {
    for (j=0; j< NUM_CAL_VARIO; j++) {
      calibration_tevario_val[i][j] = 0;
      calibration_tevario_num[i][j] = 0;
    }
  }
  for (i=0; i< NUM_CAL_VSPEED; i++) {
    calibration_speed_val[i] = 0;
    calibration_speed_num[i] = 0;
  }
}



void CalibrationSave(void) {
  int i, j;
  double v, w = 0, wav;
  StartupStore(TEXT("Calibration data for TE vario%s"),NEWLINE);
  for (i=0; i< NUM_CAL_SPEED; i++) {
    for (j=0; j< NUM_CAL_VARIO; j++) {
      if (calibration_tevario_num[i][j]>0) {
        v = i*2.0+20.0;
        w = (j-50.0)/10.0;
        wav = calibration_tevario_val[i][j]/calibration_tevario_num[i][j];
        StartupStore(TEXT("%g %g %g %d%s"), v, w, wav,
                  calibration_tevario_num[i][j],NEWLINE);
      }
    }
  }
  StartupStore(TEXT("Calibration data for ASI%s"),NEWLINE);
  for (i=0; i< NUM_CAL_VSPEED; i++) {
    if (calibration_speed_num[i]>0) {
      v = i+20.0;
      wav = calibration_speed_val[i]/calibration_speed_num[i];
      StartupStore(TEXT("%g %g %g %d%s"), v, w, wav,
                calibration_speed_num[i],NEWLINE);
    }
  }
}


void CalibrationUpdate(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  if (!Calculated->Flying) return;
  if ((!Basic->AirspeedAvailable) || (Basic->TrueAirspeed<=0)) {
    return;
  }
  double ias_to_tas = Basic->TrueAirspeed/
    max(1.0,Basic->IndicatedAirspeed);

  // Vario calibration info 
  int index_te_vario = lround(Calculated->GPSVarioTE*10)+50;
  int index_speed = lround((Basic->TrueAirspeed-20)/2);
  if (index_te_vario < 0) 
    return;
  if (index_te_vario >= NUM_CAL_VARIO) 
    return;
  if (index_speed<0) 
    return;
  if (index_speed>= NUM_CAL_SPEED) 
    return;
  
  calibration_tevario_val[index_speed][index_te_vario] += 
    Basic->Vario*ias_to_tas;
  calibration_tevario_num[index_speed][index_te_vario] ++;

  // ASI calibration info 
  int index_vspeed = lround((Basic->TrueAirspeed-20));
  if (index_vspeed<0) 
    return;
  if (index_vspeed>= NUM_CAL_VSPEED) 
    return;

  calibration_speed_val[index_vspeed] += Calculated->TrueAirspeedEstimated;
  calibration_speed_num[index_vspeed] ++;

}

#endif // --- NO CALIBRATION

static double EffectiveMacCready_internal(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
					  bool cruise_efficiency_mode) {

  if (Calculated->ValidFinish) return 0;
  if (ActiveWayPoint<=0) return 0; // no e mc before start
  if (!Calculated->ValidStart) return 0;
  if (Calculated->TaskStartTime<0) return 0;

  if (!ValidTaskPoint(ActiveWayPoint) 
      || !ValidTaskPoint(ActiveWayPoint-1)) return 0;
  if (Calculated->TaskDistanceToGo<=0) {
    return 0;
  }

  LockTaskData();

  double start_speed = Calculated->TaskStartSpeed;
  double V_bestld = GlidePolar::Vbestld;
  double energy_height_start = 
    max(0.0, start_speed*start_speed-V_bestld*V_bestld)/(9.81*2.0);

  double telapsed = Basic->Time-Calculated->TaskStartTime;
  double height_below_start = 
    Calculated->TaskStartAltitude + energy_height_start 
    - Calculated->NavAltitude - Calculated->EnergyHeight;

  double LegDistances[MAXTASKPOINTS];
  double LegBearings[MAXTASKPOINTS];

  for (int i=0; i<ActiveWayPoint; i++) {
    double w1lat = WayPointList[Task[i+1].Index].Latitude;
    double w1lon = WayPointList[Task[i+1].Index].Longitude;
    double w0lat = WayPointList[Task[i].Index].Latitude;
    double w0lon = WayPointList[Task[i].Index].Longitude;
    if (AATEnabled) {
      if (ValidTaskPoint(i+1)) {
        w1lat = Task[i+1].AATTargetLat;
        w1lon = Task[i+1].AATTargetLon;
      }
      if (i>0) {
        w0lat = Task[i].AATTargetLat;
        w0lon = Task[i].AATTargetLon;
      }
    }
    DistanceBearing(w0lat, 
                    w0lon,
                    w1lat, 
                    w1lon,
                    &LegDistances[i], &LegBearings[i]);

    if (i==ActiveWayPoint-1) {
    
      double leg_covered = ProjectedDistance(w0lon, w0lat,
                                             w1lon, w1lat,
                                             Basic->Longitude,
                                             Basic->Latitude);
      LegDistances[i] = leg_covered;
    }
    if ((StartLine==0) && (i==0)) {
      // Correct speed calculations for radius
      // JMW TODO accuracy: leg distance replace this with more accurate version
      // leg_distance -= StartRadius;
      LegDistances[0] = max(0.1,LegDistances[0]-StartRadius);
    }
  }

  // OK, distance/bearings calculated, now search for Mc

  double value_found;
  if (cruise_efficiency_mode) {
    value_found = 1.5; // max
  } else {
    value_found = 10.0; // max
  }

  for (double value_scan=0.01; value_scan<1.0; value_scan+= 0.01) {

    double height_remaining = height_below_start;
    double time_total=0;

    double mc_effective;
    double cruise_efficiency;

    if (cruise_efficiency_mode) {
      mc_effective = MACCREADY;
      if (Calculated->FinalGlide && (Calculated->timeCircling>0)) {
	mc_effective = CALCULATED_INFO.TotalHeightClimb
	  /CALCULATED_INFO.timeCircling;
      }
      cruise_efficiency = 0.5+value_scan;
    } else {
      mc_effective = value_scan*10.0;
      cruise_efficiency = 1.0;
    }

    // Now add times from start to this waypoint,
    // allowing for final glide where possible if aircraft height is below
    // start
    
    for(int i=ActiveWayPoint-1;i>=0; i--) {

      double time_this;

      double height_used_this = 
        GlidePolar::MacCreadyAltitude(mc_effective,
                                      LegDistances[i],
                                      LegBearings[i],
                                      Calculated->WindSpeed,
                                      Calculated->WindBearing,
                                      0, NULL, 
                                      (height_remaining>0), 
                                      &time_this,
                                      height_remaining, 
				      cruise_efficiency);

      height_remaining -= height_used_this;
            
      if (time_this>=0) { 
        time_total += time_this;
      } else {
        // invalid! break out of loop early
        time_total= time_this;
        i= -1;
        continue;
      }     
    }

    if (time_total<0) {
      // invalid
      continue;
    }
    if (time_total>telapsed) { 
      // already too slow
      continue;
    }

    // add time for climb from start height to height above start
    if (height_below_start<0) {
      time_total -= height_below_start/mc_effective;
    }
    // now check time..
    if (time_total<telapsed) {       
      if (cruise_efficiency_mode) {
	value_found = cruise_efficiency;
      } else {
	value_found = mc_effective;
      }
      break;
    }

  }

  UnlockTaskData();

  return value_found;
}


double EffectiveCruiseEfficiency(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  double value = EffectiveMacCready_internal(Basic, Calculated, true);
  if (value<0.75) {
    return 0.75;
  }
  return value;
}


double EffectiveMacCready(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  return EffectiveMacCready_internal(Basic, Calculated, false);
}
