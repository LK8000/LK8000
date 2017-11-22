/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   zzip_file.h
 * Author: Bruno de Lacheisserie
 * 
 * Created on 21 novembre 2017, 20:31
 */

#ifndef ZZIP_FILE_H
#define ZZIP_FILE_H

#include "openzip.h"
#include <string>
#include "Util/AllocatedArray.hpp"

class zzip_stream : public zzip_file_ptr {
public:
  zzip_stream() : _begin(nullptr), _end(nullptr), _cs(charset::detect) { }
  
  zzip_stream(const TCHAR* szFile, const char *mode) 
          : zzip_file_ptr(openzip(szFile, mode)),
            _begin(nullptr), 
            _end(nullptr), 
            _cs(charset::detect) {
  }

  zzip_stream(const zzip_stream&) = delete;
  zzip_stream(zzip_stream&&) = delete;

  bool open(const TCHAR* szFile, const char *mode) {
    _fp = openzip(szFile, mode);
    _end = _begin = nullptr;
    _cs = charset::detect;
    return (*this);
  }

  bool read_line(char* string, size_t size);

  template<size_t size>
  bool read_line(char (&string)[size]) {
    return read_line(string, size);
  }

#ifdef UNICODE
  bool read_line(wchar_t* string, size_t size);

  template<size_t size>
  bool read_line(wchar_t(&string)[size]) {
    return read_line(string, size);
  }

private:
  AllocatedArray<char> raw_string; // temporary buffer for store data before convert to wchar_t

#endif


private:

  bool read_line_raw(char* string, size_t size);

  enum charset {
      detect,  // initial value : used for detect utf8 BOM
      unknown, // used for detect charset using content ( only if BOM not found on first line )
      utf8,    // utf8 detected : -> convert 
      latin1   // latin1 (invalid utf8 code point detected) -> convert to utf8
  };  

  char _buffer[64];
  const char* _begin; // first "unread" char
  const char* _end; // end of buffer
  charset _cs; 

  std::string utf8String; // temporary member used for convert Latin1 to utf8

};
#endif /* ZZIP_FILE_H */

