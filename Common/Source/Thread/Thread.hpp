/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Thread.hpp
 * Author: Bruno de Lacheisserie
 */

#ifndef _THREAD_THREAD_HPP_
#define _THREAD_THREAD_HPP_

#if defined(__GNUC__) && defined(__MINGW32__) && !defined(__STDCPP_THREADS__)
// c++11 thread is not available with mingw
#include "Poco/Thread.hpp"
#else
#include "stdcpp/Thread.hpp"
#endif
#include <functional>

class InvokeThread : public Thread {
 public:
  InvokeThread(const char* name, std::function<void()>&& func) : Thread(name), _func(std::move(func)) {}

  void Run() override {
    _func();
  }

 private:
  std::function<void()> _func;
};

#endif  //_THREAD_THREAD_HPP_
