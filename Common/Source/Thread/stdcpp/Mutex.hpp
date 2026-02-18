/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Mutex.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 20 octobre 2016, 16:57
 */

#ifndef THREAD_STDCPP_MUTEX_HPP
#define THREAD_STDCPP_MUTEX_HPP

#include <mutex>

using Mutex = std::recursive_mutex;

class ScopeUnlock {
 public:
  explicit ScopeUnlock(Mutex& m) : _mutex(m) {
    _mutex.unlock();
  }

  ~ScopeUnlock() {
    _mutex.lock();
  }

  ScopeUnlock(const ScopeUnlock&) = delete;
  ScopeUnlock& operator=(const ScopeUnlock&) = delete;

 private:
  Mutex& _mutex;
};

#endif /* THREAD_STDCPP_MUTEX_HPP */
