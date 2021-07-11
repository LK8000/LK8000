/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"

#include "devXCOM760.h"



static BOOL XCOM760IsRadio(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL XCOM760PutVolume(PDeviceDescriptor_t d, int Volume) {
  TCHAR  szTmp[32];
  _stprintf(szTmp, TEXT("$RVOL=%d\r\n"), Volume);
  d->Com->WriteString(szTmp);
  return(TRUE);
}


static BOOL XCOM760PutFreqActive(PDeviceDescriptor_t d, double Freq, const TCHAR* StationName) {
  TCHAR  szTmp[32];
  _stprintf(szTmp, TEXT("$TXAF=%.3f\r\n"), Freq);
  d->Com->WriteString(szTmp);
  return(TRUE);
}


static BOOL XCOM760PutFreqStandby(PDeviceDescriptor_t d, double Freq,  const TCHAR* StationName) {
  TCHAR  szTmp[32];
  _stprintf(szTmp, TEXT("$TXSF=%.3f\r\n"), Freq);
  d->Com->WriteString(szTmp);
  return(TRUE);
}


static BOOL XCOM760Install(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("XCOM760"));
  d->IsRadio = XCOM760IsRadio;
  d->PutVolume = XCOM760PutVolume;
  d->PutFreqActive = XCOM760PutFreqActive;
  d->PutFreqStandby = XCOM760PutFreqStandby;
  return(TRUE);

}


BOOL xcom760Register(void){
  return(devRegister(
    TEXT("XCOM760"),
    (1l << dfRadio),
    XCOM760Install
  ));
}


/* Commands

  $TOGG: return to main screen or toggle active and standby
  $DUAL=on/off
*/
