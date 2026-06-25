/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 */

#include "externs.h"

namespace {

#ifdef _WIN32
// timegm() is a GNU extension, not standard C)
//    not available by default on MinGW / Windows 
time_t timegm (struct tm *tp) {
  return _mkgmtime(tp);
} 

#endif

}  // namespace

// simple localtime with no 24h exceeding
int LocalTime(int utc_time) {
  int localtime = (utc_time + GetUTCOffset()) % (24 * 3600);
  if (localtime < 0) {
    localtime += (24 * 3600);
  }
  return localtime;
}

int LocalTime() {
  int utc_time = WithLock(CritSec_FlightData, []() {
    return GPS_INFO.Time;
  });
  return LocalTime(utc_time);
}

time_t to_time_t(int year, int mon, int mday, int hour, int min, int sec) {
  // mon = 1..12, mday = 1..31 (no validation required)
  struct tm t = {};
  t.tm_year = year - 1900;
  t.tm_mon = mon - 1;
  t.tm_mday = mday;
  t.tm_hour = hour;
  t.tm_min = min;
  t.tm_sec = sec;
  return timegm(&t);  // UTC-safe
}

time_t to_time_t(const NMEA_INFO& info) {
  return to_time_t(info.Year, info.Month, info.Day, info.Hour, info.Minute, info.Second);
}

void from_time_t(time_t t, int& year, int& mon, int& mday, int& hour, int& min, int& sec) {
  // Convert time_t to UTC date and time components
  tm utc_tm = {};
  struct tm* pda_time;
  pda_time = gmtime_r(&t, &utc_tm);
  year = pda_time->tm_year + 1900;
  mon = pda_time->tm_mon + 1;
  mday = pda_time->tm_mday;
  hour = pda_time->tm_hour;
  min = pda_time->tm_min;
  sec = pda_time->tm_sec;
}

void from_time_t(time_t t, NMEA_INFO& info) {
  from_time_t(t, info.Year, info.Month, info.Day, info.Hour, info.Minute, info.Second);
  info.Time = info.Hour * 3600 + info.Minute * 60 + info.Second;
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

TEST_CASE("to_time_t") {
  SUBCASE("Epoch correctness") {
    CHECK(to_time_t(1970, 1, 1, 0, 0, 0) == 0);
  }

  SUBCASE("Known POSIX timestamps") {
    // 2000-01-01 00:00:00 UTC = 946684800
    CHECK(to_time_t(2000, 1, 1, 0, 0, 0) == 946684800);

    // 1999-12-31 23:59:59 UTC = 946684799
    CHECK(to_time_t(1999, 12, 31, 23, 59, 59) == 946684799);

    // 2020-01-01 (leap year)
    CHECK(to_time_t(2020, 1, 1, 0, 0, 0) == 1577836800);
  }

  SUBCASE("Leap year transitions") {
    // Feb 28 → Feb 29 (leap year)
    CHECK(to_time_t(2020, 2, 29, 0, 0, 0) ==
          to_time_t(2020, 2, 28, 0, 0, 0) + 86400);

    // Feb 28 → Mar 1 (non-leap year)
    CHECK(to_time_t(2019, 3, 1, 0, 0, 0) ==
          to_time_t(2019, 2, 28, 0, 0, 0) + 86400);

    // 2000-03-01 (day after leap day)
    CHECK(to_time_t(2000, 3, 1, 0, 0, 0) ==
          to_time_t(2000, 2, 29, 0, 0, 0) + 86400);
  }

  SUBCASE("Month boundaries") {
    CHECK(to_time_t(2023, 2, 1, 0, 0, 0) > to_time_t(2023, 1, 31, 23, 59, 59));

    CHECK(to_time_t(2023, 3, 1, 0, 0, 0) > to_time_t(2023, 2, 28, 23, 59, 59));
  }
}

TEST_CASE("day_of_week") {
  CHECK(day_of_week(0, 0) == 3);  // 1970-01-01 00:00:00 -> Thursday
  CHECK(day_of_week(1661194800, 2 * 3600) == 0);  // Mon Aug 22 2022 21:00:00 GMT+0200
  CHECK(day_of_week(1661036400, 2 * 3600) == 6);  // Sun Aug 21 2022 01:00:00 GMT+0200
  CHECK(day_of_week(1661036400, 0) == 5);  // Sat Aug 20 2022 23:00:00 GMT+0000
}

#endif
