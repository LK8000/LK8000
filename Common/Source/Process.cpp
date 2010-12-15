/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Process.cpp,v 8.3 2010/12/12 16:27:39 root Exp root $
*/

#include "StdAfx.h"
#include <windows.h>

#include "Defines.h" // VENTA3
#include "compatibility.h"
#ifdef OLDPPC
#include "XCSoarProcess.h"
#else
#include "Process.h"
#endif
#include "externs.h"
#include "Utils.h"
#include "Utils2.h"
#include "device.h"
#include "Dialogs.h"
#include "Port.h"
#include "Atmosphere.h"
#include "AATDistance.h"

extern AATDistance aatdistance;

extern int PDABatteryPercent;
extern int PDABatteryStatus;
extern int PDABatteryFlag;

// JMW added key codes,
// so -1 down
//     1 up
//     0 enter

void	AirspeedProcessing(int UpDown)
{
#if 0
  if (UpDown==0) {
    EnableCalibration = !EnableCalibration;
	// XXX InputEvents - Is this an automatic or user thing - either way, needs moving
    if (EnableCalibration) 
      DoStatusMessage(TEXT("Calibrate ON"));
    else 
      DoStatusMessage(TEXT("Calibrate OFF"));
  }
#endif
}

void	TeamCodeProcessing(int UpDown)
{	
#if 0
	int tryCount = 0;
	int searchSlot = FindFlarmSlot(TeamFlarmIdTarget);
	int newFlarmSlot = -1;


	while (tryCount < FLARM_MAX_TRAFFIC)
	{
		if (UpDown == 1)
		{
			searchSlot++;
			if (searchSlot > FLARM_MAX_TRAFFIC - 1)
			{
				searchSlot = 0;
			}
		}
		else if (UpDown == -1)
		{
			searchSlot--;
			if (searchSlot < 0)
			{
				searchSlot = FLARM_MAX_TRAFFIC - 1;
			}
		}

		if (GPS_INFO.FLARM_Traffic[searchSlot].ID != 0)
		{
			newFlarmSlot = searchSlot;
			break; // a new flarmSlot with a valid flarm traffic record was found !
		}
		tryCount++;
	}

	if (newFlarmSlot != -1)
	{
		TeamFlarmIdTarget = GPS_INFO.FLARM_Traffic[newFlarmSlot].ID;

		if (wcslen(GPS_INFO.FLARM_Traffic[newFlarmSlot].Name) != 0)
		{			
			// copy the 3 first chars from the name to TeamFlarmCNTarget
			for (int z = 0; z < 3; z++)
			{
				if (GPS_INFO.FLARM_Traffic[newFlarmSlot].Name[z] != 0)
				{
					TeamFlarmCNTarget[z] = GPS_INFO.FLARM_Traffic[newFlarmSlot].Name[z];
				}
				else
				{
					TeamFlarmCNTarget[z] = 32; // add space char
				}
			}
			TeamFlarmCNTarget[3] = 0;
		}
		else
		{		
			TeamFlarmCNTarget[0] = 0;
		}
	}
	else
	{
			// no flarm traffic to select!
			TeamFlarmIdTarget = 0;
			TeamFlarmCNTarget[0] = 0;
			return;		
	}
#endif
}

void	AltitudeProcessing(int UpDown)
{
#if 0
  #if NOSIM
  if (SIMMODE) {
	if(UpDown==1) {
	  GPS_INFO.Altitude += (100/ALTITUDEMODIFY);
	}	else if (UpDown==-1)
	  {
	    GPS_INFO.Altitude -= (100/ALTITUDEMODIFY);
	    if(GPS_INFO.Altitude < 0)
	      GPS_INFO.Altitude = 0;
	  } else if (UpDown==-2) {
	  DirectionProcessing(-1);
	} else if (UpDown==2) {
	  DirectionProcessing(1);
	}
   }
   #else
	#ifdef _SIM_
	if(UpDown==1) {
	  GPS_INFO.Altitude += (100/ALTITUDEMODIFY);
	}	else if (UpDown==-1)
	  {
	    GPS_INFO.Altitude -= (100/ALTITUDEMODIFY);
	    if(GPS_INFO.Altitude < 0)
	      GPS_INFO.Altitude = 0;
	  } else if (UpDown==-2) {
	  DirectionProcessing(-1);
	} else if (UpDown==2) {
	  DirectionProcessing(1);
	}
	#endif
  #endif
	return;
#endif
}

// VENTA3 QFE
void	QFEAltitudeProcessing(int UpDown)
{
#if 0
	short step;
	if ( ( CALCULATED_INFO.NavAltitude - QFEAltitudeOffset ) <10 ) step=1; else step=10;
	if(UpDown==1) {
	   QFEAltitudeOffset -= (step/ALTITUDEMODIFY);
	}	else if (UpDown==-1)
	  {
	    QFEAltitudeOffset += (step/ALTITUDEMODIFY);
/*
	    if(QFEAltitudeOffset < 0)
	      QFEAltitudeOffset = 0;
*/
	  } else if (UpDown==-2) {
	  DirectionProcessing(-1);
	} else if (UpDown==2) {
	  DirectionProcessing(1);
	}
	return;
#endif
}

// VENTA3 Alternates processing updown 
void Alternate1Processing(int UpDown)
{
   if (UpDown==0) {
	if ( Alternate1 <0 ) return;
	LockTaskData(); SelectedWaypoint = Alternate1; PopupWaypointDetails(); UnlockTaskData();
  } 
}
void Alternate2Processing(int UpDown)
{
   if (UpDown==0) {
	if ( Alternate2 <0 ) return;
	LockTaskData(); SelectedWaypoint = Alternate2; PopupWaypointDetails(); UnlockTaskData();
  } 
}
void BestAlternateProcessing(int UpDown)
{
   if (UpDown==0) {
	if ( BestAlternate <0 ) return;
	LockTaskData(); SelectedWaypoint = BestAlternate; PopupWaypointDetails(); UnlockTaskData();
  } 
}

void	SpeedProcessing(int UpDown)
{
  #if NOSIM
  if (SIMMODE) {
		if(UpDown==1)
			GPS_INFO.Speed += (10/SPEEDMODIFY);
		else if (UpDown==-1)
		{
			GPS_INFO.Speed -= (10/SPEEDMODIFY);
			if(GPS_INFO.Speed < 0)
				GPS_INFO.Speed = 0;
		} else if (UpDown==-2) {
			DirectionProcessing(-1);
		} else if (UpDown==2) {
			DirectionProcessing(1);
		}
  } 
  #else
	#ifdef _SIM_
		if(UpDown==1)
			GPS_INFO.Speed += (10/SPEEDMODIFY);
		else if (UpDown==-1)
		{
			GPS_INFO.Speed -= (10/SPEEDMODIFY);
			if(GPS_INFO.Speed < 0)
				GPS_INFO.Speed = 0;
		} else if (UpDown==-2) {
			DirectionProcessing(-1);
		} else if (UpDown==2) {
			DirectionProcessing(1);
		}
	#endif
  #endif
	return;
}


