/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Handle.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015
 */

#ifndef _THREAD_POCO_HANDLE_HPP_
#define _THREAD_POCO_HANDLE_HPP_
#include "Poco/Thread.h"

class ThreadHandle {
 public:
  ThreadHandle() = default;

  bool IsInside() const {
    return _thread == Poco::Thread::current();
  }

  static ThreadHandle GetCurrent() {
    return ThreadHandle(Poco::Thread::current());
  }

 private:
  explicit ThreadHandle(Poco::Thread* thread) : _thread(thread) {}

  Poco::Thread* _thread = nullptr;
};

#endif  // _THREAD_POCO_HANDLE_HPP_
