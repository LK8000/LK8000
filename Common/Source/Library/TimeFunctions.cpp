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
  int utc_time = WithLock(CritSec_FlightData, []() {
    return GPS_INFO.Time;
  });
  return LocalTime(utc_time);
}

namespace {

constexpr bool is_leap(int y) {
  return (y % 4 == 0) && (y % 100 != 0 || y % 400 == 0);
}

constexpr int days_before_year(int y) {
  // Total days from year 1 to start of year y
  --y;
  return y * 365 + y / 4 - y / 100 + y / 400;
}

constexpr int days_before_1970 = days_before_year(1970);

constexpr int days_before_month(int year, int mon) {
  // mon = 1..12
  constexpr int cum[12] = {
      0,    // Jan
      31,   // Feb
      59,   // Mar
      90,   // Apr
      120,  // May
      151,  // Jun
      181,  // Jul
      212,  // Aug
      243,  // Sep
      273,  // Oct
      304,  // Nov
      334   // Dec
  };

  int d = cum[mon - 1];
  if (mon > 2 && is_leap(year)) {
    ++d;
  }
  return d;
}

} // namespace

time_t to_time_t(int year, int mon, int mday, int hour, int min,
                           int sec) {
  // mon = 1..12, mday = 1..31 (no validation required)
  int days = days_before_year(year) - days_before_1970;
  days += days_before_month(year, mon);
  days += (mday - 1);
  return time_t(days) * 86400 + time_t(hour) * 3600 + time_t(min) * 60 + sec;
}


// ============================================================
//                      STATIC ASSERT TESTS
// ============================================================
static_assert(is_leap(2000) == true,  "Year 2000 must be leap");
static_assert(is_leap(1900) == false, "Year 1900 must NOT be leap");
static_assert(is_leap(1996) == true,  "Year 1996 must be leap");
static_assert(is_leap(1999) == false, "Year 1999 must not be leap");
// ============================================================


time_t to_time_t(const NMEA_INFO& info) {
  return to_time_t(info.Year, info.Month, info.Day, 
                   info.Hour, info.Minute, info.Second);
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

  SUBCASE("Century leap year rules") {
    CHECK(is_leap(2000) == true);   // divisible by 400
    CHECK(is_leap(1900) == false);  // divisible by 100 but NOT 400
    CHECK(is_leap(2100) == false);

    // 2000-03-01 vs 2000-02-29
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
