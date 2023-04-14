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
#include "Thread/Cond.hpp"
#include "Thread/Mutex.hpp"
#include "Util/tstring.hpp"

class wait_ack final {
 public:
  explicit wait_ack(const char* str) : wait_str(str) {}

  bool check(const TCHAR* str) {
    bool signal = WithLock(mutex, [&]() {
      if (compare_nmea(str)) {
        ready = true;
      }
      return ready;
    });

    if (signal) {
      condition.Signal();
    }
    return signal;
  }

  bool wait(unsigned timeout_ms) {
    ScopeLock lock(mutex);
    condition.Wait(mutex, timeout_ms);
    return std::exchange(ready, false);
  }

 private:
  static char valid_char(char c) { return c == '\r' || c == '\n' ? '\0' : c; }

  // compare string ingnoring trailing <CR><LF>
  bool compare_nmea(const TCHAR* str) {
    for (auto first1 = std::begin(wait_str); first1 != std::end(wait_str) && valid_char(*first1); ++first1, ++str) {
      if ((*first1) != (*str)) {
        return false;
      }
    }
    return true;
  }

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
