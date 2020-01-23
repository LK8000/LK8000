#include "doctest/doctest.h"
#include "Comm\GpsWeekNumberFix.h"
#include <assert.h>

// Convert integer date components to NMEA $GPRMC date string
const char* int2gprmc(uint16_t year, uint16_t month, uint16_t day)
{
	assert(year >= 00 && year <= 99 && month >= 1 && month <= 12
		&& day >= 1 && day <= 31);
	static char gprmc[7];
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
                    const char *gprmc = int2gprmc(year, month, day);
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