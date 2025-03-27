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
#include "MessageLog.h"
#include "utils/unique_file_ptr.h"

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

  void write_g_record(const unique_file_ptr& stream, const MD5_Base &md5) {
    // we made copy to allow to continue to update hash after Final call.
    MD5 md5_tmp(md5);
    std::string digest = md5_tmp.Final();
    if (digest.size() >= 32) {
      stream.fwrite("G", 1, 1);
      stream.fwrite(digest.data(), 1, 16);
      stream.fwrite("\r\nG", 1, 3);
      stream.fwrite(digest.data() + 16, 1, 16);
      stream.fwrite("\r\n", 1, 2);
    }
  }
} // namespace

igc_file_writer::igc_file_writer(const TCHAR *file, bool grecord)
    : file_path(file), add_grecord(grecord) {

}


bool igc_file_writer::append(const char *data, size_t size) {

  auto stream = make_unique_file_ptr(file_path.c_str(), _T("rb+"));
  if (!stream) {
    stream = make_unique_file_ptr(file_path.c_str(), _T("wb"));
  }
  if (!stream) {
    // invalid file path or missing right on target directory ?
    static bool error_reported = false;
    if (!error_reported) {
      error_reported = true;
      StartupStore(_T("ERROR: Unable to open IGC file <%s> for writing"), file_path.c_str());
    }
  }
  if (stream) {
    stream.fseek(next_record_position, SEEK_SET);

    for (; *(data) && size > 1; ++data, --size) {
      if ((*data) != 0x0D && (*data) != 0x0A) {
        char c = clean_igc_char(*data);

        if (add_grecord) {
          md5_a.Update(c);
          md5_b.Update(c);
          md5_c.Update(c);
          md5_d.Update(c);
        }
        stream.fwrite(&c, 1, 1);
      } else {
        stream.fwrite(data, 1, 1);
      }
    }

    next_record_position = stream.ftell();

    if (add_grecord) {
      write_g_record(stream, md5_a);
      write_g_record(stream, md5_b);
      write_g_record(stream, md5_c);
      write_g_record(stream, md5_d);
    }
    return true;
  }
  return false;
}
