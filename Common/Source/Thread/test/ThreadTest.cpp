/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ThreadTest.cpp
 * Author: Bruno de Lacheisserie
 */
#include "options.h"
#include "Thread/Thread.hpp"
#include "OS/Sleep.h"
#include <atomic>
#include <chrono>

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

// Concrete implementation for testing
class TestThread : public Thread {
 public:
  std::atomic<bool> ran{false};
  std::atomic<int> runCount{0};
  int sleepMs = 0;

  explicit TestThread(const char* name = "TestThread") : Thread(name) {}

 protected:
  void Run() override {
    ran = true;
    runCount++;
    if (sleepMs > 0) {
      Sleep(sleepMs);
    }
  }
};

class LongRunningThread : public Thread {
 public:
  std::atomic<bool> shouldStop{false};
  std::atomic<bool> started{false};

  explicit LongRunningThread(const char* name = "LongRunningThread")
      : Thread(name) {}

 protected:
  void Run() override {
    started = true;
    while (!shouldStop) {
      Sleep(10);
    }
  }
};

TEST_SUITE("Thread") {
  TEST_CASE("Thread can be started and joined") {
    TestThread t("test1");
    CHECK(t.Start());
    t.Join();
    CHECK(t.ran);
  }

  TEST_CASE("IsDefined returns false before start") {
    TestThread t("test2");
    CHECK_FALSE(t.IsDefined());
  }

  TEST_CASE("IsDefined returns true while running") {
    LongRunningThread t("test3");
    CHECK(t.Start());
    // Wait until thread starts
    while (!t.started) {
      Sleep(1);
    }
    CHECK(t.IsDefined());
    t.shouldStop = true;
    t.Join();
  }

  TEST_CASE("IsDefined returns false after join") {
    TestThread t("test4");
    t.Start();
    t.Join();
    CHECK_FALSE(t.IsDefined());
  }

  TEST_CASE("Start returns false if already running") {
    LongRunningThread t("test5");
    CHECK(t.Start());
    while (!t.started) {
      Sleep(1);
    }
    CHECK_FALSE(t.Start());
    t.shouldStop = true;
    t.Join();
  }

  TEST_CASE("Run is actually called") {
    TestThread t("test6");
    CHECK(t.runCount == 0);
    t.Start();
    t.Join();
    CHECK(t.runCount == 1);
  }

  TEST_CASE("IsInside returns false from outside thread") {
    LongRunningThread t("test7");
    t.Start();
    while (!t.started) {
      Sleep(1);
    }
    CHECK_FALSE(t.IsInside());
    t.shouldStop = true;
    t.Join();
  }

  TEST_CASE("IsInside returns false when thread is not running") {
    TestThread t("test_inside_not_running");
    // Before start
    CHECK_FALSE(t.IsInside());
    // After join
    t.Start();
    t.Join();
    CHECK_FALSE(t.IsInside());
  }

  TEST_CASE("IsInside returns true from inside thread") {
    class InsideCheckThread : public Thread {
     public:
      std::atomic<bool> insideResult{false};
      InsideCheckThread() : Thread("InsideCheck") {}

     protected:
      void Run() override {
        insideResult = IsInside();
      }
    };

    InsideCheckThread t;
    t.Start();
    t.Join();
    CHECK(t.insideResult);
  }

  TEST_CASE("Thread name is set correctly") {
    // Just verify construction with name doesn't crash
    TestThread t("MyNamedThread");
    t.Start();
    t.Join();
    CHECK(t.ran);
  }

  TEST_CASE("Thread runs to completion") {
    TestThread t("test9");
    t.sleepMs = 50;
    t.Start();
    t.Join();
    CHECK(t.ran);
  }

  TEST_CASE("Thread can be restarted after join") {
    TestThread t("restart_test");

    // First run
    CHECK(t.Start());
    t.Join();
    CHECK(t.ran);
    CHECK(t.runCount == 1);
    CHECK_FALSE(t.IsDefined());

    // Reset flag to distinguish second run
    t.ran = false;

    // Second run - this is the restart
    CHECK(t.Start());
    t.Join();
    CHECK(t.ran);
    CHECK(t.runCount == 2);
    CHECK_FALSE(t.IsDefined());
  }

  TEST_CASE("Thread restart accumulates run count correctly") {
    TestThread t("restart_count_test");

    for (int i = 1; i <= 3; ++i) {
      t.Start();
      t.Join();
      CHECK(t.runCount == i);
    }
  }
}
#endif
