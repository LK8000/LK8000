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

enum class wait_ack_result {
  timeout,
  success,
  error,
};

class wait_ack final {
 public:
  /**
   * @success && @error must exist until this is destroyed
   * @mutex must exist until this is destroyed
   */
  wait_ack(Mutex& mtx, const char* success, const char* error);

  /**
   * @return true and set `result` to `success` if @str matches success string,
   *         or set `result` to `error` if @str matches error string;
   *         false otherwise (result unchanged)
   */
  bool check(const char* str);

  /**
   * wait for `Success` or `Error` and reset state
   * @mutex must be locked before calling this method
   */
  wait_ack_result wait(unsigned timeout_ms);

 private:
  /**
   * compare string ignoring trailing <CR><LF>
   */
  static bool compare_nmea(const char* str, std::string_view pattern);

  Mutex& mutex;

  std::string_view success_str;
  std::string_view error_str;

  Cond condition;
  wait_ack_result result = wait_ack_result::timeout;
};

using wait_ack_weak_ptr = std::weak_ptr<wait_ack>;

using wait_ack_shared_ptr = std::shared_ptr<wait_ack>;

inline wait_ack_shared_ptr make_wait_ack_shared(Mutex& mtx,
                                                const char* success_str,
                                                const char* error_str) {
  return std::make_shared<wait_ack>(mtx, success_str, error_str);
}

#endif  // _Comm_wait_ack_h_
