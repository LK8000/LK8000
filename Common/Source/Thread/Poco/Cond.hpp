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

  template <typename _Lock, typename _Rep, typename _Period>
  std::cv_status wait_for(_Lock& __lock, const std::chrono::duration<_Rep, _Period>& __rtime) {
    if (!tryWait(__lock, std::chrono::duration_cast<std::chrono::milliseconds>(__rtime).count())) {
      return std::cv_status::timeout;
    }
    return std::cv_status::no_timeout;
  }

  void notify_all() {
    Poco::Condition::broadcast();
  }

  void notify_one() {
    Poco::Condition::signal();
  }
};

#endif /* THREAD_POCO_COND_HPP */
