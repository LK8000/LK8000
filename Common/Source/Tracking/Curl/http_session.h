/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   http_session.h
 * Author: Bruno de Lacheisserie
 *
 * Created on February 18, 2024
 */
#ifndef _TRACKING_CURL_HTTP_SESSION_H_
#define _TRACKING_CURL_HTTP_SESSION_H_

#include <string>
#include "curl_ptr.h"

class http_session {
public:
  http_session();
  ~http_session();

  static bool ssl_available();

  // returns received string, empty string if transaction failed
  std::string get(const std::string& url) const;

  std::string post(const std::string& url, const std::string& data, const char* content_type = nullptr) const;

  // for compatibility with legacy code
  std::string request(const char* server_name, int server_port, const char* query_string) const;

  std::string request(const std::string& url) const {
    return get(url);
  }

private:
  std::string request_impl(const std::string& url, const std::string* post_data, const char* content_type) const;
  curl_ptr curl;
};

#endif // _TRACKING_CURL_HTTP_SESSION_H_
