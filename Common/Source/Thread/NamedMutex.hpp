/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   NamedMutex.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015, 2:20 PM
 */

#ifndef THREAD_NAMEDMUTEX_HPP
#define THREAD_NAMEDMUTEX_HPP

#include <string>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "tchar.h"
#endif

class NamedMutex final {
 public:
  explicit NamedMutex(const std::basic_string<TCHAR>& name) {
#ifdef _WIN32
    handle_ = CreateMutexW(nullptr, FALSE, name.c_str());
    if (!handle_) {
      throw std::system_error(GetLastError(), std::system_category(), "CreateMutex failed");
    }
#else
    std::string dir = get_temp_dir();
    path_ = dir + "/" + name + ".lock";
    fd_ = ::open(path_.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd_ == -1) {
      throw std::system_error(errno, std::system_category(), "open lock file failed");
    }
#endif
  }

  ~NamedMutex() {
#ifdef _WIN32
    if (handle_) {
      CloseHandle(handle_);
    }
#else
    if (fd_ != -1) {
      ::close(fd_);
    }

#endif
  }

  void lock() {
#ifdef _WIN32
    DWORD res = WaitForSingleObject(handle_, INFINITE);
    if (res != WAIT_OBJECT_0) {
      throw std::system_error(GetLastError(), std::system_category(), "WaitForSingleObject failed");
    }
#else
    while (flock(fd_, LOCK_EX) == -1) {
      if (errno != EINTR) {
        throw std::system_error(errno, std::system_category(), "flock lock failed");
      }
    }
#endif
  }

  bool try_lock() {
#ifdef _WIN32
    DWORD res = WaitForSingleObject(handle_, 0);
    if (res == WAIT_OBJECT_0) {
      return true;
    }
    if (res == WAIT_TIMEOUT) {
      return false;
    }
    throw std::system_error(GetLastError(), std::system_category(), "try_lock failed");
#else
    int res = flock(fd_, LOCK_EX | LOCK_NB);
    if (res == 0) {
      return true;
    }
    if (errno == EWOULDBLOCK) {
      return false;
    }
    throw std::system_error(errno, std::system_category(), "flock try_lock failed");
#endif
  }

  void unlock() {
#ifdef _WIN32
    if (!ReleaseMutex(handle_)) {
      throw std::system_error(GetLastError(), std::system_category(), "ReleaseMutex failed");
    }
#else
    if (flock(fd_, LOCK_UN) == -1) {
      throw std::system_error(errno, std::system_category(), "flock unlock failed");
    }
#endif
  }

  // Non-copyable
  NamedMutex(const NamedMutex&) = delete;
  NamedMutex& operator=(const NamedMutex&) = delete;

 private:
#ifdef _WIN32
  HANDLE handle_ = NULL;
#else
  int fd_ = -1;
  std::string path_;

  static std::string get_temp_dir() {
    if (const char* tmp = std::getenv("TMPDIR")) {
      return tmp;
    }
#ifdef P_tmpdir
    return P_tmpdir;
#else
    return "/tmp";
#endif
  }

#endif
};

#endif  // THREAD_NAMEDMUTEX_HPP
