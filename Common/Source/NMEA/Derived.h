/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Derived.h
 */

#ifndef _NMEA_DERIVED_H_
#define _NMEA_DERIVED_H_

#include "types.h"

#define NUMTHERMALBUCKETS 10
#define MAXFLAPSNAME 10
#define MAX_THERMAL_SOURCES 20

// Size of array for ias values AverageClimbRate and AverageClimbRateN calculations
#define MAXAVERAGECLIMBRATESIZE  200

// number of radials to do range footprint calculation on
#ifndef UNDER_CE
#define NUMTERRAINSWEEPS 40
#else
#define NUMTERRAINSWEEPS 20
#endif

struct THERMAL_SOURCE_INFO {
    POINT Screen;
    double Latitude;
    double Longitude;
    double GroundHeight;
    double LiftRate;
    double Time;
    bool Visible;
};

struct DERIVED_INFO {
  DERIVED_INFO() = default;

  double Vario = {};
  double LD = {};
  double CruiseLD = {};
  double VMacCready = {};
  double Average30s = {};
  double BestCruiseTrack = {};
  double AverageThermal = {};
  double ThermalGain = {};
  double LastThermalAverage = {};
  double LastThermalGain = {};
  double LastThermalTime = {};
  double ClimbStartLat = {};
  double ClimbStartLong = {};
  double ClimbStartAlt = {};
  double ClimbStartTime = {};
  double CruiseStartLat = {};
  double CruiseStartLong = {};
  double CruiseStartAlt = {};
  double CruiseStartTime = {};
  double WindSpeed = {};
  double WindBearing = {};
  double Bearing = {};
  double TerrainAlt = {};
  bool   TerrainValid = {};
  double Heading = {};
  double AltitudeAGL = {};
  bool   Circling = {};
  bool   FinalGlide = {};
  bool   AutoMacCready = {};
  bool   Flying = {};
  bool	 FreeFlying = {};	// set true when powerless flight is detected. Always true for paragliders.
  double NextAltitudeRequired = {};
  double NextAltitudeRequired0 = {}; // mc=0
  double NextAltitudeDifference = {};
  double NextAltitudeDifference0 = {}; // difference with mc=0
  double TaskDistanceToGo = {};
  double TaskDistanceCovered = {};
  double TaskTimeToGo = {};
  double TaskStartTime = {};
  double TaskElapsedTime = {};
  double TaskSpeed = {};
  double TaskSpeedInstantaneous = {};
  double TaskAltitudeRequired = {};
  double TaskAltitudeDifference = {};
  double TaskAltitudeDifference0 = {}; // difference with mc=0
  double TaskAltitudeRequiredFromStart = {};
  double LegDistanceToGo = {};
  double LegDistanceCovered = {};
  double LegCrossTrackError = {};
  double LegActualTrueCourse = {};
  double LegTimeToGo = {};
  double LegStartTime = {};
  double NextLatitude = {};
  double NextLongitude = {};
  double NextAltitude = {};
  double NextAltitudeAGL = {};
  double AATMaxDistance = {};
  double AATMinDistance = {};
  double AATTargetDistance = {};
  double AATTimeToGo = {};
  double AATMaxSpeed = {};
  double AATTargetSpeed = {};
  double AATMinSpeed = {};
  double PercentCircling = {};

  double TerrainWarningLongitude = {};
  double TerrainWarningLatitude = {};
  double ObstacleDistance = {};
  double ObstacleHeight = {};
  double ObstacleAltReqd = {};
  double ObstacleAltArriv = {};

  double FarObstacle_Lat = {};
  double FarObstacle_Lon = {};
  double FarObstacle_Dist = {};

  double FarObstacle_AltArriv = {};

  double Odometer = {};
  // Paolo Ventafridda: recalcuated value with no strange assumptions. These values are trustable.
  double LKTaskETE = {};
  double EqMc = {}; // equivalent MacCready

  // JMW moved calculated waypoint info here

  double WaypointBearing = {};
  double WaypointDistance = {};

