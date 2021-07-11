/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include <ctype.h>
#include "tchar.h"

constexpr double invalid_angle = -9999;

double CUPToLat(const TCHAR *str) {
  // latitude is 4555.0X minimum
  if (!str) {
	return invalid_angle;
  }

  int degree = 0;
  // first 2 digit are degree
  if(_istdigit(str[0]) && _istdigit(str[1])) {
    degree = (str[0] - _T('0'))*10 + (str[1] - _T('0'));
  } else {
    return invalid_angle;
  }

  int minute = 0;
  // next 2 digit are minute
  if(_istdigit(str[2]) && _istdigit(str[3])) {
    minute = (str[2] - _T('0'))*10 + (str[3] - _T('0'));
  } else {
    return invalid_angle;
  }
  // next char is dot
  if(str[4] != _T('.')) {
    return invalid_angle;
  }

  int divisor = 1;
  int radix = 0;
  // next digit are decimal minutes
  const TCHAR* next = &str[5];
  for(; _istdigit(*next); ++next) {
    radix = radix * 10 + (*next - _T('0'));
	divisor *= 10;
  }

  double angle = degree + ((minute + (static_cast<double>(radix) / divisor)) / 60.);
  if((*next) == _T('S')) {
    return angle * -1;
  } else if((*next) == _T('N')) {
    return angle;
  } 
  // error
  return invalid_angle;
}

double CUPToLon(const TCHAR *str) {
  // longitude can be 01234.5X
  if (!str) {
    return invalid_angle;
  }

  int degree = 0;
  // first 3 digit are degree
  if(_istdigit(str[0]) && _istdigit(str[1]) && _istdigit(str[2])) {
    degree = (str[0] - _T('0'))*100 + (str[1] - _T('0'))*10 + (str[2] - _T('0'));
  } else {
    return invalid_angle;
  }

  int minute = 0;
  // next 2 digit are minute
  if(_istdigit(str[3]) && _istdigit(str[4])) {
    minute = (str[3] - _T('0'))*10 + (str[4] - _T('0'));
  } else {
    return invalid_angle;
  }
  // next char is dot
  if(str[5] != _T('.')) {
    return invalid_angle;
  }

  int divisor = 1;
  int radix = 0;
  // next digit are decimal minutes
  const TCHAR* next = &str[6];
  for(; _istdigit(*next); ++next) {
	radix = radix * 10 + (*next - _T('0'));
	divisor *= 10;
  }

  double angle = degree + ((minute + (static_cast<double>(radix) / divisor)) / 60.);
  if((*next) == _T('W')) {
    return angle * -1;
  } else if((*next) == _T('E')) {
    return angle;
  } 
  // error
  return invalid_angle;
}
