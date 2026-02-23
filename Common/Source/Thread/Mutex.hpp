/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Mutex.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015, 2:20 PM
 */

#ifndef THREAD_MUTEX_HPP
#define THREAD_MUTEX_HPP
#include "options.h"

#ifdef USE_STDCPP_THREADS
#include "stdcpp/Mutex.hpp"
#elif defined(USE_POCO_THREADS)
#include "Poco/Mutex.hpp"
#else
#error multithreading library is not defined
#endif


#include <functional>
#include <mutex>
#include <type_traits>
/**
 * Invoke a callable object within Mutex lock.
 */
template <typename MutexT, typename Callable, typename... Args>
std::invoke_result_t<Callable, Args...>
WithLock(MutexT& m, Callable&& fn, Args&&... args) {
  std::lock_guard<MutexT> lock(m);
  return std::invoke(std::forward<Callable>(fn), std::forward<Args>(args)...);
}

#endif /* THREAD_MUTEX_HPP */
