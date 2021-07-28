/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Cond.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 20 octobre 2016, 16:57
 */

#ifndef THREAD_STDCPP_COND_HPP
#define THREAD_STDCPP_COND_HPP

#include <condition_variable>
#include "Mutex.hpp"

class Cond : public std::condition_variable_any {
 public:
  Cond() = default;

  void Wait(Mutex& mutex) {
    std::condition_variable_any::wait(mutex);
  }

  bool Wait(Mutex& mutex, unsigned timeout_ms) {
    auto timeout = std::chrono::milliseconds(timeout_ms);
    std::cv_status ret = std::condition_variable_any::wait_for(mutex, timeout);
    return ret != std::cv_status::timeout;
  }

  void Broadcast() {
    std::condition_variable_any::notify_all();
  }

  void Signal() {
    std::condition_variable_any::notify_one();
  }
};

#endif /* THREAD_STDCPP_COND_HPP */
