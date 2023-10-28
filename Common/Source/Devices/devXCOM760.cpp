/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"

#include "devXCOM760.h"


static BOOL XCOM760PutVolume(DeviceDescriptor_t* d, int Volume) {
  TCHAR  szTmp[32];
  _stprintf(szTmp, TEXT("$RVOL=%d\r\n"), Volume);
  d->Com->WriteString(szTmp);
  return(TRUE);
}


static BOOL XCOM760PutFreqActive(DeviceDescriptor_t* d, unsigned Freq, const TCHAR* StationName) {
  TCHAR  szTmp[32];
  double Mhz = Freq / 1000.0;
  _stprintf(szTmp, TEXT("$TXAF=%.3f\r\n"), Mhz);
  d->Com->WriteString(szTmp);
  return(TRUE);
}


static BOOL XCOM760PutFreqStandby(DeviceDescriptor_t* d, unsigned Freq,  const TCHAR* StationName) {
  TCHAR  szTmp[32];
  double Mhz = Freq / 1000.0;
  _stprintf(szTmp, TEXT("$TXSF=%.3f\r\n"), Mhz);
  d->Com->WriteString(szTmp);
  return(TRUE);
}


void XCOM760Install(DeviceDescriptor_t* d) {
  _tcscpy(d->Name, TEXT("XCOM760"));
  d->PutVolume = XCOM760PutVolume;
  d->PutFreqActive = XCOM760PutFreqActive;
  d->PutFreqStandby = XCOM760PutFreqStandby;
}


/* Commands

  $TOGG: return to main screen or toggle active and standby
  $DUAL=on/off
*/
