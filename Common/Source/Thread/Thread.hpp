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
#include "options.h"
#include <functional>

#ifdef USE_STDCPP_THREADS
#include "stdcpp/Thread.hpp"
#elif defined(USE_POCO_THREADS)
#include "Poco/Thread.hpp"
#else
#error multithreading library is not defined
#endif


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
