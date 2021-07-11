/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   igc_file_writer.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 16 january 2018
 */

#include "igc_file_writer.h"
#include <cstdio>
#include <cassert>

namespace {
  // return c if valid char for IGC files
  // return space if not.
  char clean_igc_char(char c) {
    if (c >= 0x20 && c <= 0x7E && c != 0x24 &&
        c != 0x2A && c != 0x2C && c != 0x21 &&
        c != 0x5C && c != 0x5E && c != 0x7E) {
      return c;
    }
    return ' ';
  }

  void write_g_record(FILE *stream, const MD5_Base &md5) {
    // we made copy to allow to continue to update hash after Final call.
    MD5 md5_tmp(md5);
    md5_tmp.Final();
    fwrite("G", 1, 1, stream);
    fwrite(md5_tmp.digestChars, 1, 16, stream);
    fwrite("\r\nG", 1, 3, stream);
    fwrite(md5_tmp.digestChars + 16, 1, 16, stream);
    fwrite("\r\n", 1, 2, stream);
  }
} // namespace

igc_file_writer::igc_file_writer(const TCHAR *file, bool grecord)
    : file_path(file), add_grecord(grecord) {

}


bool igc_file_writer::append(const char *data, size_t size) {

  FILE *stream = _tfopen(file_path.c_str(), _T("rb+"));
  if (!stream) {
    stream = _tfopen(file_path.c_str(), _T("wb"));
  }
  assert(stream); // invalid file path or missing right on target directory ?
  if (stream) {
    fseek(stream, next_record_position, SEEK_SET);

    for (; *(data) && size > 1; ++data, --size) {
      if ((*data) != 0x0D && (*data) != 0x0A) {
        char c = clean_igc_char(*data);

        if (add_grecord) {
          md5_a.Update(reinterpret_cast<unsigned char *>(&c), 1);
          md5_b.Update(reinterpret_cast<unsigned char *>(&c), 1);
          md5_c.Update(reinterpret_cast<unsigned char *>(&c), 1);
          md5_d.Update(reinterpret_cast<unsigned char *>(&c), 1);
        }
        fwrite(&c, 1, 1, stream);
      } else {
        fwrite(data, 1, 1, stream);
      }
    }

    next_record_position = ftell(stream);

    if (add_grecord) {
      write_g_record(stream, md5_a);
      write_g_record(stream, md5_b);
      write_g_record(stream, md5_c);
      write_g_record(stream, md5_d);
    }
    fclose(stream);
    return true;
  }
  return false;
}
