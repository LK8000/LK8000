/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: McReady.cpp,v 8.3 2010/12/12 15:45:59 root Exp root $
*/

#include "StdAfx.h"
#include "options.h"
#include "McReady.h"
#include "externs.h"

#include "XCSoar.h"
#include "device.h"
#include "Utils2.h"

#include <math.h>
#include <windows.h>

double GlidePolar::RiskGamma = 0.0;
double GlidePolar::polar_a;
double GlidePolar::polar_b;
double GlidePolar::polar_c;
int GlidePolar::Vminsink = 2;
int GlidePolar::Vbestld = 2;
double GlidePolar::sinkratecache[MAXSPEED+1]; // this is in m/s !!
double GlidePolar::bestld = 0.0;
double GlidePolar::minsink = 10000.0;
double GlidePolar::BallastLitres = 0.0;
double GlidePolar::WingArea = 0.0;
double GlidePolar::WingLoading = 0.0;
double GlidePolar::WeightOffset = 0.0; 

double GlidePolar::SafetyMacCready= 0.5;
bool GlidePolar::AbortSafetyUseCurrent = false;

static int iSAFETYSPEED=0;

double GlidePolar::AbortSafetyMacCready() {
#if (1)
  if (AbortSafetyUseCurrent) {
    return MACCREADY;
  } else {
    return SafetyMacCready;
  }
#else
	switch(AltArrivMode) {
		case ALTA_MC:
			return MACCREADY;
		case ALTA_MC0:
			return 0;
		case ALTA_SMC:
			return SafetyMacCready;
		default:
			return 9; // something to notice!
	}
#endif
}

// GetAUW is returning gross weight of glider, with pilot and current ballast. 
// We now also add the offset to match chosen wing loading, just like a non-dumpable ballast
double GlidePolar::GetAUW() {
  return BallastLitres + WEIGHTS[0] + WEIGHTS[1] + GlidePolar::WeightOffset;
}

void GlidePolar::SetBallast() {
  LockFlightData();
  double BallastWeight;
  BallastLitres = WEIGHTS[2] * BALLAST;
  BallastWeight = GetAUW();
  if (WingArea>0.1) {
    WingLoading = BallastWeight/WingArea; 
  } else {
    WingLoading = 0;
  }
  BallastWeight = (double)sqrt(BallastWeight);
  double bugfactor = 1.0/BUGS;
  polar_a = POLAR[0] / BallastWeight*bugfactor;
  polar_b = POLAR[1] * bugfactor;
  polar_c = POLAR[2] * BallastWeight*bugfactor;

  // do preliminary scan to find min sink and best LD
  // this speeds up mcready calculations because we have a reduced range
  // to search across.
  // this also limits speed to fly to logical values (will never try
  // to fly slower than min sink speed)

  minsink = 10000.0;
  bestld = 0.0;
  int i;

  // Rounding errors could make SAFTEYSPEED 0.00xx and not 0
  // Now below 3kmh we consider the speed wrong
  // MAXSAFETYSPEED WAS 200, = 720kmh!!
  if ((SAFTEYSPEED<1)||(SAFTEYSPEED>=MAXSAFETYSPEED)) {
    SAFTEYSPEED=MAXSAFETYSPEED-1;
  }
  iSAFETYSPEED=(int)SAFTEYSPEED;

  // sinkratecache is an array for m/s values!! i is the speed in m/s
  for(i=4;i<=MAXSPEED;i++)
    {
      double vtrack = (double)i; // TAS along bearing in cruise
      double thesinkrate 
        =  -SinkRate(polar_a,polar_b,polar_c,0,0,vtrack);

      double ld = vtrack/thesinkrate;
      if (ld>=bestld) {
        bestld = ld;
        Vbestld = i;
      }
      if (thesinkrate<= minsink) {
        minsink = thesinkrate;
        Vminsink = i;
      }
      sinkratecache[i] = -thesinkrate;

    }
  UnlockFlightData();

  int polar_ai = iround((polar_a*10)*4096);
  int polar_bi = iround((polar_b)*4096);
  int polar_ci = iround((polar_c/10)*4096);
  int minsinki = -iround(minsink*10);
  int vbestldi = iround(Vbestld*10);
  int bestldi = iround(bestld*10);

  if (GPS_INFO.VarioAvailable) {

    TCHAR nmeabuf[100];
    wsprintf(nmeabuf,TEXT("PDVGP,%d,%d,%d,%d,%d,%d,0"),
	     polar_ai,
	     polar_bi,
	     polar_ci,
	     minsinki,
	     vbestldi,
	     bestldi);

    VarioWriteNMEA(nmeabuf);
  }

}


