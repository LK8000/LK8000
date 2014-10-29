/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Calculations.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "StdAfx.h"
#include "externs.h"
#include "LKMapWindow.h"
#include "Calculations.h"
#include "Dialogs.h"
#include "Parser.h"
#include "compatibility.h"
#include "LKProcess.h"
#include "Utils.h"
#include "McReady.h"
#include "Airspace.h"
#include "AirspaceWarning.h"
#include "Logger.h"
#include <math.h>
#include "InputEvents.h"
#include "Message.h"
#include "RasterTerrain.h"
#include "TeamCodeCalculation.h"
#include <tchar.h>
#include "ThermalLocator.h"
#include "windanalyser.h"
#include "Atmosphere.h"
#include "ContestMgr.h"
#include "AATDistance.h"
#include "NavFunctions.h" // used for team code
#include "Calculations2.h"
#include "Port.h"
#include "WindZigZag.h"
#include "device.h"
#ifdef NEWCLIMBAV
#include "ClimbAverageCalculator.h" // JMW new
#endif
#include "Waypointparser.h"
#include "LKAirspace.h"
#include "DoInits.h"

using std::min;
using std::max;

#include "utils/heapcheck.h"

using std::min;
using std::max;
extern void CalculateMagneticVariation();
extern int CalculateWindRotary(windrotary_s *wbuf, double iaspeed, double *wfrom, double *wspeed, int windcalctime, int wmode);
extern double CalculateLDRotary(ldrotary_s *buf, DERIVED_INFO *Calculated);
extern void InsertLDRotary(ldrotary_s *buf, int distance, int altitude);
extern void InsertWindRotary(windrotary_s *wbuf, double speed, double track, double altitude);
extern void ResetFlightStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

//#define DEBUGTGATES	1
//#define DEBUGATE	1

WindAnalyser *windanalyser = NULL;
AATDistance aatdistance;
DERIVED_INFO Finish_Derived_Info;
ThermalLocator thermallocator;

// 0: Final glide only
// 1: Set to average if in climb mode
// 2: Average if in climb mode, final glide in final glide mode

#define THERMAL_TIME_MIN 45.0
double CRUISE_EFFICIENCY = 1.0;

#define MAPMODE8000    !MapWindow::mode.AnyPan()&&MapSpaceMode==MSM_MAP


//double SpeedHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