void	AccelerometerProcessing(int UpDown)
{
  DWORD Temp;
  if (UpDown==0) {
    AccelerometerZero*= GPS_INFO.Gload;
    if (AccelerometerZero<1) {
      AccelerometerZero = 100;
    }
    Temp = (int)AccelerometerZero;
    SetToRegistry(szRegistryAccelerometerZero,Temp);
  }
}

void	WindDirectionProcessing(int UpDown)
{
	
	if(UpDown==1)
	{
		CALCULATED_INFO.WindBearing  += 5;
		while (CALCULATED_INFO.WindBearing  >= 360)
		{
			CALCULATED_INFO.WindBearing  -= 360;
		}
	}
	else if (UpDown==-1)
	{
		CALCULATED_INFO.WindBearing  -= 5;
		while (CALCULATED_INFO.WindBearing  < 0)
		{
			CALCULATED_INFO.WindBearing  += 360;
		}
	} else if (UpDown == 0) {
          SetWindEstimate(CALCULATED_INFO.WindSpeed,
                          CALCULATED_INFO.WindBearing);
	  #ifndef NOWINDREGISTRY
	  SaveWindToRegistry();
	  #endif
	}
	return;
}


void	WindSpeedProcessing(int UpDown)
{
	if(UpDown==1)
		CALCULATED_INFO.WindSpeed += (1/SPEEDMODIFY);
	else if (UpDown== -1)
	{
		CALCULATED_INFO.WindSpeed -= (1/SPEEDMODIFY);
		if(CALCULATED_INFO.WindSpeed < 0)
			CALCULATED_INFO.WindSpeed = 0;
	} 
	// JMW added faster way of changing wind direction
	else if (UpDown== -2) {
		WindDirectionProcessing(-1);
	} else if (UpDown== 2) {
		WindDirectionProcessing(1);
	} else if (UpDown == 0) {
          SetWindEstimate(CALCULATED_INFO.WindSpeed,
                          CALCULATED_INFO.WindBearing);
	  #ifndef NOWINDREGISTRY
	  SaveWindToRegistry();
	  #endif
	}
	return;
}

void	DirectionProcessing(int UpDown)
{
  #if NOSIM
  if (SIMMODE) {
		if(UpDown==1)
		{
			GPS_INFO.TrackBearing   += 5;
			while (GPS_INFO.TrackBearing  >= 360)
			{
				GPS_INFO.TrackBearing  -= 360;
			}
		}
		else if (UpDown==-1)
		{
			GPS_INFO.TrackBearing  -= 5;
			while (GPS_INFO.TrackBearing  < 0)
			{
				GPS_INFO.TrackBearing  += 360;
			}
		}

  }
  #else
	#ifdef _SIM_
		if(UpDown==1)
		{
			GPS_INFO.TrackBearing   += 5;
			while (GPS_INFO.TrackBearing  >= 360)
			{
				GPS_INFO.TrackBearing  -= 360;
			}
		}
		else if (UpDown==-1)
		{
			GPS_INFO.TrackBearing  -= 5;
			while (GPS_INFO.TrackBearing  < 0)
			{
				GPS_INFO.TrackBearing  += 360;
			}
		}
	#endif
  #endif
	return;
}

void	MacCreadyProcessing(int UpDown)
{

  if(UpDown==1) {
	CALCULATED_INFO.AutoMacCready=false;  // 091214 disable AutoMacCready when changing MC values
    MACCREADY += (double)0.1/LIFTMODIFY; // BUGFIX 100102
    
    if (MACCREADY>5.0) { // JMW added sensible limit
      MACCREADY=5.0;
    }
  }
  else if(UpDown==-1)
    {
	CALCULATED_INFO.AutoMacCready=false;  // 091214 disable AutoMacCready when changing MC values
      MACCREADY -= (double)0.1/LIFTMODIFY; // 100102
      if(MACCREADY < 0)
	{
	  MACCREADY = 0;
	}

  } else if (UpDown==0)
    {
      CALCULATED_INFO.AutoMacCready = !CALCULATED_INFO.AutoMacCready; 
      // JMW toggle automacready
	} 
  else if (UpDown==-2)
    {
      CALCULATED_INFO.AutoMacCready = false;  // SDP on auto maccready
      
    }
  else if (UpDown==+2)
    {
      CALCULATED_INFO.AutoMacCready = true;	// SDP off auto maccready
      
    }
  else if (UpDown==3)
    {
	CALCULATED_INFO.AutoMacCready=false;  // 091214 disable AutoMacCready when changing MC values
	MACCREADY += (double)0.5/LIFTMODIFY; // 100102
	if (MACCREADY>5.0) MACCREADY=5.0; 
      
    }
  else if (UpDown==-3)
    {
	CALCULATED_INFO.AutoMacCready=false;  // 091214 disable AutoMacCready when changing MC values
	MACCREADY -= (double)0.5/LIFTMODIFY; // 100102
	if (MACCREADY<0) MACCREADY=0; 
      
    }
  
  devPutMacCready(devA(), MACCREADY); 
  devPutMacCready(devB(), MACCREADY);
  
  return;
}


void	ForecastTemperatureProcessing(int UpDown)
{
  if (UpDown==1) {
    CuSonde::adjustForecastTemperature(0.5);
  }
  if (UpDown== -1) {
    CuSonde::adjustForecastTemperature(-0.5);
  }
}


extern void PopupWaypointDetails();

