/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Thread.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015, 2:20 PM
 */

#ifndef _THREAD_STDCPP_THREAD_HPP_
#define _THREAD_STDCPP_THREAD_HPP_

#include <thread>
#include <string>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include "Thread/Mutex.hpp"

#ifdef __linux__
#include <sys/prctl.h>
#endif

static inline void set_thread_name(const std::string& name) {
#ifdef __linux__
  prctl(PR_SET_NAME, name.c_str());
#endif
}

/**
 * Abstract base class for managed threads.
 *
 * Usage contract for subclasses:
 *  1. Always call Join() (or ensure thread has finished) before destruction.
 *  2. Never call Join() or Start() from inside Run().
 *  3. Implement a Stop() method that signals Run() to exit, then calls Join().
 *  4. Subclass destructor must ensure the thread is stopped before base
 * destructor runs.
 *
 * Example:
 *   class MyThread : public Thread {
 *    public:
 *     void Stop() { _stop = true; Join(); }
 *     ~MyThread() { assert(!IsDefined()); }
 *    protected:
 *     void Run() override { while (!_stop) { ... } }
 *    private:
 *     std::atomic<bool> _stop = false;
 *   };
 */

class Thread {
 public:
  // Deleted constructors: threads must always be named
  Thread() = delete;
  Thread(const Thread&) = delete;
  Thread(Thread&&) = delete;

  Thread& operator=(const Thread&) = delete;
  Thread& operator=(Thread&&) = delete;

  /**
   * Constructor: requires a thread name for debugging.
   * @param name Thread name visible in debuggers and system tools
   */
  explicit Thread(const char* name) : _thread_name(name ? name : "") {
    assert(name != nullptr);
  }

  /**
   * Destructor: asserts that thread has been joined before destruction.
   * This catches a common bug where subclass forgets to implement proper
   * cleanup in its destructor.
   */
  virtual ~Thread() {
    // Catch missing Join() before destruction in debug builds.
    assert(!_running.load(std::memory_order_acquire));
  }

  /**
   * Starts the thread, executing Run() on the new thread.
   *
   * BEHAVIOR:
   * - Returns false if thread is already running (prevents double-start)
   * - Blocks until the new thread has fully initialized its identity
   * - After return, IsDefined() and IsInside() will work correctly
   * - Can be called again after Join() to restart the thread
   *
   * SYNCHRONIZATION:
   * Uses condition variable to ensure Start() doesn't return until the new
   * thread has set both _thread_id and _running. This prevents any observable
   * window of inconsistent state.
   *
   * @return true if thread was started, false if already running
   */
  virtual bool Start() {
    // Block Start() until the new thread has fully initialized both _thread_id
    // and _running. This ensures no caller can observe a window where
    // _running=true but _thread_id is invalid, or vice versa.
    std::unique_lock<std::mutex> lock(_start_mutex);

    // Prevent starting an already running thread.
    // Calling Start() from inside Run() would also be caught here.
    if (_running.load(std::memory_order_acquire) || _thread.joinable()) {
      return false;
    }

    _start_ready = false;

    // _thread is only ever accessed from the owner thread (Start, Join,
    // destructor), never from inside Run(). This means we don't need to
    // synchronize access to _thread itself.
    //
    // Restart after Join() is naturally supported: std::thread is simply
    // overwritten with a new instance after the previous one has been joined
    // (joinable() == false).
    _thread = std::thread([this]() {
      // Set platform-specific thread name for debugging
      set_thread_name(_thread_name);

      // CRITICAL SECTION: Initialize thread identity atomically
      WithLock(_start_mutex, [this]() {
        // Store our own ID atomically before entering Run().
        // This is the only safe way to implement IsInside():
        //
        // WHY NOT USE _thread.get_id()?
        // We cannot read _thread.get_id() from inside the new thread because
        // the _thread object may not have been fully assigned yet on the
        // parent thread. This creates a data race and undefined behavior on
        // weak memory models (ARM, PowerPC, etc).
        //
        // By storing std::this_thread::get_id() here, we depend only on our
        // own thread-local state — no synchronization with the parent thread
        // required.
        //
        // Both _thread_id and _running are set under the same lock to ensure
        // they become atomically visible together. No other thread can observe
        // a window where _running is true but _thread_id is still invalid.
        _thread_id.store(std::this_thread::get_id(), std::memory_order_relaxed);
        _running.store(true, std::memory_order_relaxed);
        _start_ready = true;

        // Notify Start() that initialization is complete before releasing the
        // lock. Start() can now safely return.
        _start_cv.notify_one();
      });

      // Execute the subclass's thread function
      Run();

      // SHUTDOWN: Clear thread identity atomically
      // Both _running and _thread_id are cleared under the same lock for
      // consistency. Join() uses _thread.joinable() as its gate, not _running,
      // so there is no risk of Join() returning before this runs.
      WithLock(_start_mutex, [this]() {
        _running.store(false, std::memory_order_relaxed);
        _thread_id.store(std::thread::id{}, std::memory_order_relaxed);
      });
    });

    // Block until the thread has set both _thread_id and _running.
    // After this wait returns, IsInside() and IsDefined() will work correctly
    // for any thread that calls them after Start() returns.
    _start_cv.wait(lock, [this]() {
      return _start_ready;
    });

    return true;
  }