static void Vario(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void LD(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void Heading(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void CruiseLD(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void Flaps(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void Average30s(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void AverageThermal(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

extern void Turning(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

//void PercentCircling(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
 //                           const double Rate);
static void LastThermalStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void ThermalGain(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void MaxHeightGain(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void DistanceToNext(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void EnergyHeightNavAltitude(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void TaskSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                      const double this_maccready);
static void AltitudeRequired(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
			     const double this_maccready);

static void TaskStatistics(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double this_maccready);
static void InSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static bool  InFinishSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const int i);
static bool  InTurnSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const int i);
static void PredictNextPosition(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void AATStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
static void DoAutoMacCready(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
//static void ThermalBand(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void TakeoffLanding(NMEA_INFO *Basic, DERIVED_INFO *Calculated);


static void TerrainHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

//static void TerrainFootprint(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

extern void ConditionMonitorsUpdate(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void BallastDump();

// LK8000 functions
extern void DoAlternates(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int AltWaypoint); // VENTA3

#ifdef DEBUG
#define DEBUGTASKSPEED
#endif


#define TASKINDEX    Task[ActiveWayPoint].Index


int getFinalWaypoint() {
  int i;
  i=max(-1,min(MAXTASKPOINTS,ActiveWayPoint));

  i++;
  LockTaskData();
  while((i<MAXTASKPOINTS) && (Task[i].Index != -1))
    {
      i++;
    }
  UnlockTaskData();
  return i-1;
}


static bool ActiveIsFinalWaypoint() {
  return (ActiveWayPoint == getFinalWaypoint());
}

static void CheckTransitionFinalGlide(NMEA_INFO *Basic, 
                                      DERIVED_INFO *Calculated) {
  int FinalWayPoint = getFinalWaypoint();
  // update final glide mode status
  if (((ActiveWayPoint == FinalWayPoint)
       ||(ForceFinalGlide)) 
      && (ValidTaskPoint(ActiveWayPoint))) {
    
    if (Calculated->FinalGlide == 0)
      InputEvents::processGlideComputer(GCE_FLIGHTMODE_FINALGLIDE);
    Calculated->FinalGlide = 1;
  } else {
    if (Calculated->FinalGlide == 1)
      InputEvents::processGlideComputer(GCE_FLIGHTMODE_CRUISE);
    Calculated->FinalGlide = 0;
  }

}


static void CheckForceFinalGlide(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // Auto Force Final Glide forces final glide mode
  // if above final glide...
    if (AutoForceFinalGlide) {
      if (!Calculated->FinalGlide) {
        if (Calculated->TaskAltitudeDifference>120) {
          ForceFinalGlide = true;
        } else {
          ForceFinalGlide = false;
        }
      } else {
        if (Calculated->TaskAltitudeDifference<-120) {
          ForceFinalGlide = false;
        } else {
          ForceFinalGlide = true;
        }
      }
    }
}

//
// twp is a task index reference, not a waypoint index
// CAREFUL> FAIFinishHeight is considering SafetyAltitude if enabled for the wp type.
// in Calculations  height_above_finish is the difference between first and last task wp,
// but they may have different safetyaltitude appliances! This is why it should not be
// allowed to enter a landable inside a task until we get rid of this stuff.
//
double FAIFinishHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int twp) {

  int FinalWayPoint = getFinalWaypoint();

  double safetyaltitudearrival=SAFETYALTITUDEARRIVAL; 

  if (twp== -1) {
    twp = FinalWayPoint;
  }

  double wp_alt;

  if(ValidTaskPoint(twp)) {
    wp_alt = WayPointList[Task[twp].Index].Altitude;
    if (!CheckSafetyAltitudeApplies(Task[twp].Index)) safetyaltitudearrival=0;
  } else {
    #if TESTBENCH
    StartupStore(_T("..... FAIFinishHeight invalid twp=%d%s"),twp,NEWLINE);
    #endif
    wp_alt = 0;
  }

  if (twp==FinalWayPoint) {
    if (EnableFAIFinishHeight && !AATEnabled) {
      // maximum allowed loss of height in order to conform to FAI rules
      double maxHeightLoss = min(1000.0,
                                 (Calculated->TaskDistanceCovered+Calculated->TaskDistanceToGo) * 0.01);
      
      return max(max(FinishMinHeight/1000.0, safetyaltitudearrival)+ wp_alt, 
                 Calculated->TaskStartAltitude-maxHeightLoss);
    } else {
      return max(FinishMinHeight/1000.0, safetyaltitudearrival)+wp_alt;
    }
  } else {
    return wp_alt + safetyaltitudearrival;
  }
}


double SpeedHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  (void)Basic;
  if (Calculated->TaskDistanceToGo<=0) {
    return 0;
  }

  // Fraction of task distance covered
  double d_fraction = Calculated->TaskDistanceCovered/
    (Calculated->TaskDistanceCovered+Calculated->TaskDistanceToGo);

  double dh_start = Calculated->TaskStartAltitude;

  double dh_finish = FAIFinishHeight(Basic, Calculated, -1);

  // Excess height
  return Calculated->NavAltitude 
    - (dh_start*(1.0-d_fraction)+dh_finish*(d_fraction));
}



void TerrainFootprint(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  double bearing, distance;
  double lat, lon;
  bool out_of_range;

  // estimate max range (only interested in at most one screen distance away)
  // except we need to scan for terrain base, so 20km search minimum is required
  double mymaxrange = max(20000.0, MapWindow::GetApproxScreenRange());

  Calculated->TerrainBase = Calculated->TerrainAlt;

  for (int i=0; i<=NUMTERRAINSWEEPS; i++) {
    bearing = (i*360.0)/NUMTERRAINSWEEPS;
    distance = FinalGlideThroughTerrain(bearing, 
                                        Basic, 
                                        Calculated, &lat, &lon,
                                        mymaxrange, &out_of_range,
					&Calculated->TerrainBase);
    if (out_of_range) {
      FindLatitudeLongitude(Basic->Latitude, Basic->Longitude, 
                            bearing, 
                            distance,	 // limited, originally maxrange and more..
                            &lat, &lon);
    }
    Calculated->GlideFootPrint[i].x = lon;
    Calculated->GlideFootPrint[i].y = lat;
  }
  Calculated->Experimental = Calculated->TerrainBase;
}



void RefreshTaskStatistics(void) {
  LockTaskData();
  TaskStatistics(&GPS_INFO, &CALCULATED_INFO, MACCREADY);
  AATStats(&GPS_INFO, &CALCULATED_INFO);
  TaskSpeed(&GPS_INFO, &CALCULATED_INFO, MACCREADY);
  IterateEffectiveMacCready(&GPS_INFO, &CALCULATED_INFO);
  UnlockTaskData();
}


static bool IsFinalWaypoint(void) {
  bool retval;
  LockTaskData();
  if (ValidTaskPoint(ActiveWayPoint) && (Task[ActiveWayPoint+1].Index >= 0)) {
    retval = false;
  } else {
    retval = true;
  }
  UnlockTaskData();
  return retval;
}

extern int FastLogNum; // number of points to log at high rate

void AnnounceWayPointSwitch(DERIVED_INFO *Calculated, bool do_advance) {
  if (ActiveWayPoint == 0) {
    InputEvents::processGlideComputer(GCE_TASK_START); // 101014

  } else if (Calculated->ValidFinish && IsFinalWaypoint()) {
    InputEvents::processGlideComputer(GCE_TASK_FINISH);
  } else {
    InputEvents::processGlideComputer(GCE_TASK_NEXTWAYPOINT);
  }

  if (do_advance) {
    ActiveWayPoint++;
  }

  SelectedWaypoint = TASKINDEX; 

  // set waypoint detail to active task WP

  // start logging data at faster rate
  FastLogNum = 5;
}


//
// Sollfarh / Dolphin Speed calculator
//
void SpeedToFly(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  double n;
  // get load factor
  if (Basic->AccelerationAvailable) {
    n = fabs(Basic->AccelZ);
  } else {
    n = fabs(Calculated->Gload);
  }

  double delta_mc;
  double current_mc = MACCREADY;

  delta_mc = current_mc-Calculated->NettoVario;

  // TODO FIX should we use this approach?
  if (1 || (Calculated->Vario <= current_mc)) {
    // airmass value is worse than mc threshold, so find opt cruise speed

    double VOptnew;
    
    if (!ValidTaskPoint(ActiveWayPoint) || !Calculated->FinalGlide) {
      // calculate speed as if cruising, wind has no effect on opt speed
      GlidePolar::MacCreadyAltitude(delta_mc,
                                    100.0, // dummy value
                                    Basic->TrackBearing, 
                                    0.0, 
                                    0.0, 
                                    NULL, 
                                    &VOptnew, 
                                    false, 
                                    NULL, 0, CRUISE_EFFICIENCY);
    } else {
      GlidePolar::MacCreadyAltitude(delta_mc,
                                    100.0, // dummy value
                                    Basic->TrackBearing, 
                                    Calculated->WindSpeed, 
                                    Calculated->WindBearing, 
                                    0, 
                                    &VOptnew, 
                                    true,
                                    NULL, 1.0e6, CRUISE_EFFICIENCY);
    }
    
    // put low pass filter on VOpt so display doesn't jump around
    // too much
    if (Calculated->Vario <= current_mc) {
      Calculated->VOpt = max(Calculated->VOpt,
			     GlidePolar::Vminsink*sqrt(n));
    } else {
      Calculated->VOpt = max(Calculated->VOpt,
			     (double)GlidePolar::Vminsink);
    }
    Calculated->VOpt = LowPassFilter(Calculated->VOpt,VOptnew, 0.6);
    
  } else {
    // this air mass is better than maccready, so fly at minimum sink speed
    // calculate speed of min sink adjusted for load factor 
    Calculated->VOpt = GlidePolar::Vminsink*sqrt(n);
  }

}


void NettoVario(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  double n;
  // get load factor
  if (Basic->AccelerationAvailable) {
	n = fabs(Basic->AccelZ);
  } else {
	n = fabs(Calculated->Gload);
  }

  // calculate sink rate of glider for calculating netto vario

  bool replay_disabled = !ReplayLogger::IsEnabled();

  double glider_sink_rate;    
  if (Basic->AirspeedAvailable && replay_disabled) {
    glider_sink_rate= GlidePolar::SinkRate(max((double)GlidePolar::Vminsink, Basic->IndicatedAirspeed), n);
  } else {
    glider_sink_rate= GlidePolar::SinkRate(max((double)GlidePolar::Vminsink, Calculated->IndicatedAirspeedEstimated), n);
  }
  Calculated->GliderSinkRate = glider_sink_rate;

  if (Basic->NettoVarioAvailable && replay_disabled) {
	Calculated->NettoVario = Basic->NettoVario;
  } else {
	if (Basic->VarioAvailable && replay_disabled) {
		Calculated->NettoVario = Basic->Vario - glider_sink_rate;
	} else {
		Calculated->NettoVario = Calculated->Vario - glider_sink_rate;
	}
  }
}


BOOL DoCalculationsVario(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static double LastTime = 0;

  NettoVario(Basic, Calculated);
  SpeedToFly(Basic, Calculated);

  // has GPS time advanced?
  if(Basic->Time <= LastTime)
    {
      LastTime = Basic->Time; 
      return FALSE;      
    }
  LastTime = Basic->Time;

  return TRUE;
}


void Heading(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  double x0, y0, mag;
  static double LastTime = 0;
  static double lastHeading = 0;

  if (DoInit[MDI_HEADING]) {
	LastTime = 0;
	lastHeading = 0;
	DoInit[MDI_HEADING]=false;
  }

  if ((Basic->Speed>0)||(Calculated->WindSpeed>0)) {

    x0 = fastsine(Basic->TrackBearing)*Basic->Speed;
    y0 = fastcosine(Basic->TrackBearing)*Basic->Speed;
    x0 += fastsine(Calculated->WindBearing)*Calculated->WindSpeed;
    y0 += fastcosine(Calculated->WindBearing)*Calculated->WindSpeed;

    Calculated->Heading = AngleLimit360(atan2(x0,y0)*RAD_TO_DEG);

    if (!Calculated->Flying) {
      // don't take wind into account when on ground
      Calculated->Heading = Basic->TrackBearing;
    }

    // calculate turn rate in wind coordinates
    if(Basic->Time > LastTime) {
      double dT = Basic->Time - LastTime;

      Calculated->TurnRateWind = AngleLimit180(Calculated->Heading
                                               - lastHeading)/dT;

      lastHeading = Calculated->Heading;
    }
    LastTime = Basic->Time;

    // calculate estimated true airspeed
    mag = isqrt4((unsigned long)(x0*x0*100+y0*y0*100))/10.0;
    Calculated->TrueAirspeedEstimated = mag;
    Calculated->IndicatedAirspeedEstimated = mag/AirDensityRatio(Calculated->NavAltitude);
    // estimate bank angle (assuming balanced turn)
    double angle = atan(DEG_TO_RAD*Calculated->TurnRateWind*
			Calculated->TrueAirspeedEstimated/9.81);

    Calculated->BankAngle = RAD_TO_DEG*angle;
    Calculated->Gload = 1.0/max(0.001,fabs(cos(angle)));

    // estimate pitch angle (assuming balanced turn)
/*
    Calculated->PitchAngle = RAD_TO_DEG*
      atan2(Calculated->GPSVario-Calculated->Vario,
           Calculated->TrueAirspeedEstimated);
*/
	// should be used as here only when no real vario available
    Calculated->PitchAngle = RAD_TO_DEG*	
      atan2(Calculated->Vario,
           Calculated->TrueAirspeedEstimated);

    // update zigzag wind
    if (((AutoWindMode & D_AUTOWIND_ZIGZAG)==D_AUTOWIND_ZIGZAG) 
        && (!ReplayLogger::IsEnabled())) {
      double zz_wind_speed;
      double zz_wind_bearing;
      int quality;
      quality = WindZigZagUpdate(Basic, Calculated, 
                                 &zz_wind_speed, 
				 &zz_wind_bearing);
      if (quality>0) {
        SetWindEstimate(zz_wind_speed, zz_wind_bearing, quality);
/* 100118 redundant!! removed. TOCHECK
        Vector v_wind;
        v_wind.x = zz_wind_speed*cos(zz_wind_bearing*3.1415926/180.0);
        v_wind.y = zz_wind_speed*sin(zz_wind_bearing*3.1415926/180.0);
        LockFlightData();
        if (windanalyser) {
	  windanalyser->slot_newEstimate(Basic, Calculated, v_wind, quality);
        }
        UnlockFlightData();
*/
      }
    }
  // else basic speed is 0 and there is no wind.. 
  } else { 
    Calculated->Heading = Basic->TrackBearing;
    Calculated->TrueAirspeedEstimated = 0; // BUGFIX 100318
    Calculated->IndicatedAirspeedEstimated = 0; // BUGFIX 100318
  }

}


void  SetWindEstimate(const double wind_speed, 
		      const double wind_bearing, 
		      const int quality) {
  Vector v_wind;
  v_wind.x = wind_speed*cos(wind_bearing*3.1415926/180.0);
  v_wind.y = wind_speed*sin(wind_bearing*3.1415926/180.0);
  LockFlightData();
  if (windanalyser) {
    windanalyser->slot_newEstimate(&GPS_INFO, &CALCULATED_INFO, v_wind, quality);
  }
  UnlockFlightData();
}


// VENTA3 added radial
void DistanceToHome(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  int home_waypoint = HomeWaypoint;

  if (!ValidWayPoint(home_waypoint)) {
    Calculated->HomeDistance = 0.0;
    Calculated->HomeRadial = 0.0; // VENTA3
    return;
  }

  double w1lat = WayPointList[home_waypoint].Latitude;
  double w1lon = WayPointList[home_waypoint].Longitude;
  double w0lat = Basic->Latitude;
  double w0lon = Basic->Longitude;
    
  DistanceBearing(w1lat, w1lon,
                  w0lat, w0lon,
                  &Calculated->HomeDistance, &Calculated->HomeRadial);

}



bool FlightTimes(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  static double LastTime = 0;

  if ((Basic->Time != 0) && (Basic->Time <= LastTime)) {
	if ( (LastTime - Basic->Time >30 ) && (!Basic->NAVWarning) && 
	// replay logger does not consider UTC 00 incrementing by 85000 or whatever the basic time.
	// Meanwhile, we may also skip the 00 utc because of interpolation. So we consider this.
	!( ReplayLogger::IsEnabled() && Basic->Time<60)  ) {
		// Reset statistics.. (probably due to being in IGC replay mode)
		StartupStore(_T("... Time is in the past! %s%s"), WhatTimeIsIt(),NEWLINE);

		ResetFlightStats(Basic, Calculated);
		time_in_flight=0;
		time_on_ground=0;
	}

	LastTime = Basic->Time; 
	return false;      
  }

  LastTime = Basic->Time;

  double t = DetectStartTime(Basic, Calculated);
  if (t>0) {
	Calculated->FlightTime = t;
  } 
  #if 0
  else {
	if (Calculated->Flying) {
		StartupStore(_T("... negative start time=%f\n"),t);
	}
  }
  #endif

  TakeoffLanding(Basic, Calculated);

  return true;
}


void StartTask(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
	       const bool do_advance,
               const bool do_announce) {
  Calculated->ValidFinish = false;
  Calculated->TaskStartTime = Basic->Time ;
  Calculated->TaskStartSpeed = Basic->Speed;
  Calculated->TaskStartAltitude = Calculated->NavAltitude;
  Calculated->LegStartTime = Basic->Time;
  flightstats.LegStartTime[0] = Basic->Time;
  flightstats.LegStartTime[1] = Basic->Time;

  Calculated->CruiseStartLat = Basic->Latitude;
  Calculated->CruiseStartLong = Basic->Longitude;
  Calculated->CruiseStartAlt = Calculated->NavAltitude;
  Calculated->CruiseStartTime = Basic->Time;

  aatdistance.Reset();

  // JMW TODO accuracy: Get time from aatdistance module since this is
  // more accurate

  // JMW clear thermal climb average on task start
  flightstats.ThermalAverage.Reset();
  flightstats.Task_Speed.Reset();
  Calculated->AverageThermal = 0;
  Calculated->WaypointBearing=0;

  // JMW reset time cruising/time circling stats on task start
  Calculated->timeCircling = 0;
  Calculated->timeCruising = 0;
  Calculated->TotalHeightClimb = 0;

//  // reset max height gain stuff on task start REMOVE REMOVE we dont reset anymore on task start or restart
//  Calculated->MaxHeightGain = 0; // REMOVE
//  Calculated->MinAltitude = 0;   // REMOVE
//  Calculated->MaxAltitude = 0;   // REMOVE

  if (do_announce) {
    AnnounceWayPointSwitch(Calculated, do_advance);
  } else {
    if (do_advance) {
      ActiveWayPoint=1;
      SelectedWaypoint = TASKINDEX; 
    }
  }
}


void CloseCalculations() {
  if (windanalyser) {
    delete windanalyser;
    windanalyser = NULL;
  }
}



void InitCalculations(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  #if TESTBENCH
  StartupStore(TEXT(". Init Calculations%s"),NEWLINE);
  #endif

  ResetFlightStats(Basic, Calculated);

  // Initialise calculations, on first run DoInit will make it and return
  DoRangeWaypointList(Basic,Calculated);
  DoTraffic(Basic,Calculated);
  DoAirspaces(Basic,Calculated);
  DoThermalHistory(Basic,Calculated);

  InitAlarms();

  LockFlightData();
  if (!windanalyser) {
    windanalyser = new WindAnalyser();
  }
  UnlockFlightData();

}


void AverageClimbRate(NMEA_INFO *Basic, DERIVED_INFO *Calculated) 
{
  if (Basic->AirspeedAvailable && Basic->VarioAvailable  
      && (!Calculated->Circling)) {

    int vi = iround(Basic->IndicatedAirspeed);

    if ((vi<=0) || (vi>= SAFTEYSPEED)) {
      // out of range
      return;
    }
    if (Basic->AccelerationAvailable) {
      if (fabs(fabs(Basic->Gload)-1.0)>0.25) {
        // G factor too high
        return;
      }
    } 
    if (Basic->TrueAirspeed>0) {

      // TODO: Check this is correct for TAS/IAS

      double ias_to_tas = Basic->IndicatedAirspeed/Basic->TrueAirspeed;
      double w_tas = Basic->Vario*ias_to_tas;

      Calculated->AverageClimbRate[vi]+= w_tas;
      Calculated->AverageClimbRateN[vi]++;
    }
  }
}


extern bool TargetDialogOpen;
extern void CalculateOrbiter(NMEA_INFO *Basic, DERIVED_INFO *Calculated);


BOOL DoCalculations(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{

  // first thing: assign navaltitude!
  EnergyHeightNavAltitude(Basic, Calculated);
  Heading(Basic, Calculated);
  DistanceToNext(Basic, Calculated);
  DistanceToHome(Basic, Calculated);
  DetectFreeFlying(Calculated);	// check ongoing powerless flight
  DoLogging(Basic, Calculated);
  Vario(Basic,Calculated);
  TerrainHeight(Basic, Calculated);
  AltitudeRequired(Basic, Calculated, MACCREADY);
  DoAlternates(Basic,Calculated,TASKINDEX); 
  if (MAPMODE8000) {
	DoAlternates(Basic,Calculated,RESWP_LASTTHERMAL);
	DoAlternates(Basic,Calculated,RESWP_TEAMMATE);
	DoAlternates(Basic,Calculated,RESWP_FLARMTARGET);
	DoAlternates(Basic,Calculated,HomeWaypoint); 	
	if (DoOptimizeRoute()) DoAlternates(Basic,Calculated,RESWP_OPTIMIZED); 	
	for (int i=RESWP_FIRST_MARKER; i<=RESWP_LAST_MARKER; i++) {
		if (WayPointList[i].Latitude==RESWP_INVALIDNUMBER) continue;
		DoAlternates(Basic,Calculated,i);
	}
  }

  if (!TargetDialogOpen) {
    // don't calculate these if optimise function being invoked or
    // target is being adjusted
    TaskStatistics(Basic, Calculated, MACCREADY);
    AATStats(Basic, Calculated);  
    TaskSpeed(Basic, Calculated, MACCREADY);
  }

  if (!FlightTimes(Basic, Calculated)) {
    // time hasn't advanced, so don't do calculations requiring an advance
    // or movement
    return FALSE;
  }

  Turning(Basic, Calculated);
  LD(Basic,Calculated);
  CruiseLD(Basic,Calculated);

  // We calculate flaps settings only if the polar is extended.
  // We do assume that GA planes will NOT use extended polars
  if (GlidePolar::FlapsPosCount >0) Flaps(Basic,Calculated);

  Calculated->AverageLD=CalculateLDRotary(&rotaryLD,Calculated); 
  Average30s(Basic,Calculated);
  AverageThermal(Basic,Calculated);
  AverageClimbRate(Basic,Calculated);
  ThermalGain(Basic,Calculated);
  LastThermalStats(Basic, Calculated);
  //  ThermalBand(Basic, Calculated); moved to % circling function
  MaxHeightGain(Basic,Calculated);

  PredictNextPosition(Basic, Calculated);

  if (Orbiter) CalculateOrbiter(Basic,Calculated);

  CalculateOwnTeamCode(Basic, Calculated);
  CalculateTeammateBearingRange(Basic, Calculated);

  BallastDump();

  // reminder: Paragliders have AAT always on
  CalculateOptimizedTargetPos(Basic,Calculated);

  if (ValidTaskPoint(ActiveWayPoint)) {
	// only if running a real task
	if (ValidTaskPoint(1)) InSector(Basic, Calculated);
	DoAutoMacCready(Basic, Calculated);
	IterateEffectiveMacCready(Basic, Calculated);
	#ifdef DEBUGTASKSPEED
	DebugTaskCalculations(Basic, Calculated);
	#endif
  } else { // 101002
	DoAutoMacCready(Basic, Calculated); // will set only EqMC 
  }

  DoAlternates(Basic, Calculated,Alternate1); 
  DoAlternates(Basic, Calculated,Alternate2); 
  DoAlternates(Basic, Calculated,BestAlternate); 

  // Calculate nearest landing when needed
  if ( !MapWindow::mode.AnyPan() && DrawBottom && (MapSpaceMode>MSM_MAP) ) {
	switch(MapSpaceMode) {
		case MSM_LANDABLE:
		case MSM_AIRPORTS:
		case MSM_NEARTPS: // 101222
			DoNearest(Basic,Calculated);
			break;
		case MSM_COMMON:
			DoCommon(Basic,Calculated);
			break;
		case MSM_RECENT:
			DoRecent(Basic,Calculated);
			break;
	}
  }

  CalculateHeadWind(Basic,Calculated);

  ConditionMonitorsUpdate(Basic, Calculated);

  return TRUE;
}

//
// Simply returns gps or baro altitude, and total energy in use within LK
// Using total energy means adding a speed energy calculated here as 
// EnergyHeight to the arrival altitudes. It won't change glide ratios.
// As of 110913 this is totally experimental.
//
void EnergyHeightNavAltitude(NMEA_INFO *Basic, DERIVED_INFO *Calculated) 
{

  // Determine which altitude to use for nav functions
  if (EnableNavBaroAltitude && Basic->BaroAltitudeAvailable) {
    Calculated->NavAltitude = Basic->BaroAltitude;
  } else {
    Calculated->NavAltitude = Basic->Altitude;
  }

  if (UseTotalEnergy) {
	double ias_to_tas;
	double V_tas, wastefactor;

	if (Basic->AirspeedAvailable && (Basic->IndicatedAirspeed>0)) {
		ias_to_tas = Basic->TrueAirspeed/Basic->IndicatedAirspeed;
		V_tas = Basic->TrueAirspeed;
		wastefactor=0.80;
	} else {
		ias_to_tas = 1.0;
		V_tas = Calculated->TrueAirspeedEstimated;
		wastefactor=0.70;
	}
	double V_min_tas = ( GlidePolar::Vminsink + (GlidePolar::Vbestld - GlidePolar::Vminsink)/2)*ias_to_tas;
	V_tas = max(V_tas, V_min_tas);

	Calculated->EnergyHeight = ( (V_tas*V_tas-V_min_tas*V_min_tas)/(9.81*2.0)*wastefactor);
  } else 
	Calculated->EnergyHeight = 0;

}


// It is called GPSVario but it is really a vario using best altitude available.. baro if possible
void Vario(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static double LastTime = 0;
  static double LastAlt = 0;
  double myTime;

  myTime=Basic->Time; 

  if(myTime <= LastTime) {
    LastTime = myTime;
    LastAlt = Calculated->NavAltitude;
  } else {
    double Gain = Calculated->NavAltitude - LastAlt;
    double dT = (Basic->Time - LastTime);
    Calculated->GPSVario = Gain / dT;
    LastAlt = Calculated->NavAltitude;
    LastTime = myTime;
  }

  if (!Basic->VarioAvailable || ReplayLogger::IsEnabled()) {
    Calculated->Vario = Calculated->GPSVario;
  } else {
    // get value from instrument
    Calculated->Vario = Basic->Vario;
  }
}

#ifdef NEWCLIMBAV
ClimbAverageCalculator climbAverageCalculator;
void Average30s(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
	Calculated->Average30s = climbAverageCalculator.GetAverage(Basic->Time, Calculated->NavAltitude, 30);	
	Calculated->NettoAverage30s = Calculated->Average30s;
}

#endif

void Average30s(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static double LastTime = 0;
  static double Altitude[30];
  static double Vario[30];
  static double NettoVario[30];
  int Elapsed, i;
  long index = 0; 
  double Gain;
  static int num_samples = 0;
  static BOOL lastCircling = false;

  if (DoInit[MDI_AVERAGE30S]) {
	LastTime = 0;
	num_samples = 0;
	lastCircling = false;
	DoInit[MDI_AVERAGE30S]=false;
  }

  if(Basic->Time > LastTime)
    {

      if (Calculated->Circling != lastCircling) {
        num_samples = 0;
        // reset!
      }
      lastCircling = Calculated->Circling;

      Elapsed = (int)(Basic->Time - LastTime);
      for(i=0;i<Elapsed;i++)
        {
          index = (long)LastTime + i;
          index %= 30;

          Altitude[index] = Calculated->NavAltitude;
	  if (Basic->NettoVarioAvailable) {
	    NettoVario[index] = Basic->NettoVario;
	  } else {
	    NettoVario[index] = Calculated->NettoVario;
	  }
	  if (Basic->VarioAvailable) {
	    Vario[index] = Basic->Vario;
	  } else {
	    Vario[index] = Calculated->Vario;
	  }

          if (num_samples<30) {
            num_samples ++;
          }

        }

      double Vave = 0;
      double NVave = 0;
      int j;
      for (i=0; i< num_samples; i++) {
        j = (index - i) % 30;
        if (j<0) { 
          j += 30;
        }
        Vave += Vario[j];
	NVave += NettoVario[j];
      }
      if (num_samples) {
        Vave /= num_samples;
        NVave /= num_samples;
      }

      if (!Basic->VarioAvailable) {
        index = ((long)Basic->Time - 1)%30;
        Gain = Altitude[index];
        
        index = ((long)Basic->Time)%30;
        Gain = Gain - Altitude[index];

        Vave = Gain/30;
      }
      Calculated->Average30s = 
        LowPassFilter(Calculated->Average30s,Vave,0.8);
      Calculated->NettoAverage30s = 
        LowPassFilter(Calculated->NettoAverage30s,NVave,0.8);

    }
  else
    {
      if (Basic->Time<LastTime) {
	// gone back in time
	for (i=0; i<30; i++) {
	  Altitude[i]= 0;
	  Vario[i]=0;
	  NettoVario[i]=0;
	}
      }
    }
  LastTime = Basic->Time;
}

void AverageThermal(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if (Calculated->ClimbStartTime>=0) {
    if(Basic->Time > Calculated->ClimbStartTime)
      {
        double Gain = 
          Calculated->NavAltitude+Calculated->EnergyHeight 
            - Calculated->ClimbStartAlt;
        Calculated->AverageThermal  = 
          Gain / (Basic->Time - Calculated->ClimbStartTime);
      }
  }
}

void MaxHeightGain(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if (!Calculated->Flying) return;
  if (!Calculated->FreeFlying && (ISGLIDER||ISPARAGLIDER)) return;

  if (Calculated->MinAltitude>0) {
    double height_gain = Calculated->NavAltitude - Calculated->MinAltitude;
    Calculated->MaxHeightGain = max(height_gain, Calculated->MaxHeightGain);
  } else {
    Calculated->MinAltitude = Calculated->NavAltitude;
  }
  Calculated->MinAltitude = min(Calculated->NavAltitude, Calculated->MinAltitude);
  Calculated->MaxAltitude = max(Calculated->NavAltitude, Calculated->MaxAltitude);
}


void ThermalGain(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if (Calculated->ClimbStartTime>=0) {
    if(Basic->Time >= Calculated->ClimbStartTime)
      {
        Calculated->ThermalGain = 
          Calculated->NavAltitude + Calculated->EnergyHeight 
          - Calculated->ClimbStartAlt;
      }
  }
}


double LimitLD(double LD) {
  if (fabs(LD)>INVALID_GR) {
    return INVALID_GR;
  } else {
    if ((LD>=0.0)&&(LD<1.0)) {
      LD= 1.0;
    } 
    if ((LD<0.0)&&(LD>-1.0)) {
      LD= -1.0;
    }
    return LD;
  }
}


double UpdateLD(double LD, double d, double h, double filter_factor) {
  double glideangle;
  if (LD != 0) {
    glideangle = 1.0/LD;
  } else {
    glideangle = 1.0;
  }
  if (d!=0) {
    glideangle = LowPassFilter(1.0/LD, h/d, filter_factor);
    if (fabs(glideangle) > 1.0/INVALID_GR) {
      LD = LimitLD(1.0/glideangle);
    } else {
      LD = INVALID_GR;
    }
  }
  return LD;
}


void LD(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static double LastLat = 0;
  static double LastLon = 0;
  static double LastTime = 0;
  static double LastAlt = 0;

  if (Basic->Time<LastTime) {
    LastTime = Basic->Time;
    Calculated->LDvario = INVALID_GR;
    Calculated->LD = INVALID_GR;
  } 
  if(Basic->Time >= LastTime+1.0)
    {
      double DistanceFlown;
      DistanceBearing(Basic->Latitude, Basic->Longitude, 
                      LastLat, LastLon,
                      &DistanceFlown, NULL);

      Calculated->LD = UpdateLD(Calculated->LD,
                                DistanceFlown,
                                LastAlt - Calculated->NavAltitude, 0.1);

      InsertLDRotary(&rotaryLD,(int)DistanceFlown, (int)Calculated->NavAltitude);
      InsertWindRotary(&rotaryWind, GPS_INFO.Speed,GPS_INFO.TrackBearing, Calculated->NavAltitude); // 100103
      if (DistanceFlown >3 && DistanceFlown<300) Calculated->Odometer += DistanceFlown;

      LastLat = Basic->Latitude;
      LastLon = Basic->Longitude;
      LastAlt = Calculated->NavAltitude;
      LastTime = Basic->Time;
    }

  // LD instantaneous from vario, updated every reading..
  if (Basic->VarioAvailable && Basic->AirspeedAvailable 
      && Calculated->Flying) {
    Calculated->LDvario = UpdateLD(Calculated->LDvario,
                                   Basic->IndicatedAirspeed,
                                   -Basic->Vario,
                                   0.3);
  } else {
    Calculated->LDvario = INVALID_GR;
  }
}

void Flaps(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{	
	double speed = 0.0;
	if (GlidePolar::FlapsMass<=0) return; // avoid division by zero crashes
	if (Basic->AirspeedAvailable) {
		speed = (int)(Basic->IndicatedAirspeed);
	} else {
		speed = (int)(Calculated->IndicatedAirspeedEstimated);
	}

	double massCorrectionFactor = sqrt(GlidePolar::GetAUW()/GlidePolar::FlapsMass);

	for (int i=0;i<GlidePolar::FlapsPosCount-1;i++) {
		if (speed >= GlidePolar::FlapsPos[i]*massCorrectionFactor 
			&& speed < GlidePolar::FlapsPos[i+1]*massCorrectionFactor) {
			wcscpy(Calculated->Flaps,GlidePolar::FlapsName[i]);
		}
	}	
}

void CruiseLD(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{

  if(!Calculated->Circling)
    {
      double DistanceFlown;

      if (Calculated->CruiseStartTime<0) {
        Calculated->CruiseStartLat = Basic->Latitude;
        Calculated->CruiseStartLong = Basic->Longitude;
        Calculated->CruiseStartAlt = Calculated->NavAltitude;
        Calculated->CruiseStartTime = Basic->Time;
      } else {

        DistanceBearing(Basic->Latitude, Basic->Longitude, 
                        Calculated->CruiseStartLat, 
                        Calculated->CruiseStartLong, &DistanceFlown, NULL);
        Calculated->CruiseLD = 
          UpdateLD(Calculated->CruiseLD,
                   DistanceFlown,
                   Calculated->CruiseStartAlt - Calculated->NavAltitude,
                   0.5);
      }
    }
}




static void ThermalSources(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  double ground_longitude;
  double ground_latitude;
  double ground_altitude;
  thermallocator.
    EstimateThermalBase(
			Calculated->ThermalEstimate_Longitude,
			Calculated->ThermalEstimate_Latitude,
			Calculated->NavAltitude,
			Calculated->LastThermalAverage,
			Calculated->WindSpeed, 
			Calculated->WindBearing,
			&ground_longitude,
			&ground_latitude,
			&ground_altitude
			);
  
  if (ground_altitude>0) {
    double tbest=0;
    int ibest=0;

    for (int i=0; i<MAX_THERMAL_SOURCES; i++) {
      if (Calculated->ThermalSources[i].LiftRate<0.0) {
	ibest = i;
	break;
      }
      double dt = Basic->Time - Calculated->ThermalSources[i].Time;
      if (dt> tbest) {
	tbest = dt;
	ibest = i;
      }
    }
    Calculated->ThermalSources[ibest].LiftRate = 
      Calculated->LastThermalAverage;
    Calculated->ThermalSources[ibest].Latitude = ground_latitude;
    Calculated->ThermalSources[ibest].Longitude = ground_longitude;
    Calculated->ThermalSources[ibest].GroundHeight = ground_altitude;
    Calculated->ThermalSources[ibest].Time = Basic->Time;
  }
}


static void LastThermalStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static int LastCircling = FALSE;

  if((Calculated->Circling == FALSE) && (LastCircling == TRUE)
     && (Calculated->ClimbStartTime>=0))
    {
      double ThermalTime = Calculated->CruiseStartTime 
        - Calculated->ClimbStartTime;
                                      
      if(ThermalTime >0)
        {
          double ThermalGain = Calculated->CruiseStartAlt + Calculated->EnergyHeight
            - Calculated->ClimbStartAlt;

          if (ThermalGain>0) {
            if (ThermalTime>THERMAL_TIME_MIN) {

	      Calculated->LastThermalAverage = ThermalGain/ThermalTime;
	      Calculated->LastThermalGain = ThermalGain;
	      Calculated->LastThermalTime = ThermalTime;

              flightstats.ThermalAverage.
                least_squares_update(Calculated->LastThermalAverage);

#ifdef DEBUG_STATS
              DebugStore("%f %f # thermal stats\n",
                      flightstats.ThermalAverage.m,
                      flightstats.ThermalAverage.b
                      );
#endif
              if (EnableThermalLocator) {
                ThermalSources(Basic, Calculated);
              }
            }
	  }
	}
    }
  LastCircling = Calculated->Circling;
}


double AATCloseBearing(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // ensure waypoint goes in direction of track if very close
  double course_bearing;
  DistanceBearing(Task[ActiveWayPoint-1].AATTargetLat,
		  Task[ActiveWayPoint-1].AATTargetLon,
		  Basic->Latitude,
		  Basic->Longitude,
		  NULL, &course_bearing);
  
  course_bearing = AngleLimit360(course_bearing+
				 Task[ActiveWayPoint].AATTargetOffsetRadial);
  return course_bearing;
}

void DistanceToNext(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  //  LockFlightData();
  LockTaskData();

  if(ValidTaskPoint(ActiveWayPoint))
    {
      double w1lat, w1lon;
      double w0lat, w0lon;

      w0lat = WayPointList[TASKINDEX].Latitude; 
      w0lon = WayPointList[TASKINDEX].Longitude;
      DistanceBearing(Basic->Latitude, Basic->Longitude,
                      w0lat, w0lon,
                      &Calculated->WaypointDistance,
                      &Calculated->WaypointBearing);

      Calculated->ZoomDistance = Calculated->WaypointDistance;

      if (AATEnabled
	  && (ActiveWayPoint>0) && 
          ValidTaskPoint(ActiveWayPoint+1)) {

        w1lat = Task[ActiveWayPoint].AATTargetLat;
        w1lon = Task[ActiveWayPoint].AATTargetLon;

        DistanceBearing(Basic->Latitude, Basic->Longitude,
                        w1lat, w1lon,
                        &Calculated->WaypointDistance,
                        &Calculated->WaypointBearing);

        if (Calculated->WaypointDistance>AATCloseDistance()*3.0) {
          Calculated->ZoomDistance = max(Calculated->WaypointDistance,
                                         Calculated->ZoomDistance);
        } else {
	  Calculated->WaypointBearing = AATCloseBearing(Basic, Calculated);
        }

      } else if ((ActiveWayPoint==0) && (ValidTaskPoint(ActiveWayPoint+1))
                 && (Calculated->IsInSector) ) {

        // JMW set waypoint bearing to start direction if in start sector

        if (AATEnabled) {
          w1lat = Task[ActiveWayPoint+1].AATTargetLat;
          w1lon = Task[ActiveWayPoint+1].AATTargetLon;
        } else {
          w1lat = WayPointList[Task[ActiveWayPoint+1].Index].Latitude; 
          w1lon = WayPointList[Task[ActiveWayPoint+1].Index].Longitude;
        }

        DistanceBearing(Basic->Latitude, Basic->Longitude,
                        w1lat, w1lon,
                        NULL,
                        &Calculated->WaypointBearing);
      }
    }
  else
    {
      Calculated->ZoomDistance = 0;
      Calculated->WaypointDistance = 0;
      Calculated->WaypointBearing = 0;
    }
  UnlockTaskData();
  //  UnlockFlightData();
}

// Current Waypoint calculations for task (no safety?) called only once at beginning
// of DoCalculations, using MACCREADY
void AltitudeRequired(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                      const double this_maccready)
{
  //  LockFlightData();
  (void)Basic;
  LockTaskData();
  if(ValidTaskPoint(ActiveWayPoint))
    {
 	int index;
      double wp_alt = FAIFinishHeight(Basic, Calculated, ActiveWayPoint); 
      double height_above_wp = Calculated->NavAltitude + Calculated->EnergyHeight - wp_alt;

      Calculated->NextAltitudeRequired = GlidePolar::MacCreadyAltitude(this_maccready,
                        Calculated->WaypointDistance,
                        Calculated->WaypointBearing, 
                        Calculated->WindSpeed, Calculated->WindBearing, 
                        0, 0, 
			true,
			NULL, height_above_wp, CRUISE_EFFICIENCY
                        );

	if (this_maccready==0 ) Calculated->NextAltitudeRequired0=Calculated->NextAltitudeRequired;
        else
	      Calculated->NextAltitudeRequired0 = GlidePolar::MacCreadyAltitude(0,
				Calculated->WaypointDistance,
				Calculated->WaypointBearing, 
				Calculated->WindSpeed, Calculated->WindBearing, 
				0, 0, 
				true,
				NULL, height_above_wp, CRUISE_EFFICIENCY
				);


      Calculated->NextAltitudeRequired += wp_alt;
      Calculated->NextAltitudeRequired0 += wp_alt; // VENTA6

      Calculated->NextAltitudeDifference = Calculated->NavAltitude + Calculated->EnergyHeight - Calculated->NextAltitudeRequired;
      Calculated->NextAltitudeDifference0 = Calculated->NavAltitude + Calculated->EnergyHeight - Calculated->NextAltitudeRequired0;

	// We set values only for current destination active waypoint.
	index=TASKINDEX;
	WayPointCalc[index].AltArriv[ALTA_MC]=1111.0;
	WayPointCalc[index].AltArriv[ALTA_SMC]=2222.0; // FIX 091012
	WayPointCalc[index].AltArriv[ALTA_MC0]=3333.0;
	WayPointCalc[index].AltArriv[ALTA_AVEFF]=1234.0;

    }
  else
    {
      Calculated->NextAltitudeRequired = 0;
      Calculated->NextAltitudeDifference = 0;
      Calculated->NextAltitudeDifference0 = 0; // VENTA6 
    }
  UnlockTaskData();
  //  UnlockFlightData();
}


bool InTurnSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const int the_turnpoint)
{
  double AircraftBearing;

  if (!ValidTaskPoint(the_turnpoint)) return false;

  if(SectorType==0)
    {
      if(Calculated->WaypointDistance < SectorRadius)
        {
          return true;
        }
    }
  if (SectorType>0)
    {
      LockTaskData();
      DistanceBearing(WayPointList[Task[the_turnpoint].Index].Latitude,   
                      WayPointList[Task[the_turnpoint].Index].Longitude,
                      Basic->Latitude , 
                      Basic->Longitude,
                      NULL, &AircraftBearing);
      UnlockTaskData();
      
      AircraftBearing = AircraftBearing - Task[the_turnpoint].Bisector ;
      while (AircraftBearing<-180) {
        AircraftBearing+= 360;
      }
      while (AircraftBearing>180) {
        AircraftBearing-= 360;
      }

      if (SectorType==2) {
        // JMW added german rules
        if (Calculated->WaypointDistance<500) {
          return true;
        }
      }
      if( (AircraftBearing >= -45) && (AircraftBearing <= 45))
        {
          if (SectorType==1) {
            if(Calculated->WaypointDistance < SectorRadius)
              {
                return true;
              }
          } else {
            // JMW added german rules
            if(Calculated->WaypointDistance < 10000)
              {
                return true;
              }
          }
        }
    }       
  return false;
}

bool InAATTurnSector(const double longitude, const double latitude,
                    const int the_turnpoint)
{
  double AircraftBearing;
  bool retval = false;

  if (!ValidTaskPoint(the_turnpoint)) {
    return false;
  }

  double distance;
  LockTaskData();
  DistanceBearing(WayPointList[Task[the_turnpoint].Index].Latitude,
                  WayPointList[Task[the_turnpoint].Index].Longitude,
                  latitude,
                  longitude,
                  &distance, &AircraftBearing);

  if(Task[the_turnpoint].AATType ==  CIRCLE) {
    if(distance < Task[the_turnpoint].AATCircleRadius) {
      retval = true;
    }
  } else if(distance < Task[the_turnpoint].AATSectorRadius) {
    if (AngleInRange(Task[the_turnpoint].AATStartRadial,
                     Task[the_turnpoint].AATFinishRadial,
                     AngleLimit360(AircraftBearing), true)) {
      retval = true;
    }
  }

  UnlockTaskData();
  return retval;
}

bool ValidFinish(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
 (void)Basic;
  if ( ((FinishMinHeight/1000)>0) && (Calculated->TerrainValid) && (Calculated->AltitudeAGL < (FinishMinHeight/1000))) {
	return false;
  } else {
	return true;
  }
  
}

bool InFinishSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
		    const int i)
{
  static int LastInSector = FALSE;
  double AircraftBearing;
  double FirstPointDistance;
  bool retval = false;

  if (!WayPointList) return FALSE;

  if (!ValidFinish(Basic, Calculated)) return FALSE;

  // Finish invalid
  if (!ValidTaskPoint(i)) return FALSE;

  LockTaskData();

  // distance from aircraft to start point
  DistanceBearing(Basic->Latitude,
                  Basic->Longitude,
                  WayPointList[Task[i].Index].Latitude, 
                  WayPointList[Task[i].Index].Longitude,
                  &FirstPointDistance,
                  &AircraftBearing);
  bool inrange = false;
  inrange = (FirstPointDistance<FinishRadius);
  if (!inrange) {
    LastInSector = false;
  }

  if(!FinishLine) // Start Circle
    {
      retval = inrange;
      goto OnExit;
    }
        
  // Finish line
  AircraftBearing = AngleLimit180(AircraftBearing - Task[i].InBound);

  // JMW bugfix, was Bisector, which is invalid

  bool approaching;
  if(FinishLine==1) { // Finish line 
    approaching = ((AircraftBearing >= -90) && (AircraftBearing <= 90));
  } else {
    // FAI 90 degree
    approaching = !((AircraftBearing >= 135) || (AircraftBearing <= -135));
  }

  if (inrange) {

    if (LastInSector) {
      // previously approaching the finish line
      if (!approaching) {
        // now moving away from finish line
        LastInSector = false;
        retval = TRUE;
        goto OnExit;
      }
    } else {
      if (approaching) {
        // now approaching the finish line
        LastInSector = true;
      }
    }
    
  } else {
    LastInSector = false;
  }
 OnExit:
  UnlockTaskData();
  return retval;
}


/*

  Track 'TaskStarted' in Calculated info, so it can be
  displayed in the task status dialog.

  Must be reset at start of flight.

  For multiple starts, after start has been passed, need
  to set the first waypoint to the start waypoint and
  then recalculate task stats.

*/

bool ValidStartSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated, DWORD Margin) {
  bool valid = true;
  if (StartMaxSpeed!=0) {
    if (Basic->AirspeedAvailable) {
      if ((Basic->IndicatedAirspeed*1000)>(StartMaxSpeed+Margin))
        valid = false;
    } else {
	// StartMaxSpeed is in millimeters per second, and so is Margin
	if ((Basic->Speed*1000)>(StartMaxSpeed+Margin))  { //@ 101014 FIX
		valid = false;
	}
    }
  }
  return valid;
}

bool ValidStartSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  return ValidStartSpeed(Basic, Calculated, 0);
}

bool InsideStartHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated, DWORD Margin) {
  bool valid = true;
  if ((StartMaxHeight!=0)&&(Calculated->TerrainValid)) {
    if (StartHeightRef == 0) {
      if ((Calculated->AltitudeAGL*1000)>(StartMaxHeight+Margin)) // 101015
	valid = false;
    } else {
      if ((Calculated->NavAltitude*1000)>(StartMaxHeight+Margin)) // 101015
	valid = false;
    }
  }
  return valid;
}

bool InsideStartHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  return InsideStartHeight(Basic, Calculated, 0);
}

// For PGs we are using cylinders, so: 
// If we are inside cylinder return true  else false
bool InStartSector_Internal(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                           int Index, 
                           double OutBound, 
                           bool &LastInSector)
{
  (void)Calculated;
  if (!ValidWayPoint(Index)) return false;

  // No Task Loaded

  double AircraftBearing;
  double FirstPointDistance;

  // distance from aircraft to start point
  DistanceBearing(Basic->Latitude,
                  Basic->Longitude,
                  WayPointList[Index].Latitude, 
                  WayPointList[Index].Longitude,
                  &FirstPointDistance,
                  &AircraftBearing);

  bool inrange = false;
  inrange = (FirstPointDistance<StartRadius);

  if(StartLine==0) { 
	// Start Circle 
	return inrange;
  }
        
  // Start Line
  AircraftBearing = AngleLimit180(AircraftBearing - OutBound);

  // JMW bugfix, was Bisector, which is invalid

  bool approaching;
  if(StartLine==1) { // Start line 
	approaching = ((AircraftBearing >= -90) && (AircraftBearing <= 90));
  } else {
	// FAI 90 degree
	approaching = ((AircraftBearing >= -45) && (AircraftBearing <= 45));
  }

  if (inrange) {
	return approaching;
  } else {
	// cheat fail of last because exited from side
	LastInSector = false;
  }

  return false;
}


static bool InStartSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int &index,
			  BOOL *CrossedStart)
{
  static bool LastInSector = false;
  static int EntryStartSector = index;

  bool isInSector= false;
  bool retval=false;

  if (!Calculated->Flying || !ValidTaskPoint(ActiveWayPoint) || !ValidTaskPoint(0)) 
	return false;

  LockTaskData();

  bool in_height = true;

  if ((ActiveWayPoint>0) && !ValidTaskPoint(ActiveWayPoint+1)) {
	// don't detect start if finish is selected
	retval = false;
	goto OnExit;
  }

  in_height = InsideStartHeight(Basic, Calculated, StartMaxHeightMargin);

  // if waypoint is not the task 0 start wp, and it is valid, then make it the entrystartsector. ?? why ??
  if ((Task[0].Index != EntryStartSector) && (EntryStartSector>=0)) {
	LastInSector = false;
	EntryStartSector = Task[0].Index;
  }

  // are we inside the start sector?
  isInSector = InStartSector_Internal(Basic, Calculated, 
                                      Task[0].Index, Task[0].OutBound,
                                      LastInSector);
  // and within height limits?
  isInSector &= in_height;

  if (ISPARAGLIDER && PGStartOut) { // 100509
  	// we crossed the start if we were outside sector and now we are in.
	*CrossedStart = !LastInSector && isInSector;
  } else {
  	// we crossed the start if we were in sector and now we are not.
	*CrossedStart = LastInSector && !isInSector;
  }

  LastInSector = isInSector;
  if (*CrossedStart) {
	goto OnExit;
  }
  
  if (EnableMultipleStartPoints) {
    for (int i=0; i<MAXSTARTPOINTS; i++) {
      if (StartPoints[i].Active && (StartPoints[i].Index>=0)
          && (StartPoints[i].Index != Task[0].Index)) {
        
        retval = InStartSector_Internal(Basic, Calculated, 
                                        StartPoints[i].Index, 
                                        StartPoints[i].OutBound,
                                        StartPoints[i].InSector);
	retval &= in_height;
        isInSector |= retval;

        index = StartPoints[i].Index;
        *CrossedStart = StartPoints[i].InSector && !retval;
        StartPoints[i].InSector = retval;
        if (*CrossedStart) {
          if (Task[0].Index != index) {
            Task[0].Index = index;
            LastInSector = false;
            EntryStartSector = index;
            RefreshTask();
          }
          goto OnExit;
        }

      }
    }
  }

 OnExit:

  UnlockTaskData();
  return isInSector;
}

#define AUTOADVANCE_MANUAL 0
#define AUTOADVANCE_AUTO 1
#define AUTOADVANCE_ARM 2
#define AUTOADVANCE_ARMSTART 3

bool ReadyToStart(DERIVED_INFO *Calculated) {
  if (!Calculated->Flying) {
    return false;
  }
  if (!ValidGate()) return false; // 100509
  if (AutoAdvance== AUTOADVANCE_AUTO) {  
    return true;
  }
  if ((AutoAdvance== AUTOADVANCE_ARM) || (AutoAdvance==AUTOADVANCE_ARMSTART)) {
    if (AdvanceArmed) {
      return true;
    }
  }
  return false;
}


bool ReadyToAdvance(DERIVED_INFO *Calculated, bool reset=true, bool restart=false) {
  static int lastReady = -1;
  static int lastActive = -1;
  bool say_ready = false;

  // 0: Manual
  // 1: Auto
  // 2: Arm
  // 3: Arm start

  if (!Calculated->Flying) {
    lastReady = -1;
    lastActive = -1;
    return false;
  }

  if (AutoAdvance== AUTOADVANCE_AUTO) {  
    if (reset) AdvanceArmed = false;
    return true;
  }
  if (AutoAdvance== AUTOADVANCE_ARM) {
    if (AdvanceArmed) {
      if (reset) AdvanceArmed = false;
      return true;
    } else {
      say_ready = true;
    }
  }
  if (AutoAdvance== AUTOADVANCE_ARMSTART) { 
    if ((ActiveWayPoint == 0) || restart) {
      if (!AdvanceArmed) {
        say_ready = true;
      } else if (reset) { 
        AdvanceArmed = false; 
        return true;
      }
    } else {
      // JMW fixed 20070528
      if (ActiveWayPoint>0) {
        if (reset) AdvanceArmed = false;
        return true;
      }
    }
  }

  // see if we've gone back a waypoint (e.g. restart)
  if (ActiveWayPoint < lastActive) {
    lastReady = -1;
  }
  lastActive = ActiveWayPoint;

  if (say_ready) {
    if (ActiveWayPoint != lastReady) {
      InputEvents::processGlideComputer(GCE_ARM_READY);
      lastReady = ActiveWayPoint;
    }
  }
  return false;
}

// ALL TIME VALUES ARE IN SECONDS! 
bool UseGates() {
  if (!ISPARAGLIDER ) return(false);
  if (PGNumberOfGates>0) {
	if (ValidTaskPoint(0) && ValidTaskPoint(1)) {
		return(true);
	} else
		return(false);
  } else
	return(false);
}

// Is the gate time open?
bool IsGateOpen() {
   int timenow;
   timenow=LocalTime();

   if ( (timenow>=PGOpenTime) && (timenow<=PGCloseTime))
	return true;
   else
	return false;

}


// Returns the next gate number, 0-x, -1 (negative) if no gates left or time is over
int NextGate() {
  int timenow, gate, gatetime;
  timenow=LocalTime();
  if (timenow>PGCloseTime) {
	#if DEBUGATE
	StartupStore(_T("... Timenow: %d over, gate closed at %d\n"),timenow, PGCloseTime);
	#endif
	return(-1);
  }
  for (gate=0; gate<PGNumberOfGates; gate++) {
	gatetime=PGOpenTime + (gate * PGGateIntervalTime *60);
	if (timenow < gatetime) {
		#if DEBUGATE
		StartupStore(_T("... Timenow: %d Nextgate is n.%d(0-%d) at %d\n"),timenow, gate, PGNumberOfGates-1, gatetime);
		#endif
		return(gate);
	}
  }
  #if DEBUGATE
  StartupStore(_T("... Timenow: %d no NextGate\n"),timenow);
  #endif
  return(-1);
}

// Returns the specified gate time (hours), negative -1 if invalid
int GateTime(int gate) {
  if (gate<0) return(-1);
  int gatetime;
  gatetime=PGOpenTime + (gate * PGGateIntervalTime *60);
  return(gatetime);
}

// Returns the gatetime difference to current local time. Positive if gate is in the future.
int GateTimeDiff(int gate) {
  int timenow, gatetime;
  timenow=LocalTime();
  gatetime=PGOpenTime + (gate * PGGateIntervalTime *60);
  return(gatetime-timenow);
}

// Returns the current open gate number, 0-x, or -1 (negative) if out of time.
// This is NOT the next start! It tells you if a gate is open right now, within time limits.
int RunningGate() {
  int timenow, gate, gatetime;
  timenow=LocalTime();
  if (timenow<PGOpenTime || timenow>PGCloseTime) return(-1);

  // search up to gates+1 ex. 12.40 > 13:00 is end time
  // we are checking the END of the gate, so it is like having a gate+1
  for (gate=1; gate<=PGNumberOfGates; gate++) {
	gatetime=PGOpenTime + (gate * PGGateIntervalTime *60);
	// timenow cannot be lower than gate 0, because gate0 is PGOpenTime
	if (timenow < gatetime) {
  		#if DEBUGATE
 		StartupStore(_T("... Timenow: %d RunningGate n.%d (0-%d)\n"),timenow,gate-1,PGNumberOfGates-1);
  		#endif
		return(gate-1);
	}
  }
  StartupStore(_T("--- RunningGate invalid: timenow=%d Open=%d Close=%d NumGates=%d Interval=%d%s"),
	timenow,PGOpenTime,PGCloseTime,PGNumberOfGates,PGGateIntervalTime,NEWLINE);
  return(-1);
}

// Do we have some gates available, either running right now or in the future?
// Basically mytime <CloseTime...
bool HaveGates() {
  int timenow;
  timenow=LocalTime();
  if (timenow>PGCloseTime)
	return(false);
  else
	return(true);
}

// returns the current gate we are in, either in the past or in the future. 
// It does not matter if it is still valid (it is expired).
// There is ALWAYS an activegate, it cannot be negative!
int InitActiveGate() {
  int timenow;
  timenow=LocalTime();
  if (timenow<PGOpenTime) return(0);
  if (timenow>PGCloseTime) return(PGNumberOfGates-1);
  return(RunningGate());
}

void AlertGateOpen(int gate) {
  TCHAR tag[30];
  if (gate == (PGNumberOfGates-1)) {
	// LKTOKEN  _@M372_ = "LAST GATE IS OPEN" 
	_tcscpy(tag,gettext(TEXT("_@M372_")));
  } else {
	_stprintf(tag,_T("%s %d of %d %s"),
	// LKTOKEN  _@M315_ = "GATE" 
		gettext(TEXT("_@M315_")),
		gate+1, PGNumberOfGates,
	// LKTOKEN  _@M347_ = "IS OPEN" 
		gettext(TEXT("_@M347_")));
  }
  DoStatusMessage(tag);
  if (EnableSoundModes) {
	LKSound(_T("LK_GATEOPEN.WAV"));
  }

}

// Are we on the correct side of start cylinder?
bool CorrectSide() {
  // Remember that IsInSector works reversed...
#if DEBUGTGATES
StartupStore(_T("CorrectSide: PGstartout=%d InSector=%d\n"),PGStartOut,CALCULATED_INFO.IsInSector);
#endif
  if (PGStartOut && CALCULATED_INFO.IsInSector) return false;
  if (!PGStartOut && !CALCULATED_INFO.IsInSector) return false;

  return true;

}

// autonomous check for usegates, and current chosen activegate is open, so a valid start
// is available crossing the start sector..
bool ValidGate() {
  // always ok to start, if no usegates
  if (!UseGates()) return true;
  if (ActiveGate <0 || ActiveGate>=PGNumberOfGates) {
	#if DEBUGTGATES
	StartupStore(_T("... ValidGate false, bad ActiveGate\n"));
	#endif
	return false;
  }
  int timenow;
  timenow=LocalTime();
  if (timenow>PGCloseTime) {
	#if DEBUGTGATES
	StartupStore(_T("... ValidGate false, timenow>PGCloseTime\n"));
	#endif
	return false; // HaveGates
  }
  int timegate;
  timegate=GateTime(ActiveGate);
  if (timegate<1) {
	#if DEBUGTGATES
	StartupStore(_T("... ValidGate false, GateTime returned<0 for ActiveGate=%d\n"),ActiveGate);
	#endif
	return false;
  }
  if ( timenow<timegate ) {
	#if DEBUGTGATES
	StartupStore(_T("... ValidGate false, timenow<timegate for ActiveGate=%d\n"),ActiveGate);
	#endif
	return false;
  }

  #if DEBUGTGATES
  StartupStore(_T("... ValidGate TRUE for ActiveGate=%d\n"),ActiveGate);
  #endif
  return true;
}


// this is called only when ActiveWayPoint is 0, still waiting for start
static void CheckStart(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                       int *LastStartSector) {
  BOOL StartCrossed= false;

  if (UseGates()) {
#if DEBUGATE
StartupStore(_T("... CheckStart Timenow=%d OpenTime=%d CloseTime=%d ActiveGate=%d\n"),LocalTime(),PGOpenTime,PGCloseTime,ActiveGate);
#endif
  	int gatetimediff=-1;
	if ( ActiveGate<0 ) {
		// init activegate: assign first valid gate, current or future
		ActiveGate=InitActiveGate();
		if (ActiveGate<0||ActiveGate>(PGNumberOfGates-1)) {
			FailStore(_T("INVALID ActiveGate=%d"),ActiveGate);
			DoStatusMessage(_T("ERR-430 INVALID ACTIVEGATE: DISABLED"));
			PGNumberOfGates=0;
			return;		
		}
		#if DEBUGATE
		StartupStore(_T("... CheckStart: INIT ActiveGate=%d\n"),ActiveGate);
		#endif
	} else {
		if (HaveGates()) {
			gatetimediff=GateTimeDiff(ActiveGate);
			#if DEBUGATE
			StartupStore(_T("... CheckStart: ActiveGate=%d RunningGate=%d\n"),ActiveGate,RunningGate());
			StartupStore(_T("... CheckStart: gatetimediff=%d\n"),gatetimediff);
			#endif
			// a gate can be in the future , or already open!
			// case: first start, activegate is the first gate
			if (gatetimediff==0) {
				#if DEBUGATE
				StartupStore(_T("... CheckStart: ActiveGate=%d now OPEN\n"),ActiveGate);
				#endif
				AlertGateOpen(ActiveGate);
				// nothing else to do: the current activegate has just open
			} else {
				// check that also non-armed start is detected
				if (ActiveGate<(PGNumberOfGates-1)) {
					if (GateTimeDiff(ActiveGate+1)==0) {
						#if DEBUGATE
						StartupStore(_T("... CheckStart: ActiveGate+1=%d now OPEN\n"),ActiveGate);
						#endif
						ActiveGate++;
						AlertGateOpen(ActiveGate);
					}
				}
			}
			// now check for special alerts on countdown, only on current armed start
			if (gatetimediff==3600 && ((PGGateIntervalTime>=70)||ActiveGate==0) ) { 
				//  850  FIRST GATE OPEN IN 1 HOUR
				DoStatusMessage(gettext(TEXT("_@M850_")));
				if (EnableSoundModes) {
					LKSound(_T("LK_DINGDONG.WAV"));
				}
			}
			if (gatetimediff==1800 && ((PGGateIntervalTime>=45)||ActiveGate==0) ) { 
				//  851  FIRST GATE OPEN IN 30 MINUTES
				DoStatusMessage(gettext(TEXT("_@M851_")));
				if (EnableSoundModes) {
					LKSound(_T("LK_DINGDONG.WAV"));
				}
			}
			if (gatetimediff==600 && ((PGGateIntervalTime>=15)||ActiveGate==0) ) { // 10 minutes to go
				//  852  10 MINUTES TO GO
				DoStatusMessage(gettext(TEXT("_@M852_")));
				if (EnableSoundModes) {
					LKSound(_T("LK_HITONE.WAV"));
				}
			}
			if (gatetimediff==300 && ((PGGateIntervalTime>=10)||ActiveGate==0)) { // 5 minutes to go
				//  853  5 MINUTES TO GO
				DoStatusMessage(gettext(TEXT("_@M853_")));
				if (EnableSoundModes) {
					LKSound(_T("LK_HITONE.WAV"));
				}
			}
			if (gatetimediff==60) { // 1 minute to go
				if (EnableSoundModes) {
					LKSound(_T("LK_3HITONES.WAV"));
				}
			}

		} // HaveGates
	} // not init

  }

  if (ISPARAGLIDER && PGStartOut) {
	// start OUT and go in
	if (!InStartSector(Basic,Calculated,*LastStartSector, &StartCrossed)) {
		Calculated->IsInSector = false;

		if (ReadyToStart(Calculated)) {
			aatdistance.AddPoint(Basic->Longitude, Basic->Latitude, 0);
		}
		if (ValidStartSpeed(Basic, Calculated, StartMaxSpeedMargin)) {
			ReadyToAdvance(Calculated, false, true);
		}
	} else
		Calculated->IsInSector = true;
  } else {
	// start IN and go out, OLD CLASSIC MODE
	if (InStartSector(Basic,Calculated,*LastStartSector, &StartCrossed)) {
		// InSector check calling this function is resetting IsInSector at each run, so it was false.
		Calculated->IsInSector = true;

		if (ReadyToStart(Calculated)) {
			aatdistance.AddPoint(Basic->Longitude, Basic->Latitude, 0);
		}
    		// ToLo: we are ready to start even when outside start rules but within margin
		if (ValidStartSpeed(Basic, Calculated, StartMaxSpeedMargin)) {
			ReadyToAdvance(Calculated, false, true);
		}
    		// TODO accuracy: monitor start speed throughout time in start sector
  	}
  } // end start mode

  if (StartCrossed && ValidGate() ) {  // 100509

	#if DEBUGTGATES
	StartupStore(_T("... CheckStart: start crossed and valid gate!\n"));
	#endif
	
    // ToLo: Check weather speed and height are within the rules or not (zero margin)
    if(!IsFinalWaypoint() && ValidStartSpeed(Basic, Calculated) && InsideStartHeight(Basic, Calculated)) {

      // This is set whether ready to advance or not, because it will
      // appear in the flight log, so if it's valid, it's valid.
      Calculated->ValidStart = true;

      if (ReadyToAdvance(Calculated, true, true)) {
        ActiveWayPoint=0; // enforce this since it may be 1
        StartTask(Basic,Calculated, true, true);
      }
      if (Calculated->Flying) {
        Calculated->ValidFinish = false;
      }
      // JMW TODO accuracy: This causes Vaverage to go bonkers
      // if the user has already passed the start
      // but selects the start
      
      // Note: pilot must have armed advance
      // for the start to be registered

    // ToLo: If speed and height are outside the rules they must be within the margin...
    } else {
    
      if ((ActiveWayPoint<=1) 
          && !IsFinalWaypoint()
          && (Calculated->ValidStart==false)
          && (Calculated->Flying)) {
        
	#if 0
	// 101014 This is called from wrong thread, and cause bad crashes
	// moved to new GCE event inside InputEvents - paolo
        // need to detect bad starts, just to get the statistics
        // in case the bad start is the best available, or the user
        // manually started
        StartTask(Basic, Calculated, false, false);
//        Calculated->ValidStart = false;
        bool startTaskAnyway = false;
        if (ReadyToAdvance(Calculated, true, true)) {
          dlgStartTaskShowModal(&startTaskAnyway,
                                Calculated->TaskStartTime,
                                Calculated->TaskStartSpeed,
                                Calculated->TaskStartAltitude);
          if (startTaskAnyway) {
            ActiveWayPoint=0; // enforce this since it may be 1
            StartTask(Basic,Calculated, true, true);
          }
        }
        Calculated->ValidStart = startTaskAnyway;
	#else // 101014
        StartTask(Basic, Calculated, false, false);
        if (ReadyToAdvance(Calculated, true, true)) {
		InputEvents::processGlideComputer(GCE_TASK_CONFIRMSTART);
	}
	#endif
        
        if (Calculated->Flying) {
		Calculated->ValidFinish = false;
        }
      }

    }
  }
}


static BOOL CheckRestart(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                         int *LastStartSector) {
  if((Basic->Time - Calculated->TaskStartTime < 3600)
     &&(ActiveWayPoint<=1)) {

    CheckStart(Basic, Calculated, LastStartSector);
  }
  return FALSE;
}


static void CheckFinish(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  if (InFinishSector(Basic,Calculated, ActiveWayPoint)) {
    Calculated->IsInSector = true;
    aatdistance.AddPoint(Basic->Longitude,
                         Basic->Latitude,
                         ActiveWayPoint);
    if (!Calculated->ValidFinish) {
      Calculated->ValidFinish = true;
      AnnounceWayPointSwitch(Calculated, false);

      // JMWX save calculated data at finish
      memcpy(&Finish_Derived_Info, Calculated, sizeof(DERIVED_INFO));
    }
  }
}


static void AddAATPoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                        int taskwaypoint) {
  bool insector = false;
  if (taskwaypoint>0) {
    if (AATEnabled) {
      insector = InAATTurnSector(Basic->Longitude,
                                 Basic->Latitude, taskwaypoint);
    } else {
      insector = InTurnSector(Basic, Calculated, taskwaypoint);
    }
    if(insector) {
      if (taskwaypoint == ActiveWayPoint) {
        Calculated->IsInSector = true;
      }
      aatdistance.AddPoint(Basic->Longitude,
                           Basic->Latitude,
                           taskwaypoint);
    }
  }
}


static void CheckInSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  if (ActiveWayPoint>0) {
    AddAATPoint(Basic, Calculated, ActiveWayPoint-1);
  }
  AddAATPoint(Basic, Calculated, ActiveWayPoint);

  // JMW Start bug XXX

  if (aatdistance.HasEntered(ActiveWayPoint)) {
    if (ReadyToAdvance(Calculated, true, false)) {
      AnnounceWayPointSwitch(Calculated, true);
      Calculated->LegStartTime = Basic->Time;
      flightstats.LegStartTime[ActiveWayPoint] = Basic->Time;
    }
    if (Calculated->Flying) {
      Calculated->ValidFinish = false;
    }
  }
}

// This is called from main DoCalculations each time, only when running a real task
void InSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static int LastStartSector = -1;

  if (ActiveWayPoint<0) return;
  LockTaskData();

// Paragliders task system
// Case A: start entering the sector/cylinder
//		you must be outside sector when gate is open.
//		you are warned that you are already inside sector before the gate is open, when gate is opening in <10 minutes
//		task restart is manual
// Case B: start exiting the sector

  // by default, we are not in the sector
  Calculated->IsInSector = false;

  if(ActiveWayPoint == 0) {
	CheckStart(Basic, Calculated, &LastStartSector);
  } else {
	if(IsFinalWaypoint()) {
		LastStartSector = -1;
		AddAATPoint(Basic, Calculated, ActiveWayPoint-1);
		CheckFinish(Basic, Calculated);
	} else {
		if (!UseGates()) CheckRestart(Basic, Calculated, &LastStartSector); // 100507
		if (ActiveWayPoint>0) {
			CheckInSector(Basic, Calculated);
			LastStartSector = -1;
		}
	}
  }                   
  UnlockTaskData();
}


static void TerrainHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  short Alt = 0;

  RasterTerrain::Lock();
  // want most accurate rounding here
  RasterTerrain::SetTerrainRounding(0,0);
  Alt = RasterTerrain::GetTerrainHeight(Basic->Latitude, 
                                        Basic->Longitude);
  RasterTerrain::Unlock();

  if(Alt!=TERRAIN_INVALID) { // terrain invalid is now positive  ex. 32767
	Calculated->TerrainValid = true;
	if (Alt>=0) {
		Calculated->TerrainAlt = Alt;
	} else {
		// this can be still a problem for dutch users.. Todo Fix
		Calculated->TerrainAlt = 0;
	}
  } else {
	Calculated->TerrainValid = false; 
	Calculated->TerrainAlt = 0;
  }
  Calculated->AltitudeAGL = Calculated->NavAltitude - Calculated->TerrainAlt;
  if (!FinalGlideTerrain) {
	Calculated->TerrainBase = Calculated->TerrainAlt;
  }
}



static bool TaskAltitudeRequired(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                                 double this_maccready, double *Vfinal,
                                 double *TotalTime, double *TotalDistance,
                                 int *ifinal)
{
  int i;
  double w1lat;
  double w1lon;
  double w0lat;
  double w0lon;
  double LegTime, LegDistance, LegBearing, LegAltitude;
  bool retval = false;

  // Calculate altitude required from start of task

  bool isfinal=true;
  LegAltitude = 0;
  double TotalAltitude = 0;
  *TotalTime = 0; *TotalDistance = 0;
  *ifinal = 0;

  LockTaskData();

  double height_above_finish = FAIFinishHeight(Basic, Calculated, 0)-
    FAIFinishHeight(Basic, Calculated, -1);

  for(i=MAXTASKPOINTS-2;i>=0;i--) {


    if (!ValidTaskPoint(i) || !ValidTaskPoint(i+1)) continue;
    
    w1lat = WayPointList[Task[i].Index].Latitude;
    w1lon = WayPointList[Task[i].Index].Longitude;
    w0lat = WayPointList[Task[i+1].Index].Latitude;
    w0lon = WayPointList[Task[i+1].Index].Longitude;
    
    if (AATEnabled) {
      w1lat = Task[i].AATTargetLat;
      w1lon = Task[i].AATTargetLon;
      if (!isfinal) {
        w0lat = Task[i+1].AATTargetLat;
        w0lon = Task[i+1].AATTargetLon;
      }
    }
    
    DistanceBearing(w1lat, w1lon,
                    w0lat, w0lon,
                    &LegDistance, &LegBearing);

    *TotalDistance += LegDistance;
    
    LegAltitude = 
      GlidePolar::MacCreadyAltitude(this_maccready, 
                                    LegDistance, 
                                    LegBearing, 
                                    Calculated->WindSpeed, 
                                    Calculated->WindBearing,
                                    0,
                                    0,
                                    true,
                                    &LegTime,
				    height_above_finish, 
				    CRUISE_EFFICIENCY
                                    );

    // JMW CHECK FGAMT
    height_above_finish-= LegAltitude;

    TotalAltitude += LegAltitude;

    if (LegTime<0) {
		retval = false;
		goto OnExit;
    } else {
      *TotalTime += LegTime;
    }
    if (isfinal) {
      *ifinal = i+1;
      if (LegTime>0) {
        *Vfinal = LegDistance/LegTime;
      }
    }
    isfinal = false;
  }

  if (*ifinal==0) {
    retval = false;
    goto OnExit;
  }

  TotalAltitude += FAIFinishHeight(Basic, Calculated, -1);

  if (!ValidTaskPoint(*ifinal)) {
    Calculated->TaskAltitudeRequiredFromStart = TotalAltitude;
    retval = false;
  } else {
    Calculated->TaskAltitudeRequiredFromStart = TotalAltitude;
    retval = true;
  }
 OnExit:
  UnlockTaskData();
  return retval;
}


double MacCreadyOrAvClimbRate(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                              double this_maccready)
{
  double mc_val = this_maccready;
  bool is_final_glide = false;

  if (Calculated->FinalGlide) {
    is_final_glide = true;
  }

  // when calculating 'achieved' task speed, need to use Mc if
  // not in final glide, or if in final glide mode and using 
  // auto Mc, use the average climb rate achieved so far.

  if ((mc_val<0.1) || 
      (Calculated->AutoMacCready && 
       ((AutoMcMode==amcFinalGlide) ||
        ((AutoMcMode==amcFinalAndClimb)&&(is_final_glide))
        ))
      ) {

    if (flightstats.ThermalAverage.y_ave>0) {
      mc_val = flightstats.ThermalAverage.y_ave;
    } else if (Calculated->AverageThermal>0) {
      // insufficient stats, so use this/last thermal's average
      mc_val = Calculated->AverageThermal;
    }
  }
  return max(0.1, mc_val);

}


void TaskSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double this_maccready)
{
  int ifinal;
  static double LastTime = 0;
  static double LastTimeStats = 0;
  double TotalTime=0, TotalDistance=0, Vfinal=0;

  if (!ValidTaskPoint(ActiveWayPoint)) return;
  if (Calculated->ValidFinish) return;
  if (!Calculated->Flying) return;

  // in case we leave early due to error
  Calculated->TaskSpeedAchieved = 0;
  Calculated->TaskSpeed = 0;

  if (ActiveWayPoint<=0) { // no task speed before start
    Calculated->TaskSpeedInstantaneous = 0;
    return;
  }

  //  LockFlightData();
  LockTaskData();

  if (TaskAltitudeRequired(Basic, Calculated, this_maccready, &Vfinal,
                           &TotalTime, &TotalDistance, &ifinal)) {
      
    double t0 = TotalTime;
    // total time expected for task
    
    double t1 = Basic->Time-Calculated->TaskStartTime;
    // time elapsed since start
    
    double d0 = TotalDistance;
    // total task distance
    
    double d1 = Calculated->TaskDistanceCovered;
    // actual distance covered
    
    double dr = Calculated->TaskDistanceToGo;
    // distance remaining
    
    double hf = FAIFinishHeight(Basic, Calculated, -1);
    
    double h0 = Calculated->TaskAltitudeRequiredFromStart-hf;
    // total height required from start (takes safety arrival alt
    // and finish waypoint altitude into account)
    
    double h1 = max(0.0, Calculated->NavAltitude-hf);
    // height above target

    double dFinal;
    // final glide distance
    
    // equivalent speed
    double v2, v1;

    if ((t1<=0) || (d1<=0) || (d0<=0) || (t0<=0) || (h0<=0)) {
      // haven't started yet or not a real task
      Calculated->TaskSpeedInstantaneous = 0;
      //?      Calculated->TaskSpeed = 0;
      goto OnExit;
    }

    // JB's task speed...
    double hx = max(0.0, SpeedHeight(Basic, Calculated));
    double t1mod = t1-hx/MacCreadyOrAvClimbRate(Basic, Calculated, this_maccready);
    // only valid if flown for 5 minutes or more
    if (t1mod>300.0) {
      Calculated->TaskSpeedAchieved = d1/t1mod;
    } else {
      Calculated->TaskSpeedAchieved = d1/t1;
    }
    Calculated->TaskSpeed = Calculated->TaskSpeedAchieved;

    if (Vfinal<=0) {
      // can't reach target at current mc
      goto OnExit;
    }
    
    // distance that can be usefully final glided from here
    // (assumes average task glide angle of d0/h0)
    // JMW TODO accuracy: make this more accurate by working out final glide
    // through remaining turnpoints.  This will more correctly account
    // for wind.

    dFinal = min(dr, d0*min(1.0,max(0.0,h1/h0)));

    if (Calculated->ValidFinish) {
      dFinal = 0;
    }

    double dc = max(0.0, dr-dFinal); 
    // amount of extra distance to travel in cruise/climb before final glide

    // actual task speed achieved so far
    v1 = d1/t1;
    
#ifdef OLDTASKSPEED  
    // time at end of final glide
    // equivalent time elapsed after final glide
    double t2 = t1+dFinal/Vfinal;
    
    // equivalent distance travelled after final glide
    // equivalent distance to end of final glide
    double d2 = d1+dFinal;
    
    // average speed to end of final glide from here
    v2 = d2/t2;
    Calculated->TaskSpeed = max(v1,v2);
#else
    // average speed to end of final glide from here, weighted
    // according to how much extra time would be spent in cruise/climb
    // the closer dc (the difference between remaining distance and
    // final glidable distance) gets to zero, the closer v2 approaches
    // the average speed to end of final glide from here
    // in other words, the more we consider the final glide part to have
    // been earned.

    // this will be bogus at fast starts though...
    if (v1>0) {
      v2 = (d1+dc+dFinal)/(t1+dc/v1+dFinal/Vfinal);
    } else {
      v2 = (d1+dFinal)/(t1+dFinal/Vfinal);
    }
    Calculated->TaskSpeed = v2;
#endif

    if(Basic->Time < LastTime) {
      LastTime = Basic->Time;
    } else if (Basic->Time-LastTime >=1.0) {

      double dt = Basic->Time-LastTime;
      LastTime = Basic->Time;

      // Calculate contribution to average task speed.
      // This is equal to the change in virtual distance
      // divided by the time step
      
      // This is a novel concept.
      // When climbing at the MC setting, this number should
      // be similar to the estimated task speed.
      // When climbing slowly or when flying off-course,
      // this number will drop.
      // In cruise at the optimum speed in zero lift, this
      // number will be similar to the estimated task speed. 
      
      // A low pass filter is applied so it doesn't jump around
      // too much when circling.
      
      // If this number is higher than the overall task average speed,
      // it means that the task average speed is increasing.
      
      // When cruising in sink, this number will decrease.
      // When cruising in lift, this number will increase.
      
      // Therefore, it shows well whether at any time the glider
      // is wasting time.

      // VNT 090723 NOTICE: all of this is totally crazy. Did anyone ever cared to check
      // what happens with MC=0 ? Did anyone care to tell people how a simple "ETE" or TaskSpeed 
      // has been complicated over any limit?
      // TODO: start back from scratch, not possible to trust any number here.

      static double dr_last = 0;

      double mc_safe = max(0.1,this_maccready);
      double Vstar = max(1.0,Calculated->VMacCready);
      double vthis = (Calculated->LegDistanceCovered-dr_last)/dt;
      vthis /= AirDensityRatio(Calculated->NavAltitude);
      
      dr_last = Calculated->LegDistanceCovered;
      double ttg = max(1.0, Calculated->LegTimeToGo);
      //      double Vav = d0/max(1.0,t0); 
      double Vrem = Calculated->LegDistanceToGo/ttg;
      double Vref = // Vav;
	Vrem;
      double sr = -GlidePolar::SinkRate(Vstar);
      double height_diff = max(0.0, -Calculated->TaskAltitudeDifference);
      
      if (Calculated->timeCircling>30) {
	mc_safe = max(this_maccready, 
		      Calculated->TotalHeightClimb/Calculated->timeCircling);
      }
      // circling percentage during cruise/climb
      double rho_cruise = max(0.0,min(1.0,mc_safe/(sr+mc_safe)));
      double rho_climb = 1.0-rho_cruise;
      double time_climb = height_diff/mc_safe;

      // calculate amount of time in cruise/climb glide
      double rho_c = max(0.0, min(1.0, time_climb/ttg));

      if (Calculated->FinalGlide) {
	if (rho_climb>0) {
	  rho_c = max(0.0, min(1.0, rho_c/rho_climb));
	}
	if (!Calculated->Circling) {
	  if (Calculated->TaskAltitudeDifference>0) {
	    rho_climb *= rho_c;
	    rho_cruise *= rho_c;
	    // Vref = Vrem;
	  }
	}
      }

      double w_comp = min(10.0,max(-10.0,Calculated->Vario/mc_safe));
      double vdiff = vthis/Vstar + w_comp*rho_cruise + rho_climb;

      if (vthis > SAFTEYSPEED*2) {
	vdiff = 1.0;
	// prevent funny numbers when starting mid-track
      }
      //      Calculated->Experimental = vdiff*100.0;

      vdiff *= Vref;
      
      if (t1<5) {
        Calculated->TaskSpeedInstantaneous = vdiff;
        // initialise
      } else {
        static int lastActiveWayPoint = 0;
	static double tsi_av = 0;
	static int n_av = 0;
        if ((ActiveWayPoint==lastActiveWayPoint) 
	    && (Calculated->LegDistanceToGo>1000.0) 
	    && (Calculated->LegDistanceCovered>1000.0)) {
          
          Calculated->TaskSpeedInstantaneous = 
            LowPassFilter(Calculated->TaskSpeedInstantaneous, vdiff, 0.1);
          
          // update stats
          if(Basic->Time < LastTimeStats) {
            LastTimeStats = Basic->Time;
	    tsi_av = 0;
	    n_av = 0;
          } else if (n_av>=60) { 
	    tsi_av/= n_av;
            flightstats.Task_Speed.
              least_squares_update(
                                   max(0.0,
                                       Basic->Time-Calculated->TaskStartTime)/3600.0,
                                   max(0.0, min(100.0,tsi_av)));
            LastTimeStats = Basic->Time;
	    tsi_av = 0;
	    n_av = 0;
          } 
	  tsi_av += Calculated->TaskSpeedInstantaneous;
	  n_av ++;

        } else {

          Calculated->TaskSpeedInstantaneous = 
            LowPassFilter(Calculated->TaskSpeedInstantaneous, vdiff, 0.5);

	  //	  Calculated->TaskSpeedInstantaneous = vdiff;
	  tsi_av = 0;
	  n_av = 0;
	}
        lastActiveWayPoint = ActiveWayPoint;
      }
    }
  }
 OnExit:
  UnlockTaskData();

}

