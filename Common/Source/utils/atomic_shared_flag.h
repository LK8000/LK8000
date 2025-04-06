/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   atomic_shared_flag.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 06 April 2025
 */

#ifndef UTILS_ATOMIC_FLAGS_COUNTER_H
#define UTILS_ATOMIC_FLAGS_COUNTER_H

#include "Thread/Mutex.hpp"

// This class is a simple atomic counter used to
// implement a shared flag.

class atomic_shared_flag final {
 public:
  atomic_shared_flag() = default;

  atomic_shared_flag(const atomic_shared_flag&) = delete;
  atomic_shared_flag& operator=(const atomic_shared_flag&) = delete;

  atomic_shared_flag(atomic_shared_flag&&) = delete;
  atomic_shared_flag& operator=(atomic_shared_flag&&) = delete;

  void operator=(bool value) {
    if (value) {
      increment();
    }
    else {
      decrement();
    }
  }

  operator bool() const {
    ScopeLock lock(_mtx);
    return _count > 0;
  }

 private:
  // Increment the counter
  void increment() {
    ScopeLock lock(_mtx);
    ++_count;
  }

  // Decrement the counter
  void decrement() {
    ScopeLock lock(_mtx);
    if (_count > 0) {
      --_count;
    }
  }

 private:
  unsigned _count = 0;
  mutable Mutex _mtx;
};

#endif  // UTILS_ATOMIC_FLAGS_COUNTER_H
