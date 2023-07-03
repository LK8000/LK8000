/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   wait_ack.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 3 July 2023
 */

#include "options.h"
#include "wait_ack.h"

wait_ack::wait_ack(const char* str) : wait_str(str) {
  // remove trailing <CR><LF>
  tstring_view::size_type n = wait_str.find_last_not_of("\r\n");
  if (n != tstring_view::npos) {
    wait_str = wait_str.substr(0, n + 1);
  }
}

bool wait_ack::check(const TCHAR* str) {
  bool signal = WithLock(mutex, [&]() {
    if (compare_nmea(str)) {
      ready = true;
      return true;
    }
    return false;
  });

  if (signal) {
    condition.Signal();
  }
  return signal;
}

bool wait_ack::wait(unsigned timeout_ms) {
  ScopeLock lock(mutex);
  if (!ready) {
    condition.Wait(mutex, timeout_ms);
  }
  return std::exchange(ready, false);
}

static TCHAR valid_char(TCHAR c) {
  return c == _T('\r') || c == ('\n') ? _T('\0') : c;
}

bool wait_ack::compare_nmea(const TCHAR* str) {
  for (auto c : wait_str) {
    if (c != valid_char(*(str++))) {
      return false;
    }
  }
  return valid_char(*str) == _T('\0');
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include <thread>
#include <chrono>

TEST_CASE("testing the wait_ack class") {
  using namespace std::chrono_literals;

  SUBCASE("method : wait_ack::check()") {
    const char* array[] = {"abcd", "abcd\r", "abcd\n", "abcd\r\n"};

    for (auto a : array) {
      wait_ack test(a);
      for (auto b : array) {
        CHECK_UNARY(test.check(to_tstring(b).c_str()));
        CHECK_UNARY(test.check(_T("abcd\r\nef")));  // <CR><LF> is the end of string ...
        CHECK_UNARY_FALSE(test.check(_T("azerty")));
        CHECK_UNARY_FALSE(test.check(_T("a")));
      }
    }
  }

  SUBCASE("method : wait_ack::wait()") {
    wait_ack test("abcd");
    std::thread set_thread([&] {
      std::this_thread::sleep_for(500ms);
      CHECK_UNARY(test.check(_T("abcd")));
    });
    CHECK_UNARY(test.wait(5000));
    CHECK_UNARY_FALSE(test.wait(1));
    set_thread.join();
  }
}

#endif  // DOCTEST_CONFIG_DISABLE