inline double GlidePolar::_SinkRateFast(const double &MC, const int &v) {
  return sinkratecache[v]-MC;
}

double GlidePolar::SinkRateFast(const double &MC, const int &v) {
  return _SinkRateFast(MC, max(4,min(iSAFETYSPEED, v)));
}


double GlidePolar::SinkRate(double V) {

  return SinkRate(polar_a,polar_b,polar_c,0.0,0.0,V);

}


#define MIN_MACCREADY 0.000000000001


double GlidePolar::SinkRate(double V, double n) {
  double w0 = SinkRate(polar_a,polar_b,polar_c,0.0,0.0,V);
  n = max(0.1,fabs(n));
  //  double v1 = V/max(1,Vbestld);
  double v2 = Vbestld/max(Vbestld/2,V);
  return w0-(V/(2*bestld))* (n*n-1)*(v2*v2);
}


double GlidePolar::MaxOkaySink(
                          NMEA_INFO *Basic,
                          DERIVED_INFO *Calculated,
                          double DistToWP,
                          double BearingToWP,
                          double LastValue,
                          double ElevWP, // waypoint elevation (m MSL)
                          int WPindex)
  {
  // This function returns the maximum average sink (m/s) that can be
  // found on glide to the landable waypoint and still make it there
  // at the user-defined safety AGL altitude.
  // A positive returned value is sinking air.
  // This function will never return a negative value,
  // only values in the 0.0-5.0 range, even if the
  // target/destination is beyond reach or directly
  // below the aircraft.  5.0 m/s = ~1K fpm!

  int i;                 // "for" loop counter
  double AltReqd;        // altitude (m MSL) needed to reach target
  bool Improving = true; // True: ease of reaching target is improving
  int Increment = 1;     // step increment in "for" loop (1 or -1)
  double MacCready = GetMacCready(WPindex, 0);
  double SinkMax = 5.0;
  double SinkTest;
  double SinkStart = 0.1;

  // These calculations are somewhat complex (probably slow),
  // so we'll save time by starting with the
  // previously-stored value (LastValue).  Also,
  // we'll figure out whether the probability of being able
  // to reach the target is improving or not.  Then we should
  // be able to find the result in just 1-2 iterations.

  if (LastValue > 4.9) // can't be getting better (last value: 5.0)
    {
    Improving = false;
    SinkStart = 5.0;
    }
  else // last value 0.0-4.9
    {
    if (LastValue > 0.0) // could be betting better OR getting worse
      {
      AltReqd = MacCreadyAltitude(MacCready,
                                  DistToWP, 
                                  BearingToWP, 
                                  Calculated->WindSpeed, 
                                      Calculated->WindBearing,
                                  LastValue) 
                + SAFETYALTITUDEARRIVAL + ElevWP;

      if (Calculated->NavAltitude >= AltReqd)
        {
        if (CheckLandableReachableTerrainNew(Basic,
                                             Calculated,
                                             DistToWP,
                                             BearingToWP,
                                             LastValue))
          SinkStart = LastValue + 0.1;
        else
          {
          Improving = false;
          SinkStart = LastValue - 0.1;
          }
        }
      else // too low even w/o considering possible blocking terrain
        {
        Improving = false;
        SinkStart = LastValue - 0.1;
        }
      } // last value 0.1-4.9
    } // last value 0.0-4.9

  if (!Improving)
    {
    SinkMax = 0.0;
    Increment = -1;
    }

  // The following "for" loop steps through possible sink values
  // (in the 0-5 m/s range) in 0.1 m/s increments till
  // the solution is found.
  // The loop counter variable "i" is 10X the sink value.

  for (i = (int) (SinkStart * 10); (i>=0 && i<=50); i+=Increment)
    {
    SinkTest = i / 10.0;

    AltReqd = MacCreadyAltitude(MacCready,
                                DistToWP, 
                                BearingToWP, 
                                Calculated->WindSpeed, 
                                Calculated->WindBearing,
                                SinkTest) 
              + SAFETYALTITUDEARRIVAL + ElevWP;

    if ((Calculated->NavAltitude < AltReqd) && Improving)
      {
      // Stop looking.  We've found too high a sink value.
      SinkMax = SinkTest - 0.1;
      break;
      }

    // "if" high enough assuming flat terrain, then check to see
    // whether any terrain is in the way...

    if (Calculated->NavAltitude >= AltReqd)
      {
      // "if" no terrain is in the way
      if (CheckLandableReachableTerrainNew(Basic, Calculated,
                                           DistToWP, BearingToWP,
                                           SinkTest))
        {
        if (!Improving)
          {
          SinkMax = SinkTest;
          break;
          }
        }
      else // terrain is in the way
        {
        if (Improving)
          {
          SinkMax = SinkTest - 0.1;
          break; // too much sink!
          }
        }
      }
    } // "for" loop

  return round(max(0.0, SinkMax) * 10) / 10.0; // round to nearest 0.1
  }



