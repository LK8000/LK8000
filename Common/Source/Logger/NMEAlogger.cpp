/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"



bool	EnableLogNMEA = false;



// New LogNMEA
void LogNMEA(TCHAR* text) {

  TCHAR	buffer[LKSIZEBUFFERPATH];
  char snmea[LKSIZENMEA];

  static TCHAR fpname[LKSIZEBUFFERPATH];
  static bool	doinit=true;
  static bool	wasWriting=false;
  static bool	alreadyWarned=false;
  static FILE *logfp;

  if (!EnableLogNMEA) {
	if (wasWriting) {
		fclose(logfp);
		wasWriting=false;
		doinit=true;
	}
	return;
  }

  if (doinit) {
	LocalPath(buffer,TEXT(LKD_LOGS));
	_stprintf(fpname, _T("%s\\NMEA_%04d-%02d-%02d-%02d-%02d-%02d.txt"), buffer, GPS_INFO.Year, GPS_INFO.Month, GPS_INFO.Day,
		GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
	doinit=false;
  }

  if (!wasWriting) {
	logfp = _tfopen(fpname, _T("a"));
	if (logfp == NULL) {
		if (!alreadyWarned) {
			DoStatusMessage(_T("ERR-049 Cannot open NMEA log"));
			alreadyWarned=true;
		}
		return;
	}
	wasWriting=true;
  }

  sprintf(snmea,"%S",text);
  short l=strlen(snmea);
  if (l<6) return;
  if ( snmea[l-3]==0x0d && snmea[l-2]==0x0d) {
	snmea[l-2]=0x0a;
	snmea[l-1]=0;
  }

  fprintf(logfp,"%s",snmea);
  
}