// no need to use LegToGo and LegBearing, we use the active waypoint instead
// calculate also arrival altitude on obstacle

static void CheckGlideThroughTerrain(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  if (ValidNotResWayPoint(TASKINDEX)) { 
	double lat, lon;
	double farlat, farlon;
	double oldfarlat, oldfarlon, oldfardist;
	static double oldfarbearing=361;
	static double oldstartaltitude=-1;
	double startaltitude;
	double distance_soarable;
	bool out_of_range, farout_of_range;
	double fardistance_soarable;
	double minaltitude, maxaltitude;
	double newaltitude;
	int selwp;

	selwp=TASKINDEX;
	if ( WayPointCalc[selwp].AltArriv[AltArrivMode]<0 ) {
		return;
	}

	distance_soarable = 
		FinalGlideThroughTerrain(CALCULATED_INFO.WaypointBearing, Basic, Calculated, &lat, &lon, 
		CALCULATED_INFO.WaypointDistance, &out_of_range, NULL);

	// Calculate obstacles ONLY if we are in glide range, otherwise it is useless 
	if ((!out_of_range)&&(distance_soarable< CALCULATED_INFO.WaypointDistance)) {

		Calculated->TerrainWarningLatitude = lat;
		Calculated->TerrainWarningLongitude = lon;

		Calculated->ObstacleDistance = distance_soarable;

		Calculated->ObstacleHeight =  max((short)0, RasterTerrain::GetTerrainHeight(lat,lon));
		if (Calculated->ObstacleHeight == TERRAIN_INVALID) Calculated->ObstacleHeight=0; //@ 101027 FIX

		// how much height I will loose to get there
		Calculated->ObstacleAltReqd = GlidePolar::MacCreadyAltitude (MACCREADY, 
			distance_soarable, 
			CALCULATED_INFO.WaypointBearing,
			CALCULATED_INFO.WindSpeed, CALCULATED_INFO.WindBearing,
			0, 0, true,0);

		// arrival altitude over the obstacle
		// sometimes it is positive
		Calculated->ObstacleAltArriv = Calculated->NavAltitude
			 - Calculated->ObstacleAltReqd
			 - Calculated->ObstacleHeight
			 - SAFETYALTITUDETERRAIN;

		// Reminder: we already have a glide range on destination.
		minaltitude=CALCULATED_INFO.NavAltitude;
		maxaltitude=minaltitude*2;

		// if no far obstacle will be found, we shall use the first obstacle. 
		oldfarlat=lat;
		oldfarlon=lon;
		oldfardist=distance_soarable;
		if (oldstartaltitude<0) oldstartaltitude=minaltitude;

		// if bearing has changed for more than 1 deg, we dont use shortcuts
		if (fabs(oldfarbearing-CALCULATED_INFO.WaypointBearing) >= 1)  {
			startaltitude=minaltitude;
			oldfarbearing=CALCULATED_INFO.WaypointBearing;
		} else {
			startaltitude=oldstartaltitude-200;
			if (startaltitude <minaltitude) startaltitude=minaltitude;
		}

		// need to recalculate, init with first obstacle, forget old far obstacle
		// new bearing reference

		for ( newaltitude=minaltitude; newaltitude<maxaltitude; newaltitude+=50) {

			fardistance_soarable = FarFinalGlideThroughTerrain( CALCULATED_INFO.WaypointBearing, Basic, Calculated, 
				&farlat, &farlon, CALCULATED_INFO.WaypointDistance, &farout_of_range, newaltitude, NULL);

			if (fardistance_soarable< CALCULATED_INFO.WaypointDistance) {
				oldfarlat=farlat;
				oldfarlon=farlon;
				oldfardist=fardistance_soarable;
			} else break;
		}

		oldstartaltitude=newaltitude;
		Calculated->FarObstacle_Lat = oldfarlat;
		Calculated->FarObstacle_Lon = oldfarlon;
		Calculated->FarObstacle_Dist = oldfardist;
		// 0-50m positive rounding
		Calculated->FarObstacle_AltArriv = minaltitude - newaltitude;


	} else {
		Calculated->TerrainWarningLatitude = 0.0;
		Calculated->TerrainWarningLongitude = 0.0;
	}
  } else {
	Calculated->TerrainWarningLatitude = 0.0;
	Calculated->TerrainWarningLongitude = 0.0;
  }
}


