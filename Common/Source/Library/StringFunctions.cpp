/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include <ctype.h>
#include "Util/Clamp.hpp"

void PExtractParameter(TCHAR *Source, TCHAR *Destination, size_t dest_size, int DesiredFieldNumber)
{
  size_t index = 0;
  size_t dest_index = 0;
  int CurrentFieldNumber = 0;

  size_t StringLength = _tcslen(Source);

  while( (CurrentFieldNumber < DesiredFieldNumber) && (index < StringLength) ) {
    if ( Source[ index ] == ',' ) {
      CurrentFieldNumber++;
	}
    index++;
  }

  if ( CurrentFieldNumber == DesiredFieldNumber ) {
    while( (dest_index < dest_size) && (index < StringLength) &&
            (Source[ index ] != ',') && (Source[ index ] != 0x00) )
	{
	  Destination[dest_index] = Source[ index ];
	  index++; dest_index++;
	}
    assert(dest_index < dest_size);
    Destination[std::min(dest_index, dest_size-1)] = '\0';
  }
}

uint8_t HexDigit(TCHAR c) {
	constexpr size_t digit_table_symbol_count = 256;
	#define __ 0xFF
	constexpr uint8_t digit_table[digit_table_symbol_count] = {
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0x00 - 0x0F
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0x10 - 0x1F
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0x20 - 0x2F
		 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,__,__,__,__,__,__, // 0x30 - 0x3F
		__,10,11,12,13,14,15,__,__,__,__,__,__,__,__,__, // 0x40 - 0x4F
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0x50 - 0x5F
		__,10,11,12,13,14,15,__,__,__,__,__,__,__,__,__, // 0x60 - 0x6F
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0x70 - 0x7F
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0x80 - 0x8F
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0x90 - 0x9F
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0xA0 - 0xAF
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0xB0 - 0xBF
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0xC0 - 0xCF
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0xD0 - 0xDF
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__, // 0xE0 - 0xEF
		__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__  // 0xF0 - 0xFF
	};
	#undef __
	return digit_table[static_cast<uint8_t>(c)];
}

int HexStrToInt(const TCHAR *Source) {
	int nOut=0;
	while(1) {
		const unsigned int digit = HexDigit(*Source++);
		if (0xFF == digit)
			break;
		nOut = ((nOut << 4)&0xFFFFFFF0)|digit;
	}
	return nOut;
}

/**
 * Convert string to double
 * 
 * @str:    C-string beginning with the representation of a floating-point number.
 * 
 * @endptr: Reference to an already allocated object of type TCHAR*, 
 *          whose value is set by the function to the character that stops the scan.
 *          This parameter can also be a null pointer, in which case it is not used.
 * 
 * @return: On success, the function returns the converted floating point number as a value of type double.
 *          If no valid conversion could be performed, the function returns zero (0.0).
 * 
 * A valid floating point number is formated by an optional signe character (+ or -), 
 * followed by a sequence of digits, optionally containing a decimal-point character (.), 
 * optionally followed by a sequence of digits.
 */