int GlidePolar::GetSpeedToFly(
        double emcready,          // MC setting (m/s) to assume in use
        double Distance,          // to target (meters)
        double Bearing,           // to target (degrees)
        const double WindSpeed,
        const double WindBearing, 
                                              double *BestCruiseTrack,
                                              double *VMacCready, 
                                              const bool isFinalGlide,
                                              double *TimeToGo,
					      const double cruise_efficiency,
        double *TimeToDestCruise,
        double SinkAir) // magnitude of sink in air on glide.
                        // m/s, positive is sinking
  {
  int i;
  double BestSpeed;
  double BestGlideInv; // best "inverse" glide ratio found yet
  double GlideInv;     // "inverse" glide ratio
  double BestTime = 1e6;    // 1,000,000 (units?)  If seconds,
                            // this is 11.6 days!
  double CrossBearing;
  static double CrossBearingLast = -1.0;
  static double HeadWind, CrossWind=0.0;
  static double HeadWindSqd, CrossWindSqd=0.0;

  double sinkrate;
  double tcruise;   // fraction of total time spent in cruise
                    // (0-1) 0 = 0%, 1 = 100%
  double TimeToDestTotal = ERROR_TIME; // initialise to error value
  double vtot;      // ground speed squared, then ground speed
  static double WindSpeedLast = -1.0;

  // Calculate the speed that gives the best glide over the ground
  // to the target.

  SinkAir = min(0.0, SinkAir);  // negative (lift) not allowed

  // TODO accuracy: extensions to Mc to incorporate real-life issues
  // - [done] best cruise track and bearing (final glide and for waypoint)
  // - climb before or after turning waypoints.
  // - mcready ring changes with height allowing for risk and decreased rate
  // - cloud streets
  // - sink rate between thermals
  // - modify Vtrack for IAS

  //Calculate Best Glide Speed
  CrossBearing = AngleLimit360(Bearing - WindBearing);

  if ((CrossBearing != CrossBearingLast) || (WindSpeed != WindSpeedLast))
    {
    // saves a few floating point operations
    HeadWind  = WindSpeed * fastcosine(CrossBearing);
    CrossWind = WindSpeed * fastsine(CrossBearing);

    HeadWindSqd  = HeadWind * HeadWind;
    CrossWindSqd = CrossWind * CrossWind;

    // save current values as "last" values
    WindSpeedLast    = WindSpeed;
    CrossBearingLast = CrossBearing;
    }

  // Initialize variables, some to deliberately impossible values.

  BestSpeed = 2;
  BestGlideInv = 10000;
  if (BestCruiseTrack) {
    *BestCruiseTrack = Bearing;
  }

  // REWRITING DISTANCE!
  if (Distance<1.0) {
    Distance = 1;
  }

  for(i=Vminsink;i<iSAFETYSPEED;i++) {
    double vtrack_real = ((double)i); // actual airspeed
    double vtrack = vtrack_real*cruise_efficiency; 
    // TAS along bearing in cruise
	    
    // glide angle = velocity projected along path / sink rate
    // need to work out best velocity along path given wind vector
    // need to work out percent time spent cruising
    // SinkRate function returns negative value for sink

    if (isFinalGlide) {
      tcruise = 1.0;
      sinkrate = -_SinkRateFast((max(0.0,emcready) + SinkAir), i)
                 + SinkAir;
      }
    else {
	// WE ARE REWRITING EMCREADY!
      emcready = max(MIN_MACCREADY,emcready);
      sinkrate = -_SinkRateFast(0.0, i) + SinkAir;
      tcruise  = max(0.0,min(1.0,emcready/(sinkrate+emcready)));
    }

    // calculate average speed along track relative to wind
    vtot = (vtrack*vtrack * tcruise * tcruise - CrossWindSqd);
    // if able to advance against crosswind
    if (vtot>0) {
      // if able to advance against headwind
      if (vtot>HeadWindSqd) {
	// calculate average speed along track relative to ground
	vtot = sqrt(vtot)-HeadWind;
      } else {
	// can't advance at this speed
	continue;
      }
    }

    // can't advance at this speed
    if (vtot<=0) continue;
            
    bool KeepLooking = false;

    if (isFinalGlide) {
      // inverse glide ratio relative to ground
      GlideInv = sinkrate/vtot;

      // best glide angle when in final glide
      if (GlideInv <= BestGlideInv)  // better than or equal to last tested spd
        {
        KeepLooking     = true;
        BestGlideInv    = GlideInv;
        TimeToDestTotal = Distance/vtot;
      }
    } else {
      // time spent in cruise
      double Time_cruise = (tcruise/vtot)*Distance;
      double Time_climb = sinkrate*(Time_cruise/emcready);

      // total time to destination
      TimeToDestTotal = max(Time_cruise+Time_climb,0.0001);
      // best average speed when in maintaining height mode
      if (TimeToDestTotal <= BestTime) {
	KeepLooking = true;
	BestTime = TimeToDestTotal;
      }
    }
   
    if (KeepLooking) {
      BestSpeed = min(SAFTEYSPEED, vtrack_real);
      if (BestCruiseTrack) {
	// best track bearing is the track along cruise that
	// compensates for the drift during climb
	*BestCruiseTrack = 
	  atan2(CrossWind*(tcruise-1),vtot
		+HeadWind*(1-tcruise))*RAD_TO_DEG+Bearing;
      }
      if (VMacCready) {
	*VMacCready = BestSpeed;
      }

      // speed along track during cruise component
      *TimeToDestCruise = Distance*tcruise/vtot;

    } else {
	// no need to continue search, max already found..
	break; 
    }
  }

  if (TimeToGo) {
    *TimeToGo = TimeToDestTotal;
  }

  return (int) BestSpeed;
  }



