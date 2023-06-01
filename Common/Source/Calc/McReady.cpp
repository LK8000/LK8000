/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: McReady.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "McReady.h"
#include "DoInits.h"
#include "utils/stl_utils.h"
#include "Util/Clamp.hpp"


double GlidePolar::polar_a;
double GlidePolar::polar_b;
double GlidePolar::polar_c;
unsigned GlidePolar::_Vminsink = 2;
unsigned GlidePolar::_Vbestld = 2;
double GlidePolar::_sinkratecache[(MAXSPEED+1)*2]; // this is in 0.5m/s !!
double GlidePolar::bestld = 1.0;
double GlidePolar::minsink = 10000.0;
double GlidePolar::BallastLitres = 0.0;
double GlidePolar::WingArea = 0.0;
double GlidePolar::WingLoading = 0.0;
double GlidePolar::WeightOffset = 0.0;

double GlidePolar::FlapsPos[MAX_FLAPS];
TCHAR  GlidePolar::FlapsName[MAX_FLAPS][MAXFLAPSNAME+1];
int GlidePolar::FlapsPosCount = 0;
double GlidePolar::FlapsMass = 0.0;

double GlidePolar::SafetyMacCready= 0.5;

static unsigned iSAFETYSPEED=0;

// GetAUW is returning gross weight of glider, with pilot and current ballast.
// We now also add the offset to match chosen wing loading, just like a non-dumpable ballast
//
// 150428 limit wing loading minimum value to be 1.
// BallastWeight/WingArea >= 1 which means BallastWeight must be >= WingArea
// BallastLitres + WEIGHTS[0] + WEIGHTS[1] + GlidePolar::WeightOffset > WingArea
// BallastLitres + GlidePolar::WeightOffset  > (WingArea - WEIGHTS[0] - WEIGHTS[1])
// ->  GlidePolar::WeightOffset  > (WingArea - WEIGHTS[0] - WEIGHTS[1])
// and we use the above in the WeightOffset();

double GlidePolar::GetAUW() {
  return BallastLitres + WEIGHTS[0] + WEIGHTS[1] + GlidePolar::WeightOffset;
}


void GlidePolar::SetBallast() {
  ScopeLock lock(CritSec_FlightData);

  double BallastWeight;
  BallastLitres = WEIGHTS[2] * BALLAST;
  BallastWeight = GetAUW();
  // Always positive.  But sqrt requires a >=0 value and we do check anyway.
  BUGSTOP_LKASSERT(BallastWeight>=0);
  if (BallastWeight<0) {
    return; 
  }
  if (WingArea>0.1) {
    WingLoading = BallastWeight/WingArea;
  } else {
    WingLoading = 0;
  }
  BallastWeight = (double)sqrt(BallastWeight);
  BUGSTOP_LKASSERT(BUGS!=0);
  double bugfactor = 1.0/BUGS;
  BUGSTOP_LKASSERT(BallastWeight!=0);
  if (BallastWeight==0) BallastWeight=1;
  polar_a = POLAR[0] / BallastWeight*bugfactor;
  polar_b = POLAR[1] * bugfactor;
  polar_c = POLAR[2] * BallastWeight*bugfactor;

  // do preliminary scan to find min sink and best LD
  // this speeds up mcready calculations because we have a reduced range
  // to search across.
  // this also limits speed to fly to logical values (will never try
  // to fly slower than min sink speed)

  minsink = 10000.0;
  double tmp_bestld = 0.0;

  // Rounding errors could make SAFTEYSPEED 0.00xx and not 0
  // Now below 3kmh we consider the speed wrong
  // MAXSAFETYSPEED WAS 200, = 720kmh!!
  if ((SAFTEYSPEED<1)||(SAFTEYSPEED>=MAXSAFETYSPEED)) {
    SAFTEYSPEED=MAXSAFETYSPEED-1;
  }
  iSAFETYSPEED=Clamp(iround(SAFTEYSPEED*2), 8, (MAXSPEED*2));

  // _sinkratecache is an array for 0.5 m/s values!! i = irount(speed * 2) speed in m/s
  for(int _i=8;_i<=(MAXSPEED*2);_i++) {
     double vtrack = ((double)_i)/2; // TAS along bearing in cruise
     double thesinkrate = -SinkRate(polar_a,polar_b,polar_c,0,0,vtrack);

     BUGSTOP_LKASSERT(thesinkrate>0);
     if (thesinkrate<=0) thesinkrate=0.001;
     double ld = vtrack/thesinkrate;
     if (ld>=tmp_bestld) {
        tmp_bestld=ld;
        bestld = ld;
        _Vbestld = _i;
     }
     if (thesinkrate<= minsink) {
        minsink = thesinkrate;
        _Vminsink = _i;
     }
      _sinkratecache[_i] = -thesinkrate;

  }
  BUGSTOP_LKASSERT(bestld>0);
  if (bestld<=0) {
     TESTBENCH_DO_ONLY(10, StartupStore(_T(".... SetBallast bestld=%f NOT FOUND! Polar error?%s"),bestld,NEWLINE));
     bestld=1;
  }
}

