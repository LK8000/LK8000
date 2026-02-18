/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   MutexCondTest.cpp
 * Author: Bruno de Lacheisserie
 */
#include "options.h"

/*
 * Unit tests for Cond.hpp and Mutex.hpp
 */

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#include "Thread/Cond.hpp"
#include "Thread/Mutex.hpp"

#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

using namespace std::chrono_literals;

// ─── Mutex ───────────────────────────────────────────────────────────────────

TEST_CASE("Mutex: lock and unlock") {
  Mutex mtx;
  REQUIRE_NOTHROW(mtx.lock());
  REQUIRE_NOTHROW(mtx.unlock());
}

TEST_CASE("Mutex: try_lock succeeds when unlocked") {
  Mutex mtx;
  CHECK(mtx.try_lock());
  mtx.unlock();
}

TEST_CASE("Mutex: recursive locking from same thread is allowed") {
  Mutex mtx;
  REQUIRE_NOTHROW(mtx.lock());
  REQUIRE_NOTHROW(mtx.lock());  // would deadlock on a non-recursive mutex
  REQUIRE_NOTHROW(mtx.unlock());
  REQUIRE_NOTHROW(mtx.unlock());
}

TEST_CASE("Mutex: try_lock succeeds recursively from same thread") {
  Mutex mtx;
  mtx.lock();
  CHECK(mtx.try_lock());  // non-recursive would return false or deadlock
  mtx.unlock();
  mtx.unlock();
}

TEST_CASE(
    "Mutex: lock is not released until all recursive unlocks are called") {
  Mutex mtx;
  std::atomic<bool> acquired{false};

  mtx.lock();
  mtx.lock();  // second level

  mtx.unlock();  // only first unlock — lock still held

  std::thread t([&]() {
    acquired = mtx.try_lock();
    if (acquired) {
      mtx.unlock();
    }
  });
  t.join();

  CHECK_FALSE(acquired);  // still locked after one unlock

  mtx.unlock();  // now fully released
}

TEST_CASE("Mutex: try_lock from another thread fails while mutex is held") {
  // Even though the mutex is recursive, recursion only applies to the
  // owning thread — other threads must still block.
  Mutex mtx;
  mtx.lock();

  std::atomic<bool> result{true};
  std::thread t([&]() {
    result = mtx.try_lock();
  });
  t.join();

  CHECK_FALSE(result);
  mtx.unlock();
}

TEST_CASE("Mutex: ScopeUnlock temporarily releases the lock") {
  Mutex mtx;
  mtx.lock();

  std::atomic<bool> acquired{false};
  {
    ScopeUnlock ul(mtx);  // releases lock within this scope
    std::thread t([&]() {
      acquired = mtx.try_lock();
      if (acquired) {
        mtx.unlock();
      }
    });
    t.join();
  }  // lock re-acquired here

  CHECK(acquired);
  mtx.unlock();
}

TEST_CASE("Mutex: exclusive access between threads") {
  Mutex mtx;
  int counter = 0;
  const int iterations = 1000;

  auto increment = [&]() {
    for (int i = 0; i < iterations; ++i) {
      mtx.lock();
      ++counter;
      mtx.unlock();
    }
  };

  std::thread t1(increment);
  std::thread t2(increment);
  t1.join();
  t2.join();

  CHECK(counter == 2 * iterations);
}

// ─── Cond ────────────────────────────────────────────────────────────────────

TEST_CASE("Cond: wait_for times out when not signalled") {
  Mutex mtx;
  Cond cond;
  mtx.lock();

  auto status = cond.wait_for(mtx, 50ms);

  CHECK(status == std::cv_status::timeout);
  mtx.unlock();
}

TEST_CASE("Cond: wait_for returns no_timeout when notified via notify_one") {
  Mutex mtx;
  Cond cond;
  std::atomic<bool> ready{false};

  std::thread t([&]() {
    std::this_thread::sleep_for(20ms);
    mtx.lock();
    ready = true;
    cond.notify_one();
    mtx.unlock();
  });

  mtx.lock();
  auto status = cond.wait_for(mtx, 500ms);
  bool was_ready = ready.load();
  mtx.unlock();

  t.join();

  CHECK(status == std::cv_status::no_timeout);
  CHECK(was_ready);
}

