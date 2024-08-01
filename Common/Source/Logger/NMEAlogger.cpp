/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "utils/printf.h"


bool	EnableLogNMEA = false;



// New LogNMEA
void LogNMEA(const char* text, unsigned PortNum) {

static FILE *logfpall = NULL;
static FILE *logfsingle[NUMDEV]= {NULL,NULL,NULL,NULL,NULL,NULL};

static unsigned iLastPort = NUMDEV;
static bool wasWriting=false;


  if (!EnableLogNMEA) {
      if (wasWriting) { 
          if(logfpall != NULL) {
              fclose(logfpall) ;
              logfpall = NULL;
          }
          for(int dev = 0; dev < NUMDEV; dev++)
          {
            if(logfsingle[dev] != NULL) {
              fclose(logfsingle[dev]);
              logfsingle[dev] = NULL;
            }
          }

          iLastPort = NUMDEV;
          wasWriting=false;
      }
      return;
  }

  if(logfpall == NULL)
  {
        TCHAR fpname[LKSIZEBUFFERPATH];
        TCHAR buffer[LKSIZEBUFFERPATH];
        LocalPath(buffer,TEXT(LKD_LOGS));
        lk::snprintf(fpname, _T("%s%sNMEA_%04d-%02d-%02d-%02d-%02d-%02d.txt"), buffer, _T(DIRSEP), GPS_INFO.Year, GPS_INFO.Month, GPS_INFO.Day,
        GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
        logfpall = _tfopen(fpname, _T("a"));
        if (logfpall == NULL) {
          DoStatusMessage(_T("CANNOT SAVE TO NMEA LOGFILE"));
          EnableLogNMEA=false;
          return;
        }
        wasWriting=true;
  }

  if(iLastPort != NUMDEV)  /* already a port info ? */
  {
    if( iLastPort != PortNum) /* more than one port active (another than the previous) */
    {
      if(PortNum < NUMDEV)
      {
        if(logfsingle[PortNum] == NULL)
        {
          TCHAR fpname[LKSIZEBUFFERPATH];
          TCHAR buffer[LKSIZEBUFFERPATH];
          LocalPath(buffer,TEXT(LKD_LOGS));
          lk::snprintf(fpname, _T("%s%sNMEA_%c_%04d-%02d-%02d-%02d-%02d-%02d.txt"), buffer, _T(DIRSEP),_T('A')+PortNum, GPS_INFO.Year, GPS_INFO.Month, GPS_INFO.Day, GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second);
          logfsingle[PortNum] = _tfopen(fpname, _T("a"));
          if (logfsingle[PortNum] == NULL) {
            DoStatusMessage(_T("CANNOT SAVE TO NMEA LOGFILE PORT A:"));
            return; 
          }
          wasWriting=true;
        }
      }
    }
  }

  iLastPort = PortNum;

  char snmea[LKSIZENMEA];
  LKASSERT(strlen(text)<sizeof(snmea));
  if (strlen(text)>=sizeof(snmea)) return;

  strcpy(snmea,text);

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

  if (logfpall) {
    fprintf(logfpall,"%s",snmea);
    fflush(logfpall);
  }

  if(logfsingle[PortNum]) {
    fprintf(logfsingle[PortNum],"%s",snmea);
    fflush(logfsingle[PortNum]);
  }
}

