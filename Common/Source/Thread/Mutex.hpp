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

/**
 * Invoke a callable object within Mutex lock.
 */
template <typename _Mutex, typename _Callable, typename... _Args>
std::invoke_result_t<_Callable, _Args...>
WithLock(_Mutex& m, _Callable&& fn, _Args&&... args) noexcept(
    std::is_nothrow_invocable_v<_Callable, _Args...>) {
  std::lock_guard<_Mutex> lock(m);
  return std::invoke(std::forward<_Callable>(fn), std::forward<_Args>(args)...);
}

#endif /* THREAD_MUTEX_HPP */
