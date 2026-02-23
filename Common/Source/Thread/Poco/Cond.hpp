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
enum class cv_status {
  no_timeout,
  timeout
};
}

class Cond : public Poco::Condition {
 public:
  using Poco::Condition::Condition;
  using Poco::Condition::wait;

  using clock_t = std::chrono::steady_clock;

  template <class Clock, class Duration>
  using time_point_t = std::chrono::time_point<Clock, Duration>;

  template <typename Rep, typename Period>
  using duration_t = std::chrono::duration<Rep, Period>;

  template <typename Lock, typename Predicate>
  void wait(Lock& lock, Predicate predicate) {
    while (!predicate()) {
      wait(lock);
    }
  }

  template <class Lock, class Clock, class Duration>
  std::cv_status wait_until(Lock& lock,
                            const time_point_t<Clock, Duration>& abs_time) {
    auto remaining = abs_time - Clock::now();
    if (remaining <= Duration::zero()) {
      return std::cv_status::timeout;
    }
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(remaining);
    return tryWait(lock, ms.count()) ? std::cv_status::no_timeout
                                     : std::cv_status::timeout;
  }

  template <class Lock, class Clock, class Duration, class Predicate>
  bool wait_until(Lock& lock, const time_point_t<Clock, Duration>& abs_time,
                  Predicate predicate) {
    while (!predicate()) {
      if (wait_until(lock, abs_time) == std::cv_status::timeout) {
        return predicate();
      }
    }
    return true;
  }

  template <typename Lock, typename Rep, typename Period>
  std::cv_status wait_for(Lock& lock, const duration_t<Rep, Period>& rtime) {
    return wait_until(lock, clock_t::now() + rtime);
  }

  template <typename Lock, typename Rep, typename Period, typename Predicate>
  bool wait_for(Lock& lock, const duration_t<Rep, Period>& rtime,
                Predicate predicate) {
    return wait_until(lock, clock_t::now() + rtime, std::move(predicate));
  }

  void notify_all() {
    Poco::Condition::broadcast();
  }

  void notify_one() {
    Poco::Condition::signal();
  }
};

#endif /* THREAD_POCO_COND_HPP */
