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

#ifndef UTILS_ATOMIC_SHARED_FLAG_H
#define UTILS_ATOMIC_SHARED_FLAG_H

#include <atomic>
#include <cstdint>

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
      _count.fetch_add(1, std::memory_order_release);
    } else {
      // Guard against underflow: only decrement if count > 0
      uint32_t current = _count.load(std::memory_order_acquire);
      while (current > 0) {
        if (_count.compare_exchange_weak(current, current - 1,
                                         std::memory_order_acq_rel,
                                         std::memory_order_acquire)) {
          return;
        }
      }
    }
  }

  operator bool() const {
    return _count.load(std::memory_order_acquire) != 0;
  }

 private:
  std::atomic_uint32_t _count{0};
};

#endif  // UTILS_ATOMIC_SHARED_FLAG_H
