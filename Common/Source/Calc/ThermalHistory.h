/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ThermalHistory.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 25 september 2024, 16:15
 */

#ifndef _CALC_THERMAL_HISTORY_H_
#define _CALC_THERMAL_HISTORY_H_

#include "Util/tstring.hpp"
#include <optional>
#include <vector>
#include "Geographic/GeoPoint.h"

struct NMEA_INFO;
struct DERIVED_INFO;

struct THERMAL_HISTORY {
  tstring  Name;	// TH1055
  tstring  Near;	// nearby waypoint, if available
  double Time;		// start circling time
  GeoPoint position;
  double HBase;		// thermal base
  double HTop;		// total thermal gain
  double Lift;		// Avg lift rate

  double Distance;	// recalculated values
  double Bearing;
  double Arrival;
};

void InitThermalHistory();

void InsertThermalHistory(double ThTime, const GeoPoint& position, double ThBase, double ThTop, double ThAvg, bool SetMultiTarget = true);

bool IsThermalExist(const GeoPoint& position, double radius);

bool DoThermalHistory(NMEA_INFO* Basic, DERIVED_INFO* Calculated);

bool IsThermalMultitarget(size_t idx);

void SetThermalMultitarget(size_t idx, const TCHAR* Comment);

std::optional<THERMAL_HISTORY> GetThermalMultitarget();
std::optional<THERMAL_HISTORY> GetThermalHistory(size_t idx);

// Copy of runtime thermal history structure for instant use
extern std::vector<THERMAL_HISTORY> CopyThermalHistory;
// Number of Thermals updated from DoThermalHistory
extern int LKNumThermals;
extern std::vector<int> LKSortedThermals;

#endif // _CALC_THERMAL_HISTORY_H_
