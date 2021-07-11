/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


static int ByteCRC16(int value, int crcin)
{
    int k = (((crcin >> 8) ^ value) & 255) << 8;
    int crc = 0;
    int bits = 8;
    do
    {
        if (( crc ^ k ) & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc <<= 1;
        k <<= 1;
    }
    while (--bits);
    return ((crcin << 8) ^ crc);
}

WORD crcCalc(void *Buffer, size_t size){

  int crc = 0;
  unsigned char *pB = (unsigned char *)Buffer;

  do {
    int value = *pB++;
    crc = ByteCRC16(value, crc);
  } while (--size);

  return((WORD)crc);
}
