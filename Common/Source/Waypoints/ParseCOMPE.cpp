/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "utils/array_back_insert_iterator.h"
#include "Util/UTF8.hpp"


//#define COMPEDEBUG 1

extern int globalFileNum;

bool ParseCOMPEWayPointString(const TCHAR *String, WAYPOINT *Temp) {
  TCHAR tComment[(COMMENT_SIZE * 2) + 1]; // must be bigger than COMMENT_SIZE!
  TCHAR tName[NAME_SIZE + 1];

  // int flags=0;
  bool ok;
  unsigned int startpoint = 3; // default

  size_t slen = _tcslen(String);
  if (slen < 65) {
#ifdef COMPEDEBUG
    if (slen > 0) {
      StartupStore(_T("TOO SHORT LINE:<%s> slen<65%s"), String, NEWLINE);
    }
#endif
    return false;
  }

  // only handle W field, format:  W__NAME
  if (String[0] != 'W') {
#ifdef COMPEDEBUG
    StartupStore(_T("COMPE IN:<%s> missing leading W%s"), String, NEWLINE);
#endif
    return false;
  }
  if (String[1] != ' ') {
#ifdef COMPEDEBUG
    StartupStore(_T("COMPE IN:<%s> missing space after W%s"), String, NEWLINE);
#endif
    return false;
  }

  // W wptname
  if (String[2] != ' ') {
    startpoint = 2; // third char
  }
  // W  wptname
  if (String[2] == ' ') {
    if (String[3] == ' ') {
#ifdef COMPEDEBUG
      StartupStore(_T("COMPE IN:<%s> missing character after W__%s"), String,
                   NEWLINE);
#endif
      return false;
    }
    startpoint = 3; // fourth char
  }

  Temp->Visible = true; // default all waypoints visible at start
  Temp->FarVisible = true;
  Temp->Format = LKW_COMPE;
  Temp->Number = WayPointList.size();
  Temp->FileNum = globalFileNum;

#ifdef COMPEDEBUG
  StartupStore(_T("COMPE IN:<%s>%s"), String, NEWLINE);
#endif

  // Name starts at position 3 or 4, index 2 or 3 . Search for space at the end
  // of name (<=)
  auto out = array_back_inserter(tName, std::size(tName) - 1); // size - 1 to let placeholder for '\0'
  unsigned i = startpoint;
  while(String[i] != _T(' ') && String[i] != _T('\0')) {
    out = String[i++];
  }
  tName[out.length()] = _T('\0');
#ifndef UNICODE
  if (out.overflowed()) {
      CropIncompleteUTF8(tName);
  }
#endif

  // i now point to first space after name
#ifdef COMPEDEBUG
  StartupStore(_T("WP NAME size=%d: <%s>%s"), (int)_tcslen(tName), tName, NEWLINE);
#endif

  if (String[++i] != _T('A')) {
#ifdef COMPEDEBUG
    StartupStore(_T("Missing A field! %s"), NEWLINE);
#endif
    return false;
  }
  if (String[++i] != _T(' ')) {
#ifdef COMPEDEBUG
    StartupStore(_T("Missing space after A field! %s"), NEWLINE);
#endif
    return false;
  }
  i++;

  // we are now on the first digit of latitude
#ifdef UNICODE
  const TCHAR szDeg[] = {0x00BA, 0x0000}; // UTF16
#else
  const TCHAR szDeg[] = {(TCHAR)0xC2, (TCHAR)0xBA, 0x00}; // UTF8
#endif
  // search for szDeg delimiter
  unsigned int p;
  ok = false;
  const TCHAR *cp = _tcsstr(&(String[i]), szDeg);
  if (cp) {
    p = cp - String;
    ok = true;
  }
  if (!ok) {
#ifdef COMPEDEBUG
    StartupStore(_T("Missing delimiter in latitude %s"), NEWLINE);
#endif
    return false;
  }
  // p points to delimiter

  // latitude from i to i+12, starting from i counts 13
  TCHAR tLatitude[16];
  if ((p - i) > 15) {
#ifdef COMPEDEBUG
    StartupStore(_T("latitude p-i exceed 15%s"), NEWLINE);
#endif
    return false;
  }
  LK_tcsncpy(tLatitude, &String[i], p - i);

  i = p + _tcslen(szDeg); // skip delimiter
  // i points to NS

  bool north = false;
  TCHAR tNS = String[i];
  TCHAR NS[] = _T("NS");
  if ((tNS != NS[0]) && (tNS != NS[1])) {
#ifdef COMPEDEBUG
    StartupStore(_T("Wrong NS latitude! %s"), NEWLINE);
#endif
    return false;
  }
  if (tNS == NS[0])
    north = true;
#ifdef COMPEDEBUG
  StartupStore(_T("WP LATITUDE : <%s> N1S0=%d%s"), tLatitude, north, NEWLINE);
#endif

  // We are now on the space after latitude
  if (String[++i] != _T(' ')) {
#ifdef COMPEDEBUG
    StartupStore(_T("Missing space after latitude %s"), NEWLINE);
#endif
    return false;
  }
  i++;
  // we are now on the first digit of longitude
  // search for szDeg delimiter
  ok = false;
  cp = _tcsstr(&(String[i]), szDeg);
  if (cp) {
    p = cp - String;
    ok = true;
  }
  if (!ok) {
#ifdef COMPEDEBUG
    StartupStore(_T("Missing delimiter in longitude %s"), NEWLINE);
#endif
    return false;
  }
  // p points to delimiter

  TCHAR tLongitude[16];
  if ((p - i) > 15) {
#ifdef COMPEDEBUG
    StartupStore(_T("longitude p-i exceed 15%s"), NEWLINE);
#endif
    return false;
  }
  LK_tcsncpy(tLongitude, &String[i], p - i);

  i = p + _tcslen(szDeg); // skip delimiter

  // i points to EW
  bool east = false;
  TCHAR tEW = String[i];
  TCHAR EW[] = _T("EW");
  if ((tEW != EW[0]) && (tEW != EW[1])) {
#ifdef COMPEDEBUG
    StartupStore(_T("Wrong EW longitude! %s"), NEWLINE);
#endif
    return false;
  }
  if (tEW == EW[0])
    east = true;
#ifdef COMPEDEBUG
  StartupStore(_T("WP LONGITUDE : <%s> E1W0=%d%s"), tLongitude, east, NEWLINE);
#endif

  // We are now on the space after longitude
  if (String[++i] != _T(' ')) {
#ifdef COMPEDEBUG
    StartupStore(_T("Missing space after longitude %s"), NEWLINE);
#endif
    return false;
  }
  i++; // point to beginning of date of recording

  // we are now on the first digit of DATE
  // search for space delimiter
  for (p = i, ok = false; p < slen; p++) {
    if (String[p] == _T(' ')) {
      ok = true;
      break;
    }
  }
  if (!ok) {
#ifdef COMPEDEBUG
    StartupStore(_T("Missing space after DATE%s"), NEWLINE);
#endif
    return false;
  }
  // p points to space after DATE
  // i points to the presumed first character of TIME
  i = p + 1;
  if (i >= slen || String[i] == _T(' ')) {
#ifdef COMPEDEBUG
    StartupStore(_T("No TIME found%s"), NEWLINE);
#endif
    return false;
  }
  // we are now on the first digit of DATE
  // search for space delimiter
  for (p = i, ok = false; p < slen; p++) {
    if (String[p] == _T(' ')) {
      ok = true;
      break;
    }
  }
  if (!ok) {
#ifdef COMPEDEBUG
    StartupStore(_T("Missing space after TIME%s"), NEWLINE);
#endif
    return false;
  }
  // p points to space after TIME
  // i points to the presumed first character of ALTITUDE
  i = p + 1;
  if (i >= slen || String[i] == _T(' ')) {
#ifdef COMPEDEBUG
    StartupStore(_T("No ALTITUDE found%s"), NEWLINE);
#endif
    return false;
  }

  // i now points to first digit of altitude, minim 8 chars
  // this check can be avoided
  if (slen < (i + 8)) {
#ifdef COMPEDEBUG
    StartupStore(_T("Line overflow before altitude%s"), NEWLINE);
#endif
    return false;
  }

  // we are now on the first digit of altitude
  // search for space delimiter
  for (p = i, ok = false; p < slen; p++) {
    if (String[p] == _T(' ')) {
      ok = true;
      break;
    }
  }
  if (!ok) {
#ifdef COMPEDEBUG
    StartupStore(_T("Missing space after altitude%s"), NEWLINE);
#endif
    return false;
  }
  // p points to space after altitude

  TCHAR tAltitude[16];
  if ((p - i) > 15) {
#ifdef COMPEDEBUG
    StartupStore(_T("altitude p-i exceed 15%s"), NEWLINE);
#endif
    return false;
  }
  LK_tcsncpy(tAltitude, &String[i - 1], p - i);

#ifdef COMPEDEBUG
  StartupStore(_T("WP ALTITUDE : <%s>%s"), tAltitude, NEWLINE);
#endif

  i = p + 1;
  while (String[p] != _T('\0')) {
    ++p;
  }
  // we are now on first char of comment
  if ((p - i) > ((COMMENT_SIZE * 2) - 1)) {
#ifdef COMPEDEBUG
    StartupStore(_T("Comment too long%s"), NEWLINE);
#endif
    return false;
  }
  LK_tcsncpy(tComment, &String[i], p - i);

#ifdef COMPEDEBUG
  StartupStore(_T("WP COMMENT : <%s>%s"), tComment, NEWLINE);
#endif

  if (_tcslen(tName) > NAME_SIZE)
    tName[NAME_SIZE - 1] = _T('\0');
  _tcscpy(Temp->Name, tName);

  Temp->Latitude = _tcstod(tLatitude, NULL);
  if (!north)
    Temp->Latitude *= -1; // 100218
  if ((Temp->Latitude > 90) || (Temp->Latitude < -90)) {
    return false;
  }
  Temp->Longitude = _tcstod(tLongitude, NULL);
  if (!east)
    Temp->Longitude *= -1;
  if ((Temp->Longitude > 180) || (Temp->Longitude < -180)) {
    return false;
  }
  Temp->Altitude = ReadAltitude(tAltitude);
  if (Temp->Altitude == -9999)
    return false;

  Temp->Flags = TURNPOINT;
  SetWaypointComment(*Temp, tComment);
  return true;
}