  // JMW thermal band data
  double MaxThermalHeight = {};
  int    ThermalProfileN[NUMTHERMALBUCKETS] = {};
  double ThermalProfileW[NUMTHERMALBUCKETS] = {};

  double NettoVario = {};

  // Current flap
  TCHAR Flaps[MAXFLAPSNAME + 1] = {};

  // optimum speed to fly instantaneously
  double VOpt = {};

  // Maximum efficiency speed to fly
  double Vme = {};

  // JMW estimated track bearing at next time step
  double NextTrackBearing = {};

  // JMW energy height excess to slow to best glide speed
  double EnergyHeight = {};

  // Turn rate in global coordinates
  double TurnRate = {};

  // reflects whether aircraft is in a start/finish/aat/turn sector
  bool IsInSector = {};

  // detects when glider is on ground for several seconds
  bool OnGround = {};

  double NavAltitude = {};
  bool ValidStart = {};
  double TaskStartSpeed = {};
  double TaskStartAltitude = {};
  bool ValidFinish = {};

  double LDvario = {};

  double ThermalEstimate_Longitude = {};
  double ThermalEstimate_Latitude = {};
  double ThermalEstimate_W = {};
  double ThermalEstimate_R = {};

  double AverageLD = {};
  double AverageGS = {};
  double AverageDistance = {};

  THERMAL_SOURCE_INFO ThermalSources[MAX_THERMAL_SOURCES] = {};

#ifdef ENABLE_OPENGL
  GeoPoint GlideFootPrint[NUMTERRAINSWEEPS+2] = {};
#else
  GeoPoint GlideFootPrint[NUMTERRAINSWEEPS+1] = {};
#endif
  GeoPoint GlideFootPrint2[NUMTERRAINSWEEPS+1] = {};

  bool GlideFootPrint_valid = {}; // true if #GlideFootPrint well calculated
  bool GlideFootPrint2_valid = {}; // true if #GlideFootPrint2 well calculated

  TCHAR OwnTeamCode[10] = {};
  double TeammateBearing = {};
  double TeammateRange = {};
  double FlightTime = {};
  double TakeOffTime = {};
  double FreeFlightStartTime = {};
  double FreeFlightStartQNH = {};
  double FreeFlightStartQFE = {};

  double AverageClimbRate[MAXAVERAGECLIMBRATESIZE] = {};
  int AverageClimbRateN[MAXAVERAGECLIMBRATESIZE] = {};

  double HomeDistance = {};
  double HomeRadial = {};


  double ZoomDistance = {};
  double TaskSpeedAchieved = {};
  double TrueAirspeedEstimated = {};
  double IndicatedAirspeedEstimated = {};

  double timeCruising = {};
  double timeCircling = {};

  double MinAltitude = {};
  double MaxAltitude = {};
  double MaxHeightGain = {};

  double HeadWind = {};

  // Turn rate in wind coordinates
  double GPSVario = {};
  double TurnRateWind = {};
  double BankAngle = {};
  double TotalHeightClimb = {};
  double TerrainBase = {}; // lowest height within glide range
  double GRFinish = {};	// GRadient to final destination, 090203
			// Note: we don't need GRNext since this value is used when going to a landing
			// point, which is always a final glide.

  double Gload = {};
  Point3D Acceleration = {};

  double TaskAltitudeArrival = {}; // this is estimated task arrival height above ground
  bool   TaskFAI = {};             // Ist Task FAI ?
  double TaskTotalDistance = {};   // total Task Distance
  double TaskFAIDistance = {};     // FAI Task distance if Task is FAI (can be different to Total when start on leg)
};

static_assert(std::is_copy_constructible_v<DERIVED_INFO>, "mandatory...");

constexpr double ERROR_TIME = 86400;

inline
bool IsValidTaskTimeToGo(const DERIVED_INFO& info) {
  return (info.TaskTimeToGo > 0.) && (info.TaskTimeToGo < ERROR_TIME);
}

#endif // _NMEA_DERIVED_H_
