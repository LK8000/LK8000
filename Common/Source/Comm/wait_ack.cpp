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
#include <cassert>

namespace {

/**
 * remove trailing <CR><LF>
 */
void strip_crlf(std::string_view& sv) {
  std::string_view::size_type n = sv.find_last_not_of("\r\n");
  if (n != std::string_view::npos) {
    sv = sv.substr(0, n + 1);
  }
}

char valid_char(char c) {
  return (c == '\r' || c == '\n') ? '\0' : c;
}

}  // namespace

wait_ack::wait_ack(Mutex& mtx, const char* success, const char* error)
    : mutex(mtx), success_str(success ? success : ""), error_str(error ? error : "") {

  strip_crlf(success_str);
  strip_crlf(error_str);

  assert(!success_str.empty()); // can't be an empty string !
}

bool wait_ack::check(const char* str) {
  bool signal = WithLock(mutex, [&]() {
    if (compare_nmea(str, success_str)) {
      result = wait_ack_result::success;
      return true;
    }
    if (!error_str.empty() && compare_nmea(str, error_str)) {
      result = wait_ack_result::error;
      return true;
    }
    return false;
  });

  if (signal) {
    condition.Broadcast();
  }
  return signal;
}

wait_ack_result wait_ack::wait(unsigned timeout_ms) {
  if (result == wait_ack_result::timeout) {
    condition.Wait(mutex, timeout_ms);
  }
  return std::exchange(result, wait_ack_result::timeout);
}

bool wait_ack::compare_nmea(const char* str, std::string_view pattern) {
  for (auto c : pattern) {
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
      wait_ack test(mtx, a, "error");
      for (auto b : array) {
        CHECK_UNARY(test.check(b));
        CHECK_UNARY(test.check("abcd\r\nef"));  // <CR><LF> is the end of string ...
        CHECK_UNARY(test.check("error"));
        CHECK_UNARY_FALSE(test.check("azerty"));
        CHECK_UNARY_FALSE(test.check("a"));
      }
    }
  }

  SUBCASE("method : wait_ack::check() without error string") {
    Mutex mtx;
    wait_ack test(mtx, "abcd", nullptr);
    CHECK_UNARY(test.check("abcd"));
    CHECK_UNARY_FALSE(test.check("error"));
  }

  SUBCASE("method : wait_ack::wait() already signaled") {
    Mutex mtx;
    wait_ack test(mtx, "abcd", "error");

    test.check("abcd");
    {
      ScopeLock lock(mtx);
      CHECK_EQ(test.wait(0), wait_ack_result::success);
    }
  }

  SUBCASE("method : wait_ack::wait()") {
    Mutex mtx;
    ScopeLock lock(mtx);
    wait_ack test(mtx, "abcd", "error");

    std::thread success_thread([&] {
      ScopeLock lock(mtx);
      std::this_thread::sleep_for(500ms);
      test.check("abcd");
    });

    CHECK_EQ(test.wait(5000), wait_ack_result::success);
    CHECK_EQ(test.wait(1), wait_ack_result::timeout);
    success_thread.join();

    std::thread error_thread([&] {
      ScopeLock lock(mtx);
      std::this_thread::sleep_for(500ms);
      test.check("error");
    });

    CHECK_EQ(test.wait(5000), wait_ack_result::error);
    CHECK_EQ(test.wait(1), wait_ack_result::timeout);
    error_thread.join();
  }
}

#endif  // DOCTEST_CONFIG_DISABLE
