/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: devNmeaOut.cpp,v 8.2 2011/01/02 00:32:55 root Exp root $
*/

#include "externs.h"

static 
BOOL NMEAOut(DeviceDescriptor_t *d, const TCHAR* String) {
  if(d) {
    while (*String) {
      // nmea is ASCII characters, a simple cast to char is enought TCHAR string.
      char c = *String++;
      if (c == '\n') {
        // insert missing <lf> before <cr>
        d->Com->Write('\r');
      } 
      else if (c == '\r') {
        d->Com->Write(c);
        c = *String++;
        if (c != '\n') {
          // insert missing <cr> after <lf>
          d->Com->Write('\n');
        }
      }
      d->Com->Write(c);
    }
  }
  return TRUE;
}

void nmoInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("NmeaOut"));
  d->NMEAOut = NMEAOut;
}
