/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   curl_ptr.h
 * Author: Bruno de Lacheisserie
 *
 * Created on February 18, 2024
 */
#ifndef _CURL_CURL_PTR_H_
#define _CURL_CURL_PTR_H_

#include <curl/curl.h>
#include <utility>

class curl_ptr {
public:
  curl_ptr() {
    curl = curl_easy_init();
  }

  curl_ptr(curl_ptr&& src) noexcept {
    curl = std::exchange(src.curl, nullptr);
  };

  curl_ptr& operator=(curl_ptr&& src) {
    std::swap(curl, src.curl);
    return *this;
  }

  curl_ptr(const curl_ptr&) = delete;
  curl_ptr& operator=(const curl_ptr&) = delete;

  ~curl_ptr() {
    if (curl) {
      curl_easy_cleanup(curl);
    }
  }

  CURL* get() const {
    return curl;
  }

private:
  CURL* curl = nullptr;
};

#endif // _CURL_CURL_PTR_H_
