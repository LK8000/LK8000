/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: devNmeaOut.cpp,v 8.2 2011/01/02 00:32:55 root Exp root $
*/

#include "externs.h"
#include "devNmeaOut.h"
#include <regex>


BOOL NmeaOut::NMEAOut(DeviceDescriptor_t* d, const char* String) {
  if(d) {
    try {
      static const std::regex re(R"((?:!\r)\n|\r(?:!\n))", std::regex_constants::multiline); // to fix end of line...
      d->Com->WriteString(std::regex_replace(String, re, "\r\n"));
    } catch (std::exception& e) {
      StartupStore(_T("NMEAOut : %s"), to_tstring(e.what()).c_str());
    }
  }
  return TRUE;
}

void NmeaOut::Install(DeviceDescriptor_t* d){
  _tcscpy(d->Name, Name);
  d->NMEAOut = NMEAOut;
}
