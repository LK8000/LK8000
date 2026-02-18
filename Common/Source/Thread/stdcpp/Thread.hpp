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

#ifndef _THREAD_STDCPP_THREAD_HPP_
#define _THREAD_STDCPP_THREAD_HPP_

#include <thread>
#include <string>
#include <atomic>
#include <cassert>

#ifdef __linux__
#include <sys/prctl.h>
#endif

static inline void set_thread_name(const std::string& name) {
#ifdef __linux__
  prctl(PR_SET_NAME, name.c_str());
#endif
}

class Thread {
 public:
  Thread() = delete;
  Thread(const Thread&) = delete;
  Thread(Thread&&) = delete;

  explicit Thread(const char* name) : _thread_name(name) {}

  virtual ~Thread() {
    assert(!_thread.joinable());
  }

  virtual bool Start() {
    if (_thread.joinable()) {
      return false;  // Already running
    }
    _running = true;

    _thread = std::thread([&]() {
      set_thread_name(_thread_name);

      Run();

      _running = false;
    });

    return true;
  }
  
  void Join() {
    if (_thread.joinable()) {
      _thread.join();
    }
  }

  bool IsDefined() const {
    return _running;
  }

  bool IsInside() const {
    return _thread.get_id() == std::this_thread::get_id();
  }

 protected:
  virtual void Run() = 0;

 private:
  std::atomic<bool> _running = false;
  const std::string _thread_name;
  std::thread _thread;
};

#endif  //_THREAD_STDCPP_THREAD_HPP_
