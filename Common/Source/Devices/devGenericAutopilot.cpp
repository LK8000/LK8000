/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   devGenericAutopilot.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 18 september 2023
 */

#include "externs.h"
#include "devGenericAutopilot.h"
#include "devNmeaOut.h"
#include <regex>
#include "utils/printf.h"
#include "LKInterface.h"

namespace {

BOOL NMEAOut(DeviceDescriptor_t* d, const char* String) {
  static const std::regex re_filter(R"(^\$G.(GGA|RMC),.*[\n\r]*$)");
  if (std::regex_match(String, re_filter)) {
    return NmeaOut::NMEAOut(d, String);
  }
  return FALSE;
}

std::string GenerateRMB(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
  double xtd = 0.;
  GeoPoint next_pos;
  double distance = 0.;
  double bearing = 0.;
  std::string prev_name;
  std::string next_name;

  bool valid = WithLock(CritSec_TaskData, [&]() {
      int prev_index = -1;
      int next_index = -1;

      if (ValidTaskPointFast(ActiveTaskPoint)) {
        // active task turnpoint
        if (ActiveTaskPoint > 0) {
          prev_index = Task[ActiveTaskPoint - 1].Index;
        }
        next_index = Task[ActiveTaskPoint].Index;
      }

      if (next_index < 0) {
        // if no task configured, use overtarget
        next_index = GetOvertargetIndex();
      }

      if (ValidWayPointFast(next_index)) {
        const WAYPOINT& next_tp = WayPointList[next_index];
        next_pos = GetWayPointPosition(next_tp);
        next_name = to_utf8(next_tp.Name);

        const GeoPoint current = GetCurrentPosition(Basic);
        current.Reverse(next_pos, bearing, distance);
        distance =  Units::ToUser(unNauticalMiles, distance);

        if (ValidWayPointFast(prev_index)) {
          const WAYPOINT& prev_tp = WayPointList[prev_index];
          const GeoPoint prev_pos = GetWayPointPosition(prev_tp);
          prev_name = to_utf8(WayPointList[prev_index].Name);

          xtd = CrossTrackError(prev_pos, next_pos, current);
        }
        return true;
      }
      return false;
  });

  if (!valid) {
    return {};
  }

  double speed = Units::ToUser(unKnots, Basic.Speed);
  char dir = (xtd < 0) ? 'R' : 'L';
  xtd = std::min(9.9, std::abs(Units::ToUser(unNauticalMiles, xtd)));

  /*
  eg1. $GPRMB,A,0.66,L,003,004,4917.24,N,12309.57,W,001.3,052.5,000.5,V*0B

          A            Data status A = OK, V = warning
          0.66,L       Cross-track error (nautical miles, 9.9 max.),
                                                          steer Left to correct (or R = right)
          003          Origin waypoint ID
          004          Destination waypoint ID
          4917.24,N    Destination waypoint latitude 49 deg. 17.24 min. N
          12309.57,W   Destination waypoint longitude 123 deg. 09.57 min. W
          001.3        Range to destination, nautical miles
          052.5        True bearing to destination
          000.5        Velocity towards destination, knots
          V            Arrival alarm  A = arrived, V = not arrived
          *0B          mandatory checksum
  */

  int DegLat = next_pos.latitude;
  double MinLat = next_pos.latitude - DegLat;
  char NoS = 'N';
  if ((MinLat < 0) || (((MinLat - DegLat) == 0) && (DegLat < 0))) {
    NoS = 'S';
    DegLat *= -1;
    MinLat *= -1;
  }
  MinLat *= 60;

  int DegLon = next_pos.longitude;
  double MinLon = next_pos.longitude - DegLon;
  char EoW = 'E';
  if ((MinLon < 0) || (((MinLon - DegLon) == 0) && (DegLon < 0))) {
    EoW = 'W';
    DegLon *= -1;
    MinLon *= -1;
  }
  MinLon *= 60;

  char sNmea[MAX_NMEA_LEN];
  size_t len = lk::snprintf(sNmea, "$GPRMB,A,%.1f,%c,%s,%s,%02d%05.2f,%c,%03d%05.2f,%c,%.1f,%.1f,%.1f,V",
                            xtd, dir, prev_name.c_str(), next_name.c_str(),
                            DegLat, MinLat, NoS, DegLon, MinLon, EoW,
                            distance, bearing, speed);

  char* pChar = sNmea + len;
  lk::snprintf(pChar, MAX_NMEA_LEN - len, "*%02X\r\n", nmea_crc(sNmea + 1));

  return sNmea;
}

BOOL SendData(DeviceDescriptor_t* d, const NMEA_INFO& Basic, const DERIVED_INFO& Calculated) {
  if (d && d->Com) {
    std::string data = GenerateRMB(Basic, Calculated);
    if (!data.empty()) {
      return d->Com->Write(data.data(), data.size());
    }
  }
  return FALSE;
}

}  // namespace

void GenericAutopilot::Install(DeviceDescriptor_t* d) {
  _tcscpy(d->Name, Name);
  d->SendData = SendData;
  d->NMEAOut = NMEAOut;
}