double GlidePolar::MacCreadyAltitude_internal(double emcready,
                                              double Distance, 
                                              double Bearing, 
                                              const double WindSpeed, 
                                              const double WindBearing, 
                                              double *BestCruiseTrack,
                                              double *VMacCready, 
                                              const bool isFinalGlide,
                                              double *TimeToGo,
                                              const double cruise_efficiency,
                                              double Sink) // m/s
  {
  int SpeedToFly;
  double TimeToDestCruise = -1; // initialise to error value

  SpeedToFly = GetSpeedToFly(emcready,
                             Distance, 
                             Bearing,
                             WindSpeed,
                             WindBearing,
                             BestCruiseTrack,
                             VMacCready, 
                             isFinalGlide,
                             TimeToGo,
                             cruise_efficiency,
                             &TimeToDestCruise,
                             Sink);

  // This is the altitude needed to final glide to destination
  return (Sink - SinkRateFast(0, SpeedToFly)) * TimeToDestCruise;
}


double GlidePolar::SinkRate(double a,double b, double c, 
                            double MC, double HW, double V)
{
  double temp;

  // Quadratic form: w = c+b*(V)+a*V*V

  temp =  a*(V+HW)*(V+HW);
  temp += b*(V+HW);
  temp += c-MC;

  return temp;
}



