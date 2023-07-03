/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   wait_ack.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 Avril 2023
 */

#ifndef _Comm_wait_ack_h_
#define _Comm_wait_ack_h_

#include <memory>
#include <utility>
#include "Thread/Cond.hpp"
#include "Thread/Mutex.hpp"
#include "tchar.h"
#include "Util/tstring.hpp"

class wait_ack final {
 public:
  /**
   * @str must exist until this is destroyed
   */
  explicit wait_ack(const char* str);

  /**
   * return true and set `ready` state if @str is same as string provided to ctor
   * rmq : even if param type is TCHAR, only work for ascci 7bit character on Win32
   */
  bool check(const TCHAR* str);

  /**
   * wait for and reset `ready` state
   */
  bool wait(unsigned timeout_ms);

 private:
  /**
   * compare string ignoring trailing <CR><LF>
   */
  bool compare_nmea(const TCHAR* str);

  std::string_view wait_str;

  Mutex mutex;
  Cond condition;
  bool ready = false;
};

using wait_ack_weak_ptr = std::weak_ptr<wait_ack>;

using wait_ack_shared_ptr = std::shared_ptr<wait_ack>;

inline wait_ack_shared_ptr make_wait_ack_shared(const char* str) {
  return std::make_shared<wait_ack>(str);
}

#endif  // _Comm_wait_ack_h_
