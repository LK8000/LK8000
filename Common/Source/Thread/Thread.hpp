/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Thread.hpp
 * Author: Bruno de Lacheisserie
 */

#ifndef _THREAD_THREAD_HPP_
#define _THREAD_THREAD_HPP_

#include "Poco/Thread.h"
#include <tuple>
#include <utility>

#ifdef __linux__
#include <linux/prctl.h>  /* Definition of PR_* constants */
#include <sys/prctl.h>
#endif

class Thread : protected Poco::Runnable {
public:
    explicit Thread(const char* name) : _thread(name) {}

    virtual bool Start() {
        _thread.start(*this);
        return _thread.isRunning();
    }

    void Join() {
        _thread.join();
    }

    bool IsInside() const {
        return (_thread.currentTid() == _thread.tid());
    }

    bool IsDefined() const {
        return _thread.isRunning();
    }

protected:
    virtual void Run() = 0;

    void run() override {
#ifdef __linux__
        prctl(PR_SET_NAME, _thread.name().c_str());
#endif
        Run();
    }

private:
    Poco::Thread _thread;
};

template <typename Callable, typename... Args>
class InvokeThread : public Thread {
 public:
  InvokeThread(const char* name, Callable&& func, Args&&... args)
      : Thread(name),
        func(std::forward<Callable>(func)),
        args(std::forward<Args>(args)...) {}

  void Run() override {
    std::apply(func, args);
  }

 private:
  std::decay_t<Callable> func;
  std::tuple<std::decay_t<Args>...> args;
};

#endif //_THREAD_THREAD_HPP_
