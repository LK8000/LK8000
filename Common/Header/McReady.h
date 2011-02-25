#if !defined(AFX_MCREADY_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_MCREADY_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Calculations.h"
#include "Parser.h"

#define ALTABOVETARGET 1.0e6  // 1,000,000
#define CRUISEEFF 1.0
#define MAXSAFETYSPEED  100	// 360kmh
#define MAXSPEED	100	// 360kmh = 100ms

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
                                  double AltitudeAboveTarget = ALTABOVETARGET,
                                  double cruise_efficiency = CRUISEEFF,
                                  double Sink = 0.0);

  static double MacCreadyAltitude(double MCREADY,
                                  double Distance, 
                                  const double Bearing,
                                  const double WindSpeed,
                                  const double WindBearing,
                                  double Sink);

  static double MaxOkaySink(NMEA_INFO *Basic,
                            DERIVED_INFO *Calculated,
                            double DistToWP,
                            double BearingToWP,
                            double LastValue,
                            double AltWP,
                            int WPindex);
  
  static double MacCreadyRisk(double HeightAboveTerrain, double MaxThermalHeight,
                              double MCREADY);

  static void SetBallast();
  static double GetAUW();

  static double AbortSafetyMacCready();
  static double SafetyMacCready;
  static bool AbortSafetyUseCurrent;

  //  static double BallastFactor;
  static double RiskGamma;
  static double polar_a;
  static double polar_b;
  static double polar_c;
  static int Vminsink; //@ int, because holding only integer m/s values
  static int Vbestld;
  static double bestld;
  static double minsink;
  static double BallastLitres;
  static double WingArea;
  static double WingLoading;
  static double WeightOffset;
  
  static double sinkratecache[MAXSPEED+1];

  static double SinkRate(double Vias);
  static double SinkRate(double Vias, 
                double loadfactor);
  static double SinkRate(double a,double b, double c, 
                         double MC, double HW, double V);
  static double FindSpeedForSinkRate(double w);
  static double SinkRateFast(const double &MC, const int &v);
 private:
  static double _SinkRateFast(const double &MC, const int &v);
  static double MacCreadyAltitude_internal(double MCREADY, 
                                           double Distance, 
                                           const double Bearing, 
                                           const double WindSpeed, 
                                           const double WindBearing, 
                                           double *BestCruiseTrack, 
                                           double *VMacCready, 
                                           const bool isFinalGlide, 
                                           double *timetogo,
					   const double cruise_efficiency,
                                           double Sink = 0.0);


  static int GetSpeedToFly(double emcready, 
                           double Distance, 
                           const double Bearing, 
                           const double WindSpeed, 
                           const double WindBearing, 
                           double *BestCruiseTrack, 
                           double *VMacCready, 
                           const bool isFinalGlide, 
                           double *TimeToGo,
                           const double cruise_efficiency,
                           double *TimeToDestCruise,
                           double SinkAir = 0.0);

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
					       const double cruise_efficiency,
                                               double Sink = 0.0);

};

#endif
