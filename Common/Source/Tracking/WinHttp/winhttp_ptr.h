/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   winhttp_ptr.h
 * Author: Bruno de Lacheisserie
 *
 * Created on March 18, 2026
 */
#ifndef WINHTTP_WINHTTP_PTR_H_
#define WINHTTP_WINHTTP_PTR_H_

#include "Defines.h"

#include <windows.h>
#include <winhttp.h>
#include <utility>

class winhttp_handle_ptr {
 public:
  winhttp_handle_ptr() = default;

  explicit winhttp_handle_ptr(HINTERNET handle) : hHandle(handle) {}

  winhttp_handle_ptr(winhttp_handle_ptr&& src) noexcept {
    hHandle = std::exchange(src.hHandle, nullptr);
  }

  winhttp_handle_ptr& operator=(winhttp_handle_ptr&& src) noexcept {
    std::swap(hHandle, src.hHandle);
    return *this;
  }

  winhttp_handle_ptr(const winhttp_handle_ptr&) = delete;
  winhttp_handle_ptr& operator=(const winhttp_handle_ptr&) = delete;

  virtual ~winhttp_handle_ptr() {
    if (hHandle) {
      WinHttpCloseHandle(hHandle);
    }
  }

  HINTERNET get() const noexcept {
    return hHandle;
  }

  explicit operator bool() const noexcept {
    return hHandle != nullptr;
  }

 private:
  HINTERNET hHandle = nullptr;
};

class winhttp_session_ptr : public winhttp_handle_ptr {
 public:
  winhttp_session_ptr()
      : winhttp_handle_ptr(WinHttpOpen(L"LK8000/" LKVERSION L"." LKRELEASE,
                                       WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                       WINHTTP_NO_PROXY_NAME,
                                       WINHTTP_NO_PROXY_BYPASS, 0)) {}
};

#endif  // WINHTTP_WINHTTP_PTR_H_
