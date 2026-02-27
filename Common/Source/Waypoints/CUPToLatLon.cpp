/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "Compiler.h"
#include "options.h"
#include <ctype.h>
#include "tchar.h"

constexpr double invalid_angle = -9999;

double CUPToLat(const char* str) {
  // latitude is 4555.0X minimum
  if (!str) {
    return invalid_angle;
  }

  int degree = 0;
  // first 2 digit are degree
  if (isdigit(str[0]) && isdigit(str[1])) {
    degree = (str[0] - '0') * 10 + (str[1] - '0');
  }
  else {
    return invalid_angle;
  }

  int minute = 0;
  // next 2 digit are minute
  if (isdigit(str[2]) && isdigit(str[3])) {
    minute = (str[2] - '0') * 10 + (str[3] - '0');
  }
  else {
    return invalid_angle;
  }
  // next char is dot
  if (str[4] != '.') {
    return invalid_angle;
  }

  int divisor = 1;
  int radix = 0;
  // next digit are decimal minutes
  const char* next = &str[5];
  for (; isdigit(*next); ++next) {
    radix = radix * 10 + (*next - '0');
    divisor *= 10;
  }

  double angle = degree + ((minute + (static_cast<double>(radix) / divisor)) / 60.);
  if ((*next) == 'S') {
    return angle * -1;
  }
  else if ((*next) == 'N') {
    return angle;
  }
  // error
  return invalid_angle;
}

double CUPToLon(const char* str) {
  // longitude can be 01234.5X
  if (!str) {
    return invalid_angle;
  }

  int degree = 0;
  // first 3 digit are degree
  if(isdigit(str[0]) && isdigit(str[1]) && isdigit(str[2])) {
    degree = (str[0] - '0') * 100 + (str[1] - '0') * 10 + (str[2] - '0');
  } else {
    return invalid_angle;
  }

  int minute = 0;
  // next 2 digit are minute
  if (isdigit(str[3]) && isdigit(str[4])) {
    minute = (str[3] - '0') * 10 + (str[4] - '0');
  }
  else {
    return invalid_angle;
  }
  // next char is dot
  if(str[5] != _T('.')) {
    return invalid_angle;
  }

  int divisor = 1;
  int radix = 0;
  // next digit are decimal minutes
  const char* next = &str[6];
  for (; isdigit(*next); ++next) {
    radix = radix * 10 + (*next - _T('0'));
    divisor *= 10;
  }

  double angle = degree + ((minute + (static_cast<double>(radix) / divisor)) / 60.);
  if((*next) == 'W') {
    return angle * -1;
  } else if((*next) == 'E') {
    return angle;
  }
  // error
  return invalid_angle;
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("CUPToLatLon") {

	SUBCASE("CUPToLat") {
		CHECK(CUPToLat("4555.5555N") == doctest::Approx(45.925925).epsilon(0.0000001));
		CHECK(CUPToLat("4555.0S") == doctest::Approx(-45.916667).epsilon(0.0000001));
		CHECK(CUPToLat("955.0S") == invalid_angle);
		CHECK(CUPToLat("9555.55x") == invalid_angle);
		CHECK(CUPToLat("9555.55") == invalid_angle);
		CHECK(CUPToLat("9555") == invalid_angle);
		CHECK(CUPToLat("955") == invalid_angle);
		CHECK(CUPToLat("") == invalid_angle);
		CHECK(CUPToLat(nullptr) == invalid_angle);
	}

	SUBCASE("CUPToLon") {
		CHECK(CUPToLon("12555.5555E") == doctest::Approx(125.925925).epsilon(0.0000001));
		CHECK(CUPToLon("04555.0W") == doctest::Approx(-45.916667).epsilon(0.0000001));
		CHECK(CUPToLon("9555.0S") == invalid_angle);
		CHECK(CUPToLon("95555.55x") == invalid_angle);
		CHECK(CUPToLon("99555.55") == invalid_angle);
		CHECK(CUPToLon("9555") == invalid_angle);
		CHECK(CUPToLon("955") == invalid_angle);
		CHECK(CUPToLon("") == invalid_angle);
		CHECK(CUPToLon(nullptr) == invalid_angle);
	}
}
#endif
