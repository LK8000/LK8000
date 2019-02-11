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

class zzip_stream {
public:
  zzip_stream() : _current_char(nullptr), _end(nullptr), _cs(charset::detect) { }

  zzip_stream(const TCHAR* szFile, const char *mode);

  zzip_stream(const zzip_stream&) = delete;
  zzip_stream(zzip_stream&&) = delete;

  bool open(const TCHAR* szFile, const char *mode);

  void close() { 
    _fp.close();
  }

  operator bool() const {
    return (_fp); 
  }

  template<typename char_type, size_t size>
  bool read_line(char_type (&string)[size]) {
    return read_line(string, size);
  }

  bool read_line(char* string, size_t size);

#ifdef UNICODE
  bool read_line(wchar_t* string, size_t size);

private:
  AllocatedArray<char> raw_string; // temporary buffer for store data before convert to wchar_t

#endif

private:
  char read_char();
  void read_buffer();

  bool read_line_raw(char* string, size_t size);

  enum charset {
      detect,  // initial value : used for detect utf8 BOM
      unknown, // used for detect charset using content ( only if BOM not found on first line )
      utf8,    // utf8 detected : -> convert 
      latin1   // latin1 (invalid utf8 code point detected) -> convert to utf8
  };  

  zzip_file_ptr _fp;

  char _buffer[1024];
  const char* _current_char; // first "unread" char
  const char* _end;          // end of buffer
  bool _end_of_file;         // true if end of file is reached.

  charset _cs; // file charset

  std::string utf8String; // temporary member used for convert Latin1 to utf8
};
#endif /* ZZIP_FILE_H */
