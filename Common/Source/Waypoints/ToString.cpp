/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"

  // Number,Latitude,Longitude,Altitude,Flags,Name,Comment
  // Number starts at 1
  // Lat/long expressed as D:M:S[N/S/E/W]
  // Altitude as XXXM
  // Flags: T,H,A,L


void WaypointFlagsToString(int FlagsNum,
                           TCHAR *Flags) {

  if ((FlagsNum & AIRPORT) == AIRPORT) {
    _tcscat(Flags,TEXT("A"));
  }
  if ((FlagsNum & TURNPOINT) == TURNPOINT) {
    _tcscat(Flags,TEXT("T"));
  }
  if ((FlagsNum & LANDPOINT) == LANDPOINT) {
    _tcscat(Flags,TEXT("L"));
  }
  if ((FlagsNum & HOME) == HOME) {
    _tcscat(Flags,TEXT("H"));
  }
  if ((FlagsNum & START) == START) {
    _tcscat(Flags,TEXT("S"));
  }
  if ((FlagsNum & FINISH) == FINISH) {
    _tcscat(Flags,TEXT("F"));
  }
  if ((FlagsNum & RESTRICTED) == RESTRICTED) {
    _tcscat(Flags,TEXT("R"));
  }
  if ((FlagsNum & WAYPOINTFLAG) == WAYPOINTFLAG) {
    _tcscat(Flags,TEXT("W"));
  }
  if (_tcslen(Flags)==0) {
    _tcscat(Flags,TEXT("T"));
  }
}

void LongitudeToCUPString(double Longitude,
                               TCHAR *Buffer) {
  TCHAR EW[] = TEXT("WE");
  double dd, mm, ss;
  double lon;
  int idd,imm,iss;

  int sign = Longitude<0 ? 0 : 1;
  lon=fabs(Longitude);

  dd = (int)lon;
  mm = (lon-dd) * 60.0;
  ss = (mm-(int)mm)*1000;

  idd = (int)dd;
  imm = (int)mm;
  iss = (int)(ss+0.5); // QUI
  if (iss>=1000) {
	imm++;
	iss-=1000;
  }

  _stprintf(Buffer, TEXT("%03d%02d.%03d%c"), idd,imm,iss, EW[sign]);
}

void LatitudeToCUPString(double Latitude,
                              TCHAR *Buffer) {
  TCHAR EW[] = TEXT("SN");
  double dd, mm, ss;
  double lat;

  int sign = Latitude<0 ? 0 : 1;
  lat = fabs(Latitude);

  dd = (int)lat;
  mm = (lat-dd) * 60.0;
  ss = (mm-(int)mm)*1000;

  mm = (int)mm;
  ss = (int)(ss+0.5);
  if (ss>=1000) {
	mm++;
	ss-=1000;
  }

  _stprintf(Buffer, TEXT("%02.0f%02.0f.%03.0f%c"), dd, mm, ss, EW[sign]);
}

void WaypointLongitudeToString(double Longitude,
                               TCHAR *Buffer) {
  TCHAR EW[] = TEXT("WE");
  int dd, mm, ss;

  int sign = Longitude<0 ? 0 : 1;
  Longitude = fabs(Longitude);

  dd = (int)Longitude;
  Longitude = (Longitude - dd) * 60.0;
  mm = (int)(Longitude);
  Longitude = (Longitude - mm) * 60.0;
  ss = (int)(Longitude + 0.5);
  if (ss >= 60)
    {
      mm++;
      ss -= 60;
    }
  if (mm >= 60)
    {
      dd++;
      mm -= 60;
    }
  _stprintf(Buffer, TEXT("%03d:%02d:%02d%c"), dd, mm, ss, EW[sign]);
}


void WaypointLatitudeToString(double Latitude,
                              TCHAR *Buffer) {
  TCHAR EW[] = TEXT("SN");
  int dd, mm, ss;

  int sign = Latitude<0 ? 0 : 1;
  Latitude = fabs(Latitude);

  dd = (int)Latitude;
  Latitude = (Latitude - dd) * 60.0;
  mm = (int)(Latitude);
  Latitude = (Latitude - mm) * 60.0;
  ss = (int)(Latitude + 0.5);
  if (ss >= 60) {
    mm++;
    ss -= 60;
  }
  if (mm >= 60) {
    dd++;
    mm -= 60;
  }
  _stprintf(Buffer, TEXT("%02d:%02d:%02d%c"), dd, mm, ss, EW[sign]);
}
