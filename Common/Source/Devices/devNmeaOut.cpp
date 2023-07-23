/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: devNmeaOut.cpp,v 8.2 2011/01/02 00:32:55 root Exp root $
*/

#include "externs.h"
#include <regex>

static 
BOOL NMEAOut(DeviceDescriptor_t* d, const TCHAR* String) {
  if(d) {
    try {
      std::regex re(R"(((?!\r)\n|\r(?!\n)))"); // to fix end of line...
      d->Com->WriteString(std::regex_replace(to_utf8(String), re, "\r\n").c_str());
    } catch (std::exception& e) {
      StartupStore(_T("NMEAOut : %s"), to_tstring(e.what()).c_str());
    }
  }
  return TRUE;
}

void nmoInstall(DeviceDescriptor_t* d){
  _tcscpy(d->Name, TEXT("NmeaOut"));
  d->NMEAOut = NMEAOut;
}
