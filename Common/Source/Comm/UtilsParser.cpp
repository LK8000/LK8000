/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include <ctype.h>


/*
 * Copy a provided string into the supplied buffer, terminate on
 * the checksum separator, split into an array of parameters,
 * and return the number of parameters found.
 */
namespace {

template<typename CharT>
void ExtractParameter(const CharT *Source, CharT *Destination, int DesiredFieldNumber) {
  if (!Destination || !Source) {
    return;
  }

  auto isSeparator = [](CharT c) -> bool {
    return (c == ',' || c == '*');
  };

  // find start of requested field
  for (int nField = 0; *Source && nField < DesiredFieldNumber; ) {
    if (isSeparator(*Source++)) {
      nField++;
    }
  }

  // copy until next field
  while (*Source && !isSeparator(*Source)) {
    (*Destination++) = (*Source++);
  }
  *Destination = '\0'; // add trailing '\0'
}

} // namespace

void NMEAParser::ExtractParameter(const char* Source, char* Destination, int DesiredFieldNumber) {
  ::ExtractParameter(Source, Destination, DesiredFieldNumber);
}

/*
 * Same as ExtractParameters, but also validate the length of
 * the string and the NMEA checksum.
 */
size_t NMEAParser::ValidateAndExtract(const char* src, char* dst, size_t dstsz, char** arr, size_t arrsz) {
  if (!NMEAChecksum(src))
    return 0;

  return ExtractParameters(src, dst, arr, arrsz);
}

size_t NMEAParser::ExtractParameters(const char* src, char* dst, char** arr, size_t sz) {
  if (!src || !(*src)) {
    return 0;
  }

  size_t i = 0;
  arr[i++] = dst;

  while (*src && *src != '*') {
    if (*src == ',') {
      *dst = '\0';
      arr[i++] = dst + 1;
    } else {
      *dst = *src;
    }
    ++src;
    ++dst;
  }
  *dst = '\0';
  return i;
}

double NMEAParser::ParseAltitude(const char* value, const char* format) {
  if (!value || !format) {
    TestLog(_T(".... ParseAltitude: null value or null format!"));
    BUGSTOP_LKASSERT(0);
    return 0;
  }

  double alt = StrToDouble(value, nullptr);
  if (format[0] == 'f' || format[0] == 'F') {
    alt /= TOFEET;
  }
  return alt;
}

BOOL NMEAParser::NMEAChecksum(const char *String) {
  if (!CheckSum) {
    return TRUE;
  }

  uint8_t CalcCheckSum = 0;
  uint8_t ReadCheckSum = 0;

  if (*String && *String == '$') {
    ++String;
  }

  while (*String && *String != '*') {
    CalcCheckSum = (unsigned char)(CalcCheckSum ^ *(String++));
  }

  if (*String == '*') {
    while(*String) {
      uint8_t digit = HexDigit(*(++String));
      if (0xFF == digit) {
        break;
      }
      ReadCheckSum = (ReadCheckSum << 4)|digit;
    }
  } else {
    return FALSE; // no checksum
  }
  
  return (CalcCheckSum == ReadCheckSum);
}

#ifdef UNICODE

void NMEAParser::ExtractParameter(const wchar_t* Source, wchar_t* Destination, int DesiredFieldNumber) {
  ::ExtractParameter(Source, Destination, DesiredFieldNumber);
}

#endif

double EastOrWest(double in, TCHAR EoW)
{
  if(EoW == 'W')
    return -in;
  else
    return in;
}



double NorthOrSouth(double in, TCHAR NoS)
{
  if(NoS == 'S')
    return -in;
  else
    return in;
}

double MixedFormatToDegrees(double mixed)
{
  double mins, degrees;

  degrees = (int) (mixed/100);
  mins = (mixed - degrees*100)/60;

  return degrees+mins;
}

uint8_t NMEAParser::AppendChecksum(char *String, size_t size) {
  constexpr char hex_chars[16] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
  };
  
  if(!String) {
    return 0xFF;
  }
  
  char* it = String;
  char* end = std::next(String,size);

  if (*(it++) != '$') {
    return -1;
  }
  uint8_t iCheckSum = 0;
  while( (*it) != '\0' && it != end ) {
    iCheckSum ^= *(it++);
  }
  if(std::distance(it, end) < 5) {
    assert(false); // buffer overflow
    return 0xFF;
  }
  
  *(it++) = '*';
  *(it++) = (hex_chars[(iCheckSum & 0xF0) >> 4]);
  *(it++) = (hex_chars[(iCheckSum & 0x0F) >> 0]);  
  *(it++) = '\r';
  *(it++) = '\n';
  *(it) = '\0';

  return iCheckSum;  
}
