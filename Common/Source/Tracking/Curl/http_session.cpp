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
#include "../../../Header/Defines.h"

http_session::http_session() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

http_session::~http_session() {
  curl_global_cleanup();
}

namespace {

size_t data_write_to_string(void* buf, size_t size, size_t nmemb, void* userp) {
  if(userp) {
    std::string& data = *static_cast<std::string*>(userp);
    data.append(static_cast<char*>(buf), size * nmemb);
    return size * nmemb;
  }
  return 0;
}

bool curl_version_ssl() {
  curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);
  return data->features & CURL_VERSION_SSL;
}

} // namespace

bool http_session::ssl_available() {
  static bool ssl = curl_version_ssl(); // thread safe since C++11
  return ssl;
}

std::string http_session::request_impl(const std::string& url, const std::string* post_data, const char* content_type) const {
  static constexpr char protocols[] = "http,https";
  try {
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);

    struct curl_slist* headers = nullptr;
    if (post_data) {
        curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, post_data->c_str());
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, static_cast<long>(post_data->size()));
        if (content_type) {
            std::string ct = "Content-Type: ";
            ct += content_type;
            headers = curl_slist_append(headers, ct.c_str());
        }
    } else {
        curl_easy_setopt(curl.get(), CURLOPT_HTTPGET, 1L);
    }
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);
    
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_PROTOCOLS_STR, protocols);
    curl_easy_setopt(curl.get(), CURLOPT_REDIR_PROTOCOLS_STR, protocols);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, LKFORK "/" LKVERSION "." LKRELEASE);
    curl_easy_setopt(curl.get(), CURLOPT_MAXREDIRS, 5L);

#ifndef NDEBUG
    curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);
#endif

#ifdef KOBO
    /* no TLS certificate validation because Kobos usually don't
	   have the correct date/time in the real-time clock, which
	   causes the certificate validation to fail */
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
#endif

    std::string response;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, data_write_to_string);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl.get());
    if (headers) {
        curl_slist_free_all(headers);
        curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, nullptr);
    }
    if (res == CURLE_OK) {
      return response;
    }
  }
  catch (std::exception& e) {
    fprintf(stderr, "http_session : %s\n", e.what());
  }
  return {};
}

std::string http_session::get(const std::string& url) const {
  return request_impl(url, nullptr, nullptr);
}

std::string http_session::post(const std::string& url, const std::string& data, const char* content_type) const {
  return request_impl(url, &data, content_type);
}

std::string http_session::request(const char* server_name, int server_port, const char* query_string) const {
  std::string url = (server_port == 443) ? "https://" : "http://";
  url += server_name;
  url += ":";
  url += std::to_string(server_port);
  url += query_string;
  return http_session::request(url);
}