inline double GlidePolar::_SinkRateFast(const double &MC, const unsigned &v) {
    if(iSAFETYSPEED >= std::size(_sinkratecache)){
        iSAFETYSPEED = 0; // avoid buffer overflow
    }
    BUGSTOP_LKASSERT(iSAFETYSPEED > 8U);
    BUGSTOP_LKASSERT(v >= 8U && v <= iSAFETYSPEED);

    return _sinkratecache[Clamp(v, 8U, iSAFETYSPEED)] - MC;
}

double GlidePolar::SinkRateFast(const double &MC, const double &v) {
  return _SinkRateFast(MC, iround(v*2));
}


double GlidePolar::SinkRate(double V) {

  return SinkRate(polar_a,polar_b,polar_c,0.0,0.0,V);

}


#define MIN_MACCREADY 0.000000000001


double GlidePolar::SinkRate(double V, double n) {
  double w0 = SinkRate(polar_a,polar_b,polar_c,0.0,0.0,V);
  n = max(0.1,fabs(n));
  //  double v1 = V/max(1,Vbestld);
  double v2 = Vbestld()/max(Vbestld()/2.0,V);
  BUGSTOP_LKASSERT(bestld>0);
  if (bestld<=0) bestld=1; // UNMANAGED
  return w0-(V/(2*bestld))* (n*n-1)*(v2*v2);
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
                                            #ifdef BCT_ALT_FIX
                                              const double cruise_efficiency,
                                              const double TaskAltDiff)
                                            #else
					      const double cruise_efficiency)
                                            #endif
{

  double BestSpeed, BestGlide, Glide;
  double BestSinkRate, TimeToDestCruise;
  static double HeadWind, CrossWind=0.0;
  static double CrossBearingLast= -1.0;
  static double WindSpeedLast= -1.0;
  double CrossBearing;
  double BestTime;
  static double HeadWindSqd, CrossWindSqd=0.0;

  CrossBearing = AngleLimit360(Bearing - WindBearing);
  if ((CrossBearing != CrossBearingLast)||(WindSpeed != WindSpeedLast)) {
    // saves a few floating point operations
    HeadWind = WindSpeed * fastcosine(CrossBearing);
    CrossWind = WindSpeed * fastsine(CrossBearing);
    HeadWindSqd = HeadWind*HeadWind;
    CrossWindSqd = CrossWind*CrossWind;

    // save old values
    CrossBearingLast = CrossBearing;
    WindSpeedLast = WindSpeed;
  }

  double sinkrate;
  double tc; // time spent in cruise

  // TODO accuracy: extensions to Mc to incorporate real-life issues
  // - [done] best cruise track and bearing (final glide and for waypoint)
  // - climb before or after turning waypoints.
  // - mcready ring changes with height allowing for risk and decreased rate
  // - cloud streets
  // - sink rate between thermals
  // - modify Vtrack for IAS

  //Calculate Best Glide Speed
  BestSpeed = 4; // 4 m/s is less speed for _sinkRatecache
  BestGlide = 10000;
  BestTime = 1e6;

  // The following makes sure BCT gets set to Bearing in the event that
  // we're unable to advance against the wind with any glide speed.

  if (BestCruiseTrack) {
    *BestCruiseTrack = Bearing;
  }

  double vtot;
  // REWRITING DISTANCE!
  if (Distance<1.0) {
    Distance = 1;
  }

  double TimeToDestTotal = ERROR_TIME; // initialise to error value
  TimeToDestCruise = -1; // initialise to error value

  #ifdef BCT_ALT_FIX
  bool SpeedFound = false;
  #endif

  for(unsigned _i=_Vminsink;_i<=iSAFETYSPEED;_i++) {
    double vtrack_real = ((double)_i)/2.0; // actual airspeed
    double vtrack = vtrack_real*cruise_efficiency;
    // TAS along bearing in cruise

    // glide angle = velocity projected along path / sink rate
    // need to work out best velocity along path given wind vector
    // need to work out percent time spent cruising
    // SinkRate function returns negative value for sink

    if (isFinalGlide) {
      sinkrate = -_SinkRateFast(max(0.0,emcready), _i);
      // tc=1 will make wind 0 and not taken into account
      tc = 1.0; // assume no circling, e.g. final glide at best LD
      // with no climbs
    } else {
	// WE ARE REWRITING EMCREADY!
      emcready = max(MIN_MACCREADY,emcready);
      sinkrate = -_SinkRateFast(0.0, _i);
      BUGSTOP_LKASSERT((sinkrate+emcready)!=0);
      if ( (sinkrate+emcready)==0 ) sinkrate+=0.1; // to check
      tc = max(0.0,min(1.0,emcready/(sinkrate+emcready)));
    }

    // calculate average speed along track relative to wind
    vtot = (vtrack*vtrack*tc*tc-CrossWindSqd);
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

    bool bestfound = false;

    if (isFinalGlide) {
      // inverse glide ratio relative to ground
      Glide = sinkrate/vtot;

      // best glide angle when in final glide
      if (Glide <= BestGlide) {
	bestfound = true;
	BestGlide = Glide;
	TimeToDestTotal = Distance/vtot;
      }
    } else {
      // time spent in cruise
      double Time_cruise = (tc/vtot)*Distance;
      BUGSTOP_LKASSERT(emcready!=0);
      if (emcready==0) emcready=0.1;
      double Time_climb = sinkrate*(Time_cruise/emcready);

      // total time to destination
      TimeToDestTotal = max(Time_cruise+Time_climb,0.0001);
      // best average speed when in maintaining height mode
      if (TimeToDestTotal <= BestTime) {
	bestfound = true;
	BestTime = TimeToDestTotal;
      }
    }

    if (bestfound) {
      BestSpeed = min(SAFTEYSPEED, vtrack_real);
      #ifdef BCT_ALT_FIX
      SpeedFound = true;
      #else

      if (BestCruiseTrack) {
	// best track bearing is the track along cruise that
	// compensates for the drift during climb

	*BestCruiseTrack =
	  atan2(CrossWind*(tc-1),vtot
		+HeadWind*(1-tc))*RAD_TO_DEG+Bearing;
      }
      #endif

      if (VMacCready) {
	*VMacCready = BestSpeed;
      }

      // speed along track during cruise component
      TimeToDestCruise = Distance*tc/vtot;

    } else {
	// no need to continue search, max already found..
	break;
    }
  }

  #ifdef BCT_ALT_FIX
  if (SpeedFound && BestCruiseTrack && !isFinalGlide) {

    // Calculate "best cruise track", the track along cruise that
    // compensates for the drift during climbs.

    double MoreAltReqd0; // alt. deficit from current position

    // If there's no task set, then calculate BCT assuming
    // (1) the intent is to arrive at the destination at the current
    // altitude and (2) the received Bearing (current ground track)
    // is the desired average track.

    if ((CALCULATED_INFO.TaskAltitudeDifference == 0) &&
        (CALCULATED_INFO.TaskDistanceToGo == 0)) {

        Distance = 100000; // 100 km

        // alt loss on 100km glide on current track
        MoreAltReqd0 = GlidePolar::MacCreadyAltitude(
                       emcready,
                       Distance,
                       Bearing,
                       WindSpeed,
                       WindBearing,
                       0, 0, true, 0);
    } else { // task is set
      Distance = CALCULATED_INFO.TaskDistanceToGo;
      MoreAltReqd0 = -TaskAltDiff;
    }

    // If there's no crosswind or no more altitude is needed to make
    // goal, then leave BCT as earlier set (to Bearing).

    if ((CrossWind != 0) && (MoreAltReqd0 > 0)) {

      double BrgLimit = Bearing;      // (deg) closest BCT can
                                      // be to Bearing
      double WindLimit = WindBearing; // (deg) closest BCT can
                                      // be to WindBearing
      double BCTguess;                // (deg) guessed best
                                      // cruise track
      double Precision;       // (deg) we know we�re at least
                              // this close to the real answer
      double AngleBrg;        // (deg) angle opposite brg-to-WP
                              // side of triangle
      double AngleCruise = 0; // (deg) angle opposite cruise
                              // side of triangle
      double AngleDrift;  // (deg) angle opposite climb/drift
                          // side of triangle
      double SinAngleCruise = 0; // sine of AngleCruise
      double Multiplier, DistDrift = 0, DistCruise;
      double MoreAltReqd, AltGain, AltLossBrg = 0, AltLossCruise;

      // Adjust the initial WindLimit, if the wind is
      // partially tailing.

      if (HeadWind < 0) {

        // if crossing from the right
        if (CrossWind < 0)
          WindLimit = AngleLimit360(Bearing + 90);

        // if crossing from the left
        else
          WindLimit = AngleLimit360(Bearing - 90);
      }

      while (true) {

        // Guess a BCT in the middle of the range of possibilities
        BCTguess = (BrgLimit + WindLimit) / 2.0;

        // correct for case when limits are on opposite sides
        // of due north
        if (fabs(BrgLimit - WindLimit) > 180)
          BCTguess = AngleLimit360(BCTguess - 180);

        // How close is this guess... worst case?
        Precision = fabs(BCTguess - BrgLimit);

        // correct for case when these angles are on opposite
        // sides of due north
        if (Precision > 90)
          Precision = 360 - Precision;

        if (Precision <= 1.0) { // within 1 deg is close enough
          *BestCruiseTrack = BCTguess;
          break;
        }

        // The following calculates some things that need to be
        // calculated only once and only if the first guess (above)
        // wasn�t close enough.

        if (DistDrift == 0) { // is 0 first iteration of loop only

          // alt loss on glide from current position on current-WP
          // bearing for the remaining task distance

          AltLossBrg = GlidePolar::MacCreadyAltitude(
                       emcready,
                       Distance,
                       Bearing,
                       WindSpeed,
                       WindBearing,
                       0, 0, true, 0);

          AngleCruise    = fabs(AngleLimit180(180 - (Bearing - WindBearing)));
          SinAngleCruise = fastsine(AngleCruise);
        }

        // angles opposite the drift/climb and brg-to-WP sides
        // of the triangle

        AngleDrift = fabs(AngleLimit180(BCTguess - Bearing));
        AngleBrg   = 180 - AngleCruise - AngleDrift;

        // use law of sines to calc other triangle side lengths.
        // We�ll use multiplier twice, so calculate it just once:

        BUGSTOP_LKASSERT(AngleBrg!=0);
	if (AngleBrg==0) AngleBrg=1;
        Multiplier = Distance / fastsine(AngleBrg);

        // lengths of circling/drifting & cruise sides of triangle

        DistDrift  = Multiplier * fastsine(AngleDrift);
        DistCruise = Multiplier * SinAngleCruise;

        // altitude gained while circling

        BUGSTOP_LKASSERT(WindSpeed!=0);

        double WindSpeedCopy = (WindSpeed == 0) ? 0.01 : WindSpeed;
        AltGain = DistDrift * emcready / WindSpeedCopy;

        // altitude lost while gliding on guessed BCT

        AltLossCruise = GlidePolar::MacCreadyAltitude(
                        emcready,
                        DistCruise,
                        BCTguess,
                        WindSpeed,
                        WindBearing,
                        0, 0, true, 0);

        // altitude deficit for this guess
        MoreAltReqd = MoreAltReqd0 + (AltLossCruise - AltLossBrg) - AltGain;

        if (MoreAltReqd < 0)     // have more alt than needed
          WindLimit = BCTguess;  // try BCT closer to Bearing
        else
          BrgLimit  = BCTguess;  // try BCT closer to wind dir
      } // �while� loop calculating BCT
    } // �if� there is crosswind & more alt is needed
  } // "if" SpeedFound && BestCruiseTrack && !isFinalGlide

  #endif // BCT_ALT_FIX

  BestSinkRate = SinkRateFast(0,BestSpeed);

  if (TimeToGo) {
    *TimeToGo = TimeToDestTotal;
  }

  // this is the altitude needed to final glide to destination
  return -BestSinkRate * TimeToDestCruise;
}


