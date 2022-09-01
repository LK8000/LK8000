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

  if (month > 11) {
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

// Calculate the ISO 8601 day of the week number 
//   now - Unix timestamp like that from time(NULL)
//   Return value: Monday=0, ... Sunday=6, 
unsigned day_of_week(time_t now, int utc_offset) {
	// Calculate number of seconds since midnight 1 Jan 1970 local time
	time_t localtime = now + (utc_offset);
	// Convert to number of days since 1 Jan 1970
	unsigned days_since_epoch = localtime / 86400;
	// 1 Jan 1970 was a Thursday, so add 3 so Monday is day 0, and mod 7
	return (days_since_epoch + 3) % 7;
}


#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("TimeFunctions") {
  SUBCASE("day_of_week") {
    CHECK(day_of_week(0, 0) == 3); // 1970-01-01 00:00:00 -> Thursday
    CHECK(day_of_week(1661194800, 2 * 3600) == 0); // Mon Aug 22 2022 21:00:00 GMT+0200
    CHECK(day_of_week(1661036400, 2 * 3600) == 6); // Sun Aug 21 2022 01:00:00 GMT+0200
    CHECK(day_of_week(1661036400, 0) == 5); // Sat Aug 20 2022 23:00:00 GMT+0000
  }
}

#endif
