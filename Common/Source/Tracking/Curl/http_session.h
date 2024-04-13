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

  // Do a transaction with server
  // returns received string, empty string if transaction failed
  std::string request(const char* server_name, int server_port, const char* query_string) const;

  std::string request(const std::string& url) const;

private:
  curl_ptr curl;
};

#endif // _TRACKING_CURL_HTTP_SESSION_H_
