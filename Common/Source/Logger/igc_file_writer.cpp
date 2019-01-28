/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   igc_file_writer.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 16 january 2018
 */

#include "igc_file_writer.h"

namespace {
  // returns 1 if valid char for IGC files
  char clean_igc_char(char c) {
    if ((c >= 0x20 && c <= 0x7E && c != 0x0D && c != 0x0A && c != 0x24 &&
        c != 0x2A && c != 0x2C && c != 0x21 && c != 0x5C && c != 0x5E &&
        c != 0x7E)) {
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
    : file_path(file), next_record_position(), add_grecord(grecord),
      md5_a(0x63e54c01, 0x25adab89, 0x44baecfe, 0x60f25476),
      md5_b(0x41e24d03, 0x23b8ebea, 0x4a4bfc9e, 0x640ed89a),
      md5_c(0x61e54e01, 0x22cdab89, 0x48b20cfe, 0x62125476),
      md5_d(0xc1e84fe8, 0x21d1c28a, 0x438e1a12, 0x6c250aee) {}


bool igc_file_writer::append(const char *data, size_t size) {

  FILE *stream = _tfopen(file_path, _T("rb+"));
  if (!stream) {
    stream = _tfopen(file_path, _T("wb"));
  }
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