/*
	1	Next waypoint
	0	Show waypoint details
	-1	Previous waypoint
	2	Next waypoint with wrap around
	-2	Previous waypoint with wrap around
*/
void NextUpDown(int UpDown)
{

  if (!ValidTaskPoint(ActiveWayPoint)) {	// BUGFIX 091116
	StartupStore(_T(". DBG-801 activewaypoint%s"),NEWLINE);
	return;
  }

  LockTaskData();

  if(UpDown>0) {
    // this was a bug. checking if AWP was < 0 assuming AWP if inactive was -1; actually it can also be 0, a bug is around
    if(ActiveWayPoint < MAXTASKPOINTS) {
      // Increment Waypoint
      if(Task[ActiveWayPoint+1].Index >= 0) {
	if(ActiveWayPoint == 0)	{
	  // manual start
	  // TODO bug: allow restart
	  // TODO bug: make this work only for manual
	  if (CALCULATED_INFO.TaskStartTime==0) {
	    CALCULATED_INFO.TaskStartTime = GPS_INFO.Time;
	  }
	}
	ActiveWayPoint ++;
	AdvanceArmed = false;
	CALCULATED_INFO.LegStartTime = GPS_INFO.Time ;
      }
      // No more, try first
      else 
        if((UpDown == 2) && (Task[0].Index >= 0)) {
          /* ****DISABLED****
          if(ActiveWayPoint == 0)	{
            // TODO bug: allow restart
            // TODO bug: make this work only for manual
            
            // TODO bug: This should trigger reset of flight stats, but 
            // should ask first...
            if (CALCULATED_INFO.TaskStartTime==0) {
              CALCULATED_INFO.TaskStartTime = GPS_INFO.Time ;
            }            
          }
          */
          AdvanceArmed = false;
          ActiveWayPoint = 0;
          CALCULATED_INFO.LegStartTime = GPS_INFO.Time ;
        }
    }
  }
  else if (UpDown<0){
    if(ActiveWayPoint >0) {

      ActiveWayPoint --;
      /*
	XXX How do we know what the last one is?
	} else if (UpDown == -2) {
	ActiveWayPoint = MAXTASKPOINTS;
      */
    } else {
      if (ActiveWayPoint==0) {

        RotateStartPoints();

	// restarted task..
	//	TODO bug: not required? CALCULATED_INFO.TaskStartTime = 0;
      }
    }
    aatdistance.ResetEnterTrigger(ActiveWayPoint);    
  } 
  else if (UpDown==0) {
    SelectedWaypoint = Task[ActiveWayPoint].Index;
    PopupWaypointDetails();
  }
  if (ActiveWayPoint>=0) {
    SelectedWaypoint = Task[ActiveWayPoint].Index;
  }
  UnlockTaskData();
}


void NoProcessing(int UpDown)
{
  (void)UpDown;
  return;
}

void FormatterLowWarning::AssignValue(int i) {
  InfoBoxFormatter::AssignValue(i);
  switch (i) {
  case 1:
    minimum = ALTITUDEMODIFY*SAFETYALTITUDETERRAIN;
    break;
  case 2:
    minimum = 0.5*LIFTMODIFY*CALCULATED_INFO.MacCreadyRisk;
    break;
  case 21:
    minimum = 0.667*LIFTMODIFY*CALCULATED_INFO.MacCreadyRisk;
    break;
  default:
    break;
  }
}


void FormatterTime::SecsToDisplayTime(int d) {
  bool negative = (d<0);
  int dd = abs(d) % (3600*24);

  hours = (dd/3600);
  mins = (dd/60-hours*60);
  seconds = (dd-mins*60-hours*3600);
  hours = hours % 24;
  if (negative) {
    if (hours>0) {
      hours = -hours;
    } else if (mins>0) {
      mins = -mins;
    } else {
      seconds = -seconds;
    }
  }
  Valid = TRUE;
}


int TimeLocal(int localtime) {
  localtime += GetUTCOffset();
  if (localtime<0) {
    localtime+= 3600*24;
  }
  return localtime;
}

int DetectCurrentTime() {
  int localtime = (int)GPS_INFO.Time;
  return TimeLocal(localtime);
}

// simple localtime with no 24h exceeding
int LocalTime() {
  int localtime = (int)GPS_INFO.Time;
  localtime += GetUTCOffset();
  if (localtime<0) {
	localtime+= 86400;
  } else {
	if (localtime>86400) {
		localtime -=86400;
	}
  }
  return localtime;
}

