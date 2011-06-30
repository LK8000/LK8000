//#include "map.h"
/*
Copyright_License {

  XCSoar Glide Computer - http://xcsoar.sourceforge.net/
  Copyright (C) 2000 - 2005  

  	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@bigfoot.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

}
*/
#include "StdAfx.h"

#include <stdlib.h>
#include "maperror.h"

#include <limits.h>

#include "utils/heapcheck.h"

/* originally found at http://www.snippets.org/ */

size_t msGetBitArraySize(int numbits)
{
  return((numbits + CHAR_BIT - 1) / CHAR_BIT);
}

char *msAllocBitArray(int numbits)
{
  char *array = (char*)calloc((numbits + CHAR_BIT - 1) / CHAR_BIT, sizeof(char));
  
  return(array);
}

int msGetBit(char *array, int index)
{
  array += index / CHAR_BIT;
  return (*array & (1 << (index % CHAR_BIT))) != 0;    /* 0 or 1 */
}

void msSetBit(char *array, int index, int value)
{
  array += index / CHAR_BIT;
  if (value)
    *array |= 1 << (index % CHAR_BIT);           /* set bit */
  else    
    *array &= ~(1 << (index % CHAR_BIT));        /* clear bit */
}

void msFlipBit(char *array, int index)
{
  array += index / CHAR_BIT;
  *array ^= 1 << (index % CHAR_BIT);                   /* flip bit */
}