double StrToDouble(const TCHAR *str, const TCHAR **endptr) {

  if (!str) {
    return 0.0;
  }

  int64_t num = 0;
  int64_t radix = 0;
  int64_t divisor = 1;
  int neg = 1;

  // skip leading whitespace characters
  while ((*str == _T(' ')) || (*str == _T('\t'))) {
    ++str;
  }
  // skip explicit '+'
  if(*str == _T('+')) {
    ++str;
  }

  if(*str == _T('\0')) {
    // for compatibility with previous version, 
    // we need to return without setting endptr in case of empty string
    return 0.;
  }

  if (*str == _T('-')) {
    neg = -1;
    ++str;
  }

  while ((*str >= _T('0')) && (*str <= _T('9'))) {
    num = (num * 10) + (*str - _T('0'));
    ++str;
  }

  if (*str == _T('.')) {
    ++str;
    while ((*str >= _T('0')) && (*str <= _T('9'))) {
      radix = (radix * 10) + (*str - _T('0'));
      divisor *= 10;
      ++str;
    }
  }

  if(endptr) {
    *endptr = str;
  }

  return (((double) num) + ((double) radix / (double)divisor)) * (double)neg;
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("testing the StrToDouble function") {

	SUBCASE("Valid literal string imput") {
		CHECK(StrToDouble("0", nullptr) == 0);
		CHECK(StrToDouble("1", nullptr) == 1);
		CHECK(StrToDouble("2.2", nullptr) == 2.2);
		CHECK(StrToDouble("1234567890.0123456789", nullptr) == 1234567890.0123456789);
		CHECK(StrToDouble("-1.1", nullptr) == -1.1);
		CHECK(StrToDouble("  -1.1", nullptr) == -1.1);
		CHECK(StrToDouble("  1.1", nullptr) == 1.1);
		CHECK(StrToDouble("0.1", nullptr) == 0.1);
		CHECK(StrToDouble(".1", nullptr) == 0.1);
		CHECK(StrToDouble("6.6657667387", nullptr) == 6.6657667387);

		const char* sz = nullptr;
		CHECK(StrToDouble(".1", &sz) == 0.1);
		CHECK((sz && sz[0] == '\0') == true);
	}

	SUBCASE("partial Valid literal string imput") {
		CHECK(StrToDouble("0a", nullptr) == 0);
		CHECK(StrToDouble("1a", nullptr) == 1);
		CHECK(StrToDouble("2.2a", nullptr) == 2.2);
		CHECK(StrToDouble("1234567890.0123456789a", nullptr) == 1234567890.0123456789);
		CHECK(StrToDouble("-1.1a", nullptr) == -1.1);

    const char *sz = nullptr;
    CHECK(StrToDouble("0a", &sz) == 0);
    CHECK((sz && sz[0] == 'a') == true);

    CHECK(StrToDouble("1b", &sz) == 1);
    CHECK((sz && sz[0] == 'b') == true);

    CHECK(StrToDouble("2.2c", &sz) == 2.2);
    CHECK((sz && sz[0] == 'c') == true);

    CHECK(StrToDouble("1234567890.0123456789d", &sz) == 1234567890.0123456789);
    CHECK((sz && sz[0] == 'd') == true);

    CHECK(StrToDouble("-1.1e", &sz) == -1.1);
    CHECK((sz && sz[0] == 'e') == true);
  }

	SUBCASE("invalid literal string imput") {
		const char* sz = nullptr;
		CHECK(StrToDouble("a", &sz) == 0);
		CHECK((sz && sz[0] == 'a') == true);

		CHECK(StrToDouble(" ba", &sz) == 0);
		CHECK((sz && sz[0] == 'b') == true);

		CHECK(StrToDouble(",2", &sz) == 0);
		CHECK((sz && sz[0] == ',') == true);
	}

	SUBCASE("invalid literal empty string") {
		const char* sz = nullptr;
		CHECK(StrToDouble("", &sz) == 0);
		CHECK(sz == nullptr);

		CHECK(StrToDouble(nullptr, nullptr) == 0);
	}
}

#endif

// RMN: Volkslogger outputs data in hex-strings.  Function copied from StrToDouble
// Note: Decimal-point and decimals disregarded.  Assuming integer amounts only.
double HexStrToDouble(TCHAR *Source, TCHAR **Stop)
{
  int index = 0;
  int StringLength        = 0;
  double Sum = 0;
  int neg = 0;

  if (Source==NULL) return 0.0;
  StringLength = _tcslen(Source);

  while((index < StringLength) && ((Source[index] == ' ')||(Source[index]==9)))
    // JMW added skip for tab stop
    {
      index ++;
    }
  if (index >= StringLength) {
      goto _hexstrtodouble_return;
  }
  if (Source[index]=='-') {
    neg=1;
    index++;
  }

  while(
  (index < StringLength)	 &&
	(	( (Source[index]>= '0') && (Source [index] <= '9')  ) ||
		( (Source[index]>= 'A') && (Source [index] <= 'F')  ) ||
		( (Source[index]>= 'a') && (Source [index] <= 'f')  )
		)
	)
    {
      if((Source[index]>= '0') && (Source [index] <= '9'))	  {
		Sum = (Sum*16) + (Source[ index ] - '0');
		index ++;
	  }
	  if (index >= StringLength) goto _hexstrtodouble_return;
	  if((Source[index]>= 'A') && (Source [index] <= 'F'))	  {
		Sum = (Sum*16) + (Source[ index ] - 'A' + 10);
		index ++;
	  }
	  if (index >= StringLength) goto _hexstrtodouble_return;
	  if((Source[index]>= 'a') && (Source [index] <= 'f'))	  {
		Sum = (Sum*16) + (Source[ index ] - 'a' + 10);
		index ++;
	  }
    }

_hexstrtodouble_return:
  if(Stop != NULL) {
      if (index < StringLength) {
          *Stop = &Source[index];
      } else {
          *Stop = &Source[StringLength];
      }
  }

  if (neg) {
    return -Sum;
  } else {
    return Sum;
  }
}

// Trim trailing space
void TrimRight(TCHAR* str) {
    TCHAR * end = str + _tcslen(str);
    while(end > str && _istspace(*(end-1))) end--;
    // Write new null terminator
    *(end) = 0;
}

TCHAR* StringMallocParse(const TCHAR* old_string) {
  TCHAR buffer[2048];	// Note - max size of any string we cope with here !
  TCHAR* new_string;
  unsigned int used = 0;
  unsigned int i;

  // Transcode special characters
  for (i = 0; i < _tcslen(old_string); i++) {
    if (used < 2045) {
      if (old_string[i] == '\\' ) {
        if (old_string[i + 1] == 'r') {
          buffer[used++] = '\r';
          i++;
        } else if (old_string[i + 1] == 'n') {
          buffer[used++] = '\n';
          i++;
        } else if (old_string[i + 1] == '\\') {
          buffer[used++] = '\\';
          i++;
        } else {
          buffer[used++] = old_string[i];
        }
      } else {
        buffer[used++] = old_string[i];
      }
    }
  };
  // right trim string
  while(used > 0 && buffer[used-1] == _T(' ')) {
    --used;
  }
  buffer[used++] =_T('\0');

  TCHAR *pstart = _tcsstr(buffer, _T("_@M"));
  if (pstart==NULL) {
	goto _notoken;
  }

  // find end of token
  TCHAR *pnext;
  pnext = std::find(pstart+3, &buffer[used], _T('_'));
  if(pnext == &buffer[used]) {
    StartupStore(_T("...... Menu Label incorrect, invalid token: <%s>" NEWLINE),buffer);
    goto _notoken; // invalid token
  }
  if(std::distance(pstart, pnext) > 9) {
	StartupStore(_T("...... Menu Label incorrect, token too big: <%s>" NEWLINE),buffer);
	goto _notoken;
  }
  ++pnext;

  TCHAR lktoken[10];
  *std::copy(pstart, pnext, lktoken) = _T('\0');
  *pstart = _T('\0');

  const TCHAR *tokentext;
  tokentext = LKGetText(lktoken);
  size_t new_len;
  new_len = _tcslen(buffer) + _tcslen(tokentext) + _tcslen(pnext);
  new_string = (TCHAR *)malloc((new_len+1)*sizeof(TCHAR));
  if (new_string==NULL) {
	OutOfMemory(_T(__FILE__),__LINE__);
	return NULL;
  }
  _stprintf(new_string,_T("%s%s%s"), buffer, tokentext, pnext);
  return new_string;

_notoken:
  new_string = (TCHAR *)malloc((_tcslen(buffer)+1)*sizeof(TCHAR));
  if (new_string==NULL) {
	OutOfMemory(_T(__FILE__),__LINE__);
	return NULL;
  }
  _tcscpy(new_string, buffer);
  return new_string;
}

/*
 * Implementation of the _splitpath runtime library function with wide character strings
 * Copyright 2000, 2004 Martin Fuchs -- GPL licensed - WINE project
 */
void LK_tsplitpath(const TCHAR* path, TCHAR* drv, TCHAR* dir, TCHAR* name, TCHAR* ext)
{
	const TCHAR* end; /* end of processed string */
	const TCHAR* p;	  /* search pointer */
	const TCHAR* s;	  /* copy pointer */

	/* extract drive name */
	if (path[0] && path[1]==':') {
		if (drv) {
			*drv++ = *path++;
			*drv++ = *path++;
			*drv = '\0';
		} else path+=2;
	} else if (drv)
		*drv = '\0';

	/* search for end of string or stream separator */
	for(end=path; *end && *end!=':'; )
		end++;

	/* search for begin of file extension */
	for(p=end; p>path && *--p!='\\' && *p!='/'; )
		if (*p == '.') {
			end = p;
			break;
		}

	if (ext)
		for(s=end; (*ext=*s++); )
			ext++;

	/* search for end of directory name */
	for(p=end; p>path; )
		if (*--p=='\\' || *p=='/') {
			p++;
			break;
		}

	if (name) {
		for(s=p; s<end; )
			*name++ = *s++;

		*name = '\0';
	}

	if (dir) {
		for(s=path; s<p; )
			*dir++ = *s++;

		*dir = '\0';
	}
}

//
// v5 NOTE about LK_tcsncpy
// Remember: the destination must ALWAYS be sized numchars+1 at least!!
// We set a safe 0 in dest[numchars] position. Careful!
//

#if USELKASSERT
void LK_tcsncpy_internal(TCHAR *dest, const TCHAR *src, const unsigned int numofchars, const unsigned int sizedest, const int line, const TCHAR *filename)
#else
void LK_tcsncpy_internal(TCHAR *dest, const TCHAR *src, const unsigned int numofchars)
#endif
{

  #if USELKASSERT
  if (dest == NULL || sizedest==0) {
	StartupStore(_T("[ASSERT FAILURE (LK_tcsncpy dest)] in %s line %d, sizedest=%d") NEWLINE, filename, line,sizedest);
	return;
	//LKASSERT(false); // does not work during startup!
  }
  // We need space for 0 termination, otherwise we are in trobles
  // So numofchars must be < than the destination string.
  // Notice> we cannot check sizeof of a pointer, so anything with an address size instead of an array size
  // will be excluded. Better than nothing.
  if (  sizedest>sizeof(dest) && numofchars >= sizedest ) {
	StartupStore(_T("[ASSERT FAILURE (LK_tcsncpy dest)] in %s line %d dstsize=%d srcsize=%d") NEWLINE, filename, line,sizedest,numofchars);
	_tcsncpy(dest,src,sizedest-1);
	dest[sizedest-1] = '\0';
	return;
	//LKASSERT(false);
  }
  #endif

  // if source is null we safely ignore it, and give an empty string as result
  if (src == NULL || src[0]=='\0') {
	dest[0] = '\0';
	return;
  }

  _tcsncpy(dest,src,numofchars);
  dest[numofchars]='\0';

}

const TCHAR *AngleToWindRose(int angle) {

  if (angle < 0) {
    angle = angle%360 + 360;
  }
  
  // Valid index values: 0 - 16
  static const TCHAR * const windrose[] = {
    TEXT("N"), TEXT("NNE"), TEXT("NE"), TEXT("ENE"), 
    TEXT("E"), TEXT("ESE"), TEXT("SE"), TEXT("SSE"), 
    TEXT("S"), TEXT("SSW"), TEXT("SW"), TEXT("WSW"),
    TEXT("W"), TEXT("WNW"), TEXT("NW"), TEXT("NNW"), 
  };

  /*
   * tricks : for avoid rounding error with fixed point calculation, we use 360*4 for full circle instead of 360
   *   like that, slot_size = 90 instead of 22.5 and slot_offset = 45 instead of 11.25
   */  
  constexpr unsigned slot_size = 360 * 4 / std::size(windrose); // 22.5° for each sector
  constexpr unsigned slot_offset = slot_size / 2; // 11.25° offset 
  
  const unsigned index = ((angle * 4 + slot_offset) / slot_size) & 0x000F;
  return (windrose[index]);
}

///////////////////////////////////////////////////////////////////////
// Extract H, M, S from string like "HH:MM:SS"
//   Sec output parameter is optional
void StrToTime(LPCTSTR szString, int *Hour, int *Min, int *Sec) {
    LKASSERT(Hour && Min);
    TCHAR* sz = NULL;
    if (szString && szString[0]) {
        *Hour = Clamp<int>(_tcstol(szString, &sz, 10), 0, 23);
        if (*sz == _T(':')) {
            *Min = Clamp<int>(_tcstol(sz + 1, &sz, 10), 0, 59);

            if (Sec && (*sz == _T(':'))) {
                *Sec = Clamp<int>(_tcstol(sz + 1, &sz, 10), 0, 59);
            }
        }
    }
}