int DetectStartTime(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // JMW added restart ability
  //
  // we want this to display landing time until next takeoff 

  static int starttime = -1;
  static int lastflighttime = -1;

  if (Calculated->Flying) {
    if (starttime == -1) {
      // hasn't been started yet
      
      starttime = (int)GPS_INFO.Time;

      lastflighttime = -1;
    }
    // bug? if Basic time rolled over 00 UTC, negative value is returned
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


void FormatterTime::AssignValue(int i) {
  switch (i) {
  case 9:
    SecsToDisplayTime((int)CALCULATED_INFO.LastThermalTime);
    break;
  case 36:
    SecsToDisplayTime((int)CALCULATED_INFO.FlightTime);
    break;
  case 39:
    SecsToDisplayTime(DetectCurrentTime());
    break;
  case 40:
    SecsToDisplayTime((int)(GPS_INFO.Time));
    break;
  case 46:
    SecsToDisplayTime((int)(CALCULATED_INFO.LegTimeToGo+DetectCurrentTime()));
    Valid = ValidTaskPoint(ActiveWayPoint) && 
      (CALCULATED_INFO.LegTimeToGo< 0.9*ERROR_TIME);
    break;
  default:
    break;
  }
}


void FormatterAATTime::AssignValue(int i) {
  double dd;
  if (AATEnabled && ValidTaskPoint(ActiveWayPoint)) {
    dd = CALCULATED_INFO.TaskTimeToGo;
    if ((CALCULATED_INFO.TaskStartTime>0.0) && (CALCULATED_INFO.Flying)
        &&(ActiveWayPoint>0)) {
      dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
    }
    dd= max(0,min(24.0*3600.0,dd))-AATTaskLength*60;
    if (dd<0) {
      status = 1; // red
    } else {
      if (CALCULATED_INFO.TaskTimeToGoTurningNow > (AATTaskLength+5)*60) {
        status = 2; // blue
      } else {
        status = 0;  // black
      }
    }
  } else {
    dd = 0;
    status = 0; // black
  }

  switch (i) {
  case 27:
    SecsToDisplayTime((int)CALCULATED_INFO.AATTimeToGo);
    Valid = (ValidTaskPoint(ActiveWayPoint) && AATEnabled
	     && (CALCULATED_INFO.AATTimeToGo< 0.9*ERROR_TIME));
    break;
  case 41: 
#if (OLD_CALCULATIONS) 
    SecsToDisplayTime((int)(CALCULATED_INFO.TaskTimeToGo));
    Valid = ValidTaskPoint(ActiveWayPoint) 
      && (CALCULATED_INFO.TaskTimeToGo< 0.9*ERROR_TIME);
#else
    SecsToDisplayTime((int)(CALCULATED_INFO.LKTaskETE));
    Valid = ValidTaskPoint(ActiveWayPoint) 
      && (CALCULATED_INFO.LKTaskETE< 0.9*ERROR_TIME) && CALCULATED_INFO.LKTaskETE>0;

    if (!Valid) {
	 if ( (WayPointCalc[Task[ActiveWayPoint].Index].NextETE > 0) && !ValidTaskPoint(1)) {
		SecsToDisplayTime((int)(WayPointCalc[Task[ActiveWayPoint].Index].NextETE));
		Valid=true;
	}
    }

#endif
    break;

  case 42:
#if (OLD_CALCULATIONS)
	SecsToDisplayTime((int)(CALCULATED_INFO.LegTimeToGo));
	Valid = ValidTaskPoint(ActiveWayPoint) && (CALCULATED_INFO.LegTimeToGo< 0.9*ERROR_TIME);
#else
	if (ValidTaskPoint(ActiveWayPoint)) {
		SecsToDisplayTime((int)(WayPointCalc[Task[ActiveWayPoint].Index].NextETE));
		Valid = (WayPointCalc[Task[ActiveWayPoint].Index].NextETE < 0.9*ERROR_TIME);
	} else
		Valid=false;
#endif
	break;

  case 45:
    SecsToDisplayTime((int)(CALCULATED_INFO.TaskTimeToGo+DetectCurrentTime()));
    Valid = ValidTaskPoint(ActiveWayPoint)
      && (CALCULATED_INFO.TaskTimeToGo< 0.9*ERROR_TIME);
    break;
  case 62:
    if (AATEnabled && ValidTaskPoint(ActiveWayPoint)) {
      SecsToDisplayTime((int)dd);
      Valid = (dd< 0.9*ERROR_TIME);
    } else {
      SecsToDisplayTime(0);
      Valid = false;
    }
    break;
  default:
    break;
  }
}

// TODO enhancement: crop long text or provide alternate
// e.g. 10300 ft ==> 10,3
// e.g. "Ardlethan" => "Ardl."

void InfoBoxFormatter::AssignValue(int i) {
int index;
  switch (i) {
  case 0:
    Value = ALTITUDEMODIFY*CALCULATED_INFO.NavAltitude;
    break;
  case 1:
    Value = ALTITUDEMODIFY*CALCULATED_INFO.AltitudeAGL  ;
    Valid = CALCULATED_INFO.TerrainValid;
    break;
  case 2:
    Value = LIFTMODIFY*CALCULATED_INFO.Average30s;
    break;
  case 3:
    Value = CALCULATED_INFO.WaypointBearing;
    Valid = CALCULATED_INFO.WaypointDistance > 10.0;
    break;
  case 4:
    if (CALCULATED_INFO.LD== 999) {
      Valid = false;
    } else {
      Valid = true;
      Value = CALCULATED_INFO.LD; 
    }
    break;
  case 5:
    if (CALCULATED_INFO.CruiseLD== 999) {
      Valid = false;
    } else {
      Valid = true;
      Value = CALCULATED_INFO.CruiseLD; 
    }
    break;
  case 6:
    Value = SPEEDMODIFY*GPS_INFO.Speed;
    break;
  case 7:
    Value = LIFTMODIFY*CALCULATED_INFO.LastThermalAverage;
    break;
  case 8:
    Value = ALTITUDEMODIFY*CALCULATED_INFO.LastThermalGain;
    break;
  case 10:    
    Value = iround(LIFTMODIFY*MACCREADY*10)/10.0;
    break;
  case 11:
    Value = DISTANCEMODIFY*CALCULATED_INFO.WaypointDistance;
    Valid = ValidTaskPoint(ActiveWayPoint);
    break;
  case 12:
    Value = ALTITUDEMODIFY*WayPointCalc[Task[ActiveWayPoint].Index].AltArriv[AltArrivMode];
    Valid = ValidTaskPoint(ActiveWayPoint);
    break;
  case 13:
    Value = ALTITUDEMODIFY*WayPointCalc[Task[ActiveWayPoint].Index].AltReqd[AltArrivMode];
    Valid = ValidTaskPoint(ActiveWayPoint);
    break;
  case 14:
    Value = 0; // Next Waypoint Text
    break;
  case 15:
    Value = ALTITUDEMODIFY*CALCULATED_INFO.TaskAltitudeDifference;
    Valid = ValidTaskPoint(ActiveWayPoint);
    break;
  case 16:
    Value = ALTITUDEMODIFY*CALCULATED_INFO.TaskAltitudeRequired;
    Valid = ValidTaskPoint(ActiveWayPoint);
    break;
  case 17:
    Value = TASKSPEEDMODIFY*CALCULATED_INFO.TaskSpeed;
    if (ActiveWayPoint>=1) {
      Valid = ValidTaskPoint(ActiveWayPoint);
    } else {
      Valid = false;
    }
    break;
  case 18:
    if (CALCULATED_INFO.ValidFinish) {
      Value = DISTANCEMODIFY*CALCULATED_INFO.WaypointDistance;
    } else {
      Value = DISTANCEMODIFY*CALCULATED_INFO.TaskDistanceToGo; 
    }
    Valid = ValidTaskPoint(ActiveWayPoint);
    break;
  case 19:
    //if (CALCULATED_INFO.LDFinish== 999) {
    if (CALCULATED_INFO.LDFinish>ALTERNATE_MAXVALIDGR) {
      Valid = false;
    } else {
      Valid = ValidTaskPoint(ActiveWayPoint);
      if (CALCULATED_INFO.ValidFinish) {
        Value = 0;
      } else {
        Value = CALCULATED_INFO.LDFinish; 
      }
    }
    break;
  case 20:
    Value = ALTITUDEMODIFY*CALCULATED_INFO.TerrainAlt ;
    Valid = CALCULATED_INFO.TerrainValid;
    break;
  case 21:
    Value = LIFTMODIFY*CALCULATED_INFO.AverageThermal;
    break;
  case 22:
    Value = ALTITUDEMODIFY*CALCULATED_INFO.ThermalGain;
    break;
  case 23:
    Value = GPS_INFO.TrackBearing;
    break;
  case 24:
    if (GPS_INFO.VarioAvailable) {
      Value = LIFTMODIFY*GPS_INFO.Vario;
    } else {
      Value = LIFTMODIFY*CALCULATED_INFO.Vario;
    }
    break;
  case 25:
    Value = SPEEDMODIFY*CALCULATED_INFO.WindSpeed;
    break;
  case 26:
    Value = CALCULATED_INFO.WindBearing;
    break;
  case 28:
    Value = DISTANCEMODIFY*CALCULATED_INFO.AATMaxDistance ; 
    Valid = ValidTaskPoint(ActiveWayPoint) && AATEnabled;
    break;
  case 29:
    Value = DISTANCEMODIFY*CALCULATED_INFO.AATMinDistance ; 
    Valid = ValidTaskPoint(ActiveWayPoint) && AATEnabled;
    break;
  case 30:
    Value = TASKSPEEDMODIFY*CALCULATED_INFO.AATMaxSpeed;
    Valid = ValidTaskPoint(ActiveWayPoint) && AATEnabled;
    if (CALCULATED_INFO.AATTimeToGo<1) {
      Valid = false;
    }
    break;
  case 31:
    Value = TASKSPEEDMODIFY*CALCULATED_INFO.AATMinSpeed;
    Valid = ValidTaskPoint(ActiveWayPoint) && AATEnabled;
    if (CALCULATED_INFO.AATTimeToGo<1) {
      Valid = false;
    }
    break;
  case 32:
    Valid = GPS_INFO.AirspeedAvailable;
    Value = SPEEDMODIFY*GPS_INFO.IndicatedAirspeed;
    break;
  case 33:
    Valid = GPS_INFO.BaroAltitudeAvailable;
    Value = ALTITUDEMODIFY*GPS_INFO.BaroAltitude;
    break;
  case 34:
    Value = SPEEDMODIFY*CALCULATED_INFO.VMacCready; 
    break;
  case 35:
    Value = CALCULATED_INFO.PercentCircling;
    break;
  case 37:
    Valid = GPS_INFO.AccelerationAvailable;
    Value = GPS_INFO.Gload;
    break;
  case 38:
    // if (CALCULATED_INFO.LDNext== 999) {
    if (CALCULATED_INFO.LDNext >ALTERNATE_MAXVALIDGR) {
      Valid = false;
    } else {
      Valid = ValidTaskPoint(ActiveWayPoint);
      Value = CALCULATED_INFO.LDNext;
    }
    break;
  case 43:
    //    Valid = GPS_INFO.AirspeedAvailable;
    Value = CALCULATED_INFO.VOpt*SPEEDMODIFY;
    if ( Value <-999 || Value >999) Valid=false; // 091214 FIX
    break;
  case 44:
    //    Valid = GPS_INFO.AirspeedAvailable;
    Value = CALCULATED_INFO.NettoVario*LIFTMODIFY;
    if (Value<-999 || Value>999) Valid=false; // 091214 FIX
    break;
  case 48:
    Value = GPS_INFO.OutsideAirTemperature;
    break;
  case 49:
    Value = GPS_INFO.RelativeHumidity;
    break;
  case 50:
    Value = CuSonde::maxGroundTemperature;
    break;
  case 51:
    Value = DISTANCEMODIFY*CALCULATED_INFO.AATTargetDistance ; 
    Valid = ValidTaskPoint(ActiveWayPoint) && AATEnabled;
    break;
  case 52:
    Value = TASKSPEEDMODIFY*CALCULATED_INFO.AATTargetSpeed;
    Valid = ValidTaskPoint(ActiveWayPoint) && AATEnabled;
    if (CALCULATED_INFO.AATTimeToGo<1) {
      Valid = false;
    }
    break;
  case 53:
    //if (CALCULATED_INFO.LDvario== 999) {
    if (CALCULATED_INFO.LDvario>ALTERNATE_MAXVALIDGR) {
      Valid = false;
    } else {
      Valid = GPS_INFO.VarioAvailable && GPS_INFO.AirspeedAvailable;
      Value = CALCULATED_INFO.LDvario; 
    }
    break;
  case 54:
    Valid = GPS_INFO.AirspeedAvailable;
    Value = SPEEDMODIFY*GPS_INFO.TrueAirspeed;
    break;
  case 56: // team bearing
    Value = CALCULATED_INFO.TeammateBearing;
    Valid = true;
  case 58: // team range
	  if (TeammateCodeValid)
	  {
    Value = DISTANCEMODIFY*CALCULATED_INFO.TeammateRange;
    if (Value > 100)
      {
        _tcscpy(Format, _T("%.0lf")); 
      }
    else
      {
        _tcscpy(Format, _T("%.1lf"));	
      }
    Valid = true;
	  }
	  else
	  {
		  Valid = false;
	  }
    break;
  case 59:
    Value = TASKSPEEDMODIFY*CALCULATED_INFO.TaskSpeedInstantaneous;
    if (ActiveWayPoint>=1) {
      Valid = ValidTaskPoint(ActiveWayPoint);
    } else {
      Valid = false;
    }
    break;
  case 60:
    Value = DISTANCEMODIFY*CALCULATED_INFO.HomeDistance ; 
    if (HomeWaypoint>=0) {
      Valid = ValidWayPoint(HomeWaypoint);
    } else {
      Valid = false;
    }
    break;
  case 61:
    Value = TASKSPEEDMODIFY*CALCULATED_INFO.TaskSpeedAchieved;
    if (ActiveWayPoint>=1) {
      Valid = ValidTaskPoint(ActiveWayPoint);
    } else {
      Valid = false;
    }
    break;
  case 63:
    if (CALCULATED_INFO.timeCircling>0) {
      Value = LIFTMODIFY*CALCULATED_INFO.TotalHeightClimb
        /CALCULATED_INFO.timeCircling;
      Valid = true;
    } else {
      Value = 0.0;
      Valid = false;
    }
    break;
  case 64:
    Value = LIFTMODIFY*CALCULATED_INFO.DistanceVario;
    if (ActiveWayPoint>=1) {
      Valid = ValidTaskPoint(ActiveWayPoint);
    } else {
      Valid = false;
    }
    break;
  case 65: // battery voltage
#if (WINDOWSPC<1)
#ifndef GNAV
   #if 100228
   if (PDABatteryFlag==BATTERY_FLAG_CHARGING || PDABatteryStatus==AC_LINE_ONLINE) {
	_tcscpy(Format,TEXT("%2.0f%%C"));  // 100228
   } else {
	_tcscpy(Format,TEXT("%2.0f%%D"));  // 100228
   }
   Value = PDABatteryPercent;
   Valid = true;
   #else
   Value = PDABatteryPercent;
   Valid = true;
   #endif
#else
    Value = GPS_INFO.SupplyBatteryVoltage;
    if (Value>0.0) {
      Valid = true;
    } else {
      Valid = false;
    }
#endif
#else	// on PC no battery value
    Value = 0.0;
    Valid = false;
#endif
    break;
  case 66: // VENTA-ADDON added GR Final
    //if (CALCULATED_INFO.GRFinish== 999) {
    if (CALCULATED_INFO.GRFinish>ALTERNATE_MAXVALIDGR) {
      Valid = false;
    } else {
      Valid = ValidTaskPoint(ActiveWayPoint);
      if (CALCULATED_INFO.ValidFinish) {
	Value = 0;
      } else {
	Value = CALCULATED_INFO.GRFinish;
	if (Value >100 )
	  {
	    _tcscpy(Format, _T("%1.0f"));
	  }
	else
	  {
	    _tcscpy(Format, _T("%1.1f"));
	  }
      }
    }
    break;
  case 70:	// VENTA3 QFE
//    Valid = GPS_INFO.Altitude;
    Value = ALTITUDEMODIFY* (GPS_INFO.Altitude-QFEAltitudeOffset);
    if (Value<0 || Value>999) Valid=false; // 091214 FIX
    break;
  case 71:
    if ( CALCULATED_INFO.AverageLD == 0) {
      Valid = false;
    } else {
      Valid = true;
      Value = CALCULATED_INFO.AverageLD; 
      if (Value<0)
	    _tcscpy(Format, _T("^^^"));
      else if (Value>=999) 
	    _tcscpy(Format, _T("+++"));
      else
	    _tcscpy(Format, _T("%2.0f"));
	
    }
    break;
  case 72:
    Valid=false;
    if ( ValidTaskPoint(ActiveWayPoint) != false ) {
        index = Task[ActiveWayPoint].Index;
        if (index>=0) {
                Value=WayPointCalc[index].GR;
    		if (Value >0 && Value<ALTERNATE_MAXVALIDGR) Valid = true;
		if (Value >100 ) _tcscpy(Format, _T("%1.0f"));
			else _tcscpy(Format, _T("%1.1f"));
	}
    }
    break;

  case 73:
	Value=(TOFEET*CALCULATED_INFO.NavAltitude)/100.0;
	Valid=true;
	_tcscpy(Format, _T("%.0f"));
    break;

  case 78:
        Value = CALCULATED_INFO.HomeRadial;
        Valid = (CALCULATED_INFO.HomeDistance > 10.0);
	//_tcscpy(Format, _T("%.0f")); 
	break;

  case 74: // distance flown
    if (CALCULATED_INFO.TaskDistanceCovered != 0)
      {
	Value = DISTANCEMODIFY*CALCULATED_INFO.TaskDistanceCovered;
	Valid = true;
      }
    else
      {
	Value = 0.0;
	Valid = false;
      }
    break;
  case LK_AIRSPACEDIST:
	if (NearestAirspaceHDist >0) {
        	Value = DISTANCEMODIFY*NearestAirspaceHDist;
       		Valid = true;
		_tcscpy(Format, _T("%1.1f")); 
	} else {
		Valid=false;
		Value = -1;
	}
	break;
  case LK_EXTBATTBANK:
	if (GPS_INFO.ExtBatt_Bank >0) {
		Value=GPS_INFO.ExtBatt_Bank;
		Valid=true;
		_tcscpy(Format, _T("%1.0d")); 
	} else {
		Valid=false;
		Value = -1;
	}
	break;
  case LK_EXTBATT1VOLT:
	if (GPS_INFO.ExtBatt1_Voltage >0) {
		Value=GPS_INFO.ExtBatt1_Voltage;
		Valid=true;
	} else {
		Valid=false;
		Value = -1;
	}
	break;
  case LK_EXTBATT2VOLT:
	if (GPS_INFO.ExtBatt2_Voltage >0) {
		Value=GPS_INFO.ExtBatt2_Voltage;
		Valid=true;
	} else {
		Valid=false;
		Value = -1;
	}
	break;
  case LK_ODOMETER:
    Value = DISTANCEMODIFY*CALCULATED_INFO.Odometer; 
    if (CALCULATED_INFO.Odometer>0) {
	if (Value >=100 ) _tcscpy(Format, _T("%1.0f"));
		else _tcscpy(Format, _T("%1.1f"));
      	Valid = true;
    } else {
      Valid = false;
    }
    break;
  case LK_AQNH: // 100126
    if (ALTITUDEMODIFY==TOFEET)
 	   Value = TOMETER*GPS_INFO.Altitude;
    else
 	   Value = TOFEET*GPS_INFO.Altitude;
    break;
  case LK_AALTAGL: // 100126
    if (ALTITUDEMODIFY==TOFEET)
    	Value = TOMETER*CALCULATED_INFO.AltitudeAGL  ;
    else
    	Value = TOFEET*CALCULATED_INFO.AltitudeAGL  ;
    Valid = CALCULATED_INFO.TerrainValid;
    break;
  case LK_HGPS:
    Value = ALTITUDEMODIFY*GPS_INFO.Altitude; 
    break;
  case LK_EQMC:    
	if (CALCULATED_INFO.EqMc <0) {
		Value=0;
		Valid=false;
	} else {
		Value = iround(LIFTMODIFY*CALCULATED_INFO.EqMc*10)/10.0;
		Valid=true;
	}
    break;
/* REMOVE
  case 75:
	Valid=false;
	if ( ValidWayPoint(Alternate1)) {
              Value=ALTITUDEMODIFY*WayPointCalc[Alternate1].AltArriv;
              if ( (Value >ALTDIFFLIMIT) && (Value <9999) ) Valid=true;
	}
    break;
  case 76:
	Valid=false;
	if ( ValidWayPoint(Alternate2)) {
              Value=ALTITUDEMODIFY*WayPointCalc[Alternate2].AltArriv;
              if ( (Value >ALTDIFFLIMIT) && (Value <9999) ) Valid=true;
	}
    break;
  case 77:
	Valid=false;
	if ( ValidWayPoint(BestAlternate)) {
              Value=ALTITUDEMODIFY*WayPointCalc[BestAlternate].AltArriv;
              if ( (Value >ALTDIFFLIMIT) && (Value <9999) ) Valid=true;
	}
    break;
*/

/*
  case xx: // termik liga points
    if (CALCULATED_INFO.TermikLigaPoints != 0)
      {
	Value = CALCULATED_INFO.TermikLigaPoints;
	Valid = true;
      }
    else
      {
	Value = 0.0;
	Valid = false;
      }
    break;
*/

  case LK_EXP1:
        Value = Experimental1/1000;
        Valid = true;
	_tcscpy(Format, _T("%1.1f")); 
	break;
  case LK_EXP2: 
        Value = Experimental2;
        Valid = true;
	_tcscpy(Format, _T("%+1.1f")); 
	break;
  default:
    break;
  };
}


TCHAR *InfoBoxFormatter::GetCommentText(void) {
  return CommentText;
}

BOOL InfoBoxFormatter::isValid(void) {
  return Valid;
}

void InfoBoxFormatter::RenderInvalid(int *color) {
  _stprintf(CommentText,TEXT(""));
  _stprintf(Text,TEXT("---"));
  *color = -1;
}


TCHAR *InfoBoxFormatter::Render(int *color) {
  if (Valid) {
    _stprintf(Text,
              Format,
              Value );
    *color = 0;
  } else {
    RenderInvalid(color);
  }
  return(Text);
}

TCHAR *InfoBoxFormatter::RenderTitle(int *color) { // VENTA3
  if (Valid) {
    _stprintf(Text,
              Format,
              Value );
    *color = 0;
  } else {
    RenderInvalid(color);
  }
  return(Text);
}

TCHAR *FormatterLowWarning::Render(int *color) {

  if (Valid) {
    _stprintf(Text,
              Format,
              Value );
    if (Value<minimum) {
      *color = 1; // red
    } else {
      *color = 0;
    }
  } else {
    RenderInvalid(color);
  }
  return(Text);
}


TCHAR *FormatterTime::Render(int *color) {
  if (!Valid) {
    RenderInvalid(color);
    _stprintf(Text,TEXT("--:--"));
  } else {
    if ((hours<0) || (mins<0) || (seconds<0)) {
      // Time is negative
      *color = 1; // red!
      if (hours<0) { // hh:mm, ss
        _stprintf(Text,
                  TEXT("%02d:%02d"),
                  hours, mins );
        _stprintf(CommentText,
                  TEXT("%02d"),
                  seconds);
      } else if (mins<0) { // mm:ss
        _stprintf(Text,
                  TEXT("%02d:%02d"),
                  mins, seconds );
        _stprintf(CommentText,
                  TEXT(""));
      } else {
        _stprintf(Text,
                  TEXT("-00:%02d"),
                  abs(seconds));
        _stprintf(CommentText,
                  TEXT(""));
      }
    } else {
      // Time is positive
      *color = 0; // black
      if (hours>0) { // hh:mm, ss
        _stprintf(Text,
                  TEXT("%02d:%02d"),
                  hours, mins );
        _stprintf(CommentText,
                  TEXT("%02d"),
                  seconds);
      } else { // mm:ss
        _stprintf(Text,
                  TEXT("%02d:%02d"),
                  mins, seconds );
        _stprintf(CommentText,
                  TEXT(""));
      }
    }
  }
  return(Text);
}


TCHAR *FormatterAATTime::Render(int *color) {
  if (!Valid) {
    RenderInvalid(color);
    _stprintf(Text,TEXT("--:--"));
  } else {

    *color = status;

    if ((hours<0) || (mins<0) || (seconds<0)) {
      // Time is negative
      if (hours<0) { // hh:mm, ss
        _stprintf(Text,
                  TEXT("%02d:%02d"),
                  hours, mins );
        _stprintf(CommentText,
                  TEXT("%02d"),
                  seconds);
      } else if (mins<0) { // mm:ss
        _stprintf(Text,
                  TEXT("%02d:%02d"),
                  mins, seconds );
        _stprintf(CommentText,
                  TEXT(""));
      } else {
        _stprintf(Text,
                  TEXT("-00:%02d"),
                  abs(seconds));
        _stprintf(CommentText,
                  TEXT(""));
      }
    } else {
      // Time is positive
      if (hours>0) { // hh:mm, ss
        _stprintf(Text,
                  TEXT("%02d:%02d"),
                  hours, mins );
        _stprintf(CommentText,
                  TEXT("%02d"),
                  seconds);
      } else { // mm:ss
        _stprintf(Text,
                  TEXT("%02d:%02d"),
                  mins, seconds );
        _stprintf(CommentText,
                  TEXT(""));
      }
    }
  }
  return(Text);
}


TCHAR *FormatterWaypoint::Render(int *color) {
  int thewaypoint = ActiveWayPoint;
  LockTaskData();
  if(ValidTaskPoint(thewaypoint))
    {
      int index = Task[thewaypoint].Index;
      if ((index>=0) && (WayPointList[index].Reachable)) {
	*color = 2; // blue text
      } else {
	*color = 0; // black text
      }
      if ( DisplayTextType == DISPLAYFIRSTTHREE)
        {
          _tcsncpy(Text,WayPointList[index].Name,3);
          Text[3] = '\0';
        }
      else if( DisplayTextType == DISPLAYNUMBER)
        {
          _stprintf(Text,TEXT("%d"),
		    WayPointList[index].Number );
        }
      else
        {
          _tcsncpy(Text,WayPointList[index].Name,
                   (sizeof(Text)/sizeof(TCHAR))-1);
          Text[(sizeof(Text)/sizeof(TCHAR))-1] = '\0';
        }
    }
  else
    {
      Valid = false;
      RenderInvalid(color);
    }
  UnlockTaskData();

  return(Text);
}

// VENTA3 Alternate destinations
TCHAR *FormatterAlternate::RenderTitle(int *color) {
  
  LockTaskData();
  if(ValidWayPoint(ActiveAlternate))
    {
      if ( DisplayTextType == DISPLAYFIRSTTHREE)
        {
          _tcsncpy(Text,WayPointList[ActiveAlternate].Name,3);
          Text[3] = '\0';
        }
      else if( DisplayTextType == DISPLAYNUMBER)
        {
          _stprintf(Text,TEXT("%d"),
		    WayPointList[ActiveAlternate].Number );
        }
      else
        {
          _tcsncpy(Text,WayPointList[ActiveAlternate].Name,
                   (sizeof(Text)/sizeof(TCHAR))-1);
          Text[(sizeof(Text)/sizeof(TCHAR))-1] = '\0';
        }
    }
  else
    {
      Valid = false;
      RenderInvalid(color);
    }
  UnlockTaskData();

  return(Text);
}

/*
 * Currently even if set for FIVV, colors are not used.
 */
TCHAR *FormatterAlternate::Render(int *color) {
 //int active=ActiveAlternate; REMOVE
  LockTaskData();
  if(Valid && ValidWayPoint(ActiveAlternate)) {
	switch (WayPointCalc[ActiveAlternate].VGR ) {
		case 0:
			// impossible, give a magenta debug color;
			*color = 5; 
			break;
		case 1:
#ifdef FIVV
			*color = 0; // green
#else
			*color = 0; // blue
#endif
			break;
		case 2:
#ifdef FIVV
			*color = 0; // yellow 4
#else
			*color = 0; // normale white
#endif
			break;
		case 3:
			*color = 1; // red
			break;
		default:
			// even more impossible, give debug color magenta
			*color = 5;
			break;
	}

//	Value=WayPointCalc[ActiveAlternate].GR;    BUGFIX 090918

	_stprintf(Text,Format,Value);
  } else {
	Valid = false;
	RenderInvalid(color);
  }
   UnlockTaskData();
   return(Text);
}

// I know all of this is terrible, but no time to clean it
void FormatterAlternate::AssignValue(int i) { 
  LockTaskData();
   switch (i) {
	case 67:
		if (OnAlternate1 == false ) { // first run, activate calculations
			OnAlternate1 = true;	
        		Value=INVALID_GR;
		} else {
			if ( ValidWayPoint(Alternate1) ) Value=WayPointCalc[Alternate1].GR; 
			else Value=INVALID_GR;
		}
		break;
	case 75:
		if (OnAlternate1 == false ) {
			OnAlternate1 = true;	
        		Value=INVALID_DIFF; 
		} else {
			if ( ValidWayPoint(Alternate1) ) Value=WayPointCalc[Alternate1].AltArriv[AltArrivMode]; 
			else Value=INVALID_DIFF;
		}
		break;
	case 68:
		if (OnAlternate2 == false ) { 
			OnAlternate2 = true;	
        		Value=INVALID_GR;
		} else {
			if ( ValidWayPoint(Alternate2) ) Value=WayPointCalc[Alternate2].GR; 
			else Value=INVALID_GR;
		}
		break;
	case 76:
		if (OnAlternate2 == false ) {
			OnAlternate2 = true;	
        		Value=INVALID_DIFF; 
		} else {
			if ( ValidWayPoint(Alternate2) ) Value=WayPointCalc[Alternate2].AltArriv[AltArrivMode]; 
			else Value=INVALID_DIFF;
		}
		break;
	case 69:
		if (OnBestAlternate == false ) { // first run, waiting for slowcalculation loop
			OnBestAlternate = true;		// activate it
        		Value=INVALID_GR;
		} else {
			if ( ValidWayPoint(BestAlternate)) Value=WayPointCalc[BestAlternate].GR;
			else Value=INVALID_GR;
		}
		break;
	case 77:
		if (OnBestAlternate == false ) { 
			OnBestAlternate = true;	
        		Value=INVALID_DIFF;
		} else {
			if ( ValidWayPoint(BestAlternate) ) Value=WayPointCalc[BestAlternate].AltArriv[AltArrivMode]; 
			else Value=INVALID_DIFF;
		}
		break;
	default:
		Value=66.6; // something evil to notice..
		break;
   }
 
   Valid=false;
   if (i>= 75) { 
	// arrival altitude values, ALTDIFFLIMIT is GREATER than INVALID_DIFF
	if (Value >ALTDIFFLIMIT) {
		Valid=true;
		_tcscpy(Format, _T("%+2.0f"));
	} else {
		_tcscpy(Format, _T("%+2.0f"));
	}
   } else {
	// GR values, ALTERNATE_MAXGR is ok because INVALID_GR is greater
	   if ((Value < ALTERNATE_MAXVALIDGR) && (Value>0)) {
		Valid = true;
		if (Value >= 100 )
		  {
		    _tcscpy(Format, _T("%1.0f"));
		  }
		else
		  {
		    _tcscpy(Format, _T("%1.1f"));
		  }
	   } 
  }
 
   
   UnlockTaskData();
}

TCHAR *FormatterDiffBearing::Render(int *color) {

  if (ValidTaskPoint(ActiveWayPoint) 
      && CALCULATED_INFO.WaypointDistance > 10.0) {
    Valid = true;

    Value = CALCULATED_INFO.WaypointBearing -  GPS_INFO.TrackBearing;

    if (Value < -180.0)
      Value += 360.0;
    else
    if (Value > 180.0)
      Value -= 360.0;

#ifndef __MINGW32__
    if (Value > 1)
      _stprintf(Text, TEXT("%2.0f°»"), Value);
    else if (Value < -1)
      _stprintf(Text, TEXT("«%2.0f°"), -Value);
    else
      _tcscpy(Text, TEXT("«»"));
#else
    if (Value > 1)
      _stprintf(Text, TEXT("%2.0fÂ°Â»"), Value);
    else if (Value < -1)
      _stprintf(Text, TEXT("Â«%2.0fÂ°"), -Value);
    else
      _tcscpy(Text, TEXT("Â«Â»"));
#endif
    *color = 0;
  } else {
    Valid = false;
    RenderInvalid(color);
  }

  return(Text);
}




TCHAR *FormatterTeamCode::Render(int *color) {

  if(ValidWayPoint(TeamCodeRefWaypoint))
    {
      *color = 0; // black text
       _tcsncpy(Text,CALCULATED_INFO.OwnTeamCode,5);
       Text[5] = '\0';
    }
  else
    {
      RenderInvalid(color);
    }

  return(Text);
}


TCHAR *FormatterDiffTeamBearing::Render(int *color) {

  if(ValidWayPoint(TeamCodeRefWaypoint) && TeammateCodeValid) {
    Valid = true;
    
    Value = CALCULATED_INFO.TeammateBearing -  GPS_INFO.TrackBearing;
    
    if (Value < -180.0)
      Value += 360.0;
    else
      if (Value > 180.0)
        Value -= 360.0;
    
#ifndef __MINGW32__
    if (Value > 1)
      _stprintf(Text, TEXT("%2.0f°»"), Value);
    else if (Value < -1)
      _stprintf(Text, TEXT("«%2.0f°"), -Value);
    else
      _tcscpy(Text, TEXT("«»"));
#else
    if (Value > 1)
      _stprintf(Text, TEXT("%2.0fÂ°Â»"), Value);
    else if (Value < -1)
      _stprintf(Text, TEXT("Â«%2.0fÂ°"), -Value);
    else
      _tcscpy(Text, TEXT("Â«Â»"));
#endif
    *color = 0;
    
  } else {
    Valid = false;
    RenderInvalid(color);
  }
  
  return(Text);
}


/*

if ((Calculated->FinalGlide) && (Calculated->Circling) && (Calculated->AverageThermal>0)) {
}
*/



InfoBoxFormatter::InfoBoxFormatter(TCHAR *theformat) {
  _tcscpy(Format, theformat);
  Valid = TRUE;
  Value = 0.0;
  Text[0] = 0;
  CommentText[0] = 0;
}
