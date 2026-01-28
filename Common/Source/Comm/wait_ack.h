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
#include <string_view>
#include "Thread/Cond.hpp"
#include "Thread/Mutex.hpp"

class wait_ack final {
 public:
  /**
   * @str must exist until this is destroyed
   * @mutex must exist until this is destroyed
   */
  wait_ack(Mutex& mtx, const char* str);

  /**
   * return true and set `ready` state if @str is same as string provided to ctor
   * rmq : even if param type is TCHAR, only work for ascci 7bit character on Win32
   */
  bool check(const char* str);

  /**
   * wait for and reset `ready` state
   * @mutex must be locked before calling this method
   */
  bool wait(unsigned timeout_ms);

 private:
  /**
   * compare string ignoring trailing <CR><LF>
   */
  bool compare_nmea(const char* str);

  Mutex& mutex;

  std::string_view wait_str;

  Cond condition;
  bool ready = false;
};

using wait_ack_weak_ptr = std::weak_ptr<wait_ack>;

using wait_ack_shared_ptr = std::shared_ptr<wait_ack>;

inline wait_ack_shared_ptr make_wait_ack_shared(Mutex& mtx, const char* str) {
  return std::make_shared<wait_ack>(mtx, str);
}

#endif  // _Comm_wait_ack_h_
