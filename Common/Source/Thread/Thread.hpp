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
#include <utility>


#ifdef USE_STDCPP_THREADS
#include "stdcpp/Thread.hpp"
#elif defined(USE_POCO_THREADS)
#include "Poco/Thread.hpp"
#else
#error multithreading library is not defined
#endif

template <typename Callable, typename... Args>
class InvokeThread : public Thread {
 public:
  InvokeThread(const char* name, Callable&& func, Args&&... args)
      : Thread(name),
        func(std::forward<Callable>(func)),
        args(std::forward<Args>(args)...) {}

  void Run() override {
    std::apply(func, args);
  }

 private:
  std::decay_t<Callable> func;
  std::tuple<std::decay_t<Args>...> args;
};

#endif  //_THREAD_THREAD_HPP_
