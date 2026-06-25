/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include <stdio.h>      /* printf */
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include "utils/stringext.h"
#include "utils/unique_file_ptr.h"
#include "OS/Memory.h"
#include "LocalPath.h"

#ifdef ANDROID
#include <android/log.h>
#endif


void DebugStore(const char* fmt, ...) {
#ifndef NDEBUG

  static Mutex mutex;
  const std::lock_guard lock(mutex);

  static TCHAR szFileName[MAX_PATH];
  static bool initialised = false;
  if (!initialised) {
    LocalPath(szFileName, TEXT(LKF_DEBUG));
    initialised = true;
  }
  auto stream = make_unique_file_ptr(szFileName,TEXT("w"));

  if(stream) {
      va_list args;
      va_start(args, fmt);
      stream.vfprintf(fmt, args);
      va_end(args);
  }
#endif
}


namespace {

constexpr size_t MAX_LOG_SIZE = 1024*1024; // 1MB

#ifdef UNICODE
// Trim trailing space
void TrimRight(char* str) {
    char * end = str + strlen(str);
    while(end > str && isspace(*(end-1))) end--;
    // Write new null terminator
    *(end) = 0;
}
#endif

void StartupStoreWrite(const char* sbuf) {
  static Mutex mutex;
  const std::lock_guard lock(mutex);

  static TCHAR szFileName[MAX_PATH];
  static bool initialised = false;

  if (!initialised) {
    LocalPath(szFileName, _T(LKF_RUNLOG));

    size_t filesize = lk::filesystem::getFileSize(szFileName);
    if (filesize > MAX_LOG_SIZE) {
      TCHAR szFileNameOld[MAX_PATH];
      LocalPath(szFileNameOld, _T(LKF_RUNLOG) _T(".old"));
      lk::filesystem::deleteFile(szFileNameOld);
      lk::filesystem::moveFile(szFileName, szFileNameOld);
    }

    initialised = true;
  }

  auto startupStoreFile = make_unique_file_ptr(szFileName, _T("ab+"));
  if (startupStoreFile) {
    size_t i = strlen(sbuf);

    if (i > 0) {
      startupStoreFile.fprintf("[%09u] %s\r\n", MonotonicClockMS(), sbuf);
    }
  }
}

}  // namespace


void StartupStoreV(const char* fmt, va_list args)
{
  va_list args_copy;
  va_copy(args_copy, args);

  int size = vsnprintf(nullptr, 0, fmt, args_copy);
  va_end(args_copy);

  if (size < 0) {
    return;
  }

  auto buffer = std::make_unique<char[]>(size + 1);
  char* buf = buffer.get();

  vsnprintf(buf, size + 1, fmt, args);
  buf[size] = '\0';

  TrimRight(buf);

#ifdef ANDROID
  __android_log_print(ANDROID_LOG_INFO, "LK8000", "%s\n", buf);
#elif defined(__linux__) && !defined(NDEBUG)
  printf("%s\n", buf);
#endif

  StartupStoreWrite(buf);
}

#ifdef UNICODE

void StartupStoreV(const wchar_t* fmt, va_list args)
{
  va_list args_copy;
  va_copy(args_copy, args);

  int size = vswprintf(nullptr, 0, fmt, args_copy);
  va_end(args_copy);

  if (size < 0) {
    return;
  }

  auto buffer = std::make_unique<wchar_t[]>(size + 1);
  wchar_t* buf = buffer.get();

  vswprintf(buf, size + 1, fmt, args);
  buf[size] = L'\0';

  TrimRight(buf);

  std::string utf8_str = to_utf8(buf);

  StartupStoreWrite(utf8_str.c_str());
}

#endif

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
  }
  return szHex;
}

namespace {

bool has_non_printable(const char* bytes, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    unsigned char c = static_cast<unsigned char>(bytes[i]);
    if (!std::isprint(c) && !std::isspace(c)) {
      return true;
    }
  }
  return false;
}

std::string escape_special_chars(const char* bytes, size_t size) {
  std::string result;
  result.reserve(size);  // Reserve at least the input size

  for (size_t i = 0; i < size; ++i) {
    switch (bytes[i]) {
      case '\n':
        result += R"(\n)";
        break;
      case '\r':
        result += R"(\r)";
        break;
      case '\t':
        result += R"(\t)";
        break;
      case '\v':
        result += R"(\v)";
        break;
      case '\f':
        result += R"(\f)";
        break;
      default:
        result += bytes[i];
        break;
    }
  }

  return result;
}

}// namespace

tstring data_string(const void* data, size_t size) {
  const char* bytes = static_cast<const char*>(data);

  if (has_non_printable(bytes, size)) {
    return toHexString(bytes, size);
  }
  return to_tstring(escape_special_chars(bytes, size));
}

void StartupLogFreeRamAndStorage() {
    size_t freeram = CheckFreeRam()/1024;
    size_t freestorage = FindFreeSpace(LKGetLocalPath());
    StartupStore(TEXT(". Free ram=%u K  storage=%u K"), (unsigned int)freeram,(unsigned int)freestorage);
}
