/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Cond.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015
 */

#ifndef THREAD_POCO_COND_HPP
#define THREAD_POCO_COND_HPP

#include "Mutex.hpp"
#include "Poco/Condition.h"
#include <chrono>

namespace std {
  enum class cv_status { no_timeout, timeout };
}

class Cond : public Poco::Condition {
 public:
  using Poco::Condition::Condition;

  template <typename Lock, typename Rep, typename Period>
  std::cv_status wait_for(Lock& lock, const std::chrono::duration<Rep, Period>& rtime) {
    if (!tryWait(lock, std::chrono::duration_cast<std::chrono::milliseconds>(rtime).count())) {
      return std::cv_status::timeout;
    }
    return std::cv_status::no_timeout;
  }

  template <class Lock, class Clock, class Duration>
  std::cv_status wait_until(Lock& lock, const std::chrono::time_point<Clock, Duration>& abs_time) {
    auto remaining = abs_time - Clock::now();
    if (remaining <= Duration::zero()) {
        return std::cv_status::timeout;
    }
    return wait_for(lock, remaining);
  }

  void notify_all() {
    Poco::Condition::broadcast();
  }

  void notify_one() {
    Poco::Condition::signal();
  }
};

#endif /* THREAD_POCO_COND_HPP */