  /**
   * Waits for the thread to finish execution.
   *
   * BEHAVIOR:
   * - Must be called before destructor (asserted in debug builds)
   * - Can be called multiple times safely (no-op if already joined)
   * - Asserts if called from within the thread itself (would deadlock)
   *
   * SYNCHRONIZATION:
   * Uses _thread.joinable() as the authoritative gate (not _running) because
   * it's set by std::thread's constructor and cleared only by join()/detach().
   * Using _running would miss the brief window during thread startup.
   */
  void Join() {
    // Deadlock prevention: joining from inside the thread itself would block
    // forever. This is a programming error and is caught here.
    assert(!IsInside());

    // _thread.joinable() is the authoritative gate: it's set by std::thread's
    // constructor and cleared only by join()/detach(). Using _running here
    // would miss the startup window between thread creation and Run() entry.
    if (_thread.joinable()) {
      _thread.join();
    }
  }

  /**
   * Returns true if the thread is currently running.
   *
   * "Running" means between Start() returning and Run() completing.
   * Safe to call from any thread at any time.
   *
   * IMPLEMENTATION NOTE:
   * Uses acquire ordering to ensure visibility of all writes made by the
   * thread before it set _running=true (in Start) or cleared it (in shutdown).
   *
   * @return true if thread is running, false otherwise
   */
  bool IsDefined() const {
    return _running.load(std::memory_order_acquire);
  }

  /**
   * Returns true if called from within this thread's own Run() context.
   *
   * USAGE:
   * Used to detect when code is executing within the thread, allowing for:
   * - Preventing deadlock (e.g., Join() asserts !IsInside())
   * - Conditional logic based on execution context
   * - Debugging/assertions about threading requirements
   *
   * IMPLEMENTATION:
   * Checks _running first as a fast-path: if not running, _thread_id is
   * guaranteed to be invalid. Only if _running is true do we compare thread
   * IDs. Since _thread_id is set before _running (see Start()), once _running
   * is observed as true, _thread_id is guaranteed to hold a valid ID.
   *
   * Safe to call from any thread, including from Run() itself.
   *
   * @return true if called from this thread's Run() context, false otherwise
   */
  bool IsInside() const {
    // Check _running first: if not running, _thread_id may be {} or stale.
    // This is a performance optimization and correctness check:
    // - Performance: avoids expensive get_id() call if thread isn't running
    // - Correctness: ensures we only compare IDs when _thread_id is valid
    //
    // Memory ordering: acquire on _running ensures we see the _thread_id
    // value that was written before _running was set to true.
    return _running.load(std::memory_order_acquire) &&
           (_thread_id.load(std::memory_order_acquire) ==
            std::this_thread::get_id());
  }

 protected:
  /**
   * Thread entry point - implemented by subclasses.
   *
   * CONTRACT:
   * - Must eventually return to allow clean shutdown
   * - Should check a stop flag periodically to enable cancellation
   * - Must never call Join() or Start() (would cause deadlock/error)
   */
  virtual void Run() = 0;

 private:
  // Thread name for debugging (visible in system tools)
  const std::string _thread_name;

  // Clean boolean sentinel for thread liveness.
  // WHY NOT RELY ON std::thread::id{}?
  // Using thread::id{} as a "not running" sentinel is problematic because:
  // - std::thread::id{} behavior is implementation-defined
  // - Comparing against it may not be reliable across platforms
  // - A separate bool is clearer and more explicit about intent
  std::atomic<bool> _running{false};

  // Thread identity: only used for IsInside() checks.
  // WHY STORE THIS SEPARATELY?
  // The thread stores its own ID (not the parent) to avoid data races with
  // the parent thread's writes to the _thread object. See Start() for details.
  std::atomic<std::thread::id> _thread_id{};

  // Startup synchronization primitives.
  // PURPOSE: Ensures Start() does not return until both _running and
  // _thread_id are fully initialized by the new thread. This prevents any
  // observable window of inconsistent state where one is set but not the other.
  std::mutex _start_mutex;
  std::condition_variable _start_cv;
  bool _start_ready{false};

  // The actual std::thread object.
  // THREAD SAFETY: Only accessed from the owner thread (Start, Join,
  // destructor), never from inside Run(). Therefore no synchronization is
  // needed on this member itself.
  std::thread _thread;
};

#endif  //_THREAD_STDCPP_THREAD_HPP_