double GlidePolar::FindSpeedForSinkRate(double w) {
  // find the highest speed that provides a sink rate less than
  // the specified sink rate
  double vbest= Vminsink;
  for (int v=4; v<iSAFETYSPEED; v++) {
    double wthis = _SinkRateFast(0, v);
    if (wthis<w) {
      vbest = v;
    }
  }
  return vbest;
}


double GlidePolar::MacCreadyAltitude_heightadjust(double emcready, 
						  double Distance, 
						  double Bearing, 
						  const double WindSpeed, 
						  const double WindBearing, 
						  double *BestCruiseTrack,
						  double *VMacCready, 
						  const bool isFinalGlide,
                                                  double *TimeToGo,
                                                  const double AltitudeAboveTarget,
						  const double cruise_efficiency,
                                      double Sink)
  {
  double Altitude;
  double TTG = 0;

  if (!isFinalGlide || (AltitudeAboveTarget<=0)) {

    // if not in final glide or below target altitude, need to 
    // climb-cruise the whole way

    Altitude = MacCreadyAltitude_internal(emcready,
                                          Distance, Bearing,
                                          WindSpeed, WindBearing,
                                          BestCruiseTrack,
                                          VMacCready,
                                          false,
                                          &TTG,
					  cruise_efficiency,
                                          Sink);
    }
  else {

    // if final glide mode and can final glide part way

    double t_t = ERROR_TIME;
    double h_t = MacCreadyAltitude_internal(emcready,
                                            Distance, Bearing,
                                            WindSpeed, WindBearing,
                                            BestCruiseTrack,
                                            VMacCready,
                                            true,
                                            &t_t,
					    cruise_efficiency,
                                            Sink);

    if (h_t<=0) {
      // error condition, no distance to travel
      TTG = t_t;
      Altitude = 0;

    } else {
      double h_f = AltitudeAboveTarget; 
      // fraction of leg that can be final glided
      double f_final = min(1.0,max(0.0,h_f/h_t));

      if (f_final < 1.0) {
        // if need to climb-cruise part of the way

	double d_cc = Distance*(1.0 - f_final);
        double t_c;
	double h_c = MacCreadyAltitude_internal(emcready,
						d_cc, Bearing,
						WindSpeed, WindBearing,
						BestCruiseTrack,
						VMacCready,
						false,
						&t_c,
						cruise_efficiency,
       Sink);

        if (h_c<0) {
          // impossible at this Mc, so must be final glided
          Altitude = -1;
          TTG = ERROR_TIME;
        } else {
          Altitude = f_final * h_t + h_c;
	  TTG = f_final * t_t + t_c;
        }
 
      } else {

        // can final glide the whole way

        Altitude = h_t;
        TTG = t_t;

      }
    }
  }
  
  if (TimeToGo) {
    *TimeToGo = TTG;
  }
  return Altitude;


}


double GlidePolar::MacCreadyAltitude(double emcready, 
                                     double Distance, 
				     const double Bearing, 
                                     const double WindSpeed, 
				                                     const double WindBearing,
                                     double Sink)
  {
  return GlidePolar::MacCreadyAltitude(emcready,
                                       Distance, 
                                       Bearing, 
                                       WindSpeed, 
                                       WindBearing, 
                                       0,
                                       0, 
                                       true,
                                       0,
                                       ALTABOVETARGET,
                                       CRUISEEFF,
                                       Sink);
  }



