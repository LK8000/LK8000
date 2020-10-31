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

void StartupStore(const TCHAR *Str, ...)
{
  static Mutex mutex;
  ScopeLock Lock(mutex);

  TCHAR buf[1024]; // 2 kByte for unicode, 1kByte for utf-8

  va_list ap;
  va_start(ap, Str);
  _vsntprintf(buf, std::size(buf), Str, ap);
  va_end(ap);

  buf[std::size(buf) -1] = _T('\0'); // grant string is null terminated.

  TrimRight(buf);

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

void StartupLogFreeRamAndStorage() {
    size_t freeram = CheckFreeRam()/1024;
    size_t freestorage = FindFreeSpace(LKGetLocalPath());
    StartupStore(TEXT(". Free ram=%u K  storage=%u K"), (unsigned int)freeram,(unsigned int)freestorage);
}
