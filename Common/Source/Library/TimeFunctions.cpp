/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#include "externs.h"

// simple localtime with no 24h exceeding
int LocalTime(int utc_time) {
  int localtime = (utc_time + GetUTCOffset()) % (24 * 3600);
  if (localtime < 0) {
    localtime += (24 * 3600);
  }
  return localtime;
}

int LocalTime() {
  return LocalTime(GPS_INFO.Time);
}

namespace {

// Unix timestamp calculation helpers
constexpr bool isleap(unsigned y) {
  return (!((y) % 400) || (!((y) % 4) && ((y) % 100)));
}

unsigned monthtoseconds(bool isleap, unsigned month) {
  static constexpr unsigned _secondstomonth[12] = {
      0,
      24 * 60 * 60 * 31,
      24 * 60 * 60 * (31 + 28),
      24 * 60 * 60 * (31 + 28 + 31),
      24 * 60 * 60 * (31 + 28 + 31 + 30),
      24 * 60 * 60 * (31 + 28 + 31 + 30 + 31),
      24 * 60 * 60 * (31 + 28 + 31 + 30 + 31 + 30),
      24 * 60 * 60 * (31 + 28 + 31 + 30 + 31 + 30 + 31),
      24 * 60 * 60 * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31),
      24 * 60 * 60 * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
      24 * 60 * 60 * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
      24 * 60 * 60 * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)
  };

  if ((month > 11) || (month < 0)) {
    return 0;
  }
  unsigned sec = _secondstomonth[month];
  if (isleap && (month > 1)) {
    return sec + 24 * 60 * 60;
  }
  return sec;
}

uint64_t yeartoseconds(unsigned y) {
  if (y < 1970) {
    return 0;
  }
  constexpr uint64_t day = 24 * 60 * 60;

  uint64_t sec = y - 1970U;
  sec *= 365 * day;
  sec += (((y - 1) - 1968) / 4) * day;
  return sec;
}

} // namespace

time_t to_time_t(const NMEA_INFO& info) {
  time_t t = yeartoseconds(info.Year);
  t += monthtoseconds(isleap(info.Year), (info.Month - 1) % 12);
  t += (info.Day - 1) * 3600 * 24;
  t += info.Hour * 3600 + info.Minute * 60 + info.Second;
  return t;
}
