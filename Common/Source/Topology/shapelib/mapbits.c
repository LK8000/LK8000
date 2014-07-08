/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

   This part of the code is taken from ShapeLib 1.1.5
   Copyright (c) 1999, Frank Warmerdam

   This software is available under the following "MIT Style" license, or at the option 
   of the licensee under the LGPL (see LICENSE.LGPL). 
   This option is discussed in more detail in shapelib.html.

   Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
   and associated documentation files (the "Software"), to deal in the Software without restriction, 
   including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
   subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all copies 
   or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
   PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
   FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include <stdlib.h>
#include <limits.h>
#include "options.h"

#include "utils/heapcheck.h"

#if MAPSHAPEERROR
/* originally found at http://www.snippets.org/ */
size_t msGetBitArraySize(int numbits)
{
  return((numbits + CHAR_BIT - 1) / CHAR_BIT);
}
#endif

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

#if MAPSHAPEERROR
void msFlipBit(char *array, int index)
{
  array += index / CHAR_BIT;
  *array ^= 1 << (index % CHAR_BIT);                   /* flip bit */
}
#endif
