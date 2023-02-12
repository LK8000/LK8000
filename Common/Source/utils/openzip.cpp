/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   openzip.c
 * Author: Bruno de Lacheisserie
 *
 * Created on December 20, 2014, 3:28 PM
 */
#include <stdlib.h>
#include <zzip/lib.h>
#include <tchar.h>
#include "utils/stringext.h"
#include <unistd.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef O_SYNC
#define O_SYNC 0
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK 0
#endif

extern "C" {

#ifdef WIN32
#ifdef UNDER_CE

#include <string.h> 
#include <windows.h>

int wince_open(const char* path, int oflag, ...) {

  size_t path_len = strlen(path);
  if (path_len >= MAX_PATH)
    return -1;

  DWORD fileaccess = 0U;
  switch (oflag & (O_RDONLY | O_WRONLY | O_RDWR)) {
    case O_RDONLY:
      fileaccess = GENERIC_READ;
      break;
    case O_WRONLY:
      fileaccess = GENERIC_WRITE;
      break;
    case O_RDWR:
      fileaccess = GENERIC_READ | GENERIC_WRITE;
      break;
    default:
      return -1;
  }

  DWORD filecreate = 0;
  switch (oflag & (O_CREAT | O_EXCL | O_TRUNC)) {
    case 0:
    case O_EXCL: /* ignore EXCL w/o CREAT */
      filecreate = OPEN_EXISTING;
      break;
    case O_CREAT:
      filecreate = OPEN_ALWAYS;
      break;
    case O_CREAT | O_EXCL:
    case O_CREAT | O_TRUNC | O_EXCL:
      filecreate = CREATE_NEW;
      break;
    case O_TRUNC:
    case O_TRUNC | O_EXCL: /* ignore EXCL w/o CREAT */
      filecreate = TRUNCATE_EXISTING;
      break;
    case O_CREAT | O_TRUNC:
      filecreate = CREATE_ALWAYS;
      break;
    default:
      /* this can't happen ... all cases are covered */
      return -1;
  }

  TCHAR wpath[MAX_PATH];
  from_utf8(path, wpath, MAX_PATH);

  DWORD fileshare = FILE_SHARE_READ | FILE_SHARE_WRITE;
  DWORD fileattrib = FILE_ATTRIBUTE_NORMAL;

  HANDLE hnd = CreateFileW(wpath, fileaccess, fileshare, NULL, filecreate, fileattrib, NULL);
  if (hnd == INVALID_HANDLE_VALUE)
    return -1;

  if (oflag & O_APPEND)
    SetFilePointer(hnd, 0, NULL, FILE_END);

  return (int)hnd;
}

#else

static int winpc_open(zzip_char_t* filename, int flags, ...) {
  TCHAR wpath[MAX_PATH];
  from_utf8(filename, wpath, MAX_PATH);
  return (_topen(wpath, flags));
}

#endif
#endif

static zzip_strings_t ext[] = {".zip", ".ZIP", "", 0};

static const struct zzip_plugin_io default_io = {
#ifdef WIN32
#ifdef UNDER_CE
    &wince_open,
#else
    &winpc_open,
#endif
#else
    &open,
#endif
    &close,
    &_zzip_read,
    &_zzip_lseek,
    &zzip_filesize,
    1, 1,
    &_zzip_write
};

static zzip_plugin_io_t get_default_io() {
  return (zzip_plugin_io_t)&default_io;
}

ZZIP_FILE* openzip(const char* szFile, const char* mode) {
  int o_flags = 0;
  int o_modes = 0664;
  if (!mode)
    mode = "rb";

  for (; *mode; mode++) {
    switch (*mode) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        continue; /* ignore if not attached to other info */
      case 'r':
        o_flags |= mode[1] == '+' ? O_RDWR : O_RDONLY;
        break;
      case 'w':
        o_flags |= mode[1] == '+' ? O_RDWR : O_WRONLY;
        o_flags |= O_TRUNC;
        break;
      case 'b':
        o_flags |= O_BINARY;
        break;
      case 'f':
        o_flags |= O_NOCTTY;
        break;
      case 'i':
        o_modes |= ZZIP_CASELESS;
        break;
      case '*':
        o_modes |= ZZIP_NOPATHS;
        break;
      case 'x':
        o_flags |= O_EXCL;
        break;
      case 's':
        o_flags |= O_SYNC;
        break;
      case 'n':
        o_flags |= O_NONBLOCK;
        break;
      case 'o':
        o_modes &= ~07;
        o_modes |= ((mode[1] - '0')) & 07;
        continue;
      case 'g':
        o_modes &= ~070;
        o_modes |= ((mode[1] - '0') << 3) & 070;
        continue;
      case 'u':
        o_modes &= ~0700;
        o_modes |= ((mode[1] - '0') << 6) & 0700;
        continue;
      case 'q':
        o_modes |= ZZIP_FACTORY;
        break;
      case 'z':   /* compression level */
        continue; /* currently ignored, just for write mode */
    }
  }

  return zzip_open_ext_io(szFile, o_flags, o_modes, ext, get_default_io());
}

}  // extern "C"
