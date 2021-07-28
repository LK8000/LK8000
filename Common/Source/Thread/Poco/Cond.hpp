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

class Cond : public Poco::Condition {
 public:
  Cond() = default;

  void Wait(Mutex& mutex) {
    Poco::Condition::wait<Mutex>(mutex);
  }

  bool Wait(Mutex& mutex, unsigned timeout_ms) {
    return Poco::Condition::tryWait<Mutex>(mutex, timeout_ms);
  }

  void Broadcast() {
    Poco::Condition::broadcast();
  }

  void Signal() {
    Poco::Condition::signal();
  }
};

#endif /* THREAD_POCO_COND_HPP */
