/***********************************************************************
**
**   utils.cpp
**
**   This file is part of libkfrgcs.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id: vlutils.cpp,v 1.1 2007/09/14 17:13:24 jwharington Exp $
**
***********************************************************************/

#include <ctype.h>
#include <stdio.h>

#include "utils/heapcheck.h"

char *utoa(unsigned value, char *digits, int base)
{
    const char *s = "0123456789abcdefghijklmnopqrstuvwxyz";

    if (base == 0)
        base = 10;
    if (digits == NULL || base < 2 || base > 36)
        return NULL;
    if (value < (unsigned) base) {
        digits[0] = s[value];
        digits[1] = '\0';
    } else {
        char *p;
        for (p = utoa(value / ((unsigned)base), digits, base);
             *p;
             p++);
        utoa( value % ((unsigned)base), p, base);
    }
    return digits;
}

char *itoa(int value, char *digits, int base)
{
  unsigned u;

  char *d = digits;
  if(base == 0)  base = 10;

  if(digits == NULL || base < 2 || base > 36)  return NULL;
  if(value < 0)
    {
      *d++ = '-';
      u = -value;
    }
  else
      u = value;

  utoa(u, d, base);
  return digits;
}


char *ltoa(long value, char *digits, int base)
{
  char *d;
  unsigned u;

  d = digits;
  if(base == 0)  base = 10;

  if(digits == NULL || base < 2 || base > 36)  return NULL;

  if (value < 0)
    {
      *d++ = '-';
      u = -value;
    }
  else
      u = value;

  utoa(u, d, base);
  return digits;
}

char *strupr(char *str)
{
  char *string = str;

  if (str)
    {
      for ( ; *str; ++str)
          *str = toupper(*str);
    }

  return string;
}
