/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include <stdio.h>      /* printf */
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include "utils/stringext.h"
#include "OS/Memory.h"

void StartupLogFreeRamAndStorage() {
  size_t freeram = CheckFreeRam()/1024;
  TCHAR buffer[MAX_PATH];
  LocalPath(buffer);
  size_t freestorage = FindFreeSpace(buffer);
  StartupStore(TEXT(". Free ram=%u K  storage=%u K") NEWLINE, (unsigned int)freeram,(unsigned int)freestorage);
}


void DebugStore(const char *Str, ...)
{
#if defined(DEBUG)
  char buf[MAX_PATH];
  va_list ap;
  int len;

  va_start(ap, Str);
  len = vsprintf(buf, Str, ap);
  va_end(ap);

  LockFlightData();
  FILE *stream;
  TCHAR szFileName[] = TEXT(LKF_DEBUG);
  static bool initialised = false;
  if (!initialised) {
    initialised = true;
    stream = _wfopen(szFileName,TEXT("w"));
  } else {
    stream = _wfopen(szFileName,TEXT("a+"));
  }

  fwrite(buf,len,1,stream);

  fclose(stream);
  UnlockFlightData();
#endif
}


#if 0 // 130207 no more unsed in V4
void FailStore(const TCHAR *Str, ...)
{
  TCHAR buf[MAX_PATH];
  va_list ap;

  va_start(ap, Str);
  _vstprintf(buf, Str, ap);
  va_end(ap);

  FILE *stream=NULL;
  static TCHAR szFileName[MAX_PATH];
  static bool initialised = false;

  if (!initialised) {
	LocalPath(szFileName, TEXT(LKF_FAILLOG));
	stream = _tfopen(szFileName, TEXT("ab+"));
	if (stream) {
		fclose(stream);
	}
	initialised = true;
  }
  stream = _tfopen(szFileName,TEXT("ab+"));
  if (stream == NULL) {
	StartupStore(_T("------ FailStore failed, cannot open <%s>%s"), szFileName, NEWLINE);
	return;
  }
  fprintf(stream, "------%s%04d%02d%02d-%02d:%02d:%02d [%09u] FailStore Start, Version %s%s (%s %s) FreeRam=%ld %s",SNEWLINE,
	GPS_INFO.Year,GPS_INFO.Month,GPS_INFO.Day, GPS_INFO.Hour,GPS_INFO.Minute,GPS_INFO.Second,
	(unsigned int)GetTickCount(),LKVERSION, LKRELEASE,
	"",
#if WINDOWSPC >0
	"PC",
#else
	#ifdef PNA
	"PNA",
	#else
	"PDA",
	#endif
#endif

CheckFreeRam(),SNEWLINE);
  fprintf(stream, "Message: %S%s", buf, SNEWLINE);
  fprintf(stream, "GPSINFO: Latitude=%f Longitude=%f Altitude=%f Speed=%f %s",
	GPS_INFO.Latitude, GPS_INFO.Longitude, GPS_INFO.Altitude, GPS_INFO.Speed, SNEWLINE);

  fclose(stream);
  StartupStore(_T("------ %s%s"),buf,NEWLINE);
}
#endif


Poco::FastMutex  CritSec_StartupStore;

void LockStartupStore() {
	CritSec_StartupStore.lock();
}

void UnlockStartupStore() {
	CritSec_StartupStore.unlock();
}

void StartupStore(const TCHAR *Str, ...)
{
  TCHAR buf[(MAX_PATH*2)+1]; // 260 chars normally  FIX 100205
  va_list ap;

  va_start(ap, Str);
  _vsntprintf(buf, array_size(buf), Str, ap);
  va_end(ap);

  LockStartupStore();

  FILE *startupStoreFile = NULL;
  static TCHAR szFileName[MAX_PATH];
  static bool initialised = false;
  if (!initialised) {
	LocalPath(szFileName, TEXT(LKF_RUNLOG));
	initialised = true;
  }

  startupStoreFile = _tfopen(szFileName, TEXT("ab+"));
  if (startupStoreFile != NULL) {
    char sbuf[(MAX_PATH*2)+1]; // FIX 100205

    int i = TCHAR2utf(buf, sbuf, sizeof(sbuf));

    if (i > 0) {
      if (sbuf[i - 1] == 0x0a && (i == 1 || (i > 1 && sbuf[i-2] != 0x0d)))
        sprintf(sbuf + i - 1, SNEWLINE);
      fprintf(startupStoreFile, "[%09u] %s", MonotonicClockMS(), sbuf);
    }
    fclose(startupStoreFile);
  }
  UnlockStartupStore();
}