void TaskStatistics(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                    const double this_maccready)
{

  if (!ValidTaskPoint(ActiveWayPoint) || 
      ((ActiveWayPoint>0) && !ValidTaskPoint(ActiveWayPoint-1))) {


    Calculated->LegSpeed = 0;
    Calculated->LegDistanceToGo = 0;
    Calculated->LegDistanceCovered = 0;
    Calculated->LegTimeToGo = 0;

    if (!AATEnabled) {
      Calculated->AATTimeToGo = 0;
    }

    //    Calculated->TaskSpeed = 0;

    Calculated->TaskDistanceToGo = 0;
    Calculated->TaskDistanceCovered = 0;
    Calculated->TaskTimeToGo = 0;
    Calculated->LKTaskETE = 0; 
    Calculated->TaskTimeToGoTurningNow = -1;

    Calculated->TaskAltitudeRequired = 0;
    Calculated->TaskAltitudeDifference = 0;
    Calculated->TaskAltitudeDifference0 = 0;

    Calculated->TerrainWarningLatitude = 0.0;
    Calculated->TerrainWarningLongitude = 0.0;

    Calculated->GRFinish = INVALID_GR;
   

    Calculated->FinalGlide = 0;
    CheckGlideThroughTerrain(Basic, Calculated); // BUGFIX 091123
    
    // no task selected, so work things out at current heading

    GlidePolar::MacCreadyAltitude(this_maccready, 100.0, 
                                  Basic->TrackBearing, 
                                  Calculated->WindSpeed, 
                                  Calculated->WindBearing, 
                                  &(Calculated->BestCruiseTrack),
                                  &(Calculated->VMacCready),
                                  (Calculated->FinalGlide==1),
                                  NULL, 1.0e6, CRUISE_EFFICIENCY);
    return;
  }

  //  LockFlightData();
  LockTaskData();

  // Calculate Task Distances
  // First calculate distances for this waypoint

  double LegCovered, LegToGo=0;
  double LegDistance, LegBearing=0;
  bool calc_turning_now;

  double w1lat;
  double w1lon;
  double w0lat;
  double w0lon;
  
  if (AATEnabled && (ActiveWayPoint>0) && (ValidTaskPoint(ActiveWayPoint+1))) {
    w1lat = Task[ActiveWayPoint].AATTargetLat;
    w1lon = Task[ActiveWayPoint].AATTargetLon;
  } else {
    w1lat = WayPointList[TASKINDEX].Latitude;
    w1lon = WayPointList[TASKINDEX].Longitude;
  }
  
  DistanceBearing(Basic->Latitude, 
                  Basic->Longitude, 
                  w1lat, 
                  w1lon, 
                  &LegToGo, &LegBearing);

  if (AATEnabled && (ActiveWayPoint>0) && ValidTaskPoint(ActiveWayPoint+1)
      && Calculated->IsInSector && (this_maccready>0.1) ) {
    calc_turning_now = true;
  } else {
    calc_turning_now = false;
  }

  if (ActiveWayPoint<1) {
    LegCovered = 0;
    if (ValidTaskPoint(ActiveWayPoint+1)) {  // BUGFIX 091221
      LegToGo=0;
    }
   } else {
    if (AATEnabled) {
      // TODO accuracy: Get best range point to here...
      w0lat = Task[ActiveWayPoint-1].AATTargetLat;
      w0lon = Task[ActiveWayPoint-1].AATTargetLon;
    } else {
      w0lat = WayPointList[Task[ActiveWayPoint-1].Index].Latitude;
      w0lon = WayPointList[Task[ActiveWayPoint-1].Index].Longitude;
    }
    
    DistanceBearing(w1lat, 
                    w1lon,
                    w0lat, 
                    w0lon,
                    &LegDistance, NULL);
    
    LegCovered = ProjectedDistance(w0lon, w0lat,
                                   w1lon, w1lat,
                                   Basic->Longitude,
                                   Basic->Latitude);

    if ((StartLine==0) && (ActiveWayPoint==1)) {
      // Correct speed calculations for radius
      // JMW TODO accuracy: legcovered replace this with more accurate version
      // LegDistance -= StartRadius;
      LegCovered = max(0.0, LegCovered-StartRadius);
    }
  }
  
  Calculated->LegDistanceToGo = LegToGo;
  Calculated->LegDistanceCovered = LegCovered;
  Calculated->TaskDistanceCovered = LegCovered;
  
  if (Basic->Time > Calculated->LegStartTime) {
    if (flightstats.LegStartTime[ActiveWayPoint]<0) {
      flightstats.LegStartTime[ActiveWayPoint] = Basic->Time;
    }
    Calculated->LegSpeed = Calculated->LegDistanceCovered
      / (Basic->Time - Calculated->LegStartTime); 
  }

  // Now add distances for start to previous waypoint
 
    if (!AATEnabled) {
      for(int i=0;i< ActiveWayPoint-1; i++)
        {
          if (!ValidTaskPoint(i) || !ValidTaskPoint(i+1)) continue;
          
          w1lat = WayPointList[Task[i].Index].Latitude;
          w1lon = WayPointList[Task[i].Index].Longitude;
          w0lat = WayPointList[Task[i+1].Index].Latitude;
          w0lon = WayPointList[Task[i+1].Index].Longitude;
          
          DistanceBearing(w1lat, 
                          w1lon,
                          w0lat, 
                          w0lon,
                          &LegDistance, NULL);                      
          Calculated->TaskDistanceCovered += LegDistance;
        }
    } else if (ActiveWayPoint>0) {
      // JMW added correction for distance covered
      Calculated->TaskDistanceCovered = 
        aatdistance.DistanceCovered(Basic->Longitude,
                                    Basic->Latitude,
                                    ActiveWayPoint);
    }

  CheckTransitionFinalGlide(Basic, Calculated);

  // accumulators
  double TaskAltitudeRequired = 0;
  double TaskAltitudeRequired0 = 0;
  Calculated->TaskDistanceToGo = 0;
  Calculated->TaskTimeToGo = 0;
  Calculated->LKTaskETE = 0;
  Calculated->TaskTimeToGoTurningNow = 0;

  double LegTime0;

  // Calculate Final Glide To Finish
  
  int FinalWayPoint = getFinalWaypoint();

  double final_height = FAIFinishHeight(Basic, Calculated, -1);
  double total_energy_height = Calculated->NavAltitude + Calculated->EnergyHeight;
  double height_above_finish = total_energy_height - final_height;
  
  // Now add it for remaining waypoints
  int task_index= FinalWayPoint;

  double StartBestCruiseTrack = -1; 

    while ((task_index>ActiveWayPoint) && (ValidTaskPoint(task_index))) {
      double this_LegTimeToGo;
      bool this_is_final = (task_index==FinalWayPoint)
	|| ForceFinalGlide;

      this_is_final = true; // JMW CHECK FGAMT
      
      if (AATEnabled) {
	w1lat = Task[task_index].AATTargetLat;
	w1lon = Task[task_index].AATTargetLon;
	w0lat = Task[task_index-1].AATTargetLat;
	w0lon = Task[task_index-1].AATTargetLon;
      } else {
	w1lat = WayPointList[Task[task_index].Index].Latitude;
	w1lon = WayPointList[Task[task_index].Index].Longitude;
	w0lat = WayPointList[Task[task_index-1].Index].Latitude;
	w0lon = WayPointList[Task[task_index-1].Index].Longitude;
      }
      
      double NextLegDistance, NextLegBearing;
      
      DistanceBearing(w0lat, 
		      w0lon,
		      w1lat, 
		      w1lon,
		      &NextLegDistance, &NextLegBearing);
      
      double LegAltitude = GlidePolar::
	MacCreadyAltitude(this_maccready, 
			  NextLegDistance, NextLegBearing, 
			  Calculated->WindSpeed, 
			  Calculated->WindBearing, 
			  0, 0,
			  this_is_final,
			  &this_LegTimeToGo,
			  height_above_finish, CRUISE_EFFICIENCY);

      double LegAltitude0 = GlidePolar::
	MacCreadyAltitude(0, 
			  NextLegDistance, NextLegBearing, 
			  Calculated->WindSpeed, 
			  Calculated->WindBearing, 
			  0, 0,
			  true,
			  &LegTime0, 1.0e6, CRUISE_EFFICIENCY
			  );
      
      if (LegTime0>=0.9*ERROR_TIME) {
	// can't make it, so assume flying at current mc
	LegAltitude0 = LegAltitude;
      }          

      TaskAltitudeRequired += LegAltitude;
      TaskAltitudeRequired0 += LegAltitude0;
      
      Calculated->TaskDistanceToGo += NextLegDistance;
      Calculated->TaskTimeToGo += this_LegTimeToGo;      

	if (task_index==1) {
		StartBestCruiseTrack = NextLegBearing;
	}

      if (calc_turning_now) {
	if (task_index == ActiveWayPoint+1) {
	  
	  double NextLegDistanceTurningNow, NextLegBearingTurningNow;
	  double this_LegTimeToGo_turningnow=0;
	  
	  DistanceBearing(Basic->Latitude, 
			  Basic->Longitude,
			  w1lat, 
			  w1lon,
			  &NextLegDistanceTurningNow, 
			  &NextLegBearingTurningNow);
	  
	  GlidePolar::
	    MacCreadyAltitude(this_maccready, 
			      NextLegDistanceTurningNow, 
			      NextLegBearingTurningNow, 
			      Calculated->WindSpeed, 
			      Calculated->WindBearing, 
			      0, 0,
			      this_is_final,
			      &this_LegTimeToGo_turningnow,
			      height_above_finish, CRUISE_EFFICIENCY); 
	  Calculated->TaskTimeToGoTurningNow += this_LegTimeToGo_turningnow;
	} else {
	  Calculated->TaskTimeToGoTurningNow += this_LegTimeToGo;
	}
      }
      
      height_above_finish-= LegAltitude;
      
      task_index--;
    }


  // current waypoint, do this last!

  if (AATEnabled && (ActiveWayPoint>0) && ValidTaskPoint(ActiveWayPoint+1) && Calculated->IsInSector) {
	if (Calculated->WaypointDistance<AATCloseDistance()*3.0) {
		LegBearing = AATCloseBearing(Basic, Calculated);
	}
  }

  double LegAltitude = 
    GlidePolar::MacCreadyAltitude(this_maccready, 
                                  LegToGo, 
                                  LegBearing, 
                                  Calculated->WindSpeed, 
                                  Calculated->WindBearing,
                                  &(Calculated->BestCruiseTrack),
                                  &(Calculated->VMacCready),

				  // (Calculated->FinalGlide==1),
				  true,  // JMW CHECK FGAMT

                                  &(Calculated->LegTimeToGo),
                                  height_above_finish, CRUISE_EFFICIENCY);
  
  double LegAltitude0 = 
    GlidePolar::MacCreadyAltitude(0, 
                                  LegToGo, 
                                  LegBearing, 
                                  Calculated->WindSpeed, 
                                  Calculated->WindBearing,
                                  0,
                                  0,
                                  true,
                                  &LegTime0, 1.0e6, CRUISE_EFFICIENCY
                                  );

  // fix problem of blue arrow wrong in task sector
  if (StartBestCruiseTrack>=0)  // use it only if assigned, workaround
	if (Calculated->IsInSector && (ActiveWayPoint==0)) {
		// set best cruise track to first leg bearing when in start sector
		Calculated->BestCruiseTrack = StartBestCruiseTrack;
	} 

  // JMW TODO accuracy: Use safetymc where appropriate

  if (LegTime0>= 0.9*ERROR_TIME) {
    // can't make it, so assume flying at current mc
    LegAltitude0 = LegAltitude;
  }

  TaskAltitudeRequired += LegAltitude;
  TaskAltitudeRequired0 += LegAltitude0;
  Calculated->TaskDistanceToGo += LegToGo;
  Calculated->TaskTimeToGo += Calculated->LegTimeToGo;

  height_above_finish-= LegAltitude;

  if (calc_turning_now) {
    Calculated->TaskTimeToGoTurningNow += 
      Basic->Time-Calculated->TaskStartTime;
  } else {
    Calculated->TaskTimeToGoTurningNow = -1;
  }


  
  Calculated->TaskAltitudeRequired = TaskAltitudeRequired + final_height;
  
  TaskAltitudeRequired0 += final_height;
  
  Calculated->TaskAltitudeDifference = total_energy_height - Calculated->TaskAltitudeRequired; 
  Calculated->TaskAltitudeDifference0 = total_energy_height - TaskAltitudeRequired0;
  Calculated->NextAltitudeDifference0 = total_energy_height - Calculated->NextAltitudeRequired0;

  Calculated->GRFinish= CalculateGlideRatio(Calculated->TaskDistanceToGo, Calculated->NavAltitude - final_height);

  if (Calculated->TaskSpeedAchieved >0)
	Calculated->LKTaskETE = Calculated->TaskDistanceToGo/Calculated->TaskSpeedAchieved;
  else
	Calculated->LKTaskETE=0;

  CheckGlideThroughTerrain(Basic, Calculated);
  
  CheckForceFinalGlide(Basic, Calculated);
  
  UnlockTaskData();

}