double GlidePolar::SinkRate(double a,double b, double c,
                            double MC, double HW, double V)
{
  double temp;

  // Quadratic form: w = c+b*(V)+a*V*V

  temp =  a*(V+HW)*(V+HW);
  temp += b*(V+HW);
  temp += c-MC;

  #if TESTBENCH
  if (temp>=0) {
      StartupStore(_T("... SinkRate error! a=%.2f b=%.2f c=%.2f MC=%.2f HW=%.2f V=%.2f = %.2f%s"),
        a,b,c,MC,HW,V,temp,NEWLINE);
  }
  #endif

  return temp;
}



double GlidePolar::FindSpeedForSinkRate(double w) {
  if(iSAFETYSPEED >= std::size(_sinkratecache)){
      iSAFETYSPEED = 0.; // avoid buffer overflow
      LKASSERT(FALSE); //
  }
  // find the highest speed that provides a sink rate less than
  // the specified sink rate
  double vbest= ((double)_Vminsink)/2.0;
  for (unsigned v=_Vminsink+1; v<=iSAFETYSPEED; v++) {
    double wthis = _sinkratecache[v];
    if (wthis>w) {
      vbest = ((double)(v-1))/2.0;
    }
  }
  return vbest;
}


double GlidePolar::FindSpeedForSinkRateAccurate(double w) {
  // find the highest speed that provides a sink rate less than
  // the specified sink rate
  double vbest= Vminsink();
  for (int v=(int)(Vminsink()*TOKPH); v<SAFTEYSPEED*TOKPH; v++) {
    double vms = (double)v/TOKPH;
    double wthis = SinkRate(polar_a,polar_b,polar_c,0,0,vms);
    if (wthis>w) {
      vbest = vms;
    }
  }
  return vbest;
}

