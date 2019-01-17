/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   igc_file_writer.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 16 january 2018
 */

#ifndef _LOGGER_IGC_FILE_WRITER_H_
#define _LOGGER_IGC_FILE_WRITER_H_

#include "md5.h"
#include "tchar.h"

class igc_file_writer final {

  igc_file_writer() = delete;
  igc_file_writer(const igc_file_writer &) = delete;
  igc_file_writer(igc_file_writer &&) = delete;

  igc_file_writer& operator=(const igc_file_writer &) = delete;
  igc_file_writer& operator=(igc_file_writer &&) = delete;

public:

  explicit igc_file_writer(const TCHAR *file, bool grecord);

  template <size_t size> 
  bool append(const char (&data)[size]) {
    static_assert(size > 0, "invalid size");
    return append(data, size);
  }

private:
  bool append(const char *data, size_t size);

  const TCHAR *file_path; /** full path of target igc file */

  int next_record_position; /** position of G record */
  const bool add_grecord; /** true if G record must be added to file */

  MD5_Base md5_a;
  MD5_Base md5_b;
  MD5_Base md5_c;
  MD5_Base md5_d;
};

#endif //_LOGGER_IGC_FILE_WRITER_H_