void DoAutoMacCready(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  bool is_final_glide = false;

  if (!Calculated->AutoMacCready) return;

  //  LockFlightData();
  LockTaskData();

  double mc_new = MACCREADY;
  static bool first_mc = true;

  if ( AutoMcMode==amcEquivalent ) {
	if ( CALCULATED_INFO.Circling != TRUE && CALCULATED_INFO.OnGround != TRUE) {
		if (Calculated->EqMc>=0) {
			// MACCREADY = LowPassFilter(MACCREADY,Calculated->EqMc,0.8); 
			MACCREADY = Calculated->EqMc;
		} else {
			// -1.0 is used as an invalid flag. Normally flying at -1 MC means almost flying
			// at stall speed, which is pretty unusual. Maybe in wave conditions?
			if (Calculated->EqMc >-1) {
				MACCREADY=Calculated->EqMc*-1;
			}
		}
	}
	UnlockTaskData();
	return;
  }

  // otherwise, if AutoMc for finalglide or "both", return if no goto
  if (!ValidTaskPoint(ActiveWayPoint)) {
	UnlockTaskData();
	return;
  }

  if (Calculated->FinalGlide && ActiveIsFinalWaypoint()) {  
    is_final_glide = true;
  } else {
    first_mc = true;
  }

  double av_thermal = -1;
  if (flightstats.ThermalAverage.y_ave>0) {
    if (Calculated->Circling && (Calculated->AverageThermal>0)) {
      av_thermal = (flightstats.ThermalAverage.y_ave
		*flightstats.ThermalAverage.sum_n 
		+ Calculated->AverageThermal)/
	(flightstats.ThermalAverage.sum_n+1);
    } else {
      av_thermal = flightstats.ThermalAverage.y_ave;
    }
  } else if (Calculated->Circling && (Calculated->AverageThermal>0)) {
    // insufficient stats, so use this/last thermal's average
    av_thermal = Calculated->AverageThermal;
  }

  if (!ValidTaskPoint(ActiveWayPoint)) {
    if (av_thermal>0) {
      mc_new = av_thermal;
    }
  } else if ( ((AutoMcMode==amcFinalGlide)||(AutoMcMode==amcFinalAndClimb)) && is_final_glide) {

      if (Calculated->TaskAltitudeDifference0>0) {
	
      // only change if above final glide with zero Mc
      // otherwise when we are well below, it will wind Mc back to
      // zero
      
      double slope = 
	(Calculated->NavAltitude + Calculated->EnergyHeight
	 - FAIFinishHeight(Basic, Calculated, ActiveWayPoint))/
	(Calculated->WaypointDistance+1);
      
      double mc_pirker = PirkerAnalysis(Basic, Calculated,
					Calculated->WaypointBearing,
					slope);
      mc_pirker = max(0.0, mc_pirker);
      if (first_mc) {
	// don't allow Mc to wind down to zero when first achieving
	// final glide; but do allow it to wind down after that
	if (mc_pirker >= mc_new) {
	  mc_new = mc_pirker;
	  first_mc = false;
	} else if (AutoMcMode==amcFinalAndClimb) {
	  // revert to averager based auto Mc
	  if (av_thermal>0) {
	    mc_new = av_thermal;
	  }
	}
      } else {
	mc_new = mc_pirker;
      }
    } else { // below final glide at zero Mc, never achieved final glide
      if (first_mc && (AutoMcMode==amcFinalAndClimb)) {
	// revert to averager based auto Mc
	if (av_thermal>0) {
	  mc_new = av_thermal;
	}
      }
    }
  } else if ( (AutoMcMode==amcAverageClimb) || ((AutoMcMode==amcFinalAndClimb)&& !is_final_glide) ) {
    if (av_thermal>0) {
      mc_new = av_thermal;
    }
  }

  MACCREADY = LowPassFilter(MACCREADY,mc_new,0.15);

  UnlockTaskData();
  //  UnlockFlightData();

}


