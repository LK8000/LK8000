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

#ifndef _THREAD_STDCPP_HANDLE_HPP_
#define _THREAD_STDCPP_HANDLE_HPP_
#include <thread>

class ThreadHandle {
 public:
  ThreadHandle() = default;

  bool IsInside() const {
    return _thread_id == std::this_thread::get_id();
  }

  static ThreadHandle GetCurrent() {
    return ThreadHandle(std::this_thread::get_id());
  }

 private:
  explicit ThreadHandle(std::thread::id thread_id) : _thread_id(thread_id) {}

  std::thread::id _thread_id;
};

#endif  // _THREAD_STDCPP_HANDLE_HPP_
