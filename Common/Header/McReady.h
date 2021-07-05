#if !defined(AFX_MCREADY_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_MCREADY_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

class GlidePolar {
 public:

  static double MacCreadyAltitude(double MCREADY, double Distance,
                                  const double Bearing,
				  const double WindSpeed,
                                  const double WindBearing,
				  double *BestCruiseTrack,
                                  double *VMacCready,
				  const bool isFinalGlide,
                                  double *timetogo,
                                  double AltitudeAboveTarget=1.0e6,
                                #ifdef BCT_ALT_FIX
                                  double cruise_efficiency=1.0,
                                  double TaskAltDiff=-1.0e6);
                                #else
				  double cruise_efficiency=1.0);
                                #endif

  static void SetBallast();
  static double GetAUW();

  static double SafetyMacCready;

  //  static double BallastFactor;
  static double polar_a;
  static double polar_b;
  static double polar_c;
  static double bestld;
  static double minsink;
  static double BallastLitres;
  static double WingArea;
  static double WingLoading;
  static double WeightOffset;

  static double FlapsPos[MAX_FLAPS];
  static TCHAR  FlapsName[MAX_FLAPS][MAXFLAPSNAME+1];
  static int FlapsPosCount;
  static double FlapsMass;

  static double SinkRate(double Vias);
  static double SinkRate(double Vias,
                double loadfactor);
  static double SinkRate(double a,double b, double c,
                         double MC, double HW, double V);
  static double FindSpeedForSinkRate(double w);
  static double FindSpeedForSinkRateAccurate(double w);
  static double FindSpeedForSlope(double s);
  static double SinkRateFast(const double &MC, const double &v);

  static double EquMC(double ias);
  static double STF(double MC, double Vario, double HeadWind);


 private:
  static double MacCreadyAltitude_internal(double MCREADY,
                                           double Distance,
                                           const double Bearing,
                                           const double WindSpeed,
                                           const double WindBearing,
                                           double *BestCruiseTrack,
                                           double *VMacCready,
                                           const bool isFinalGlide,
                                           double *timetogo,
                                         #ifdef BCT_ALT_FIX
                                           const double cruise_efficiency,
                                           const double TaskAltDiff);
                                         #else
					   const double cruise_efficiency);
                                         #endif

  static double MacCreadyAltitude_heightadjust(double MCREADY,
                                               double Distance,
                                               const double Bearing,
                                               const double WindSpeed,
                                               const double WindBearing,
                                               double *BestCruiseTrack,
                                               double *VMacCready,
                                               const bool isFinalGlide,
                                               double *timetogo,
                                               const double AltitudeAboveTarget,
                                             #ifdef BCT_ALT_FIX
                                               const double cruise_efficiency,
                                               const double TaskAltDiff);
                                             #else
					       const double cruise_efficiency);
                                             #endif

// SinkRate Cache
public:
	static double Vminsink() {
		return ((double)_Vminsink)/2.0;
	}
	static double Vbestld() {
		return ((double)_Vbestld)/2.0;
	}
    static double SinkRateBestLd() {
		return _sinkratecache[_Vbestld];
	}

private:
	static double _SinkRateFast(const double &MC, const unsigned &v);

	static double _sinkratecache[(MAXSPEED+1)*2]; // index = irount(speed * 2); // speed in m/s !!

	static unsigned _Vminsink; //@ unsigned int, because is Array index of _sinkratecache (iRound(v*2)) integer m/s values
	static unsigned _Vbestld;
};

#endif