extern int AIRSPACEWARNINGS;
extern int WarningTime;
extern int AcknowledgementTime;


void PredictNextPosition(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  if(Calculated->Circling)
    {
      Calculated->NextLatitude = Basic->Latitude;
      Calculated->NextLongitude = Basic->Longitude;
      Calculated->NextAltitude = 
        Calculated->NavAltitude + Calculated->Average30s * WarningTime;
    }
  else
    {
      FindLatitudeLongitude(Basic->Latitude, 
                            Basic->Longitude, 
                            Basic->TrackBearing, 
                            Basic->Speed*WarningTime,
                            &Calculated->NextLatitude,
                            &Calculated->NextLongitude);

       Calculated->NextAltitude = Calculated->NavAltitude + Calculated->Average30s * WarningTime;
    }
    // We are assuming that terrain altitude will not change much, not accurate though.
    // This is used by airspace engine.
    Calculated->NextAltitudeAGL = Calculated->NextAltitude - Calculated->TerrainAlt;

}


void AATStats_Time(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // Task time to go calculations

  #if 100710
  double aat_tasktime_elapsed, aat_tasklength_seconds;
  aat_tasktime_elapsed = Basic->Time - Calculated->TaskStartTime;
  aat_tasklength_seconds = AATTaskLength*60;
  #else 
  double aat_tasktime_elapsed = Basic->Time - Calculated->TaskStartTime;
  double aat_tasklength_seconds = AATTaskLength*60;
  #endif

  if (ActiveWayPoint==0) {
    // BUG fixed in dlgTaskWaypoint: changing AATTaskLength had no effect until restart
    // because AATTimeToGo was reset only once.
    if (Calculated->AATTimeToGo==0) {
      Calculated->AATTimeToGo = aat_tasklength_seconds;
    }
  } else if (aat_tasktime_elapsed>=0) {
    Calculated->AATTimeToGo = max(0.0,
				  aat_tasklength_seconds 
				  - aat_tasktime_elapsed);
  }

  if(ValidTaskPoint(ActiveWayPoint) && (Calculated->AATTimeToGo>0)) {
    Calculated->AATMaxSpeed = 
      Calculated->AATMaxDistance / Calculated->AATTimeToGo;
    Calculated->AATMinSpeed = 
      Calculated->AATMinDistance / Calculated->AATTimeToGo;
    Calculated->AATTargetSpeed = 
      Calculated->AATTargetDistance / Calculated->AATTimeToGo;
  }
}


