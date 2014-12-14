/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "utils/stringext.h"



bool	EnableLogNMEA = false;



// New LogNMEA
void LogNMEA(TCHAR* text, int PortNum) {

static FILE *logfp = NULL;
static FILE *logfp0= NULL;
static FILE *logfp1= NULL;
static int iLastPort =-1;
static bool wasWriting=false;


  if (!EnableLogNMEA) {
      if (wasWriting) { 
          if(logfp != NULL) {
              fclose(logfp) ;
              logfp = NULL;
          }
          if(logfp0 != NULL) {
              fclose(logfp0);
              logfp0 = NULL;
          }
          if(logfp1 != NULL) {
              fclose(logfp1);
              logfp1 = NULL;
          }
          iLastPort =-1;
          wasWriting=false;
      }
      return;
  }

  if(logfp == NULL)
  {
        TCHAR fpname[LKSIZEBUFFERPATH];
        TCHAR buffer[LKSIZEBUFFERPATH];
	LocalPath(buffer,TEXT(LKD_LOGS));
	_stprintf(fpname, _T("%s%sNMEA_%04d-%02d-%02d-%02d-%02d-%02d.txt"), buffer, _T(DIRSEP), GPS_INFO.Year, GPS_INFO.Month, GPS_INFO.Day,
	GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
	logfp = _tfopen(fpname, _T("a"));
	if (logfp == NULL) {
		DoStatusMessage(_T("CANNOT SAVE TO NMEA LOGFILE"));
		EnableLogNMEA=false;
		return;
	}
        wasWriting=true;
  }

  if(iLastPort != -1)  /* already a port info ? */
  {
    if( iLastPort != PortNum) /* more than one port active (another than the previous) */
	{
	  if(logfp0 == NULL)
	  {
                TCHAR fpname[LKSIZEBUFFERPATH];
                TCHAR buffer[LKSIZEBUFFERPATH];
		LocalPath(buffer,TEXT(LKD_LOGS));
	    _stprintf(fpname, _T("%s%sNMEA_A_%04d-%02d-%02d-%02d-%02d-%02d.txt"), buffer, _T(DIRSEP), GPS_INFO.Year, GPS_INFO.Month, GPS_INFO.Day, GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
		logfp0 = _tfopen(fpname, _T("a"));
		if (logfp0 == NULL) {
		  DoStatusMessage(_T("CANNOT SAVE TO NMEA LOGFILE PORT A:"));
		  return; }
                wasWriting=true;
	  }

	  if(logfp1 == NULL)
	  {
                TCHAR fpname[LKSIZEBUFFERPATH];
                TCHAR buffer[LKSIZEBUFFERPATH];
        LocalPath(buffer,TEXT(LKD_LOGS));
		_stprintf(fpname, _T("%s%sNMEA_B_%04d-%02d-%02d-%02d-%02d-%02d.txt"), buffer, _T(DIRSEP), GPS_INFO.Year, GPS_INFO.Month, GPS_INFO.Day, GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
		logfp1 = _tfopen(fpname, _T("a"));
		if (logfp1 == NULL) {
		  DoStatusMessage(_T("CANNOT SAVE TO NMEA LOGFILE PORT B:"));
		  return; }
                wasWriting=true;
	  }
	}
  }
  iLastPort = PortNum;


  char snmea[LKSIZENMEA];
  LKASSERT(_tcslen(text)<sizeof(snmea));
  if (_tcslen(text)>=sizeof(snmea)) return;

  TCHAR2usascii(text, snmea, LKSIZENMEA);

  short l=strlen(snmea);
  if (l<6) return;
  if ( snmea[l-3]==0x0d && snmea[l-2]==0x0d) {
	snmea[l-2]=0x0a;
	snmea[l-1]=0;
  }
  l=strlen(snmea); // surely >3
  if ( snmea[l-1]==0x0a && snmea[l-2]==0x0a) {
	snmea[l-1]=0;
  }

  if( logfp != NULL)
    fprintf(logfp,"%s",snmea);

   if(PortNum ==0)
   {
     if(logfp0 != NULL)
       fprintf(logfp0,"%s",snmea);
   }
   else
   {
     if(PortNum ==1)
       if(logfp1 != NULL)
         fprintf(logfp1,"%s",snmea);
   }
}

