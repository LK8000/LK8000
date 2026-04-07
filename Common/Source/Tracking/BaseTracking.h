/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   BaseTracking.h
 * Author: Bruno de Lacheisserie
 *
 * Created on February 18, 2024
 */

#ifndef TRACKING_BASETRACKING_H
#define TRACKING_BASETRACKING_H

#include <optional>
#include <utility>
#include <cassert>
#include "Thread/Thread.hpp"
#include "Thread/Cond.hpp"
#include "http_session.h"
#include "ITrackingHandler.h"

template <typename T>
class BaseTracking : public Thread, public ITrackingHandler {
 public:
  using Thread::Thread;

  ~BaseTracking() override {
    // Derived class must call StopAndJoin() in its destructor
    assert(!IsDefined());
  }

 protected:
  void StopAndJoin() {    
    WithLock(queue_mtx, [&]() {
      thread_stop = true;
    });
    queue_cv.Broadcast();
    if (IsDefined()) {
      Join();
    }
  }

  void Run() override {
    http_session http;
    do {
      auto item = WithLock(queue_mtx, [&]() {
        return std::exchange(queue, std::nullopt);
      });

      if (item) {
        Send(http, item.value());
      }
    } while (Wait());
  }

  virtual void Send(http_session& http, const T& item) = 0;

  void Push(T&& item) {
    WithLock(queue_mtx, [&]() {
      queue = std::move(item);
    });
    queue_cv.Broadcast();
  }

 private:
  bool Wait() {
    ScopeLock lock(queue_mtx);
    while (!thread_stop && !queue.has_value()) {
      queue_cv.Wait(queue_mtx);
    }
    return !thread_stop;
  }

  bool thread_stop = false;
  std::optional<T> queue;
  Mutex queue_mtx;
  Cond queue_cv;
};

#endif  // TRACKING_BASETRACKING_H_
