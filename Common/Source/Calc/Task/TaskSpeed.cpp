/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "CalcTask.h"
#include "McReady.h"



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

  if (!ValidTaskPoint(ActiveTaskPoint)) return;
  if (Calculated->ValidFinish) return;
  if (!Calculated->Flying) return;

  // in case we leave early due to error
  Calculated->TaskSpeedAchieved = 0;
  Calculated->TaskSpeed = 0;

  if (ActiveTaskPoint<=0) { // no task speed before start
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

    BUGSTOP_LKASSERT(h0!=0);
    if (h0==0) h0=1;
    dFinal = min(dr, d0*min(1.0,max(0.0,h1/h0)));

    if (Calculated->ValidFinish) {
      dFinal = 0;
    }


    // actual task speed achieved so far
    v1 = d1/t1;
#define OLDTASKSPEED
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
    double dc = max(0.0, dr-dFinal);
    // amount of extra distance to travel in cruise/climb before final glide

    LKASSERT((t1+dc/v1+dFinal/Vfinal)!=0);
    LKASSERT((t1+dFinal/Vfinal)!=0);
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

      BUGSTOP_LKASSERT(dt!=0);
      if (dt==0) dt=1;
      double vthis = (Calculated->LegDistanceCovered-dr_last)/dt;
      vthis = IndicatedAirSpeed(vthis, QNHAltitudeToQNEAltitude(Basic->Altitude));

      dr_last = Calculated->LegDistanceCovered;
      double ttg = max(1.0, Calculated->LegTimeToGo);
      //      double Vav = d0/max(1.0,t0); 
      double Vrem = Calculated->LegDistanceToGo/ttg;
      double Vref = // Vav;
	Vrem;
      double sr = -GlidePolar::SinkRate(Vstar);
      double height_diff = max(0.0, -Calculated->TaskAltitudeDifference);
      
      if (Calculated->timeCircling>30) {
	mc_safe = max(mc_safe, 
		      Calculated->TotalHeightClimb/Calculated->timeCircling);
      }
      // circling percentage during cruise/climb
      double rho_cruise = max(0.0,min(1.0,mc_safe/(sr+mc_safe)));
      double rho_climb = 1.0-rho_cruise;
      BUGSTOP_LKASSERT(mc_safe!=0);
      if (mc_safe==0) mc_safe=0.1;
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

      LKASSERT(mc_safe!=0);
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
        if ((ActiveTaskPoint==lastActiveWayPoint) 
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
        lastActiveWayPoint = ActiveTaskPoint;
      }
    }
  }
 OnExit:
  UnlockTaskData();

}
