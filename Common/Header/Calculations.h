/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Calculations.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(AFX_CALCULATIONS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_CALCULATIONS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "mapshape.h"

#define NUMTHERMALBUCKETS 10
#define MAX_THERMAL_SOURCES 20

typedef struct _THERMAL_SOURCE_INFO
{
  POINT Screen;
  double Latitude;
  double Longitude;
  double GroundHeight;
  double LiftRate;
  bool Visible;
  double Time;
} THERMAL_SOURCE_INFO;

typedef struct _THERMAL_HISTORY
{
  bool   Valid;
  TCHAR  Name[10];	// TH1055
  TCHAR  Near[20];	// nearby waypoint, if available
  double Time;		// start circling time
  double Latitude;
  double Longitude;
  double HBase;		// thermal base
  double HTop;		// total thermal gain
  double Lift;		// Avg lift rate

  double Distance;	// recalculated values
  double Bearing;
  double Arrival;
} THERMAL_HISTORY;


typedef struct _DERIVED_INFO
{
  double Vario;
  double LD;
  double CruiseLD;
  double AverageLD;
  double VMacCready;
  double Average30s;
  double NettoAverage30s;
  double BestCruiseTrack;
  double AverageThermal;
  double ThermalGain;
  double LastThermalAverage;
  double LastThermalGain;
  double LastThermalTime;
  double ClimbStartLat;
  double ClimbStartLong;
  double ClimbStartAlt;
  double ClimbStartTime;
  double CruiseStartLat;
  double CruiseStartLong;
  double CruiseStartAlt;
  double CruiseStartTime;
  double WindSpeed;
  double WindBearing;
  double Bearing;
  double TerrainAlt;
  bool   TerrainValid;
  double Heading;
  double AltitudeAGL;
  int    Circling;
  int    FinalGlide;
  int    AutoMacCready; // TODO FIX bool
  int    Flying;	// TODO FIX bool ?
  bool	 FreeFlying;	// set true when powerless flight is detected. Always true for paragliders.
  double NextAltitudeRequired;
  double NextAltitudeRequired0; // mc=0
  double NextAltitudeDifference;
  double NextAltitudeDifference0; // difference with mc=0 
  double FinalAltitudeRequired;
  double FinalAltitudeDifference;
  double TaskDistanceToGo;
  double TaskDistanceCovered;
  double TaskTimeToGo;
  double TaskStartTime;
  double TaskSpeed;
  double TaskSpeedInstantaneous;
  double TaskAltitudeRequired;
  double TaskAltitudeDifference;
  double TaskAltitudeDifference0; // difference with mc=0
  double TaskAltitudeRequiredFromStart;
  double LegDistanceToGo;
  double LegDistanceCovered;
  double LegTimeToGo;
  double LegStartTime;
  double LegSpeed;
  double NextLatitude;
  double NextLongitude;
  double NextAltitude;
  double NextAltitudeAGL;
  double AATMaxDistance;
  double AATMinDistance;
  double AATTargetDistance;
  double AATTimeToGo;
  double AATMaxSpeed;
  double AATTargetSpeed;
  double AATMinSpeed;
  double PercentCircling;

  double TerrainWarningLongitude;
  double TerrainWarningLatitude;
  double ObstacleDistance;
  double ObstacleHeight;
  double ObstacleAltReqd;
  double ObstacleAltArriv;

  double FarObstacle_Lat;
  double FarObstacle_Lon;
  double FarObstacle_Dist;

  double FarObstacle_Height;
  double FarObstacle_AltReqd;
  double FarObstacle_AltArriv;

  double Odometer;
  // Paolo Ventafridda: recalcuated value with no strange assumptions. These values are trustable.
  double LKTaskETE;
  double EqMc; // equivalent MacCready

  // JMW moved calculated waypoint info here

  double WaypointBearing;
  double WaypointDistance;
  double WaypointSpeed; 

  // JMW thermal band data
  double MaxThermalHeight;
  int    ThermalProfileN[NUMTHERMALBUCKETS];
  double ThermalProfileW[NUMTHERMALBUCKETS];

  double NettoVario;

  // Current flap
  TCHAR Flaps[MAXFLAPSNAME+1];

  // optimum speed to fly instantaneously
  double VOpt; 

  // JMW estimated track bearing at next time step
  double NextTrackBearing;

  // JMW energy height excess to slow to best glide speed
  double EnergyHeight;

  // Turn rate in global coordinates
  double TurnRate;
  
  // reflects whether aircraft is in a start/finish/aat/turn sector
  bool IsInSector; 
  bool IsInAirspace;

  // detects when glider is on ground for several seconds
  bool OnGround;

  double NavAltitude;
  bool ValidStart;
  double TaskStartSpeed;
  double TaskStartAltitude;
  bool ValidFinish;

  double LDvario;

  double ThermalEstimate_Longitude;
  double ThermalEstimate_Latitude;
  double ThermalEstimate_W;
  double ThermalEstimate_R;

  THERMAL_SOURCE_INFO ThermalSources[MAX_THERMAL_SOURCES];

  pointObj GlideFootPrint[NUMTERRAINSWEEPS+1];

  TCHAR OwnTeamCode[10];
  double TeammateBearing;
  double TeammateRange;
  double TeammateLatetude;
  double TeammateLongitude;
  double FlightTime;
  double TakeOffTime;
  double FreeFlightStartTime;
  double FreeFlightStartQNH;
  double FreeFlightStartQFE;

  double AverageClimbRate[200];
  long AverageClimbRateN[200];

  double HomeDistance;
  double HomeRadial; 
  

  double ZoomDistance;
  double TaskSpeedAchieved;
  double TrueAirspeedEstimated;
  double IndicatedAirspeedEstimated;

  double timeCruising;
  double timeCircling;

  double MinAltitude;
  double MaxAltitude;
  double MaxHeightGain;

  double HeadWind;

  // Turn rate in wind coordinates
  double GPSVario;
  double TurnRateWind;
  double BankAngle;
  double PitchAngle;
  double TaskTimeToGoTurningNow;
  double TotalHeightClimb;
  double GliderSinkRate;
  double Gload;
  double Essing;
  double TerrainBase; // lowest height within glide range
  double GRFinish;	// GRadient to final destination, 090203
			// Note: we don't need GRNext since this value is used when going to a landing
			// point, which is always a final glide.

  double Experimental;
  // JMW note, new items should go at the bottom of this struct before experimental!
} DERIVED_INFO;


#ifdef GTL2
void DoAlternates(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int AltWaypoint);
#endif
int DoCalculations(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
int DoCalculationsVario(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
void DoCalculationsSlow(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
void SearchBestAlternate(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
void DoNearest(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
// void DoNearestTurnpoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
void DoCommon(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
bool DoTraffic(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
bool DoAirspaces(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
bool DoTarget(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
bool DoThermalHistory(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
bool IsThermalMultitarget(int idx);
void SetThermalMultitarget(int idx);
int  GetThermalMultitarget(void);
void DoRecent(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
bool DoRangeWaypointList(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
bool DoCommonList(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
double GetAzimuth(void);
unsigned int GetWpChecksum(unsigned int);
bool SaveRecentList();
bool LoadRecentList();
void ResetRecentList();
void ResetTask();
void InsertRecentList(int newwp);
void RemoveRecentList(int newwp);
double CalculateWaypointArrivalAltitude(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int thepoint); // VENTA3
double GetCurrentEfficiency(DERIVED_INFO *Calculated, short effmode);


bool ClearAirspaceWarnings(const bool ack, const bool allday=false);
void RefreshTaskStatistics(void);
void  SetWindEstimate(const double speed, const double bearing, const int quality=6);

void LoadCalculationsPersist(DERIVED_INFO *Calculated);
void SaveCalculationsPersist(DERIVED_INFO *Calculated);
void DeleteCalculationsPersist(void);

void CloseCalculations(void);


	// gates are configured, and used by gliders also? todo
bool UseGates(void);		
	// we are inside open and close time so a gate is open
bool IsGateOpen(void);		
	// returns the next gate number or -1
int NextGate(void);		
	// Returns the specified gate time (hours), negative -1 if invalid
int GateTime(int gate);		
	// Returns the gatetime difference to current local time. 
	// Positive if gate is in the future.
int GateTimeDiff(int gate);
	// Returns the current open gate number, 0-x, or -1 (negative) if out of time.
	// This is NOT the next start! It tells you if a gate is open right now, within time limits.
int RunningGate(void);
	// Do we have some gates available, either running right now or in the future?
	// Basically mytime <CloseTime...
bool HaveGates(void);
	// returns the current gate we are in, either in the past or in the future.
	// It does not matter if it is still valid (it is expired).
	// There is ALWAYS an activegate, it cannot be negative!
int InitActiveGate(void);
	// autonomous check for usegates, and current chosen activegate is open, so a valid start
	// is available crossing the start sector..
bool ValidGate(void);
	// Are we on the correct side of start cylinder?
bool CorrectSide(void);
void ResetFreeFlightStats(DERIVED_INFO *Calculated);


void InitCalculations(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

void StartTask(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                      const bool doadvance, const bool doannounce);

bool  InAATTurnSector(const double longitude, const double latitude, const int thepoint);

void IterateEffectiveMacCready(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

double FAIFinishHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int wp);
int getFinalWaypoint(void);
bool InsideStartHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
bool ValidStartSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
bool InsideStartHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated, DWORD Margin);
bool ValidStartSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated, DWORD Margin);

void InsertThermalHistory(double ThTime,  double ThLat, double ThLon, double ThBase,double ThTop, double ThAvg);
void InitThermalHistory(void);

#ifdef GTL2
double FinalGlideThroughTerrain(const double bearing,
                                const double start_lat,
                                const double start_lon,
                                const double start_alt,
#else
double FinalGlideThroughTerrain(const double bearing, NMEA_INFO *Basic, 
#endif
                                DERIVED_INFO *Calculated,
                                double *retlat, double *retlon,
                                const double maxrange,
				bool *outofrange,
				double *TerrainBase = NULL); 

double FarFinalGlideThroughTerrain(const double bearing, NMEA_INFO *Basic, 
                                DERIVED_INFO *Calculated,
                                double *retlat, double *retlon,
                                const double maxrange,
				bool *outofrange,
				const double testaltitude,
				double *TerrainBase = NULL);

double AltitudeNeededToPassObstacles(const double startLat, const double startLon, const double startAlt,
                                const double wpLat, const double wpLon,
                                const double wpBearing, const double wpDistance,
                                DERIVED_INFO *Calculated);


void BallastDump();

int FindFlarmSlot(const int flarmId);
int FindFlarmSlot(const TCHAR *flarmCN);
bool IsFlarmTargetCNInRange(void);
void AlertBestAlternate(short soundmode); 

double CalculateGlideRatio(const double d, const double h);
bool CheckSafetyAltitudeApplies(const int wpindex);
double GetSafetyAltitude(const int wpindex);
short GetVisualGlideRatio(const double arrival, const double gr);
bool IsSafetyAltitudeInUse(const int wpindex);

void CalculateHeadWind(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

#endif