TEST_CASE("Cond: wait_for returns no_timeout when notified via notify_all") {
  Mutex mtx;
  Cond cond;
  std::atomic<int> woken{0};
  const int num_waiters = 3;

  auto waiter = [&]() {
    mtx.lock();
    cond.wait_for(mtx, 500ms);
    ++woken;
    mtx.unlock();
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < num_waiters; ++i) {
    threads.emplace_back(waiter);
  }

  std::this_thread::sleep_for(30ms);  // let all waiters block
  cond.notify_all();

  for (auto& t : threads) {
    t.join();
  }

  CHECK(woken == num_waiters);
}

TEST_CASE("Cond: wait_until times out when deadline is in the past") {
  Mutex mtx;
  Cond cond;
  mtx.lock();

  auto past = std::chrono::steady_clock::now() - 100ms;
  auto status = cond.wait_until(mtx, past);

  CHECK(status == std::cv_status::timeout);
  mtx.unlock();
}

TEST_CASE("Cond: wait_until returns no_timeout when notified before deadline") {
  Mutex mtx;
  Cond cond;
  std::atomic<bool> signalled{false};

  std::thread t([&]() {
    std::this_thread::sleep_for(20ms);
    mtx.lock();
    signalled = true;
    cond.notify_one();
    mtx.unlock();
  });

  mtx.lock();
  auto deadline = std::chrono::steady_clock::now() + 500ms;
  auto status = cond.wait_until(mtx, deadline);
  bool was_signalled = signalled.load();
  mtx.unlock();

  t.join();

  CHECK(status == std::cv_status::no_timeout);
  CHECK(was_signalled);
}

TEST_CASE("Cond: producer-consumer basic synchronisation") {
  Mutex mtx;
  Cond cond;
  int value = 0;
  std::atomic<bool> consumed{false};

  std::thread producer([&]() {
    std::this_thread::sleep_for(20ms);
    mtx.lock();
    value = 42;
    cond.notify_one();
    mtx.unlock();
  });

  std::thread consumer([&]() {
    mtx.lock();
    // predicate loop guards against spurious wakeups
    while (value == 0) {
      if (cond.wait_for(mtx, 500ms) == std::cv_status::timeout) {
        break;
      }
    }
    consumed = (value == 42);
    mtx.unlock();
  });

  producer.join();
  consumer.join();

  CHECK(consumed);
}

TEST_CASE("Cond: wait with predicate") {
  Mutex mtx;
  Cond cond;
  bool ready = false;

  std::thread t([&]() {
    std::this_thread::sleep_for(20ms);
    mtx.lock();
    ready = true;
    cond.notify_one();
    mtx.unlock();
  });

  mtx.lock();
  cond.wait(mtx, [&]() {
    return ready;
  });
  CHECK(ready);
  mtx.unlock();

  t.join();
}

TEST_CASE("Cond: wait_for with predicate returns true when condition met") {
  Mutex mtx;
  Cond cond;
  bool ready = false;

  std::thread t([&]() {
    std::this_thread::sleep_for(20ms);
    mtx.lock();
    ready = true;
    cond.notify_one();
    mtx.unlock();
  });

  mtx.lock();
  bool result = cond.wait_for(mtx, 500ms, [&]() {
    return ready;
  });
  CHECK(result);
  CHECK(ready);
  mtx.unlock();

  t.join();
}

TEST_CASE("Cond: wait_for with predicate returns false on timeout") {
  Mutex mtx;
  Cond cond;
  bool ready = false;

  mtx.lock();
  bool result = cond.wait_for(mtx, 10ms, [&]() {
    return ready;
  });
  CHECK_FALSE(result);
  mtx.unlock();
}

TEST_CASE("Cond: wait_until with predicate returns true when condition met") {
  Mutex mtx;
  Cond cond;
  bool ready = false;

  std::thread t([&]() {
    std::this_thread::sleep_for(20ms);
    mtx.lock();
    ready = true;
    cond.notify_one();
    mtx.unlock();
  });

  mtx.lock();
  auto deadline = std::chrono::steady_clock::now() + 500ms;
  bool result = cond.wait_until(mtx, deadline, [&]() {
    return ready;
  });
  CHECK(result);
  CHECK(ready);
  mtx.unlock();

  t.join();
}

TEST_CASE("Cond: wait_until with predicate returns false on timeout") {
  Mutex mtx;
  Cond cond;
  bool ready = false;

  mtx.lock();
  auto deadline = std::chrono::steady_clock::now() + 10ms;
  bool result = cond.wait_until(mtx, deadline, [&]() {
    return ready;
  });
  CHECK_FALSE(result);
  mtx.unlock();
}

#endif