double GlidePolar::FindSpeedForSlope(double s) {
    if (polar_a != 0. && s != 0.) {
        return -((polar_b * s + 1) / s / polar_a) * 1.0 / 2.0;
    }
    return Vminsink();
}

double GlidePolar::EquMC(double ias) {
    if (ias<1||ias>70) return -1;
    return polar_c-polar_a*ias*ias;
}

double GlidePolar::STF(double MC, double Vario, double HeadWind) {
    if (polar_a != 0.) {
        double dTmp = polar_a * (Vario - MC + polar_a * HeadWind * HeadWind + polar_b * HeadWind + polar_c);
        if (dTmp > 0) {
            return max(Vminsink(), HeadWind - (sqrt(dTmp) / polar_a)); // STF speed can't be less than min sink Speed.
        }
    }
    return Vminsink();
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
                                                #ifdef BCT_ALT_FIX
                                                  const double cruise_efficiency,
                                                  const double TaskAltDiff)
                                                #else
						  const double cruise_efficiency)
                                                #endif
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
                                        #ifdef BCT_ALT_FIX
                                          cruise_efficiency,
                                          TaskAltDiff);
                                        #else
					  cruise_efficiency);
                                        #endif
  } else {

    // if final glide mode and can final glide part way

    double t_t = ERROR_TIME;
    double h_t = MacCreadyAltitude_internal(emcready,
                                            Distance, Bearing,
                                            WindSpeed, WindBearing,
                                            BestCruiseTrack,
                                            VMacCready,
                                            true,
                                            &t_t,
                                          #ifdef BCT_ALT_FIX
                                            cruise_efficiency,
                                            TaskAltDiff);
                                          #else
					    cruise_efficiency);
                                          #endif

    if (h_t<=0) {
      // error condition, no distance to travel
      TTG = t_t;
      Altitude = 0;

    } else {
      double h_f = AltitudeAboveTarget;
      // fraction of leg that can be final glided
      double f = min(1.0,max(0.0,h_f/h_t));

      if (f<1.0) {
        // if need to climb-cruise part of the way

	double d_c = Distance*(1.0 - f);

	double t_c;
	double h_c = MacCreadyAltitude_internal(emcready,
						d_c, Bearing,
						WindSpeed, WindBearing,
						BestCruiseTrack,
						VMacCready,
						false,
						&t_c,
          #ifdef BCT_ALT_FIX
            cruise_efficiency,
            TaskAltDiff);
          #else
						cruise_efficiency);
          #endif

        if (h_c<0) {
          // impossible at this Mc, so must be final glided
          Altitude = -1;
          TTG = ERROR_TIME;
        } else {
          Altitude = f*h_t + h_c;
	  TTG = f*t_t + t_c;
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
                                     double *BestCruiseTrack,
                                     double *VMacCready,
                                     const bool isFinalGlide,
                                     double *TimeToGo,
                                     const double AltitudeAboveTarget,
                                   #ifdef BCT_ALT_FIX
                                     const double cruise_efficiency,
                                     const double TaskAltDiff) {
                                   #else
				     const double cruise_efficiency) {
                                   #endif


#if (LK_CACHECALC && LK_CACHECALC_MCA)

  #define CASIZE  LK_CACHECALC_MCA
  static unsigned cacheIndex = 0;

  bool cacheFound=false;
  unsigned i = 0;
  double cur_checksum;
  double cur_Distance=Distance;
  double cur_emcready=emcready;

#ifndef BCT_ALT_FIX
  // BCT and VMC are available in _internal at the cost of an atan2 calculation more..
  // since there is a 50% ratio of cache hits, these precalculated value will raise of 2% this ratio.
#endif

#ifndef BCT_ALT_FIX
  static double cur_BestCruiseTrack;
#endif
  static double cur_VMacCready;

  static double cache_checksum[CASIZE];
  static double cache_altitude[CASIZE];
  static double cache_emcready[CASIZE];
  static double cache_Distance[CASIZE];
  static double cache_Bearing[CASIZE];
  static double cache_WindSpeed[CASIZE];
  static double cache_WindBearing[CASIZE];
#ifndef BCT_ALT_FIX
  static double cache_BestCruiseTrack[CASIZE];	// out
#endif
  static double cache_VMacCready[CASIZE];	// out
  static double cache_TimeToGo[CASIZE];		// out
  static double cache_AltitudeAboveTarget[CASIZE];
  static double cache_cruise_efficiency[CASIZE];
  static bool   cache_isFinalGlide[CASIZE];
#ifdef BCT_ALT_FIX
  static double cache_TaskAltDiff[CASIZE];
#endif

  if (DoInit[MDI_MCREADYCACHE]) {
	for (i=0; i<CASIZE; i++) {
		cache_checksum[i]=0;
		cache_altitude[i]=0;
		cache_emcready[i]=0;
		cache_Distance[i]=0;
		cache_Bearing[i]=0;
		cache_WindSpeed[i]=0;
		cache_WindBearing[i]=0;
  #ifndef BCT_ALT_FIX
		cache_BestCruiseTrack[i]=0;
  #endif
		cache_VMacCready[i]=0;
		cache_TimeToGo[i]=0;
		cache_AltitudeAboveTarget[i]=0;
		cache_cruise_efficiency[i]=0;
		cache_isFinalGlide[i]=false;
  #ifdef BCT_ALT_FIX
    cache_TaskAltDiff[i]=0;
  #endif
	}
	cacheIndex=0;
	DoInit[MDI_MCREADYCACHE]=false;
  }

#ifdef BCT_ALT_FIX
  cur_checksum = emcready + Distance + Bearing + WindSpeed + WindBearing +
                 AltitudeAboveTarget + cruise_efficiency + TaskAltDiff;
#else
  cur_checksum = emcready+Distance+Bearing+WindSpeed+WindBearing+AltitudeAboveTarget+cruise_efficiency;
#endif
  cacheFound=false;

  #ifdef BCT_ALT_FIX
  // Look in the cache only if the MCA call didn't request an update to BCT,
  // which isn't cached.  It isn't cached, because it's calculated using
  // data (task distance remaining) not included as an MCA argument.
  if (!BestCruiseTrack) {
  #endif

  #if LK_CACHECALC_MCA_STAT
  Cache_Calls_MCA++;
  #endif
  // search with no particular order in the cache
  for (i=0; i<CASIZE; i++) {
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
  #ifdef BCT_ALT_FIX
	if (cache_TaskAltDiff[i] != TaskAltDiff) {
		#if LK_CACHECALC_MCA_STAT
		Cache_False_MCA++;
		#endif
		continue;
	}
  #endif

#if 0
	// ONLY IF WE ARE NOT CACHING BCT and VMC (and probably it is better to move this at the beginning
	// of checks, in this case, since mostly these we accounting for negative cache after positive checksum
	// if input values match, still check if also output values were available previously.
	// TTG is always available, but BTC and VMC are not necessarily calculated

  #ifndef BCT_ALT_FIX
	if (BestCruiseTrack) {
		if (cache_BestCruiseTrack[i]<0 ) {
			#if LK_CACHECALC_MCA_STAT
			Cache_Incomplete_MCA++;
			#endif
			continue;
		}
	}
  #endif

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

#ifdef BCT_ALT_FIX
  } // if BCT is NULL or zero (no BCT calculation needed, look in cache)
#endif

  if (cacheFound) {
	#if LK_CACHECALC_MCA_STAT
	Cache_Hits_MCA++;
	#endif

  #ifndef BCT_ALT_FIX
	if ( BestCruiseTrack ) *BestCruiseTrack=cache_BestCruiseTrack[i];
  #endif
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
  bool invalidMc = (emcready<MIN_MACCREADY);
  bool invalidAltitude = false;

  if (!invalidMc || isFinalGlide) {
    Altitude = MacCreadyAltitude_heightadjust(emcready,
                                              Distance, Bearing,
                                              WindSpeed, WindBearing,
#if (LK_CACHECALC && LK_CACHECALC_MCA)
              #ifndef BCT_ALT_FIX
					      // we always calculate them inside _internal for caching, just like TTG
              #endif
                                            #ifdef BCT_ALT_FIX
                                              BestCruiseTrack,
                // we always calculate VMC inside _internal for caching, just like TTG
                                            #else
                                              &cur_BestCruiseTrack,
                                            #endif
                                              &cur_VMacCready,
#else
                                              BestCruiseTrack,
                                              VMacCready,
#endif
                                              isFinalGlide,
                                              &TTG,
                                              AltitudeAboveTarget,
                                            #ifdef BCT_ALT_FIX
                                              cruise_efficiency,
                                              TaskAltDiff);
                                            #else
					      cruise_efficiency);
                                            #endif
#if (LK_CACHECALC && LK_CACHECALC_MCA)

  #ifndef BCT_ALT_FIX
    if (BestCruiseTrack) *BestCruiseTrack=cur_BestCruiseTrack;
  #endif

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
                                          #ifdef BCT_ALT_FIX
                                            BestCruiseTrack,
                                          #else
                                            &cur_BestCruiseTrack,
                                          #endif
                                            &cur_VMacCready,
#else
                                            BestCruiseTrack,
                                            VMacCready,
#endif
                                            true,
                                            &TTG, 1.0e6,
                                          #ifdef BCT_ALT_FIX
                                            cruise_efficiency,
                                            TaskAltDiff);
                                          #else
					    cruise_efficiency);
                                          #endif

#if (LK_CACHECALC && LK_CACHECALC_MCA)

  #ifndef BCT_ALT_FIX
    if (BestCruiseTrack) *BestCruiseTrack=cur_BestCruiseTrack;
  #endif

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
	if (++cacheIndex>=CASIZE) cacheIndex=0;

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
  #ifdef BCT_ALT_FIX
    cache_TaskAltDiff[cacheIndex] = TaskAltDiff;
  #endif

	#if 1
  #ifndef BCT_ALT_FIX
	cache_BestCruiseTrack[cacheIndex]= cur_BestCruiseTrack;
  #endif
	cache_VMacCready[cacheIndex] = cur_VMacCready;
	#else
	// IN CASE WE ARE NOT CACHING ALWAYS ALSO BCT and VMC
	// cache output values  where -1 mean no output

  #ifndef BCT_ALT_FIX
	if (BestCruiseTrack)
		cache_BestCruiseTrack[cacheIndex]= *BestCruiseTrack;
	else
		cache_BestCruiseTrack[cacheIndex]= -1;
  #endif

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
