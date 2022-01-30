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
size_t NMEAParser::ExtractParameters(const TCHAR *src, TCHAR *dst, TCHAR **arr, size_t sz)
{
  TCHAR c, *p;
  size_t i = 0;

  _tcscpy(dst, src);
  p = _tcschr(dst, _T('*'));
  if (p)
    *p = _T('\0');

  p = dst;
  do {
    arr[i++] = p;
    p = _tcschr(p, _T(','));
    if (!p)
      break;
    c = *p;
    *p++ = _T('\0');
  } while (i != sz && c != _T('\0'));

  return i;
}




/*
 * Same as ExtractParameters, but also validate the length of
 * the string and the NMEA checksum.
 */
size_t NMEAParser::ValidateAndExtract(const TCHAR *src, TCHAR *dst, size_t dstsz, TCHAR **arr, size_t arrsz)
{
  size_t len = _tcslen(src);

  if (len <= 6 || len >= dstsz)
    return 0;
  if (!NMEAChecksum(src))
    return 0;

  return ExtractParameters(src, dst, arr, arrsz);
}




void NMEAParser::ExtractParameter(const TCHAR *Source, 
				  TCHAR *Destination, 
				  int DesiredFieldNumber)
{
  int dest_index = 0;
  int CurrentFieldNumber = 0;
  int StringLength = _tcslen(Source);
  const TCHAR *sptr = Source;
  const TCHAR *eptr = Source+StringLength;

  if (!Destination) return;

  while( (CurrentFieldNumber < DesiredFieldNumber) && (sptr<eptr) )
    {
      if (*sptr == ',' || *sptr == '*' )
        {
          CurrentFieldNumber++;
        }
      ++sptr;
    }

  Destination[0] = '\0'; // set to blank in case it's not found..

  if ( CurrentFieldNumber == DesiredFieldNumber )
    {
      while( (sptr < eptr)    &&
             (*sptr != ',') &&
             (*sptr != '*') &&
             (*sptr != '\0') )
        {
          if(IsASCII((char)*sptr)) {
            Destination[dest_index] = *sptr;
          }  else  {
            Destination[dest_index] = '_'; 
					}
          ++sptr; ++dest_index;
        }
      Destination[dest_index] = '\0';
    }
}




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



int NAVWarn(TCHAR c)
{
  if(c=='A')
    return FALSE;
  else
    return TRUE;
}




double NMEAParser::ParseAltitude(TCHAR *value, const TCHAR *format)
{
  if (value==NULL || format==NULL) {
#if TESTBENCH
  StartupStore(_T(".... ParseAltitude: null value or null format!\n"));
#endif
    BUGSTOP_LKASSERT(0);
    return 0;
  }

  double alt = StrToDouble(value, NULL);
  if (format[0] == _T('f') || format[0] == _T('F')) {
    alt /= TOFEET;
  }
  return alt;
}




double MixedFormatToDegrees(double mixed)
{
  double mins, degrees;

  degrees = (int) (mixed/100);
  mins = (mixed - degrees*100)/60;

  return degrees+mins;
}




BOOL NMEAParser::NMEAChecksum(const TCHAR *String)
{
  if (!CheckSum) return TRUE;
  unsigned char CalcCheckSum = 0;
  unsigned char ReadCheckSum;
  int End;
  int i;
  TCHAR c1,c2;
  unsigned char v1 = 0,v2 = 0;

  const TCHAR *pEnd = _tcschr(String,_T('*'));
  if(pEnd == NULL)
    return FALSE;

  // Fix problem of EW micrologger missing a digit in checksum
  // now we have *XY 
  // line is terminating with 0a (\n) so count is 4 not 3!
  if(_tcslen(pEnd)<4) {
	// no checksum, only a * ?
	if (_tcslen(pEnd)==1) {
		return FALSE;
	}
	// try to recover the missing digit
	c1 = _T('0');
	c2 = pEnd[1];
  } else {
	c1 = pEnd[1], c2 = pEnd[2];
  }

  if(_istdigit(c1))
    v1 = (unsigned char)(c1 - '0');
  if(_istdigit(c2))
    v2 = (unsigned char)(c2 - '0');
  if(_istalpha(c1))
    v1 = (unsigned char)(toupper(c1) - 'A' + 10);
  if(_istalpha(c2))
    v2 = (unsigned char)(toupper(c2) - 'A' + 10);

  ReadCheckSum = (unsigned char)((v1<<4) + v2);          

  End =(int)( pEnd - String);

  for(i=1;i<End;i++)
    {
      CalcCheckSum = (unsigned char)(CalcCheckSum ^ String[i]);
    }

  if(CalcCheckSum == ReadCheckSum)
    return TRUE;
  else
    return FALSE;
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
