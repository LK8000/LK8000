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

#ifdef ANDROID
#include <android/log.h>
#endif

void StartupLogFreeRamAndStorage() {
  size_t freeram = CheckFreeRam()/1024;
  TCHAR buffer[MAX_PATH];
  LocalPath(buffer);
  size_t freestorage = FindFreeSpace(buffer);
  StartupStore(TEXT(". Free ram=%u K  storage=%u K") NEWLINE, (unsigned int)freeram,(unsigned int)freestorage);
}

/**
 * it's for debug build only.
 *  - file are cleared by first use.
 */
void DebugStore(const char *Str, ...)
{
#ifndef NDEBUG

  FILE *stream = nullptr;

  static Mutex mutex;
  ScopeLock Lock(mutex);

  static TCHAR szFileName[MAX_PATH];
  static bool initialised = false;
  if (!initialised) {
    LocalPath(szFileName, TEXT(LKF_DEBUG));
    stream = _tfopen(szFileName,TEXT("w"));
    initialised = true;
  } else {
    stream = _tfopen(szFileName,TEXT("a+"));
  }

  if(stream) {
      va_list ap;
      va_start(ap, Str);
      vfprintf(stream, Str, ap);
      va_end(ap);
      fclose(stream);
  }
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


void StartupStore(const TCHAR *Str, ...)
{
  static Mutex mutex;
  ScopeLock Lock(mutex);

  TCHAR buf[1024]; // 2 kByte for unicode, 1kByte for utf-8

  va_list ap;
  va_start(ap, Str);
  _vsntprintf(buf, array_size(buf), Str, ap);
  va_end(ap);

  buf[array_size(buf) -1] = _T('\0'); // grant string is null terminated.

  /* remove trailing space
   *   ' '	(0x20)	space (SPC)
   *   '\t'	(0x09)	horizontal tab (TAB)
   *   '\n'	(0x0a)	newline (LF)
   *   '\v'	(0x0b)	vertical tab (VT)
   *   '\f'	(0x0c)	feed (FF)
   *   '\r'	(0x0d)	carriage return (CR)
   */
  for(size_t i = _tcslen(buf)-1; _istspace(buf[i]); --i) {
      buf[i] = '\0';
  }  
  
  
#ifdef ANDROID
  __android_log_print(ANDROID_LOG_INFO, "LK8000","%s\n", buf);
#elif defined(__linux__) && !defined(NDEBUG)
  printf("%s\n", buf);
#endif
  

  FILE *startupStoreFile = NULL;
  static TCHAR szFileName[MAX_PATH];
  static bool initialised = false;
  if (!initialised) {
	LocalPath(szFileName, TEXT(LKF_RUNLOG));
	initialised = true;
  }

  startupStoreFile = _tfopen(szFileName, TEXT("ab+"));
  if (startupStoreFile != NULL) {
#ifdef UNICODE
    /* each codepoints can be encoded in one to four bytes.
    * worst case : ( <number of codepoint> x 4 ) + <size of '\0'>
    */
    const size_t buff_size = (_tcslen(buf) * 4) + 1;
    char sbuf[buff_size]; 
    size_t i = TCHAR2utf(buf, sbuf, buff_size);
#else
    char* sbuf = buf; // string are already utf-8 encoded, no need to convert.
    size_t i = strlen(sbuf);
#endif

    if (i > 0) {
      fprintf(startupStoreFile, "[%09u] %s" SNEWLINE, MonotonicClockMS(), sbuf);
    }
    fclose(startupStoreFile);
  }
}

tstring toHexString(const void* data, size_t size) {
  tstring szHex;
  szHex.reserve(size * 3); // 3 char for each byte
  constexpr TCHAR hex_chars[16] = {
          _T('0'), _T('1'), _T('2'), _T('3'), _T('4'), _T('5'), _T('6'), _T('7'),
          _T('8'), _T('9'), _T('A'), _T('B'), _T('C'), _T('D'), _T('E'), _T('F')
  };

  const uint8_t* p = static_cast<const uint8_t*>(data);
  const uint8_t* pend = p + size;
  for(;p < pend; ++p) {
    szHex += (hex_chars[((*p) & 0xF0) >> 4]);
    szHex += (hex_chars[((*p) & 0x0F) >> 0]);
    szHex += ' ';
  }
  return szHex;
}