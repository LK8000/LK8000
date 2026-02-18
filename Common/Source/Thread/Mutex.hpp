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

#if defined(__GNUC__) && defined(__MINGW32__) && !defined(__STDCPP_THREADS__)
// c++11 thread is not available with mingw
#include "Poco/Mutex.hpp"
#else
#include "stdcpp/Mutex.hpp"
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
