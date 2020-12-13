/******************************************************************************
 *
 * Copyright (C) u-blox AG
 * u-blox AG, Thalwil, Switzerland
 *
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice is
 * included in all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY. IN PARTICULAR, NEITHER THE AUTHOR NOR U-BLOX MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF
 * THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 *****************************************************************************/

#include "options.h"
#include "GpsWeekNumberFix.h"
#include <cassert>

/** 
 * GPS week number roll-over workaround
 * Application Note : UBX-19016936 - R01 - 16-Jul-2019
 * https://www.u-blox.com/sites/default/files/GPS-WeekNumber-Rollover-Workaround_AppNote_%28UBX-19016936%29.pdf
 */


/**
 * known day_of_year for each month:
 * Major index 0 is for non-leap years, and 1 is for leap years
 * Minor index is for month number 1 .. 12, 0 at index 0 is number of days
 * before January
 */
static constexpr int32_t month_days[2][13] = {
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}};

/**
 * Count the days since start of 1980
 * Counts year * 356 days + leap days + month lengths + days in month
 * The leap days counting needs the "+ 1" because GPS year 0 (i.e. 1980) was a leap year
 */
static int32_t day_number_1980(int32_t year, int32_t month, int32_t day) {
  int32_t gps_years = year - 1980;
  int32_t leap_year = (gps_years % 4 == 0) ? 1 : 0;
  int32_t day_of_year = month_days[leap_year][month - 1] + day;
  if (gps_years == 0) {
    return day_of_year;
  }
  return gps_years * 365 + ((gps_years - 1) / 4) + 1 + day_of_year;
}

/**
 * Convert day_number since start of 1980 to year, month, and day:
 * - integer division of (day_number - 1) by 365.25 gives year number for 1980 to 2099
 * - day number - (year number * 365 days + leap days) gives day of year
 *   The leap days needs "+ 1" because GPS year 0 (i.e. 1980) was a leap year
 * - (day_of_year - 1) / 31 + 1 gives lower limit for month, but this may be one too low
 *   the guessed month is adjusted by checking the month lengths
 * - days in month is left when the month lengths are subtracted
 * - year must still be adjusted by 1980
 */
static void date_1980(int32_t day_number, int32_t &year, int32_t &month,
               int32_t &day) {
  int32_t gps_years = ((day_number - 1) * 100) / 36525;
  int32_t leap_year = (gps_years % 4 == 0) ? 1 : 0;
  int32_t day_of_year = day_number;
  if (gps_years > 0) {
    day_of_year = day_number - (gps_years * 365 + ((gps_years - 1) / 4) + 1);
  }
  int32_t month_of_year = (day_of_year - 1) / 31 + 1;
  if (day_of_year > month_days[leap_year][month_of_year]) {
    month_of_year++;
  }
  day = day_of_year - month_days[leap_year][month_of_year - 1];
  month = month_of_year;
  year = 1980 + gps_years;
}

/**
 * workaround for the GPS week number roll-over issue
 */
static void week_number_rollover_workaround(int32_t &year, int32_t &month, int32_t &day) {
  // calculate how many days there are since start of 1980
  int32_t day_num = day_number_1980(year, month, day);

  // adjust date only if it is before previous GPS week number roll-over
  // which happened 2019-04-06. That is 14341 days after start of 1980
  if (day_num <= 14341) {
    day_num = day_num + 1024 * 7;
  }

  // convert the day number back to integer year, month and day
  date_1980(day_num, year, month, day);
}

/**
 * convert character ['0' .. '9'] to uint32_t
 */
static int32_t to_num(TCHAR c) {
  return (c - _T('0'));
}

bool parse_rmc_date(const TCHAR *gprmc, size_t gprmc_size, int32_t &year, int32_t &month, int32_t &day) {
  if (gprmc_size >= 6) {
    day = 10 * to_num(gprmc[0]) + to_num(gprmc[1]);
    month = 10 * to_num(gprmc[2]) + to_num(gprmc[3]);
    year = 10 * to_num(gprmc[4]) + to_num(gprmc[5]);
    if (year >= 0 && year <= 99 && month >= 1 && month <= 12 && day >= 1 && day <= 31) {
      // NMEA $GPRMC year number has only 2 digits
      year = year + ((year > 79) ? 1900 : 2000);
      week_number_rollover_workaround(year, month, day);

      return true;
    }
  }
  return false;
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include <string.h>

// Convert integer date components to NMEA $GPRMC date string
static const TCHAR* int2gprmc(uint16_t year, uint16_t month, uint16_t day)
{
	assert(year >= 00 && year <= 99 && month >= 1 && month <= 12
		&& day >= 1 && day <= 31);
	static TCHAR gprmc[7];
	gprmc[0] = '0' + (day / 10);
	gprmc[1] = '0' + (day % 10);
	gprmc[2] = '0' + (month / 10);
	gprmc[3] = '0' + (month % 10);
	gprmc[4] = '0' + (year / 10);
	gprmc[5] = '0' + (year % 10);
	gprmc[6] = '\0';
	return gprmc;
}


TEST_CASE("rmc parse date with week rollover workarround") {
	int32_t _year;
	int32_t _month;
	int32_t _day;

	SUBCASE("any valid input string from 010100 to 311299") {
		for (uint16_t year = 0; year <= 99; year++) {
			for (uint16_t month = 1; month <= 12; month++) {
				for (uint16_t day = 1; day <= 31; day++) {
          const TCHAR *gprmc = int2gprmc(year, month, day);
					CHECK(parse_rmc_date(gprmc, strlen(gprmc), _year, _month, _day));
				}
			}
		}

		SUBCASE("invalid string") {
			CHECK_FALSE(parse_rmc_date("aabbcc", 6, _year, _month, _day));
			CHECK_FALSE(parse_rmc_date("000000", 6, _year, _month, _day));
			CHECK_FALSE(parse_rmc_date("999999", 6, _year, _month, _day));
			CHECK_FALSE(parse_rmc_date("a", 1, _year, _month, _day));
		}

		SUBCASE("valid date  2019-04-07 (with rollover)") {
            CHECK(parse_rmc_date("220899", 6, _year, _month, _day));
			CHECK(_year == 2019);
			CHECK(_month >= 4);
			CHECK(_day >= 7);
		}

		SUBCASE("valid date  2019-04-07 (without rollover)") {
			CHECK(parse_rmc_date("070419", 6, _year, _month, _day));
			CHECK(_year == 2019);
			CHECK(_month >= 4);
			CHECK(_day >= 7);
		}
	}
}
#endif
