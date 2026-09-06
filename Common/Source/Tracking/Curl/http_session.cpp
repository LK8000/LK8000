/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   http_session.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on February 18, 2024
 */
#include "http_session.h"
#include <stdexcept>
#include <format>
#include "../../../Header/Defines.h"

#ifdef ANDROID
#include <android/log.h>
#endif

http_session::http_session() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

http_session::~http_session() {
  curl_global_cleanup();
}

namespace {

size_t data_write_to_string(void* buf, size_t size, size_t nmemb, void* userp) {
  if(userp) {
    auto& data = *static_cast<std::string*>(userp);

    using char_type = std::string::traits_type::char_type;
    size_t char_size = sizeof(char_type);
    size_t string_size = (size * nmemb) / char_size;
    data.append(static_cast<char_type*>(buf), string_size);
    return string_size;
  }
  return 0;
}

bool curl_version_ssl() {
  curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);
  return data->features & CURL_VERSION_SSL;
}

} // namespace

bool http_session::ssl_available_impl() {
  static bool ssl = curl_version_ssl(); // thread safe since C++11
  return ssl;
}

std::string http_session::request_impl(const std::string& url, const optional_string& post_data, const optional_string& content_type) const {
  static constexpr char protocols[] = "http,https";
  try {
    curl.setopt(CURLOPT_URL, url.c_str());
    curl.setopt(CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);

    struct curl_slist* headers = nullptr;
    if (post_data) {
        curl.setopt(CURLOPT_POST, 1L);
        curl.setopt(CURLOPT_POSTFIELDS, post_data->c_str());
        curl.setopt(CURLOPT_POSTFIELDSIZE, static_cast<long>(post_data->size()));
        if (content_type) {
            std::string ct = "Content-Type: ";
            ct += *content_type;
            headers = curl_slist_append(headers, ct.c_str());
        }
    } else {
        curl.setopt(CURLOPT_HTTPGET, 1L);
    }
    curl.setopt(CURLOPT_HTTPHEADER, headers);
    
    curl.setopt(CURLOPT_FOLLOWLOCATION, 1L);
    curl.setopt(CURLOPT_PROTOCOLS_STR, protocols);
    curl.setopt(CURLOPT_REDIR_PROTOCOLS_STR, protocols);
    curl.setopt(CURLOPT_USERAGENT, LKFORK "/" LKVERSION "." LKRELEASE);
    curl.setopt(CURLOPT_MAXREDIRS, 5L);

#ifndef NDEBUG
    curl.setopt(CURLOPT_VERBOSE, 1L);
#endif

#ifdef KOBO
    /* no TLS certificate validation because Kobos usually don't
	   have the correct date/time in the real-time clock, which
	   causes the certificate validation to fail */
    curl.setopt(CURLOPT_SSL_VERIFYPEER, 0L);
#endif

    std::string response;
    curl.setopt(CURLOPT_WRITEFUNCTION, data_write_to_string);
    curl.setopt(CURLOPT_WRITEDATA, &response);

    // Set timeout to prevent indefinite hangs during shutdown
    curl.setopt(CURLOPT_TIMEOUT, 30L);  // 30 seconds total timeout
    curl.setopt(CURLOPT_CONNECTTIMEOUT, 10L);  // 10 seconds connection timeout

    CURLcode res = curl.perform();

    if (headers) {
      curl.setopt(CURLOPT_HTTPHEADER, nullptr);
      curl_slist_free_all(headers);
    }

    if (res != CURLE_OK) {
      throw std::runtime_error(curl_easy_strerror(res));
    }

    return response;
  }
  catch (std::exception& e) {
    if (last_error != e.what()) {
      last_error = e.what();
      auto error = std::format("request failed: <{}> : {}\n", url, e.what());
#ifdef ANDROID
      __android_log_print(ANDROID_LOG_ERROR, "LK8000", "%s", error.c_str());
#else
      fprintf(stderr, "%s", error.c_str());
#endif
    }
  }
  return {};
}
