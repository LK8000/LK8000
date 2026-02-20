/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Thread.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015, 2:20 PM
 */

#ifndef _THREAD_POCO_THREAD_HPP_
#define _THREAD_POCO_THREAD_HPP_

#include "Poco/Thread.h"
#include <cassert>

class Thread : protected Poco::Runnable {
 public:
  Thread() = delete;
  Thread(const Thread&) = delete;
  Thread(Thread&&) = delete;

  Thread& operator=(const Thread&) = delete;
  Thread& operator=(Thread&&) = delete;

  explicit Thread(const char* name) : _thread(name ? name : "") {
    assert(name != nullptr);
  }

  ~Thread() override {
    assert(!_thread.isRunning());
  }

  virtual bool Start() {
    Poco::ScopedLock<Poco::Mutex> lock(_mutex);
    if (_thread.isRunning()) {
      return false;
    }
    _thread.start(*this);
    return true;
  }
  void Join() {
    _thread.join();
  }

  bool IsDefined() const {
    return _thread.isRunning();
  }

  bool IsInside() const {
    return (_thread.currentTid() == _thread.tid());
  }

 protected:
  virtual void Run() = 0;

 private:
  void run() override {
    Run();
  }

  mutable Poco::Mutex _mutex;
  Poco::Thread _thread;
};

#endif  //_THREAD_POCO_THREAD_HPP_
