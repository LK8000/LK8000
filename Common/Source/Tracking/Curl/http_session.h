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

#include "../http_session_base.h"
#include "curl_ptr.h"

class http_session : public http_session_base<http_session> {
  friend class http_session_base<http_session>;

 public:
  using base = http_session_base<http_session>;
  using optional_string = base::optional_string;

  http_session();
  ~http_session();

 private:
  static bool ssl_available_impl();

  std::string request_impl(const std::string& url,
                          const optional_string& post_data,
                          const optional_string& content_type) const;

  curl_ptr curl;

  mutable std::string last_error; // to avoid spamming logs with the same error
};

#endif // _TRACKING_CURL_HTTP_SESSION_H_
