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
#include <utility>

wait_ack::wait_ack(Mutex& mtx, const char* str) : mutex(mtx), wait_str(str) {
  // remove trailing <CR><LF>
  std::string_view::size_type n = wait_str.find_last_not_of("\r\n");
  if (n != std::string_view::npos) {
    wait_str = wait_str.substr(0, n + 1);
  }
}

bool wait_ack::check(const char* str) {
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
  if (!ready) {
    condition.Wait(mutex, timeout_ms);
  }
  return std::exchange(ready, false);
}

static char valid_char(char c) {
  return (c == '\r' || c == '\n') ? '\0' : c;
}

bool wait_ack::compare_nmea(const char* str) {
  for (auto c : wait_str) {
    if (c != valid_char(*(str++))) {
      return false;
    }
  }
  return valid_char(*str) == '\0';
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include <thread>
#include <chrono>

TEST_CASE("testing the wait_ack class") {
  using namespace std::chrono_literals;

  SUBCASE("method : wait_ack::check()") {
    const char* array[] = {"abcd", "abcd\r", "abcd\n", "abcd\r\n"};
    Mutex mtx;
    for (auto a : array) {
      wait_ack test(mtx, a);
      for (auto b : array) {
        CHECK_UNARY(test.check(b));
        CHECK_UNARY(test.check("abcd\r\nef"));  // <CR><LF> is the end of string ...
        CHECK_UNARY_FALSE(test.check("azerty"));
        CHECK_UNARY_FALSE(test.check("a"));
      }
    }
  }

  SUBCASE("method : wait_ack::wait()") {
    Mutex mtx;
    ScopeLock lock(mtx);
    wait_ack test(mtx, "abcd");

    std::thread set_thread([&] {
      ScopeLock lock(mtx);
      std::this_thread::sleep_for(500ms);
      test.check("abcd");
    });

    CHECK_UNARY(test.wait(5000));
    CHECK_UNARY_FALSE(test.wait(1));
    set_thread.join();
  }
}

#endif  // DOCTEST_CONFIG_DISABLE