double GlidePolar::MacCreadyAltitude(double emcready,
                                     double Distance, 
                                     const double Bearing, 
                                     const double WindSpeed, 
                                     const double WindBearing, 
                                     double *BestCruiseTrack,
                                     double *VMacCready, 
                                     const bool isFinalGlide,
                                     double *TimeToGo,
                                     const double AltitudeAboveTarget,
				     const double cruise_efficiency,
                                     double Sink)
  {

#if (LK_CACHECALC && LK_CACHECALC_MCA)

  #define CASIZE  LK_CACHECALC_MCA
  static bool doinit=true;
  static int cacheIndex;

  bool cacheFound=false;
  int i;
  double cur_checksum;
  double cur_Distance=Distance;
  double cur_emcready=emcready;

  // BCT and VMC are available in _internal at the cost of an atan2 calculation more..
  // since there is a 50% ratio of cache hits, these precalculated value will raise of 2% this ratio.
  static double cur_BestCruiseTrack;
  static double cur_VMacCready;

  static double cache_checksum[CASIZE];
  static double cache_altitude[CASIZE];
  static double cache_emcready[CASIZE];
  static double cache_Distance[CASIZE];
  static double cache_Bearing[CASIZE];
  static double cache_WindSpeed[CASIZE];
  static double cache_WindBearing[CASIZE];
  static double cache_BestCruiseTrack[CASIZE];	// out
  static double cache_VMacCready[CASIZE];	// out
  static double cache_TimeToGo[CASIZE];		// out
  static double cache_AltitudeAboveTarget[CASIZE];
  static double cache_cruise_efficiency[CASIZE];
  static bool   cache_isFinalGlide[CASIZE];
  static double cache_Sink[CASIZE];

  if (doinit) {
	for (i=0; i<CASIZE; i++) {
		cache_checksum[i]=0;
		cache_altitude[i]=0;
		cache_emcready[i]=0;
		cache_Distance[i]=0;
		cache_Bearing[i]=0;
		cache_WindSpeed[i]=0;
		cache_WindBearing[i]=0;
		cache_BestCruiseTrack[i]=0;
		cache_VMacCready[i]=0;
		cache_TimeToGo[i]=0;
		cache_AltitudeAboveTarget[i]=0;
		cache_cruise_efficiency[i]=0;
		cache_isFinalGlide[i]=false;
       cache_Sink[i] = 0;
      }
    cacheIndex=0;
	doinit=false;
  }

  cur_checksum = emcready+Distance+Bearing+WindSpeed+WindBearing+AltitudeAboveTarget+cruise_efficiency + Sink;
  cacheFound=false;

  #if LK_CACHECALC_MCA_STAT
  Cache_Calls_MCA++;
  #endif
  // search with no particular order in the cache
  for (i=0; i<=CASIZE; i++) {
	if (cache_checksum[i] != cur_checksum ) continue;

	if (cache_emcready[i] != emcready) {
		#if LK_CACHECALC_MCA_STAT
		Cache_False_MCA++; 
		#endif
		continue;
	}
	if (cache_Distance[i] != Distance) {
		#if LK_CACHECALC_MCA_STAT
		Cache_False_MCA++; 
		#endif
		continue;
	}
	if (cache_Bearing[i] != Bearing) {
		#if LK_CACHECALC_MCA_STAT
		Cache_False_MCA++;
		#endif
		continue;
	}
	if (cache_WindSpeed[i] != WindSpeed) {
		#if LK_CACHECALC_MCA_STAT
		Cache_False_MCA++;
		#endif
		continue;
	}
	if (cache_WindBearing[i] != WindBearing) {
		#if LK_CACHECALC_MCA_STAT
		Cache_False_MCA++;
		#endif
		continue;
	}
	if (cache_isFinalGlide[i] != isFinalGlide) {
		#if LK_CACHECALC_MCA_STAT
		Cache_False_MCA++;
		#endif
		continue;
	}
	if (cache_AltitudeAboveTarget[i] != AltitudeAboveTarget) {
		#if LK_CACHECALC_MCA_STAT
		Cache_False_MCA++;
		#endif
		continue;
	}
	if (cache_cruise_efficiency[i] != cruise_efficiency) {
		#if LK_CACHECALC_MCA_STAT
		Cache_False_MCA++;
		#endif
		continue;
	      }

    if (cache_Sink[i] != Sink)
      {
      #if LK_CACHECALC_MCA_STAT
      Cache_False_MCA++;
      #endif
      continue;
      }

#if 0
	// ONLY IF WE ARE NOT CACHING BCT and VMC (and probably it is better to move this at the beginning
	// of checks, in this case, since mostly these we accounting for negative cache after positive checksum
	// if input values match, still check if also output values were available previously.
	// TTG is always available, but BTC and VMC are not necessarily calculated
	if (BestCruiseTrack) {
		if (cache_BestCruiseTrack[i]<0 ) {
			#if LK_CACHECALC_MCA_STAT
			Cache_Incomplete_MCA++;
			#endif
			continue;
		}
	}
	if (VMacCready) {
		if (cache_VMacCready[i]<0 ) {
			#if LK_CACHECALC_MCA_STAT
			Cache_Incomplete_MCA++;
			#endif
			continue;
		}
	}
#endif
	// input and output values matching.  TTG may have not been asked previously, but nevertheless cached, so OK!
	cacheFound=true;
	break;
  }
  if (cacheFound) {
	#if LK_CACHECALC_MCA_STAT
	Cache_Hits_MCA++;
	#endif
	if ( BestCruiseTrack ) *BestCruiseTrack=cache_BestCruiseTrack[i];
	if ( VMacCready ) *VMacCready = cache_VMacCready[i];
	if ( TimeToGo ) *TimeToGo = cache_TimeToGo[i];
	return cache_altitude[i];
  } else {
	#if LK_CACHECALC_MCA_STAT
	Cache_Fail_MCA++;
	#endif
  }

#endif

  double TTG = ERROR_TIME;
  double Altitude = -1;
  //bool invalidMc = (emcready<MIN_MACCREADY);
  bool invalidAltitude = false;

  if ((emcready >= MIN_MACCREADY) || isFinalGlide) {
    Altitude = MacCreadyAltitude_heightadjust(emcready,
                                              Distance, Bearing,
                                              WindSpeed, WindBearing,
#if (LK_CACHECALC && LK_CACHECALC_MCA)
					      // we always calculate them inside _internal for caching, just like TTG
                                              &cur_BestCruiseTrack,
                                              &cur_VMacCready,
#else
                                              BestCruiseTrack,
                                              VMacCready,
#endif
                                              isFinalGlide,
                                              &TTG,
                                              AltitudeAboveTarget,
					      cruise_efficiency,
                                              Sink);

    #if (LK_CACHECALC && LK_CACHECALC_MCA)
    if (BestCruiseTrack) *BestCruiseTrack=cur_BestCruiseTrack;
    if (VMacCready) *VMacCready=cur_VMacCready;
#endif

    if (Altitude<0) {
      invalidAltitude = true;
    } else {
      // All ok
      if (TTG<0.9*ERROR_TIME) {
        goto onExit;
      }
    }
  }

  // Never going to make it at this rate, so assume final glide
  // with no climb
  // This can occur if can't make progress against headwind,
  // or if Mc is too small

  Altitude = MacCreadyAltitude_heightadjust(emcready,
                                            Distance, Bearing,
                                            WindSpeed, WindBearing,
#if (LK_CACHECALC && LK_CACHECALC_MCA)
                                            &cur_BestCruiseTrack,
                                            &cur_VMacCready,
#else
                                            BestCruiseTrack,
                                            VMacCready,
#endif
                                            true,
                                            &TTG,
                                            ALTABOVETARGET,
                                            cruise_efficiency,
                                            Sink);

#if (LK_CACHECALC && LK_CACHECALC_MCA)
    if (BestCruiseTrack) *BestCruiseTrack=cur_BestCruiseTrack;
    if (VMacCready) *VMacCready=cur_VMacCready;
#endif

  if (invalidAltitude) {
    TTG += ERROR_TIME;
    // if it failed due to invalid Mc, need to increase estimated
    // time and the glider better find that lift magically
  }

 onExit:
  if (TimeToGo) {
    *TimeToGo = TTG;
  }

#if (LK_CACHECALC && LK_CACHECALC_MCA)
  if (!cacheFound) {
	// add inside cache
	if (++cacheIndex==CASIZE) cacheIndex=0;

	cache_checksum[cacheIndex] = cur_checksum;

#if 0 // 100328
#if 100131
	if (cur_emcready != emcready) {
		if (cur_Distance != Distance) {
			DoStatusMessage(_T("ERR-085 CACHE MCA DIST + EMC"));
		} else {
			DoStatusMessage(_T("ERR-084 CACHE MCA EMCREADY"));
		}
	} else {
		if (cur_Distance != Distance) {
			DoStatusMessage(_T("ERR-083 CACHE MCA DISTANCE"));
		}
	}
#else
	if (cur_Distance != Distance  && cur_emcready != emcready ) {
		DoStatusMessage(_T("ERR CACHE MCA DIST + EMC"));
		goto goto_test;
	}
	if (cur_Distance != Distance  ) DoStatusMessage(_T("ERR CACHE MCA DISTANCE"));
	if (cur_emcready != emcready  ) DoStatusMessage(_T("ERR CACHE MCA EMCREADY"));
	goto_test: 
#endif
#endif
	cache_emcready[cacheIndex] = cur_emcready; // 100328
	cache_Distance[cacheIndex] = cur_Distance; // 100328
	cache_Bearing[cacheIndex] = Bearing;
	cache_WindSpeed[cacheIndex] = WindSpeed;
	cache_WindBearing[cacheIndex] = WindBearing;
	cache_isFinalGlide[cacheIndex] = isFinalGlide;
	cache_AltitudeAboveTarget[cacheIndex] = AltitudeAboveTarget;
	cache_cruise_efficiency[cacheIndex] = cruise_efficiency;
       cache_Sink[cacheIndex] = Sink;

  #if 1
	cache_BestCruiseTrack[cacheIndex]= cur_BestCruiseTrack;
	cache_VMacCready[cacheIndex] = cur_VMacCready;
	#else
	// IN CASE WE ARE NOT CACHING ALWAYS ALSO BCT and VMC
	// cache output values  where -1 mean no output
	if (BestCruiseTrack)
		cache_BestCruiseTrack[cacheIndex]= *BestCruiseTrack;
	else
		cache_BestCruiseTrack[cacheIndex]= -1;
	if (VMacCready)
		cache_VMacCready[cacheIndex] = *VMacCready;
	else
		cache_VMacCready[cacheIndex] = -1;
	#endif
	// TTG is always available
	cache_TimeToGo[cacheIndex] = TTG;
	cache_altitude[cacheIndex] = Altitude;
 }

#endif

  return Altitude;

}

static double FRiskFunction(double x, double k) {
  return 2.0/(1.0+exp(-x*k))-1.0;
}

double GlidePolar::MacCreadyRisk(double HeightAboveTerrain, 
                                 double MaxThermalHeight, 
                                 double MC) {
  double riskmc = MC;

  double hthis = max(1.0, HeightAboveTerrain);
  double hmax = max(hthis, MaxThermalHeight);
  double x = hthis/hmax;
  double f;
  
  if (RiskGamma<0.1) {
    return MC;
  } else if (RiskGamma>0.9) {
    f = x;
  } else {
    double k;
    k = 1.0/(RiskGamma*RiskGamma)-1.0;
    f = FRiskFunction(x, k)/FRiskFunction(1.0, k);
  }
  double mmin = 0; // min(MC,AbortSafetyMacCready());
  riskmc = f*riskmc+(1-f)*mmin;
  return riskmc;
}
