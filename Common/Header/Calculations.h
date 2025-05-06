/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Calculations.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(AFX_CALCULATIONS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_CALCULATIONS_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#include "NMEA/Info.h"
#include "NMEA/Derived.h"


void DoAlternates(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int AltWaypoint);
bool DoCalculations(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
void DoCalculationsVario(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
void DoCalculationsSlow(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
bool SearchBestAlternate(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
void DoNearest(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
// void DoNearestTurnpoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
void DoCommon(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
bool DoTraffic(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
bool DoAirspaces(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
bool DoTarget(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
void DoRecent(NMEA_INFO *Basic, DERIVED_INFO *Calculated); 
bool DoRangeWaypointList(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
bool DoCommonList(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
double GetAzimuth(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated);

void ResetTask(bool showConfirmMsg);

void SaveRecentList();
void LoadRecentList();
void InsertRecentList(int wp_idx);
void RemoveRecentList(int wp_idx);
void ResetRecentList();


double CalculateWaypointArrivalAltitude(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int thepoint); // VENTA3
double GetCurrentEfficiency(DERIVED_INFO *Calculated, short effmode);


bool ClearAirspaceWarnings(const bool ack, const bool allday=false);
void RefreshTaskStatistics(void);
void  SetWindEstimate(const double speed, const double bearing, const int quality=6);

void LoadCalculationsPersist(DERIVED_INFO *Calculated);
void SaveCalculationsPersist(DERIVED_INFO *Calculated);
void DeleteCalculationsPersist(void);

void CloseCalculations(void);


	// Are we on the correct side of start cylinder?
bool CorrectSide(const DERIVED_INFO& Calculated);
  // return true if Start cylinder must be crossed from inside to outside
bool ExitStart(const DERIVED_INFO& Calculated);

void ResetFreeFlightStats(DERIVED_INFO *Calculated);


void InitCalculations(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

void StartTask(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                      const bool doadvance, const bool doannounce);

void IterateEffectiveMacCready(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

double FAIFinishHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int wp);
int getFinalWaypoint(void);
bool InsideStartHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
bool ValidStartSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
bool InsideStartHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated, unsigned Margin);
bool ValidStartSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated, unsigned Margin);

double FinalGlideThroughTerrain(const double bearing,
                                const double start_lat,
                                const double start_lon,
                                const double start_alt,
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
bool IsSafetyMacCreadyInUse(const int wpindex);

void CalculateHeadWind(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

#endif