void AATStats_Distance(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  int i;
  double MaxDistance, MinDistance, TargetDistance;

  //  LockFlightData();
  LockTaskData();

  MaxDistance = 0; MinDistance = 0; TargetDistance = 0;
  // Calculate Task Distances

  if(ValidTaskPoint(ActiveWayPoint)) 
    {
      i=ActiveWayPoint;

      double LegToGo=0, TargetLegToGo=0;

      if (i > 0 ) { //RLD only include distance from glider to next leg if we've started the task
        DistanceBearing(Basic->Latitude , Basic->Longitude , 
                        WayPointList[Task[i].Index].Latitude, 
                        WayPointList[Task[i].Index].Longitude,
                        &LegToGo, NULL);

        DistanceBearing(Basic->Latitude , Basic->Longitude , 
                        Task[i].AATTargetLat, 
                        Task[i].AATTargetLon,
                        &TargetLegToGo, NULL);

        if(Task[i].AATType == CIRCLE)
        {
          MaxDistance = LegToGo + (Task[i].AATCircleRadius );  // ToDo: should be adjusted for angle of max target and for national rules
          MinDistance = LegToGo - (Task[i].AATCircleRadius );  
        }
        else
        {
          MaxDistance = LegToGo + (Task[i].AATSectorRadius );  // ToDo: should be adjusted for angle of max target.  
          MinDistance = LegToGo;
        }

        TargetDistance = TargetLegToGo;
      }

      i++;
      while(ValidTaskPoint(i)) {
	double LegDistance, TargetLegDistance;
	
	DistanceBearing(WayPointList[Task[i].Index].Latitude, 
			WayPointList[Task[i].Index].Longitude,
			WayPointList[Task[i-1].Index].Latitude, 
			WayPointList[Task[i-1].Index].Longitude,
			&LegDistance, NULL);
	
	DistanceBearing(Task[i].AATTargetLat,
			Task[i].AATTargetLon,
			Task[i-1].AATTargetLat,
			Task[i-1].AATTargetLon,
			&TargetLegDistance, NULL);
	
	MaxDistance += LegDistance;
	MinDistance += LegDistance;
	
	if(Task[ActiveWayPoint].AATType == CIRCLE) {
	  // breaking out single Areas increases accuracy for start
	  // and finish
	  
	  // sector at start of (i)th leg
	  if (i-1 == 0) {// first leg of task
	    // add nothing
	    MaxDistance -= StartRadius; // e.g. Sports 2009 US Rules A116.3.2.  To Do: This should be configured multiple countries
	    MinDistance -= StartRadius;
	  } else { // not first leg of task
	    MaxDistance += (Task[i-1].AATCircleRadius);  //ToDo: should be adjusted for angle of max target
	    MinDistance -= (Task[i-1].AATCircleRadius);  //ToDo: should be adjusted for angle of max target
	  }
	  
	  // sector at end of ith leg
	  if (!ValidTaskPoint(i+1)) {// last leg of task
	    // add nothing
	    MaxDistance -= FinishRadius; // To Do: This can be configured for finish rules
	    MinDistance -= FinishRadius;
	  } else { // not last leg of task
	    MaxDistance += (Task[i].AATCircleRadius);  //ToDo: should be adjusted for angle of max target
	    MinDistance -= (Task[i].AATCircleRadius);  //ToDo: should be adjusted for angle of max target
	  }
	} else { // not circle (pie slice)
	  // sector at start of (i)th leg
	  if (i-1 == 0) {// first leg of task
	    // add nothing
	    MaxDistance += 0; // To Do: This can be configured for start rules
	  } else { // not first leg of task
	    MaxDistance += (Task[i-1].AATCircleRadius);  //ToDo: should be adjusted for angle of max target
	  }
	  
	  // sector at end of ith leg
	  if (!ValidTaskPoint(i+1)) {// last leg of task
	    // add nothing
	    MaxDistance += 0; // To Do: This can be configured for finish rules
	  } else { // not last leg of task
	    MaxDistance += (Task[i].AATCircleRadius);  //ToDo: should be adjusted for angle of max target
	  }
	}
	TargetDistance += TargetLegDistance;
	i++;
      }
      
      // JMW TODO accuracy: make these calculations more accurate, because
      // currently they are very approximate.

      Calculated->AATMaxDistance = MaxDistance;
      Calculated->AATMinDistance = MinDistance;
      Calculated->AATTargetDistance = TargetDistance;
    }
  UnlockTaskData();
  //  UnlockFlightData();
}


void AATStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{

  if (!WayPointList 
      || !AATEnabled 
      || Calculated->ValidFinish) return ;

  AATStats_Distance(Basic, Calculated);
  AATStats_Time(Basic, Calculated);

}


void ThermalBand(NMEA_INFO *Basic, DERIVED_INFO *Calculated) 
{
  static double LastTime = 0;
  if(Basic->Time <= LastTime)
    {
      LastTime = Basic->Time;
      return;
    }
  LastTime = Basic->Time;

  // JMW TODO accuracy: Should really work out dt here, 
  //           but i'm assuming constant time steps
  double dheight = Calculated->NavAltitude -Calculated->TerrainBase; 

  int index, i, j;

  if (dheight<0) {
    return; // nothing to do.
  }
  if (Calculated->MaxThermalHeight==0) {
    Calculated->MaxThermalHeight = dheight;
  }

  // only do this if in thermal and have been climbing
  if ((!Calculated->Circling)||(Calculated->Average30s<0)) return;

  //  LockFlightData(); 

  if (dheight > Calculated->MaxThermalHeight) {

    // moved beyond ceiling, so redistribute buckets
    double max_thermal_height_new;
    double tmpW[NUMTHERMALBUCKETS];
    int tmpN[NUMTHERMALBUCKETS];
    double h;

    // calculate new buckets so glider is below max
    double hbuk = Calculated->MaxThermalHeight/NUMTHERMALBUCKETS;
  
    max_thermal_height_new = max(1.0, Calculated->MaxThermalHeight);
    while (max_thermal_height_new<dheight) {
      max_thermal_height_new += hbuk;
    }

    // reset counters
    for (i=0; i<NUMTHERMALBUCKETS; i++) {
      tmpW[i]= 0.0;
      tmpN[i]= 0;
    }
    // shift data into new buckets
    for (i=0; i<NUMTHERMALBUCKETS; i++) {
      h = (i)*(Calculated->MaxThermalHeight)/(NUMTHERMALBUCKETS); 
      // height of center of bucket
      j = iround(NUMTHERMALBUCKETS*h/max_thermal_height_new);

      if (j<NUMTHERMALBUCKETS) {
        if (Calculated->ThermalProfileN[i]>0) {
          tmpW[j] += Calculated->ThermalProfileW[i];
          tmpN[j] += Calculated->ThermalProfileN[i];
        }
      }
    }
    for (i=0; i<NUMTHERMALBUCKETS; i++) {
      Calculated->ThermalProfileW[i]= tmpW[i];
      Calculated->ThermalProfileN[i]= tmpN[i];
    }
    Calculated->MaxThermalHeight= max_thermal_height_new;
  }

  index = min(NUMTHERMALBUCKETS-1,
	      iround(NUMTHERMALBUCKETS*(dheight/max(1.0,
		     Calculated->MaxThermalHeight))));

  Calculated->ThermalProfileW[index]+= Calculated->Vario;
  Calculated->ThermalProfileN[index]++;
  //  UnlockFlightData();

}

void LatLon2Flat(double lon, double lat, int *scx, int *scy) {
  *scx = (int)(lon*fastcosine(lat)*100);
  *scy = (int)(lat*100);
}


int CalculateWaypointApproxDistance(int scx_aircraft, int scy_aircraft,
                                    int i) {

  // Do preliminary fast search, by converting to screen coordinates
  int sc_x, sc_y;
  LatLon2Flat(WayPointList[i].Longitude, 
              WayPointList[i].Latitude, &sc_x, &sc_y);
  int dx, dy;
  dx = scx_aircraft-sc_x;
  dy = scy_aircraft-sc_y;

  return isqrt4(dx*dx+dy*dy);
}


// This is also called by DoNearest and it is overwriting AltitudeRequired 
double CalculateWaypointArrivalAltitude(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int i) {

  double altReqd;
  double wDistance, wBearing;
  double wStartDistance=0, wStartBearing=0;
  double safetyaltitudearrival;

  safetyaltitudearrival=GetSafetyAltitude(i);


  DistanceBearing(Basic->Latitude, 
                  Basic->Longitude,
                  WayPointList[i].Latitude, 
                  WayPointList[i].Longitude,
                  &wDistance, &wBearing);

  WayPointCalc[i].Distance = wDistance;
  WayPointCalc[i].Bearing  = wBearing;

	altReqd = GlidePolar::MacCreadyAltitude ( GetMacCready(i,GMC_DEFAULT),
		wDistance, wBearing, 
		Calculated->WindSpeed, Calculated->WindBearing, 
		0, 0, true, &WayPointCalc[i].NextETE);
	// if gates are in use with a real task, and we are at start 
	// then calculate ETE for reaching the cylinder. Also working when we are 
	// in the wrong side of cylinder
	if (UseGates() && !DoOptimizeRoute()) {
		if (ActiveWayPoint==0 && i==Task[0].Index ) { 
			if (PGStartOut) {
				if (CorrectSide()) {
					// start out,  from outside cylinder
					wStartDistance=wDistance-StartRadius;
					wStartBearing=wBearing;
				} else {
					// start out,  but inside cylinder
					wStartDistance=StartRadius-wDistance;
					wStartBearing=wBearing+180;
					if (wStartBearing>360) wStartBearing-=360;
				}
			} else {
				if (CorrectSide()) {
					// start in, correct side is inside cylinder
					wStartDistance=StartRadius-wDistance;
					wStartBearing=wBearing+180;
					if (wStartBearing>360) wStartBearing-=360;
				} else {
					// start in, and we are still outside
					wStartDistance=wDistance-StartRadius;
					wStartBearing=wBearing;
				}
			}

			// we don't use GetMacCready(i,GMC_DEFAULT)
			GlidePolar::MacCreadyAltitude ( MACCREADY,
			wStartDistance, wStartBearing, 
			Calculated->WindSpeed, Calculated->WindBearing, 
			0, 0, true, &WayPointCalc[i].NextETE);
			#ifdef DEBUGTGATES
			StartupStore(_T("wStartDistance=%f wStartBearing=%f\n"),wStartDistance,wStartBearing);
			#endif
		}
	}

        // we should build a function for this since it is used also in lkcalc
	WayPointCalc[i].AltReqd[AltArrivMode]  = altReqd+safetyaltitudearrival+WayPointList[i].Altitude -Calculated->EnergyHeight; 
	WayPointCalc[i].AltArriv[AltArrivMode] = Calculated->NavAltitude + Calculated->EnergyHeight
						- altReqd 
						- WayPointList[i].Altitude 
						- safetyaltitudearrival;
/*
		WayPointCalc[i].AltArriv[ALTA_AVEFF] = Calculated->NavAltitude 
							- (wDistance / GetCurrentEfficiency(Calculated, 0)) 
							- WayPointList[i].Altitude
							-safetyaltitudearrival; 

		WayPointCalc[i].AltReqd[ALTA_AVEFF] = Calculated->NavAltitude - WayPointCalc[i].AltArriv[ALTA_AVEFF];
		WayPointCalc[i].NextETE=600.0;
*/
 
   return(WayPointCalc[i].AltArriv[AltArrivMode]); 

}


void DoAutoQNH(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  static int done_autoqnh = 0;

  if (DoInit[MDI_DOAUTOQNH]) {
	done_autoqnh=0;
	DoInit[MDI_DOAUTOQNH]=false;
  }

  // Reject if already done
  if (done_autoqnh==10) return;

  // Reject if in IGC logger mode
  if (ReplayLogger::IsEnabled()) return;

  // Reject if no valid GPS fix
  if (Basic->NAVWarning) return;

  // Reject if no baro altitude
  if (!Basic->BaroAltitudeAvailable) return;

  // Reject if terrain height is invalid
  if (!Calculated->TerrainValid) return;

  if (QNH != 1013.25) return;

  if (Basic->Speed<TakeOffSpeedThreshold) {
    done_autoqnh++;
  } else {
    done_autoqnh= 0; // restart...
  }

  if (done_autoqnh==10) {
	double fixaltitude = Calculated->TerrainAlt;

	// if we have a valid fix, and a valid home waypoint, then if we are close to it assume we are at home
	// and use known altitude, instead of presumed terrain altitude which is always approximated
	double hdist=0;
	if (ValidWayPoint(HomeWaypoint)) {
		DistanceBearing(Basic->Latitude, Basic->Longitude, 
			WayPointList[HomeWaypoint].Latitude, WayPointList[HomeWaypoint].Longitude,&hdist,NULL);

		if (hdist <2000) {
			fixaltitude=WayPointList[HomeWaypoint].Altitude;
			StartupStore(_T(". AutoQNH: setting QNH to HOME waypoint altitude=%.0f m%s"),fixaltitude,NEWLINE);
		} else {
			if (fixaltitude!=0)
				StartupStore(_T(". AutoQNH: setting QNH to average terrain altitude=%.0f m%s"),fixaltitude,NEWLINE);
			else
				StartupStore(_T(". AutoQNH: cannot set QNH, impossible terrain altitude%s"),NEWLINE);
		}
	} else {
		// 101121 extend search for nearest wp
		int i=FindNearestWayPoint(Basic->Longitude, Basic->Latitude, 2000);
		if ( (i>RESWP_END) && (WayPointList[i].Altitude>0) ) {  // avoid using TAKEOFF wp
			fixaltitude=WayPointList[i].Altitude;
			#if ALPHADEBUG
			StartupStore(_T(". AutoQNH: setting QNH to nearest <%s> waypoint altitude=%.0f m%s"),
				WayPointList[i].Name,fixaltitude,NEWLINE);
			#endif
		} else {
			#if ALPHADEBUG
			if (fixaltitude!=0)
				StartupStore(_T(". AutoQNH: setting QNH to average terrain altitude=%.0f m%s"),fixaltitude,NEWLINE);
			else
				StartupStore(_T(". AutoQNH: cannot set QNH, impossible terrain altitude%s"),NEWLINE);
			#endif
		}
	}
	if (fixaltitude!=0) {
		QNH = FindQNH(Basic->BaroAltitude, fixaltitude);
		TCHAR qmes[80];
		if (PressureHg) 
			_stprintf(qmes,_T("QNH set to %.2f, Altitude %.0f%s"),QNH/TOHPA,fixaltitude,
			Units::GetUnitName(Units::GetUserAltitudeUnit()));
		else
			_stprintf(qmes,_T("QNH set to %.2f, Altitude %.0f%s"),QNH,fixaltitude,
			Units::GetUnitName(Units::GetUserAltitudeUnit()));
		DoStatusMessage(qmes);
		CAirspaceManager::Instance().QnhChangeNotify(QNH);
	}
  }
}



void IterateEffectiveMacCready(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // nothing yet.
}


int FindFlarmSlot(int flarmId)
{
  for(int z = 0; z < FLARM_MAX_TRAFFIC; z++)
    {
      if (GPS_INFO.FLARM_Traffic[z].ID == flarmId)
	{
	  return z;
	}
    }
  return -1;
}

int FindFlarmSlot(TCHAR *flarmCN)
{
  for(int z = 0; z < FLARM_MAX_TRAFFIC; z++)
    {
      if (wcscmp(GPS_INFO.FLARM_Traffic[z].Name, flarmCN) == 0)
	{
	  return z;
	}
    }
  return -1;
}

bool IsFlarmTargetCNInRange()
{
  return false;
}

 
 void BallastDump ()
 {
   static double BallastTimeLast = -1;

   if (BallastTimerActive) {
         // JMW only update every 5 seconds to stop flooding the devices
     if (GPS_INFO.Time > BallastTimeLast+5) {
 //      double BALLAST_last = BALLAST;
       double dt = GPS_INFO.Time - BallastTimeLast;
       double percent_per_second = 1.0/max(10, BallastSecsToEmpty);
       BALLAST -= dt*percent_per_second;
       if (BALLAST<0) {
         BallastTimerActive = false;
         BALLAST = 0.0;
         GlidePolar::SetBallast(); 
         devPutBallast(devA(), BALLAST); // 
         devPutBallast(devB(), BALLAST); //
       }
  //     if (fabs(BALLAST-BALLAST_last)>0.01) removed the change check, will be send every 5s for long ballast dumps
       { // Ulli changed from 5% to 1% because sometimes it stopped at 55%
         GlidePolar::SetBallast();
         devPutBallast(devA(), BALLAST);
         devPutBallast(devB(), BALLAST);
       }
       BallastTimeLast = GPS_INFO.Time;
     }
   } else {
     BallastTimeLast = GPS_INFO.Time;
   }
 }


int DetectStartTime(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // we want this to display landing time until next takeoff

  static int starttime = -1;
  static int lastflighttime = -1;

  if (Calculated->Flying) {
    if (starttime == -1) {
      // hasn't been started yet

      starttime = (int)GPS_INFO.Time;

      lastflighttime = -1;
    }
    return (int)GPS_INFO.Time-starttime;

  } else {

    if (lastflighttime == -1) {
      // hasn't been stopped yet
      if (starttime>=0) {
        lastflighttime = (int)Basic->Time-starttime;
      } else {
        return 0; // no last flight time
      }
      // reset for next takeoff
      starttime = -1;
    }
  }

  // return last flighttime if it exists
  return max(0,lastflighttime);
}

// HeadWind error will be shown as -999
// These values are all in m/s
// We can have a serious problem when the headwind is so strong that the
// aircraft is actually flying backwards. 
// In such case, the heading will show correctly and the pilot should see
// that there is a problem.
//
// A positive value indicates a headwind, and a negative value indicates a tailwind.
//
void CalculateHeadWind(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  static double CrossBearingLast= -1.0;
  static double WindSpeedLast= -1.0;

  if (Basic->NAVWarning) {
	Calculated->HeadWind  = -999;
	return;
  }

  double CrossBearing;
  CrossBearing = AngleLimit360(Calculated->Heading - Calculated->WindBearing);

  #if 1 // vector wind
  if ((CrossBearing != CrossBearingLast)||(Calculated->WindSpeed != WindSpeedLast)) {
	Calculated->HeadWind = Calculated->WindSpeed * fastcosine(CrossBearing);
	// CrossWind = WindSpeed * fastsine(CrossBearing);  UNUSED
	CrossBearingLast = CrossBearing;
	WindSpeedLast = Calculated->WindSpeed;
  }
  #else
  if (Basic->AirspeedAvailable) {
	Calculated->HeadWind = Basic->TrueAirspeed - Basic->Speed;
  } else {
	// for estimated IAS, this is also vector wind
	Calculated->HeadWind = Calculated->TrueAirspeedEstimated - Basic->Speed;
  }
  #endif
  //StartupStore(_T("..... CrossBearing=%f  hdwind=%f windspeed=%f\n"),CrossBearing,Calculated->HeadWind, Calculated->WindSpeed);

}

